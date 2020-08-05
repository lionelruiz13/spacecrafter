#include "VirtualSurface.hpp"

VirtualSurface::VirtualSurface(Vulkan *_master) : refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(_master->refSwapChainFramebuffers), refFrameIndex(_master->refFrameIndex), master(_master)
{
    transferPool = _master->getTransferPool();
    transferQueue = _master->getTransferQueue();
    graphicsQueue = _master->assignGraphicsQueue();
}

VirtualSurface::~VirtualSurface()
{
}

void VirtualSurface::submitTransfert(VkCommandBuffer &command, VkSubmitInfo *submitInfo)
{
    vkQueueSubmit(transferQueue, 1, submitInfo, VK_NULL_HANDLE);
}

void VirtualSurface::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    master->createBuffer(size, usage, properties, buffer, bufferMemory);
}
