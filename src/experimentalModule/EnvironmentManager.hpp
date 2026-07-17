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

// ============================================================================
// Aggregation authority of the environment layer (EnvironmentModule.hpp) -
// the BodyDecor replacement on the new path. Owned by SSystemFactory during
// migration (it needs only the Camera and the frame clock; it slides into
// the frame task at the S4 inversion, where "aggregated by the system"
// becomes literal).
//
// Per frame (update):
//   1. Resolve the active chain (reference -> isolated root) and fire
//      enter()/leave() member edges on the chain DIFF (single edge
//      authority - see ModularBody::enterEnvironment note).
//   2. Seed EnvironmentState (defaults + broadcast inputs: the atmosphere
//      user flag - old BodyDecor::atmState, mirrored from its three write
//      sites), then run member update()s: grounded members only while the
//      camera is anchored on the reference (onBody), InAoI members for every
//      chain body. Fixed combination rules applied here (drawBody =
//      !drawLandscape).
//   3. Compute skyBrightness (the old executor formula, camera-sourced sun).
//   4. Snapshot the atmosphere compute input (consumed by the executor's
//      async job through the work queue's happens-before - the compute
//      itself stays on the old path's work thread, same thread parity).
//   5. driveEngines (modular phase only): write the shared engines exactly
//      where the old BodyDecor branches wrote them (atmosphere fader target,
//      milkyway iris selection, atmosphere model on reference change).
//      Fader writes are change-gated by AFader::operator= itself.
//
// Draw dispatch (called from the executor at the exact frame positions the
// old calls occupied - frame-sequencing input during migration):
//   drawBackdrop: chain InAoI members, root-most first (milkyway 2D behind
//                 everything; old call position = before nebulas/stars).
//   drawSky:      chain InAoI members (atmosphere - after the bodies), then
//                 the reference's grounded members (landscape+fog) gated by
//                 state.drawLandscape - the old executor order and gate.
// ============================================================================
class EnvironmentManager {
public:
    EnvironmentManager(MilkyWay *milky, Atmosphere *atmosphere);
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
    // New-path input snapshot for Atmosphere::computeColor - built in
    // update() on the main thread, read by the executor's async job (ordered
    // by the work queue push/pop).
    inline const AtmosphereComputeInput &getAtmosphereInput() const {
        return atmInput;
    }
    // Engine access for members instantiated before wiring (AtmosphereEnv).
    inline Atmosphere *getAtmosphereEngine() const {
        return atmosphere;
    }
    // Frame clock for members needing absolute time (zodiacal rotation).
    double julianDay = 0;
    // Zodiacal placement inputs (MilkyWay::ZodiacalInput supplier, new-path
    // authority - INTENT 11.32): sun direction already in the eye frame
    // (star observed position); the home-body orbit-plane normal stays in
    // the ROOT-ALIGNED frame - MilkyWayEnv rotates it with its own chain
    // rotation at draw (flat-chain contract: all root-aligned frames share
    // one orientation). valid=false -> engine falls back to the simple
    // time-rotation placement.
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
