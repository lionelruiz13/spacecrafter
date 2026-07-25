#include <string>
#include <cmath>
#include "experimentalModule/ModuleLoader.hpp"
#include "experimentalModule/meshModules/LayeredMesh.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "LayeredMeshLoader.hpp"

uint8_t LayeredMeshLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    if (params["tex_map"].empty())
        return 0;
    if (params["tex_night"].empty() && params["tex_normal"].empty() && params["tex_heightmap"].empty())
        return 0; // plain body - BasicMeshLoader's case
    return 24; // outbid BasicMeshLoader (16)
}

bool LayeredMeshLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<LayeredMesh*>(module);
}

std::unique_ptr<BodyModule> LayeredMeshLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
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
    if (!obj)
        return nullptr;
    LayeredMesh::Config cfg;
    cfg.day = params["tex_map"];
    cfg.night = params["tex_night"];
    cfg.specular = params["tex_specular"];
    cfg.normal = params["tex_normal"];
    cfg.heightmap = params["tex_heightmap"];
    const bool night = !cfg.night.empty();
    const bool normal = !cfg.normal.empty();
    const bool heightmap = !cfg.heightmap.empty();
    const bool specular = !cfg.specular.empty();
    // B27 A6 (§11.73): the surface-lighting lineage is a DECLARED capability of
    // the BODY (`surface_model`, D10key §11.79(e)), resolved once by the loader
    // authority (ModularSystem::loadBody) - which is where the D14 format scope
    // lives (legacy `type = Moon` still grants it, a composed file must declare
    // it). Reading it from the body instead of re-reading `params["type"]` here
    // is what unblocks a lunar-lineage surface on ANY body (I4: the behaviour is
    // the body's capability, not its type string).
    cfg.moonClass = (target->getSurfaceModel() == SurfaceModel::LUNAR);
    // Row/family selection - the old selectShader ORDER (header comment):
    if (cfg.moonClass) {
        if (heightmap) {
            cfg.tessellated = true;
            cfg.midVariant = normal ? MeshFamilies::VARIANT_BUMP : 0;
        } else {
            cfg.tessellated = false;
            cfg.midVariant = night ? MeshFamilies::VARIANT_NIGHT : MeshFamilies::VARIANT_BUMP;
        }
    } else {
        if (night && heightmap && specular) {
            cfg.tessellated = true;
            cfg.midVariant = MeshFamilies::VARIANT_NIGHT;
        } else if (night) {
            if (heightmap)
                cLog::get()->write("LayeredMesh: '" + target->getEnglishName() + "' has tex_night+tex_heightmap but no tex_specular - degrading to the flat night row (the old my_earth class requires a specular map)", LOG_TYPE::L_WARNING);
            cfg.tessellated = false;
            cfg.midVariant = MeshFamilies::VARIANT_NIGHT;
        } else if (normal) {
            cfg.tessellated = false;
            cfg.midVariant = MeshFamilies::VARIANT_BUMP;
        } else { // heightmap only
            cfg.tessellated = true;
            cfg.midVariant = 0;
        }
    }
    cfg.rayCapable = normal && heightmap;
    cfg.rayVariant = (night && specular) ? MeshFamilies::VARIANT_NIGHT : 0;
    // Atmosphere-ambient params of the ray-march lighting (old parse
    // formulas, protosystem.cpp:875-881; 0 defaults - strToFloat("") == 0).
    cfg.atmColor = Vec3f(
        Utility::strToFloat(params["atmosphere_ambient_r"]),
        Utility::strToFloat(params["atmosphere_ambient_g"]),
        Utility::strToFloat(params["atmosphere_ambient_b"]));
    cfg.sunDeviation = sinf(Utility::strToFloat(params["atmosphere_sun_deviation"]) * M_PI / 180.f);
    cfg.atmDeviation = sinf(Utility::strToFloat(params["atmosphere_ambient_deviation"]) * M_PI / 180.f);
    auto mesh = std::make_unique<LayeredMesh>(obj, std::move(cfg));
    addNearComponent(target, mesh.get());
    return mesh;
}
