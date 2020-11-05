#include "VirtualSurface.hpp"
#include "VertexBuffer.hpp"
#include "BufferMgr.hpp"
#include "tools/log.hpp"

VertexBuffer::VertexBuffer(VirtualSurface *_master, int size,
    const VkVertexInputBindingDescription &_bindingDesc,
    const std::vector<VkVertexInputAttributeDescription> &_attributeDesc,
    bool isExternallyUpdated, bool isStreamUpdate) : master(_master), bindingDesc(_bindingDesc)
{
    bufferSize = _bindingDesc.stride * size;
    attributeDesc.assign(_attributeDesc.begin(), _attributeDesc.end());
    if (isStreamUpdate) {
        subBuffer = master->acquireBuffer(bufferSize, false);
        data = master->getBufferPtr(subBuffer);
        offset = subBuffer.offset;
        return;
    }

    _master->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);
    _master->mapMemory(stagingBufferMemory, &data);
    _master->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    master->setObjectName(stagingBuffer, VK_OBJECT_TYPE_BUFFER, "staging vertexBuffer at V" + std::to_string(reinterpret_cast<long>(this)));
    master->setObjectName(vertexBuffer, VK_OBJECT_TYPE_BUFFER, "vertexBuffer at V" + std::to_string(reinterpret_cast<long>(this)));

    if (!isExternallyUpdated) {
        // Initialize update
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _master->getTransferPool();
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(_master->refDevice, &allocInfo, &updater);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;

        vkBeginCommandBuffer(updater, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(updater, stagingBuffer, vertexBuffer, 1, &copyRegion);

        vkEndCommandBuffer(updater);

        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &updater;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.waitSemaphoreCount = 0;
        master->setObjectName(updater, VK_OBJECT_TYPE_COMMAND_BUFFER, "update for VertexBuffer at V" + std::to_string(reinterpret_cast<long>(this)));
    }
}

VertexBuffer::~VertexBuffer()
{
    detach();
    if (vertexBuffer) {
        vkDeviceWaitIdle(master->refDevice);
        vkDestroyBuffer(master->refDevice, vertexBuffer, nullptr);
        master->free(vertexBufferMemory);
    } else {
        master->releaseBuffer(subBuffer);
    }
}

void VertexBuffer::print(std::ostringstream &oss)
{
    oss << "\t\t\tCustom name : " << customName << "\n";
    oss << "\t\t\tExternal update : " << ((submitInfo.commandBufferCount) ? "false" : "true") << "\n\t\t\tblocSize : " << bindingDesc.stride << "\n\t\t\tSize : " << bufferSize << "\n\t\t\tNbBlocs : " << bufferSize / bindingDesc.stride << "\n";
    oss << "\t\t\tIsDetached : " << (stagingBuffer ? "false\n" : "true\n");
    oss << "\t\t\tBufferHandle : " << reinterpret_cast<void *>(vertexBuffer) << "\n\t\t\tMemoryHandle : " << reinterpret_cast<void *>(vertexBufferMemory.memory) << "\n\t\t\tMemoryOffset : " << vertexBufferMemory.offset << " (" << reinterpret_cast<void *>(vertexBufferMemory.offset) << ")\n\t\t\tMemorySize : " << vertexBufferMemory.size << " (" << reinterpret_cast<void *>(vertexBufferMemory.size) << ")\n";
    if (stagingBuffer) {
        oss << "\t\t\tstagingBufferHandle : " << reinterpret_cast<void *>(stagingBuffer) << "\n\t\t\tstagingMemoryHandle : " << reinterpret_cast<void *>(stagingBufferMemory.memory) << "\n\t\t\tstagingMemoryOffset : " << stagingBufferMemory.offset << " (" << reinterpret_cast<void *>(stagingBufferMemory.offset) << ")\n\t\t\tstagingMemorySize : " << stagingBufferMemory.size << " (" << reinterpret_cast<void *>(stagingBufferMemory.size) << ")\n";
    }
}

void VertexBuffer::setName(const std::string &name)
{
    customName = name;
}

void VertexBuffer::update(int size)
{
    if (submitInfo.commandBufferCount > 0) {
        if (size != 0)
            master->submitTransfer(&submitInfo);
    } else {
        cLog::get()->write("Attempt to update detached VertexBuffer", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
    }
}

void VertexBuffer::update(VkCommandBuffer cmdBuffer)
{
    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);
}

void VertexBuffer::detach()
{
    if (stagingBuffer != VK_NULL_HANDLE) {
        master->unmapMemory(stagingBufferMemory);
        master->waitTransferQueueIdle();
        if (submitInfo.commandBufferCount > 0) {
            vkFreeCommandBuffers(master->refDevice, master->getTransferPool(), 1, &updater);
            submitInfo.commandBufferCount = 0;
        }
        vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
        master->free(stagingBufferMemory);
        data = nullptr;
        stagingBuffer = VK_NULL_HANDLE;
    }
}
