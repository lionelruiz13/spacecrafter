#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "SubMemory.hpp"
#include <memory>

class VirtualSurface;
class TextureMgr;
class TextureImage;

class Texture {
public:
    //! @param _master The virtual surface using this texture for draw
    Texture(VirtualSurface *_master, TextureMgr *_mgr, bool isDepthAttachment = false, int width = -1, int height = -1);
    //! @param keepOnCPU If true, you can load/unload this texture from GPU
    Texture(VirtualSurface *_master, TextureMgr *_mgr, std::string filename = "", bool keepOnCPU = true, bool multisampling = false);
    Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU = false, bool multisampling = false, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB, bool createSampler = true, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    Texture();
    virtual ~Texture();
    //! Export texture to GPU
    void use();
    //! Release texture on GPU
    virtual void unuse();
    //! Acquire pointer to staging memory
    void acquireStagingMemoryPtr(void **pPixels);
    //! Release pointer to staging memory
    void releaseStagingMemoryPtr();
    VkDescriptorImageInfo *getInfo() {return &imageInfo;}
    static void setTextureDir(std::string _textureDir) {textureDir = _textureDir;}
    bool isValid() {return isOk;}
protected:
    static std::string textureDir;
    VirtualSurface *master;
    TextureMgr *mgr;
    bool isOk;
    VkDescriptorImageInfo imageInfo;
    int texWidth, texHeight;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SubMemory stagingBufferMemory;
    VkFence fence;
    std::unique_ptr<TextureImage> image;
    int useCount = 0;
private:
    void init(VirtualSurface *_master, TextureMgr *_mgr, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB);
    //! Destroy staging resources
    void destroyStagingResources();
    VkSemaphore semaphore;
    VkFormat format;
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
    VkCommandBuffer cmdBuffer;
    VkSubmitInfo updateSubmitInfo;
};

#endif /* end of include guard: TEXTURE_HPP */
