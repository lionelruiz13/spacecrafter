#include <string>
#include "experimentalModule/ModuleLoader.hpp"
#include "experimentalModule/bodyModules/AtmExtModule.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "tools/utility.hpp"
#include "AtmExtLoader.hpp"

uint8_t AtmExtLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    if (params["atmosphere_ext_model"].empty())
        return 0;
    return 16; // same slot-uncontested convention as BasicMeshLoader
}

bool AtmExtLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<AtmExtModule*>(module);
}

std::unique_ptr<BodyModule> AtmExtLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Shell geometry = the body's own sphere mesh source (old: currentObj) -
    // same acquisition as BasicMeshLoader so both resolve to the same ObjL.
    ObjL *obj;
    {
        const std::string &modelName = params["model_name"];
        if (modelName.empty()) {
            obj = ObjLMgr::instance->selectDefault();
        } else {
            ObjLMgr::instance->insertObj(modelName);
            obj = ObjLMgr::instance->select(modelName);
        }
    }
    if (obj) {
        // Old parse parity (protosystem.cpp:871): factor default 1.05; the
        // gradient path is passed RAW to s_texture (old AtmExt ctor did the
        // same with tableAtmosphere - s_texture resolves internally).
        const float radiusFactor = params["atmosphere_radius_factor"].empty()
            ? 1.05f : static_cast<float>(Utility::strToDouble(params["atmosphere_radius_factor"]));
        auto shell = std::make_unique<AtmExtModule>(obj, params["atmosphere_ext_model"], radiusFactor);
        // nearComponent, loaded AFTER the MESH module (deduction order) so
        // the shell records after the disc in the body's command buffer -
        // old drawBody/drawAtmExt order, same depth slice.
        addNearComponent(target, shell.get());
        return shell;
    }
    return nullptr;
}
