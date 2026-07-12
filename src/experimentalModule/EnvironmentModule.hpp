#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"

class ModularBody;
class Renderer;

// ============================================================================
// DRAFT interface - pending convergence with Vixy (INTENT.md 3.8).
// Verification pass 2026-07-12 (INTENT.md 11.23): the three existing clients
// (milkyway, landscape, atmosphere-from-ground) + the BodyDecor management
// layer were decompressed requirement-by-requirement against this contract
// (predecessor-code rule); every addition below carries the requirement that
// forced it. Replacement is TRANSPARENT when every requirement has a carrier
// here or at a named seam - the remaining convergence points are listed at
// the end, none blocks the shape of this interface.
//
// Scope [vixy, 2026-07-11]: everything "outside" whose rendering depends on
// the camera's location relative to its body - milkyway (earth-centered 2D
// version vs 3D version), from-ground atmosphere, landscape, and so on.
// NOT the from-space atmosphere rim: that is a per-body ATMOSPHERE-slot
// BodyModule (decision 2026-07-11) - two features sharing one word.
//
// Selection: the camera's reference body selects the active set; grounded
// environments take priority over InAoI environments (ModularBody relation
// lists groundedEnvironment / environment). enter()/leave() fire on camera
// transitions (wired through ModularBody::enterEnvironment/leaveEnvironment,
// currently empty hooks) - they are the (de)activation edges for resource
// priorities. The old BodyDecor thresholds (AtmosphereParams::limLandscape,
// limSup - body.hpp) become the body data driving exactly these edges:
// grounded-attachment below limLandscape, atmosphere-active below limSup -
// the altitude tests BodyDecor::bodyAssign performed become structural
// (attachment state), not per-frame flag recomputation.
//
// Replaces (management layer): BodyDecor entirely - its three gates
// (canDrawLandscape/canDrawBody/canDrawMeteor) and atmosphere-state flag
// become fields of the aggregated EnvironmentState below, kept seam-readable
// during migration (core.cpp consumers: ssystemFactory->draw gate,
// searchAround gate, meteors gate, save/load of the atmosphere flag).
// ============================================================================

// Aggregated per-frame output of the ACTIVE environment set - the carrier of
// every cross-cutting requirement found in the old flow (INTENT.md 11.23):
// - worldAdaptationLuminance -> ToneReproductor BEFORE anything draws (old:
//   tone_converter->setWorldAdaptationLuminance(atmosphere->...) in update);
// - skyBrightness -> landscape shading, nebula dimming, meteor gating, star
//   refraction flag (the old core->sky_brightness shared state, including
//   the eclipse-dimming term from the atmosphere intensity);
// - the three BodyDecor gates + atmosphere activity (iris/standard milkyway
//   texture selection reads insideAtmosphere - the old useIrisTexture calls).
// Produced by members during update(), aggregated by the system (fixed
// combination rules, e.g. max luminance, min gates), then BROADCAST back to
// members and exposed at the SSystemFactory seam. Both directions matter:
// members produce it AND adapt to the aggregate (milkyway variant).
struct EnvironmentState {
    float worldAdaptationLuminance = 3.75f; // old no-atmosphere baseline
    float skyBrightness = 0;
    float atmosphereIntensity = 0; // fade-weighted; eclipse dimming included
    bool insideAtmosphere = false; // below limSup with hasAtmosphere
    bool drawLandscape = false;    // below limLandscape (grounded attachment)
    bool drawBody = true;          // !drawLandscape (old bodyAssign coupling)
    bool allowMeteors = false;     // insideAtmosphere && dark enough
};

class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    // Camera entered the space this environment covers. Activation edge:
    // raise resource priorities here - never load synchronously (C3/C4).
    // (Landscape textures and the 3D-galaxy volume are the big clients; the
    // lowest-LoD-resident rule D6 gives re-entry something drawable.)
    virtual void enter(ModularBody *body) {}
    // Camera left. Deactivation edge: lower priorities; the lowest LoD stays
    // resident (D6) so re-entry always has something drawable.
    virtual void leave(ModularBody *body) {}
    // Per-frame while active; cameraLocalPos = camera position in the body's
    // local frame (the quantity everything camera-location-relative needs);
    // deltaTime carries the fader/animation clock (old milkyway/atmosphere
    // faders, landscape landing animation - they cannot tick without it).
    // Contributes this member's terms into `state` (aggregation happens
    // outside, in the system - [vixy: aggregated by the system]).
    // HEAVY recomputation (the atmosphere sky-color table - it already runs
    // on its own thread in the old path, asyncUpdateLoop) does NOT belong
    // here: it is a work-domain task publishing its table swap through the
    // render chain (INTENT.md 8, C1-C4); update() only reads the published
    // state and emits the request when inputs changed.
    // Ephemeris inputs (old computeColor: jd, sun & moon local positions,
    // moon phase, latitude/altitude, per-body temperature/humidity) come
    // from the body/system, not from parameters: light direction is
    // ModularBody::getLightPosition-class static state, the "moon" term
    // generalizes to the reference body's satellites (the old
    // getMoon()/getEarth() hardcode dissolves), lat/alt from the Camera,
    // temperature/humidity from the body's atmosphere params.
    // Return true when updates are no longer required until the next enter().
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                        float deltaTime, EnvironmentState &state) {
        return true;
    }
    // Backdrop pass - drawn BEFORE all bodies (ex: milkyway 2D iris/standard,
    // milkyway 3D volume), in the multisample pass the bodies draw in.
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Sky/near pass - drawn AFTER bodies (ex: atmosphere-from-ground,
    // landscape, fog). PASS NOTE (verification-forced): in the old flow this
    // content records in PASS_FOREGROUND (single-sample, no depth), a
    // different render pass than the bodies - the per-category fixed order
    // [vixy] is therefore a FRAME-SEQUENCING input to the Renderer/frame
    // task (which pass, which segment), not a batch-flush rule (distinct
    // from the PipelineFamily occlusion contract).
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

// Remaining convergence points (deliberately NOT designed here) - none
// changes the hook shape above [INTENT.md 11.23 carries the full
// requirement->carrier table]:
// - EnvironmentState aggregation rules when several members contribute the
//   same term (max/min/sum per field).
// - The seam exposure of EnvironmentState during migration (BodyDecor
//   consumers read through SSystemFactory).
// - Landscape association data (per-body landscape name, the old
//   satellite->moon / sun->sun special rules become body data) and the
//   member-replacement path for `setLandscape` (slot-replacement semantics,
//   like BodyModule slots).
// - Zodiacal-light toggling (old per-executor-mode enableZodiacal) as
//   reference-body-class state on the milkyway member.
// - The 3D-galaxy model placement (old hardcoded setModel matrices,
//   core.cpp) becoming the galaxy body's chain transform (G2) - post-parity.
// ============================================================================

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
