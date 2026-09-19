#include "AxisModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include <cmath>

bool AxisModule::show = false; // old Axis::actualdrawaxis init (axis.cpp:27)

AxisModule::AxisModule() : BodyModule(BodyModuleType::AXIS) {}
AxisModule::~AxisModule() = default; // VertexBuffer complete here

namespace {
struct AxisFamilyData {
    std::unique_ptr<VertexArray> vertexModel; // 1 binding, vec3 pos (axis.cpp:118-120)
    PipelineFamily family;
    std::unique_ptr<SharedBuffer<Vec3f>> uColor;
    Set *set; // renderer-pool owned
};
AxisFamilyData &axisFamily()
{
    static AxisFamilyData data = []() -> AxisFamilyData {
        Renderer &renderer = Context::instance->renderer;
        AxisFamilyData d;
        d.vertexModel = std::make_unique<VertexArray>(*VulkanMgr::instance, 3*sizeof(float));
        d.vertexModel->createBindingEntry(3*sizeof(float));
        d.vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
        SetContractDesc contract;
        contract.name = "axisLine";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},
        };
        contract.expectedSets = 1; // one shared set (old static Axis::set)
        PipelineFamilyDesc desc;
        desc.name = "AXIS";
        desc.vertex = d.vertexModel.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "body_Axis.vert.spv", .frag = "body_Axis.frag.spv"}}};
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        color.state.lineWidth = 3.f;
        desc.passes.push_back(std::move(color));
        desc.pushConstants = {{VK_SHADER_STAGE_VERTEX_BIT, 0,
                               static_cast<uint16_t>(sizeof(Mat4f) + sizeof(Vec3f))}};
        d.family = renderer.allocateFamily(std::move(desc));
        d.uColor = std::make_unique<SharedBuffer<Vec3f>>(*Context::instance->uniformMgr);
        **d.uColor = Vec3f(1.f, 0.f, 0.f); // old fixed red (axis.cpp:137)
        d.set = renderer.allocSet(d.family, 0);
        d.set->bindUniform(*d.uColor, 0);
        d.set->update();
        Context::onManagerTeardown([] {
            axisFamily().uColor.reset();
        });
        return d;
    }();
    return data;
}
} // namespace

void AxisModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!show)
        return;
    AxisFamilyData &data = axisFamily();
    const FamilyBound bound = renderer.bind(data.family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade, logged once
    if (!line) {
        // Old lazy per-body 2-vertex buffer (axis.cpp:43-46), tinyMgr-backed
        // persistent mapping - written every draw, read by the GPU this frame.
        line = data.vertexModel->createBuffer(0, 2, Context::instance->tinyMgr.get());
        pPos = static_cast<Vec3f *>(Context::instance->tinyMgr->getPtr(line->get()));
    }
    const float len = 1.4f * body->getScaledRadius();
    pPos[0] = Vec3f(0.f, 0.f, len);
    pPos[1] = Vec3f(0.f, 0.f, -len);
    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
    } pushData;
    pushData.ModelViewMatrix = mat;
    pushData.clipping_fov = renderer.getClippingFov();
    bound.layout->pushConstant(renderer, 0, &pushData);
    bound.layout->bindSet(renderer, *data.set);
    VertexArray::bind(renderer, line->get());
    vkCmdDraw(renderer, 2, 1, 0, 0);
}
