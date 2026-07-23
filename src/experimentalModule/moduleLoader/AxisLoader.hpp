#ifndef AXIS_LOADER_HPP_
#define AXIS_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the AXIS slot. Deduction lives in deduceBodyModuleList (default
// for every body with a mesh - the old Body carried an Axis member
// unconditionally; the mesh gate follows the landing-zone rule and keeps
// axis lines off model-only artificial bodies, a documented divergence).
class AxisLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: AXIS_LOADER_HPP_ */
