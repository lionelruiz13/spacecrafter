#include <string>
#include "experimentalModule/ModuleLoader.hpp"
#include "experimentalModule/meshModules/BasicMesh.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "BasicMeshLoader.hpp"

BasicMeshLoader *BasicMeshLoader::instance = nullptr;

BasicMeshLoader::BasicMeshLoader(const std::string &eclipseMap) :
    texEclipseMap(eclipseMap, TEX_LOAD_TYPE_PNG_SOLID)
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
    return 16; // Leave enough lower and higher values to siplify adding module loaders
}

bool BasicMeshLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<BasicMesh*>(module); // dynamic_cast return nullptr if the module is not a BasicMesh
}

void BasicMeshLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
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
    if (obj)
        getNearComponents(target).push_back(std::make_shared<BasicMesh>(obj, params["tex_map"]));
}
