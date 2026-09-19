#ifndef MODULE_LOADER_MGR_HPP_
#define MODULE_LOADER_MGR_HPP_

#include "BodyModule.hpp"
#include <memory>
#include <map>
#include <vector>

class ModuleLoader;
class OrbitLoader;
class Orbit;

class ModuleLoaderMgr {
public:
    ModuleLoaderMgr();
    ~ModuleLoaderMgr();
    // Initialize every Loader, declared in modules.cpp
    void init();
    void registerModule(BodyModuleType type, std::unique_ptr<ModuleLoader> loader);
    void registerModule(const std::string &key, std::unique_ptr<OrbitLoader> loader);
    void registerModule(std::unique_ptr<OrbitLoader> loader);
    void loadModule(BodyModuleType type, ModularBody *target, std::map<std::string, std::string> &params, const std::string &slot = {});
    std::unique_ptr<Orbit> loadOrbit(std::map<std::string, std::string> &params);
    static std::string_view moduleTypeName(BodyModuleType type);
    // Reverse lookup for the `module =` key; ok=false on an unknown name
    // (caller logs the valid values, S2(f) - see moduleTypeNames()).
    static BodyModuleType moduleTypeFromName(const std::string &name, bool &ok);
    // Every valid `module =` value, comma-separated - for actionable logs.
    static std::string moduleTypeNames();
    static ModuleLoaderMgr instance;
private:
    std::vector<std::unique_ptr<ModuleLoader>> loaders[static_cast<uint8_t>(BodyModuleType::NB_MODULE_TYPE)];
    std::map<std::string, std::unique_ptr<OrbitLoader>> orbitLoaders;
    std::unique_ptr<OrbitLoader> defaultOrbitLoader;
};

#endif /* end of include guard: MODULE_LOADER_MGR_HPP_ */
