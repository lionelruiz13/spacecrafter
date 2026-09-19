#ifndef TRAIL_MODULE_HPP_
#define TRAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <vector>
#include <memory>

class VertexBuffer;

// Record and draw the path of a body, from ModularSystem::drawTrails
class TrailModule : public BodyModule {
public:
    TrailModule(const Vec3f &color, int maxTrail, double deltaTrail);
    ~TrailModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::TRAIL || type == BodyColorType::ALL)
            color = c;
    }
    virtual void dumpState(std::ostream &out) const override;

    // Restart from the current position if record, else stop recording
    void startTrail(bool record);

    struct TrailPoint {
        Vec3f pos;   // parent-relative
        double jd;
    };
    // Override by name until the next global toggle, restart if enabled
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
    // For saving and restoring the trail whole
    const std::vector<TrailPoint> &getPoints() const { return points; }
    void restorePoints(std::vector<TrailPoint> &&pts) {
        points = std::move(pts);
        firstPoint = points.empty();
        if (!points.empty())
            lastJD = points.front().jd;
    }

    // Settle the fader and re-evaluate the missed samples from the orbit
    virtual void resumeAfterHidden(ModularBody *body) override;

    static void setGlobalShow(bool b) { show = b; ++flagGeneration; }
    // Return false if the whole trail phase can be skipped
    static bool anyActive() { return show || activeCount > 0; }

    static bool show;
    static uint32_t flagGeneration; // bumped by every global toggle
    static int activeCount; // modules with a live fader
    static Vec3f defaultColor; // config object_trails_color
protected:
    // Return the per-name override while live, else the global master
    bool wantShown(ModularBody *body) const;
    // Append the current position, at deltaTrail cadence
    void accumulate(ModularBody *body);
    void resetTrail();

    std::vector<TrailPoint> points; // newest first
    LinearFader fader;
    Vec3f color;
    Vec3f authoredColor;   // as given by the data
    double lastJD = 0;      // sim time of the last appended point
    bool recording = false; // follows the display flag, not the visibility
    bool firstPoint = true; // pending fresh start
    uint64_t accumulateCount = 0; // for dumpState
    int maxTrail = 1460;    // point cap + time window in deltaTrail units
    double deltaTrail = 1;  // sampling period in sim days

    bool live = false;         // counted in activeCount
    int8_t nameOverride = -1;  // -1 follow master, 0/1 forced
    uint32_t overrideGen = 0;

    std::unique_ptr<VertexBuffer> line;
};

#endif /* end of include guard: TRAIL_MODULE_HPP_ */
