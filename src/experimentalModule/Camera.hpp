#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/vecmath.hpp"
#include "tools/utility.hpp"
#include "tools/sc_const.hpp"
#include "EntityCore/Executor/Task.hpp"
#include <cmath>
#include <atomic>
#include <iosfwd>

class Renderer;
class ModularBody;
class ModularSystem;

// Two complementary viewing systems [vixy: 2026-07-12]: both are the SAME
// parametrization family around a different trust axis - ALTAZ references the
// zenith (center-of-body -> observer, lat/lon dependent), EQUATORIAL
// references the axis orthogonal to the equatorial circle (astronomical
// relevance). Implementation: one mount fold F in the view composition
// (identity vs X(π/2−lat)); under EQUATORIAL the placement's latitude term
// cancels algebraically (F·X(lat−π/2)=I), so latitude moves leave the sky
// fixed and longitude acts as hour-angle - the astronomically correct
// behavior falls out of the composition.
enum class CameraMount : uint8_t {
    ALTAZ,
    EQUATORIAL,
};

class Camera {
public:
    Camera(ModularBody *reference, float longitude, float latitude, float altitude);
    // Return the distance to the reference in AU (relative to datum_radius, the
    // altitude zero-point). Feeds the altitude readout and `moveto altitude`.
    float distanceToReference() const;
    // The shared PROXIMITY-FACTOR authority (B10 iv-b, §5.2): the base the
    // interactive movers (multAlt / moveRelLon / moveRelLat) multiply, measured
    // to the CLOSEST REACHABLE position (ground_radius), NOT the datum. When
    // `escaping` (an OUTWARD/lateral step, i.e. away from the ground) the result
    // is floored to ANTISTUCK_ESCAPE_FLOOR·radius so height 0 is never a fixed
    // point in any direction — the §5.18 "stuck at the surface" defect. Inward
    // steps are NOT floored (the free-mode descent clamp in update() stops them
    // at ground_radius, R4 stop-and-hold).
    float proximityFactor(bool escaping) const;
    // Dual-path trace harness (INTENT.md 11.14): serialize the full observer
    // state (reference, pose, modes, halfFov) as one JSON object.
    void dumpTrace(std::ostream &out) const;
    // Change the reference body without moving
    void switchToBody(ModularBody *dst);
    // Change the reference body
    void warpToBody(ModularBody *dst);
    void update(double jd, float deltaTime);
    void draw(Renderer &renderer);
    void setBoundToSurface(bool b);
    void setFreeMode(bool b);

    // View moves are SMOOTHED with constant minimal acceleration (two
    // quadratic phases, equal |a|, velocity-continuous retarget) - the
    // dome-comfort law [vixy: 2026-07-12: smallest motion sickness projected
    // into a half-sphere dome viewed from inside]. duration <= 0 snaps.
    // isMaxDuration scales the duration with angle/π (old Rotator semantics).
    void lookTo(const Vec3f &direction, float duration = 1, bool isMaxDuration = false);
    void lookTo(float _alt, float _az, float duration = 1, bool isMaxDuration = false);
    void lookRel(float deltaAlt, float deltaAz, float duration = 1, bool isMaxDuration = false);

    // Change the mount, keeping the current view exactly (deduce-identical-
    // view primitive). ALTAZ params are alt/az; EQUATORIAL params are DE/HA.
    void setMount(CameraMount m);
    inline CameraMount getMount() const {
        return mount;
    }

    // Sky-lock (old flag_lock_equ_pos, the "equatorial-mount sky-lock"): hold
    // the eye orientation FIXED in the reference body's EQUATORIAL frame as
    // sidereal time advances, instead of the default LOCAL-horizon lock. ON
    // captures the current body->eye rotation (viewRotation*placementRotation);
    // update() then re-derives (alt,az,heading) against the sidereal-advanced
    // placement every frame so the composed rotation stays == the captured one
    // -> same RA/DE, drifting alt/az (the old-path `equ_vision` held while
    // `local_vision = earthEquToLocal(equ_vision)` is recomputed, navigator.cpp
    // :128-130). Reproduces the shipped-config observable (VIEW_EQUATOR +
    // flag_lock_equ_pos = frozen equatorial sky) EXACTLY, mount-independently,
    // since the whole composed rotation is held. DORMANT under tracking
    // (`target`) and in-flight view plans (viewT>0) — the old precedence
    // auto_move > tracking > lock — and in freeMode. SUSPENDED for Vixy
    // (INTENT 11.58): (i) how a VIEW_HORIZON mount composes (old rolls with the
    // zenith holding direction-only; this holds the full orientation), (ii) the
    // free-mode composition, (iii) the old select-while-tracking auto-enable.
    void setSkyLock(bool b);
    inline bool getSkyLock() const {
        return skyLocked;
    }

