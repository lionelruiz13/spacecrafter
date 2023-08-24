#ifndef BODY_MODULE_HPP_
#define BODY_MODULE_HPP_

#include "tools/vecmath.hpp"

class ModularBody;
class Renderer;

enum class BodyModuleType : unsigned char {
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

enum class RelativePosition : unsigned char {
    ANY = 0x0, // Relative position has no importance : they doesn't overlap on screen
    FRONT = 0x1,
    OVERLAP = 0x2,
    BACK = 0x4,
};

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
    virtual bool update(ModularBody *body, float scaledRadius) {
        boundingRadius = scaledRadius;
        return true;
    }
    // Draw this Body Module
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) = 0;
    // Draw a minimalist representation of this Body Module, without depth buffer
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Draw a shadow to project onto another body
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) {}
    // Draw a shadow for self-shadowing
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
    // Draw a trace for orbit tracing
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) {}
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
protected:
    float boundingRadius;
    BodyModuleType type;
    bool loaded = false; // May be used internally
};

#endif /* end of include guard: BODY_MODULE_HPP_ */
