#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"

class VirtualSurface;

/**
*   Accessible memory from CPU synchronized with accessible memory from GPU
*/
class Buffer {
public:
    Buffer(VirtualSurface *_master, int size, VkBufferUsageFlags usage);
    ~Buffer();
    VkBuffer &get() {return buffer ? buffer : subBuffer.buffer;}
    int getOffset() {return offset;}
    //! Display informations about this buffer
    void print(std::ostringstream &oss);
    //! Update vertex content with data member
    void update();
    //! Intermediate buffer, write here (or read)
    void *data;
    //! Release staging resources
    void detach();
    //! Flush intermediate buffer to make storage buffer update visible
    void invalidate();
    //! Set name to attach to buffer
    void setName(const std::string &name);
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    SubMemory stagingBufferMemory;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SubMemory bufferMemory;
    VkBuffer buffer = VK_NULL_HANDLE;
    SubBuffer subBuffer;
    int offset = 0;
    VkSubmitInfo submitInfo{};
};

#endif /* end of include guard: BUFFER_HPP */
