#include "Vulkan.hpp"
#include "VirtualSurface.hpp"
#include "BufferMgr.hpp"
#include "tools/log.hpp"

int BufferMgr::uniformOffsetAlignment;

BufferMgr::BufferMgr(VirtualSurface *_master, int _bufferBlocSize) : master(_master)
{
    bufferBlocSize = _bufferBlocSize;
    if (!master->createBuffer(bufferBlocSize, UNIFIED_BUFFER_FLAGS | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory)) {
        throw std::runtime_error("Failed to create buffer bloc");
    }
    master->setObjectName(buffer, VK_OBJECT_TYPE_BUFFER, "BufferMgr");
    SubBuffer subBuffer;
    subBuffer.buffer = buffer;
    subBuffer.offset = 0;
    subBuffer.size = bufferBlocSize;
    insert(subBuffer);

    if (!master->createBuffer(bufferBlocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingMemory)) {
        throw std::runtime_error("Failed to create staging buffer bloc");
    }
    master->setObjectName(stagingBuffer, VK_OBJECT_TYPE_BUFFER, "staging BufferMgr");
    master->mapMemory(stagingMemory, &data);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = master->getTransferPool();
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &cmdBuffer);
    master->setObjectName(cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "update for BufferMgr");
}

BufferMgr::~BufferMgr()
{
    if (isAlive) {
        isAlive = false;
        releaseThread->join();
    }
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    master->free(bufferMemory);
    vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
    master->free(stagingMemory);
}

SubBuffer BufferMgr::acquireBuffer(int size, bool isUniform)
{
    std::lock_guard<std::mutex> lock(mutex);
    SubBuffer buffer;
    for (auto availableSubBuffer = availableSubBufferZones.begin(); availableSubBuffer != availableSubBufferZones.end(); ++availableSubBuffer) {
        if (availableSubBuffer->front().size < size)
            continue;
        if (!isUniform) {
            buffer = availableSubBuffer->back();
            availableSubBuffer->pop_back();
            if (buffer.size > size) {
                SubBuffer tmp = buffer;
                tmp.size -= size;
                tmp.offset += size;
                buffer.size = size;
                insert(tmp);
            }
            if (buffer.offset + buffer.size > maxOffset) maxOffset = buffer.offset + buffer.size;
            if (availableSubBuffer->size() == 0) availableSubBufferZones.erase(availableSubBuffer);
            return buffer;
        } // else
        const auto itEnd = availableSubBuffer->end();
        for (auto it = availableSubBuffer->begin(); it != itEnd; ++it) {
            if (!it->possibleUniform)
                break;
            if (it->offset % uniformOffsetAlignment > 0 && it->size + it->offset % uniformOffsetAlignment - uniformOffsetAlignment < size)
                continue;
            buffer = *it;
            availableSubBuffer->erase(it);
            if (buffer.offset % uniformOffsetAlignment > 0) {
                SubBuffer tmp = buffer;
                tmp.size = uniformOffsetAlignment - buffer.offset % uniformOffsetAlignment;
                buffer.offset += tmp.size;
                buffer.size -= tmp.size;
                insert(tmp);
            }
            if (buffer.size > size) {
                SubBuffer tmp = buffer;
                tmp.size -= size;
                tmp.offset += size;
                buffer.size = size;
                insert(tmp);
            }
            if (buffer.offset + buffer.size > maxOffset) maxOffset = buffer.offset + buffer.size;
            if (availableSubBuffer->size() == 0) availableSubBufferZones.erase(availableSubBuffer);
            return buffer;
        }
    }
    cLog::get()->write("Can't allocate buffer in global buffer !", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
    buffer.buffer = VK_NULL_HANDLE;
    return buffer;
}

void BufferMgr::insert(SubBuffer &subBuffer)
{
    subBuffer.possibleUniform = (subBuffer.offset % uniformOffsetAlignment == 0) || (subBuffer.size > uniformOffsetAlignment - subBuffer.offset % uniformOffsetAlignment);
    const auto itEnd = availableSubBufferZones.end();
    for (auto it = availableSubBufferZones.begin(); it != itEnd; ++it) {
        if (it->front().size == subBuffer.size) {
            if (subBuffer.possibleUniform)
                it->push_front(subBuffer);
            else
                it->push_back(subBuffer);
            if (it->size() == 500 && !isAlive) {
                isAlive = true;
                releaseThread = std::make_unique<std::thread>(startMainloop, this);
            }
            return;
        } else if (it->front().size > subBuffer.size) {
            std::list<SubBuffer> tmpList;
            tmpList.push_back(subBuffer);
            availableSubBufferZones.insert(it, tmpList);
            return;
        }
    }
    std::list<SubBuffer> tmpList;
    tmpList.push_back(subBuffer);
    availableSubBufferZones.push_back(tmpList);
}

void BufferMgr::releaseBuffer(SubBuffer &subBuffer)
{
    mutexQueue.lock();
    releaseStack.push_back(subBuffer);
    mutexQueue.unlock();
    if (!isAlive)
        releaseBuffer();
}

void BufferMgr::releaseBuffer()
{
    mutexQueue.lock();
    SubBuffer subBuffer = releaseStack.back();
    releaseStack.pop_back();
    mutexQueue.unlock();
    int buffBegin = subBuffer.offset;
    int buffEnd = buffBegin + subBuffer.size;

    mutex.lock();
    for (auto availableSubBuffer = availableSubBufferZones.begin(); availableSubBuffer != availableSubBufferZones.end(); ++availableSubBuffer) {
        const auto itEnd = availableSubBuffer->end();
        for (auto it = availableSubBuffer->begin(); it != itEnd; ++it) {
            if (it->offset == buffEnd) {
                subBuffer.size += it->size;
            } else if (it->offset + it->size == buffBegin) {
                subBuffer.offset = it->offset;
                subBuffer.size += it->size;
            } else {
                continue;
            }
            if (it == availableSubBuffer->begin()) {
                availableSubBuffer->erase(it);
                it = availableSubBuffer->begin();
            } else {
                auto tmpIt = it;
                --it;
                availableSubBuffer->erase(tmpIt);
            }
        }
        if (availableSubBuffer->size() == 0) {
            if (availableSubBuffer == availableSubBufferZones.begin()) {
                availableSubBufferZones.erase(availableSubBuffer);
                availableSubBuffer = availableSubBufferZones.begin();
            } else {
                auto tmpIt = availableSubBuffer;
                --availableSubBuffer;
                availableSubBufferZones.erase(tmpIt);
            }
        }
    }
    if (subBuffer.offset + subBuffer.size >= maxOffset && subBuffer.offset < maxOffset) {
        maxOffset = subBuffer.offset;
    }
    insert(subBuffer);
    mutex.unlock();
}

void *BufferMgr::getPtr(SubBuffer &subBuffer)
{
    return static_cast<char *>(data) + subBuffer.offset; // static_cast for GCC
}

void BufferMgr::update()
{
    if (maxOffset != lastMaxOffset) {
        if (maxOffset == 0) return;
        lastMaxOffset = maxOffset;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        VkBufferCopy copyRegion{0, 0, (size_t) lastMaxOffset};
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer, buffer, 1, &copyRegion);
        vkEndCommandBuffer(cmdBuffer);
    }
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;
    master->submitTransfer(&submitInfo);
}

void BufferMgr::startMainloop(BufferMgr *self)
{
    while (true) {
        while (self->isAlive && self->releaseStack.empty())
            std::this_thread::yield();
        if (!self->isAlive)
            return;
        self->releaseBuffer();
    }
}
