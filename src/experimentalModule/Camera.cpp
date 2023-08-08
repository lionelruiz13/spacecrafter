#include "Camera.hpp"
#include "ModularSystem.hpp"
#include <cmath>

// Remark : neutral is (1, 0, 0), up is (0, 0, 1)

Camera::Camera(ModularBody *reference, float longitude, float latitude, float altitude) :
    reference(reference), longitude(longitude), latitude(latitude), distance(altitude+reference->getAltitudeReference())
{
}

float Camera::distanceToReference() const
{
   return distance - reference->getAltitudeReference();
}

void Camera::switchToBody(ModularBody *dst)
{
    bool oldFreeMode = freeMode;
    bool oldBoundToSurface = boundToSurface;
    setBoundToSurface(false);
    setFreeMode(true);
    view.setRotation(view.getMatrix().multiplyFast(reference->calculateSwitchCompensation(dst)).toQuaternion());
    reference = dst;
    setFreeMode(oldFreeMode);
    setBoundToSurface(oldBoundToSurface);
    if (oldFreeMode)
        recomputeAltAzHeading();
}

void Camera::update(float jd, float deltaTime)
{
    if (target)
        moveTo(observedToLocalPos(target->getObservedPosition()), 5, true);
    view.update(deltaTime);
    Mat4f mat;
    if (freeMode) {
        if (auto newRef = reference->findBetterReference()) {
            switchToBody(newRef);
        }
        mat = view.getMatrix();
        mat.multiplyTranslation(position);
    } else {
        mat = view.getMatrix();
        mat.multiplyTranslation(Vec3f(0, 0, distance));
        mat = mat.multiplyFast(Mat4f::yrotation(M_PI_2-latitude)).multiplyFast(Mat4f::zrotation(longitude));
    }
    if (boundToSurface)
        mat = mat.multiplyFast(reference->computeBodyToSurface());
    system = ModularBody::dispatchUpdate(reference, jd, mat);
}

void Camera::draw(Renderer &renderer)
{
    system->drawSystem(renderer);
}

void Camera::setFreeMode(bool b)
{
    if (b == freeMode)
        return;
    if (b) {
        view.setRotation(view.getMatrix()
            .multiplyFast(Mat4f::yrotation(M_PI_2-latitude))
            .multiplyFast(Mat4f::zrotation(longitude))
            .toQuaternion()
        );
        Utility::spheToRect(longitude, latitude, position);
        position *= distance;
    } else {
        view.setRotation(view.getMatrix()
            .multiplyFast(Mat4f::zrotation(-longitude))
            .multiplyFast(Mat4f::yrotation(latitude-M_PI_2))
            .toQuaternion()
        );
        Utility::rectToSphe(&longitude, &latitude, position);
        distance = position.length();
    }
    freeMode = b;
    recomputeAltAzHeading();
}

void Camera::recomputeAltAzHeading()
{
    Vec3f direction = view.getMatrix().multiplyWithoutTranslation({1, 0, 0});
    if ((direction[0] + direction[1]) == 0) {
        if (std::signbit(direction[2])) {
            az = -M_PI_2;
            direction = view.getMatrix().multiplyWithoutTranslation({0, 0, 1});
        } else {
            az = M_PI_2;
            direction = view.getMatrix().multiplyWithoutTranslation({0, 0, -1});
        }
        alt = atan2(direction[1], direction[0]) - heading;
    } else {
        Utility::rectToSphe(&az, &alt, direction);
    }
}

void Camera::setBoundToSurface(bool b)
{
    if (b == boundToSurface)
        return;
    if (b) {
        if (freeMode) {
            position = reference->computeSurfaceToBody().multiplyWithoutTranslation(position);
        } else {
            longitude -= reference->getAxisRotation();
        }
    } else {
        if (freeMode) {
            position = reference->computeBodyToSurface().multiplyWithoutTranslation(position);
        } else {
            longitude += reference->getAxisRotation();
        }
    }
    boundToSurface = b;
}

void Camera::moveTo(const Vec3f &direction, float duration, bool isMaxDuration)
{
    if ((direction[0] + direction[1]) == 0) {
        az = std::copysign(M_PI_2, direction[2]);
    } else {
        Utility::rectToSphe(&az, &alt, direction);
    }
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}

void Camera::moveTo(float _alt, float _az, float duration, bool isMaxDuration)
{
    alt = _alt;
    az = _az;
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}

void Camera::moveRel(float deltaAlt, float deltaAz, float duration, bool isMaxDuration)
{
    alt = std::fmod(alt+deltaAlt, M_PI*2);
    az = std::fmod(az+deltaAz, M_PI*2);
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}
