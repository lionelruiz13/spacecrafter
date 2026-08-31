#ifndef OORT_LOADER_HPP_
#define OORT_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the OORT slot (B5, INTENT S6.9 content-migration PILOT). Rides the
// CUSTOM loader family with an explicit slot (the GRID precedent, S6.7
// declaration half), keyed on the `oort=true` marker so it competes only for
// bodies that requested it - GridLoader/StarLoader keep every other CUSTOM
// body (isLikely 0 here otherwise). Routes the module as a NEAR component so
// the body's regime machinery gates the low edge of the old altitude-gated draw
// (OortModule.hpp).
class OortLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: OORT_LOADER_HPP_ */
