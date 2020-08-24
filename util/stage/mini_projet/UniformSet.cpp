#include "VirtualSurface.hpp"
#include "UniformSet.hpp"
#include "PipelineLayout.hpp"
#include "Uniform.hpp"

UniformSetMgr::UniformSetMgr(VirtualSurface *_master, int maxUniformBufferSet) : master(_master)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = maxUniformBufferSet;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = poolSize.descriptorCount;

    if (vkCreateDescriptorPool(_master->refDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
    }
}

UniformSetMgr::~UniformSetMgr()
{
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyDescriptorPool(master->refDevice, pool, nullptr);
}

UniformSet::UniformSet(VirtualSurface *_master, UniformSetMgr *_mgr, PipelineLayout *_layout) : master(_master)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _mgr->getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_layout->getDescriptorLayout();

    if (vkAllocateDescriptorSets(_master->refDevice, &allocInfo, &set) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
    }
}

void UniformSet::bindUniform(Uniform *uniform, int binding)
{
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = uniform->getBufferInfo();

    vkUpdateDescriptorSets(master->refDevice, 1, &descriptorWrite, 0, nullptr);
}
