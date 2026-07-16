#ifndef SHADOW_SERVICE_HPP_
#define SHADOW_SERVICE_HPP_

#include "PipelineFamily.hpp"
#include "tools/vecmath.hpp"
#include <memory>
#include <vector>

class ModularBody;
class BodyModule;
class ObjL;
class Set;
class Texture;
template <typename T> class SharedBuffer;

// ============================================================================
// ShadowService - the shadow-map production half of G7 (S5, composition-typed
// since 2026-07-16), Renderer-owned. Design + full old-path mapping:
// shadow-paths.md B. Responsibilities:
//
// - LAYER POOL: N blurred shadow layers (R8 array, N = max_shadow_cast, D5
//   parameter). A layer is the LIGHT-TRANSMISSION silhouette of one
//   (caster body, PROJECTING MODULE) pair - module-granular, because
//   per-entry receiver application multiplies transmissions (products
//   commute: per-module layers compose exactly like one combined caster
//   map) while keeping per-module absorbtion and within-body exclusion
//   expressible (ShadowProjection.hpp). Keyed by (caster, source module,
//   penumbra radius px, caster-local light direction) with the old
//   DrawHelper::drawShadower cache semantics ported: reuse while |dRadius| <
//   SHADOW_RADIUS_TOLERANCE px and light-direction cos^2 >=
//   SHADOW_INVALIDATING_ANGLE; two-phase aging (unused one frame ->
//   free-able, the old beginDraw walk). The light direction key is
//   CASTER-LOCAL (transpose(rot(mat)) * (L - C)): camera-independent, like
//   the old heliocentric getLocalSunDirection - an eye-space key would
//   invalidate (or worse, silently mismatch) cached layers on camera motion.
// - TYPED PRODUCTION - the composition-type seam. Every type shares one
//   mechanism (R8 coverage silhouette -> disc blur -> pooled layer ->
//   scalar-coverage x absorbtion application); a type is a WORD of the job
//   vocabulary: what geometry writes the silhouette and what value it
//   writes. Current words:
//     * OPAQUE_MESH (BMT_PROJECT_G1_SHADOW): the caster mesh through
//       SHADOW_SHAPE (shadow_trace.vert REUSED + shadow_shape.frag writing
//       coverage 1).
//     * TEXTURED_ANNULUS (BMT_PROJECT_G8_SHADOW): a vertex-less quad in the
//       caster's equatorial plane through SHADOW_RING, frag = radial ring
//       texture alpha (graded transmission).
//   Open axis, deliberately: BMT_PROJECT_BISHADOW gets its word (and, if
//   bicolor requires it, a second layer channel) when its semantics converge
//   (suspended for Vixy - shadow-paths.md D1); the vocabulary is where it
//   lands, nothing else moves.
//   All words render into the shared R8 scratch (context.shadowShape /
//   renderShadowShape, PassKind::SHADOW_SHAPE) composited coverage-over
//   (1 - T_total = c_src + c_dst(1 - c_src)), then one disc-blur dispatch
//   (SHADOW_BLUR COMPUTE bank, radius = variant key; shadowBlur.comp reads
//   the R8 as float quantized to 0..255 so the sliding-window accumulator
//   stays EXACT integer arithmetic - G1 layers are bit-identical to the
//   stencil-era output, G8 grading carries at the layer's own 8-bit
//   precision). The S5-era stencil target (B3 candidate B) was binary by
//   construction; its preconditions shifted when G8 forced a float target
//   to exist - recorded in shadow-paths.md B3.
// - RECORDING WINDOW: jobs are declared during the frame (indexed by the
//   Vulkan frameIdx) and recorded in the PRE-COLOR window on the helper
//   thread (DrawHelper preFrameRecorder hook - exactly where the old path
//   records its shadow passes). This jobs-as-data shape IS the S4 seam: the
//   dedicated compute thread (RenderChain.hpp thread model) will consume the
//   same jobs; consumers (receivers sampling the layer array) never change.
//   Jobs stay DATA (no callbacks into module code at record time) for the
//   same reason.
// - The silhouette scratch target, render pass and resolution are the
//   Context ones (shadowShape / renderShadowShape / shadowRes - path-neutral
//   app infrastructure, shared serially within the same recording window).
//   The LAYER array is service-owned: sharing the old path's layers would
//   cross the two paths' caches during the dual-path phase.
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

    // Acquire a layer for (caster, source module, radiusPx,
    // casterLocalLightDir). Returns the layer index, or -1 when the pool is
    // exhausted (the caller selects in occlusion order, so what drops is the
    // least significant shadow - replaces the old "unpredictible failsafe"
    // slot steal) or when the blur pipeline for that radius is not resident
    // yet (C3: skip this frame, the bank is still building - old parity: no
    // shadow before shadow_ready). *reused reports a cache hit (no
    // production needed).
    int acquire(ModularBody *caster, const BodyModule *source, float radiusPx,
                const Vec3f &casterLocalLightDir, bool *reused);

    // OPAQUE_MESH word (G1): write the silhouette matrix of a newly-assigned
    // layer (mat3 mapping the caster mesh to shadow-map NDC - shadow-paths.md
    // B2) and queue its production: mesh recorded in the silhouette pass
    // (coverage 1 per fragment), then blurred.
    void produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh);

    // TEXTURED_ANNULUS word (G8): same matrix contract; the silhouette is a
    // vertex-less unit quad in the caster's equatorial plane (z=0), frag =
    // ring texture alpha over the radial band [innerRatio, 1] (r normalized
    // to the module's OUTER radius - the ShadowCaster radius the silhouette
    // matrix was built with). texSet = the module's SHADOW_RING set 1
    // (makeAnnulusTexSet), which must outlive the frame (module-owned - I5a).
    void produceAnnulus(int idx, const Mat4f &silhouetteMat, Set *texSet, float innerRatio);

    // Allocate a SHADOW_RING set 1 holding the caster's ring texture
    // (COMBINED_IMAGE_SAMPLER binding 0; the sampler is the texture's own -
    // ring strips want the default CLAMP). Module-owned; create lazily at
    // first drawShadow (the service exists by then - computeShadows gates on
    // it). Returns nullptr while the service is uninitialized.
    std::unique_ptr<Set> makeAnnulusTexSet(Texture &tex);

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
        const BodyModule *source = nullptr; // the projecting module (layer key half)
        float radiusPx = 0;
        Vec3f lightDir {}; // caster-local (camera-independent cache key)
        bool used = false;
        int radius = 0;    // int blur radius (spec key)
        std::unique_ptr<SharedBuffer<BlurUniform>> uniform;
        std::unique_ptr<SharedBuffer<float[12]>> traceMat; // mat3, std140
        std::unique_ptr<Set> traceSet; // SHADOW_SHAPE/SHADOW_RING set 0 (shared contract)
        std::unique_ptr<Set> blurSet;  // SHADOW_BLUR set 0
    };
    // The typed-job vocabulary (data-only - the S4 handoff shape).
    struct Job {
        enum class Kind : uint8_t { OPAQUE_MESH, TEXTURED_ANNULUS };
        uint8_t slot;
        Kind kind;
        ObjL *mesh;        // OPAQUE_MESH
        Set *texSet;       // TEXTURED_ANNULUS (module-owned, outlives the frame)
        float innerRatio;  // TEXTURED_ANNULUS: inner/outer radius
    };
    std::vector<Slot> slots;
    std::vector<Job> jobs[3];   // per Vulkan frameIdx; fencing guards reuse
    std::vector<VkImageView> layerViews;
    std::unique_ptr<Texture> layers; // R8 array, service-owned
    PipelineFamily shapeFamily;
    PipelineFamily ringFamily;  // TEXTURED_ANNULUS silhouettes (SHADOW_RING)
    PipelineFamily blurFamily;
    Renderer *renderer = nullptr;
    uint32_t maxRadius = 0;
    uint8_t curFrame = 0;
    bool inited = false;
    bool layersInitialized = false; // first-record layout transition
    bool exhaustedLogged = false;
};

#endif /* end of include guard: SHADOW_SERVICE_HPP_ */
