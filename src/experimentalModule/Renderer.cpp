#include "Renderer.hpp"
#include "ModularBody.hpp"
#include "bodyModule/halo.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"

#define CMD_BATCH_SIZE 8

void Renderer::init(ToneReproductor *_eye)
{
    allocateCommands();
    eye = _eye;
}

void Renderer::beginDraw(uint8_t _frameIdx)
{
    cmdIdx = UINT16_MAX;
    frameIdx = _frameIdx;
    frame = Context::instance->frame[frameIdx];
}

void Renderer::beginBodyDraw()
{
    Halo::beginDraw();
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
}

void Renderer::endBodyDraw()
{
    Halo::endDraw();
    if (cmd) {
        vkEndCommandBuffer(cmd);
        cmd = VK_NULL_HANDLE;
    }
}

void Renderer::clearDepth()
{
    nextCommandBuffer();
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    Halo::nextDraw(cmd);
    VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
    VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
    vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
}

void Renderer::drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag)
{
    auto &data = Halo::global->pData[Halo::global->offset + Halo::global->size++];
    data.pos = pos;
    data.Color = color;
    data.rmag = rmag;
}

float Renderer::adaptLuminance(float world_luminance) const
{
    return eye->adaptLuminance(world_luminance);
}

void Renderer::nextCommandBuffer()
{
    if (cmd)
        vkEndCommandBuffer(cmd);
    if (++cmdIdx >= cmds[frameIdx].size())
        allocateCommands();
    cmd = frame->begin(cmds[frameIdx][cmdIdx], PASS_MULTISAMPLE_DEPTH);
    frame->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

void Renderer::allocateCommands()
{
    const auto oldSize = cmds[i].size();
    for (uint8_t i = 0; i < 3; ++i) {
        cmds[i].resize(oldSize+CMD_BATCH_SIZE);
        if (Context::instance->frame[i]->createExternal(cmds[i].data()+oldSize, CMD_BATCH_SIZE) != VK_SUCCESS) {
            VulkanMgr::instance->putLog("Renderer: Failed to allocate command buffer", LogType::ERROR);
        }
    }
}
