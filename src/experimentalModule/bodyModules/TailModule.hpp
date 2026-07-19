#ifndef TAIL_MODULE_HPP_
#define TAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <vector>

// TAIL slot - comet gas/dust tail (landing zone of the old Tail on
// SmallBody). Color pass, no depth (tails are translucent overlays).
// BATCHING CONTRACT: the old path batched all tails through a global
// singleton (Tail::global beginDraw/drawBatch/endDraw). Globals-as-batchers
// are exactly what G9 dissolves: batching is a Renderer service - modules
// submit, the Renderer coalesces per pipeline family (BMT_REPLICATED is the
// declared hint). No module-owned global state.
// Data: ejection/coma parameters (from body params), light position and
// observer direction (ModularBody statics/public interface), body magnitude
// inputs (albedo, radius - public).
// Regime: far components (a tail is visible when the nucleus is a dot).
// Deduction rule: comet-typed bodies (param type=Comet) or explicit.
//
// PORT (INTENT §11.43; row 12) - reconciliation of the landing zone with the
// established SYSTEM-PHASE line-family shape (TRAIL §11.41) plus the Renderer
// batching service (HINT §11.24):
// - Regime & sweep: reconciled from "far components" to a SYSTEM phase, like
//   TRAIL. The old tail was accumulated in SmallBody::drawHalo (draw-time,
//   whenever the comet was drawn), batched with the halos, and rendered
//   depth-less; the new path sweeps a dedicated tailComponents list every frame
//   (ModularSystem::drawTails, gated on anyActive()) and hands each module the
//   PARENT POSITION frame (matLocalToBodyPos . translation(-ecl)) - the frame
//   the parent-relative orbit positions live in, which for a sun-parented comet
//   IS the old nav->getHelioToEyeMat() (root-aligned VSOP87 -> eye). This is
//   what makes update() live (a far-routed module never receives update(); the
//   spec declares it, so the module must be swept). The tail no longer draws at
//   the per-body halo-batch boundary but once after the body pass (like TRAIL):
//   a documented occlusion divergence, immaterial for a depth-less translucent
//   overlay (a comet occluded by a nearer planet disc is not in shipped data).
// - Batch: Renderer-owned instanced batch (Renderer::submitTail/beginTailDraw/
//   flushTails) reusing body_tail.{vert,frag}.spv VERBATIM (parity by
//   construction; deployed SPVs, no shader work - T4 precedent). Shared cone/
//   strip geometry via an instance-rate VertexArray + a primitive-restart index
//   (FixedState::stripBreaks), built once and Renderer-owned; the per-tail
//   InstanceData is uploaded per frame (old Tail::endDraw scheme) and drawn with
//   ONE vkCmdDrawIndexed. The pipeline lives in the registry, NOT in a module
//   global (Tail::global dissolved). Push-constant fov + spec-8 projection type
//   (registry-injected, §11.33); NO descriptor set (the tail shader has none) -
//   which is why it rides its own Renderer entry, not the generic uboSet-binding
//   batchPush service (halo/hint). Whether TAIL + row-5 RING_ASTEROID should be
//   folded into a generalized instanced batchPush service is SUSPENDED for Vixy.
// - Multi-tail: the old bound up to THREE Tail objects per comet (gas / dust /
//   optional extra - protosystem.cpp:805-851). A comet's tails are ONE feature,
//   so one TailModule holds a SubTail list (each its own ejection/coefRadius/
//   color/deltaTraceJD + JD cache) and submits one instance per sub-tail. The
//   landing zone's singular {ejectionForce, coefRadius, color} become that list;
//   coefRadius is Vec3f (the old quadratic radius profile {xx,x,base}), not the
//   scalar sketch - a scalar cannot express the tail's radius-over-length shape.
// - Magnitude coupling SUSPENDED (§11.43): the coma/tail SIZE model
//   (projectpluto comet_tail_formula, getComaDiameterAndTailLengthAU) needs an
//   absolute magnitude H + activity slope G (old SmallBody absoluteMagnitude/
//   slopeParameter, from params apparent_magnitude/slope). The landing zone says
//   "albedo, radius (public)" - a NUCLEUS magnitude, physically a different
//   quantity (it excludes coma brightness), which would change every tail size.
//   The DOMINANT observable is the old H/G model: mirrored here (loader carries
//   apparent_magnitude->absoluteMagnitude, slope->slopeParameter); the
//   albedo/radius derivation is Vixy's to decide.
class TailModule : public BodyModule {
public:
    // One sub-tail (old Tail): shape params + per-tail JD cache. The expansion
    // vectors live in the PARENT (root-aligned) frame; draw() rotates them into
    // eye space with the parent position frame the sweep hands in.
    struct SubTail {
        float deltaTraceJD;      // orbital-velocity sampling window (days)
        float ejectionForce;     // anti-sunward ejection strength
        float ejectionLinearity; // velocity-vs-radial ejection blend
        Vec3f coefRadius;        // radius quadratic profile {xx, x, base}
        Vec3f color;             // tail RGB
        // JD cache (old Tail::draw): recomputed only when sim time changes.
        float lastJD = 0;
        Vec3f cachedExpansionInitial{};    // parent-frame
        Vec3f cachedExpansionCorrection{}; // parent-frame
        Vec3f cachedCoefRadius{};          // coefRadius * comaDiameter (AU)
    };

    TailModule(std::vector<SubTail> &&subTails,
               float absoluteMagnitude, float slopeParameter);
    ~TailModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;

    // The tail phase (ModularSystem::drawTails) is skipped entirely when no
    // comet with a tail is loaded - the default (no comets) pays nothing (no
    // sweep). Mirrors TrailModule::anyActive (the always-run sweep hung scene E,
    // §11.39: the gate is mandatory).
    static bool anyActive() { return activeCount > 0; }

protected:
    // Coma diameter + tail length in AU as a function of heliocentric distance
    // r (AU), from the comet photometric H (absoluteMagnitude) + activity slope
    // G (slopeParameter). Faithful port of SmallBody::getComaDiameterAndTailLen
    // gthAU (body_smallbody.hpp:64-78, projectpluto comet_tail_formula), incl.
    // the lastR skip cache. Returns {comaDiameter, tailLength} in AU.
    Vec2f comaDiameterAndTailLengthAU(float r);
    // Sum the orbit position up the parent chain at date jd (root-aligned
    // VSOP87), the old Body::getPositionAtDate form (body.cpp:1291).
    static Vec3f orbitPositionAtDate(ModularBody *body, double jd);

    std::vector<SubTail> subTails;
    float absoluteMagnitude; // old SmallBody::absoluteMagnitude (param apparent_magnitude)
    float slopeParameter;    // old SmallBody::slopeParameter (param slope)
    float lastR = 0;         // coma/tail-size cache key (old SmallBody::lastR)
    Vec2f cachedComaTailAU{};// {comaDiameter, tailLength} AU (shared across sub-tails)
    bool drawThisFrame = false; // update() gate: false when comaDiameter > tailLength

    static int activeCount;  // live TAIL modules (phase-gate input)
};

#endif /* end of include guard: TAIL_MODULE_HPP_ */
