#ifndef STAR_MODULE_HPP_
#define STAR_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// BODY-slot module family for stars (landing zone of the old Sun/BodyStar
// subclasses - big textured halo + volumetric star viewer). The old path
// encoded "star" in the class hierarchy (Sun::drawGL override, private
// pipelineBigHalo); here the body's STAR nature (BodyType bit, isStar()) is
// data, and the star's appearance is this module.
//
// LANDING-ZONE RECONCILIATION (Opus 4.8, 2026-07-19, INTENT §11.44):
// - The reachable feature is the SUN's BIG HALO (old Sun::drawBigHalo +
//   sun_big_halo.{vert,geom,frag} + tex_big_halo, additive screen-space glow,
//   no depth) - the dominant remaining cross-phase A/B residual (§11.19a,
//   §11.19, §11.21, §11.22, §11.24: "the SUN's big-halo glow ... vs bare
//   disc"). It is drawn by drawBigHalo() below.
// - The Sun's SURFACE disc already renders through the MESH module (tex_map,
//   §11.19a "bare disc" == old body_sun.frag, measured parity), so this module
//   draws NO surface for the Sun; the header's draw()=="volumetric star
//   surface (StarViewer)" is the BodyStar case (type=Star), which is
//   UNREACHABLE in shipped data (default_ssystem.ini has only type=Sun) - its
//   STAR_VIEWER / CORONA families (big_star_halo.*, big_star_corona.frag) are
//   SUSPENDED for Vixy (INTENT §11.44 / §12 row 14).
// - Regime: FAR component. The big halo is a screen-space glow at
//   ModularBody::screenPos (ignores the surface matrix), so it belongs where
//   farComponents draw - BEFORE clearDepth+the near disc, in every regime the
//   Sun spans (surface / mid-band / far). Far components are invoked through
//   draw(); drawNoDepth() delegates to the same helper defensively (a far
//   component never receives it, but the header names the halo there).
// - color: comes LIVE from ModularBody::getHaloColor() each frame (== old
//   myColor->getHalo(), respects the color-command seam); no stored member.
// - Light emission stays body-level (ModularBody::updateAsLightSource, driven
//   by ModularSystem's star designation) - NOT this module.
// HALO SIZE (the load-bearing correction, INTENT §11.44): the ssystem.ini
// big_halo_size is DEAD for the main Sun - old SolarSystem::setFlagSunScale
// OVERWRITES it (setHaloSize(200) unscaled, 200+SunScale*40 scaled;
// solarsystem.hpp:96-107), called at init from config flag_sun_scaled
// (core.cpp:566). So the effective size is the SUN-SCALE value (base 200),
// not the data's 1. sunHaloSize is a static (one reachable Sun) driven by the
// SSystemFactory::setFlagSunScale seam - the new-path mirror of that override.
// Data: halo texture (Renderer SUN_HALO service resource, §9 load-time only).
// Deduction rule: STAR-typed bodies with tex_big_halo (deduceBodyModuleList).
class StarModule : public BodyModule {
public:
    StarModule() : BodyModule(BodyModuleType::CUSTOM) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Sun-scale halo-size seam (old SolarSystem::setFlagSunScale override):
    // SSystemFactory::setFlagSunScale computes 200 (unscaled) / 200+SunScale*40
    // (scaled) and sets it here - the new-path route of the size the old sun
    // draws with. Default = the unscaled base (config default flag_sun_scaled=
    // false).
    static void setSunHaloSize(float s) { sunHaloSize = s; }
protected:
    // Faithful port of old Sun::drawBigHalo (body_sun.cpp:171-192): compute
    // rmag/cmag/radius from the body state and queue the SUN_HALO service.
    void drawBigHalo(Renderer &renderer, ModularBody *body);
    static float sunHaloSize;
};

#endif /* end of include guard: STAR_MODULE_HPP_ */
