#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "SubMemory.hpp"
#include <memory>

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
    //! @param keepOnCPU If true, you can load/unload this texture from GPU
    Texture(VirtualSurface *_master, TextureMgr *_mgr, std::string filename = "", bool keepOnCPU = true, bool mipmap = false);
    Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU = false, bool mipmap = false, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM, bool createSampler = true, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    Texture();
    virtual ~Texture();
    //! Export texture to GPU
    void use();
    //! Release texture on GPU (invalidate all previous bindings)
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
    bool isValid() {return isOk;}
    //! Internal use only
    VkImage getImage();
    //! Write texture size in width and height arguments
    void getDimensions(int &width, int &height) {width=texWidth;height=texHeight;}
    //! Return number of use() calls minus unuse() calls count
    int getUseCount() {return useCount;}
protected:
    static std::string textureDir;
    VirtualSurface *master;
    TextureMgr *mgr;
    bool isOk;
    VkDescriptorImageInfo imageInfo;
    int texWidth=0, texHeight=0;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SubMemory stagingBufferMemory;
    std::unique_ptr<TextureImage> image;
    int useCount = 0;
private:
    void init(VirtualSurface *_master, TextureMgr *_mgr, bool _mipmap, VkFormat _format = VK_FORMAT_R8G8B8A8_UNORM);
    //! Destroy staging resources
    void destroyStagingResources();
    VkFormat format;
    bool mipmap = false;
    int mipmapCount = 1;
};

//! For texture streaming like a video
class StreamTexture : public Texture {
public:
    StreamTexture(VirtualSurface *_master, TextureMgr *_mgr, bool externUpdate = false);
    virtual ~StreamTexture();
    void update(); // Update texture content, set externUpdate to true to use
    void use(int width, int height);
    virtual void unuse() override;
private:
    VkFence fence;
    VkCommandBuffer cmdBuffer;
    VkSubmitInfo updateSubmitInfo;
};

#endif /* end of include guard: TEXTURE_HPP */
