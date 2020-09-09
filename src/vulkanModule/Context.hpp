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

typedef struct GlobalContext {
    Vulkan *vulkan;
    TextureMgr *textureMgr;
    ResourceTracker *tracker;
    PipelineLayout *globalLayout; // for ubo_cam
    Set *globalSet;
} GlobalContext;

typedef struct ThreadContext {
    GlobalContext *global;
    VirtualSurface *surface;
    SetMgr *setMgr;
    CommandMgr *commandMgr;
} ThreadContext;

#endif /* CONTEXT_HPP */
