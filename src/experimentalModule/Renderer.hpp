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

// The only Vulkan surface of the module; every method executes inside the render chain (RenderChain.hpp)
class Renderer {
public:
    Renderer();
    ~Renderer();
    void init(ToneReproductor *eye);
    // Begin recording for this frame index. Called from the frame task only.
    void beginDraw(uint8_t frameIdx);
    void beginBodyDraw();
    void endBodyDraw();
    // Enter the depth slice of one body: once per body drawn with depth, in the far->near order of notableBody
    // Bodies whose [zCenter-boundingRadius, zCenter+boundingRadius] overlap share one depth, cleared at the first one
    void clearDepth(float zCenter, float boundingRadius);
    // Clipping range of one body drawn without depth (the rasterizer clips outside it even then): no clear, no slice
    inline void enterDepthlessSlice(float zCenter, float boundingRadius) {
        clippingFov.v[0] = zCenter - boundingRadius;
        clippingFov.v[1] = zCenter + boundingRadius;
    }
    // System phases after the bodies, each leaving the command buffer open
    // Clear the depth over getOrbitDepthBucket() and enter TRACE: the discs drawn here hide the orbit lines
    void beginOrbitTrace();
    void beginOrbitLines();
    // Trail and tail phases are depth-less COLOR, without depth clear
    void beginTrailDraw();
    void beginTailDraw();
    // Layout of the body_tail.vert instance inputs (locations 2-7); every vector is in eye space
    struct TailInstance {
        Vec3f offset;             // comet eye-space position
        Vec3f expandDirection;    // eye-space initial expansion (parentFrame-applied)
        Vec3f expandCorrection;   // eye-space expansion correction
        Vec3f coefRadius;         // radius profile {xx,x,base} * comaDiameter (AU)
        Vec3f color;              // tail RGB
        Vec3f modelViewMatrix[3]; // 3x3 orienting rotation
    };
    // Length of the shared tail geometry; the submitter divides its directionCorrection by it
    static constexpr int TAIL_TIME_SEGMENTS = 16;
    void submitTail(const TailInstance &inst);
    void flushTails();
    // One partitioned depth range (see partitioning contract above).
    struct DepthBucket {
        float znear, zfar;
    };
    // Union range of the bodies which must hide their orbit line; {0,0} = none, draw the orbits without depth
    // znear may be <= 0 with the camera inside the range: clamp at use
    inline const DepthBucket &getOrbitDepthBucket() const {
        return orbitBucket;
    }
    // Screen-space services: pos is in rect space [-1,1] (ModularBody::getScreenPos)
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    void setHaloTexture(const std::string &texName);
    // Recorded at once into the current command buffer: call before drawing the disc. No-op until setSunHaloTexture
    void drawSunHalo(const std::pair<float, float> &pos, const Vec3f &color,
                     float rmag, float cmag, float radius);
    void setSunHaloTexture(const std::string &texName, const std::string &path);
    void drawHint(const std::pair<float, float> &pos, const Vec4f &color);
    // Drawn at endBodyDraw, on top of everything; at most one call per frame. sizePx = full diameter in render pixels
    void drawPointer(const std::pair<float, float> &pos, float sizePx);
    // Selection pointer visibility, written only by Core::setFlagSelectedObjectPointer
    static bool showPointer;
    float adaptLuminance(float world_luminance) const;
    inline operator VkCommandBuffer() {
        return cmd;
    }
    inline const Vec3f &getClippingFov() const {
        return clippingFov;
    }

    // Pipeline-family registry (PipelineFamily.hpp): allocate from the registration path, never from the frame task
    SetContract allocateSetContract(SetContractDesc &&desc);
    // The pre-allocated app-wide UBO set contract (context.uboSet).
    SetContract globalUboContract() const;
    // Allocate a family; dedup by desc.name (same name = same underlying
    // family, lifetime = union of live handles).
    PipelineFamily allocateFamily(PipelineFamilyDesc &&desc);
    // Bind the best resident variant for the current pass kind; record against the returned layout. Draw hooks only
    // Never blocks: non-resident bits are dropped for this frame and their build enqueued (see FamilyBound::got)
    FamilyBound bind(const PipelineFamily &family, VariantKey wanted = 0);
    // New Set of the family's set contract setIndex, owned by the caller; nullptr for an external contract
    Set *allocSet(const PipelineFamily &family, uint8_t setIndex);
    void batchPush(const PipelineFamily &family, const void *instance);
    // bind() with explicit pass and command buffer, for recording outside the frame task (shadow production)
    FamilyBound bindIn(const PipelineFamily &family, PassKind pass, VkCommandBuffer cmd, VariantKey wanted = 0);
    // Resident pipeline of one exact variant, valid this frame; nullptr = build enqueued, skip that geometry
    // To switch pipelines mid-record against one layout: bind()/bindIn() first, no set change in between
    Pipeline *peek(const PipelineFamily &family, PassKind pass, VariantKey wanted = 0);
    // key = integer variant of a COMPUTE bank; layout is nullptr while not resident: skip the dispatch, never wait
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

    // Gravity-oriented label at an already-projected anchor; shifts are pixels in the rotated frame
    void printGravity(s_font *font, const std::pair<float, float> &pos,
                      const std::string &str, const Vec4f &color,
                      float xshift, float yshift);

    // Tear down the registry and the services; call first in Context::~Context, while every manager is alive
    void releaseRegistry();

private:
    // ---- Batching service internals (defined in PipelineRegistry.cpp) ----
    // Rebind batch resources that changed (e.g. halo texture) - frame start.
    void batchBegin();
    void batchFlush();
    // Frame end: plan the staging->vertex transfer of the drawn region and swap the buffer halves
    void batchEnd();
    void ensureHaloFamily();
    void ensureHintFamily();
    void ensureSunHaloFamily();
    void ensureTailFamily();
    // Built at init(); the in-frame calls are first-use fallbacks only
    void ensurePointerFamily();
    // Record the queued pointer (if any) into the current cmd - called by
    // endBodyDraw after the trailing batchFlush: on top of everything.
    void recordPointer();
    void allocateCommands();
    void nextCommandBuffer();
    PipelineFamily haloFamily;
    PipelineFamily hintFamily;
    PipelineFamily pointerFamily;
    // Pointer and sun halo resources, released by releaseRegistry() while the managers are alive
    std::unique_ptr<VertexBuffer> pointerVertex; // 4 corners, planCopy'd per use
    std::unique_ptr<s_texture> pointerTex;       // pointer_planet.png
    std::unique_ptr<Set> pointerSet;
    float pointerTimeMs = 0;   // breathing clock
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
