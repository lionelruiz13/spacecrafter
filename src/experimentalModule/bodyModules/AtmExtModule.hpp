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

// From-space atmosphere rim shell: a near component drawn after the disc, depth-tested, without depth write
// Receives no shadow; not gated by `flag atmosphere`, only by the per-body data and the size thresholds
class AtmExtModule : public BodyModule {
public:
    // gradientPath = resolved atmosphere_ext_model texture path; radiusFactor = atmosphere_radius_factor
    AtmExtModule(ObjL *mesh, const std::string &gradientPath, float radiusFactor);
    virtual ~AtmExtModule();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_TRANSLUCENT;
    }
    virtual bool isLoaded() override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // boundingRadius includes the shell: it is what reserves depth-slice room for it
    virtual bool update(ModularBody *body, float scaledRadius) override {
        boundingRadius = scaledRadius * radiusFactor;
        return true;
    }

    // std140 layout of binding 0 of atm.vert/tesc/tese/frag: field order and types are GPU-visible
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
