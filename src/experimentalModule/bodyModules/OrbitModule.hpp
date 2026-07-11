#ifndef ORBIT_MODULE_HPP_
#define ORBIT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

// ORBIT slot - the orbit line of a body (landing zone of the old
// OrbitPlot/Orbit2D/Orbit3D). Two hooks:
// - draw: the orbit polyline (color pass). Data: the body's Orbit object
//   (sampled into a point buffer), parent transform, per-body orbit color.
// - drawTrace: NOT this module's line - the trace pass belongs to the BODY
//   mesh cutting its hole (see BodyModule.hpp TRACE). The orbit LINE is the
//   consumer of those holes: it is drawn depth-tested against the trace
//   buffer so it hides behind bodies (old cmdBodyDepth/cmdOrbit pass pair).
// Regime: far components - an orbit is visible when its body may not be.
// The old system-level orbitBucket depth calibration moves to the Renderer's
// depth-range partitioning (Renderer.hpp).
// Deduction rule: default-on for bodies with a non-still orbit; param
// orbit=false suppresses. Global + per-name toggles ride the seam
// (setFlagPlanetsOrbits / per-name variant).
class OrbitModule : public BodyModule {
public:
    OrbitModule() : BodyModule(BodyModuleType::ORBIT) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;

    static bool showPlanets;    // old setFlagPlanetsOrbits
    static bool showSatellites; // old setFlagSatellitesOrbits
protected:
    LinearFader fader;
    Vec3f color;
};

#endif /* end of include guard: ORBIT_MODULE_HPP_ */
