#include "Vulkan.hpp"
#include "VirtualSurface.hpp"
#include "CommandMgr.hpp"
#include "Texture.hpp"
#include "BufferMgr.hpp"
#include <thread>

VirtualSurface::VirtualSurface(Vulkan *_master, int index) : refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(swapChainFramebuffers), refFrameIndex(_master->refFrameIndex), master(_master)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _master->getTransferQueueFamilyIndex();

    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &transferPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool for transfer operations.");
    }
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = _master->getGraphicsQueueIndex();
    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool for transfer operations.");
    }
    graphicsQueue = _master->assignGraphicsQueue();
    _master->assignSwapChainFramebuffers(swapChainFramebuffers, index);
    swapChainExtent = _master->getSwapChainExtent();
    viewportState = _master->getViewportState();
    bufferMgr = std::make_unique<BufferMgr>(this);
}

VirtualSurface::VirtualSurface(Vulkan *_master, std::vector<std::shared_ptr<Texture>> &frames, Texture &depthBuffer, int width, int height) : refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(swapChainFramebuffers), refFrameIndex(frameIndex), master(_master)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = 0; // Optionel
    poolInfo.queueFamilyIndex = _master->getTransferQueueFamilyIndex();

    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &transferPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool for transfer operations.");
    }
    poolInfo.queueFamilyIndex = _master->getGraphicsQueueIndex();
    if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool for transfer operations.");
    }
    graphicsQueue = _master->assignGraphicsQueue();
    viewportState = _master->getViewportState();
    swapChainExtent.width = (width == -1) ? viewportState.pViewports->width : width;
    swapChainExtent.height = (height == -1) ? abs(viewportState.pViewports->height) : height;
    viewport.x = viewport.y = viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    scissors.offset.x = scissors.offset.y = 0;
    scissors.extent = swapChainExtent;
    viewportState.pScissors = &scissors;
    viewportState.pViewports = &viewport;
    createFramebuffer(frames, depthBuffer);
    bufferMgr = std::make_unique<BufferMgr>(this);
}

VirtualSurface::~VirtualSurface()
{
    vkDestroyCommandPool(refDevice, transferPool, nullptr);
    vkDestroyCommandPool(refDevice, cmdPool, nullptr);
    if (ownFramebuffers) {
        for (auto &fbuff : swapChainFramebuffers) {
            vkDestroyFramebuffer(refDevice, fbuff, nullptr);
        }
    }
}

void VirtualSurface::createFramebuffer(std::vector<std::shared_ptr<Texture>> &frames, Texture &depthBuffer)
{
    swapChainFramebuffers.resize(frames.size());
    VkImageView depthImage = depthBuffer.getInfo()->imageView;
    for (size_t i = 0; i < frames.size(); i++) {
        VkImageView attachments[] = {
            frames[i]->getInfo()->imageView,
            depthImage
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = refRenderPass[1];
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(refDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            return;
        }
    }
    ownFramebuffers = true;
}

void VirtualSurface::submitTransfer(VkSubmitInfo *submitInfo, VkFence fence)
{
    master->submitTransfer(submitInfo, fence);
}

void VirtualSurface::waitTransferQueueIdle()
{
    master->waitTransferQueueIdle();
}

bool VirtualSurface::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties)
{
    return master->createBuffer(size, usage, properties, buffer, bufferMemory, preferedProperties);
}

void VirtualSurface::free(SubMemory& bufferMemory) {master->free(bufferMemory);}
void VirtualSurface::mapMemory(SubMemory& bufferMemory, void **data) {master->mapMemory(bufferMemory, data);}
void VirtualSurface::unmapMemory(SubMemory& bufferMemory) {master->unmapMemory(bufferMemory);}
SubBuffer VirtualSurface::acquireBuffer(int size, bool isUniform) {return bufferMgr->acquireBuffer(size, isUniform);}
void *VirtualSurface::getBufferPtr(SubBuffer &buffer) {return bufferMgr->getPtr(buffer);}
void VirtualSurface::releaseBuffer(SubBuffer &buffer) {bufferMgr->releaseBuffer(buffer);}


int VirtualSurface::getNextFrame()
{
    while (frameIndexQueue.empty())
        std::this_thread::yield();
    return frameIndexQueue.front();
}

void VirtualSurface::releaseFrame()
{
    frameIndexQueue.pop();
}

void VirtualSurface::acquireNextFrame()
{
    while (frameIndexQueue.size() == MAX_FRAMES_IN_FLIGHT - 1)
        std::this_thread::yield();
    frameIndex = (frameIndex + 1) % swapChainFramebuffers.size();
}

void VirtualSurface::submitFrame()
{
    bufferMgr->update();
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

size_t &VirtualSurface::getGraphicsQueueIndex() {return master->getGraphicsQueueIndex();}
const VkPhysicalDeviceFeatures &VirtualSurface::getDeviceFeatures() {return master->getDeviceFeatures();}
VkPipelineCache &VirtualSurface::getPipelineCache() {return master->getPipelineCache();}
