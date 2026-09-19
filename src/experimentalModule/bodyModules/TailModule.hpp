#ifndef TAIL_MODULE_HPP_
#define TAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <vector>

// Comet tails (gas / dust / optional extra): one SubTail each, submitted as one instance of the Renderer's tail batch
// Depth-less; swept by the system-level tail phase (ModularSystem::drawTails), where mat = the PARENT position frame
class TailModule : public BodyModule {
public:
    // The expansion vectors live in the PARENT frame; draw() rotates them into eye space
    struct SubTail {
        float deltaTraceJD;      // orbital-velocity sampling window (days)
        float ejectionForce;     // anti-sunward ejection strength
        float ejectionLinearity; // velocity-vs-radial ejection blend
        Vec3f coefRadius;        // radius quadratic profile {xx, x, base}
        Vec3f color;             // tail RGB
        // JD cache: recomputed only when sim time changes
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

    // False -> the whole tail phase is skipped
    static bool anyActive() { return activeCount > 0; }

protected:
    // {comaDiameter, tailLength} in AU at heliocentric distance r (AU), from the comet magnitude and slope
    Vec2f comaDiameterAndTailLengthAU(float r);
    // Sum the orbit position up the parent chain at date jd (root-aligned VSOP87)
    static Vec3f orbitPositionAtDate(ModularBody *body, double jd);

    std::vector<SubTail> subTails;
    float absoluteMagnitude; // comet photometric H (param apparent_magnitude)
    float slopeParameter;    // activity slope G (param slope)
    float lastR = 0;         // coma/tail-size cache key
    Vec2f cachedComaTailAU{};// {comaDiameter, tailLength} AU (shared across sub-tails)
    bool drawThisFrame = false; // update() gate: false when comaDiameter > tailLength

    static int activeCount;  // live TAIL modules (phase-gate input)
};

#endif /* end of include guard: TAIL_MODULE_HPP_ */
