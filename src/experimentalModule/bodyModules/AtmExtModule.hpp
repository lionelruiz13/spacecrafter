#ifndef ATM_EXT_MODULE_HPP_
#define ATM_EXT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// ATMOSPHERE slot - the FROM-SPACE atmosphere rim shell around a body
// (landing zone of the old AtmExt). Decision 2026-07-11: the two things the
// old path called "atmosphere" split - this per-body rim is a BodyModule;
// the from-ground sky (+ BodyDecor visibility logic) is an EnvironmentModule
// concern. One word, two features, two homes.
// Color pass, depth-aware (drawn around the body in the body's depth slice).
// Data: atmosphere table/color/radius-factor params (body data), sun
// position (ModularBody::getLightPosition), body oblateness (public).
// Regime: near components; the old gating (screen_sz > 10, angular size,
// distance) becomes this module's own early-out in draw - regime picks the
// candidate set, the module refines.
// Deduction rule: bodies with atmosphere params (tableAtmosphere present).
class AtmExtModule : public BodyModule {
public:
    AtmExtModule() : BodyModule(BodyModuleType::ATMOSPHERE) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
protected:
    Vec3f atmColor;
    float radiusFactor; // atmosphere radius = body radius * factor
};

#endif /* end of include guard: ATM_EXT_MODULE_HPP_ */
