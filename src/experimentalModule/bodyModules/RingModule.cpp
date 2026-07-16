#include "RingModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Resource/Set.hpp"

RingModule::RingModule(std::unique_ptr<s_texture> tex, float innerRadius, float outerRadius) :
    BodyModule(BodyModuleType::RING), tex(std::move(tex)),
    innerRadius(innerRadius), outerRadius(outerRadius)
{
}

RingModule::~RingModule() = default;

void RingModule::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    // Declare the TEXTURED_ANNULUS job (typed vocabulary, ShadowService).
    // The set is created lazily HERE because the service initializes at its
    // first enabled use, possibly after this module loaded - and drawShadow
    // only ever runs once the service is up (computeShadows gates on it).
    if (!texSet) {
        texSet = renderer.shadow.makeAnnulusTexSet(tex->getTexture());
        if (!texSet)
            return; // service not ready - skip this frame (C3 ladder)
    }
    renderer.shadow.produceAnnulus(idx, mat, texSet.get(), innerRadius / outerRadius);
}

ShadowCaster RingModule::getShadowCaster(ModularBody *body, const Vec3f &lightPos) const
{
    // Silhouette extent = OUTER ring radius (the annulus quad's unit).
    // Absorbtion {0.7,0.7,0.7}: old parity - body_ringed.frag:54 composed
    // diffuse *= mix(1.0, 0.3, alpha) == 1 - alpha*0.7, i.e. coverage = alpha
    // (the layer) times a neutral 0.7 depth (the entry). A per-ring
    // shadow-color data key is a convergence point, not implemented.
    // Clip half-space: the ring plane through the body center, normal = the
    // spin axis (getMat column 2 - the SAME axis the receiver sun-frame
    // basis uses) oriented toward the sun; receivers apply the entry only
    // beyond the plane (dot(P, n) + w <= 0) - the z-order the z-less layer
    // cannot carry (ShadowProjection.hpp derivation).
    const Vec3f center = body->getObservedPosition();
    const Mat4f &m = body->getMat();
    Vec3f n(m.r[8], m.r[9], m.r[10]);
    if (n.dot(lightPos - center) < 0)
        n = -n;
    return {outerRadius, Vec3f(0.7f, 0.7f, 0.7f), Vec4f(n[0], n[1], n[2], -n.dot(center))};
}
