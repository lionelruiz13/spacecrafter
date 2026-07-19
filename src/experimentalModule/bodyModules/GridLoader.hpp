#ifndef GRID_LOADER_HPP_
#define GRID_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the GRID slot (BodyModuleType::CUSTOM). Unlike every other family,
// GRID is NOT deduced (deduceBodyModuleList returns a bare BodyModuleType and
// cannot name a slot): a body opts in explicitly with planet_grid=true, and
// ModularSystem installs it through loadModule's explicit `slot` parameter into
// the named "GRID" slot (§6.7 declaration half, INTENT §11.42). It is the first
// and only user of that slot argument.
class GridLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: GRID_LOADER_HPP_ */
