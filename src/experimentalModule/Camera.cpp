#include "Camera.hpp"
#include "JsonNum.hpp"
#include "ModularSystem.hpp"
#include "ModularSystemFormat.hpp"
#include "RenderChain.hpp"
#include "tools/log.hpp"
#include <cmath>
#include <iomanip>
#include <ostream>
#include <sstream>

// Remark : neutral is (1, 0, 0), up is (0, 0, 1)
// In spheToRect/rectToSphe, xyz is xzy
// The z axis is inverted : negative is forward

Camera *Camera::instance = nullptr;
float Camera::minHalfFov = 8.7e-7;
float Camera::maxHalfFov = 3.05;

static constexpr float VIEW_OFFSET_RAMP_RATE = 2.f;

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

Vec3f Camera::getPlace() const
{
    if (!freeMode)
        return Vec3f(longitude, latitude, distanceToReference());
    const Vec3f place = posePartToPose(-position, longitude, latitude);
    return Vec3f(place[0], place[1], position.length() - reference->getAltitudeReference());
}

// ---- View composition authority (see Camera.hpp) ---------------------------

Mat4f Camera::fold() const
{
    if (mount == CameraMount::EQUATORIAL && !freeMode)
        return Mat4f::xrotation(M_PI_2 - foldLat);
    return Mat4f::identity();
}

Mat4f Camera::viewRotation() const
{
    return Mat4f::zrotation(heading+M_PI)
        .multiplyFast(Mat4f::xrotation(M_PI_2-alt))
        .multiplyFast(Mat4f::zrotation(az-M_PI_2))
        .multiplyFast(fold());
}

Mat4f Camera::placementRotation() const
{
    Mat4f m = freeMode ? Mat4f::identity()
        : Mat4f::xrotation(latitude-M_PI_2).multiplyFast(Mat4f::zrotation(-longitude));
    if (boundToSurface)
        m = m.multiplyFast(reference->computeSurfaceToBody());
    return m;
}

Mat4f Camera::viewOffsetEyeRotation() const
{
    const float o = effectiveViewOffset();
    if (o == 0.f)
        return Mat4f::identity();
    return Mat4f::xrotation(o * ModularBody::halfFov);
}

Mat4f Camera::renderViewRotation() const
{
    return viewOffsetEyeRotation().multiplyFast(viewRotation());
}

Mat4f Camera::viewMat() const
{
    // Z body_axis
    // X statique, Y et Z bougent avec alt/az
    Mat4f mat{renderViewRotation()};
    if (freeMode) {
        mat.multiplyTranslation(position);
    } else {
        mat.multiplyTranslation(Vec3f(0, 0, -distance));
        mat = mat.multiplyFast(Mat4f::xrotation(latitude-M_PI_2)).multiplyFast(Mat4f::zrotation(-longitude));
    }
    if (boundToSurface)
        mat = mat.multiplyFast(reference->computeSurfaceToBody());
    return mat;
}

Vec3f Camera::localToBodyEqu(Vec3f v) const
{
    if (!freeMode) {
        v = Mat4f::zrotation(longitude)
                .multiplyFast(Mat4f::xrotation(M_PI_2 - latitude))
                .multiplyWithoutTranslation(v);
    }
    if (boundToSurface)
        v = reference->computeBodyToSurface().multiplyWithoutTranslation(v);
    return v;
}

Vec3f Camera::observedToBodyEquPos(const Vec3f &observedPos) const
{
    return localToBodyEqu(renderViewRotation().transpose()
        .multiplyWithoutTranslation(observedPos));
}

Vec3f Camera::observedToBodyLocalPos(const Vec3f &observedPos) const
{
    return observedToBodyEquPos(observedPos)
        + localToBodyEqu(freeMode ? -position : Vec3f(0, 0, distance));
}

std::pair<float, float> Camera::observedPosToRaDe(const Vec3f &observedPos) const
{
    const Vec3f direction = observedToBodyEquPos(observedPos);
    std::pair<float, float> ret;
    // Pole test was (x + y) == 0, which also swallowed every x == -y
    // direction (same defect class as the old lookTo branch).
    if (direction[0] == 0 && direction[1] == 0) {
        ret.first = 0;
        ret.second = std::copysign(M_PI_2, direction[2]);
    } else {
        Utility::rectToSphe(&ret.first, &ret.second, direction);
    }
    return ret;
}

