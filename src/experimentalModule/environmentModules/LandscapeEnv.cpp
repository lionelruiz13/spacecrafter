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
    // BodyDecor::bodyAssign, landscape/body half: below limLandscape the
    // landscape shows and the body's own disc doesn't (drawBody is derived
    // by the manager as !drawLandscape - fixed combination rule).
    state.drawLandscape = altitudeM < body->envParams.limLandscape;
    return false;
}

void LandscapeEnv::drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!engine)
        return;
    // Old-local -> camera-zenith frame conversion, then zenith -> eye.
    // (See header; sky brightness and fader state live in the engine,
    // written by the executor's shared per-frame update.)
    engine->drawEnv(Camera::instance->viewRotation()
                    .multiplyFast(Mat4f::zrotation(-M_PI_2)));
}
