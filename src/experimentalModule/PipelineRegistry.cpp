#include "PipelineFamily.hpp"
#include "Renderer.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/ComputePipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp" // SUN_HALO service uniforms (row 14)
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "tools/s_texture.hpp"
#include "tools/call_system.hpp" // SUN_HALO: setBigHalo path-existence probe
#include "tools/draw_helper.hpp" // tail pass: helper->nextDraw segment boundary
#include "tools/object_base.hpp" // pointer service: getFontResolution (the
                                 // SCK_FONT_RESOLUTION_SIZE config authority)
#include "bodyModule/hints.hpp"  // hint service: computeHintsAt + nbrFacets
                                 // (single authority on the circle shape)
#include "ModularBody.hpp"       // pointer service: viewportRadius, deltaTime
#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <deque>

// ============================================================================
// Pipeline-family registry - implementation of the Renderer registry surface
// (contract: PipelineFamily.hpp + Renderer.hpp; design: INTENT.md 10.3,
// converged 2026-07-12 at plan approval).
//
// INTERIM WORK DOMAIN (explicit divergence, reconciled in INTENT.md 12):
// the §8/G5 work domain (Taskable pools) is the S4 track and does not exist
// yet. Lazy variant builds therefore run on a dedicated builder thread here,
// and publication is an atomic ready-flag (release-store by the builder,
// acquire-load in bind()) instead of a render-chain task. This preserves the
// observable contract (bind never blocks, base always resident, builds off
// the frame path - C3) with one machinery divergence: publication ordering
// is per-slot instead of chain-serialized, which is sufficient while slots
// are write-once (built exactly once, never swapped). The S4 port replaces
// the queue+thread with work-domain tasks and the ready-flag with a publish
// task; config-driven REBUILDS (publish-swap of an already-ready slot) must
// NOT be implemented on this interim - they need the chain ordering.
// Precedent for concurrent pipeline builds: the 4-thread shadow pipeline
// build + pipelineCache concurrent use (context.cpp, INTENT.md 10.3.3).
//
// Entry lifetime: entries are application-lifetime; release() only
// decrements the refcount and reclamation is deferred (dedup by name means a
// re-allocation reuses the entry; a handle can never dangle). This is the
// conservative superset of the header's "lifetime = union of live handles" -
// revisit only if registry pressure ever materializes (uint8 index space,
// 255 entries).
// ============================================================================

namespace {

const char *passName(PassKind pass) {
    switch (pass) {
        case PassKind::COLOR: return "COLOR";
        case PassKind::SELF_SHADOW: return "SELF_SHADOW";
        case PassKind::SHADOW_SHAPE: return "SHADOW_SHAPE";
        case PassKind::TRACE: return "TRACE";
        default: return "?";
    }
}

struct VariantSlot {
    VariantSlot(VariantKey key) : key(key) {}
    VariantKey key;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<ComputePipeline> compute; // COMPUTE families (integer-keyed)
    std::atomic<bool> ready{false}; // interim publish - see file header
};

struct PassEntry {
    const PassDesc *desc = nullptr; // into FamilyEntry::desc.passes (stable:
                                    // desc moved once, entry never relocates)
    std::deque<VariantSlot> variants; // [0] = base, ready at allocation;
                                      // appended by bind() (frame task) only,
                                      // deque = stable addresses for builder
};

// Batching service state of one batched family (BatchDesc engaged) - the
// generalized portage of the old HaloContext (halo.cpp): persistent staging
// ring ping-ponged in two halves per frame, device vertex buffer mirroring
// it, flushes recorded at the per-body command-buffer boundaries.
struct BatchState {
    std::unique_ptr<VertexBuffer> vertex;
    SubBuffer staging {};
    char *pData = nullptr;
    uint32_t initialOffset = 0; // start of this frame's half (old scheme kept:
    uint32_t offset = 0;        // 2 halves, alternated at batchEnd)
    uint32_t size = 0;          // pending (unflushed) instances
    uint16_t stride;
    uint32_t capacity;          // instances per half (BatchDesc contract)
    bool overflowLogged = false;
    // Service-side per-family resources (halo: the batch texture; a family
    // without one leaves them empty). Generalize when the second textured
    // batched family (HINT/TAIL) lands.
    std::unique_ptr<s_texture> tex;
    s_texture *boundTex = nullptr;
    std::unique_ptr<Set> set;
};

struct FamilyEntry {
    PipelineFamilyDesc desc;
    std::unique_ptr<PipelineLayout> layout;
    PassEntry passes[static_cast<size_t>(PassKind::NB_PASS_KIND)];
    std::unique_ptr<BatchState> batch; // engaged iff desc.batch
    VariantKey axisMask = 0;      // union of declared axis bits
    VariantKey shaderSwapMask = 0; // bits selecting shaderTable entries
    VariantKey undefinedMask = 0;  // STATE_OVERRIDE bits (payload [open] in
                                   // the contract): declared but not yet
                                   // providable - masked out of every request
    uint16_t refs = 0;
    bool misroutedLogged[static_cast<size_t>(PassKind::NB_PASS_KIND)] = {};
    // Pass declared but disabled at allocation (base build failed - shader
    // absent/broken). Distinguished so the first-bind log names the REAL
    // cause instead of a phantom trait-routing defect (the failure must
    // self-name - a "bound in undeclared pass" message sent the diagnosis
    // toward trait routing when the cause was an undeployed shader file).
    bool disabled[static_cast<size_t>(PassKind::NB_PASS_KIND)] = {};
};

struct SetContractEntry {
    SetContractDesc desc;
    PipelineLayout *layout = nullptr; // owned unless external
    std::unique_ptr<PipelineLayout> owned;
    bool external = false; // globalUboContract: layout owned by UBOCam
                           // (context.layouts.front())
    uint16_t refs = 0;
};

struct BuildJob {
    FamilyEntry *family;
    PassKind pass;
    VariantSlot *slot;
};

struct Registry {
    std::deque<SetContractEntry> contracts;
    std::deque<FamilyEntry> families;
    // VertexArrays of Renderer-owned service families (halo, later hint/
    // trace): the family desc references them (I5), so they live here -
    // released by releaseRegistry() while the buffer managers are alive.
    std::vector<std::unique_ptr<VertexArray>> servicePatterns;
    // Descriptor-pool aggregation (INTENT 10.3 decision 5): pools are sized
    // from the contracts they serve. Counters aggregate arraySize *
    // expectedSets per descriptor type over all allocated contracts.
    uint32_t aggSets = 0;
    uint32_t aggUniform = 0, aggDynUniform = 0, aggTexture = 0;
    uint32_t aggStorageBuf = 0, aggStorageImg = 0, aggSampledImg = 0;
    std::vector<std::unique_ptr<SetMgr>> pools;
    uint32_t poolRemaining = 0; // set-count budget left on pools.back()
    // Descriptor types pools.back() was CREATED with. The aggregate grows as
    // contracts are allocated - a pool made before a contract introduced a
    // type cannot serve that contract (found live: the S1-era pool had no
    // SAMPLED_IMAGE; the S5 blur contract added it; allocSet handed out the
    // old pool -> AllocateDescriptorSets-WrongType). Coverage is re-checked
    // per allocSet, not assumed from creation order.
    uint32_t poolTypeMask = 0;
    // ...and the per-type COUNTS pools.back() was created with. Same defect
    // class as poolTypeMask, one dimension over (B12, INTENT §11.123): the
    // mask says the pool KNOWS a type, never that it has ENOUGH of it. Pools
    // are sized from the aggregate AT CREATION, so the first contract to
    // allocate a set fixes the first pool's per-type budget - and the first
    // body loaded is the Sun, whose STAR_SURFACE contract wants ONE texture.
    // Every later 6-texture layered set then came out of a 4-texture pool:
    // measured as WARNING-VkDescriptorSetAllocateInfo-descriptorCount
    // ("allocate 6 ... this pool only has 4 ... will fail on others") - a
    // portability failure, not a warning to tolerate. Checked per allocSet
    // exactly like the type mask.
    uint32_t poolCapUniform = 0, poolCapDynUniform = 0, poolCapTexture = 0;
    uint32_t poolCapStorageBuf = 0, poolCapStorageImg = 0, poolCapSampledImg = 0;
    // Interim work domain (see file header).
    std::mutex mtx;
    std::condition_variable cv;
    std::deque<BuildJob> jobs;
    std::thread builder;
    bool run = false;

