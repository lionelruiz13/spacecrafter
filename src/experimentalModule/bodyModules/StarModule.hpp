#ifndef STAR_MODULE_HPP_
#define STAR_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

class StarModule : public BodyModule {
public:
    StarModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    static void setSunHaloSize(float s) { sunHaloSize = s; }
protected:
    // Faithful port of old Sun::drawBigHalo (body_sun.cpp:171-192): compute
    // rmag/cmag/radius from the body state and queue the SUN_HALO service.
    void drawBigHalo(Renderer &renderer, ModularBody *body);
    static float sunHaloSize;
};

#endif /* end of include guard: STAR_MODULE_HPP_ */
