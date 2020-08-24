#ifndef UNIFORM_SET_HPP
#define UNIFORM_SET_HPP

#include <vulkan/vulkan.h>
#include <vector>
namespace v {

class VirtualSurface;
class PipelineLayout;
class Uniform;

class UniformSetMgr {
public:
    UniformSetMgr(VirtualSurface *_master, int maxUniformBufferSet);
    ~UniformSetMgr();
    VkDescriptorPool &getDescriptorPool() {return pool;}
private:
    VirtualSurface *master;
    VkDescriptorPool pool;
};

class UniformSet {
public:
    UniformSet(VirtualSurface *_master, UniformSetMgr *_mgr, PipelineLayout *_layout);
    void bindUniform(Uniform *uniform, int binding);
    VkDescriptorSet *get() {return &set;}
private:
    VirtualSurface *master;
    VkDescriptorSet set;
};
}

#ifndef OPENGL_HPP
using namespace v;
#endif

#endif /* end of include guard: UNIFORM_SET_HPP */