    void enqueue(const BuildJob &job) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            jobs.push_back(job);
            if (!run) {
                run = true;
                builder = std::thread(&Registry::builderLoop, this);
            }
        }
        cv.notify_one();
    }
    void builderLoop();
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!run)
                return;
            run = false;
        }
        cv.notify_one();
        builder.join();
    }
    ~Registry() {
        stop();
        // FamilyEntry batch Sets are pool-allocated temporaries (allocSet:
        // new Set(..., *pools.back(), ..., temporary=true)); ~Set -> uninit ->
        // mgr.destroySet() dereferences that SetMgr. `pools` is declared AFTER
        // `families`, so reverse-declaration member teardown destroys the pools
        // FIRST -> the batch Sets then call destroySet() on a dead SetMgr
        // (found live: shutdown SIGSEGV in SetMgr::destroySet, garbage handle).
        // Tear families down here, while their pools are still alive - the same
        // invariant releaseRegistry() already keeps for the Renderer-owned pool
        // Sets (pointerSet/shadow), applied to the batch Sets it cannot reach
        // because they live inside `families`.
        families.clear();
    }
};

std::unique_ptr<Registry> reg;

Registry &registry()
{
    if (!reg)
        reg = std::make_unique<Registry>();
    return *reg;
}

// ---- TAIL instanced batch (row 12) --------------------------------------
// Renderer-owned replacement for the old Tail::global singleton (tail.cpp):
// shared cone/strip geometry + a primitive-restart index built once, per-tail
// instances (Renderer::TailInstance) accumulated per frame and drawn with ONE
// vkCmdDrawIndexed. The PIPELINE lives in the registry (a normal family, so
// the C3 degradation + spec-8 injection are free); only the batch RESOURCES
// are file-static here - the same division the halo/hint families keep (handle
// on the Renderer, buffers off the frame path). The GPU buffers are released
// in Renderer::releaseRegistry() (managers still alive), NOT at static
// teardown (that is the shutdown-SIGSEGV class, §11.15d). Geometry constants
// are the old tail.cpp macros verbatim.
namespace {
constexpr int NB_MAX_TAILS = 1024;
constexpr int NB_TAIL_LINES = 16;
constexpr int NB_TAIL_LENGTH = Renderer::TAIL_TIME_SEGMENTS; // 16
constexpr int NB_TAIL_HEAD = 1 + NB_TAIL_LINES * NB_TAIL_LINES / 4; // 65
constexpr int NB_TAIL_LINE_INDICES = (1 + NB_TAIL_LINES / 4 + NB_TAIL_LENGTH) * 2; // 42
constexpr int NB_TAIL_VERTICES = NB_TAIL_HEAD + NB_TAIL_LINES * NB_TAIL_LENGTH; // 321
constexpr int NB_TAIL_INDICES = NB_TAIL_LINES * NB_TAIL_LINE_INDICES; // 672
static_assert(sizeof(Renderer::TailInstance) == 24 * sizeof(float),
              "TailInstance must match body_tail.vert instance inputs (8 vec3)");

struct TailVertex { Vec3f normal; float timeOffset; };

struct TailBatch {
    PipelineFamily family;                  // registry-owned pipeline
    std::unique_ptr<VertexBuffer> geometry; // binding 0 (vertex rate), built once
    std::unique_ptr<VertexBuffer> instance; // binding 1 (instance rate), per-frame
    SubBuffer index {};                     // shared primitive-restart index
    std::vector<Renderer::TailInstance> data; // this frame's instances
    float fov = 0;                          // push constant (half-fov radians)
    bool built = false;                     // family + buffers created
    bool geometryUploaded = false;          // static geometry/index copied
    bool overflowLogged = false;
};
TailBatch tailBatch;
} // namespace

// Build one pipeline variant of a family pass. Runs on the registration
// path (base variants, synchronous) or the builder thread (lazy variants) -
// reads only immutable-after-allocation state (FamilyEntry::desc, layout).
std::unique_ptr<Pipeline> buildVariant(FamilyEntry &f, PassKind pass, VariantKey key)
{
    Context &context = *Context::instance;
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    RenderMgr *render = nullptr;
    int subpass = 0;
    std::vector<VkDynamicState> dynamicStates;
    switch (pass) {
        case PassKind::COLOR:
        case PassKind::TRACE:
            // One subpass carries all body-path drawing (INTENT 10.3.1).
            render = context.render.get();
            subpass = PASS_MULTISAMPLE_DEPTH;
            break;
        case PassKind::SELF_SHADOW:
            // Header profile: depth GREATER, dynamic viewport (variable-size
            // targets). Validated against the old shadowTrace pipeline
            // (bodyShader.cpp:410-426) at S5 - state profile matches; old
            // additionally set setFrontFace(), carried by the family's
            // reverseFrontFace when its first client lands.
            render = context.renderSelfShadow.get();
            dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            break;
        case PassKind::SHADOW_SHAPE:
            // Dynamic viewport: the old shadowShape pipeline baked
            // frameShadow->makeViewport() (shadowRes^2) - a fixed viewport at
            // SCREEN size here would rasterize the silhouette wrong. The
            // recording service sets viewport/scissor at pass begin
            // (resolution is a D5 parameter, not a bake-time constant).
            // Target: the R8 coverage pass (typed silhouettes - the family's
            // FixedState carries the coverage-over blend; the old stencil
            // REPLACE ops are gone with the binary target).
            render = context.renderShadowShape.get();
            dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            break;
        default:
            return nullptr;
    }
    const PassDesc &pd = *f.passes[static_cast<size_t>(pass)].desc;
    const FixedState &st = pd.state;
    auto p = std::make_unique<Pipeline>(vkmgr, *render, subpass, f.layout.get(), dynamicStates);
    p->setCullMode(st.cull);
    if (st.reverseFrontFace)
        p->setFrontFace();
    p->setBlendMode(st.blend);
    if (st.patchControlPoints) {
        p->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
        p->setTessellationState(st.patchControlPoints);
    } else {
        p->setTopology(st.topology, st.stripBreaks);
    }
    bool depthTest = st.depthTest;
    bool depthWrite = st.depthWrite;
    if (key & VARIANT_NO_DEPTH)
        depthTest = depthWrite = false;
    switch (pass) {
        case PassKind::SELF_SHADOW:
            p->setDepthStencilMode(VK_TRUE, VK_TRUE, VK_COMPARE_OP_GREATER);
            break;
        case PassKind::SHADOW_SHAPE:
            // Depth explicitly off; coverage is written as COLOR (the
            // family's frag + blend state), not stencil - the typed R8
            // target carries graded transmission, which a stencil cannot.
            p->setDepthStencilMode();
            break;
        default:
            if (!depthTest || !depthWrite)
                p->setDepthStencilMode(depthTest ? VK_TRUE : VK_FALSE, depthWrite ? VK_TRUE : VK_FALSE);
            break;
    }
    if (st.lineWidth)
        p->setLineWidth(st.lineWidth);
    if (f.desc.vertex) {
        p->bindVertex(*f.desc.vertex);
        for (uint8_t i = 0; i < 8; ++i) {
            if (st.removedVertexEntries & (1 << i))
                p->removeVertexEntry(i);
        }
    }
    // Shader row: SHADER_SWAP bits select it; resolve() guarantees the row
    // exists for any key that reaches here.
    const ShaderSet *shaders = nullptr;
    const VariantKey swapBits = key & f.shaderSwapMask;
    for (const auto &row : pd.shaderTable) {
        if (row.shaderBits == swapBits) {
            shaders = &row.shaders;
            break;
        }
    }
    if (!shaders) {
        vkmgr.putLog("PipelineRegistry: no shader row for '" + f.desc.name + "' " + passName(pass) + " variant " + std::to_string(key), LogType::ERROR);
        return nullptr;
    }
    // Specialization constants are applied to every stage; entries a stage's
    // SPIR-V doesn't declare are ignored by specification, which keeps this
    // uniform (the old path applied them selectively per stage).
    auto bindStage = [&p, &f, key](const std::string &file) {
        if (file.empty())
            return;
        p->bindShader(file);
        // Global projection mode (INTENT 11.33): spec-const 8 is the
        // custom_project.glsl dispatch - registry-injected so EVERY family
        // inherits the mainline multi-mode mechanism without declaring it
        // (single authority; a family supplying id 8 itself wins - skip).
        // Launch-constant (Context::projectionType, config-parsed at App
        // init - the old path's 52 per-pipeline sites share the precondition);
        // stages whose SPIR-V doesn't declare id 8 ignore the entry.
        if (std::none_of(f.desc.specValues.begin(), f.desc.specValues.end(),
                         [](const SpecConstant &sv) { return sv.constantId == 8; })) {
            const uint32_t mode = static_cast<uint32_t>(Context::projectionType);
            p->setSpecializedConstant(8, &mode, sizeof(mode)); // copied (memcpy) at call
        }
        for (const auto &sv : f.desc.specValues)
            p->setSpecializedConstant(sv.constantId, &sv.value, sizeof(sv.value));
        for (const auto &axis : f.desc.axes) {
            if (axis.effect == VariantEffect::SPEC_CONSTANT) {
                const VkBool32 v = (key & axis.bit) ? VK_TRUE : VK_FALSE;
                p->setSpecializedConstant(axis.specConstantId, &v, sizeof(v));
            }
        }
    };
    bindStage(shaders->vert);
    bindStage(shaders->tesc);
    bindStage(shaders->tese);
    bindStage(shaders->geom);
    bindStage(shaders->frag);
    // Stable name = stable pipelineCache identity across runs.
    char name[96];
    snprintf(name, sizeof(name), "%s#%s+%04x", f.desc.name.c_str(), passName(pass), key);
    p->build(name);
    return p;
}

