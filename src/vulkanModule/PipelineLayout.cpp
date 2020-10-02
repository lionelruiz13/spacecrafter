#include "VirtualSurface.hpp"
#include "PipelineLayout.hpp"
#include "tools/log.hpp"

VkSamplerCreateInfo PipelineLayout::DEFAULT_SAMPLER = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, VK_TRUE, 8.0f, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE};

PipelineLayout::PipelineLayout(VirtualSurface *_master) : master(_master)
{
    DEFAULT_SAMPLER.anisotropyEnable = master->getDeviceFeatures().samplerAnisotropy;
}

PipelineLayout::~PipelineLayout()
{
    for (auto &tmp : sampler)
        vkDestroySampler(master->refDevice, tmp, nullptr);
    if (builded) {
        vkDestroyPipelineLayout(master->refDevice, pipelineLayout, nullptr);
    }
    for (int index : descriptorPos) {
        vkDestroyDescriptorSetLayout(master->refDevice, descriptor[index], nullptr);
    }
}

void PipelineLayout::setTextureLocation(uint32_t binding, const VkSamplerCreateInfo *samplerInfo)
{
    if (samplerInfo) {
        VkSampler tmpSampler;
        if (vkCreateSampler(master->refDevice, samplerInfo, nullptr, &tmpSampler) != VK_SUCCESS) {
            throw std::runtime_error("échec de la creation d'un sampler!");
        }
        sampler.push_back(tmpSampler);
    }
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = binding;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = samplerInfo ? &sampler.back() : nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uniformsLayout.push_back(samplerLayoutBinding);
}

void PipelineLayout::setUniformLocation(VkShaderStageFlags stage, uint32_t binding, uint32_t arraySize, bool isVirtual)
{
    VkDescriptorSetLayoutBinding uniformCollection{};
    uniformCollection.binding = binding;
    uniformCollection.descriptorType = (isVirtual) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformCollection.descriptorCount = arraySize;
    uniformCollection.stageFlags = stage;
    //uniformCollection.pImmutableSamplers = nullptr; // Optionnel
    uniformsLayout.push_back(uniformCollection);
}

void PipelineLayout::buildLayout(VkDescriptorSetLayoutCreateFlags flags)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uniformsLayout.size();
    layoutInfo.pBindings = uniformsLayout.data();
    layoutInfo.flags = flags;

    VkDescriptorSetLayout tmp;
    if (vkCreateDescriptorSetLayout(master->refDevice, &layoutInfo, nullptr, &tmp) != VK_SUCCESS) {
        isOk = false;
        cLog::get()->write("Faild to create Layout", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        tmp = VK_NULL_HANDLE;
    }
    descriptorPos.push_back(descriptor.size());
    descriptor.push_back(tmp);
    uniformsLayout.clear();
}

void PipelineLayout::setGlobalPipelineLayout(PipelineLayout *pl)
{
    VkDescriptorSetLayout tmp = pl->getDescriptorLayout();
    if (tmp == VK_NULL_HANDLE) {
        isOk = false;
        cLog::get()->write("Use of invalid Layout", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
    }
    descriptor.push_back(tmp);
}

void PipelineLayout::build()
{
    if (!isOk) {
        cLog::get()->write("Can't build invalid PipelineLayout", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        return;
    }
    assert(uniformsLayout.empty()); // there mustn't be unbuilded layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptor.size();
    pipelineLayoutInfo.pSetLayouts = descriptor.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

    if (vkCreatePipelineLayout(master->refDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        cLog::get()->write("Faild to create PipelineLayout", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        pipelineLayout = VK_NULL_HANDLE;
        return;
    }
    builded = true;
}

void PipelineLayout::setPushConstant(VkShaderStageFlags stage, uint32_t offset, uint32_t size)
{
    pushConstants.emplace_back(VkPushConstantRange{stage, offset, size});
}
