#include "PipelineFamily.hpp"
#include "Renderer.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "tools/s_texture.hpp"
#include <algorithm>
#include <atomic>
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
        case PassKind::SHADOW_STENCIL: return "SHADOW_STENCIL";
        case PassKind::TRACE: return "TRACE";
        default: return "?";
    }
}

struct VariantSlot {
    VariantSlot(VariantKey key) : key(key) {}
    VariantKey key;
    std::unique_ptr<Pipeline> pipeline;
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
    }
};

std::unique_ptr<Registry> reg;

Registry &registry()
{
    if (!reg)
        reg = std::make_unique<Registry>();
    return *reg;
}

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
            // targets). S5 (G7 port) validates this against the old shadow
            // pipelines before first use - no family declares this pass yet.
            render = context.renderSelfShadow.get();
            dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            break;
        case PassKind::SHADOW_STENCIL:
            render = context.renderShadow.get();
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
        case PassKind::SHADOW_STENCIL: {
            const VkStencilOpState op {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0xff, 0xff, 1};
            p->setStencilMode(op, op);
            break;
        }
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
        auto pipeline = buildVariant(*job.family, job.pass, job.slot->key);
        if (pipeline && pipeline->get() != VK_NULL_HANDLE) {
            job.slot->pipeline = std::move(pipeline);
            job.slot->ready.store(true, std::memory_order_release);
        } // failure stays non-resident: bind() keeps falling back to base,
          // the build error is already logged at its definition site.
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
        // Trait-routing defect: hooks are only invoked within their matching
        // pass kind - log once per (family, pass), then no-op.
        if (!f.misroutedLogged[static_cast<size_t>(pass)]) {
            f.misroutedLogged[static_cast<size_t>(pass)] = true;
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

void createPool(Registry &r)
{
    // Each pool is sized from the CURRENT aggregate of allocated contracts -
    // it always covers the layouts it serves (INTENT 10.3 decision 5 /
    // 11.1 structural fix). Growth = one more aggregate-sized pool.
    // expectedSets is the sizing contract: a client exceeding it exhausts the
    // budget visibly (allocation failure at a named site), never silently.
    const uint32_t sets = std::max(r.aggSets, 16u);
    r.pools.push_back(std::make_unique<SetMgr>(*VulkanMgr::instance, sets,
        r.aggUniform, r.aggTexture, r.aggStorageBuf, r.aggStorageImg,
        true, r.aggDynUniform, r.aggSampledImg));
    r.poolRemaining = sets;
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
        // Compute families land with G7's shadow implementation (S5) -
        // deferral accepted at plan approval (INTENT 10.3 open 4).
        VulkanMgr::instance->putLog("PipelineRegistry: COMPUTE family '" + desc.name + "' not implemented yet (lands with G7/S5)", LogType::ERROR);
        return {};
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
            VulkanMgr::instance->putLog("PipelineRegistry: BASE build failed for family '" + f.desc.name + "' pass " + passName(pd.pass) + " - pass disabled", LogType::ERROR);
            pe.desc = nullptr;
            pe.variants.clear();
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

void Renderer::releaseRegistry()
{
    if (!reg)
        return;
    // Called from the START of Context::~Context - every manager is alive:
    // release the staging SubBuffers properly, then drop the whole registry
    // (pipelines, layouts, pools, service patterns, builder thread).
    for (auto &f : reg->families) {
        if (f.batch && f.batch->pData)
            Context::instance->stagingMgr->releaseBuffer(f.batch->staging);
    }
    reg.reset();
}

FamilyBound Renderer::bind(const PipelineFamily &family, VariantKey wanted)
{
    if (!family)
        return {nullptr, 0}; // null handle: allocation failed and was logged
    auto &r = *reg; // a valid handle implies the registry exists
    return resolveAndBind(r, r.families[family.id()], passKind, cmd, wanted);
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
    if (!r.poolRemaining)
        createPool(r);
    --r.poolRemaining;
    // Caller owns (store in a unique_ptr); pools are registry-lifetime, which
    // outlives every module (modules die with the body tree, before Context).
    return new Set(*VulkanMgr::instance, *r.pools.back(), e.layout, -1, false, true);
}
