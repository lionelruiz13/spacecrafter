#include "StarLoader.hpp"
#include "experimentalModule/bodyModules/StarModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "tools/context.hpp"
#include "tools/utility.hpp"

uint8_t StarLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return (target->isStar() && !params["tex_big_halo"].empty()) ? 200 : 0;
}

bool StarLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<StarModule *>(module);
}

std::unique_ptr<BodyModule> StarLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    auto module = std::make_unique<StarModule>();
    Context::instance->renderer.setSunHaloTexture(params["tex_big_halo"], params["path"]);
    // FAR regime: screen-space glow at screenPos, drawn (via draw()) BEFORE the
    // near disc in every regime the Sun spans (INTENT S11.44).
    addFarComponent(target, module.get());
    return module;
}
