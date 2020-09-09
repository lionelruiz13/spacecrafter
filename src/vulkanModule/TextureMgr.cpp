#include "Vulkan.hpp"
#include "TextureMgr.hpp"
#include <mutex>

TextureMgr::TextureMgr(Vulkan *_master, uint32_t _chunkSize) : master(_master), chunkSize(_chunkSize)
{
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
        vkFreeMemory(master->refDevice, mem.first, nullptr);
    }
    for (auto &sampler : samplers) {
        vkDestroySampler(master->refDevice, sampler.second, nullptr);
    }
}

int TextureMgr::getSamplerID(const VkSamplerCreateInfo &samplerInfo)
{
    int samplerID = static_cast<int>(samplerInfo.addressModeU);
    samplerID |= static_cast<int>(samplerInfo.magFilter) << 2;
    samplerID |= static_cast<int>(samplerInfo.minFilter) << 3;
    samplerID |= static_cast<int>(samplerInfo.maxLod) << 4;
    return samplerID;
}

VkSampler TextureMgr::createSampler(const VkSamplerCreateInfo &samplerInfo)
{
    int samplerID = getSamplerID(samplerInfo);
    VkSampler sampler = samplers[samplerID];
    if (sampler)
        return sampler;
    if (vkCreateSampler(master->refDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("échec de la creation d'un sampler!");
    }
    samplers[samplerID] = sampler;
    return sampler;
}

TextureImage *TextureMgr::createImage(const std::pair<short, short> &size, VkFormat format)
{
    static std::mutex mtx;
    mtx.lock();
    if (format == VK_FORMAT_R8G8B8A8_SRGB && !availableImages[size].empty()) {
        TextureImage *tmp = new TextureImage(this, availableImages[size].back(), size);
        availableImages[size].pop_back();
        mtx.unlock();
        return tmp;
    }
    // create image
    VkImage image;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.first;
    imageInfo.extent.height = size.second;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(master->refDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'une image!");
    }

    VkImageView imageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(master->refDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'une image!");
    }

    // get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(master->refDevice, image, &memRequirements);

    if (format != VK_FORMAT_R8G8B8A8_SRGB) {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = master->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        mtx.unlock();
        VkDeviceMemory memory;
        if (vkAllocateMemory(master->refDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS)
            return nullptr;
        vkBindImageMemory(master->refDevice, image, memory, 0);
        return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size, memory);
    }

    // search memory to bind
    VkDeviceMemory memory = VK_NULL_HANDLE;
    for (auto &mem : allocatedMemory) {
        int offset = ((mem.second - 1) / memRequirements.alignment + 1) * memRequirements.alignment;
        if (offset + memRequirements.size <= chunkSize) {
            vkBindImageMemory(master->refDevice, image, mem.first, offset);
            mem.second = offset + memRequirements.size;
            mtx.unlock();
            return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size);
        }
    }
    // create new memory space
    if (memRequirements.size > chunkSize) {
        mtx.unlock();
        std::cerr << "Error : Image memory size is bigger than chunk size" << std::endl;
        vkDestroyImageView(master->refDevice, imageView, nullptr);
        vkDestroyImage(master->refDevice, image, nullptr);
        return nullptr;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = chunkSize;
    allocInfo.memoryTypeIndex = master->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    while (vkAllocateMemory(master->refDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        allocInfo.memoryTypeIndex = master->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocInfo.memoryTypeIndex + 1);
        if (allocInfo.memoryTypeIndex == UINT32_MAX) {
            mtx.unlock();
            std::cerr << "Faild to allocate chunk of memory for textures." << std::endl;
            vkDestroyImageView(master->refDevice, imageView, nullptr);
            vkDestroyImage(master->refDevice, image, nullptr);
            return nullptr;
        }
    }
    allocatedMemory.push_back({memory, memRequirements.size});
    mtx.unlock();
    vkBindImageMemory(master->refDevice, image, memory, 0);
    return new TextureImage(this, std::pair<VkImage, VkImageView>(image, imageView), size);
}

void TextureMgr::releaseImage(const std::pair<VkImage, VkImageView> &image, const std::pair<short, short> &size)
{
    availableImages[size].push_back(image);
}

void TextureMgr::destroyImage(const VkImage &image, const VkImageView &imageView, const VkDeviceMemory &memory)
{
    vkDestroyImageView(master->refDevice, imageView, nullptr);
    vkDestroyImage(master->refDevice, image, nullptr);
    vkFreeMemory(master->refDevice, memory, nullptr);
}

TextureImage::TextureImage(TextureMgr *_master, const std::pair<VkImage, VkImageView> &_image, const std::pair<short, short> &_size, VkDeviceMemory _memory) : master(_master), image(_image.first), imageView(_image.second), size(_size), memory(_memory)
{
}

TextureImage::~TextureImage()
{
    if (memory == VK_NULL_HANDLE)
        master->releaseImage(std::pair<VkImage, VkImageView>(image, imageView), size);
    else
        master->destroyImage(image, imageView, memory);
}
