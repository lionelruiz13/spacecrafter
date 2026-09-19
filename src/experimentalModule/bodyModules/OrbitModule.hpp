#ifndef ORBIT_MODULE_HPP_
#define ORBIT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <memory>

class VertexBuffer;

// Draw the orbit line, from ModularSystem::drawOrbits only
class OrbitModule : public BodyModule {
public:
    OrbitModule(const Vec3f &color, bool closeOrbit);
    ~OrbitModule();
    // With mat the parent position frame
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
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
    int getShownOverride() const override {
        return (overrideGen == flagGeneration) ? nameOverride : -1;
    }
    // Override by name, hold until the next global toggle
    virtual void setShown(bool b) override {
        nameOverride = b ? 1 : 0;
        overrideGen = flagGeneration;
        if (b && !live) { live = true; ++activeCount; }
    }
    // Settle the fader, as a hidden body gets no update
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(wantShown(body));
        const bool nowLive = fader.getInterstate() > 1e-6f;
        if (nowLive != live) {
            live = nowLive;
            activeCount += nowLive ? 1 : -1;
        }
    }
    static void setGlobalPlanets(bool b) { showPlanets = b; ++flagGeneration; }
    static void setGlobalSatellites(bool b) { showSatellites = b; ++flagGeneration; }
    // Return false if the whole orbit phase can be skipped
    static bool anyActive() { return showPlanets || showSatellites || activeCount > 0; }

    static bool showPlanets;
    static bool showSatellites;
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount;     // modules with a live fader
    static Vec3f defaultColor;  // config planet_orbits_color
protected:
    // Return the per-name override, else the master of its class
    bool wantShown(ModularBody *body) const;
    void sampleOrbit(ModularBody *body);

    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // as given by the data
    bool closeOrbit;
    bool live = false;             // counted in activeCount
    int8_t nameOverride = -1;      // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;

    std::unique_ptr<Vec3d[]> orbitPoint; // parent-relative
    bool sampled = false;
    double lastSampleJD = 0;

    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: ORBIT_MODULE_HPP_ */