// COMPUTE bank variant: the integer key rides specialization constant 0
// (PipelineFamilyDesc contract); desc.specValues supplies the rest.
std::unique_ptr<ComputePipeline> buildComputeVariant(FamilyEntry &f, VariantKey key)
{
    auto p = std::make_unique<ComputePipeline>(*VulkanMgr::instance, f.layout.get());
    p->bindShader(f.desc.computeShader);
    const uint32_t keyValue = key;
    p->setSpecializedConstant(0, keyValue);
    for (const auto &sv : f.desc.specValues)
        p->setSpecializedConstant(sv.constantId, &sv.value, sizeof(sv.value));
    p->build();
    return p;
}

void Registry::builderLoop()
{
    std::unique_lock<std::mutex> lock(mtx);
    while (true) {
        cv.wait(lock, [this]{ return !jobs.empty() || !run; });
        if (jobs.empty() && !run)
            return;
        BuildJob job = jobs.front();
        jobs.pop_front();
        lock.unlock();
        if (job.family->desc.kind == PipelineFamilyDesc::Kind::COMPUTE) {
            auto pipeline = buildComputeVariant(*job.family, job.slot->key);
            if (pipeline && pipeline->get() != VK_NULL_HANDLE) {
                job.slot->compute = std::move(pipeline);
                job.slot->ready.store(true, std::memory_order_release);
            }
        } else {
            auto pipeline = buildVariant(*job.family, job.pass, job.slot->key);
            if (pipeline && pipeline->get() != VK_NULL_HANDLE) {
                job.slot->pipeline = std::move(pipeline);
                job.slot->ready.store(true, std::memory_order_release);
            }
        } // failure stays non-resident: bind() keeps falling back to base
          // (graphics) or the dispatch is skipped (compute); the build error
          // is already logged at its definition site.
        lock.lock();
    }
}

// Drop one variant bit for fallback: the reserved NO_DEPTH bit drops first
// (a forced-depth fallback is the old ringed-body behavior), then declared
// axes by dropPriority - HIGHER drops first (VariantAxis contract).
VariantKey dropOneBit(const FamilyEntry &f, VariantKey key)
{
    if (key & VARIANT_NO_DEPTH)
        return key & ~VARIANT_NO_DEPTH;
    const VariantAxis *victim = nullptr;
    for (const auto &axis : f.desc.axes) {
        if ((key & axis.bit) && (!victim || axis.dropPriority > victim->dropPriority))
            victim = &axis;
    }
    return victim ? key & ~victim->bit : 0;
}

// Largest providable key <= wanted: a key is providable when its SHADER_SWAP
// subset matches a shaderTable row (entry 0 covers 0, so this terminates).
VariantKey providableKey(const FamilyEntry &f, const PassDesc &pd, VariantKey key)
{
    while (true) {
        const VariantKey swapBits = key & f.shaderSwapMask;
        for (const auto &row : pd.shaderTable) {
            if (row.shaderBits == swapBits)
                return key;
        }
        key = dropOneBit(f, key);
    }
}

VariantSlot *findSlot(PassEntry &pe, VariantKey key)
{
    for (auto &slot : pe.variants) {
        if (slot.key == key)
            return &slot;
    }
    return nullptr;
}

// Core of bind(): resolve the best resident variant for (family, pass),
// enqueue missing builds, bind the pipeline. Shared by Renderer::bind (module
// path) and the batching service flush.
FamilyBound resolveAndBind(Registry &r, FamilyEntry &f, PassKind pass, VkCommandBuffer cmd, VariantKey wanted)
{
    PassEntry &pe = f.passes[static_cast<size_t>(pass)];
    if (!pe.desc) {
        // Either a trait-routing defect (hooks are only invoked within their
        // matching pass kind) or a pass disabled at allocation (base build
        // failed). Log once per (family, pass) with the real cause, no-op.
        if (!f.misroutedLogged[static_cast<size_t>(pass)]) {
            f.misroutedLogged[static_cast<size_t>(pass)] = true;
            if (f.disabled[static_cast<size_t>(pass)])
                VulkanMgr::instance->putLog("PipelineRegistry: family '" + f.desc.name + "' pass " + passName(pass) + " is DISABLED (base build failed - check the shader files named at allocation); drawing skipped", LogType::ERROR);
            else
                VulkanMgr::instance->putLog("PipelineRegistry: family '" + f.desc.name + "' bound in undeclared pass " + passName(pass), LogType::ERROR);
        }
        return {nullptr, 0};
    }
    // Normalize: undeclared/undefined bits are silently non-providable;
    // NO_DEPTH is COLOR-only (contract).
    wanted &= (f.axisMask & ~f.undefinedMask)
            | ((pass == PassKind::COLOR) ? VARIANT_NO_DEPTH : 0);
    VariantSlot *bound = &pe.variants.front(); // base: always resident
    while (true) {
        const VariantKey key = providableKey(f, *pe.desc, wanted);
        VariantSlot *slot = findSlot(pe, key);
        if (slot) {
            if (slot->ready.load(std::memory_order_acquire)) {
                bound = slot;
                break;
            } // else: queued, not resident yet - fall back further
        } else {
            pe.variants.emplace_back(key);
            r.enqueue({&f, pass, &pe.variants.back()});
        }
        if (key == 0)
            break; // base bound (slot[0] is ready by construction)
        wanted = dropOneBit(f, key);
    }
    bound->pipeline->bind(cmd);
    return {f.layout.get(), bound->key};
}

// Bit per supported descriptor type (pool-coverage tracking).
uint32_t typeBit(VkDescriptorType type)
{
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return 0x01;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return 0x02;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return 0x04;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return 0x08;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return 0x10;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return 0x20;
        default: return 0;
    }
}

void createPool(Registry &r)
{
    // Each pool is sized from the CURRENT aggregate of allocated contracts -
    // it always covers the layouts it serves (INTENT 10.3 decision 5 /
    // 11.1 structural fix; coverage re-checked per allocSet since the
    // aggregate grows - see poolTypeMask). Growth = one more aggregate-sized
    // pool. expectedSets is the sizing contract: a client exceeding it
    // exhausts the budget visibly (allocation failure at a named site),
    // never silently.
    const uint32_t sets = std::max(r.aggSets, 16u);
    r.pools.push_back(std::make_unique<SetMgr>(*VulkanMgr::instance, sets,
        r.aggUniform, r.aggTexture, r.aggStorageBuf, r.aggStorageImg,
        true, r.aggDynUniform, r.aggSampledImg));
    r.poolRemaining = sets;
    r.poolTypeMask = (r.aggUniform ? 0x01 : 0) | (r.aggDynUniform ? 0x02 : 0)
                   | (r.aggTexture ? 0x04 : 0) | (r.aggStorageBuf ? 0x08 : 0)
                   | (r.aggStorageImg ? 0x10 : 0) | (r.aggSampledImg ? 0x20 : 0);
    // What ONE set out of this pool may ask for, per type (see poolCap*).
    r.poolCapUniform = r.aggUniform;
    r.poolCapDynUniform = r.aggDynUniform;
    r.poolCapTexture = r.aggTexture;
    r.poolCapStorageBuf = r.aggStorageBuf;
    r.poolCapStorageImg = r.aggStorageImg;
    r.poolCapSampledImg = r.aggSampledImg;
}

// Per-SET descriptor need of a contract, by type (arraySize summed over the
// bindings of that type). The pool-coverage question is per set, not
// aggregate: vkAllocateDescriptorSets fails when ONE set exceeds what the pool
// holds, whatever the total budget says.
struct SetNeed {
    uint32_t uniform = 0, dynUniform = 0, texture = 0;
    uint32_t storageBuf = 0, storageImg = 0, sampledImg = 0;
};

SetNeed perSetNeed(const SetContractDesc &d)
{
    SetNeed n;
    for (const auto &b : d.bindings) {
        const uint32_t c = b.arraySize ? b.arraySize : 1;
        switch (b.type) {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: n.uniform += c; break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: n.dynUniform += c; break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: n.texture += c; break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: n.storageBuf += c; break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: n.storageImg += c; break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: n.sampledImg += c; break;
            default: break;
        }
    }
    return n;
}

} // namespace

// ---- RegistryHandle refcounting (registration domain only) -----------------

template <>
void RegistryHandle<SetContractTag>::acquire()
{
    if (idx != NONE && reg)
        ++reg->contracts[idx].refs;
}

template <>
void RegistryHandle<SetContractTag>::release()
{
    if (idx != NONE && reg)
        --reg->contracts[idx].refs;
}

