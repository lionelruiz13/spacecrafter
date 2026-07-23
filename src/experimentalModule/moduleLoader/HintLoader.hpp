#ifndef HINT_LOADER_HPP_
#define HINT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the HINT slot (HintModule). Requested for every named body by
// deduceBodyModuleList (param hint=false suppresses the request there, not
// here - an isLikely of 0 would fire the "no loader available" warning for a
// deliberate suppression).
class HintLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: HINT_LOADER_HPP_ */
