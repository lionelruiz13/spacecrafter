#ifndef ENVIRONMENT_MANAGER_HPP_
#define ENVIRONMENT_MANAGER_HPP_

#include "EnvironmentModule.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include <vector>

class Camera;
class ModularBody;
class ModularSystem;
class MilkyWay;
class Landscape;
class Renderer;
class ToneReproductor;

// Run the EnvironmentModule of every body from the reference up to the root
class EnvironmentManager {
public:
    EnvironmentManager(MilkyWay *milky, Atmosphere *atmosphere);
    ~EnvironmentManager();
    // deltaTime in seconds; driveEngines = allowed to write the shared engine state
    void update(Camera &camera, double jd, float deltaTime, bool driveEngines);
    // Call before the sky content and the bodies
    void drawBackdrop(Renderer &renderer);
    // Call after the bodies
    void drawSky(Renderer &renderer);
    inline const EnvironmentState &getState() const {
        return state;
    }
    inline void setAtmosphereUserFlag(bool b) {
        atmosphereUserFlag = b;
    }
    inline bool getAtmosphereUserFlag() const {
        return atmosphereUserFlag;
    }
    // Call whenever Core swaps the Landscape object
    void setLandscape(Landscape *landscape);
    // Built by update() on the main thread, read by the async job of the executor
    inline const AtmosphereComputeInput &getAtmosphereInput() const {
        return atmInput;
    }
    inline Atmosphere *getAtmosphereEngine() const {
        return atmosphere;
    }
    // Call from the destructor of a body, leave() is not fired
    static void notifyBodyDestroyed(ModularBody *body);
    double julianDay = 0;
    Vec3d zodiacalSunDirEye;
    Vec3d zodiacalEclipticNormalRoot;
    bool zodiacalValid = false; // If false, the engine falls back to its time rotation
    static EnvironmentManager *instance;
private:
    void buildAtmosphereInput(Camera &camera, ModularBody *reference);
    EnvironmentState state;
    AtmosphereComputeInput atmInput;
    std::vector<ModularBody *> activeChain; // Chain of the last update
    std::vector<ModularBody *> newChain;    // Scratch, avoid per-frame alloc
    MilkyWay *milky;
    Atmosphere *atmosphere;
    ModularBody *lastReference = nullptr;
    ModularSystem *system = nullptr;
    bool atmosphereUserFlag = false;
    bool onBody = false;
};

#endif /* end of include guard: ENVIRONMENT_MANAGER_HPP_ */
