#include "PhotosphereLoader.hpp"
#include "experimentalModule/bodyModules/PhotosphereModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "ojmModule/objl_mgr.hpp"

uint8_t PhotosphereLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return (target->isStar() && !params["tex_map"].empty()) ? 200 : 0;
}

bool PhotosphereLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<PhotosphereModule *>(module);
}

std::unique_ptr<BodyModule> PhotosphereLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Same mesh selection as BasicMeshLoader (the sphere, or a named model).
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
        auto mesh = std::make_unique<PhotosphereModule>(obj, params["tex_map"]);
        // NEAR regime, exactly where BasicMesh routes - the emissive base is
        // not distance-gated (PhotosphereModule.hpp; design note S3.2).
        addNearComponent(target, mesh.get());
        return mesh;
    }
    return nullptr;
}
