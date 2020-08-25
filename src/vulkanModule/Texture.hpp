#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include <string>

class VirtualSurface;
class TextureImage;
class TextureMgr;

class Texture {
public:
    //! @param keepOnCPU If true, you can load/unload this texture from GPU
    Texture(VirtualSurface *_master, TextureMgr *_mgr, std::string filename = "", bool keepOnCPU = true);
    ~Texture();
    //! Export texture to GPU
    void use();
    //! Release texture on GPU
    void unuse();
private:
    VirtualSurface *master;
    TextureMgr *mgr;
    int texWidth, texHeight;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkSemaphore semaphore;
    VkFence fence;
    std::unique_ptr<TextureImage> image = nullptr;
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