void Camera::setViewOffset(double offset)
{
    // The [-0.5,0.5] clamp lives in the ONE sink Core::setViewOffset (both S2(c)
    // channels funnel through it, core.cpp); store the already-clamped value.
    viewOffset = offset;
}

void Camera::restoreViewOffsetLatch(bool armed)
{
    viewOffsetArmed = armed;
    // The ramp snaps to the latch it belongs to (D32): the file records a
    // condition, and the transition is the motion into it.
    viewOffsetTransition = armed ? 1.f : 0.f;
}

void Camera::armViewOffset(bool armed)
{
    viewOffsetArmed = armed;
}

void Camera::advanceViewOffset(float deltaTime)
{
    const float target = viewOffsetArmed ? 1.f : 0.f;
    if (viewOffsetTransition == target)
        return;
    const float step = VIEW_OFFSET_RAMP_RATE * deltaTime;
    if (viewOffsetTransition < target) {
        viewOffsetTransition += step;
        if (viewOffsetTransition > target)
            viewOffsetTransition = target;
    } else {
        viewOffsetTransition -= step;
        if (viewOffsetTransition < target)
            viewOffsetTransition = target;
    }
}

Vec3f Camera::paramForward() const
{
    // Inverse of lookTo's parameter derivation (az=-lng, alt=-lat):
    // d = (cos az*cos alt, -sin az*cos alt, -sin alt)
    const float ca = cosf(alt);
    return Vec3f(cosf(az)*ca, -sinf(az)*ca, -sinf(alt));
}

void Camera::recoverParams(const Mat4f &totalRot)
{
    const Mat4f core = totalRot
        .multiplyFast(placementRotation().transpose())
        .multiplyFast(fold().transpose());
    // core = Z(g1)*X(g2)*Z(g3), column-major element (i,j) = r[j*4+i]
    float c2 = core.r[10];
    if (c2 > 1.f) c2 = 1.f;
    if (c2 < -1.f) c2 = -1.f;
    const float g2 = acosf(c2);
    float g1, g3;
    if (fabsf(c2) > 0.999999f) {
        // Pole singularity: only g1+-g3 is defined - keep the current az
        g3 = az - M_PI_2;
        const float base = atan2f(core.r[1], core.r[0]);
        g1 = (c2 > 0.f) ? (base - g3) : (base + g3);
    } else {
        g1 = atan2f(core.r[8], -core.r[9]);
        g3 = atan2f(core.r[2], core.r[6]);
    }
    heading = g1 - M_PI;
    alt = M_PI_2 - g2;
    az = g3 + M_PI_2;
    foldLat = latitude; // params are now derived against the current fold
    // The in-flight plans were expressed in the previous param frame
    viewT = 0;
    hdgT = 0;
}

static bool solvePlan(float v0, float d, float T, float &t1, float &a)
{
    if (T <= 0.f)
        return false;
    if (fabsf(v0) < 1e-9f) {
        t1 = T * 0.5f;
        a = 4.f*d/(T*T);
        return true;
    }
    // t1 = (2d -/+ sqrt(2*(T^2v0^2 - 2Tdv0 + 2d^2)))/(2v0); a = v0/(T - 2t1)
    const float disc = T*T*v0*v0 - 2.f*T*d*v0 + 2.f*d*d; // = (Tv0-d)^2 + d^2 >= 0
    const float s = sqrtf(2.f*disc);
    bool found = false;
    for (const float cand : {(2.f*d - s)/(2.f*v0), (2.f*d + s)/(2.f*v0)}) {
        if (cand >= 0.f && cand <= T && fabsf(T - 2.f*cand) > 1e-9f) {
            const float ca = v0/(T - 2.f*cand);
            if (!found || fabsf(ca) < fabsf(a)) { // prefer minimal |a|
                t1 = cand;
                a = ca;
                found = true;
            }
        }
    }
    if (!found) { // measured zero occurrences over the tested domain; degrade
        t1 = T * 0.5f; // to a fresh plan (small velocity discontinuity) rather
        a = 4.f*d/(T*T); // than a NaN view
    }
    return true;
}

// s(t) and v(t) of the two-phase profile
static inline float planPos(float v0, float a, float t1, float t)
{
    if (t <= t1)
        return v0*t + 0.5f*a*t*t;
    const float v1 = v0 + a*t1;
    const float dt = t - t1;
    return v0*t1 + 0.5f*a*t1*t1 + v1*dt - 0.5f*a*dt*dt;
}

