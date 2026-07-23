#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"

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
    bool reloadSystem() {
        if (systemFilename.empty())
            return false;
        clearChildren();
        if (composedFile)
            loadComposedSystem(systemFilename);
        else
            loadSystem(systemFilename);
        return true;
    }
    // Whether this system has a data file behind it (see reloadSystem).
    inline bool hasSystemFile() const {
        return !systemFilename.empty();
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
    // Load a body
    void loadBody(std::map<std::string, std::string> &param);
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
    // Internally used by ModularBody to inform the destruction of body in this system
    inline void removeBody(ModularBody *body) {
        auto ptr = sortedSystemBodies.data();
        while (*ptr != body)
            ++ptr;
        *ptr = nullptr;
        needCleanUp = true;
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
                            BodyModuleType type);
    // Apply some hardcoded content
    void applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param);
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    std::string systemFilename;
    // Which reader systemFilename belongs to (reloadSystem dispatch):
    // false = legacy loadSystem, true = composed loadComposedSystem.
    bool composedFile = false;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
