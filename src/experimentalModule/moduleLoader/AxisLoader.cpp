#include "AxisLoader.hpp"
#include "experimentalModule/bodyModules/AxisModule.hpp"

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
    addNearComponent(target, module.get());
    return module;
}
