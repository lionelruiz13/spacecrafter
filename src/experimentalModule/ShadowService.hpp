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

// Renderer-owned production of the blurred shadow layers: one layer per (caster body, projecting module) pair.
// Productions are queued as data during the frame, then recorded in the pre-color window on the helper thread.
// A layer holds R = mean occlusion coverage, G = full-umbra fraction; receivers sample layerArray().
class ShadowService {
public:
    ShadowService();
    ~ShadowService();

    // Shadow switch of this render path, initialized from the experimental_shadows config key
    static bool enabled;

    // Frame entry (called from Renderer::beginDraw): two-phase slot aging +
    // job list reset for this frame index.
    void beginFrame(uint8_t frameIdx);

    // Layer index for this key, or -1 if the pool is exhausted or the blur pipeline of this radius is not built yet.
    // Call in occlusion order: what drops is the least significant shadow. *reused = cached layer, nothing to produce.
    // casterLocalLightDir is in the caster frame, so the key does not depend on the camera
    int acquire(ModularBody *caster, const BodyModule *source, float radiusPx,
                const Vec3f &casterLocalLightDir, bool *reused);

    // Queue the production of layer idx from the caster mesh; silhouetteMat (as mat3) maps the mesh to shadow-map NDC
    void produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh);

    // Same, from a ring in the caster's equatorial plane: ring texture alpha over [innerRatio, 1] of the outer radius.
    // texSet comes from makeAnnulusTexSet, stays module-owned and must outlive the frame
    void produceAnnulus(int idx, const Mat4f &silhouetteMat, Set *texSet, float innerRatio);

    // Same as produce for an Ojm model, whose unit normalization the caller folds into silhouetteMat.
    // model stays module-owned and must outlive the frame
    void produceOjm(int idx, const Mat4f &silhouetteMat, Ojm *model);

    // Queue the self-shadow depth render of model, at most once per frame. m (as mat3) = model -> sun-frame NDC,
    // and must be the very value the consuming fragment projects with. No-op while uninitialized
    void produceSelfDepth(const Mat4f &m, Ojm *model);

    // Descriptor set holding the ring texture for produceAnnulus, owned by the caller; nullptr while uninitialized
    std::unique_ptr<Set> makeAnnulusTexSet(Texture &tex);

    // Record the productions queued for frameIdx. Helper thread; cmd = the frame's primary buffer, outside any pass
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
        // Accumulator value when every texel of the disc is fully occluded: the umbra test of the G channel
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
    // One queued production; data only, nothing calls into module code at record time
    struct Job {
        enum class Kind : uint8_t { OPAQUE_MESH, TEXTURED_ANNULUS, OPAQUE_OJM, SELF_DEPTH };
        uint8_t slot;      // layer slot; unused for SELF_DEPTH (single MAIN target today)
        Kind kind;
        ObjL *mesh;        // OPAQUE_MESH
        Ojm *ojm;          // OPAQUE_OJM / SELF_DEPTH (module-owned, outlives the frame)
        Set *texSet;       // TEXTURED_ANNULUS (module-owned, outlives the frame)
        float innerRatio;  // TEXTURED_ANNULUS: inner/outer radius
    };
    std::vector<Slot> slots;
    std::vector<Job> jobs[3];   // per Vulkan frameIdx; fencing guards reuse
                                // (SELF_DEPTH jobs record FIRST)
    std::vector<VkImageView> layerViews;
    std::unique_ptr<Texture> layers; // R8 array, service-owned
    PipelineFamily shapeFamily;
    PipelineFamily ringFamily;  // TEXTURED_ANNULUS silhouettes (SHADOW_RING)
    PipelineFamily selfFamily;  // SELF_DEPTH pass (SELF_SHADOW PassKind; shares
                                // the trace SetContract - one mat3 contract for
                                // every geometry word)
    PipelineFamily blurFamily;
    // SELF_DEPTH matrix buffer/set (one self-shadowed body per frame)
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
