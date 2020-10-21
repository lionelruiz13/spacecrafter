#include "VirtualSurface.hpp"
#include "SetMgr.hpp"
#include "tools/log.hpp"

SetMgr::SetMgr(VirtualSurface *_master, int maxSet, int maxUniformSet, int maxTextureSet, int maxStorageBufferSet) : master(_master)
{
    std::array<VkDescriptorPoolSize, 3> poolSize;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = maxUniformSet != -1 ? maxUniformSet : maxSet;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = maxTextureSet != -1 ? maxTextureSet : maxSet;
    poolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[2].descriptorCount = maxStorageBufferSet;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (maxStorageBufferSet > 0) ? 3 : 2;
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = maxSet;
    if (poolSize[0].descriptorCount == 0) {
        poolInfo.pPoolSizes++;
        poolInfo.poolSizeCount--;
    }

    pools.resize(1);
    if (vkCreateDescriptorPool(_master->refDevice, &poolInfo, nullptr, pools.data()) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
    }

}

void SetMgr::extend()
{
    VkDescriptorPool tmpPool;
    std::array<VkDescriptorPoolSize, 2> poolSize;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = 32;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = 32;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = 32;
    if (vkCreateDescriptorPool(master->refDevice, &poolInfo, nullptr, &tmpPool) != VK_SUCCESS) {
        cLog::get()->write("Failed to extend SetMgr", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        return;
    }
    cLog::get()->write("Expansion of SetMgr with a new pool that can hold 32 sets totaling 32 uniform buffers and 32 textures", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
    pools.push_back(tmpPool);
}

SetMgr::~SetMgr()
{
    vkDeviceWaitIdle(master->refDevice);
    for (auto pool : pools)
        vkDestroyDescriptorPool(master->refDevice, pool, nullptr);
}
