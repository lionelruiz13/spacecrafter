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

    // Reload a system
    void reloadSystem() {
        clearChildren();
        loadSystem(systemFilename);
    }
    // Load a system
    void loadSystem(const std::string &filename);
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
    // Apply some hardcoded content
    void applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param);
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    std::string systemFilename;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
