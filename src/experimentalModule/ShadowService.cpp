#include "ShadowService.hpp"
#include "Renderer.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"
#include <cstring>

// Shadow constants (SHADOW_LOCAL_SIZE, SHADOW_RADIUS_TOLERANCE,
// SHADOW_INVALIDATING_ANGLE, MAX_RADIUS_HARD_LIMIT) come from context.hpp -
// the existing single authority, shared with the old path by design: they are
// cache tolerances and workgroup geometry, not path policy (shadow-paths.md
// A2.5/A2.10 for what each guards).

bool ShadowService::enabled = false; // initialized from config (app.cpp seam)

// Coverage-over compositing on the R8 silhouette target:
// c' = c_src + c_dst * (1 - c_src), i.e. 1 - T_src*T_dst - transmissions of
// independent occluders multiply, exactly. Opaque fragments (c_src = 1)
// saturate regardless of overlap order.
static const VkPipelineColorBlendAttachmentState BLEND_COVERAGE_OVER {
    VK_TRUE,
    VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_OP_ADD,
    VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
    VK_COLOR_COMPONENT_R_BIT
};

ShadowService::ShadowService() = default;
ShadowService::~ShadowService() = default;

void ShadowService::ensureInit(Renderer &_renderer)
{
    if (inited)
        return;
    renderer = &_renderer;
    Context &context = *Context::instance;
    auto &vkmgr = *VulkanMgr::instance;
    const uint32_t res = context.shadowRes;
    maxRadius = std::min(res / 2, MAX_RADIUS_HARD_LIMIT + 2U) - 1U; // old bank sizing (context.cpp:76)

    // Service-owned blurred-layer array (R8; layers = caster budget). The
    // silhouette scratch reuses context.shadowShape/renderShadowShape (shared
    // serially within the recording window - never concurrently).
    const uint8_t budget = context.maxShadowCast;
    layers = std::make_unique<Texture>(vkmgr, TextureInfo{
        .width=(int) res, .height=(int) res, .nbChannels=1,
        .arrayLayers=budget,
        .usage=VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .format=VK_FORMAT_R8_UNORM,
        .name="New-path projected shadows"});
    layers->use();

    // Contracts. Trace = the old traceLayout (vertex mat3 UBO) - ONE handle
    // shared by both silhouette families (set 0), so slot traceSets bind
    // either family interchangeably (the traceLayout-sharing pattern,
    // PipelineFamily.hpp SetContract block). Annulus adds its texture as a
    // separate set 1 (module-owned - a layout-invariant split: slot-owned
    // vs module-owned data never share a Set).
    SetContractDesc traceContract;
    traceContract.name = "shadowTraceMat";
    traceContract.bindings = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}};
    traceContract.expectedSets = budget;
    SetContract traceHandle = renderer->allocateSetContract(std::move(traceContract));
    SetContractDesc annulusContract;
    annulusContract.name = "shadowRingTex";
    annulusContract.bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}};
    annulusContract.expectedSets = 4; // ringed bodies are rare (D5-scaled expectation, not a budget)
    SetContractDesc blurContract;
    blurContract.name = "shadowBlur";
    blurContract.bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT},
    };
    blurContract.expectedSets = budget;

    // OPAQUE_MESH silhouettes (G1): position-only mesh, frag writes coverage 1.
    PipelineFamilyDesc shape;
    shape.name = "SHADOW_SHAPE";
    shape.vertex = context.ojmVertexArray.get(); // referenced, Context-owned (I5)
    shape.sets.push_back(traceHandle);
    PassDesc shapePass;
    shapePass.pass = PassKind::SHADOW_SHAPE;
    shapePass.shaderTable = {{0, {.vert = "shadow_trace.vert.spv", .frag = "shadow_shape.frag.spv"}}}; // vert REUSED
    shapePass.state.blend = BLEND_COVERAGE_OVER;
    shapePass.state.cull = false;            // old shadowShape: no cull call
    shapePass.state.depthTest = false;       // old setDepthStencilMode()
    shapePass.state.depthWrite = false;
    shapePass.state.removedVertexEntries = 0b110; // position only (old removeVertexEntry(1)(2))
    shape.passes.push_back(std::move(shapePass));
    shapeFamily = renderer->allocateFamily(std::move(shape));

    // TEXTURED_ANNULUS silhouettes (G8): vertex-less strip quad in the
    // caster's equatorial plane, frag = radial ring texture alpha.
    PipelineFamilyDesc ring;
    ring.name = "SHADOW_RING";
    ring.sets.push_back(traceHandle);
    ring.sets.push_back(renderer->allocateSetContract(std::move(annulusContract)));
    ring.pushConstants.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4}); // innerRatio
    PassDesc ringPass;
    ringPass.pass = PassKind::SHADOW_SHAPE;
    ringPass.shaderTable = {{0, {.vert = "shadowRing.vert.spv", .frag = "shadowRing.frag.spv"}}};
    ringPass.state.blend = BLEND_COVERAGE_OVER;
    ringPass.state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    ringPass.state.cull = false;             // the annulus is seen from either side
    ringPass.state.depthTest = false;
    ringPass.state.depthWrite = false;
    ring.passes.push_back(std::move(ringPass));
    ringFamily = renderer->allocateFamily(std::move(ring));

    PipelineFamilyDesc blur;
    blur.name = "SHADOW_BLUR";
    blur.kind = PipelineFamilyDesc::Kind::COMPUTE;
    blur.buildPolicy = PipelineFamilyDesc::BuildPolicy::EAGER_ASYNC_ALL;
    blur.computeShader = "shadowBlur.comp.spv"; // float-input port of shadow.comp (quantized-exact)
    blur.sets.push_back(renderer->allocateSetContract(std::move(blurContract)));
    blur.specValues = {{1, res}};   // border; constant 0 = radius (the key)
    blur.eagerVariants = static_cast<uint16_t>(maxRadius);
    blurFamily = renderer->allocateFamily(std::move(blur));
    if (!shapeFamily || !ringFamily || !blurFamily) {
        VulkanMgr::instance->putLog("ShadowService: family allocation failed - shadows disabled", LogType::ERROR);
        enabled = false;
        return;
    }

    slots.resize(budget);
    layerViews.reserve(budget);
    for (uint8_t i = 0; i < budget; ++i) {
        auto &s = slots[i];
        s.uniform = std::make_unique<SharedBuffer<BlurUniform>>(*context.uniformMgr);
        s.traceMat = std::make_unique<SharedBuffer<float[12]>>(*context.uniformMgr);
        s.traceSet.reset(renderer->allocSet(shapeFamily, 0));
        s.blurSet.reset(renderer->allocSet(blurFamily, 0));
        s.traceSet->bindUniform(s.traceMat, 0);
        s.blurSet->bindUniform(s.uniform, 0);
        s.blurSet->bindImage(*context.shadowShape, 1);
        layerViews.push_back(layers->createView(0, 1, i, 1));
        s.blurSet->bindStorageImage(layerViews.back(), 2);
    }

    // Production recording rides the pre-color window (where the old path
    // records ITS shadow passes) - the DrawHelper hook is the channel.
    context.helper->setPreFrameRecorder([this](VkCommandBuffer cmd, unsigned char frameIdx) {
        record(cmd, frameIdx);
    });
    inited = true;
}

