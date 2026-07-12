#include "Renderer.hpp"
#include "ModularBody.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "tools/s_font.hpp"
// Complete types for the unique_ptr service members (pointer service).
#include "tools/s_texture.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

#define CMD_BATCH_SIZE 8

bool Renderer::showPointer = false; // mirrored from Core::object_pointer_visibility

Renderer::Renderer()
{
}

void Renderer::init(ToneReproductor *_eye)
{
    allocateCommands();
    eye = _eye;
    // Service families whose resources exist at startup: build now
    // (base-sync off the frame path - C3; the in-frame ensure calls are
    // first-use fallbacks only).
    ensurePointerFamily();
    ensureHintFamily();
}

void Renderer::beginDraw(uint8_t _frameIdx)
{
    // Depth-bucket input READ point: the partitioning consumer takes
    // ModularBody::drainNotableBodies() here (filled by this frame's update;
    // cleared at the next update start - see dispatchUpdate). Bucket math
    // lands with the partitioning implementation.
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
    batchBegin(); // batching service (was Halo::beginDraw - borrow dissolved)
    clippingFov.v[2] = ModularBody::halfFov;
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
}

void Renderer::endBodyDraw()
{
    batchFlush(); // trailing batched content into the last cmd: drawn after
                  // every body = on top, per the occlusion contract (the old
                  // path needed a dedicated trailing command buffer for this;
                  // recording before ending the frame cmd replaces it)
    recordPointer(); // after the trailing flush: the selection pointer draws
                     // on top of everything (old path drew it after the whole
                     // system, depth-less - same final order)
    vkEndCommandBuffer(cmd);
    batchEnd(); // transfer plan + half ping-pong (was Halo::endDraw)
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
    batchFlush(); // per-body boundary: previous bodies' batched content lands
                  // at the START of this body's cmd (Halo::nextDraw parity -
                  // over its own body's disc, behind this nearer body's)
    VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
    VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
    vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
    clippingFov.v[0] = zCenter - boundingRadius;
    clippingFov.v[1] = zCenter + boundingRadius;
}

// drawHalo, drawHint and the pointer live with the batching/service side
// (PipelineRegistry.cpp).

void Renderer::printGravity(s_font *font, const std::pair<float, float> &pos,
                            const std::string &str, const Vec4f &color,
                            float xshift, float yshift)
{
    if (!font || str.empty())
        return;
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    const auto px = vkmgr.rectToRender(pos); // same anchor space as drawHint/halo
    // Viewport geometry from the scissor rect (see header: equals the old
    // Projector disk-viewport center/radius by construction).
    const VkRect2D &rect = vkmgr.getScreenRect();
    const float dx = px.first - (rect.offset.x + rect.extent.width * 0.5f);
    const float dy = px.second - (rect.offset.y + rect.extent.height * 0.5f);
    const float radius = std::min(rect.extent.width, rect.extent.height) * 0.5f;
    // Too far outside the disk to be visible (old early-out, verbatim rule)
    if (sqrtf(dx*dx + dy*dy) > radius + font->getStrLen(str))
        return;
    // Faithful port of printGravity180's math (projector.cpp:393-415):
    // tangential orientation from the viewport center; the '- 1' pixel bias
    // is kept verbatim - it desingularizes atan2 at the exact center at the
    // cost of an angle bias that vanishes with distance, and pixel parity
    // with the old path matters more than the cleaner atan2(dx, dy) form.
    const float theta = M_PI + atan2f(dx, dy - 1.f);
    Mat4f mvp = Mat4f::ortho2D(rect.offset.x, rect.offset.x + rect.extent.width,
                               rect.offset.y, rect.offset.y + rect.extent.height);
    mvp = mvp * Mat4f::translation(Vec3f(px.first, px.second, 0));
    mvp = mvp * Mat4f::rotation(Vec3f(0, 0, -1), -theta);
    mvp = mvp * Mat4f::translation(Vec3f(xshift, -yshift, 0));
    mvp = mvp * Mat4f::scaling(Vec3f(1, -1, 1));
    // Delegation: s_font::print renders/caches the string texture and queues
    // a DRAW_PRINT into the DrawHelper - the same channel drawHint rides.
    font->print(0, 0, str, color, mvp, 0);
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
