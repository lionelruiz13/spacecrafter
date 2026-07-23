#ifndef ORBIT_LINE_LOADER_HPP_
#define ORBIT_LINE_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the ORBIT slot (the orbit LINE - named OrbitLineLoader to avoid the
// existing experimentalModule/OrbitLoader.hpp, which loads orbit COORDINATE
// functions, a different family). Deduction lives in deduceBodyModuleList
// (default for a body with a non-still orbit - orbit_visualization_period
// present, the same gate the old draw used, re.sidereal_period > 0; param
// orbit=false suppresses). Routes into the dedicated orbitComponents list (the
// orbit pass is system-driven, not a screen-size regime).
class OrbitLineLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: ORBIT_LINE_LOADER_HPP_ */
