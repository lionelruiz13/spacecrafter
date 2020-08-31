#include "VirtualSurface.hpp"
#include "SetMgr.hpp"

SetMgr::SetMgr(VirtualSurface *_master, int maxSet, int maxUniformSet, int maxTextureSet) : master(_master)
{
    std::array<VkDescriptorPoolSize, 2> poolSize;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = maxUniformSet != -1 ? maxUniformSet : maxSet;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = maxTextureSet != -1 ? maxTextureSet : maxSet;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = maxSet;

    if (vkCreateDescriptorPool(_master->refDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
    }
}

SetMgr::~SetMgr()
{
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyDescriptorPool(master->refDevice, pool, nullptr);
}
