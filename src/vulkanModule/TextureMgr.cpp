#include "Vulkan.hpp"
#include "TextureMgr.hpp"
#include <thread>
#include "MemoryManager.hpp"
#include "tools/log.hpp"
#include "CommandMgr.hpp"
// for default sampler
#include "PipelineLayout.hpp"

TextureMgr::TextureMgr(Vulkan *_master, uint32_t _chunkSize) : master(_master), chunkSize(_chunkSize)
{
    master->setTextureMgr(this);
    PipelineLayout::DEFAULT_SAMPLER.anisotropyEnable = master->getDeviceFeatures().samplerAnisotropy;
    defaultSampler = createSampler(PipelineLayout::DEFAULT_SAMPLER);
    if (defaultSampler == VK_NULL_HANDLE) {
        cLog::get()->write("Failed to create default sampler, abort.", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        exit(1);
    }
    syncObject.resize(4);
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (auto &syncObj : syncObject) {
        if (vkCreateFence(master->refDevice, &fenceInfo, nullptr, &syncObj.first) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence.");
        }
        if (vkCreateSemaphore(master->refDevice, &semaphoreInfo, nullptr, &syncObj.second) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphore.");
        }
    }
    cmdMgrSingleUse = std::make_unique<CommandMgr>(_master);
}

TextureMgr::~TextureMgr()
{
    for (auto &syncObj : syncObject) {
        vkDestroyFence(master->refDevice, syncObj.first, nullptr);
        vkDestroySemaphore(master->refDevice, syncObj.second, nullptr);
    }
    for (auto &imageType : availableImages) {
        for (auto &image : imageType.second) {
            vkDestroyImageView(master->refDevice, image.second, nullptr);
            vkDestroyImage(master->refDevice, image.first, nullptr);
        }
    }
    for (auto &mem : allocatedMemory) {
        master->free(mem.first);
    }
    for (auto &sampler : samplers) {
        vkDestroySampler(master->refDevice, sampler.second, nullptr);
    }
}

std::unique_ptr<TextureImage> TextureMgr::queryImage(TextureImage *ptr)
{
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it->get() == ptr) {
            std::unique_ptr<TextureImage> tmp = std::move(*it);
            cache.erase(it);
            return tmp;
        }
    }
    return nullptr;
}

void TextureMgr::cacheImage(std::unique_ptr<TextureImage> &textureImage)
{
    cache.push_back(std::move(textureImage));
}
void TextureMgr::releaseCachedTextures()
{
    cache.clear();
}

int TextureMgr::getSamplerID(const VkSamplerCreateInfo &samplerInfo)
{
    int samplerID = static_cast<int>(samplerInfo.addressModeU);
    samplerID |= static_cast<int>(samplerInfo.magFilter) << 2;
    samplerID |= static_cast<int>(samplerInfo.minFilter) << 4;
    samplerID |= static_cast<int>(samplerInfo.maxLod) << 6;
    return samplerID;
}

VkSampler TextureMgr::createSampler(const VkSamplerCreateInfo &samplerInfo)
{
    int samplerID = getSamplerID(samplerInfo);
    VkSampler sampler = samplers[samplerID];
    if (sampler)
        return sampler;
    if (vkCreateSampler(master->refDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        sampler = defaultSampler;
        cLog::get()->write("Faild to create sampler, fallback to default.", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
    }
    samplers[samplerID] = sampler;
    return sampler;
}

void TextureMgr::acquireSyncObject(VkFence *fence, VkSemaphore *semaphore)
{
    while (syncObject.empty())
        std::this_thread::yield();
    syncObjectAccess.lock();
    auto &tmp = syncObject.back();
    *fence = tmp.first;
    *semaphore = tmp.second;
    syncObject.pop_back();
    syncObjectAccess.unlock();
}

void TextureMgr::releaseSyncObject(VkFence fence, VkSemaphore semaphore)
{
    syncObjectAccess.lock();
    syncObject.emplace_back(fence, semaphore);
    syncObjectAccess.unlock();
}

TextureImage *TextureMgr::createImage(const std::pair<short, short> &size, uint32_t depth, bool mipmap, VkFormat format, VkImageUsageFlags usage, bool isDepthAttachment, bool useConcurrency)
{
    // create image
    VkImage image;
    VkImageCreateInfo imageInfo{};
    uint32_t indices[2];
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = (depth > 1) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.first;
    imageInfo.extent.height = size.second;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = mipmap ? static_cast<uint32_t>(std::floor(std::log2(std::max(size.first, size.second)))) + 1 : 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = mipmap ? (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage) : usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    if (useConcurrency) {
        indices[0] = master->getGraphicsQueueIndex();
        indices[1] = master->getTransferQueueFamilyIndex();
        imageInfo.queueFamilyIndexCount = 2;
        imageInfo.pQueueFamilyIndices = indices;
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    } else
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(master->refDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        cLog::get()->write("Faild to create VkImage", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        return nullptr;
    }

    VkImageView imageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = (depth > 1) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = (isDepthAttachment) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(master->refDevice, image, &memRequirements);
    SubMemory memory = master->getMemoryManager()->malloc(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memory.memory == VK_NULL_HANDLE) {
        vkDestroyImage(master->refDevice, image, nullptr);
        return nullptr;
    }
    if (vkBindImageMemory(master->refDevice, image, memory.memory, memory.offset) != VK_SUCCESS) {
        cLog::get()->write("Faild to bind memory to VkImage", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        vkDestroyImage(master->refDevice, image, nullptr);
        master->free(memory);
        return nullptr;
    }
    if (vkCreateImageView(master->refDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        cLog::get()->write("Faild to create VkImageView", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        vkDestroyImage(master->refDevice, image, nullptr);
        master->free(memory);
        return nullptr;
    }
    return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size, &memory);
}

void TextureMgr::releaseImage(const std::pair<VkImage, VkImageView> &image, const std::pair<short, short> &size)
{
    availableImages[size].push_back(image);
}

void TextureMgr::destroyImage(const VkImage &image, const VkImageView &imageView, SubMemory &memory)
{
    vkDestroyImageView(master->refDevice, imageView, nullptr);
    vkDestroyImage(master->refDevice, image, nullptr);
    master->free(memory);
}

TextureImage::TextureImage(TextureMgr *_master, const std::pair<VkImage, VkImageView> &_image, const std::pair<short, short> &_size, SubMemory *_memory) : master(_master), image(_image.first), imageView(_image.second), size(_size)
{
    if (_memory) {
        memory = *_memory;
    } else {
        memory.memory = VK_NULL_HANDLE;
    }
}

TextureImage::~TextureImage()
{
    if (memory.memory == VK_NULL_HANDLE)
        master->releaseImage(std::pair<VkImage, VkImageView>(image, imageView), size);
    else
        master->destroyImage(image, imageView, memory);
}
