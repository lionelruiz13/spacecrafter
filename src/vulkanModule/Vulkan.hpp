#ifndef VULKAN_HPP_
#define VULKAN_HPP_

#include <vulkan/vulkan.hpp>
#include "SubMemory.hpp"
#include "tools/vecmath.hpp"
#include <array>

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

class Vulkan {
public:
    Vulkan(const char *_AppName, const char *_EngineName, SDL_Window *window, int nbVirtualSurfaces, int width = 600, int height = 600, int chunkSize = 256*1024*1024, bool enableDebugLayers = true);
    ~Vulkan();
    void initQueues(uint32_t nbQueues = 1);
    void sendFrame();
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    //! for malloc
    MemoryManager *getMemoryManager() {return memoryManager;}
    void free(SubMemory& bufferMemory);
    void mapMemory(SubMemory& bufferMemory, void **data);
    void unmapMemory(SubMemory& bufferMemory);
    size_t getTransferQueueFamilyIndex() {return transferQueueFamilyIndex[0];}
    void submitTransfer(VkSubmitInfo *submitInfo);
    VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    VkQueue assignGraphicsQueue() {static short i = 0; return graphicsAndPresentQueues[i++];}
    VkSwapchainKHR *assignSwapChain() {return swapChain.data();}
    void assignSwapChainFramebuffers(std::vector<VkFramebuffer> &framebuffers, int index) {framebuffers.assign(swapChainFramebuffers.begin(), swapChainFramebuffers.end());}
    auto &getGraphicsQueueIndex() {return graphicsAndPresentQueueFamilyIndex[0];}
    VirtualSurface *getVirtualSurface() {static short i = 0; return virtualSurface[i++].get();}
    VkExtent2D &getSwapChainExtent() {return swapChainExtent;}
    void finalize();
    void waitReady();
    void acquireNextFrame();
    const VkPhysicalDeviceFeatures &getDeviceFeatures() {return deviceFeatures;}
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const uint32_t firstIndex = 0, VkMemoryPropertyFlags preferedProperties = 0, bool *isOptimal = nullptr);
    VkPipelineCache &getPipelineCache() {return pipelineCache;}
    //! Only MemoryManager should use this method
    VkPhysicalDevice getPhysicalDevice() {return physicalDevice;}

    //! getDevice();
    const VkDevice &refDevice;
    //! getRenderPass();
    const std::vector<VkRenderPass> &refRenderPass;
    //! getSwapChainFramebuffers();
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const uint32_t &refFrameIndex;
    const VkSemaphore &refImageAvailableSemaphore;
private:
    const char *AppName;
    const char *EngineName;
    MemoryManager *memoryManager;
    VkPipelineViewportStateCreateInfo viewportState{};
    VkViewport viewport{};
    VkRect2D scissor{};
    uint32_t frameIndex;
    int switcher = 0; // switch between MAX_FRAME_IN_FLIGHT elements
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
    std::vector<size_t> presentQueueFamilyIndex;
    std::vector<size_t> transferQueueFamilyIndex;
    std::vector<VkQueue> graphicsAndPresentQueues;
    std::vector<VkQueue> graphicsQueues;
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

    void createRenderPass();
    std::vector<VkRenderPass> renderPass;

    void createFramebuffer();
    std::vector<VkFramebuffer> swapChainFramebuffers;

    void createVirtualSurfaces();
    std::vector<std::unique_ptr<VirtualSurface>> virtualSurface;

    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    void createCommandPool();
    VkCommandPool commandPool;

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool cmdPool = VK_NULL_HANDLE);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue = VK_NULL_HANDLE);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue queue = VK_NULL_HANDLE, VkImageAspectFlagBits aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createCommandBuffer();
    std::vector<VkCommandBuffer> commandBuffers;

    void createDepthResources();
    VkImage depthImage;
    SubMemory depthImageMemory;
    VkImageView depthImageView;

    void createVertexBuffer();
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    void createIndexBuffer();
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    void createDescriptorSetLayout();
    VkDescriptorSetLayout descriptorSetLayout;
    //VkPipelineLayout pipelineLayout;

    void createUniformBuffers();
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    void createDescriptorPool();
    VkDescriptorPool descriptorPool;

    void createDescriptorSets();
    std::vector<VkDescriptorSet> descriptorSets;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    void initDebug(vk::InstanceCreateInfo *instanceCreateInfo);
    void startDebug();
    void destroyDebug();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void * /*pUserData*/);
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VkDebugUtilsMessengerEXT callback;

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
