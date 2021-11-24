#include "tools/draw_helper.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"
#include "tools/s_font.hpp"
#include "coreModule/ubo_cam.hpp"
#include "EntityCore/Resource/TileMap.hpp"

#define MAX_CMDS 16
#define WRITE_PRINT(x, y, tx, ty) *(ptr++) = x; *(ptr++) = y; *(ptr++) = data.texture->tx; *ptr = data.texture->ty; ptr += 3
#define WRITE_PRINTH(x, y, t1x, t1y, t2x, t2y) *(ptr++) = x; *(ptr++) = y; *(ptr++) = t1x; *(ptr++) = t1y; *(ptr++) = t2x; *(ptr++) = t2y

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

    // Ensure no concurrency for set allocation
    layoutPrint = std::make_unique<PipelineLayout>(vkmgr);
    layoutPrint->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layoutPrint->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER); // Maybe not a good idea to use default sampler
    layoutPrint->buildLayout();
    setPrints = std::make_unique<Set>(vkmgr, *context.setMgr, layoutPrint.get());
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

void DrawHelper::init()
{
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;

    vertexPrint = std::make_unique<VertexArray>(vkmgr, context.multiVertexArray->alignment);
    vertexPrint->createBindingEntry(6*sizeof(float));
    vertexPrint->addInput(VK_FORMAT_R32G32_SFLOAT);
    vertexPrint->addInput(VK_FORMAT_R32G32_SFLOAT);
    vertexPrintH = std::make_unique<VertexArray>(vkmgr, context.multiVertexArray->alignment);
    vertexPrintH->createBindingEntry(6*sizeof(float));
    vertexPrintH->addInput(VK_FORMAT_R32G32_SFLOAT);
    vertexPrintH->addInput(VK_FORMAT_R32G32_SFLOAT);
    vertexPrintH->addInput(VK_FORMAT_R32G32_SFLOAT);

    layoutPrint->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16); // Color
    layoutPrint->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 16, sizeof(Mat4f)); // MVP
    layoutPrint->build();

    layoutPrintH = std::make_unique<PipelineLayout>(vkmgr);
    layoutPrintH->setGlobalPipelineLayout(layoutPrint.get());
    layoutPrintH->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 12); // Color
    layoutPrintH->build();

    setPrints->bindUniform(*UBOCam::ubo, 0);
    setPrints->bindTexture(**s_font::tileMap, 1);
}

void DrawHelper::bindPrint(VkCommandBuffer cmd)
{
    while (pipelinePrint.size() <= internalSubpass) {
        if (!layoutPrintH)
            init();
        auto pipeline = std::make_unique<Pipeline>(*VulkanMgr::instance, *Context::instance->render, pipelinePrint.size(), layoutPrint.get());
        pipeline->bindVertex(*vertexPrint);
        pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        pipeline->bindShader("sfontPrint.vert.spv");
        pipeline->bindShader("sfontPrint.frag.spv");
        pipeline->build();
        pipelinePrint.push_back(std::move(pipeline));
    }
    switch (lastFlag) {
        case DRAW_PRINT:
            break;
        case DRAW_PRINTH:
            pipelinePrint[internalSubpass]->bind(cmd);
            break;
        default:
            pipelinePrint[internalSubpass]->bind(cmd);
            layoutPrint->bindSet(cmd, *setPrints);
    }
}

void DrawHelper::bindPrintH(VkCommandBuffer cmd)
{
    while (pipelinePrintH.size() <= internalSubpass) {
        if (!layoutPrintH)
            init();
        auto pipeline = std::make_unique<Pipeline>(*VulkanMgr::instance, *Context::instance->render, pipelinePrintH.size(), layoutPrintH.get());
        pipeline->bindVertex(*vertexPrintH);
        pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        pipeline->bindShader("sfontHorizontal.vert.spv");
        pipeline->bindShader("sfontHorizontal.frag.spv");
        pipeline->build();
        pipelinePrintH.push_back(std::move(pipeline));
    }
    switch (lastFlag) {
        case DRAW_PRINTH:
            break;
        case DRAW_PRINT:
            pipelinePrintH[internalSubpass]->bind(cmd);
            break;
        default:
            pipelinePrintH[internalSubpass]->bind(cmd);
            layoutPrintH->bindSet(cmd, *setPrints);
    }
}

