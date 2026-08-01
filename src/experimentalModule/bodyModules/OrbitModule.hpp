#ifndef ORBIT_MODULE_HPP_
#define ORBIT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <memory>

class VertexBuffer;

// ORBIT slot - the orbit line of a body (landing zone of the old
// OrbitPlot/Orbit2D/Orbit3D). Two hooks:
// - draw: the orbit polyline (color pass). Data: the body's Orbit object
//   (sampled into a point buffer), parent transform, per-body orbit color.
// - drawTrace: NOT this module's line - the trace pass belongs to the BODY
//   mesh cutting its hole (see BodyModule.hpp TRACE / TraceFamily.hpp). The
//   orbit LINE is the consumer of those holes: it is drawn depth-tested
//   against the trace buffer so it hides behind bodies (old cmdBodyDepth/
//   cmdOrbit pass pair).
// Regime: far components - an orbit is visible when its body may not be. The
// module is NOT routed into a screen-size regime list, though: the whole
// orbit pass (trace + line) is a SYSTEM-level phase driven by
// ModularSystem::drawOrbits after the body draw (old solarsystem_display.cpp
// orbit phase), under the Renderer's orbit-union depth range
// (getOrbitDepthBucket, §11.30). So orbit modules live only in the slot + the
// dedicated orbitComponents list (ModularBody).
//
// Representation: the old path shipped Orbit2D AND Orbit3D but instantiated
// ONLY Orbit3D (body_bigbody/moon/smallbody/artificial - grep 2026-07-19).
// Orbit2D is dead generality; the port carries the 3D form only (body_orbit3d
// shaders VERBATIM) and records the retirement (INTENT §11.39). One
// representation matches the header's single `draw`.
//
// Flags (master model, AXIS precedent): showPlanets/showSatellites are the
// global masters set by the seam (setFlagPlanetsOrbits/Satellites). A module's
// fader targets the master applicable to its body CLASS - satellite (parent is
// a body) -> showSatellites, else showPlanets - unless a per-name override is
// set (setFlagPlanetsOrbits(name,b), old body->setFlagOrbit direct set).
// Deduction: default for bodies with a non-still orbit; param orbit=false
// suppresses the module entirely (deduce gate, not a runtime flag).
class OrbitModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (AxisModule precedent). Reconciles the landing-zone header's inline ctor.
    OrbitModule(const Vec3f &color, bool closeOrbit);
    ~OrbitModule();
    // COLOR draw of the orbit line, in the PARENT position frame (mat), under
    // the orbit-union depth range. Advances the fader (tick-at-draw: orbit
    // modules have no regime-list update() channel; the phase calls update()
    // explicitly, but the fader tick lives with the draw for the same-frame
    // consumption HintModule uses).
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Advances the fader toward the effective target and (re)samples the orbit
    // points when visible. Called by ModularSystem::drawOrbits per frame.
    virtual bool update(ModularBody *body, float scaledRadius) override;
    // Runtime orbit-color seam (old Body::setColor "orbit"). Self-selects on
    // the ORBIT channel; every other channel hits the base no-op.
    virtual void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::ORBIT || type == BodyColorType::ALL)
            color = c;
    }

    // Per-name override (old body->setFlagOrbit): forces this body's orbit
    // regardless of the master, but only until the NEXT global toggle - the
    // generation stamp makes a later setGlobal* overwrite it (old parity:
    // setFlagPlanetsOrbits bulk-set every body's fader, clobbering per-name).
    bool getColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::ORBIT)
            return false;
        out = color;
        return true;
    }
    void captureAuthored() override { authoredColor = color; }
    bool getAuthoredColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::ORBIT)
            return false;
        out = authoredColor;
        return true;
    }
    //! -1 = follows the master (an override staled by a global toggle counts as
    //! following: that is exactly what `wantShown` does with it).
    int getShownOverride() const override {
        return (overrideGen == flagGeneration) ? nameOverride : -1;
    }
    virtual void setShown(bool b) override {
        nameOverride = b ? 1 : 0;
        overrideGen = flagGeneration;
        // Kick the system-level orbit phase so this module's update() runs and
        // its fader can rise even when both global masters are off (the phase
        // is gated on anyActive() - see below).
        if (b && !live) { live = true; ++activeCount; }
    }
    // THE UNHIDE EDGE (B39 §11.117 / D23 clause iv): while its body was hidden
    // this module received no update() at all (the whole subtree was out of
    // ModularSystem::drawOrbits' sweep), so its DISPLAY fader - which advances in
    // wall time - froze wherever it stood. Snapping it to the target it would
    // have settled at is the as-if answer: a fade lasts under a second, and
    // without this an orbit line switched OFF while the body was hidden fades out
    // AFTER the body reappears. Nothing else of this module is time-behind: the
    // sampled polyline is re-derived from the body's own (barrier-refreshed) date
    // by update()'s resample test on the next frame.
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(wantShown(body));
        const bool nowLive = fader.getInterstate() > 1e-6f;
        if (nowLive != live) { // keep the phase-gate counter update() maintains
            live = nowLive;
            activeCount += nowLive ? 1 : -1;
        }
    }
    // Global masters + their generation (seam entry - both-paths mirror). A
    // global toggle bumps the generation, staling every per-name override.
    static void setGlobalPlanets(bool b) { showPlanets = b; ++flagGeneration; }
    static void setGlobalSatellites(bool b) { showSatellites = b; ++flagGeneration; }
    // The orbit pass (ModularSystem::drawOrbits) is skipped entirely when this
    // is false - the default (orbits off) pays nothing (no trace sweep, no
    // depth clear). A master being on kicks the phase; a fading-out module
    // keeps it alive (activeCount) until its fader reaches 0, so the fade
    // completes. Row 8 perf gate (the always-run cost was an S9 concern).
    static bool anyActive() { return showPlanets || showSatellites || activeCount > 0; }

    static bool showPlanets;    // old setFlagPlanetsOrbits (global master)
    static bool showSatellites; // old setFlagSatellitesOrbits (global master)
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount;     // modules with a live fader (phase-gate input)
    // Config default orbit color (planet_orbits_color), wired at the
    // setDefaultBodyColor seam like HintModule::defaultLabelColor.
    static Vec3f defaultColor;
protected:
    // Effective visibility target for THIS body this frame (master ∧ class,
    // or the per-name override). Structural class test (I4): a satellite has a
    // BODY parent (parent && !parent->isSystem()); a planet's parent is the
    // system node.
    bool wantShown(ModularBody *body) const;
    // (Re)sample ORBIT_POINTS positions of the orbit over one visualization
    // period, parent-relative (root-aligned VSOP87 - old computeOrbit shape,
    // full recompute; the old incremental cache is an S9 perf item).
    void sampleOrbit(ModularBody *body);

    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // what the DATA gave it (D30's delta baseline)
    bool closeOrbit;
    bool live = false;             // this module currently counts in activeCount
    int8_t nameOverride = -1;      // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;      // flagGeneration at which the override was set

    // Sampled orbit polyline (parent-relative), + the smoothing points that
    // carry the line cleanly through the body center (old ORBIT_ADDITIONNAL).
    std::unique_ptr<Vec3d[]> orbitPoint;
    bool sampled = false;
    double lastSampleJD = 0;

    // GPU line buffer (ORBIT_POINTS + ORBIT_ADDITIONNAL_POINTS vec3), refilled
    // each drawn frame from orbitPoint via the transfer path (old Orbit3D).
    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: ORBIT_MODULE_HPP_ */
