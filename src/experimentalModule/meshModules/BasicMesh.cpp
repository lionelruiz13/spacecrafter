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
    set(*VulkanMgr::instance, *Context::instance->setMgr, BodyShader::getShaderNormal()->layout, -1, false, true),
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
    set.bindUniform(vert, 0);
    set.bindUniform(frag, 1);
    set.bindTexture(mapTexture.getTexture(), 2);
    set.bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
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
    BodyShader::getShaderNormal()->pipeline->bind(renderer);
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
            set.uninit();
            set.bindUniform(vert, 0);
            set.bindUniform(frag, 1);
            set.bindTexture(TEX(0, mapTexture), 2);
            set.bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3); // No big texture for the eclipse map
            bigTextureMapping = texmap;
        }
    } else if (bigTextureMapping) {
        set.uninit();
        set.bindUniform(vert, 0);
        set.bindUniform(frag, 1);
        set.bindTexture(mapTexture.getTexture(), 2);
        set.bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
        bigTextureMapping = 0;
    }
    BodyShader::getShaderNormal()->layout->bindSets(renderer, {set, *Context::instance->uboSet});
	mesh->draw(renderer, screenSize*1024);
}

void BasicMesh::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    BodyShader::getShaderNormal()->pipelineNoDepth->bind(renderer);
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
        set.uninit();
        set.bindUniform(vert, 0);
        set.bindUniform(frag, 1);
        set.bindTexture(mapTexture.getTexture(), 2);
        set.bindTexture(BasicMeshLoader::instance->texEclipseMap.getTexture(), 3);
        bigTextureMapping = 0;
    }
    BodyShader::getShaderNormal()->layout->bindSets(renderer, {set, *Context::instance->uboSet});
	mesh->drawLow(renderer);
}

void BasicMesh::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    BodyShader::getShaderShadowShape()->pipeline->bind(renderer);
    BodyShader::getShaderShadowShape()->layout->bindSet(renderer, *Context::instance->shadowData[idx].traceSet);
    mesh->bind(renderer);
    mesh->draw(renderer, Context::instance->shadowRes);
}

void BasicMesh::drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // BodyShader::getShaderShadowTrace()->pipeline->bind(renderer);
    // BodyShader::getShaderShadowTrace()->layout->bindSet(renderer, traceSet);
    // mesh->bind(renderer);
    // mesh->draw(renderer, Context::instance->selfShadowRes);
}

void BasicMesh::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    depthTraceInfo pdata {mat, renderer.getClippingFov(), boundingRadius, body->getOneMinusOblateness()};
    BodyShader::getShaderDepthTrace()->layout->pushConstant(renderer, 0, &pdata);
    mesh->drawLow(renderer);
}
