#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "tools/vecmath.hpp"
#include "PipelineFamily.hpp"
#include "ShadowService.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class ToneReproductor;
class FrameMgr;
class Set;
class s_font;
class s_texture;
class VertexBuffer;
template<typename> class SharedBuffer;

class Renderer {
public:
    Renderer();
    ~Renderer();
    void init(ToneReproductor *eye);
    // Begin recording for this frame index. Called from the frame task only.
    void beginDraw(uint8_t frameIdx);
    void beginBodyDraw();
    void endBodyDraw();
    void clearDepth(float zCenter, float boundingRadius);
    inline void enterDepthlessSlice(float zCenter, float boundingRadius) {
        clippingFov.v[0] = zCenter - boundingRadius;
        clippingFov.v[1] = zCenter + boundingRadius;
    }
    void beginOrbitTrace();
    void beginOrbitLines();
    void beginTrailDraw();
    void beginTailDraw();
    struct TailInstance {
        Vec3f offset;             // comet eye-space position (old eye_planet)
        Vec3f expandDirection;    // eye-space initial expansion (parentFrame-applied)
        Vec3f expandCorrection;   // eye-space expansion correction
        Vec3f coefRadius;         // radius profile {xx,x,base} * comaDiameter (AU)
        Vec3f color;              // tail RGB
        Vec3f modelViewMatrix[3]; // 3x3 orienting rotation
    };
    // NB_TAIL_LENGTH of the shared geometry (old tail.cpp): the submitter needs
    // it for the directionCorrection/NB_TAIL_LENGTH orientation term.
    static constexpr int TAIL_TIME_SEGMENTS = 16;
    void submitTail(const TailInstance &inst);
    void flushTails();
    // One partitioned depth range (see partitioning contract above).
    struct DepthBucket {
        float znear, zfar;
    };
    inline const DepthBucket &getOrbitDepthBucket() const {
        return orbitBucket;
    }
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    void setHaloTexture(const std::string &texName);
    void drawSunHalo(const std::pair<float, float> &pos, const Vec3f &color,
                     float rmag, float cmag, float radius);
    void setSunHaloTexture(const std::string &texName, const std::string &path);
    void drawHint(const std::pair<float, float> &pos, const Vec4f &color);
    void drawPointer(const std::pair<float, float> &pos, float sizePx);
    // Old Core::object_pointer_visibility mirror (set through
    // Core::setFlagSelectedObjectPointer - the single choke point).
    static bool showPointer;
    float adaptLuminance(float world_luminance) const;
    inline operator VkCommandBuffer() {
        return cmd;
    }
    inline const Vec3f &getClippingFov() const {
        return clippingFov;
    }

    SetContract allocateSetContract(SetContractDesc &&desc);
    // The pre-allocated app-wide UBO set contract (context.uboSet).
    SetContract globalUboContract() const;
    // Allocate a family; dedup by desc.name (same name = same underlying
    // family, lifetime = union of live handles).
    PipelineFamily allocateFamily(PipelineFamilyDesc &&desc);
    FamilyBound bind(const PipelineFamily &family, VariantKey wanted = 0);
    Set *allocSet(const PipelineFamily &family, uint8_t setIndex);
    void batchPush(const PipelineFamily &family, const void *instance);
    FamilyBound bindIn(const PipelineFamily &family, PassKind pass, VkCommandBuffer cmd, VariantKey wanted = 0);
    Pipeline *peek(const PipelineFamily &family, PassKind pass, VariantKey wanted = 0);
    FamilyBound bindCompute(const PipelineFamily &family, VariantKey key, VkCommandBuffer cmd);
    // Non-binding residency query for a COMPUTE bank key (acquire-time gate:
    // don't hand out a layer whose blur cannot run this frame).
    bool computeReady(const PipelineFamily &family, VariantKey key) const;

    ShadowService shadow;
    // The pass kind currently recording; bind() resolves against it, hooks
    // are only invoked within their matching pass kind (trait-routed).
    inline PassKind currentPassKind() const {
        return passKind;
    }

    void printGravity(s_font *font, const std::pair<float, float> &pos,
                      const std::string &str, const Vec4f &color,
                      float xshift, float yshift);

    void releaseRegistry();

private:
    // ---- Batching service internals (defined in PipelineRegistry.cpp) ----
    // Rebind batch resources that changed (e.g. halo texture) - frame start.
    void batchBegin();
    void batchFlush();
    // Frame end: plan the staging->vertex transfer of the drawn region and
    // ping-pong the buffer halves (portage of Halo::endDraw bookkeeping).
    void batchEnd();
    void ensureHaloFamily();
    void ensureHintFamily();
    void ensureSunHaloFamily();
    // Lazy build of the TAIL instanced batch (row 12): shared geometry + index
    // + instance buffer + pipeline family, built once on first submit/flush.
    void ensureTailFamily();
    // Built at init() (base-sync at startup, C3-clean; the in-frame ensure
    // calls are first-use fallbacks only, like the halo's).
    void ensurePointerFamily();
    // Record the queued pointer (if any) into the current cmd - called by
    // endBodyDraw after the trailing batchFlush: on top of everything.
    void recordPointer();
    void allocateCommands();
    void nextCommandBuffer();
    PipelineFamily haloFamily;
    PipelineFamily hintFamily;
    PipelineFamily pointerFamily;
    // Pointer service resources (old ObjectBase statics, Renderer-owned;
    // released by releaseRegistry() while the managers are alive).
    std::unique_ptr<VertexBuffer> pointerVertex; // 4 corners, planCopy'd per use
    std::unique_ptr<s_texture> pointerTex;       // pointer_planet.png
    std::unique_ptr<Set> pointerSet;
    float pointerTimeMs = 0;   // breathing clock (old ObjectBase::local_time)
    bool pointerQueued = false;
    struct { float x, y, size; } pointerData; // render px, resolved size
    PipelineFamily sunHaloFamily;
    std::unique_ptr<VertexBuffer> sunHaloVertex; // 1 screen point, planCopy'd per draw
    std::unique_ptr<s_texture> sunHaloTex;       // tex_big_halo (big_halo.png)
    std::unique_ptr<Set> sunHaloSet;
    std::unique_ptr<SharedBuffer<float>> uSunRmag, uSunCmag, uSunRadius;
    std::unique_ptr<SharedBuffer<Vec3f>> uSunColor;
    bool sunHaloTexBound = false; // deferred texture bind (frame start, post-upload)
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    ToneReproductor *eye;
    FrameMgr *frame;
    PassKind passKind = PassKind::COLOR;
    std::vector<VkCommandBuffer> cmds[3];
    Vec3f clippingFov;
    // ---- Depth-range partitioning state (built by beginDraw, walked by
    // clearDepth - same effective-thread serialization as cmdIdx/frameIdx).
    std::vector<DepthBucket> depthBuckets; // disjoint, far->near (build = draw order)
    std::vector<std::pair<float, float>> sliceScratch; // (distance, radius) build scratch
    DepthBucket orbitBucket {0, 0};
    uint32_t bucketIdx = 0;     // advance-only cursor (draw order == build order)
    int32_t enteredBucket = -1; // last bucket cleared this frame (-1 = none)
    bool bucketMissLogged = false; // rate-limit the contract-breach log (per frame)
    uint16_t cmdIdx;
    uint8_t frameIdx;
};

#endif /* end of include guard: RENDERER_HPP_ */
