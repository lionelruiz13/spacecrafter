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
namespace ModularSystemFormat {
class Section;
}

// Two complementary viewing systems [vixy: 2026-07-12]: both are the SAME
// parametrization family around a different trust axis - ALTAZ references the
// zenith (center-of-body -> observer, lat/lon dependent), EQUATORIAL
// references the axis orthogonal to the equatorial circle (astronomical
// relevance). Implementation: one mount fold F in the view composition
// (identity vs X(pi/2-lat)); under EQUATORIAL the placement's latitude term
// cancels algebraically (F*X(lat-pi/2)=I), so latitude moves leave the sky
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
    // The shared PROXIMITY-FACTOR authority (B10 iv-b, S5.2): the base the
    // interactive movers (multAlt / moveRelLon / moveRelLat) multiply, measured
    // to the CLOSEST REACHABLE position (ground_radius), NOT the datum. When
    // `escaping` (an OUTWARD/lateral step, i.e. away from the ground) the result
    // is floored to ANTISTUCK_ESCAPE_FLOOR*radius so height 0 is never a fixed
    // point in any direction -- the S5.18 "stuck at the surface" defect. Inward
    // steps are NOT floored (the free-mode descent clamp in update() stops them
    // at ground_radius, R4 stop-and-hold).
    float proximityFactor(bool escaping) const;
    // Dual-path trace harness (INTENT.md 11.14): serialize the full observer
    // state (reference, pose, modes, halfFov) as one JSON object.
    void dumpTrace(std::ostream &out) const;
    // Change the reference body without moving
    // b31-design S2 group B, through the session file (SessionFile.hpp): this
    // camera's own state, written into and read back from one section. The
    // camera writes it because the camera OWNS it -- a save that asked the
    // control surface would get the OLD path's answer for half of these
    // (B33/S11.108(f), and S3.4(e) makes "read the model that draws" the
    // sharpest rule in the design).
    // saveSession writes the SETTLED value of anything in flight (D32): a move,
    // a zoom and a view-smoothing plan are saved as the state they were heading
    // for, never mid-ramp. `heading` is deliberately absent -- see the note the
    // save writes into the file, and D28.
    // restoreSession ASSIGNS every value and clears every plan, so restoring
    // twice, or restoring from the state a restore produced, lands on the same
    // place (D33: the file is a preset, and a preset that drifts is not one).
    // The reference body is NOT restored here: re-seating it is a warp, whose
    // environment enter/leave edges have subscribers.
    void saveSession(ModularSystemFormat::Section &out) const;
    void restoreSession(const ModularSystemFormat::Section &in);
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
    // isMaxDuration scales the duration with angle/pi (old Rotator semantics).
    void lookTo(const Vec3f &direction, float duration = 1, bool isMaxDuration = false);
    void lookTo(float _alt, float _az, float duration = 1, bool isMaxDuration = false);
    // The exact counterpart of `Navigator::updateMove(deltaAz, deltaAlt, fov)`,
    // and its parameters are OLD'S convention, not this class's: +deltaAlt
    // raises the VIEW and deltaAz turns it the way old's `azVision -= deltaAz`
    // does, so a caller hands the SAME two numbers to both paths (I2 -- the two
    // call sites, the key ramp and the mouse drag, cannot drift apart). Old's
    // pole clamp and its zero/zero guard are reproduced; a duration <= 0 snaps
    // (which is what an interactive ramp needs: old has no acceleration and no
    // deceleration, so a smoothing plan would ease in at the press and keep
    // moving after the release). See Camera.cpp for the sign derivation and the
    // measured float32 reason the snap assigns instead of round-tripping.
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
    // (`target`) and in-flight view plans (viewT>0) -- the old precedence
    // auto_move > tracking > lock -- and in freeMode. SUSPENDED for Vixy
    // (INTENT 11.58): (i) how a VIEW_HORIZON mount composes (old rolls with the
    // zenith holding direction-only; this holds the full orientation), (ii) the
    // free-mode composition, (iii) the old select-while-tracking auto-enable.
    void setSkyLock(bool b);
    inline bool getSkyLock() const {
        return skyLocked;
    }

    // READBACK ONLY (INTENT S11.133, B34's ramp member): the view parameters
    // this camera holds, in ITS OWN convention -- (alt, az, heading), where
    // az = -lng and alt = -lat of the forward direction in the param frame
    // (paramForward / viewRotation are the authority on that, Camera.cpp).
    // Const, side-effect-free, consumed only by the dump channel: the ramp
    // instrument needs the NEW path's half of a per-step comparison, and
    // `dumpTrace` only reports the state at dump time.
    inline Vec3f getViewParams() const {
        return Vec3f(alt, az, heading);
    }

    void moveHeading(float deltaHeading);
    void setHeading(float heading, float duration = 0);
    inline float getHeading() const {
        return heading;
    }

    void moveRel(const Vec3f &position, float duration = 0, bool calculateDuration = false);
    //! Step by a vector given in the EYE frame -- the RENDER eye frame, the one
    //! the operator is looking at.  Every vector its callers hand it is
    //! expressed there: descend() casts the screen-centre ray (0,0,-1) against
    //! `viewMat().getTranslation()` and passes the selected body's
    //! `getObservedPosition()`, all of which carry the B17 offset rotation R'
    //! (Camera.cpp).  So the inverse that undoes them is the FULL one, which is
    //! what `observedToLocalPos` now is (S11.216); before that change a descent
    //! with `set zoom_offset` armed stepped 27 deg away from the ray under the
    //! screen centre [measured: the step lay along Rv^T*z to 0.0121 deg and
    //! 26.998 deg from (R'.Rv)^T*z, harness/f96_offset.py].  Byte-identical at
    //! offset 0, which is every shipped default.
    inline void moveEyeRel(const Vec3f &position, float duration = 0) {
        moveRel(observedToLocalPos(position), duration);
    }
    inline void moveRelLon(float lon, float delay = 0) {
        if (freeMode) {
            // Lateral free-flight velocity ~ proximity to the ground (S5.2);
            // floored (escaping=true) -- lateral is never descent, so it must
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
    // Natural altitude control (S5.2 iv): a multiplier on the proximity to the
    // vertical the observer descends along. Free-flight geometry (view ray near
    // / last-selected far) lives in descend() (Camera.cpp, needs the complete
    // ModularBody); anchored stays the legacy radial altitude there. coef<1
    // descends, coef>1 ascends; floored only OUTWARD (coef>1) so takeoff from
    // height 0 is always possible.
    inline void multAlt(float coef) {
        descend(coef);
    }
    // View-directed free descent (B21, S11.72), the single altitude-geometry
    // authority behind multAlt and the `camera action descend` command. In
    // free flight "down" points along the VIEW RAY when the reference is a body
    // (toward the surface point under the screen centre, Q6/A18) and toward the
    // LAST SELECTED body when the reference is a system (far/galactic, R6
    // S11.70(e)); anchored keeps the legacy radial altitude. Rides B10's
    // proximityFactor() and composes with update()'s R4 ground clamp -- it
    // changes only the descent DIRECTION, never when a reference transition
    // fires (dispatch S2 carve-out). Defined in Camera.cpp (complete
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
    // viewRotation() = Z(heading+pi)*X(pi/2-alt)*Z(az-pi/2)*F  with F the mount
    // fold (see CameraMount). Everything else (inverse transforms, lookTo's
    // parameter derivation, transition deduction) is DERIVED from it - the
    // S11.19 divergences (missing -pi/2, az sign, missing roll pi) were exactly
    // independent re-statements of this composition drifting apart.
    Mat4f fold() const;
    Mat4f viewRotation() const;
    // Rotation applied downstream of the view (placement + surface fold)
    Mat4f placementRotation() const;

    // ---- View offset (old zoom_offset / [navigation] view_offset) ----------
    // Baked port of the old navigator's fov-coupled view rotation (B17,
    // S11.63/S11.79(c) -- reproduce old EXACTLY). The offset shifts the rendered
    // dome centre by `viewOffset` FRACTION of the dome radius (the "percent of
    // fov radius" the old comment names, core.cpp:2110), fov-INDEPENDENTLY: old
    // rotated the eye view matrix by view_offset*(fov/2) and the fisheye
    // transfer (r = theta/halfFov) turns that into a constant fractional shift
    // (navigator.cpp:309; S11.63(c)). Reproduced here as a render-only pitch of
    // `offset*halfFov` radians in the PHYSICAL eye/screen frame (see
    // viewOffsetEyeRotation for why the eye frame, not old's pre-heading chain
    // slot: the latter breaks the B13/B18 held-view composition the row
    // mandates). RENDER-ONLY: viewRotation() (the param<->view authority consumed
    // by lookTo and recoverParams) stays offset-free, exactly as old applies the
    // offset only to mat_local_to_eye, never to the vision math.
    // ~~and by observedToLocalPos~~ [CORRECTED 2026-09-06, S11.216 / S5.138:
    // observedToLocalPos is an INVERSE, not a param authority. Its input is an
    // OBSERVED position, which viewMat has already turned by R', so an inverse
    // that divided only by Rv left the offset pitch inside the answer -- measured
    // 27.000007 deg at the shipped fov 180 with `set zoom_offset 0.3`. It now
    // takes renderViewRotation()^T, the same full inverse the RA/DE members have
    // taken since F91 (S11.213(f)); the offset is still render-only in the sense
    // that matters -- no VIEW PARAMETER carries it.]
    //
    // setViewOffset is fed by Core::setViewOffset -- the ONE sink both S2(c)
    // channels ([navigation] view_offset at startup AND `set zoom_offset <v>` at
    // runtime, R11 S11.70) funnel into (core.cpp) -- so BOTH channels land here
    // through a single authority (I2); the [-0.5,0.5] clamp stays in that sink.
    // Value 0 (the default) is inaction: the offset path is byte-identical to
    // the pre-B17 render (D12 -- no silent acting default; offset 0 = no rotation).
    void setViewOffset(double offset);
    inline double getViewOffset() const {
        return viewOffset;
    }
    // Arm/disarm the offset transition (old view_offset_transition,
    // navigator.cpp:73-78): the offset is INERT at a fresh un-moved view and
    // ARMS on a commanded view move, resetting on a zoom-out-to-init -- hence
    // "zoom_offset". Reproduced faithfully: lookTo (the new-path analog of the
    // old navigator->moveTo that armed it) arms; Core::autoZoomOut disarms. The
    // transition ramps smoothly toward the armed target each frame; its
    // ENDPOINTS (0 fresh, 1 armed) are byte-exact against old, the ramp CURVE is
    // perceptual-parity (the new move law differs from old's atan easing).
    void armViewOffset(bool armed);

    //! The latch as a RESTORED CONDITION: D32 saves the latch and snaps its
    //! ramp, so this is `armViewOffset` with the transition landed rather than
    //! started. It lives beside the old path's half in Core::restoreViewOffset
    //! because one restore must not leave the two paths' offsets disagreeing -
    //! the old one is what pitches the star field.
    void restoreViewOffsetLatch(bool armed);

    // THE exact rotational inverse of the RENDER view rotation: observed(eye)
    // frame -> the frame the view acts on (zenith frame when anchored, body
    // frame when free).  ONE authority (I2) for every consumer that has to undo
    // what viewMat did -- observedPosToAltAz, EnvironmentManager's sun
    // direction, moveEyeRel and update()'s tracking feedback all read it and
    // none re-derives it; the RA/DE members below compose the SAME rotation
    // (see observedToBodyEquPos).
    //
    // WHY renderViewRotation()^T AND NOT viewRotation()^T (INTENT S11.216,
    // row S5.138): getObservedPosition() is R' * (the offset-free eye position)
    // -- viewOffsetEyeRotation's own note says so and viewMat composes exactly
    // that -- so dividing only by Rv leaves the B17 offset pitch INSIDE the
    // answer, conjugated into the local frame as Rv^T . R' . Rv: a rotation by
    // `offset*halfFov` about Rv^T*x_eye.  MEASURED at the shipped fov 180 with
    // `set zoom_offset 0.3`: the alt/az readout turned 27.000007 deg away from
    // the old path's, whose readouts carry no eye-frame content at all
    // (S11.201 -- old's offset lives in the draw and the aim), on every body
    // the update walk re-evaluates.  Old-parity is the target here by S11.52(b),
    // derived and not chosen.  BYTE-IDENTICAL to viewRotation()^T at every
    // shipped default: offset 0 => viewOffsetEyeRotation() returns the identity
    // on an exact float compare (Camera.cpp), so nothing below moves until the
    // offset is both non-zero AND armed.
    inline Vec3f observedToLocalPos(const Vec3f &observedPos) const {
        return renderViewRotation().transpose().multiplyWithoutTranslation(observedPos);
    }

    // ---- THE READOUT FRAME: viewMat()'s inverse (INTENT S11.213) ----------
    // viewMat() is, factor for factor (Camera.cpp, viewMat):
    //     anchored: R' . Rv . T(0,0,-distance) . X(lat-pi/2) . Z(-longitude) [. S]
    //     free:     R' . Rv . T(position)                                    [. S]
    // with Rv = viewRotation(), R' = viewOffsetEyeRotation() and
    // S = reference->computeSurfaceToBody().  Its inverse is therefore
    //     [S^T .] Z(+longitude) . X(pi/2-lat) . T(0,0,+distance) . Rv^T . R'^T
    // and that is what the members below compose, once, in Camera.cpp.  S^T is
    // spelled `computeBodyToSurface()`, which IS zrotation(+getAxisRotation())
    // and so is exactly S's transpose (ModularBody.hpp).
    //
    // WHICH FRAME THAT IS -- DERIVED FROM OLD'S CODE, NOT CHOSEN (S11.52(b),
    // S11.213(b)): the reference body's equatorial frame OF DATE, with right
    // ascension counted from the equinox that body's own sidereal time counts
    // from.  Old builds `mat_local_to_earth_equ` as Z(siderealTime + longitude)
    // . Y(90-latitude) [anchor_point_body.cpp, navigator.cpp:232], and
    // `getAxisRotation()` is that same sidereal time plus the pi/2 the mesh
    // convention carries -- so the fold, taken in FULL, lands on old's frame
    // exactly (measured 0.000000000 deg over 64 directions,
    // harness/f91_frame.cpp).  The old S11.4 caveat that used to sit here is
    // CLOSED: its decision (1) is definitional (S11.198(b)(1)) and its decision
    // (2) is the tester's own answer, round-3 R27 (S11.207(b) row 14).
    //
    // WHY THE FOLD MATTERS TWICE OVER: dropping it entirely is S5.86's defect
    // (the answer then rotates with the reference body); dropping only its
    // constant pi/2 leaves a fixed 90 deg right-ascension error, which is what
    // S11.158(f2) measured offline and recorded as a "-90.0003 deg zero point".
    //
    // R'^T, NOT JUST Rv^T: `getObservedPosition()` is R' * (the offset-free eye
    // position) -- viewOffsetEyeRotation's own note says so -- so an inverse
    // that divided only by Rv would leave the B17 view-offset pitch inside the
    // answer.  Old's RA/DE has no eye-frame content at all (it never passes
    // through a view matrix), so undoing R' is what old-parity requires as well
    // as what "the inverse of viewMat" means.  Byte-identical to Rv^T whenever
    // the offset is inert, which is every shipped default (offset 0 => R' ==
    // identity).
    //
    // THAT ROTATION IS NOW WRITTEN ONCE, ABOVE (S11.216): `observedToLocalPos`
    // IS `renderViewRotation()^T`, and these members compose it rather than
    // restate it.  Between F91 and S11.216 the split was real and measured --
    // this readout undid R' while the alt/az one did not, so with the offset
    // armed the two readouts of one body answered in frames 27 deg apart (row
    // S5.138).  ~~and update()'s tracking feedback already undoes it~~ [the
    // tracking feedback's hand-applied R'^T is GONE with the same change: it
    // now hands getObservedPosition() straight to observedToLocalPos, because
    // applying R'^T twice would put the offset back in.  One composition, one
    // place -- the I2 lesson S11.153 paid for, applied to the last consumer
    // that had its own copy of it.]

    //! Observed(eye) frame -> the reference's equatorial frame, ROTATION ONLY:
    //! the origin stays where `observedPos` already puts it, at the OBSERVER.
    //! TOPOCENTRIC, which is old's contract for this quantity twice over --
    //! `Body::getEarthEquPos` is `nav->helioToEarthPosEqu(...)`, whose own doc
    //! reads "equatorial coordinate but centered on the observer position"
    //! [navigator.hpp, body.cpp] -- and the tester's answer R27: "The RA/DE
    //! must be the value from our position."  THE single authority (I2) for
    //! every new-path readout that has to land in old's frame.
    Vec3f observedToBodyEquPos(const Vec3f &observedPos) const;

    //! The LITERAL inverse of viewMat(): the same frame, origin restored to the
    //! reference body's CENTRE.  Kept because it is the map viewMat defines and
    //! the one the round-trip probes score (harness/f34_probe_inverse.cpp,
    //! f91_frame.cpp); it has NO in-tree consumer since S11.213, both readouts
    //! wanting the observer as origin.  Expressed from the authority above
    //! rather than beside it: the origin term is linear, so carrying it through
    //! the same rotations at the end is exact, and the topocentric answer keeps
    //! the shorter chain (no cancellation for a body close to the observer).
    Vec3f observedToBodyLocalPos(const Vec3f &observedPos) const;

    //! (ra, de) of an observed position, in the frame above.
    std::pair<float, float> observedPosToRaDe(const Vec3f &observedPos) const;
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
    // The interactive ZOOM ramp's sink (B34, INTENT S11.133): put the drawn fov
    // at `halfFov` NOW, exactly as `Projector::changeFov` puts the old fov there
    // now. It is NOT `setHalfFov(x, 0)`: with a zoom plan in flight that call
    // RE-PLANS rather than applying, and with duration 0 the re-plan never lands
    // at all (`zoomDuration` becomes 0 before `update()` can run the block), so
    // the drawn fov would freeze while old's ramped on. Old writes its fov under
    // an in-flight auto-zoom too and lets the plan overwrite it on the next
    // frame; this reproduces that -- the value lands now, any plan keeps its own
    // schedule. Same clamp as setHalfFov (the two share the class's fov range).
    void setHalfFovNow(float halfFov);

    // ---- WHERE THE OBSERVER IS, as a vector (B4(iv), S11.141) -------------
    // The scripted transitions ask a question the spherical triple cannot
    // answer: "where is the eye, in a frame some OTHER body also lives in".
    // The three methods below are one inverse pair plus its re-expression, and
    // they are derived from viewMat() -- the matrix the renderer actually uses --
    // never from a second parametrization: whatever convention viewMat holds,
    // the eye origin's position in the reference's frame is exactly -R^T*t of
    // that affine map, so these cannot drift from what is drawn (I2).
    //
    // getReferenceRelativePosition: the eye's offset from the reference body's
    // centre, in the reference's ACCUMULATED EQUATORIAL frame (the surface fold
    // is undone, so the value is comparable across a spinning reference and is
    // the frame calculateSwitchCompensation speaks).
    Vec3f getReferenceRelativePosition() const;
    // The same offset re-expressed in `body`'s accumulated equatorial frame,
    // through the ONE inter-body transform the reference switches use
    // (ModularBody::calculateSwitchCompensation). `body` == reference returns
    // the value above unchanged.
    Vec3f positionRelativeTo(const ModularBody *body) const;
    // The eye's position in the ROOT (Universe) frame -- the frame the old
    // path's anchors call "heliocentric ecliptic" (the chain above the Sun is
    // at the origin on the shipped data: MEASURED, Sun/SolarSystem/MilkyWay all
    // dump ecl [0,0,0], S11.141 -- asserted by the gate, not assumed).
    // DOUBLE, because the terms are ~1 AU and the answers ~1e-5 AU.
    Vec3d getRootPosition() const;
    // The exact INVERSE of getReferenceRelativePosition: put the eye at `pos`
    // in the reference's accumulated equatorial frame. `holdView` keeps the
    // composed orientation across the placement (A38/B13 -- a reference switch
    // holds the whole orientation), which is what a transition that must not
    // turn the image needs; without it the view rides the placement, which is
    // what a plain observer move does. A zero-length `pos` keeps the current
    // longitude/latitude (they parametrize nothing at the centre) and only
    // zeroes the distance.
    void placeAt(const Vec3f &pos, bool holdView);

    // The exact inverse of moveTo's target: the legacy spherical triple this
    // camera IS at - (longitude, latitude) in radians, altitude above the
    // reference's altitude datum in AU - in BOTH modes, for the same reason
    // moveTo is absolute in that triple in both modes (see moveTo: "moveto is
    // the legacy positioning surface and must stay meaningful in free flight").
    // A control surface that can COMMAND a place must be able to report the
    // place it is at, and the observatory getters are the readout that has to
    // answer for the path that DRAWS (B33, S11.108(f)).
    // Anchored: the pose members themselves. Free: derived from `position` with
    // the SAME conversion setFreeMode(false) uses to leave free flight
    // (posePartToPose, the one authority) - the spherical members are frozen at
    // the free-mode entry, so reporting them there would name a place the
    // camera has left. That sameness is a REQUIREMENT and not a convenience:
    // this member and the exit have to name one place, or leaving free flight
    // moves the observer to somewhere the readout never reported (S5.80).
    // Units stay rad/AU: the deg/metres conversion is the
    // CoreLink seam's, where observerMoveTo already does the write half of it.
    Vec3f getPlace() const;

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
    // Added by S11.213: the class had no longitude accessor at all, and
    // ModularObject's local-hour-angle line was reading getLatitude() in the
    // slot old spells `observatory->getLongitude()` [body.cpp].  A missing
    // accessor is a plausible mechanism for that slip, so it is supplied here
    // rather than worked around at the call site.
    inline float getLongitude() const {
        return longitude;
    }
    // This disallow using multiple cameras, but multiple cameras can't be used simultaneously anyway
    static Camera *instance;
private:
    //! Steps 3 and 4 of viewMat()'s inverse: the zenith(local) frame -> the
    //! reference body's equatorial frame.  Written ONCE because both public
    //! members of S11.213 need exactly it, and because the pair that drifted
    //! apart before was two re-statements of one composition (I2, the S5.80
    //! shape recorded just below).
    Vec3f localToBodyEqu(Vec3f v) const;

    // ---- THE pose part: one authority for triple <-> cartesian ------------
    // S5.80/S11.153. `posePart` is viewMat()'s ANCHORED branch solved for the
    // eye, and `posePartToPose` is its exact inverse; between them they are the
    // ONLY place in this class where the legacy spherical triple and a
    // cartesian place convert into each other. That is the point: the free-mode
    // teleport S5.80 recorded was FOUR independent re-statements of this
    // conversion (setFreeMode both ways, moveTo's free branch, getPlace) drifting
    // away from the composition viewMat actually uses -- the same shape as
    // S11.19's three drifted view compositions, which is why the fix is a single
    // authority and not four corrected expressions (I2).
    //
    //   viewMat anchored = R . T(0,0,-distance) . X(lat-pi/2) . Z(-lon) [. S]
    //     => the eye it draws is  S^-1 . posePart(lon,lat,distance)
    //   viewMat free     = R . T(position) [. S]
    //     => the eye it draws is  -S^-1 . position
    //   so the two describe the SAME place exactly when position == -posePart(),
    //   for every surface fold S -- the fold cancels, which is why neither
    //   member below mentions it (checked bound AND unbound, harness/f40_probe.cpp).
    //
    // NOTE the -pi/2 the pair (sin lon, -cos lon) carries: the anchored pose
    // azimuth is `longitude - pi/2`, this class's own longitude origin (S5.49's
    // channel -- reported, never corrected here: correcting it would move every
    // ANCHORED place, which is the baseline).
    static inline Vec3f posePart(float lon, float lat, float dist) {
        const float cl = std::cos(lat);
        return Vec3f(dist * cl * std::sin(lon), -dist * cl * std::cos(lon),
                     dist * std::sin(lat));
    }
    // The exact inverse: the (longitude, latitude, distance) triple whose pose
    // part is `p`. At p == 0 the eye is AT the reference's centre, where the
    // angles parametrize nothing -- the caller's current pair is returned rather
    // than atan2(0,0)/asin(0/0), so a point anchor keeps the place it arrived
    // with (placeAt's own rule, which this member now carries for every caller).
    static inline Vec3f posePartToPose(const Vec3f &p, float lonAt0, float latAt0) {
        const float d = p.length();
        if (d == 0.f)
            return Vec3f(lonAt0, latAt0, 0.f);
        return Vec3f(std::atan2(p[0], -p[1]), std::asin(p[2]/d), d);
    }
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
    // Euler extraction of totalRot*placement^-1*F^-1; pole-degenerate case
    // keeps the current az (same policy as lookTo). Drops any in-flight view
    // plan (the plan's frame changed); tracking re-plans next frame.
    void recoverParams(const Mat4f &totalRot);
    // Advance the view/heading smoothing plans (constant-min-acceleration law)
    void advanceView(float deltaTime);
    // The render view rotation = viewRotation() with the view offset pitch
    // inserted at old's chain position (between the heading roll and the base
    // view, navigator.cpp:309/314). Delegates to viewRotation() verbatim when
    // the offset is inert (effective 0) so the no-offset render stays BYTE-
    // identical to the pre-B17 path (I2: viewRotation() is the sole composition
    // authority; this mirrors it with exactly one added factor).
    Mat4f renderViewRotation() const;
    // THE view matrix (eye <- the reference's accumulated-equatorial frame)
    // this camera's CURRENT parameters describe -- what update() hands to
    // dispatchUpdate, and therefore what the reference's `mat` (and so
    // getObservedPosition()) is a once-per-frame COPY of. Every consumer that
    // needs the reference's geometry in the eye frame BETWEEN two updates asks
    // here rather than reading that copy back (S5.32, Camera.cpp).
    Mat4f viewMat() const;
    // The view offset as an eye-space rotation R' (mat_render == R'*mat_free);
    // identity when inert. Both the render composition and the tracking
    // feedback-undo use it (Camera.cpp).
    Mat4f viewOffsetEyeRotation() const;
    // Advance the view-offset transition (old view_offset_transition ramp).
    void advanceViewOffset(float deltaTime);
    inline float effectiveViewOffset() const {
        return static_cast<float>(viewOffset) * viewOffsetTransition;
    }
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
    // and held every frame while skyLocked -- the new-path analog of the old
    // path's held `equ_vision`.
    bool skyLocked = false;
    Mat4f lockedSkyRot;
    // View offset (old navigator view_offset / view_offset_transition, B17).
    // viewOffset: the clamped [-0.5,0.5] scalar fed by Core::setViewOffset (both
    // channels). viewOffsetTransition: the arming ramp (0 inert .. 1 armed).
    // viewOffsetArmed: the sticky latch a commanded move sets and a zoom-out
    // clears. All default to the inert state => zero render effect until the
    // operator sets a non-zero offset AND a commanded move arms it.
    double viewOffset = 0;
    float viewOffsetTransition = 0;
    bool viewOffsetArmed = false;
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
