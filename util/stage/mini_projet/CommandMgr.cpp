#include "VirtualSurface.hpp"

CommandMgr::CommandMgr(VirtualSurface *_master, int nbCommandBuffers, bool singleUseCommands) : master(_master), refDevice(_master->refDevice), refRenderPass(_master->refRenderPass), refSwapChainFramebuffers(_master->refSwapChainFramebuffers), refFrameIndex(_master->refFrameIndex), singleUse(singleUseCommands)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = singleUseCommands ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;
    poolInfo.queueFamilyIndex = _master->getGraphicsQueueIndex();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) nbCommandBuffers;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    frames.resize(refSwapChainFramebuffers.size());
    for (auto &frame : frames) {
        if (vkCreateCommandPool(refDevice, &poolInfo, nullptr, &frame.cmdPool) != VK_SUCCESS) {
            throw std::runtime_error("échec de la création d'une command pool !");
        }
        allocInfo.commandPool = frame.cmdPool;
        frame.commandBuffers.resize(nbCommandBuffers);
        frame.submitInfo.resize(nbCommandBuffers);
        if (vkAllocateCommandBuffers(refDevice, &allocInfo, frame.commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("échec de l'allocation de command buffers!");
        }
        if (vkCreateFence(refDevice, &fenceInfo, nullptr, &frame.fence) != VK_SUCCESS) {
            throw std::runtime_error("Faild to create fence.");
        }
        frame.actual = VK_NULL_HANDLE;
    }
}

CommandMgr::~CommandMgr()
{
    for (auto &frame : frames) {
        vkWaitForFences(refDevice, 1, &frame.fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(refDevice, frame.fence, nullptr);
        vkDestroyCommandPool(refDevice, frame.cmdPool, nullptr);
    }
}

//! SubmitInfo/SubmitList can be optimized
void CommandMgr::setSubmitState(int index, bool submitState)
{
    const void *target = frames[0].submitInfo[index].pCommandBuffers;
    auto tmp = std::find_if(frames[0].submitList.begin(), frames[0].submitList.end(), [target](auto &sinfo){return sinfo.pCommandBuffers == target;});

    if (submitState) {
        if (tmp == frames[0].submitList.end())
            for (auto &frame : frames)
                frame.submitList.push_back(frame.submitInfo[index]);
    } else {
        if (tmp != frames[0].submitList.end()) {
            auto dec = tmp - frames[0].submitList.begin();
            for (auto &frame : frames)
                frame.submitList.erase(frame.submitList.begin() + dec);
        }
    }
}

//! SubmitInfo/SubmitList can be optimized
void CommandMgr::setSubmitInfo(int index, VkSubmitInfo &_submitInfo)
{
    _submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; // just to be shure
    _submitInfo.commandBufferCount = 1;
    for (auto &frame : frames) {
        _submitInfo.pCommandBuffers = &frame.commandBuffers[index];
        frame.submitInfo[index] = _submitInfo;
    }
}

void CommandMgr::submit()
{
    vkWaitForFences(refDevice, 1, &frames[refFrameIndex].fence, VK_TRUE, UINT64_MAX);
    vkResetFences(refDevice, 1, &frames[refFrameIndex].fence);
    if (vkQueueSubmit(queue, frames[refFrameIndex].submitList.size(), frames[refFrameIndex].submitList.data(), frames[refFrameIndex].fence) != VK_SUCCESS) {
        std::cerr << "\e[31mError : Faild to submit commands.\e[0m" << std::endl;
    }
}

void CommandMgr::init(int index)
{
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

void CommandMgr::compile()
{
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
    if (singleUse) {
        vkCmdDrawIndexedIndirect(actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndexedIndirectCommand));
        return;
    }
    for (auto &frame : frames) {
        vkCmdDrawIndexedIndirect(frame.actual, drawArgsArray->get(), offset, drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }
}

void CommandMgr::beginRenderPass()
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = refRenderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = master->swapChainExtent;

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    if (singleUse) {
        renderPassInfo.framebuffer = refSwapChainFramebuffers[refFrameIndex];
        vkCmdBeginRenderPass(actual, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return;
    }
    for (uint32_t i = 0; i < frames.size(); i++) {
        renderPassInfo.framebuffer = refSwapChainFramebuffers[i];
        vkCmdBeginRenderPass(frames[i].actual, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
}

void CommandMgr::bindVertex(VertexBuffer *vertex, uint32_t firstBinding, uint32_t bindingCount, VkDeviceSize offset)
{
    if (singleUse) {
        vkCmdBindVertexBuffers(actual, firstBinding, bindingCount, &vertex->get(), &offset);
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindVertexBuffers(frame.actual, firstBinding, bindingCount, &vertex->get(), &offset);
    }
}

void CommandMgr::bindPipeline(Pipeline *pipeline)
{
    if (singleUse) {
        vkCmdBindPipeline(actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindPipeline(frame.actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());
    }
}

void CommandMgr::bindUniform(PipelineLayout *pipelineLayout, Uniform *uniform)
{
    if (singleUse) {
        vkCmdBindDescriptorSets(actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), 0,  1, uniform->getDescriptorSet(), 0, nullptr);
        return;
    }
    for (auto &frame : frames) {
        vkCmdBindDescriptorSets(frame.actual, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getPipelineLayout(), 0,  1, uniform->getDescriptorSet(), 0, nullptr);
    }
}

void CommandMgr::endRenderPass()
{
    if (singleUse) {
        vkCmdEndRenderPass(actual);
        return;
    }
    for (auto &frame : frames) {
        vkCmdEndRenderPass(frame.actual);
    }
}
