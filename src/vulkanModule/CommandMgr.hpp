#ifndef COMMAND_MGR_HPP
#define COMMAND_MGR_HPP

#include <vulkan/vulkan.h>
#include <list>
#include <vector>
#include <set>
#include <array>

class Vulkan;
class VirtualSurface;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Buffer;
class Set;
class Texture;

/**
    multisampling is considered disabled if
    RESOLVE renderPassType require RESOLVE renderPassCompatibility
    SINGLE_SAMPLE renderPassType require SINGLE_SAMPLE renderPassCompatibility
    all other renderPassType require DEFAULT renderPassCompatibility
*/
enum class renderPassType : uint8_t {
    CLEAR = 0,
    DEFAULT = 1,
    CLEAR_DEPTH_BUFFER_DONT_SAVE = 2, // Clear depth buffer and use it for this render pass only
    CLEAR_DEPTH_BUFFER = 3, // Clear depth buffer and save it for next render pass
    USE_DEPTH_BUFFER = 4, // Load previous depth buffer and save it for next render pass
    USE_DEPTH_BUFFER_DONT_SAVE = 5, // Load previous depth buffer without saving it for next render pass
    PRESENT = 6, // Prepair frameBuffer for presentation (only use if multisampling is disabled)
    DRAW_USE = 7, // Prepair framebuffer for use in shader as sampler2D
    SINGLE_PASS = 8, // combine CLEAR and PRESENT - require SINGLE_SAMPLE renderPassCompatibility
    SINGLE_PASS_DRAW_USE = 9, // combine CLEAR and DRAW_USE
    DEPTH_BUFFER_SINGLE_PASS_DRAW_USE = 10, // combine CLEAR, CLEAR_DEPTH_BUFFER_DONT_SAVE and DRAW_USE
    SINGLE_PASS_DRAW_USE_ADDITIVE = 11, // combine DEFAULT and DRAW_USE
    DEPTH_BUFFER_SINGLE_PASS_DRAW_USE_ADDITIVE = 12, // combine CLEAR_DEPTH_BUFFER_DONT_SAVE and DRAW_USE
    RESOLVE_DEFAULT = 13, // multisampling resolve with OVERWRITE pass for single sampled attachment
    RESOLVE_PRESENT = 14, // multisampling resolve with PRESENT action for single sampled attachment
    SINGLE_SAMPLE_CLEAR = 15, // single sampled CLEAR without depth buffer attachment
    SINGLE_SAMPLE_DEFAULT = 16, // single sampled DEFAULT without depth buffer attachment
    SINGLE_SAMPLE_PRESENT = 17 // single sampled PRESENT without depth buffer attachment
};

enum class renderPassCompatibility : uint8_t {
    DEFAULT = 0,
    RESOLVE = 1,
    SINGLE_SAMPLE = 2
};

