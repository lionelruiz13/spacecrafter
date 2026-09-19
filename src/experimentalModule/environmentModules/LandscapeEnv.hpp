#ifndef LANDSCAPE_ENV_HPP_
#define LANDSCAPE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Landscape;

class LandscapeEnv : public EnvironmentModule {
public:
    LandscapeEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    // Draw in the landscape frame (x=South, y=East, z=Up)
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Owned and re-seated by Core at every swap, may be null
    static Landscape *engine;
};

#endif
