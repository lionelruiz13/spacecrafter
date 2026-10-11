#ifndef STAR_FIELD_ENV_HPP_
#define STAR_FIELD_ENV_HPP_

#include "../EnvironmentModule.hpp"

// Build the star field's snapshot for the executor's async job, the stars draw from it
class StarFieldEnv : public EnvironmentModule {
public:
    StarFieldEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
};

#endif
