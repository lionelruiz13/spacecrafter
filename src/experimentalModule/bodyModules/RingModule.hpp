#ifndef RING_MODULE_HPP_
#define RING_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class s_texture;
class Set;

// RING slot - planetary ring (landing zone of the old Ring on BigBody,
// including the instanced asteroid-ring variant).
//
// IMPLEMENTED (2026-07-16, shadow-composition wave): the G8 CASTER half -
// rings project their transparency-graded shadow onto the planet (within-body
// pair) and onto anything in the light corridor (moons - a case the old path
// never covered). drawShadow declares a TEXTURED_ANNULUS job to the
// ShadowService (radial alpha = coverage); getShadowCaster supplies the outer
// radius as silhouette extent, the old-parity absorbtion ({0.7,0.7,0.7} ==
// the old body_ringed.frag mix(1.0, 0.3, alpha) composition under the S5
// change of variable), and the ring-plane clip half-space (z-less layer gate,
// ShadowProjection.hpp derivation).
//
// ROW-4 SCOPE (S1+S3-gated, not yet implemented): draw (ring surface, up/down
// halves by camera side, LOD by distance - old pipelineRing), drawTrace (ring
// depth for orbit-line occlusion - old pipelineDepthTrace), RECEIVING the
// planet's shadow in its color draw (BMT_RECEIVE_SHADOW + the meshShadowFill
// idiom - the within-body mesh entry is already emitted by the selection).
// boundingRadius stays 0 until then: a module's boundingRadius is its DRAWN
// extent (feeds body screenSize via halfAngularSize), and the caster half
// draws nothing - setting the outer radius here would inflate Saturn's
// screenSize ~x2.3 with no drawn content (the 11.26(2) coupling class, to
// escalate WITH the color port where 10.3(6) makes the larger radius
// mandatory).
// Asteroid variant: mass-instanced small bodies - BodyType::MINOR_BODY +
// MBT_REPLICATED territory (cluster-optimized, exempt from inter-body
// shadowing; the old threadAsteroid pre-generation becomes a work-domain
// task).
// Deduction rule: param tex_ring (ModularBody.cpp deduceBodyModuleList);
// loader = RingLoader (registered since the caster half landed).
class RingModule : public BodyModule {
public:
    // Radii in AU (unscaled - the same convention as ModularBody::getRadius,
    // which the mesh caster vocabulary uses); texture = radial strip, alpha =
    // opacity, OUTER edge at u=0 (old body_ringed.frag:52 orientation).
    RingModule(std::unique_ptr<s_texture> tex, float innerRadius, float outerRadius);
    ~RingModule();
    virtual uint32_t getTraits() const override {
        return BMT_PROJECT_G8_SHADOW;
    }
    // Caster half only - color drawing is row-4 scope (see header).
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override {}
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const override;
    virtual bool update(ModularBody *body, float scaledRadius) override {
        boundingRadius = 0; // no drawn extent yet (header block)
        return true;
    }
protected:
    std::unique_ptr<s_texture> tex;
    std::unique_ptr<Set> texSet; // SHADOW_RING set 1, created lazily at first drawShadow
    float innerRadius;
    float outerRadius;
};

#endif /* end of include guard: RING_MODULE_HPP_ */
