#ifndef SESSION_FILE_HPP_
#define SESSION_FILE_HPP_

#include <string>

// B31 slice 3 - THE SESSION FILE (b31-design §3.2/§3.3/§3.5).
//
// WHAT IT IS. A bookmark, not data. `~/.spacecrafter/sessions/<name>.ini` is
// MACHINE-OWNED and DISPOSABLE: nothing in it is authored, nothing in it is
// shipped, and a version later it means nothing. That is exactly why it is a
// SECOND artifact and not a section added to the content files: content is the
// product (D9, frozen field, hand-edited, replaced by a paid delivery), and
// making every session save rewrite paid data would open a destruction surface
// at the highest-frequency operation there is. The content half already has its
// own channel - `body action save`, §11.121 - and a session REFERENCES the
// files it needs by path instead of copying what is in them (§9(2): a session
// file must never become a data delivery, which is what lets a restore see a
// correction that landed in the data underneath it).
//
// WHAT IT IS FOR [vixy, D33 §11.113(l), verbatim]: "user want control and
// especially the ability to recover to an established clean state, most often
// used between sessions each time with new public (thus no continuity exists
// for the public)". So it is a PRESET, loaded repeatedly, not a resume point:
//   * EXPLICIT ONLY - nothing is saved at quit, nothing is loaded at start,
//     and there is no config key for a policy. No acting default exists here to
//     log (D12 satisfied by construction).
//   * LOAD IS IDEMPOTENT - loading the same file twice lands on the same state,
//     and so does loading it from the state it produced. Everything below is
//     written as an ASSIGNMENT for that reason; there is no relative term.
//
// HOW DEEP "as-if" CUTS [D32 §11.113(k)]: transients SNAP TO THEIR SETTLED
// TARGET. A move, a zoom, a view smoothing plan in flight is saved as the state
// it was heading for - a still frame reproduces, a mid-ramp state does not, and
// the tracker re-plans every frame so a mid-plan save has no fixed point at all
// (§11.55(d), and T4 below demands one). The carve-outs D32 names are the
// persistent CONDITIONS rather than the motions: the view-offset ARMED latch is
// saved as the latch it is (its arming ramp snaps), and the screen fader's
// value is a condition too. Accumulated trail points are the third carve-out
// and belong with the per-body ledger.
//
// AND WHAT A RUNNING SHOW CONTRIBUTES [D36 §11.113(o)]: declarative state IN,
// time-bearing state OUT. A script's position in its queue, its wait timer and
// a video's playback offset are never written: the engine does not take a
// position inside somebody's authored script (§2(b)).
//
// THE MANIFEST (§3.3) is the file's other half: which system files this session
// needs, and which reader each belongs to. It is what makes the file portable -
// D32 made the artifact a DIAGNOSTIC one (a user attaches it to a report), and
// a file that has to be readable on another install must say what it assumed.
// It is also, by construction, the same record an in-session unload/reload of a
// stellar system needs (O3), which is why the two are one representation.
//
// WHAT THIS SLICE DOES NOT CARRY, and why - each is a boundary, not an
// omission, and each is written INTO the file it is absent from so that a later
// slice finds a note instead of an archaeology problem:
//   * `heading` (§2 row B6): what it MEANS across a reference change is D28's
//     open question. Saving a number whose semantics is pending would bake the
//     pending answer into a file.
//   * the per-body override ledger (§2 group D) and `planet_scale`: the next
//     slice. It needs an identity key (D34: plain englishName) and a miss
//     report, which are its own obligations.
//   * the bulk value rows (§2 E3/E4/E5 - 97 flags, 43 `set` values, 46
//     colours): the READ half does not exist as an authority. Today the only
//     code in the tree that knows a flag's current value is
//     AppCommandInterface::setFlag's FV_TOGGLE branch, and it knows it only
//     while mutating it; a save cannot use that, and writing a second 97-case
//     switch beside it is the duplication I2 exists to forbid. The correct
//     shape - extract the read into one `readFlag`, and let the toggle branch
//     consume it - is a mechanical refactor of the old command surface with its
//     own regression surface, and it is the natural companion of the ledger
//     slice, which needs a per-body read half of exactly the same kind.
//   * media, script-engine and runtime-catalogue rows (§2 groups G6/H/I): each
//     classified by D36 but none has a readback surface either.
//
// THE SAVE IS A D8 USE-SITE (§6.1): every body it names is brought to the
// current date before it is read (`useNow`, the §11.76 barrier with its 4 extra
// iterations), because a body the engine deliberately froze would otherwise be
// serialized at its freeze date and the error would become permanent.
namespace SessionFile {

// Everything a session needs from the application that is not the camera, the
// body tree or the system tree - stated as what it is FOR, so a caller never
// has to know how the session file is laid out and the session never has to
// know what a Core is (I1).
class Host {
public:
    virtual ~Host() = default;
    // Time (§2 group A). The simulation date and the rate it advances at are
    // the two most load-bearing scalars in the file.
    virtual double getJDay() const = 0;
    virtual void setJDay(double jd) = 0;
    virtual double getTimeSpeed() const = 0;
    virtual void setTimeSpeed(double speed) = 0;
    virtual bool getTimePaused() const = 0;
    virtual void setTimePaused(bool paused) = 0;
    // Selection and tracking (§2 rows C1/C5). Selection is re-established by
    // replaying the NAME through the one routing seam, never by holding a
    // pointer across a process boundary.
    virtual std::string getSelectedName() const = 0;
    virtual bool selectByName(const std::string &name) = 0;
    virtual void deselect() = 0;
    virtual bool getTracking() const = 0;
    virtual void setTracking(bool on) = 0;
    // The observer's reference body (§2 row B1). Restoring it is a WARP, not an
    // assignment: entering and leaving a body's environment are edges somebody
    // is subscribed to, and the warp is what publishes them.
    virtual bool warpToBody(const std::string &name) = 0;
    // The observer's PLACE (§2 row B3), through the seam that moves BOTH paths
    // while both exist. §2 row B19 excludes the old Observer/Navigator twin
    // from the file on the grounds that "setters are dual so it follows" - and
    // that only holds if the restore uses the dual SETTER instead of assigning
    // the camera's own members. It is not a detail: the star field, the milky
    // way and the nebulae are still drawn from the OLD observer, so a restore
    // that moves only the camera puts the right body in front of the wrong sky
    // (measured: every camera field identical and 111001 px>8 of dome still
    // differing, against an in-scene A/A floor of 0).
    // Degrees and METRES, as the seam takes them - and as `moveto` takes them,
    // which is the same thing.
    virtual void moveObserverTo(double latDeg, double lonDeg, double altMetres) = 0;
    // The field of view (§2 row B11) and the sky lock (§2 row B9), for the same
    // reason as the place above and it is the SAME lesson: both have a dual
    // seam, and the half of each that the camera does not own still draws.
    // The old projector's fov is what scales every star, so a restore that set
    // only the camera's left the right body inside a sky drawn at the launch
    // fov (measured: 441 lit pixels in the saved frame against 2871 in the
    // restored one - the same sky, drawn at 45 deg and at 180). Degrees, as
    // `zoom fov` takes them.
    virtual void setFov(double degrees) = 0;
    virtual void setSkyLock(bool locked) = 0;
};

// Write the session to `~/.spacecrafter/sessions/<name>.ini`, atomically,
// through the ONE serialization authority (ModularSystemFormat::write - the
// same writer the content files go through, I2). `name` is a plain file NAME:
// a path is refused, because a session is written where sessions live and
// nowhere else - and in particular never into the frozen legacy corpus (D35,
// §2.0 D13). Returns false and says why, in the log, on any refusal or any
// write failure; a failed write leaves any pre-existing file untouched.
bool save(Host &host, const std::string &name);

// Restore a session. Idempotent (D33): every value is assigned, so loading the
// same file twice - or loading it from the state it just produced - lands on
// the same state. A value that cannot be re-established (a body the data no
// longer declares) is REPORTED and the rest of the file still applies: a
// silently dropped restore is a show that looks wrong with no trace of why.
bool load(Host &host, const std::string &name);

// The default session name, used when the operator names none.
constexpr const char *DEFAULT_NAME = "session";
// The directory sessions live in, relative to the working directory the app
// runs in (~/.spacecrafter). Absent = "no session"; nothing bootstraps it, so
// there is no shipped default_session.ini to go stale (§9(6)).
constexpr const char *DIRECTORY = "sessions";
// The file format's own version. A restore of an unknown version REFUSES with
// a diagnostic rather than half-applying (§9(1)): silent partial restoration of
// a file from the future is the worst failure available here.
constexpr int FORMAT_VERSION = 1;

} // namespace SessionFile

#endif /* end of include guard: SESSION_FILE_HPP_ */
