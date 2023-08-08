#include "ModuleLoaderMgr.hpp"
#include "ModuleLoader.hpp"
#include "OrbitLoader.hpp"

ModuleLoaderMgr ModuleLoaderMgr::instance;

ModuleLoaderMgr::ModuleLoaderMgr()
{
}

ModuleLoaderMgr::~ModuleLoaderMgr()
{
}

void ModuleLoaderMgr::registerModule(BodyModuleType type, std::unique_ptr<ModuleLoader> loader)
{
    loaders[static_cast<uint8_t>(type)].push_back(std::move(loader));
}

void ModuleLoaderMgr::registerModule(const std::string &key, std::unique_ptr<OrbitLoader> loader)
{
    orbitLoaders[key] = std::move(loader);
}

void ModuleLoaderMgr::registerModule(std::unique_ptr<OrbitLoader> loader)
{
    defaultOrbitLoader = std::move(loader);
}

void ModuleLoaderMgr::loadModule(BodyModuleType type, ModularBody *target, std::map<std::string, std::string> &params)
{
    uint8_t i = 0;
    ModuleLoader *loader;
    for (auto &l : loaders[static_cast<uint8_t>(type)]) {
        uint8_t tmp = l->isLikely(target, params);
        if (tmp > i) {
            loader = l.get();
            if (tmp == UINT8_MAX)
                break;
            i = tmp;
        }
    }
    if (loader) {
        loader->load(target, params);
    }
}

std::unique_ptr<Orbit> ModuleLoaderMgr::loadOrbit(std::map<std::string, std::string> &params)
{
    std::unique_ptr<Orbit> ret;
    try {
        ret = orbitLoaders.at(params["coord_func"])->load(params);
    } catch (...) {
        ret = defaultOrbitLoader->load(params);
    }
    return ret;
}
