#include "GridLoader.hpp"
#include "experimentalModule/bodyModules/PlanetGridModule.hpp"

uint8_t GridLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly (planet_grid=true); no competition (sole CUSTOM loader)
}

bool GridLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<PlanetGridModule *>(module);
}

std::unique_ptr<BodyModule> GridLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    auto module = std::make_unique<PlanetGridModule>();
    // Near regime: the grid only means anything on a resolved disc, and its
    // depth-tested draw consumes the depth-slice range set by clearDepth (which
    // only near-regime draws run under). Added AFTER the deduced MESH (opaque,
    // inserted before any translucent sibling) - so the grid draws after the
    // body surface, as the spec requires.
    addNearComponent(target, module.get());
    return module;
}
