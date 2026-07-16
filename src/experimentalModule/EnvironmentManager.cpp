#include "EnvironmentManager.hpp"
#include "ModularBody.hpp"
#include "ModularSystem.hpp"
#include "Camera.hpp"
#include "environmentModules/LandscapeEnv.hpp"
#include "coreModule/milkyway.hpp"
#include "tools/sc_const.hpp"
#include <algorithm>

EnvironmentManager *EnvironmentManager::instance = nullptr;

EnvironmentManager::EnvironmentManager(MilkyWay *milky, Atmosphere *atmosphere) :
    milky(milky), atmosphere(atmosphere)
{
    instance = this;
}

void EnvironmentManager::setLandscape(Landscape *landscape)
{
    LandscapeEnv::engine = landscape;
}

void EnvironmentManager::update(Camera &camera, double jd, float deltaTime, bool driveEngines)
{
    julianDay = jd;
    ModularBody *reference = camera.getReferenceBody();
    system = camera.getCurrentSystem();
    // 1. Active chain + member edges on the diff (single edge authority).
    newChain.clear();
    for (ModularBody *b = reference; b; b = b->getParent())
        newChain.push_back(b);
    for (ModularBody *b : activeChain) {
        if (std::find(newChain.begin(), newChain.end(), b) == newChain.end()) {
            for (auto &m : b->groundedEnvironment)
                m->leave(b);
            for (auto &m : b->environment)
                m->leave(b);
        }
    }
    for (ModularBody *b : newChain) {
        if (std::find(activeChain.begin(), activeChain.end(), b) == activeChain.end()) {
            for (auto &m : b->groundedEnvironment)
                m->enter(b);
            for (auto &m : b->environment)
                m->enter(b);
        }
    }
    activeChain = newChain;

    // 2. State production. onBody = the old observatory->isOnBody(): the
    // camera is anchored on a landable body (system references and free
    // flight are the old anchor-point case -> anchorAssign branch =
    // EnvironmentState defaults + iris texture).
    onBody = !camera.isFreeMode() && !reference->isSystem();
    state = EnvironmentState{};
    state.atmosphereUserFlag = atmosphereUserFlag;
    if (onBody) {
        // Camera distance from the body CENTER in AU (distanceToReference
        // is relative to the altitude reference - re-add it; members
        // subtract it back per their own body's convention).
        const Vec3f refPos(0, 0, camera.distanceToReference() + reference->getAltitudeReference());
        for (auto &m : reference->groundedEnvironment)
            m->update(reference, refPos, deltaTime, state);
        for (ModularBody *b : activeChain) {
            const Vec3f pos = (b == reference)
                ? refPos : Vec3f(0, 0, b->getDistanceToObserver());
            for (auto &m : b->environment)
                m->update(b, pos, deltaTime, state);
        }
    } else {
        // Anchor branch: InAoI members of the chain still update (the
        // milkyway backdrop; an atmosphere member's gates all collapse to
        // false above limSup anyway) - grounded members don't.
        for (ModularBody *b : activeChain) {
            const Vec3f pos(0, 0, b->getDistanceToObserver());
            for (auto &m : b->environment)
                m->update(b, pos, deltaTime, state);
        }
    }
    state.drawBody = !state.drawLandscape; // fixed combination rule

    // 3. Sky brightness - port of the executor formula (solarSystemModule
    // update: sun z in the local zenith frame + the eclipse-dimming term
    // from the PREVIOUS frame's atmosphere intensity, same phase
    // relationship as the old flow).
    if (ModularBody *star = system->getSystemStar()) {
        Vec3f sunLocal = camera.observedToLocalPos(star->getObservedPosition());
        sunLocal.normalize();
        float sb = (sunLocal[2] < -0.1f/1.5f) ? 0.01f
                 : 0.01f + 1.5f*(sunLocal[2] + 0.1f/1.5f);
        if (atmosphere->getFadeIntensity() == 1)
            sb *= atmosphere->getIntensity() + 0.1f;
        state.skyBrightness = sb;
    }

    // 4. Atmosphere compute input snapshot (new-path sources).
    buildAtmosphereInput(camera, reference);

    // 5. Engine drive - modular phase only; the exact BodyDecor write set.
    if (driveEngines) {
        if (onBody) {
            if (reference != lastReference) // old setLandscapeToBody edge
                atmosphere->setModel(reference->envParams.model);
            if (reference->envParams.hasAtmosphere) {
                atmosphere->setFlagShow(state.atmosphereActive);
                milky->useIrisTexture(!state.atmosphereActive);
            } else { // bodyAssign !hasAtmosphere branch
                atmosphere->setFlagShow(false);
                milky->useIrisTexture(true);
            }
        } else {
            // anchorAssign branch: iris only - the atmosphere fader is
            // deliberately NOT written (old parity).
            milky->useIrisTexture(true);
        }
        lastReference = reference;
    }
}

