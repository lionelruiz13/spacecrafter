#ifndef UNIFORM_HPP
#define UNIFORM_HPP

#include <vulkan/vulkan.h>
#include "SubBuffer.hpp"

class VirtualSurface;

class Uniform {
public:
    //! @param isVirtual if true, this uniform can and must be bound with bindVirtualUniform
    Uniform(VirtualSurface *_master, VkDeviceSize size, bool isVirtual = false);
    ~Uniform();

    //VkShaderStageFlags getStage() {return stages;}
    void *data; // Use it like a pointer to writable allocated GPU memory
    VkDescriptorBufferInfo *getBufferInfo() {return &bufferInfo;}
    void update();
    // used if isVirtual is true
    int getOffset() {return offset;}
private:
    VirtualSurface *master;
    SubBuffer buffer;
    VkDescriptorBufferInfo bufferInfo{};
    int offset; // used if isVirtual is true
};

#endif /* end of include guard: UNIFORM_HPP */
