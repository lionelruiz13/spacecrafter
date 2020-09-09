#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <vulkan/vulkan.h>
#include <list>
#include <vector>
#include <array>
class Vulkan;

typedef struct SubMemory {
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    uint32_t memoryIndex;
} SubMemory;

class MemoryManager
{
public:

    MemoryManager(Vulkan *_master, uint32_t _streamBufferSize = 16*1024, uint32_t _chunkSize = 128*1024*1024);
    ~MemoryManager();
    //! @param isStatic hint to tell if
    SubMemory malloc(const VkMemoryRequirements &memRequirements, const VkMemoryPropertyFlags &properties, bool isDeviceLocal, bool isShortLived);
    void free(const SubMemory &memory);

private:
    Vulkan *master;
    typedef struct Memory {
        std::list<SubMemory> availableSpaces;
        std::vector<VkDeviceMemory> memoryChunks;
    } Memory;
    std::array<Memory, VK_MAX_MEMORY_TYPES> memory;
    const uint32_t chunkSize;
};

#endif /* end of include guard: MEMORY_MANAGER_HPP */
