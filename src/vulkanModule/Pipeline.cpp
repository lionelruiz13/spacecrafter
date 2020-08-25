#include "VirtualSurface.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "VertexBuffer.hpp"
#include <fstream>

Pipeline::Pipeline(VirtualSurface *_master, PipelineLayout *layout, std::vector<VkDynamicState> _dynamicStates) : master(_master)
{
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optionnel
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optionnel
    colorBlending.blendConstants[1] = 0.0f; // Optionnel
    colorBlending.blendConstants[2] = 0.0f; // Optionnel
    colorBlending.blendConstants[3] = 0.0f; // Optionnel

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Tout élément trop loin ou trop près est ramené à la couche la plus loin ou la plus proche
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Désactiver la géométrie
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = 0;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optionnel
    rasterizer.depthBiasClamp = 0.0f; // Optionnel
    rasterizer.depthBiasSlopeFactor = 1.0f; // Optionnel

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Possibilité d'interrompre les liaisons entre les vertices pour les modes _STRIP
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pViewportState = &master->getViewportState();

    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = &depthStencil; // Optionnel
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optionnel
    pipelineInfo.layout = layout->getPipelineLayout();
    pipelineInfo.renderPass = _master->refRenderPass[1];
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optionnel
    pipelineInfo.basePipelineIndex = -1; // Optionnel

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optionnel
    depthStencil.maxDepthBounds = 1.0f; // Optionnel
    depthStencil.stencilTestEnable = VK_FALSE;
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(master->refDevice, graphicsPipeline, nullptr);
}

void Pipeline::bindShader(const std::string &filename, VkShaderStageFlagBits stage, const std::string entry)
{
    if (stage & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) && master->getDeviceFeatures().tessellationShader == VK_FALSE)
        return;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    char *buffer = new char [fileSize + sizeof(uint32_t)];

    file.seekg(0);
    file.read(buffer, fileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fileSize;
    createInfo.pCode = reinterpret_cast<uint32_t*>(buffer);

    VkPipelineShaderStageCreateInfo tmp{};
    tmp.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tmp.stage = stage;
    tmp.pSpecializationInfo = nullptr; // define constant values if any

    pNames.push_front(entry);
    tmp.pName = pNames.front().c_str();

    if (vkCreateShaderModule(master->refDevice, &createInfo, nullptr, &tmp.module) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création d'un module shader!");
    }
    delete buffer;
    shaderStages.push_back(tmp);
}

void Pipeline::setTopology(VkPrimitiveTopology state, bool enableStripBreaks)
{
    inputAssembly.topology = state;
    inputAssembly.primitiveRestartEnable = enableStripBreaks ? VK_TRUE : VK_FALSE;
}

void Pipeline::setBlendMode(const VkPipelineColorBlendAttachmentState &blendMode)
{
    colorBlendAttachment = blendMode;
}

void Pipeline::setDepthStencilMode(VkBool32 enableDepthTest, VkBool32 enableDepthWrite, VkCompareOp compare, VkBool32 enableDepthBiais, float depthBiasClamp, float depthBiasSlopeFactor)
{
    depthStencil.depthTestEnable = enableDepthTest;
    depthStencil.depthWriteEnable = enableDepthWrite;
    depthStencil.depthCompareOp = compare;
    rasterizer.depthBiasEnable = enableDepthBiais;
    rasterizer.depthBiasClamp = depthBiasClamp;
    rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
}

void Pipeline::bindVertex(VertexBuffer &vertex, uint32_t binding)
{
    auto tmp = vertex.getBindingDesc();
    tmp.binding = binding;
    bindingDescriptions.push_back(tmp);
    std::vector<VkVertexInputAttributeDescription> tmpAttributeDesc = vertex.getAttributeDesc();
    std::for_each(tmpAttributeDesc.begin(), tmpAttributeDesc.end(), [binding](auto &value){value.binding = binding;});
    attributeDescriptions.insert(attributeDescriptions.end(), tmpAttributeDesc.begin(), tmpAttributeDesc.end());
}

void Pipeline::build()
{
    VkPipelineMultisampleStateCreateInfo multisampling{}; // Pour l'anti-aliasing
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optionnel
    multisampling.pSampleMask = nullptr; // Optionnel
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optionnel
    multisampling.alphaToOneEnable = VK_FALSE; // Optionnel

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pMultisampleState = &multisampling;

    // Héritage - le changement entre 2 pipes ayant le même parent est plus rapide
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optionnel
    pipelineInfo.basePipelineIndex = -1; // Optionnel
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(master->refDevice, master->getPipelineCache(), 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la pipeline graphique!");
    }
    for (auto &stage : shaderStages) {
        vkDestroyShaderModule(master->refDevice, stage.module, nullptr);
    }
    pNames.clear();
    shaderStages.clear();
}
