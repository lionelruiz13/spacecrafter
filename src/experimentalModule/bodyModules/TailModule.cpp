#include "TailModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/Renderer.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/sc_const.hpp" // AU
#include <cmath>

int TailModule::activeCount = 0;

// TAIL instanced batch - row 12 (INTENT S11.43). Faithful port of the old
// Tail (tail.cpp) + SmallBody comet-tail machinery (body_smallbody.cpp), with
// the global singleton Tail::global dissolved into the Renderer batching
// service (submitTail/flushTails). The update()/draw() split maps the old
// draw-time work onto the system-phase sweep (ModularSystem::drawTails):
// update() computes the coma/tail size + the PARENT-frame expansion vectors
// (frame-independent, JD-cached); draw() rotates them into eye space with the
// parent position frame the sweep hands in (the old nav->getHelioToEyeMat()).

TailModule::TailModule(std::vector<SubTail> &&subTails,
                       float absoluteMagnitude, float slopeParameter)
    : BodyModule(BodyModuleType::TAIL), subTails(std::move(subTails)),
      absoluteMagnitude(absoluteMagnitude), slopeParameter(slopeParameter)
{
    ++activeCount; // a comet always shows its tail (no on/off flag) - the phase
                   // runs while any TAIL module is loaded (anyActive gate).
}

TailModule::~TailModule()
{
    --activeCount;
}

Vec2f TailModule::comaDiameterAndTailLengthAU(float r)
{
    // Faithful port of SmallBody::getComaDiameterAndTailLengthAU
    // (body_smallbody.hpp:64-78, projectpluto comet_tail_formula), incl. the
    // lastR skip cache and the draw-site "/ AU" folded in (tail.cpp:150).
    if (std::abs(lastR / r - 1.f) > 0.0001f) { // avoid recomputing if ~same
        const float mhelio = absoluteMagnitude + slopeParameter * log10f(r);
        float tmp = powf(10.f, -r);
        const float Do = powf(10.f, ((-0.0033f*mhelio - 0.07f) * mhelio + 3.25f)) * (1.f - tmp);
        tmp *= tmp;
        const float common = (1.f - tmp);
        tmp *= tmp;
        const float Lo = powf(10.f, ((-0.0075f*mhelio - 0.19f) * mhelio + 2.1f)) * (1.f - tmp) * 1000.f;
        cachedComaTailAU.set(Do * common / static_cast<float>(AU),
                             Lo * common / static_cast<float>(AU));
        lastR = r;
    }
    return cachedComaTailAU;
}

Vec3f TailModule::orbitPositionAtDate(ModularBody *body, double jd)
{
    // The comet's own orbit position at jd, root-aligned VSOP87 (parent-
    // relative) - the same frame the parent position frame (drawTails) maps to
    // eye, so the expansion vectors transform consistently. Shipped comets are
    // sun-parented, so this equals the old heliocentric Body::getPositionAtDate.
    double v[3] = {0, 0, 0};
    if (const Orbit *o = body->getOrbit())
        o->positionAtTimevInVSOP87Coordinates(jd, v);
    return Vec3f(static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
}

bool TailModule::update(ModularBody *body, float scaledRadius)
{
    boundingRadius = scaledRadius; // the tail never inflates the body bound (it
                                   // is a screen overlay, not in a regime list)
    // Heliocentric distance r (AU): |comet - sun| in observer space (the old
    // distToSun = |eye_planet - eye_sun|, tail.cpp:148-149). Both statics/public.
    const float r = (body->getObservedPosition() - ModularBody::getLightPosition()).length();
    const Vec2f comaTail = comaDiameterAndTailLengthAU(r);
    // Old gate (tail.cpp:151): coma wider than the tail is long -> no tail.
    drawThisFrame = comaTail[0] <= comaTail[1];
    if (!drawThisFrame)
        return false;
    const double jd = body->getLastJD();
    for (auto &sub : subTails) {
        if (jd == sub.lastJD)
            continue; // JD-cache (old tail.cpp:156): recompute only on time change
        const Vec3f cur = orbitPositionAtDate(body, jd);
        const Vec3f half = orbitPositionAtDate(body, jd - sub.deltaTraceJD / 2);
        Vec3f past = orbitPositionAtDate(body, jd - sub.deltaTraceJD);
        const Vec3f velocityCorrection = half * 2 - cur - past;
        const Vec3f velocity = (past - cur) + velocityCorrection;
        Vec3f curDir = cur; curDir.normalize();
        past.normalize(); // pastPos radial (old normalizes currentPos + pastPos)
        const float factors = sub.ejectionForce * comaTail[1]; // tailLength (AU)
        sub.cachedExpansionInitial =
            velocity + curDir * (sub.ejectionLinearity * factors);
        sub.cachedExpansionCorrection =
            velocityCorrection * -2.f + (past - curDir * sub.ejectionLinearity) * factors;
        sub.cachedCoefRadius = sub.coefRadius * comaTail[0]; // comaDiameter (AU)
        sub.lastJD = jd;
    }
    return false; // needs the per-frame tick (like TRAIL: never self-deregister)
}

void TailModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // mat = the PARENT position frame (drawTails) = root-aligned VSOP87 -> eye,
    // the new-path nav->getHelioToEyeMat(). Faithful port of Tail::draw's eye
    // transform + instance push (tail.cpp:172-178).
    if (!drawThisFrame)
        return;
    const Vec3f offset = body->getObservedPosition(); // eye_planet
    for (const auto &sub : subTails) {
        const Vec3f initialDirection = mat.multiplyWithoutTranslation(sub.cachedExpansionInitial);
        const Vec3f directionCorrection = mat.multiplyWithoutTranslation(sub.cachedExpansionCorrection);
        Vec3f tmp = initialDirection + directionCorrection / float(Renderer::TAIL_TIME_SEGMENTS);
        tmp /= sqrtf(tmp[0]*tmp[0] + tmp[1]*tmp[1]);
        const Mat4f m = Mat4f::zrotation(tmp[1], tmp[0]) * Mat4f::xrotation(M_PI_2 - atanf(-tmp[2]));
        Renderer::TailInstance inst;
        inst.offset = offset;
        inst.expandDirection = initialDirection;
        inst.expandCorrection = directionCorrection;
        inst.coefRadius = sub.cachedCoefRadius;
        inst.color = sub.color;
        inst.modelViewMatrix[0] = Vec3f(m.r[0], m.r[1], m.r[2]);
        inst.modelViewMatrix[1] = Vec3f(m.r[4], m.r[5], m.r[6]);
        inst.modelViewMatrix[2] = Vec3f(m.r[8], m.r[9], m.r[10]);
        renderer.submitTail(inst);
    }
}
