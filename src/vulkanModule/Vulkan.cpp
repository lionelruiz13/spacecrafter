#include "Vulkan.hpp"
#include "VirtualSurface.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include "mainModule/sdl_facade.hpp"
#include <SDL2/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool Vulkan::isAlive = false;

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    1, 0, 2, 2, 3, 1
};

Vulkan::Vulkan(const char *_AppName, const char *_EngineName, SDL_Window *window, int nbVirtualSurfaces, int width, int height) : refDevice(device), refRenderPass(renderPass), refSwapChainFramebuffers(swapChainFramebuffers), refFrameIndex(frameIndex), refImageAvailableSemaphore(imageAvailableSemaphore), AppName(_AppName), EngineName(_EngineName)
{
    if (isAlive) {
        std::cerr << "Error: There must be only one Vulkan instance" << std::endl;
        return;
    }
    isAlive = true;
    uint32_t sdl2ExtensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &sdl2ExtensionCount, nullptr))
        std::runtime_error("Fatal : Faild to found Vulkan extension for SDL2.");

    size_t initialSize = instanceExtension.size();
    instanceExtension.resize(initialSize + sdl2ExtensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &sdl2ExtensionCount, instanceExtension.data() + initialSize))
        std::runtime_error("Fatal : Faild to found Vulkan extension for SDL2.");

    initDevice(_AppName, _EngineName, window, true);
    initQueues(nbVirtualSurfaces);
    initSwapchain(width, height, nbVirtualSurfaces);
    createImageViews();
    createRenderPass();
    createCommandPool();
    createDepthResources();
    createFramebuffer();
    createVirtualSurfaces();

    // Déformation de l'image
    viewport.width = (float) std::min(swapChainExtent.width, swapChainExtent.height);
    viewport.height = viewport.width;
    viewport.x = (swapChainExtent.width - viewport.width) / 2.f;
    viewport.y = (swapChainExtent.height - viewport.height) / 2.f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Découpage de l'image
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // Viewport
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = VK_SEMAPHORE_TYPE_BINARY;
    for (uint16_t i = 0; i < imageAvailableSemaphores.size(); i++) {
        vkCreateSemaphore(refDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
    }
    VkPipelineCacheCreateInfo cacheCreateInfo;
    cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cacheCreateInfo.pNext = nullptr;
    cacheCreateInfo.flags = 0;
    cacheCreateInfo.initialDataSize = 0;
    cacheCreateInfo.pInitialData = nullptr;
    if (vkCreatePipelineCache(device, &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS) {
        std::cerr << "Faild to create pipeline cache.";
    }
}

Vulkan::~Vulkan()
{
    isAlive = false;
    vkDeviceWaitIdle(device);
    std::cout << "Release resources\n";
    virtualSurface.clear(); // Release all attached resources
    for (auto &sem : imageAvailableSemaphores)
        vkDestroySemaphore(device, sem, nullptr);
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto &tmp : renderPass)
        vkDestroyRenderPass(device, tmp, nullptr);
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto &tmp : swapChain)
        vkDestroySwapchainKHR(device, tmp, nullptr);
    vkDestroySurfaceKHR(instance.get(), surface, nullptr);
    vkDestroyPipelineCache(device, pipelineCache, nullptr);
    vkDestroyDevice(device, nullptr);
    if (hasLayer)
        destroyDebug();
}

void Vulkan::submitTransfer(VkSubmitInfo *submitInfo)
{
    static std::mutex mtx;
    static int dispatch = 0;
    mtx.lock();
    vkQueueSubmit(transferQueues[dispatch], 1, submitInfo, VK_NULL_HANDLE);
    dispatch = (dispatch + 1) % transferQueues.size();
    mtx.unlock();
}

void Vulkan::finalize()
{
    presentSemaphores.resize(swapChainFramebuffers.size());
    for (uint8_t frame = 0; frame < swapChainFramebuffers.size(); frame++) {
        virtualSurface.front()->waitReady();
        for (uint8_t i = 1; i < virtualSurface.size(); i++) {
            virtualSurface[i]->waitReady();
            virtualSurface[i]->link(frame, virtualSurface[i - 1].get());
        }
        presentSemaphores[frame] = virtualSurface.back()->getBottomSemaphore(frame);
    }
    acquireNextFrame();
    isReady = true;
}

