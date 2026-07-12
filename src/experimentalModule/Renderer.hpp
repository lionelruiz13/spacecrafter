#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "tools/vecmath.hpp"
#include "PipelineFamily.hpp"
#include "ShadowService.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class ToneReproductor;
class FrameMgr;
class Set;
class s_font;
class s_texture;
class VertexBuffer;

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
    // Tears down the pipeline-family registry (pipelines, layouts, pools,
    // interim builder thread) - runs at Context destruction, i.e. while the
    // Vulkan device is still alive (Renderer is Context's first member, hence
    // destroyed last among Context members, before VulkanMgr).
    ~Renderer();
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
    // Queue a halo through the Renderer batching service (HALO service
    // family - the Halo::global borrow is dissolved, 2026-07-12 S1(c)).
    // Occlusion follows the batched screen-space contract
    // (PipelineFamily.hpp BatchDesc): flush at the per-body boundaries.
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    // Halo texture seam: mirrors Body::setTexHaloMap (solarsystem_tex sets
    // "planethalo.png" at init) - the service loads its own s_texture
    // (texCache dedups by name); rebind happens at the next batchBegin.
    void setHaloTexture(const std::string &texName);
    // Queue a hint circle at a body's screen position (rect space [-1,1], i.e.
    // ModularBody::getScreenPos) through the HINT batched service family -
    // the DrawHelper DRAW_HINT_POS seam entry is dissolved (2026-07-12, row 6
    // completion; the halo half was dissolved at S1). Occlusion contract:
    // flush at the per-body boundaries (PipelineFamily.hpp BatchDesc).
    // color.a carries the per-body fader interstate (per-vertex color - the
    // reason the batched shader variant exists: push constants cannot batch
    // per-body colors).
    void drawHint(const std::pair<float, float> &pos, const Vec4f &color);
    // Selection pointer (old ObjectBase::drawPointer, OBJECT_BODY case -
    // same shaders, POINTER service family): four animated corner brackets
    // around the selected body. pos is rect space [-1,1]; sizePx is the
    // body's on-screen FULL diameter in render pixels (screenSize * 2 *
    // viewportRadius - the old getOnScreenSize form). Old rules carried
    // here, not by the caller: visibility flag, the 10%-of-viewport
    // suppression (a large disc needs no pointer), breathing animation
    // (20 + 10*sin(0.002 t), t advanced by ModularBody::deltaTime), and the
    // sqrt(viewportHeight/fontResolution) resolution scale. Queued content
    // records at endBodyDraw AFTER every body and batch flush = always on
    // top, old draw order (pointer drew after the whole system).
    // Call once per frame at most (single selected body by construction).
    void drawPointer(const std::pair<float, float> &pos, float sizePx);
    // Old Core::object_pointer_visibility mirror (set through
    // Core::setFlagSelectedObjectPointer - the single choke point).
    static bool showPointer;
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
    // ---- Service-window binds (S5/G7) -------------------------------------
    // bind() is frame-task API (current pass kind, frame cmd). Shadow
    // production records in the PRE-COLOR window on the helper thread
    // (DrawHelper::submit - where the old path records its shadow passes),
    // with its own cmd: these entries take pass and cmd explicitly. Registry
    // access is read-only there (slots are write-once + atomic ready flag) -
    // legal off the frame task by construction.
    FamilyBound bindIn(const PipelineFamily &family, PassKind pass, VkCommandBuffer cmd, VariantKey wanted = 0);
    // COMPUTE bank bind: key = integer variant (spec constant 0, see
    // PipelineFamilyDesc). Returns nullptr layout if that key's pipeline is
    // not resident yet - the caller skips the dispatch (C3), never waits.
    FamilyBound bindCompute(const PipelineFamily &family, VariantKey key, VkCommandBuffer cmd);
    // Non-binding residency query for a COMPUTE bank key (acquire-time gate:
    // don't hand out a layer whose blur cannot run this frame).
    bool computeReady(const PipelineFamily &family, VariantKey key) const;

    // ---- Shadow service (S5/G7 - contract in ShadowService.hpp) -----------
    // Renderer-owned per the normative buffer/projection block above; the
    // ModularSystem orchestration produces into it, receiver modules sample
    // its layer array. released by releaseRegistry() (managers alive).
    ShadowService shadow;
    // The pass kind currently recording; bind() resolves against it, hooks
    // are only invoked within their matching pass kind (trait-routed).
    inline PassKind currentPassKind() const {
        return passKind;
    }

    // ---- Text service (INTENT §10.3 open #3 RESOLVED - projection-paths C7,
    // accepted at plan approval 2026-07-12) ------------------------------------
    // Gravity-oriented label at an already-projected anchor. Pure function of
    // (anchor, viewport geometry, font, string): NO Projector, NO Navigator,
    // no frame context - printGravity180's frame-matrix dependency was
    // apparent, not real (verified: its math consumes only the pixel anchor,
    // viewport center/radius and the font - projector.cpp:393-415).
    // - pos is rect space [-1,1] (ModularBody::getScreenPos - the same anchor
    //   the halo and hint circle use, so label/circle/halo coincide by
    //   construction).
    // - Viewport geometry = the VulkanMgr scissor rect: the SAME rect
    //   rectToRender maps into and the fisheye projection is centered on -
    //   one geometry authority (I2). Old-path equivalence by construction:
    //   Projector::setViewportDisk builds the centered square t=min(w,h),
    //   so its center (w/2,h/2) and radius t/2 equal the scissor's.
    // - shifts are pixels, applied in the rotated (gravity) frame; the old
    //   hint label passes (10 + onScreenSize/2) for both.
    // Drawing is a delegation to s_font::print -> DrawHelper (text is not a
    // pipeline family - §10.2); legal from any draw hook, same channel and
    // segment semantics as drawHint. NOT batched by this class: DrawHelper
    // segments already carry the occlusion contract for screen-space content.
    void printGravity(s_font *font, const std::pair<float, float> &pos,
                      const std::string &str, const Vec4f &color,
                      float xshift, float yshift);

    // Explicit registry teardown - called FIRST in Context::~Context (all
    // Context members still alive: staging release, s_texture destruction
    // and pipeline destruction all need live managers). ~Renderer keeps a
    // no-op safety net for the case Context::~Context changes.
    void releaseRegistry();

