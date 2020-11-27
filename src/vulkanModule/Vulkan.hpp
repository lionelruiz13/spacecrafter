#ifndef VULKAN_HPP_
#define VULKAN_HPP_

#include <vulkan/vulkan.hpp>
#include "SubMemory.hpp"
#include "tools/vecmath.hpp"
#include <array>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <atomic>

class SDL_Window;

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VirtualSurface;
class MemoryManager;
class CommandMgr;
class TextureMgr;

class Vulkan {
public:
    Vulkan(const char *_AppName, const char *_EngineName, SDL_Window *window, int nbVirtualSurfaces, int width = 600, int height = 600, int chunkSize = 256*1024*1024, bool enableDebugLayers = true, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, std::string _cachePath = "\0");
    ~Vulkan();
    void initQueues(uint32_t nbQueues = 1);
    //! send frame for submission
    void sendFrame();
    //! submit frame (don't use directly)
    void submitFrame();
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    //! for malloc
    MemoryManager *getMemoryManager() {return memoryManager;}
    void free(SubMemory& bufferMemory);
    void mapMemory(SubMemory& bufferMemory, void **data);
    void unmapMemory(SubMemory& bufferMemory);
    size_t getTransferQueueFamilyIndex() {return transferQueueFamilyIndex[0];}
    void submitTransfer(VkSubmitInfo *submitInfo, VkFence fence = VK_NULL_HANDLE);
    void submitGraphic(VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE);
    void submitCompute(VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE);
    void waitTransferQueueIdle(bool waitCompletion = true);
    void waitGraphicQueueIdle();
    void waitIdle();
    VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    VkQueue assignGraphicsQueue() {return graphicsAndPresentQueues[assignedGraphicQueueCount++];}
    VkSwapchainKHR *assignSwapChain() {return swapChain.data();}
    void assignSwapChainFramebuffers(std::vector<VkFramebuffer> &framebuffers, int index) {framebuffers.assign(swapChainFramebuffers.begin(), swapChainFramebuffers.end());}
    auto &getGraphicsQueueIndex() {return graphicsAndPresentQueueFamilyIndex[0];}
    auto &getComputeQueueIndex() {return computeQueueFamilyIndex.empty() ? graphicsAndPresentQueueFamilyIndex[0] : computeQueueFamilyIndex[0];}
    VirtualSurface *getVirtualSurface() {static short i = 0; return virtualSurface[i++].get();}
    VkExtent2D &getSwapChainExtent() {return swapChainExtent;}
    void finalize();
    void waitReady();
    void acquireNextFrame();
    const VkPhysicalDeviceFeatures &getDeviceFeatures() {return deviceFeatures;}
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const uint32_t firstIndex = 0, VkMemoryPropertyFlags preferedProperties = 0, bool *isOptimal = nullptr);
    VkPipelineCache &getPipelineCache() {return pipelineCache;}
    void submit(CommandMgr *cmdMgr);
    //! Only MemoryManager should use this method
    VkPhysicalDevice getPhysicalDevice() {return physicalDevice;}
    void setupInterceptor(void *_pUserData, void(*_interceptor)(void *pUserData, void *pData, uint32_t width, uint32_t height));
    void intercept() {interceptNextFrame = true;}
    void setObjectName(void *handle, VkObjectType type, const std::string &name);
    //! Called by MemoryManager when getting low of memory,
    void releaseUnusedMemory();
    void setTextureMgr(TextureMgr *texMgr) {textureMgr = texMgr;}

    //! getDevice();
    const VkDevice &refDevice;
    //! getRenderPass();
    const std::vector<VkRenderPass> &refRenderPass;
    const std::vector<VkRenderPass> &refRenderPassExternal;
    const std::vector<VkRenderPass> &refRenderPassCompatibility;
    //! getSwapChainFramebuffers();
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const std::vector<VkFramebuffer> &refResolveFramebuffers;
    const std::vector<VkFramebuffer> &refSingleSampleFramebuffers;
    const uint32_t &refFrameIndex;
    const VkSemaphore &refImageAvailableSemaphore;
