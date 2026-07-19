#include "StarLoader.hpp"
#include "StarModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "tools/context.hpp"
#include "tools/utility.hpp"

uint8_t StarLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    // STAR-typed body with a big-halo texture (old Sun::setBigHalo gate). 0 on
    // everything else so the co-registered GridLoader (unconditional 16, the
    // explicit planet_grid load) is never contested; 200 > 16 so this wins the
    // deduced CUSTOM load on the Sun. No 255 short-circuit (leave headroom).
    return (target->isStar() && !params["tex_big_halo"].empty()) ? 200 : 0;
}

bool StarLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<StarModule *>(module);
}

std::unique_ptr<BodyModule> StarLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // big_halo_size default 50 = old protosystem.cpp:690 (the shipped Sun
    // carries 200); createInfo-supplied, per §12 row 14.
    // NOTE: big_halo_size from data is NOT read - it is DEAD for the main Sun
    // (old SolarSystem::setFlagSunScale overwrites the Sun's halo size to 200;
    // solarsystem.hpp:100). The effective size lives in StarModule::sunHaloSize,
    // driven by the SSystemFactory::setFlagSunScale seam (INTENT §11.44).
    auto module = std::make_unique<StarModule>();
    // Big-halo texture into the Renderer SUN_HALO service (old Sun::setBigHalo,
    // body_sun.cpp:109-121: try path+file, else the standard texture paths).
    // Single-texture service: only the reachable Sun has a big halo; a second
    // reachable star with its own tex_big_halo would need per-instance
    // textures (SUSPENDED with STAR_VIEWER/CORONA, INTENT §11.44).
    Context::instance->renderer.setSunHaloTexture(params["tex_big_halo"], params["path"]);
    // FAR regime: screen-space glow at screenPos, drawn (via draw()) BEFORE the
    // near disc in every regime the Sun spans (INTENT §11.44).
    addFarComponent(target, module.get());
    return module;
}
