#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

// Circle marker + name label at the body's screenPos (far component: 2D screen space, no depth)
class HintModule : public BodyModule {
public:
    HintModule(const Vec3f &labelColor) :
        BodyModule(BodyModuleType::HINT), labelColor(labelColor),
        authoredLabelColor(labelColor) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Answers the LABEL channel only; the base no-op handles every other channel
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

    // The fader only advances at draw (a far component gets no update): settle it when the body is unhidden
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(show);
    }

    // Label font, not owned
    static void setFont(s_font *font);
    static bool show; // Global hint visibility (setFlagHints)
    // Label color of a body whose data gives none (config planet_names_color)
    static Vec3f defaultLabelColor;
protected:
    LinearFader fader; // per-body, target = show
    Vec3f labelColor;
    Vec3f authoredLabelColor; // what the DATA gave it (baseline of a saved color change)
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
