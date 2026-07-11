#ifndef MODULE_LOADER_HPP_
#define MODULE_LOADER_HPP_

#include "ModularBody.hpp"
#include <map>
#include <string>

// Loading contract (ordering is load-bearing):
// 1. ModuleLoaderMgr selects the loader (isLikely, highest value wins, 255 short-circuits)
// 2. loader->load() creates the module AND routes it into the regime lists (add*Component)
// 3. ModuleLoaderMgr installs ownership into the target slot: ModularBody::slot(slotID, module)
//    slot() erases the *replaced* module's routing only - the new module must already be routed
// A load() which doesn't route produces an owned-but-never-drawn module.
class ModuleLoader {
public:
    // Return a value who determine which loader to use for loading, the biggest value who come first is chosen.
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const = 0;
    // Return true if this BodyModule come from this object
    virtual bool isLoaderOf(BodyModule *module) const {return false;}
    // Load a BodyModule for this body and route it into the regime lists (see loading contract above)
    // Ownership of the returned module is installed into the slot by ModuleLoaderMgr, not here.
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
