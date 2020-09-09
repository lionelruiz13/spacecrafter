#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "TextureMgr.hpp"

class VirtualSurface;

class Texture {
public:
    //! @param keepOnCPU If true, you can load/unload this texture from GPU
    Texture(VirtualSurface *_master, TextureMgr *_mgr, std::string filename = "", bool keepOnCPU = true, bool multisampling = false);
    Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU = false, bool multisampling = false, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB, bool createSampler = true, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    ~Texture();
    //! Export texture to GPU
    void use();
    //! Release texture on GPU
    void unuse();
    //! Acquire pointer to staging memory
    void acquireStagingMemoryPtr(void **pPixels);
    //! Release pointer to staging memory
    void releaseStagingMemoryPtr();
    VkDescriptorImageInfo *getInfo() {return &imageInfo;}
    static void setTextureDir(std::string _textureDir) {textureDir = _textureDir;}
private:
    VkFormat format;
    //! Destroy staging resources
    void destroyStagingResources();
    void init(VirtualSurface *_master, TextureMgr *_mgr, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB);
    static std::string textureDir;
    VkDescriptorImageInfo imageInfo;
    VirtualSurface *master;
    TextureMgr *mgr;
    int texWidth, texHeight;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkSemaphore semaphore;
    VkFence fence;
    std::unique_ptr<TextureImage> image = nullptr;
    int useCount = 0;
};

//! For texture streaming like a video
class StreamTexture {
public:
    StreamTexture(VirtualSurface *_master, bool externUpdate = false);
    ~StreamTexture();
    void update(); // Update texture content, set externUpdate to true to use
    void use(int height, int width);
    void unuse();
    void *data;
};

#endif /* end of include guard: TEXTURE_HPP */
