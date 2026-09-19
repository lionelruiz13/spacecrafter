#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"
#include "atmosphereModule/atmosphere_commun.hpp" // ATMOSPHERE_MODEL (3-line enum header)

class ModularBody;
class Renderer;

// Altitude thresholds of the environment of a body, in meters
struct BodyEnvironmentParams {
    float limInf = 40000.f;       // below: atmosphere drawn (when user flag on)
    float limSup = 80000.f;       // below: inside the atmosphere zone
    float limLandscape = 10000.f; // below: grounded attachment (landscape)
    bool hasAtmosphere = false;
    ATMOSPHERE_MODEL model = ATMOSPHERE_MODEL::NONE_MODEL;
};

// Per-frame aggregate of the active members: reset by the manager, filled by their update(), then read by everyone
struct EnvironmentState {
    float worldAdaptationLuminance = 3.75f; // old no-atmosphere baseline
    float skyBrightness = 0;
    float atmosphereIntensity = 0; // fade-weighted; eclipse dimming included
    bool atmosphereUserFlag = false; // input: the user's atmosphere intent, broadcast by the manager
    bool insideAtmosphere = false; // below limSup with hasAtmosphere
    bool atmosphereActive = false; // atmosphere actually drawn: insideAtmosphere && atmosphereUserFlag && below limInf
    bool drawLandscape = false;    // below limLandscape (grounded attachment)
    bool drawBody = true;          // !drawLandscape, set by the manager
    bool allowMeteors = false;     // insideAtmosphere && userFlag (the
                                   // sky_brightness < 0.1 half stays at the
                                   // meteor consumer, as in the old executor)
};

// Rendering which depends on where the camera is relative to a body: milkyway, from-ground atmosphere, landscape...
// Active for every body of the chain reference -> root; grounded members only while the camera is anchored on the body.
class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    // Camera entered the space this environment covers: raise resource priorities, never load synchronously
    virtual void enter(ModularBody *body) {}
    // Camera left: lower priorities; the lowest LoD stays resident so re-entry always has something drawable
    virtual void leave(ModularBody *body) {}
    // Per frame while active: contribute to state. cameraLocalPos in AU, only its length is meaningful; deltaTime in s
    // Return true when no update is required until the next enter()
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                        float deltaTime, EnvironmentState &state) {
        return true;
    }
    // Drawn before the sky content and every body (milkyway, zodiacal light). mat = frame of body -> eye,
    // valid for orientation only when body is above the current system
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Drawn after the bodies (from-ground atmosphere, landscape, fog); same mat
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
