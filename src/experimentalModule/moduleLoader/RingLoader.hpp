#ifndef RING_LOADER_HPP_
#define RING_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the RING slot (RingModule - currently its G8 caster half; the
// color/trace port is row-4 scope). Bids on tex_ring, the same key the
// deduction uses (ModularBody::deduceBodyModuleList) - old parse:
// protosystem.cpp ring block (rings=true + tex_ring + ring_inner/outer_size).
class RingLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: RING_LOADER_HPP_ */