void Vulkan::waitReady()
{
    while (!isReady)
        std::this_thread::yield();
}

SwapChainSupportDetails Vulkan::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool Vulkan::checkDeviceExtensionSupport(VkPhysicalDevice pDevice) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtension.begin(), deviceExtension.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

bool Vulkan::isDeviceSuitable(VkPhysicalDevice pDevice) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0)
        return false;

    bool extensionsSupported = checkDeviceExtensionSupport(pDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return extensionsSupported && swapChainAdequate;
}

void Vulkan::initDevice(const char *AppName, const char *EngineName, SDL_Window *window, bool _hasLayer)
{
    hasLayer = _hasLayer;

    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo applicationInfo( AppName, 1, EngineName, 1, VK_API_VERSION_1_1);

    // initialize the vk::InstanceCreateInfo
    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, hasLayer ? validationLayers.size() : 0, validationLayers.data(), instanceExtension.size(), instanceExtension.data());

    if (hasLayer)
        initDebug(&instanceCreateInfo);

    // create a UniqueInstance
    instance = vk::createInstanceUnique(instanceCreateInfo);

    if (hasLayer)
        startDebug();

    initWindow(window);

    // get a physicalDevice
    for (const auto &pDevice : instance->enumeratePhysicalDevices()) {
        if (isDeviceSuitable(pDevice)) {
            physicalDevice = pDevice;
            if (pDevice.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                break;
            }
        }
    }
}

void Vulkan::initQueues(uint32_t nbQueues)
{
    // get the QueueFamilyProperties of PhysicalDevice
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // {TO CHECK} register all index into queueFamiliyProperties which supports graphics, present, or both
    size_t i = 0;
    std::vector<float> queuePriority(nbQueues, 0.0f);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pQueuePriorities = queuePriority.data();
    queueCreateInfo.pNext = nullptr;

    for (auto &qfp : queueFamilyProperties) {
        VkBool32 presentSupport = true;
        queueCreateInfo.queueCount = std::min(qfp.queueCount, nbQueues);
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (qfp.queueFlags & vk::QueueFlagBits::eGraphics) {
            if (presentSupport)
                graphicsAndPresentQueueFamilyIndex.push_back(i);
            else
                graphicsQueueFamilyIndex.push_back(i);
        } else if (presentSupport && !(qfp.queueFlags & vk::QueueFlagBits::eCompute))
            presentQueueFamilyIndex.push_back(i);
        else if (qfp.queueFlags & vk::QueueFlagBits::eTransfer && !(qfp.queueFlags & vk::QueueFlagBits::eCompute))
            transferQueueFamilyIndex.push_back(i);
        else {
            i++;
            continue;
        }
        // Build createInfo for this familyQueue
        queueCreateInfo.queueFamilyIndex = i;
        queueCreateInfos.push_back(queueCreateInfo);
        i++;
    }
    assert(graphicsQueueFamilyIndex.size() + graphicsAndPresentQueueFamilyIndex.size() > 0);
    assert(presentQueueFamilyIndex.size() + graphicsAndPresentQueueFamilyIndex.size() > 0);
    assert(transferQueueFamilyIndex.size() > 0);

    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedDeviceFeatures);

    deviceFeatures.geometryShader = supportedDeviceFeatures.geometryShader;
    deviceFeatures.tessellationShader = supportedDeviceFeatures.tessellationShader;
    deviceFeatures.samplerAnisotropy = supportedDeviceFeatures.samplerAnisotropy;
    deviceFeatures.multiDrawIndirect = supportedDeviceFeatures.multiDrawIndirect;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtension.size();
    createInfo.ppEnabledExtensionNames = deviceExtension.data();
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    int maxQueues;
    VkQueue tmpQueue;
    for (auto index : graphicsAndPresentQueueFamilyIndex) {
        maxQueues = std::min(queueFamilyProperties[index].queueCount, nbQueues);
        for (int j = 0; j < maxQueues; j++) {
            std::cout << "Create graphicAndPresentQueue\n";
            vkGetDeviceQueue(device, index, j, &tmpQueue);
            graphicsAndPresentQueues.push_back(tmpQueue);
        }
    }
    for (auto index : graphicsQueueFamilyIndex) {
        maxQueues = std::min(queueFamilyProperties[index].queueCount, nbQueues);
        for (int j = 0; j < maxQueues; j++) {
            std::cout << "Create graphicQueue\n";
            vkGetDeviceQueue(device, index, j, &tmpQueue);
            graphicsQueues.push_back(tmpQueue);
        }
    }
    for (auto index : presentQueueFamilyIndex) {
        maxQueues = std::min(queueFamilyProperties[index].queueCount, nbQueues);
        for (int j = 0; j < maxQueues; j++) {
            std::cout << "Create presentQueue\n";
            vkGetDeviceQueue(device, index, j, &tmpQueue);
            presentQueues.push_back(tmpQueue);
        }
    }
    for (auto index : transferQueueFamilyIndex) {
        maxQueues = std::min(queueFamilyProperties[index].queueCount, nbQueues);
        for (int j = 0; j < maxQueues; j++) {
            std::cout << "Create transferQueue\n";
            vkGetDeviceQueue(device, index, j, &tmpQueue);
            transferQueues.push_back(tmpQueue);
        }
    }
}

