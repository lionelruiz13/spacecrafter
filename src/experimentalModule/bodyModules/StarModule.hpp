#ifndef STAR_MODULE_HPP_
#define STAR_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// BODY-slot module family for stars (landing zone of the old Sun/BodyStar
// subclasses - big textured halo + volumetric star viewer). The old path
// encoded "star" in the class hierarchy (Sun::drawGL override, private
// pipelineBigHalo); here the body's STAR nature (BodyType bit, isStar()) is
// data, and the star's appearance is this module.
// Hooks:
// - draw: volumetric/textured star surface (old StarViewer path) when
//   resolved.
// - drawNoDepth: the big halo glow (old tex_big_halo/pipelineBigHalo) - a
//   screen-space glow, no depth.
// The small point-halo fallback stays in ModularBody::drawHalo (already
// ported core behavior, not a module).
// Light emission: the body IS the light source - ModularBody::
// updateAsLightSource, driven by ModularSystem (star designation), not by
// this module.
// Data: brightness (createInfo), halo texture (resource layer), star color.
// Deduction rule: STAR-typed bodies with tex_big_halo or star params.
class StarModule : public BodyModule {
public:
    StarModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
protected:
    float bigHaloSize;
    Vec3f color;
};

#endif /* end of include guard: STAR_MODULE_HPP_ */
