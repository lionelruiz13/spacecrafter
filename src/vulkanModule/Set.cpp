#include "VirtualSurface.hpp"
#include "SetMgr.hpp"
#include "Set.hpp"

#include "PipelineLayout.hpp"
#include "Uniform.hpp"
#include "Texture.hpp"
#include "TextureMgr.hpp"

Set::Set() {}

Set::Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout) : master(_master)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _mgr->getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_layout->getDescriptorLayout();

    if (vkAllocateDescriptorSets(_master->refDevice, &allocInfo, &set) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
    }
}

Set::~Set() {}

void Set::bindUniform(Uniform *uniform, uint32_t binding, uint32_t arraySize)
{
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, arraySize,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr, uniform->getBufferInfo(), nullptr});
}

void Set::bindTexture(Texture *texture, uint32_t binding)
{
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, 1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        texture->getInfo(), nullptr, nullptr});
}

/*
void Set::bindTexture(TextureImage *texture, int binding)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->getImageView();
    imageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(master->refDevice, 1, &descriptorWrite, 0, nullptr);
}
*/

void Set::update()
{
    vkUpdateDescriptorSets(master->refDevice, writeSet.size(), writeSet.data(), 0, nullptr);
    writeSet.clear();
}

VkDescriptorSet *Set::get()
{
    if (!writeSet.empty())
        update();
    return &set;
}
