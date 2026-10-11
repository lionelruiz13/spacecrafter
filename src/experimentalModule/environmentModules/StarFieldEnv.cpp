#include "StarFieldEnv.hpp"
#include "../ModularBody.hpp"
#include "../Camera.hpp"
#include "../EnvironmentManager.hpp"
#include "../Renderer.hpp"
#include "navModule/navigator.hpp" // mat_j2000_to_vsop87 (extern const Mat4d)
#include "tools/sc_const.hpp"
#include <algorithm>

bool StarFieldEnv::update(ModularBody *body, const Vec3f &cameraLocalPos,
                          float deltaTime, EnvironmentState &state)
{
    const Camera &camera = *Camera::instance;
    const Mat4d eyeToLocal(Vec3d(camera.observedToLocalPos(Vec3f(1, 0, 0))),
                           Vec3d(camera.observedToLocalPos(Vec3f(0, 1, 0))),
                           Vec3d(camera.observedToLocalPos(Vec3f(0, 0, 1))),
                           Vec3d(0, 0, 0));
    const Mat4d j2000ToEye = rotationOf(body->getMat()) * mat_j2000_to_vsop87;
    StarFieldInput &in = EnvironmentManager::instance->starInput;
    in.j2000ToLocal = eyeToLocal * j2000ToEye;
    in.law.localToEye = eyeToLocal.transpose();
    in.law.halfFov = ModularBody::halfFov;
    in.law.mode = ModularBody::projectionMode;
    in.law.radius = Renderer::getViewportRadius();
    in.law.cx = Renderer::getViewportCenter()[0];
    in.law.cy = Renderer::getViewportCenter()[1];
    // The grid search keeps a zone only if a corner is inside: exact for d <= 0
    in.cone[0].n = j2000ToEye.transpose().multiplyWithoutTranslation(Vec3d(0, 0, -1));
    in.cone[0].d = std::min<double>(cos(ModularBody::cullHalfFov), 0);
    in.fovDeg = ModularBody::halfFov * (360. / M_PI);
    in.altitudeM = camera.distanceToReference() * (1000 * AU);
    return false;
}
