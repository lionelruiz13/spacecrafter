#include "VirtualSurface.hpp"
#include "Buffer.hpp"
#include "BufferMgr.hpp"

Buffer::Buffer(VirtualSurface *_master, int size, VkBufferUsageFlags usage) : master(_master)
{
    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, stagingBuffer, stagingBufferMemory);
        _master->mapMemory(stagingBufferMemory, &data);
        _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
    } else {
        _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);
        _master->mapMemory(stagingBufferMemory, &data);
        _master->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
    }

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
    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        vkCmdCopyBuffer(updater, buffer, stagingBuffer, 1, &copyRegion);
    } else {
        vkCmdCopyBuffer(updater, stagingBuffer, buffer, 1, &copyRegion);
    }

    vkEndCommandBuffer(updater);

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &updater;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;
}

Buffer::~Buffer()
{
    detach();
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    master->free(bufferMemory);
}

void Buffer::print()
{
    std::cout << "\thasDebugPrint : false\n";
}

void Buffer::update()
{
    master->submitTransfer(&submitInfo);
}

void Buffer::detach()
{
    if (data != nullptr) {
        master->unmapMemory(stagingBufferMemory);
        master->waitTransferQueueIdle();
        if (submitInfo.commandBufferCount > 0) {
            vkFreeCommandBuffers(master->refDevice, master->getTransferPool(), 1, &updater);
            submitInfo.commandBufferCount = 0;
        }
        vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
        master->free(stagingBufferMemory);
        data = nullptr;
    }
}

void Buffer::invalidate()
{
    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = stagingBufferMemory.memory;
    range.offset = stagingBufferMemory.offset;
    range.size = stagingBufferMemory.size;
    master->waitTransferQueueIdle();
    vkInvalidateMappedMemoryRanges(master->refDevice, 1, &range);
}
