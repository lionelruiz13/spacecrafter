#ifndef CONTEXT_HPP
#define CONTEXT_HPP

class Vulkan;
class TextureMgr;
class ResourceTracker;

class PipelineLayout;
class Set;

class VirtualSurface;
class SetMgr;
class CommandMgr;
class ThreadedCommandBuilder;

typedef struct GlobalContext {
    Vulkan *vulkan;
    TextureMgr *textureMgr;
    ResourceTracker *tracker;
    PipelineLayout *globalLayout; // for uboCam
    Set *globalSet;
} GlobalContext;

typedef struct ThreadContext {
    GlobalContext *global;
    VirtualSurface *surface;
    SetMgr *setMgr;
    CommandMgr *commandMgr;
    CommandMgr *commandMgrSingleUse;
    ThreadedCommandBuilder *commandMgrSingleUseInterface;
    CommandMgr *commandMgrDynamic; // for individual re-recording
} ThreadContext;

// Temporary, for test only
ThreadContext *getContext();

#endif /* CONTEXT_HPP */