void EnvironmentManager::buildAtmosphereInput(Camera &camera, ModularBody *reference)
{
    atmInput.jd = julianDay;
    atmInput.prj = nullptr;
    atmInput.eyeToLocal = camera.viewRotation().transpose();
    atmInput.halfFov = ModularBody::halfFov;
    atmInput.latitudeDeg = camera.getLatitude() * (180. / M_PI);
    atmInput.altitudeM = camera.distanceToReference() * (1000 * AU);
    // temperature/humidity: the old hardcode (15 C / 40%) until per-body
    // data exists (INTENT 11.23 generalization note).
    atmInput.temperatureC = 15.f;
    atmInput.relativeHumidity = 40.f;
    ModularBody *star = system->getSystemStar();
    if (!star) {
        atmInput.sunPos.set(0, 0, -1);
        atmInput.moonPos.set(0, 0, -1);
        atmInput.moonRadiusKm = 0;
        atmInput.moonPhase = 0;
        return;
    }
    const Vec3f sunEye = star->getObservedPosition();
    {
        const Vec3f v = atmInput.eyeToLocal.multiplyWithoutTranslation(sunEye);
        atmInput.sunPos.set(v[0], v[1], v[2]);
    }
    // "Moon" generalized to the reference body's largest satellite (the old
    // getMoon() hardcode dissolves - INTENT 11.23). Earth: the Moon, same
    // by construction; bodies without satellites get a below-horizon null
    // moon (no skybright contribution, no eclipse term).
    ModularBody *moon = nullptr;
    for (auto &c : reference->childs) {
        if (!moon || c.getRadius() > moon->getRadius())
            moon = &c;
    }
    if (moon) {
        const Vec3f moonEye = moon->getObservedPosition();
        const Vec3f v = atmInput.eyeToLocal.multiplyWithoutTranslation(moonEye);
        atmInput.moonPos.set(v[0], v[1], v[2]);
        atmInput.moonRadiusKm = moon->getRadius() * AU; // physical, unscaled (old parity)
        // Illuminated fraction - Body::get_phase ported to eye-frame
        // vectors (angles are frame-invariant; obs = the reference body
        // CENTER, the old call used Earth's heliocentric position).
        const Vec3f refEye = reference->getObservedPosition();
        const double sq = (double)(refEye - sunEye).lengthSquared();
        const double Rq = (double)(moonEye - sunEye).lengthSquared();
        const double pq = (double)(refEye - moonEye).lengthSquared();
        const double cos_chi = (pq + Rq - sq) / (2.0 * sqrt(pq * Rq));
        atmInput.moonPhase = (1.0 - acos(cos_chi)/M_PI) * cos_chi
                           + sqrt(1.0 - cos_chi*cos_chi) / M_PI;
    } else {
        atmInput.moonPos.set(0, 0, -camera.distanceToReference());
        atmInput.moonRadiusKm = 0;
        atmInput.moonPhase = 0;
    }
}

void EnvironmentManager::drawBackdrop(Renderer &renderer)
{
    // Root-most first: the milkyway backdrop draws behind everything the
    // frame adds after this call position (old executor order).
    bool aboveSystem = true;
    for (auto it = activeChain.rbegin(); it != activeChain.rend(); ++it) {
        ModularBody *b = *it;
        // Mats above the current system are not maintained by
        // dispatchUpdate - substitute the system's (flat-chain contract:
        // same orientation, EnvironmentModule.hpp drawBackdrop note).
        const Mat4f &mat = aboveSystem ? system->getMat() : b->getMat();
        for (auto &m : b->environment)
            m->drawBackdrop(renderer, b, mat);
        if (b == system)
            aboveSystem = false;
    }
}

void EnvironmentManager::drawSky(Renderer &renderer)
{
    // InAoI members first (atmosphere - additive over the multisample
    // content), then the reference's grounded members (landscape+fog,
    // PASS_FOREGROUND) - the old executor order, with its gate.
    bool aboveSystem = false; // walking ref->root this time
    for (ModularBody *b : activeChain) {
        const Mat4f &mat = aboveSystem ? system->getMat() : b->getMat();
        for (auto &m : b->environment)
            m->drawSky(renderer, b, mat);
        if (b == system)
            aboveSystem = true;
    }
    if (onBody && state.drawLandscape) {
        ModularBody *reference = activeChain.front();
        for (auto &m : reference->groundedEnvironment)
            m->drawSky(renderer, reference, reference->getMat());
    }
}
