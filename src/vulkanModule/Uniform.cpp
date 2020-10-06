#include "VirtualSurface.hpp"
#include "Uniform.hpp"
#include "BufferMgr.hpp"

Uniform::Uniform(VirtualSurface *_master, VkDeviceSize size, bool isVirtual) : master(_master)
{
    buffer = master->acquireBuffer(size, true);
    data = master->getBufferPtr(buffer);
    bufferInfo.buffer = buffer.buffer;
    bufferInfo.offset = buffer.offset;
    bufferInfo.range = size;
    if (isVirtual) {
        offset = bufferInfo.offset;
        bufferInfo.offset = 0;
    }
}

Uniform::~Uniform()
{
    master->releaseBuffer(buffer);
}

void Uniform::update()
{}
