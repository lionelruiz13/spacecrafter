#include "MilkyWayEnv.hpp"
#include "../ModularBody.hpp"
#include "../EnvironmentManager.hpp"
#include "coreModule/milkyway.hpp"
#include "navModule/navigator.hpp" // mat_j2000_to_vsop87 (extern const Mat4d)

void MilkyWayEnv::drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    Mat4d rot(
        mat.r[0], mat.r[1], mat.r[2], 0,
        mat.r[4], mat.r[5], mat.r[6], 0,
        mat.r[8], mat.r[9], mat.r[10], 0,
        0, 0, 0, 1);
    const EnvironmentManager &mgr = *EnvironmentManager::instance;
    MilkyWay::ZodiacalInput zi;
    if (mgr.zodiacalValid) {
        zi.sunDirEye = mgr.zodiacalSunDirEye;
        zi.eclipticNormalEye = rot * mgr.zodiacalEclipticNormalRoot;
        zi.valid = true;
    }
    engine->drawEnv(eye, rot * mat_j2000_to_vsop87, mgr.julianDay, zi);
}
