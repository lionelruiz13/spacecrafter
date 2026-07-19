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
        // Stable opaque-first partition (2026-07-18): TRANSLUCENT modules
        // (BMT_TRANSLUCENT) draw AFTER every opaque sibling - blending needs
        // the opaque content beneath it, and a translucent module drawn first
        // also depth-blocks the opaque fill behind it (found live at the
        // row-4 ring port: RING deduces before MESH, the inner ring went
        // opaque-black over the never-drawn planet limb; the old path encoded
        // this ordering in its explicit drawBody-then-drawRings calls,
        // body.cpp:1139-1140). Enforced HERE at routing time - the draw loops
        // stay untouched (zero hot-path cost) and deduction order stops being
        // load-bearing for correctness.
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
    // Orbit-line modules (row 8). NOT a screen-size regime: the orbit pass
    // (trace + line) is system-driven under the orbit-union depth range
    // (ModularSystem::drawOrbits), so orbit modules live in their own list.
    static inline void addOrbitComponent(ModularBody *target, BodyModule *module) {
        target->orbitComponents.push_back(module);
    }
    // Trail-line modules (row 9). NOT a screen-size regime: the trail pass
    // (accumulation + line) is system-driven every frame so accumulation
    // continues while the body is invisible (ModularSystem::drawTrails), so
    // trail modules live in their own list.
    static inline void addTrailComponent(ModularBody *target, BodyModule *module) {
        target->trailComponents.push_back(module);
    }
};

#endif /* end of include guard: MODULE_LOADER_HPP_ */
