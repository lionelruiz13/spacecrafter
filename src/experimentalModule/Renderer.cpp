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
#include <algorithm> // depth-bucket merge sort (beginDraw)

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
    // Shadow service: UNCONDITIONAL init (independent of the enabled flag) -
    // every MESH Set binds the layer array at slot 3, so the texture must
    // exist even when shadows are off (ShadowService.hpp).
    shadow.ensureInit(*this);
}

void Renderer::beginDraw(uint8_t _frameIdx)
{
    // ---- Depth-range partitioning build (S3) --------------------------------
    // Input: ModularBody::drainNotableBodies() - filled by this frame's update
    // (which runs before draw, SSystemFactory::update/draw), cleared at the
    // next update start. Old-path parity: computePreDraw's bucket merge
    // (solarsystem_display.cpp:136-176), with two resolved differences:
    // - no 1.1 margin (boundingRadius is inclusive by definition, decision 6);
    // - no znear clamp at build (the old 1e-10 fed a Projector needing
    //   positive planes; the new shaders consume the raw range - landed
    //   scenes run znear < 0 today, measured parity §11.27).
    sliceScratch.clear();
    orbitBucket = {0, 0};
    // Old needOrbitDepth gate is 10 px full diameter (absolute); px =
    // screenSize * 2 * viewportRadius (the drawHalo screen_r form).
    const float orbitScreenSize = 5.f / ModularBody::getViewportRadius();
    for (ModularBody *body : ModularBody::drainNotableBodies()) {
        if (!*body)
            continue; // old gate: only bodies visible ON SCREEN reserve a slice
        const float r = body->getBoundingRadius();
        if (r <= 0)
            continue; // old guard (a body without extent needs no slice)
        const float dist = body->getDistanceToObserver();
        sliceScratch.emplace_back(dist, r);
        if (body->getScreenSize() > orbitScreenSize) {
            // Orbit union range (consumer: ORBIT port, see getOrbitDepthBucket)
            if (orbitBucket.znear == 0 && orbitBucket.zfar == 0) {
                orbitBucket = {dist - r, dist + r};
            } else {
                if (orbitBucket.znear > dist - r)
                    orbitBucket.znear = dist - r;
                if (orbitBucket.zfar < dist + r)
                    orbitBucket.zfar = dist + r;
            }
        }
    }
    // Merge in draw order (far->near): notableBody is in tree-update order,
    // the merge needs distance order.
    std::sort(sliceScratch.begin(), sliceScratch.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });
    depthBuckets.clear();
    bool open = false;
    DepthBucket db;
    for (const auto &e : sliceScratch) {
        const float znear = e.first - e.second;
        const float zfar = e.first + e.second;
        if (open && db.znear < zfar) {
            // Overlaps the current (farther) bucket: merge. Both edges may
            // extend - "artificial planets may cover real planets" (old
            // comment): a nearer body's range can still reach farther out.
            if (db.znear > znear)
                db.znear = znear;
            if (db.zfar < zfar)
                db.zfar = zfar;
        } else {
            if (open)
                depthBuckets.push_back(db);
            db = {znear, zfar};
            open = true;
        }
    }
    if (open)
        depthBuckets.push_back(db);
    bucketIdx = 0;
    enteredBucket = -1;
    bucketMissLogged = false;
    // ------------------------------------------------------------------------
    cmdIdx = 0;
    frameIdx = _frameIdx;
    frame = Context::instance->frame[frameIdx].get();
    clippingFov.v[2] = ModularBody::halfFov;
    if (shadow)
        shadow.beginFrame(frameIdx); // slot aging + this frame's job list
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
    // ---- Bucket-entry actions (S3 partitioning - contract in the header) ---
    // Locate this body's bucket: draw order (far->near) equals build order,
    // so the cursor only ever advances - O(buckets) per frame total.
    while (bucketIdx + 1 < depthBuckets.size() && zCenter < depthBuckets[bucketIdx].znear)
        ++bucketIdx;
    if (bucketIdx < depthBuckets.size()
        && zCenter >= depthBuckets[bucketIdx].znear
        && zCenter <= depthBuckets[bucketIdx].zfar) {
        if (static_cast<int32_t>(bucketIdx) != enteredBucket) {
            // First body of this bucket: clear. Same-bucket successors keep the
            // depth content (per-pixel mutual occlusion is the point of merging).
            enteredBucket = static_cast<int32_t>(bucketIdx);
            VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
            VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
            vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        }
        // The SHARED depth mapping, re-established on EVERY call and not only
        // at bucket entry: same-bucket bodies must write comparable depth
        // values, which is why the range is the BUCKET's and never per-body -
        // and the bucket is the authority, so reading it again costs two loads
        // and removes an assumption. The assumption was that nothing writes
        // clippingFov between two same-bucket bodies; enterDepthlessSlice
        // (INTENT §5.52) is exactly such a writer, and a mid-band body can sort
        // between two members of one bucket. Re-establishing here means the
        // depth-less override cannot outlive the body that asked for it,
        // without that body having to save/restore renderer state (I2: one
        // authority for the range, consulted, not cached in a caller).
        clippingFov.v[0] = depthBuckets[bucketIdx].znear;
        clippingFov.v[1] = depthBuckets[bucketIdx].zfar;
    } else {
        // Out-of-coverage slice. Two known producers:
        // - out-of-ORDER: a body attached between this frame's sort and its
        //   draw (events-thread interim, INTENT §8.4.1 precondition; the S4
        //   publish-task handoff closes it structurally) draws at the sorted
        //   tail - its range may lie in an already-passed bucket. Measured
        //   live at every `body action load` (INTENT §11.30): one frame,
        //   self-healing.
        // - out-of-LIST: a body drawing without a notable entry (a real
        //   contract breach - no known producer).
        // Both degrade to the per-body clear + range: correct in isolation,
        // and optimal even for the in-passed-bucket case - that bucket's
        // depth content was already wiped by later buckets' clears, so there
        // is nothing left to merge with. Self-names once per frame with the
        // values needed to attribute the producer.
        if (!bucketMissLogged) {
            bucketMissLogged = true;
            VulkanMgr::instance->putLog("Renderer: depth slice outside bucket coverage - per-body fallback (out-of-order attach or notable-list breach): zCenter=" + std::to_string(zCenter) + " r=" + std::to_string(boundingRadius) + " buckets=" + std::to_string(depthBuckets.size()) + " idx=" + std::to_string(bucketIdx) + (depthBuckets.empty() ? "" : " cur=[" + std::to_string(depthBuckets[bucketIdx].znear) + "," + std::to_string(depthBuckets[bucketIdx].zfar) + "]"), LogType::WARNING);
        }
        enteredBucket = -1; // don't suppress the next real bucket's clear
        VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
        VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
        vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        clippingFov.v[0] = zCenter - boundingRadius;
        clippingFov.v[1] = zCenter + boundingRadius;
    }
}

