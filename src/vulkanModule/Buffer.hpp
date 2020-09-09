#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>

class VirtualSurface;

class Buffer {
public:
    Buffer(VirtualSurface *_master, int size, VkBufferUsageFlags usage);
    ~Buffer();
    VkBuffer &get() {return buffer;}
    //! Update vertex content with data member
    void update();
    //! Intermediate buffer, write your vertex here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory bufferMemory;
    VkBuffer buffer;
    VkSubmitInfo submitInfo{};
};

#endif /* end of include guard: BUFFER_HPP */