void Vulkan::initWindow(SDL_Window *window)
{
    if (SDL_Vulkan_CreateSurface(window, instance.get(), &surface) != SDL_TRUE) {
        throw std::runtime_error("failed to create window surface!");
    }
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
    // Real size is not always supported
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {width, height};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void Vulkan::initSwapchain(int width, int height, int nbVirtualSurfaces)
{
    //nbVirtualSurfaces = 1;
    swapChain.resize(nbVirtualSurfaces);
    imageIndex.resize(nbVirtualSurfaces);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities, width, height);
    swapChainImageFormat = surfaceFormat.format;

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1; // Pas besoin de 3D stéréoscopique
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Transfert nécessaire pour l'affichage
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Pas de transparence pour le contenu de la fenêtre
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_FALSE; // ne pas calculer les pixels masqués par une autre fenêtre
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    for (uint8_t i = 0; i < swapChain.size(); i++) {
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain[i]) != VK_SUCCESS) {
            throw std::runtime_error("échec de la création de la swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain[i], &finalImageCount, nullptr);
        swapChainImages.resize(finalImageCount * (i + 1));
        vkGetSwapchainImagesKHR(device, swapChain[i], &finalImageCount, swapChainImages.data() + finalImageCount * i);
    }
}

VkImageView Vulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("échec de la creation de la vue sur une image!");
    }

    return imageView;
}

void Vulkan::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
    }
}

void Vulkan::createVirtualSurfaces()
{
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        virtualSurface.push_back(std::make_unique<VirtualSurface>(this, i));
    }
}

void Vulkan::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    //colorAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    //depthAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments{colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass[0]) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la render pass!");
    }
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass[1]) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la render pass!");
    }
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass[2]) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la render pass!");
    }
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass[3]) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la render pass!");
    }
}

void Vulkan::createFramebuffer()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass[1];
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("échec de la création d'un framebuffer!");
        }
    }
}

void Vulkan::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = 0; // Optionel
    poolInfo.queueFamilyIndex = graphicsAndPresentQueueFamilyIndex[0];
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création d'une command pool !");
    }
}

void Vulkan::sendFrame()
{
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        virtualSurface[i]->getNextFrame();
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &presentSemaphores[frameIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChain.data();
    presentInfo.pImageIndices = &frameIndex;
    presentInfo.pResults = nullptr; // Optionnel
    if (vkQueuePresentKHR(graphicsAndPresentQueues[0], &presentInfo) != VK_SUCCESS) {
        std::cerr << "Presentation FAILED\n";
    }
    acquireNextFrame();
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        virtualSurface[i]->releaseFrame();
    }
}

