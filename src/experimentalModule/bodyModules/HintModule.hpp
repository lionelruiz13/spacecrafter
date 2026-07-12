#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

// HINT slot - circle marker + name label around a body (landing zone of the
// old Hints collaborator; hints.cpp read Body privates as a friend, this
// module reads only the public ModularBody interface: getScreenPos,
// getScreenSize, getNameI18n).
// Circle: drawn through Renderer::drawHint (DrawHelper hint-batch seam borrow,
// same class as the halo's Halo::global poke - both dissolve into the
// Renderer batching service, INTENT 10.4.6). Position source = the SAME
// screenPos as the halo, so hint/halo/body coincide by construction.
// Label: PENDING - the Projector gravity-text channel was invalidated
// [vixy: 2026-07-11, INTENT 10.3.7]; the replacement (Renderer text service,
// pure function of screenPos + viewport + font - projection-paths.md C7) is
// an open convergence point (INTENT open #3). The font seam (setFont, wired
// from SSystemFactory::registerFont) is kept for that landing.
// Regime: far components (2D screen-space, no depth - drawn behind the body).
// KNOWN DELTA vs old path: farComponents are skipped at screenSize > 20%
// (ModularBody::draw contract); the old path drew hints on arbitrarily large
// discs. Accepted: at >20% the circle is buried in the disc; revisit with the
// label port if name-on-disc matters.
// Deduction rule: every named body receives a HINT module by default;
// param hint=false suppresses it (ModularBody::deduceBodyModuleList). Global
// toggle via show (SSystemFactory::setFlagHints seam - sets BOTH paths).
// Fading: old per-body LinearFader not yet ported (binary show for now); the
// fader member is the landing slot.
class HintModule : public BodyModule {
public:
    HintModule(const Vec3f &labelColor) :
        BodyModule(BodyModuleType::HINT), labelColor(labelColor) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static void setFont(s_font *font) {
        hintFont = font;
    }
    static bool show; // Global hint visibility (old setFlagHints)
    // Default label color (config planet_names_color) - wired at the
    // SSystemFactory::setDefaultBodyColor seam, like the old BodyColor default.
    static Vec3f defaultLabelColor;
protected:
    LinearFader fader;
    Vec3f labelColor; // Per-body label color (old BodyColor::getLabel)
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
