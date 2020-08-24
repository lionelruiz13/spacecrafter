#include "VirtualSurface.hpp"
#include "Uniform.hpp"

/*
VkDescriptorPoolSize poolSize{};
poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
poolSize.descriptorCount = 0;

VkDescriptorPoolCreateInfo poolInfo{};
poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
poolInfo.poolSizeCount = 1;
poolInfo.pPoolSizes = &poolSize;
poolInfo.maxSets = poolSize.descriptorCount;

VkDescriptorPool descriptorPool;
if (vkCreateDescriptorPool(refDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
}
*/

Uniform::Uniform(VirtualSurface *_master, VkDeviceSize size, VkShaderStageFlags _stages) : master(_master), stages(_stages)
{
    if (!_master->createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        std::cout << "Uniform can't be on Device and directly accessible." << std::endl;
    }
    vkMapMemory(_master->refDevice, bufferMemory, 0, size, 0, &data);
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;
}

Uniform::~Uniform()
{
    vkUnmapMemory(master->refDevice, bufferMemory);
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    vkFreeMemory(master->refDevice, bufferMemory, nullptr);
}
