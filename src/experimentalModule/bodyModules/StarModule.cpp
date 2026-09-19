#include "StarModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include <cmath>

float StarModule::sunHaloSize = 200.f;

void StarModule::drawBigHalo(Renderer &renderer, ModularBody *body)
{
    if (sunHaloSize <= 0.f)
        return; // old setHaloSize clamps negatives to 0 -> no halo
    // old screen_sz (body.cpp:1072 = getOnScreenSize) AND old uRadius are the
    // SAME value (getOnScreenSize) - one variable here.
    const float screenR = body->getScreenSize() * 2.f * ModularBody::getViewportRadius();
    float rmag = sunHaloSize / 2.f / sqrtf(body->getDistanceToObserver());
    // old: cmag = rmag/screen_sz, clamped to 1 (screen_sz==0 -> inf -> 1).
    float cmag = (screenR > 0.f) ? rmag / screenR : 1.f;
    if (cmag > 1.f)
        cmag = 1.f;
    if (rmag < screenR * 2.f) {
        cmag *= rmag / (screenR * 2.f);
        rmag = screenR * 2.f;
    }
    if (rmag < 32.f)
        rmag = 32.f;
    renderer.drawSunHalo(body->getScreenPos(), body->getHaloColor() * ModularBody::drawAlpha, rmag, cmag, screenR);
}

void StarModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &)
{
    drawBigHalo(renderer, body);
}

// A far component never receives drawNoDepth; the header names the halo here,
// so delegate defensively (identical output were the module ever near-routed).
void StarModule::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &)
{
    drawBigHalo(renderer, body);
}
