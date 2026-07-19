#include "StarModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include <cmath>

// Effective sun big-halo size. Default = the old setFlagSunScale(false) base
// (solarsystem.hpp:100); driven at init + on toggle by SSystemFactory::
// setFlagSunScale. The ssystem.ini big_halo_size (=1) is DEAD for the main Sun
// (old overwrites it) - measured: old draws rmag=99 (bhs=200), the data's bhs=1
// gave rmag=32 (INTENT §11.44).
float StarModule::sunHaloSize = 200.f;

// FAITHFUL PORT of old Sun::drawBigHalo (body_sun.cpp:171-192). The values are
// byte-identical to the old form; every line carries its old-path reference.
// Screen-geometry mapping (established §11.19a, measured on the point halo):
//   old screen_sz == Body::getOnScreenSize == screenSize * 2 * viewportRadius
//   old nav->getObserverHelioPos().length() == the Sun's distance to observer
//     (the observer-to-heliocentre distance; the Sun IS the heliocentre)
//   old myColor->getHalo() == ModularBody::getHaloColor() (== createInfo color,
//     the same value the ported point halo uses, §11.19a byte-parity)
//   old screenPos (render px) == rectToRender(getScreenPos()) (done in the
//     Renderer service, like every other screen-space service).
void StarModule::drawBigHalo(Renderer &renderer, ModularBody *body)
{
    if (sunHaloSize <= 0.f)
        return; // old setHaloSize clamps negatives to 0 -> no halo
    // old screen_sz (body.cpp:1072 = getOnScreenSize) AND old uRadius are the
    // SAME value (getOnScreenSize) - one variable here.
    const float screenR = body->getScreenSize() * 2.f * ModularBody::viewportRadius;
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
    renderer.drawSunHalo(body->getScreenPos(), body->getHaloColor(), rmag, cmag, screenR);
}

// FAR component: the sole live hook (farComponents are drawn via draw(), in
// every regime the Sun spans - BEFORE clearDepth + the near MESH disc, so the
// additive glow sits behind the opaque disc, old drawGL order:
// drawBigHalo -> drawBody).
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
