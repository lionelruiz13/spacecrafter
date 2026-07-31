#include "AtmExtModule.hpp"
#include "tools/context.hpp"
#include "tools/log.hpp"
#include "bodyModule/body_tesselation.hpp"
#include "ojmModule/objl.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include <cmath>

// ATM_EXT family - port of the old AtmExt::_dataSet pipelines
// (atm_ext.cpp:12-65) onto the registry. All four shader stages REUSED
// VERBATIM; the family's own contract is the ONLY set (the atm shaders are
// self-contained - no cam_block; adding the global UBO set would diverge
// from the verbatim .spv for nothing). NO_DEPTH rides the reserved variant
// bit (replaces the old prebuilt pipelineNoDepth clone).
static const PipelineFamily &atmExtFamily()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        SetContractDesc contract;
        contract.name = "atmExt";
        contract.bindings = {
            // Old layout: UBO visible to all four stages (atm_ext.cpp:19),
            // gradient sampled in frag with the default sampler (:20).
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
        };
        contract.expectedSets = 8; // shell bodies in live data: Earth/Mars/Venus/Titan (+margin)
        PipelineFamilyDesc desc;
        desc.name = "ATM_EXT";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "atm.vert.spv", .tesc = "atm.tesc.spv",
                                  .tese = "atm.tese.spv", .frag = "atm.frag.spv"}}};
        // Old fixed state, verbatim (atm_ext.cpp:23-34): SRC_ALPHA blend with
        // MAX blend op (brighten-only), cull + reversed winding (tessellated),
        // PATCH_LIST(3), vertex entries 1 (texcoord) + 2 (normal) stripped.
        // DEPTH: test ON, WRITE OFF (INTENT 5.33 - the one divergence from the
        // old pipeline state, which wrote depth only because it used the
        // EntityCore default). This shell is a TRANSLUCENT brighten-only glow
        // (BMT_TRANSLUCENT, MAX blend): it composites OVER what is behind it
        // and occludes nothing, so writing its own depth can only be wrong.
        // Measured consequence of the write: on Earth the shell stands at
        // atmosphere_radius_factor * scaledRadius = 1.03 * 6378.14 km and its
        // depth is the LAST thing written in the parent's merged bucket, so it
        // was a 191.34 km wall killing every grounded body below it - taller
        // than the proxy shell (5.29) and than any terrain (5.30) it was
        // hiding. The module's own header always stated the contract as
        // "depth-tested", never depth-writing (I1).
        color.state.blend = BLEND_SRC_ALPHA;
        color.state.blend.colorBlendOp = VK_BLEND_OP_MAX;
        color.state.depthWrite = false;
        color.state.reverseFrontFace = true;
        color.state.patchControlPoints = 3;
        color.state.removedVertexEntries = 0b110;
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

AtmExtModule::AtmExtModule(ObjL *mesh, const std::string &gradientPath, float radiusFactor) :
    BodyModule(BodyModuleType::ATMOSPHERE),
    mesh(mesh), gradient(gradientPath, TEX_LOAD_TYPE_PNG_ALPHA),
    family(atmExtFamily()),
    set(Context::instance->renderer.allocSet(family, 0)),
    uniform(*Context::instance->uniformMgr),
    radiusFactor(radiusFactor)
{
}

AtmExtModule::~AtmExtModule()
{
}

bool AtmExtModule::isLoaded()
{
    if (loaded)
        return true;
    if (gradient.isLoading())
        return false;
    // Old guard (atm_ext.cpp:72-73): a gradient below 32 texels disables the
    // shell permanently. Self-naming instead of silent (C3 ladder).
    if (gradient.getTexture().getTextureSize() < 32) {
        if (!disabled) {
            cLog::get()->write("AtmExtModule: gradient texture unusable (<32 texels), shell DISABLED for this body", LOG_TYPE::L_WARNING);
            disabled = true;
        }
        loaded = true; // never blocks the body's drawLoaded path
        return true;
    }
    set->bindUniform(uniform, 0);
    set->bindTexture(gradient.getTexture(), 1);
    loaded = true;
    return true;
}

void AtmExtModule::drawShell(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted)
{
    if (disabled)
        return;
    const float scaledRadius = body->getScaledRadius();
    const float distance = body->getDistanceToObserver();
    // Old gate, translated (body.cpp:1183-1184):
    // - screen_sz > 10 px: old px = screenSize * viewportHeight (the verified
    //   halo formula, INTENT 11.19a) = screenSize * 2 * viewportRadius;
    // - full angular size > 2 deg, from the BODY radius (not boundingRadius -
    //   the shell itself must not feed its own gate);
    // - observer outside the shell by 1%.
    if (body->getScreenSize() * 2.f * ModularBody::getViewportRadius() <= 10.f)
        return;
    const float squaredDistance = distance * distance;
    const float squaredRadius = scaledRadius * scaledRadius;
    if (squaredDistance <= squaredRadius)
        return; // inside the body - grounded/in regime, not the shell's case
    if (2.f * atan2f(scaledRadius, sqrtf(squaredDistance - squaredRadius)) <= 2.f * static_cast<float>(M_PI / 180.))
        return;
    if (distance <= scaledRadius * radiusFactor * 1.01f)
        return;
    const FamilyBound bound = renderer.bind(family, wanted);
    if (!bound.layout)
        return; // pass unavailable (e.g. shader not deployed) - logged at its
                // definition site; the body's other content still draws (C3)
    mesh->bind(renderer);
    uniform->ModelViewMatrix = mat;
    uniform->sunPos = ModularBody::getLightPosition();
    uniform->planetRadius = scaledRadius;
    uniform->bodyPos = mat.getTranslation();
    uniform->planetOneMinusOblateness = body->getOneMinusOblateness();
    uniform->clipping_fov = renderer.getClippingFov();
    uniform->atmRadius = scaledRadius * radiusFactor;
    {   // Both-paths seam: the SAME BodyTesselation object the old path
        // reads (injected at SolarSystemTex construction) - animated
        // `body tesselation` transitions identical by construction.
        auto &tes = ModularBody::getTesselation();
        uniform->TesParam = tes ? Vec2i(tes->getMinTesLevel(), tes->getMaxTesLevel())
                                : Vec2i(1, 1);
    }
    uniform->atmAlpha = 1; // old "Apply fader here" TODO kept as-is (parity)
    bound.layout->bindSet(renderer, *set);
    mesh->draw(renderer, body->getScreenSize() * 2.f * ModularBody::getViewportRadius());
}

void AtmExtModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    drawShell(renderer, body, mat, 0);
}

void AtmExtModule::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Reachable only at extreme fisheye fovs (>~180deg: 2deg angular can sit
    // below the 16px near-regime floor) - the old path drew the shell there
    // through its noDepth pipeline; same fallback semantics as BasicMesh.
    drawShell(renderer, body, mat, VARIANT_NO_DEPTH);
}
