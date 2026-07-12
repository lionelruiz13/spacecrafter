#ifndef SHADOW_SERVICE_HPP_
#define SHADOW_SERVICE_HPP_

#include "PipelineFamily.hpp"
#include "tools/vecmath.hpp"
#include <memory>
#include <vector>

class ModularBody;
class ObjL;
class Set;
class Texture;
template <typename T> class SharedBuffer;

// ============================================================================
// ShadowService - the shadow-map production half of G7 (S5), Renderer-owned.
// Design + full old-path mapping: shadow-paths.md B. Responsibilities:
//
// - LAYER POOL: N blurred shadow layers (R8 array, N = max_shadow_cast, D5
//   parameter), keyed by (caster, penumbra radius px, caster-local light
//   direction) with the old DrawHelper::drawShadower cache semantics ported:
//   reuse while |dRadius| < SHADOW_RADIUS_TOLERANCE px and light-direction
//   cos^2 >= SHADOW_INVALIDATING_ANGLE; two-phase aging (unused one frame ->
//   free-able, the old beginDraw walk). The light direction key is
//   CASTER-LOCAL (transpose(rot(mat)) * (L - C)): camera-independent, like
//   the old heliocentric getLocalSunDirection - an eye-space key would
//   invalidate (or worse, silently mismatch) cached layers on camera motion.
// - PRODUCTION: for each newly-assigned layer, one silhouette pass
//   (SHADOW_SHAPE service family on the SHADOW_STENCIL pass kind, the old
//   shadowShape; shadow_trace.vert REUSED) followed by one disc-blur
//   dispatch (SHADOW_BLUR COMPUTE bank, radius = variant key; shadowBlur.comp
//   = shadow.comp with the sampled-image access made format-correct:
//   utexture2D + samplerless texelFetch - closes the r8ui/D24S8 VUID class
//   structurally, shadow-paths.md B3 candidate B).
// - RECORDING WINDOW: jobs are declared during the frame (indexed by the
//   Vulkan frameIdx) and recorded in the PRE-COLOR window on the helper
//   thread (DrawHelper preFrameRecorder hook - exactly where the old path
//   records its shadow passes). This jobs-as-data shape IS the S4 seam: the
//   dedicated compute thread (RenderChain.hpp thread model) will consume the
//   same jobs; consumers (receivers sampling the layer array) never change.
// - The silhouette scratch target, render pass and resolution are the
//   Context ones (shadowTrace / renderShadow / shadowRes - path-neutral app
//   infrastructure, shared serially within the same recording window). The
//   LAYER array is service-owned: sharing the old path's layers would cross
//   the two paths' caches during the dual-path phase.
//
// enabled: the new-path shadow switch. Initialized from the same config key
// as the old path (experimental_shadows) for A/B parity; toggled PLAINLY by
// the flag command (the old XOR quirk is a defect, not reproduced). Default
// convergence point: shadow-paths.md D3.
// ============================================================================
class ShadowService {
public:
    ShadowService();
    ~ShadowService();

    // New-path shadow switch (see header block).
    static bool enabled;

    // Frame entry (called from Renderer::beginDraw): two-phase slot aging +
    // job list reset for this frame index.
    void beginFrame(uint8_t frameIdx);

    // Acquire a layer for (caster, radiusPx, casterLocalLightDir).
    // Returns the layer index, or -1 when the pool is exhausted (the caller
    // selects in occlusion order, so what drops is the least significant
    // shadow - replaces the old "unpredictible failsafe" slot steal) or when
    // the blur pipeline for that radius is not resident yet (C3: skip this
    // frame, the bank is still building - old parity: no shadow before
    // shadow_ready). *reused reports a cache hit (no production needed).
    int acquire(ModularBody *caster, float radiusPx, const Vec3f &casterLocalLightDir, bool *reused);

    // Write the silhouette matrix of a newly-assigned layer (mat3 mapping the
    // caster mesh to shadow-map NDC - shadow-paths.md B2) and queue its
    // production: mesh recorded in the silhouette pass, then blurred.
    void produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh);

    // The pre-color recording hook (DrawHelper::preFrameRecorder): records
    // every queued production of this frame. Helper thread; cmd is the
    // frame's primary command buffer, outside any render pass.
    void record(VkCommandBuffer cmd, uint8_t frameIdx);

    // The blurred-layer array, for receiver descriptor binding (sampler2DArray).
    Texture *layerArray() {
        return layers.get();
    }
    // True once the service is usable (families allocated, buffers created).
    inline operator bool() const {
        return inited;
    }
    // Idempotent; called at first enabled use (needs Context fully built).
    void ensureInit(Renderer &renderer);
    // Teardown while managers are alive (Renderer::releaseRegistry path).
    void release();

private:
    // std140 mirror of shadowBlur.comp binding 0 (scalar array stride 16).
    struct BlurUniform {
        float pixelCount;
        float _pad[3];
        struct PaddedInt {
            int v;
            int _pad[3];
        } offsets[511];
    };
    struct Slot {
        ModularBody *caster = nullptr;
        float radiusPx = 0;
        Vec3f lightDir {}; // caster-local (camera-independent cache key)
        bool used = false;
        int radius = 0;    // int blur radius (spec key)
        std::unique_ptr<SharedBuffer<BlurUniform>> uniform;
        std::unique_ptr<SharedBuffer<float[12]>> traceMat; // mat3, std140
        std::unique_ptr<Set> traceSet; // SHADOW_SHAPE set 0
        std::unique_ptr<Set> blurSet;  // SHADOW_BLUR set 0
    };
    struct Job {
        uint8_t slot;
        ObjL *mesh;
    };
    std::vector<Slot> slots;
    std::vector<Job> jobs[3];   // per Vulkan frameIdx; fencing guards reuse
    std::vector<VkImageView> layerViews;
    std::unique_ptr<Texture> layers; // R8 array, service-owned
    PipelineFamily shapeFamily;
    PipelineFamily blurFamily;
    Renderer *renderer = nullptr;
    uint32_t maxRadius = 0;
    uint8_t curFrame = 0;
    bool inited = false;
    bool layersInitialized = false; // first-record layout transition
    bool exhaustedLogged = false;
};

#endif /* end of include guard: SHADOW_SERVICE_HPP_ */
