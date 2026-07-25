#include "TrailLoader.hpp"
#include "experimentalModule/bodyModules/TrailModule.hpp"
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
    // B27 A7 (§11.73): the trail sample count is a DECLARED capability of the
    // body (`trail_length`, D10key §11.79(e)), resolved once by the loader
    // authority (ModularSystem::loadBody) - which is where the D14 format scope
    // lives. Legacy source, kept there and frozen (D9): the old per-class value
    // (protosystem.cpp:645-801 dispatch; trail.cpp ctor defaults) BigBody
    // (Planet/Dwarf) 1460, SmallBody Comet 2920, Asteroid/KBO/unknown 60
    // (protosystem.cpp:531-532). No `type` read here any more (I4).
    auto module = std::make_unique<TrailModule>(
        trailColor.empty() ? TrailModule::defaultColor : Utility::strToVec3f(trailColor),
        target->getTrailLength(), 1.0 /* DeltaTrail: always 1 sim-day (old never data-set) */);
    addTrailComponent(target, module.get());
    return module;
}
