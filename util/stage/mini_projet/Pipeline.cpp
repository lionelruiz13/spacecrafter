#include "VirtualSurface.hpp"

Pipeline::Pipeline(VirtualSurface *_master, PipelineLayout *layout, std::vector<VkDynamicState> _dynamicStates) : master(_master)
{
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Tout élément trop loin ou trop près est ramené à la couche la plus loin ou la plus proche
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Désactiver la géométrie
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optionnel
    rasterizer.depthBiasClamp = 0.0f; // Optionnel
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optionnel

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Possibilité d'interrompre les liaisons entre les vertices pour les modes _STRIP
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pViewportState = &master->getViewportState();

    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = nullptr; // Optionnel
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optionnel
    pipelineInfo.layout = layout->getPipelineLayout();
    pipelineInfo.renderPass = _master->refRenderPass;
}

void Pipeline::bindShader(Shader *shader)
{
    shaderStages.assign(shader->getStageInfo().begin(), shader->getStageInfo().end());
}

void Pipeline::setTopology(VkPrimitiveTopology state, bool enableStripBreaks)
{
    inputAssembly.topology = state;
    inputAssembly.primitiveRestartEnable = enableStripBreaks ? VK_TRUE : VK_FALSE;
}

void Pipeline::setBlendMode(const VkPipelineColorBlendAttachmentState &blendMode, float depthBiasClamp, float depthBiasSlopeFactor)
{
    colorBlendAttachment = blendMode;
    rasterizer.depthBiasClamp = depthBiasClamp;
    rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
}

void Pipeline::bindVertex(VertexBuffer *vertex, uint32_t binding)
{
    auto tmp = vertex->getBindingDesc();
    tmp.binding = binding;
    bindingDescriptions.push_back(tmp);
    std::vector<VkVertexInputAttributeDescription> tmpAttributeDesc = vertex->getAttributeDesc();
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

    if (vkCreateGraphicsPipelines(master->refDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la pipeline graphique!");
    }
}
