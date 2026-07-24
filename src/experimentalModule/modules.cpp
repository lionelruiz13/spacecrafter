#include "ModuleLoaderMgr.hpp"
#include "ModularBody.hpp"
#include "OrbitLoader.hpp"
#include "ModuleLoader.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "tools/s_texture.hpp"
#include "appModule/space_date.hpp"
#include "EntityCore/EntityCore.hpp"
extern const Mat4d mat_j2000_to_vsop87;

#define SOLAR_MASS 1.989e30
#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22
#define MARS_MASS  0.64185e24

#include "orbitModules/BaryOrbitLoader.hpp"
#include "orbitModules/CometOrbitLoader.hpp"
#include "orbitModules/EarthOrbitLoader.hpp"
#include "orbitModules/ElipticOrbitLoader.hpp"
#include "orbitModules/LocationOrbitLoader.hpp"
#include "orbitModules/LunarOrbitLoader.hpp"
#include "orbitModules/SpecialOrbitLoader.hpp"
#include "orbitModules/StillOrbitLoader.hpp"
#include "orbitModules/SurfacePointOrbitLoader.hpp"

#include "moduleLoader/BasicMeshLoader.hpp"
#include "moduleLoader/LayeredMeshLoader.hpp"
#include "moduleLoader/HintLoader.hpp"
#include "moduleLoader/AtmExtLoader.hpp"
#include "moduleLoader/RingLoader.hpp"
#include "moduleLoader/OjmLoader.hpp"
#include "moduleLoader/AxisLoader.hpp"
#include "moduleLoader/OrbitLineLoader.hpp"
#include "moduleLoader/TrailLoader.hpp"
#include "moduleLoader/TailLoader.hpp"
#include "moduleLoader/GridLoader.hpp"
#include "moduleLoader/StarLoader.hpp"
#include "moduleLoader/OortLoader.hpp"

void ModuleLoaderMgr::init()
{
    registerModule("barycenter", std::make_unique<BaryOrbitLoader>());
    registerModule("comet_orbit", std::make_unique<CometOrbitLoader>());
    registerModule("earth_custom", std::make_unique<EarthOrbitLoader>());
    registerModule("ell_orbit", std::make_unique<ElipticOrbitLoader>());
    registerModule("location_orbit", std::make_unique<LocationOrbitLoader>());
    registerModule("lunar_custom", std::make_unique<LunarOrbitLoader>());
    registerModule("still_orbit", std::make_unique<StillOrbitLoader>());
    registerModule("surface_point", std::make_unique<SurfacePointOrbitLoader>()); // B24 grounded/launchpad provider (INTENT 11.78; spelling pending sign-off)
    registerModule(std::make_unique<SpecialOrbitLoader>());

    registerModule(BodyModuleType::MESH, std::make_unique<BasicMeshLoader>()); // eclipse LUT retired at S5 (shadow-paths.md B4)
    registerModule(BodyModuleType::MESH, std::make_unique<LayeredMeshLoader>()); // row 2: layered discs (outbids BasicMesh on layered texture keys)
    registerModule(BodyModuleType::HINT, std::make_unique<HintLoader>());
    registerModule(BodyModuleType::ATMOSPHERE, std::make_unique<AtmExtLoader>()); // from-space rim shell (row 13)
    registerModule(BodyModuleType::RING, std::make_unique<RingLoader>()); // G8 caster half (shadow-composition wave); color/trace = row 4
    registerModule(BodyModuleType::OJM, std::make_unique<OjmLoader>()); // row 3: artificial 3D-model bodies (OJM wave)
    registerModule(BodyModuleType::AXIS, std::make_unique<AxisLoader>()); // row 10: rotation-axis line (first line-class family)
    registerModule(BodyModuleType::ORBIT, std::make_unique<OrbitLineLoader>()); // row 8: orbit line (second line-class family; TRACE prepass consumer)
    registerModule(BodyModuleType::TRAIL, std::make_unique<TrailLoader>()); // row 9: trail line (position accumulation over sim time; invisible-tick)
    registerModule(BodyModuleType::TAIL, std::make_unique<TailLoader>()); // row 12: comet gas/dust tail (instanced batch, Renderer-owned; Tail::global dissolved)
    registerModule(BodyModuleType::CUSTOM, std::make_unique<GridLoader>()); // row 11: lat/lon grid (explicit-slot only, §6.7 declaration half)
    registerModule(BodyModuleType::CUSTOM, std::make_unique<StarLoader>()); // row 14: star big-halo glow (deduced; wins CUSTOM over Grid on stars, 0 on planets)
    registerModule(BodyModuleType::CUSTOM, std::make_unique<OortLoader>()); // B5 §6.9 pilot: oort point cloud (explicit slot OORT, oort=true opt-in, 0 on every other CUSTOM body)
}
