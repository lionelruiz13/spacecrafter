#ifndef TRAIL_MODULE_HPP_
#define TRAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"
#include <vector>

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
class TrailModule : public BodyModule {
public:
    TrailModule() : BodyModule(BodyModuleType::TRAIL) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;

    void startTrail(bool record);
    static bool show; // old setFlagTrails
protected:
    struct TrailPoint {
        Vec3f pos;
        double jd;
    };
    std::vector<TrailPoint> points;
    LinearFader fader;
    Vec3f color;
    double lastJD = 0;
    bool recording = false;
};

#endif /* end of include guard: TRAIL_MODULE_HPP_ */
