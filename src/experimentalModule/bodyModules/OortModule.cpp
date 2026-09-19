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
Vec3f OortModule::cloudColor{};

OortModule::OortModule(unsigned int nbr, const Vec3f &color)
    : BodyModule(BodyModuleType::CUSTOM), nbPoints(nbr)
{
    cloudColor = color;
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Renderer &renderer = Context::instance->renderer;

    // Vertex layout: heliocentric-ecliptic AU points (old Oort m_dataGL, vec3).
    vertexModel = std::make_unique<VertexArray>(vkmgr);
    vertexModel->createBindingEntry(3 * sizeof(float));
    vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);

    vertex = vertexModel->createBuffer(0, nbPoints, Context::instance->globalBuffer.get());
    Vec3f *dst = (Vec3f *) Context::instance->transfer->planCopy(vertex->get());
    std::mt19937 rng = oortRng();
    for (unsigned int i = 0; i < nbPoints; ++i) {
        const Vec3f p = oortSamplePoint(rng);
        const float r = p.length();
        if (r > cloudExtent)
            cloudExtent = r;
        *(dst++) = p;
    }

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