template <>
void RegistryHandle<PipelineFamilyTag>::acquire()
{
    if (idx != NONE && reg)
        ++reg->families[idx].refs;
}

template <>
void RegistryHandle<PipelineFamilyTag>::release()
{
    if (idx != NONE && reg)
        --reg->families[idx].refs;
}

// ---- Renderer registry surface ---------------------------------------------

Renderer::~Renderer()
{
    reg.reset(); // joins the builder thread, destroys pipelines/layouts/pools
                 // while the device is alive (see header note)
}

SetContract Renderer::allocateSetContract(SetContractDesc &&desc)
{
    auto &r = registry();
    for (size_t i = 0; i < r.contracts.size(); ++i) {
        if (r.contracts[i].desc.name == desc.name) {
            ++r.contracts[i].refs;
            return SetContract(static_cast<uint8_t>(i));
        }
    }
    if (r.contracts.size() >= SetContract::NONE) {
        VulkanMgr::instance->putLog("PipelineRegistry: set-contract registry full", LogType::ERROR);
        return {};
    }
    r.contracts.emplace_back();
    auto &e = r.contracts.back();
    e.desc = std::move(desc);
    e.refs = 1;
    e.owned = std::make_unique<PipelineLayout>(*VulkanMgr::instance);
    e.layout = e.owned.get();
    for (const auto &b : e.desc.bindings) {
        const uint32_t count = static_cast<uint32_t>(b.arraySize) * e.desc.expectedSets;
        switch (b.type) {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                e.layout->setUniformLocation(b.stages, b.binding, b.arraySize, false);
                r.aggUniform += count;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                e.layout->setUniformLocation(b.stages, b.binding, b.arraySize, true);
                r.aggDynUniform += count;
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                if (b.arraySize > 1)
                    e.layout->setTextureArrayLocation(b.binding, b.arraySize, b.sampler ? &*b.sampler : &PipelineLayout::DEFAULT_SAMPLER, b.stages);
                else
                    e.layout->setTextureLocation(b.binding, b.sampler ? &*b.sampler : &PipelineLayout::DEFAULT_SAMPLER, b.stages);
                r.aggTexture += count;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                e.layout->setStorageBufferLocation(b.stages, b.binding, b.arraySize);
                r.aggStorageBuf += count;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                e.layout->setImageLocation(b.binding, b.stages, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
                r.aggStorageImg += count;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                e.layout->setImageLocation(b.binding, b.stages, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
                r.aggSampledImg += count;
                break;
            default:
                VulkanMgr::instance->putLog("PipelineRegistry: unsupported descriptor type in contract '" + e.desc.name + "'", LogType::ERROR);
                break;
        }
    }
    e.layout->buildLayout();
    r.aggSets += e.desc.expectedSets;
    return SetContract(static_cast<uint8_t>(r.contracts.size() - 1));
}

SetContract Renderer::globalUboContract() const
{
    auto &r = registry();
    for (size_t i = 0; i < r.contracts.size(); ++i) {
        if (r.contracts[i].external) {
            ++r.contracts[i].refs;
            return SetContract(static_cast<uint8_t>(i));
        }
    }
    if (Context::instance->layouts.empty()) {
        // Precondition: UBOCam inserted the global layout at layouts.front()
        // (ubo_cam.cpp) - family allocation happens at module-loading time,
        // after Core init; reaching this is an initialization-order defect.
        VulkanMgr::instance->putLog("PipelineRegistry: globalUboContract before UBOCam exists", LogType::ERROR);
        return {};
    }
    r.contracts.emplace_back();
    auto &e = r.contracts.back();
    e.desc.name = "__globalUBO";
    e.desc.expectedSets = 0; // the Set exists already (context.uboSet)
    e.layout = Context::instance->layouts.front().get(); // UBOCam's layout
    e.external = true;
    e.refs = 1;
    return SetContract(static_cast<uint8_t>(r.contracts.size() - 1));
}

PipelineFamily Renderer::allocateFamily(PipelineFamilyDesc &&desc)
{
    auto &r = registry();
    for (size_t i = 0; i < r.families.size(); ++i) {
        if (r.families[i].desc.name == desc.name) {
            ++r.families[i].refs;
            return PipelineFamily(static_cast<uint8_t>(i));
        }
    }
    if (r.families.size() >= PipelineFamily::NONE) {
        VulkanMgr::instance->putLog("PipelineRegistry: family registry full", LogType::ERROR);
        return {};
    }
    if (desc.kind == PipelineFamilyDesc::Kind::COMPUTE) {
        // COMPUTE bank (S5/G7, INTENT 10.3 open 4): integer-keyed variants,
        // key = spec constant 0. The bank lives in passes[0] (no PassKind
        // applies to compute; slot 0 is the container, desc stays null so
        // graphics bind() misroutes loudly if pointed here).
        r.families.emplace_back();
        FamilyEntry &f = r.families.back();
        f.desc = std::move(desc);
        f.refs = 1;
        f.layout = std::make_unique<PipelineLayout>(*VulkanMgr::instance);
        for (const auto &sc : f.desc.sets) {
            if (sc)
                f.layout->setGlobalPipelineLayout(r.contracts[sc.id()].layout);
            else
                VulkanMgr::instance->putLog("PipelineRegistry: null SetContract in family '" + f.desc.name + "'", LogType::ERROR);
        }
        for (const auto &pc : f.desc.pushConstants)
            f.layout->setPushConstant(pc.stages, pc.offset, pc.size);
        f.layout->build();
        if (f.desc.buildPolicy == PipelineFamilyDesc::BuildPolicy::EAGER_ASYNC_ALL) {
            PassEntry &pe = f.passes[0];
            for (VariantKey key = 1; key <= f.desc.eagerVariants; ++key) {
                pe.variants.emplace_back(key);
                r.enqueue({&f, PassKind::COLOR, &pe.variants.back()});
            }
        }
        return PipelineFamily(static_cast<uint8_t>(r.families.size() - 1));
    }
    r.families.emplace_back();
    FamilyEntry &f = r.families.back();
    f.desc = std::move(desc);
    f.refs = 1;
    for (const auto &axis : f.desc.axes) {
        if ((axis.bit & (axis.bit - 1)) || !axis.bit || (axis.bit & VARIANT_NO_DEPTH) || (f.axisMask & axis.bit)) {
            VulkanMgr::instance->putLog("PipelineRegistry: invalid/duplicate axis bit in family '" + f.desc.name + "' axis '" + axis.name + "'", LogType::ERROR);
            continue;
        }
        f.axisMask |= axis.bit;
        if (axis.effect == VariantEffect::SHADER_SWAP) {
            f.shaderSwapMask |= axis.bit;
        } else if (axis.effect == VariantEffect::STATE_OVERRIDE) {
            // Payload [open] in the contract - defined with its first client;
            // until then the bit is declared-but-not-providable.
            VulkanMgr::instance->putLog("PipelineRegistry: STATE_OVERRIDE payload undefined, axis '" + axis.name + "' of '" + f.desc.name + "' masked out", LogType::WARNING);
            f.undefinedMask |= axis.bit;
        }
    }
    f.layout = std::make_unique<PipelineLayout>(*VulkanMgr::instance);
    for (const auto &sc : f.desc.sets) {
        if (sc)
            f.layout->setGlobalPipelineLayout(r.contracts[sc.id()].layout);
        else
            VulkanMgr::instance->putLog("PipelineRegistry: null SetContract in family '" + f.desc.name + "'", LogType::ERROR);
    }
    for (const auto &pc : f.desc.pushConstants)
        f.layout->setPushConstant(pc.stages, pc.offset, pc.size);
    f.layout->build();
    for (const auto &pd : f.desc.passes) {
        if (pd.shaderTable.empty() || pd.shaderTable.front().shaderBits != 0) {
            VulkanMgr::instance->putLog("PipelineRegistry: family '" + f.desc.name + "' pass " + passName(pd.pass) + " lacks the base shader row (entry 0, shaderBits==0)", LogType::ERROR);
            continue;
        }
        PassEntry &pe = f.passes[static_cast<size_t>(pd.pass)];
        pe.desc = &pd; // stable: f.desc.passes buffer owned by the entry
        pe.variants.emplace_back(0);
        pe.variants.back().pipeline = buildVariant(f, pd.pass, 0); // base-sync
        if (!pe.variants.back().pipeline || pe.variants.back().pipeline->get() == VK_NULL_HANDLE) {
            // Base MUST be resident (base-always-ready, C3) - a family whose
            // base fails to build cannot honor bind(); disable the pass so
            // bind() reports the misroute instead of binding a null pipeline.
            VulkanMgr::instance->putLog("PipelineRegistry: BASE build failed for family '" + f.desc.name + "' pass " + passName(pd.pass) + " (shader '" + pd.shaderTable.front().shaders.vert + "'/'" + pd.shaderTable.front().shaders.frag + "') - pass disabled", LogType::ERROR);
            pe.desc = nullptr;
            pe.variants.clear();
            f.disabled[static_cast<size_t>(pd.pass)] = true;
            continue;
        }
        pe.variants.back().ready.store(true, std::memory_order_release);
    }
    if (f.desc.batch) {
        // Batching service (BatchDesc contract): staging ring in two halves
        // (the old HaloContext scheme - 3 frames in flight over 2 halves is
        // the shipped old-path guarantee, ported unchanged).
        Context &context = *Context::instance;
        f.batch = std::make_unique<BatchState>();
        f.batch->stride = f.desc.batch->instanceStride;
        f.batch->capacity = f.desc.batch->perFrameCapacity;
        f.batch->vertex = f.desc.vertex->createBuffer(0, f.batch->capacity * 2, context.ojmBufferMgr.get());
        f.batch->staging = context.stagingMgr->acquireBuffer(f.batch->vertex->get().size);
        f.batch->pData = static_cast<char *>(context.stagingMgr->getPtr(f.batch->staging));
    }
    return PipelineFamily(static_cast<uint8_t>(r.families.size() - 1));
}

void Renderer::batchPush(const PipelineFamily &family, const void *instance)
{
    FamilyEntry &fe = reg->families[family.id()];
    if (!fe.batch)
        return; // not a batched family (BatchDesc absent) - misuse, no-op
    BatchState &b = *fe.batch;
    if (b.offset + b.size >= b.capacity * 2) {
        // Overflow: drop the instance (the old path clamped the transfer and
        // warned about glitches; dropping is the strictly-safer variant).
        if (!b.overflowLogged) {
            b.overflowLogged = true;
            VulkanMgr::instance->putLog("Renderer batch '" + reg->families[family.id()].desc.name + "' overflow (capacity " + std::to_string(b.capacity) + "/half) - instances dropped this frame", LogType::WARNING);
        }
        return;
    }
    memcpy(b.pData + static_cast<size_t>(b.offset + b.size++) * b.stride, instance, b.stride);
}

void Renderer::batchBegin()
{
    if (!reg)
        return;
    for (auto &f : reg->families) {
        if (!f.batch)
            continue;
        BatchState &b = *f.batch;
        // Texture rebind on change (old Halo::beginDraw semantics).
        if (b.tex && b.boundTex != b.tex.get()) {
            b.set->bindTexture(b.tex->getTexture(), 0);
            b.boundTex = b.tex.get();
        }
    }
    // SUN_HALO (row 14) is a non-batch family: bind its big-halo texture here,
    // at frame start (after recordTransfer's upload), exactly once - same
    // deferral as the batched halo above. getTexture() in the frame task is the
    // established pattern (the halo binds the same way).
    if (sunHaloSet && sunHaloTex && !sunHaloTexBound) {
        sunHaloSet->bindTexture(sunHaloTex->getTexture(), 0);
        sunHaloTexBound = true;
    }
}

void Renderer::batchFlush()
{
    if (!reg)
        return;
    for (auto &f : reg->families) {
        if (!f.batch || !f.batch->size)
            continue;
        BatchState &b = *f.batch;
        if (b.set && !b.boundTex)
            continue; // texture set exists but nothing bound yet - drawing
                      // would bind an unwritten descriptor (old: !tex_halo
                      // -> no draw; drawHalo also refuses to queue then)
        const FamilyBound bound = resolveAndBind(*reg, f, passKind, cmd, 0);
        if (!bound.layout)
            continue;
        std::vector<VkDescriptorSet> sets {*Context::instance->uboSet->get()};
        if (b.set)
            sets.push_back(*b.set->get());
        bound.layout->bindSets(cmd, sets);
        b.vertex->bind(cmd);
        vkCmdDraw(cmd, b.size, 1, b.offset, 0);
        b.offset += b.size;
        b.size = 0;
    }
}

void Renderer::batchEnd()
{
    if (!reg)
        return;
    Context &context = *Context::instance;
    for (auto &f : reg->families) {
        if (!f.batch)
            continue;
        BatchState &b = *f.batch;
        // Unflushed remainder would mean content queued outside the body-draw
        // window; endBodyDraw flushes before ending the frame, so this is 0.
        b.size = 0;
        b.overflowLogged = false;
        const int sizeBytes = (b.offset - b.initialOffset) * b.stride;
        const int offsetBytes = b.initialOffset * b.stride;
        b.initialOffset = b.initialOffset ? 0 : b.offset; // ping-pong (old endDraw)
        b.offset = b.initialOffset;
        if (sizeBytes)
            context.transfer->planCopyBetween(b.staging, b.vertex->get(), sizeBytes, offsetBytes, offsetBytes);
    }
}

void Renderer::ensureHaloFamily()
{
    if (haloFamily)
        return;
    auto &r = registry();
    // Halo instance layout: {vec2 pos, vec3 color, float rmag} = 24 bytes -
    // must match body_halo.vert inputs and HALO_STRIDE (halo.cpp port).
    auto pattern = std::make_unique<VertexArray>(*VulkanMgr::instance, Context::instance->ojmAlignment);
    pattern->createBindingEntry(6 * sizeof(float));
    pattern->addInput(VK_FORMAT_R32G32_SFLOAT);      // pos
    pattern->addInput(VK_FORMAT_R32G32B32_SFLOAT);   // color
    pattern->addInput(VK_FORMAT_R32_SFLOAT);         // rmag
    SetContractDesc texContract;
    texContract.name = "haloTex";
    texContract.bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}};
    texContract.expectedSets = 1;
    PipelineFamilyDesc desc;
    desc.name = "HALO";
    desc.vertex = pattern.get();
    desc.sets.push_back(globalUboContract());
    desc.sets.push_back(allocateSetContract(std::move(texContract)));
    PassDesc color;
    color.pass = PassKind::COLOR;
    color.shaderTable = {{0, {.vert = "body_halo.vert.spv", .geom = "body_halo.geom.spv", .frag = "body_halo.frag.spv"}}};
    color.state.blend = BLEND_ADD;
    color.state.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    color.state.cull = false;
    color.state.depthTest = false;  // halos are screen-space glow: inherently
    color.state.depthWrite = false; // depth-less (base state, not a variant)
    desc.passes.push_back(std::move(color));
    desc.batch = BatchDesc{6 * sizeof(float), 8192}; // 8192/half == old 16384 total
    r.servicePatterns.push_back(std::move(pattern));
    haloFamily = allocateFamily(std::move(desc));
    if (haloFamily)
        r.families[haloFamily.id()].batch->set.reset(allocSet(haloFamily, 1));
}

