#ifndef TAIL_LOADER_HPP_
#define TAIL_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the TAIL slot (TailModule, row 12). Requested by
// deduceBodyModuleList for comet-typed bodies carrying an apparent_magnitude
// (the old gate: SmallBody bound tails only with both apparent_magnitude AND
// slope present, protosystem.cpp:802). Reads the gas / dust / optional extra
// sub-tail parameters (protosystem.cpp:805-851) into one module's SubTail list.
class TailLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: TAIL_LOADER_HPP_ */
