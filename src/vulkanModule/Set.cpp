#include "VirtualSurface.hpp"
#include "SetMgr.hpp"
#include "Set.hpp"

#include "PipelineLayout.hpp"
#include "Uniform.hpp"
#include "Texture.hpp"
#include "TextureMgr.hpp"
#include "Buffer.hpp"
#include "tools/log.hpp"

Set::Set() {}

Set::Set(VirtualSurface *_master, SetMgr *_mgr, PipelineLayout *_layout, int setBinding) : master(_master), mgr(_mgr)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _mgr->getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_layout->getDescriptorLayout(setBinding);
    if (*allocInfo.pSetLayouts == VK_NULL_HANDLE) {
        cLog::get()->write("Can't create Set from invalid Layout", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
        set = VK_NULL_HANDLE;
        return;
    }
    if (!createDescriptorSet(&allocInfo)) {
        // retry (because it could have been solved)
        if (!createDescriptorSet(&allocInfo)) {
            cLog::get()->write("Failed to create Set from this SetMgr", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
            set = VK_NULL_HANDLE;
        }
    }
}

Set::~Set() {}

bool Set::createDescriptorSet(VkDescriptorSetAllocateInfo *allocInfo)
{
    switch (vkAllocateDescriptorSets(master->refDevice, allocInfo, &set)) {
        case VK_SUCCESS:
            return true;
        case VK_ERROR_FRAGMENTED_POOL:
            cLog::get()->write("Not enough continuous space in SetMgr to allocate this Set", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            cLog::get()->write("Not enough space in SetMgr to allocate this Set", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            cLog::get()->write("Not enough memory for Set", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
            return false;
        default:
            cLog::get()->write("Invalid Set", LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
            set = VK_NULL_HANDLE;
            return true;
    }
    mgr->extend();
    allocInfo->descriptorPool = mgr->getDescriptorPool();
    return false;
}

void Set::bindUniform(Uniform *uniform, uint32_t binding)
{
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, 1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr, uniform->getBufferInfo(), nullptr});
}

void Set::bindTexture(Texture *texture, uint32_t binding)
{
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, 1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        texture->getInfo(), nullptr, nullptr});
}

void Set::bindStorageBuffer(Buffer *buffer, uint32_t binding, uint32_t range)
{
    VkDescriptorBufferInfo buffInfo{};
    buffInfo.buffer = buffer->get();
    buffInfo.offset = buffer->getOffset();
    buffInfo.range = range;
    storageBufferInfo.push_front(buffInfo);
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, 1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        nullptr, &storageBufferInfo.front(), nullptr});
}

int Set::bindVirtualUniform(Uniform *uniform, uint32_t binding, uint32_t arraySize)
{
    dynamicOffsets.push_back(uniform->getOffset());
    writeSet.emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0, arraySize,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        nullptr, uniform->getBufferInfo(), nullptr});
    return dynamicOffsets.size() - 1;
}

void Set::setVirtualUniform(Uniform *uniform, int virtualUniformID)
{
    dynamicOffsets[virtualUniformID] = uniform->getOffset();
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
    if (set) {
        vkUpdateDescriptorSets(master->refDevice, writeSet.size(), writeSet.data(), 0, nullptr);
        writeSet.clear();
        storageBufferInfo.clear();
    }
}

VkDescriptorSet *Set::get()
{
    if (!writeSet.empty())
        update();
    return &set;
}
