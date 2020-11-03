#ifndef THREADED_COMMAND_BUILDER_HPP
#define THREADED_COMMAND_BUILDER_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include "CommandMgr.hpp"
union CmdEvent;

class ThreadedCommandBuilder {
public:
    ThreadedCommandBuilder(CommandMgr *_master);
    ~ThreadedCommandBuilder();
    void waitCompiled(uint8_t nbCommands = 1);
    void waitIdle(bool resetCompiledCount = true);
    //! Exit ThreadedCommandBuilder's mainloop
    void terminate();
private:
    void mainloop();
    static void startMainloop(ThreadedCommandBuilder *self) {self->mainloop();}
    CommandMgr *master;
    std::mutex mutex; // mutex for events queue
    std::queue<CmdEvent> events;
    std::mutex mutexExec; // mutex for access to CommandMgr
    static const int BUFFER_SIZE;
    std::vector<char> buffer;
    uint32_t bufferOffset = 0;
    std::vector<Set> setCache;
    uint32_t setCacheOffset = 0;
    std::atomic<uint32_t> usedSetCacheCount;
    uint8_t isCompiled = 0;
    bool processing = false;
    std::thread thread;
public:
    // Virtual CommandMgr's calls
    void reset();
    void init(int index, bool compileSelected = true);
    void select(int index);
    void grab(VkCommandBuffer buffer = VK_NULL_HANDLE);
    void beginRenderPass(renderPassType renderPassType, renderPassCompatibility compatibility = renderPassCompatibility::DEFAULT);
    void endRenderPass();
    void updateVertex(VertexArray *vertex);
    void bindVertex(VertexArray *vertex);
    void bindPipeline(Pipeline *pipeline);
    //! Doesn't support virtual uniform yet
    void bindSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    void pushSet(PipelineLayout *pipelineLayout, Set *uniform, int binding = 0);
    void pushConstant(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, const void *data, uint32_t size);
    //! Don't copy content of data, assume his content is unchanged before the next frame
    void pushConstantNoCopy(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, void *data, uint32_t size);
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void compile();
    // Combined CommandMgr's calls
    void init(int index, Pipeline *pipeline, renderPassType renderPassType = renderPassType::DEFAULT, bool compileSelected = true, renderPassCompatibility compatibility = renderPassCompatibility::DEFAULT);
    // Direct CommandMgr's calls
    int getCommandIndex();
    void setSubmission(int index, bool needDepthBuffer = false, CommandMgr *target = nullptr) {master->setSubmission(index, needDepthBuffer, target);}
};

#endif /* end of include guard: THREADED_COMMAND_BUILDER_HPP */