void ShadowService::release()
{
    if (!inited)
        return;
    if (Context::instance->helper)
        Context::instance->helper->setPreFrameRecorder(nullptr);
    slots.clear();      // Sets + SharedBuffers while managers are alive
    // createView returns RAW views - the caller owns destruction (the S5
    // comment "views die with the Texture" was wrong: Texture::createView
    // tracks nothing; 8 ImageViews leaked to vkDestroyDevice object-tracking
    // on every shutdown that had initialized the service - found 2026-07-16).
    for (VkImageView v : layerViews)
        vkDestroyImageView(VulkanMgr::instance->refDevice, v, nullptr);
    layerViews.clear();
    layers.reset();
    shapeFamily = {};
    ringFamily = {};
    blurFamily = {};
    inited = false;
}

void ShadowService::beginFrame(uint8_t frameIdx)
{
    curFrame = frameIdx;
    jobs[frameIdx].clear();
    exhaustedLogged = false;
    // Two-phase aging, old DrawHelper::beginDraw walk (draw_helper.cpp:105-116):
    // a slot unused for one full frame frees; a used slot re-arms for reuse.
    for (auto &s : slots) {
        if (s.caster) {
            if (s.used) {
                s.used = false;
            } else {
                s.caster = nullptr;
                s.source = nullptr;
            }
        }
    }
}

