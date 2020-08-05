#include "VirtualSurface.hpp"
#include <fstream>

Shader::Shader(VirtualSurface *master) : refDevice(master->refDevice) {}

Shader::Shader(Vulkan *master) : refDevice(master->refDevice) {}

Shader::~Shader()
{
    for (auto &info : stageInfo) {
        vkDestroyShaderModule(refDevice, info.module, nullptr);
    }
}

void Shader::load(const std::string &filename, VkShaderStageFlagBits stage, const std::string entry)
{
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

    pNames.push_back(entry);
    tmp.pName = pNames.rbegin()->c_str();

    if (vkCreateShaderModule(refDevice, &createInfo, nullptr, &tmp.module) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création d'un module shader!");
    }
    delete buffer;
    stageInfo.push_back(tmp);
}
