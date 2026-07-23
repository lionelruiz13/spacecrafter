#ifndef LAYERED_MESH_LOADER_HPP_
#define LAYERED_MESH_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the layered disc (row 2): outbids BasicMeshLoader (16) with 24
// when any layered texture key (tex_night / tex_normal / tex_heightmap) is
// present alongside tex_map. The row/family/regime rules mirror the OLD
// selectShader ORDER exactly, including the Moon/Planet split - the ini
// `type` string is the carrier (new-path BodyType maps both to CUSTOM_BODY):
//
//   type==Moon (body_moon.cpp selectShader - heightmap FIRST):
//     heightmap -> MESH_TES (+BUMP when normal present; the old
//       MOON_NORMAL_TES samples the normal map unconditionally - a
//       heightmap-only moon falls to the TES base row instead of the old
//       bind-nothing crash; no live body)
//     else night -> MESH_LAYERED+NIGHT (old MOON_NIGHT = body_night shaders)
//     else normal -> MESH_LAYERED+BUMP (old MOON_BUMP = body_bump shaders)
//   else (Planet class, body_bigbody.cpp selectShader - night, norm, heightmap):
//     night && heightmap -> MESH_TES+NIGHT (my_earth; requires specular -
//       old binds tex_specular unconditionally; absent -> MESH_LAYERED+NIGHT
//       + the degradation is visible in the loader log; no live body)
//     else night -> MESH_LAYERED+NIGHT (body_night)
//     else normal -> MESH_LAYERED+BUMP (body_bump - Mars! norm tested
//       BEFORE heightmap in old, so Mars is flat at mid range)
//     else heightmap -> MESH_TES base (body_normal_tes)
//
//   rayCapable = normal && heightmap (old CoI classes TES_SHADOW /
//   NIGHT_TES_SHADOW both require them); rayVariant = NIGHT when
//   night && specular (my_earth_shadow class).
class LayeredMeshLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: LAYERED_MESH_LOADER_HPP_ */
