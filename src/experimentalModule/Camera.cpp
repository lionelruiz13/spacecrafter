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

// ---- View composition authority (see Camera.hpp) ---------------------------

Mat4f Camera::fold() const
{
    // EQUATORIAL anchored: map the zenith frame so that z -> polar axis (the
    // pole sits at (0, cos lat, sin lat) in the zenith frame). In freeMode the
    // acting frame is the body frame - already polar-aligned - so the fold is
    // identity for both mounts (free-mode parameters are DE/RA-like).
    // foldLat, not latitude: old-mount semantics keep the view LOCAL-frame
    // locked under observer moves (sky-locking is old flag_lock_equ_pos, a
    // separate feature); update() re-derives the params when latitude moves
    // (measured before the interception: a 5.55° moveto latitude change
    // pitched the whole sky by exactly that angle vs old).
    if (mount == CameraMount::EQUATORIAL && !freeMode)
        return Mat4f::xrotation(M_PI_2 - foldLat);
    return Mat4f::identity();
}

Mat4f Camera::viewRotation() const
{
    // heading+π: without it the camera frame is rolled 180° about the view
    // axis relative to the old path (measured 179.994° on every body,
    // INTENT 11.19b). This composition is the SINGLE AUTHORITY on the
    // alt/az/heading convention; observedToLocalPos is its exact transpose
    // and lookTo/recoverParams are its exact inverses.
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

Vec3f Camera::paramForward() const
{
    // Inverse of lookTo's parameter derivation (az=−lng, alt=−lat):
    // d = (cos az·cos alt, −sin az·cos alt, −sin alt)
    const float ca = cosf(alt);
    return Vec3f(cosf(az)*ca, -sinf(az)*ca, -sinf(alt));
}

// Deduce the (heading, alt, az) configuration providing the visually
// identical view [vixy: 2026-07-12] - ZXZ Euler extraction, validated
// standalone against 50k random rotations (worst 2.8e-7, pole cases
// included with the keep-az policy).
void Camera::recoverParams(const Mat4f &totalRot)
{
    const Mat4f core = totalRot
        .multiplyFast(placementRotation().transpose())
        .multiplyFast(fold().transpose());
    // core = Z(g1)·X(g2)·Z(g3), column-major element (i,j) = r[j*4+i]
    float c2 = core.r[10];
    if (c2 > 1.f) c2 = 1.f;
    if (c2 < -1.f) c2 = -1.f;
    const float g2 = acosf(c2);
    float g1, g3;
    if (fabsf(c2) > 0.999999f) {
        // Pole singularity: only g1±g3 is defined - keep the current az
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

// ---- Constant-minimal-acceleration smoothing --------------------------------
// Law [vixy: 2026-07-12: smallest motion sickness in a dome]: two quadratic
// phases with equal |a| (accel until t1, decel until T), zero end velocity,
// velocity-continuous retargets. Closed form validated standalone (20k random
// (v0,d,T): exact landing, zero end velocity, minimal-|a| root).
static bool solvePlan(float v0, float d, float T, float &t1, float &a)
{
    if (T <= 0.f)
        return false;
    if (fabsf(v0) < 1e-9f) {
        t1 = T * 0.5f;
        a = 4.f*d/(T*T);
        return true;
    }
    // t1 = (2d ∓ √(2·(T²v0² − 2Tdv0 + 2d²)))/(2v0); a = v0/(T − 2t1)
    const float disc = T*T*v0*v0 - 2.f*T*d*v0 + 2.f*d*d; // = (Tv0−d)² + d² ≥ 0
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
    bool oldFreeMode = freeMode;
    bool oldBoundToSurface = boundToSurface;
    setBoundToSurface(false);
    setFreeMode(true); // placement is now identity: total rotation == viewRotation()
    // The compensation maps dst-local coordinates to current-reference-local
    // coordinates - the same view over the new chain (visual continuity).
    const Mat4f R = viewRotation().multiplyFast(reference->calculateSwitchCompensation(dst));
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
    recoverParams(R);
    setFreeMode(oldFreeMode);
    setBoundToSurface(oldBoundToSurface);
}

void Camera::warpToBody(ModularBody *dst)
{
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
}

void Camera::update(double jd, float deltaTime)
{
    // Frame clock for per-frame animations (module faders, pointer breathing):
    // MILLISECONDS, the old-path LinearFader convention. Written here - the
    // single per-frame entry point of the new path - before any body update
    // runs (ModularBody::deltaTime contract).
    ModularBody::deltaTime = deltaTime * 1000.f;
    // Latitude interception (see fold()): keep the zenith-frame view
    // direction across observer latitude moves - old-mount parity. The
    // in-flight view plan lives in the param frame; re-express its endpoints.
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
    // Z body_axis
    // X statique, Y et Z bougent avec alt/az
    // (The 2026 Moon-divergence note that lived here is resolved: the delta was
    //  EMB wiring + per-hop tilts + this longitude sign - INTENT.md 11.14,
    //  harness/predict.py carries the measurements.)
    Mat4f mat{viewRotation()};
    if (freeMode) {
        if (auto newRef = reference->findBetterReference()) {
            switchToBody(newRef);
        }
        mat.multiplyTranslation(position);
    } else {
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
    // Deduce-identical-view transition [vixy: 2026-07-12] - replaces the
    // half-disabled Rotator machinery (removed): capture the total view
    // rotation in the OLD decomposition, convert the position state, then
    // recover the parameters under the NEW decomposition.
    const Mat4f R = viewRotation().multiplyFast(placementRotation());
    if (b) {
        Utility::spheToRect(-longitude, latitude, position);
        position *= distance;
    } else {
        Utility::rectToSphe(&longitude, &latitude, position);
        longitude = -longitude;
        distance = position.length();
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

void Camera::lookTo(const Vec3f &direction, float duration, bool isMaxDuration)
{
    // Parameter derivation is the exact inverse of viewRotation() (authority):
    // centering `direction` (acting frame) requires az=−lng, alt=−lat of the
    // FOLDED direction (param frame). Smoothed per the dome-comfort law.
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
        // Antipodal: any axis orthogonal to cur - prefer the one keeping the
        // move in the azimuthal plane (rotate about the param-frame pole
        // projected out of cur)
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
    lookTo(alt + deltaAlt, az + deltaAz, duration, isMaxDuration);
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

// Dual-path trace harness (INTENT.md 11.14).
void Camera::dumpTrace(std::ostream &out) const
{
    out << std::setprecision(9) << "{\"reference\":\""
        << (reference ? reference->getEnglishName() : "") << "\",\"freeMode\":"
        << (freeMode ? "true" : "false") << ",\"boundToSurface\":"
        << (boundToSurface ? "true" : "false")
        << ",\"mount\":\"" << (mount == CameraMount::EQUATORIAL ? "equatorial" : "altaz")
        << "\",\"longitude\":" << longitude << ",\"latitude\":" << latitude
        << ",\"distance\":" << distance
        << ",\"alt\":" << alt << ",\"az\":" << az << ",\"heading\":" << heading
        << ",\"position\":[" << position[0] << ',' << position[1] << ',' << position[2]
        << "],\"halfFov\":" << ModularBody::halfFov << ",\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << lastDispatchedMat.r[i] << ((i < 15) ? "," : "");
    out << "]}";
}
