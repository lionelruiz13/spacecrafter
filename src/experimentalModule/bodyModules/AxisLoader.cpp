#include "AxisLoader.hpp"
#include "AxisModule.hpp"

uint8_t AxisLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly by deduceBodyModuleList; no competition expected
}

bool AxisLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<AxisModule *>(module);
}

std::unique_ptr<BodyModule> AxisLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    auto module = std::make_unique<AxisModule>();
    // Near regime: the axis only means anything on a resolved disc, and its
    // projection consumes the depth-slice range set by clearDepth (which only
    // near-regime draws run under).
    addNearComponent(target, module.get());
    return module;
}
