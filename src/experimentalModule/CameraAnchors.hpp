#ifndef CAMERA_ANCHORS_HPP_
#define CAMERA_ANCHORS_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/utility.hpp"
#include <string>
#include <map>
#include <vector>
#include <iosfwd>

class Camera;
class ModularBody;
class ModularSystem;

// ============================================================================
// CameraAnchors - the NEW path's named-anchor layer (B4, S12 row 19, S7).
//
// WHAT IT IS FOR (I1): a show declares WHERE the camera is attached by NAME -
// "put me on the point that orbits the Moon", "put me on Mars keeping my
// angle" - through two channels that must mean the same thing (S2(c)):
// the authored `anchor.ini` and the runtime `camera action create/switch/drop/
// follow_rotation` commands. This class owns that vocabulary; the Camera below
// it owns only reference/pose/modes and knows nothing about names.
//
// THE THREE KINDS [R3, tester 2026-07-22, S11.70(b)] - each with the anchor.ini
// `type` value that declares it, and what the camera does when you switch to it:
//
//  (1) ON_ORBIT  (`type = orbit`) - PRIMARY, the kind actually used: a point
//      riding an orbit around a body. The anchor OWNS an invisible ModularBody
//      carrying that orbit, parented to `parent = <body>`; the camera simply
//      references it. Everything that makes the anchor hold through the body's
//      motion (ephemeris evaluation, parent-frame accumulation, reference-switch
//      compensation, lifetime) is the tree's, not this class's - I2: there is no
//      second position authority here.
//
//  (2) ATTACHED  (`type = body` + `body_name = <body>`) - attached to a body.
//      `follow_rotation = false` on the anchor is the kind R3 calls the NEEDED
//      GAP ("an object attached to a body keeping its angle to the planet"):
//      the camera holds its lat/lon in the body's EQUATORIAL frame, so the body
//      spins under it and the approach angle is preserved. That is exactly
//      Camera::setBoundToSurface(false) - measured as today's meaning of "not
//      following the rotation" (see S11.111). `follow_rotation = true` re-binds
//      to the surface (today's default for a body reference). The key ABSENT is
//      INACTION: the camera's current bind state is left alone, so every
//      pre-existing scene is bit-unchanged and there is no acting default to
//      log (D12; the D18 "inaction is no rotation" precedent).
//      D28 DEPENDENCY: what "keeping the angle" does to the image ROLL across
//      the reference change the switch performs is D28's open question, not this
//      class's - the switch uses Camera::warpToBody, so the kind inherits
//      TODAY's answer (B13 S11.61: the whole orientation is held) and follows D28
//      wherever it lands, with no anchor-side re-statement to keep in sync.
//
//  (3) FIXED_POINT (`type = point` | `type = observatory`) - a fixed point in AU.
//      R3: "only useful in the Universe mode... nothing moves there", so its
//      coordinates are read in the ROOT (Universe) frame - the one frame in
//      which a fixed point stays fixed - and the anchor owns an invisible body
//      parked there. Switching to it therefore PUTS the camera in the universe
//      frame rather than refusing outside it: on the new path the reference
//      chain IS the mode (S6.9), so "valid only in Universe mode" is realized by
//      the anchor living at the universe root, not by a mode test. The choice is
//      logged at the switch (D12) so a show that lands there sees why.
//      `observatory` folds into the same kind: it differs from `point` only by
//      its local-frame parametrization (anchor_point_observatory.cpp:32-40
//      zrot(lon)yrot(90-lat)), and the new-path camera parametrizes EVERY
//      reference that way already (Camera::placementRotation) - the distinction
//      has no new-path counterpart, so both spellings load.
//
// OWNED ANCHOR BODIES are created HIDDEN: parent-owned, position-ticking (B19
// S11.54), outside every draw/pick walk by construction - an anchor is a place,
// never a thing to see or click. They carry radius/datum/ground 0 (altitude is
// measured from the point itself; a point is not landable) and BodyType::ANCHOR,
// which is what the dump reports so a harness can tell an anchor from a body.
//
// LIFETIME (I5): the registry holds ModularBodyPtr, and an owned body that a
// system reload destroyed is REBUILT from the declaration it was made from
// (the params map kept in each entry is the authority) - checked at every use
// through the name registry, never by dereferencing a stale pointer.
//
// THE SCRIPTED TRANSITIONS (B4(iv), S11.141) - the C3 time-driven half of row
// 19, dual since 2026-08-09. `camera action move_to / transition_to` now reach
// this class as well as the old AnchorManager, and they are expressed with the
// SAME structural answer the three kinds are: A TRAVEL MOVES THE PLACE. The old
// path travels by writing a new heliocentric position onto the current anchor
// every frame; here the place is a body, a body's position IS its orbit, and a
// travel IS a position-at-date function - so a travel is a re-declared MOTION
// LAW (ModularBody::setOrbit) and there is still exactly one position authority
// (I2), still evaluated by the tree, still a pure function of the date (so a
// dropped frame or a time jump lands where the date says, as old's does).
//
// NOT HERE: cross-session persistence [Q5: explicitly not needed];
// saveCameraPosition, which stays the old AnchorManager's (S5.41 / B31 rule the
// serializer); the ROLL half of transitionToBody and `align_with` - see
// transitionToBody below and S11.141 for the terms that do not derive.
// ============================================================================

