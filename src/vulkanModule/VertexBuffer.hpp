#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "SubMemory.hpp"

class VirtualSurface;

class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *_master, int size,
        const VkVertexInputBindingDescription &_bindingDesc,
        const std::vector<VkVertexInputAttributeDescription> &_attributeDesc,
        bool isExternallyUpdated = false);
    ~VertexBuffer();
    VkBuffer &get() {return vertexBuffer;}
    //! Update vertex content with data member
    //! @param size hint on number of bytes to update
    void update(int size = -1);
    void update(VkCommandBuffer cmdBuffer);
    const VkVertexInputBindingDescription &getBindingDesc() {return bindingDesc;}
    const std::vector<VkVertexInputAttributeDescription> &getAttributeDesc() {return attributeDesc;}
    //! Intermediate buffer, write your vertex here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    SubMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    SubMemory vertexBufferMemory;
    VkBuffer vertexBuffer;
    VkDeviceSize bufferSize;
    VkSubmitInfo submitInfo{};
    VkVertexInputBindingDescription bindingDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
};

#endif /* end of include guard: VERTEX_BUFFER_HPP */
