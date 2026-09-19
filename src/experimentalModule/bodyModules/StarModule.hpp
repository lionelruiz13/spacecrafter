#ifndef STAR_MODULE_HPP_
#define STAR_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// Draw the big additive halo of a star at the body's screenPos
class StarModule : public BodyModule {
public:
    StarModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Ignore the big_halo_size of the data
    static void setSunHaloSize(float s) { sunHaloSize = s; }
protected:
    void drawBigHalo(Renderer &renderer, ModularBody *body);
    static float sunHaloSize;
};

#endif /* end of include guard: STAR_MODULE_HPP_ */
