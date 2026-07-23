#include "HintLoader.hpp"
#include "experimentalModule/bodyModules/HintModule.hpp"
#include "tools/utility.hpp"

uint8_t HintLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly by deduceBodyModuleList; no competition expected
}

bool HintLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<HintModule *>(module);
}

std::unique_ptr<BodyModule> HintLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Per-body label color (old BodyColor: param label_color, else the config
    // default planet_names_color wired at the setDefaultBodyColor seam).
    const std::string &labelColor = params["label_color"];
    auto module = std::make_unique<HintModule>(
        labelColor.empty() ? HintModule::defaultLabelColor : Utility::strToVec3f(labelColor));
    addFarComponent(target, module.get());
    return module;
}