    void moveHeading(float deltaHeading);
    void setHeading(float heading, float duration = 0);
    inline float getHeading() const {
        return heading;
    }

    void moveRel(const Vec3f &position, float duration = 0, bool calculateDuration = false);
    inline void moveEyeRel(const Vec3f &position, float duration = 0) {
        moveRel(observedToLocalPos(position), duration);
    }
    inline void moveRelLon(float lon, float delay = 0) {
        if (freeMode) {
            // Lateral free-flight velocity ∝ proximity to the ground (§5.2);
            // floored (escaping=true) — lateral is never descent, so it must
            // stay escapable at height 0.
            lon *= proximityFactor(true) * 5.f;
            moveEyeRel({lon, 0, 0}, delay);
        } else {
            moveRel({lon, 0, 0}, delay);
        }
    }
    inline void moveRelLat(float lat, float delay = 0) {
        if (freeMode) {
            lat *= proximityFactor(true) * 5.f;
            moveEyeRel({0, lat, 0}, delay);
        } else {
            moveRel({0, lat, 0}, delay);
        }
    }
    // With alt in meters
    inline void moveRelAlt(double alt, float delay = 0) {
        alt /= 1000*AU;
        if (freeMode) {
            moveEyeRel({0, 0, static_cast<float>(-alt)}, delay);
        } else {
            moveRel({0, 0, static_cast<float>(alt)}, delay);
        }
    }
    // Natural altitude control (§5.2 iv): a multiplier on the proximity to the
    // vertical the observer descends along. Free-flight geometry (view ray near
    // / last-selected far) lives in descend() (Camera.cpp, needs the complete
    // ModularBody); anchored stays the legacy radial altitude there. coef<1
    // descends, coef>1 ascends; floored only OUTWARD (coef>1) so takeoff from
    // height 0 is always possible.
    inline void multAlt(float coef) {
        descend(coef);
    }
    // View-directed free descent (B21, §11.72), the single altitude-geometry
    // authority behind multAlt and the `camera action descend` command. In
    // free flight "down" points along the VIEW RAY when the reference is a body
    // (toward the surface point under the screen centre, Q6/A18) and toward the
    // LAST SELECTED body when the reference is a system (far/galactic, R6
    // §11.70(e)); anchored keeps the legacy radial altitude. Rides B10's
    // proximityFactor() and composes with update()'s R4 ground clamp — it
    // changes only the descent DIRECTION, never when a reference transition
    // fires (dispatch §2 carve-out). Defined in Camera.cpp (complete
    // ModularBody: getObservedPosition/getSelected/isSystem).
    void descend(float coef);
    // Target is the legacy spherical triple (lon, lat, altitude-in-AU) in
    // BOTH modes - moveto is the legacy positioning surface and must stay
    // meaningful in free flight (2(c): one control surface, both modes).
    // The previous freeMode branch subtracted `position` (a cartesian
    // vector) from the spherical triple - a frame mismatch that made every
    // legacy move garbage in free flight (INTENT 11.36). Defined in
    // Camera.cpp (needs the complete ModularBody for the altitude reference).
    void moveTo(const Vec3f &pos, float duration = 0, bool calculateDuration = false);

    // ---- View composition: THE single authority on conventions ------------
    // viewRotation() = Z(heading+π)·X(π/2−alt)·Z(az−π/2)·F  with F the mount
    // fold (see CameraMount). Everything else (inverse transforms, lookTo's
    // parameter derivation, transition deduction) is DERIVED from it - the
    // §11.19 divergences (missing −π/2, az sign, missing roll π) were exactly
    // independent re-statements of this composition drifting apart.
    Mat4f fold() const;
    Mat4f viewRotation() const;
    // Rotation applied downstream of the view (placement + surface fold)
    Mat4f placementRotation() const;

