#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"

class VirtualSurface;

class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *_master, int size,
        const VkVertexInputBindingDescription &_bindingDesc,
        const std::vector<VkVertexInputAttributeDescription> &_attributeDesc,
        bool isExternallyUpdated = false, bool isStreamUpdate = false);
    ~VertexBuffer();
    VkBuffer &get() {return vertexBuffer ? vertexBuffer : subBuffer.buffer;}
    int getOffset() {return offset;}
    //! Display informations about this VertexBuffer
    void print(std::ostringstream &oss);
    //! Update vertex content with data member
    //! @param size hint on number of bytes to update
    void update(int size = -1);
    void update(VkCommandBuffer cmdBuffer);
    const VkVertexInputBindingDescription &getBindingDesc() {return bindingDesc;}
    const std::vector<VkVertexInputAttributeDescription> &getAttributeDesc() {return attributeDesc;}
    //! Intermediate buffer, write your vertex here
    void *data;
    //! Destroy staging resources and only keep GPU-side buffer
    void detach();
    //! Set custom name visible on debug layers
    void setName(const std::string &name);
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    SubMemory stagingBufferMemory;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SubMemory vertexBufferMemory;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceSize bufferSize;
    int offset = 0;
    SubBuffer subBuffer;
    VkSubmitInfo submitInfo{};
    std::string customName;
    VkVertexInputBindingDescription bindingDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
};

#endif /* end of include guard: VERTEX_BUFFER_HPP */
