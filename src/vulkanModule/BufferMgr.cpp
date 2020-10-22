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
    SubBuffer subBuffer;
    subBuffer.buffer = buffer;
    subBuffer.offset = 0;
    subBuffer.size = bufferBlocSize;
    availableSubBuffer.push_back(subBuffer);

    if (!master->createBuffer(bufferBlocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingMemory)) {
        throw std::runtime_error("Failed to create staging buffer bloc");
    }
    master->mapMemory(stagingMemory, &data);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = master->getTransferPool();
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &cmdBuffer);
}

BufferMgr::~BufferMgr()
{
    vkDestroyBuffer(master->refDevice, buffer, nullptr);
    master->free(bufferMemory);
    vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
    master->free(stagingMemory);
}

SubBuffer BufferMgr::acquireBuffer(int size, bool isUniform)
{
    // PATCH
    if (size % uniformOffsetAlignment)
        size = (size / uniformOffsetAlignment + 1) * uniformOffsetAlignment;
    // END PATCH
    SubBuffer buffer;
    const auto itEnd = availableSubBuffer.end();
    for (auto it = availableSubBuffer.begin(); it != itEnd; ++it) {
        if (it->size < size)
            continue;
        buffer = *it;
        availableSubBuffer.erase(it);
        if (buffer.size > size) {
            SubBuffer tmp = buffer;
            tmp.size -= size;
            tmp.offset += size;
            buffer.size = size;
            insert(tmp);
        }
        if (buffer.offset + buffer.size > maxOffset) maxOffset = buffer.offset + buffer.size;
        return buffer;
    }
    cLog::get()->write("Can't allocate buffer in global buffer !", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
    buffer.buffer = VK_NULL_HANDLE;
    return buffer;
}

void BufferMgr::insert(SubBuffer &subBuffer)
{
    const auto itEnd = availableSubBuffer.end();
    for (auto it = availableSubBuffer.begin(); it != itEnd; ++it) {
        if (it->size >= subBuffer.size) {
            availableSubBuffer.insert(it, subBuffer);
            return;
        }
    }
    availableSubBuffer.push_back(subBuffer);
}

void BufferMgr::releaseBuffer(SubBuffer &subBuffer)
{
    int buffBegin = subBuffer.offset;
    int buffEnd = buffBegin + subBuffer.size;

    const auto itEnd = availableSubBuffer.end();
    for (auto it = availableSubBuffer.begin(); it != itEnd; ++it) {
        if (it->offset == buffEnd) {
            subBuffer.size += it->size;
        } else if (it->offset + it->size == buffBegin) {
            subBuffer.offset = it->offset;
            subBuffer.size += it->size;
        } else {
            continue;
        }
        if (it == availableSubBuffer.begin()) {
            availableSubBuffer.erase(it);
            it = availableSubBuffer.begin();
        } else {
            auto tmpIt = it;
            --it;
            availableSubBuffer.erase(tmpIt);
        }
    }
    if (subBuffer.offset + subBuffer.size >= maxOffset && subBuffer.offset < maxOffset) {
        maxOffset = subBuffer.offset;
    }
    insert(subBuffer);
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
        VkBufferCopy copyRegion{};
        copyRegion.size = maxOffset;
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
