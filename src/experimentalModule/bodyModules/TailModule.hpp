#ifndef TAIL_MODULE_HPP_
#define TAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <vector>

// Draw comet tails, with mat the parent position frame
class TailModule : public BodyModule {
public:
    struct SubTail {
        float deltaTraceJD;      // velocity sampling window, in days
        float ejectionForce;
        float ejectionLinearity; // velocity-vs-radial ejection blend
        Vec3f coefRadius;        // quadratic profile {xx, x, base}
        Vec3f color;
        float lastJD = 0;        // JD of the cached values
        Vec3f cachedExpansionInitial{};    // parent-frame
        Vec3f cachedExpansionCorrection{}; // parent-frame
        Vec3f cachedCoefRadius{};          // coefRadius * comaDiameter (AU)
    };

    TailModule(std::vector<SubTail> &&subTails,
               float absoluteMagnitude, float slopeParameter);
    ~TailModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;

    // Return false if the whole tail phase can be skipped
    static bool anyActive() { return activeCount > 0; }

protected:
    // With r the heliocentric distance in AU
    Vec2f comaDiameterAndTailLengthAU(float r);
    // Return the position at date jd, summed up the parent chain
    static Vec3f orbitPositionAtDate(ModularBody *body, double jd);

    std::vector<SubTail> subTails;
    float absoluteMagnitude; // H, param apparent_magnitude
    float slopeParameter;    // G, param slope
    float lastR = 0;         // cache key of cachedComaTailAU
    Vec2f cachedComaTailAU{};// {comaDiameter, tailLength}
    bool drawThisFrame = false; // false when comaDiameter > tailLength

    static int activeCount;  // live TAIL modules
};

#endif /* end of include guard: TAIL_MODULE_HPP_ */
