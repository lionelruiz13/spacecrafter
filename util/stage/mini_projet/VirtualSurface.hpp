#include "Vulkan.hpp"

class VirtualSurface {
public:
    VirtualSurface(Vulkan *_master);
    ~VirtualSurface();
    VkCommandPool &getTransfertPool() {return transferPool;};
    void submitTransfert(VkCommandBuffer &command, VkSubmitInfo *submitInfo);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    const VkPipelineViewportStateCreateInfo &getViewportState() {return master->getViewportState();}
    auto &getGraphicsQueueIndex() {return master->getGraphicsQueueIndex();}

    const VkDevice &refDevice;
    const VkRenderPass &refRenderPass;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    const int &refFrameIndex;

    VkExtent2D swapChainExtent;
private:
    VkCommandPool transferPool;
    VkQueue transferQueue;
    VkQueue graphicsQueue;
    Vulkan *master;
    int swapchainSize;
};

class Shader {
public:
    Shader();
    // Note : the extension define what kind of shader it was
    void init(std::string filename);
    VkPipelineShaderStageCreateInfo getStageInfo() {return stageInfo;}
private:
    VkPipelineShaderStageCreateInfo stageInfo;
};

class Uniform {
public:
    Uniform(VkDeviceSize _uniformSize, VkShaderStageFlags _stages);

    void update();
    VkShaderStageFlags getStage() {return stages;};
private:
    VkDeviceSize uniformSize;
    VkShaderStageFlags stages;
};

class Buffer {
public:
    Buffer(VirtualSurface *_master, int size) {}
    ~Buffer();
    VkBuffer &get() {return buffer;}
    void update();
    //! Intermediate buffer, write your datas here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory bufferMemory;
    VkBuffer buffer;
    VkSubmitInfo submitInfo{};
};

class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *_master, int size,
        const VkVertexInputBindingDescription &_bindingDesc,
        const std::vector<VkVertexInputAttributeDescription> &_attributeDesc);
    //! Update vertex content with data member
    VkBuffer &get() {return vertexBuffer;}
    void update();
    const VkVertexInputBindingDescription &getBindingDesc() {return bindingDesc;};
    const std::vector<VkVertexInputAttributeDescription> &getAttributeDesc() {return attributeDesc;};
    //! Intermediate buffer, write your vertex here
    void *data;
private:
    VirtualSurface *master;
    VkCommandBuffer updater;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer vertexBuffer;
    VkSubmitInfo submitInfo{};
    VkVertexInputBindingDescription bindingDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
};

class PipelineLayout {
public:
    PipelineLayout(VirtualSurface *_master);
    ~PipelineLayout();
    //! @brief Bind uniform
    //! @param stages combination of flags describing the types of shader accessing it (vertex, fragment, etc.)
    void bindUniform(Uniform *uniform, uint32_t binding);
    void pushConstant(); // set constant values
    //! Build pipelineLayout.
    void build();
    VkPipelineLayout &getPipelineLayout() {return pipelineLayout;};
    const std::vector<VkDescriptorSet> &getDescriptorSets() {return uniforms;};
private:
    VirtualSurface *master;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayoutBinding> uniformsLayout;
    std::vector<VkDescriptorSet> uniforms;
    VkDescriptorSetLayout descriptor;
    bool builded = false;
};

class Pipeline {
public:
    Pipeline(VirtualSurface *master, PipelineLayout *layout, std::vector<VkDynamicState> _dynamicStates = {});
    ~Pipeline();
    void bindShader(Shader *shader);
    void bindVertex(VertexBuffer *vertex, uint32_t binding);
    //! Set draw topology
    void setTopology(VkPrimitiveTopology state, bool enableStripBreaks = false);
    //! Set blend mode
    void setBlendMode(const VkPipelineColorBlendAttachmentState &blendMode, float depthBiasClamp, float depthBiasSlopeFactor);
    //! Build pipeline for use
    void build();
    VkPipeline &get() {return graphicsPipeline;}
private:
    VirtualSurface *master;
    VkPipeline graphicsPipeline;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDynamicStateCreateInfo dynamicState{};

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

class CommandMgr {
public:
    CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool singleUseCommands);
    ~CommandMgr();
    //! Set dependency, must be done before enabling submit
    void setSubmitInfo(int index, VkSubmitInfo &_submitinfo);
    //! Start recording commands
    void init(int index);
    //! @brief begin render pass
    //! @param clearColor set to 1 to clear screen, 0 otherwise
    void beginRenderPass();
    void endRenderPass();
    void bindVertex(VertexBuffer *vertex, uint32_t firstBinding = 0, uint32_t bindingCount = 1, VkDeviceSize offset = 0);
    void bindPipeline(Pipeline *pipeline);
    void bindDescriptorSets(PipelineLayout *pipelineLayout);
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    //! @brief Multiple draw using buffer content as draw arguments.
    //! @param drawArgsArray content must be VkDrawIndirectCommand.
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 0, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount);
    //! Finalize recording
    void compile();
    //! define if on command buffer must be submitted
    void setSubmitState(int index, bool submitState);
    //! submit all commands
    void submit();
    //! reset all command buffer (for single-use command buffers)
    void reset();
private:
    VirtualSurface *master;
    const VkDevice &refDevice;
    VkQueue queue;
    VkCommandBuffer actual;
    const VkRenderPass &refRenderPass;
    const std::vector<VkFramebuffer> &refSwapChainFramebuffers;
    //! Content attached to frame
    struct frame {
        VkCommandPool cmdPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSubmitInfo> submitInfo;
        std::vector<VkSubmitInfo> submitList;
        VkCommandBuffer actual;
        VkFence fence;
    };
    //! actual frame index
    const int &refFrameIndex;
    std::vector<struct frame> frames;
    const bool singleUse;
};
