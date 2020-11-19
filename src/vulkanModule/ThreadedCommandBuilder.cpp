#include "CommandMgr.hpp"
#include "CmdEvent.hpp"
#include "ThreadedCommandBuilder.hpp"
#include "Set.hpp"
#include <cstring> // for memcpy
#include <cassert> // for assert

#define CALL(func, flag, args) case CmdEventType::flag:mutexExec.lock();master->func args;mutexExec.unlock();break

#define PUSH(type, name) arg.type = name; events.push(arg); ++nbArgs
#define DEF(flag) CmdEventType eventType = CmdEventType::flag;CmdEvent arg;int nbArgs=-1; mutex.lock(); PUSH(type, eventType)
#define UDEF() assert(getEventArgCount(eventType) == nbArgs);mutex.unlock()

#define CHECK(count) assert(getEventArgCount(event.type) == count)

#define ARG0() ();CHECK(0)
#define ARG1(type1) (arg1.type1);CHECK(1)
#define ARG2(type1, type2) (arg2.type1, arg1.type2);CHECK(2)
#define ARG3(type1, type2, type3) (arg3.type1, arg2.type2, arg1.type3);CHECK(3)
#define ARG4(type1, type2, type3, type4) (arg4.type1, arg3.type2, arg2.type3, arg1.type4);CHECK(4)
#define ARG5(type1, type2, type3, type4, type5) (arg5.type1, arg4.type2, arg3.type3, arg2.type4, arg1.type5);CHECK(5)

ThreadedCommandBuilder::ThreadedCommandBuilder(CommandMgr *_master) : master(_master), usedSetCacheCount(0), thread(startMainloop, this)
{
    setCache.resize(64);
    buffer.resize(65536);
}

ThreadedCommandBuilder::~ThreadedCommandBuilder()
{
    terminate();
    thread.join();
}

void ThreadedCommandBuilder::waitCompiled(uint8_t nbCommands)
{
    while (isCompiled < nbCommands) {
        std::this_thread::yield();
    }
    isCompiled -= nbCommands;
}

void ThreadedCommandBuilder::waitIdle(bool resetCompiledCount)
{
    while (!events.empty() || processing) {
        std::this_thread::yield();
    }
    if (resetCompiledCount) {
        isCompiled = 0;
    }
}

void ThreadedCommandBuilder::mainloop()
{
    CmdEvent event, arg1, arg2, arg3, arg4, arg5;
    while (true) {
        while (events.empty())
            std::this_thread::yield();
        processing = true;
        mutex.lock();
        if (events.empty()) {
            mutex.unlock();
            processing = false;
            continue;
        }
        event = events.front();
        events.pop();
        switch (getEventArgCount(event.type)) {
            case 5: arg5 = events.front(); events.pop(); [[fallthrough]];
            case 4: arg4 = events.front(); events.pop(); [[fallthrough]];
            case 3: arg3 = events.front(); events.pop(); [[fallthrough]];
            case 2: arg2 = events.front(); events.pop(); [[fallthrough]];
            case 1: arg1 = events.front(); events.pop(); [[fallthrough]];
            default: break;
        }
        mutex.unlock();
        switch (event.type) {
            CALL(reset, RESET, ARG0());
            CALL(init, INIT, ARG2(i,b));
            CALL(select, SELECT, ARG1(i));
            CALL(grab, GRAB, ARG1(cmd));
            CALL(beginRenderPass, BEGIN_RENDER_PASS, ARG2(passT, passC));
            CALL(endRenderPass, END_RENDER_PASS, ARG0());
            CALL(updateVertex, UPDATE_VERTEX_ARRAY, ARG1(ptrV));
            CALL(bindVertex, BIND_VERTEX_ARRAY, ARG1(ptrV));
            CALL(bindPipeline, BIND_PIPELINE, ARG1(ptrP));
            CALL(bindSet, BIND_SET, ARG3(ptrPL, ptrS, i));
            CALL(pushSet, PUSH_SET, ARG3(ptrPL, ptrS, i);--usedSetCacheCount);
            CALL(pushConstant, PUSH_CONSTANT, ARG5(ptrPL, sf, ui, ptr, ui));
            CALL(draw, DRAW, ARG4(ui, ui, ui, ui));
            CALL(drawIndexed, DRAW_INDEXED, ARG5(ui, ui, ui, ui, ui));
            CALL(compile, COMPILE, ARG0();isCompiled++);
            case CmdEventType::TERMINATE:
                mutex.lock();
                while (!events.empty())
                    events.pop();
                mutex.unlock();
                processing = false;
                return;
        }
        processing = false;
    }
}

void ThreadedCommandBuilder::terminate()
{
    DEF(TERMINATE);
    UDEF();
}

