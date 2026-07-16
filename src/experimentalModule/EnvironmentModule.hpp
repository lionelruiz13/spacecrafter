#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"
#include "atmosphereModule/atmosphere_commun.hpp" // ATMOSPHERE_MODEL (3-line enum header)

class ModularBody;
class Renderer;

// ============================================================================
// Interface implemented 2026-07-16 (S8 wave, escalated to full members by
// Vixy mandate: "fully implement the EnvironmentModule on the new path, not
// to depend on it [the old orchestration] for parity - atmosphere, landscape,
// milkyway; parity on Earth and Mars, landed"). The 11.23 verification pass
// remains the requirement->carrier source; convergence points that were
// open below are now resolved by the parity mandate itself: where a rule was
// undecided, the old path's observable behavior IS the specification.
//
// Scope [vixy, 2026-07-11]: everything "outside" whose rendering depends on
// the camera's location relative to its body - milkyway (earth-centered 2D
// version vs 3D version), from-ground atmosphere, landscape, and so on.
// NOT the from-space atmosphere rim: that is a per-body ATMOSPHERE-slot
// BodyModule (decision 2026-07-11) - two features sharing one word.
//
// MIGRATION FORM (this wave): members are STATE PRODUCERS + DRAW ROUTERS
// wrapping the app-level engines (MilkyWay, Atmosphere, Landscape+Fog - the
// same objects the old path drives, made path-neutral by matrix/direction
// parameterized entry points). Sharing the engines is the s_font /
// BodyTesselation precedent: the layer being replaced is the ORCHESTRATION
// (BodyDecor gates, executor matrix supply, navigator directions), so the
// A/B comparison isolates exactly that layer; textures/faders/pipelines are
// path-neutral infrastructure and stay single-instance. The old executor
// orchestration can be deleted without touching the new path's rendering -
// that is the "does not depend on it" criterion. Aggregation lives in
// EnvironmentManager (owned by SSystemFactory during migration; it slides
// into the frame task at the S4 inversion).
//
// Selection: the camera's reference body selects the active set = the union
// of environment members along the chain reference -> isolated root
// (grounded members only while the camera is anchored on the body).
// enter()/leave() fire on chain-membership edges, computed by the manager as
// a per-frame chain diff (a reference switch fires edges only on the bodies
// that actually joined/left - shared ancestors see none). The
// ModularBody::enterEnvironment/leaveEnvironment hooks the camera calls
// remain the wiring points; the manager's diff is the single authority so
// endpoint hooks and deep warps produce identical edge semantics.
// The old BodyDecor thresholds (AtmosphereParams::limLandscape/limSup/
// limInf) become per-body data (ModularBody::envParams, parsed from the same
// ssystem.ini keys with the same defaults as protosystem.cpp:865-882).
//
// Replaces (management layer): BodyDecor entirely - its three gates
// (canDrawLandscape/canDrawBody/canDrawMeteor) and atmosphere-state flag
// become fields of the aggregated EnvironmentState below, seam-readable
// during migration (SSystemFactory::getEnvironmentState; core.cpp consumers:
// searchAround gate, meteors gate, save/load of the atmosphere flag).
// ============================================================================

// Per-body environment data - the old AtmosphereParams fields that drive the
// attachment/activity edges (same keys, same defaults: protosystem.cpp).
// Units: meters of altitude (old observatory->getAltitude convention).
struct BodyEnvironmentParams {
    float limInf = 40000.f;       // below: atmosphere drawn (when user flag on)
    float limSup = 80000.f;       // below: inside the atmosphere zone
    float limLandscape = 10000.f; // below: grounded attachment (landscape)
    bool hasAtmosphere = false;
    ATMOSPHERE_MODEL model = ATMOSPHERE_MODEL::NONE_MODEL;
};

