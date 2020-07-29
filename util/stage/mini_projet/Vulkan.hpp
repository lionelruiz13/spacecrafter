#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan {
public:
    Vulkan(const char *_AppName, const char *_EngineName, GLFWwindow *window);
    ~Vulkan();
    void initQueues(short nbQueues = 1);
private:
    const char *AppName;
    const char *EngineName;

    void initDevice(const char *AppName, const char *EngineName, GLFWwindow *window, bool _hasLayer = false);
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;

    void initWindow(GLFWwindow *window);
    VkSurfaceKHR surface;

    std::vector<size_t> graphicsAndPresentQueueFamilyIndex;
    std::vector<size_t> graphicsQueueFamilyIndex;
    std::vector<size_t> presentQueueFamilyIndex;
    std::vector<VkQueue> graphicsAndPresentQueues;
    std::vector<VkQueue> graphicsQueues;
    std::vector<VkQueue> presentQueues;
    VkDevice device;

    void initSwapchain();

    /*
    void initCommandBuffer();
    vk::UniqueCommandPool commandPool;
    vk::UniqueCommandBuffer commandBuffer;
    //*/

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
