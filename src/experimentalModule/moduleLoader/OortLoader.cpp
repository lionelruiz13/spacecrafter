#include "OortLoader.hpp"
#include "experimentalModule/bodyModules/OortModule.hpp"
#include "tools/utility.hpp"

uint8_t OortLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    // Only when explicitly requested (oort=true): the sole opt-in for the OORT
    // slot. 0 otherwise, so the shared CUSTOM family's GridLoader (16) and
    // StarLoader keep every non-oort body.
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
    // NEAR component: the cloud is hidden while the observer sits inside the
    // body's scaledRadius·2 (in/grounded regime) and shown once outside it -
    // the LOW edge of the old altitude gate, reproduced through the regime
    // machinery (ModularBody::draw), never a distance test in the module.
    addNearComponent(target, module.get());
    return module;
}