void Renderer::drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag)
{
    ensureHaloFamily(); // in-frame first-call fallback; normally built at the
                        // setHaloTexture seam (startup), C3-clean
    if (!haloFamily)
        return;
    BatchState &b = *reg->families[haloFamily.id()].batch;
    if (!b.tex)
        return; // old semantics: no halo texture -> nothing queued
    struct {
        std::pair<float, float> pos;
        Vec3f color;
        float rmag;
    } data {VulkanMgr::instance->rectToRender(pos), color, rmag};
    static_assert(sizeof(data) == 6 * sizeof(float), "halo instance layout must match body_halo.vert");
    batchPush(haloFamily, &data);
}

void Renderer::setHaloTexture(const std::string &texName)
{
    ensureHaloFamily();
    if (!haloFamily)
        return;
    BatchState &b = *reg->families[haloFamily.id()].batch;
    // Same creation flags as Halo::setTexHaloMap (halo.cpp:217).
    b.tex = std::make_unique<s_texture>(texName, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
    b.boundTex = nullptr; // rebind at next batchBegin
}

void Renderer::ensureHintFamily()
{
    if (hintFamily)
        return;
    auto &r = registry();
    // Hint circle as a batched service family - the dissolution of the
    // DrawHelper DRAW_HINT_POS seam entry (the last Renderer borrow of that
    // class; the halo half was dissolved at S1). Same occlusion contract,
    // same mechanism as the halo now: flush at the per-body boundaries.
    // LINE_LIST instead of the old LINE_STRIP: strips of separate circles
    // would connect across circles in one batched draw; segments batch
    // freely. Per-vertex color replaces the old per-draw push constant
    // (colors vary per body - a push constant cannot batch them).
    auto pattern = std::make_unique<VertexArray>(*VulkanMgr::instance, Context::instance->ojmAlignment);
    pattern->createBindingEntry(6 * sizeof(float));
    pattern->addInput(VK_FORMAT_R32G32_SFLOAT);       // pos (render px)
    pattern->addInput(VK_FORMAT_R32G32B32A32_SFLOAT); // color (fader in alpha)
    PipelineFamilyDesc desc;
    desc.name = "HINT";
    desc.vertex = pattern.get();
    desc.sets.push_back(globalUboContract());
    PassDesc color;
    color.pass = PassKind::COLOR;
    color.shaderTable = {{0, {.vert = "bodyHintsBatch.vert.spv", .frag = "bodyHintsBatch.frag.spv"}}};
    color.state.blend = BLEND_SRC_ALPHA; // old hints pipeline: ctor default
    color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    color.state.cull = false;
    color.state.depthTest = false;  // old setDepthStencilMode(FALSE, FALSE)
    color.state.depthWrite = false;
    desc.passes.push_back(std::move(color));
    // 48 segment-vertices per circle; D3: few bodies visible at a time -
    // 128 circles per half is ample (overflow logs, old endDraw behavior).
    desc.batch = BatchDesc{6 * sizeof(float), 48 * 128};
    r.servicePatterns.push_back(std::move(pattern));
    hintFamily = allocateFamily(std::move(desc));
}

void Renderer::drawHint(const std::pair<float, float> &pos, const Vec4f &color)
{
    ensureHintFamily(); // first-use fallback; normally built at init()
    if (!hintFamily)
        return;
    // Circle shape: Hints::computeHintsAt stays the single authority
    // (radius/facets identical to the old path by construction) - it dies
    // with the old path by moving into this service then.
    float strip[(Hints::nbrFacets + 1) * 2];
    float *stripPtr = strip;
    const int points = Hints::computeHintsAt(VulkanMgr::instance->rectToRender(pos), stripPtr);
    struct Vtx {
        float x, y;
        Vec4f color;
    } v;
    static_assert(sizeof(Vtx) == 6 * sizeof(float), "hint instance layout must match bodyHintsBatch.vert");
    v.color = color;
    // Strip -> segment expansion (LINE_LIST): {p0,p1},{p1,p2},...
    for (int i = 0; i < points - 1; ++i) {
        v.x = strip[i*2];
        v.y = strip[i*2+1];
        batchPush(hintFamily, &v);
        v.x = strip[i*2+2];
        v.y = strip[i*2+3];
        batchPush(hintFamily, &v);
    }
}

void Renderer::ensureTailFamily()
{
    TailBatch &t = tailBatch;
    Context &context = *Context::instance;
    if (!t.built) {
        // Vertex format = the old TailContext pattern verbatim (tail.cpp:60-71):
        // binding 0 = shared geometry {vec3 normal, float timeOffset} at vertex
        // rate; binding 1 = the per-tail instance (8 vec3) at instance rate.
        auto pattern = std::make_unique<VertexArray>(*VulkanMgr::instance);
        pattern->createBindingEntry(4 * sizeof(float));
        pattern->addInput(VK_FORMAT_R32G32B32_SFLOAT); // normal
        pattern->addInput(VK_FORMAT_R32_SFLOAT);       // timeOffset
        pattern->createBindingEntry(24 * sizeof(float), VK_VERTEX_INPUT_RATE_INSTANCE);
        for (int i = 0; i < 8; ++i) // offset, expandDir, expandCorr, coefRadius, color, mat3(x3)
            pattern->addInput(VK_FORMAT_R32G32B32_SFLOAT);
        t.geometry = pattern->createBuffer(0, NB_TAIL_VERTICES, context.globalBuffer.get());
        t.instance = pattern->createBuffer(1, NB_MAX_TAILS, context.globalBuffer.get());
        t.index = context.indexBufferMgr->acquireBuffer(NB_TAIL_INDICES * sizeof(uint16_t));
        // Pipeline family = the old TailContext pipeline (tail.cpp:104-116):
        // push fov, spec-8 projection type (registry-injected, §11.33),
        // TRIANGLE_STRIP + primitive restart (stripBreaks), cull, NO depth
        // (setDepthStencilMode()), blend SRC_ALPHA (EntityCore ctor default).
        // NO descriptor set - the tail shader has none. body_tail.{vert,frag}.spv
        // reused VERBATIM.
        PipelineFamilyDesc desc;
        desc.name = "TAIL";
        desc.vertex = pattern.get();
        desc.pushConstants = {{VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint16_t>(sizeof(float))}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "body_tail.vert.spv", .frag = "body_tail.frag.spv"}}};
        color.state.blend = BLEND_SRC_ALPHA;
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        color.state.stripBreaks = true;
        color.state.cull = true;
        color.state.depthTest = false;
        color.state.depthWrite = false;
        desc.passes.push_back(std::move(color));
        registry().servicePatterns.push_back(std::move(pattern)); // registry-owned (I5)
        t.family = allocateFamily(std::move(desc));
        t.data.reserve(NB_MAX_TAILS);
        t.built = true;
    }
    if (t.built && !t.geometryUploaded) {
        // Shared geometry + index built ONCE (old TailContext ctor, tail.cpp:
        // 74-102). Retried until the staging has room (tiny: 321 verts + 672
        // idx); flushTails gates on geometryUploaded so nothing draws meanwhile.
        auto *vptr = context.transfer->planCopy<TailVertex>(t.geometry->get());
        auto *iptr = context.transfer->planCopy<uint16_t>(t.index);
        if (!vptr || !iptr)
            return; // staging full this frame - retry next
        Vec2f angles[NB_TAIL_LINES];
        for (int i = 0; i < NB_TAIL_LINES; ++i)
            angles[i] = Vec2f(cos(i * (2 * M_PI / NB_TAIL_LINES)), sin(i * (2 * M_PI / NB_TAIL_LINES)));
        *(vptr++) = TailVertex{{0, 0, 1}, 0}; // head tip
        for (int i = 0; i++ < NB_TAIL_LINES / 4;) { // coma-head rings
            const Vec2f angle = angles[i];
            for (int j = 0; j < NB_TAIL_LINES; ++j)
                *(vptr++) = {{angles[j][0] * angle[1], angles[j][1] * angle[1], angle[0]}, 0};
        }
        for (int i = 0; i++ < NB_TAIL_LENGTH;) { // tail-length rings
            const float timeOffset = i / float(NB_TAIL_LENGTH);
            for (int j = 0; j < NB_TAIL_LINES; ++j)
                *(vptr++) = {{angles[j][0], angles[j][1], 0}, timeOffset};
        }
        for (int i = 0; i < NB_TAIL_LINES; ++i) { // per-line primitive-restart strips
            *(iptr++) = 0;
            for (int j = 1; j < NB_TAIL_LINES * (NB_TAIL_LINES / 4 + NB_TAIL_LENGTH); j += NB_TAIL_LINES) {
                *(iptr++) = j + i;
                *(iptr++) = j + (i + 1) % NB_TAIL_LINES;
            }
            *(iptr++) = UINT16_MAX;
        }
        t.geometryUploaded = true;
    }
}

