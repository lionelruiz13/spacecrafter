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

// AXIS line family - port of the old Axis static pipeline (axis.cpp:112-142)
// onto the registry: body_Axis shaders VERBATIM (parity by construction),
// LINE_STRIP, line width 3, ONE shared frag uniform carrying the color (the
// old uColor: a single static red {1,0,0} shared by every body - per-body
// color would diverge from old behavior, so the Set is family-level, not
// per-module). First line-class family: ORBIT/TRAIL/GRID reuse the shape
// with their own contracts (separate families where contracts differ -
// INTENT §10.1).
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
        // Old fixed state (axis.cpp:128-134): LINE_STRIP topology, line width
        // 3, default blend/cull/depth (depth test+write on - the axis hides
        // behind its own disc through the shared bucket depth mapping).
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        color.state.lineWidth = 3.f;
        desc.passes.push_back(std::move(color));
        d.family = renderer.allocateFamily(std::move(desc));
        d.uColor = std::make_unique<SharedBuffer<Vec3f>>(*Context::instance->uniformMgr);
        **d.uColor = Vec3f(1.f, 0.f, 0.f); // old fixed red (axis.cpp:137)
        d.set = renderer.allocSet(d.family, 0);
        d.set->bindUniform(*d.uColor, 0);
        d.set->update();
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
    // CPU fisheye, old Axis::computeAxis VERBATIM in float (axis.cpp:62-80):
    // endpoints at body-frame (0,0,+-1.4*scaledRadius) - the passed mat folds
    // the surface spin (z-rotation), which leaves +-z invariant, so this
    // equals the old body mat exactly. clipping = the CURRENT depth slice
    // (bucket range, S3): the SAME mapping the disc's own draw used, so the
    // z-test axis-vs-disc is exact by construction. Old used the scale-
    // mutated radius member -> getScaledRadius() here (moon_scale parity).
    const Vec3f &clip = renderer.getClippingFov();
    const float len = 1.4f * body->getScaledRadius();
    for (int i = 0; i < 2; ++i) {
        Vec3f pos = mat * Vec3f(0.f, 0.f, i ? -len : len);
        const float rq1 = pos[0]*pos[0] + pos[1]*pos[1];
        const float depth = (sqrtf(rq1 + pos[2]*pos[2]) - clip[0]) / (clip[1] - clip[0]);
        pos /= sqrtf(rq1) + 1e-30f; // don't divide by zero (old comment kept)
        const float f = (atanf(pos[2]) / static_cast<float>(M_PI) + 0.5f) * static_cast<float>(M_PI) / clip[2];
        pPos[i] = Vec3f(pos[0] * f, pos[1] * f, depth);
    }
    bound.layout->bindSet(renderer, *data.set);
    VertexArray::bind(renderer, line->get());
    vkCmdDraw(renderer, 2, 1, 0, 0);
}
