#include "StarFieldEnv.hpp"
#include "../ModularBody.hpp"
#include "../Camera.hpp"
#include "../EnvironmentManager.hpp"
#include "tools/sc_const.hpp"
#include <algorithm>

bool StarFieldEnv::update(ModularBody *body, const Vec3f &cameraLocalPos,
                          float deltaTime, EnvironmentState &state)
{
    const CameraSkyView &view = EnvironmentManager::instance->getSkyView();
    StarFieldInput &in = EnvironmentManager::instance->starInput;
    in.j2000ToLocal = view.j2000ToLocal;
    in.law = view.law;
    // The grid search keeps a zone only if a corner is inside: exact for d <= 0
    in.cone[0].n = view.getPrecEquVision();
    in.cone[0].d = std::min<double>(cos(ModularBody::cullHalfFov), 0);
    in.fovDeg = view.getFov();
    in.altitudeM = Camera::instance->distanceToReference() * (1000 * AU);
    return false;
}
