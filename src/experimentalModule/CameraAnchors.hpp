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
// CameraAnchors - the NEW path's named-anchor layer (B4, §12 row 19, S7).
//
// WHAT IT IS FOR (I1): a show declares WHERE the camera is attached by NAME -
// "put me on the point that orbits the Moon", "put me on Mars keeping my
// angle" - through two channels that must mean the same thing (§2(c)):
// the authored `anchor.ini` and the runtime `camera action create/switch/drop/
// follow_rotation` commands. This class owns that vocabulary; the Camera below
// it owns only reference/pose/modes and knows nothing about names.
//
// THE THREE KINDS [R3, tester 2026-07-22, §11.70(b)] - each with the anchor.ini
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
//      following the rotation" (see §11.111). `follow_rotation = true` re-binds
//      to the surface (today's default for a body reference). The key ABSENT is
//      INACTION: the camera's current bind state is left alone, so every
//      pre-existing scene is bit-unchanged and there is no acting default to
//      log (D12; the D18 "inaction is no rotation" precedent).
//      D28 DEPENDENCY: what "keeping the angle" does to the image ROLL across
//      the reference change the switch performs is D28's open question, not this
//      class's - the switch uses Camera::warpToBody, so the kind inherits
//      TODAY's answer (B13 §11.61: the whole orientation is held) and follows D28
//      wherever it lands, with no anchor-side re-statement to keep in sync.
//
//  (3) FIXED_POINT (`type = point` | `type = observatory`) - a fixed point in AU.
//      R3: "only useful in the Universe mode... nothing moves there", so its
//      coordinates are read in the ROOT (Universe) frame - the one frame in
//      which a fixed point stays fixed - and the anchor owns an invisible body
//      parked there. Switching to it therefore PUTS the camera in the universe
//      frame rather than refusing outside it: on the new path the reference
//      chain IS the mode (§6.9), so "valid only in Universe mode" is realized by
//      the anchor living at the universe root, not by a mode test. The choice is
//      logged at the switch (D12) so a show that lands there sees why.
//      `observatory` folds into the same kind: it differs from `point` only by
//      its local-frame parametrization (anchor_point_observatory.cpp:32-40
//      zrot(lon)yrot(90-lat)), and the new-path camera parametrizes EVERY
//      reference that way already (Camera::placementRotation) - the distinction
//      has no new-path counterpart, so both spellings load.
//
// OWNED ANCHOR BODIES are created HIDDEN: parent-owned, position-ticking (B19
// §11.54), outside every draw/pick walk by construction - an anchor is a place,
// never a thing to see or click. They carry radius/datum/ground 0 (altitude is
// measured from the point itself; a point is not landable) and BodyType::ANCHOR,
// which is what the dump reports so a harness can tell an anchor from a body.
//
// LIFETIME (I5): the registry holds ModularBodyPtr, and an owned body that a
// system reload destroyed is REBUILT from the declaration it was made from
// (the params map kept in each entry is the authority) - checked at every use
// through the name registry, never by dereferencing a stale pointer.
//
// NOT HERE: cross-session persistence [Q5: explicitly not needed]; the old
// path's scripted animated transitions (moveTo/transitionTo*) and
// saveCameraPosition, which stay the old AnchorManager's - the old path is
// unchanged by construction and keeps serving old-path scenes.
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

    //! Channel 1 (§2(c)): load an authored anchor file. Grammar = the shipped
    //! `anchor.ini`'s, unchanged: `[section]` lines separate anchors (the
    //! section NAME is not read - it never was, AnchorManager::load:226-258),
    //! `key = value` inside, `#` comments. Each anchor block is handed to add()
    //! verbatim, so the two channels share ONE declaration grammar (I2).
    //! Divergence recorded (§11.111): a file whose LAST block is not followed by
    //! a section header is loaded here and dropped by the old loader (which
    //! flushes only on a `[`); the shipped file ends with `[end]`, so both agree
    //! on it.
    void load(const std::string &path);

    //! Channel 2 (§2(c)): declare an anchor from a parameter hash - the SAME
    //! keys the file uses, which is what `camera action create name X type Y ...`
    //! already delivers (the args-passthrough grammar, anchor_manager.cpp:271).
    //! Returns false and says what is wrong (§2(f)) on a missing/unknown `type`,
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
    //! (coreLink.cpp:1124) - divergence recorded in §11.111, the command's own
    //! documented argument being honoured here.
    bool setFollowRotation(const std::string &name, bool value, Camera &camera);

    //! Name of the anchor the camera was last switched to through this class
    //! ("" if none). Not a poll of the camera: the camera can leave an anchor by
    //! other means (set home_planet, free-flight escalation), and this reports
    //! the last DECLARED attachment, which is what the anchor surface owns.
    inline const std::string &getCurrent() const {
        return currentName;
    }

    //! Harness observable (§11.111): current anchor + kind + follow-rotation
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
    //! Resolve `type` to a kind; false (+ §2(f) log naming the valid values) if
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

    ModularSystem *root;
    std::vector<Anchor> anchors;
    std::string currentName;
};

#endif /* end of include guard: CAMERA_ANCHORS_HPP_ */
