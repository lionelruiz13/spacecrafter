#include "CameraSkyView.hpp"
#include "Camera.hpp"
#include "ModularBody.hpp"
#include "Renderer.hpp"
#include "EnvironmentModule.hpp"
#include "navModule/navigator.hpp" // mat_j2000_to_vsop87 (extern const Mat4d)
#include "tools/context.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include <algorithm>

// Return m in double precision
static Mat4d toMat4d(const Mat4f &m)
{
    Mat4d d;
    for (int i = 0; i < 16; ++i)
        d.r[i] = m.r[i];
    return d;
}

// Return the matrix whose columns are the images of the three basis vectors
template<class F>
static Mat4d basisImages(F image)
{
    return Mat4d(Vec3d(image(Vec3f(1, 0, 0))), Vec3d(image(Vec3f(0, 1, 0))),
                 Vec3d(image(Vec3f(0, 0, 1))), Vec3d(0, 0, 0));
}

CameraSkyView::CameraSkyView()
{
    dome.localToEye.set(0, -1, 0, 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
    dome.halfFov = M_PI_2;
}

void CameraSkyView::update(const Camera &camera, const ModularBody &system)
{
    const Mat4d eyeToLocal = basisImages(Camera::oldLocalToLocal).transpose()
        * basisImages([&camera](const Vec3f &v) { return camera.observedToLocalPos(v); });
    law.localToEye = eyeToLocal.transpose();
    law.halfFov = ModularBody::halfFov;
    law.mode = ModularBody::projectionMode;
    dome.radius = law.radius = Renderer::getViewportRadius();
    dome.cx = law.cx = Renderer::getViewportCenter()[0];
    dome.cy = law.cy = Renderer::getViewportCenter()[1];
    earthEquToEye = basisImages([&camera](const Vec3f &v) { return camera.observedToBodyEquPos(v); }).transpose();
    const double lat = std::clamp(camera.getLatitude() * (180. / M_PI), -89.5, 89.5);
    earthEquFixedToEye = law.localToEye
        * (Mat4d::zrotation(-camera.getLongitude()) * Mat4d::yrotation((90. - lat) * (M_PI / 180.))).transpose();
    eclipticToEye = earthEquToEye * Mat4d::xrotation(23.4392803055555555556*(M_PI/180));
    const Mat4d systemToEye = toMat4d(system.getMat());
    j2000ToEye = EnvironmentModule::rotationOf(system.getMat()) * mat_j2000_to_vsop87;
    galacticToEye = j2000ToEye * Mat4d::zrotation(14.8595*(M_PI/180))*Mat4d::yrotation(-61.8717*(M_PI/180))*Mat4d::zrotation(55.5*(M_PI/180));
    j2000ToLocal = eyeToLocal * j2000ToEye;
    earthEquToLocalMat = eyeToLocal * earthEquToEye;
    localToEarthEquMat = earthEquToLocalMat.transpose();
    helioToLocalMat = eyeToLocal * systemToEye;
    helioToEarthEquMat = earthEquToEye.transpose() * systemToEye;
    precEquVision = j2000ToEye.transpose().multiplyWithoutTranslation(Vec3d(0, 0, -1));
}

bool CameraSkyView::projectLocal(const Vec3d &v, Vec3d &win) const
{
    return law.project(v, win);
}

// The disc lies inside the viewport: the projection is the check
bool CameraSkyView::projectLocalCheck(const Vec3d &v, Vec3d &win) const
{
    return law.project(v, win);
}

bool CameraSkyView::projectEarthEqu(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(earthEquToEye, v, win);
}

bool CameraSkyView::projectEarthEquFixed(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(earthEquFixedToEye, v, win);
}

bool CameraSkyView::projectEarthEcliptic(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(eclipticToEye, v, win);
}

bool CameraSkyView::projectJ2000(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(j2000ToEye, v, win);
}

bool CameraSkyView::projectJ2000Check(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(j2000ToEye, v, win);
}

bool CameraSkyView::projectJ2000LineCheck(const Vec3d &v1, Vec3d &win1, const Vec3d &v2, Vec3d &win2) const
{
    return projectFrom(j2000ToEye, v1, win1) && projectFrom(j2000ToEye, v2, win2);
}

bool CameraSkyView::projectJ2000Galactic(const Vec3d &v, Vec3d &win) const
{
    return projectFrom(galacticToEye, v, win);
}

bool CameraSkyView::projectDomeFixed(const Vec3d &v, Vec3d &win) const
{
    return dome.project(v, win);
}

Vec3d CameraSkyView::getCursorPosEqu(int x, int y) const
{
    double sx = x;
    if (Context::rearProjection) // The final blit mirrors the frame
        sx = 2. * law.cx - sx;
    const auto pos = VulkanMgr::instance->screenToRect({static_cast<uint16_t>(sx), static_cast<uint16_t>(y)});
    return earthEquToEye.transpose().multiplyWithoutTranslation(law.direction(pos.first, -pos.second));
}

double CameraSkyView::getFov() const
{
    return ModularBody::halfFov * (360. / M_PI);
}

int CameraSkyView::getViewportWidth() const
{
    return static_cast<int>(2 * Renderer::getViewportRadius());
}

int CameraSkyView::getViewportHeight() const
{
    return static_cast<int>(2 * Renderer::getViewportRadius());
}

double CameraSkyView::getViewportRadius() const
{
    return Renderer::getViewportRadius();
}

Vec3d CameraSkyView::getViewportCenter() const
{
    return Vec3d(Renderer::getViewportCenter()[0], Renderer::getViewportCenter()[1], Renderer::getViewportRadius());
}

Mat4f CameraSkyView::getMatProjectionOrtho2D() const
{
    return Renderer::getMvp2D();
}

void CameraSkyView::printGravity180(s_font *font, float x, float y, const std::string &str, Vec4f Color, float xshift, float yshift) const
{
    Context::instance->renderer.printGravityPx(font, x, y, str, Color, xshift, yshift);
}

Mat4f CameraSkyView::getMatLocalToEye() const
{
    return law.localToEye.convert();
}

Mat4f CameraSkyView::getMatJ2000ToEye() const
{
    return j2000ToEye.convert();
}

Mat4f CameraSkyView::getMatEarthEquToEye() const
{
    return earthEquToEye.convert();
}

Vec3d CameraSkyView::earthEquToLocal(const Vec3d &v) const
{
    return earthEquToLocalMat * v;
}

Vec3d CameraSkyView::localToEarthEqu(const Vec3d &v) const
{
    return localToEarthEquMat * v;
}

Vec3d CameraSkyView::helioToLocal(const Vec3d &v) const
{
    return helioToLocalMat * v;
}

Vec3d CameraSkyView::helioToEarthEqu(const Vec3d &v) const
{
    return helioToEarthEquMat * v;
}

const Vec3d &CameraSkyView::getPrecEquVision() const
{
    return precEquVision;
}
