#ifndef VIRTUAL_SURFACE_HPP
#define VIRTUAL_SURFACE_HPP

#include <queue>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cassert>
#include <memory>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"

#define VK_HOST_MEMORY (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

class Vulkan;
class CommandMgr;
class Texture;
class BufferMgr;

/**
    \brief Represent real or virtual surface.
    Every object created from the same VirtualSurface must be used in the same thread
*/
class VirtualSurface {
public:
    VirtualSurface(Vulkan *_master, int index, VkSampleCountFlagBits _sampleCount, bool _isThreaded);
    VirtualSurface(Vulkan *_master, std::vector<std::shared_ptr<Texture>> &frames, Texture &depthBuffer, int width = -1, int height = -1);
    ~VirtualSurface();
    VkCommandPool &getTransferPool() {return transferPool;}
    VkCommandPool &getCommandPool() {return cmdPool;}
    //! Relay transfer queue submission to master
    void submitTransfer(VkSubmitInfo *submitInfo, VkFence fence = VK_NULL_HANDLE);
    //! Relay graphic queue submission to master
    void submitGraphic(VkSubmitInfo &submitInfo);
    //! Wait all transfer commandBuffer to complete
    void waitTransferQueueIdle();
    //! Wait all graphic commandBuffer to be submitted
    void waitGraphicQueueIdle();
    //! Relay createBuffer to master
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    //! Relay free to MemoryManager
    void free(SubMemory& bufferMemory);
    //! Relay mapMemory to MemoryManager
    void mapMemory(SubMemory& bufferMemory, void **data);
    //! Relay unmapMemory to MemoryManager
    void unmapMemory(SubMemory& bufferMemory);
    //! Internally used
    const VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    //! Internally used
    size_t &getGraphicsQueueIndex();
    //! Internally used
    size_t getTransferQueueIndex();
    //! Internally used
    VkQueue &getQueue() {return graphicsQueue;}
    //! Internally used
    int getNextFrame();
    //! Internally used
    void releaseFrame();
    //! Catch next frame
    void acquireNextFrame();
    //! Submit frame
    void submitFrame();
    //! Wait for consumption of submitted frames
    void waitEmpty();
    //! Tell if all submitted frames were consumed
    bool isEmpty() {return frameIndexQueue.empty();}
    //! Register commandMgr and call submit on frame submission
    void registerCommandMgr(CommandMgr *commandMgr) {commandMgrList.push_back(commandMgr);}
    //! Internally used, link VirtualSurface owned by master between them
    void link(uint8_t frameIndex, VirtualSurface *dependent);
    //! Internally used to link VirtualSurface owned by master between them and with frame acquire
    void setTopSemaphore(uint8_t frameIndex, const VkSemaphore &semaphore);
    //! Internally used to link VirtualSurface owned by master between them and with frame present
    const VkSemaphore &getBottomSemaphore(uint8_t frameIndex);
    //! Link owned VirtualSurface and their commandMgr to make command line
    void finalize(bool waitMaster = true);
    //! Internally used, wait finalize to complete
    void waitReady();
    //! Internally used, return features state (enabled/disabled)
    const VkPhysicalDeviceFeatures &getDeviceFeatures();
    //! Internally used, link cache to all Pipeline
    VkPipelineCache &getPipelineCache();
    //! Internally used, inform if this VirtualSurface own his framebuffers (by extension, if it is owned by master)
    bool ownCompleteFramebuffer() {return ownFramebuffers;}
    //! Acquire subBuffer from global buffer
    SubBuffer acquireBuffer(int size, bool isUniform = false);
    //! Get pointer to staging buffer memory
    void *getBufferPtr(SubBuffer &buffer);
    //! Release subBuffer back to global buffer
    void releaseBuffer(SubBuffer &buffer);
    //! Attach name to vulkan handle
    void setObjectName(void *handle, VkObjectType type, const std::string &name);

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
    bool isThreaded = true; // If threaded, don't use thread helper for command submission
};

#endif /* end of include guard: VIRTUAL_SURFACE_HPP */
