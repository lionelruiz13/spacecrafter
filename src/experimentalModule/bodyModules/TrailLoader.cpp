#include "TrailLoader.hpp"
#include "TrailModule.hpp"
#include "tools/utility.hpp"

uint8_t TrailLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly by deduceBodyModuleList; no competition expected
}

bool TrailLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<TrailModule *>(module);
}

std::unique_ptr<BodyModule> TrailLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Per-body trail color (old BodyColor: param trail_color, else the config
    // default object_trails_color wired at the setDefaultBodyColor seam).
    const std::string &trailColor = params["trail_color"];
    // MaxTrail = old per-class value (protosystem.cpp:645-801 dispatch;
    // trail.cpp constructor defaults). BigBody (Planet/Dwarf) = 1460, SmallBody
    // Comet = 2920, SmallBody Asteroid/KBO = 60. Unknown type -> old
    // UNKNOWN->ASTEROID->SmallBody 60 (protosystem.cpp:531-532).
    const std::string &type = params["type"];
    int maxTrail;
    if (type == "Planet" || type == "Dwarf")
        maxTrail = 1460;
    else if (type == "Comet")
        maxTrail = 2920;
    else
        maxTrail = 60; // Asteroid / KBO / unknown
    auto module = std::make_unique<TrailModule>(
        trailColor.empty() ? TrailModule::defaultColor : Utility::strToVec3f(trailColor),
        maxTrail, 1.0 /* DeltaTrail: always 1 sim-day (old never data-set) */);
    addTrailComponent(target, module.get());
    return module;
}
