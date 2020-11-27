#include "ComputePipeline.hpp"
#include "VirtualSurface.hpp"
#include "PipelineLayout.hpp"
#include "tools/log.hpp"
#include <fstream>
#include <cstring>

std::string ComputePipeline::shaderDir = "./";

ComputePipeline::ComputePipeline(VirtualSurface *_master, PipelineLayout *layout) : master(_master)
{
    pipelineInfo.layout = layout->getPipelineLayout();
    isOk = (pipelineInfo.layout != VK_NULL_HANDLE);
}

ComputePipeline::~ComputePipeline()
{
    if (computePipeline != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(master->refDevice);
        vkDestroyPipeline(master->refDevice, computePipeline, nullptr);
    }
}

void ComputePipeline::bindShader(const std::string &filename, const std::string entry)
{
    entryName = entry;
    pipelineInfo.stage.pName = entry.c_str();
    std::ifstream file(shaderDir + filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        cLog::get()->write("Failed to open file '" + filename + "'", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        isOk = false;
        entryName.clear();
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
    if (vkCreateShaderModule(master->refDevice, &createInfo, nullptr, &pipelineInfo.stage.module) != VK_SUCCESS) {
        cLog::get()->write("Failed to create shader module from file '" + filename + "'", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        isOk = false;
        return;
    }
    delete[] buffer;
    master->setObjectName(pipelineInfo.stage.module, VK_OBJECT_TYPE_SHADER_MODULE, filename);
    name = "Use " + filename;
}

void ComputePipeline::setSpecializedConstant(uint32_t constantID, void *data, size_t size)
{
    specializationInfo.entry.push_back({constantID, static_cast<uint32_t>(specializationInfo.data.size()), size});
    specializationInfo.data.resize(specializationInfo.data.size() + size);
    memcpy(specializationInfo.data.data() + specializationInfo.entry.back().offset, data, size);
    specializationInfo.info.mapEntryCount = specializationInfo.entry.size();
    specializationInfo.info.pMapEntries = specializationInfo.entry.data();
    specializationInfo.info.dataSize = specializationInfo.data.size();
    specializationInfo.info.pData = reinterpret_cast<void *>(specializationInfo.data.data());
}

void ComputePipeline::build()
{
    if (!isOk || entryName.empty()) {
        cLog::get()->write("Can't build invalid Pipeline", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        if (!entryName.empty())
            vkDestroyShaderModule(master->refDevice, pipelineInfo.stage.module, nullptr);
        return;
    }
    if (vkCreateComputePipelines(master->refDevice, master->getPipelineCache(), 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        cLog::get()->write("Faild to create compute Pipeline", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        computePipeline = VK_NULL_HANDLE;
    } else {
        master->setObjectName(computePipeline, VK_OBJECT_TYPE_PIPELINE, name);
    }
    vkDestroyShaderModule(master->refDevice, pipelineInfo.stage.module, nullptr);
}
