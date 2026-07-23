#ifndef ATM_EXT_LOADER_HPP_
#define ATM_EXT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the ATMOSPHERE slot (AtmExtModule - the from-space rim shell,
// row 13). Selected when the body declares an atmosphere gradient
// (atmosphere_ext_model); the deduction-side precondition mirrors the old
// parse gate exactly (protosystem.cpp:865: has_atmosphere or
// atmosphere_lim_landscape present) - see ModularBody::deduceBodyModuleList.
class AtmExtLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: ATM_EXT_LOADER_HPP_ */
