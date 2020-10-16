#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "SubMemory.hpp"

class VirtualSurface;

class Buffer {
public:
    Buffer(VirtualSurface *_master, int size, VkBufferUsageFlags usage);
    ~Buffer();
    VkBuffer &get() {return buffer;}
    int getOffset() {return 0;}
    //! Display informations about this buffer
    void print();
    //! Update vertex content with data member
    void update();
    //! Intermediate buffer, write your vertex here
    void *data;
    //! Release staging resources
    void detach();
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    SubMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    SubMemory bufferMemory;
    VkBuffer buffer;
    VkSubmitInfo submitInfo{};
};

#endif /* end of include guard: BUFFER_HPP */
