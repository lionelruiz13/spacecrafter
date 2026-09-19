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

// Draw the atmosphere seen from space, ignoring the atmosphere flag
class AtmExtModule : public BodyModule {
public:
    AtmExtModule(ObjL *mesh, const std::string &gradientPath, float radiusFactor);
    virtual ~AtmExtModule();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_TRANSLUCENT;
    }
    virtual bool isLoaded() override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Include the shell in boundingRadius to reserve its depth room
    virtual bool update(ModularBody *body, float scaledRadius) override {
        boundingRadius = scaledRadius * radiusFactor;
        return true;
    }

    // Must match binding 0 of atm.vert/tesc/tese/frag (std140)
    struct atmExtUBO {
        Mat4f ModelViewMatrix;
        Vec3f sunPos;
        float planetRadius;
        Vec3f bodyPos;
        float planetOneMinusOblateness;
        Vec3f clipping_fov;
        float atmRadius;
        Vec2i TesParam;   // [min_tes_lvl, max_tes_lvl]
        float atmAlpha;
    };
protected:
    // With wanted = 0 or VARIANT_NO_DEPTH
    void drawShell(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted);

    bool loaded = false;
    bool disabled = false; // gradient texture unusable (<32 texels)
    ObjL *mesh;
    s_texture gradient;
    PipelineFamily family;
    std::unique_ptr<Set> set;
    SharedBuffer<atmExtUBO> uniform;
    float radiusFactor;
};

#endif /* end of include guard: ATM_EXT_MODULE_HPP_ */