/**
*   \brief Handle commandBuffer, including allocation, build, synchronisation and submisssion
*   Every calls to this CommandMgr must be in the same thread, or access to CommandMgr must be externally synchronized
*   However, calls to CommandMgr doesn't depend to VirtualSurface's thread (excepted for frame submission if not external)
*/
class CommandMgr {
public:
    //! Create CommandMgr for assisted command recording only (singleUseCommands == true, isExternal == true)
    CommandMgr(Vulkan *_master);
    //! @param nbCommandBuffers number of commandBuffers to create
    CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool submissionPerFrame = false, bool singleUseCommands = false, bool isExternal = false, bool enableIndividualReset = false);
    ~CommandMgr();
    //! @brief Start recording command
    //! @param compileSelected if true, compile selected command
    void init(int index, bool compileSelected = true);
    //! Start recording command, begin render pass and bind pipeline
    void init(int index, Pipeline *pipeline, renderPassType renderPassType = renderPassType::DEFAULT, bool compileSelected = true, renderPassCompatibility compatibility = renderPassCompatibility::DEFAULT);
    //! Start recording new command, begin render pass, bind pipeline and return command index
    int initNew(Pipeline *pipeline, renderPassType renderPassType = renderPassType::DEFAULT, bool compileSelected = true, renderPassCompatibility compatibility = renderPassCompatibility::DEFAULT);
    //! Change selected command, new selected command must be initialized and not compiled
    void select(int index);
    //! Select external command, available for CommandMgr created with singleUseCommands
    //! @param buffer the commandBuffer to select, or VK_NULL_HANDLE to deselect
    void grab(VkCommandBuffer buffer = VK_NULL_HANDLE);
    //! @brief begin render pass
    //! @param renderPassType inform where renderPass is situated
    void beginRenderPass(renderPassType renderPassType, renderPassCompatibility compatibility = renderPassCompatibility::DEFAULT);
    void endRenderPass();
    void updateVertex(VertexArray *vertex);
    void updateVertex(VertexBuffer *vertex);
    void bindVertex(VertexArray *vertex);
    void bindVertex(VertexBuffer *vertex, uint32_t firstBinding = 0, VkDeviceSize offset = 0);
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
    void vkIf(Buffer *bool32, VkDeviceSize offset = 0, bool invert = false);
    //! @brief end conditionnal rendering (affect draw commands only)
    void vkEndIf();
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    //! @brief Multiple draw using buffer content as draw arguments.
    //! @param drawArgsArray content must be VkDrawIndirectCommand.
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    //! @brief Add pipeline image barrier
    //! @param miplevel specify which mip level is concerned, or -1 for all mip levels
    void addImageBarrier(Texture *texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t miplevel = UINT32_MAX, uint32_t miplevelcount = 1);
    //! Record all dependencies previously added
    void compileBarriers(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkPipelineStageFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT);
    //! Execute blit operation
    void blit(Texture *src, Texture *dst, uint32_t srcMipLevel = 0, uint32_t dstMipLevel = 0, VkFilter filter = VK_FILTER_LINEAR);
    //! Finalize recording and deselect command
    void compile();
    //! @brief set command submission to be submitted by .submit()
    void setSubmission(int index, bool needDepthBuffer = false, CommandMgr *target = nullptr);
    //! reset submission state for all commands
    void resetSubmission();
    //! declare submisssion
    void submitGuard();
    //! execute submission
    void submitAction();
    //! submit all commands (combine submitGuard and submitAction)
    void submit();
    //! reset all command buffer (for single-use command buffers), but keep their submission state
    void reset();
    //! wait all pending submissions complete
    void waitGraphicQueueIdle();
    //! wait all submitted command for actual frameIndex end
    void waitCompletion() {vkWaitForFences(refDevice, 1, &frames[refFrameIndex].fence, VK_TRUE, UINT64_MAX);}
    //! wait all submitted command for specific frameIndex end
    void waitCompletion(uint32_t frameIndex) {vkWaitForFences(refDevice, 1, &frames[frameIndex].fence, VK_TRUE, UINT64_MAX);}
    bool isCompleted(uint32_t frameIndex) {return (vkWaitForFences(refDevice, 1, &frames[frameIndex].fence, VK_TRUE, 0) == VK_SUCCESS);}
    void setTopSemaphore(int frameIndex, const VkSemaphore &topSemaphore) {isLinked=true;frames[frameIndex].topSemaphore = topSemaphore;}
    const VkSemaphore &getBottomSemaphore(int frameIndex) {return frames[frameIndex].bottomSemaphore;}
    int getCommandIndex();
    //! Internal use only
    static void setPFN_vkCmdPushDescriptorSetKHR(PFN_vkCmdPushDescriptorSetKHR pfn) {PFN_pushSet = pfn;}
    //! Internal use only
    static void setPFN_vkCmdBeginConditionalRenderingEXT(PFN_vkCmdBeginConditionalRenderingEXT pfn) {PFN_vkIf = pfn;}
    //! Internal use only
    static void setPFN_vkCmdEndConditionalRenderingEXT(PFN_vkCmdEndConditionalRenderingEXT pfn) {PFN_vkEndIf = pfn;}
    //! Tell if one command is in recording state
    bool isRecording() const {return frames[0].actual != VK_NULL_HANDLE;}
    //! Immediately release as many unused memory as possible, to call when out of memory
    void releaseUnusedMemory();
private:
    //! resolve semaphore dependencies for one frame
    void resolve(uint8_t frameIndex);

    static VkPipelineStageFlags defaultStage;
    VirtualSurface *master;
    const VkDevice &refDevice;
    VkQueue queue;
    VkCommandBuffer actual;
    const std::vector<VkRenderPass> &refRenderPass;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const std::vector<VkFramebuffer> &refResolveFramebuffers;
    const std::vector<VkFramebuffer> &refSingleSampleFramebuffers;
    VkCommandPool cmdPool = VK_NULL_HANDLE; // used if singleUse is set to false
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
    std::vector<VkImageMemoryBarrier> imageBarrier;
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
    bool isLinked = false;
    short nbCommandBuffers;
    short autoIndex = 0;
    bool inRenderPass = false;
    bool hasPipeline = false; // tell if there is a pipeline binded
};

#endif /* end of include guard: COMMAND_MGR_HPP */
