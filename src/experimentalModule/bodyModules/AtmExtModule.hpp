#ifndef ATM_EXT_MODULE_HPP_
#define ATM_EXT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
class ObjL;

// ATMOSPHERE slot - the FROM-SPACE atmosphere rim shell around a body
// (port of the old AtmExt, atm_ext.{hpp,cpp} - row 13). Decision 2026-07-11:
// the two things the old path called "atmosphere" split - this per-body rim
// is a BodyModule; the from-ground sky (+ BodyDecor visibility logic) is an
// EnvironmentModule concern (S8). One word, two features, two homes.
//
// Shaders atm.{vert,tesc,tese,frag} REUSED VERBATIM (self-contained UBO +
// gradient sampler, no cam_block - POINTER precedent: parity by
// construction). Blend = SRC_ALPHA with colorBlendOp MAX (brighten-only
// compositing over the disc). Drawn as nearComponent AFTER the disc in the
// body's depth slice, depth-TESTED and depth-WRITE-FREE (INTENT 5.33): a
// translucent brighten-only shell composites over whatever is behind it and
// occludes nothing, so it must not leave its own surface in the bucket's
// depth. It stands radiusFactor*scaledRadius up (191.34 km on Earth) and,
// being the LAST thing drawn in the parent's near list, its depth write was
// killing every grounded body below that height - taller than the proxy
// shell (5.29) and than any terrain (5.30). The one deliberate divergence
// from the old pipeline state, which wrote depth only by inheriting the
// EntityCore default (atm_ext.cpp:16 never calls setDepthStencilMode).
//
// PARITY LANDMINES (verified on old source, do not "fix"):
// - the shell receives NO shadow/eclipse input (atm.* have zero shadow
//   uniforms) - a from-space lunar-eclipse umbra does NOT dim the rim;
// - `flag atmosphere` does NOT gate it (that flag reaches only the ground
//   sky dome) - gating is per-body data + the size thresholds below;
// - atmAlpha stays 1 (the old "Apply fader here" TODO is kept as-is).
//
// Old gate (body.cpp drawAtmExt), translated to new-path units in draw():
//   screen_sz > 10 px && full angular size > 2 deg
//   && distance > radius * radiusFactor * 1.01
// Deduction rule mirrors the old parse precondition exactly (protosystem.cpp
// 865-871): (has_atmosphere || atmosphere_lim_landscape present) &&
// atmosphere_ext_model non-empty.
class AtmExtModule : public BodyModule {
public:
    // gradientPath = resolved atmosphere_ext_model texture path;
    // radiusFactor = atmosphere_radius_factor (default 1.05, old parity).
    AtmExtModule(ObjL *mesh, const std::string &gradientPath, float radiusFactor);
    virtual ~AtmExtModule();
    virtual uint32_t getTraits() const override {
        // TRANSLUCENT: the shell blends (SRC_ALPHA + MAX) over the disc - the
        // after-opaque ordering was previously implicit in deduction order
        // ("Positioned after MESH/OJM" comment), now declared (routing
        // partition, ModuleLoader::addNearComponent).
        return BMT_USE_DEPTH | BMT_TRANSLUCENT;
    }
    virtual bool isLoaded() override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // boundingRadius = scaledRadius * radiusFactor: the inclusive-radius
    // contract (BodyModule.hpp) is what reserves depth-slice room for the
    // shell - the contract, not a renderer guard, carries the safety.
    // Consequence (convergence point, plan 2026-07-15): body boundingRadius
    // grows <=3% on shell bodies, shifting screenSize-derived visuals by the
    // same factor (old hid the shell inside its 1.1 depth margin).
    virtual bool update(ModularBody *body, float scaledRadius) override {
        boundingRadius = scaledRadius * radiusFactor;
        return true;
    }

    // std140 mirror of atm.vert/tesc/tese/frag binding 0 - exact copy of the
    // old AtmExt::_uniform (atm_ext.hpp:20-30). Field order and types are the
    // GPU-visible layout (verified offsets: Vec2i @112, atmAlpha @120).
    struct atmExtUBO {
        Mat4f ModelViewMatrix;
        Vec3f sunPos;
        float planetRadius;
        Vec3f bodyPos;
        float planetOneMinusOblateness;
        Vec3f clipping_fov;
        float atmRadius;
        Vec2i TesParam;   // [min_tes_lvl, max_tes_lvl]
        float atmAlpha;   // transparency scale, 1 (old fader TODO kept)
    };
protected:
    // Shared draw path of draw()/drawNoDepth(); wanted = 0 or VARIANT_NO_DEPTH.
    void drawShell(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted);

    bool loaded = false;
    bool disabled = false; // gradient texture unusable (<32 texels, old guard)
    ObjL *mesh;
    s_texture gradient;
    PipelineFamily family;
    std::unique_ptr<Set> set;
    SharedBuffer<atmExtUBO> uniform;
    float radiusFactor;
};

#endif /* end of include guard: ATM_EXT_MODULE_HPP_ */
