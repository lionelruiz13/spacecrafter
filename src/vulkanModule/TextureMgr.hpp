#ifndef TEXTURE_MGR_HPP
#define TEXTURE_MGR_HPP

#include <vulkan/vulkan.h>
#include <map>
#include "SubMemory.hpp"

class Vulkan;
class TextureImage;

class TextureMgr {
public:
    TextureMgr(Vulkan *_master, uint32_t _chunkSize = 256*1024*1024);
    ~TextureMgr();
    TextureImage *createImage(const std::pair<short, short> &size, bool mipmap, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, bool isDepthAttachment = false);
    void releaseImage(const std::pair<VkImage, VkImageView> &image, const std::pair<short, short> &size);
    void destroyImage(const VkImage &image, const VkImageView &imageView, SubMemory &memory);
    VkSampler createSampler(const VkSamplerCreateInfo &samplerInfo);
private:
    //! Return unique value for this combination of parameters
    int getSamplerID(const VkSamplerCreateInfo &samplerInfo);
    Vulkan *master;
    VkSampler defaultSampler = VK_NULL_HANDLE;
    const uint32_t chunkSize;
    // pair of memory/lowest_offset_available
    std::vector<std::pair<SubMemory, int>> allocatedMemory;
    // pair : {width, height}
    std::map<std::pair<short, short>, std::vector<std::pair<VkImage, VkImageView>>> availableImages;
    // samplers
    std::map<int, VkSampler> samplers;
};

class TextureImage {
public:
    TextureImage(TextureMgr *_master, const std::pair<VkImage, VkImageView> &_image, const std::pair<short, short> &_size, SubMemory *_memory = nullptr);
    ~TextureImage();
    VkImage &getImage() {return image;}
    VkImageView &getImageView() {return imageView;}
private:
    TextureMgr *master;
    VkImage image;
    VkImageView imageView;
    std::pair<short, short> size;
    SubMemory memory;
};

#endif /* end of include guard: TEXTURE_MGR_HPP */
