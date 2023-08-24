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

#include "bodyModules/BasicMeshLoader.hpp"

void ModuleLoaderMgr::init()
{
    registerModule("barycenter", std::make_unique<BaryOrbitLoader>());
    registerModule("comet_orbit", std::make_unique<CometOrbitLoader>());
    registerModule("earth_custom", std::make_unique<EarthOrbitLoader>());
    registerModule("ell_orbit", std::make_unique<ElipticOrbitLoader>());
    registerModule("location_orbit", std::make_unique<LocationOrbitLoader>());
    registerModule("lunar_custom", std::make_unique<LunarOrbitLoader>());
    registerModule("still_orbit", std::make_unique<StillOrbitLoader>());
    registerModule(std::make_unique<SpecialOrbitLoader>());

    registerModule(BodyModuleType::MESH, std::make_unique<BasicMeshLoader>("bodies/eclipse_map.png"));
}
