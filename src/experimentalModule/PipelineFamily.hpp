#ifndef PIPELINE_FAMILY_HPP_
#define PIPELINE_FAMILY_HPP_

#include <vulkan/vulkan.h>
#include <cstdint>

class VertexArray;
class PipelineLayout;

// ============================================================================
// Pipeline-family contract (INTENT.md §10.3) - the descriptive half of
// "modules DESCRIBE, the Renderer EXECUTES" (Renderer.hpp).
//
// A pipeline family is the named contract between a module kind and the
// Renderer: vertex format, descriptor contract, push-constant blocks and, per
// supported pass kind, a shader set + fixed-state profile, plus variant axes
// (feature bits). The Renderer owns the registry and every Pipeline object;
// modules hold a PipelineFamilyID and call Renderer::bind() inside their draw
// hooks, then record their own geometry (bind-and-record). This replaces
// SHADER_USE/selectShader and the drawState_t bank (bodyShader.hpp): what was
// enum arithmetic (pipelineOffset, pipelineNoDepth = pipeline+2) becomes
// declared variant bits.
//
// Residency ladder (mirrors the texture LoD ladder, D6 - something drawable
// always exists):
// - the base variant (VariantKey 0) of every declared pass kind is built
//   synchronously at registration - base-always-ready (C3 corollary);
// - non-base variants are built lazily in the work domain and published to
//   the registry as a render-chain task (C1); never the cross-product (D5).
// - bind() never blocks: it binds the best RESIDENT variant covered by the
//   wanted key (greedy bit-drop by VariantAxis::dropPriority) and enqueues
//   the missing build; FamilyBound::got reports what actually bound.
// Config-driven rebuilds (tesselation level, dynamic resolution) follow the
// same path: rebuild in the work domain, publish swap; EntityCore defers old
// pipeline destruction by 3 VulkanMgr::update() - frames in flight covered.
// ============================================================================

// The four drawing types (BodyModule.hpp hooks), 1:1. NO-DEPTH is NOT a pass
// kind: within the main pass, depth-less drawing is pipeline state (the old
// path's pipelineNoDepth), so it is the reserved variant bit VARIANT_NO_DEPTH.
// The Renderer owns the PassKind -> (render pass, subpass, state profile,
// command stream) table:
//   COLOR          -> (render, subpass 0)
//   SELF_SHADOW    -> (renderSelfShadow, 0), depth GREATER, dynamic viewport
//                     (variable-size targets: MAIN/SECONDARY self-shadow
//                     resolutions, ModularBody.hpp constants)
//   SHADOW_STENCIL -> (renderShadow, 0), stencil REPLACE
//   TRACE          -> (render, 0), vertex-only depth stream (the orbit-hole
//                     analog of the old cmdBodyDepth buffer)
// bind() resolves against the CURRENT pass kind - a module structurally
// cannot bind a COLOR pipeline while TRACE records; hooks are only invoked
// within their matching pass kind, routed by the module's BMT_* traits
// (a trait not declared = a pass never received, BodyModule.hpp).
// For TRACE and the shadow passes the Renderer binds the service pipeline
// once per stream; a module's drawTrace/drawShadow only pushes constants and
// draws (BasicMesh::drawTrace already has this shape).
enum class PassKind : uint8_t {
    COLOR,
    SELF_SHADOW,
    SHADOW_STENCIL,
    TRACE,
    NB_PASS_KIND
};

// Feature bits of a family. 0 = base variant, always resident.
// Bits 0x0001..0x4000 are family-declared (VariantAxis). Reserved bits carry
// renderer-owned semantics valid across families; a family not supporting a
// reserved bit sees it dropped like any other non-providable bit (this
// reproduces e.g. the old ringed-body force-depth behavior).
using VariantKey = uint16_t;
constexpr VariantKey VARIANT_NO_DEPTH = 0x8000; // depth test+write off; COLOR pass only (drawNoDepth hook)

