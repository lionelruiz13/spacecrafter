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
    // A system node has no surface to bind to (its spin state is not surface
    // semantics) - a system reference drops the bind; it re-arms on the next
    // body reference through the normal setBoundToSurface route.
    if (dst->isSystem())
        setBoundToSurface(false);
}

void Camera::warpToBody(ModularBody *dst)
{
    if (!freeMode) {
        // Anchored warp = old home-planet semantics: same lat/lon/ALTITUDE
        // over the new body (altitude preserved, never the center distance -
        // a stale center distance can sit inside the new body or far above
        // it, and it desynchronizes every consumer until the next moveto).
        distance = dst->getAltitudeReference()
                 + (distance - reference->getAltitudeReference());
    }
    // Keep the same ABSOLUTE sky direction across the reference change (Q2/A11,
    // INTENT 11.61): capture the eye orientation and the inter-frame rotation
    // against the OLD reference, then recover (alt,az,heading) under dst so the
    // composed view reproduces the SAME orientation in the common inertial frame
    // as the reference frame rotates under it. Same `calculateSwitchCompensation`
    // switchToBody uses; the difference is warpToBody ALSO teleports the observer
    // (lat/lon/altitude re-based above) - position and look direction are
    // orthogonal. Done DIRECTLY (recoverParams once), NOT through switchToBody's
    // setFreeMode/setBoundToSurface round-trip: those toggles rewrite longitude
    // by getAxisRotation and, with the reference changing mid-toggle, would
    // subtract dst's axis rotation from a longitude that added the old body's -
    // corrupting the observer's location warpToBody must keep exact.
    const Mat4f comp = reference->calculateSwitchCompensation(dst);
    const Mat4f R = viewRotation().multiplyFast(placementRotation()).multiplyFast(comp);
    if (skyLocked) // hold the same ABSOLUTE equatorial orientation across the switch
        lockedSkyRot = lockedSkyRot.multiplyFast(comp);
    reference->leaveEnvironment();
    reference = dst;
    reference->enterEnvironment();
    recoverParams(R);
    if (dst->isSystem()) // same no-surface rule as switchToBody
        setBoundToSurface(false);
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
    // Sky-lock (old flag_lock_equ_pos): hold the equatorial-frame orientation
    // fixed as the body spins under the anchored observer. Re-derive the params
    // against the CURRENT placement (computeSurfaceToBody has advanced with jd)
    // so viewRotation()*placementRotation() stays == lockedSkyRot: the RA/DE is
    // held, the alt/az drift. Dormant under tracking / in-flight view plans /
    // freeMode (old precedence auto_move > tracking > lock); those frames
    // re-capture, so a resumed hold starts from the present view. Runs AFTER
    // the observer-move block so it also holds the sky when the observer moves.
    if (skyLocked) {
        if (!freeMode && !target && viewT <= 0.f)
            recoverParams(lockedSkyRot);
        else
            lockedSkyRot = viewRotation().multiplyFast(placementRotation());
    }
    // Z body_axis
    // X statique, Y et Z bougent avec alt/az
    // (The 2026 Moon-divergence note that lived here is resolved: the delta was
    //  EMB wiring + per-hop tilts + this longitude sign - INTENT.md 11.14,
    //  harness/predict.py carries the measurements.)
    Mat4f mat{viewRotation()};
    if (freeMode) {
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
    // harness: absolute (root-aligned) look direction (INTENT 11.61, B13). `mat`
    // is eye <- the reference's accumulated-equatorial frame; multiplying by the
    // reference's accumulatedBodyToBodyPos(jd) is EXACTLY the `flat` dispatchUpdate
    // computes (eye <- root), so the eye-forward (-z in eye space) expressed in
    // root coords is frame-independent and comparable across a reference switch.
    {
        const Mat4f absMat = mat.multiplyFast(reference->accumulatedBodyToBodyPos(jd));
        lastAbsFwd = Vec3f(-absMat.r[2], -absMat.r[6], -absMat.r[10]);
    }
    system = ModularBody::dispatchUpdate(reference, jd, mat);
    system->updateSystem();
    // Reference transitions AFTER the dispatch (INTENT 11.36): the decision
    // reads THIS frame's fresh distances. Deciding before the walk read the
    // PREVIOUS frame's - right in steady state, wrong across discontinuities:
    // a warped-to reference still carried its distance-as-a-far-body, and the
    // escalation undid the explicit warp on the next frame (measured, scene C/D
    // regression). The switch itself lands on the next frame's mat;
    // calculateSwitchCompensation (which reads the caches this dispatch just
    // refreshed) makes the transition seamless at the switch instant.
    // Auto-transitions are FREE-FLIGHT-ONLY, both directions (INTENT 11.36
    // scene-E finding): an anchored reference is an EXPLICIT declaration -
    // legacy scripts (immutable, 2(b)) rely on `set home_planet X` +
    // `moveto altitude small` with any prior altitude, and an anchored
    // escalation in the frames between the two commands cascades the
    // reference away and races the moveto (measured: reference landed on
    // SolarSystem at solar-radius distance). The old path never re-references
    // an explicit anchor either - its altitude-driven executor dispatch is a
    // DISPLAY regime, not a reference change (that display question at
    // anchored galactic altitudes is suspended for Vixy).
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

void Camera::moveTo(const Vec3f &pos, float duration, bool calculateDuration)
{
    if (freeMode) {
        // Legacy spherical target converted to a free position - the
        // setFreeMode(true) convention (spheToRect(-lon, lat) * center
        // distance); altitude counts from the reference's altitude reference,
        // matching the anchored branch's distanceToReference() semantics.
        Vec3f dst;
        Utility::spheToRect(-pos[0], pos[1], dst);
        dst *= reference->getAltitudeReference() + pos[2];
        if (duration > 0) {
            moveRel(dst - position, duration, calculateDuration);
        } else {
            position = dst; // absolute snap: += (dst - position) cancels
        }
    } else if (duration > 0) {
        moveRel(pos - Vec3f(longitude, latitude, distanceToReference()), duration, calculateDuration);
    } else {
        // Absolute snap by ASSIGNMENT: moveTo is absolute by meaning, and the
        // delta form dies on float cancellation across scales - a 552 AU ->
        // 2.3e-5 AU move has its target below the ulp of the start value
        // (measured: distance landed on 0.0 exactly, INTENT 11.36 scene E).
        longitude = pos[0];
        latitude = pos[1];
        distance = reference->getAltitudeReference() + pos[2];
    }
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

void Camera::setSkyLock(bool b)
{
    if (b == skyLocked)
        return;
    skyLocked = b;
    // Freeze the CURRENT equatorial-frame orientation (body->eye) to hold. The
    // held value is captured against the current placement, so update()'s per-
    // frame recoverParams against the sidereal-advanced placement keeps the
    // composed rotation on it. Off just releases: update() re-captures each
    // non-holding frame so a later re-lock starts from the present view.
    if (b)
        lockedSkyRot = viewRotation().multiplyFast(placementRotation());
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
        << (reference ? reference->getEnglishName() : "")
        // Tracked body by name ("" = not tracking): the only camera-side body
        // reference that was invisible to the harness, and the one a system
        // reload must re-seat (INTENT 11.55).
        << "\",\"tracked\":\"" << (target ? target->getEnglishName() : "")
        << "\",\"freeMode\":"
        << (freeMode ? "true" : "false") << ",\"boundToSurface\":"
        << (boundToSurface ? "true" : "false")
        << ",\"mount\":\"" << (mount == CameraMount::EQUATORIAL ? "equatorial" : "altaz")
        << "\",\"skyLocked\":" << (skyLocked ? "true" : "false")
        << ",\"longitude\":" << longitude << ",\"latitude\":" << latitude
        << ",\"distance\":" << distance
        << ",\"alt\":" << alt << ",\"az\":" << az << ",\"heading\":" << heading
        // Absolute (root-aligned common-inertial) look direction - the B13
        // reference-change / free-mode continuity observable (INTENT 11.61).
        << ",\"absFwd\":[" << lastAbsFwd[0] << ',' << lastAbsFwd[1] << ',' << lastAbsFwd[2] << ']'
        << ",\"position\":[" << position[0] << ',' << position[1] << ',' << position[2]
        << "],\"refAoI\":" << (reference ? reference->getAreaOfInfluence() : 0)
        << ",\"refDist\":" << (reference ? reference->getDistanceToObserver() : 0)
        << ",\"refCached\":" << ((reference && reference->isCacheFresh()) ? "true" : "false")
        << ",\"refParent\":\"" << ((reference && reference->getParent()) ? reference->getParent()->getEnglishName() : "") << '"'
        << ",\"halfFov\":" << ModularBody::halfFov
        << ",\"cullHalfFov\":" << ModularBody::cullHalfFov << ",\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << lastDispatchedMat.r[i] << ((i < 15) ? "," : "");
    out << "]}";
}