//! Which of the three R3 kinds an anchor is. The `type` data value maps here
//! once, in CameraAnchors.cpp (parseKind) - no consumer re-reads the string.
enum class AnchorKind : uint8_t {
    ON_ORBIT,    //!< `type = orbit`  - a point on an orbit around a body (PRIMARY)
    ATTACHED,    //!< `type = body`   - attached to a body (+/- follow_rotation)
    FIXED_POINT, //!< `type = point` | `type = observatory` - a fixed point in AU
};

class CameraAnchors {
public:
    //! @param root the tree root (the Universe node): the frame FIXED_POINT
    //! anchors are read in, and the parent their bodies are created under.
    explicit CameraAnchors(ModularSystem *root) : root(root) {}
    ~CameraAnchors();

    //! Channel 1 (S2(c)): load an authored anchor file. Grammar = the shipped
    //! `anchor.ini`'s, unchanged: `[section]` lines separate anchors (the
    //! section NAME is not read - it never was, AnchorManager::load:226-258),
    //! `key = value` inside, `#` comments. Each anchor block is handed to add()
    //! verbatim, so the two channels share ONE declaration grammar (I2).
    //! Divergence recorded (S11.111): a file whose LAST block is not followed by
    //! a section header is loaded here and dropped by the old loader (which
    //! flushes only on a `[`); the shipped file ends with `[end]`, so both agree
    //! on it.
    void load(const std::string &path);

    //! Channel 2 (S2(c)): declare an anchor from a parameter hash - the SAME
    //! keys the file uses, which is what `camera action create name X type Y ...`
    //! already delivers (the args-passthrough grammar, anchor_manager.cpp:271).
    //! Returns false and says what is wrong (S2(f)) on a missing/unknown `type`,
    //! a missing target, an unloadable orbit, or a name that is already a body.
    bool add(stringHash_t params);

    //! Drop a declared anchor and destroy the body it owns. REFUSES to drop the
    //! current anchor (the old path's own rule, AnchorManager::removeAnchor:194).
    bool remove(const std::string &name);

    //! Attach the camera to a declared anchor. A name that is not a declared
    //! anchor but IS a body is treated as an implicit ATTACHED anchor - the old
    //! path creates one per body automatically, and the dual-path seam
    //! (SSystemFactory::syncCameraReference) has always resolved names that way.
    bool switchTo(const std::string &name, Camera &camera);

    //! `camera action follow_rotation name X value true|false` on the new path:
    //! sets the named anchor's follow-rotation state and applies it immediately
    //! when that anchor is the current one. NAME-SCOPED, unlike the old path,
    //! whose CoreLink drops the argument and toggles a manager-wide flag
    //! (coreLink.cpp:1124) - divergence recorded in S11.111, the command's own
    //! documented argument being honoured here.
    bool setFollowRotation(const std::string &name, bool value, Camera &camera);

    // ---- The scripted transitions (B4(iv), S11.141) ------------------------
    // Every one of these mirrors an old `AnchorManager` member reachable from a
    // shipped `camera action` command, and mirrors its REFUSALS too: the script
    // must get the same answer from both paths, so the seam's `oldOk || newOk`
    // degenerates to one answer (SSystemFactory).

