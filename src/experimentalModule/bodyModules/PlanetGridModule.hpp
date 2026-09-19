#ifndef PLANET_GRID_MODULE_HPP_
#define PLANET_GRID_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// Draw the grid, tropics and polar circles at 1.05 body radius
class PlanetGridModule : public BodyModule {
public:
    PlanetGridModule();
    ~PlanetGridModule();
    // Inflate boundingRadius to fit the grid in the depth slice
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void dumpState(std::ostream &out) const override;

    static bool show; // rides the axis flag

    // Follow the sky lines LINE_TROPIC / LINE_CIRCLE_POLAR
    static bool showTropics;
    static bool showPolarCircles;

    static void setColors(const Vec3f &meridian, const Vec3f &parallel);
    // To call each frame, only a color change re-bakes the vertices
    static void setTropicPolar(bool showTropics, bool showPolarCircles,
                               const Vec3f &tropic, const Vec3f &polarCircle);
protected:
    Vec3f meridianColor;
    Vec3f parallelColor;
    Vec3f tropicColor;
    Vec3f polarColor;

    std::unique_ptr<VertexBuffer> buffer; // lazy, unit-sphere positions + baked colors
    uint32_t vertexCount = 0;
    uint32_t mainCount = 0;    // meridians + parallels, at offset 0
    uint32_t tropicFirst = 0;
    uint32_t tropicCount = 0;
    uint32_t polarFirst = 0;
    uint32_t polarCount = 0;
    bool built = false;
    uint32_t syncedColorGen = 0;
    float bodyAxialTilt = 0.f;
    bool hasTropics = false;
};

#endif /* end of include guard: PLANET_GRID_MODULE_HPP_ */
