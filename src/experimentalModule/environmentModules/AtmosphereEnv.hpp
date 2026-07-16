#ifndef ATMOSPHERE_ENV_HPP_
#define ATMOSPHERE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Atmosphere;

// From-ground atmosphere - InAoI environment member, present iff the body
// has has_atmosphere=true (parse gate identical to the old AtmosphereParams
// block; bodies without it fall to the EnvironmentState defaults, exactly
// BodyDecor::bodyAssign's !hasAtmosphere branch).
//
// Migration form: wraps the app's Atmosphere engine (skylight/skybright
// tables, fader, staging buffer, pipeline - shared with the old path). The
// HEAVY per-frame sky-table compute stays on the executor's async thread
// (asyncUpdateLoop - the old path's work domain; thread-parity preserved,
// snapshot handed through the work queue's happens-before). What this wave
// replaces is the compute's INPUT SOURCE in the modular phase: sun/moon
// directions, lat/alt and per-cell grid directions come from the camera and
// the body chain (EnvironmentManager::buildAtmosphereInput) instead of the
// navigator/projector. update() here contributes the gates + mirrors the
// engine's photometric outputs into the state.
class AtmosphereEnv : public EnvironmentModule {
public:
    AtmosphereEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    // The engine's draw is fader-gated internally (old executor called it
    // unconditionally at this frame position). Pipeline pass is
    // PASS_MULTISAMPLE_DEPTH, executed after the bodies by toExecute order
    // (EnvironmentModule.hpp pass note).
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Engine access rides EnvironmentManager::instance (members are
    // instantiated at body-load time, BEFORE Core creates the engines and
    // wires the manager - solar system loads in the factory constructor).
    // update()/drawSky() only run post-wiring (manager-dispatched).
};

#endif
