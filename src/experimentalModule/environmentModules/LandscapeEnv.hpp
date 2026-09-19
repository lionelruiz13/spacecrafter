#ifndef LANDSCAPE_ENV_HPP_
#define LANDSCAPE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Landscape;

class LandscapeEnv : public EnvironmentModule {
public:
    LandscapeEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    static Landscape *engine;
};

#endif
