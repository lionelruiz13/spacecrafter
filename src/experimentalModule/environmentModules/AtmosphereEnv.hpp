#ifndef ATMOSPHERE_ENV_HPP_
#define ATMOSPHERE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Atmosphere;

// Draw the atmosphere seen from the ground
class AtmosphereEnv : public EnvironmentModule {
public:
    AtmosphereEnv() = default;
    // Copy the photometric outputs of the engine into state
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
};

#endif
