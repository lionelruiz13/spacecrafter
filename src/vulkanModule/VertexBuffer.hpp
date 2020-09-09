#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>

class VirtualSurface;

class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *_master, int size,
        const VkVertexInputBindingDescription &_bindingDesc,
        const std::vector<VkVertexInputAttributeDescription> &_attributeDesc,
        bool isExternallyUpdated = false);
    ~VertexBuffer();
    //! Update vertex content with data member
    VkBuffer &get() {return vertexBuffer;}
    void update();
    void update(VkCommandBuffer cmdBuffer);
    const VkVertexInputBindingDescription &getBindingDesc() {return bindingDesc;}
    const std::vector<VkVertexInputAttributeDescription> &getAttributeDesc() {return attributeDesc;}
    //! Intermediate buffer, write your vertex here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer vertexBuffer;
    VkDeviceSize bufferSize;
    VkSubmitInfo submitInfo{};
    VkVertexInputBindingDescription bindingDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
};

#endif /* end of include guard: VERTEX_BUFFER_HPP */
