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

enum class PassKind : uint8_t {
    COLOR,
    SELF_SHADOW,
    SHADOW_SHAPE,
    TRACE,
    NB_PASS_KIND
};

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

struct SetBindingDesc {
    uint8_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stages;
    uint16_t arraySize = 1;
    std::optional<VkSamplerCreateInfo> sampler; // COMBINED_IMAGE_SAMPLER:
                                                // nullopt = default sampler
};

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

struct SpecConstant {
    uint8_t constantId;
    uint32_t value;
};

struct BatchDesc {
    uint16_t instanceStride;   // bytes per pushed instance
    uint16_t perFrameCapacity; // ring sizing (D5 parameter); overflow clamps
                               // and logs (old Halo::endDraw behavior)
};

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
    std::string computeShader;      // COMPUTE only
    uint16_t eagerVariants = 0;     // COMPUTE + EAGER_ASYNC_ALL: build keys 1..N
};

struct FamilyBound {
    PipelineLayout *layout; // bind Sets / push constants against this
                            // (frame-scoped raw pointer - I5(a))
    VariantKey got;         // wanted minus dropped bits; got != wanted means a
                            // fallback drew this frame and builds are enqueued
};

#endif /* end of include guard: PIPELINE_FAMILY_HPP_ */