void Renderer::beginTailDraw()
{
    // Fresh command buffer, depth-less COLOR (mirrors beginTrailDraw): the tail
    // is depth-less (old setDepthStencilMode() off), no bucket. Flush pending
    // batched content (the last body's halos) before the tails.
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    nextCommandBuffer();
    batchFlush();
    passKind = PassKind::COLOR;
    tailBatch.data.clear();
    tailBatch.overflowLogged = false;
    // fov push = half-fov radians (old prj->getFov() * pi/360; ModularBody::
    // halfFov is already the half field-of-view in radians).
    tailBatch.fov = ModularBody::halfFov;
}

void Renderer::submitTail(const TailInstance &inst)
{
    ensureTailFamily();
    if (!tailBatch.family)
        return;
    if (static_cast<int>(tailBatch.data.size()) >= NB_MAX_TAILS) {
        if (!tailBatch.overflowLogged) {
            tailBatch.overflowLogged = true;
            VulkanMgr::instance->putLog("Renderer TAIL batch overflow (capacity "
                + std::to_string(NB_MAX_TAILS) + ") - tails dropped this frame", LogType::WARNING);
        }
        return;
    }
    tailBatch.data.push_back(inst);
}

void Renderer::flushTails()
{
    TailBatch &t = tailBatch;
    if (t.data.empty())
        return;
    ensureTailFamily();
    if (!t.geometryUploaded)
        return; // geometry not resident yet (staging was full) - skip this frame
    const FamilyBound bound = bind(t.family); // resolves against passKind COLOR
    if (!bound.layout)
        return; // base build failed (shader absent) - C3 degrade
    const int n = static_cast<int>(t.data.size());
    const int bytes = n * static_cast<int>(sizeof(TailInstance));
    void *dst = Context::instance->transfer->planCopy(t.instance->get(), 0, bytes);
    if (!dst)
        return; // staging full this frame - skip (C3, no stall)
    memcpy(dst, t.data.data(), bytes);
    bound.layout->pushConstant(cmd, 0, &t.fov);
    t.geometry->bind(cmd);    // binding 0 (shared geometry)
    t.instance->bind(cmd, 0); // binding 1 (this frame's instances)
    vkCmdBindIndexBuffer(cmd, t.index.buffer, t.index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, NB_TAIL_INDICES, n, 0, 0, 0);
}