// Aggregated per-frame output of the ACTIVE environment set - the carrier of
// every cross-cutting requirement found in the old flow (INTENT.md 11.23):
// - worldAdaptationLuminance -> ToneReproductor BEFORE anything draws (old:
//   tone_converter->setWorldAdaptationLuminance(atmosphere->...) in update);
// - skyBrightness -> landscape shading, nebula dimming, meteor gating, star
//   refraction flag (the old core->sky_brightness shared state, including
//   the eclipse-dimming term from the atmosphere intensity);
// - the BodyDecor gates + atmosphere activity (iris/standard milkyway
//   texture selection reads atmosphereActive - the old useIrisTexture calls).
// Produced by members during update(), aggregated by the manager (fixed
// combination rules: drawBody = !drawLandscape), then BROADCAST back (the
// atmosphereUserFlag input; the milkyway iris adaptation) and exposed at the
// SSystemFactory seam. Both directions matter: members produce it AND adapt
// to the aggregate.
struct EnvironmentState {
    float worldAdaptationLuminance = 3.75f; // old no-atmosphere baseline
    float skyBrightness = 0;
    float atmosphereIntensity = 0; // fade-weighted; eclipse dimming included
    // INPUT (broadcast): the user's atmosphere intent - old
    // BodyDecor::atmState, mirrored from the same three write sites
    // (config load, setLandscapeToBody auto-rule, atmosphereSetFlag command).
    bool atmosphereUserFlag = false;
    bool insideAtmosphere = false; // below limSup with hasAtmosphere
    // Atmosphere actually drawn: insideAtmosphere && userFlag && below
    // limInf (the exact BodyDecor::bodyAssign branch that calls
    // setFlagShow(true) + useIrisTexture(false)).
    bool atmosphereActive = false;
    bool drawLandscape = false;    // below limLandscape (grounded attachment)
    bool drawBody = true;          // !drawLandscape (old bodyAssign coupling)
    bool allowMeteors = false;     // insideAtmosphere && userFlag (the
                                   // sky_brightness < 0.1 half stays at the
                                   // meteor consumer, as in the old executor)
};

class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    // Camera entered the space this environment covers (chain-membership
    // edge, fired by the EnvironmentManager diff). Activation edge: raise
    // resource priorities here - never load synchronously (C3/C4). Migration
    // form: engines are app-lifetime resident, so these are structural no-ops
    // carrying the edge semantics for the resource system (D6 clients:
    // landscape textures, 3D-galaxy volume).
    virtual void enter(ModularBody *body) {}
    // Camera left. Deactivation edge: lower priorities; the lowest LoD stays
    // resident (D6) so re-entry always has something drawable.
    virtual void leave(ModularBody *body) {}
    // Per-frame while active; cameraLocalPos = camera position in the body's
    // local frame, in AU. MIGRATION CONVENTION: the manager passes
    // (0, 0, distance) - only the magnitude is meaningful (altitude tests);
    // the true direction becomes available when per-body placement data
    // migrates (post-parity). deltaTime in seconds (fader/animation clock -
    // engine faders are auto-ticking or executor-ticked during migration).
    // Contributes this member's terms into `state`; aggregation and engine
    // driving happen in the manager. Return true when updates are no longer
    // required until the next enter().
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                        float deltaTime, EnvironmentState &state) {
        return true;
    }
    // Backdrop pass - drawn BEFORE all old-path sky content and all bodies
    // (ex: milkyway 2D iris/standard + zodiacal light), in the multisample
    // pass. `mat` is the owner's chain frame -> eye matrix; for owners above
    // the current system (isolated roots, whose mat dispatchUpdate does not
    // maintain), the manager substitutes the CURRENT SYSTEM's mat - correct
    // for orientation by the flat-chain contract (all root-aligned frames
    // share one orientation; only translations differ, irrelevant for
    // backdrop spheres).
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Sky/near pass - drawn AFTER bodies (ex: atmosphere-from-ground,
    // landscape, fog). PASS NOTE (corrected at implementation, 2026-07-16):
    // the landscape+fog engines record in PASS_FOREGROUND (single-sample, no
    // depth) but the atmosphere engine's pipeline is PASS_MULTISAMPLE_DEPTH
    // (additive blend over the multisample content, executed after the
    // bodies by toExecute order) - the 11.23 note claiming both were
    // FOREGROUND was wrong on the atmosphere half (atmosphere.cpp:110/127).
    // Frame-sequencing stays a Renderer/frame-task input; during migration
    // the executor's call position carries it (same position as the old
    // atmosphere->draw/landscape->draw block).
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

// Remaining convergence points (kept from the 11.23 draft, none blocking):
// - Landscape association data (per-body landscape name; the old
//   satellite->moon / sun->sun rules stay in Core::setLandscapeToBody during
//   migration - Core still owns WHICH landscape is current, the member owns
//   drawing it on the new path) and slot-replacement semantics for
//   setLandscape (migration: engine pointer re-seated through the
//   SSystemFactory seam at every swap, I5).
// - Zodiacal-light toggling (old per-executor-mode enableZodiacal) stays on
//   the shared engine (executor onEnter/onExit writes it; path-neutral).
// - The 3D-galaxy model placement (old hardcoded setModel matrices) becoming
//   the galaxy body's chain transform (G2) - post-parity, and with it the
//   milkyway 2D/3D regime switch (inGalaxy/inUniverse executors keep the old
//   drawing until then - this wave covers the solar/stellar executors).
// ============================================================================

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
