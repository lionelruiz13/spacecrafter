#include "Renderer.hpp"
#include "ModularBody.hpp"
#include "bodyModule/halo.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"

#define CMD_BATCH_SIZE 8

Renderer::Renderer()
{
}

void Renderer::init(ToneReproductor *_eye)
{
    allocateCommands();
    eye = _eye;
}

void Renderer::beginDraw(uint8_t _frameIdx)
{
    // Depth-bucket input READ point: the partitioning consumer takes
    // ModularBody::drainNotableBodies() here (filled by this frame's update;
    // cleared at the next update start - see dispatchUpdate). Bucket math
    // lands with the partitioning implementation.
    hintQueue.clear(); // previous frame's entries are consumed (see member note)
    cmdIdx = 0;
    frameIdx = _frameIdx;
    frame = Context::instance->frame[frameIdx].get();
    clippingFov.v[2] = ModularBody::halfFov;
    cmd = cmds[frameIdx].front();
    frame->begin(cmd, PASS_MULTISAMPLE_DEPTH);
    frame->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

void Renderer::beginBodyDraw()
{
    Halo::beginDraw();
    clippingFov.v[2] = ModularBody::halfFov;
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
}

void Renderer::endBodyDraw()
{
    Halo::endDraw();
    vkEndCommandBuffer(cmd);
}

void Renderer::clearDepth(float zCenter, float boundingRadius)
{
    // Helper segment BEFORE the body's own command buffer: screen-space
    // content queued during this body's draw (its hint circle) must execute
    // before its geometry - hint BEHIND the disc, like the old path
    // (PipelineFamily.hpp occlusion contract; measured: the Moon's hint drew
    // in FRONT of the disc with the previous order [vixy: 2026-07-12]).
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    nextCommandBuffer();
    Halo::nextDraw(cmd);
    VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
    VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
    vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
    clippingFov.v[0] = zCenter - boundingRadius;
    clippingFov.v[1] = zCenter + boundingRadius;
}

void Renderer::drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag)
{
    auto &data = Halo::global->pData[Halo::global->offset + Halo::global->size++];
    data.pos = VulkanMgr::instance->rectToRender(pos);
    data.Color = color;
    data.rmag = rmag;
}

void Renderer::drawHint(const std::pair<float, float> &pos, const Vec4f &color)
{
    auto &data = hintQueue.emplace_back();
    data.flag = DRAW_HINT_POS;
    data.color = color;
    data.pos = VulkanMgr::instance->rectToRender(pos); // same space as old Body::screenPos
    Context::instance->helper->draw(&data);
}

float Renderer::adaptLuminance(float world_luminance) const
{
    return eye->adaptLuminance(world_luminance);
}

void Renderer::nextCommandBuffer()
{
    vkEndCommandBuffer(cmd);
    if (++cmdIdx >= cmds[frameIdx].size())
        allocateCommands();
    cmd = cmds[frameIdx][cmdIdx];
    frame->begin(cmd, PASS_MULTISAMPLE_DEPTH);
    frame->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

void Renderer::allocateCommands()
{
    const auto oldSize = cmds[0].size();
    for (uint8_t i = 0; i < 3; ++i) {
        cmds[i].resize(oldSize+CMD_BATCH_SIZE);
        if (Context::instance->frame[i]->createExternal(cmds[i].data()+oldSize, CMD_BATCH_SIZE) != VK_SUCCESS) {
            VulkanMgr::instance->putLog("Renderer: Failed to allocate command buffer", LogType::ERROR);
        }
    }
}
