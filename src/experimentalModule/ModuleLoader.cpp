#include "ModuleLoader.hpp"

bool ModuleLoader::reroute(ModularBody *target, BodyModule *module, const std::string &relation)
{
    // Resolve the destination FIRST - an unknown name must leave the module's
    // current routing untouched (the loader's choice stays, caller logs).
    void (*add)(ModularBody *, BodyModule *);
    if (relation == "far")
        add = addFarComponent;
    else if (relation == "near")
        add = addNearComponent;
    else if (relation == "grounded")
        add = addGroundedComponent;
    else if (relation == "in")
        add = addInComponent;
    else if (relation == "orbit")
        add = addOrbitComponent;
    else if (relation == "trail")
        add = addTrailComponent;
    else if (relation == "tail")
        add = addTailComponent;
    else
        return false;
    std::erase(target->farComponents, module);
    std::erase(target->nearComponents, module);
    std::erase(target->groundedComponents, module);
    std::erase(target->inComponents, module);
    std::erase(target->orbitComponents, module);
    std::erase(target->trailComponents, module);
    std::erase(target->tailComponents, module);
    add(target, module);
    return true;
}
