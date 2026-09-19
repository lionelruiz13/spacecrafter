#ifndef LANDSCAPE_ENV_HPP_
#define LANDSCAPE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Landscape;

// Grounded landscape (+ fog) of every landable body: owns the drawLandscape gate and the camera-derived draw
class LandscapeEnv : public EnvironmentModule {
public:
    LandscapeEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    // The landscape mesh lives in the old local frame (x=South, y=East, z=Up): drawn with viewRotation() * Z(-pi/2)
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // The single current landscape, Core-owned; Core re-seats it at every swap (may be null)
    static Landscape *engine;
};

#endif
