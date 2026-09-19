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

class EnvironmentManager {
public:
    EnvironmentManager(MilkyWay *milky, Atmosphere *atmosphere);
    ~EnvironmentManager();
    // deltaTime in seconds. driveEngines: only the active (drawing) path may
    // write shared engine state - the modular phase flag during migration.
    void update(Camera &camera, double jd, float deltaTime, bool driveEngines);
    void drawBackdrop(Renderer &renderer);
    void drawSky(Renderer &renderer);
    inline const EnvironmentState &getState() const {
        return state;
    }
    // Broadcast input mirror (old BodyDecor::setAtmosphereState sites:
    // config load, setLandscapeToBody auto-rule, atmosphereSetFlag command).
    inline void setAtmosphereUserFlag(bool b) {
        atmosphereUserFlag = b;
    }
    inline bool getAtmosphereUserFlag() const {
        return atmosphereUserFlag;
    }
    // Landscape engine re-seat (Core::setLandscape/loadLandscape swap the
    // object - I5: the owner re-seats every dependent reference).
    void setLandscape(Landscape *landscape);
    inline const AtmosphereComputeInput &getAtmosphereInput() const {
        return atmInput;
    }
    // Engine access for members instantiated before wiring (AtmosphereEnv).
    inline Atmosphere *getAtmosphereEngine() const {
        return atmosphere;
    }
    static void notifyBodyDestroyed(ModularBody *body);
    // Frame clock for members needing absolute time (zodiacal rotation).
    double julianDay = 0;
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