void Renderer::ensurePointerFamily()
{
    if (pointerFamily)
        return;
    auto &r = registry();
    // Old ObjectBase pointer, OBJECT_BODY case, ported verbatim: 4 POINT_LIST
    // vertices {vec2 corner pos (render px), float motif index 1..4}; the
    // geometry shader expands each into a 20x20 px textured bracket
    // (object_base_pointer.geom - shaders REUSED, visual parity by
    // construction). States = the old pipeline's effective states:
    // blend SRC_ALPHA (EntityCore ctor default, never overridden), cull off
    // (default cullMode 0), depth off (setDepthStencilMode() defaults).
    auto pattern = std::make_unique<VertexArray>(*VulkanMgr::instance, Context::instance->ojmAlignment);
    pattern->createBindingEntry(3 * sizeof(float));
    pattern->addInput(VK_FORMAT_R32G32_SFLOAT); // corner position (render px)
    pattern->addInput(VK_FORMAT_R32_SFLOAT);    // motif index 1..4
    SetContractDesc texContract;
    texContract.name = "pointerTex";
    texContract.bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}};
    texContract.expectedSets = 1;
    PipelineFamilyDesc desc;
    desc.name = "POINTER";
    desc.vertex = pattern.get();
    desc.sets.push_back(globalUboContract());
    desc.sets.push_back(allocateSetContract(std::move(texContract)));
    desc.pushConstants.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec3f)});
    PassDesc color;
    color.pass = PassKind::COLOR;
    color.shaderTable = {{0, {.vert = "object_base_pointer.vert.spv", .geom = "object_base_pointer.geom.spv", .frag = "object_base_pointer.frag.spv"}}};
    color.state.blend = BLEND_SRC_ALPHA;
    color.state.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    color.state.cull = false;
    color.state.depthTest = false;
    color.state.depthWrite = false;
    desc.passes.push_back(std::move(color));
    r.servicePatterns.push_back(std::move(pattern));
    pointerFamily = allocateFamily(std::move(desc));
    if (pointerFamily) {
        Context &context = *Context::instance;
        pointerVertex = r.servicePatterns.back()->createBuffer(0, 4, context.globalBuffer.get());
        pointerTex = std::make_unique<s_texture>("pointer_planet.png");
        pointerSet.reset(allocSet(pointerFamily, 1));
        pointerSet->bindTexture(pointerTex->getTexture(), 0);
    }
}

void Renderer::drawPointer(const std::pair<float, float> &pos, float sizePx)
{
    if (!showPointer)
        return;
    ensurePointerFamily(); // first-use fallback; normally built at init()
    if (!pointerFamily)
        return;
    // Old suppression rule: a disc above 10% of the viewport radius is
    // distracting to point at (object_base.cpp:221).
    if (sizePx > ModularBody::viewportRadius * 0.1f)
        return;
    // Breathing animation; the clock only ticks while a pointer is drawn,
    // like the old static local_time (phase origin is arbitrary either way).
    pointerTimeMs += ModularBody::deltaTime;
    float size = sizePx + 20.f + 10.f * sinf(0.002f * pointerTimeMs);
    // Resolution scale: old sqrt(viewportHeight / fontResolution) - same
    // config authority (SCK_FONT_RESOLUTION_SIZE via ObjectBase).
    size *= sqrtf(VulkanMgr::instance->getScreenRect().extent.height
                  / ObjectBase::getFontResolution());
    const auto px = VulkanMgr::instance->rectToRender(pos);
    pointerData = {px.first, px.second, size};
    pointerQueued = true;
}

void Renderer::recordPointer()
{
    if (!pointerQueued)
        return;
    pointerQueued = false;
    const FamilyBound bound = resolveAndBind(*reg, reg->families[pointerFamily.id()], passKind, cmd, 0);
    if (!bound.layout)
        return;
    // Corner layout ported verbatim (object_base.cpp:228-242).
    float *data = (float *) Context::instance->transfer->planCopy(pointerVertex->get());
    if (!data)
        return;
    const float h = pointerData.size * 0.5f;
    *(data++) = pointerData.x - h; *(data++) = pointerData.y + h; *(data++) = 1.f;
    *(data++) = pointerData.x + h; *(data++) = pointerData.y + h; *(data++) = 2.f;
    *(data++) = pointerData.x + h; *(data++) = pointerData.y - h; *(data++) = 3.f;
    *(data++) = pointerData.x - h; *(data++) = pointerData.y - h; *(data++) = 4.f;
    bound.layout->bindSets(cmd, {*Context::instance->uboSet->get(), *pointerSet->get()});
    // Old fixed body-pointer color (object_base.cpp:98).
    static const Vec3f bodyColor(1.f, 0.3f, 0.3f);
    bound.layout->pushConstant(cmd, 0, &bodyColor);
    pointerVertex->bind(cmd);
    vkCmdDraw(cmd, 4, 1, 0, 0);
}

// ---- SUN_HALO service (row 14): the old Sun big halo -----------------------
// The old Sun owned pipelineBigHalo + descriptorSetBigHalo + haloCmds[3]
// (static secondary command buffers). Here the pipeline is a registry family,
// the uniforms/set/vertex are Renderer-owned, and the draw is RECORDED live
// into the frame command buffer (the S1 note: "static cmd buffers dissolve
// into frame-task recording"). sun_big_halo.{vert,geom,frag}.spv reused
// VERBATIM. cam_block.glsl puts the global UBO at set 1, so globalUboContract
// goes SECOND in desc.sets (the old bindSets({descriptorSetBigHalo, uboSet})
// order); the local set (tex + 4 uniforms) is set 0.
void Renderer::ensureSunHaloFamily()
{
    if (sunHaloFamily)
        return;
    auto &r = registry();
    Context &context = *Context::instance;
    // 1-point vertex {vec2 screen pos (render px)} - old m_bigHaloGL
    // (body_sun.cpp:129-131).
    auto pattern = std::make_unique<VertexArray>(*VulkanMgr::instance, context.ojmAlignment);
    pattern->createBindingEntry(sizeof(Vec2f));
    pattern->addInput(VK_FORMAT_R32G32_SFLOAT);
    // Local set (set 0): old layoutBigHalo bindings (body_sun.cpp:135-141).
    SetContractDesc local;
    local.name = "sunHalo";
    local.bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // tex_big_halo
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT},          // Rmag
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},          // cmag
        {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},          // radius
        {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},          // color
    };
    local.expectedSets = 1;
    PipelineFamilyDesc desc;
    desc.name = "SUN_HALO";
    desc.vertex = pattern.get();
    desc.sets.push_back(allocateSetContract(std::move(local))); // set 0 (local)
    desc.sets.push_back(globalUboContract());                   // set 1 (cam_block)
    // viewportY spec constant (id 0, float) = screen height px - old
    // createHaloShader(getScreenRect().extent.height) (body_sun.cpp:74).
    const float viewportY = static_cast<float>(VulkanMgr::instance->getScreenRect().extent.height);
    SpecConstant vy{0, 0};
    memcpy(&vy.value, &viewportY, sizeof(float));
    desc.specValues.push_back(vy);
    PassDesc color;
    color.pass = PassKind::COLOR;
    color.shaderTable = {{0, {.vert = "sun_big_halo.vert.spv", .geom = "sun_big_halo.geom.spv", .frag = "sun_big_halo.frag.spv"}}};
    color.state.blend = BLEND_ADD; // old setBlendMode(BLEND_ADD)
    color.state.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    color.state.cull = false;       // POINT_LIST: no faces (halo/hint parity)
    color.state.depthTest = false;  // old setDepthStencilMode(VK_FALSE, VK_FALSE)
    color.state.depthWrite = false;
    desc.passes.push_back(std::move(color));
    r.servicePatterns.push_back(std::move(pattern)); // registry-owned (I5)
    sunHaloFamily = allocateFamily(std::move(desc));
    if (sunHaloFamily) {
        sunHaloVertex = r.servicePatterns.back()->createBuffer(0, 1, context.globalBuffer.get());
        uSunRmag = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
        uSunCmag = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
        uSunRadius = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
        uSunColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
        sunHaloSet.reset(allocSet(sunHaloFamily, 0)); // set index 0 (the local contract)
        sunHaloSet->bindUniform(uSunRmag, 1);
        sunHaloSet->bindUniform(uSunCmag, 2);
        sunHaloSet->bindUniform(uSunRadius, 3);
        sunHaloSet->bindUniform(uSunColor, 4);
        // binding 0 (texture) is bound by setSunHaloTexture.
    }
}

void Renderer::setSunHaloTexture(const std::string &texName, const std::string &path)
{
    ensureSunHaloFamily();
    if (!sunHaloFamily || texName.empty())
        return;
    // Old Sun::setBigHalo (body_sun.cpp:109-121): existing path+file, else let
    // s_texture resolve it against the standard texture paths. PNG_SOLID like
    // the old load type.
    const std::string full = path + texName;
    if (CallSystem::fileExist(full))
        sunHaloTex = std::make_unique<s_texture>(full, TEX_LOAD_TYPE_PNG_SOLID);
    else
        sunHaloTex = std::make_unique<s_texture>(texName, TEX_LOAD_TYPE_PNG_SOLID);
    // Bind DEFERRED to frame start (batchBegin), NOT here: getTexture() at
    // load time creates the image UNINITIALIZED (no transfer cmd) and captures
    // a black view that never refreshes (the halo service has the same deferral
    // - setHaloTexture stashes, batchBegin rebinds after recordTransfer's
    // upload). Binding here left farHalo black (measured, INTENT §11.44).
    sunHaloTexBound = false;
}

