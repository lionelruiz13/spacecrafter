#ifndef TRAIL_MODULE_HPP_
#define TRAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <vector>
#include <memory>

class VertexBuffer;

class TrailModule : public BodyModule {
public:
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
    virtual void dumpState(std::ostream &out) const override;

    void startTrail(bool record);

    struct TrailPoint {
        Vec3f pos;   // parent-relative (root-aligned), drawn in the parent frame
        double jd;   // sim time of the sample
    };
    virtual void setShown(bool b) override;
    bool getColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::TRAIL)
            return false;
        out = color;
        return true;
    }
    void captureAuthored() override { authoredColor = color; }
    bool getAuthoredColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::TRAIL)
            return false;
        out = authoredColor;
        return true;
    }
    int getShownOverride() const override {
        return (overrideGen == flagGeneration) ? nameOverride : -1;
    }
    const std::vector<TrailPoint> &getPoints() const { return points; }
    void restorePoints(std::vector<TrailPoint> &&pts) {
        points = std::move(pts);
        firstPoint = points.empty();
        if (!points.empty())
            lastJD = points.front().jd;
    }

    virtual void resumeAfterHidden(ModularBody *body) override;

    // Global master + generation (seam entry - both-paths mirror). A global
    // toggle bumps the generation, staling every per-name override.
    static void setGlobalShow(bool b) { show = b; ++flagGeneration; }
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
    void resetTrail();

    std::vector<TrailPoint> points; // NEWEST FIRST (index 0 = brightest - old push_front)
    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // what the DATA gave it (D30's delta baseline)
    double lastJD = 0;      // sim time of the last appended point
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
