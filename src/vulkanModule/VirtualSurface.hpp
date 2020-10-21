#ifndef VIRTUAL_SURFACE_HPP
#define VIRTUAL_SURFACE_HPP

#include <queue>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cassert>
#include <memory>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"

#define VK_HOST_MEMORY VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

class Vulkan;
class CommandMgr;
class Texture;
class BufferMgr;

class VirtualSurface {
public:
    VirtualSurface(Vulkan *_master, int index, VkSampleCountFlagBits _sampleCount);
    VirtualSurface(Vulkan *_master, std::vector<std::shared_ptr<Texture>> &frames, Texture &depthBuffer, int width = -1, int height = -1);
    ~VirtualSurface();
    VkCommandPool &getTransferPool() {return transferPool;}
    VkCommandPool &getCommandPool() {return cmdPool;}
    void submitTransfer(VkSubmitInfo *submitInfo, VkFence fence = VK_NULL_HANDLE);
    void waitTransferQueueIdle();
    void waitGraphicQueueIdle();
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    void free(SubMemory& bufferMemory);
    void mapMemory(SubMemory& bufferMemory, void **data);
    void unmapMemory(SubMemory& bufferMemory);
    const VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    size_t &getGraphicsQueueIndex();
    size_t getTransferQueueIndex();
    VkQueue &getQueue() {return graphicsQueue;}
    int getNextFrame();
    void releaseFrame();
    void acquireNextFrame();
    void submitFrame();
    void waitEmpty();
    bool isEmpty() {return frameIndexQueue.empty();}
    void registerCommandMgr(CommandMgr *commandMgr) {commandMgrList.push_back(commandMgr);}
    void link(uint8_t frameIndex, VirtualSurface *dependent);
    void setTopSemaphore(uint8_t frameIndex, const VkSemaphore &semaphore);
    const VkSemaphore &getBottomSemaphore(uint8_t frameIndex);
    void finalize(bool waitMaster = true);
    void waitReady();
    const VkPhysicalDeviceFeatures &getDeviceFeatures();
    VkPipelineCache &getPipelineCache();
    bool ownCompleteFramebuffer() {return ownFramebuffers;}
    SubBuffer acquireBuffer(int size, bool isUniform = false);
    void *getBufferPtr(SubBuffer &buffer);
    void releaseBuffer(SubBuffer &buffer);

    const VkDevice &refDevice;
    const std::vector<VkRenderPass> &refRenderPass;
    const std::vector<VkRenderPass> &refRenderPassCompatibility;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const std::vector<VkFramebuffer> &refResolveFramebuffers;
    const std::vector<VkFramebuffer> &refSingleSampleFramebuffers;
    const uint32_t &refFrameIndex;
    const VkSampleCountFlagBits sampleCount;

    VkExtent2D swapChainExtent;
private:
    void createFramebuffer(std::vector<std::shared_ptr<Texture>> &frames, Texture &depthBuffer);
    std::vector<CommandMgr*> commandMgrList;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkViewport viewport;
    VkRect2D scissors;
    VkPipelineViewportStateCreateInfo viewportState;
    bool ownFramebuffers = false;

    VkCommandPool transferPool;
    VkCommandPool cmdPool;
    VkQueue graphicsQueue;
    Vulkan *master;
    std::unique_ptr<BufferMgr> bufferMgr;
    int swapchainSize;
    uint32_t frameIndex = -1;
    std::queue<uint32_t> frameIndexQueue;
    std::queue<uint32_t> *dependencyFrameIndexQueue = nullptr;
    bool isReady = false;
};

#endif /* end of include guard: VIRTUAL_SURFACE_HPP */
