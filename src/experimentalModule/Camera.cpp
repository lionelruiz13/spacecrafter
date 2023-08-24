#include "Camera.hpp"
#include "ModularSystem.hpp"
#include <cmath>

// Remark : neutral is (1, 0, 0), up is (0, 0, 1)

Camera *Camera::instance = nullptr;
float Camera::minHalfFov = 8.7e-7;
float Camera::maxHalfFov = 3.05;

Camera::Camera(ModularBody *reference, float longitude, float latitude, float altitude) :
    reference(reference), longitude(longitude), latitude(latitude), distance(altitude+reference->getAltitudeReference())
{
    instance = this;
    reference->enterEnvironment();
    system = ModularSystem::systemOf(reference);
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
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
    setFreeMode(oldFreeMode);
    setBoundToSurface(oldBoundToSurface);
    if (oldFreeMode)
        recomputeAltAzHeading();
}

void Camera::warpToBody(ModularBody *dst)
{
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
}

void Camera::update(double jd, float deltaTime)
{
    if (target) // Note : the tracked position is from the last update
        lookTo(observedToLocalPos(target->getObservedPosition()), 5, true);
    if (zoomDuration) {
        zoomTimer += deltaTime;
        if (zoomTimer > zoomDuration) {
            zoomDuration = 0;
            ModularBody::halfFov = dstHalfFov;
        } else {
            ModularBody::halfFov = srcHalfFov * std::pow(dstHalfFov/srcHalfFov, calculateZoomCoef());
        }
    }
    view.update(deltaTime);
    if (moveDuration) {
        moveDuration -= deltaTime;
        if (moveDuration < 0) {
            deltaTime += moveDuration;
            moveDuration = 0;
        }
        if (freeMode) {
            position += deltaPosition * deltaTime;
        } else {
            longitude += deltaPosition[0] * deltaTime;
            latitude += deltaPosition[1] * deltaTime;
            distance += deltaPosition[2] * deltaTime;
        }
    }
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
    system->updateSystem();
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

void Camera::lookTo(const Vec3f &direction, float duration, bool isMaxDuration)
{
    if ((direction[0] + direction[1]) == 0) {
        az = std::copysign(M_PI_2, direction[2]);
    } else {
        Utility::rectToSphe(&az, &alt, direction);
    }
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}

void Camera::lookTo(float _alt, float _az, float duration, bool isMaxDuration)
{
    alt = _alt;
    az = _az;
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}

void Camera::lookRel(float deltaAlt, float deltaAz, float duration, bool isMaxDuration)
{
    alt = std::fmod(alt+deltaAlt, M_PI*2);
    az = std::fmod(az+deltaAz, M_PI*2);
    view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(az, alt)), duration, isMaxDuration);
}

void Camera::moveRel(const Vec3f &deltaPos, float duration, bool calculateDuration)
{
    if (duration > 0) {
        if (calculateDuration)
            duration *= (fabs(deltaPos[0]) + fabs(deltaPos[1])) * 50;
        moveDuration = duration;
        deltaPosition = deltaPos / duration;
    } else {
        if (freeMode) {
            position += deltaPos;
        } else {
            longitude += deltaPos[0];
            latitude += deltaPos[1];
            distance += deltaPos[2];
        }
    }
}

void Camera::moveHeading(float deltaHeading)
{
    view.setRelRotation(Vec4f::zrotation(deltaHeading));
    heading += deltaHeading;
}

void Camera::setHeading(float _heading)
{
    moveHeading(_heading - heading);
}

void Camera::setHalfFov(float halfFov, float duration)
{
    if (halfFov < minHalfFov) {
        halfFov = minHalfFov;
    } else if (halfFov > maxHalfFov) {
        halfFov = maxHalfFov;
    }
    // halfFov now represents the dstFovFactor
    if (duration + zoomDuration) {
        if (zoomDuration == 0 || ((halfFov > dstHalfFov) ^ (halfFov > ModularBody::halfFov))) {
            // Not currently zooming in this direction, simply start a new zoom without initial velocity
            zoomTimer = 0;
            zoomDuration = duration;
            srcHalfFov = ModularBody::halfFov;
            dstHalfFov = halfFov;
        } else {
            // Already zooming, current fov and inertia must be preserved
            const float oldCoefVelocity = calculateZoomCoefVelocity();
            const float oldInertia = std::pow(dstHalfFov/srcHalfFov, oldCoefVelocity);
            if (duration < zoomDuration - zoomTimer)
                duration = zoomDuration - zoomTimer;
            srcHalfFov = ModularBody::halfFov * ModularBody::halfFov / halfFov;
            dstHalfFov = halfFov;
            zoomDuration = 2 * duration;
            zoomTimer = zoomDuration / 2;
            const float maxNewInertia = std::pow(dstHalfFov/srcHalfFov, calculateZoomCoefVelocity());
            if (oldInertia < maxNewInertia) {
                // Accelerate furthermore (maybe to fix)
                constexpr float delta = 0.5;
                const float accelerationDuration = (delta * duration - oldCoefVelocity * duration*duration) / (2*delta - oldCoefVelocity * duration);
                const float decelerationDuration = duration - accelerationDuration;
                zoomTimer = duration - accelerationDuration * 2;
                zoomDuration = duration + zoomTimer;
            } else {
                // Cut duration to decelerate on time
                zoomDuration *= maxNewInertia / oldInertia;
                zoomTimer = zoomDuration / 2;
            }
        }
    } else { // No transition, apply the change immediately
        ModularBody::halfFov = halfFov;
    }
}

void Camera::setAltitude(double altitude)
{
    distance = altitude*1000*AU+reference->getAltitudeReference();
}
