#include "OortModule.hpp"
#include "tools/context.hpp"
#include "coreModule/oort.hpp" // oortSamplePoint - the shared spatial law (I2)
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"

bool OortModule::show = false;

OortModule::OortModule(unsigned int nbr, const Vec3f &color)
    : BodyModule(BodyModuleType::CUSTOM), nbPoints(nbr), cloudColor(color)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Renderer &renderer = Context::instance->renderer;

    // Vertex layout: heliocentric-ecliptic AU points (old Oort m_dataGL, vec3).
    vertexModel = std::make_unique<VertexArray>(vkmgr);
    vertexModel->createBindingEntry(3 * sizeof(float));
    vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);

    // Point cloud, materialized from the SHARED spatial law (I2, B5 §6.9):
    // oortSamplePoint() is the single authority both paths draw from. Same
    // upload path as the old Oort::populate (globalBuffer + planCopy staging).
    vertex = vertexModel->createBuffer(0, nbPoints, Context::instance->globalBuffer.get());
    Vec3f *dst = (Vec3f *) Context::instance->transfer->planCopy(vertex->get());
    for (unsigned int i = 0; i < nbPoints; ++i) {
        const Vec3f p = oortSamplePoint();
        const float r = p.length();
        if (r > cloudExtent)
            cloudExtent = r;
        *(dst++) = p;
    }

    // Pipeline family "OORT" - oort.vert/frag VERBATIM (shared with the old
    // path). set 0 = cam_block (globalUboContract, the shader's set=0), set 1 =
    // local {ModelViewMatrix (vert), color/fader (frag)} - the old oort layout.
    SetContractDesc local;
    local.name = "oortLocal";
    local.bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},
    };
    local.expectedSets = 1;
    PipelineFamilyDesc desc;
    desc.name = "OORT";
    desc.vertex = vertexModel.get();
    desc.sets.push_back(renderer.globalUboContract());              // set 0 (cam_block)
    desc.sets.push_back(renderer.allocateSetContract(std::move(local))); // set 1 (local)
    PassDesc colorPass;
    colorPass.pass = PassKind::COLOR;
    colorPass.shaderTable = {{0, {.vert = "oort.vert.spv", .frag = "oort.frag.spv"}}};
    // Old fixed state (oort.cpp createSC_context): POINT_LIST, no depth
    // (setDepthStencilMode() default off). Alpha blend so the frag color.a
    // (fader) fades the diffuse cloud; no cull (points).
    colorPass.state.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    colorPass.state.blend = BLEND_SRC_ALPHA;
    colorPass.state.cull = false;
    colorPass.state.depthTest = false;
    colorPass.state.depthWrite = false;
    desc.passes.push_back(std::move(colorPass));
    family = renderer.allocateFamily(std::move(desc));

    set = renderer.allocSet(family, 1);
    uMat = std::make_unique<SharedBuffer<Mat4f>>(*Context::instance->uniformMgr);
    uFrag = std::make_unique<SharedBuffer<Frag>>(*Context::instance->uniformMgr);
    set->bindUniform(uMat, 0);
    set->bindUniform(uFrag, 1);
    set->update();
    uFrag->get().color = cloudColor;
    uFrag->get().fader = 1.f;
}

OortModule::~OortModule() = default;

bool OortModule::update(ModularBody *body, float scaledRadius)
{
    // Static cloud: the bounding radius is the geometry extent (all-direction
    // visibility inside the cloud), NOT the body's navigational scaledRadius
    // (which gates the near/in regime and thus the LOW edge of the draw). The
    // two are deliberately decoupled: radius small (regime low edge) vs extent
    // large (visibility) - see OortModule.hpp.
    boundingRadius = cloudExtent;
    return true;
}

void OortModule::render(Renderer &renderer, const Mat4f &mat)
{
    if (!show)
        return;
    const FamilyBound bound = renderer.bind(family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade, logged once
    *uMat = mat;
    uFrag->get().color = cloudColor;
    uFrag->get().fader = 1.f;
    bound.layout->bindSets(renderer, {*Context::instance->uboSet, *set});
    VertexArray::bind(renderer, vertex->get());
    vkCmdDraw(renderer, nbPoints, 1, 0, 0);
}

void OortModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    render(renderer, mat);
}

void OortModule::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    render(renderer, mat);
}
