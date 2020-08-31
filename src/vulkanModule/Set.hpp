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
    Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout);
    ~Set();
    void bindUniform(Uniform *uniform, int binding);
    void bindTexture(Texture *texture, int binding);
    void bindTexture(TextureImage *texture, int binding);
    VkDescriptorSet *get() {return &set;}
private:
    VirtualSurface *master;
    VkDescriptorSet set;
};

#endif /* end of include guard: SET_HPP */
