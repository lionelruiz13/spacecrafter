#ifndef CMD_EVENT_HPP
#define CMD_EVENT_HPP

enum class CmdEventType : uint8_t {
    RESET,
    INIT,
    BEGIN_RENDER_PASS,
    BIND_VERTEX_ARRAY,
    BIND_VERTEX_BUFFER,
    BIND_INDEX,
    BIND_PIPELINE,
    BIND_SET,
    PUSH_SET,
    PUSH_CONSTANT,
    DRAW,
    INDIRECT_DRAW,
    DRAW_INDEXED,
    INDIRECT_DRAW_INDEXED,
    END_RENDER_PASS,
    COMPILE,
    TERMINATE
};

union CmdEvent {
    CmdEventType type;
    struct {
        CmdEventType type;
        int index;
    } init;
    struct {
        CmdEventType type;
        renderPassType passType;
    } beginRenderPass;
    struct {
        CmdEventType type;
        VertexArray *vertex;
    } bindVertexArray;
    struct {
        CmdEventType type;
        VertexBuffer *vertex;
        uint32_t firstBinding;
        uint32_t bindingCount;
        VkDeviceSize offset;
    } bindVertexBuffer;
    struct {
        CmdEventType type;
        Buffer *buffer;
        VkIndexType indexType;
        VkDeviceSize offset;
    } bindIndex;
    struct {
        CmdEventType type;
        Pipeline *pipeline;
    } bindPipeline;
    struct {
        CmdEventType type;
        PipelineLayout *pipelineLayout;
        Set *uniform;
        int binding;
    } set;
    struct {
        CmdEventType type;
        PipelineLayout *pipelineLayout;
        VkShaderStageFlags stage;
        uint32_t offset;
        const void *data;
        uint32_t size;
    } pushConstant;
    struct {
        CmdEventType type;
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    } draw;
    struct {
        CmdEventType type;
        Buffer *drawArgsArray;
        VkDeviceSize offset;
        uint32_t drawCount;
    } indirectDraw;
    struct {
        CmdEventType type;
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;
    } drawIndexed;
    struct {
        CmdEventType type;
        Buffer *drawArgsArray;
        VkDeviceSize offset;
        uint32_t drawCount;
    } indirectDrawIndexed;
};

#endif /* end of include guard: CMD_EVENT_HPP */