static inline float planVel(float v0, float a, float t1, float t)
{
    return (t <= t1) ? (v0 + a*t) : (v0 + a*t1 - a*(t - t1));
}

// Rodrigues rotation of v about unit axis k by angle
static inline Vec3f rotateAbout(const Vec3f &v, const Vec3f &k, float angle)
{
    const float c = cosf(angle), s = sinf(angle);
    return v*c + (k^v)*s + k*(k.dot(v))*(1.f-c);
}

void Camera::advanceView(float deltaTime)
{
    if (viewT > 0.f) {
        viewTimer += deltaTime;
        float s;
        if (viewTimer >= viewT) {
            s = viewAngle; // exact landing (end velocity 0 by construction)
            viewT = 0.f;
        } else {
            s = planPos(viewV0, viewA, viewT1, viewTimer);
        }
        const Vec3f dir = rotateAbout(viewFrom, viewAxis, s);
        // derive (alt, az) from the smoothed direction - authority inversion
        const float r = dir.length();
        if (dir[0] == 0.f && dir[1] == 0.f) {
            alt = -std::copysign(M_PI_2, dir[2]); // pole: az kept
        } else {
            az = -atan2f(dir[1], dir[0]);
            alt = -asinf(dir[2]/r);
        }
    }
    if (hdgT > 0.f) {
        hdgTimer += deltaTime;
        if (hdgTimer >= hdgT) {
            heading = hdgTarget;
            hdgT = 0.f;
        } else {
            heading = hdgFrom + planPos(hdgV0, hdgA, hdgT1, hdgTimer);
        }
    }
}

void Camera::switchToBody(ModularBody *dst)
{
    dst->useNow();
    bool oldFreeMode = freeMode;
    bool oldBoundToSurface = boundToSurface;
    setBoundToSurface(false);
    setFreeMode(true); // placement is now identity: total rotation == viewRotation()
    // The compensation maps dst-local coordinates to current-reference-local
    // coordinates - the same view over the new chain (visual continuity).
    const Mat4f comp = reference->calculateSwitchCompensation(dst);
    const Mat4f R = viewRotation().multiplyFast(comp);
    if (skyLocked) // hold the same ABSOLUTE equatorial orientation across the switch
        lockedSkyRot = lockedSkyRot.multiplyFast(comp);
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
    recoverParams(R);
    setFreeMode(oldFreeMode);
    setBoundToSurface(oldBoundToSurface);
    if (dst->isSystem())
        setBoundToSurface(false);
}

void Camera::warpToBody(ModularBody *dst)
{
    dst->useNow(); // becoming the reference is a use - see switchToBody
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
    if (dst->isSystem()) // same no-surface rule as switchToBody
        setBoundToSurface(false);
}

Vec3f Camera::getReferenceRelativePosition() const
{
    const Mat4f m = viewMat();
    const Vec3f t = m.getTranslation();
    return Vec3f(-(m.r[0]*t[0] + m.r[1]*t[1] + m.r[2]*t[2]),
                 -(m.r[4]*t[0] + m.r[5]*t[1] + m.r[6]*t[2]),
                 -(m.r[8]*t[0] + m.r[9]*t[1] + m.r[10]*t[2]));
}

Vec3d Camera::getRootPosition() const
{
    const Vec3f p = reference->accumulatedBodyToBodyPos(reference->getLastJD())
        .transpose().multiplyWithoutTranslation(getReferenceRelativePosition());
    return reference->getCachedRootPosition() + Vec3d(p[0], p[1], p[2]);
}

Vec3f Camera::positionRelativeTo(const ModularBody *body) const
{
    if (body == static_cast<const ModularBody *>(reference))
        return getReferenceRelativePosition();
    const Vec3d rel = getRootPosition() - body->getCachedRootPosition();
    return body->accumulatedBodyToBodyPos(body->getLastJD())
        .multiplyWithoutTranslation(Vec3f(rel[0], rel[1], rel[2]));
}

