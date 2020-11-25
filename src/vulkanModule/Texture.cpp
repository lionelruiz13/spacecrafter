#include "VirtualSurface.hpp"
#include "TextureMgr.hpp"
#include "Texture.hpp"
#include "PipelineLayout.hpp" // for DEFAULT_SAMPLER
#include "CommandMgr.hpp" // for mipmap
#include "tools/log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string Texture::textureDir = "./";

Texture::Texture(){}

void Texture::init(VirtualSurface *_master, TextureMgr *_mgr, bool _mipmap, VkFormat _format)
{
    master = _master;
    mgr = _mgr;
    mipmap = _mipmap;
    format = _format;
    image = nullptr;

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = VK_NULL_HANDLE;
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, const std::string &filename, bool keepOnCPU, bool _mipmap, bool createSampler, VkFormat _format, int nbChannels, bool is3d)
{
    init(_master, _mgr, _mipmap, _format);
    int texChannels;
    stbi_uc* pixels = stbi_load((textureDir + filename).c_str(), &texWidth, &texHeight, &texChannels, nbChannels);
    if (!pixels) {
        cLog::get()->write("Faild to load image '" + filename + "'", LOG_TYPE::L_WARNING);
        isOk = false;
        return;
    }
    VkDeviceSize imageSize = texWidth * texHeight * nbChannels;
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = texWidth;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    if (is3d) {
        uint32_t height = texHeight;
        texWidth = texHeight = texDepth = 1 << (static_cast<uint32_t>(std::log2(texHeight))*2/3);
        region.imageExtent = {(uint32_t) texWidth, (uint32_t) texHeight, height/texHeight};
        while (region.imageOffset.z < texDepth) {
            regions.push_back(region);
            region.bufferOffset += texWidth;
            region.imageOffset.z += region.imageExtent.depth;
        }
    } else {
        region.imageExtent = {(uint32_t) texWidth, (uint32_t) texHeight, 1};
        regions.push_back(region);
    }
    if (mipmap)
        mipmapCount = static_cast<uint32_t>(std::log2(std::max(texWidth, texHeight))) + 1;
    _master->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);

    void *data;
    _master->mapMemory(stagingBufferMemory, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    _master->unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    if (createSampler) {
        VkSamplerCreateInfo samplerInfo = PipelineLayout::DEFAULT_SAMPLER;
        samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.maxLod = mipmapCount;
        imageInfo.sampler = _mgr->createSampler(samplerInfo);
    }

    imageName = filename;
    if (!keepOnCPU) {
        use();
        destroyStagingResources();
    } else {
        _master->setObjectName(stagingBuffer, VK_OBJECT_TYPE_BUFFER, "staging buffer of texture " + imageName);
    }
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU, bool _mipmap, VkFormat _format, const std::string &name, bool createSampler, VkSamplerAddressMode addressMode) : texWidth(width), texHeight(height)
{
    init(_master, _mgr, _mipmap, _format);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (mipmap)
        mipmapCount = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    _master->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);

    void *data;
    _master->mapMemory(stagingBufferMemory, &data);
    memcpy(data, content, static_cast<size_t>(imageSize));
    _master->unmapMemory(stagingBufferMemory);

    if (createSampler) {
        VkSamplerCreateInfo samplerInfo = PipelineLayout::DEFAULT_SAMPLER;
        samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = addressMode;
        samplerInfo.maxLod = mipmapCount;
        imageInfo.sampler = _mgr->createSampler(samplerInfo);
    }

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {(uint32_t) texWidth, (uint32_t) texHeight, 1};
    regions.push_back(region);

    imageName = name;
    if (!keepOnCPU) {
        use();
        destroyStagingResources();
    } else {
        _master->setObjectName(stagingBuffer, VK_OBJECT_TYPE_BUFFER, "staging buffer of texture " + imageName);
    }
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, bool isDepthAttachment, int width, int height) : master(_master), mgr(_mgr)
{
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = VK_NULL_HANDLE;

    texWidth = (width == -1) ? _master->getViewportState().pViewports->width : width;
    texHeight = (height == -1) ? abs(_master->getViewportState().pViewports->height) : height;
    if (isDepthAttachment) {
        image = std::unique_ptr<TextureImage>(mgr->createImage(std::pair<short, short>(texWidth, texHeight), 1, false, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, true));
        master->setObjectName(image->getImage(), VK_OBJECT_TYPE_IMAGE, "FBO depth attachment");
    } else {
        image = std::unique_ptr<TextureImage>(mgr->createImage(std::pair<short, short>(texWidth, texHeight), 1, false, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        master->setObjectName(image->getImage(), VK_OBJECT_TYPE_IMAGE, "FBO color attachment");
    }
    if (image == nullptr) {
        cLog::get()->write("Faild to create external image attachment", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
    } else {
        imageInfo.imageView = image->getImageView();
    }
    imageName = "FrameBuffer attachment of VirtualSurface";
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, const std::string &filename, int width, int height, const std::string &name, VkSamplerAddressMode addressMode, VkFormat _format, int nbChannels)
{
    init(_master, _mgr, true, _format);
    int texChannels;
    stbi_uc* pixels = stbi_load((textureDir + filename).c_str(), &texWidth, &texHeight, &texChannels, nbChannels);
    assert(texWidth % width == 0 && texHeight % height == 0);
    VkDeviceSize imageSize = texWidth * texHeight * nbChannels;
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = texWidth;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {(uint32_t) width, (uint32_t) height, (uint32_t) texHeight/height};
    texDepth = texWidth * texHeight / (width * height);
    while (region.bufferOffset < (uint32_t) texWidth) {
        regions.push_back(region);
        region.bufferOffset += width;
        region.imageOffset.z += region.imageExtent.depth;
    }
    texWidth = width;
    texHeight = height;
    mipmapCount = static_cast<uint32_t>(std::log2(std::max(std::max(texWidth, texHeight), texDepth))) + 1;

    if (!pixels) {
        cLog::get()->write("Faild to load image '" + filename + "'", LOG_TYPE::L_WARNING);
        isOk = false;
        return;
    }
    _master->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);

    void *data;
    _master->mapMemory(stagingBufferMemory, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    _master->unmapMemory(stagingBufferMemory);
    stbi_image_free(pixels);
    VkSamplerCreateInfo samplerInfo = PipelineLayout::DEFAULT_SAMPLER;
    samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxLod = mipmapCount;
    imageInfo.sampler = _mgr->createSampler(samplerInfo);
    imageName = filename;
    use();
    destroyStagingResources();
}

void Texture::acquireStagingMemoryPtr(void **pPixels)
{
    master->mapMemory(stagingBufferMemory, pPixels);
}

void Texture::releaseStagingMemoryPtr()
{
    master->unmapMemory(stagingBufferMemory);
}

VkImage Texture::getImage()
{
    return image->getImage();
}

Texture::~Texture()
{
    if (imagePtr)
        image = mgr->queryImage(imagePtr);
    image = nullptr;
    if (useCount > 0) {
        useCount = 1;
        unuse();
    }
    destroyStagingResources();
}

void Texture::destroyStagingResources()
{
    if (stagingBuffer) {
        if (useCount > 0)
            master->waitTransferQueueIdle();
        else
            isOk = false;
        vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
        master->free(stagingBufferMemory);
        stagingBuffer = VK_NULL_HANDLE;
        regions.clear();
    }
}

bool Texture::use(bool forceUpdate)
{
    if (++useCount != 1)
        return true;
    if (imagePtr) {
        image = mgr->queryImage(imagePtr);
        imagePtr = nullptr;
        if (image && !forceUpdate)
            return true;
    }
    if (image == nullptr)
        image = std::unique_ptr<TextureImage>(mgr->createImage(std::pair<short, short>(texWidth, texHeight), texDepth, mipmap, format));
    if (image == nullptr) {
        cLog::get()->write("Faild to create image support", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        useCount--;
        return false;
    }
    CommandMgr *cmdMgr = mgr->getBuilder();
    VkFence fence;
    VkSemaphore semaphore;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = master->getTransferPool();
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    cmdMgr->grab(commandBuffer);
    cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
    cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
    if (!mipmap) {
        cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, 0);
        cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    if (mipmap) {
        mgr->acquireSyncObject(&fence, &semaphore);
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &semaphore;
    }
    master->submitTransfer(&submitInfo);

    if (mipmap) {
        allocInfo.commandPool = master->getCommandPool();
        VkCommandBuffer commandBuffer2;
        vkAllocateCommandBuffers(master->refDevice, &allocInfo, &commandBuffer2);
        vkBeginCommandBuffer(commandBuffer2, &beginInfo);
        cmdMgr->grab(commandBuffer2);

        for (int i = 0; i < mipmapCount - 1; ++i) {
            cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, i);
            cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
            cmdMgr->blit(this, this, i, i + 1);
        }
        cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, 0, mipmapCount - 1);
        cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, mipmapCount - 1);
        cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        vkEndCommandBuffer(commandBuffer2);
        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &semaphore;
        submitInfo.pWaitDstStageMask = &stage;
        submitInfo.pCommandBuffers = &commandBuffer2;
        submitInfo.signalSemaphoreCount = 0;
        vkResetFences(master->refDevice, 1, &fence);
        master->waitTransferQueueIdle(false);
        if (vkQueueSubmit(master->getQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
            throw std::runtime_error("Error : Failed to submit commands.");
        }
        vkWaitForFences(master->refDevice, 1, &fence, VK_TRUE, UINT64_MAX);
        mgr->releaseSyncObject(fence, semaphore);
        vkFreeCommandBuffers(master->refDevice, master->getCommandPool(), 1, &commandBuffer2);
    }
    imageInfo.imageView = image->getImageView();
    if (!imageName.empty()) {
        master->setObjectName(image->getImage(), VK_OBJECT_TYPE_IMAGE, imageName);
        master->setObjectName(image->getImageView(), VK_OBJECT_TYPE_IMAGE_VIEW, imageName);
    }
    return true;
}

void Texture::unuse()
{
    if (--useCount == 0) {
        mgr->cacheImage(image);
        imageInfo.imageView = VK_NULL_HANDLE;
        vkFreeCommandBuffers(master->refDevice, master->getTransferPool(), 1, &commandBuffer);
    }
    if (useCount < 0) useCount = 0;
}

StreamTexture::StreamTexture(VirtualSurface *_master, TextureMgr *_mgr, bool externUpdate)
{
    master = _master;
    mgr = _mgr;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    if (vkCreateFence(master->refDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence.");
    }
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = VK_NULL_HANDLE;
    updateSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    updateSubmitInfo.pNext = nullptr;
    updateSubmitInfo.signalSemaphoreCount = updateSubmitInfo.waitSemaphoreCount = updateSubmitInfo.commandBufferCount = 0;
}

StreamTexture::~StreamTexture()
{
    if (useCount > 0) {
        useCount = 1;
        unuse();
    }
    vkDestroyFence(master->refDevice, fence, nullptr);
}

bool StreamTexture::use(int width, int height, VkFormat format)
{
    if (++useCount != 1) {
        assert(width == texWidth && height == texHeight);
        return true;
    }
    texWidth = width;
    texHeight = height;
    image = std::unique_ptr<TextureImage>(mgr->createImage(std::pair<short, short>(texWidth, texHeight), 1, false, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, false, true));
    if (image == nullptr || !master->createBuffer(texWidth * texHeight, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory)) {
        useCount--;
        return false;
    }
    master->setObjectName(stagingBuffer, VK_OBJECT_TYPE_BUFFER, "staging buffer of Stream texture");

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = master->getTransferPool();
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    CommandMgr *cmdMgr = mgr->getBuilder();
    cmdMgr->grab(cmdBuffer);

    cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
    cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {(uint32_t) texWidth, (uint32_t) texHeight, 1};
    vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    cmdMgr->addImageBarrier(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, 0);
    cmdMgr->compileBarriers(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    vkEndCommandBuffer(cmdBuffer);

    updateSubmitInfo.commandBufferCount = 1;
    updateSubmitInfo.pCommandBuffers = &cmdBuffer;
    imageInfo.imageView = image->getImageView();
    master->setObjectName(image->getImage(), VK_OBJECT_TYPE_IMAGE, "Stream texture");
    master->setObjectName(image->getImageView(), VK_OBJECT_TYPE_IMAGE_VIEW, "Stream texture");
    return true;
}

void StreamTexture::unuse()
{
    if (--useCount == 0) {
        image = nullptr;
        imageInfo.imageView = VK_NULL_HANDLE;
        master->waitTransferQueueIdle();
        vkFreeCommandBuffers(master->refDevice, master->getTransferPool(), 1, &cmdBuffer);
        vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
        master->free(stagingBufferMemory);
        stagingBuffer = VK_NULL_HANDLE;
    }
    if (useCount < 0) useCount = 0;
}

void StreamTexture::update()
{
    master->submitTransfer(&updateSubmitInfo);
}