// One declared feature bit and what setting it does to the build.
struct VariantAxis {
    const char *name;   // debug + pipelineCache naming ("clouds", "notex", ...)
    VariantKey bit;     // the single non-reserved bit this axis owns
    enum Effect : uint8_t {
        SPEC_CONSTANT,  // boolean specialization constant = bit state
        SHADER_SWAP,    // selects the ShaderVariant table entry (see PassDesc)
        STATE_OVERRIDE  // FixedState delta - payload [open]: defined with its
                        // first client (today's only state variant, no-depth,
                        // is the reserved bit above)
    } effect;
    uint8_t dropPriority;   // fallback order: HIGHER drops first when the
                            // combination is non-resident or non-providable
    uint8_t specConstantId; // SPEC_CONSTANT only
};

// Shader stage set; nullptr = stage absent.
struct ShaderSet {
    const char *vert = nullptr;
    const char *tesc = nullptr;
    const char *tese = nullptr;
    const char *geom = nullptr;
    const char *frag = nullptr;
};

// One entry of a pass's shader table: the shaders used when the requested
// variant's SHADER_SWAP bits equal shaderBits (SPEC_CONSTANT/STATE bits never
// select shaders). Entry 0 must cover shaderBits == 0 (the base). A
// combination absent from the table is not providable - its bits drop in
// fallback. Porting note: today's distinct shader files (night/bump/tes...)
// enter as SHADER_SWAP entries unchanged; a future layered mesh shader
// collapses them into SPEC_CONSTANT axes with no API change.
struct ShaderVariant {
    VariantKey shaderBits;
    ShaderSet shaders;
};

// Fixed pipeline state of one pass of a family (the subset the old path
// actually exercises; extend only when a client forces it).
struct FixedState {
    const VkPipelineColorBlendAttachmentState *blend = nullptr; // nullptr = opaque (BLEND_NONE)
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool stripBreaks = false;       // primitive restart (Tail-style strips)
    bool cull = true;
    bool reverseFrontFace = false;  // tesselated bodies use reversed winding
    bool depthTest = true;
    bool depthWrite = true;
    uint8_t patchControlPoints = 0; // >0 implies PATCH_LIST + tessellation
    uint8_t removedVertexEntries = 0; // bitmask of vertex locations stripped
                                      // (removeVertexEntry pattern: depth/sun
                                      // variants drop Tex2D/Normal3D)
    float lineWidth = 0;            // 0 = Pipeline default
};

// A pass kind this family participates in.
struct PassDesc {
    PassKind pass;
    const ShaderVariant *shaderTable; // entry 0 = base (shaderBits == 0)
    uint8_t shaderTableSize;
    FixedState state;
};

// One binding of a FAMILY_OWNED set - declarative, so the same data that
// builds the layout also sizes the descriptor pools: a pool is always sized
// from the layouts it serves, which structurally closes the class of error
// where a pool lacks a type its layouts use (INTENT §11.1: shadowLayout's
// SAMPLED_IMAGE vs SetMgr's hardcoded table).
struct SetBindingDesc {
    uint8_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stages;
    uint16_t arraySize = 1;
    const VkSamplerCreateInfo *sampler = nullptr; // COMBINED_IMAGE_SAMPLER: nullptr = default sampler
};

// Set roles:
// - FAMILY_OWNED: bindings declared here; Sets allocated via
//   Renderer::allocSet from the family-aggregated pool.
// - GLOBAL_UBO: appends the app-wide UBO set (context.uboSet) - the declared
//   form of the setGlobalPipelineLayout idiom.
// - EXTERNAL: reuses another component's set emplacement (the traceLayout
//   sharing pattern: ring depth-trace rides the body trace contract).
struct SetDesc {
    enum Role : uint8_t { FAMILY_OWNED, GLOBAL_UBO, EXTERNAL } role;
    const SetBindingDesc *bindings = nullptr; // FAMILY_OWNED
    uint8_t bindingCount = 0;
    PipelineLayout *external = nullptr;       // EXTERNAL: donor layout
    uint16_t expectedSets = 0; // FAMILY_OWNED pool-sizing input: expected live
                               // Set count (D5-scaled parameter, not a budget)
};

