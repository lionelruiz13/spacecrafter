#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"
#include "ModularSystemFormat.hpp"

// Shadow orchestration (LIVE, S5/G7 - design: shadow-paths.md B2): the
// system level decides WHICH bodies shadow which - per-module hooks
// (drawShadow/drawSelfShadow) only say HOW. Successor of the old
// SolarSystemDisplay::computePreDraw shadowingBody ranking + bindShadows
// plumbing, generalized past the center-of-interest restriction: EVERY drawn
// body is a receiver candidate; casters are bodies whose modules declare
// BMT_PROJECT_* traits. Selection = the ported light-cylinder test + the
// peak-occlusion >= 1/16 gate [vixy: 2026-07-12] (the old "4%" comment's
// exact form); ranking by occlusion, so pool exhaustion drops the least
// significant shadows. MINOR_BODY bodies never participate (D3 border case).
// Self-shadow bucket selection (MAIN vs SECONDARY resolution) remains
// second-pass: no self-shadow client exists before the OJM/terrain ports
// (shadow-paths.md C3).
class ModularSystem : public ModularBody {
public:
    ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info);
    // Children must be destroyed while THIS class's members still exist:
    // every content body's dtor deregisters from sortedSystemBodies - the
    // implicit order (members die before the base dtor's clearChildren)
    // made that a use-after-free on every system teardown (INTENT 5.23).
    ~ModularSystem() override {
        clearChildren();
    }

    // Re-read this system's content from the data file it was loaded from:
    // every content body is destroyed and rebuilt from the file, so the FILE
    // is the authority (bodies deleted from it disappear, edited values take
    // effect). Recreated bodies are positioned at the CURRENT simulated date,
    // not at launch (the ctor seeds lastJD from the parent, and this node is
    // on the camera's chain) - a reload is not a restart.
    // Observation state (camera pose/reference, tracked body, selection) is
    // NOT this node's concern: identity across the rebuild is by NAME, and
    // the holder of each reference re-seats it - SSystemFactory::
    // reloadCurrentSystem is that authority for the camera-side references.
    // Returns false when this node has no source file (systems created empty:
    // the universe/milkyway spine nodes, and the anchor-only systems built by
    // createSystem) - there is nothing to reload FROM, and clearing the
    // children would destroy the spine instead.
    // The rebuild waits for the frames in flight before destroying anything
    // (Context::quiesceFrames): what it destroys is still referenced by the
    // drawing thread's recording and by the GPU (INTENT 5.58). Defined in the
    // .cpp for that reason - the wait is not something a header may declare
    // away, and this is a commanded path, not an inlined one.
    bool reloadSystem();
    // Whether this system has a data file behind it (see reloadSystem).
    inline bool hasSystemFile() const {
        return !systemFilename.empty();
    }
    // The file this system was loaded from, and WHICH READER it belongs to.
    // Together they are the session manifest's per-system record (b31-design
    // §3.3): a session says what it assumed was loaded, so that the file stays
    // readable on an install that has something else loaded — which is what
    // D32's diagnostic-artifact reading demands of it.
    inline const std::string &getSystemFilename() const {
        return systemFilename;
    }
    inline bool isComposedFile() const {
        return composedFile;
    }
    // Load a system
    void loadSystem(const std::string &filename);
    // B24 composed-system format (INTENT §11.78(d); the `type=` respell is
    // Vixy-signed-off, D16 §11.79(j)). Same capability authorities as the
    // legacy path (loadBody / ModuleLoaderMgr::loadModule) behind a different,
    // thin parser (ModularSystemFormat) - a section is a DECLARATION carried by
    // ONE key, `type=`:
    //   type = <family> (a value in ModuleLoaderMgr's family vocabulary:
    //     CUSTOM/MESH/OJM/... - the code's own enum, I2) -> an explicit MODULE:
    //     body= names the target node (declared EARLIER in the file - the
    //     findBody forward-reference rule, same as parent=), optional slot=
    //     (multi-instance, the GRID precedent), optional relation= re-routes
    //     (far|near|grounded|in|orbit|trail|tail via ModuleLoader::reroute);
    //     every other key OVERLAYS the node's params for this one load (module
    //     key wins - per-module customization; params stay homed on the node
    //     because loaders read body params, the capability model's shape).
    //   type = anything else (the `BODY` sentinel, a legacy Planet/Moon/Sun
    //     body-type carried transitionally on the node until B27/B25-emit
    //     materializes capability keys, or absent) -> a NODE: loadBody, which
    //     reads `type=` as the body-type exactly as the legacy loader does and
    //     honors relation = orbiting|grounded|inner (supersedes the legacy
    //     bound_to_surface alias; the only data route to INNER) and
    //     compose = deduced|explicit (explicit -> module list comes ONLY from
    //     the BodyModule declarations, deduction off).
    // The two `type=` roles never collide: a module names its node with body=,
    // a node never does (that binding, not a value guess, disambiguates a
    // mistyped family, §2(f)); the composed vs legacy `type=` namespaces stay
    // apart because a different loader reads each file (D16 §11.79(j)).
    // Sets systemFilename + composedFile, so reloadSystem() re-reads THIS
    // file (a composed system reloads like a legacy one, B16 parity).
    void loadComposedSystem(const std::string &filename);
    // Generate the machine-owned `.ini.disabled` twin of a legacy system file
    // (the B25 generation half, INTENT §11.51(a)): parse `legacyFilename`
    // (section order preserved - parent-before-child is load-bearing),
    // re-express every LOADED body as an explicit composition
    // (compose = explicit + one BodyModule declaration per family
    // deduceBodyModuleList selects on the live body; bound_to_surface
    // translated to relation=grounded, one authority per generated file) and
    // write through the one atomic writer (ModularSystemFormat::write).
    // Must run AFTER the legacy load - deduction queries live body state.
    // The semantic-equivalence promise [vixy, §11.50(b)]: loading the twin
    // must reproduce the legacy load exactly; that makes generation a
    // corpus-wide coverage test of the composition grammar
    // (harness/b24_equivalence.py). applyHardcodedContent capability keys are
    // NOT emitted yet (B27 step 3 / B25-emit - a separate task; spellings are
    // ratified, D10key §11.79(e)). Until it lands, a node's legacy body-type
    // stays under `type=` here (a non-family value = a valid node declaration).
    void generateComposedTwin(const std::string &legacyFilename, const std::string &outPath);
    // Write THIS system to a composed file (B31 slice 2, b31-design §4.1; the
    // route is Vixy's own [§11.51(a)]: "save a system on-the-fly as well by
    // targeting without the .disabled or under a different name from scripts").
    // What it is FOR: a body a script pushed into the live tree exists only in
    // memory, and this is what turns it into ordinary authored data - from the
    // next launch it is loaded by the ordinary loader and its identity is what
    // every authored body's identity already is (no new identity key, §4.1).
    //
    // A FILE THAT ALREADY EXISTS IS EDITED, NEVER REBUILT: its own content is
    // the base (the sections this system was loaded from when it targets its own
    // file - annotations included - otherwise a parse of the target), so
    // comments, layout, malformed lines and keys this engine does not
    // understand come back untouched (§11.66(b), the F13 layer). What this slice
    // ADDS to such a file is exactly what is missing from it: a declaration for
    // every live body the file does not declare, and the annotations the loader
    // produced about the data it read. What it deliberately does NOT do is edit
    // a declaration the file already carries - a value an operator changed at
    // runtime is the session ledger's (b31-design §2 group D), a later slice, and
    // silently rewriting an author's line here would pre-empt that decision.
    // A target that does not exist is built whole from the tree, like the twin.
    //
    // WHICH BODIES: this system's own subtree, hidden bodies included (a hidden
    // body is declared data - [Goldilocks_Zone] ships hidden = true), stopping at
    // a nested system node (its content belongs to that system's file), and only
    // bodies that carry a declaration (ModularBody::declaredParams) - an
    // engine-minted body (a camera anchor, the B5 pilot oort) has nothing
    // declared to write and must not become authored content.
    //
    // WHICH FILES may be written is NOT this level's decision and this level
    // cannot enforce it: the legacy ssystem.ini is READ-ONLY forever (D35,
    // §2.0 D13). The path convention and that enforcement live at the
    // SSystemFactory seam that owns them (saveCurrentSystem).
    // Returns false when the file could not be written (the writer left any
    // previous content untouched and said why).
    bool saveSystem(const std::string &outPath);
    // Load a body. `origin` is the section it was declared by, when that section
    // belongs to a file this engine may write (a composed file): the loader
    // annotates it in place with what it diagnosed (b31-design §5.3). Null for a
    // legacy file (READ-ONLY forever, D35) and for a script's parameter map -
    // there is no datum in a writable file to annotate, and the log line is then
    // the whole diagnostic channel.
    //! `supplemental` says the caller is the RUNTIME push route (`body action
    //! load` -> SSystemFactory::addBody), not a file load - the provenance bit
    //! `body action clear` selects on (ModularBody::supplemental, B34
    //! §11.108(f); old carries the same bit as ProtoSystem::addBody's
    //! `deletable` argument, false for file bodies). It is a parameter and not
    //! a post-hoc mark by the caller because only THIS function knows whether
    //! this call created the body: a load refused for a duplicate name would
    //! otherwise re-brand the body that is already there.
    void loadBody(std::map<std::string, std::string> &param,
                  ModularSystemFormat::Section *origin = nullptr,
                  bool supplemental = false);
    //! Drop every RUNTIME-PUSHED body of this system's own content, hidden ones
    //! included, and return true if anything was removed (old's mirror:
    //! ProtoSystem::removeSupplementalBodies over `isDeleteable`, which walks
    //! `systemBodies` and therefore reaches hidden bodies too - verified at
    //! source, not assumed). The DECISION whether a clear may run at all is the
    //! old path's and is taken at the seam (SSystemFactory), so this function is
    //! the mechanism alone.
    //! Content only: the walk stops at a nested system node, and a body with no
    //! declaration (camera anchor, engine-minted node) is never supplemental, so
    //! neither can be taken - see ModularBody::supplemental.
    bool removeSupplementalBodies();
    //! Fresh-restart every trail of this system's own content (old's mirror:
    //! ProtoSystem::startTrails over `systemBodies`; same per-system scope, same
    //! hidden-inclusive reach). `record` is the caller's own value - both live
    //! callers pass the current global trail flag (config init, setHomePlanet).
    void startTrails(bool record);
    // Update this system
    void updateSystem();
    // Draw this system - the FRAME entry (shadow orchestration, body-draw
    // begin/end, selection pointer). Nested systems inside the loop dispatch
    // through drawNested, never through this.
    void drawSystem(Renderer &renderer);
    // The sorted body loop alone (no begin/end, no pointer) - shared by the
    // frame entry and nested draws.
    void drawSystemBodies(Renderer &renderer);
    // The orbit pass (row 8): after the body draw, cut the trace holes then
    // draw the orbit lines depth-tested against them, under the orbit-union
    // depth range (old solarsystem_display.cpp orbit phase). Two sweeps of the
    // sorted list (all traces, then all lines - a nearer body's disc must hide
    // a farther body's orbit, so every trace precedes every line).
    void drawOrbits(Renderer &renderer);
    // The trail pass (row 9): after the body draw, sweep every EVALUATED body's
    // trail module - accumulate its current position at sim time (ticks even
    // while the body is off-screen: the row-9 invisible-tick contract) and draw
    // the fading polyline (COLOR, no depth). System-driven, not a regime list,
    // for the same reason as drawOrbits (accumulation must run every frame). One
    // command buffer (beginTrailDraw), gated on TrailModule::anyActive() (an
    // always-run sweep hangs scene E - the mandatory ORBIT-precedent gate).
    void drawTrails(Renderer &renderer);
    // The tail pass (row 12): after the body draw, sweep every EVALUATED body's
    // TAIL module - update() (coma/tail size + parent-frame expansion, JD-cached)
    // then draw() (submit the eye-space instance to the Renderer batch), and
    // flush the whole batch with ONE instanced draw (COLOR, no depth). Renderer-
    // owned batch (the old Tail::global singleton dissolved). System-driven like
    // drawTrails, gated on TailModule::anyActive() (the mandatory ORBIT-precedent
    // gate) so a comet-free system pays nothing.
    void drawTails(Renderer &renderer);
    // Draw THIS system as an entry of an enclosing system's loop (camera
    // outside): >= SYSTEM_VISIBILITY_SUBSYSTEM_SIZE px on screen -> nested
    // content draw at this node's sort position (correct by D1/D2: subtree
    // extent << inter-system distance; light state saved/restored around it -
    // lightPosition scope note in ModularBody.hpp). Below -> star-halo proxy.
    // Nested shadows deliberately absent: they engage when the camera enters
    // the system (it becomes current); a foreign system's casters at these
    // distances are sub-pixel (INTENT 11.36).
    void drawNested(Renderer &renderer);
    // The far-system point visual (D3's halo-only common case at system
    // scale): the system IS its star visually - star photometry (magnitude
    // at the NODE's fresh distance, star disc floor, star halo color) through
    // the shared drawHaloCore. Starless or halo-suppressed star: nothing.
    void drawStarProxy(Renderer &renderer);
    // Internally used by ModularBody to inform the creation of body in this system
    inline void addBody(ModularBody *body) {
        sortedSystemBodies.push_back(body);
        needCleanUp = true;
    }
    // Internally used by ModularBody to inform the destruction of body in this
    // system. BOUNDED since B39 (§11.117), and absence is now a LEGAL state, not
    // a broken invariant: hide() takes the parked subtree OUT of this list
    // (unregisterBody below), so a body destroyed while hidden - an anchor body
    // dropped by `camera action drop`, a hidden body's `body action reload` -
    // is legitimately not here. The previous unbounded `while (*ptr != body)`
    // ran off the end of the vector in exactly that case.
    inline void removeBody(ModularBody *body) {
        auto ptr = sortedSystemBodies.data();
        auto const end = ptr + sortedSystemBodies.size();
        while (ptr != end) {
            if (*ptr == body) {
                *ptr = nullptr; // compacted by the next cleanUp()
                needCleanUp = true;
                return;
            }
            ++ptr;
        }
    }
    // Take a body OUT of the rendered/pickable universe without destroying it
    // (hide()). Distinct from removeBody deliberately: this one ERASES rather
    // than nulling, because a null entry is dereferenced unguarded by three of
    // the draw sweeps (they rely on cleanUp() running between the edit and the
    // next draw, which holds for destruction inside a load but is a needless
    // new exposure for an operator command). Erase preserves the sort order, so
    // the near-sorted bubble sort keeps its O(N) case. No-op if absent (double
    // hide, or a body born under a parked node and never registered).
    inline void unregisterBody(ModularBody *body) {
        for (auto it = sortedSystemBodies.begin(); it != sortedSystemBodies.end(); ++it) {
            if (*it == body) {
                sortedSystemBodies.erase(it);
                return;
            }
        }
    }
    inline std::vector<ModularBody *>::const_iterator begin() const {
        return sortedSystemBodies.begin();
    }
    inline std::vector<ModularBody *>::const_iterator end() const {
        return sortedSystemBodies.end();
    }
    // Find the body at the given normalized screen position (in range [-1, 1])
    ModularBody *findBodyAt(const std::pair<float, float> &screenPos) const;
    // Return the star of this system, nullptr while unassigned. The member
    // sentinel for "unassigned" is star == this (loadBody note: a valid
    // light-position default) - that sentinel must never leak to callers as
    // a fake star (galaxy/universe systems are legitimately starless; the
    // sentinel leaked the SYSTEM NODE itself as light source/sky input).
    inline ModularBody *getSystemStar() const {
        return (star == this) ? nullptr : static_cast<ModularBody *>(star);
    }
    // Find the system in which the given body is
    static inline ModularSystem *systemOf(ModularBody *body) {
        while (body->isNotIsolated)
            body = body->parent;
        return static_cast<ModularSystem *>(body);
    }
