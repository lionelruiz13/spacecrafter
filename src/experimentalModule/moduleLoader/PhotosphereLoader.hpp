#ifndef PHOTOSPHERE_LOADER_HPP_
#define PHOTOSPHERE_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// MESH slot of a body that EMITS light and has a color map: outbids the other MESH loaders, bids 0 on everything else
// The bid reads isStar(), not the type string: a composed twin (light_source = true) must select it too
class PhotosphereLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: PHOTOSPHERE_LOADER_HPP_ */
