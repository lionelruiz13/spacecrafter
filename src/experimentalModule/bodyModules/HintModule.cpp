#include "HintModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include <cmath>

bool HintModule::show = false;
Vec3f HintModule::defaultLabelColor{};
s_font *HintModule::hintFont = nullptr;

void HintModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!show)
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
    renderer.drawHint(body->getScreenPos(), Vec4f(labelColor[0], labelColor[1], labelColor[2], 1.f));
    // Label: pending the Renderer text service (INTENT open #3) - see header.
}
