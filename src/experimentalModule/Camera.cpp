#include "Camera.hpp"
#include "ModularSystem.hpp"
#include "RenderChain.hpp"
#include <cmath>
#include <iomanip>
#include <ostream>

// Remark : neutral is (1, 0, 0), up is (0, 0, 1)
// In spheToRect/rectToSphe, xyz is xzy
// The z axis is inverted : negative is forward

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
    if (target) { // Note : the tracked position is from the last update
        lookTo(observedToLocalPos(target->getObservedPosition()), 5, true);
    }
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
    // Z body_axis
    // X statique, Y et Z bougent avec alt/az
    // (The 2026 Moon-divergence note that lived here is resolved: the delta was
    //  EMB wiring + per-hop tilts + this longitude sign - INTENT.md 11.14,
    //  harness/predict.py carries the measurements.)
    // heading+π: without it the camera frame is rolled 180° about the view
    // axis relative to the old path - the whole sky point-reflected on screen
    // (measured 2026-07-12, horizon-mount A/B: constant 179.994° roll on every
    // body, radii equal to 0.05 px; the halo/hint position divergence). A roll
    // about the view axis leaves the tracked direction invariant, which is why
    // the position-layer harness (P1-P5) never saw it - it was absorbed into
    // P5's D_common. This composition is the SINGLE AUTHORITY on the
    // alt/az/heading convention: observedToLocalPos (Camera.hpp) must remain
    // its exact rotational inverse, and lookTo's az=-lng/alt=-lat inversion is
    // derived from it (roll-invariant, so unaffected by the +π).
    Mat4f mat{Mat4f::zrotation(heading+M_PI).multiplyFast(Mat4f::xrotation(M_PI_2-alt)).multiplyFast(Mat4f::zrotation(az-M_PI_2))};
    if (freeMode) {
        if (auto newRef = reference->findBetterReference()) {
            switchToBody(newRef);
        }
        // mat = view.getMatrix();
        mat.multiplyTranslation(position);
    } else {
        // mat = view.getMatrix();
        mat.multiplyTranslation(Vec3f(0, 0, -distance));
        // -longitude: longitude is east-positive (data/UI convention). Measured
        // against the old path (harness 2026-07-11): with +longitude the
        // observer azimuth in the Earth frame was axisRot - lon, old (exact by
        // its own composition) is sidereal + lon. The setBoundToSurface
        // transitions were already consistent with the negative sign.
        mat = mat.multiplyFast(Mat4f::xrotation(latitude-M_PI_2)).multiplyFast(Mat4f::zrotation(-longitude));
    }
    if (boundToSurface)
        mat = mat.multiplyFast(reference->computeSurfaceToBody());
    lastDispatchedMat = mat; // harness: dump what actually ran (INTENT 11.14a)
    system = ModularBody::dispatchUpdate(reference, jd, mat);
    system->updateSystem();
}

void Camera::draw(Renderer &renderer)
{
    frameDrawTask.renderer = &renderer;
    frameDrawTask.camera = this;
    frameDrawTask.done.store(false, std::memory_order_relaxed);
    RenderChain::instance.execute(&frameDrawTask);
    // Common case: the chain was idle, the task ran inplace above and done is
    // already true. Chained case (in-flight publish): wait - see the BRIDGE
    // note in Camera.hpp.
    while (!frameDrawTask.done.load(std::memory_order_acquire))
        ;
}

void Camera::FrameDrawTask::start(Taskable *target)
{
    camera->system->drawSystem(*renderer);
    done.store(true, std::memory_order_release);
    target->endTask(this);
}

