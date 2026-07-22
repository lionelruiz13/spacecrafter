#include "ModuleLoaderMgr.hpp"
#include "ModuleLoader.hpp"
#include "OrbitLoader.hpp"
#include "tools/log.hpp"
#include <array>

std::array<std::string_view, static_cast<uint8_t>(BodyModuleType::NB_MODULE_TYPE)> defaultModuleName{
    "CUSTOM",
    "MESH",
    "OJM",
    "VOLUMETRIC",
    "RING",
    "HINT",
    "POINTER",
    "ORBIT",
    "TRAIL",
    "TAIL",
    "ATMOSPHERE",
    "AXIS"
};

ModuleLoaderMgr ModuleLoaderMgr::instance;

std::string_view ModuleLoaderMgr::moduleTypeName(BodyModuleType type)
{
    return defaultModuleName[static_cast<uint8_t>(type)];
}

BodyModuleType ModuleLoaderMgr::moduleTypeFromName(const std::string &name, bool &ok)
{
    for (uint8_t i = 0; i < static_cast<uint8_t>(BodyModuleType::NB_MODULE_TYPE); ++i) {
        if (defaultModuleName[i] == name) {
            ok = true;
            return static_cast<BodyModuleType>(i);
        }
    }
    ok = false;
    return BodyModuleType::CUSTOM;
}

std::string ModuleLoaderMgr::moduleTypeNames()
{
    std::string ret;
    for (const auto &name : defaultModuleName) {
        if (!ret.empty())
            ret += ", ";
        ret += name;
    }
    return ret;
}

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

void ModuleLoaderMgr::loadModule(BodyModuleType type, ModularBody *target, std::map<std::string, std::string> &params, const std::string &slot)
{
    uint8_t i = 0;
    ModuleLoader *loader = nullptr;
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
        if (slot.empty())
            target->slot(ModularBody::slotID[defaultModuleName[static_cast<uint8_t>(type)]], loader->load(target, params));
        else
            target->slot(ModularBody::slotID[slot], loader->load(target, params));
    } else {
        // A deduced/requested module with no capable loader must be VISIBLE:
        // silently skipping was defect INTENT.md 5.4 (OJM/RING deduced but
        // unregistered - bodies quietly lost their mesh).
        cLog::get()->write("No loader available for module type '" + std::string(defaultModuleName[static_cast<uint8_t>(type)]) + "' requested by body '" + target->getEnglishName() + "'", LOG_TYPE::L_WARNING);
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
