#include "StarModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include <cmath>

// Effective sun big-halo size. Default = the old setFlagSunScale(false) base
// (solarsystem.hpp:100); driven at init + on toggle by SSystemFactory::
// setFlagSunScale. The ssystem.ini big_halo_size (=1) is DEAD for the main Sun
// (old overwrites it) - measured: old draws rmag=99 (bhs=200), the data's bhs=1
// gave rmag=32 (INTENT S11.44).
float StarModule::sunHaloSize = 200.f;

// FAITHFUL PORT of old Sun::drawBigHalo (body_sun.cpp:171-192). The values are
// byte-identical to the old form; every line carries its old-path reference.
// Screen-geometry mapping (established S11.19a, measured on the point halo):
//   old screen_sz == Body::getOnScreenSize == screenSize * 2 * viewportRadius
//   old nav->getObserverHelioPos().length() == the Sun's distance to observer
//     (the observer-to-heliocentre distance; the Sun IS the heliocentre)
//   old myColor->getHalo() == ModularBody::getHaloColor() (== createInfo color,
//     the same value the ported point halo uses, S11.19a byte-parity)
//   old screenPos (render px) == rectToRender(getScreenPos()) (done in the
//     Renderer service, like every other screen-space service).
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
    // System-collapse cross-fade (B22, INTENT 11.64/11.81): the star's big-halo
    // glow is the DOMINANT visual of a resolved nested system, so it MUST take
    // the fade or the collapse POPS at the threshold (measured live: ~74% of the
    // dot->resolved swing appeared unfaded - 11.64's own "the interior fades IN"
    // contract, falsified at 11.80's surface).
    // NB the fade rides `color`, NOT `cmag`: sun_big_halo.frag is
    //   FragColor = color * (farHalo*max(1,cmag+0.1) + nearHalo)
    // so cmag is FLOORED (max(1,..)) for the texture glow and IGNORED by the
    // procedural nearHalo disc - scaling cmag barely dims it (measured ~2000 of
    // ~28000). `color` multiplies the whole FragColor, so color*=drawAlpha fades
    // BOTH the glow and the disc linearly. drawAlpha is 1.0 in every frame outside
    // ModularSystem::drawNested's band (default 1.f; set to savedAlpha*t for the
    // interior, restored after) => x1.0f EXACT IEEE identity everywhere the
    // collapse is not mid-fade (the same inert guarantee as drawHaloCore).
    renderer.drawSunHalo(body->getScreenPos(), body->getHaloColor() * ModularBody::drawAlpha, rmag, cmag, screenR);
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
