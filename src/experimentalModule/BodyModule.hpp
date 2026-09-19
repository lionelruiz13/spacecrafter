#ifndef BODY_MODULE_HPP_
#define BODY_MODULE_HPP_

#include "tools/vecmath.hpp"
#include <cstdint>
#include <iosfwd>
#include <string>

class ModularBody;
class Renderer;

// Selects the loader family of a module (ModuleLoaderMgr); its slot on a body is another identity (ModularBody::slotID)
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

// HALO is owned by the body, LABEL/ORBIT/TRAIL by the HINT/ORBIT/TRAIL modules; NONE = unknown name, a no-op
enum class BodyColorType : uint8_t { HALO, LABEL, ORBIT, TRAIL, ALL, NONE };

inline BodyColorType parseBodyColorType(const std::string &name) {
    if (name == "halo")  return BodyColorType::HALO;
    if (name == "label") return BodyColorType::LABEL;
    if (name == "orbit") return BodyColorType::ORBIT;
    if (name == "trail") return BodyColorType::TRAIL;
    if (name == "all")   return BodyColorType::ALL;
    return BodyColorType::NONE;
}

// Declarative needs of a module: a pass whose trait is not declared is never received
// Bodies of BodyType::MINOR_BODY never take part in inter-body shadowing, whatever their traits
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
    BMT_PROJECT_G8_SHADOW =     0x00000100, // Project greyscale transmission shadow (graded casters: rings)
    BMT_TRANSLUCENT =           0x00000800, // COLOR output blends: drawn after every opaque module of its regime list
    BMT_RECEIVE_SHADOW =        0x00000400, // Samples the body's receivedShadows in its COLOR draw, never its own
};

// What one projecting module casts: one shadow layer per (body, projecting module)
struct ShadowCaster {
    float radius; // Silhouette extent in scaled AU: body radius for a mesh (not boundingRadius), outer radius of a ring
    Vec3f absorbtion; // Per channel, 1 = fully absorbed under full coverage
    Vec4f clip; // Eye-space plane: applies only where dot(P, xyz) + w <= 0; (0,0,0,-1) = always (solid casters)
};

// A feature in a ModularBody slot: knows how to draw itself, never when. draw*() must work with partial resources
// May be destroyed between frames (body reload): release resources through their deferred-release channels only
class BodyModule {
public:
    BodyModule(BodyModuleType type = BodyModuleType::CUSTOM) : type(type) {}
    virtual ~BodyModule() = default;
    // Return true if this body module is loaded
    virtual bool isLoaded() {
        return true;
    }
    // A hint, never a stall. keepFrames = how long the preloaded resolution stays resident while unused
    virtual void preload(ModularBody *body, int keepFrames) {}
    // Update this body module, return true when update is no longer required for this body
    // Ordering guarantee: update() must have run at least once before compare() or
    // getBoundingRadius() - boundingRadius is undefined until then.
    virtual bool update(ModularBody *body, float scaledRadius) {
        boundingRadius = scaledRadius;
        return true;
    }
    virtual uint32_t getTraits() const {
        return 0;
    }
    // The four drawing types (each hook = one pass kind, gated by traits):
    // 1. COLOR - the visible image.
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) = 0;
    // 1b. COLOR, minimalist depth-less variant - used at small screen sizes
    //     where depth-correct drawing is indistinguishable.
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // 2. SHADOW - declare to the ShadowService the silhouette cast on other surfaces (mat), into its layer idx.
    //    Gated by BMT_PROJECT_*
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    // 2b. Caster descriptor, only called with a BMT_PROJECT_* trait; the default is a solid whole-body caster
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const;
    // 3. SELF-SHADOW - declare the self-shadow depth job (mat = model -> sun-frame NDC) and keep mat for this frame's
    //    draw(), which must project with the same value. Gated by BMT_*_SELF_SHADOW, only called on the nominated body
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // 4. TRACE - depth hole where the body hides the orbit lines. Gated by BMT_DEPTH_TRACE
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Per-body commands: a module answers for what it owns and ignores the rest (a getter returns false = not mine)
    virtual void setShown(bool shown) {}
    // Called once when the body is shown again, its position already at the current date: nothing ran while hidden,
    // so rebuild what time would have moved; where the past is not computable, restart and log it
    virtual void resumeAfterHidden(ModularBody *body) {}
    // Creating or replacing a skin never activates it; switchTexSkin(true) without a skin is a no-op
    virtual void createTexSkin(const std::string &texName) {}
    virtual void switchTexSkin(bool use) {}
    virtual void setColor(BodyColorType type, const Vec3f &c) {}
    virtual bool getColor(BodyColorType type, Vec3f &out) const { return false; }
    // The color the data gave, whatever was set since
    virtual bool getAuthoredColor(BodyColorType type, Vec3f &out) const { return false; }
    virtual bool getSkinUse(bool &out) const { return false; }
    // -1 = follows the global flag, 0/1 = forced for this body (the override, not the effective visibility)
    virtual int getShownOverride() const { return -1; }
    // Snapshot what the data gave this module; called once the load has finished writing into the body
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
    virtual void dumpState(std::ostream &out) const;
    inline BodyModuleType getType() const {
        return type;
    }
protected:
    // Smallest radius of a sphere enclosing everything this module draws: no margin is ever needed on top of it
    float boundingRadius = 0;
    BodyModuleType type;
};

#endif /* end of include guard: BODY_MODULE_HPP_ */
