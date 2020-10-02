#ifndef SET_HPP
#define SET_HPP

#include <vulkan/vulkan.h>
#include <vector>

class VirtualSurface;
class SetMgr;
class PipelineLayout;

class Uniform;
class Texture;
class TextureImage;

class Set {
public:
    //! create a Set which can be push
    Set();
    //! create a Set which can be bind
    Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout, int setBinding = -1);
    ~Set();
    void bindUniform(Uniform *uniform, uint32_t binding, uint32_t arraySize = 1);
    void bindTexture(Texture *texture, uint32_t binding);
    //void bindTexture(TextureImage *texture, int binding);
    VkDescriptorSet *get();
    std::vector<VkWriteDescriptorSet> &getWrites() {return writeSet;}
    void clear() {writeSet.clear();}
    //! Manually update bindings
    void update();
private:
    // create descriptorSet, return true on success
    bool createDescriptorSet(VkDescriptorSetAllocateInfo *allocInfo);
    VirtualSurface *master;
    SetMgr *mgr;
    std::vector<VkWriteDescriptorSet> writeSet;
    VkDescriptorSet set;
};

#endif /* end of include guard: SET_HPP */
