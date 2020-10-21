#ifndef SET_MGR_HPP
#define SET_MGR_HPP

#include <vulkan/vulkan.h>

class VirtualSurface;

class SetMgr {
public:
    SetMgr(VirtualSurface *_master, int maxSet, int maxUniformSet = -1, int maxTextureSet = -1, int maxStorageBufferSet = 0);
    ~SetMgr();
    void extend();
    VkDescriptorPool &getDescriptorPool() {return pools.back();}
private:
    VirtualSurface *master;
    std::vector<VkDescriptorPool> pools;
};

#endif /* end of include guard: SET_MGR_HPP */
