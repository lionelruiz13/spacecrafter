#ifndef RING_MODULE_HPP_
#define RING_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class s_texture;
class Set;
class Ring2D;
class VertexArray;
template <typename T> class SharedBuffer;
struct bodyRingVert;
struct bodyRingFrag;

// Draw a planetary ring, included in the body's boundingRadius
class RingModule : public BodyModule {
public:
    // Radii in AU, unscaled. Outer edge of tex at u=0
    RingModule(std::unique_ptr<s_texture> tex, float innerRadius, float outerRadius,
               const Vec3f &shadowColor);
    ~RingModule();
    virtual uint32_t getTraits() const override {
        return BMT_PROJECT_G8_SHADOW | BMT_RECEIVE_SHADOW | BMT_USE_DEPTH | BMT_DEPTH_TRACE
             | BMT_TRANSLUCENT; // draw after opaque siblings
    }
    virtual bool isLoaded() override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Draw with depth, as there is no NO_DEPTH variant
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override {
        draw(renderer, body, mat);
    }
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual ShadowCaster getShadowCaster(ModularBody *body, const Vec3f &lightPos) const override;

    static void setLodSlices(int low, int medium, int high);

protected:
    void buildGeometry();

    std::unique_ptr<s_texture> tex;
    std::unique_ptr<Set> texSet; // SHADOW_RING set 1, created at first drawShadow
    std::unique_ptr<Set> set;    // RING family set 0
    std::unique_ptr<SharedBuffer<bodyRingVert>> uVert;
    std::unique_ptr<SharedBuffer<bodyRingFrag>> uFrag;
    std::unique_ptr<Ring2D> strips[6]; // 3 LODs x UP/DOWN half, lazily built
    float innerRadius;
    float outerRadius;
    float mc = 1;                // body scaling factor
    Vec3f shadowColor;           // material absorbtion (ring_shadow_color)
    bool loaded = false;
    static Vec3i lodSlices;
};

#endif /* end of include guard: RING_MODULE_HPP_ */
