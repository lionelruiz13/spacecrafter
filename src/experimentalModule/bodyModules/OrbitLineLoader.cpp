#include "OrbitLineLoader.hpp"
#include "OrbitModule.hpp"
#include "tools/utility.hpp"

uint8_t OrbitLineLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly by deduceBodyModuleList; no competition expected
}

bool OrbitLineLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<OrbitModule *>(module);
}

std::unique_ptr<BodyModule> OrbitLineLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Per-body orbit color (old BodyColor: param orbit_color, else the config
    // default planet_orbits_color wired at the setDefaultBodyColor seam).
    const std::string &orbitColor = params["orbit_color"];
    // close_orbit default true (protosystem.cpp:546 strToBool(..., 1)).
    const bool close = Utility::strToBool(params["close_orbit"], true);
    auto module = std::make_unique<OrbitModule>(
        orbitColor.empty() ? OrbitModule::defaultColor : Utility::strToVec3f(orbitColor),
        close);
    addOrbitComponent(target, module.get());
    return module;
}
