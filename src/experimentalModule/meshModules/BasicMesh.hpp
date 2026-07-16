#ifndef BASIC_MESH_HPP_
#define BASIC_MESH_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/meshModules/MeshFamilies.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
class ObjL;

class BasicMesh : public BodyModule {
public:
    BasicMesh(ObjL *mesh, const std::string &texturePath);
    virtual ~BasicMesh();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_PROJECT_G1_SHADOW | BMT_RECEIVE_SHADOW;
    }
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    void invalidate();
private:
    bool loaded = false;
    uint16_t bigTextureMapping = 0;
    ObjL *mesh;
    s_texture mapTexture;
    PipelineFamily family; // MESH family handle (MeshFamilies::meshNormal)
    std::unique_ptr<Set> set; // allocated from the family's contract pools (Renderer::allocSet)
    SharedBuffer<globalVertProj> vert;
    SharedBuffer<meshFrag> frag; // Gen-2 receiver data (was globalFrag - the
                                 // retired Gen-1 LUT feed, shadow-paths.md B4)
};

#endif /* end of include guard: BASIC_MESH_HPP_ */