void Camera::setFreeMode(bool b)
{
    if (b == freeMode)
        return;

    // Longitude sign flipped together with Camera::update's surface placement
    // (east-positive convention, see there). PENDING RUNTIME VERIFICATION: the
    // harness only exercises surface mode; free<->surface continuity (and the
    // yrotation-vs-xrotation asymmetry with update()) still needs a dedicated
    // run before being trusted.
    if (b) {
        view.setRotation(view.getMatrix()
            .multiplyFast(Mat4f::yrotation(latitude-M_PI_2))
            .multiplyFast(Mat4f::zrotation(-longitude))
            .toQuaternion()
        );
        Utility::spheToRect(-longitude, latitude, position);
        position *= distance;
    } else {
        view.setRotation(view.getMatrix()
            .multiplyFast(Mat4f::zrotation(longitude))
            .multiplyFast(Mat4f::yrotation(M_PI_2-latitude))
            .toQuaternion()
        );
        Utility::rectToSphe(&longitude, &latitude, position);
        longitude = -longitude;
        distance = position.length();
    }
    freeMode = b;
    recomputeAltAzHeading();
}

void Camera::recomputeAltAzHeading()
{
    Vec3f direction = view.getMatrix().multiplyWithoutTranslation({1, 0, 0});
    if (direction[0] == 0 && direction[1] == 0) { // was x+y==0: misrouted every x==−y direction
        if (std::signbit(direction[2])) {
            alt = M_PI_2;
            direction = view.getMatrix().multiplyWithoutTranslation({0, 0, 1});
        } else {
            alt = -M_PI_2;
            direction = view.getMatrix().multiplyWithoutTranslation({0, 0, -1});
        }
        az = atan2(direction[1], direction[0]) - heading;
    } else {
        Utility::rectToSphe(&az, &alt, direction);
        alt = -alt;
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
    // Inverse of update()'s view build Z(heading)·X(π/2−alt)·Z(az−π/2):
    // centering `direction` (local frame, spheToRect convention) requires
    // az = −lng(direction), alt = −lat(direction). Was az = +lng: together
    // with observedToLocalPos's missing −π/2 the tracking fixed point sat
    // 2·lng−π/2 away from the target in azimuth (see observedToLocalPos).
    if (direction[0] == 0 && direction[1] == 0) {
        // Pole singularity: alt fully determined, az free — keep the current
        // az rather than snapping it. (The previous test x+y==0 also
        // misrouted every x==−y direction here.)
        alt = -std::copysign(M_PI_2, direction[2]);
    } else {
        Utility::rectToSphe(&az, &alt, direction);
        az = -az;
        alt = -alt;
    }
    // view.setRotation(((Mat4f::zrotation(heading).multiplyFast(Mat4f::xrotation(M_PI_2-alt)).multiplyFast(Mat4f::zrotation(az))).toQuaternion()));
    // view.moveTo(((Mat4f::zrotation(heading).multiplyFast(Mat4f::zxrotation(az, alt))).toQuaternion()), duration, isMaxDuration);
}

void Camera::lookTo(float _alt, float _az, float duration, bool isMaxDuration)
{
    alt = _alt;
    az = _az;
    // view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(-az, M_PI_2-alt)), duration, isMaxDuration);
}

void Camera::lookRel(float deltaAlt, float deltaAz, float duration, bool isMaxDuration)
{
    alt = std::fmod(alt+deltaAlt, M_PI*2);
    az = std::fmod(az+deltaAz, M_PI*2);
    // view.moveTo(Vec4f::zrotation(heading).combineQuaternions(Vec4f::zyrotation(-az, -alt)), duration, isMaxDuration);
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
    distance = altitude/(1000*AU)+reference->getAltitudeReference();
}

// Dual-path trace harness (INTENT.md 11.14).
void Camera::dumpTrace(std::ostream &out) const
{
    out << std::setprecision(9) << "{\"reference\":\""
        << (reference ? reference->getEnglishName() : "") << "\",\"freeMode\":"
        << (freeMode ? "true" : "false") << ",\"boundToSurface\":"
        << (boundToSurface ? "true" : "false")
        << ",\"longitude\":" << longitude << ",\"latitude\":" << latitude
        << ",\"distance\":" << distance
        << ",\"alt\":" << alt << ",\"az\":" << az << ",\"heading\":" << heading
        << ",\"position\":[" << position[0] << ',' << position[1] << ',' << position[2]
        << "],\"halfFov\":" << ModularBody::halfFov << ",\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << lastDispatchedMat.r[i] << ((i < 15) ? "," : "");
    out << "]}";
}
