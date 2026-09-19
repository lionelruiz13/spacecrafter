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

// Record the Vulkan commands of the module, only called from the render chain
class Renderer {
public:
    Renderer();
    ~Renderer();
    void init(ToneReproductor *eye);
    void beginDraw(uint8_t frameIdx);
    void beginBodyDraw();
    void endBodyDraw();
    // Prepare the depth buffer for a depth range, call once per body from far to near
    void clearDepth(float zCenter, float boundingRadius);
    inline void enterDepthlessSlice(float zCenter, float boundingRadius) {
        clippingFov.v[0] = zCenter - boundingRadius;
        clippingFov.v[1] = zCenter + boundingRadius;
    }
    // Clear the depth, then trace the discs which hide the orbit lines
    void beginOrbitTrace();
    void beginOrbitLines();
    void beginTrailDraw();
    void beginTailDraw();
    // Instance inputs of body_tail.vert, in eye space
    struct TailInstance {
        Vec3f offset; // Comet position
        Vec3f expandDirection;
        Vec3f expandCorrection;
        Vec3f coefRadius; // Radius profile {xx, x, base} * comaDiameter, in AU
        Vec3f color;
        Vec3f modelViewMatrix[3];
    };
    static constexpr int TAIL_TIME_SEGMENTS = 16;
    void submitTail(const TailInstance &inst);
    void flushTails();
    struct DepthBucket {
        float znear, zfar;
    };
    // Return the depth range of bodies hiding orbit lines, {0,0} = none. znear may be <= 0
    inline const DepthBucket &getOrbitDepthBucket() const {
        return orbitBucket;
    }
    // With pos in rect space [-1, 1]
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    void setHaloTexture(const std::string &texName);
    // Draw at once: call before drawing the disc, after setSunHaloTexture
    void drawSunHalo(const std::pair<float, float> &pos, const Vec3f &color,
                     float rmag, float cmag, float radius);
    void setSunHaloTexture(const std::string &texName, const std::string &path);
    void drawHint(const std::pair<float, float> &pos, const Vec4f &color);
    // Draw on top of everything, at most once per frame. sizePx = diameter in render pixels
    void drawPointer(const std::pair<float, float> &pos, float sizePx);
    // Only written by Core::setFlagSelectedObjectPointer
    static bool showPointer;
    float adaptLuminance(float world_luminance) const;
    inline operator VkCommandBuffer() {
        return cmd;
    }
    inline const Vec3f &getClippingFov() const {
        return clippingFov;
    }

    // Allocate from the registration path, never from the frame task
    SetContract allocateSetContract(SetContractDesc &&desc);
    SetContract globalUboContract() const;
    // Same desc.name = same family, alive while any handle is
    PipelineFamily allocateFamily(PipelineFamilyDesc &&desc);
    // Bind the best resident variant for the current pass, never blocks. Draw hooks only
    FamilyBound bind(const PipelineFamily &family, VariantKey wanted = 0);
    // Return a new Set owned by the caller, nullptr for an external contract
    Set *allocSet(const PipelineFamily &family, uint8_t setIndex);
    void batchPush(const PipelineFamily &family, const void *instance);
    // bind() outside the frame task
    FamilyBound bindIn(const PipelineFamily &family, PassKind pass, VkCommandBuffer cmd, VariantKey wanted = 0);
    // Return this exact variant if resident, else nullptr: skip. Call bind()/bindIn() first
    Pipeline *peek(const PipelineFamily &family, PassKind pass, VariantKey wanted = 0);
    FamilyBound bindCompute(const PipelineFamily &family, VariantKey key, VkCommandBuffer cmd);
    bool computeReady(const PipelineFamily &family, VariantKey key) const;

    ShadowService shadow;
    inline PassKind currentPassKind() const {
        return passKind;
    }

    // Print a label at projected pos, shifts are in pixels of the rotated frame
    void printGravity(s_font *font, const std::pair<float, float> &pos,
                      const std::string &str, const Vec4f &color,
                      float xshift, float yshift);

    // Call first in Context::~Context, while every manager is alive
    void releaseRegistry();

private:
    void batchBegin();
    void batchFlush();
    void batchEnd();
    void ensureHaloFamily();
    void ensureHintFamily();
    void ensureSunHaloFamily();
    void ensureTailFamily();
    void ensurePointerFamily();
    void recordPointer();
    void allocateCommands();
    void nextCommandBuffer();
    PipelineFamily haloFamily;
    PipelineFamily hintFamily;
    PipelineFamily pointerFamily;
    std::unique_ptr<VertexBuffer> pointerVertex;
    std::unique_ptr<s_texture> pointerTex;
    std::unique_ptr<Set> pointerSet;
    float pointerTimeMs = 0; // Breathing clock
    bool pointerQueued = false;
    struct { float x, y, size; } pointerData; // In render pixels
    PipelineFamily sunHaloFamily;
    std::unique_ptr<VertexBuffer> sunHaloVertex;
    std::unique_ptr<s_texture> sunHaloTex;
    std::unique_ptr<Set> sunHaloSet;
    std::unique_ptr<SharedBuffer<float>> uSunRmag, uSunCmag, uSunRadius;
    std::unique_ptr<SharedBuffer<Vec3f>> uSunColor;
    bool sunHaloTexBound = false;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    ToneReproductor *eye;
    FrameMgr *frame;
    PassKind passKind = PassKind::COLOR;
    std::vector<VkCommandBuffer> cmds[3];
    Vec3f clippingFov;
    std::vector<DepthBucket> depthBuckets; // Disjoint, from far to near
    std::vector<std::pair<float, float>> sliceScratch; // (distance, radius)
    DepthBucket orbitBucket {0, 0};
    uint32_t bucketIdx = 0;
    int32_t enteredBucket = -1; // Last bucket cleared this frame
    bool bucketMissLogged = false;
    uint16_t cmdIdx;
    uint8_t frameIdx;
};

#endif /* end of include guard: RENDERER_HPP_ */
