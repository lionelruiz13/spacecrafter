#ifndef PHOTOSPHERE_LOADER_HPP_
#define PHOTOSPHERE_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Bid on isStar() with tex_map, outbid the other MESH loaders
class PhotosphereLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: PHOTOSPHERE_LOADER_HPP_ */
