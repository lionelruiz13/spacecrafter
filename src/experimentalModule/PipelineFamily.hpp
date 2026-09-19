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
class Pipeline;
class PipelineLayout;
class Renderer;

// A pipeline family is the contract between a module kind and the Renderer: described once, then referred to by handle
// The base variant of every declared pass is built at allocation, the others lazily; Renderer::bind() never blocks
// Allocation and handle release belong to the registration path, never the frame task

// One per BodyModule draw hook. Depth-less drawing is not a pass kind but the VARIANT_NO_DEPTH bit of COLOR
enum class PassKind : uint8_t {
    COLOR,
    SELF_SHADOW,
    SHADOW_SHAPE,
    TRACE,
    NB_PASS_KIND
};

// Feature bits of a family; 0 = base variant, always resident. 0x0001..0x4000 are declared by the family (VariantAxis),
// reserved bits mean the same for every family. A bit the family cannot provide is dropped
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

// The shaders used when the SHADER_SWAP bits of the wanted variant equal shaderBits; an absent combination is dropped
struct ShaderVariant {
    VariantKey shaderBits;
    ShaderSet shaders;
};

// Fixed pipeline state of one pass of a family; extend only when a client forces it
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

// One binding of a set contract; the same data builds the layout and sizes the descriptor pools
struct SetBindingDesc {
    uint8_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stages;
    uint16_t arraySize = 1;
    std::optional<VkSamplerCreateInfo> sampler; // COMBINED_IMAGE_SAMPLER:
                                                // nullopt = default sampler
};

// Allocated once (Renderer::allocateSetContract), then shared by any number of families holding the SetContract
struct SetContractDesc {
    std::string name;                     // debug + dedup identity
    std::vector<SetBindingDesc> bindings;
    uint16_t expectedSets = 0; // pool-sizing input: expected live Set count, not a budget
};

struct PushConstantDesc {
    VkShaderStageFlags stages;
    uint16_t offset;
    uint16_t size;
};

// Device- or config-derived specialization value supplied by the registrant, distinct from the variant bits
struct SpecConstant {
    uint8_t constantId;
    uint32_t value;
};

// Batching service of a heavily replicated family (Renderer::batchPush). Batched content is occluded by bodies
// as if everything was drawn farthest->closest; the order between batched families is free
struct BatchDesc {
    uint16_t instanceStride;   // bytes per pushed instance
    uint16_t perFrameCapacity; // ring sizing; overflow clamps and logs
};

// Registry descriptor: copies share, destruction releases, default = null. Copy and destroy in the registration path
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

template <> void RegistryHandle<SetContractTag>::acquire();
template <> void RegistryHandle<SetContractTag>::release();
template <> void RegistryHandle<PipelineFamilyTag>::acquire();
template <> void RegistryHandle<PipelineFamilyTag>::release();

// Moved into the registry by Renderer::allocateFamily; same name = same family
// A variant axis must not change the descriptor contract: a feature which changes the bindings is a separate family
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
    // Referenced, not owned: must outlive the family
    VertexArray *vertex = nullptr;  // nullptr = vertex-less, or COMPUTE
    // Set index in the shaders = position in this vector.
    std::vector<SetContract> sets;
    std::vector<PushConstantDesc> pushConstants;
    std::vector<SpecConstant> specValues;
    std::vector<PassDesc> passes;   // GRAPHICS only
    std::vector<VariantAxis> axes;
    std::optional<BatchDesc> batch; // engaged = batched family
    // A COMPUTE family is a bank of pipelines keyed by the integer value of VariantKey, passed as specialization
    // constant 0 (which specValues must not use)
    std::string computeShader;      // COMPUTE only
    uint16_t eagerVariants = 0;     // COMPUTE + EAGER_ASYNC_ALL: build keys 1..N
};

// layout == nullptr: the pass is unavailable for this family and nothing was bound; the caller records nothing
struct FamilyBound {
    PipelineLayout *layout; // bind Sets / push constants against this; valid for this frame only
    VariantKey got;         // wanted minus dropped bits; got != wanted means a
                            // fallback drew this frame and builds are enqueued
};

#endif /* end of include guard: PIPELINE_FAMILY_HPP_ */