int ShadowService::acquire(ModularBody *caster, const BodyModule *source, float radiusPx,
                           const Vec3f &lightDir, bool *reused)
{
    // Floor the penumbra at 1 px BEFORE keying/filter math: within-body pairs
    // legitimately request smooth = 0 (sharp ring shadows), and the disc
    // filter below computes sqrtf(rq - i*i) up to i = radius >= 1 - with
    // rq < 1 that is sqrtf(negative) = NaN into the offsets (a latent class
    // of the old drawShadower math, unreachable while every caster was a
    // distant body; the within-body path makes it reachable). Flooring keeps
    // the cache key consistent with what is actually produced.
    if (radiusPx < 1.f)
        radiusPx = 1.f;
    // Ported drawShadower scan (draw_helper.cpp:491-538): matchLevel
    // 1 = free slot, 2 = same (caster, source) pair (radius/light moved -
    // re-render), 3 = same pair within tolerances (cache hit).
    uint8_t matchLevel = 0;
    int idx = -1;
    const int count = static_cast<int>(slots.size());
    for (int i = 0; i < count; ++i) {
        auto &s = slots[i];
        switch (matchLevel) {
            case 0:
                if (!s.used && !s.caster) {
                    idx = i;
                    matchLevel = 1;
                }
                [[fallthrough]];
            case 1:
                if (s.caster == caster && s.source == source && !s.used) {
                    idx = i;
                    matchLevel = 2;
                }
                [[fallthrough]];
            case 2:
                if (s.caster == caster && s.source == source && fabsf(s.radiusPx - radiusPx) < SHADOW_RADIUS_TOLERANCE) {
                    float tmp = lightDir.dot(s.lightDir);
                    tmp *= tmp;
                    if (tmp >= s.lightDir.lengthSquared() * lightDir.lengthSquared() * SHADOW_INVALIDATING_ANGLE) {
                        idx = i;
                        matchLevel = 3;
                    }
                }
        }
    }
    if (idx < 0) {
        // Pool exhausted. Callers acquire in occlusion order, so the dropped
        // shadow is the least significant one - the structured replacement of
        // the old "shadow shapes are mostly unpredictible" slot steal.
        if (!exhaustedLogged) {
            exhaustedLogged = true;
            VulkanMgr::instance->putLog("ShadowService: caster budget exhausted - least significant shadows dropped this frame (increase max_shadow_cast)", LogType::WARNING);
        }
        return -1;
    }
    auto &s = slots[idx];
    if (matchLevel == 3) {
        s.used = true;
        *reused = true;
        return idx;
    }
    // (Re)assignment: the blur pipeline for this radius must be resident
    // before the layer can be produced (C3: never wait - old parity is the
    // global shadow_ready gate; per-key readiness is strictly finer).
    int radius = static_cast<int>(radiusPx);
    if (radius < 1)
        radius = 1;
    else if (radius > static_cast<int>(maxRadius))
        radius = static_cast<int>(maxRadius);
    if (!renderer->computeReady(blurFamily, static_cast<VariantKey>(radius)))
        return -1; // bank still building; no log - transient by construction
    *reused = false;
    s.caster = caster;
    s.source = source;
    s.radiusPx = radiusPx;
    s.lightDir = lightDir;
    s.used = true;
    s.radius = radius;
    // Disc-filter descriptor (old submit-side math, draw_helper.cpp:463-471).
    // pixelCount carries the x255 of the blur's quantized reads (each texel
    // contributes round(value*255) to the integer accumulator - EXACT for
    // the 0/255 G1 values, 8-bit-exact for G8 grading).
    auto &u = **s.uniform;
    int pixelCount = 0;
    const float rq = radiusPx * radiusPx;
    for (int i = 0; i <= radius; ++i) {
        const int tmp = static_cast<int>(sqrtf(rq - i * i));
        pixelCount += tmp;
        u.offsets[i].v = tmp;
    }
    u.pixelCount = (pixelCount * 4 + 1) * 255.f;
    return idx;
}

