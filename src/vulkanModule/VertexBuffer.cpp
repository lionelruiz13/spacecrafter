#include "VirtualSurface.hpp"
#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(VirtualSurface *_master, int size,
    const VkVertexInputBindingDescription &_bindingDesc,
    const std::vector<VkVertexInputAttributeDescription> &_attributeDesc) : master(_master), bindingDesc(_bindingDesc)
{
    VkDeviceSize bufferSize = _bindingDesc.stride * size;

    _master->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);
    vkMapMemory(_master->refDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    _master->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

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
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(updater, stagingBuffer, vertexBuffer, 1, &copyRegion);

    vkEndCommandBuffer(updater);

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &updater;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;

    attributeDesc.assign(_attributeDesc.begin(), _attributeDesc.end());
}

VertexBuffer::~VertexBuffer()
{
    vkUnmapMemory(master->refDevice, stagingBufferMemory);
    vkDeviceWaitIdle(master->refDevice);
    vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
    vkFreeMemory(master->refDevice, stagingBufferMemory, nullptr);
    vkDestroyBuffer(master->refDevice, vertexBuffer, nullptr);
    vkFreeMemory(master->refDevice, vertexBufferMemory, nullptr);
}

void VertexBuffer::update()
{
    master->submitTransfer(&submitInfo);
}
