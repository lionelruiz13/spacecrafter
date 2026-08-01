#ifndef BASIC_MESH_HPP_
#define BASIC_MESH_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/meshModules/MeshFamilies.hpp"
#include "experimentalModule/meshModules/SkinnableColorMap.hpp"
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
    // Skin seam (old Body::createTexSkin/switchMapSkin - contract at
    // BodyModule.hpp): swaps the color map at binding 2, never the big-texture
    // path (a script skin has no big texture; old binds tex_current the same way).
    // The RULES live in SkinnableColorMap (I2 - shared with the star
    // photosphere since B12); this module only binds what it is told to bind.
    virtual void createTexSkin(const std::string &texName) override;
    virtual void switchTexSkin(bool use) override;
    bool getSkinUse(bool &out) const override { out = colorMap.isSkinUsed(); return true; }
    void invalidate();
private:
    // One rebind site for every binding-state change (was 3 copy-pasted
    // blocks; the skin state would have made it 5).
    void bindColor(Texture &color);
    bool loaded = false;
    ObjL *mesh;
    SkinnableColorMap colorMap;
    PipelineFamily family; // MESH family handle (MeshFamilies::meshNormal)
    std::unique_ptr<Set> set; // allocated from the family's contract pools (Renderer::allocSet)
    SharedBuffer<globalVertProj> vert;
    SharedBuffer<meshFrag> frag; // Gen-2 receiver data (was globalFrag - the
                                 // retired Gen-1 LUT feed, shadow-paths.md B4)
};

#endif /* end of include guard: BASIC_MESH_HPP_ */
