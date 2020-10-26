#include "VirtualSurface.hpp"
#include "PipelineLayout.hpp"
#include "Pipeline.hpp"
#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "CommandMgr.hpp"
#include "tools/log.hpp"
#include <fstream>
#include <algorithm>

std::string Pipeline::shaderDir = "./";
float Pipeline::defaultLineWidth = 1.0f;

Pipeline::Pipeline(VirtualSurface *_master, PipelineLayout *layout, std::vector<VkDynamicState> _dynamicStates) : master(_master)
{
    colorBlendAttachment = BLEND_SRC_ALPHA;

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
    rasterizer.lineWidth = defaultLineWidth;
    rasterizer.cullMode = 0;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optionnel
    rasterizer.depthBiasClamp = 0.0f; // Optionnel
    rasterizer.depthBiasSlopeFactor = 1.0f; // Optionnel

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optionnel
    depthStencil.maxDepthBounds = 1.0f; // Optionnel
    depthStencil.stencilTestEnable = VK_FALSE;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = master->getDeviceFeatures().sampleRateShading;
    multisampling.rasterizationSamples = master->sampleCount;
    multisampling.minSampleShading = .2f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    isOk = (pipelineInfo.layout != VK_NULL_HANDLE);
}

Pipeline::~Pipeline()
{
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(master->refDevice);
        vkDestroyPipeline(master->refDevice, graphicsPipeline, nullptr);
    }
}

void Pipeline::bindShader(const std::string &filename, const std::string entry)
{
    size_t first = filename.find_first_of(".") + 1;
    size_t size = filename.find_last_of(".") - first;
    std::string shaderType = filename.substr(first, size);
    if (shaderType.compare("vert") == 0) {
        bindShader(filename, VK_SHADER_STAGE_VERTEX_BIT, entry);
        return;
    }
    if (shaderType.compare("frag") == 0) {
        bindShader(filename, VK_SHADER_STAGE_FRAGMENT_BIT, entry);
        return;
    }
    if (shaderType.compare("geom") == 0) {
        bindShader(filename, VK_SHADER_STAGE_GEOMETRY_BIT, entry);
        return;
    }
    if (shaderType.compare("tesc") == 0) {
        bindShader(filename, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, entry);
        return;
    }
    if (shaderType.compare("tese") == 0) {
        bindShader(filename, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, entry);
        return;
    }
}

void Pipeline::bindShader(const std::string &filename, VkShaderStageFlagBits stage, const std::string entry)
{
    if (stage & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) && master->getDeviceFeatures().tessellationShader == VK_FALSE)
        return;

    std::ifstream file(shaderDir + filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        cLog::get()->write("Failed to open file '" + filename + "'", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        isOk = false;
        return;
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

    SpecializationInfo specInfo;
    specInfo.info.mapEntryCount = 0;
    specInfo.info.dataSize = 0;
    specializationInfo.push_front(specInfo);
    tmp.pSpecializationInfo = &specializationInfo.front().info;

    if (vkCreateShaderModule(master->refDevice, &createInfo, nullptr, &tmp.module) != VK_SUCCESS) {
        cLog::get()->write("Failed to create shader module from file '" + filename + "'", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        isOk = false;
        return;
    }
    delete buffer;
    shaderStages.push_back(tmp);
}

void Pipeline::setSpecializedConstant(uint32_t constantID, void *data, size_t size)
{
    SpecializationInfo &specInfo = specializationInfo.front();
    specInfo.entry.push_back({constantID, static_cast<uint32_t>(specInfo.data.size()), size});
    specInfo.data.resize(specInfo.data.size() + size);
    memcpy(specInfo.data.data() + specInfo.entry.back().offset, data, size);
    specInfo.info.mapEntryCount = specInfo.entry.size();
    specInfo.info.pMapEntries = specInfo.entry.data();
    specInfo.info.dataSize = specInfo.data.size();
    specInfo.info.pData = reinterpret_cast<void *>(specInfo.data.data());
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

void Pipeline::setTessellationState(uint32_t patchControlPoints)
{
    tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellation.pNext = nullptr;
    tessellation.flags = 0;
    tessellation.patchControlPoints = patchControlPoints;
    pipelineInfo.pTessellationState = &tessellation;
}

void Pipeline::setLineWidth(float lineWidth)
{
    rasterizer.lineWidth = lineWidth;
}

void Pipeline::setRenderPassCompatibility(renderPassCompatibility compatibility)
{
   pipelineInfo.renderPass = master->refRenderPassCompatibility[static_cast<int>(compatibility)];
   if (compatibility == renderPassCompatibility::SINGLE_SAMPLE) {
       multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   }
}

void Pipeline::bindVertex(VertexArray *vertex, uint32_t binding)
{
    auto tmp = vertex->getVertexBindingDesc();
    tmp.binding = binding;
    bindingDescriptions.push_back(tmp);
    std::vector<VkVertexInputAttributeDescription> tmpAttributeDesc = vertex->getVertexAttributeDesc();
    std::for_each(tmpAttributeDesc.begin(), tmpAttributeDesc.end(), [binding](auto &value){value.binding = binding;});
    attributeDescriptions.insert(attributeDescriptions.end(), tmpAttributeDesc.begin(), tmpAttributeDesc.end());

    if (vertex->hasInstanceBuffer()) {
        binding++;
        tmp = vertex->getInstanceBindingDesc();
        tmp.binding = binding;
        bindingDescriptions.push_back(tmp);
        tmpAttributeDesc = vertex->getInstanceAttributeDesc();
        std::for_each(tmpAttributeDesc.begin(), tmpAttributeDesc.end(), [binding](auto &value){value.binding = binding;});
        attributeDescriptions.insert(attributeDescriptions.end(), tmpAttributeDesc.begin(), tmpAttributeDesc.end());
    }
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
    if (!isOk || bindingDescriptions.empty() || shaderStages.empty()) {
        cLog::get()->write("Can't build invalid Pipeline", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        for (auto &stage : shaderStages) {
            vkDestroyShaderModule(master->refDevice, stage.module, nullptr);
        }
        return;
    }

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
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optionnel
    pipelineInfo.basePipelineIndex = -1; // Optionnel
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(master->refDevice, master->getPipelineCache(), 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        cLog::get()->write("Faild to create Pipeline", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        graphicsPipeline = VK_NULL_HANDLE;
    }
    for (auto &stage : shaderStages) {
        vkDestroyShaderModule(master->refDevice, stage.module, nullptr);
    }
    pNames.clear();
    shaderStages.clear();
}
