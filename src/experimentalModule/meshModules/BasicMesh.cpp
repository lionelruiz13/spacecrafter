#include "BasicMesh.hpp"
#include "tools/s_texture.hpp"
#include "tools/file_path.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/moduleLoader/BasicMeshLoader.hpp"
// (fillShadows lifted to meshShadowFill.hpp when row 2 added more receiver
//  families - single authority, I2.)
#include "experimentalModule/meshModules/meshShadowFill.hpp"
#include "experimentalModule/bodyModules/TraceFamily.hpp"

BasicMesh::BasicMesh(ObjL *mesh, const std::string &texturePath) : BodyModule(BodyModuleType::MESH),
    mesh(mesh), colorMap(FilePath(texturePath,FilePath::TFP::TEXTURE).toString()),
    family(MeshFamilies::meshNormal()),
    set(Context::instance->renderer.allocSet(family, 0)),
    vert(*Context::instance->uniformMgr), frag(*Context::instance->uniformMgr)
{
    frag->nbShadowingBodies = 0;
}

BasicMesh::~BasicMesh()
{
}

bool BasicMesh::isLoaded()
{
    if (loaded)
        return true;
    if (colorMap.isLoading())
        return false;
    set->bindUniform(vert, 0);
    set->bindUniform(frag, 1);
    set->bindTexture(colorMap.map().getTexture(), 2);
    set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
    loaded = true;
    return true;
}

void BasicMesh::preload(ModularBody *body, int keepFrames)
{
    colorMap.preload(keepFrames);
}

void BasicMesh::bindColor(Texture &color)
{
    set->uninit();
    set->bindUniform(vert, 0);
    set->bindUniform(frag, 1);
    set->bindTexture(color, 2);
    set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
}

void BasicMesh::createTexSkin(const std::string &texName)
{
    colorMap.createSkin(texName);
}

void BasicMesh::switchTexSkin(bool use)
{
    colorMap.switchSkin(use);
}

void BasicMesh::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    const FamilyBound bound = renderer.bind(family);
    if (!bound.layout)
        return; // pass unavailable (base build failed)
    mesh->bind(renderer);
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    vert->planetScaledRadius = boundingRadius;
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
    fillPlainShadows(frag, body, this);
    const auto screenSize = body->getScreenSize();
    if (Texture *color = colorMap.resolve(screenSize > ModularBody::bigTextureGate()))
        bindColor(*color);
    bound.layout->bindSets(renderer, {*set, *Context::instance->uboSet});
	mesh->draw(renderer, screenSize*1024);
}

void BasicMesh::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Reserved NO_DEPTH variant; until its lazy build is resident, bind falls
    // back to the depth-on base (got reports it) - transient, first frames only.
    const FamilyBound bound = renderer.bind(family, VARIANT_NO_DEPTH);
    if (!bound.layout)
        return; // same degradation as draw()
    mesh->bind(renderer);
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    vert->planetScaledRadius = boundingRadius;
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
    fillPlainShadows(frag, body, this);
    // Same binding-state machine as draw(), minus the big-texture branch
    // (the noDepth band never engages big textures).
    if (Texture *color = colorMap.resolve(false))
        bindColor(*color);
    bound.layout->bindSets(renderer, {*set, *Context::instance->uboSet});
	mesh->drawLow(renderer);
}

void BasicMesh::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    renderer.shadow.produce(idx, mat, mesh);
}

void BasicMesh::drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
}

void BasicMesh::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    const FamilyBound bound = renderer.bind(TraceFamily::sphere());
    if (!bound.layout)
        return; // trace shader not deployed - C3 degrade (orbits draw depth-free)
    TraceInfo info;
    info.ModelViewMatrix = mat;                                 // body mat (old mat.convert())
    info.clipping_fov = renderer.getClippingFov();              // the ORBIT range this frame
    info.planetScaledRadius = body->getScaledRadius();          // solid disc (old _radius = radius)
    info.planetOneMinusOblateness = body->getOneMinusOblateness();
    bound.layout->pushConstant(renderer, 0, &info);
    mesh->bind(renderer);
    // Same LOD as the drawn disc (old currentObj->draw at the display LOD) so
    // the depth silhouette matches the colored body exactly.
    mesh->draw(renderer, body->getScreenSize()*1024);
}
