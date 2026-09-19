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

// HALO is owned by the body, LABEL/ORBIT/TRAIL by the HINT/ORBIT/TRAIL modules
enum class BodyColorType : uint8_t { HALO, LABEL, ORBIT, TRAIL, ALL, NONE };

inline BodyColorType parseBodyColorType(const std::string &name) {
    if (name == "halo")  return BodyColorType::HALO;
    if (name == "label") return BodyColorType::LABEL;
    if (name == "orbit") return BodyColorType::ORBIT;
    if (name == "trail") return BodyColorType::TRAIL;
    if (name == "all")   return BodyColorType::ALL;
    return BodyColorType::NONE;
}

// A draw hook is only called if its trait is declared
enum BodyModuleTraits {
    BMT_REPLICATED =            0x00000001, // Heavily used, some batching or similar strategy is recommended
    BMT_CACHED =                0x00000002, // Some state should be cached for performance, maybe a bad idea ?
    BMT_DYNAMIC =               0x00000004, // Use dynamic resolution mechanism
    BMT_DEPTH_TRACE =           0x00000008, // Affect the depth buffer
    BMT_USE_DEPTH =             0x00000010, // Use the depth buffer (pointer/halo doesn't)
    BMT_BASIC_SELF_SHADOW =     0x00000020, // Project monochrome self-shadowing
    BMT_RGBA8_SELF_SHADOW =     0x00000040, // Project RGBA8 self-shadowing
    BMT_PROJECT_G1_SHADOW =     0x00000080, // Project opaque-silhouette shadow (solid casters)
    BMT_PROJECT_G8_SHADOW =     0x00000100, // Project greyscale transmission shadow (graded casters: rings)
    BMT_TRANSLUCENT =           0x00000800, // Blended, drawn after the opaque modules
    BMT_RECEIVE_SHADOW =        0x00000400, // Receive the shadows of other bodies
};

struct ShadowCaster {
    float radius; // Silhouette radius in scaled AU (not boundingRadius)
    Vec3f absorbtion; // Per channel, 1 = fully absorbed
    Vec4f clip; // Eye-space plane, cast only where dot(P, xyz) + w <= 0
};

// May be destroyed between frames, release resources through deferred release only
class BodyModule {
public:
    BodyModule(BodyModuleType type = BodyModuleType::CUSTOM) : type(type) {}
    virtual ~BodyModule() = default;
    // Return true if this body module is loaded
    virtual bool isLoaded() {
        return true;
    }
    // Preload content for use in a near future, kept for keepFrames frames while unused
    virtual void preload(ModularBody *body, int keepFrames) {}
    // Update this body module, return true when update is no longer required for this body
    virtual bool update(ModularBody *body, float scaledRadius) {
        boundingRadius = scaledRadius;
        return true;
    }
    virtual uint32_t getTraits() const {
        return 0;
    }
    // Draw this Body Module
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) = 0;
    // Draw a minimalist representation of this Body Module, without depth buffer
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Draw a shadow to project onto another body, into the shadow layer idx
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const;
    // Draw a shadow for self-shadowing, draw() must then project with the same mat
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Draw a trace for orbit tracing
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // A module answers for what it owns, a getter returns false otherwise
    virtual void setShown(bool shown) {}
    // Rebuild what time would have moved while the body was hidden
    virtual void resumeAfterHidden(ModularBody *body) {}
    virtual void createTexSkin(const std::string &texName) {}
    virtual void switchTexSkin(bool use) {}
    virtual void setColor(BodyColorType type, const Vec3f &c) {}
    virtual bool getColor(BodyColorType type, Vec3f &out) const { return false; }
    // Return the color given by the data
    virtual bool getAuthoredColor(BodyColorType type, Vec3f &out) const { return false; }
    virtual bool getSkinUse(bool &out) const { return false; }
    // -1 = follow the global flag, 0/1 = forced for this body
    virtual int getShownOverride() const { return -1; }
    virtual void captureAuthored() {}
    // Compare an object position and radius with this ModularBody
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
    float boundingRadius = 0; // Enclose everything this module draws
    BodyModuleType type;
};

#endif /* end of include guard: BODY_MODULE_HPP_ */