struct PushConstantDesc {
    VkShaderStageFlags stages;
    uint16_t offset;
    uint16_t size;
};

// Device- or config-derived specialization values, distinct from variant
// bits: the registrant supplies them at registration (isFloat64Supported,
// viewport height, self-shadow resolution, ...). A config change re-supplies
// them and triggers the work-domain rebuild + publish swap of the family's
// built pipelines - never an in-frame stall.
struct SpecConstant {
    uint8_t constantId;
    uint32_t value;
};

// Batching service, declared by heavily-replicated families (BMT_REPLICATED).
// The Renderer owns per-family ring buffers; modules append instances with
// Renderer::batchPush (inline-hot). Flush cadence carries an OCCLUSION
// CONTRACT [vixy: 2026-07-11]: batched screen-space content (halo, hint) is
// occluded as if drawn farthest->closest - a nearer body, drawn later, covers
// the already-flushed content of farther bodies (bodies draw
// farthest->nearest; flush points are the per-body command-buffer
// boundaries). Relative order BETWEEN batched families inside one flush
// segment is unconstrained: halo may draw above hint even when behind,
// whenever it improves performance - only body-vs-batch occlusion is
// contractual. Shared geometry (Tail-style instanced strips) comes from the
// family's VertexArray instance-rate entries, not from this struct.
struct BatchDesc {
    uint16_t instanceStride;   // bytes per pushed instance
    uint16_t perFrameCapacity; // ring sizing (D5 parameter); overflow clamps
                               // and logs (old Halo::endDraw behavior)
};

using PipelineFamilyID = uint8_t;
constexpr PipelineFamilyID PIPELINE_FAMILY_NONE = UINT8_MAX;

// The full family description. Registered once (registerFamily dedups by
// name - per-instance builders like Sun/Ring dissolve into one family plus
// per-instance Sets); the desc and everything it points to must stay valid
// until registration returns (the registry copies what it keeps).
//
// HARD RULE - layout invariance: a variant axis must not change the
// descriptor contract. (family, pass) -> layout is constant across variants;
// a feature that changes bindings is a SEPARATE family (the old artificial
// vs artificialShadowed pair). This keeps Sets reusable across variants and
// pool aggregation valid.
struct PipelineFamilyDesc {
    const char *name;
    enum Kind : uint8_t { GRAPHICS, COMPUTE } kind = GRAPHICS;
    enum BuildPolicy : uint8_t {
        BASE_SYNC_LAZY_VARIANTS, // default: base at registration, variants on demand
        EAGER_ASYNC_ALL          // whole bank built in the work domain (the
                                 // shadow-blur bank: radius = variant key)
    } buildPolicy = BASE_SYNC_LAZY_VARIANTS;
    VertexArray *vertex = nullptr;  // nullptr = vertex-less, or COMPUTE
    const SetDesc *sets = nullptr;
    uint8_t setCount = 0;
    const PushConstantDesc *pushConstants = nullptr;
    uint8_t pushConstantCount = 0;
    const SpecConstant *specValues = nullptr; // device/config-derived; see SpecConstant
    uint8_t specValueCount = 0;
    const PassDesc *passes = nullptr; // GRAPHICS only
    uint8_t passCount = 0;
    const VariantAxis *axes = nullptr;
    uint8_t axisCount = 0;
    const BatchDesc *batch = nullptr; // non-null = batched family
    const char *computeShader = nullptr; // COMPUTE only; variants = spec constant
};

// bind() result: what to record against, and what actually bound.
struct FamilyBound {
    PipelineLayout *layout; // bind Sets / push constants against this
    VariantKey got;         // wanted minus dropped bits; got != wanted means a
                            // fallback drew this frame and builds are enqueued
};

#endif /* end of include guard: PIPELINE_FAMILY_HPP_ */
