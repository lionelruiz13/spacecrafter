#ifndef ATM_EXT_LOADER_HPP_
#define ATM_EXT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// ATMOSPHERE slot (AtmExtModule): bids on atmosphere_ext_model; the has-atmosphere precondition is the deduction's
class AtmExtLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: ATM_EXT_LOADER_HPP_ */
