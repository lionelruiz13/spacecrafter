#include "OjmModule.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/meshModules/meshShadowFill.hpp"
#include "ojmModule/ojm.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include <cstring>

// SHADER_SWAP axes of the OJM family (file-local: no other client).
// TEXLESS switches per SHAPE (Ojm::record pointer-pair, peek'd - INTENT
// 10.3 rule 4); SHADOWED switches per DRAW (self-shadow nominated or
// received entries).
static constexpr VariantKey VARIANT_TEXLESS = 0x0001;
static constexpr VariantKey VARIANT_SHADOWED = 0x0002;

static uint32_t floatBits(float v)
{
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits; // SpecConstant carries raw bytes (PipelineRegistry applies &value, sizeof)
}

// The OJM family - old shaderArtificial + shaderArtificialShadowed dissolved
// into ONE family / ONE layout (INTENT 10.3 rule 4) with 4 shader rows on
// two SHADER_SWAP axes. State = the old pipelines exactly (bodyShader.cpp
// 239-305): cull on, no blend, triangle list, ojmVertexArray, depth on
// (NO_DEPTH = the reserved bit, replacing the old pipeline[2..3] pair).
// Spec constants: 7 = float64 fisheye (the old SHADOWED pipelines omitted it
// - an old-path oversight, not intent; family-wide here = plain-row parity
// everywhere, divergence documented shadow-paths.md G), 0 = self-shadow
// resolution as float bits (selfShadow.glsl depthTextureSize).
static const PipelineFamily &ojmFamily()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Context &context = *Context::instance;
        Renderer &renderer = context.renderer;

        SetContractDesc tex;
        tex.name = "ojmShapeTex";
        tex.bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}};
        // Per-TEXTURE sets, allocated lazily by s_texture::bindTexture
        // (texture->ojmSet, shared across shapes/models using the texture).
        tex.expectedSets = 32;

        SetContractDesc block;
        block.name = "ojmBlock";
        block.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},   // ojmVert (NormalMatrix)
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT}, // ojmGeom
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}, // ojmLight / ojmShadowBlock
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // self-shadow depth
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // layer array
        };
        block.expectedSets = 8; // 2 Sets per OJM body (D5 expectation, not a budget)

        float selfShadowRes;
        {
            int tmp;
            context.shadowBuffer->getDimensions(tmp, tmp);
            selfShadowRes = tmp;
        }

        PipelineFamilyDesc desc;
        desc.name = "OJM";
        desc.vertex = context.ojmVertexArray.get();
        desc.sets = {renderer.globalUboContract(),
                     renderer.allocateSetContract(std::move(tex)),
                     renderer.allocateSetContract(std::move(block))};
        desc.pushConstants = {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 44}}; // MaterialInfo (old layout 44 bytes)
        desc.specValues = {{7, Context::instance->isFloat64Supported},
                           {0, floatBits(selfShadowRes)}};
        desc.axes = {
            {"texless", VARIANT_TEXLESS, VariantEffect::SHADER_SWAP, 1},
            {"shadowed", VARIANT_SHADOWED, VariantEffect::SHADER_SWAP, 2},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {
            {0,                                  {.vert = "body_artificial.vert.spv", .geom = "body_artificial.geom.spv", .frag = "body_artificial_tex.frag.spv"}},
            {VARIANT_TEXLESS,                    {.vert = "body_artificial.vert.spv", .geom = "body_artificial.geom.spv", .frag = "body_artificial_notex.frag.spv"}},
            {VARIANT_SHADOWED,                   {.vert = "body_artificial.vert.spv", .geom = "body_artificial.geom.spv", .frag = "ojmShadowTex.frag.spv"}},
            {VARIANT_SHADOWED | VARIANT_TEXLESS, {.vert = "body_artificial.vert.spv", .geom = "body_artificial.geom.spv", .frag = "ojmShadowNotex.frag.spv"}},
        };
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

OjmModule::OjmModule(std::shared_ptr<Ojm> model) : BodyModule(BodyModuleType::OJM),
    model(std::move(model)),
    uVert(*Context::instance->uniformMgr), uGeom(*Context::instance->uniformMgr),
    uLight(*Context::instance->uniformMgr), uShadow(*Context::instance->uniformMgr)
{
    uShadow->nbShadowingBodies = 0;
    uShadow->selfShadowOn = 0;
}

OjmModule::~OjmModule() = default;

bool OjmModule::isLoaded()
{
    if (!model || !model->getOk())
        return false; // load failed - never drawable (old parity: radius zeroed by the loader)
    if (!bound) {
        // Deferred like BasicMesh::isLoaded: the layer array exists once the
        // Renderer initialized; shadowBuffer is app-lifetime Context infra.
        const PipelineFamily &family = ojmFamily();
        Renderer &renderer = Context::instance->renderer;
        setPlain.reset(renderer.allocSet(family, 2));
        setShadow.reset(renderer.allocSet(family, 2));
        if (!setPlain || !setShadow)
            return false; // allocation failure already logged at its site
        setPlain->bindUniform(uVert, 0);
        setPlain->bindUniform(uGeom, 1);
        setPlain->bindUniform(uLight, 2);
        setPlain->bindTexture(*Context::instance->shadowBuffer, 3);
        setPlain->bindTexture(*renderer.shadow.layerArray(), 4);
        setShadow->bindUniform(uVert, 0);
        setShadow->bindUniform(uGeom, 1);
        setShadow->bindUniform(uShadow, 2);
        setShadow->bindTexture(*Context::instance->shadowBuffer, 3);
        setShadow->bindTexture(*renderer.shadow.layerArray(), 4);
        bound = true;
    }
    return true;
}

void OjmModule::drawInternal(Renderer &renderer, ModularBody *body, const Mat4f &mat, VariantKey base)
{
    if (!isLoaded())
        return; // C3: nothing drawable yet (or ever, on load failure)
    const bool wantShadow = selfShadowActive
        || (ShadowService::enabled && body->getReceivedShadows());
    const VariantKey want = base | (wantShadow ? VARIANT_SHADOWED : 0);
    const FamilyBound bnd = renderer.bind(ojmFamily(), want);
    if (!bnd.layout) {
        selfShadowActive = false;
        return; // pass unavailable - logged at its definition site
    }
    const bool shadowed = bnd.got & VARIANT_SHADOWED;
    const float radius = body->getScaledRadius();
    // Old drawBody parity (body_artificial.cpp:132-136): the near-component
    // mat already carries zrot(axisRotation + 90) (ModularBody
    // computeBodyToSurface); normals from the UNSCALED rotation.
    mat.setMat3(uVert->NormalMatrix);
    uGeom->ModelViewMatrix = mat * Mat4f::scaling(radius);
    uGeom->clipping_fov = renderer.getClippingFov();
    const Vec3f L = ModularBody::getLightPosition();
    if (shadowed) {
        auto &f = *uShadow;
        // Production/consumption consistency: the SAME matrix value the
        // nomination handed to produceSelfDepth (BodyModule.hpp contract).
        selfShadowMat.setMat3(f.ShadowMatrix);
        // Frame reconciliation (documented divergence, shadow-paths.md G):
        // everything eye-space - the old shadowed frag mixed heliocentric
        // ModelMatrix with eye-space normals (body_artificial.cpp:138 vs 133).
        mat.setMat3(f.ModelMatrix);
        f.ModelPosition = Vec3f(mat.r[12], mat.r[13], mat.r[14]);
        f.lightDirection = body->getObservedPosition() - L;
        f.lightDirection.normalize();
        f.LightIntensity = Vec3f(1.f, 1.f, 1.f); // old hardcode
        f.selfShadowOn = selfShadowActive ? 1.f : 0.f;
        fillOjmShadows(uShadow, body, mat, radius, this);
        bnd.layout->bindSet(renderer, *Context::instance->uboSet);
        bnd.layout->bindSet(renderer, *setShadow, 2);
    } else {
        uLight->Position = L; // old eye_sun
        uLight->Intensity = Vec3f(1.f, 1.f, 1.f);
        bnd.layout->bindSet(renderer, *Context::instance->uboSet);
        bnd.layout->bindSet(renderer, *setPlain, 2);
    }
    // Per-shape tex/notex switch: peek the pair of the BOUND row (exact keys;
    // a missing texless build skips those shapes this frame - C3 transient).
    Pipeline *pair[2] = {
        renderer.peek(ojmFamily(), PassKind::COLOR, bnd.got),
        renderer.peek(ojmFamily(), PassKind::COLOR, bnd.got | VARIANT_TEXLESS),
    };
    // bind() left pair[0] bound - selectedPipeline 0, like the old record calls.
    model->record(renderer, pair, bnd.layout, 0);
    selfShadowActive = false; // frame-scoped (BodyModule.hpp contract)
}

void OjmModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    drawInternal(renderer, body, mat, 0);
}

void OjmModule::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    drawInternal(renderer, body, mat, VARIANT_NO_DEPTH);
}

void OjmModule::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    // OPAQUE_OJM word: unit geometry (ojm.cpp load normalization), matrix
    // applies unchanged - same contract as the mesh word.
    renderer.shadow.produceOjm(idx, mat, model.get());
}

void OjmModule::drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Nomination hook (BodyModule.hpp contract): declare the depth job and
    // keep the SAME matrix for the color fill (single-computation
    // consistency, ShadowService.hpp header).
    selfShadowMat = mat;
    selfShadowActive = true;
    renderer.shadow.produceSelfDepth(mat, model.get());
}
