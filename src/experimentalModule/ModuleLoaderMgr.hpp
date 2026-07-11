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
    // Load one module onto target. Loader selection is competitive: highest
    // isLikely() among the loaders registered for this type wins (255
    // short-circuits). Slot identity defaults to the type's name; an explicit
    // `slot` creates/replaces that named slot instead (several modules of one
    // type per body). No capable loader -> logged warning, nothing installed.
    // Module declaration model (decision 2026-07-11): deduction from body
    // params (ModularBody::deduceBodyModuleList) extended per ported family,
    // PLUS explicit declarations in params for overrides and custom slots -
    // the param syntax lands with the first multi-module body (D2).
    void loadModule(BodyModuleType type, ModularBody *target, std::map<std::string, std::string> &params, const std::string &slot = {});
    std::unique_ptr<Orbit> loadOrbit(std::map<std::string, std::string> &params);
    static ModuleLoaderMgr instance;
private:
    std::vector<std::unique_ptr<ModuleLoader>> loaders[static_cast<uint8_t>(BodyModuleType::NB_MODULE_TYPE)];
    std::map<std::string, std::unique_ptr<OrbitLoader>> orbitLoaders;
    std::unique_ptr<OrbitLoader> defaultOrbitLoader;
};

#endif /* end of include guard: MODULE_LOADER_MGR_HPP_ */
