#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <forward_list>

class VirtualSurface;
class VertexBuffer;
class VertexArray;
class PipelineLayout;

const VkPipelineColorBlendAttachmentState BLEND_NONE {VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
const VkPipelineColorBlendAttachmentState BLEND_SRC_ALPHA {VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
const VkPipelineColorBlendAttachmentState BLEND_DST_ALPHA {VK_TRUE, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

class Pipeline {
public:
    Pipeline(VirtualSurface *master, PipelineLayout *layout, std::vector<VkDynamicState> _dynamicStates = {});
    ~Pipeline();
    void bindShader(const std::string &filename, const std::string entry = "main");
    void bindShader(const std::string &filename, VkShaderStageFlagBits stage, const std::string entry = "main");
    void bindVertex(VertexArray *vertex, uint32_t binding = 0);
    void bindVertex(VertexBuffer &vertex, uint32_t binding);
    void setCullMode(bool enable) {rasterizer.cullMode = enable ? VK_CULL_MODE_BACK_BIT : 0;}
    //! Set draw topology
    void setTopology(VkPrimitiveTopology state, bool enableStripBreaks = false);
    //! Set blend mode
    void setBlendMode(const VkPipelineColorBlendAttachmentState &blendMode);
    //! Set depth test mode
    void setDepthStencilMode(VkBool32 enableDepthTest, VkBool32 enableDepthWrite, VkCompareOp compare = VK_COMPARE_OP_GREATER, VkBool32 enableDepthBiais = VK_FALSE, float depthBiasClamp = 0.0f, float depthBiasSlopeFactor = 1.0f);
    //! Build pipeline for use
    void build();
    VkPipeline &get() {return graphicsPipeline;}
    static void setShaderDir(const std::string &_shaderDir) {shaderDir = _shaderDir;}
private:
    static std::string shaderDir;
    VirtualSurface *master;
    VkPipeline graphicsPipeline;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};

    std::forward_list<std::string> pNames;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

#endif /* end of include guard: PIPELINE_HPP */
