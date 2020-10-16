#include "Vulkan.hpp"
#include "TextureMgr.hpp"
#include <mutex>
#include "MemoryManager.hpp"
#include "tools/log.hpp"
// for default sampler
#include "PipelineLayout.hpp"

TextureMgr::TextureMgr(Vulkan *_master, uint32_t _chunkSize) : master(_master), chunkSize(_chunkSize)
{
    PipelineLayout::DEFAULT_SAMPLER.anisotropyEnable = master->getDeviceFeatures().samplerAnisotropy;
    defaultSampler = createSampler(PipelineLayout::DEFAULT_SAMPLER);
    if (defaultSampler == VK_NULL_HANDLE) {
        cLog::get()->write("Failed to create default sampler, abort.", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        exit(1);
    }
}

TextureMgr::~TextureMgr()
{
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

TextureImage *TextureMgr::createImage(const std::pair<short, short> &size, bool mipmap = false, VkFormat format, VkImageUsageFlags usage, bool isDepthAttachment)
{
    // static std::mutex mtx;
    // std::lock_guard<std::mutex> lck(mtx);
    // if (format == VK_FORMAT_R8G8B8A8_SRGB && usage == VK_IMAGE_USAGE_SAMPLED_BIT && !availableImages[size].empty()) {
    //     TextureImage *tmp = new TextureImage(this, availableImages[size].back(), size);
    //     availableImages[size].pop_back();
    //     return tmp;
    // }
    // create image
    VkImage image;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.first;
    imageInfo.extent.height = size.second;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipmap ? static_cast<uint32_t>(std::floor(std::log2(std::max(size.first, size.second)))) + 1 : 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(master->refDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        cLog::get()->write("Faild to create VkImage", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        return nullptr;
    }

    VkImageView imageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
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

    // // get memory requirements
    // VkMemoryRequirements memRequirements;
    // vkGetImageMemoryRequirements(master->refDevice, image, &memRequirements);
    // SubMemory memory;
    // memory.memory = VK_NULL_HANDLE;
    //
    // if (format != VK_FORMAT_R8G8B8A8_SRGB || usage != (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)) {
    //     memory = master->getMemoryManager()->malloc(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //     if (memory.memory == VK_NULL_HANDLE)
    //         return nullptr;
    //     vkBindImageMemory(master->refDevice, image, memory.memory, memory.offset);
    //     return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size, &memory);
    // }
    //
    // // search memory to bind
    // for (auto &mem : allocatedMemory) {
    //     int offset = ((mem.second - 1) / memRequirements.alignment + 1) * memRequirements.alignment;
    //     if (offset + memRequirements.size <= chunkSize) {
    //         vkBindImageMemory(master->refDevice, image, mem.first.memory, mem.first.offset + offset);
    //         mem.second = offset + memRequirements.size;
    //         return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size);
    //     }
    // }
    // // create new memory space
    // if (memRequirements.size > chunkSize) {
    //     std::cerr << "Error : Image memory size is bigger than chunk size" << std::endl;
    //     vkDestroyImageView(master->refDevice, imageView, nullptr);
    //     vkDestroyImage(master->refDevice, image, nullptr);
    //     return nullptr;
    // }
    //
    // memory = master->getMemoryManager()->malloc(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //
    // if (memory.memory == VK_NULL_HANDLE) {
    //     std::cerr << "Failed to allocate chunk of memory for textures." << std::endl;
    //     vkDestroyImageView(master->refDevice, imageView, nullptr);
    //     vkDestroyImage(master->refDevice, image, nullptr);
    //     return nullptr;
    // }
    // allocatedMemory.push_back({memory, memRequirements.size});
    // vkBindImageMemory(master->refDevice, image, memory.memory, memory.offset);
    // return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size);
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
