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

/**
*   \brief Handle binding for pipelineLayout layout
*   Include uniform, uniform texture and storage buffer
*/
class Set {
public:
    //! create a Set which can be push
    Set();
    //! create a Set which can be bind
    //! Do not bind anything to it while bound to commandBuffer which can be submitted
    Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout, int setBinding = -1);
    ~Set();
    //! Bind uniform to this set
    void bindUniform(Uniform *uniform, uint32_t binding);
    //! Bind texture to this set
    void bindTexture(Texture *texture, uint32_t binding);
    //! Bind storage buffer to this set
    void bindStorageBuffer(Buffer *buffer, uint32_t binding, uint32_t range);
    //! Bind uniform location
    //! @return virtualUniformId used as reference in setVirtualUniform
    int bindVirtualUniform(Uniform *master, uint32_t binding, uint32_t arraySize = 1);
    //! Attach uniform to uniform location, can be used regardless to previous binding to commandBuffer
    void setVirtualUniform(Uniform *uniform, int virtualUniformID);
    //void bindTexture(TextureImage *texture, int binding);
    VkDescriptorSet *get();
    //! Internal use only
    std::vector<uint32_t> &getDynamicOffsets() {return dynamicOffsets;}
    //! Internal use only
    std::vector<VkWriteDescriptorSet> &getWrites() {return writeSet;}
    //! For ThreadedCommandBuilder only
    void swapWrites(std::vector<VkWriteDescriptorSet> &writeSetExt) {writeSet.swap(writeSetExt);}
    //! Remove all previous binding, for push set only
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
