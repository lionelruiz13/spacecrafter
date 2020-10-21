#ifndef SET_HPP
#define SET_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <forward_list>

class VirtualSurface;
class SetMgr;
class PipelineLayout;

class Uniform;
class Texture;
class TextureImage;
class Buffer;

class Set {
public:
    //! create a Set which can be push
    Set();
    //! create a Set which can be bind
    Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout, int setBinding = -1);
    ~Set();
    void bindUniform(Uniform *uniform, uint32_t binding);
    void bindTexture(Texture *texture, uint32_t binding);
    void bindStorageBuffer(Buffer *buffer, uint32_t binding, uint32_t range);
    //! Bind uniform location
    //! @return virtualUniformId to bind uniform to this location
    int bindVirtualUniform(Uniform *master, uint32_t binding, uint32_t arraySize = 1);
    //! Attach uniform to uniform location, doesn't affect set bindings to commandMgr before this call
    void setVirtualUniform(Uniform *uniform, int virtualUniformID);
    //void bindTexture(TextureImage *texture, int binding);
    VkDescriptorSet *get();
    std::vector<uint32_t> &getDynamicOffsets() {return dynamicOffsets;}
    std::vector<VkWriteDescriptorSet> &getWrites() {return writeSet;}
    void clear() {writeSet.clear();}
    //! Manually update bindings
    void update();
private:
    // create descriptorSet, return true on success
    bool createDescriptorSet(VkDescriptorSetAllocateInfo *allocInfo);
    VirtualSurface *master;
    SetMgr *mgr;
    std::forward_list<VkDescriptorBufferInfo> storageBufferInfo;
    std::vector<VkWriteDescriptorSet> writeSet;
    std::vector<uint32_t> dynamicOffsets;
    VkDescriptorSet set;
};

#endif /* end of include guard: SET_HPP */