    // Exact rotational inverse of viewRotation(): camera(observed) frame ->
    // the frame the view acts on (zenith frame when anchored, body frame when
    // free). NOTE: observedPosToRaDe/AltAz below inherit the §11.19 frame
    // corrections — their absolute calibration against the old path is the
    // still-open INTENT §11.4 caveat.
    inline Vec3f observedToLocalPos(const Vec3f &observedPos) const {
        return viewRotation().transpose().multiplyWithoutTranslation(observedPos);
    }
    inline Vec3f observedToBodyLocalPos(const Vec3f &observedPos) const {
        Vec3f ret = observedToLocalPos(observedPos);
        if (freeMode) {
            ret -= position;
        } else {
            ret.v[2] -= distance;
            ret = Mat4f::yrotation(latitude-M_PI_2).multiplyWithoutTranslation(ret);
            ret = Mat4f::zrotation(-longitude).multiplyWithoutTranslation(ret);
        }
        return ret;
    }
    inline std::pair<float, float> observedPosToRaDe(const Vec3f &observedPos) const {
        Vec3f direction = observedToBodyLocalPos(observedPos);
        std::pair<float, float> ret;
        // Pole test was (x + y) == 0, which also swallowed every x == −y
        // direction (same defect class as the old lookTo branch).
        if (direction[0] == 0 && direction[1] == 0) {
            ret.second = 0;
            ret.first = std::copysign(M_PI_2, direction[2]);
        } else {
            Utility::rectToSphe(&ret.first, &ret.second, direction);
        }
        return ret;
    }
    inline std::pair<float, float> observedPosToAltAz(const Vec3f &observedPos) const {
        Vec3f direction = observedToLocalPos(observedPos);
        std::pair<float, float> ret;
        if (direction[0] == 0 && direction[1] == 0) { // was x+y==0, see RaDe
            ret.first = std::copysign(M_PI_2, direction[2]);
            ret.second = 0;
        } else {
            Utility::rectToSphe(&ret.second, &ret.first, direction);
        }
        return ret;
    }
    inline void trackBody(nullptr_t) {
        target = nullptr;
    }
    inline void trackBody(ModularBody *body) {
        target = body;
    }
    inline ModularBody *getReferenceBody() const {
        return reference;
    }
    inline ModularBody *getTrackedBody() const {
        return target;
    }
    // Re-seat the reference on the object that REPLACED the current reference
    // (same body by identity, new C++ object - system reload). Deliberately
    // NOT switchToBody/warpToBody: nothing moved and nothing changed frame,
    // so no compensation and no altitude re-basing may be applied - the pose
    // parameters (lon/lat/distance or free position, view, heading) describe
    // an observer that did not move. Callers own the identity claim (by name).
    inline void rebindReference(ModularBody *dst) {
        reference = dst;
    }
    // Same for the tracked body (nullptr = stop tracking).
    inline void rebindTarget(ModularBody *dst) {
        target = dst;
    }
    inline ModularSystem *getCurrentSystem() const {
        return system;
    }
    inline bool isFreeMode() const {
        return freeMode;
    }
    void setHalfFov(float halfFov, float duration = 0.5);

