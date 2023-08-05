#include "tools/draw_helper.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"
#include "tools/s_font.hpp"
#include "coreModule/ubo_cam.hpp"
#include "EntityCore/Resource/TileMap.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "bodyModule/hints.hpp"
#include "coreModule/nebula.hpp"
#include "mediaModule/video_player.hpp"
#include "starModule/hip_star_mgr.hpp"

#define MAX_CMDS 64
#define WRITE_PRINT(x, y, tx, ty) *(ptr++) = x; *(ptr++) = y; *(ptr++) = data.texture->tx; *ptr = data.texture->ty; ptr += 3
#define WRITE_PRINTH(x, y, t1x, t1y, t2x, t2y) *(ptr++) = x; *(ptr++) = y; *(ptr++) = t1x; *(ptr++) = t1y; *(ptr++) = t2x; *(ptr++) = t2y

DrawHelper::DrawHelper() : nebulaMat(*Context::instance->uniformMgr)
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
        for (int j = 0; j < MAX_CMDS; ++j)
            context.frame[i]->setName(drawer[i].cmds[j], "Helper " + std::to_string(i) + "-" + std::to_string(j));
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
    for (int i = 0; i < 3; ++i) {
        vkDestroyCommandPool(VulkanMgr::instance->refDevice, drawer[i].cmdPool, nullptr);
    }
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
                drawNebula(data->nebula);
                break;
            case SIGNAL_NEBULA:
                if (data->sigPass.subpass == UINT8_MAX) {
                    if (hasRecordedNebula) {
                        vkEndCommandBuffer(drawer[internalVFrameIdx].nebula);
                        hasRecordedNebula = false;
                    } else {
                        drawer[internalVFrameIdx].cancelledCmds.push_back(drawer[internalVFrameIdx].nebula);
                    }
                }
                break;
            case FRAME_SUBMIT:
                submit(data->submit.frameIdx, data->submit.lastFrameIdx);
                break;
        }
    }
    queue.release();
}

void DrawHelper::beginDraw(unsigned char subpass, FrameMgr &frame)
{
    drawer[externalVFrameIdx].frame = &frame;
    extCmdIdx = 0;
    beginDraw(subpass);
    // Clear unused shadow slots, so they can be reused
    uint8_t i = Context::instance->maxShadowCast;
    while (i--) {
        auto &sd = Context::instance->shadowData[i];
        if (sd.body) {
            if (sd.used) {
                sd.used = false;
            } else {
                sd.body = nullptr;
            }
        }
    }
}

