#ifndef PIPELINE_FAMILY_HPP_
#define PIPELINE_FAMILY_HPP_

#include "EntityCore/Resource/Pipeline.hpp" // BLEND_NONE (single authority for blend presets)
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <utility>

class VertexArray;
class PipelineLayout;
class Renderer;

// ============================================================================
// Pipeline-family contract (INTENT.md §10.3) - the descriptive half of
// "modules DESCRIBE, the Renderer EXECUTES" (Renderer.hpp).
//
// Paradigm [vixy: 2026-07-11]: describe once, then refer by descriptor. A
// resource (set contract, pipeline family) is ALLOCATED from its description
// exactly once - a cold, one-time cost, which is why descriptions freely use
// owning types (std::string, std::vector): correctness by construction
// instead of externally-asserted lifetimes, at a cost that never recurs.
// What comes back is a descriptor - an integer index under the hood, wrapped
// in a handle that manages lifetime implicitly (copies share, destruction
// releases). Hot paths only ever carry the index. Descriptions moved into the
// registry stay available for lazy variant builds with no lifetime clause.
//
// A pipeline family is the named contract between a module kind and the
// Renderer: vertex format, referenced set contracts, push-constant blocks
// and, per supported pass kind, a shader set + fixed-state profile, plus
// variant axes (feature bits). The Renderer owns the registry and every
// Pipeline object; modules hold a PipelineFamily handle and call
// Renderer::bind() inside their draw hooks, then record their own geometry
// (bind-and-record). This replaces SHADER_USE/selectShader and the
// drawState_t bank (bodyShader.hpp): what was enum arithmetic
// (pipelineOffset, pipelineNoDepth = pipeline+2) becomes declared variant
// bits.
//
// Residency ladder (mirrors the texture LoD ladder, D6 - something drawable
// always exists):
// - the base variant (VariantKey 0) of every declared pass kind is built
//   synchronously at allocation - base-always-ready (C3 corollary);
// - non-base variants are built lazily in the work domain and published to
//   the registry as a render-chain task (C1); never the cross-product (D5).
// - bind() never blocks: it binds the best RESIDENT variant covered by the
//   wanted key (greedy bit-drop by VariantAxis::dropPriority) and enqueues
//   the missing build; FamilyBound::got reports what actually bound.
// Config-driven rebuilds (tesselation level, dynamic resolution, the
// SpecConstant values) follow the same path: rebuild in the work domain,
// publish swap; EntityCore defers old pipeline destruction by 3
// VulkanMgr::update() - frames in flight covered.
//
// Domains: allocation and handle release belong to the registration domain
// (module-loader registration path / events thread) - never the frame task;
// registry refcounts are plain integers under that domain's serialization
// (same reasoning as the §8.2.5 pin counters). bind()/batchPush() inside the
// frame task are registry-read-only.
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

// What setting an axis bit does to the build.
enum class VariantEffect : uint8_t {
    SPEC_CONSTANT,  // boolean specialization constant = bit state
    SHADER_SWAP,    // selects the ShaderVariant table entry (see PassDesc)
    STATE_OVERRIDE  // FixedState delta - payload [open]: defined with its
                    // first client (today's only state variant, no-depth, is
                    // the reserved bit above)
};

// One declared feature bit of a family.
struct VariantAxis {
    std::string name;       // debug + pipelineCache naming ("clouds", ...)
    VariantKey bit;         // the single non-reserved bit this axis owns
    VariantEffect effect;
    uint8_t dropPriority;   // fallback order: HIGHER drops first when the
                            // combination is non-resident or non-providable
    uint8_t specConstantId = 0; // SPEC_CONSTANT only
};

// Shader stage set; empty = stage absent.
struct ShaderSet {
    std::string vert;
    std::string tesc;
    std::string tese;
    std::string geom;
    std::string frag;
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
    VkPipelineColorBlendAttachmentState blend = BLEND_NONE;
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
    std::vector<ShaderVariant> shaderTable; // entry 0 = base (shaderBits == 0)
    FixedState state;
};

// One binding of a set contract - declarative, so the same data that builds
// the layout also sizes the descriptor pools: a pool is always sized from the
// layouts it serves, which structurally closes the class of error where a
// pool lacks a type its layouts use (INTENT §11.1: shadowLayout's
// SAMPLED_IMAGE vs SetMgr's hardcoded table).
struct SetBindingDesc {
    uint8_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stages;
    uint16_t arraySize = 1;
    std::optional<VkSamplerCreateInfo> sampler; // COMBINED_IMAGE_SAMPLER:
                                                // nullopt = default sampler
};

// A descriptor-set contract, allocated once via
// Renderer::allocateSetContract and referenced by descriptor from any number
// of families. This single mechanism covers what would otherwise be three
// cases: a family's own sets, the app-wide global UBO set
// (Renderer::globalUboContract()), and cross-family sharing (the old
// traceLayout pattern: ring depth-trace rides the body trace contract by
// holding the same SetContract).
struct SetContractDesc {
    std::string name;                     // debug + dedup identity
    std::vector<SetBindingDesc> bindings;
    uint16_t expectedSets = 0; // pool-sizing input: expected live Set count
                               // (D5-scaled parameter, not a budget)
};

