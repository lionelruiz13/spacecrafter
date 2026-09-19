#ifndef OJM_LOADER_HPP_
#define OJM_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// OJM slot: bids on model_name + type=Artificial (any other model_name is BasicMeshLoader's named ObjL)
// Scales the body radius by the model's own radius (0 on load failure: never drawn) and disables the body's halo
class OjmLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: OJM_LOADER_HPP_ */
