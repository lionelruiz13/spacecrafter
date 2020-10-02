#include "VirtualSurface.hpp"
#include "TextureMgr.hpp"
#include "Texture.hpp"
#include "PipelineLayout.hpp" // for DEFAULT_SAMPLER

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string Texture::textureDir = "./";

void Texture::init(VirtualSurface *_master, TextureMgr *_mgr, VkFormat _format)
{
    master = _master;
    mgr = _mgr;
    format = _format;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(master->refDevice, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("Faild to create semaphore for texture submission.");
    }
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(master->refDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Faild to create fence.");
    }

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = VK_NULL_HANDLE;
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, std::string filename, bool keepOnCPU, bool multisampling)
{
    init(_master, _mgr);
    int texChannels;
    stbi_uc* pixels = stbi_load((textureDir + filename).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("échec du chargement de l'image!");
    }
    _master->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);

    void *data;
    _master->mapMemory(stagingBufferMemory, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    _master->unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    if (!keepOnCPU) {
        use();
        destroyStagingResources();
    }
}

Texture::Texture(VirtualSurface *_master, TextureMgr *_mgr, void *content, int width, int height, bool keepOnCPU, bool multisampling, VkFormat _format, bool createSampler, VkSamplerAddressMode addressMode) : texWidth(width), texHeight(height)
{
    init(_master, _mgr, _format);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    _master->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_HOST_MEMORY, stagingBuffer, stagingBufferMemory);

    void *data;
    _master->mapMemory(stagingBufferMemory, &data);
    memcpy(data, content, static_cast<size_t>(imageSize));
    _master->unmapMemory(stagingBufferMemory);

    if (createSampler) {
        VkSamplerCreateInfo samplerInfo = PipelineLayout::DEFAULT_SAMPLER;
        samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = addressMode;
        samplerInfo.maxLod = 0; // max mipmap index
        imageInfo.sampler = _mgr->createSampler(samplerInfo);
    }

    if (!keepOnCPU) {
        use();
        destroyStagingResources();
    }
}

void Texture::acquireStagingMemoryPtr(void **pPixels)
{
    master->mapMemory(stagingBufferMemory, pPixels);
}

void Texture::releaseStagingMemoryPtr()
{
    master->unmapMemory(stagingBufferMemory);
}

Texture::~Texture()
{
    destroyStagingResources();
}

void Texture::destroyStagingResources()
{
    if (stagingBuffer) {
        vkDestroySemaphore(master->refDevice, semaphore, nullptr);
        vkDestroyFence(master->refDevice, fence, nullptr);
        vkDestroyBuffer(master->refDevice, stagingBuffer, nullptr);
        master->free(stagingBufferMemory);
        stagingBuffer = VK_NULL_HANDLE;
    }
}

void Texture::use()
{
    if (++useCount != 1)
        return;
    image = std::unique_ptr<TextureImage>(mgr->createImage(std::pair<short, short>(texWidth, texHeight)));
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = master->getTransferPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image->getImage();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

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

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphore;
    master->submitTransfer(&submitInfo);

    allocInfo.commandPool = master->getCommandPool();

    VkCommandBuffer commandBuffer2;
    vkAllocateCommandBuffers(master->refDevice, &allocInfo, &commandBuffer2);
    vkBeginCommandBuffer(commandBuffer2, &beginInfo);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
        commandBuffer2,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    vkEndCommandBuffer(commandBuffer2);

    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stage;
    submitInfo.pCommandBuffers = &commandBuffer2;
    submitInfo.signalSemaphoreCount = 0;
    vkResetFences(master->refDevice, 1, &fence);
    if (vkQueueSubmit(master->getQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
        std::runtime_error("Error : Faild to submit commands.");
    }
    vkWaitForFences(master->refDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    vkFreeCommandBuffers(master->refDevice, master->getTransferPool(), 1, &commandBuffer);
    vkFreeCommandBuffers(master->refDevice, master->getCommandPool(), 1, &commandBuffer2);
    imageInfo.imageView = image->getImageView();
}

void Texture::unuse()
{
    if (--useCount == 0) {
        image = nullptr;
        imageInfo.imageView = VK_NULL_HANDLE;
    }
}