void Camera::placeAt(const Vec3f &pos, bool holdView)
{
    const Mat4f R = holdView ? viewRotation().multiplyFast(placementRotation()) : Mat4f::identity();
    const Vec3f p = boundToSurface
        ? reference->computeSurfaceToBody().multiplyWithoutTranslation(pos)
        : pos;
    if (freeMode) {
        // viewMat's free branch is mat = R*T(position) => p = -position.
        position = -p;
    } else {
        const Vec3f place = posePartToPose(p, longitude, latitude);
        longitude = place[0];
        latitude = place[1];
        distance = place[2];
        if (holdView)
            foldLat = latitude;
    }
    if (holdView)
        recoverParams(R);
}

void Camera::update(double jd, float deltaTime)
{
    ModularBody::deltaTime = deltaTime * 1000.f;
    reference->refreshFrameState(jd);
    if (latitude != foldLat && mount == CameraMount::EQUATORIAL && !freeMode) {
        const Mat4f refold = Mat4f::xrotation(M_PI_2 - latitude)
            .multiplyFast(Mat4f::xrotation(foldLat - M_PI_2));
        const Vec3f dir = refold.multiplyWithoutTranslation(paramForward());
        if (dir[0] == 0.f && dir[1] == 0.f) {
            alt = -std::copysign(M_PI_2, dir[2]);
        } else {
            az = -atan2f(dir[1], dir[0]);
            alt = -asinf(dir[2]/dir.length());
        }
        if (viewT > 0.f) {
            viewFrom = refold.multiplyWithoutTranslation(viewFrom);
            viewAxis = refold.multiplyWithoutTranslation(viewAxis);
        }
    }
    foldLat = latitude;
    if (target) { // Note : the tracked position is from the last update
        lookTo(observedToLocalPos(target->getObservedPosition()), 5, true);
    }
    advanceView(deltaTime);
    advanceViewOffset(deltaTime);
    if (zoomDuration) {
        zoomTimer += deltaTime;
        if (zoomTimer > zoomDuration) {
            zoomDuration = 0;
            ModularBody::setHalfFov(dstHalfFov); // maintains cullHalfFov (INTENT 11.33)
        } else {
            ModularBody::setHalfFov(srcHalfFov * std::pow(dstHalfFov/srcHalfFov, calculateZoomCoef()));
        }
    }
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
    if (freeMode && !reference->isSystem()) {
        const float ground = reference->getScaledGroundRadius();
        const float len = position.length();
        if (len < ground && len > 0.f)
            position *= ground / len;
    }
    if (skyLocked) {
        if (!freeMode && !target && viewT <= 0.f)
            recoverParams(lockedSkyRot);
        else
            lockedSkyRot = viewRotation().multiplyFast(placementRotation());
    }
    const Mat4f mat = viewMat();
    lastDispatchedMat = mat; // harness: dump what actually ran
    {
        const Mat4f absMat = mat.multiplyFast(reference->accumulatedBodyToBodyPos(jd));
        lastAbsFwd = Vec3f(-absMat.r[2], -absMat.r[6], -absMat.r[10]);
    }
    system = ModularBody::dispatchUpdate(reference, jd, mat);
    system->updateSystem();
    if (freeMode) {
        // The camera's own distance to the reference - never the body's
        // cached member (findBetterReference contract).
        if (auto newRef = reference->findBetterReference(position.length()))
            switchToBody(newRef);
    }
}

void Camera::draw(Renderer &renderer)
{
    frameDrawTask.renderer = &renderer;
    frameDrawTask.camera = this;
    frameDrawTask.done.store(false, std::memory_order_relaxed);
    RenderChain::instance.execute(&frameDrawTask);
    while (!frameDrawTask.done.load(std::memory_order_acquire))
        ;
}

void Camera::FrameDrawTask::start(Taskable *target)
{
    camera->system->drawSystem(*renderer);
    done.store(true, std::memory_order_release);
    target->endTask(this);
}

void Camera::moveTo(const Vec3f &pos, float duration, bool calculateDuration)
{
    if (freeMode) {
        const Vec3f dst = -posePart(pos[0], pos[1],
                                    reference->getAltitudeReference() + pos[2]);
        if (duration > 0) {
            moveRel(dst - position, duration, calculateDuration);
        } else {
            position = dst; // absolute snap: += (dst - position) cancels
        }
    } else if (duration > 0) {
        moveRel(pos - Vec3f(longitude, latitude, distanceToReference()), duration, calculateDuration);
    } else {
        longitude = pos[0];
        latitude = pos[1];
        distance = reference->getAltitudeReference() + pos[2];
    }
}

