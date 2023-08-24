#ifndef BASIC_MESH_HPP_
#define BASIC_MESH_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "bodyModule/bodyShader.hpp"
class ObjL;

class BasicMesh : public BodyModule {
public:
    BasicMesh(ObjL *mesh, const std::string &texturePath);
    virtual ~BasicMesh();
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    void invalidate();
private:
    uint16_t bigTextureMapping = 0;
    ObjL *mesh;
    s_texture mapTexture;
    Set set;
    SharedBuffer<globalVertProj> vert;
    SharedBuffer<globalFrag> frag;
};

#endif /* end of include guard: BASIC_MESH_HPP_ */