    // Compatibility methods, only work while not in freeMode
    inline void setLongitude(float l) {
        longitude = l*M_PI/180;
    }
    inline void setLatitude(float l) {
        latitude = l*M_PI/180;
    }
    void setAltitude(double altitude);
    inline float getLatitude() const {
        return latitude;
    }
    // This disallow using multiple cameras, but multiple cameras can't be used simultaneously anyway
    static Camera *instance;
private:
    // Calculate intermediate zoom coefficient
    inline float calculateZoomCoef() const {
        float coef = zoomTimer / zoomDuration;
        if (coef > 0.5) {
            coef = 1 - coef;
            coef = 1 - 2*coef*coef;
        } else
            coef *= 2*coef;
        return coef;
    }
    // Calculate zoom coefficient velocity (derivated for one second)
    inline float calculateZoomCoefVelocity() const {
        float coef = zoomTimer / zoomDuration;
        coef = (coef > 0.5) ? 4*(1-coef) : 4*coef;
        return coef / zoomDuration;
    }
    // Deduce (heading, alt, az) reproducing `totalRot` under the CURRENT
    // modes/mount/placement - "the configuration which provides the visually
    // identical view" [vixy: 2026-07-12], the ONE primitive behind every
    // transition (freeMode, boundToSurface, mount, reference switch). ZXZ
    // Euler extraction of totalRot·placement⁻¹·F⁻¹; pole-degenerate case
    // keeps the current az (same policy as lookTo). Drops any in-flight view
    // plan (the plan's frame changed); tracking re-plans next frame.
    void recoverParams(const Mat4f &totalRot);
    // Advance the view/heading smoothing plans (constant-min-acceleration law)
    void advanceView(float deltaTime);
    // Current forward direction in the PARAM frame (post-fold), from alt/az
    Vec3f paramForward() const;
    // D1: the frame draw rides the render chain (RenderChain.hpp) - THE
    // serialization point all publish tasks order against. Full update+draw
    // fusion (main-thread-as-cadencer inversion) is second-pass; until then
    // the existing App loop calls draw(), which executes the task inplace
    // when the chain is idle (the common case - zero behavior change).
    // BRIDGE: if the frame task gets chained behind an in-flight publish,
    // draw() waits for completion on `done` - the App flow needs the command
    // buffers recorded before it submits. Bounded-tiny wait (publishes are
    // O(small) by contract); disappears entirely at the inversion, where
    // submission itself becomes part of the frame task.
    class FrameDrawTask : public Task {
    public:
        virtual void start(Taskable *target) override;
        Renderer *renderer = nullptr;
        Camera *camera = nullptr;
        std::atomic<bool> done{true};
    } frameDrawTask;
    ModularBodyPtr reference;
    ModularBodyPtr target;
    ModularSystem *system; // Determined on construction and update
    // ---- View smoothing plan (constant minimal acceleration) --------------
    // Great-circle move of the forward direction in the param frame: rotate
    // viewFrom about viewAxis by s(t), where s follows the two-phase profile
    // (accel a until t1, decel -a until T, end velocity 0). Velocity carries
    // across retargets (tracking re-plans every frame). Solver validated
    // standalone: 20k random (v0,d,T) plans, exact landing, minimal-|a| root.
    Vec3f viewFrom;        // path start direction (unit, param frame)
    Vec3f viewAxis;        // rotation axis (unit, param frame)
    float viewAngle = 0;   // total planned angle from viewFrom (signed = 0..)
    float viewT1 = 0;      // acceleration phase end
    float viewT = 0;       // plan duration; 0 = no plan
    float viewTimer = 0;
    float viewV0 = 0;      // velocity at plan start (rad/s along the path)
    float viewA = 0;       // phase-1 acceleration (signed)
    // Heading 1-D plan, same law
    float hdgTarget = 0, hdgT1 = 0, hdgT = 0, hdgTimer = 0, hdgV0 = 0, hdgA = 0, hdgFrom = 0;
    float alt = 0;
    float az = 0;
    float heading = 0;
    // Latitude baked into the EQUATORIAL fold when the params were last
    // derived. Old-path mount semantics: the view direction is LOCAL-frame
    // locked under observer moves regardless of mount (sky-locking is a
    // separate feature, old flag_lock_equ_pos); the mount only sets the
    // up-reference. update() re-derives the params across latitude changes to
    // preserve the zenith-frame direction (ALTAZ: fold=I, nothing to do).
    float foldLat = 0;
    CameraMount mount = CameraMount::ALTAZ;
    // Sky-lock state (old flag_lock_equ_pos). lockedSkyRot is the body->eye
    // rotation (viewRotation*placementRotation) captured when the lock engaged
    // and held every frame while skyLocked — the new-path analog of the old
    // path's held `equ_vision`.
    bool skyLocked = false;
    Mat4f lockedSkyRot;
    Vec3f position;
    Vec3f deltaPosition;
    float moveDuration = 0;
    Mat4f lastDispatchedMat; // harness: the mat handed to dispatchUpdate (INTENT 11.14a)
    // harness: the eye's forward (screen-center) direction in the ROOT-aligned
    // common-inertial frame (INTENT 11.61, B13). lastDispatchedMat is eye <-
    // reference's accumulated-equatorial frame (body-specific); this is the same
    // look direction re-expressed in the single root frame every body shares, so
    // it is directly comparable ACROSS a reference switch - the absolute-sky-
    // direction continuity observable Q2/A11 asks for (alt/az is frame-relative).
    Vec3f lastAbsFwd;
    float longitude;
    float latitude;
    float distance; // In AU
    float zoomDuration = 0;
    float zoomTimer;
    float srcHalfFov;
    float dstHalfFov;
    static float minHalfFov;
    static float maxHalfFov;
    bool freeMode = false;
    bool boundToSurface = true;
};

#endif /* end of include guard: CAMERA_HPP_ */
