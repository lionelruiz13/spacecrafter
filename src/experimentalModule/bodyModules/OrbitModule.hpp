#ifndef ORBIT_MODULE_HPP_
#define ORBIT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <memory>

class VertexBuffer;

class OrbitModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (AxisModule precedent). Reconciles the landing-zone header's inline ctor.
    OrbitModule(const Vec3f &color, bool closeOrbit);
    ~OrbitModule();
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
        if (b && !live) { live = true; ++activeCount; }
    }
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
    static bool anyActive() { return showPlanets || showSatellites || activeCount > 0; }

    static bool showPlanets;    // old setFlagPlanetsOrbits (global master)
    static bool showSatellites; // old setFlagSatellitesOrbits (global master)
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount;     // modules with a live fader (phase-gate input)
    // Config default orbit color (planet_orbits_color), wired at the
    // setDefaultBodyColor seam like HintModule::defaultLabelColor.
    static Vec3f defaultColor;
protected:
    bool wantShown(ModularBody *body) const;
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
