#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

// HINT slot - circle marker + name label around a body (landing zone of the
// old Hints collaborator; hints.cpp read Body privates as a friend, this
// module reads only the public ModularBody interface: getScreenPos,
// getScreenSize, getNameI18n).
// Label rendering goes through the Projector gravity-text path with a shared
// font - the font arrives via setFont (wired from SSystemFactory::registerFont
// at the seam), replacing the old static Body::planet_name_font friend-read.
// Regime: far components (2D screen-space, no depth - drawn behind the body).
// Deduction rule: every named body receives a HINT module by default;
// param hint=false suppresses it. Global toggle via show (setFlagHints seam).
// Implementation: D2 verification slice.
class HintModule : public BodyModule {
public:
    HintModule() : BodyModule(BodyModuleType::HINT) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static void setFont(s_font *font) {
        hintFont = font;
    }
    static bool show; // Global hint visibility (old setFlagHints)
protected:
    LinearFader fader;
    Vec3f labelColor; // Per-body label color (old BodyColor::getLabel)
    Vec3f circleColor;
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
