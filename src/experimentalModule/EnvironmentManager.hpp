#ifndef ENVIRONMENT_MANAGER_HPP_
#define ENVIRONMENT_MANAGER_HPP_

#include "EnvironmentModule.hpp"
#include "atmosphereModule/atmosphere.hpp" // AtmosphereComputeInput
#include <vector>

class Camera;
class ModularBody;
class ModularSystem;
class MilkyWay;
class Landscape;
class Renderer;
class ToneReproductor;

// Aggregates the EnvironmentModule members of the chain reference -> root: fires their enter()/leave() on the chain
// difference between two frames, runs their update() into one EnvironmentState and dispatches their draws.
class EnvironmentManager {
public:
    EnvironmentManager(MilkyWay *milky, Atmosphere *atmosphere);
    ~EnvironmentManager();
    // deltaTime in seconds. driveEngines: only the active (drawing) path may
    // write shared engine state - the modular phase flag during migration.
    void update(Camera &camera, double jd, float deltaTime, bool driveEngines);
    // Chain members, root-most first; to call before the sky content and the bodies
    void drawBackdrop(Renderer &renderer);
    // Chain members, then the grounded members of the reference while state.drawLandscape; to call after the bodies
    void drawSky(Renderer &renderer);
    inline const EnvironmentState &getState() const {
        return state;
    }
    // The user's atmosphere intent, broadcast to the members; to set wherever BodyDecor::setAtmosphereState is called
    inline void setAtmosphereUserFlag(bool b) {
        atmosphereUserFlag = b;
    }
    inline bool getAtmosphereUserFlag() const {
        return atmosphereUserFlag;
    }
    // Re-seat the landscape engine: to call whenever Core swaps the Landscape object
    void setLandscape(Landscape *landscape);
    // Input of Atmosphere::computeColor, built by update() on the main thread and read by the executor's async job
    inline const AtmosphereComputeInput &getAtmosphereInput() const {
        return atmInput;
    }
    // Engine access for members instantiated before wiring (AtmosphereEnv).
    inline Atmosphere *getAtmosphereEngine() const {
        return atmosphere;
    }
    // To call from the destructor of a body: drops it from the chain kept across frames, without firing leave()
    static void notifyBodyDestroyed(ModularBody *body);
    // Frame clock for members needing absolute time (zodiacal rotation).
    double julianDay = 0;
    // Zodiacal light placement: sun direction in the eye frame, orbit-plane normal of the home body in the root-aligned
    // frame (the member rotates it at draw). zodiacalValid false -> the engine falls back to its time-rotation one
    Vec3d zodiacalSunDirEye;
    Vec3d zodiacalEclipticNormalRoot;
    bool zodiacalValid = false;
    static EnvironmentManager *instance;
private:
    void buildAtmosphereInput(Camera &camera, ModularBody *reference);
    EnvironmentState state;
    AtmosphereComputeInput atmInput;
    std::vector<ModularBody *> activeChain; // last update's chain (diff base)
    std::vector<ModularBody *> newChain;    // scratch (avoid per-frame alloc)
    MilkyWay *milky;
    Atmosphere *atmosphere;
    ModularBody *lastReference = nullptr; // atmosphere model write edge
    ModularSystem *system = nullptr;
    bool atmosphereUserFlag = false; // broadcast input (old BodyDecor::atmState)
    bool onBody = false;
};

#endif /* end of include guard: ENVIRONMENT_MANAGER_HPP_ */
