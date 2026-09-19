#ifndef HINT_MODULE_HPP_
#define HINT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "tools/fader.hpp"

class s_font;

// Draw the circle and name label at the body's screenPos
class HintModule : public BodyModule {
public:
    HintModule(const Vec3f &labelColor) :
        BodyModule(BodyModuleType::HINT), labelColor(labelColor),
        authoredLabelColor(labelColor) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
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

    // Settle the fader, as it only advances at draw
    virtual void resumeAfterHidden(ModularBody *body) override {
        fader.reset(show);
    }

    // Label font, not owned
    static void setFont(s_font *font);
    static bool show;
    static Vec3f defaultLabelColor; // config planet_names_color
protected:
    LinearFader fader; // target = show
    Vec3f labelColor;
    Vec3f authoredLabelColor; // as given by the data
    static s_font *hintFont;
};

#endif /* end of include guard: HINT_MODULE_HPP_ */
