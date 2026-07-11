#ifndef PLANET_GRID_MODULE_HPP_
#define PLANET_GRID_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// Custom slot "GRID" - latitude/longitude grid at ~1.05 body radius (landing
// zone of the old PlanetGrid friend). Color pass, depth-tested, drawn after
// the body surface. Data: body transform + radius; meridian/parallel colors.
// Regime: near components. Clean landing - the old code's only body coupling
// was reading mat + radius through friendship.
// Deduction rule: none (explicit declaration only, or created by the seam's
// setFlagPlanetGrid toggle) - demonstrates the explicit-slot half of the
// module declaration model (ModuleLoaderMgr::loadModule slot parameter).
class PlanetGridModule : public BodyModule {
public:
    PlanetGridModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show; // old setFlagPlanetGrid
protected:
    Vec3f meridianColor;
    Vec3f parallelColor;
};

#endif /* end of include guard: PLANET_GRID_MODULE_HPP_ */
