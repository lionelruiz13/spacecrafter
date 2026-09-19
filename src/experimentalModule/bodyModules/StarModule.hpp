#ifndef STAR_MODULE_HPP_
#define STAR_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// Appearance of a STAR body, far component: the big additive screen-space halo at the body's screenPos
// Draws no surface (the disc is the MESH slot); light emission stays body-level
class StarModule : public BodyModule {
public:
    StarModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Halo size of the single Sun, driven by the sun-scale flag; the data's big_halo_size is not used
    static void setSunHaloSize(float s) { sunHaloSize = s; }
protected:
    // Queue the SUN_HALO service from the body state; color = ModularBody::getHaloColor()
    void drawBigHalo(Renderer &renderer, ModularBody *body);
    static float sunHaloSize;
};

#endif /* end of include guard: STAR_MODULE_HPP_ */
