#ifndef OJM_LOADER_HPP_
#define OJM_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the OJM slot (OjmModule - artificial 3D-model bodies). Bids on
// model_name + type=Artificial, the same compound key the deduction uses
// (ModularBody::deduceBodyModuleList; the OTHER model_name consumer - named
// ObjL on non-artificial bodies - is BasicMeshLoader's, old parse
// protosystem.cpp:634-641 vs 727-739).
// Load side carries the old Artificial ctor parity (body_artificial.cpp:87-93):
// model path = model3D/<name>/<name>.ojm, body radius scaled by the model's
// own radius (initialRadius *= obj3D->getRadius()), load failure -> radius 0
// (never drawn); PLUS the halo suppression the old class did by overriding
// drawHalo to nothing.
class OjmLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: OJM_LOADER_HPP_ */
