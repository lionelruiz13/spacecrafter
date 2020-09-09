#ifndef TEXTURE_MGR_HPP
#define TEXTURE_MGR_HPP

#include <vulkan/vulkan.h>
#include <map>

class Vulkan;
class TextureImage;

class TextureMgr {
public:
    TextureMgr(Vulkan *_master, uint32_t _chunkSize = 64*1024*1024);
    ~TextureMgr();
    TextureImage *createImage(const std::pair<short, short> &size, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    void releaseImage(const std::pair<VkImage, VkImageView> &image, const std::pair<short, short> &size);
    void destroyImage(const VkImage &image, const VkImageView &imageView, const VkDeviceMemory &memory);
    //! Return unique value for this combination of parameters
    int getSamplerID(const VkSamplerCreateInfo &samplerInfo);
    VkSampler createSampler(const VkSamplerCreateInfo &samplerInfo);
private:
    Vulkan *master;
    const uint32_t chunkSize;
    // pair of memory/lowest_offset_available
    std::vector<std::pair<VkDeviceMemory, int>> allocatedMemory;
    // pair : {width, height}
    std::map<std::pair<short, short>, std::vector<std::pair<VkImage, VkImageView>>> availableImages;
    // samplers
    std::map<int, VkSampler> samplers;
};

class TextureImage {
public:
    TextureImage(TextureMgr *_master, const std::pair<VkImage, VkImageView> &_image, const std::pair<short, short> &_size, VkDeviceMemory _memory = VK_NULL_HANDLE);
    ~TextureImage();
    VkImage &getImage() {return image;}
    VkImageView &getImageView() {return imageView;}
private:
    TextureMgr *master;
    VkImage image;
    VkImageView imageView;
    std::pair<short, short> size;
    VkDeviceMemory memory;
};

#endif /* end of include guard: TEXTURE_MGR_HPP */
