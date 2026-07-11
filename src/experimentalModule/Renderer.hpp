#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "tools/vecmath.hpp"
#include <vulkan/vulkan.h>
#include <vector>

class ToneReproductor;
class FrameMgr;

// ============================================================================
// The Renderer is the ONLY Vulkan surface of the experimental module: bodies
// and modules DESCRIBE (which pass, which traits, which data), the Renderer
// EXECUTES (pipelines, command buffers, depth/stencil strategy). No module
// owns a pipeline or records into a raw command buffer on its own authority -
// this dissolves the old body<->pipeline coupling (SHADER_USE/selectShader):
// modules request pipeline families through their declared traits and
// descriptor contracts; the Renderer owns every drawState.
//
// All Renderer methods execute inside the render chain (RenderChain.hpp) -
// serialization comes from the chain, not from thread identity. Recording
// happens within the frame task, into per-frameIdx command buffers (3 frames
// in flight, per Context).
//
// Depth-range partitioning (the consumer of ModularBody::notableBody):
// child body size is negligible against parent size (D1) and body size is
// negligible against parent-child distance (D2), so NO single depth range can
// hold both - the full range must be SPLIT between the bodies that need a
// bucket, according to their needs. Only a handful of bodies are large enough
// on screen at any time (D3); notableBody lists exactly those, per frame, and
// must be drained every frame (see its contract in ModularBody.hpp).
// clearDepth(zCenter, boundingRadius) prepares the depth slice of one such
// body. The partitioning consumer is not implemented yet (INTENT.md 5.3/D4).
//
// The four pass kinds a module can receive (see BodyModule.hpp - gated by
// the module's BMT_* traits):
//   COLOR (draw / drawNoDepth) - visible image
//   SHADOW (drawShadow) - stencil map projected onto other bodies
//   SELF-SHADOW (drawSelfShadow) - self-shadow depth + grounded-slice prefill
//   TRACE (drawTrace) - depth hole hiding orbit lines behind the body
//
// Buffers (normative, moved from ModularBody.hpp):
// - Orbit depth buffer: depth bounds calibrated for the smallest system
//   visible [only update bound values] - it may go wrong though
// - Body depth buffer: cleared for each significant body (one slice per
//   notableBody entry - see partitioning above)
// - Self-shadowing depth buffer: large for the main body
//   (MAIN_SELF_SHADOWING_RESOLUTION), small for others (SECONDARY_...)
// - Shadow casting stencil buffer
//
// Shadow projection (normative, moved from ModularBody.hpp):
// - With outer orbiting ModularBody
// - Outer: DepthBuffer shared with grounded ModularBody for drawing and
//   shadowing
// - Surface: DepthBuffer split between grounded ModularBody, parent's depth
//   trace is drawn in each DepthBuffer (this is why drawSelfShadow doubles
//   as the grounded-slice prefill), project shadow with and between grounded
//   ModularBody
// Bodies of BodyType::MINOR_BODY never participate in inter-body shadowing.
// ============================================================================
class Renderer {
public:
    Renderer();
    void init(ToneReproductor *eye);
    // Begin recording for this frame index. Called from the frame task only.
    void beginDraw(uint8_t frameIdx);
    void beginBodyDraw();
    void endBodyDraw();
    // Prepare the depth buffer for drawing in the given depth range - one
    // slice of the partitioned depth range (see partitioning contract above).
    void clearDepth(float zCenter, float boundingRadius);
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    float adaptLuminance(float world_luminance) const;
    inline operator VkCommandBuffer() {
        return cmd;
    }
    inline const Vec3f &getClippingFov() const {
        return clippingFov;
    }
private:
    void allocateCommands();
    void nextCommandBuffer();
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    ToneReproductor *eye;
    FrameMgr *frame;
    std::vector<VkCommandBuffer> cmds[3];
    Vec3f clippingFov;
    uint16_t cmdIdx;
    uint8_t frameIdx;
};

#endif /* end of include guard: RENDERER_HPP_ */
