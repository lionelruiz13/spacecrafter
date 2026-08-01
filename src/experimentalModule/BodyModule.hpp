#ifndef BODY_MODULE_HPP_
#define BODY_MODULE_HPP_

#include "tools/vecmath.hpp"
#include <cstdint>
#include <iosfwd>
#include <string>

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

// The runtime per-body color channels (old BodyColor::TYPE_COLOR). Each channel
// is owned by exactly one place: HALO by the body (ModularBody::haloColor,
// consumed by drawHalo), LABEL/ORBIT/TRAIL by the HINT/ORBIT/TRAIL modules. ALL
// sets every channel; NONE = the token did not name a channel (no-op, old
// searched the same set and warned - the warning still fires on the old side).
enum class BodyColorType : uint8_t { HALO, LABEL, ORBIT, TRAIL, ALL, NONE };

// String -> channel, the SEAM's translation (old BodyColor::translate, minus
// its warning: the old path still emits it on the dual-write, so the new side
// stays silent to avoid a double log). The command surface is the same strings
// (`body name X color <halo|label|orbit|trail|all> value r,g,b`).
inline BodyColorType parseBodyColorType(const std::string &name) {
    if (name == "halo")  return BodyColorType::HALO;
    if (name == "label") return BodyColorType::LABEL;
    if (name == "orbit") return BodyColorType::ORBIT;
    if (name == "trail") return BodyColorType::TRAIL;
    if (name == "all")   return BodyColorType::ALL;
    return BodyColorType::NONE;
}

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
    BMT_PROJECT_G1_SHADOW =     0x00000080, // Project opaque-silhouette shadow (solid casters;
                                            // umbra/antumbra structure carried by the layer's
                                            // two channels - ShadowService.hpp)
    BMT_PROJECT_G8_SHADOW =     0x00000100, // Project greyscale TRANSMISSION shadow (graded
                                            // casters - rings; up to 10 simultaneous)
    // 0x00000200 (BMT_PROJECT_BISHADOW) RETIRED 2026-07-18: its semantics -
    // umbra/antumbra ("bicolor") zone structure resolving at close range
    // [vixy: 2026-07-18] - became UNIVERSAL for every solid caster with the
    // two-channel layer (R = mean coverage, G = true umbra; the physical-
    // sharp composition in receivedShadows.glsl). A capability every G1
    // caster now has is not a distinct trait; the bit value stays reserved
    // so old logs remain readable.
    BMT_TRANSLUCENT =           0x00000800, // COLOR output blends over sibling content: the
                                            // routing layer orders this module AFTER every
                                            // opaque module of the same regime list
                                            // (ModuleLoader::addNearComponent - the structural
                                            // form of the old explicit drawBody-then-drawRings
                                            // ordering). Declare on every blended module (RING,
                                            // ATMOSPHERE shell, future TAIL).
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
    // shadowAbsorbtion (Earth {0.6,0.88,1} - its complement is the red umbra).
    // Rings: the transparency grading lives in the layer COVERAGE; absorbtion
    // carries the material darkening depth (mix(1.0, 0.3, alpha) ==
    // 1 - alpha*0.7 -> {0.7,0.7,0.7}).
    // The SELECTION maps this declared value per word semantics into the
    // entry's (aT transmission, gR refraction-glow) pair - physical-sharp
    // composition, 2026-07-18 [vixy]; derivation: ShadowProjection.hpp.
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
// Destruction: a module is owned polymorphically (ModularBody::components is a
// vector<unique_ptr<BodyModule>>) and therefore deleted through THIS type, so
// the destructor must be virtual - without it every subclass destructor is
// skipped and the module's own resources (Sets, buffers, textures) are never
// released ([expr.delete]/3 UB; INTENT §5.51 measured it as 373 ASan
// new-delete-type-mismatch reports per shutdown, 746 across a reload). Free in
// layout: the class already has a vtable. What a subclass destructor may do is
// bounded by WHEN it runs: bodies die with the tree, i.e. before Context and
// its managers (the pools/BufferMgrs a Set or VertexBuffer releases into are
// still alive - Renderer::allocSet states the same invariant from the other
// side), but also mid-session on `body action reload`, where the release
// happens between frames rather than after a waitIdle - so a module must
// release only through the deferred-release channels its resources provide.
class BodyModule {
public:
    BodyModule(BodyModuleType type = BodyModuleType::CUSTOM) : type(type) {}
    virtual ~BodyModule() = default;
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
    // Per-body visibility override (old Body::setFlagOrbit -> orbit_fader).
    // Base no-op; ORBIT (and later TRAIL) route a per-name seam toggle here
    // via ModularBody's dedicated component list. NOT the four drawing types.
    virtual void setShown(bool shown) {}
    // The UNHIDE edge (B39 §11.117, from D23: "behave as if they never were
    // hidden when unhidden"). While its body was hidden this module received no
    // update() and no draw() at all - the whole subtree was outside every sweep -
    // so any state of this module that TIME would have moved is now behind.
    // Called once per module of the re-shown subtree, AFTER the body's position
    // has been brought to the current date (the D8 barrier), so a module may
    // rebuild from the body's own state or from its orbit.
    // The base no-op is the right answer for a module holding no time-dependent
    // state (the setShown/createTexSkin contract - no sniffing at the broadcast).
    // Where the past is NOT computable the override must degrade to a fresh start
    // and LOG it (§2.0 D12: a behaviour the author did not write must be visible).
    virtual void resumeAfterHidden(ModularBody *body) {}
    // Runtime skin-texture seam (old Body::createTexSkin/switchMapSkin;
    // commands `body name X skin_tex <file>` / `body name X skin_use on|off`).
    // Contract mirrors old exactly: creating (or REPLACING) a skin never
    // activates it (old resets tex_current to the map on replace); activation
    // requires an existing skin (switch(true) with no skin is a no-op).
    // Modules drawing the body color map override (MESH family); every other
    // module ignores (same default-no-op contract as setShown).
    virtual void createTexSkin(const std::string &texName) {}
    virtual void switchTexSkin(bool use) {}
    // Runtime per-body color seam (old BodyColor / Body::setColor; command
    // `body name X color <label|orbit|trail|all> value r,g,b`). Each color
    // module self-selects on the channel it owns (LABEL=Hint, ORBIT=Orbit,
    // TRAIL=Trail); every other module ignores it (default no-op, the
    // createTexSkin/setShown contract - no type sniffing at the broadcast).
    // HALO is body-owned and handled by ModularBody, not by any module.
    virtual void setColor(BodyColorType type, const Vec3f &c) {}
    // THE READ HALF of the three seams above (b31-design §2 rows D4/D7/D8; the
    // per-body twin of readFlag, INTENT §11.129). Same self-selection contract:
    // a module answers for the channel it owns and stays silent otherwise, so
    // the caller never sniffs a type to find out who holds what (I4). False =
    // "not mine", which is not the same answer as a value.
    virtual bool getColor(BodyColorType type, Vec3f &out) const { return false; }
    //! The same, for what the DATA gave it - so the ledger can record a change
    //! rather than a snapshot (D30).
    virtual bool getAuthoredColor(BodyColorType type, Vec3f &out) const { return false; }
    virtual bool getSkinUse(bool &out) const { return false; }
    // The per-body visibility OVERRIDE, not the effective visibility: -1 when
    // this body follows the global master (including when a global toggle has
    // since staled the override), 0/1 when an operator forced it. The ledger
    // records overrides, so it must be able to tell "forced off" from
    // "following a master that is off".
    virtual int getShownOverride() const { return -1; }
    // Snapshot what the DATA gave this module, so a later save can tell an
    // operator's change from an author's value (D30's delta rule). Called once
    // the load has finished writing into the body.
    virtual void captureAuthored() {}
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
    // Harness instrument (INTENT 11.56): write THIS module's own state as one
    // JSON value into the dual-path trace (ModularBody::dumpTrace). The module
    // owns its state, so the module writes it - the dump site never sniffs a
    // module's type to reach inside it (I4). Default = `null` (a module with
    // no externally-observable state contributes nothing to observe).
    // Defined in ModularBody.cpp (keeps <ostream> out of every module TU).
    virtual void dumpState(std::ostream &out) const;
    // Which FAMILY this module belongs to - the value the composed format's
    // `type=` key carries for it (ModuleLoaderMgr's own enumeration, I2). The
    // module says what it is; no caller sniffs its class to find out (I4).
    inline BodyModuleType getType() const {
        return type;
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
