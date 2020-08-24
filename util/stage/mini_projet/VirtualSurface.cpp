#include "VirtualSurface.hpp"
#include "CommandMgr.hpp"
#include <thread>

VirtualSurface::VirtualSurface(Vulkan *_master, int index) : refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(swapChainFramebuffers), refFrameIndex(_master->refFrameIndex), imageAvailableSemaphore(_master->refImageAvailableSemaphore), master(_master)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = 0; // Optionel
    poolInfo.queueFamilyIndex = _master->getTransferQueueFamilyIndex();

    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &transferPool) != VK_SUCCESS) {
        throw std::runtime_error("Faild to create command pool for transfer operations.");
    }
    poolInfo.queueFamilyIndex = _master->getGraphicsQueueIndex();
    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Faild to create command pool for transfer operations.");
    }
    graphicsQueue = _master->assignGraphicsQueue();
    pSwapChain = _master->assignSwapChain();
    _master->assignSwapChainFramebuffers(swapChainFramebuffers, index);
    swapChainExtent = _master->getSwapChainExtent();
}

VirtualSurface::~VirtualSurface()
{
    vkDestroyCommandPool(refDevice, transferPool, nullptr);
    vkDestroyCommandPool(refDevice, cmdPool, nullptr);
}

void VirtualSurface::submitTransfer(VkSubmitInfo *submitInfo)
{
    master->submitTransfer(submitInfo);
}

bool VirtualSurface::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties)
{
    return master->createBuffer(size, usage, properties, buffer, bufferMemory, preferedProperties);
}

void VirtualSurface::getNextFrame()
{
    while (frameIndexQueue.empty())
        std::this_thread::yield();
}

void VirtualSurface::releaseFrame()
{
    frameIndexQueue.pop();
}

void VirtualSurface::acquireNextFrame()
{
    while (frameIndexQueue.size() == MAX_FRAMES_IN_FLIGHT - 1)
        std::this_thread::yield();
}

void VirtualSurface::submitFrame()
{
    while (dependencyFrameIndexQueue && dependencyFrameIndexQueue->empty())
        std::this_thread::yield();
    for (uint8_t i = 0; i < commandMgrList.size(); ++i) {
        commandMgrList[i]->submit();
    }
    frameIndexQueue.push(frameIndex);
}

void VirtualSurface::waitEmpty()
{
    while (!frameIndexQueue.empty())
        std::this_thread::yield();
}

void VirtualSurface::finalize(bool waitMaster)
{
    // link CommandMgr
    for (uint8_t i = 1; i < commandMgrList.size(); i++) {
        for (uint8_t j = 0; j < swapchainSize; j++)
            commandMgrList[i]->setTopSemaphore(j, commandMgrList[i - 1]->getBottomSemaphore(j));
    }
    // synchronize with master
    isReady = true;
    if (waitMaster)
        master->waitReady();
}

void VirtualSurface::waitReady()
{
    while (!isReady)
        std::this_thread::yield();
}

void VirtualSurface::link(uint8_t frameIndex, VirtualSurface *dependency)
{
    dependencyFrameIndexQueue = &dependency->frameIndexQueue;
    commandMgrList.front()->setTopSemaphore(frameIndex, dependency->commandMgrList.back()->getBottomSemaphore(frameIndex));
}

void VirtualSurface::setTopSemaphore(uint8_t frameIndex, const VkSemaphore &semaphore)
{
    commandMgrList.front()->setTopSemaphore(frameIndex, semaphore);
}

const VkSemaphore &VirtualSurface::getBottomSemaphore(uint8_t frameIndex)
{
    return commandMgrList.back()->getBottomSemaphore(frameIndex);
}
