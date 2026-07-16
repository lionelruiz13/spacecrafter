#ifndef OJM_MODULE_HPP_
#define OJM_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp" // VariantKey (drawInternal)
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>

class Ojm;
class Set;
struct ojmVert;
struct ojmGeom;
struct ojmLight;
struct ojmShadowBlock;

// OJM slot family (BODY-role) - arbitrary 3D model bodies: artificial
// satellites, spacecraft, stations (landing zone of the old Artificial
// subclass; IMPLEMENTED 2026-07-16, OJM wave).
//
// Data model (old parse parity, protosystem.cpp:634-641 + 727-739 - the
// TWO model_name consumers are distinct): type=artificial + model_name ->
// THIS module (an Ojm: per-shape materials, Phong lighting, self-shadow);
// any OTHER type with model_name -> a named ObjL consumed by the MESH
// family (Phobos/Deimos class - BasicMeshLoader already carries it). The
// deduction gate lives in ModularBody::deduceBodyModuleList accordingly.
//
// Drawing (all four types):
// - draw/drawNoDepth: the model through the OJM family - old ShaderArtificial
//   ported. ONE family, ONE layout (INTENT §10.3 rule 4 layout-invariance),
//   4 shader rows = the old 4 frags on two SHADER_SWAP axes: TEXLESS
//   (per-SHAPE, switched mid-record by Ojm::record via Renderer::peek - a
//   shape without map_Ka uses the notex row) x SHADOWED (per-DRAW: selected
//   when this body self-shadows this frame OR has received-shadow entries -
//   the old plain/shadowed drawState pair, CoI-gated then, state-gated now).
//   Lighting parity note: the two rows keep their OLD lighting models
//   (plain = eye-space phong on Light.Position; shadowed = direction-based
//   with specular from lightDirection) - the old path switched lighting math
//   with the CoI exactly like this; divergence class carried, not created.
// - drawShadow: G1 silhouette through the OPAQUE_OJM word
//   (ShadowService::produceOjm). NO extra normalization: Ojm vertices are
//   normalized to the unit sphere at load (ojm.cpp readOJM, v /= radius) -
//   the same unit-geometry convention as ObjL, so the orchestration's
//   silhouette matrix applies unchanged. The model's original radius
//   survives as Ojm::getRadius(), consumed by the LOADER's body-radius
//   scaling (old: initialRadius *= obj3D->getRadius()).
// - drawSelfShadow: BMT_BASIC_SELF_SHADOW first client - declares the
//   SELF_DEPTH job and keeps {matrix, active} for the COLOR fill
//   (single-computation consistency; full contract at BodyModule.hpp hook
//   + ShadowService.hpp header). An OJM body is the main beneficiary of
//   high-resolution self-shadowing when it is the body of interest.
// - drawTrace: orbit-hole depth - rides S3 with every other trace client.
//
// Receiving: BMT_RECEIVE_SHADOW - entries sampled in the shadowed COLOR row
// (receivedShadows.glsl include; rows folded through the model-view map like
// the ray-march receiver, since the OJM frag works in model space). BETTER
// than old by construction: the old artificial received entries only as CoI;
// every drawn OJM body receives here (documented divergence, shadow-paths G).
//
// No halo: the old Artificial suppressed drawHalo entirely - carried by the
// loader clearing the body's halo flag (halo emission is body state, not
// this module's concern).
// Resources: the shared Ojm (D6 - build-once via Ojm::load recycler +
// native-form cache, no eviction pressure); synchronous load at construction
// is old parity (S4/G5 does not gate module ports).
// Deduction: model_name + type=artificial (see above); loader = OjmLoader.
class OjmModule : public BodyModule {
public:
    // model: shared Ojm (load failure -> module reports isLoaded() false
    // forever and draws nothing; the old path zeroed the body radius - the
    // loader carries that parity at construction).
    OjmModule(std::shared_ptr<Ojm> model);
    ~OjmModule();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_BASIC_SELF_SHADOW
             | BMT_PROJECT_G1_SHADOW | BMT_RECEIVE_SHADOW;
    }
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool isLoaded() override;
protected:
    // Shared draw body of draw/drawNoDepth (variant bit is the only delta).
    void drawInternal(Renderer &renderer, ModularBody *body, const Mat4f &mat, VariantKey base);
    std::shared_ptr<Ojm> model; // shared resource (Ojm::load recycler, D6)
    // Two Sets on ONE layout (the old set / ext->shadowSet pair, ported):
    // binding 2 carries ojmLight on the plain rows and ojmShadowBlock on the
    // shadowed rows - same binding TYPE (layout invariance), different
    // module-owned buffer. Both bind the self-shadow depth + layer array at
    // 3/4 (always-valid descriptors; the plain rows never read them).
    std::unique_ptr<Set> setPlain;
    std::unique_ptr<Set> setShadow;
    SharedBuffer<ojmVert> uVert;
    SharedBuffer<ojmGeom> uGeom;
    SharedBuffer<ojmLight> uLight;        // plain rows, binding 2
    SharedBuffer<ojmShadowBlock> uShadow; // shadowed rows, binding 2
    Mat4f selfShadowMat;        // the nomination's matrix (consumption copy)
    bool selfShadowActive = false; // frame-scoped (BodyModule.hpp contract)
    bool bound = false;         // set bindings done at first successful load
};

#endif /* end of include guard: OJM_MODULE_HPP_ */
