#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
namespace v {

class VirtualSurface;

class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *_master, int size,
        const VkVertexInputBindingDescription &_bindingDesc,
        const std::vector<VkVertexInputAttributeDescription> &_attributeDesc);
    ~VertexBuffer();
    //! Update vertex content with data member
    VkBuffer &get() {return vertexBuffer;}
    void update();
    const VkVertexInputBindingDescription &getBindingDesc() {return bindingDesc;};
    const std::vector<VkVertexInputAttributeDescription> &getAttributeDesc() {return attributeDesc;};
    //! Intermediate buffer, write your vertex here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer vertexBuffer;
    VkSubmitInfo submitInfo{};
    VkVertexInputBindingDescription bindingDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
};
}

#ifndef OPENGL_HPP
using namespace v;
#endif

#endif /* end of include guard: VERTEX_BUFFER_HPP */
