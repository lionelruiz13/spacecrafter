#ifndef RING_MODULE_HPP_
#define RING_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class s_texture;
class Set;
class Ring2D;
class VertexArray;
template <typename T> class SharedBuffer;
struct bodyRingVert;
struct bodyRingFrag;

// RING slot - planetary ring (the old Ring on BigBody, row 4).
//
// CASTER half (2026-07-16, shadow-composition wave): TEXTURED_ANNULUS word -
// rings project their transparency-graded shadow onto the planet (within-body
// pair) and onto anything in the light corridor (moons - a case the old path
// never covered). getShadowCaster supplies the SCALED outer radius, the
// material absorbtion and the ring-plane clip half-space.
//
// COLOR + RECEIVE half (row 4, 2026-07-18): bodyRing.vert/.frag - the
// ring_planet.* port. The old analytic planet-shadow test is replaced by
// BMT_RECEIVE_SHADOW: the planet's G1 entry (emitted by computeShadows the
// moment a second receiving module exists on the body) carries the planet
// shadow band, and every corridor caster shades the ring identically (first
// non-disc, non-model receiver). Geometry: the six Ring2D strips (LOD slices
// from config rings_low/medium/high via setLodSlices - the modelRingInit
// both-paths seam; stacks 4/8/16), UP/DOWN half selection by the observer's
// side of the ring plane, LOD by on-screen px (<30 low, <300 medium - old
// Ring::draw thresholds). Built lazily at first loaded draw (old
// Ring::initialize parity; S4 work-domain candidate).
//
// EXTENT CONTRACT (the row-4 escalation, RESOLVED by old-path evidence):
// update() returns boundingRadius = outerRadius * mc (drawn extent,
// 10.3(6)) - body screenSize becomes ring-inclusive, which IS old behavior:
// BigBody::getOnScreenSize uses rings->getOuterRadius()
// (body_bigbody.cpp:270-279) and calculateBoundingRadius takes
// max(atm-factored radius, ring outer) (:283-296). The ~x2.3 Saturn
// screenSize inflation is parity, not divergence; the prior deferral existed
// only because the caster half had no drawn content.
// mc = scaledRadius / bodyRadius covers moon_scale/setScaling by
// construction (old rings->multiplyRadius analog); the caster silhouette
// scales with it (unscaled-radius defect closed 2026-07-18).
//
// Routing: NEAR-ONLY (2026-07-18 correction - the F-era near+far routing
// double-drew in the 0.008-0.2 screenSize band where ModularBody::draw runs
// BOTH lists, and far modules never receive update()). Near membership
// already covers the drawNoDepth band down to ~3 px < the old 5-px ring
// cutoff (body.cpp:1132); below that the old path drew nothing either.
// NO_DEPTH: the family deliberately declares no such variant - bind falls
// back by bit-drop to forced depth, the old ringed behavior (10.3 rule 1,
// body.cpp:1055-1059).
//
// Asteroid variant: row 5 (MINOR_BODY + MBT_REPLICATED, work-domain
// pre-generation). Named divergence until then: the old path shows the
// instanced asteroid ring when observerDistance < 10 * outerRadius.
// Deduction: tex_ring + ring_inner_size/ring_outer_size (RingLoader);
// ring_shadow_color additive data key (absent -> derived old-parity {0.7} -
// shadow-paths.md D8: fidelity decides casting, a color key expands, a gate
// key forecloses).
class RingModule : public BodyModule {
public:
    // Radii in AU (unscaled - the ModularBody::getRadius convention);
    // texture = radial strip, alpha = opacity, OUTER edge at u=0
    // (old body_ringed.frag:52 orientation).
    RingModule(std::unique_ptr<s_texture> tex, float innerRadius, float outerRadius,
               const Vec3f &shadowColor);
    ~RingModule();
    virtual uint32_t getTraits() const override {
        return BMT_PROJECT_G8_SHADOW | BMT_RECEIVE_SHADOW | BMT_USE_DEPTH | BMT_DEPTH_TRACE
             | BMT_TRANSLUCENT; // draws after opaque siblings (routing partition)
    }
    virtual bool isLoaded() override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // NO_DEPTH variant deliberately unsupported (header block) - the base
    // no-op would HIDE the ring in the 0.0015-0.008 band; route to draw,
    // where the registry's bit-drop restores forced depth (old parity).
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override {
        draw(renderer, body, mat);
    }
    // TRACE (ring depth for orbit-line holes, old pipelineDepthTrace):
    // recording is row-8 scope with its ORBIT consumer (verification height -
    // holes have no observable without lines); the trait is declared, the
    // hook stays empty until then (BasicMesh precedent).
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const override;

    // Both-paths seam: LOD slice counts from config rings_low/medium/high
    // (core.cpp modelRingInit path; mirrored by SSystemFactory for the new
    // path). Stacks stay 4/8/16 (old Ring::initialize).
    static void setLodSlices(int low, int medium, int high);

protected:
    void buildGeometry();

    std::unique_ptr<s_texture> tex;
    std::unique_ptr<Set> texSet; // SHADOW_RING set 1, created lazily at first drawShadow
    std::unique_ptr<Set> set;    // RING family set 0
    std::unique_ptr<SharedBuffer<bodyRingVert>> uVert;
    std::unique_ptr<SharedBuffer<bodyRingFrag>> uFrag;
    // Six LOD/half strips (old Ring lowUP..highDOWN); lazily built.
    std::unique_ptr<Ring2D> strips[6];
    float innerRadius;
    float outerRadius;
    float mc = 1;                // body scaling factor (update)
    Vec3f shadowColor;           // material absorbtion (D8 key or derived {0.7})
    bool loaded = false;
    static Vec3i lodSlices;      // config seam (setLodSlices)
};

#endif /* end of include guard: RING_MODULE_HPP_ */
