#include "Vulkan.hpp"
#include "VirtualSurface.hpp"
#include "VertexBuffer.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "CommandMgr.hpp"
#include "Set.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "tools/log.hpp"
#include <algorithm> // std::find
#include <iterator> // std::distance

VkPipelineStageFlags CommandMgr::defaultStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
PFN_vkCmdPushDescriptorSetKHR CommandMgr::PFN_pushSet;
PFN_vkCmdBeginConditionalRenderingEXT CommandMgr::PFN_vkIf;
PFN_vkCmdEndConditionalRenderingEXT CommandMgr::PFN_vkEndIf;

CommandMgr::CommandMgr(Vulkan *_master) : master(nullptr), refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(_master->refSwapChainFramebuffers), refResolveFramebuffers(_master->refResolveFramebuffers), refSingleSampleFramebuffers(_master->refSingleSampleFramebuffers), refFrameIndex(_master->refFrameIndex), singleUse(true), submissionPerFrame(true), nbCommandBuffers(0) {}

CommandMgr::CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool submissionPerFrame, bool singleUseCommands, bool isExternal, bool enableIndividualReset) : master(_master), refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(_master->refSwapChainFramebuffers), refResolveFramebuffers(_master->refResolveFramebuffers), refSingleSampleFramebuffers(_master->refSingleSampleFramebuffers), refFrameIndex(_master->refFrameIndex), singleUse(singleUseCommands), submissionPerFrame(submissionPerFrame), nbCommandBuffers(nbCommandBuffers)
{
    if (!isExternal)
        _master->registerCommandMgr(this);
    queue = _master->getQueue();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = singleUseCommands ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : (enableIndividualReset ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0);
    poolInfo.queueFamilyIndex = _master->getGraphicsQueueIndex();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) nbCommandBuffers;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (!singleUse && nbCommandBuffers > 0) {
        if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
            throw std::runtime_error("échec de la création d'une command pool !");
        }
        allocInfo.commandPool = cmdPool;
    }
    frames.resize(refSwapChainFramebuffers.size());
    for (auto &frame : frames) {
        if (vkCreateSemaphore(refDevice, &semaphoreInfo, nullptr, &frame.bottomSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphore for previously allocated command buffers.");
        }
        if (vkCreateFence(refDevice, &fenceInfo, nullptr, &frame.fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence.");
        }
        frame.actual = VK_NULL_HANDLE;
        if (nbCommandBuffers == 0) {
            // This commandMgr don't manage commandBuffer internally
            frame.cmdPool = VK_NULL_HANDLE;
            continue;
        }
        if (singleUse) {
            if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &frame.cmdPool) != VK_SUCCESS) {
                throw std::runtime_error("échec de la création d'une command pool !");
            }
            allocInfo.commandPool = frame.cmdPool;
        }
        frame.commandBuffers.resize(nbCommandBuffers);
        if (vkAllocateCommandBuffers(refDevice, &allocInfo, frame.commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("échec de l'allocation de command buffers!");
        }
        frame.signalSemaphores.resize(nbCommandBuffers);
        auto tmp = frame.signalSemaphores.data();
        for (int i = 0; i < nbCommandBuffers; i++) {
            if (vkCreateSemaphore(refDevice, &semaphoreInfo, nullptr, tmp++) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create semaphore for previously allocated command buffers.");
            }
        }
    }
}

