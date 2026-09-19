#ifndef MODULE_LOADER_HPP_
#define MODULE_LOADER_HPP_

#include "ModularBody.hpp"
#include <map>
#include <string>

// Loading contract: ModuleLoaderMgr selects the loader (isLikely); load() creates the module AND routes it
// (add*Component); then ModuleLoaderMgr installs ownership with ModularBody::slot(), which only erases the routing
// of the replaced module. A load() which doesn't route produces an owned-but-never-drawn module.
class ModuleLoader {
public:
    // Return a value who determine which loader to use for loading, the biggest value who come first is chosen.
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const = 0;
    // Return true if this BodyModule come from this object
    virtual bool isLoaderOf(BodyModule *module) const {return false;}
    // Load a BodyModule for this body and route it into the regime lists (see loading contract above)
    // Ownership of the returned module is installed into the slot by ModuleLoaderMgr, not here.
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) = 0;
    // Re-home an already slotted module into the list named by relation: far, near, grounded, in, orbit, trail, tail
    // false on an unknown relation, the module keeps its routing
    static bool reroute(ModularBody *target, BodyModule *module, const std::string &relation);
protected:
    static inline void addFarComponent(ModularBody *target, BodyModule *module) {
        target->farComponents.push_back(module);
    }
    // Translucent modules (BMT_TRANSLUCENT) stay after every opaque one: blending needs the opaque content beneath
    static inline void addNearComponent(ModularBody *target, BodyModule *module) {
        if (module->getTraits() & BMT_TRANSLUCENT) {
            target->nearComponents.push_back(module);
        } else {
            auto it = target->nearComponents.begin();
            while (it != target->nearComponents.end() && !((*it)->getTraits() & BMT_TRANSLUCENT))
                ++it;
            target->nearComponents.insert(it, module);
        }
    }
    static inline void addGroundedComponent(ModularBody *target, BodyModule *module) {
        target->groundedComponents.push_back(module);
    }
    static inline void addInComponent(ModularBody *target, BodyModule *module) {
        target->inComponents.push_back(module);
    }
    // Orbit, trail and tail are not screen-size regimes: their passes are driven by the ModularSystem, hence own lists
    static inline void addOrbitComponent(ModularBody *target, BodyModule *module) {
        target->orbitComponents.push_back(module);
    }
    static inline void addTrailComponent(ModularBody *target, BodyModule *module) {
        target->trailComponents.push_back(module);
    }
    static inline void addTailComponent(ModularBody *target, BodyModule *module) {
        target->tailComponents.push_back(module);
    }
};

#endif /* end of include guard: MODULE_LOADER_HPP_ */
