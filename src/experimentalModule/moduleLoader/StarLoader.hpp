#ifndef STAR_LOADER_HPP_
#define STAR_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the star's big-halo module (StarModule, INTENT S12 row 14).
// DEDUCED (deduceBodyModuleList) for STAR-typed bodies carrying a tex_big_halo
// (the old Sun::setBigHalo gate, protosystem.cpp:687-691). Registered under
// BodyModuleType::CUSTOM alongside GridLoader; isLikely returns 0 on non-star
// bodies so the explicit planet_grid=true GRID load is never contested, and a
// high value on stars so it wins the deduced CUSTOM load over GridLoader's
// unconditional 16. Wires the big_halo texture into the Renderer SUN_HALO
// service (per-sun data -> load-time seam, S9: no runtime big-halo command)
// and routes the module into the FAR regime (screen-space glow at screenPos,
// drawn before the near disc - INTENT S11.44).
class StarLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: STAR_LOADER_HPP_ */
