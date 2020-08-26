#ifndef VIRTUAL_SURFACE_HPP
#define VIRTUAL_SURFACE_HPP

#include "Vulkan.hpp"
#include <queue>

#define VK_HOST_MEMORY VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT

class CommandMgr;

class VirtualSurface {
public:
    VirtualSurface(Vulkan *_master, int index);
    ~VirtualSurface();
    VkCommandPool &getTransferPool() {return transferPool;}
    VkCommandPool &getCommandPool() {return cmdPool;}
    void submitTransfer(VkSubmitInfo *submitInfo);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    const VkPipelineViewportStateCreateInfo &getViewportState() {return master->getViewportState();}
    auto &getGraphicsQueueIndex() {return master->getGraphicsQueueIndex();}
    VkQueue &getQueue() {return graphicsQueue;}
    void getNextFrame();
    void releaseFrame();
    void acquireNextFrame();
    void submitFrame();
    void waitEmpty();
    const VkSemaphore *getImageAvailableSemaphore() {return &imageAvailableSemaphore;}
    void registerCommandMgr(CommandMgr *commandMgr) {commandMgrList.push_back(commandMgr);}
    void link(uint8_t frameIndex, VirtualSurface *dependent);
    void setTopSemaphore(uint8_t frameIndex, const VkSemaphore &semaphore);
    const VkSemaphore &getBottomSemaphore(uint8_t frameIndex);
    void finalize(bool waitMaster = true);
    void waitReady();
    const VkPhysicalDeviceFeatures &getDeviceFeatures() {return master->getDeviceFeatures();}
    VkPipelineCache &getPipelineCache() {return master->getPipelineCache();}

    const VkDevice &refDevice;
    const std::array<VkRenderPass, 4> &refRenderPass;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const uint32_t &refFrameIndex;

    VkExtent2D swapChainExtent;
private:
    //! selected imageAvailableSemaphore
    const VkSemaphore &imageAvailableSemaphore;
    //int switcher = 0; // switch between MAX_FRAME_IN_FLIGHT elements
    //std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::vector<CommandMgr*> commandMgrList;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkSwapchainKHR *pSwapChain;
    VkCommandPool transferPool;
    VkCommandPool cmdPool;
    VkQueue graphicsQueue;
    Vulkan *master;
    int swapchainSize;
    uint32_t frameIndex;
    std::queue<uint32_t> frameIndexQueue;
    std::queue<uint32_t> *dependencyFrameIndexQueue = nullptr;
    bool isReady = false;
};

#endif /* end of include guard: VIRTUAL_SURFACE_HPP */
