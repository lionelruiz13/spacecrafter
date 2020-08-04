#include "Vulkan.hpp"

class VirtualSurface {
public:
    VirtualSurface(Vulkan *master);
    ~VirtualSurface();
private:
};

class PipelineLayout {
public:
    PipelineLayout(VirtualSurface *master);
    void bindUniform(Uniform *uniform);
    void pushConstant(); // set constant values
    //! Build pipelineLayout.
    void build();
    //! @brief Create uniform - can be performed after building
    //! @param stages combination of flags describing the types of shader accessing it (vertex, fragment, etc.)
};

class Pipeline {
public:
    Pipeline(VirtualSurface *master, PipelineLayout *layout, std::vector<VkDynamicState> dynamicStates = {});
    ~Pipeline();
    //! Use this pipeline for draw
    void use(vkCommandBuffer *cmdBuffer);
    void bindShader(Shader *shader);
    void bindVertex(VertexBuffer *vertex);
    //! Set draw topology
    void setTopology(topology state, bool enableStripBreaks = false);
    //! Set blend mode
    void setBlendMode(VkPipelineColorBlendAttachmentState blendMode);
    //! Build pipeline for use
    void build();
    enum class topology {
        POINT_LIST = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        LINE_STRIP = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        TRIANGLE_STRIP = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        TRIANGLE_FAN = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        LINE_LIST_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
        LINE_STRIP_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
        TRIANGLE_LIST_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
        TRIANGLE_STRIP_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
        PATCH_LIST = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
    };
};

class Shader {
public:
    Shader();
    // Note : the extension define what kind of shader it was
    void init(std::string filename);
};

class Uniform {
public:
    Uniform(VkDeviceSize uniformSize, VkShaderStageFlags stages);

    void update();
};

template <struct T>
class VertexBuffer {
public:
    VertexBuffer(VirtualSurface *master, int size, const VkVertexInputBindingDescription &bindingDesc, const std::vector<VkVertexInputAttributeDescription> attributeDesc);
    //! Update vertex content with data member
    void update();
    T *data;
};

class CommandMgr {
public:
    CommandMgr(VirtualSurface *master, int nbCommandStack, bool singleUseCommands);
    //! Start recording commands
    void init(int index);
    void beginRenderPass();
    void endRenderPass();
    void bindVertex(VertexBuffer *vertex);
    void bindPipeline(Pipeline *pipeline);
    void bindUniform(Uniform *uniform);
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    //! @brief Multiple draw using buffer content as draw arguments.
    //! @param drawArgsArray content must be VkDrawIndirectCommand.
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount);
    //! Compile command stack for submisson
    void compile();
    //! submit command stack
    void submit(int index);
    //! reset all command stack (for single-use command buffers)
    void reset();
}
