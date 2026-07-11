#ifndef RING_MODULE_HPP_
#define RING_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// RING slot - planetary ring (landing zone of the old Ring on BigBody,
// including the instanced asteroid-ring variant). The most hook-complete
// module family:
// - draw: the ring surface (up/down halves by camera side, LOD by distance).
// - drawTrace: ring depth for orbit-line occlusion (old pipelineDepthTrace).
// - drawShadow / drawSelfShadow: rings cast shadow on the planet AND receive
//   the planet's shadow - both directions ride the shadow pass contracts
//   (Renderer.hpp; ModularBody shadow-projection block).
// Traits: BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_PROJECT_G8_SHADOW (ring
// shadows are graded by transparency).
// Data: ring texture (via the resource layer - heavily shared candidate,
// D7), inner/outer radius, fading factor; light direction from
// ModularBody::getLightPosition().
// Asteroid variant: mass-instanced small bodies - that is BodyType::
// MINOR_BODY + MBT_REPLICATED territory (cluster-optimized, exempt from
// inter-body shadowing; the old threadAsteroid pre-generation becomes a
// work-domain task).
// Deduction rule: param tex_ring (already deduced - ModularBody.cpp
// deduceBodyModuleList); loader registration lands with the implementation.
class RingModule : public BodyModule {
public:
    RingModule() : BodyModule(BodyModuleType::RING) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
protected:
    float innerRadius;
    float outerRadius;
};

#endif /* end of include guard: RING_MODULE_HPP_ */
