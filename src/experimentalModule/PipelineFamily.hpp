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

// Describe a pipeline family once, then refer to it by handle

// One per BodyModule draw hook, drawNoDepth being COLOR with VARIANT_NO_DEPTH
enum class PassKind : uint8_t {
    COLOR,
    SELF_SHADOW,
    SHADOW_SHAPE,
    TRACE,
    NB_PASS_KIND
};

// Feature bits of a family, 0 = base variant, always resident
using VariantKey = uint16_t;
constexpr VariantKey VARIANT_NO_DEPTH = 0x8000; // Depth test and write off, COLOR pass only

enum class VariantEffect : uint8_t {
    SPEC_CONSTANT, // Boolean specialization constant
    SHADER_SWAP, // Select the ShaderVariant entry
    STATE_OVERRIDE // Not implemented, the axis is masked out
};

struct VariantAxis {
    std::string name;
    VariantKey bit; // Single non-reserved bit
    VariantEffect effect;
    uint8_t dropPriority; // The higher is dropped first
    uint8_t specConstantId = 0; // SPEC_CONSTANT only
};

// Empty = stage absent
struct ShaderSet {
    std::string vert;
    std::string tesc;
    std::string tese;
    std::string geom;
    std::string frag;
};

// Shaders used when the SHADER_SWAP bits of the wanted variant equal shaderBits
struct ShaderVariant {
    VariantKey shaderBits;
    ShaderSet shaders;
};

struct FixedState {
    VkPipelineColorBlendAttachmentState blend = BLEND_NONE;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool stripBreaks = false; // Primitive restart
    bool cull = true;
    bool reverseFrontFace = false;
    bool depthTest = true;
    bool depthWrite = true;
    uint8_t patchControlPoints = 0; // > 0 implies PATCH_LIST and tessellation
    uint8_t removedVertexEntries = 0; // Bitmask of the vertex locations stripped
    float lineWidth = 0; // 0 = Pipeline default
};

struct PassDesc {
    PassKind pass;
    std::vector<ShaderVariant> shaderTable; // Entry 0 is the base (shaderBits == 0)
    FixedState state;
};

struct SetBindingDesc {
    uint8_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stages;
    uint16_t arraySize = 1;
    std::optional<VkSamplerCreateInfo> sampler; // nullopt = default sampler
};

// Allocated once, then shared between families
struct SetContractDesc {
    std::string name; // Same name = same contract
    std::vector<SetBindingDesc> bindings;
    uint16_t expectedSets = 0; // Expected live Set count for pool sizing, not a limit
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

// For Renderer::batchPush, the draw order between batched families is unspecified
struct BatchDesc {
    uint16_t instanceStride; // In bytes
    uint16_t perFrameCapacity; // In instances, the overflow is dropped and logged
};

// Shared registry reference, copy and destroy it in the registration path only
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
    explicit RegistryHandle(uint8_t idx) : idx(idx) {}
    void acquire(); // Both are no-op on NONE
    void release();
    uint8_t idx = NONE;
};

using SetContract = RegistryHandle<struct SetContractTag>;
using PipelineFamily = RegistryHandle<struct PipelineFamilyTag>;

template <> void RegistryHandle<SetContractTag>::acquire();
template <> void RegistryHandle<SetContractTag>::release();
template <> void RegistryHandle<PipelineFamilyTag>::acquire();
template <> void RegistryHandle<PipelineFamilyTag>::release();

// Same name = same family. An axis must not change the bindings, that is another family
struct PipelineFamilyDesc {
    std::string name;
    enum class Kind : uint8_t { GRAPHICS, COMPUTE };
    Kind kind = Kind::GRAPHICS;
    enum class BuildPolicy : uint8_t {
        BASE_SYNC_LAZY_VARIANTS, // Base at allocation, variants on demand
        EAGER_ASYNC_ALL // Whole bank built in the work domain
    };
    BuildPolicy buildPolicy = BuildPolicy::BASE_SYNC_LAZY_VARIANTS;
    VertexArray *vertex = nullptr; // Not owned, must outlive the family. nullptr = vertex-less
    std::vector<SetContract> sets; // Set index in the shaders = position
    std::vector<PushConstantDesc> pushConstants;
    std::vector<SpecConstant> specValues;
    std::vector<PassDesc> passes; // GRAPHICS only
    std::vector<VariantAxis> axes;
    std::optional<BatchDesc> batch;
    // With COMPUTE, the VariantKey is an integer passed as the reserved spec constant 0
    std::string computeShader; // COMPUTE only
    uint16_t eagerVariants = 0; // With EAGER_ASYNC_ALL, build the keys 1..N
};

// layout == nullptr: nothing was bound, record nothing
struct FamilyBound {
    PipelineLayout *layout; // Valid for this frame only
    VariantKey got; // wanted minus the dropped bits, whose builds are enqueued
};

#endif /* end of include guard: PIPELINE_FAMILY_HPP_ */
