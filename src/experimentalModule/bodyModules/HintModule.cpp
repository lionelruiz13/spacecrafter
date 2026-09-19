#include "HintModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "bodyModule/hints.hpp" // hintCircleRadius (shared shape/shift authority)
#include "tools/s_font.hpp"     // getFontSize (setFont radius mirror)
#include <cmath>

bool HintModule::show = false;
Vec3f HintModule::defaultLabelColor{};
s_font *HintModule::hintFont = nullptr;

void HintModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    fader = show;
    fader.update(ModularBody::deltaTime);
    if (!fader.getInterstate())
        return;
    if (!body->isPrimary()) {
        if (ModularBody *parent = body->getParent()) {
            const float separation = (body->getObservedPosition() - parent->getObservedPosition()).length();
            // old fovdeg = full fov in degrees; ModularBody::halfFov is the half
            // fov in radians: fovdeg = halfFov * (360/pi)
            const float angDist = 300.f * atanf(separation / body->getDistanceToObserver())
                                / (ModularBody::halfFov * (360.f / M_PI));
            if (angDist <= 0.25f)
                return;
        }
    }
    // Old drawHints: ONE color for label and circle - label RGB with the
    // fader interstate as alpha (hints.cpp:71-74).
    const Vec4f color(labelColor[0], labelColor[1], labelColor[2], fader.getInterstate());
    renderer.drawHint(body->getScreenPos(), color);
    const float shift = Hints::hintCircleRadius * 1.2f
                      + body->getScreenSize() * ModularBody::getViewportRadius();
    renderer.printGravity(hintFont, body->getScreenPos(), body->getNameI18n(),
                          color, shift, shift);
}

void HintModule::setFont(s_font *font)
{
    hintFont = font;
    Hints::hintCircleRadius = font->getFontSize() * 0.6f;
}