void Renderer::beginOrbitTrace()
{
    // Fresh command buffer for the orbit pass (its own depth ownership, distinct
    // from the per-body buckets - old cmdBodyDepth was a dedicated buffer too).
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    nextCommandBuffer();
    // Flush any pending batched content (the last body's halos) BEFORE the
    // orbit lines - old Halo::endDraw ran before the orbit phase.
    batchFlush();
    // Orbit-union range: clear + set only when a body reserved a slice; {0,0}
    // means nothing on-screen is large enough (old backup-plane path) and the
    // orbit modules draw depth-free (they read the same {0,0} and drop depth).
    if (orbitBucket.znear != 0.f || orbitBucket.zfar != 0.f) {
        VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
        VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
        vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        // Old clamp: max(orbitBucket.znear, 1e-8) (solarsystem_display.cpp:303).
        clippingFov.v[0] = (orbitBucket.znear > 1e-8f) ? orbitBucket.znear : 1e-8f;
        clippingFov.v[1] = orbitBucket.zfar;
    }
    passKind = PassKind::TRACE;
}

void Renderer::beginOrbitLines()
{
    // Same command buffer, same depth content and range: the orbit COLOR
    // pipelines depth-test (LEQUAL) against the traces just written.
    passKind = PassKind::COLOR;
}

void Renderer::beginTrailDraw()
{
    // Fresh command buffer for the trail pass. Mirrors beginOrbitTrace MINUS the
    // depth clear/range: the trail is depthless (its pipelines have depth
    // test+write off), so no bucket is needed and the depth buffer is left
    // untouched. Flush pending batched content (the last body's halos) BEFORE
    // the trails, matching the old body-pass order.
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    nextCommandBuffer();
    batchFlush();
    passKind = PassKind::COLOR;
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
