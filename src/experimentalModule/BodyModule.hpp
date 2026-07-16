#ifndef BODY_MODULE_HPP_
#define BODY_MODULE_HPP_

#include "tools/vecmath.hpp"
#include <cstdint>

class ModularBody;
class Renderer;

// The loader-family selector: which competing loader pool handles this module
// kind (ModuleLoaderMgr keys its loader arrays on it). Slot IDENTITY on a body
// is a separate concept, carried by StringID (ModularBody::slotID) - one type
// may fill several named slots.
enum class BodyModuleType : uint8_t {
    CUSTOM, // For modules without a specific role
    MESH,
    OJM,
    VOLUMETRIC,
    RING,
    HINT,
    POINTER,
    ORBIT,
    TRAIL,
    TAIL,
    ATMOSPHERE,
    AXIS,
    NB_MODULE_TYPE // For array size
};

enum class RelativePosition : uint8_t {
    ANY = 0x0, // Relative position has no importance : they doesn't overlap on screen
    FRONT = 0x1,
    OVERLAP = 0x2,
    BACK = 0x4,
};

// Declarative rendering-pipeline needs of a module. The Renderer reads traits
// to gate passes and select pipeline families - modules never touch the
// pipeline directly. A trait a module doesn't declare is a pass it never
// receives. Note: bodies of BodyType::MINOR_BODY (mass-instanced, e.g. an
// upscaled asteroid ring) are exempt from inter-body shadowing regardless of
// their modules' shadow traits - in reality too small for it to be observable.
enum BodyModuleTraits {
    BMT_REPLICATED =            0x00000001, // Heavily used, some batching or similar strategy is recommended
    BMT_CACHED =                0x00000002, // Some state should be cached for performance, maybe a bad idea ?
    BMT_DYNAMIC =               0x00000004, // Use dynamic resolution mechanism
    BMT_DEPTH_TRACE =           0x00000008, // Affect the depth buffer
    BMT_USE_DEPTH =             0x00000010, // Use the depth buffer (pointer/halo doesn't)
    BMT_BASIC_SELF_SHADOW =     0x00000020, // Project monochrome self-shadowing
    BMT_RGBA8_SELF_SHADOW =     0x00000040, // Project RGBA8 self-shadowing
    BMT_PROJECT_G1_SHADOW =     0x00000080, // Project monochrome shadow
    BMT_PROJECT_G8_SHADOW =     0x00000100, // Project greyscale shadow (up to 10 is possible)
    BMT_PROJECT_BISHADOW =      0x00000200, // Project bicolor shadow - ONLY WORKS WHEN 10 TIMES CLOSER
    BMT_RECEIVE_SHADOW =        0x00000400, // Samples the body's receivedShadows in its COLOR
                                            // draw (ShadowProjection.hpp). Declared so the
                                            // orchestration knows WHO receives: bodies with no
                                            // receiving module get no entries (no dead fills),
                                            // and within-body projection (ring<->planet, one
                                            // ModularBody) pairs a projecting module with the
                                            // OTHER receiving modules - never with itself (a
                                            // surface never samples a layer containing its own
                                            // silhouette; see ModularSystem::computeShadows).
};

// What a projecting module casts - the per-MODULE half of the shadow-caster
// vocabulary (the per-BODY half - position, light corridor - stays in the
// orchestration). One caster layer is produced per (body, projecting module):
// per-entry application multiplies (1 - cov_i * absorbtion_i) over entries,
// and products commute, so per-module layers compose EXACTLY like one
// combined transmission map ((1-c_mesh)(1-c_ring) = T_mesh * T_ring) while
// keeping per-module absorbtion and within-body exclusion expressible.
struct ShadowCaster {
    // Silhouette extent (AU, scaled): the disc the layer maps. Mesh modules:
    // the body radius (NOT boundingRadius - a shell-inflated bounding radius
    // would shrink the silhouette in its layer for no coverage gain).
    // Ring modules: the OUTER ring radius.
    float radius;
    // Per-channel shadow absorption of THIS module's casting (1 = channel
    // fully absorbed under full coverage). Mesh modules default to the body's
    // shadowAbsorbtion (Earth {0,1,1} -> red umbra). Rings: the transparency
    // grading lives in the layer COVERAGE; absorbtion carries the old-parity
    // darkening depth (mix(1.0, 0.3, alpha) == 1 - alpha*0.7 -> {0.7,0.7,0.7}).
    Vec3f absorbtion;
    // Receiver-side half-space gate, eye-space plane (xyz, w): the entry
    // applies only where dot(P, xyz) + w <= 0. Solid casters: the DEGENERATE
    // plane (0,0,0,-1) - always applies (selection's light-corridor test
    // already carries their z-order). PLANAR casters (rings) need it because
    // the blurred layer is z-less: a ray crossing the ring plane BEHIND the
    // surface it hits must not shade it. The exact rule for any receiver
    // point P: shade iff P lies on the anti-sun side of the caster's plane
    // (crossing-before-hit <=> hit beyond the plane), i.e. xyz = plane normal
    // oriented toward the sun, w = -dot(xyz, casterCenter). Derivation +
    // the sphere no-false-band proof: ShadowProjection.hpp.
    Vec4f clip;
};

