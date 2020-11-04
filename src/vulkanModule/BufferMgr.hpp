#ifndef BUFFER_MGR_HPP
#define BUFFER_MGR_HPP

#include <vulkan/vulkan.h>
#include "SubMemory.hpp"
#include "SubBuffer.hpp"
#include <vector>
#include <memory>
#include <list>
#include <thread>
#include <mutex>

class Vulkan;
class VirtualSurface;

#define UNIFIED_BUFFER_FLAGS (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)

/**
*   \brief Manage SubBuffer allocation like MemoryManager
*   In VirtualSurface, content of buffer allocated is synchronized when calling VirtualSurface::submitFrame
*   Else, synchronization occur when calling update()
*/
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
    void releaseBuffer(); // Release next buffer in stack
    static void startMainloop(BufferMgr *self);
    std::unique_ptr<std::thread> releaseThread;
    std::vector<SubBuffer> releaseStack;
    bool isAlive = false;
    std::mutex mutex, mutexQueue;

    void insert(SubBuffer &subBuffer);
    static int uniformOffsetAlignment;
    SubMemory bufferMemory;
    VkBuffer buffer;
    //! each std::list in this std::list are equally sized SubBuffer
    std::list<std::list<SubBuffer>> availableSubBufferZones;
    //std::list<SubBuffer> availableSubBuffer;
    int bufferBlocSize;
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
