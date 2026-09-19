#ifndef AXIS_LOADER_HPP_
#define AXIS_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// AXIS slot: no gate here, the deduction requests it for every body with a mesh
class AxisLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: AXIS_LOADER_HPP_ */