CommandMgr::~CommandMgr()
{
    vkDeviceWaitIdle(refDevice);
    for (auto &frame : frames) {
        vkDestroySemaphore(refDevice, frame.bottomSemaphore, nullptr);
        for (auto &sem : frame.signalSemaphores)
            vkDestroySemaphore(refDevice, sem, nullptr);
        vkWaitForFences(refDevice, 1, &frame.fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(refDevice, frame.fence, nullptr);
        if (singleUse && frame.cmdPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(refDevice, frame.cmdPool, nullptr);
    }
    if (!singleUse && cmdPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(refDevice, cmdPool, nullptr);
}

void CommandMgr::setSubmission(int index, bool newBatch, CommandMgr *target)
{
    if (target == nullptr) target = this;
    // Create new batch
    if (newBatch || !target->inBatch) {
        target->inBatch = true;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = stages;
        submitInfo.signalSemaphoreCount = 1;
        if (target->submissionPerFrame) {
            auto &frame = target->frames[target->refFrameIndex];
            auto &srcFrame = frames[refFrameIndex];
            // set dependency to previous command
            submitInfo.pWaitSemaphores = frame.submittedSignalSemaphores.empty() ? &frame.topSemaphore : &frame.submittedSignalSemaphores.back();
            // add dependency for next command
            frame.submittedSignalSemaphores.push_back(srcFrame.signalSemaphores[index]);
            submitInfo.pSignalSemaphores = &frame.submittedSignalSemaphores.back();
            // add commandBuffer storage
            frame.submittedCommandBuffers.emplace_back();
            // append submission state
            frame.submitList.push_back(submitInfo);
        } else {
            for (auto &frame : target->frames) {
                // set dependency to previous command
                submitInfo.pWaitSemaphores = frame.submittedSignalSemaphores.empty() ? &frame.topSemaphore : &frame.submittedSignalSemaphores.back();
                // add dependency for next command
                frame.submittedSignalSemaphores.push_back(frame.signalSemaphores[index]);
                submitInfo.pSignalSemaphores = &frame.submittedSignalSemaphores.back();
                // add commandBuffer storage
                frame.submittedCommandBuffers.emplace_back();
                // append submission state
                frame.submitList.push_back(submitInfo);
            }
        }
    }
    // Add command to the last batch
    if (target->submissionPerFrame) {
        auto &frame = target->frames[target->refFrameIndex];
        auto &srcFrame = frames[refFrameIndex];
        // add commandBuffer to the last batch
        frame.submittedCommandBuffers.back().push_back(srcFrame.commandBuffers[index]);
        // apply changes to vkSubmitInfo
        frame.submitList.back().commandBufferCount = frame.submittedCommandBuffers.back().size();
        frame.submitList.back().pCommandBuffers = frame.submittedCommandBuffers.back().data();
    } else {
        for (auto &frame : target->frames) {
            // add commandBuffer to the last batch
            frame.submittedCommandBuffers.back().push_back(frame.commandBuffers[index]);
            // apply changes to vkSubmitInfo
            frame.submitList.back().commandBufferCount = frame.submittedCommandBuffers.back().size();
            frame.submitList.back().pCommandBuffers = frame.submittedCommandBuffers.back().data();
        }
    }
}

void CommandMgr::resetSubmission()
{
    for (auto &frame : frames) {
        frame.submittedCommandBuffers.clear();
        frame.submittedSignalSemaphores.clear();
        frame.submitList.clear();
        needResolve = true;
    }
}

void CommandMgr::submitGuard()
{
    inBatch = false;
    vkWaitForFences(refDevice, 1, &frames[frameIndex].fence, VK_TRUE, UINT64_MAX);
    frameIndex = refFrameIndex;
    vkWaitForFences(refDevice, 1, &frames[frameIndex].fence, VK_TRUE, UINT64_MAX);
    vkResetFences(refDevice, 1, &frames[frameIndex].fence);
}

void CommandMgr::submitAction()
{
    if (needResolve) {
        if (submissionPerFrame) {
            resolve(frameIndex);
        } else {
            for (uint8_t frameIndex = 0; frameIndex < frames.size(); frameIndex++)
                resolve(frameIndex);
        }
        needResolve = false;
    }
    for (uint8_t i = 0; i < frames[frameIndex].submitList.size() - 1; ++i)
        master->submitGraphic(frames[frameIndex].submitList[i]);
    if (vkQueueSubmit(queue, 1, &frames[frameIndex].submitList.back(), frames[frameIndex].fence) != VK_SUCCESS) {
        cLog::get()->write("Command submission from CommandMgr has failed (see vulkan-layers with debug=true)", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
    }
    if (submissionPerFrame) {
        frames[frameIndex].submittedCommandBuffers.clear();
        frames[frameIndex].submittedSignalSemaphores.clear();
        frames[frameIndex].submitList.clear();
        needResolve = true;
    }
}

void CommandMgr::submit()
{
    submitGuard();
    submitAction();
}

void CommandMgr::resolve(uint8_t frameIndex)
{
    if (isLinked) {
        if (frames[frameIndex].submitList.empty()) {
            // If there is no commands, link topSemaphore to bottomSemaphore
            VkSubmitInfo submitInfo;
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &frames[frameIndex].topSemaphore;
            submitInfo.pWaitDstStageMask = &defaultStage;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &frames[frameIndex].bottomSemaphore;
            submitInfo.commandBufferCount = 0;
            submitInfo.pCommandBuffers = nullptr;
            frames[frameIndex].submitList.push_back(submitInfo);
        } else {
            // // MERGE TEST
            // VkSubmitInfo submitInfo;
            // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            // submitInfo.pNext = nullptr;
            // submitInfo.waitSemaphoreCount = 1;
            // submitInfo.pWaitSemaphores = &frames[frameIndex].topSemaphore;
            // submitInfo.pWaitDstStageMask = &defaultStage;
            // submitInfo.signalSemaphoreCount = 1;
            // submitInfo.pSignalSemaphores = &frames[frameIndex].bottomSemaphore;
            // std::vector<VkCommandBuffer> pCmdBuff;
            // pCmdBuff.reserve(frames[frameIndex].submittedCommandBuffers.size());
            // for (auto &vecCmdBuff : frames[frameIndex].submittedCommandBuffers) {
            //     for (auto &cmdBuff : vecCmdBuff) {
            //         pCmdBuff.push_back(cmdBuff);
            //     }
            // }
            // frames[frameIndex].submittedCommandBuffers.front().swap(pCmdBuff);
            // submitInfo.commandBufferCount = frames[frameIndex].submittedCommandBuffers.front().size();
            // submitInfo.pCommandBuffers = frames[frameIndex].submittedCommandBuffers.front().data();
            // frames[frameIndex].submitList.clear();
            // frames[frameIndex].submitList.push_back(submitInfo);
            // // MERGE TEST END
            frames[frameIndex].submitList.back().pSignalSemaphores = &frames[frameIndex].bottomSemaphore;
        }
    } else {
        frames[frameIndex].submitList.front().waitSemaphoreCount = 0;
        frames[frameIndex].submitList.back().signalSemaphoreCount = 0;
    }
}

void CommandMgr::init(int index, bool compileSelected)
{
    if (compileSelected && isRecording()) {
        int actualIndex = std::distance(frames[refFrameIndex].commandBuffers.begin(),
            std::find(frames[refFrameIndex].commandBuffers.begin(),
                frames[refFrameIndex].commandBuffers.end(),
                (singleUse) ? actual : frames[refFrameIndex].actual));
        cLog::get()->write("Implicit compilation of commandBuffer at index " + ((actualIndex < nbCommandBuffers) ? std::to_string(actualIndex) : "<external>"), LOG_TYPE::L_WARNING, LOG_FILE::VULKAN);
        compile();
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (singleUse) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        actual = frames[refFrameIndex].commandBuffers[index];
        if (vkBeginCommandBuffer(actual, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("erreur au début de l'enregistrement d'un command buffer!");
        }
        return;
    }
    beginInfo.flags = 0;
    for (auto &frame : frames) {
        frame.actual = frame.commandBuffers[index];
        if (vkBeginCommandBuffer(frame.actual, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("erreur au début de l'enregistrement d'un command buffer!");
        }
    }
}

void CommandMgr::init(int index, Pipeline *pipeline, renderPassType renderPassType, bool compileSelected, renderPassCompatibility compatibility)
{
    init(index, compileSelected);
    beginRenderPass(renderPassType, compatibility);
    bindPipeline(pipeline);
}

int CommandMgr::initNew(Pipeline *pipeline, renderPassType renderPassType, bool compileSelected, renderPassCompatibility compatibility)
{
    int index = getCommandIndex();

    init(index, pipeline, renderPassType, compileSelected, compatibility);
    return index;
}

void CommandMgr::select(int index)
{
    // assume in render pass with valid pipeline bound
    inRenderPass = true;
    hasPipeline = true;
    if (singleUse) {
        actual = frames[refFrameIndex].commandBuffers[index];
        return;
    }
    for (auto &frame : frames) {
        frame.actual = frame.commandBuffers[index];
    }
}

void CommandMgr::grab(VkCommandBuffer buffer)
{
    assert(singleUse);
    // assume in render pass with valid pipeline bound, allow use of graphic commands
    inRenderPass = true;
    hasPipeline = true;
    actual = buffer;
}

void CommandMgr::compile()
{
    if (inRenderPass)
        endRenderPass();
    if (singleUse) {
        vkEndCommandBuffer(actual);
        actual = VK_NULL_HANDLE;
        return;
    }
    for (auto &frame : frames) {
        vkEndCommandBuffer(frame.actual);
        frame.actual = VK_NULL_HANDLE;
    }
}

void CommandMgr::reset()
{
    vkWaitForFences(refDevice, 1, &frames[refFrameIndex].fence, VK_TRUE, UINT64_MAX);
    vkResetCommandPool(refDevice, frames[refFrameIndex].cmdPool, 0);
}

void CommandMgr::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (!hasPipeline) return;
    if (singleUse) {
        vkCmdDraw(actual, vertexCount, instanceCount, firstVertex, firstInstance);
        return;
    }
    for (auto &frame : frames) {
        vkCmdDraw(frame.actual, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

void CommandMgr::indirectDraw(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount)
{
    if (!hasPipeline) return;
    offset += drawArgsArray->getOffset();
    if (singleUse) {
        vkCmdDrawIndirect(actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndirectCommand));
        return;
    }
    for (auto &frame : frames) {
        vkCmdDrawIndirect(frame.actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndirectCommand));
    }
}

void CommandMgr::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    if (!hasPipeline) return;
    if (singleUse) {
        vkCmdDrawIndexed(actual, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        return;
    }
    for (auto &frame : frames) {
        vkCmdDrawIndexed(frame.actual, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

void CommandMgr::indirectDrawIndexed(Buffer *drawArgsArray, VkDeviceSize offset, uint32_t drawCount)
{
    if (!hasPipeline) return;
    offset += drawArgsArray->getOffset();
    if (singleUse) {
        vkCmdDrawIndexedIndirect(actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndexedIndirectCommand));
        return;
    }
    for (auto &frame : frames) {
        vkCmdDrawIndexedIndirect(frame.actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }
}

void CommandMgr::beginRenderPass(renderPassType renderPassType, renderPassCompatibility compatibility)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = refRenderPass[static_cast<uint8_t>(renderPassType)];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = master->swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.f, 0.f, 0.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    inRenderPass = true;
    if (singleUse) {
        switch (compatibility) {
            case renderPassCompatibility::DEFAULT:
                renderPassInfo.framebuffer = refSwapChainFramebuffers[refFrameIndex];
                break;
            case renderPassCompatibility::RESOLVE:
                renderPassInfo.framebuffer = refResolveFramebuffers[refFrameIndex];
                break;
            case renderPassCompatibility::SINGLE_SAMPLE:
                renderPassInfo.framebuffer = refSingleSampleFramebuffers[refFrameIndex];
                break;
        }
        vkCmdBeginRenderPass(actual, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return;
    }
    for (uint32_t i = 0; i < frames.size(); i++) {
        switch (compatibility) {
            case renderPassCompatibility::DEFAULT:
                renderPassInfo.framebuffer = refSwapChainFramebuffers[i];
                break;
            case renderPassCompatibility::RESOLVE:
                renderPassInfo.framebuffer = refResolveFramebuffers[i];
                break;
            case renderPassCompatibility::SINGLE_SAMPLE:
                renderPassInfo.framebuffer = refSingleSampleFramebuffers[i];
                break;
        }
        vkCmdBeginRenderPass(frames[i].actual, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
}

void CommandMgr::updateVertex(VertexArray *vertex)
{
    if (singleUse) {
        vertex->update(actual);
        return;
    }
    for (auto &frame : frames) {
        vertex->update(frame.actual);
    }
}

void CommandMgr::updateVertex(VertexBuffer *vertex)
{
    if (singleUse) {
        vertex->update(actual);
        return;
    }
    for (auto &frame : frames) {
        vertex->update(frame.actual);
    }
}

void CommandMgr::bindVertex(VertexArray *vertex)
{
    vertex->bind(this);
}

void CommandMgr::bindVertex(VertexBuffer *vertex, uint32_t firstBinding, VkDeviceSize offset)
{
    if (!hasPipeline) return;
    if (singleUse) {
        vkCmdBindVertexBuffers(actual, firstBinding, 1, &vertex->get(), &offset);
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindVertexBuffers(frame.actual, firstBinding, 1, &vertex->get(), &offset);
    }
}

void CommandMgr::bindIndex(Buffer *buffer, VkIndexType indexType, VkDeviceSize offset)
{
    if (!hasPipeline) return;
    offset += buffer->getOffset();
    if (singleUse) {
        vkCmdBindIndexBuffer(actual, buffer->get(), offset, indexType);
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindIndexBuffer(frame.actual, buffer->get(), offset, indexType);
    }
}

void CommandMgr::bindPipeline(Pipeline *pipeline)
{
    if (!inRenderPass || pipeline->get() == VK_NULL_HANDLE) {
        hasPipeline = false;
        return;
    }
    hasPipeline = true;
    if (singleUse) {
        vkCmdBindPipeline(actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindPipeline(frame.actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());
    }
}

void CommandMgr::bindSet(PipelineLayout *pipelineLayout, Set *uniform, int binding)
{
    if (!hasPipeline) return;
    if (singleUse) {
        vkCmdBindDescriptorSets(actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), binding, 1, uniform->get(), uniform->getDynamicOffsets().size(), uniform->getDynamicOffsets().data());
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindDescriptorSets(frame.actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), binding, 1, uniform->get(), uniform->getDynamicOffsets().size(), uniform->getDynamicOffsets().data());
    }
}

void CommandMgr::pushSet(PipelineLayout *pipelineLayout, Set *uniform, int binding)
{
    if (!hasPipeline) return;
    if (singleUse) {
        PFN_pushSet(actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), binding, uniform->getWrites().size(), uniform->getWrites().data());
        return;
    }
    for (auto &frame : frames) {
        PFN_pushSet(frame.actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), binding, uniform->getWrites().size(), uniform->getWrites().data());
    }
}

void CommandMgr::endRenderPass()
{
    inRenderPass = false;
    hasPipeline = false;
    if (singleUse) {
        vkCmdEndRenderPass(actual);
        return;
    }
    for (auto &frame : frames) {
        vkCmdEndRenderPass(frame.actual);
    }
}

void CommandMgr::pushConstant(PipelineLayout *pipelineLayout, VkShaderStageFlags stage, uint32_t offset, const void *data, uint32_t size)
{
    if (!hasPipeline) return;
    if (singleUse) {
        vkCmdPushConstants(actual, pipelineLayout->getPipelineLayout(), stage, offset, size, data);
        return;
    }
    for (auto &frame : frames) {
        vkCmdPushConstants(frame.actual, pipelineLayout->getPipelineLayout(), stage, offset, size, data);
    }
}

void CommandMgr::vkIf(Buffer *bool32, VkDeviceSize offset, bool invert)
{
    VkConditionalRenderingBeginInfoEXT cond {VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT, nullptr, bool32->get(), bool32->getOffset() + offset, 0};
    if (invert) cond.flags = VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;
    for (auto &frame : frames) {
        PFN_vkIf(frame.actual, &cond);
    }
}

void CommandMgr::addImageBarrier(Texture *texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t miplevel, uint32_t miplevelcount)
{
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture->getImage();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    if (miplevel == UINT32_MAX) {
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    } else {
        barrier.subresourceRange.baseMipLevel = miplevel;
        barrier.subresourceRange.levelCount = miplevelcount;
    }
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    imageBarrier.push_back(barrier);
}

void CommandMgr::compileBarriers(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkPipelineStageFlags dependencyFlags)
{
    if (singleUse) {
        vkCmdPipelineBarrier(actual, srcStage, dstStage, dependencyFlags, 0, nullptr, 0, nullptr, imageBarrier.size(), imageBarrier.data());
        imageBarrier.clear();
        return;
    }
    for (auto &frame : frames) {
        vkCmdPipelineBarrier(frame.actual, srcStage, dstStage, dependencyFlags, 0, nullptr, 0, nullptr, imageBarrier.size(), imageBarrier.data());
    }
    imageBarrier.clear();
}

void CommandMgr::blit(Texture *src, Texture *dst, uint32_t srcMipLevel, uint32_t dstMipLevel, VkFilter filter)
{
    int srcWidth, srcHeight, dstWidth, dstHeight;
    src->getDimensions(srcWidth, srcHeight);
    dst->getDimensions(dstWidth, dstHeight);
    const VkImageBlit imageBlit = {
        {VK_IMAGE_ASPECT_COLOR_BIT, srcMipLevel, 0, 1},
        {{0, 0, 0}, {std::max(srcWidth >> srcMipLevel, 1), std::max(srcHeight >> srcMipLevel, 1), 1}},
        {VK_IMAGE_ASPECT_COLOR_BIT, dstMipLevel, 0, 1},
        {{0, 0, 0}, {std::max(dstWidth >> dstMipLevel, 1), std::max(dstHeight >> dstMipLevel, 1), 1}}
    };
    if (singleUse) {
        vkCmdBlitImage(actual, src->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
        return;
    }
    for (auto &frame : frames) {
        vkCmdBlitImage(frame.actual, src->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
    }
}

void CommandMgr::vkEndIf()
{
    for (auto &frame : frames) {
        PFN_vkEndIf(frame.actual);
    }
}

void CommandMgr::setName(int commandIndex, const std::string &name)
{
    for (uint32_t i = 0; i < frames.size(); ++i) {
        master->setObjectName(frames[i].commandBuffers[commandIndex], VK_OBJECT_TYPE_COMMAND_BUFFER, std::to_string(i) + " " + name);
    }
}

void CommandMgr::waitGraphicQueueIdle()
{
    master->waitGraphicQueueIdle();
}

int CommandMgr::getCommandIndex()
{
    if (autoIndex < nbCommandBuffers)
        return autoIndex++;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) frames.size();
    allocInfo.commandPool = cmdPool;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    std::vector<VkCommandBuffer> tmp;
    tmp.resize(allocInfo.commandBufferCount);
    if (singleUse) {
        allocInfo.commandBufferCount = 1;
    } else {
        if (vkAllocateCommandBuffers(refDevice, &allocInfo, tmp.data()) != VK_SUCCESS) {
            throw std::runtime_error("échec de l'allocation de command buffers!");
        }
    }
    auto it = tmp.begin();
    for (auto &frame : frames) {
        if (singleUse) {
            allocInfo.commandPool = frame.cmdPool;
            if (vkAllocateCommandBuffers(refDevice, &allocInfo, &(*it)) != VK_SUCCESS) {
                throw std::runtime_error("échec de l'allocation de command buffers!");
            }
        }
        frame.commandBuffers.push_back(*it);
        it++;
        frame.signalSemaphores.resize(autoIndex + 1);
        if (vkCreateSemaphore(refDevice, &semaphoreInfo, nullptr, &frame.signalSemaphores.back()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphore for previously allocated command buffers.");
        }
    }
    return autoIndex++;
}

void CommandMgr::releaseUnusedMemory()
{
    if (singleUse) {
        for (uint32_t i = 0; i < frames.size(); ++i) {
            if (i == refFrameIndex) continue;
            vkWaitForFences(refDevice, 1, &frames[i].fence, VK_TRUE, UINT64_MAX);
            vkResetCommandPool(refDevice, frames[i].cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
        return;
    }
    vkTrimCommandPool(refDevice, cmdPool, 0);
}