private:
    // Shadow orchestration (see class comment): fills each drawn receiver's
    // receivedShadows and drives ShadowService production. Runs at drawSystem
    // start - positions updated, renderer frame begun (frameIdx known).
    void computeShadows(Renderer &renderer);
    // One BodyModule declaration of the composed format (loadComposedSystem's
    // module half). `nodeParams` = this file's node sections by body name,
    // the overlay base. `type` = the module family, already resolved from the
    // section's `type=` value by the caller (D16 §11.79(j): the one `type=` key
    // is BOTH the node/module selector AND the family name).
    void loadDeclaredModule(std::map<std::string, std::string> &params, const std::string &header,
                            const std::map<std::string, std::map<std::string, std::string>> &nodeParams,
                            BodyModuleType type, ModularSystemFormat::Section *origin);
    // --- ONE composed-format emitter, two callers (I2) -------------------
    // The node section's parameters for a live body: what the data declared,
    // plus the capability keys the body CARRIES that a composed load would not
    // otherwise reproduce (the legacy `type` string grants them; the composed
    // format does not - D14). Reads the body, not its name (I4).
    static stringHash_t composedNodeParams(const ModularBody *body, const stringHash_t &declared);
    // The whole machine-built declaration of one live body: its node section
    // (compose = explicit) followed by one BodyModule declaration per module.
    // The two go together and are ONE decision: `compose = explicit` turns
    // deduction off, so a file that carries it must carry the declarations too.
    static void appendWholeDeclaration(ModularBody *body, const stringHash_t &declared,
                                       std::vector<ModularSystemFormat::Section> &out);
    // This system's own content, parents first (the findBody forward-reference
    // rule is the format's ordering requirement) - see saveSystem for the
    // membership rules.
    static void collectContentBodies(ModularBody *node, std::vector<ModularBody *> &out);
    // Apply some hardcoded content
    void applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param);
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    std::string systemFilename;
    // The composed file this system was loaded from, AS PARSED - every line of
    // it, in order (the F13 layer's whole-file record), kept so that a save can
    // give the file back whole instead of rebuilding it, and so that the
    // annotations the loader produced while reading it have something to travel
    // on until an explicit save writes them (b31-design §5.3; a load NEVER
    // rewrites the file - D33 decided against it, §11.113(l)).
    // EMPTY after a legacy load, deliberately: the legacy file's layout is not a
    // write base, because that file is READ-ONLY forever (D35) and its twin is
    // machine-owned and built whole.
    std::vector<ModularSystemFormat::Section> loadedSections;
    // Which reader systemFilename belongs to (reloadSystem dispatch):
    // false = legacy loadSystem, true = composed loadComposedSystem.
    bool composedFile = false;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
