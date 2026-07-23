#include <string>
#include "experimentalModule/ModuleLoader.hpp"
#include "experimentalModule/meshModules/BasicMesh.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "BasicMeshLoader.hpp"

BasicMeshLoader *BasicMeshLoader::instance = nullptr;

// (eclipse-map LUT retired at S5 - Gen-2 projected shadows replace it
//  outright [vixy: 2026-07-12], shadow-paths.md B4)
BasicMeshLoader::BasicMeshLoader()
{
    instance = this;
}

BasicMeshLoader::~BasicMeshLoader()
{
    instance = nullptr;
}

uint8_t BasicMeshLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    if (params["tex_map"].empty())
        return 0;
    return 16; // Leave enough lower and higher values to simplify adding module loaders
}

bool BasicMeshLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<BasicMesh*>(module); // dynamic_cast return nullptr if the module is not a BasicMesh
}

std::unique_ptr<BodyModule> BasicMeshLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
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
        auto mesh = std::make_unique<BasicMesh>(obj, params["tex_map"]);
        addNearComponent(target, mesh.get());
        return mesh;
    }
    return nullptr;
}
