#ifndef BUFFER_MGR_HPP
#define BUFFER_MGR_HPP

#include <vulkan/vulkan.h>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"
#include <list>

class Vulkan;
class VirtualSurface;

#define UNIFIED_BUFFER_FLAGS VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT

//! Manage virtual buffering for Uniform
class BufferMgr {
public:
    BufferMgr(VirtualSurface *_master, int _bufferBlocSize = 512*1024);
    ~BufferMgr();
    SubBuffer acquireBuffer(int size, bool isUniform = false);
    void releaseBuffer(SubBuffer &subBuffer);
    void *getPtr(SubBuffer &subBuffer);
    void update();
    static void setUniformOffsetAlignment(int alignment) {uniformOffsetAlignment = alignment;}
private:
    static int uniformOffsetAlignment;
    void insert(SubBuffer &subBuffer);
    SubMemory bufferMemory;
    VkBuffer buffer;
    std::list<SubBuffer> availableSubBuffer;
    int bufferBlocSize;
    std::list<SubBuffer> zonesToUpdate;
    VkCommandBuffer cmdBuffer;
    VkSubmitInfo submitInfo;
    int lastMaxOffset = -1;
    int maxOffset = 0;
    VirtualSurface *master;
    SubMemory stagingMemory;
    VkBuffer stagingBuffer;
    void *data;
};

#endif /* end of include guard: BUFFER_MGR_HPP */
