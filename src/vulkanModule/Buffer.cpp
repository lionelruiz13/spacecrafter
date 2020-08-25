#include "VirtualSurface.hpp"
#include "Buffer.hpp"

Buffer::Buffer(VirtualSurface *_master, int size, VkBufferUsageFlags usage) : master(_master)
{
    _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);
    vkMapMemory(_master->refDevice, stagingBufferMemory, 0, size, 0, &data);
    _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

    // Initialize update
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _master->getTransferPool();
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(_master->refDevice, &allocInfo, &updater);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    vkBeginCommandBuffer(updater, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(updater, stagingBuffer, buffer, 1, &copyRegion);

    vkEndCommandBuffer(updater);

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &updater;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;
}

Buffer::~Buffer()
{
    vkUnmapMemory(master->refDevice, stagingBufferMemory);
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
    vkFreeMemory(master->refDevice, stagingBufferMemory, nullptr);
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    vkFreeMemory(master->refDevice, bufferMemory, nullptr);
}

void Buffer::update()
{
    master->submitTransfer(&submitInfo);
}
