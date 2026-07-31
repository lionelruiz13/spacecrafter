#ifndef PHOTOSPHERE_LOADER_HPP_
#define PHOTOSPHERE_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// MESH-family loader of the near-surface star family (B12). Co-registered with
// BasicMeshLoader (16) and LayeredMeshLoader under BodyModuleType::MESH; bids
// 200 on a body that EMITS light and has a colour map, 0 on everything else -
// so a star's one MESH slot holds the photosphere and every planet's holds
// what it held before, with no gate on any draw path (G6: the loaders answer
// "which kind of surface is this body's surface", which is the question
// competitive selection exists for).
//
// The bid reads isStar() - the STAR bit, "a body who emit light" - and NOT the
// legacy `type` string: the composed format carries the same capability as
// `light_source = true` (ModularSystem.cpp:1129-1147, and generateComposedTwin
// emits it at :1732), so a composed twin of a star re-selects this module. A
// type-string bid would have made every twin's star silently revert to the lit
// planet mesh.
class PhotosphereLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: PHOTOSPHERE_LOADER_HPP_ */
