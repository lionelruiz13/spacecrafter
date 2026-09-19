#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

class HintModule : public BodyModule {
public:
    HintModule(const Vec3f &labelColor) :
        BodyModule(BodyModuleType::HINT), labelColor(labelColor),
        authoredLabelColor(labelColor) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Runtime label-color seam (old Body::setColor "label"). Self-selects on
    // the LABEL channel; the base no-op handles every other channel.
    bool getColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::LABEL)
            return false;
        out = labelColor;
        return true;
    }
    void captureAuthored() override { authoredLabelColor = labelColor; }
    bool getAuthoredColor(BodyColorType type, Vec3f &out) const override {
        if (type != BodyColorType::LABEL)
            return false;
        out = authoredLabelColor;
        return true;
    }
    void setColor(BodyColorType type, const Vec3f &c) override {
        if (type == BodyColorType::LABEL || type == BodyColorType::ALL)
            labelColor = c;
    }

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
    Vec3f authoredLabelColor; // what the DATA gave it (D30's delta baseline)
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
