#ifndef COMMAND_MGR_HPP
#define COMMAND_MGR_HPP

#include <vulkan/vulkan.h>
#include <list>
#include <vector>
#include <set>
#include <array>

class VirtualSurface;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Buffer;
class Set;

enum class renderPassType : uint8_t {
    CLEAR = 0,
    DEFAULT = 1,
    CLEAR_DEPTH_BUFFER_DONT_SAVE = 2, // Clear depth buffer and use it for this render pass only
    CLEAR_DEPTH_BUFFER = 3, // Clear depth buffer and save it for next render pass
    USE_DEPTH_BUFFER = 4, // Load previous depth buffer and save it for next render pass
    USE_DEPTH_BUFFER_DONT_SAVE = 5, // Load previous depth buffer without saving it for next render pass
    PRESENT = 6, // Prepair frameBuffer for presentation
    SINGLE_PASS = 7 // This command is the unique command of the whole
};

class CommandMgr {
public:
    //! @param nbCommandBuffers number of commandBuffers to create
    ~CommandMgr();
    CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool submissionPerFrame = false, bool singleUseCommands = false, bool isExternal = false);
    //! Start recording commands
    void init(int index);
    //! @brief begin render pass
    //! @param renderPassType inform where renderPass is situated
    void beginRenderPass(renderPassType renderPassType);
    void endRenderPass();
    void updateVertex(VertexArray *vertex);
    void bindVertex(VertexArray *vertex);
    void bindVertex(VertexBuffer *vertex, uint32_t firstBinding = 0, uint32_t bindingCount = 1, VkDeviceSize offset = 0);
    void bindIndex(Buffer *buffer, VkIndexType indexType, VkDeviceSize offset = 0);
    void bindPipeline(Pipeline *pipeline);
    //! @brief bind uniform buffers and textures
    void bindSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    //! @brief push uniform buffers and textures
    void pushSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    //! @brief push constant values
    void pushConstant(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, const void *data, uint32_t size);
    //! @brief begin conditionnal rendering (affect draw commands only)
    //! @param offset must be a multiple of 4
    void vkIf(Buffer *bool32, VkDeviceSize offset, bool invert);
    //! @brief end conditionnal rendering (affect draw commands only)
    void vkEndIf();
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    //! @brief Multiple draw using buffer content as draw arguments.
    //! @param drawArgsArray content must be VkDrawIndirectCommand.
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    //! Finalize recording
    void compile();
    //! @brief set command submission to be submitted by .submit()
    void setSubmission(int index, bool needDepthBuffer = false, CommandMgr *target = nullptr);
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
    int getCommandIndex();
    static void setPFN_vkCmdPushDescriptorSetKHR(PFN_vkCmdPushDescriptorSetKHR pfn) {PFN_pushSet = pfn;}
    static void setPFN_vkCmdBeginConditionalRenderingEXT(PFN_vkCmdBeginConditionalRenderingEXT pfn) {PFN_vkIf = pfn;}
    static void setPFN_vkCmdEndConditionalRenderingEXT(PFN_vkCmdEndConditionalRenderingEXT pfn) {PFN_vkEndIf = pfn;}
    bool isRecording() const {return actual != VK_NULL_HANDLE;}
private:
    //! resolve semaphore dependencies for one frame
    void resolve(uint8_t frameIndex);

    static VkPipelineStageFlags defaultStage;
    VirtualSurface *master;
    const VkDevice &refDevice;
    VkQueue queue;
    VkCommandBuffer actual;
    const std::array<VkRenderPass, 8> &refRenderPass;
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
    static PFN_vkCmdPushDescriptorSetKHR PFN_pushSet;
    static PFN_vkCmdBeginConditionalRenderingEXT PFN_vkIf;
    static PFN_vkCmdEndConditionalRenderingEXT PFN_vkEndIf;
    const VkPipelineStageFlags stages[2] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT};
    //! actual frame index
    const uint32_t &refFrameIndex;
    std::vector<struct frame> frames;
    const bool singleUse;
    const bool submissionPerFrame;
    bool needResolve = true;
    short nbCommandBuffers;
    short autoIndex = 0;
};

#endif /* end of include guard: COMMAND_MGR_HPP */
