#ifndef TAIL_MODULE_HPP_
#define TAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <vector>

class TailModule : public BodyModule {
public:
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

    static bool anyActive() { return activeCount > 0; }

protected:
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
