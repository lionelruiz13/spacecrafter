#ifndef PHOTOSPHERE_MODULE_HPP_
#define PHOTOSPHERE_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp"
#include "experimentalModule/meshModules/SkinnableColorMap.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
class ObjL;

// The SURFACE member of the near-surface star family (B12; design note
// claude/b12-design.md). It fills the body's MESH slot on a STAR - the disc a
// star shows - where a planet's slot holds a BasicMesh.
//
// WHY A TYPE AND NOT A MESH VARIANT (I4). A star's disc is not a lit surface
// with a flag flipped: it EMITS. The observable Q21 asks for - limb darkening
// now, granulation/spots as content, a chromosphere shell beside it - is the
// behaviour of the photosphere, so it belongs to a photosphere type; parking
// it on the generic planet mesh would make that type carry star physics it has
// no other use for, and would leave the shell (a second geometry, additive)
// with nowhere to live. §4.3 of the design note.
//
// HOW IT REPLACES THE MESH: by G6 loader COMPETITION, not by a suppression
// rule. deduceBodyModuleList still emits MESH for any body with tex_map;
// PhotosphereLoader bids 200 on isStar() where BasicMeshLoader bids 16, so the
// star's one MESH slot is filled by this module and no draw path ever asks
// whether a body is a star. Nothing in ModularBody changes.
//
// REGIME: nearComponents, exactly where BasicMesh routes. NOT distance-gated -
// a star's surface brightness is distance-invariant (radiance is conserved
// along a ray), and the divergence this retires is measured at 1 AU (§11.44,
// zoom fov 30: new disc median 21 vs old 725). What the near regime will add
// later is DETAIL (granulation/spots), whose boundary is the G4 criterion
// "smallest added structure >= 1 px" - design note §3.
//
// THE LAW (fragment shader bodyStarSurface.frag): Eddington grey-atmosphere
// limb darkening L(mu) = (3*mu + 2)/5, DERIVED, not fitted and not recalled -
// there is no measured coefficient in it (derivation: design note §5.1). At
// disc centre L(1) = 1, so the centre reproduces the old path's body_sun.frag
// (the texel, unmodified) exactly: the change is monotone and anchored.
//
// TRAITS: BMT_USE_DEPTH | BMT_DEPTH_TRACE. The shadow traits BasicMesh carries
// are deliberately NOT declared and the drop is provably inert: the shadow
// orchestration already skips isStar() bodies in the caster scan, the receiver
// scan and the self-shadow nomination (ModularSystem.cpp:321, 375, 404), so
// the Sun's mesh has declared them since row 1 without ever being selected.
class PhotosphereModule : public BodyModule {
public:
    PhotosphereModule(ObjL *mesh, const std::string &texturePath);
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE;
    }
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Skin seam: a star's map is swappable like any other body's - the shipped
    // Sun's own data carries `#tex_skin = bodies/sun304A-sdo.jpg`, so this is a
    // demonstrated use, not a hypothetical one. Rules in SkinnableColorMap.
    virtual void createTexSkin(const std::string &texName) override;
    virtual void switchTexSkin(bool use) override;
    bool getSkinUse(bool &out) const override { out = colorMap.isSkinUsed(); return true; }
private:
    // The one binding site (mirrors BasicMesh::bindColor; the set contract is
    // this family's, which is why the binding cannot live in the shared map).
    void bindColor(Texture &color);
    void fillVert(ModularBody *body, const Mat4f &mat);
    bool loaded = false;
    ObjL *mesh;
    SkinnableColorMap colorMap;
    PipelineFamily family;
    std::unique_ptr<Set> set;
    // globalVertProj REUSED verbatim (I2: one CPU authority for the sphere
    // vertex block). planetRadius/LightPosition are unread by this family's
    // vertex stage - filled anyway so the block is never partly undefined.
    SharedBuffer<globalVertProj> vert;
};

#endif /* end of include guard: PHOTOSPHERE_MODULE_HPP_ */
