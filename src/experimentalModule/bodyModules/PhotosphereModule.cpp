#include "PhotosphereModule.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/bodyModules/TraceFamily.hpp"
#include "tools/context.hpp"
#include "tools/file_path.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"

namespace {
// The STAR_SURFACE pipeline family. Registration-domain accessor, lazily
// allocated on first module construction (the MeshFamilies pattern).
//
// Set contract: ONE set - {0: globalVertProj, 1: the colour map}. It carries
// neither the receive-shadow block nor the shadow layer array (this family
// receives no shadows - header) and no global UBO: a self-lit surface reads
// neither `ambient` nor `time`, so cam_block is not included by either stage,
// and declaring a set no shader reads would be a contract with no reader
// (TraceFamily precedent: a family may declare fewer sets).
const PipelineFamily &starSurfaceFamily()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        // The old path's map sampler: default + REPEAT on U (bodyShader.cpp
        // createShader; body_sun.cpp's layoutSun does exactly the same).
        VkSamplerCreateInfo mapSampler = PipelineLayout::DEFAULT_SAMPLER;
        mapSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SetContractDesc contract;
        contract.name = "bodyStarSurface";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},           // globalVertProj
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // mapTexture
        };
        contract.expectedSets = 4; // D5 parameter: live stars (one today; pools grow by aggregate)
        PipelineFamilyDesc desc;
        desc.name = "STAR_SURFACE";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "bodyStarSurface.vert.spv", .frag = "bodyStarSurface.frag.spv"}}};
        // color.state: FixedState defaults == the old sun pipeline
        // (body_sun.cpp createSunShader: cull on, BLEND_NONE, triangle list,
        // depth test+write on).
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}
} // namespace

PhotosphereModule::PhotosphereModule(ObjL *mesh, const std::string &texturePath)
    : BodyModule(BodyModuleType::MESH),
      mesh(mesh), colorMap(FilePath(texturePath, FilePath::TFP::TEXTURE).toString()),
      family(starSurfaceFamily()),
      set(Context::instance->renderer.allocSet(family, 0)),
      vert(*Context::instance->uniformMgr)
{
}

bool PhotosphereModule::isLoaded()
{
    if (loaded)
        return true;
    if (colorMap.isLoading())
        return false;
    set->bindUniform(vert, 0);
    set->bindTexture(colorMap.map().getTexture(), 1);
    loaded = true;
    return true;
}

void PhotosphereModule::preload(ModularBody *body)
{
    colorMap.preload();
}

void PhotosphereModule::bindColor(Texture &color)
{
    set->uninit();
    set->bindUniform(vert, 0);
    set->bindTexture(color, 1);
}

void PhotosphereModule::createTexSkin(const std::string &texName)
{
    colorMap.createSkin(texName);
}

void PhotosphereModule::switchTexSkin(bool use)
{
    colorMap.switchSkin(use);
}

void PhotosphereModule::fillVert(ModularBody *body, const Mat4f &mat)
{
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = Context::instance->renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    vert->planetScaledRadius = boundingRadius;
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
}

void PhotosphereModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    const FamilyBound bound = renderer.bind(family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - same C3 degradation
                // as BasicMesh: the body's other content still draws
    mesh->bind(renderer);
    fillVert(body, mat);
    const auto screenSize = body->getScreenSize();
    if (Texture *color = colorMap.resolve(screenSize > 0.2))
        bindColor(*color);
    bound.layout->bindSets(renderer, {*set});
    mesh->draw(renderer, screenSize * 1024);
}

void PhotosphereModule::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Reserved NO_DEPTH variant; until its lazy build is resident, bind falls
    // back to the depth-on base (BasicMesh has the same transient).
    const FamilyBound bound = renderer.bind(family, VARIANT_NO_DEPTH);
    if (!bound.layout)
        return;
    mesh->bind(renderer);
    fillVert(body, mat);
    // The depth-less mid band never engages big textures (BasicMesh parity).
    if (Texture *color = colorMap.resolve(false))
        bindColor(*color);
    bound.layout->bindSets(renderer, {*set});
    mesh->drawLow(renderer);
}

void PhotosphereModule::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Row-8 TRACE prepass, identical to BasicMesh's: a star with an orbit (a
    // script-loaded companion star) must cut its own orbit line's hole. The
    // shipped Sun carries no orbit_visualization_period, so this is unexercised
    // on shipped data - present because dropping it would silently foreclose
    // the case, not because it fires today.
    const FamilyBound bound = renderer.bind(TraceFamily::sphere());
    if (!bound.layout)
        return; // trace shader not deployed - C3 degrade (orbits draw depth-free)
    TraceInfo info;
    info.ModelViewMatrix = mat;
    info.clipping_fov = renderer.getClippingFov();
    info.planetScaledRadius = body->getScaledRadius();
    info.planetOneMinusOblateness = body->getOneMinusOblateness();
    bound.layout->pushConstant(renderer, 0, &info);
    mesh->bind(renderer);
    mesh->draw(renderer, body->getScreenSize() * 1024);
}
