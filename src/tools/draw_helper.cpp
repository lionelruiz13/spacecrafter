#include "tools/draw_helper.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"

#define MAX_CMDS 16

DrawHelper::DrawHelper()
{
    thread = std::thread(&DrawHelper::mainloop, this);
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, context.graphicFamily->id};
    VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, VK_NULL_HANDLE, VK_COMMAND_BUFFER_LEVEL_SECONDARY, MAX_CMDS};
    for (int i = 0; i < 3; ++i) {
        vkCreateCommandPool(vkmgr.refDevice, &poolInfo, nullptr, &drawer[i].cmdPool);
        allocInfo.commandPool = drawer[i].cmdPool;
        drawer[i].cmds.resize(MAX_CMDS);
        vkAllocateCommandBuffers(vkmgr.refDevice, &allocInfo, drawer[i].cmds.data());
        drawer[i].nebula = drawer[i].cmds.back();
        drawer[i].cmds.back() = VK_NULL_HANDLE;
    }
}

DrawHelper::~DrawHelper()
{
    queue.close();
    thread.join();
}

void DrawHelper::mainloop()
{
    DrawData *data = nullptr;
    unsigned char subpass = UINT8_MAX;

    queue.acquire();
    while (queue.pop(data)) {
        switch (data->flag) {
            case DRAW_PRINT:
                drawPrint(data->print);
                lastFlag = DRAW_PRINT;
                break;
            case DRAW_PRINTH:
                drawPrintH(data->printh);
                lastFlag = DRAW_PRINTH;
                break;
            case DRAW_HINT:
                drawHint(data->hint);
                lastFlag = DRAW_HINT;
                break;
            case SIGNAL_PASS:
                endDrawCommand(subpass);
                subpass = data->sigPass.subpass;
                beginDrawCommand(subpass);
                lastFlag = SIGNAL_PASS;
                break;
            case DRAW_NEBULA:
                break;
            case SIGNAL_NEBULA:
                break;
        }
    }
    queue.release();
}

void DrawHelper::beginDraw(unsigned char subpass, FrameMgr &frame)
{
    this->frame = &frame;
    extCmdIdx = 0;
    beginDraw(subpass);
}

void DrawHelper::beginDraw(unsigned char subpass)
{
    externalSubpass = subpass;
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_PASS, .subpass=subpass});
    queue.emplace((DrawData *) &sigpass.back());
}

void DrawHelper::nextDraw(unsigned char subpass)
{
    pushCommand();
    beginDraw(subpass);
}

void DrawHelper::endDraw()
{
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_PASS, .subpass=UINT8_MAX});
    queue.emplace((DrawData *) &sigpass.back());
    pushCommand();
}

void DrawHelper::pushCommand()
{
    queue.flush();
    frame->toExecute(drawer[externalVFrameIdx].cmds[extCmdIdx++], externalSubpass);
}

void DrawHelper::beginNebulaDraw(const Mat4f &mat)
{
    nebulaMat = mat;
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=0});
    queue.emplace((DrawData *) &sigpass.back());
}

void DrawHelper::endNebulaDraw()
{
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=UINT8_MAX});
    queue.emplace((DrawData *) &sigpass.back());
}

void DrawHelper::submit()
{
    if (!frame)
        return;
    // Wait completion of previous events
    while (!drawer[externalVFrameIdx].hasCompleted) {
        queue.flush();
        drawer[externalVFrameIdx].waitMutex.lock();
        drawer[externalVFrameIdx].waitMutex.unlock();
    }
    sigpass.clear();
    drawer[externalVFrameIdx].hasCompleted = false;
    frame->cancelExecution(drawer[externalVFrameIdx++].cancelledCmds);
    externalVFrameIdx %= 3;
    frame = nullptr;
}

void DrawHelper::beginDrawCommand(unsigned char subpass)
{
    auto &d = drawer[internalVFrameIdx];
    if (subpass == UINT8_MAX) {
        // End of frame
        d.hasCompleted = true;
        d.waitMutex.unlock();
        ++internalVFrameIdx;
        internalVFrameIdx %= 3;
    } else {
        internalSubpass = subpass;
    }
}

void DrawHelper::endDrawCommand(unsigned char subpass)
{
    auto &d = drawer[internalVFrameIdx];
    if (subpass == UINT8_MAX) {
        // Start of frame
        vkResetCommandPool(VulkanMgr::instance->refDevice, d.cmdPool, 0);
        d.cancelledCmds.clear();
        d.intCmdIdx = 0;
        d.waitMutex.lock();
    } else {
        if (hasRecorded) {
            vkEndCommandBuffer(d.cmds[d.intCmdIdx++]);
            hasRecorded = false;
        } else {
            d.cancelledCmds.push_back(d.cmds[d.intCmdIdx++]);
        }
    }
}

VkCommandBuffer DrawHelper::getCmd()
{
    auto &d = drawer[internalVFrameIdx];
    VkCommandBuffer cmd = d.cmds[d.intCmdIdx];
    if (!hasRecorded) {
        hasRecorded = true;
        const VkDeviceSize zero = 0;
        frame->begin(cmd, internalSubpass);
        vkCmdBindVertexBuffers(cmd, 0, 1, &Context::instance->multiVertexMgr->getBuffer(), &zero);
    }
    return cmd;
}

void DrawHelper::drawPrint(s_print &data)
{
}

void DrawHelper::drawPrintH(s_printh &data)
{
}

void DrawHelper::drawHint(DrawData::s_hint &data)
{
}

// void DrawHelper::waitCompletionOf(int frameIdx)
// {
//     if (!drawer[frameIdx].hasCompleted) {
//         drawer[frameIdx].waiter.lock();
//         drawer[frameIdx].waiter.unlock();
//     }
// }
