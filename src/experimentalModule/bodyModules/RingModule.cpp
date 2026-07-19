#include "RingModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/meshModules/meshShadowFill.hpp"
#include "experimentalModule/bodyModules/TraceFamily.hpp"
#include "bodyModule/ring.hpp" // Ring2D geometry (old->new include, dies with the old path)
#include "tools/context.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"

Vec3i RingModule::lodSlices(64, 256, 512); // config defaults (rings_low/medium/high)

void RingModule::setLodSlices(int low, int medium, int high)
{
    lodSlices = Vec3i(low, medium, high);
}

// RING family - port of the old Ring::createSC_context color pipeline
// (ring.cpp:106-148) onto the registry. Fixed state verbatim: TRIANGLE_STRIP,
// cull ON, BLEND_SRC_ALPHA (the EntityCore Pipeline DEFAULT the old code
// relied on implicitly - the registry default is BLEND_NONE, so it must be
// explicit here). Spec 7 = float64; 8 = projection (registry-injected,
// INTENT 11.33). NO NO_DEPTH variant: bit-drop fallback = the old ringed
// forced-depth behavior (10.3 rule 1, body.cpp:1055-1059).
// Family-scoped vertex array (file-static: buildGeometry constructs Ring2D
// buffers against it; the family - hence this array - outlives every module
// instance, I5).
static std::unique_ptr<VertexArray> ringVertexArray;

static const PipelineFamily &ringColorFamily()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        ringVertexArray = std::make_unique<VertexArray>(*VulkanMgr::instance);
        ringVertexArray->createBindingEntry(3 * sizeof(float)); // old ring.cpp:117-120
        ringVertexArray->addInput(VK_FORMAT_R32G32_SFLOAT);     // pos2D
        ringVertexArray->addInput(VK_FORMAT_R32_SFLOAT);        // tex1D
        SetContractDesc contract;
        contract.name = "bodyRing";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
        };
        contract.expectedSets = 4; // ringed bodies in live data: Saturn/Uranus (+margin)
        PipelineFamilyDesc desc;
        desc.name = "RING";
        desc.vertex = ringVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "bodyRing.vert.spv", .frag = "bodyRing.frag.spv"}}};
        color.state.blend = BLEND_SRC_ALPHA;
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

RingModule::RingModule(std::unique_ptr<s_texture> tex, float innerRadius, float outerRadius,
                       const Vec3f &shadowColor) :
    BodyModule(BodyModuleType::RING), tex(std::move(tex)),
    innerRadius(innerRadius), outerRadius(outerRadius), shadowColor(shadowColor)
{
    const PipelineFamily &family = ringColorFamily();
    set.reset(Context::instance->renderer.allocSet(family, 0));
    uVert = std::make_unique<SharedBuffer<bodyRingVert>>(*Context::instance->uniformMgr);
    uFrag = std::make_unique<SharedBuffer<bodyRingFrag>>(*Context::instance->uniformMgr);
}

RingModule::~RingModule() = default;

bool RingModule::isLoaded()
{
    if (loaded)
        return true;
    if (tex->isLoading())
        return false;
    set->bindUniform(uVert, 0);
    set->bindUniform(uFrag, 1);
    set->bindTexture(tex->getTexture(), 2);
    // Binding 3 = the ShadowService layer array (valid descriptor required
    // even when shadows are off - BasicMesh idiom).
    set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
    buildGeometry();
    loaded = true;
    return true;
}

void RingModule::buildGeometry()
{
    // Old Ring::initialize strip set (ring.cpp:93-100): three LODs x two
    // halves; slice counts from config (setLodSlices), stacks 4/8/16.
    // Ring2D plans its vertex upload through Context::transfer at build -
    // one-time cost at first loaded call (old lazy-initialize parity; S4
    // work-domain candidate). ringVertexArray exists: the ctor ran
    // ringColorFamily().
    for (int lod = 0; lod < 3; ++lod) {
        const int slices = lodSlices[lod];
        const int stacks = 4 << lod; // 4/8/16, old initialize
        strips[lod * 2 + 0] = std::make_unique<Ring2D>(innerRadius, outerRadius, slices, stacks, true, *ringVertexArray);
        strips[lod * 2 + 1] = std::make_unique<Ring2D>(innerRadius, outerRadius, slices, stacks, false, *ringVertexArray);
    }
}

bool RingModule::update(ModularBody *body, float scaledRadius)
{
    // Extent contract (header): drawn extent = scaled outer radius; the
    // ring-inclusive body screenSize this produces is old-path behavior
    // (BigBody::getOnScreenSize, body_bigbody.cpp:270-279).
    const float bodyRadius = body->getRadius();
    mc = (bodyRadius > 0) ? scaledRadius / bodyRadius : 1.f;
    boundingRadius = outerRadius * mc;
    return true;
}

void RingModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!loaded)
        return; // drawLoaded path re-enters once isLoaded (C3)
    const FamilyBound bound = renderer.bind(ringColorFamily(), 0);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - self-named at definition
    auto &v = **uVert;
    v.ModelViewMatrix = mat;
    v.ModelViewMatrixInverse = mat.inverse();
    v.clipping_fov = renderer.getClippingFov();
    v.RingScale = mc;
    const Vec3f bodyPos = mat.getTranslation();
    v.PlanetPosition = bodyPos;
    // Old computeDraw:1032-1045: eye-space direction body -> sun, normalized.
    Vec3f light = ModularBody::getLightPosition() - bodyPos;
    light.normalize();
    v.LightDirection = light;
    // Observer side of the ring plane (old ring.cpp:252-255): z column dot
    // translation of the draw matrix.
    const float h = mat.r[8] * mat.r[12] + mat.r[9] * mat.r[13] + mat.r[10] * mat.r[14];
    v.SunnySideUp = (h > 0.f) ? 1.f : 0.f;
    v.fadingFactor = 100000.f; // no asteroid variant yet (old else-branch, ring.cpp:279)
    fillPlainShadows(*uFrag, body, this);
    bound.layout->bindSet(renderer, *set);
    // LOD by on-screen px (old thresholds, ring.cpp:284-297), half by h.
    const float px = body->getScreenSize() * 2.f * ModularBody::viewportRadius;
    const int lod = (px < 30.f) ? 0 : (px < 300.f) ? 1 : 2;
    Ring2D *strip = strips[lod * 2 + (h > 0.f ? 0 : 1)].get();
    if (strip)
        strip->draw(renderer);
}

void RingModule::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Row-8 TRACE consumer (INTENT §11.40): the ring annulus into the orbit-
    // union depth range so an orbit line is cut behind the ring. Old
    // Ring::drawDepthTrace (ring.cpp:300-304): push mc into the depthTrace
    // ModelViewMatrix's radius slot, draw lowUP. The new-path ring-trace
    // family is a distinct pipeline+layout (not the body's), so push the FULL
    // TraceInfo here (the old path shared one layout across sphere+ring, hence
    // it re-pushed only mc). Geometry: the LOW-LOD up half (old lowUP) - a
    // coarse silhouette suffices for a depth cut.
    if (!loaded)
        return; // strips not built yet (C3; the color draw guards the same way)
    Ring2D *strip = strips[0].get(); // low, up half (buildGeometry: lod0 h=true)
    if (!strip)
        return;
    const FamilyBound bound = renderer.bind(TraceFamily::ring(ringVertexArray.get()));
    if (!bound.layout)
        return; // trace shader not deployed - C3 degrade (orbits draw depth-free)
    TraceInfo info;
    info.ModelViewMatrix = mat;                    // body mat (same as the COLOR ring draw)
    info.clipping_fov = renderer.getClippingFov(); // the ORBIT range this frame
    info.planetScaledRadius = mc;                  // RingScale (old drawDepthTrace push)
    info.planetOneMinusOblateness = 1.f;           // ring is flat (z=0) - no effect
    bound.layout->pushConstant(renderer, 0, &info);
    strip->draw(renderer);
}

void RingModule::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    // TEXTURED_ANNULUS job (typed vocabulary, ShadowService). The set is
    // created lazily HERE because the service initializes at its first
    // enabled use - drawShadow only runs once it is up (computeShadows gate).
    if (!texSet) {
        texSet = renderer.shadow.makeAnnulusTexSet(tex->getTexture());
        if (!texSet)
            return; // service not ready - skip this frame (C3 ladder)
    }
    renderer.shadow.produceAnnulus(idx, mat, texSet.get(), innerRadius / outerRadius);
}

ShadowCaster RingModule::getShadowCaster(ModularBody *body, const Vec3f &lightPos) const
{
    // Silhouette extent = SCALED outer radius (the unscaled form was a latent
    // defect: under body scaling the drawn ring and its shadow diverged -
    // closed 2026-07-18 with the extent contract). Absorbtion = the material
    // transmission (D8 key or the derived old-parity {0.7} - the selection
    // maps it to (aT, gR=0), ShadowProjection.hpp). Clip half-space: ring
    // plane through the body center, normal = spin axis toward the sun.
    const Vec3f center = body->getObservedPosition();
    const Mat4f &m = body->getMat();
    Vec3f n(m.r[8], m.r[9], m.r[10]);
    if (n.dot(lightPos - center) < 0)
        n = -n;
    return {outerRadius * mc, shadowColor, Vec4f(n[0], n[1], n[2], -n.dot(center))};
}
