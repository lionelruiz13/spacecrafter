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
        // 2023-master merge (INTENT 11.32): body_Axis.vert projects
        // IN-SHADER now (custom_project.glsl multi-mode dispatch) - the
        // interface is {ModelViewMatrix, clipping_fov} push constants +
        // object-space vec3 endpoints (old axis.cpp:118 layout mirror).
        // Spec constant 8 (projection mode) is REGISTRY-INJECTED on every
        // family since INTENT 11.33 - no per-family declaration needed.
        desc.pushConstants = {{VK_SHADER_STAGE_VERTEX_BIT, 0,
                               static_cast<uint16_t>(sizeof(Mat4f) + sizeof(Vec3f))}};
        d.family = renderer.allocateFamily(std::move(desc));
        d.uColor = std::make_unique<SharedBuffer<Vec3f>>(*Context::instance->uniformMgr);
        **d.uColor = Vec3f(1.f, 0.f, 0.f); // old fixed red (axis.cpp:137)
        d.set = renderer.allocSet(d.family, 0);
        d.set->bindUniform(*d.uColor, 0);
        d.set->update();
        // This data is a function-local static: it is destroyed at
        // __run_exit_handlers, long after ~Context destroyed uniformMgr, and
        // ~SharedBuffer releases into the BufferMgr reference it captured at
        // construction. That was INTENT 5.55 - one `flag planets_axis on` and
        // the process died at exit, on every binary. Hand the sub-allocation
        // back while the manager is alive (contract: context.hpp
        // onManagerTeardown). Only uColor is manager-owned: vertexModel holds
        // a VulkanMgr reference but releases nothing, `set` is registry-pool
        // owned (dropped by releaseRegistry), and ~PipelineFamily is guarded
        // on the reset registry (PipelineRegistry.cpp:613-630) - so what stays
        // for static teardown provably touches no dead manager.
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
    // Post-merge interface (INTENT 11.32; old computeAxis, axis.cpp:71-77):
    // OBJECT-SPACE endpoints (0,0,+-1.4*scaledRadius); projection runs
    // in-shader (custom_project - the old CPU atan-form fisheye this module
    // first ported was retired by 2023-master along with the CPU path).
    // The passed mat folds only the surface spin (z-rotation), which leaves
    // +-z invariant, so pushing it as ModelViewMatrix equals the old body
    // mat exactly. clipping_fov = the CURRENT depth slice (bucket range,
    // S3): the SAME mapping the disc's own draw used, so the z-test
    // axis-vs-disc stays exact by construction. Old used the scale-mutated
    // radius member -> getScaledRadius() here (moon_scale parity).
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