void Camera::setFreeMode(bool b)
{
    if (b == freeMode)
        return;
    const Mat4f R = viewRotation().multiplyFast(placementRotation());
    if (b) {
        position = -posePart(longitude, latitude, distance);
    } else {
        const Vec3f place = posePartToPose(-position, longitude, latitude);
        longitude = place[0];
        latitude = place[1];
        distance = place[2];
    }
    freeMode = b;
    recoverParams(R);
}

void Camera::setBoundToSurface(bool b)
{
    if (b == boundToSurface)
        return;
    const Mat4f R = viewRotation().multiplyFast(placementRotation()); // deduce-identical-view
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
    recoverParams(R);
}

void Camera::setMount(CameraMount m)
{
    if (m == mount)
        return;
    const Mat4f R = viewRotation().multiplyFast(placementRotation()); // deduce-identical-view
    mount = m;
    recoverParams(R);
}

void Camera::setSkyLock(bool b)
{
    if (b == skyLocked)
        return;
    skyLocked = b;
    if (b)
        lockedSkyRot = viewRotation().multiplyFast(placementRotation());
}

void Camera::lookTo(const Vec3f &direction, float duration, bool isMaxDuration)
{
    Vec3f dirP = fold().multiplyWithoutTranslation(direction);
    const float len = dirP.length();
    if (!(len > 0.f))
        return;
    dirP /= len;
    const Vec3f cur = paramForward();
    float cosA = cur.dot(dirP);
    if (cosA > 1.f) cosA = 1.f;
    if (cosA < -1.f) cosA = -1.f;
    const float angle = acosf(cosA);
    if (duration <= 0.f || angle < 1e-6f) { // snap
        viewT = 0.f;
        if (dirP[0] == 0.f && dirP[1] == 0.f) {
            alt = -std::copysign(M_PI_2, dirP[2]); // pole: az kept (see recoverParams)
        } else {
            az = -atan2f(dirP[1], dirP[0]);
            alt = -asinf(dirP[2]);
        }
        return;
    }
    Vec3f axis = cur^dirP;
    const float axisLen = axis.length();
    if (axisLen < 1e-6f) {
        axis = Vec3f(0,0,1) - cur*cur[2];
        if (axis.length() < 1e-6f)
            axis = Vec3f(1,0,0);
        axis.normalize();
    } else {
        axis /= axisLen;
    }
    // Velocity continuity across retargets (tracking re-plans every frame):
    // carry the along-path speed projected on the new axis.
    float v0 = 0.f;
    if (viewT > 0.f)
        v0 = planVel(viewV0, viewA, viewT1, viewTimer) * viewAxis.dot(axis);
    float T = isMaxDuration ? duration * (angle / M_PI) : duration;
    if (T < 0.2f)
        T = 0.2f; // lower bound: keeps retarget accelerations finite
    float t1, a;
    if (!solvePlan(v0, angle, T, t1, a))
        return;
    viewFrom = cur;
    viewAxis = axis;
    viewAngle = angle;
    viewT1 = t1;
    viewT = T;
    viewTimer = 0.f;
    viewV0 = v0;
    viewA = a;
}

void Camera::lookTo(float _alt, float _az, float duration, bool isMaxDuration)
{
    // Convert the parameter target to a direction and ride the same smoothed
    // great-circle path (paramForward's formula at the target parameters).
    const float ca = cosf(_alt);
    const Vec3f dirP(cosf(_az)*ca, -sinf(_az)*ca, -sinf(_alt));
    // dirP is already in the param frame: undo the fold lookTo will re-apply
    lookTo(fold().transpose().multiplyWithoutTranslation(dirP), duration, isMaxDuration);
}

void Camera::lookRel(float deltaAlt, float deltaAz, float duration, bool isMaxDuration)
{
    if (deltaAlt == 0.f && deltaAz == 0.f)
        return; // old's `if (deltaAz || deltaAlt)` guard (navigator.cpp:201)
    float viewAlt = -alt;
    if (deltaAlt != 0.f) {
        if (viewAlt + deltaAlt <= (float)M_PI_2 && viewAlt + deltaAlt >= -(float)M_PI_2)
            viewAlt += deltaAlt;
        if (viewAlt + deltaAlt > (float)M_PI_2)
            viewAlt = (float)M_PI_2 - 0.000001f;   // Prevent bug
        if (viewAlt + deltaAlt < -(float)M_PI_2)
            viewAlt = -(float)M_PI_2 + 0.000001f;  // Prevent bug
    }
    float newAz = az + deltaAz;
    if (duration > 0.f) {
        lookTo(-viewAlt, newAz, duration, isMaxDuration);
        return;
    }
    viewT = 0.f; // a snap drops any in-flight view plan (lookTo's own rule)
    alt = -viewAlt;
    if (newAz > (float)M_PI)
        newAz -= 2.f * (float)M_PI;
    else if (newAz <= -(float)M_PI)
        newAz += 2.f * (float)M_PI;
    az = newAz;
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
    heading += deltaHeading;
    if (hdgT > 0.f) // keep an in-flight heading plan consistent
        hdgTarget += deltaHeading;
}