void DrawHelper::beginDraw(unsigned char subpass)
{
    externalSubpass = subpass;
    drawer[externalVFrameIdx].sigpass.emplace_back(s_sigpass{.flag=SIGNAL_PASS, .subpass=subpass});
    while (!queue.emplace((DrawData *) &drawer[externalVFrameIdx].sigpass.back()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void DrawHelper::nextDraw(unsigned char subpass)
{
    pushCommand();
    beginDraw(subpass);
}

void DrawHelper::endDraw()
{
    drawer[externalVFrameIdx].sigpass.emplace_back(s_sigpass{.flag=SIGNAL_PASS, .subpass=UINT8_MAX});
    while (!queue.emplace((DrawData *) &drawer[externalVFrameIdx].sigpass.back()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    pushCommand();
}

void DrawHelper::pushCommand()
{
    auto &d = drawer[externalVFrameIdx];
    queue.flush();
    d.frame->toExecute(d.cmds[extCmdIdx++], externalSubpass);
}

void DrawHelper::beginNebulaDraw(const Mat4f &mat)
{
    nebulaMat = mat;
    drawer[externalVFrameIdx].sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=PASS_BACKGROUND});
    while (!queue.emplace((DrawData *) &drawer[externalVFrameIdx].sigpass.back()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void DrawHelper::endNebulaDraw()
{
    auto &d = drawer[externalVFrameIdx];
    d.sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=UINT8_MAX});
    queue.flush();
    while (!queue.emplace((DrawData *) &d.sigpass.back()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    d.frame->toExecute(d.nebula, PASS_BACKGROUND);
}

void DrawHelper::beginDrawCommand(unsigned char subpass)
{
    if (subpass != UINT8_MAX)
        internalSubpass = subpass;
}

void DrawHelper::endDrawCommand(unsigned char subpass)
{
    auto &d = drawer[internalVFrameIdx];
    if (subpass == UINT8_MAX) {
        // Start of frame
        d.waitMutex.lock();
        vkResetCommandPool(VulkanMgr::instance->refDevice, d.cmdPool, 0);
        d.cancelledCmds.clear();
        d.intCmdIdx = 0;
        frame = d.frame;
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
        frame->beginAsync(cmd, internalSubpass);
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
    layoutPrintH->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16); // Color
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
        pipeline->setDepthStencilMode();
        pipeline->bindVertex(*vertexPrint);
        pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        pipeline->bindShader("sfontPrint.vert.spv");
        pipeline->bindShader("sfontPrint.frag.spv");
        pipeline->build("drawPrint");
        pipelinePrint.push_back(std::move(pipeline));
    }
    switch (lastFlag) {
        case DRAW_PRINT:
            break;
        case DRAW_PRINTH:
            pipelinePrint[internalSubpass]->bind(cmd);
            layoutPrint->bindSet(cmd, *setPrints);
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
        pipeline->setDepthStencilMode();
        pipeline->bindVertex(*vertexPrintH);
        pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        pipeline->bindShader("sfontHorizontal.vert.spv");
        pipeline->bindShader("sfontHorizontal.frag.spv");
        pipeline->build("drawPrintH");
        pipelinePrintH.push_back(std::move(pipeline));
    }
    switch (lastFlag) {
        case DRAW_PRINTH:
            break;
        case DRAW_PRINT:
            pipelinePrintH[internalSubpass]->bind(cmd);
            layoutPrintH->bindSet(cmd, *setPrints);
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
    if (lastFlag != DRAW_HINT) {
        hintColor = data.color;
        Hints::bind(cmd, hintColor);
    } else if (hintColor != data.color) {
        hintColor = data.color;
        Hints::push(cmd, hintColor);
    }
    if (drawIdx + MAX_HINT_IDX_ > MAX_IDX)
        drawIdx = 0;
    float *ptr = ((float *) Context::instance->multiVertexMgr->getPtr()) + drawIdx * 6;
    const int drawCount = data.self->computeHints(ptr);
    assert(drawCount / 3 < MAX_HINT_IDX_);
    vkCmdDraw(cmd, drawCount, 1, drawIdx * 3, 0);
    drawIdx += (drawCount + 2) / 3;
}

void DrawHelper::drawNebula(DrawData::s_nebula &data)
{
    auto cmd = drawer[internalVFrameIdx].nebula;
    if (!hasRecordedNebula) {
        hasRecordedNebula = true;
        const VkDeviceSize zero = 0;
        frame->beginAsync(cmd, PASS_BACKGROUND);
        vkCmdBindVertexBuffers(cmd, 0, 1, &Context::instance->multiVertexMgr->getBuffer(), &zero);
        layoutNebula = Nebula::initDraw(cmd);
        if (!setNebula) {
            setNebula = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, layoutNebula);
            setNebula->bindUniform(*UBOCam::ubo, 0);
            setNebula->bindUniform(nebulaMat, 1);
        }
        layoutNebula->bindSets(cmd, {*setNebula, *data.set});
    } else
        layoutNebula->bindSet(cmd, *data.set, 1);
    layoutNebula->pushConstant(cmd, 0, &data.color);
    if (drawIdx + 4 > MAX_IDX)
        drawIdx = 0;
    int64_t *ptr = ((int64_t *) Context::instance->multiVertexMgr->getPtr()) + drawIdx * 3;
    for (int i = 0; i < 12; ++i)
        ptr[i] = data.data[i];
    vkCmdDraw(cmd, 4, 1, drawIdx, 0);
    drawIdx += 4;
}

void DrawHelper::waitFrame(unsigned char frameIdx)
{
    for (uint8_t i = 0; i < 3; ++i) {
        if (drawer[i].submitData.frameIdx == frameIdx) {
            while (!drawer[i].hasCompleted) {
                queue.flush();
                drawer[i].waitMutex.lock();
                drawer[i].waitMutex.unlock();
            }
            drawer[i].submitData.frameIdx = UINT8_MAX;
            drawer[i].hasCompleted = false;
            return;
        }
    }
}

void DrawHelper::submitFrame(unsigned char frameIdx, unsigned char lastFrameIdx)
{
    drawer[externalVFrameIdx].submitData.frameIdx = frameIdx;
    drawer[externalVFrameIdx].submitData.lastFrameIdx = lastFrameIdx;
    while (!queue.emplace((DrawData *) &drawer[externalVFrameIdx++].submitData))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    externalVFrameIdx %= 3;
    queue.flush();
}

// y = sqrt(radius*radius - cst*cst)

void DrawHelper::submit(unsigned char frameIdx, unsigned char lastFrameIdx)
{
    currentLastFrameIdx = lastFrameIdx;
    auto &d = drawer[internalVFrameIdx++];
    if (!d.cancelledCmds.empty())
        frame->cancelExecution(d.cancelledCmds);
    auto cmd = frame->preBegin();
    if (notInitialized) {
        Context::instance->shadow->use(cmd, Implicit::LAYOUT); // Perform a fully implicit layout transition
        notInitialized = false;
    }
    player->recordUpdate(cmd);
    s_texture::recordTransfer(cmd);
    if (s_font::tileMap)
		s_font::tileMap->uploadChanges(cmd, Implicit::SRC_LAYOUT);
    Context::instance->transfers[lastFrameIdx]->copy(cmd);
    Context::instance->transferSync->placeBarrier(cmd);
    if (Context::instance->starUsed[frameIdx])
        Context::instance->starUsed[frameIdx]->updateFramebuffer(cmd);
    if (d.selfShadow) {
        Context::instance->renderSelfShadow->begin(0, cmd);
        d.selfShadow->drawShadow(cmd);
        vkCmdEndRenderPass(cmd);
        d.selfShadow = nullptr;
    }
    for (auto &s : d.shadowers) {
        Context::instance->renderShadow->begin(0, cmd);
        float radius = s.radius;
        s.body->drawShadow(cmd, s.idx);
        vkCmdEndRenderPass(cmd);
        auto &sd = Context::instance->shadowData[s.idx];
        VkImageMemoryBarrier imageBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, Context::instance->shadow->getImage(), {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, s.idx, 1}};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
        const int iradius = radius;
        if (iradius != sd.constRadius) {
            sd.constRadius = iradius;
            sd.pipeline->modifySpecializedConstant(0, iradius);
            auto oldPipeline = sd.pipeline->build(true);
            if (oldPipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(VulkanMgr::instance->refDevice, oldPipeline, nullptr); // Warning : can be in use
        }
        int pixelCount = 0;
        radius *= radius;
        for (int i = 0; i <= iradius; ++i) {
            int tmp = sqrt(radius - i*i);
            pixelCount += tmp;
            sd.uniform->offsets[i] = tmp;
        }
        sd.uniform->pixelCount = pixelCount * 4 + 1;
        sd.compute(cmd);
        imageBarrier.srcAccessMask = imageBarrier.dstAccessMask;
        imageBarrier.oldLayout = imageBarrier.newLayout;
        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    }
    d.shadowers.clear();
    frame->postBegin();
    frame->submitInline();
    frame = nullptr;
    d.sigpass.clear();
    d.hasCompleted = true;
    d.waitMutex.unlock();
    internalVFrameIdx %= 3;
}

// Implement caching here
uint8_t DrawHelper::drawShadower(Body *target, float radius)
{
    auto &vec = drawer[externalVFrameIdx].shadowers;
    auto *shadowData = Context::instance->shadowData;
    const auto nbShadowData = Context::instance->maxShadowCast;
    // Scan for matching shadowing body
    uint8_t matchLevel = 0; // 1 = empty slot, 2 = matching body, 3 = cache-matching body
    uint8_t idx = UINT8_MAX;
    auto lsd = target->getLocalSunDirection();
    radius *= halfShadowRes;
    for (uint8_t i = 0; i < nbShadowData; ++i) {
        auto &sd = shadowData[i];
        switch (matchLevel) {
            case 0:
                if (!sd.used) {
                    idx = i;
                    matchLevel = (sd.body == nullptr);
                }
                [[fallthrough]];
            case 1:
                if (sd.body == target && !sd.used) {
                    idx = i;
                    matchLevel = 2;
                }
                [[fallthrough]];
            case 2:
                if (sd.body == target && fabs(sd.radius - radius) < SHADOW_RADIUS_TOLERANCE) {
                    auto tmp = lsd[0] * sd.bodyLight[0] + lsd[1] * sd.bodyLight[1] + lsd[2] * sd.bodyLight[2];
                    tmp *= tmp;
                    const auto tmp2 = (sd.bodyLight[0] * sd.bodyLight[0] + sd.bodyLight[1] * sd.bodyLight[1] + sd.bodyLight[2] * sd.bodyLight[2]) * lsd.lengthSquared();
                    if (tmp >= tmp2 * SHADOW_INVALIDATING_ANGLE) {
                        idx = i;
                        matchLevel = 3;
                    }
                }
        }
    }
    if (idx < nbShadowData) {
        shadowData += idx;
        shadowData->used = true;
        if (matchLevel < 3) {
            shadowData->body = target;
            shadowData->bodyLight[0] = lsd[0];
            shadowData->bodyLight[1] = lsd[1];
            shadowData->bodyLight[2] = lsd[2];
            shadowData->radius = radius;
            vec.push_back({target, radius, idx});
        }
    } else {
        VulkanMgr::instance->putLog("Shadow cast count is ABOVE LIMIT - using failsafe to minimize issues, but shadow shapes are mostly unpredictible - Please, increase max_shadow_cast in " + AppSettings::Instance()->getConfigFile(), LogType::ERROR);
        for (idx = nbShadowData - 1; vec[idx].body->getBodyType() != target->getBodyType(); --idx); // Try to minimize shadow hazards
        vec[idx].body = target;
        vec[idx].radius = radius;
    }
    return idx;
}
