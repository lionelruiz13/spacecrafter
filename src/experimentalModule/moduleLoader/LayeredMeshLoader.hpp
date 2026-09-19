#ifndef LAYERED_MESH_LOADER_HPP_
#define LAYERED_MESH_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

class LayeredMeshLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: LAYERED_MESH_LOADER_HPP_ */
