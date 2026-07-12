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
    // 2. SHADOW - the silhouette this body projects onto OTHER bodies
    //    (idx = layer in the ShadowService pool; mat = the silhouette matrix
    //    computed by the orchestration - shadow-paths.md B2). Gated by
    //    BMT_PROJECT_* traits. Sync-interim contract [S5]: called on the
    //    frame path; the module DECLARES its geometry to the service
    //    (ShadowService::produce), which records it in the pre-color window
    //    - the jobs-as-data shape the S4 compute thread consumes unchanged.
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    // 3. SELF-SHADOW - dual purpose: (a) fill the self-shadow depth buffer;
    //    (b) prefill the depth-buffer slice of grounded bodies with this
    //    parent body - a grounded body's whole depth slice is negligible at
    //    parent scale (D1), so parent-vs-grounded occlusion is wrong without
    //    the prefill. Gated by BMT_*_SELF_SHADOW traits.
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
