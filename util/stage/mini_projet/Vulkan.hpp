#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include "utils.hpp"

class Vulkan {
public:
    Vulkan(const char *_AppName, const char *_EngineName);
    ~Vulkan();
private:
    const char *AppName;
    const char *EngineName;

    void initDevice(const char *AppName, const char *EngineName, short nbQueues, short nbLayers=0);
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    size_t graphicsQueueFamilyIndex;
    vk::UniqueDevice device;

    void initWindow(int width, int height);
    GLFWwindow *window;
    vk::UniqueSurfaceKHR surface;

    void initCommandBuffer();
    vk::UniqueCommandPool commandPool;
    vk::UniqueCommandBuffer commandBuffer;

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

    const char * const layers[1] = {"VK_LAYER_KHRONOS_validation"};
    short hasLayer;
    static bool isAlive;
};
