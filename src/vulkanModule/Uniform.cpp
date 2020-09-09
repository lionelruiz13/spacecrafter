#include "VirtualSurface.hpp"
#include "Uniform.hpp"

Uniform::Uniform(VirtualSurface *_master, VkDeviceSize size) : master(_master)
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

void Uniform::update()
{}
