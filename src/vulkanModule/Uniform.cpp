#include "VirtualSurface.hpp"
#include "Uniform.hpp"

Uniform::Uniform(VirtualSurface *_master, VkDeviceSize size) : master(_master)
{
    if (!_master->createBuffer(UNIFORM_BLOC_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Faild to create buffer");
    }
    _master->mapMemory(bufferMemory, &data);
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;
}

Uniform::~Uniform()
{
    master->unmapMemory(bufferMemory);
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    master->free(bufferMemory);
}

void Uniform::update()
{}
