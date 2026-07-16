#include "AtmosphereEnv.hpp"
#include "../ModularBody.hpp"
#include "../EnvironmentManager.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "tools/sc_const.hpp"

bool AtmosphereEnv::update(ModularBody *body, const Vec3f &cameraLocalPos,
                           float deltaTime, EnvironmentState &state)
{
    const double altitudeM = (double)cameraLocalPos.length() * (1000 * AU)
                           - (double)body->getAltitudeReference() * (1000 * AU);
    // BodyDecor::bodyAssign, atmosphere half (hasAtmosphere is true by
    // construction - this member only exists on such bodies):
    state.insideAtmosphere = altitudeM <= body->envParams.limSup;
    state.allowMeteors = state.insideAtmosphere && state.atmosphereUserFlag;
    state.atmosphereActive = state.allowMeteors && altitudeM < body->envParams.limInf;
    // Photometric mirror (computed by the engine's async computeColor from
    // the previous frame's inputs - same phase relationship as the old
    // executor, which reads them in update() before the next computeColor).
    Atmosphere *engine = EnvironmentManager::instance->getAtmosphereEngine();
    state.worldAdaptationLuminance = engine->getWorldAdaptationLuminance();
    state.atmosphereIntensity = engine->getIntensity();
    return false;
}

void AtmosphereEnv::drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    EnvironmentManager::instance->getAtmosphereEngine()->draw();
}