private:
    const char *AppName;
    const char *EngineName;
    std::string cachePath;
    MemoryManager *memoryManager;
    TextureMgr *textureMgr;
    VkPipelineViewportStateCreateInfo viewportState{};
    VkViewport viewport{};
    VkRect2D scissor{};
    uint32_t frameIndex;
    int switcher = 0; // switch between MAX_FRAME_IN_FLIGHT elements
    int assignedGraphicQueueCount = 0;
    std::atomic<int> selectedGraphicQueue = 0; // selected graphic queue for submissions
    std::atomic<int> selectedComputeQueue = 0; // selected compute queue for submissions
    VkSemaphore imageAvailableSemaphore;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::vector<VkSemaphore> presentSemaphores;
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkPipelineCache pipelineCache;

    void initDevice(const char *AppName, const char *EngineName, SDL_Window *window, bool _hasLayer = false);
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;

    void initWindow(SDL_Window *window);
    VkSurfaceKHR surface;

    std::vector<size_t> graphicsAndPresentQueueFamilyIndex;
    std::vector<size_t> graphicsQueueFamilyIndex;
    std::vector<size_t> computeQueueFamilyIndex;
    std::vector<size_t> presentQueueFamilyIndex;
    std::vector<size_t> transferQueueFamilyIndex;
    std::vector<VkQueue> graphicsAndPresentQueues;
    std::vector<VkQueue> graphicsQueues;
    std::vector<VkQueue> computeQueues;
    std::vector<VkQueue> presentQueues;
    std::vector<VkQueue> transferQueues;
    VkDevice device;

    void initSwapchain(int width, int height, int nbVirtualSurfaces);
    std::vector<VkSwapchainKHR> swapChain;
    uint32_t finalImageCount;
    std::vector<uint32_t> imageIndex;
    std::vector<VkImage> swapChainImages;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    void createImageViews();
    std::vector<VkImageView> swapChainImageViews;

    void createMultisample(VkSampleCountFlagBits sampleCount);
    std::vector<VkImage> multisampleImage;
    std::vector<VkImageView> multisampleImageView;
    std::vector<SubMemory> multisampleImageMemory;

    void createRenderPass(VkSampleCountFlagBits sampleCount);
    std::vector<VkRenderPass> renderPass;
    std::vector<VkRenderPass> renderPassExternal;
    std::vector<VkRenderPass> renderPassCompatibility;

    void createFramebuffer(VkSampleCountFlagBits sampleCount);
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkFramebuffer> resolveFramebuffers;
    std::vector<VkFramebuffer> singleSampleFramebuffers;

    void createVirtualSurfaces(VkSampleCountFlagBits sampleCount);
    std::vector<std::unique_ptr<VirtualSurface>> virtualSurface;

    void createCommandPool();
    VkCommandPool commandPool;

    void createDepthResources(VkSampleCountFlagBits sampleCount);
    VkImage depthImage;
    SubMemory depthImageMemory;
    VkImageView depthImageView;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<std::thread> transferThread;
    std::queue<std::pair<VkSubmitInfo, VkFence>> transferQueue;
    std::mutex transferQueueMutex;
    uint8_t transferActivity = 0;
    std::atomic<uint8_t> isTransferIdle = true;
    std::thread graphicThread;
    std::queue<CommandMgr *> graphicQueue;
    std::mutex graphicQueueMutex;
    uint8_t graphicActivity = 0;

    // Interception
    void *pUserData;
    void(*interceptor)(void *pUserData, void *pData, uint32_t width, uint32_t height) = nullptr;
    VkBuffer interceptBuffer;
    SubMemory interceptBufferMemory;
    VkCommandPool interceptPool;
    std::vector<VkSubmitInfo> interceptSubmitInfo;
    std::vector<VkCommandBuffer> interceptCmdBuffer;
    std::vector<VkSemaphore> interceptEndSemaphores;
    std::vector<void *> pInterceptBufferData;
    std::vector<VkMappedMemoryRange> interceptMemoryRange;
    bool interceptNextFrame = false;
    const VkPipelineStageFlags interceptStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    // Debug
    void initDebug(vk::InstanceCreateInfo *instanceCreateInfo);
    void startDebug();
    void destroyDebug();
    static void printDebug(std::ostringstream &oss, std::string identifier);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void * /*pUserData*/);
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VkDebugUtilsMessengerEXT callback;
    void displayPhysicalDeviceInfo(VkPhysicalDeviceProperties &prop);
    void displayEnabledFeaturesInfo();
    PFN_vkSetDebugUtilsObjectNameEXT ptr_vkSetDebugUtilsObjectNameEXT = nullptr;

    bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
    bool isDeviceSuitable(VkPhysicalDevice pDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    std::vector<const char *> instanceExtension = {"VK_KHR_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
    std::vector<const char *> deviceExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_push_descriptor", "VK_EXT_conditional_rendering"};
    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    bool hasLayer;
    static bool isAlive;
    bool isReady = false;
};

#endif
