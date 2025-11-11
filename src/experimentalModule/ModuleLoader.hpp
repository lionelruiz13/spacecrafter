#ifndef MODULE_LOADER_HPP_
#define MODULE_LOADER_HPP_

#include "ModularBody.hpp"
#include <map>
#include <string>

class ModuleLoader {
public:
    // Return a value who determine which loader to use for loading, the biggest value who come first is chosen.
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const = 0;
    // Return true if this BodyModule come from this object
    virtual bool isLoaderOf(BodyModule *module) const {return false;}
    // Load and install a BodyModule on this body
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) = 0;
protected:
    static inline void addFarComponent(ModularBody *target, BodyModule *module) {
        target->farComponents.push_back(module);
    }
    static inline void addNearComponent(ModularBody *target, BodyModule *module) {
        target->nearComponents.push_back(module);
    }
    static inline void addGroundedComponent(ModularBody *target, BodyModule *module) {
        target->groundedComponents.push_back(module);
    }
    static inline void addInComponent(ModularBody *target, BodyModule *module) {
        target->inComponents.push_back(module);
    }
};

#endif /* end of include guard: MODULE_LOADER_HPP_ */