void ThreadedCommandBuilder::reset()
{
    DEF(RESET);
    UDEF();
}

void ThreadedCommandBuilder::init(int index, bool compileSelected)
{
    DEF(INIT);
    PUSH(i, index);
    PUSH(b, compileSelected);
    UDEF();
}

void ThreadedCommandBuilder::select(int index)
{
    DEF(SELECT);
    PUSH(i, index);
    UDEF();
}

void ThreadedCommandBuilder::grab(VkCommandBuffer buffer)
{
    DEF(GRAB);
    PUSH(cmd, buffer);
    UDEF();
}

void ThreadedCommandBuilder::beginRenderPass(renderPassType renderPassType, renderPassCompatibility compatibility)
{
    DEF(BEGIN_RENDER_PASS);
    PUSH(passT, renderPassType);
    PUSH(passC, compatibility);
    UDEF();
}

void ThreadedCommandBuilder::endRenderPass()
{
    DEF(END_RENDER_PASS);
    UDEF();
}

void ThreadedCommandBuilder::updateVertex(VertexArray *vertex)
{
    DEF(UPDATE_VERTEX_ARRAY);
    PUSH(ptrV, vertex);
    UDEF();
}

void ThreadedCommandBuilder::bindVertex(VertexArray *vertex)
{
    DEF(BIND_VERTEX_ARRAY);
    PUSH(ptrV, vertex);
    UDEF();
}

void ThreadedCommandBuilder::bindPipeline(Pipeline *pipeline)
{
    DEF(BIND_PIPELINE);
    PUSH(ptrP, pipeline);
    UDEF();
}

void ThreadedCommandBuilder::bindSet(PipelineLayout *pipelineLayout, Set *uniform, int binding)
{
    DEF(BIND_SET);
    PUSH(ptrPL, pipelineLayout);
    PUSH(ptrS, uniform);
    PUSH(i, binding);
    UDEF();
}

void ThreadedCommandBuilder::pushSet(PipelineLayout *pipelineLayout, Set *uniform, int binding)
{
    while (usedSetCacheCount.load() >= setCache.size())
        std::this_thread::yield();
    DEF(PUSH_SET);
    ++usedSetCacheCount;
    PUSH(ptrPL, pipelineLayout);
    setCache[setCacheOffset].swapWrites(uniform->getWrites());
    PUSH(ptrS, &setCache[setCacheOffset]);
    if (++setCacheOffset >= setCache.size())
        setCacheOffset = 0;

    PUSH(i, binding);
    UDEF();
}

void ThreadedCommandBuilder::pushConstant(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, const void *data, uint32_t size)
{
    DEF(PUSH_CONSTANT);
    PUSH(ptrPL, pipelineLayout);
    PUSH(sf, stage);
    PUSH(ui, offset);

    if (bufferOffset + size >= buffer.size())
        bufferOffset = 0;
    memcpy(buffer.data() + bufferOffset, data, size);
    PUSH(ptr, buffer.data() + bufferOffset);
    bufferOffset += size;

    PUSH(ui, size);
    UDEF();
}

void ThreadedCommandBuilder::pushConstantNoCopy(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, void *data, uint32_t size)
{
    DEF(PUSH_CONSTANT);
    PUSH(ptrPL, pipelineLayout);
    PUSH(sf, stage);
    PUSH(ui, offset);
    PUSH(ptr, data);
    PUSH(ui, size);
    UDEF();
}

void ThreadedCommandBuilder::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    DEF(DRAW);
    PUSH(ui, vertexCount);
    PUSH(ui, instanceCount);
    PUSH(ui, firstVertex);
    PUSH(ui, firstInstance);
    UDEF();
}

void ThreadedCommandBuilder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    DEF(DRAW_INDEXED);
    PUSH(ui, indexCount);
    PUSH(ui, instanceCount);
    PUSH(ui, firstIndex);
    PUSH(ui, vertexOffset);
    PUSH(ui, firstInstance);
    UDEF();
}

void ThreadedCommandBuilder::compile()
{
    DEF(COMPILE);
    UDEF();
}

void ThreadedCommandBuilder::init(int index, Pipeline *pipeline, renderPassType renderPassType, bool compileSelected, renderPassCompatibility compatibility)
{
    init(index, compileSelected);
    beginRenderPass(renderPassType, compatibility);
    bindPipeline(pipeline);
}

int ThreadedCommandBuilder::getCommandIndex()
{
    mutexExec.lock();
    int commandIndex = master->getCommandIndex();
    mutexExec.unlock();
    return commandIndex;
}

void ThreadedCommandBuilder::setName(int commandIndex, const std::string &name)
{
    mutexExec.lock();
    master->setName(commandIndex, name);
    mutexExec.unlock();
}
