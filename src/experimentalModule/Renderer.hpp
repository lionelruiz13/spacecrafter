#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "tools/vecmath.hpp"
#include "PipelineFamily.hpp"
#include <vulkan/vulkan.h>
#include <vector>

class ToneReproductor;
class FrameMgr;
class Set;

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
// Pipeline ownership and selection live in the pipeline-family registry
// (contract: PipelineFamily.hpp; design: INTENT.md §10.3). drawNoDepth is the
// COLOR pass with the reserved VARIANT_NO_DEPTH variant bit - depth-less
// drawing is pipeline state within the pass, not a pass of its own.
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
    // boundingRadius is inclusive by definition (smallest sphere enclosing the
    // whole traced body, BodyModule.hpp) - the slice [zCenter-r, zCenter+r]
    // needs no extra margin; the old path's 1.1 factor compensated a radius
    // that wasn't defined as inclusive.
    void clearDepth(float zCenter, float boundingRadius);
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    float adaptLuminance(float world_luminance) const;
    inline operator VkCommandBuffer() {
        return cmd;
    }
    inline const Vec3f &getClippingFov() const {
        return clippingFov;
    }

    // ---- Pipeline-family registry (contract: PipelineFamily.hpp) ----------
    // Describe once, refer by descriptor: descriptions are moved in at
    // allocation (one-time cold cost), handles manage lifetime implicitly.
    // Allocation/release ride the registration domain (module-loader
    // registration path / events thread), never the frame task. Pipeline
    // builds run in the work domain (pipelineCache use is concurrency-legal;
    // precedent: Context::initShadowStructures); publication of a built
    // variant into the registry is a render-chain task (C1).
    // Allocate a descriptor-set contract, shareable across families.
    SetContract allocateSetContract(SetContractDesc &&desc);
    // The pre-allocated app-wide UBO set contract (context.uboSet).
    SetContract globalUboContract() const;
    // Allocate a family; dedup by desc.name (same name = same underlying
    // family, lifetime = union of live handles).
    PipelineFamily allocateFamily(PipelineFamilyDesc &&desc);
    // Bind the best resident variant of `family` for the CURRENT pass kind
    // into the frame command buffer, then record your geometry against the
    // returned layout (bind-and-record). Registry-read-only, never blocks
    // (C3): non-resident variant bits are dropped for this frame (by
    // dropPriority) and their build is enqueued - FamilyBound::got reports
    // what actually bound. Only callable inside a draw hook (frame task).
    FamilyBound bind(const PipelineFamily &family, VariantKey wanted = 0);
    // Allocate a Set of the family's set contract `setIndex`, from pools
    // sized by the aggregate of allocated contracts (a pool always covers
    // the layouts it serves - INTENT §11.1 structural fix). The big-texture
    // generation rebind idiom (set.uninit() + rebind on texmap change) is
    // legal on these Sets: frees are deferred by the SetMgr.
    Set *allocSet(const PipelineFamily &family, uint8_t setIndex);
    // Append one instance to a batched family. Flush cadence carries the
    // occlusion contract (PipelineFamily.hpp BatchDesc): as-if
    // farthest->closest against bodies; batch-vs-batch order free.
    void batchPush(const PipelineFamily &family, const void *instance);
    // The pass kind currently recording; bind() resolves against it, hooks
    // are only invoked within their matching pass kind (trait-routed).
    inline PassKind currentPassKind() const {
        return passKind;
    }

    // ---- Text/label channel: OPEN (INTENT §10.3 open #3, reworked) --------
    // The first design carried Projector/Navigator here; INVALIDATED
    // [vixy: 2026-07-11]: those are old-path projection machinery (system-
    // center reference), the new path projects through the parent<->child
    // matrix chain routed via the closest common parent. The gravity-text
    // path must be partially rewritten against the new chain; the channel
    // modules use to reach it will be defined by the projection-path
    // investigation (INTENT §11).
private:
    void allocateCommands();
    void nextCommandBuffer();
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    ToneReproductor *eye;
    FrameMgr *frame;
    PassKind passKind = PassKind::COLOR;
    std::vector<VkCommandBuffer> cmds[3];
    Vec3f clippingFov;
    uint16_t cmdIdx;
    uint8_t frameIdx;
};

#endif /* end of include guard: RENDERER_HPP_ */