private:
    // ---- Batching service internals (defined in PipelineRegistry.cpp) ----
    // Rebind batch resources that changed (e.g. halo texture) - frame start.
    void batchBegin();
    // Record every batched family's pending instances into the current cmd
    // (per-body boundary: portage of Halo::nextDraw semantics - the flush
    // lands at the START of the next body's command buffer, so a body's own
    // batched content draws over its disc and behind every nearer body).
    void batchFlush();
    // Frame end: plan the staging->vertex transfer of the drawn region and
    // ping-pong the buffer halves (portage of Halo::endDraw bookkeeping).
    void batchEnd();
    void ensureHaloFamily();
    void ensureHintFamily();
    // Built at init() (base-sync at startup, C3-clean; the in-frame ensure
    // calls are first-use fallbacks only, like the halo's).
    void ensurePointerFamily();
    // Record the queued pointer (if any) into the current cmd - called by
    // endBodyDraw after the trailing batchFlush: on top of everything.
    void recordPointer();
    void allocateCommands();
    void nextCommandBuffer();
    PipelineFamily haloFamily;
    PipelineFamily hintFamily;
    PipelineFamily pointerFamily;
    // Pointer service resources (old ObjectBase statics, Renderer-owned;
    // released by releaseRegistry() while the managers are alive).
    std::unique_ptr<VertexBuffer> pointerVertex; // 4 corners, planCopy'd per use
    std::unique_ptr<s_texture> pointerTex;       // pointer_planet.png
    std::unique_ptr<Set> pointerSet;
    float pointerTimeMs = 0;   // breathing clock (old ObjectBase::local_time)
    bool pointerQueued = false;
    struct { float x, y, size; } pointerData; // render px, resolved size
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
