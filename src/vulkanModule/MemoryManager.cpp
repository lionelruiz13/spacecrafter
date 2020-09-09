#include "Vulkan.hpp"
#include "MemoryManager.hpp"
#include <mutex>

MemoryManager::MemoryManager(Vulkan *_master, uint32_t _streamBufferSize, uint32_t _chunkSize) : master(_master), chunkSize(_chunkSize)
{}

SubMemory MemoryManager::malloc(const VkMemoryRequirements &memRequirements, const VkMemoryPropertyFlags &properties, bool isDeviceLocal, bool isShortLived)
{
    SubMemory memAlloc;
    VkPhysicalDeviceMemoryProperties2 memProperties = master->getGPUMemoryInfo();
    return memAlloc;
}
