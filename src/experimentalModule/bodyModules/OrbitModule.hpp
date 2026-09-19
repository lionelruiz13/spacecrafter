#ifndef ORBIT_MODULE_HPP_
#define ORBIT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <memory>

class VertexBuffer;

// Orbit line of a body. In no regime list: updated and drawn by the system-level orbit phase
// (ModularSystem::drawOrbits), depth-tested against the holes the body meshes cut in the trace pass
class OrbitModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    OrbitModule(const Vec3f &color, bool closeOrbit);
    ~OrbitModule();
    // mat = the PARENT position frame
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Advances the fader toward the effective target and (re)samples the orbit
    // points when visible. Called by ModularSystem::drawOrbits per frame.
    virtual bool update(ModularBody *body, float scaledRadius) override;
    // Answers the ORBIT channel only; the base no-op handles every other channel
    virtual void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::ORBIT || type == BodyColorType::ALL)
            color = c;
    }

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
    // Per-name override: holds until the next global toggle
    virtual void setShown(bool b) override {
        nameOverride = b ? 1 : 0;
        overrideGen = flagGeneration;
        if (b && !live) { live = true; ++activeCount; }
    }
    // No update() reaches a hidden body: settle the fader, keeping live/activeCount consistent
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(wantShown(body));
        const bool nowLive = fader.getInterstate() > 1e-6f;
        if (nowLive != live) { // keep the phase-gate counter update() maintains
            live = nowLive;
            activeCount += nowLive ? 1 : -1;
        }
    }
    // Global masters: a toggle bumps the generation, staling every per-name override
    static void setGlobalPlanets(bool b) { showPlanets = b; ++flagGeneration; }
    static void setGlobalSatellites(bool b) { showSatellites = b; ++flagGeneration; }
    // False -> the whole orbit phase is skipped; a module still fading out keeps it alive
    static bool anyActive() { return showPlanets || showSatellites || activeCount > 0; }

    static bool showPlanets;    // global master (setFlagPlanetsOrbits)
    static bool showSatellites; // global master (setFlagSatellitesOrbits)
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount;     // modules with a live fader (phase-gate input)
    // Orbit color of a body whose data gives none (config planet_orbits_color)
    static Vec3f defaultColor;
protected:
    // Visibility target: the per-name override, else the master of the body's class (satellite = parent is a body)
    bool wantShown(ModularBody *body) const;
    // (Re)sample the orbit over one visualization period, parent-relative
    void sampleOrbit(ModularBody *body);

    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // what the DATA gave it (baseline of a saved color change)
    bool closeOrbit;
    bool live = false;             // this module currently counts in activeCount
    int8_t nameOverride = -1;      // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;      // flagGeneration at which the override was set

    // Sampled orbit polyline (parent-relative) + the smoothing points carrying the line through the body center
    std::unique_ptr<Vec3d[]> orbitPoint;
    bool sampled = false;
    double lastSampleJD = 0;

    // GPU line buffer (ORBIT_POINTS + ORBIT_ADDITIONNAL_POINTS vec3), refilled each drawn frame from orbitPoint
    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: ORBIT_MODULE_HPP_ */