void DrawHelper::drawPrint(s_print &data)
{
    auto cmd = getCmd();
    bindPrint(cmd);

    if (drawIdx + 4 > MAX_IDX)
        drawIdx = 0;
    layoutPrint->pushConstant(cmd, 0, &data.Color);
    layoutPrint->pushConstant(cmd, 1, &data.MVP);
    vkCmdDraw(cmd, 4, 1, drawIdx, 0);
    float *ptr = ((float *) Context::instance->multiVertexMgr->getPtr()) + drawIdx * 6;
    drawIdx += 4;
    if (data.h < 0) {
        // upsidedown is true
        WRITE_PRINT(data.x + data.w, data.y - data.h, x2, y1);
        WRITE_PRINT(data.x, data.y - data.h, x1, y1);
        WRITE_PRINT(data.x + data.w, data.y, x2, y2);
        WRITE_PRINT(data.x, data.y, x1, y2);
    } else {
        // upsidedown is false
        WRITE_PRINT(data.x, data.y, x1, y1);
        WRITE_PRINT(data.x, data.y + data.h, x1, y2);
        WRITE_PRINT(data.x + data.w, data.y, x2, y1);
        WRITE_PRINT(data.x + data.w, data.y + data.h, x2, y2);
    }
}

void DrawHelper::drawPrintH(s_printh &data)
{
    auto cmd = getCmd();
    bindPrintH(cmd);
    int steps = 2+int(data.psi*15);
    if (drawIdx + (steps + 1) * 2 > MAX_IDX)
        drawIdx = 0;
    layoutPrintH->pushConstant(cmd, 0, &data.texColor);
    vkCmdDraw(cmd, (steps + 1) * 2, 1, drawIdx, 0);
    float *ptr = ((float *) Context::instance->multiVertexMgr->getPtr()) + drawIdx * 6;
    drawIdx += (steps + 1) * 2;

    const float t1x = data.texture[0].x1, t1w = data.texture[0].x2 - t1x, t1y1 = data.texture[0].y1, t1y2 = data.texture[0].y2;
    const float t2x = data.texture[1].x1, t2w = data.texture[1].x2 - t2x, t2y1 = data.texture[1].y1, t2y2 = data.texture[1].y2;
    // Pre-calculate points (more efficient)
	for (int i=0; i<=steps; i++) {
		float angle, p, q, ratio;
        ratio = (float) i / steps;
		angle = data.theta - data.psi * ratio;
		p = sin(angle);
		q = cos(angle);

        *(ptr++) = data.center[0]+p*data.d_sub_textureH;
        *(ptr++) = data.center[1]+q*data.d_sub_textureH;
        *(ptr++) = t1x + t1w * ratio;
        *(ptr++) = t1y1;
        *(ptr++) = t2x + t2w * ratio;
        *(ptr++) = t2y1;

        *(ptr++) = data.center[0]+p*data.d;
        *(ptr++) = data.center[1]+q*data.d;
        *(ptr++) = t1x + t1w * ratio;
        *(ptr++) = t1y2;
        *(ptr++) = t2x + t2w * ratio;
        *(ptr++) = t2y2;
	}
}

void DrawHelper::drawHint(DrawData::s_hint &data)
{
    auto cmd = getCmd();
}

// void DrawHelper::waitCompletionOf(int frameIdx)
// {
//     if (!drawer[frameIdx].hasCompleted) {
//         drawer[frameIdx].waiter.lock();
//         drawer[frameIdx].waiter.unlock();
//     }
// }
