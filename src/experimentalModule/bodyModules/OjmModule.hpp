#ifndef OJM_MODULE_HPP_
#define OJM_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

class Ojm;

// OJM slot family (BODY-role) - arbitrary 3D model bodies: artificial
// satellites, spacecraft, irregular bodies (landing zone of the old
// Artificial subclass). Hooks:
// - draw: the model with its material pipeline (old ShaderArtificial family).
// - drawShadow / drawSelfShadow: model shadow casting + self-shadowing (the
//   old ShadowExtension) - an OJM body is typically the main beneficiary of
//   high-resolution self-shadowing when it is the center of interest.
// Traits: BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_BASIC_SELF_SHADOW |
// BMT_PROJECT_G1_SHADOW.
// Data: the model resource (models are heavily shared and VRAM-cheap: D6 -
// build-once via the native-form cache, LazyOjmL path, no eviction); light
// info from ModularBody statics.
// No halo (the old Artificial suppressed it) - halo emission is already
// per-body state (isHaloEnabled), not this module's concern.
// Deduction rule: param model_name (already deduced - deduceBodyModuleList);
// loader registration lands with the implementation.
class OjmModule : public BodyModule {
public:
    OjmModule() : BodyModule(BodyModuleType::OJM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body) override;
protected:
    Ojm *model = nullptr; // Shared resource - refcounted through the resource layer
};

#endif /* end of include guard: OJM_MODULE_HPP_ */