    //! `camera action move_to target point x y z` with NO duration: put the
    //! place the camera stands on AT `posRoot` now (old setCurrentAnchorPos,
    //! anchor_manager.cpp:490 - which refuses only the on-a-body case, and in
    //! particular does NOT refuse while a travel is in flight).
    bool placeCurrentAt(const Vec3d &posRoot, Camera &camera, double jd);
    //! `camera action move_to target point x y z duration t`: travel the place
    //! to `posRoot` over `seconds` of SIMULATION time, on old's own speed curve
    //! (old moveTo(pos,time), :407). Refuses while already moving, on a body,
    //! and on a negative time; a zero time places instantly, as old does.
    bool travelToPoint(const Vec3d &posRoot, double seconds, Camera &camera, double jd);
    //! `camera action move_to target body body_name X duration t [altitude km]`:
    //! travel the place toward where body X WILL BE at arrival, stopping
    //! `altitude` km above its surface (5 radii when no altitude is given) -
    //! old moveTo(anchor,time,alt) (:459) and moveToBody (:487).
    bool travelToBody(const std::string &bodyName, double seconds, double altitudeKm,
                      Camera &camera, double jd);
    //! `camera action transition_to target point`: become a free place AT the
    //! observer's current position, without moving the observer (old
    //! transitionToPoint, :528 - which captures the observer position, then
    //! zeroes the altitude so the observer sits exactly on the new point).
    bool transitionToPoint(const std::string &name, Camera &camera, double jd);
    //! `camera action transition_to target body name X`: reference X while the
    //! observer STAYS WHERE IT IS (old transitionToBody, :549, finds the
    //! longitude/latitude that reproduce the observer's place by bisection and
    //! sets the altitude to the measured distance to the surface; here the same
    //! place is expressed directly, since the camera's pose is that triple).
    //! THE HEADING TAIL IS NOT MIRRORED - old ends with setHeading(-axisAngle)
    //! + changeHeading(0, 5s), a roll decision at a reference switch, which is
    //! D28's open question and is already answered the other way for this path
    //! by A38 ("the reference switch holds the WHOLE orientation"). S11.141
    //! carries the measurement of old's tail and the terms that fail.
    bool transitionToBody(const std::string &name, Camera &camera);

    //! Per-frame: retire a travel that has landed. Mirrors the old manager's
    //! own update-driven flag (anchor_manager.cpp:310), so "am I still moving"
    //! flips on the same tick on both paths.
    void update(double jd);
    //! True while a travel is in flight - the state the refusals above and
    //! switchTo() read (old AnchorManager::moving).
    inline bool isMoving() const {
        return moving;
    }

    //! Name of the anchor the camera was last switched to through this class
    //! ("" if none). Not a poll of the camera: the camera can leave an anchor by
    //! other means (set home_planet, free-flight escalation), and this reports
    //! the last DECLARED attachment, which is what the anchor surface owns.
    inline const std::string &getCurrent() const {
        return currentName;
    }

    //! Harness observable (S11.111): current anchor + kind + follow-rotation
    //! state + the declared set, as one JSON object. The channel-parity check
    //! (authored vs commanded must produce the same anchor) compares this
    //! together with the anchor body's own dumpTrace.
    void dumpState(std::ostream &out) const;

private:
    struct Anchor {
        std::string name;
        AnchorKind kind;
        stringHash_t params;      //!< the declaration - the re-creation authority
        ModularBodyPtr body;      //!< owned anchor body, or the ATTACHED target
        bool ownsBody = false;
        bool declaresFollowRotation = false;
        bool followRotation = true;
    };
    //! Resolve `type` to a kind; false (+ S2(f) log naming the valid values) if
    //! absent or unknown.
    static bool parseKind(const stringHash_t &params, const std::string &name, AnchorKind &out);
    //! Build (or rebuild) the body an anchor points at. ON_ORBIT/FIXED_POINT
    //! create an owned hidden body; ATTACHED resolves `body_name`. False + log
    //! when the declaration cannot produce one.
    bool buildBody(Anchor &anchor);
    //! The body is valid iff the name registry still answers with THIS pointer -
    //! a destroyed body's remnant pointers are redirected to its parent
    //! (ModularBodyPtr::redirect), so "not null" is not "alive". Rebuilds an
    //! owned body that a reload destroyed.
    bool ensureBody(Anchor &anchor);
    //! Create one hidden anchor body under `parent` with the given orbit params.
    ModularBody *createAnchorBody(const std::string &name, ModularBody *parent, stringHash_t &orbitParams);
    Anchor *find(const std::string &name);
    //! The PLACE the camera is standing on, or null - the new-path spelling of
    //! old's `typeid(*currentAnchor) != typeid(AnchorPointBody)` test. A place
    //! is an anchor that OWNS its body (kinds 1 and 3); an ATTACHED anchor and a
    //! bare body reference are bodies, and a body cannot be travelled. Asked of
    //! the anchor layer rather than of the body's type, because "is this a
    //! place" is this layer's own fact (I4).
    Anchor *currentPlace(const Camera &camera);
    //! Give `place` the travel as its motion law and remember when it lands.
    //! `targetRoot` is in ROOT coordinates; the law is evaluated in the body's
    //! PARENT frame, so the parent's own (cached, already-evaluated) position is
    //! subtracted at each date - no orbit is re-evaluated off-cadence and no
    //! Newton seed is disturbed (S11.117).
    bool installTravel(Anchor &place, const Vec3d &targetRoot, double travelDays, double jd);

    ModularSystem *root;
    std::vector<Anchor> anchors;
    std::string currentName;
    //! Travel state - the exact pair the old manager keeps (`moving` +
    //! `arrivalTime`); the trajectory itself lives in the anchor body's orbit.
    bool moving = false;
    double travelArrival = 0;
};

#endif /* end of include guard: CAMERA_ANCHORS_HPP_ */