void ShadowService::produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh)
{
    silhouetteMat.setMat3(*slots[idx].traceMat);
    jobs[curFrame].push_back({static_cast<uint8_t>(idx), Job::Kind::OPAQUE_MESH, mesh, nullptr, 0});
}

void ShadowService::produceAnnulus(int idx, const Mat4f &silhouetteMat, Set *texSet, float innerRatio)
{
    silhouetteMat.setMat3(*slots[idx].traceMat);
    jobs[curFrame].push_back({static_cast<uint8_t>(idx), Job::Kind::TEXTURED_ANNULUS, nullptr, texSet, innerRatio});
}

std::unique_ptr<Set> ShadowService::makeAnnulusTexSet(Texture &tex)
{
    if (!inited)
        return nullptr;
    std::unique_ptr<Set> set(renderer->allocSet(ringFamily, 1));
    if (set)
        set->bindTexture(tex, 0);
    return set;
}

void ShadowService::record(VkCommandBuffer cmd, uint8_t frameIdx)
{
    // Layout transition UNCONDITIONALLY at first record (old notInitialized
    // pattern): the array's descriptor is statically bound by every MESH set
    // (binding 3) even when shadows are off - an UNDEFINED-layout image
    // behind a SHADER_READ_ONLY descriptor is a validation error at the
    // first mesh draw, jobs or not.
    if (!layersInitialized) {
        layers->use(cmd, Implicit::LAYOUT);
        layersInitialized = true;
    }
    auto &pending = jobs[frameIdx];
    if (pending.empty())
        return;
    Context &context = *Context::instance;
    const uint32_t res = context.shadowRes;
    const VkViewport viewport {0, 0, (float) res, (float) res, 0.f, 1.f};
    const VkRect2D scissor {{0, 0}, {res, res}};
    for (const auto &job : pending) {
        Slot &s = slots[job.slot];
        // Typed silhouette pass into the shared R8 scratch (cleared to 0 by
        // the pass; word selects the family, the target/blur are shared).
        context.renderShadowShape->begin(0, cmd);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        switch (job.kind) {
            case Job::Kind::OPAQUE_MESH: {
                const FamilyBound bound = renderer->bindIn(shapeFamily, PassKind::SHADOW_SHAPE, cmd);
                if (bound.layout) {
                    bound.layout->bindSet(cmd, *s.traceSet);
                    job.mesh->bind(cmd);
                    job.mesh->draw(cmd, res);
                }
                break;
            }
            case Job::Kind::TEXTURED_ANNULUS: {
                const FamilyBound bound = renderer->bindIn(ringFamily, PassKind::SHADOW_SHAPE, cmd);
                if (bound.layout) {
                    bound.layout->bindSet(cmd, *s.traceSet, 0);
                    bound.layout->bindSet(cmd, *job.texSet, 1);
                    bound.layout->pushConstant(cmd, 0, &job.innerRatio);
                    vkCmdDraw(cmd, 4, 1, 0, 0); // vertex-less strip quad
                }
                break;
            }
        }
        vkCmdEndRenderPass(cmd);
        // Blur: barriers ported verbatim (draw_helper.cpp:455-478), on the
        // service-owned layer.
        VkImageMemoryBarrier imageBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            layers->getImage(), {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, job.slot, 1}};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
        const FamilyBound blur = renderer->bindCompute(blurFamily, static_cast<VariantKey>(s.radius), cmd);
        if (blur.layout) { // resident by acquire-time check; guard stays (everything can fail)
            blur.layout->bindSet(cmd, *s.blurSet, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
            vkCmdDispatch(cmd, res / SHADOW_LOCAL_SIZE, 1, 1);
        }
        imageBarrier.srcAccessMask = imageBarrier.dstAccessMask;
        imageBarrier.oldLayout = imageBarrier.newLayout;
        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    }
    pending.clear();
}
