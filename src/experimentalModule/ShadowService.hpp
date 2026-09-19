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

// Produce one blurred shadow layer per (caster body, projecting module)
class ShadowService {
public:
    ShadowService();
    ~ShadowService();

    static bool enabled; // experimental_shadows config key

    void beginFrame(uint8_t frameIdx);

    // Return the layer or -1 if none is left: call in occlusion order. *reused = nothing to produce
    int acquire(ModularBody *caster, const BodyModule *source, float radiusPx,
                const Vec3f &casterLocalLightDir, bool *reused);

    // silhouetteMat (as mat3) maps the caster to shadow-map NDC
    void produce(int idx, const Mat4f &silhouetteMat, ObjL *mesh);

    // Ring alpha over [innerRatio, 1] of the outer radius; texSet must outlive the frame
    void produceAnnulus(int idx, const Mat4f &silhouetteMat, Set *texSet, float innerRatio);

    // Fold the unit normalization of model in silhouetteMat; model must outlive the frame
    void produceOjm(int idx, const Mat4f &silhouetteMat, Ojm *model);

    // m must be the very mat3 the consuming fragment projects with; at most once per frame
    void produceSelfDepth(const Mat4f &m, Ojm *model);

    // Return nullptr while uninitialized
    std::unique_ptr<Set> makeAnnulusTexSet(Texture &tex);

    // Helper thread; cmd is the primary buffer of the frame, outside any pass
    void record(VkCommandBuffer cmd, uint8_t frameIdx);

    Texture *layerArray() {
        return layers.get();
    }
    inline operator bool() const {
        return inited;
    }
    // Need the Context fully built
    void ensureInit(Renderer &renderer);
    // Call while the managers are alive
    void release();

private:
    // std140 mirror of shadowBlur.comp binding 0
    struct BlurUniform {
        float pixelCount;
        int fullCount; // Accumulator value when the whole disc is occluded
        float _pad[2];
        struct PaddedInt {
            int v;
            int _pad[3];
        } offsets[511];
    };
    struct Slot {
        ModularBody *caster = nullptr;
        const BodyModule *source = nullptr;
        float radiusPx = 0;
        Vec3f lightDir {}; // Caster-local
        bool used = false;
        int radius = 0;    // Blur radius, specialization key
        std::unique_ptr<SharedBuffer<BlurUniform>> uniform;
        std::unique_ptr<SharedBuffer<float[12]>> traceMat; // mat3, std140
        std::unique_ptr<Set> traceSet; // SHADOW_SHAPE/SHADOW_RING set 0
        std::unique_ptr<Set> blurSet;
    };
    // Data only, nothing calls into module code at record time
    struct Job {
        enum class Kind : uint8_t { OPAQUE_MESH, TEXTURED_ANNULUS, OPAQUE_OJM, SELF_DEPTH };
        uint8_t slot;      // Unused for SELF_DEPTH
        Kind kind;
        ObjL *mesh;
        Ojm *ojm;          // OPAQUE_OJM / SELF_DEPTH
        Set *texSet;
        float innerRatio;
    };
    std::vector<Slot> slots;
    std::vector<Job> jobs[3];   // Per frameIdx, SELF_DEPTH jobs are recorded first
    std::vector<VkImageView> layerViews;
    std::unique_ptr<Texture> layers; // R = mean occlusion coverage, G = full-umbra fraction
    PipelineFamily shapeFamily;
    PipelineFamily ringFamily;
    PipelineFamily selfFamily;  // SELF_SHADOW pass, shares the trace SetContract
    PipelineFamily blurFamily;
    std::unique_ptr<SharedBuffer<float[12]>> selfMat; // One self-shadowed body per frame
    std::unique_ptr<Set> selfSet;
    Renderer *renderer = nullptr;
    uint32_t maxRadius = 0;
    uint8_t curFrame = 0;
    bool inited = false;
    bool layersInitialized = false;
    bool exhaustedLogged = false;
};

#endif /* end of include guard: SHADOW_SERVICE_HPP_ */