// The unit of feature composition: a drawable/loadable feature attached to a
// ModularBody slot. A module knows HOW to draw itself in a given regime, never
// WHEN - regime selection (far/near/grounded/in, by screen size and distance)
// belongs to ModularBody. Modules read body state through the public interface
// only; a module needing privileged body access signals a missing accessor,
// not a friend candidate.
// Lifecycle: isLoaded() is a non-blocking query; preload() is a hint, never a
// stall; draw*() must be callable with partial resources (the drawLoaded path
// draws whatever is resident - together with the always-resident lowest LoD,
// something drawable always exists).
class BodyModule {
public:
    BodyModule(BodyModuleType type = BodyModuleType::CUSTOM) : type(type) {}
    // Return true if this body module is loaded
    virtual bool isLoaded() {
        return true;
    }
    // Preload content for use in a near future
    virtual void preload(ModularBody *body) {}
    // Update this body module, return true when update is no longer required for this body
    // Ordering guarantee: update() must have run at least once before compare() or
    // getBoundingRadius() - boundingRadius is undefined until then.
    virtual bool update(ModularBody *body, float scaledRadius) {
        boundingRadius = scaledRadius;
        return true;
    }
    // Declarative trait bits (BodyModuleTraits): read by the Renderer for
    // pass routing and by the ModularSystem shadow orchestration for caster
    // selection (a module never declares = a pass never received).
    virtual uint32_t getTraits() const {
        return 0;
    }
    // The four drawing types (each hook = one pass kind, gated by traits):
    // 1. COLOR - the visible image.
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) = 0;
    // 1b. COLOR, minimalist depth-less variant - used at small screen sizes
    //     where depth-correct drawing is indistinguishable.
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // 2. SHADOW - the silhouette this module projects onto OTHER surfaces
    //    (idx = layer in the ShadowService pool; mat = the silhouette matrix
    //    computed by the orchestration - shadow-paths.md B2). Gated by
    //    BMT_PROJECT_* traits. Sync-interim contract [S5]: called on the
    //    frame path; the module DECLARES its geometry to the service
    //    (ShadowService::produce/produceAnnulus - the typed-job vocabulary;
    //    each composition type is a vocabulary word, the pool/blur/receiver
    //    machinery is shared), which records it in the pre-color window
    //    - the jobs-as-data shape the S4 compute thread consumes unchanged.
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    // 2b. Caster descriptor consumed by the selection (one layer + one
    //     receiver entry per (body, projecting module) - see ShadowCaster).
    //     Only called on modules declaring a BMT_PROJECT_* trait. The default
    //     covers solid whole-body casters (G1 mesh): body radius, body
    //     shadowAbsorbtion, no clip. Defined in ModularBody.cpp (needs the
    //     ModularBody definition).
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const;
    // 3. SELF-SHADOW - dual purpose: (a) fill the self-shadow depth buffer;
    //    (b) prefill the depth-buffer slice of grounded bodies with this
    //    parent body - a grounded body's whole depth slice is negligible at
    //    parent scale (D1), so parent-vs-grounded occlusion is wrong without
    //    the prefill. Gated by BMT_*_SELF_SHADOW traits.
    //    Contract (2026-07-16, first client = OJM): called by the shadow
    //    NOMINATION (ModularSystem::computeShadows - old CoI parity: the
    //    highest-importance drawn body with a *_SELF_SHADOW module), not by
    //    a pass walk. mat = the model->sun-frame-NDC matrix (the same sun
    //    basis as the received-shadow rows; unit-geometry convention like
    //    drawShadow - the module folds its own geometry normalization). The
    //    module (a) declares its depth job to the ShadowService
    //    (produceSelfDepth - jobs-as-data, S4 seam) and (b) keeps {mat,
    //    active-this-frame} for its COLOR fill: production and consumption
    //    must project with the SAME matrix value (single-computation
    //    consistency - ShadowService.hpp header). The active flag is
    //    frame-scoped: set here, consumed and cleared by the module's own
    //    draw() the same frame; a frame without nomination leaves it unset.
    //    Dual purpose (b) arrives with the S3 depth-partitioning consumer
    //    (shadow-paths.md D5) - the job shape already carries it.
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // 4. TRACE - depth-like pass cutting a hole where the orbit line must be
    //    hidden by the body (old-path analog: drawOrbit into cmdBodyDepth).
    //    Gated by BMT_DEPTH_TRACE.
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Compare an object position and radius with this ModularBody
    // Precondition: update() has run at least once for this module (see update)
    virtual RelativePosition compare(const Vec3f &localPos, const Vec3f &zAxis, float radius) {
        const float distance = localPos.lengthSquared();
        radius += boundingRadius;
        radius *= radius;
        if (distance > radius) {
            const float tmp = localPos.dot(zAxis);
            if ((localPos - zAxis * tmp).lengthSquared() > radius)
                return RelativePosition::ANY;
            return (tmp > 0) ? RelativePosition::BACK : RelativePosition::FRONT;
        } else {
            return RelativePosition::OVERLAP;
        }
    }
    inline float getBoundingRadius() const {
        return boundingRadius;
    }
protected:
    // The smallest radius of a sphere enclosing the WHOLE body to be traced
    // [vixy: 2026-07-11]. This single definition is what makes one value valid
    // for BOTH the visibility cone test and the depth-slice bounds - inclusive
    // by definition, so no margin factor is ever needed on top of it.
    // Defined by update(); 0 = never updated yet.
    float boundingRadius = 0;
    BodyModuleType type;
};

#endif /* end of include guard: BODY_MODULE_HPP_ */
