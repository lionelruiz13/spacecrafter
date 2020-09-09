#include "VirtualSurface.hpp"
#include "PipelineLayout.hpp"

VkSamplerCreateInfo PipelineLayout::DEFAULT_SAMPLER = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, VK_TRUE, 8.0f, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE};

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
        if (descriptorPos >= 0)
            vkDestroyDescriptorSetLayout(master->refDevice, descriptor[descriptorPos], nullptr);
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

void PipelineLayout::setUniformLocation(VkShaderStageFlags stage, uint32_t binding, uint32_t arraySize)
{
    VkDescriptorSetLayoutBinding uniformCollection{};
    uniformCollection.binding = binding;
    uniformCollection.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
        throw std::runtime_error("echec de la creation d'un set de descripteurs!");
    }
    descriptorPos = descriptor.size(); // Inform which descriptor is owned by this PipelineLayout
    descriptor.push_back(tmp);
    uniformsLayout.clear();
}

void PipelineLayout::setGlobalPipelineLayout(PipelineLayout *pl)
{
    descriptor.push_back(pl->getDescriptorLayout());
}

void PipelineLayout::build()
{
    assert(uniformsLayout.empty());
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptor.size();
    pipelineLayoutInfo.pSetLayouts = descriptor.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optionnel
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optionnel

    if (vkCreatePipelineLayout(master->refDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création du pipeline layout!");
    }
    builded = true;
}

void PipelineLayout::pushConstant() {}