struct PushConstantDesc {
    VkShaderStageFlags stages;
    uint16_t offset;
    uint16_t size;
};

// Device- or config-derived specialization values, distinct from variant
// bits: the registrant supplies them at allocation (isFloat64Supported,
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

// A registry descriptor: an integer index under the hood, wrapped so that
// lifetime is implicit - copies share (registry refcount), destruction
// releases, a description holding handles keeps what it references alive by
// construction. Default-constructed = null. acquire()/release() are defined
// with the registry (registration domain only - see the domain note above).
template <class Tag>
class RegistryHandle {
public:
    static constexpr uint8_t NONE = UINT8_MAX;
    RegistryHandle() = default;
    RegistryHandle(const RegistryHandle &other) : idx(other.idx) {
        acquire();
    }
    RegistryHandle(RegistryHandle &&other) noexcept : idx(other.idx) {
        other.idx = NONE;
    }
    RegistryHandle &operator=(RegistryHandle other) noexcept {
        std::swap(idx, other.idx);
        return *this;
    }
    ~RegistryHandle() {
        release();
    }
    explicit operator bool() const {
        return idx != NONE;
    }
    inline uint8_t id() const {
        return idx;
    }
private:
    friend class Renderer;
    explicit RegistryHandle(uint8_t idx) : idx(idx) {} // registry-issued
    void acquire(); // no-op on NONE; defined with the registry
    void release(); // no-op on NONE; defined with the registry
    uint8_t idx = NONE;
};

using SetContract = RegistryHandle<struct SetContractTag>;
using PipelineFamily = RegistryHandle<struct PipelineFamilyTag>;

// acquire()/release() are defined with the registry (PipelineRegistry.cpp);
// declared here so every TU that copies/destroys a handle refers to the same
// explicit specialization (required before use, [temp.expl.spec]).
template <> void RegistryHandle<SetContractTag>::acquire();
template <> void RegistryHandle<SetContractTag>::release();
template <> void RegistryHandle<PipelineFamilyTag>::acquire();
template <> void RegistryHandle<PipelineFamilyTag>::release();

// The full family description, moved into the registry at allocation
// (Renderer::allocateFamily). Same name = same underlying family (dedup:
// per-instance builders like Sun/Ring dissolve into one family plus
// per-instance Sets).
//
// HARD RULE - layout invariance: a variant axis must not change the
// descriptor contract. (family, pass) -> layout is constant across variants;
// a feature that changes bindings is a SEPARATE family (the old artificial
// vs artificialShadowed pair). This keeps Sets reusable across variants and
// pool aggregation valid.
struct PipelineFamilyDesc {
    std::string name;
    enum class Kind : uint8_t { GRAPHICS, COMPUTE };
    Kind kind = Kind::GRAPHICS;
    enum class BuildPolicy : uint8_t {
        BASE_SYNC_LAZY_VARIANTS, // default: base at allocation, variants on demand
        EAGER_ASYNC_ALL          // whole bank built in the work domain (the
                                 // shadow-blur bank: radius = variant key)
    };
    BuildPolicy buildPolicy = BuildPolicy::BASE_SYNC_LAZY_VARIANTS;
    // Referenced, not owned (I5: must outlive the family - VertexArray is
    // itself an allocated description, application-lifetime by convention).
    VertexArray *vertex = nullptr;  // nullptr = vertex-less, or COMPUTE
    // Set index in the shaders = position in this vector.
    std::vector<SetContract> sets;
    std::vector<PushConstantDesc> pushConstants;
    std::vector<SpecConstant> specValues;
    std::vector<PassDesc> passes;   // GRAPHICS only
    std::vector<VariantAxis> axes;
    std::optional<BatchDesc> batch; // engaged = batched family
    // ---- COMPUTE families (S5/G7) ----
    // A COMPUTE family is a BANK of compute pipelines keyed by an INTEGER
    // VariantKey VALUE - not bit-axes: the key is passed verbatim as
    // specialization constant id 0 (desc.specValues must therefore not use
    // id 0). The shadow-blur bank is the client: key = blur radius in pixels.
    // With EAGER_ASYNC_ALL, keys 1..eagerVariants are enqueued to the build
    // domain at allocation (the old path built its bank on 4 startup threads
    // - context.cpp:76-99; the interim builder thread replaces them).
    // Renderer::bindCompute() never blocks: a not-yet-built key reports
    // failure and the caller skips that dispatch (C3) - the old global
    // shadow_ready gate becomes per-key readiness, strictly finer.
    std::string computeShader;      // COMPUTE only
    uint16_t eagerVariants = 0;     // COMPUTE + EAGER_ASYNC_ALL: build keys 1..N
};

// bind() result: what to record against, and what actually bound.
struct FamilyBound {
    PipelineLayout *layout; // bind Sets / push constants against this
                            // (frame-scoped raw pointer - I5(a))
    VariantKey got;         // wanted minus dropped bits; got != wanted means a
                            // fallback drew this frame and builds are enqueued
};

#endif /* end of include guard: PIPELINE_FAMILY_HPP_ */
