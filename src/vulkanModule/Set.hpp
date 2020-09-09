#ifndef SET_HPP
#define SET_HPP

#include <vulkan/vulkan.h>

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
    Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout);
    ~Set();
    void bindUniform(Uniform *uniform, uint32_t binding, uint32_t arraySize = 1);
    void bindTexture(Texture *texture, uint32_t binding);
    //void bindTexture(TextureImage *texture, int binding);
    VkDescriptorSet *get();
    std::vector<VkWriteDescriptorSet> &getWrites() {return writeSet;}
    void clear() {writeSet.clear();}
private:
    void update();
    VirtualSurface *master;
    std::vector<VkWriteDescriptorSet> writeSet;
    VkDescriptorSet set;
};

#endif /* end of include guard: SET_HPP */
