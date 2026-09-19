#ifndef PHOTOSPHERE_MODULE_HPP_
#define PHOTOSPHERE_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp"
#include "experimentalModule/meshModules/SkinnableColorMap.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
class ObjL;

// Draw the limb-darkened disc of a star, without any shadow
class PhotosphereModule : public BodyModule {
public:
    PhotosphereModule(ObjL *mesh, const std::string &texturePath);
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE;
    }
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body, int keepFrames) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void createTexSkin(const std::string &texName) override;
    virtual void switchTexSkin(bool use) override;
    bool getSkinUse(bool &out) const override { out = colorMap.isSkinUsed(); return true; }
private:
    void bindColor(Texture &color);
    void fillVert(ModularBody *body, const Mat4f &mat);
    bool loaded = false;
    ObjL *mesh;
    SkinnableColorMap colorMap;
    PipelineFamily family;
    std::unique_ptr<Set> set;
    SharedBuffer<globalVertProj> vert;
};

#endif /* end of include guard: PHOTOSPHERE_MODULE_HPP_ */
