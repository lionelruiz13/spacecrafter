#ifndef CONTEXT_HPP
#define CONTEXT_HPP

class Vulkan;
class TextureMgr;

class PipelineLayout;
class UniformSet;

class VirtualSurface;
class UniformSetMgr;
class CommandMgr;

typedef struct GlobalContext {
    Vulkan *vulkan;
    TextureMgr *textureMgr;
    PipelineLayout *globalLayout; // for ubo_cam
    UniformSet *globalSet;
} GlobalContext;

typedef struct ThreadContext {
    GlobalContext *global;
    VirtualSurface *surface;
    UniformSetMgr *uniformSetMgr;
    CommandMgr *commandMgr;
} ThreadContext;

#endif /* CONTEXT_HPP */
