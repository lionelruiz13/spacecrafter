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
    addNearComponent(target, module.get());
    return module;
}
