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

class ShadowService {
public:
    ShadowService();
    ~ShadowService();

    // New-path shadow switch (see header block).
    static bool enabled;

    // Frame entry (called from Renderer::beginDraw): two-phase slot aging +
    // job list reset for this frame index.
    void beginFrame(uint8_t frameIdx);

    int acquire(ModularBody *caster, const BodyModule *source, float radiusPx,
                const Vec3f &casterLocalLightDir, bool *reused);

    void produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh);

    void produceAnnulus(int idx, const Mat4f &silhouetteMat, Set *texSet, float innerRatio);

    void produceOjm(int idx, const Mat4f &silhouetteMat, Ojm *model);

    void produceSelfDepth(const Mat4f &m, Ojm *model);

    std::unique_ptr<Set> makeAnnulusTexSet(Texture &tex);

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
