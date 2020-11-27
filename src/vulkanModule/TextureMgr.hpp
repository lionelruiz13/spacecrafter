#ifndef TEXTURE_MGR_HPP
#define TEXTURE_MGR_HPP

#include <vulkan/vulkan.h>
#include <map>
#include <mutex>
#include "SubMemory.hpp"

class Vulkan;
class TextureImage;
class CommandMgr;
class PipelineLayout;
class ComputePipeline;
class VirtualSurface;

class TextureMgr {
public:
    TextureMgr(Vulkan *_master, uint32_t _chunkSize = 256*1024*1024);
    ~TextureMgr();
    void initCustomMipmap(VirtualSurface *surface);
    TextureImage *createImage(const std::pair<short, short> &size, uint32_t depth=1, bool mipmap = false, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, bool isDepthAttachment = false, bool useConcurrency = false);
    void releaseImage(const std::pair<VkImage, VkImageView> &image, const std::pair<short, short> &size);
    void destroyImage(const VkImage &image, const VkImageView &imageView, SubMemory &memory);
    VkSampler createSampler(const VkSamplerCreateInfo &samplerInfo);
    CommandMgr *getBuilder() {return cmdMgrSingleUse.get();}
    void acquireSyncObject(VkFence *fence, VkSemaphore *semaphore);
    void releaseSyncObject(VkFence fence, VkSemaphore semaphore);
    //! Return corresponding cached image if there is no releaseCachedTextures call in between, otherwise return nullptr
    std::unique_ptr<TextureImage> queryImage(TextureImage *ptr);
    //! Move image in cache. Cached images are released when calling releaseCachedTextures
    void cacheImage(std::unique_ptr<TextureImage> &textureImage);
    //! Release all cached textures
    void releaseCachedTextures();
    std::vector<VkImageView> createViewArray(TextureImage *texImg, VkFormat format, bool is3d = true);
    PipelineLayout *getCustomMipmapPipelineLayout() {return customMipmapLayout.get();}
    void initCustomMipmap(CommandMgr *cmdMgr);
    void initCustomMipmapMini(CommandMgr *cmdMgr);
private:
    std::mutex syncObjectAccess;
    std::vector<std::pair<VkFence, VkSemaphore>> syncObject;
    //! Return unique value for this combination of parameters
    int getSamplerID(const VkSamplerCreateInfo &samplerInfo);
    Vulkan *master;
    std::unique_ptr<CommandMgr> cmdMgrSingleUse;
    VkSampler defaultSampler = VK_NULL_HANDLE;
    const uint32_t chunkSize;
    std::unique_ptr<PipelineLayout> customMipmapLayout;
    std::unique_ptr<ComputePipeline> customMipmap;
    std::unique_ptr<ComputePipeline> customMipmapMini;
    std::vector<std::unique_ptr<TextureImage>> cache;
    // pair of memory/lowest_offset_available
    std::vector<std::pair<SubMemory, int>> allocatedMemory;
    // pair : {width, height}
    std::map<std::pair<short, short>, std::vector<std::pair<VkImage, VkImageView>>> availableImages;
    // samplers
    std::map<int, VkSampler> samplers;
};

class TextureImage {
public:
    TextureImage(TextureMgr *_master, const std::pair<VkImage, VkImageView> &_image, const std::pair<short, short> &_size, int mipmap, SubMemory *_memory = nullptr);
    ~TextureImage();
    VkImage &getImage() {return image;}
    VkImageView &getImageView() {return imageView;}
    int getMipmapCount() {return mipmap;};
private:
    TextureMgr *master;
    VkImage image;
    VkImageView imageView;
    std::pair<short, short> size;
    int mipmap;
    SubMemory memory;
};

#endif /* end of include guard: TEXTURE_MGR_HPP */
