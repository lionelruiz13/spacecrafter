#ifndef SET_MGR_HPP
#define SET_MGR_HPP

#include <vulkan/vulkan.h>

class VirtualSurface;

class SetMgr {
public:
    SetMgr(VirtualSurface *_master, int maxSet, int maxUniformSet = -1, int maxTextureSet = -1);
    ~SetMgr();
    VkDescriptorPool &getDescriptorPool() {return pool;}
private:
    VirtualSurface *master;
    VkDescriptorPool pool;
};

#endif /* end of include guard: SET_MGR_HPP */
