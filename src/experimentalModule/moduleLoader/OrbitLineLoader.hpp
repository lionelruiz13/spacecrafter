#ifndef ORBIT_LINE_LOADER_HPP_
#define ORBIT_LINE_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// ORBIT slot, the orbit LINE (OrbitLoader loads the orbit COORDINATE functions); routed to the orbitComponents list
class OrbitLineLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: ORBIT_LINE_LOADER_HPP_ */
