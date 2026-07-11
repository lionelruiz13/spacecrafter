#ifndef ENVIRONMENT_MODULE_HPP_
#define ENVIRONMENT_MODULE_HPP_

#include "tools/vecmath.hpp"

class ModularBody;
class Renderer;

// ============================================================================
// DRAFT interface - pending convergence with Vixy (INTENT.md 3.8).
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
// priorities.
//
// Open points for convergence (deliberately NOT designed here):
// - Visibility gating (old BodyDecor: canDrawLandscape/Body/Meteor from
//   altitude + atmosphere state): per-module state aggregated by the system,
//   or a dedicated query interface?
// - Draw ordering between simultaneous environments (milkyway backdrop vs
//   atmosphere sky vs landscape) - fixed pass order or module-declared?
// - Tone-mapping interaction (atmosphere affects ToneReproductor).
// ============================================================================
class EnvironmentModule {
public:
    virtual ~EnvironmentModule() = default;
    // Camera entered the space this environment covers. Activation edge:
    // raise resource priorities here - never load synchronously (C3/C4).
    virtual void enter(ModularBody *body) {}
    // Camera left. Deactivation edge: lower priorities; the lowest LoD stays
    // resident (D6) so re-entry always has something drawable.
    virtual void leave(ModularBody *body) {}
    // Per-frame while active; cameraLocalPos = camera position in the body's
    // local frame (the quantity everything camera-location-relative needs).
    // Return true when updates are no longer required until the next enter().
    virtual bool update(ModularBody *body, const Vec3f &cameraLocalPos) {
        return true;
    }
    // Backdrop pass - drawn before all bodies (ex: milkyway 2D/3D).
    virtual void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Sky/near pass - drawn after bodies (ex: atmosphere, landscape).
    virtual void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
};

#endif /* end of include guard: ENVIRONMENT_MODULE_HPP_ */
