#ifndef PLANET_GRID_MODULE_HPP_
#define PLANET_GRID_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// Custom slot "GRID" - latitude/longitude grid at ~1.05 body radius (landing
// zone of the old PlanetGrid friend). Color pass, depth-tested, drawn after
// the body surface. Data: body transform + radius; meridian/parallel colors.
// Regime: near components. Clean landing - the old code's only body coupling
// was reading mat + radius through friendship.
// Deduction rule: none (explicit declaration only, or created by the seam's
// setFlagPlanetGrid toggle) - demonstrates the explicit-slot half of the
// module declaration model (ModuleLoaderMgr::loadModule slot parameter).
//
// PORT RECONCILIATION (2026-07-19, T4 - INTENT §11.42):
//  * Shader contract = the AXIS/ORBIT PUSH family (planet_grid.vert declares
//    push_constant {mat4 ModelViewMatrix; vec3 clipping_fov} + custom_project
//    spec-8) - NOT TRAIL's cam_block UBO. Deployed shaders reused VERBATIM
//    (parity by construction). Per-vertex color (meridian/parallel baked in).
//  * `show` rides the AXIS flag: the old flag_planet_grid has NO independent
//    setter - Body::setFlagAxis sets BOTH flag_axis and flag_planet_grid=b
//    (body.cpp:229-234). The seam mirror lives in SSystemFactory::setFlagAxis.
//  * Geometry is TILT-INDEPENDENT per the mat+radius Data contract (ModularBody
//    exposes no axial-tilt scalar): 24 meridians + evenly-spaced parallels
//    (equator included). Old's tropic/polar-circle astronomy (needs the tilt
//    scalar + a name/satellite gate) is RETIRED by the clean spec - full
//    planet-grid observable parity SUSPENDED for Vixy (INTENT §11.42).
//  * Colors: the header's per-instance members are fed from GLOBAL static
//    defaults (old's observable: sky-manager colors, same for every body). The
//    color-authority structural choice (per-instance vs global; sky-manager
//    sourcing) is SUSPENDED for Vixy.
class PlanetGridModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (the inline ctor's exception path instantiates the member destructor -
    // AXIS precedent).
    PlanetGridModule();
    ~PlanetGridModule();
    // Grid extends to 1.05 * scaledRadius: inflate the traced bounding radius so
    // the grid's near cap fits inside this body's depth slice (else the front
    // grid is znear-clipped by the bucket range).
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show; // old flag_planet_grid (rides the axis flag)

    // Global grid colors (old observable: sky-manager colors, shared by every
    // body). setColors bumps colorGeneration; each instance re-syncs its
    // per-member colors + re-bakes its vertex colors on the next draw.
    static void setColors(const Vec3f &meridian, const Vec3f &parallel);
protected:
    Vec3f meridianColor;
    Vec3f parallelColor;

    // Lazy per-body line buffer (unit-sphere positions + baked per-vertex
    // color), globalBuffer-backed. Positions body/radius-independent (scale
    // folded into the pushed matrix); color re-baked when colorGeneration moves.
    std::unique_ptr<VertexBuffer> buffer;
    uint32_t vertexCount = 0;
    uint32_t meridianVertexCount = 0; // first meridianVertexCount verts = meridians
    bool built = false;
    uint32_t syncedColorGen = 0;
};

#endif /* end of include guard: PLANET_GRID_MODULE_HPP_ */