void Camera::setHeading(float _heading, float duration)
{
    if (duration <= 0.f) {
        heading = _heading;
        hdgT = 0.f;
        return;
    }
    // 1-D constant-min-acceleration plan, velocity-continuous retarget
    const float v0 = (hdgT > 0.f) ? planVel(hdgV0, hdgA, hdgT1, hdgTimer) : 0.f;
    hdgFrom = heading;
    hdgTarget = _heading;
    hdgV0 = v0;
    hdgTimer = 0.f;
    if (solvePlan(v0, _heading - heading, duration, hdgT1, hdgA))
        hdgT = duration;
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
            if (duration == 0) // Snap, even against a zoom in flight the other way
                ModularBody::setHalfFov(halfFov);
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
        ModularBody::setHalfFov(halfFov); // maintains cullHalfFov (INTENT 11.33)
    }
}

void Camera::setAltitude(double altitude)
{
    distance = altitude/(1000*AU)+reference->getAltitudeReference();
}

void Camera::multAlt(float coef)
{
    if (!freeMode) {
        // Anchored: radial altitude
        moveRel({0, 0, velocityScaling(1) * (coef - 1)});
        return;
    }
    if (reference->isSystem()) {
        if (ModularBody *sel = ModularBody::getSelected()) {
            const Vec3f d = sel->getObservedPosition();
            if (d.lengthSquared() > 0.f) {
                moveEyeRel(d * (coef - 1));
                return;
            }
        }
        // No selection / degenerate: fall through to the view-ray step.
    }
    const Vec3f C = viewMat().getTranslation();
    const float g = reference->getScaledGroundRadius();
    const float len = C.length();
    float alt = len - g;                  // live radial altitude above the ground
    if (alt < 0.f) alt = 0.f;
    if (coef >= 1.f) {
        // Ascend along the view ray, never slower than MIN_MOVEMENT_SPEED so height 0 is escapable
        const float step = (coef - 1.f) * std::max(alt, static_cast<float>(MIN_MOVEMENT_SPEED) * reference->getScaledRadius());
        moveEyeRel({0, 0, -step});        // eye +z = backward along the ray = up
        return;
    }
    // DESCEND toward the surface point under the view ray. Cast origin +
    // s*(0,0,-1), take the near hit, move (1-coef) of the way there.
    const float Cz = C[2];
    const float disc = Cz * Cz - (C.dot(C) - g * g);
    float s = -1.f;
    if (disc >= 0.f)
        s = -Cz - sqrtf(disc);            // nearest ray^sphere ahead of the eye
    if (s > 0.f) {
        moveEyeRel({0, 0, (1.f - coef) * s}); // forward toward S; (1-coef)>0
    } else if (len > 0.f) {
        moveEyeRel(C * ((coef - 1.f) * alt / len)); // observer Delta = (1-coef)*alt*C_hat
    }
}

// ---- Session state (b31-design S2 group B; contract in Camera.hpp) ---------

namespace {

std::string f2s(float v)
{
    std::ostringstream s;
    s << std::setprecision(9) << v;
    return s.str();
}

std::string d2s(double v)
{
    std::ostringstream s;
    s << std::setprecision(17) << v;
    return s.str();
}

std::string b2s(bool v)
{
    return v ? "true" : "false";
}

bool readF(const ModularSystemFormat::Section &s, const char *key, float &out)
{
    if (const std::string *v = s.find(key)) {
        out = Utility::strToFloat(*v, out);
        return true;
    }
    return false;
}

bool readD(const ModularSystemFormat::Section &s, const char *key, double &out)
{
    if (const std::string *v = s.find(key)) {
        out = Utility::strToDouble(*v, out);
        return true;
    }
    return false;
}

bool readB(const ModularSystemFormat::Section &s, const char *key, bool &out)
{
    if (const std::string *v = s.find(key)) {
        out = (*v == "true" || *v == "1");
        return true;
    }
    return false;
}

} // namespace

