#include "PhotosphereLoader.hpp"
#include "experimentalModule/bodyModules/PhotosphereModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "ojmModule/objl_mgr.hpp"

uint8_t PhotosphereLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    // 200 > BasicMeshLoader's 16 and > LayeredMeshLoader's layered bids: a star
    // draws its own surface whatever texture layers it also carries (a night
    // side or a specular map on a light source is meaningless). No 255
    // short-circuit - leave headroom, as StarLoader does.
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
        // not distance-gated (PhotosphereModule.hpp; design note §3.2).
        addNearComponent(target, mesh.get());
        return mesh;
    }
    return nullptr;
}