void Renderer::drawSunHalo(const std::pair<float, float> &pos, const Vec3f &color,
                           float rmag, float cmag, float radius)
{
    ensureSunHaloFamily();
    if (!sunHaloFamily || !sunHaloTex || !sunHaloTexBound)
        return; // no texture, or not yet bound+uploaded (bound at frame start,
                // batchBegin) - old: if (isVisible && tex_big_halo)
    // Per-frame vertex (screenPos in render px - old screenPosF = screenPos,
    // MVP2D maps px->NDC). planCopy is staging: skip the frame if it's full (C3).
    auto *v = static_cast<std::pair<float, float> *>(
        Context::instance->transfer->planCopy(sunHaloVertex->get()));
    if (!v)
        return;
    *v = VulkanMgr::instance->rectToRender(pos);
    // Per-body uniforms (old drawBigHalo). Single-region SharedBuffer, updated
    // each frame like the old path - identical cross-frame behavior (parity).
    *uSunRmag = rmag;
    *uSunCmag = cmag;
    *uSunRadius = radius;
    *uSunColor = color;
    const FamilyBound bound = bind(sunHaloFamily); // resolves against passKind (COLOR)
    if (!bound.layout)
        return; // base build failed (shader absent) - C3 degrade
    // set 0 = local (tex+uniforms), set 1 = cam_block (old bindSets order).
    bound.layout->bindSets(cmd, {*sunHaloSet->get(), *Context::instance->uboSet->get()});
    sunHaloVertex->bind(cmd);
    vkCmdDraw(cmd, 1, 1, 0, 0);
}

void Renderer::releaseRegistry()
{
    if (!reg)
        return;
    // Called from the START of Context::~Context - every manager is alive:
    // release the staging SubBuffers properly, then drop the whole registry
    // (pipelines, layouts, pools, service patterns, builder thread), and the
    // Renderer-owned service resources (pointer, shadow) that depend on the
    // managers (shadow Sets live in registry pools - release BEFORE reg).
    shadow.release();
    pointerSet.reset();
    pointerTex.reset();
    pointerVertex.reset();
    // SUN_HALO service (row 14): Set lives in a registry pool (release before
    // reg); uniforms/vertex/texture depend on live managers (uniformMgr,
    // globalBuffer, texture cache). Family handle decremented while reg alive.
    sunHaloSet.reset();
    sunHaloTex.reset();
    sunHaloVertex.reset();
    uSunRmag.reset();
    uSunCmag.reset();
    uSunRadius.reset();
    uSunColor.reset();
    sunHaloFamily = {};
    for (auto &f : reg->families) {
        if (f.batch && f.batch->pData)
            Context::instance->stagingMgr->releaseBuffer(f.batch->staging);
    }
    // TAIL batch buffers (Renderer-owned, off the registry): release to their
    // BufferMgrs HERE, while every manager is alive - a static teardown would
    // touch a dead globalBuffer/indexBufferMgr (the shutdown-SIGSEGV class).
    if (tailBatch.index.buffer != VK_NULL_HANDLE)
        Context::instance->indexBufferMgr->releaseBuffer(tailBatch.index);
    tailBatch.geometry.reset();  // ~VertexBuffer releases to globalBuffer
    tailBatch.instance.reset();
    tailBatch.index = {};
    tailBatch.data.clear();
    tailBatch.data.shrink_to_fit();
    tailBatch.family = {};       // decrement family refcount (registry alive)
    tailBatch.built = false;
    tailBatch.geometryUploaded = false;
    reg.reset();
}

FamilyBound Renderer::bind(const PipelineFamily &family, VariantKey wanted)
{
    if (!family)
        return {nullptr, 0}; // null handle: allocation failed and was logged
    auto &r = *reg; // a valid handle implies the registry exists
    return resolveAndBind(r, r.families[family.id()], passKind, cmd, wanted);
}

FamilyBound Renderer::bindIn(const PipelineFamily &family, PassKind pass, VkCommandBuffer extCmd, VariantKey wanted)
{
    if (!family)
        return {nullptr, 0};
    auto &r = *reg;
    return resolveAndBind(r, r.families[family.id()], pass, extCmd, wanted);
}

Pipeline *Renderer::peek(const PipelineFamily &family, PassKind pass, VariantKey wanted)
{
    if (!family)
        return nullptr;
    auto &r = *reg;
    FamilyEntry &f = r.families[family.id()];
    PassEntry &pe = f.passes[static_cast<size_t>(pass)];
    if (!pe.desc)
        return nullptr; // undeclared/disabled pass - bind()'s logging path owns the message
    // Same normalization as resolveAndBind, then EXACT-or-nothing: peek is for
    // layout-invariant per-shape multi-pipeline recording (Renderer.hpp), and
    // a fallback row bound mid-record would switch shaders invisibly.
    wanted &= (f.axisMask & ~f.undefinedMask)
            | ((pass == PassKind::COLOR) ? VARIANT_NO_DEPTH : 0);
    if (providableKey(f, *pe.desc, wanted) != wanted)
        return nullptr; // combination absent from the shader table
    VariantSlot *slot = findSlot(pe, wanted);
    if (!slot) {
        pe.variants.emplace_back(wanted);
        r.enqueue({&f, pass, &pe.variants.back()});
        return nullptr; // build enqueued; caller skips those shapes (C3)
    }
    return slot->ready.load(std::memory_order_acquire) ? slot->pipeline.get() : nullptr;
}

bool Renderer::computeReady(const PipelineFamily &family, VariantKey key) const
{
    if (!family)
        return false;
    VariantSlot *slot = findSlot(reg->families[family.id()].passes[0], key);
    return slot && slot->ready.load(std::memory_order_acquire);
}

FamilyBound Renderer::bindCompute(const PipelineFamily &family, VariantKey key, VkCommandBuffer extCmd)
{
    if (!family)
        return {nullptr, 0};
    auto &r = *reg;
    FamilyEntry &f = r.families[family.id()];
    // The bank container is passes[0] (see the COMPUTE allocation branch).
    VariantSlot *slot = findSlot(f.passes[0], key);
    if (!slot) {
        // Outside the eager range: lazy-build it (same policy shape as
        // graphics variants) and skip this dispatch.
        f.passes[0].variants.emplace_back(key);
        r.enqueue({&f, PassKind::COLOR, &f.passes[0].variants.back()});
        return {nullptr, 0};
    }
    if (!slot->ready.load(std::memory_order_acquire))
        return {nullptr, 0}; // not resident: caller skips the dispatch (C3)
    slot->compute->bind(extCmd);
    return {f.layout.get(), key};
}

Set *Renderer::allocSet(const PipelineFamily &family, uint8_t setIndex)
{
    auto &r = registry();
    FamilyEntry &f = r.families[family.id()];
    if (setIndex >= f.desc.sets.size() || !f.desc.sets[setIndex]) {
        VulkanMgr::instance->putLog("PipelineRegistry: allocSet - invalid set index on family '" + f.desc.name + "'", LogType::ERROR);
        return nullptr;
    }
    SetContractEntry &e = r.contracts[f.desc.sets[setIndex].id()];
    if (e.external) {
        VulkanMgr::instance->putLog("PipelineRegistry: allocSet on external contract '" + e.desc.name + "' (the Set pre-exists, e.g. context.uboSet)", LogType::ERROR);
        return nullptr;
    }
    uint32_t needed = 0;
    for (const auto &b : e.desc.bindings)
        needed |= typeBit(b.type);
    // A pool serves this contract only if it knows every type AND holds enough
    // of each for ONE set (poolCap* note above - the aggregate at creation is
    // the first allocator's, not this contract's).
    const SetNeed n = perSetNeed(e.desc);
    const bool tooSmall = n.uniform > r.poolCapUniform
                       || n.dynUniform > r.poolCapDynUniform
                       || n.texture > r.poolCapTexture
                       || n.storageBuf > r.poolCapStorageBuf
                       || n.storageImg > r.poolCapStorageImg
                       || n.sampledImg > r.poolCapSampledImg;
    if (!r.poolRemaining || (needed & ~r.poolTypeMask) || tooSmall)
        createPool(r); // budget exhausted OR the contract uses a type this
                       // pool predates OR one of its sets does not fit
    --r.poolRemaining;
    // Caller owns (store in a unique_ptr); pools are registry-lifetime, which
    // outlives every module (modules die with the body tree, before Context).
    return new Set(*VulkanMgr::instance, *r.pools.back(), e.layout, -1, false, true);
}
