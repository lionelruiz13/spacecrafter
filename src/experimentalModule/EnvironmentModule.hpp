#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"
#include "atmosphereModule/atmosphere_commun.hpp" // ATMOSPHERE_MODEL (3-line enum header)

class ModularBody;
class Renderer;

struct BodyEnvironmentParams {
    float limInf = 40000.f;       // below: atmosphere drawn (when user flag on)
    float limSup = 80000.f;       // below: inside the atmosphere zone
    float limLandscape = 10000.f; // below: grounded attachment (landscape)
    bool hasAtmosphere = false;
    ATMOSPHERE_MODEL model = ATMOSPHERE_MODEL::NONE_MODEL;
};

struct EnvironmentState {
    float worldAdaptationLuminance = 3.75f; // old no-atmosphere baseline
    float skyBrightness = 0;
    float atmosphereIntensity = 0; // fade-weighted; eclipse dimming included
    bool atmosphereUserFlag = false;
    bool insideAtmosphere = false; // below limSup with hasAtmosphere
    bool atmosphereActive = false;
    bool drawLandscape = false;    // below limLandscape (grounded attachment)
    bool drawBody = true;          // !drawLandscape (old bodyAssign coupling)
    bool allowMeteors = false;     // insideAtmosphere && userFlag (the
                                   // sky_brightness < 0.1 half stays at the
                                   // meteor consumer, as in the old executor)
};

class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    virtual void enter(ModularBody *body) {}
    // Camera left. Deactivation edge: lower priorities; the lowest LoD stays
    // resident (D6) so re-entry always has something drawable.
    virtual void leave(ModularBody *body) {}
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                        float deltaTime, EnvironmentState &state) {
        return true;
    }
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
