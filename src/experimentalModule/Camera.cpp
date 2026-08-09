#include "Camera.hpp"
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

// View-offset arming ramp rate (fraction of the 0..1 transition per second).
// Old ramped view_offset_transition over the arming auto_move (navigator.cpp
// :73-78, ~1-4 s of atan easing); the new move law differs, so only the
// ENDPOINTS (0 inert / 1 armed) are byte-matched to old — the ramp curve is
// perceptual-parity. Rate chosen so the offset arms smoothly within a fraction
// of a second (B17, §11.79(c)).
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
    // The free-mode place lives in `position`; the spherical members were
    // frozen when free flight was entered. Converted back exactly as
    // setFreeMode(false) does it, and `distance` is deliberately NOT used: it
    // keeps a stale value in free mode on purpose (lateral-velocity parity,
    // B10 §11.71), so distanceToReference() would report where the observer
    // was when it took off.
    float lon, lat;
    Utility::rectToSphe(&lon, &lat, position);
    return Vec3f(-lon, lat, position.length() - reference->getAltitudeReference());
}

// Shared proximity-factor authority (B10 iv-b, §5.2). Base measured to the
// ground (ground_radius), NOT the datum: == distanceToReference() for every
// default (ground==datum) body, so bit-identical to today until the two radii
// differ. Uses the SAME `distance` member as distanceToReference (legacy
// free-mode velocity parity); the geometric ground barrier is enforced
// separately by update()'s free-mode descent clamp on `position`. The floor is
// radius-relative (stays defined at an enterable body's centre) and applies to
// the outward/escape step only (§5.18).
float Camera::proximityFactor(bool escaping) const
{
    float p = distance - reference->getScaledGroundRadius();
    if (escaping) {
        const float floor = static_cast<float>(ANTISTUCK_ESCAPE_FLOOR) * reference->getScaledRadius();
        if (p < floor)
            p = floor;
    }
    return p;
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

// The B17 view offset expressed as a fixed EYE-SPACE (screen-frame) rotation R'
// such that mat_render == R' · mat_free (mat_free = the offset-free view). A pure
// pitch about the eye x-axis of `offset·halfFov` — old's fov-coupled magnitude
// (navigator.cpp:309), applied in the PHYSICAL eye frame (leftmost, downstream
// of everything incl. the heading roll). Identity when inert (effective 0) so
// the no-offset path is byte-identical to pre-B17.
//
// Why the physical eye frame, NOT old's pre-heading chain position: old pitches
// BELOW its heading zrotation (navigator.cpp:309 before :314), coupling the
// offset to the heading. In the new path the heading is a DECOMPOSITION param
// that B13 (recoverParams) rewrites across a reference switch to hold the SAME
// physical view (measured: Earth→Mars param-heading moves 2.655° while the
// physical eye orientation is held). Coupling the offset to that param-heading
// makes the offset JUMP across a held ref switch (measured 2.06° absDelta,
// >0.05° B13 tol) — corrupting the B13/B18 composition the row mandates. Applied
// in the physical eye frame the offset is a constant screen pitch: it composes
// invariantly (a held view stays held, shifted) AND reproduces old's OBSERVABLE
// across ref switches (old holds heading in warpToBody, so old's offset is held
// too). Matches old EXACTLY at heading 0 (the dome-show norm, all B17 A/B legs).
// heading≠0 STATIC divergence from old (old rolls the offset with the view; this
// keeps it screen-fixed) is recorded for Vixy — a dome-space offset is arguably
// the more correct reading, and old's pre-roll coupling can't hold the B13
// mandate. Also the rotation the tracking feedback must undo (update()):
// getObservedPosition() == R'·(offset-free eye pos), R'ᵀ recovers the true pos.
Mat4f Camera::viewOffsetEyeRotation() const
{
    const float o = effectiveViewOffset();
    if (o == 0.f)
        return Mat4f::identity();
    return Mat4f::xrotation(o * ModularBody::halfFov);
}

// Render view rotation = the eye-space offset pitch composed (leftmost) with
// viewRotation() (the SOLE composition authority — I2). The fisheye transfer
// turns the offset·halfFov pitch into a constant fraction-of-dome shift
// (§11.63(c), measured fov-independent). Byte-identical to viewRotation() when
// the offset is inert (R' == identity).
Mat4f Camera::renderViewRotation() const
{
    return viewOffsetEyeRotation().multiplyFast(viewRotation());
}

// THE view matrix this camera's CURRENT parameters describe: eye <- the
// reference's accumulated-equatorial frame. update() hands exactly this to
// dispatchUpdate, so `reference->getObservedPosition()` is a COPY of its
// translation, republished once per frame.
//
// It is a function and not only a stored result because the copy is a
// per-frame SNAPSHOT and the parameters are not: anything the camera does
// between two updates (a descent, a moveRel, a look) changes the geometry
// while the snapshot still describes where the eye was at the last dispatch.
// Reading the snapshot back was §11.108(e)'s measured breach — ten
// `camera action descend` in ONE frame all saw the same reference position
// and compounded LINEARLY (x0.9) instead of geometrically (0.99^10), a D8
// use-site-barrier violation reachable from any script or the TCP channel.
// Consumers that need the reference in the eye frame ask HERE (I2: the
// composition is written once, in this function, and every reader of it is
// exact by construction rather than by cadence).
Mat4f Camera::viewMat() const
{
    // Z body_axis
    // X statique, Y et Z bougent avec alt/az
    // (The 2026 Moon-divergence note that lived here is resolved: the delta was
    //  EMB wiring + per-hop tilts + this longitude sign - INTENT.md 11.14,
    //  harness/predict.py carries the measurements.)
    // renderViewRotation() == viewRotation() unless the B17 view offset is
    // active (armed + non-zero); the offset is a render-only pitch (see
    // Camera.hpp) applied downstream of tracking / sky-lock, exactly as old
    // applied it in the navigator stage below those.
    Mat4f mat{renderViewRotation()};
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
    return mat;
}

void Camera::setViewOffset(double offset)
{
    // The [-0.5,0.5] clamp lives in the ONE sink Core::setViewOffset (both §2(c)
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

// Advance the arming transition toward its latched target (old
// view_offset_transition, navigator.cpp:73-78). Ramp only — never a snap — so
// the offset eases in/out; steady state reaches the target EXACTLY (clamped),
// giving byte-exact endpoints against old.
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
    // BECOMING THE REFERENCE IS A USE (D8 §11.76 / B39 §11.117): a hidden body
    // is a legal reference - `S10.sts` makes one its home_planet - and since B39
    // it no longer ticks, so its cached position is at its hide date until asked.
    // calculateSwitchCompensation below reads exactly that cached position, of
    // dst AND of every hop between dst and the common parent (useNow walks up),
    // so the barrier has to fire BEFORE the compensation, not after.
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
    // A system node has no surface to bind to (its spin state is not surface
    // semantics) - a system reference drops the bind; it re-arms on the next
    // body reference through the normal setBoundToSurface route.
    if (dst->isSystem())
        setBoundToSurface(false);
}

void Camera::warpToBody(ModularBody *dst)
{
    dst->useNow(); // becoming the reference is a use - see switchToBody
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

// ---- Where the observer IS, as a vector (B4(iv), §11.141) ------------------
// viewMat() is the affine map the renderer consumes, and its input frame is the
// reference's ACCUMULATED EQUATORIAL frame BY CONTRACT (see viewMat's own
// declaration and dispatchUpdate's parameter) - the surface fold, when the
// camera is bound, is a factor INSIDE that map, not a frame outside it. The eye
// is the origin of the eye frame, so the eye's position p in that input frame
// is the solution of Rot·p + t = 0, i.e. p = −Rotᵀ·t.
//
// Derived from the drawn matrix rather than from the pose members on purpose:
// it is then right in freeMode and anchored, bound and unbound, and with or
// without the view offset (a left-multiplied rotation cancels in −Rotᵀ·t), and
// it cannot fall out of step with what is on screen (I2). MEASURED against the
// old path's own observer position on the same frame (§11.141): the two agree
// to 2.6e-08 AU, which is the float32 floor of a 1 AU subtraction, and the
// three rejected candidates (undoing the fold, transposing the tilt, flipping
// the sign) miss by 1.1e-05 / 3.2e-05 / 8.5e-05 AU - so the composition below
// is discriminated, not assumed.
Vec3f Camera::getReferenceRelativePosition() const
{
    const Mat4f m = viewMat();
    const Vec3f t = m.getTranslation();
    return Vec3f(-(m.r[0]*t[0] + m.r[1]*t[1] + m.r[2]*t[2]),
                 -(m.r[4]*t[0] + m.r[5]*t[1] + m.r[6]*t[2]),
                 -(m.r[8]*t[0] + m.r[9]*t[1] + m.r[10]*t[2]));
}

// `accumulatedBodyToBodyPos` maps ROOT-ALIGNED axes to the body's equatorial
// axes, and its transpose maps back - the direction is the MEASURED one (the
// candidate table above), and it is the pairing that reproduces the old path's
// observer position. Positions are accumulated in DOUBLE: the terms are ~1 AU
// and the answer is often ~1e-5 AU, so float32 would lose 6e-08 AU (9 km) to
// cancellation on every transition.
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
    // Hold the composed orientation across the placement when asked (A38/B13:
    // a reference switch holds the whole orientation). Captured BEFORE the
    // pose members move, recovered after — the deduce-identical-view primitive
    // every other transition in this class uses.
    const Mat4f R = holdView ? viewRotation().multiplyFast(placementRotation()) : Mat4f::identity();
    // Algebraic inverse of the SAME composition getReferenceRelativePosition
    // reads. viewMat is [...]·X(lat−π/2)·Z(−lon)·S (or [...]·T(position)·S),
    // with S = computeSurfaceToBody() when bound, so its input-frame eye
    // position is Sᵀ·(the pose part) and S·pos is the pose part back.
    const Vec3f p = boundToSurface
        ? reference->computeSurfaceToBody().multiplyWithoutTranslation(pos)
        : pos;
    if (freeMode) {
        // viewMat's free branch is mat = R·T(position) ⇒ p = −position.
        position = -p;
    } else {
        // viewMat's anchored branch is mat = R·T(0,0,−distance)·X(lat−π/2)·Z(−lon),
        // so p = Z(lon)·X(π/2−lat)·(0,0,distance) = distance·(cosφ·sinλ,
        // −cosφ·cosλ, sinφ). Inverted below; the −π/2 the pair (sinλ, −cosλ)
        // carries is this class's own longitude origin, not a correction.
        const float d = p.length();
        distance = d;
        if (d > 0.f) {
            latitude = std::asin(p[2]/d);
            longitude = std::atan2(p[0], -p[1]);
        }
        // d == 0: the eye is AT the reference's centre, where longitude and
        // latitude parametrize nothing — they are left alone rather than
        // replaced by atan2(0,0), so a point anchor keeps the place readout it
        // arrived with (the old path leaves lon/lat alone there too).
    }
    if (holdView)
        recoverParams(R);
}

void Camera::update(double jd, float deltaTime)
{
    // Frame clock for per-frame animations (module faders, pointer breathing):
    // MILLISECONDS, the old-path LinearFader convention. Written here - the
    // single per-frame entry point of the new path - before any body update
    // runs (ModularBody::deltaTime contract).
    ModularBody::deltaTime = deltaTime * 1000.f;
    // §5.32: the reference is the ONE node dispatchUpdate can skip update()
    // for, and every consumer below reads its spin and its reach with no drawn
    // gate — the bound placement (placementRotation / the tail of this
    // function), the persistent longitude conversions of setBoundToSurface /
    // setFreeMode, and findBetterReference's threshold. Bring both to THIS
    // frame's date here, before the first of them: without it the camera
    // composes on the last frame the reference happened to be drawn on, which
    // is one frame ago at best and unbounded when the observer is not looking
    // at its own reference.
    reference->refreshFrameState(jd);
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
        // getObservedPosition() rides the offset-included render mat (R'·P_free);
        // centre the OFFSET-FREE position so the offset does not get absorbed by
        // the tracking (old centres the body's true equatorial position, then
        // the offset pitches the view — the tracked body ends up off-centre by
        // the offset, the zoom_offset purpose). R'ᵀ recovers P_free; identity
        // when the offset is inert (bit-identical to the pre-B17 tracking).
        Vec3f p = viewOffsetEyeRotation().transpose()
                      .multiplyWithoutTranslation(target->getObservedPosition());
        lookTo(observedToLocalPos(p), 5, true);
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
    // Free-mode descent clamp (B10 iii, R4 STOP-AND-HOLD): the observer cannot
    // descend past ground_radius. Enforced on `position` (the actual free-mode
    // geometry) every frame, so integrated glides AND instant interactive
    // descents settle-and-HOLD at the ground rather than crossing it or
    // asymptoting toward it. A SYSTEM reference has no landable surface (the
    // EnvironmentManager onBody rule, I4) so it carries no ground clamp —
    // flying INTO a galaxy / solar system stays free. ground_radius == 0
    // (enterable body) ⇒ no clamp, descent to the centre allowed. Anchored
    // mode has no clamp by design (moveto altitude -X is an explicit
    // declaration). This is the ONLY descent floor in Camera — there was none
    // before (§5.2 finding ii), so it is inert for every default body until
    // the observer would cross its ground in free flight.
    if (freeMode && !reference->isSystem()) {
        const float ground = reference->getScaledGroundRadius();
        const float len = position.length();
        if (len < ground && len > 0.f)
            position *= ground / len;
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
    const Mat4f mat = viewMat();
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

// THE EXACT COUNTERPART OF `Navigator::updateMove(deltaAz, deltaAlt, fov)`
// (navigator.cpp:181-221), and its parameters are OLD'S, not this class's:
// +deltaAlt raises the VIEW (old's `altVision += deltaAlt`) and deltaAz turns it
// the way old's `azVision -= deltaAz` does. The camera's own parameters are the
// NEGATIVES of that pair — `az = −lng`, `alt = −lat` of the forward direction in
// the param frame (paramForward above; `viewRotation()·paramForward() == (0,0,−1)`
// is the derivation) — so old's `+deltaAlt` is `−deltaAlt` here and old's
// `deltaAz` is `+deltaAz` here. MEASURED before this was written, on five bodies
// spread over 140° of azimuth: `alt_cam + altVision_old` is 0 to 1.1e-5 rad and
// `az_cam + azVision_old` is constant at +25.264°, while both opposite mappings
// spread by the full range (INTENT §11.133(b)(c)).
// Stating the convention HERE, once, is what keeps the two call sites from
// drifting apart (I2): `Core::updateMove` (the key ramp) and `Core::dragView`
// both hand the SAME numbers to `navigation->updateMove` and to this.
void Camera::lookRel(float deltaAlt, float deltaAz, float duration, bool isMaxDuration)
{
    if (deltaAlt == 0.f && deltaAz == 0.f)
        return; // old's `if (deltaAz || deltaAlt)` guard (navigator.cpp:201)
    // The view altitude in old's convention, with old's pole clamp reproduced
    // line for line INCLUDING its look-ahead: the second and third tests read
    // the ALREADY-MUTATED value, so the clamp fires one step early (whenever
    // altVision + 2·deltaAlt would cross a pole) and pins at ±(π/2 − 1e-6).
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
    // SNAP: assign the parameters. `lookTo(alt, az, 0)` would route them through
    // a direction and back, which is the identity in exact arithmetic (D8 as-if)
    // but NOT in float32 near a pole — MEASURED: a clamped view altitude of
    // π/2 − 1e-6 collapses onto EXACTLY ±π/2 (its sine rounds to 1), and once
    // there the recovered azimuth FLIPS BY π every frame, i.e. a 180° image roll
    // per frame, because viewRotation() composes `az` into the roll. The float32
    // threshold is ε > 2^-11.5 = 3.45e-4 rad, 345× old's clamp epsilon.
    // Assigning keeps old's epsilon and keeps cos(alt) > 0, so the view stays on
    // its own side of the pole.
    viewT = 0.f; // a snap drops any in-flight view plan (lookTo's own rule)
    alt = -viewAlt;
    // Keep the azimuth bounded, as old's rectToSphe round trip does: a float32
    // parameter of magnitude 1000 rad has a 6e-5 rad quantum, which would
    // quantize the view itself over a long uninterrupted turn. One branch, not
    // a per-step trigonometric round trip: within (−π, π] the step is left
    // untouched, so the mirror pays no rounding old does not pay.
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

// See the header: the interactive zoom ramp's sink, the immediate half of
// setHalfFov with no re-planning.
void Camera::setHalfFovNow(float halfFov)
{
    if (halfFov < minHalfFov) {
        halfFov = minHalfFov;
    } else if (halfFov > maxHalfFov) {
        halfFov = maxHalfFov;
    }
    ModularBody::setHalfFov(halfFov); // maintains cullHalfFov (INTENT 11.33)
}

void Camera::setAltitude(double altitude)
{
    distance = altitude/(1000*AU)+reference->getAltitudeReference();
}

// View-directed free descent authority (B21, §11.72). The vertical the observer
// moves along in free flight depends on the regime, riding B10's
// proximityFactor() (I2):
//   * NEAR a body (reference is NOT a system): the VIEW RAY — descend toward
//     the surface point under the screen centre (eye forward, -z). Q6/A18
//     (§11.48). update()'s free-mode clamp holds the floor at ground_radius
//     (R4 stop-and-hold) — this composes with it, it does not fight it.
//   * FAR / galactic (reference IS a system, no landable surface underfoot):
//     aim at the LAST SELECTED body (R6 §11.70(e)). The existing distance-
//     driven reference transition (findBetterReference, update()) then captures
//     the body and the near-field view-ray descent takes over. B21 changes only
//     the descent DIRECTION here, never WHEN a transition fires (dispatch §2
//     carve-out — the escalation/anchor policy is B20/§6.9 territory).
// `coef` is the altitude multiplier (multAlt semantics): coef<1 descends,
// coef>1 ascends. The along-axis step is multiplicative on the vertical's
// length (ground proximity near, distance-to-selected far), so it mirrors the
// legacy natural altitude control on both ends.
void Camera::descend(float coef)
{
    if (!freeMode) {
        // Anchored: legacy proximity-scaled radial altitude (unchanged).
        moveRel({0, 0, proximityFactor(coef > 1.f) * (coef - 1)});
        return;
    }
    // FAR / galactic: the reference is a system (no landable surface
    // underfoot) ⇒ aim at the LAST SELECTED body (R6 §11.70(e)). d is sel's
    // centre in the eye frame; moveEyeRel maps it so the observer steps by
    // (1-coef)·|d| TOWARD sel for coef<1 (descend) and away for coef>1
    // (ascend) — multiplicative on the distance to sel, mirroring the near
    // case's multiplicative-on-altitude feel. The existing distance-driven
    // reference transition (findBetterReference, update()) then captures the
    // body and the near-field view-ray descent takes over — B21 changes only
    // the descent DIRECTION, never WHEN a transition fires (dispatch §2).
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
    // NEAR field: descend toward the surface point under the view ray. Work in
    // the EYE frame — observer at the origin, forward = −z, the ground sphere
    // centred at the reference centre C (= reference->getObservedPosition(),
    // |C| == the live observer distance) with radius g. This is LIVE geometry:
    // it must NOT read the free-mode-stale `distance` member (B10 §11.71 —
    // proximityFactor/distanceToReference keep that stale value for lateral-
    // velocity parity, but a descent that has to reach the ground needs the
    // live position). Cast origin + s·(0,0,−1), take the near hit, and move a
    // (1−coef) fraction of the way to it; update()'s R4 clamp holds the floor.
    // LIVE geometry, from this camera's own parameters (viewMat) and not from
    // the reference's per-frame cache: N descents inside one frame must
    // compound like N descents spread over N frames (§5.32/§11.108(e)).
    const Vec3f C = viewMat().getTranslation();
    const float g = reference->getScaledGroundRadius();
    const float len = C.length();
    float alt = len - g;                  // live radial altitude above the ground
    if (alt < 0.f) alt = 0.f;
    if (coef >= 1.f) {
        // ASCEND: back off along the view ray (reverse of the descent). The step
        // is proportional to the LIVE altitude and FLOORED to
        // ANTISTUCK_ESCAPE_FLOOR·radius so takeoff from height 0 is ALWAYS
        // possible — the §5.18 anti-stuck rule B10 enforces on every OUTWARD
        // step (proximityFactor's escape floor), restored here on live geometry.
        float step = (coef - 1.f) * alt;
        const float floor = static_cast<float>(ANTISTUCK_ESCAPE_FLOOR)
                          * reference->getScaledRadius();
        if (step < floor)
            step = floor;
        moveEyeRel({0, 0, -step});        // eye +z = backward along the ray = up
        return;
    }
    // DESCEND toward the surface point under the view ray. Cast origin +
    // s·(0,0,−1), take the near hit, move (1−coef) of the way there.
    const float Cz = C[2];
    const float disc = Cz * Cz - (C.dot(C) - g * g);
    float s = -1.f;
    if (disc >= 0.f)
        s = -Cz - sqrtf(disc);            // nearest ray∩sphere ahead of the eye
    if (s > 0.f) {
        moveEyeRel({0, 0, (1.f - coef) * s}); // forward toward S; (1−coef)>0
    } else if (len > 0.f) {
        // The view ray misses the ground OR the sphere sits entirely behind the
        // eye (looking away): descend RADIALLY toward the reference centre so
        // "down" still lowers the observer.
        moveEyeRel(C * ((coef - 1.f) * alt / len)); // observer Δ = (1−coef)·alt·Ĉ
    }
}

// ---- Session state (b31-design §2 group B; contract in Camera.hpp) ---------

namespace {

// Round-trip-exact text for the two float widths this camera stores. A session
// that comes back to a different number than it saved is not a session, and
// the default ostream precision is what turned a JD into a 10-day quantum in
// the surface this file replaces (§5.41).
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

// Reading is by ASSIGNMENT or not at all: a key the file does not carry leaves
// the live value alone, which is what makes a hand-trimmed file apply the part
// it does carry instead of half-destroying the state (§4.2's report-and-keep,
// on the value side).
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

    // D32: a move in flight is saved WHERE IT WAS GOING. update() integrates
    // deltaPosition for the remaining moveDuration, so the settled state is
    // reachable in closed form and needs no assumption about frame timing.
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
    // ALTITUDE, in metres above the datum - not the `distance` member it is
    // derived from. Two reasons, and neither is cosmetic: it is the unit the
    // operator's own `moveto` takes, so a hand-edited session says what it
    // looks like it says; and the restore hands it straight to the seam that
    // moves BOTH paths, which is what keeps the old-path sky under the new
    // path's observer (§2 rows B3/B19).
    out.appendEntry("altitude", d2s(static_cast<double>(dist - reference->getAltitudeReference())
                                    * 1000.0 * AU));
    out.appendEntry("position", f2s(pos[0]) + "," + f2s(pos[1]) + "," + f2s(pos[2]));

    // D32 again, on the view: an in-flight smoothing plan lands on
    // rotateAbout(viewFrom, viewAxis, viewAngle) - advanceView's own exact
    // landing, taken here instead of a sampled mid-path direction. Tracking
    // re-plans EVERY frame (§11.55(d)), so a mid-plan save has no fixed point
    // and T4 could not exist.
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
    // §2 row B6. Written as a note rather than as a value, and attached to the
    // ABSENT key so that it lands at the section's end rather than above an
    // unrelated datum: the exclusion has to survive into the artifact, or the
    // slice that adds it has to rediscover why it is missing.
    out.annotate("heading", "excluded-pending-D28",
        "`heading` is NOT part of this session. What it MEANS across a change of "
        "reference body is an open product question (DECISIONS_PENDING D28 / "
        "INTENT A38): the new path holds the whole orientation across a switch, "
        "so the heading PARAMETER is rewritten by that switch and a saved number "
        "would bake an answer nobody has given. A restored session therefore "
        "keeps the heading the running app already has. To fix: answer D28, then "
        "add `heading` here.");

    // FOV IN DEGREES, which is what `zoom fov` takes and what the dual seam
    // that restores it takes - one representation, converted once. Writing the
    // camera's own half-angle instead cost a float->double->float round trip
    // through that seam and the value came back one ulp off, which is enough to
    // lose T4's byte-identity (measured: 3425 vs 3424 bytes).
    out.appendEntry("fov", d2s(static_cast<double>(zoomDuration ? dstHalfFov : ModularBody::halfFov)
                               * (360.0 / M_PI)));
    out.appendEntry("sky_locked", b2s(skyLocked));
    // The HELD matrix, not a derivation of it: lockedSkyRot is the rotation
    // captured when the lock engaged and it is not recoverable from anything
    // else in this file (§2 row B9). Written even while the lock is OFF, where
    // it is dormant rather than absent - the alternative (omit it, and let a
    // restore keep whatever the running app had) makes the restored state
    // depend on what the app was doing before the restore, which is exactly
    // what D33's idempotence forbids.
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
    // Every plan is cleared FIRST. A file describes a settled state, so a plan
    // that survived a restore would immediately start moving away from what was
    // just restored - and the second restore would then land somewhere else,
    // which is exactly the idempotence D33 requires (T4's fixed point).
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

    // longitude / latitude / altitude are NOT assigned here: they are restored
    // through the dual seam (SessionFile::Host::moveObserverTo), before this
    // runs, so that the old observer - which still draws the whole sky - moves
    // with the camera. Assigning them here would silently undo that.
    if (const std::string *v = in.find("position")) {
        Vec3f p = position;
        if (std::sscanf(v->c_str(), "%f,%f,%f", &p.v[0], &p.v[1], &p.v[2]) == 3)
            position = p;
    }
    readF(in, "alt", alt);
    readF(in, "az", az);
    // half_fov and sky_locked are NOT applied here either: both have a seam
    // that drives the old path too (the old projector's fov scales every star;
    // the old navigation flag is the sky lock's other half), and the session
    // restores them through it - see SessionFile::Host.
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
    // view_offset and its latch are NOT applied here: they have a seam that
    // drives the old path too (Core::setViewOffset is the ONE sink both §2(c)
    // channels funnel into, and the old navigator's offset is what pitches the
    // star field), so the session restores them through it - see
    // SessionFile::Host::setViewOffset. Same reason as the place, the fov and
    // the sky lock above.
    // foldLat is DERIVED (§2 row B7) and update() re-derives it; setting it
    // here would make the first restored frame re-fold a view that is already
    // expressed in the restored latitude's frame.
    foldLat = latitude;
}

// Dual-path trace harness (INTENT.md 11.14).
void Camera::dumpTrace(std::ostream &out) const
{
    const Vec3d rootPos = reference ? getRootPosition() : Vec3d(0, 0, 0);
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
        // View offset (B17): the clamped scalar, its arming transition, and the
        // EFFECTIVE offset (scalar·transition) that the render pitch uses — the
        // numeric observable for the offset A/B and the arming state channel.
        << ",\"viewOffset\":" << viewOffset
        << ",\"viewOffsetTransition\":" << viewOffsetTransition
        << ",\"viewOffsetEff\":" << effectiveViewOffset()
        << ",\"longitude\":" << longitude << ",\"latitude\":" << latitude
        << ",\"distance\":" << distance
        << ",\"alt\":" << alt << ",\"az\":" << az << ",\"heading\":" << heading
        // Absolute (root-aligned common-inertial) look direction - the B13
        // reference-change / free-mode continuity observable (INTENT 11.61).
        << ",\"absFwd\":[" << lastAbsFwd[0] << ',' << lastAbsFwd[1] << ',' << lastAbsFwd[2] << ']'
        << ",\"position\":[" << position[0] << ',' << position[1] << ',' << position[2]
        // Where the EYE is, in the ROOT (Universe) frame, in AU (B4(iv),
        // §11.141). The old path's observer position is recoverable from
        // `helioToEye` the same way (−Rᵀ·t), so this is the field that makes a
        // scripted travel comparable between the two paths per step - a pose
        // triple cannot be compared across two different references.
        // 17 digits, locally: this one field is a ~1 AU quantity whose
        // INTERESTING part is often 1e-5 AU, so the dump's own 9 digits would
        // quantize a travel's per-step comparison to 150 m.
        << "],\"rootPos\":[" << std::setprecision(17)
        << rootPos[0] << ',' << rootPos[1] << ',' << rootPos[2] << std::setprecision(9)
        << "],\"refAoI\":" << (reference ? reference->getAreaOfInfluence() : 0)
        << ",\"refDist\":" << (reference ? reference->getDistanceToObserver() : 0)
        << ",\"refCached\":" << ((reference && reference->isCacheFresh()) ? "true" : "false")
        << ",\"refParent\":\"" << ((reference && reference->getParent()) ? reference->getParent()->getEnglishName() : "") << '"'
        // Selected body + the observer's distance to it (== the far-mode
        // descent target, B21 §11.72): getObservedPosition() is what
        // Camera::descend aims at when the reference is a system. "" / 0 when
        // nothing is selected. Frame-independent scalar — the far-case metric a
        // system-distance dump cannot read off the (old-system-driven) per-body
        // list for a runtime-loaded target.
        << ",\"selected\":\"" << (ModularBody::getSelected() ? ModularBody::getSelected()->getEnglishName() : "")
        << "\",\"selDist\":" << (ModularBody::getSelected() ? ModularBody::getSelected()->getObservedPosition().length() : 0.f)
        << ",\"halfFov\":" << ModularBody::halfFov
        << ",\"cullHalfFov\":" << ModularBody::cullHalfFov
        // The two gaps b31-design §6.2 T2 names: the HELD sky-lock matrix (it
        // is state, not a derivation of anything else here) and the in-flight
        // plans (a session snaps them to their settled target, D32 - so a dump
        // that cannot see a plan cannot witness that they were snapped).
        << ",\"lockedSkyRot\":[";
    for (int i = 0; i < 16; ++i)
        out << lockedSkyRot.r[i] << ((i < 15) ? "," : "");
    out << "],\"plans\":{\"viewT\":" << viewT << ",\"hdgT\":" << hdgT
        << ",\"zoomDuration\":" << zoomDuration << ",\"moveDuration\":" << moveDuration
        << "},\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << lastDispatchedMat.r[i] << ((i < 15) ? "," : "");
    out << "]}";
}
