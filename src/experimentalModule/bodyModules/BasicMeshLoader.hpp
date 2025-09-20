#ifndef BASIC_MESH_LOADER_HPP_
#define BASIC_MESH_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

class BasicMeshLoader : public ModuleLoader {
    friend class BasicMesh;
public:
    BasicMeshLoader(const std::string &eclipseMap);
    virtual ~BasicMeshLoader();
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual void load(ModularBody *target, std::map<std::string, std::string> &params) override;
    static BasicMeshLoader *instance;
private:
    s_texture texEclipseMap;
};

#endif /* end of include guard: BASIC_MESH_LOADER_HPP_ */
