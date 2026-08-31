#ifndef PLANET_GRID_MODULE_HPP_
#define PLANET_GRID_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// Custom slot "GRID" - latitude/longitude grid at ~1.05 body radius (landing
// zone of the old PlanetGrid friend). Color pass, depth-tested, drawn after
// the body surface. Data: body transform + radius; meridian/parallel colors.
// Regime: near components. Clean landing - the old code's only body coupling
// was reading mat + radius through friendship.
// Deduction rule: none (explicit declaration only, or created by the seam's
// setFlagPlanetGrid toggle) - demonstrates the explicit-slot half of the
// module declaration model (ModuleLoaderMgr::loadModule slot parameter).
//
// PORT RECONCILIATION (2026-07-19, T4 - INTENT S11.42):
//  * Shader contract = the AXIS/ORBIT PUSH family (planet_grid.vert declares
//    push_constant {mat4 ModelViewMatrix; vec3 clipping_fov} + custom_project
//    spec-8) - NOT TRAIL's cam_block UBO. Deployed shaders reused VERBATIM
//    (parity by construction). Per-vertex color (meridian/parallel baked in).
//  * `show` rides the AXIS flag: the old flag_planet_grid has NO independent
//    setter - Body::setFlagAxis sets BOTH flag_axis and flag_planet_grid=b
//    (body.cpp:229-234). The seam mirror lives in SSystemFactory::setFlagAxis.
//  * Geometry: 24 meridians + evenly-spaced parallels (equator included), plus
//    the astronomically-meaningful tropic (+/-axialTilt) and polar (+/-(90 -
//    axialTilt)) circles RESTORED (INTENT S11.57, B23 / Q16). The tilt comes
//    from ModularBody::getAxialTilt() (the ssystem.ini axial_tilt key, already
//    loaded into RotationElements::axialTilt - the S11.42 "no axial-tilt scalar"
//    was a missing getter, not missing data). Tropics ride the LINE_TROPIC
//    sky-line flag, polar circles the LINE_CIRCLE_POLAR flag - the old coupling
//    (body.cpp:1257-1258) is deliberate: they are the lines that show obliquity
//    directly. Tropics only on non-satellite non-star bodies (old's isSatellite/
//    !="Sun" gate; the name-sniff is retired for the clean isStar() type test).
//  * Colors: the header's per-instance members are fed from GLOBAL static
//    defaults (old's observable: sky-manager colors, same for every body). The
//    color-authority structural choice (per-instance vs global; sky-manager
//    sourcing) is SUSPENDED for Vixy.
class PlanetGridModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (the inline ctor's exception path instantiates the member destructor -
    // AXIS precedent).
    PlanetGridModule();
    ~PlanetGridModule();
    // Grid extends to 1.05 * scaledRadius: inflate the traced bounding radius so
    // the grid's near cap fits inside this body's depth slice (else the front
    // grid is znear-clipped by the bucket range).
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Harness instrument (INTENT S11.57): the tropic/polar latitudes actually
    // baked for this body (measured from the running process, DoD-2), plus the
    // flag gates. `hasTropics`/`bodyAxialTilt` are set at first build; before
    // that the fields read their construction defaults (a body whose grid never
    // drew). Emitted as one JSON object into ModularBody::dumpTrace's "grid".
    virtual void dumpState(std::ostream &out) const override;

    static bool show; // old flag_planet_grid (rides the axis flag)

    // Per-element sky-line flag gates (old body.cpp:1257-1258 polled these from
    // the sky managers each draw): the tropic circles ride LINE_TROPIC, the
    // polar circles ride LINE_CIRCLE_POLAR. Pushed each frame by the seam
    // (Core::syncPlanetGridSkyState -> SSystemFactory::setPlanetGridTropicPolar)
    // - a per-frame poll faithfully reproducing old's per-frame read (a true
    // I3 push would couple SkyLineMgr to this module - wrong layering).
    static bool showTropics;
    static bool showPolarCircles;

    // Global grid colors (old observable: sky-manager colors, shared by every
    // body). setColors bumps colorGeneration; each instance re-syncs its
    // per-member colors + re-bakes its vertex colors on the next draw.
    static void setColors(const Vec3f &meridian, const Vec3f &parallel);
    // Tropic/polar dynamic state (flags + colors), pushed each frame from the
    // sky-line managers (old drew tropicColor=LINE_TROPIC, polarColor=
    // LINE_CIRCLE_POLAR). Colors bump colorGeneration only on change (no
    // per-frame re-bake); flags gate the draw ranges, no re-bake.
    static void setTropicPolar(bool showTropics, bool showPolarCircles,
                               const Vec3f &tropic, const Vec3f &polarCircle);
protected:
    Vec3f meridianColor;
    Vec3f parallelColor;
    Vec3f tropicColor;
    Vec3f polarColor;

    // Lazy per-body line buffer (unit-sphere positions + baked per-vertex
    // color), globalBuffer-backed. Meridian/parallel positions are body-
    // independent, but the tropic (+/-axialTilt) and polar (+/-(90-axialTilt))
    // circles ARE body-specific (this body's obliquity) - so the buffer holds
    // this body's geometry. Colors re-baked when colorGeneration moves; the
    // scale is folded into the pushed matrix.
    std::unique_ptr<VertexBuffer> buffer;
    uint32_t vertexCount = 0;
    // Draw ranges (first-vertex offsets into the non-indexed LINE_LIST buffer).
    // Meridians + parallels are one always-on block; tropics and polar circles
    // are gated by their sky-line flags.
    uint32_t mainCount = 0;    // meridians + parallels, at offset 0
    uint32_t tropicFirst = 0;  // 0 count when the body has no tropics
    uint32_t tropicCount = 0;
    uint32_t polarFirst = 0;
    uint32_t polarCount = 0;
    bool built = false;
    uint32_t syncedColorGen = 0;
    // This body's baked geometry (set at first build; the tropic/polar circles
    // are body-specific). bodyAxialTilt in degrees (the obliquity the circles
    // read out); hasTropics = the !isSatellite && !isStar gate.
    float bodyAxialTilt = 0.f;
    bool hasTropics = false;
};

#endif /* end of include guard: PLANET_GRID_MODULE_HPP_ */
