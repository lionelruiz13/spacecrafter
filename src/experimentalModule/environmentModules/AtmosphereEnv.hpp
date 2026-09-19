#ifndef ATMOSPHERE_ENV_HPP_
#define ATMOSPHERE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Atmosphere;

// From-ground atmosphere of a body with has_atmosphere=true, drawn by the app's Atmosphere engine
// The engine is reached through EnvironmentManager::instance, wired after this member is built
class AtmosphereEnv : public EnvironmentModule {
public:
    AtmosphereEnv() = default;
    // Sets the atmosphere gates and copies the engine's photometric outputs into state
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
};

#endif