void Camera::saveSession(ModularSystemFormat::Section &out) const
{
    out.appendEntry("reference", reference ? reference->getEnglishName() : "");
    out.appendEntry("tracked", target ? target->getEnglishName() : "");
    out.appendEntry("free_mode", b2s(freeMode));
    out.appendEntry("bound_to_surface", b2s(boundToSurface));
    out.appendEntry("mount", (mount == CameraMount::EQUATORIAL) ? "equatorial" : "altaz");

    Vec3f pos = position;
    float lon = longitude, lat = latitude, dist = distance;
    if (moveDuration > 0.f) {
        if (freeMode) {
            pos += deltaPosition * moveDuration;
        } else {
            lon += deltaPosition[0] * moveDuration;
            lat += deltaPosition[1] * moveDuration;
            dist += deltaPosition[2] * moveDuration;
        }
    }
    out.appendEntry("longitude", f2s(lon));   // radians, like alt/az below
    out.appendEntry("latitude", f2s(lat));
    out.appendEntry("altitude", d2s(static_cast<double>(dist - reference->getAltitudeReference())
                                    * 1000.0 * AU));
    out.appendEntry("position", f2s(pos[0]) + "," + f2s(pos[1]) + "," + f2s(pos[2]));

    float sAlt = alt, sAz = az;
    if (viewT > 0.f) {
        const Vec3f dir = rotateAbout(viewFrom, viewAxis, viewAngle);
        const float r = dir.length();
        if (dir[0] == 0.f && dir[1] == 0.f) {
            sAlt = -std::copysign(M_PI_2, dir[2]);
        } else {
            sAz = -atan2f(dir[1], dir[0]);
            sAlt = -asinf(dir[2]/r);
        }
    }
    out.appendEntry("alt", f2s(sAlt));
    out.appendEntry("az", f2s(sAz));
    out.annotate("heading", "excluded-pending-D28",
        "`heading` is NOT part of this session. What it MEANS across a change of "
        "reference body is an open product question (DECISIONS_PENDING D28 / "
        "INTENT A38): the new path holds the whole orientation across a switch, "
        "so the heading PARAMETER is rewritten by that switch and a saved number "
        "would bake an answer nobody has given. A restored session therefore "
        "keeps the heading the running app already has. To fix: answer D28, then "
        "add `heading` here.");

    out.appendEntry("fov", d2s(static_cast<double>(zoomDuration ? dstHalfFov : ModularBody::halfFov)
                               * (360.0 / M_PI)));
    out.appendEntry("sky_locked", b2s(skyLocked));
    {
        std::string m;
        for (int i = 0; i < 16; ++i)
            m += (i ? "," : "") + f2s(lockedSkyRot.r[i]);
        out.appendEntry("sky_rot", m);
    }
    // D32's first named carve-out: the ARMING LATCH is a sticky condition and
    // is saved; its ramp is a motion and snaps to the latch's endpoint.
    out.appendEntry("view_offset", d2s(viewOffset));
    out.appendEntry("view_offset_armed", b2s(viewOffsetArmed));
}

void Camera::restoreSession(const ModularSystemFormat::Section &in)
{
    viewT = 0.f;
    hdgT = 0.f;
    zoomDuration = 0.f;
    moveDuration = 0.f;
    deltaPosition = Vec3f(0, 0, 0);

    if (const std::string *v = in.find("tracked")) {
        target = v->empty() ? nullptr : ModularBody::findBodyOnce(*v);
        if (!v->empty() && !target)
            cLog::get()->write("Session restore: the tracked body '" + *v + "' is not in this "
                "tree, so tracking is off. Every other value of the session still applied. "
                "To fix: load the system that declares it before restoring, or drop the key.",
                LOG_TYPE::L_WARNING);
    }
    bool b = freeMode;
    if (readB(in, "free_mode", b))
        setFreeMode(b);
    b = boundToSurface;
    if (readB(in, "bound_to_surface", b))
        setBoundToSurface(b);
    if (const std::string *v = in.find("mount"))
        setMount((*v == "equatorial") ? CameraMount::EQUATORIAL : CameraMount::ALTAZ);

    if (const std::string *v = in.find("position")) {
        Vec3f p = position;
        if (std::sscanf(v->c_str(), "%f,%f,%f", &p.v[0], &p.v[1], &p.v[2]) == 3)
            position = p;
    }
    readF(in, "alt", alt);
    readF(in, "az", az);
    if (const std::string *v = in.find("sky_rot")) {
        Mat4f m;
        const char *p = v->c_str();
        int i = 0;
        for (; i < 16 && *p; ++i) {
            m.r[i] = static_cast<float>(atof(p));
            const char *comma = strchr(p, ',');
            if (!comma) { ++i; break; }
            p = comma + 1;
        }
        if (i == 16)
            lockedSkyRot = m;
    }
    foldLat = latitude;
}

