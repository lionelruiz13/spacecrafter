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
    // Skin seam (old Body::createTexSkin/switchMapSkin - contract at
    // BodyModule.hpp): swaps the color map at binding 2, never the big-texture
    // path (a script skin has no big texture; old binds tex_current the same way).
    virtual void createTexSkin(const std::string &texName) override;
    virtual void switchTexSkin(bool use) override;
    void invalidate();
private:
    // The drawn color texture (old Body::tex_current): the skin when active
    // AND resident (a loading skin keeps the map bound - an s_texture bound
    // before its upload is an uninitialized descriptor), else the map.
    s_texture *activeColorTex();
    // One rebind site for every binding-state change (was 3 copy-pasted
    // blocks; the skin state would have made it 5).
    void bindColor(Texture &color);
    bool loaded = false;
    // Binding-state key of the set's color slot: 0 = plain map,
    // big-texture bit = TEXMAP1 mapping, BIND_SKIN = skin texture.
    static constexpr uint16_t BIND_SKIN = 0x8000;
    uint16_t texBinding = 0;
    bool skinUse = false;
    std::unique_ptr<s_texture> skinTexture;
    ObjL *mesh;
    s_texture mapTexture;
    PipelineFamily family; // MESH family handle (MeshFamilies::meshNormal)
    std::unique_ptr<Set> set; // allocated from the family's contract pools (Renderer::allocSet)
    SharedBuffer<globalVertProj> vert;
    SharedBuffer<meshFrag> frag; // Gen-2 receiver data (was globalFrag - the
                                 // retired Gen-1 LUT feed, shadow-paths.md B4)
};

#endif /* end of include guard: BASIC_MESH_HPP_ */
