#include "BasicMesh.hpp"
#include "tools/s_texture.hpp"
#include "tools/file_path.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/bodyModules/BasicMeshLoader.hpp"

BasicMesh::BasicMesh(ObjL *mesh, const std::string &texturePath) : BodyModule(BodyModuleType::MESH),
    mesh(mesh), mapTexture(FilePath(texturePath,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true),
    family(MeshFamilies::meshNormal()),
    set(Context::instance->renderer.allocSet(family, 0)),
    vert(*Context::instance->uniformMgr), frag(*Context::instance->uniformMgr)
{
}

BasicMesh::~BasicMesh()
{
}

bool BasicMesh::isLoaded()
{
    if (loaded)
        return true;
    if (BasicMeshLoader::instance->texEclipseMap.isLoading() || mapTexture.isLoading())
        return false;
    set->bindUniform(vert, 0);
    set->bindUniform(frag, 1);
    set->bindTexture(mapTexture.getTexture(), 2);
    set->bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
    loaded = true;
    return true;
}

void BasicMesh::preload(ModularBody *body)
{
    int tmp = s_texture::setBigTextureLifetime(100);
    mapTexture.prioritize(LoadPriority::PRELOAD);
    mapTexture.getBigTexture();
    s_texture::setBigTextureLifetime(tmp);
}

void BasicMesh::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    const FamilyBound bound = renderer.bind(family);
    mesh->bind(renderer);
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    vert->planetScaledRadius = boundingRadius;
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
    frag->SunHalfAngle = body->getLightHalfAngle();
    const auto screenSize = body->getScreenSize();
    if (screenSize > 0.2) {
        TEXMAP1(mapTexture);
        if (bigTextureMapping != texmap) {
            set->uninit();
            set->bindUniform(vert, 0);
            set->bindUniform(frag, 1);
            set->bindTexture(TEX(0, mapTexture), 2);
            set->bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3); // No big texture for the eclipse map
            bigTextureMapping = texmap;
        }
    } else if (bigTextureMapping) {
        set->uninit();
        set->bindUniform(vert, 0);
        set->bindUniform(frag, 1);
        set->bindTexture(mapTexture.getTexture(), 2);
        set->bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
        bigTextureMapping = 0;
    }
    bound.layout->bindSets(renderer, {*set, *Context::instance->uboSet});
	mesh->draw(renderer, screenSize*1024);
}

void BasicMesh::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Reserved NO_DEPTH variant; until its lazy build is resident, bind falls
    // back to the depth-on base (got reports it) - transient, first frames only.
    const FamilyBound bound = renderer.bind(family, VARIANT_NO_DEPTH);
    mesh->bind(renderer);
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    vert->planetScaledRadius = boundingRadius;
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
    frag->SunHalfAngle = body->getLightHalfAngle();
    if (bigTextureMapping) {
        set->uninit();
        set->bindUniform(vert, 0);
        set->bindUniform(frag, 1);
        set->bindTexture(mapTexture.getTexture(), 2);
        set->bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
        bigTextureMapping = 0;
    }
    bound.layout->bindSets(renderer, {*set, *Context::instance->uboSet});
	mesh->drawLow(renderer);
}

void BasicMesh::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    // Lands with G7 (S5): SHADOW_STENCIL service family bound by the Renderer
    // per stream; this hook then only pushes constants and draws (the old
    // body bound old-path shadowShape state - zero call sites in the new
    // path, INTENT.md 11.13; removed with the bodyShader borrow, 2026-07-12).
}

void BasicMesh::drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Lands with G7 (S5): SELF_SHADOW service family - same shape as
    // drawShadow (was already fully commented out before the re-home).
}

void BasicMesh::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Lands with S3/S5: TRACE service family bound once per stream by the
    // Renderer; this hook pushes {mat, clipping_fov, boundingRadius,
    // oneMinusOblateness} and draws low-LOD (the pre-rework body already had
    // that shape but pushed against the old depthTrace layout without any
    // pipeline bound - the INTENT.md 11.13 defect; zero call sites today).
}
