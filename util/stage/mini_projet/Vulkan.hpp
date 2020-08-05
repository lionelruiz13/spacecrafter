#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "tools/vecmath.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan {
public:
    Vulkan(const char *_AppName, const char *_EngineName, GLFWwindow *window);
    ~Vulkan();
    void initQueues(uint32_t nbQueues = 1);
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage, float degreePerSecond);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandPool getTransferPool() {return transferCommandPool[0];}
    VkQueue getTransferQueue() {return transferQueues[0];}
    VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    VkRenderPass &getRenderPass() {return renderPass;}
    VkQueue assignGraphicsQueue() {static short i = 1; return graphicsAndPresentQueues[i++];}
    auto &getGraphicsQueueIndex() {return graphicsAndPresentQueueFamilyIndex[0];}

    //! getDevice();
    const VkDevice &refDevice;
    //! getRenderPass();
    const VkRenderPass &refRenderPass;
    //! getSwapChainFramebuffers();
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const int &refFrameIndex;
private:
    const char *AppName;
    const char *EngineName;
    VkPipelineViewportStateCreateInfo viewportState;
    int frameIndex;

    void initDevice(const char *AppName, const char *EngineName, GLFWwindow *window, bool _hasLayer = false);
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;

    void initWindow(GLFWwindow *window);
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

    void initSwapchain(int width, int height);
    std::vector<VkSwapchainKHR> swapChain;
    std::vector<uint32_t> imageIndex;
    std::vector<VkImage> swapChainImages;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    void createImageViews();
    std::vector<VkImageView> swapChainImageViews;

    void createRenderPass();
    VkRenderPass renderPass;

    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    void createFramebuffer();
    std::vector<VkFramebuffer> swapChainFramebuffers;

    void createCommandPool();
    std::vector<VkCommandPool> commandPool;
    std::vector<VkCommandPool> transferCommandPool;

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createCommandBuffer();
    std::vector<VkCommandBuffer> commandBuffers;

    void createSemaphore();
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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
    void createTextureImage();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView createImageView(VkImage image, VkFormat format);
    void createTextureImageView();
    VkImageView textureImageView;

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
    std::vector<const char *> instanceExtension = {"VK_KHR_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    std::vector<const char *> deviceExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    bool hasLayer;
    static bool isAlive;
};
