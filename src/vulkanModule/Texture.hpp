#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "SubMemory.hpp"
#include <memory>
#include <vector>

class VirtualSurface;
class TextureMgr;
class TextureImage;

/**
*   \brief Manage texture, including mipmapping, writing, reading and sample
*/
class Texture {
public:
    /**
    * @param _master The virtual surface using this texture for draw
    * @param keepOnCPU If true, you can load/unload this texture from GPU
    */
    Texture(VirtualSurface *_master, TextureMgr *_mgr, bool isDepthAttachment = false, int width = -1, int height = -1);
    /**
    * @param keepOnCPU If true, you can load/unload this texture from GPU
    * @param is3d If true, read as a power-of-two cube texture (width=height=depth) ordered in columns and lines
    * @param useCustomMipmapComputation If true, compute each mipmap and apply value *= (2 - value) to each pixel's of the mipmap
    */
    Texture(VirtualSurface *_master, TextureMgr *_mgr, const std::string &filename, bool keepOnCPU = true, bool mipmap = false, bool createSampler=false, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM, int nbChannels=4, bool is3d = false, bool useCustomMipmapComputation = false);
    Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU = false, bool mipmap = false, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM, const std::string &name = "unnamed", bool createSampler = true, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    //! Create 3D texture with mipmapping and sampler
    Texture(VirtualSurface *_master, TextureMgr *_mgr, const std::string &filename, int width, int height, const std::string &name = "unnamed 3D", VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM, int nbChannels=4);
    Texture();
    virtual ~Texture();
    //! Export texture to GPU and return true on success. If forceUpdate is false, changes made in staging memory may not appear
    bool use(bool forceUpdate = false);
    //! Release texture on GPU (may invalidate all previous bindings)
    virtual void unuse();
    //! Acquire pointer to staging memory
    void acquireStagingMemoryPtr(void **pPixels);
    //! Release pointer to staging memory
    void releaseStagingMemoryPtr();
    //! Internal use only
    VkDescriptorImageInfo *getInfo() {return &imageInfo;}
    //! Define texture path
    static void setTextureDir(std::string _textureDir) {textureDir = _textureDir;}
    //! Inform if texture support is valid or not
    bool isValid() const {return isOk;}
    //! Internal use only
    VkImage getImage();
    //! Write texture size in width and height arguments
    void getDimensions(int &width, int &height) const {width=texWidth;height=texHeight;}
    //! Write texture size in width, height and depth arguments
    void getDimensions(int &width, int &height, int &depth) const {width=texWidth;height=texHeight;depth=texDepth;}
    int getMipmapCount() const {return mipmapCount;}
    //! Return number of use() calls minus unuse() calls count
    int getUseCount() const {return useCount;}
protected:
    static std::string textureDir;
    VirtualSurface *master;
    TextureMgr *mgr;
    bool isOk = true;
    VkDescriptorImageInfo imageInfo;
    int texWidth=0, texHeight=0, texDepth=1;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SubMemory stagingBufferMemory;
    std::unique_ptr<TextureImage> image;
    TextureImage *imagePtr = nullptr;
    int useCount = 0;
private:
    void init(VirtualSurface *_master, TextureMgr *_mgr, bool _mipmap, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM);
    //! Destroy staging resources
    void destroyStagingResources();
    void computeMipmap(VkCommandBufferAllocateInfo &allocInfo, VkSubmitInfo &submitInfo, VkFence &fence);
    void computeCustomMipmap(VkCommandBufferAllocateInfo &allocInfo, VkSubmitInfo &submitInfo, VkFence &fence);
    VkFormat format;
    bool mipmap = false;
    bool customMipmap = false;
    int mipmapCount = 1;
    std::string imageName;
    VkCommandBuffer commandBuffer;
    std::vector<VkBufferImageCopy> regions;
};

//! For texture streaming like a video
class StreamTexture : public Texture {
public:
    StreamTexture(VirtualSurface *_master, TextureMgr *_mgr, bool externUpdate = false);
    virtual ~StreamTexture();
    void update(); // Update texture content, set externUpdate to true to use
    bool use(int width, int height, VkFormat format = VK_FORMAT_R8_UNORM);
    virtual void unuse() override;
private:
    VkFence fence;
    VkCommandBuffer cmdBuffer;
    VkSubmitInfo updateSubmitInfo{};
};

#endif /* end of include guard: TEXTURE_HPP */
