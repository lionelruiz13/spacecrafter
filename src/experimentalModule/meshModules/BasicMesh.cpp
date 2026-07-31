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
    // Binding 3 = the ShadowService blurred-layer array (always allocated at
    // Renderer::init, independent of the enabled flag - a valid descriptor
    // must exist even when shadows are off).
    set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
    loaded = true;
    return true;
}

void BasicMesh::preload(ModularBody *body)
{
    colorMap.preload();
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
        return; // pass unavailable (base build failed, e.g. shader file not
                // deployed) - already logged at its definition site; drawing
                // degrades to the body's other content instead of null-deref
                // (latent since S1, materialized by the first absent shader)
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
    // Binding-state machine (SkinnableColorMap - the shared authority): every
    // transition (skin on/off, skin load completion, big texture appear/drop)
    // lands on exactly one compare, and only a CHANGE comes back here.
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
    // Declares this mesh's silhouette for layer idx; the service records it
    // in the pre-color window (sync-interim contract, BodyModule.hpp hook 2;
    // mat = the silhouette matrix from the orchestration).
    renderer.shadow.produce(idx, mat, mesh);
}

void BasicMesh::drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Lands with the first self-shadow client (OJM/terrain, INTENT.md 12
    // rows 2-3): the SELF_SHADOW pass profile exists in the registry; no
    // MESH-sphere client needs it (a sphere's self-shadow is its lit
    // hemisphere, already analytic in the lighting).
}

void BasicMesh::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Row-8 TRACE prepass (S3 consumer live, 2026-07-19): writes this body's
    // disc into the depth buffer under the orbit-union range so the orbit LINE
    // vanishes behind it. Old drawOrbit(cmdBodyDepth,…): push depthTraceInfo,
    // draw the sphere (body.cpp:1215-1217). The Renderer binds the shared
    // sphere-trace pipeline for us (bind-and-record; passKind == TRACE).
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
