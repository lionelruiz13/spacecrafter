#ifndef OORT_LOADER_HPP_
#define OORT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Custom slot "OORT": bids only on the oort=true marker, every other CUSTOM body stays GridLoader's or StarLoader's
class OortLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: OORT_LOADER_HPP_ */
