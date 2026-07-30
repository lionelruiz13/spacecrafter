#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

// HINT slot - circle marker + name label around a body (replaces the old
// Hints collaborator; hints.cpp read Body privates as a friend, this module
// reads only the public ModularBody interface: getScreenPos, getScreenSize,
// getNameI18n).
// Circle: Renderer::drawHint -> HINT batched service family (the DrawHelper
// DRAW_HINT_POS seam borrow is dissolved, 2026-07-12). Position source = the
// SAME screenPos as the halo, so hint/halo/body coincide by construction.
// Label: Renderer::printGravity (text service - pure function of screenPos +
// viewport + font, projection-paths.md C7; INTENT open #3 accepted). String =
// getNameI18n (old getSkyLabel returned nameI18 - its "+ scaling" comment was
// stale); shift = 10 + onScreenSizePx/2 both axes (old drawHints); color =
// labelColor with the fader interstate as alpha (old Color(label, fader)).
// Font: setFont, wired at the SSystemFactory::registerFont seam - the SAME
// s_font object as the old path (shared render cache).
// Fading: per-body LinearFader (default 2000 ms, old hint_fader), target =
// the global show flag, assigned per draw (idempotent on unchanged target).
// TICK-AT-DRAW divergence, deliberate: far-routed modules receive no update()
// calls (updateCache iterates near/in only), so the fader advances by
// ModularBody::deltaTime at draw time. Consequence: a fade freezes while the
// body is not drawn - invisible then by definition; on re-entry the ramp
// resumes instead of being settled (sub-second transient; the old path
// ticked faders in wall time regardless of visibility).
// Regime: far components (2D screen-space, no depth - drawn behind the body).
// KNOWN DELTA vs old path (unchanged by the label port, Vixy decision
// pending): farComponents are skipped at screenSize > 20% - the old path
// drew hint labels on arbitrarily large discs, so a zoomed-in planet shows
// its name in the old phase only. Escalated with the label port (was
// circle-only before): INTENT 11.19 residual.
// Old isolate-selected interplay (SolarSystemSelected: selected body keeps /
// inverts hints) is seam-level policy - lands with the per-body flag seam
// (S6), not here.
// Deduction rule: every named body receives a HINT module by default;
// param hint=false suppresses it (ModularBody::deduceBodyModuleList). Global
// toggle via show (SSystemFactory::setFlagHints seam - sets BOTH paths).
class HintModule : public BodyModule {
public:
    HintModule(const Vec3f &labelColor) :
        BodyModule(BodyModuleType::HINT), labelColor(labelColor) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Runtime label-color seam (old Body::setColor "label"). Self-selects on
    // the LABEL channel; the base no-op handles every other channel.
    void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::LABEL || type == BodyColorType::ALL)
            labelColor = c;
    }

    // THE UNHIDE EDGE (B39 §11.117 / D23 clause iv): this module's fader is
    // ticked AT DRAW (see the header note above), so while the body was hidden it
    // did not advance at all. Snap it to the global target the next draw would
    // have aimed at - the header already records that a fade "freezes while the
    // body is not drawn" and that on re-entry "the ramp resumes instead of being
    // settled"; for the HIDDEN case D23 makes settled the required answer.
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(show);
    }

    static void setFont(s_font *font);
    static bool show; // Global hint visibility (old setFlagHints)
    // Default label color (config planet_names_color) - wired at the
    // SSystemFactory::setDefaultBodyColor seam, like the old BodyColor default.
    static Vec3f defaultLabelColor;
protected:
    LinearFader fader; // per-body, 2000 ms default = old hint_fader
    Vec3f labelColor; // Per-body label color (old BodyColor::getLabel)
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
