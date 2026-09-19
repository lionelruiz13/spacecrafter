#ifndef GRID_LOADER_HPP_
#define GRID_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Never deduced, loaded explicitly for planet_grid=true
class GridLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: GRID_LOADER_HPP_ */