void Vulkan::acquireNextFrame()
{
    VkResult result = vkAcquireNextImageKHR(device, swapChain[0], UINT64_MAX, imageAvailableSemaphores[switcher], VK_NULL_HANDLE, &frameIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        std::runtime_error("Faild to acquire next frame.");
    virtualSurface[0]->setTopSemaphore(frameIndex, imageAvailableSemaphores[switcher]);
    switcher = (switcher + 1) % MAX_FRAMES_IN_FLIGHT;
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const uint32_t firstIndex, VkMemoryPropertyFlags preferedProperties, bool *isOptimal)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    preferedProperties |= properties;

    for (uint32_t i = firstIndex; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & preferedProperties) == preferedProperties) {
            if (isOptimal)
                *isOptimal = true;
            return i;
        }
    }

    for (uint32_t i = firstIndex; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    if (firstIndex == 0)
        throw std::runtime_error("Expected memory type for buffer is not available.");
    return UINT32_MAX;
}

bool Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'un buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    bool isOptimal = false;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, 0, preferedProperties, &isOptimal);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation de memoire!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    return isOptimal;
}

VkCommandBuffer Vulkan::beginSingleTimeCommands(VkCommandPool cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (queue == VK_NULL_HANDLE) queue = transferQueues[0];
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Vulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue queue, VkImageAspectFlagBits aspectMask)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = graphicsAndPresentQueueFamilyIndex[0];
    barrier.dstQueueFamilyIndex = transferQueueFamilyIndex[0];
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    endSingleTimeCommands(commandBuffer, queue);
}

void Vulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    endSingleTimeCommands(commandBuffer);

    commandBuffer = beginSingleTimeCommands(commandPool);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    endSingleTimeCommands(commandBuffer, graphicsAndPresentQueues[0]);
}

void Vulkan::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Vulkan::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Vulkan::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding uniformCollection{};
    uniformCollection.binding = 0;
    uniformCollection.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformCollection.descriptorCount = 1;
    uniformCollection.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uniformCollection.pImmutableSamplers = nullptr; // Optionnel

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniformCollection, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'un set de descripteurs!");
    }
}

void Vulkan::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }
}

void Vulkan::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
    }
}


void Vulkan::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImages.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
    }
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Vulkan::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'une image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void Vulkan::createDepthResources()
{
    createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = depthImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    endSingleTimeCommands(commandBuffer, graphicsAndPresentQueues[0]);
}

// =============== DEBUG =============== //
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Vulkan::initDebug(vk::InstanceCreateInfo *instanceCreateInfo)
{
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;
    debugCreateInfo.pUserData = nullptr; // Optionnel

    instanceCreateInfo->setPNext(&debugCreateInfo);
}

void Vulkan::startDebug()
{
    std::vector<vk::LayerProperties>     layerProperties     = vk::enumerateInstanceLayerProperties();
    std::vector<vk::ExtensionProperties> extensionProperties = vk::enumerateInstanceExtensionProperties();

    if (CreateDebugUtilsMessengerEXT(instance.get(), &debugCreateInfo, nullptr, &callback) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Vulkan::destroyDebug()
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance.get(), callback, nullptr);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void * /*pUserData*/)
{
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        && pCallbackData->pMessageIdName == std::string("Loader Message"))
        return VK_TRUE;
    std::cerr << vk::to_string( static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>( messageSeverity ) ) << ": "
        << vk::to_string( static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageTypes ) ) << ":\n";
    std::cerr << "\t" << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    std::cerr << "\t" << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    std::cerr << "\t" << "message         = <" << pCallbackData->pMessage << ">\n";
    if (0 < pCallbackData->queueLabelCount) {
        std::cerr << "\t" << "Queue Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++) {
            std::cerr << "\t\t" << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount) {
        std::cerr << "\t" << "CommandBuffer Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
            std::cerr << "\t\t" << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->objectCount) {
        std::cerr << "\t" << "Objects:\n";
        for ( uint8_t i = 0; i < pCallbackData->objectCount; i++ )
        {
            std::cerr << "\t\t" << "Object " << (int) i << "\n";
            std::cerr << "\t\t\t" << "objectType   = "
            << vk::to_string( static_cast<vk::ObjectType>( pCallbackData->pObjects[i].objectType ) ) << "\n";
            std::cerr << "\t\t\t" << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if (pCallbackData->pObjects[i].pObjectName) {
                std::cerr << "\t\t\t" << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
            }
        }
    }
    return VK_TRUE;
}
