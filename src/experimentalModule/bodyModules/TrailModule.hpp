#ifndef TRAIL_MODULE_HPP_
#define TRAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <vector>
#include <memory>

class VertexBuffer;

// Historical path of a body: parent-relative positions accumulated over sim time, drawn as a fading depth-less polyline
// In no regime list: swept every frame by ModularSystem::drawTrails, so an off-screen body keeps recording
// Recording is gated by the display FLAG (off = history discarded), independently of the body's visibility
class TrailModule : public BodyModule {
public:
    TrailModule(const Vec3f &color, int maxTrail, double deltaTrail);
    ~TrailModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
    // Answers the TRAIL channel only; the base no-op handles every other channel
    virtual void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::TRAIL || type == BodyColorType::ALL)
            color = c;
    }
    virtual void dumpState(std::ostream &out) const override;

    // true = fresh restart from the body's current position; false = stop recording
    void startTrail(bool record);

    struct TrailPoint {
        Vec3f pos;   // parent-relative (root-aligned), drawn in the parent frame
        double jd;   // sim time of the sample
    };
    // Per-name override, holds until the next global toggle; enabling restarts the trail
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
    // The accumulated trail is session content: saved and restored whole, not re-derived
    const std::vector<TrailPoint> &getPoints() const { return points; }
    void restorePoints(std::vector<TrailPoint> &&pts) {
        points = std::move(pts);
        firstPoint = points.empty();
        if (!points.empty())
            lastJD = points.front().jd;
    }

    // A hidden body is not swept: settle the fader and re-evaluate the missed samples through the body's orbit
    // Without an evaluable orbit, falls back to a logged fresh start
    virtual void resumeAfterHidden(ModularBody *body) override;

    // Global master: a toggle bumps the generation, staling every per-name override
    static void setGlobalShow(bool b) { show = b; ++flagGeneration; }
    // False -> the whole trail phase is skipped (no sweep, no accumulation)
    static bool anyActive() { return show || activeCount > 0; }

    static bool show; // global display master (setFlagTrails)
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount; // modules with a live fader (phase-gate input)
    // Trail color of a body whose data gives none (config object_trails_color)
    static Vec3f defaultColor;
protected:
    // Effective visibility target for THIS body this frame (per-name override
    // while live, else the global master).
    bool wantShown(ModularBody *body) const;
    // Append the body's current parent-relative position at its sim time (deltaTrail cadence, maxTrail cap and window)
    void accumulate(ModularBody *body);
    // The fresh start of every re-enable path: discard the history, restart at the body's current position
    void resetTrail();

    std::vector<TrailPoint> points; // NEWEST FIRST (index 0 = brightest)
    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // what the DATA gave it (baseline of a saved color change)
    double lastJD = 0;      // sim time of the last appended point
    // Driven by the display flag (wantShown), not by the fader nor by the body's visibility
    bool recording = false;
    bool firstPoint = true; // pending fresh start
    // Entries into accumulate() since construction (trace harness)
    uint64_t accumulateCount = 0;
    int maxTrail = 1460;    // point cap + time window in deltaTrail units
    double deltaTrail = 1;  // sampling period in sim days

    bool live = false;         // this module currently counts in activeCount
    int8_t nameOverride = -1;  // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;  // flagGeneration at which the override was set

    // GPU line buffer (maxTrail vec3), refilled each drawn frame from `points`.
    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: TRAIL_MODULE_HPP_ */
