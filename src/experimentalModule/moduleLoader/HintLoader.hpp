#ifndef HINT_LOADER_HPP_
#define HINT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

class HintLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: HINT_LOADER_HPP_ */
