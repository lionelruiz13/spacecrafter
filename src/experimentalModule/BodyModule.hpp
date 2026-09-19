#ifndef BODY_MODULE_HPP_
#define BODY_MODULE_HPP_

#include "tools/vecmath.hpp"
#include <cstdint>
#include <iosfwd>
#include <string>

class ModularBody;
class Renderer;

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

enum class BodyColorType : uint8_t { HALO, LABEL, ORBIT, TRAIL, ALL, NONE };

inline BodyColorType parseBodyColorType(const std::string &name) {
    if (name == "halo")  return BodyColorType::HALO;
    if (name == "label") return BodyColorType::LABEL;
    if (name == "orbit") return BodyColorType::ORBIT;
    if (name == "trail") return BodyColorType::TRAIL;
    if (name == "all")   return BodyColorType::ALL;
    return BodyColorType::NONE;
}

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
    BMT_TRANSLUCENT =           0x00000800, // COLOR output blends over sibling content: the
    BMT_RECEIVE_SHADOW =        0x00000400, // Samples the body's receivedShadows in its COLOR
};

struct ShadowCaster {
    float radius;
    Vec3f absorbtion;
    Vec4f clip;
};

class BodyModule {
public:
    BodyModule(BodyModuleType type = BodyModuleType::CUSTOM) : type(type) {}
    virtual ~BodyModule() = default;
    // Return true if this body module is loaded
    virtual bool isLoaded() {
        return true;
    }
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
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    virtual void setShown(bool shown) {}
    virtual void resumeAfterHidden(ModularBody *body) {}
    virtual void createTexSkin(const std::string &texName) {}
    virtual void switchTexSkin(bool use) {}
    virtual void setColor(BodyColorType type, const Vec3f &c) {}
    virtual bool getColor(BodyColorType type, Vec3f &out) const { return false; }
    //! The same, for what the DATA gave it - so the ledger can record a change
    //! rather than a snapshot (D30).
    virtual bool getAuthoredColor(BodyColorType type, Vec3f &out) const { return false; }
    virtual bool getSkinUse(bool &out) const { return false; }
    virtual int getShownOverride() const { return -1; }
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
    float boundingRadius = 0;
    BodyModuleType type;
};

#endif /* end of include guard: BODY_MODULE_HPP_ */
