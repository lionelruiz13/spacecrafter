#include "CommandMgr.hpp"
#include "CmdEvent.hpp"
#include "ThreadedCommandBuilder.hpp"
#include <thread>

ThreadedCommandBuilder::ThreadedCommandBuilder(CommandMgr *_master) : master(_master) {}
ThreadedCommandBuilder::~ThreadedCommandBuilder() {}

void ThreadedCommandBuilder::waitCompiled(uint8_t nbCommands)
{
    while (isCompiled < nbCommands) {
        std::this_thread::yield();
    }
    isCompiled -= nbCommands;
}

void ThreadedCommandBuilder::waitIdle()
{
    while (!events.empty()) {
        std::this_thread::yield();
    }
}

void ThreadedCommandBuilder::mainloop()
{
    while (true) {
        while (events.empty()) {
            std::this_thread::yield();
        }
        CmdEvent &event = events.front();
        switch (event.type) {
            case CmdEventType::RESET:
                master->reset();
                break;
            case CmdEventType::INIT:
                master->init(event.init.index);
                break;
            case CmdEventType::BEGIN_RENDER_PASS:
                break;
            case CmdEventType::BIND_VERTEX_ARRAY:
                break;
            case CmdEventType::BIND_VERTEX_BUFFER:
                break;
            case CmdEventType::BIND_INDEX:
                break;
            case CmdEventType::BIND_PIPELINE:
                break;
            case CmdEventType::BIND_SET:
                break;
            case CmdEventType::PUSH_SET:
                break;
            case CmdEventType::PUSH_CONSTANT:
                break;
            case CmdEventType::DRAW:
                break;
            case CmdEventType::INDIRECT_DRAW:
                break;
            case CmdEventType::DRAW_INDEXED:
                break;
            case CmdEventType::INDIRECT_DRAW_INDEXED:
                break;
            case CmdEventType::END_RENDER_PASS:
                master->endRenderPass();
                break;
            case CmdEventType::COMPILE:
                master->compile();
                isCompiled++;
                break;
            case CmdEventType::TERMINATE:
                while (!events.empty())
                    events.pop();
                return;
        }
        events.pop();
    }
}
