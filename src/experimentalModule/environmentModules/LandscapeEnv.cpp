#include "LandscapeEnv.hpp"
#include "../ModularBody.hpp"
#include "../Camera.hpp"
#include "coreModule/landscape.hpp"
#include "tools/sc_const.hpp"

Landscape *LandscapeEnv::engine = nullptr;

bool LandscapeEnv::update(ModularBody *body, const Vec3f &cameraLocalPos,
                          float deltaTime, EnvironmentState &state)
{
    // Altitude in meters above the body's altitude reference (the camera
    // distance convention, validated against the old observer in scenes A-D).
    const double altitudeM = (double)cameraLocalPos.length() * (1000 * AU)
                           - (double)body->getAltitudeReference() * (1000 * AU);
    state.drawLandscape = altitudeM < body->envParams.limLandscape;
    return false;
}

void LandscapeEnv::drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!engine)
        return;
    engine->drawEnv(Camera::instance->viewRotation()
                    .multiplyFast(Mat4f::zrotation(-M_PI_2)));
}
