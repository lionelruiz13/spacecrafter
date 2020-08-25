#ifndef COMMAND_MGR_HPP
#define COMMAND_MGR_HPP

#include <vulkan/vulkan.h>
#include <list>
#include <vector>
#include <set>
namespace v {

class VirtualSurface;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Buffer;
class UniformSet;

enum class renderPassType : uint8_t {
    CLEAR = 0,
    DEFAULT = 1,
    PRESENT = 2 // Prepair frameBuffer for presentation
};

class CommandMgr {
public:
    //! @param nbCommandBuffers number of commandBuffers to create
    ~CommandMgr();
    CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool submissionPerFrame = false, bool singleUseCommands = false);
    //! Start recording commands
    void init(int index);
    //! @brief begin render pass
    //! @param renderPassType inform where renderPass is situated
    void beginRenderPass(renderPassType renderPassType);
    void endRenderPass();
    void bindVertex(VertexBuffer *vertex, uint32_t firstBinding = 0, uint32_t bindingCount = 1, VkDeviceSize offset = 0);
    void bindIndex(Buffer *buffer, VkIndexType indexType, VkDeviceSize offset = 0);
    void bindPipeline(Pipeline *pipeline);
    //! @brief update uniform value
    void bindUniformSet(PipelineLayout *pipelineLayout, UniformSet *uniform);
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    //! @brief Multiple draw using buffer content as draw arguments.
    //! @param drawArgsArray content must be VkDrawIndirectCommand.
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 0, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount);
    //! Finalize recording
    void compile();
    //! @brief set command submission to be submitted by .submit()
    //! @param withPrevious set to true if you don't need ordered draw between this command and the previous command(s)
    void setSubmission(int index, bool withPrevious = false, VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    //! reset submission state for all commands
    void resetSubmission();
    //! submit all commands
    void submit();
    //! reset all command buffer (for single-use command buffers), but keep their submission state
    void reset();
    //! wait all submitted command end
    void waitCompletion(uint32_t frameIndex) {vkWaitForFences(refDevice, 1, &frames[frameIndex].fence, VK_TRUE, UINT64_MAX);}
    void setTopSemaphore(int frameIndex, const VkSemaphore &topSemaphore) {frames[frameIndex].topSemaphore = topSemaphore;}
    const VkSemaphore &getBottomSemaphore(int frameIndex) {return frames[frameIndex].bottomSemaphore;}
private:
    //! resolve semaphore dependencies for one frame
    void resolve(uint8_t frameIndex);

    static VkPipelineStageFlags defaultStage;
    VirtualSurface *master;
    const VkDevice &refDevice;
    VkQueue queue;
    VkCommandBuffer actual;
    const std::array<VkRenderPass, 3> &refRenderPass;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    VkCommandPool cmdPool; // used if singleUse is set to false
    //! Content attached to frame
    struct frame {
        VkCommandPool cmdPool; // used if singleUse is set to true
        VkCommandBuffer actual; // CommandBuffer on recording state
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> signalSemaphores;

        // Submission
        VkSemaphore topSemaphore;
        std::vector<std::vector<VkCommandBuffer>> submittedCommandBuffers;
        std::list<VkSemaphore> submittedSignalSemaphores; // Content address is used
        std::vector<VkSubmitInfo> submitList;
        VkSemaphore bottomSemaphore;
        VkFence fence;
    };
    std::set<VkPipelineStageFlags> stages; // Content address is used
    //! actual frame index
    const uint32_t &refFrameIndex;
    std::vector<struct frame> frames;
    const bool singleUse;
    const bool submissionPerFrame;
    bool needResolve = true;
};
}

#ifndef OPENGL_HPP
using namespace v;
#endif

#endif /* end of include guard: COMMAND_MGR_HPP */
