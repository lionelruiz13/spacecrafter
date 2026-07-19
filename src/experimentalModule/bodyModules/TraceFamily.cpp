#include "TraceFamily.hpp"
#include "experimentalModule/Renderer.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/VertexArray.hpp"

namespace {
// Common description of a trace family: one vertex-only depth pipeline in the
// body subpass, position-only, spec-const 8 registry-injected. Only vertex +
// topology differ between the sphere and ring variants.
PipelineFamily buildTrace(const char *name, VertexArray *vertex,
                          VkPrimitiveTopology topology, uint8_t removedEntries)
{
    Renderer &renderer = Context::instance->renderer;
    PipelineFamilyDesc desc;
    desc.name = name;
    desc.vertex = vertex;
    // Old depthTrace pushed depthTraceInfo in the vertex stage only
    // (bodyShader.cpp:371). One block, offset 0.
    desc.pushConstants = {{VK_SHADER_STAGE_VERTEX_BIT, 0,
                           static_cast<uint16_t>(sizeof(TraceInfo))}};
    // my_atan in custom_project.glsl reads float64 support at spec 7 (old
    // depthTrace set it, bodyShader.cpp:383). Spec 8 (projection mode) is
    // registry-injected (INTENT §11.33) - no per-family declaration.
    desc.specValues = {{7, Context::instance->isFloat64Supported}};
    PassDesc trace;
    trace.pass = PassKind::TRACE;
    // Vertex-only: no fragment shader (depth-only write, old depthTrace had no
    // frag either). custom_project.glsl handles the projection in-shader.
    trace.shaderTable = {{0, {.vert = "body_depth_trace.vert.spv"}}};
    // Old fixed state (bodyShader.cpp:375-382 / ring.cpp:134-138): cull on,
    // BLEND_NONE, depth test+write ON (the whole point - it writes the hole).
    trace.state.topology = topology;
    trace.state.removedVertexEntries = removedEntries;
    desc.passes.push_back(std::move(trace));
    return renderer.allocateFamily(std::move(desc));
}
} // namespace

const PipelineFamily &TraceFamily::sphere()
{
    static PipelineFamily family = buildTrace(
        "TRACE_SPHERE", Context::instance->ojmVertexArray.get(),
        // Old depthTrace: TRIANGLE_LIST, removeVertexEntry(1) + (2) (drop the
        // ojm Tex2D + Normal3D attributes - only position feeds the disc).
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, (1 << 1) | (1 << 2));
    return family;
}

const PipelineFamily &TraceFamily::ring(VertexArray *ringVertex)
{
    // First caller (RingModule::drawTrace) supplies the ring VertexArray; the
    // family dedups by name so later callers get the same one (the arg is
    // ignored after the first build - the ring vertex is application-lifetime).
    static PipelineFamily family = buildTrace(
        "TRACE_RING", ringVertex,
        // Old ring depth trace: TRIANGLE_STRIP, removeVertexEntry(1) (drop the
        // ring's tex-coord, keep position - ring.cpp:136-138).
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, (1 << 1));
    return family;
}
