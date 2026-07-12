#include "BasicMesh.hpp"
#include "tools/s_texture.hpp"
#include "tools/file_path.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/bodyModules/BasicMeshLoader.hpp"

BasicMesh::BasicMesh(ObjL *mesh, const std::string &texturePath) : BodyModule(BodyModuleType::MESH),
    mesh(mesh), mapTexture(FilePath(texturePath,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true),
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
    if (mapTexture.isLoading())
        return false;
    set->bindUniform(vert, 0);
    set->bindUniform(frag, 1);
    set->bindTexture(mapTexture.getTexture(), 2);
    // Binding 3 = the ShadowService blurred-layer array (always allocated at
    // Renderer::init, independent of the enabled flag - a valid descriptor
    // must exist even when shadows are off).
    set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
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

// Fill the Gen-2 receiver block from the body's received-shadow state
// (produced by ModularSystem::computeShadows - contract: ShadowProjection.hpp).
// Gate on the service flag too: entries may be stale from the frame the flag
// switched off.
static void fillShadows(SharedBuffer<meshFrag> &frag, ModularBody *body)
{
    const ReceivedShadows &received = body->getReceivedShadows();
    if (ShadowService::enabled && received) {
        auto &f = *frag;
        f.shadowRow0 = received.row0;
        f.shadowRow1 = received.row1;
        int nb = 0;
        for (const auto &e : received.entries) {
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
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
    fillShadows(frag, body);
    const auto screenSize = body->getScreenSize();
    if (screenSize > 0.2) {
        TEXMAP1(mapTexture);
        if (bigTextureMapping != texmap) {
            set->uninit();
            set->bindUniform(vert, 0);
            set->bindUniform(frag, 1);
            set->bindTexture(TEX(0, mapTexture), 2);
            set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
            bigTextureMapping = texmap;
        }
    } else if (bigTextureMapping) {
        set->uninit();
        set->bindUniform(vert, 0);
        set->bindUniform(frag, 1);
        set->bindTexture(mapTexture.getTexture(), 2);
        set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
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
    fillShadows(frag, body);
    if (bigTextureMapping) {
        set->uninit();
        set->bindUniform(vert, 0);
        set->bindUniform(frag, 1);
        set->bindTexture(mapTexture.getTexture(), 2);
        set->bindTexture(*Context::instance->renderer.shadow.layerArray(), 3);
        bigTextureMapping = 0;
    }
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
    // Lands with S3: TRACE service family bound once per stream by the
    // Renderer; this hook pushes {mat, clipping_fov, boundingRadius,
    // oneMinusOblateness} and draws low-LOD (the pre-rework body already had
    // that shape but pushed against the old depthTrace layout without any
    // pipeline bound - the INTENT.md 11.13 defect; zero call sites today).
}
