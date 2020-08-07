#include "VirtualSurface.hpp"

PipelineLayout::PipelineLayout(VirtualSurface *_master) : master(_master)
{
}

PipelineLayout::~PipelineLayout()
{
    if (builded) {
        vkDestroyPipelineLayout(master->refDevice, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(master->refDevice, descriptor, nullptr);
    }
}

void PipelineLayout::addUniform(Uniform *uniform, uint32_t binding)
{
    VkDescriptorSetLayoutBinding uniformCollection{};
    uniformCollection.binding = binding;
    uniformCollection.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformCollection.descriptorCount = 1;
    uniformCollection.stageFlags = uniform->getStage();
    //uniformCollection.pImmutableSamplers = nullptr; // Optionnel
    uniformsLayout.push_back(uniformCollection);
}

void PipelineLayout::build()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uniformsLayout.size();
    layoutInfo.pBindings = uniformsLayout.data();

    if (vkCreateDescriptorSetLayout(master->refDevice, &layoutInfo, nullptr, &descriptor) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'un set de descripteurs!");
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptor;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optionnel
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optionnel

    if (vkCreatePipelineLayout(master->refDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création du pipeline layout!");
    }
    builded = true;
}

void PipelineLayout::pushConstant() {}
