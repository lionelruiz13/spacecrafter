#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"
#include "atmosphereModule/atmosphere_commun.hpp"

class ModularBody;
class Renderer;

// Altitude thresholds of the environment of a body, in meters
struct BodyEnvironmentParams {
    float limInf = 40000.f;       // Atmosphere drawn below
    float limSup = 80000.f;       // Inside the atmosphere below
    float limLandscape = 10000.f; // Landscape drawn below
    bool hasAtmosphere = false;
    ATMOSPHERE_MODEL model = ATMOSPHERE_MODEL::NONE_MODEL;
};

// Reset by the manager each frame, then filled by the update() of the active members
struct EnvironmentState {
    float worldAdaptationLuminance = 3.75f; // Value without atmosphere
    float skyBrightness = 0;
    float atmosphereIntensity = 0; // Eclipse dimming included
    bool atmosphereUserFlag = false; // Input, broadcast by the manager
    bool insideAtmosphere = false;
    bool atmosphereActive = false; // insideAtmosphere && atmosphereUserFlag && below limInf
    bool drawLandscape = false;
    bool drawBody = true;          // !drawLandscape
    bool allowMeteors = false;     // The sky brightness test stays at the consumer
};

class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    // Raise resource priorities, never load synchronously
    virtual void enter(ModularBody *body) {}
    // Lower priorities, the lowest LoD must stay resident
    virtual void leave(ModularBody *body) {}
    // cameraLocalPos in AU, only its length is meaningful; return true to stop updates until enter()
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                        float deltaTime, EnvironmentState &state) {
        return true;
    }
    // Draw before the sky and the bodies; mat orientation is valid only above the current system
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Draw after the bodies, same mat
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
