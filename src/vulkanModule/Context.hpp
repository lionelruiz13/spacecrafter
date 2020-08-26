#ifndef CONTEXT_HPP
#define CONTEXT_HPP

class Vulkan;
class TextureMgr;
class PipelineLayout;

class VirtualSurface;
class UniformSetMgr;

class CommandMgr;

typedef struct GlobalContext {
    Vulkan *vulkan;
    TextureMgr *textureMgr;
    PipelineLayout *globalLayout; // for ubo_cam
} GlobalContext;

typedef struct ThreadContext {
    GlobalContext *global;
    VirtualSurface *surface;
    UniformSetMgr *uniformSetMgr;
} ThreadContext;

typedef struct Context {
    ThreadContext *thread;
    CommandMgr *commandMgr;
} Context;

#endif /* CONTEXT_HPP */
