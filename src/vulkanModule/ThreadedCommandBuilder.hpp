#ifndef THREADED_COMMAND_BUILDER
#define THREADED_COMMAND_BUILDER

#include <queue>
class CommandMgr;
union CmdEvent;

class ThreadedCommandBuilder {
public:
    ThreadedCommandBuilder(CommandMgr *_master);
    ~ThreadedCommandBuilder();
    void mainloop();
    void waitCompiled(uint8_t nbCommands = 1);
    void waitIdle();
private:
    CommandMgr *master;
    std::queue<CmdEvent> events;
    uint8_t isCompiled = 0;
public:
    // Virtual CommandMgr's calls
    void init(int index);
    void beginRenderPass(renderPassType renderPassType);
    void endRenderPass();
    void bindVertex(VertexArray *vertex);
    void bindVertex(VertexBuffer *vertex, uint32_t firstBinding = 0, uint32_t bindingCount = 1, VkDeviceSize offset = 0);
    void bindIndex(Buffer *buffer, VkIndexType indexType, VkDeviceSize offset = 0);
    void bindPipeline(Pipeline *pipeline);
    void bindSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    void pushSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    void pushConstant(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, const void *data, uint32_t size);
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset = 0, uint32_t drawCount = 1);
    void compile();
    //! Exit ThreadedCommandBuilder's mainloop
    void terminate();
};

#endif /* end of include guard: THREADED_COMMAND_BUILDER */
