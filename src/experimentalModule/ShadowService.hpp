#ifndef SHADOW_SERVICE_HPP_
#define SHADOW_SERVICE_HPP_

#include "PipelineFamily.hpp"
#include "tools/vecmath.hpp"
#include <memory>
#include <vector>

class ModularBody;
class BodyModule;
class ObjL;
class Ojm;
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
//     * OPAQUE_OJM (BMT_PROJECT_G1_SHADOW, Ojm-model bodies): the model
//       geometry through the SAME SHADOW_SHAPE family - Ojm and ObjL share
//       the vertex layout (context.ojmVertexArray) and the silhouette
//       matrix contract; only the geometry supplier differs
//       (Ojm::drawShadow binds its own buffers). Added 2026-07-16 with the
//       OJM port - the first post-design word, landing exactly as the
//       vocabulary promised: a Job kind + a record case, nothing else moved.
//   The former open axis (BMT_PROJECT_BISHADOW) RESOLVED + DISSOLVED
//   2026-07-18 [vixy: umbra/antumbra zones]: the layer is R8G8 - R = mean
//   sun-occlusion coverage (the historical channel, value-preserved), G =
//   exact true-umbra fraction (integer-accumulator saturation test, exact
//   for binary silhouettes; graded content triggers in fully-opaque cores
//   only). Every solid caster carries the structure - no separate word.
//   Composition: receivedShadows.glsl (physical-sharp, T = 1 - c*aT + u*gR).
// - SELF-SHADOW PRODUCTION (2026-07-16, first client = OJM): the per-frame
//   depth render of the nominated body's own geometry into the self-shadow
//   depth target (context.shadowBuffer / renderSelfShadow - path-neutral app
//   infrastructure shared SERIALLY with the old path, the shadowShape
//   precedent). Rides the SAME jobs-as-data shape and the SAME pre-color
//   recording window as layer production - deliberately: this whole
//   production side is the S4 seam (the dedicated compute thread consumes
//   the job lists unchanged), and self-shadow recorded ad-hoc at draw time
//   would be a second mechanism for S4 to re-plumb (the exact class the
//   composition-typed rework removed). Recording order within the window:
//   self-depth jobs FIRST, then layer jobs (old DrawHelper::submit order).
//   Consumption stays with the receiving module (PCF in its COLOR frag);
//   the production matrix and the consumption matrix are the SAME value,
//   computed once by the nomination (ModularSystem::computeShadows) and
//   handed to both sides - the old path guaranteed this consistency by
//   aliasing one buffer (OjmShadowFrag's leading mat3 doubling as the
//   shadow_trace binding); the new path guarantees it by single
//   computation, structurally rather than incidentally.
//   Targets: ONE MAIN target today (old parity: exactly one self-shadowed
//   body, the highest-importance one). The MAIN/SECONDARY resolution ladder
//   (ModularBody.hpp constants) extends HERE - a job carries its slot, so
//   buckets land without re-plumbing; SECONDARY arrives with the S3
//   depth-partitioning consumer (the grounded-slice-prefill dual purpose
//   needs it; shadow-paths.md D5).
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

    // OPAQUE_OJM word (G1, Ojm-model bodies): same matrix contract as
    // OPAQUE_MESH (the caller folds the model's own unit normalization into
    // silhouetteMat - Ojm raw vertices span the model's own radius, ObjL's
    // span the unit sphere); geometry recorded via Ojm::drawShadow. The Ojm
    // is module-owned and must outlive the frame (I5a - Ojm lifetimes are
    // application-long, D6 build-once).
    void produceOjm(int idx, const Mat4f &silhouetteMat, Ojm *model);

    // SELF-SHADOW production (header block). Called by the nomination
    // (ModularSystem::computeShadows) at most once per frame today (single
    // MAIN target - old parity). m = the model->sun-frame-NDC matrix (mat3
    // semantics, same contract as the silhouette words - shadow_trace.vert
    // consumes rows, z lands in [0,1] via the vert's *0.5+0.5); model = the
    // nominated module's geometry. The SAME matrix value must be what the
    // consuming fragment projects with (single-computation consistency,
    // header block). No-op while the service is uninitialized.
    void produceSelfDepth(const Mat4f &m, Ojm *model);

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
        // Integer saturation value of the blur's quantized accumulator
        // ((4*sum(offsets)+1)*255): sum == fullCount iff EVERY texel of the
        // disc reads 255 - the exact full-occlusion (umbra) test feeding the
        // layer's G channel (2026-07-18 two-channel rework [vixy]). Exact for
        // binary silhouettes; for graded (G8) content it detects only the
        // fully-opaque core - true umbra by definition there too.
        int fullCount;
        float _pad[2];
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
        enum class Kind : uint8_t { OPAQUE_MESH, TEXTURED_ANNULUS, OPAQUE_OJM, SELF_DEPTH };
        uint8_t slot;      // layer slot; unused for SELF_DEPTH (single MAIN target today)
        Kind kind;
        ObjL *mesh;        // OPAQUE_MESH
        Ojm *ojm;          // OPAQUE_OJM / SELF_DEPTH (module-owned, application-long - D6)
        Set *texSet;       // TEXTURED_ANNULUS (module-owned, outlives the frame)
        float innerRatio;  // TEXTURED_ANNULUS: inner/outer radius
    };
    std::vector<Slot> slots;
    std::vector<Job> jobs[3];   // per Vulkan frameIdx; fencing guards reuse
                                // (SELF_DEPTH jobs record FIRST - old submit order)
    std::vector<VkImageView> layerViews;
    std::unique_ptr<Texture> layers; // R8 array, service-owned
    PipelineFamily shapeFamily;
    PipelineFamily ringFamily;  // TEXTURED_ANNULUS silhouettes (SHADOW_RING)
    PipelineFamily selfFamily;  // SELF_DEPTH pass (SELF_SHADOW PassKind; shares
                                // the trace SetContract - one mat3 contract for
                                // every geometry word)
    PipelineFamily blurFamily;
    // SELF_DEPTH matrix buffer/set (single MAIN target today - header block;
    // per-bucket when the SECONDARY ladder lands with S3).
    std::unique_ptr<SharedBuffer<float[12]>> selfMat;
    std::unique_ptr<Set> selfSet;
    Renderer *renderer = nullptr;
    uint32_t maxRadius = 0;
    uint8_t curFrame = 0;
    bool inited = false;
    bool layersInitialized = false; // first-record layout transition
    bool exhaustedLogged = false;
};

#endif /* end of include guard: SHADOW_SERVICE_HPP_ */
