#ifndef TRAIL_MODULE_HPP_
#define TRAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <vector>
#include <memory>

class VertexBuffer;

// TRAIL slot - historical path polyline of a body (landing zone of the old
// Trail friend). Accumulates positions over simulated time in update(),
// draws a fading polyline in the color pass (no depth - screen-legible even
// when the body is small).
// Data: the body's ecliptic position over time (public transform interface),
// per-body trail color, sampling period (old DeltaTrail/MaxTrail).
// Regime: far components. update() keeps returning false while accumulation
// is active (it needs the per-frame tick even when nothing draws).
// Deduction rule: default for moving bodies, created inert; recording starts
// via the seam (startTrails / setFlagTrail).
//
// PORT (INTENT §11.41; row 9) - reconciliation of the landing zone with the
// established line-family shape (ORBIT §11.39 / AXIS §11.31):
// - Frame & accumulation: the accumulation buffer holds PARENT-RELATIVE
//   positions (body->getEclipticPos, root-aligned VSOP87 - old
//   get_heliocentric_ecliptic_pos maps to this for the trail set, whose parent
//   is the ~fixed system root), sampled at the body's SIM time (getLastJD).
//   Both ride recursiveTranslationUpdate, refreshed for EVERY evaluated body
//   even when invisible (§3.2/G4) - the row-9 invisible-tick contract. The
//   trail is NOT in a screen-size regime list (far-components never get
//   update()): it lives in a dedicated trailComponents list swept every frame
//   by ModularSystem::drawTrails (the OrbitModule system-phase precedent), so
//   accumulation continues while the body is off-screen (old drew the trail in
//   both the visible AND off-screen branch, body.cpp:1131/1163).
// - Draw: body_trail.{vert,geom,frag} REUSED VERBATIM (parity by
//   construction). They read main_clipping_fov from cam_block (context.uboSet)
//   - the SAME camera-block authority the new-path body shaders use (I2) - so
//   the family binds the global UBO set; own push contract (frag Vec3f color @0,
//   vert {int nbPoints, mat4, float fader} @12 = old layoutTrail,
//   trail.cpp:192-193). LINE_STRIP + geometry subdivision, BLEND_SRC_ALPHA for
//   the per-vertex fade, NO depth (old setDepthStencilMode() = test+write off).
// - Flags (master model, ORBIT precedent): `show` is the global display master
//   (old setFlagTrails, command `flag object_trails on|off`); a module's fader
//   targets it unless a per-name override (setShown) is live at the current
//   generation.
// - THE RECORDING GATE (§11.56, closes the §11.41 suspension) [vixy Q14
//   2026-07-21]: the DISPLAY FLAG gates RECORDING. Flag off => accumulation
//   STOPS at once and the recorded history is DISCARDED; flag on => recording
//   restarts FRESH from the body's current position. Stated reason: nobody
//   pays for accumulating a trail nobody sees. The gate is the FLAG, not the
//   fader interstate (a toggle inside the fade window is still a re-enable),
//   while the fader stays the DISPLAY gate so the fade-out is unchanged.
// - THE TWO GATES ARE INDEPENDENT, and that is a requirement, not a detail.
//   Trail flag off => not recording, whatever the body's visibility.
//   Body HIDDEN (or merely off-screen) => STILL recording [vixy Q13 / A10,
//   §11.54]: drawTrails sweeps every EVALUATED body, hidden ones included.
//   Reading the two as one gate produces a wrong implementation (§13.B B11).
// - Deduce: TRAIL for a non-still orbit (orbit_visualization_period>0) that is
//   NOT a satellite and NOT type=Artificial (old BigBody+SmallBody set:
//   Planet/Dwarf 1460, Comet 2920, Asteroid/KBO 60; Moon/Sun/Star/Center/
//   Artificial carry no Trail). DeltaTrail is always 1 day (old never data-set).
class TrailModule : public BodyModule {
public:
    // Ctor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (ORBIT/AXIS precedent). Reconciles the landing zone's inline default ctor
    // with the per-body parameters the old Trail carried (color + MaxTrail +
    // DeltaTrail; the loader supplies them).
    TrailModule(const Vec3f &color, int maxTrail, double deltaTrail);
    ~TrailModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
    // Runtime trail-color seam (old Body::setColor "trail"). Self-selects on
    // the TRAIL channel; every other channel hits the base no-op.
    virtual void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::TRAIL || type == BodyColorType::ALL)
            color = c;
    }
    // Harness instrument (INTENT 11.56): the recording gate's observable -
    // point count, recording/fader (the TWO gates, side by side) and
    // accumulateCount (entries into accumulate(): a frozen counter over
    // advancing simulated time is the "the work stopped" evidence).
    virtual void dumpState(std::ostream &out) const override;

    // Old Trail::startTrail: enable => fresh restart (first_point set - the next
    // accumulate clears + starts over); disable => stop (recording, mirrored not
    // relied on - the fader is the accumulation gate). Seam: startTrails (init +
    // perspective reset, core.cpp:369/1720).
    void startTrail(bool record);

    // Per-name override (old Body::setFlagTrail -> Trail::setFlagTrail): forces
    // this body's trail regardless of the master, until the next global toggle
    // (generation stamp = old bulk-set clobber parity). Enabling resets the
    // trail (old startTrail(true)) and kicks the system phase.
    virtual void setShown(bool b) override;

    // Global master + generation (seam entry - both-paths mirror). A global
    // toggle bumps the generation, staling every per-name override.
    static void setGlobalShow(bool b) { show = b; ++flagGeneration; }
    // The trail pass (ModularSystem::drawTrails) is skipped entirely when this
    // is false - the default (trails off) pays nothing (no sweep, no
    // accumulation). Mirrors OrbitModule::anyActive (the always-run sweep hung
    // scene E - the gate is mandatory).
    static bool anyActive() { return show || activeCount > 0; }

    static bool show; // old setFlagTrails (global display master)
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount; // modules with a live fader (phase-gate input)
    // Config default trail color (object_trails_color), wired at the
    // setDefaultBodyColor seam like OrbitModule::defaultColor.
    static Vec3f defaultColor;
