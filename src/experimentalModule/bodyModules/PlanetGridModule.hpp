#ifndef PLANET_GRID_MODULE_HPP_
#define PLANET_GRID_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// Custom slot "GRID": meridians/parallels at 1.05 body radius + the tropic and polar circles of the body's tilt
// Near component, depth-tested. No deduction rule: declared explicitly or created by the grid toggle
class PlanetGridModule : public BodyModule {
public:
    PlanetGridModule();
    ~PlanetGridModule();
    // Inflates boundingRadius to the grid radius so its near cap fits in the body's depth slice
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void dumpState(std::ostream &out) const override;

    static bool show; // has no flag of its own: rides the axis flag

    // Follow the sky lines LINE_TROPIC / LINE_CIRCLE_POLAR
    static bool showTropics;
    static bool showPolarCircles;

    // Global grid colors; each instance re-bakes its vertex colors on its next draw
    static void setColors(const Vec3f &meridian, const Vec3f &parallel);
    // To call each frame: the flags gate the draw ranges, only a color change re-bakes
    static void setTropicPolar(bool showTropics, bool showPolarCircles,
                               const Vec3f &tropic, const Vec3f &polarCircle);
protected:
    Vec3f meridianColor;
    Vec3f parallelColor;
    Vec3f tropicColor;
    Vec3f polarColor;

    // Lazy per-body line buffer (unit-sphere positions + baked colors): the tropic/polar circles are body-specific
    std::unique_ptr<VertexBuffer> buffer;
    uint32_t vertexCount = 0;
    uint32_t mainCount = 0;    // meridians + parallels, at offset 0
    uint32_t tropicFirst = 0;  // 0 count when the body has no tropics
    uint32_t tropicCount = 0;
    uint32_t polarFirst = 0;
    uint32_t polarCount = 0;
    bool built = false;
    uint32_t syncedColorGen = 0;
    float bodyAxialTilt = 0.f;
    bool hasTropics = false;
};

#endif /* end of include guard: PLANET_GRID_MODULE_HPP_ */
