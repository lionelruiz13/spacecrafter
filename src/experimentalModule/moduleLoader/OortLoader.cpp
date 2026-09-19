#include "OortLoader.hpp"
#include "experimentalModule/bodyModules/OortModule.hpp"
#include "tools/utility.hpp"

uint8_t OortLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return Utility::isTrue(params["oort"]) ? 200 : 0;
}

bool OortLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<OortModule *>(module);
}

std::unique_ptr<BodyModule> OortLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    const unsigned int nbr = Utility::strToInt(params["oort_elements"], 10000);
    const Vec3f color = params["oort_color"].empty()
        ? Vec3f(0.f, 0.5f, 1.f) : Utility::strToVec3f(params["oort_color"]);
    auto module = std::make_unique<OortModule>(nbr, color);
    addNearComponent(target, module.get());
    return module;
}