protected:
    // Effective visibility target for THIS body this frame (per-name override
    // while live, else the global master).
    bool wantShown(ModularBody *body) const;
    // Append the body's current parent-relative position at its sim time, with
    // the old updateTrail cadence/cap/prune semantics (trail.cpp:120-163).
    void accumulate(ModularBody *body);
    // THE fresh start (Q14): DISCARD the recorded history and arm a restart at
    // the body's current position. Single authority for every re-enable path
    // (global flag rising edge, per-name setShown, old-path startTrail).
    void resetTrail();

    struct TrailPoint {
        Vec3f pos;   // parent-relative (root-aligned), drawn in the parent frame
        double jd;   // sim time of the sample
    };
    std::vector<TrailPoint> points; // NEWEST FIRST (index 0 = brightest - old push_front)
    LinearFader fader;
    Vec3f color;
    double lastJD = 0;      // sim time of the last appended point
    // THE recording gate's state: true = this module is accumulating. Driven by
    // the DISPLAY flag (wantShown), not by the fader - see update()'s gate
    // comment and INTENT 11.56. Independent of the body's visibility.
    bool recording = false;
    bool firstPoint = true; // pending fresh start (old first_point)
    // Instrument (INTENT 11.56): entries into accumulate() since construction.
    // Frozen counter + advancing sim time = the accumulation code did not run.
    uint64_t accumulateCount = 0;
    int maxTrail = 1460;    // old MaxTrail (point cap + time window in DeltaTrail units)
    double deltaTrail = 1;  // old DeltaTrail (sim-day sampling period)

    bool live = false;         // this module currently counts in activeCount
    int8_t nameOverride = -1;  // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;  // flagGeneration at which the override was set

    // GPU line buffer (maxTrail vec3), refilled each drawn frame from `points`.
    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: TRAIL_MODULE_HPP_ */
