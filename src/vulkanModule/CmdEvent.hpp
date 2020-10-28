#ifndef CMD_EVENT_HPP
#define CMD_EVENT_HPP

enum class CmdEventType : uint8_t {
    RESET,
    INIT,
    SELECT,
    GRAB,
    BEGIN_RENDER_PASS,
    END_RENDER_PASS,
    UPDATE_VERTEX_ARRAY,
    BIND_VERTEX_ARRAY,
    BIND_PIPELINE,
    BIND_SET,
    PUSH_SET,
    PUSH_CONSTANT,
    DRAW,
    DRAW_INDEXED,
    COMPILE,
    TERMINATE
};

static inline int getEventArgCount(CmdEventType eventType)
{
    switch (eventType) {
        case CmdEventType::RESET:
        case CmdEventType::END_RENDER_PASS:
        case CmdEventType::COMPILE:
        case CmdEventType::TERMINATE: return 0;
        case CmdEventType::SELECT:
        case CmdEventType::GRAB:
        case CmdEventType::UPDATE_VERTEX_ARRAY:
        case CmdEventType::BIND_VERTEX_ARRAY:
        case CmdEventType::BIND_PIPELINE: return 1;
        case CmdEventType::INIT:
        case CmdEventType::BEGIN_RENDER_PASS: return 2;
        case CmdEventType::BIND_SET:
        case CmdEventType::PUSH_SET: return 3;
        case CmdEventType::DRAW: return 4;
        case CmdEventType::DRAW_INDEXED:
        case CmdEventType::PUSH_CONSTANT: return 5;
    }
    return 0;
}

union CmdEvent {
    CmdEventType type;
    renderPassType passT;
    renderPassCompatibility passC;
    VkCommandBuffer cmd;
    VkShaderStageFlags sf;
    bool b;
    int i;
    uint32_t ui;
    size_t size;
    void *ptr;
    VertexArray *ptrV;
    Pipeline *ptrP;
    PipelineLayout *ptrPL;
    Set *ptrS;
};

#endif /* end of include guard: CMD_EVENT_HPP */
