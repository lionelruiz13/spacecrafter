#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <memory>
#include <vector>
#include <mutex>
#include "EntityCore/Forward.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
class HipStarMgr;
class DrawHelper;
class QueueFamily;

enum {
    PASS_BACKGROUND = 0, // multi-sample, no depth buffer
    // PASS_STAR_FBO, // star framebuffer, no depth buffer
    PASS_MULTISAMPLE_DEPTH, // multi-sample, depth buffer
    PASS_MULTISAMPLE_FRONT = PASS_MULTISAMPLE_DEPTH, // multi-sample, no depth buffer
    PASS_FOREGROUND, // single sample, no depth buffer
};

class Context {
public:
    Context();
    ~Context();

    static Context *instance;
    std::unique_ptr<DrawHelper> helper;
    std::unique_ptr<BufferMgr> stagingMgr;
    std::unique_ptr<BufferMgr> texStagingMgr;
    std::vector<std::unique_ptr<TransferMgr>> transfers;
    TransferMgr *transfer; // Transfer selected now
    std::unique_ptr<BufferMgr> readbackMgr;
    std::unique_ptr<BufferMgr> globalBuffer; // For vertex (and instance) buffer without specific alignment
    std::unique_ptr<BufferMgr> uniformMgr;
    std::unique_ptr<BufferMgr> tinyMgr; // For very small allocations (vertex, instance and indirect), device local
    std::unique_ptr<BufferMgr> ojmBufferMgr;
    std::unique_ptr<VertexArray> ojmVertexArray;
    std::unique_ptr<BufferMgr> indexBufferMgr;
    // multiVertex -> s_font and hints
    std::unique_ptr<BufferMgr> multiVertexMgr;
    std::unique_ptr<VertexArray> multiVertexArray;
    std::unique_ptr<SetMgr> setMgr;
    std::unique_ptr<Texture> starColorAttachment;
    std::vector<HipStarMgr *> starUsed; // nullptr if not used at this frame, otherwise a pointer to a HipStarMgr which operate a draw
    //std::vector<std::unique_ptr<SyncEvent>> starSync; // synchronize access to starColorAttachment
    std::unique_ptr<SyncEvent> transferSync; // synchronize transfers
    std::unique_ptr<RenderMgr> render;
    std::vector<std::unique_ptr<FrameMgr>> frame;
    std::unique_ptr<RenderMgr> renderAlone; // Single-pass rendering without depth buffer
    std::vector<std::unique_ptr<FrameMgr>> frameAlone;
    std::vector<VkFence> fences;
    std::vector<VkFence> debugFences;
    std::vector<VkSemaphore> semaphores;
    std::vector<VkCommandBuffer> graphicTransferCmd;
    std::vector<std::unique_ptr<Pipeline>> pipelines;
    std::vector<std::unique_ptr<PipelineLayout>> layouts;
    std::vector<Pipeline *> pipelineArray;
    // Add copy from staging buffer to a buffer
    std::unique_ptr<Set> uboSet;
    // For secondary commands
    VkCommandPool cmdPool;
    // Just because that's easier for allocations
    VkCommandBufferAllocateInfo cmdInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, 0, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 0};
    const QueueFamily *graphicFamily;
    VkQueue graphicQueue;
    VkQueue computeQueue;
    unsigned int ojmAlignment = 3*8*sizeof(float); // obj : POS3D(3) TEXTURE(2) NORMAL(3)
    uint32_t frameIdx = 2;
    uint32_t lastFrameIdx = 1;
};

#endif /* end of include guard: CONTEXT_HPP_ */
