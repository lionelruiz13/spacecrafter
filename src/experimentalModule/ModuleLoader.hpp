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
    virtual void load(ModularBody *target, std::map<std::string, std::string> &params) = 0;
protected:
    static inline std::list<std::shared_ptr<BodyModule>> &getFarComponents(ModularBody *target) {
        return target->farComponents;
    }
    static inline std::list<std::shared_ptr<BodyModule>> &getNearComponents(ModularBody *target) {
        return target->nearComponents;
    }
    static inline std::list<std::shared_ptr<BodyModule>> &getInComponents(ModularBody *target) {
        return target->inComponents;
    }
};

#endif /* end of include guard: MODULE_LOADER_HPP_ */
