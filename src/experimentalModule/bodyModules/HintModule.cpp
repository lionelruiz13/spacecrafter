#include "HintModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include <cmath>

bool HintModule::show = false;
Vec3f HintModule::defaultLabelColor{};
s_font *HintModule::hintFont = nullptr;

void HintModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Per-body fader toward the global flag; tick-at-draw (see header:
    // far-routed modules have no update() channel). Assignment is idempotent
    // while the target is unchanged (LinearFader::operator= early-outs).
    fader = show;
    fader.update(ModularBody::deltaTime);
    if (!fader.getInterstate())
        return;
    // Angular-separation gate (old path: ang_dist = 300·atan(|ecl|/dist)/fov°,
    // drawn only when > 0.25 - body.cpp:961 + drawHints) - suppresses the hint
    // spam of satellites huddled around their planet at wide fov. Star-typed
    // bodies skip it like the old BodySun::drawHints (its |ecl| ~ 0 would
    // otherwise suppress the Sun's own hint forever).
    if (!body->isStar()) {
        if (ModularBody *parent = body->getParent()) {
            const float separation = (body->getObservedPosition() - parent->getObservedPosition()).length();
            // old fov° = full fov in degrees; ModularBody::halfFov is the half
            // fov in radians: fov° = halfFov · (360/π)
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
    // Label through the text service (C7): same anchor as the circle; shift =
    // 10 + onScreenSizePx/2 on both axes (old drawHints tmp), px full
    // diameter = screenSize * 2 * viewportRadius (drawHalo screen_r form).
    const float shift = 10.f + body->getScreenSize() * ModularBody::viewportRadius;
    renderer.printGravity(hintFont, body->getScreenPos(), body->getNameI18n(),
                          color, shift, shift);
}