// Dual-path trace harness (INTENT.md 11.14).
void Camera::dumpTrace(std::ostream &out) const
{
    const Vec3d rootPos = reference ? getRootPosition() : Vec3d(0, 0, 0);
    out << std::setprecision(9) << "{\"reference\":\""
        << (reference ? reference->getEnglishName() : "")
        << "\",\"tracked\":\"" << (target ? target->getEnglishName() : "")
        << "\",\"freeMode\":"
        << (freeMode ? "true" : "false") << ",\"boundToSurface\":"
        << (boundToSurface ? "true" : "false")
        << ",\"mount\":\"" << (mount == CameraMount::EQUATORIAL ? "equatorial" : "altaz")
        << "\",\"skyLocked\":" << (skyLocked ? "true" : "false")
        << ",\"viewOffset\":" << jn(viewOffset)
        << ",\"viewOffsetTransition\":" << jn(viewOffsetTransition)
        << ",\"viewOffsetEff\":" << jn(effectiveViewOffset())
        << ",\"longitude\":" << jn(longitude) << ",\"latitude\":" << jn(latitude)
        << ",\"distance\":" << jn(distance)
        << ",\"alt\":" << jn(alt) << ",\"az\":" << jn(az) << ",\"heading\":" << jn(heading)
        // Absolute (root-aligned common-inertial) look direction - the B13
        // reference-change / free-mode continuity observable (INTENT 11.61).
        << ",\"absFwd\":[" << jn(lastAbsFwd[0]) << ',' << jn(lastAbsFwd[1]) << ',' << jn(lastAbsFwd[2]) << ']'
        << ",\"position\":[" << jn(position[0]) << ',' << jn(position[1]) << ',' << jn(position[2])
        << "],\"rootPos\":[" << std::setprecision(17)
        << jn(rootPos[0]) << ',' << jn(rootPos[1]) << ',' << jn(rootPos[2]) << std::setprecision(9)
        << "],\"refAoI\":" << jn(reference ? reference->getAreaOfInfluence() : 0.f)
        << ",\"refDist\":" << jn(reference ? reference->getDistanceToObserver() : 0.f)
        << ",\"refCached\":" << ((reference && reference->isCacheFresh()) ? "true" : "false")
        << ",\"refParent\":\"" << ((reference && reference->getParent()) ? reference->getParent()->getEnglishName() : "") << '"'
        << ",\"selected\":\"" << (ModularBody::getSelected() ? ModularBody::getSelected()->getEnglishName() : "")
        << "\",\"selDist\":" << jn(ModularBody::getSelected() ? ModularBody::getSelected()->getObservedPosition().length() : 0.f)
        << ",\"halfFov\":" << jn(ModularBody::halfFov)
        << ",\"cullHalfFov\":" << jn(ModularBody::cullHalfFov)
        << ",\"lockedSkyRot\":[";
    for (int i = 0; i < 16; ++i)
        out << jn(lockedSkyRot.r[i]) << ((i < 15) ? "," : "");
    out << "],\"plans\":{\"viewT\":" << jn(viewT) << ",\"hdgT\":" << jn(hdgT)
        << ",\"zoomDuration\":" << jn(zoomDuration) << ",\"moveDuration\":" << jn(moveDuration)
        << "},\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << jn(lastDispatchedMat.r[i]) << ((i < 15) ? "," : "");
    out << "]}";
}

float Camera::velocityScaling(float deltaTime) const
{
    const float minSpeed = MIN_MOVEMENT_SPEED * reference->getScaledRadius();
    float velocity = distanceToReference();
    if (velocity < minSpeed)
        velocity = minSpeed;
    return velocity * deltaTime;
}
