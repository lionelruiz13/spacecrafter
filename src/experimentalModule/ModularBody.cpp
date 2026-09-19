#include "ModularBody.hpp"
#include "JsonNum.hpp"
#include <cstring>
#include <iomanip>
#include <limits>
#include <ostream>
#include "ModularBodyPtr.hpp"
#include "ModularSystem.hpp"
#include "EnvironmentManager.hpp"
#include "Camera.hpp" // closeRangeComponents: WHICH body the environment state is about
#include "RenderChain.hpp"
#include "bodyModules/TrailModule.hpp" // startTrail: the fresh-restart seam's callee
#include "tools/log.hpp"
#include "tools/translator.hpp"
#include "tools/utility.hpp"
#include "tools/context.hpp" // quiesceFrames (mid-session release precondition)
#include "EntityCore/Core/VulkanMgr.hpp"

Vec3f ModularBody::lightPosition;
float ModularBody::lightDistance;
float ModularBody::lightSize;
ModularBody *ModularBody::lastFit = nullptr;
std::map<std::string, ModularBody *> ModularBody::bodyReference;
float ModularBody::halfFov = M_PI_2;
int ModularBody::projectionMode = ProjectionTransfer::FISHEYE;
float ModularBody::cullHalfFov = M_PI_2;
bool ModularBody::flagLightTravelTime = false; // set from config via SSystemFactory
Vec3f ModularBody::defaultHaloColor{};
float ModularBody::haloScale = 1;
float ModularBody::haloSizeLimit = 9;
float ModularBody::drawAlpha = 1.f;
float ModularBody::viewportRadius = 1;
float ModularBody::earlyVisibilityScreenSize = BODY_EARLY_VISIBILITY_BOUNDING_SIZE / 2.f;
float ModularBody::fullVisibilityScreenSize = BODY_FULL_VISIBILITY_BOUNDING_SIZE / 2.f;
float ModularBody::bigTextureScreenSize = BODY_BIG_TEXTURE_BOUNDING_SIZE / 2.f;
std::vector<ModularBody *> ModularBody::notableBody;
float ModularBody::deltaTime = 0;
double ModularBody::currentJD = 0; // written once per frame by dispatchUpdate (B39)
std::shared_ptr<BodyTesselation> ModularBody::bodyTesselation; // both-paths seam
ModularBody *ModularBody::selectedBody = nullptr;
Translator *ModularBody::translator = nullptr;
StringIDCluster ModularBody::slotID;
Tracer ModularBody::tracer{80, 24};

void ModularBody::unpin()
{
    if (--pins == 0 && parked)
        RenderChain::instance.onPinDrained(this);
}

ShadowCaster BodyModule::getShadowCaster(ModularBody *body, const Vec3f &) const
{
    return {body->getRadius(), body->getShadowAbsorbtion(), Vec4f(0, 0, 0, -1)};
}

// Default module state dump: nothing externally observable (INTENT 11.56).
void BodyModule::dumpState(std::ostream &out) const
{
    out << "null";
}

ModularBody::ModularBody(ModularBody *parent, ModularBodyCreateInfo &info) :
    englishName(std::move(info.englishName)), parent(parent), orbit(std::move(info.orbit)), re(info.re), haloColor(info.haloColor), albedo(info.albedo), shadowAbsorbtion(info.shadowAbsorbtion), scaling(1), radius(info.radius), datumRadius(info.datumRadius), groundRadius(info.groundRadius), one_minus_oblateness(1-info.oblateness), solLocalDay(info.solLocalDay), bodyType(info.bodyType), siderealTimeModel(info.siderealTimeModel), surfaceModel(info.surfaceModel), trailLength(info.trailLength), composedDeclaration(info.composedDeclaration), primary(info.primary), isHaloEnabled(info.isHaloEnabled)
{
    if (datumRadius < 0.f) datumRadius = radius;
    if (groundRadius < 0.f) groundRadius = radius;
    if (translator)
        nameI18 = translator->translateUTF8(englishName);
    captureAuthoredState();
    auto &ref = bodyReference[englishName];
    if (ref && !englishName.empty()) {
        // It is too late to abort the body creation, so we replace it.
        // Children transfer by list move (unique_ptr - no slicing: nested
        // ModularSystem children survive the transfer intact).
        cLog::get()->write("Body with name '" + englishName + "' already exists, replacing it", LOG_TYPE::L_INFO);
        groundedBodies = std::move(ref->groundedBodies);
        orbitingBodies = std::move(ref->orbitingBodies);
        innerBodies = std::move(ref->innerBodies);
        hiddenBodies = std::move(ref->hiddenBodies);
        forEachVisibleChild([this](ModularBody &child) { child.parent = this; });
        for (auto &child : hiddenBodies)
            child->parent = this;
        if (ref->pointerCount)
            ModularBodyPtr::redirect(ref, this);
        ref->remove(true); // runs BEFORE the map reassignment below - the old
        bodyReference[englishName] = this; // dtor erases the slot, recreate it
    } else {
        ref = this;
    }
    if (parent) {
        lastJD = parent->lastJD;
        Vec3d tmp;
        if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
            (*oscFunc)(lastJD,lastJD,tmp);
        } else {
            orbit->positionAtTimevInVSOP87Coordinates(lastJD,lastJD,tmp);
        }
        eclipticPos = tmp;
    }
}

ModularBody *ModularBody::createChild(ModularBodyCreateInfo &info, BodyRelation rel)
{
    auto owned = std::make_unique<ModularBody>(this, info);
    ModularBody *ret = owned.get();
    ret->relation = rel;
    ret->boundToSurface = (rel == BodyRelation::GROUNDED); // the cache's single write site
    listOf(rel).push_back(std::move(owned));
    registerToSystem(ret);
    ret->propagateRenderHidden(renderHidden);
    return ret;
}

ModularSystem *ModularBody::createChildSystem(ModularBodyCreateInfo &info, BodyRelation rel)
{
    auto owned = std::make_unique<ModularSystem>(this, info);
    ModularSystem *ret = owned.get();
    ret->relation = rel;
    ret->boundToSurface = (rel == BodyRelation::GROUNDED);
    listOf(rel).push_back(std::move(owned));
    registerToSystem(ret);
    ret->propagateRenderHidden(renderHidden);
    return ret;
}

void ModularBody::registerToSystem(ModularBody *child)
{
    auto p = this;
    while (p->isNotIsolated)
        p = p->parent;
    static_cast<ModularSystem *>(p)->addBody(child);
}

ModularSystem *ModularBody::owningSystem() const
{
    if (!parent)
        return nullptr; // parentless root: never registered anywhere
    auto p = parent;
    while (p->isNotIsolated)
        p = p->parent;
    return static_cast<ModularSystem *>(p);
}

void ModularBody::propagateRenderHidden(bool ancestorHidden)
{
    const bool nowHidden = ancestorHidden || (relation < BodyRelation::GROUNDED);
    if (nowHidden != renderHidden) {
        renderHidden = nowHidden;
        if (ModularSystem *system = owningSystem()) {
            if (nowHidden)
                system->unregisterBody(this);
            else
                system->addBody(this);
        }
    }
    forEachVisibleChild([nowHidden](ModularBody &child) {
        child.propagateRenderHidden(nowHidden);
    });
    for (auto &child : hiddenBodies)
        child->propagateRenderHidden(nowHidden);
}

std::unique_ptr<Orbit> ModularBody::setOrbit(std::unique_ptr<Orbit> newOrbit)
{
    if (parent && orbit) {
        if (auto *binary = dynamic_cast<BinaryOrbit *>(parent->orbit.get()))
            binary->clearSecondaryOrbit(orbit.get());
    }
    std::unique_ptr<Orbit> previous = std::move(orbit);
    orbit = std::move(newOrbit);
    evaluatedJD = -std::numeric_limits<double>::infinity();
    return previous;
}

Vec3d ModularBody::getPositionAtDate(double jd) const
{
    Vec3d pos{};
    Vec3d tmp;
    for (const ModularBody *b = this; b->parent; b = b->parent) {
        if (!b->orbit)
            continue;
        b->orbit->positionAtTimevInVSOP87Coordinates(jd, tmp);
        pos += tmp;
    }
    return pos;
}

ModularBody::~ModularBody()
{
    if (parent && orbit) {
        if (auto *binary = dynamic_cast<BinaryOrbit *>(parent->orbit.get()))
            binary->clearSecondaryOrbit(orbit.get());
    }
    if (parent) {
        auto p = parent;
        while (p->isNotIsolated)
            p = p->parent; // Search for system center
        static_cast<ModularSystem *>(p)->removeBody(this);
    }
    clearChildren(); // children (incl. hidden - parent-owned) destroyed BEFORE
                     // our own redirect: their remnant pointers redirect to us,
                     // then ours (theirs included) redirect to our parent
    components.clear();
    groundedEnvironment.clear();
    environment.clear();
    bodyReference.erase(englishName);
    if (lastFit == this)
        lastFit = nullptr;
    EnvironmentManager::notifyBodyDestroyed(this);
    if (pointerCount)
        ModularBodyPtr::redirect(this, parent);
}

void ModularBody::startTrail(bool record)
{
    for (auto *m : trailComponents)
        static_cast<TrailModule *>(m)->startTrail(record);
}

bool ModularBody::remove(bool recursive)
{
    if (Context::instance)
        Context::instance->quiesceFrames();
    if (recursive || !ownsAnyChild()) {
        auto &src = parent->listOf(relation);
        const auto end = src.end();
        for (auto it = src.begin(); it != end; ++it) {
            if (it->get() == this) {
                src.erase(it); // destroys this
                return true;
            }
        }
        return false; // registration invariant broken - do not pretend removal
    } else {
        return false;
    }
}

void ModularBody::recursiveUpdate(double jd, const Mat4f &matLocalToBodyPos)
{
    // Cache the position frame (the reference-body path reaches here directly
    // from dispatchUpdate, not through transformParentToBodyPos - ORBIT row 8).
    this->matLocalToBodyPos = matLocalToBodyPos;
    mat = matLocalToBodyPos.multiplyFast(accumulatedBodyPosToBody(jd));
    update(jd, mat);
    // Grounded children live in the surface frame (`mat` - the accumulated
    // equatorial frame); orbiting/inner ride the flat chain (frame contract).
    for (auto &c : groundedBodies)
        c->selectiveUpdate(jd, mat);
    for (auto &c : orbitingBodies)
        c->selectiveUpdate(jd, matLocalToBodyPos);
    for (auto &c : innerBodies)
        c->selectiveUpdate(jd, matLocalToBodyPos);
    // Parked children do NOT tick (B39, D23): publish the frame their
    // use-site barrier will need - see publishParkedFrame.
    publishParkedFrame(jd, matLocalToBodyPos);
}

ModularSystem *ModularBody::dispatchUpdate(ModularBody *body, double jd, Mat4f mat_local_to_body)
{
    notableBody.clear();
    currentJD = jd;
    body->preUpdate(jd, mat_local_to_body);
    Mat4f flat = mat_local_to_body.multiplyFast(body->accumulatedBodyToBodyPos(jd));
    if (body->isVisible) {
        body->recursiveUpdate(jd, flat);
    } else {
        body->mat = mat_local_to_body;
        body->matLocalToBodyPos = flat;
        for (auto &c : body->groundedBodies)
            c->recursiveTranslationUpdate(jd, mat_local_to_body);
        for (auto &c : body->orbitingBodies)
            c->recursiveTranslationUpdate(jd, flat);
        for (auto &c : body->innerBodies)
            c->recursiveTranslationUpdate(jd, flat);
        body->publishParkedFrame(jd, flat);
    }
    while (body->isNotIsolated) {
        body->transformBodyToParent(jd, flat);
        ModularBody *parent = body->parent;
        const Mat4f parentTilted = flat.multiplyFast(parent->accumulatedBodyPosToBody(jd));
        for (auto &b : parent->groundedBodies) {
            if (b.get() != body)
                b->selectiveUpdate(jd, parentTilted);
        }
        for (auto &b : parent->orbitingBodies) {
            if (b.get() != body)
                b->selectiveUpdate(jd, flat);
        }
        for (auto &b : parent->innerBodies) {
            if (b.get() != body)
                b->selectiveUpdate(jd, flat);
        }
        parent->publishParkedFrame(jd, flat);
        body = parent;
        body->mat = parentTilted; // assign BEFORE update: update() reads the member
        body->matLocalToBodyPos = flat;
        body->preUpdate(jd, flat);
        body->update(jd, parentTilted);
    }
    return static_cast<ModularSystem *>(body);
}

// The D8 use-site barrier (S11.76(b)) - see the header for why +4 and why the
// frame comes from the parent rather than from this body.
bool ModularBody::useNow()
{
    if (!renderHidden || !parent)
        return true; // the walks still evaluate this body: fresh by construction
    const bool dateFresh = (evaluatedJD == currentJD);
    const bool parentServed = parent->useNow();
    if (!(parent->renderHidden ? parentServed : parent->parkedFramePublished)) {
        if (!unservedLogged) {
            unservedLogged = true;
            cLog::get()->write("Position of '" + englishName + "' was used, but "
                "its parent '" + parent->englishName + "' has not published a "
                "position frame for its parked children: the loaded system's "
                "update walk has not visited '" + parent->englishName
                + "' since '" + englishName + "' was parked under it, so there "
                "is no frame to compute '" + englishName + "' in. This use is "
                "REFUSED: '" + englishName + "' keeps its unevaluated position "
                "(the zero vector), so its distance, RA/DE, alt/az and magnitude "
                "stay the degenerate readout instead of becoming a "
                "plausible-looking wrong one. Two ways to be here: '"
                + parent->englishName + "' is OUTSIDE that walk and always will "
                "be (the walk stops at the system node, so anything under the "
                "universe root is) - then declare '" + englishName + "' under a "
                "body the loaded system walks; or '" + englishName + "' was "
                "created or reloaded just now - then the next frame's walk "
                "publishes and the use after it is served.",
                LOG_TYPE::L_WARNING);
        }
        return false;
    }
    const Mat4f &parentFlat = parent->renderHidden ? parent->matLocalToBodyPos
                                                   : parent->parkedChildFrame;
    const Mat4f frame = boundToSurface
        ? parentFlat.multiplyFast(parent->accumulatedBodyPosToBody(currentJD))
        : parentFlat;
    if (dateFresh && sameFrame(frame, evaluatedFrame))
        return true; // this date AND this camera frame: already computed, by a use
    evaluatedFrame = frame;
    const int extra = dateFresh ? 0 : RESUME_EXTRA_ITERATIONS;
    for (int i = 0; i <= extra; ++i)
        recursiveTranslationUpdate(currentJD, frame);
    return true;
}

void ModularBody::resumeModulesAfterHidden()
{
    for (auto &module : components) {
        if (module)
            module->resumeAfterHidden(this);
    }
    forEachVisibleChild([](ModularBody &child) {
        child.resumeModulesAfterHidden();
    });
}

void ModularBody::select()
{
    isSelected = true;
    selectedBody = this;
}

void ModularBody::deselect()
{
    isSelected = false;
    if (selectedBody == this)
        selectedBody = nullptr;
}

void ModularBody::updateCache()
{
    bool cached = !scaling.isTransiting();
    const float display = getDisplayScaling();
    scaledRadius = radius * display;
    scaledDatumRadius = datumRadius * display;
    scaledGroundRadius = groundRadius * display;
    boundingRadius = scaledRadius;
    for (auto &module : nearComponents) {
        cached &= module->update(this, scaledRadius);
        if (boundingRadius < module->getBoundingRadius())
            boundingRadius = module->getBoundingRadius();
    }
    for (auto &module : inComponents) {
        cached &= module->update(this, scaledRadius);
    }
    for (auto &c : groundedBodies)
        c->setInheritedScaling(display);
    for (auto &c : hiddenBodies)
        if (c->boundToSurface)
            c->setInheritedScaling(display);
    if (parent)
        parent->invalidateCachedState();
    updateReach();
    if (cached)
        uncached = false;
}

void ModularBody::updateReach()
{
    float subsystem = boundingRadius;
    forEachVisibleChild([&subsystem](ModularBody &c) {
        const float tmp = c.eclipticPos.length() + c.subsystemRadius;
        if (subsystem < tmp)
            subsystem = tmp;
    });
    // Take some extra margin for the system radius
    subsystemRadius = subsystem * 1.1f;
    float aoi = std::max(boundingRadius * 128 / getDisplayScaling(), subsystemRadius * 16);
    const float sibCap = eclipticPos.length() * 0.6f;
    if (sibCap > 0)
        aoi = std::min(aoi, sibCap);
    areaOfInfluence = aoi;
}

const std::vector<BodyModule *> *ModularBody::closeRangeComponents()
{
    if (distance < scaledRadius)
        return &inComponents;
    if (EnvironmentManager::instance && Camera::instance
            && !EnvironmentManager::instance->getState().drawBody
            && Camera::instance->getReferenceBody() == this)
        return nullptr;
    if (distance < scaledRadius * BODY_SURFACE_HEIGHT) {
        bool surfaceReady = !groundedComponents.empty();
        for (auto *module : groundedComponents)
            surfaceReady &= module->isLoaded();
        if (surfaceReady)
            return &groundedComponents;
    }
    return &nearComponents;
}

void ModularBody::drawLoaded(Renderer &renderer)
{
    loaded = true;
    const auto matrix = mat.multiplyFast(computeBodyToSurface()); // TODO Fix ojml ?
    if (screenSize > fullVisibilityGate()) {
        if (screenSize < 0.2) {
            // far BEFORE clearDepth - hint behind the disc (see draw())
            for (auto &module : farComponents)
                module->draw(renderer, this, mat);
            renderer.clearDepth(distance, boundingRadius);
            for (auto &module : nearComponents) {
                if (module->isLoaded()) {
                    module->draw(renderer, this, matrix);
                } else
                    loaded = false;
            }
        } else {
            renderer.clearDepth(distance, boundingRadius);
            for (auto &module : farComponents)
                module->draw(renderer, this, mat);
            const auto * const components = closeRangeComponents();
            if (components) {
                for (auto *module : *components) {
                    if (module->isLoaded()) {
                        module->draw(renderer, this, matrix);
                    } else
                        loaded = false;
                }
            }
            if (components != &nearComponents) {
                for (auto *module : nearComponents)
                    loaded &= module->isLoaded();
            }
        }
    } else {
        for (auto &module : farComponents)
            module->draw(renderer, this, mat);
        // Same depth-less mapping as draw()'s band (INTENT S5.52) - this is the
        // load-checking copy of the same ladder, so it takes the same call.
        renderer.enterDepthlessSlice(distance, boundingRadius);
        for (auto &module : nearComponents) {
            if (module->isLoaded()) {
                module->drawNoDepth(renderer, this, matrix);
            } else
                loaded = false;
        }
        drawHalo(renderer);
    }
    for (auto &module : inComponents)
        loaded &= module->isLoaded();
}

void ModularBody::setChildNoLongerVisible()
{
    forEachVisibleChild([](ModularBody &c) {
        if (c.isChildVisible)
            c.setChildNoLongerVisible();
        c.distance = 0;
        c.isVisible = false;
    });
    isChildVisible = false;
}

ModularBody *ModularBody::findBodyNameI18n(const std::string &nameI18)
{
    for (auto &b : bodyReference) {
        if (b.second->nameI18 == nameI18)
            return b.second;
    }
    return nullptr;
}

bool ModularBody::hide()
{
    if (!parent || relation < BodyRelation::GROUNDED)
        return false; // roots can't hide; already hidden
    auto &src = parent->listOf(relation);
    const auto end = src.end();
    for (auto it = src.begin(); it != end; ++it) {
        if (it->get() != this)
            continue;
        isVisible = false;
        if (isChildVisible)
            setChildNoLongerVisible();
        parent->hiddenBodies.push_back(std::move(*it));
        src.erase(it);
        relation = static_cast<BodyRelation>(static_cast<int>(relation) - 3); // -> HIDDEN_*
        parent->invalidateCachedState();
        // B39 (S11.117): leave the RENDERED universe - this subtree out of the
        // owning system's sorted list, the effective flag set on every node.
        propagateRenderHidden(parent->renderHidden);
        return true;
    }
    return false;
}

bool ModularBody::show()
{
    if (!parent || relation >= BodyRelation::GROUNDED)
        return false; // not hidden
    auto &hid = parent->hiddenBodies;
    const auto end = hid.end();
    for (auto it = hid.begin(); it != end; ++it) {
        if (it->get() == this) {
            relation = static_cast<BodyRelation>(static_cast<int>(relation) + 3); // -> origin list
            parent->listOf(relation).push_back(std::move(*it));
            hid.erase(it);
            parent->invalidateCachedState();
            useNow();
            propagateRenderHidden(parent->renderHidden);
            resumeModulesAfterHidden();
            return true;
        }
    }
    return false;
}

Mat4f ModularBody::calculateSwitchCompensation(const ModularBody *to) const
{
    const ModularBody *common = findCommonParent(to);
    Mat4f diff = accumulatedBodyToBodyPos(lastJD);
    // this -> common (applied last: common -> this descent, -ecl each)
    for (auto body = this; body != common; body = body->parent)
        body->transformParentToBody(diff);
    // common -> to (applied first: to -> common climb, +ecl each), reversed
    std::vector<const ModularBody *> travel;
    for (auto body = to; body != common; body = body->parent)
        travel.push_back(body);
    while (travel.size()) {
        travel.back()->transformBodyToParent(diff);
        travel.pop_back();
    }
    return diff.multiplyFast(to->accumulatedBodyPosToBody());
}

void ModularBody::setViewportRadius(float halfRenderWidthPx)
{
    viewportRadius = halfRenderWidthPx;
    const float px2ss = 1.f / (2.f * halfRenderWidthPx);
    earlyVisibilityScreenSize = BODY_EARLY_VISIBILITY_BOUNDING_SIZE * px2ss;
    fullVisibilityScreenSize = BODY_FULL_VISIBILITY_BOUNDING_SIZE * px2ss;
    bigTextureScreenSize = BODY_BIG_TEXTURE_BOUNDING_SIZE * px2ss;
}

void ModularBody::setTranslator(Translator &_translator)
{
    translator = &_translator;
    for (auto &ref : bodyReference)
        ref.second->nameI18 = _translator.translateUTF8(ref.second->englishName);
    setViewportRadius(VulkanMgr::instance->getScreenRect().extent.width / 2.f);
}

std::vector<BodyModuleType> ModularBody::deduceBodyModuleList(std::map<std::string, std::string> &param)
{
    std::vector<BodyModuleType> ret;
    if (param.count("tex_ring") && Utility::strToBool(param["rings"], false))
        ret.push_back(BodyModuleType::RING);
    if (param.count("tex_map"))
        ret.push_back(BodyModuleType::MESH);
    if (param.count("model_name")) {
        const std::string &type = param["type"];
        if (type.size() >= 4 && std::memcmp(type.data(), "Arti", 4) == 0)
            ret.push_back(BodyModuleType::OJM);
    }
    if ((param.count("has_atmosphere") || param.count("atmosphere_lim_landscape"))
        && !param["atmosphere_ext_model"].empty())
        ret.push_back(BodyModuleType::ATMOSPHERE);
    if (param.count("tex_map"))
        ret.push_back(BodyModuleType::AXIS);
    if (!englishName.empty() && !Utility::isFalse(param["hint"]))
        ret.push_back(BodyModuleType::HINT);
    if (Utility::strToDouble(param["orbit_visualization_period"], 0.0) > 0.0
        && !Utility::isFalse(param["orbit"]))
        ret.push_back(BodyModuleType::ORBIT);
    {
        const std::string &type = param["type"];
        const bool artificial = type.size() >= 4 && std::memcmp(type.data(), "Arti", 4) == 0;
        if (Utility::strToDouble(param["orbit_visualization_period"], 0.0) > 0.0
            && !isSatellite() && !artificial)
            ret.push_back(BodyModuleType::TRAIL);
    }
    {
        const std::string &type = param["type"];
        const bool comet = type.size() >= 4 && std::memcmp(type.data(), "Come", 4) == 0;
        const bool hasMag = !param["apparent_magnitude"].empty();
        if (!Utility::isFalse(param["tail"])
            && (Utility::isTrue(param["tail"]) || (comet && hasMag)))
            ret.push_back(BodyModuleType::TAIL);
    }
    if (isStar() && !param["tex_big_halo"].empty())
        ret.push_back(BodyModuleType::CUSTOM);
    return ret;
}

void ModularBody::dumpTrace(std::ostream &out) const
{
    out << std::setprecision(9) << "{\"parent\":\""
        << (parent ? parent->englishName : "") << "\",\"ecl\":["
        << jn(eclipticPos[0]) << ',' << jn(eclipticPos[1]) << ',' << jn(eclipticPos[2])
        << "],\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << jn(mat.r[i]) << ((i < 15) ? "," : "");
    out << "],\"dist\":" << jn(distance)
        << ",\"screen\":[" << jn(screenPos.first) << ',' << jn(screenPos.second)
        << "],\"axisRot\":" << jn(computeAxisRotation(lastJD))
        << ",\"attitude\":" << jn(computeAxisRotation(lastJD))
        << ",\"surfaceLocked\":" << (surfaceLockedAttitude ? "true" : "false")
        << ",\"bodyType\":" << static_cast<int>(bodyType)
        << ",\"primary\":" << (primary ? "true" : "false")
        << ",\"surfaceModel\":" << static_cast<int>(surfaceModel)
        << ",\"trailLength\":" << jn(trailLength)
        << ",\"composedDecl\":" << (composedDeclaration ? "true" : "false")
        << ",\"boundingRadius\":" << jn(boundingRadius)
        << ",\"scaledDatumRadius\":" << jn(scaledDatumRadius)
        << ",\"scaledGroundRadius\":" << jn(scaledGroundRadius)
        << ",\"scaling\":" << jn(static_cast<float>(scaling))
        << ",\"scalingTarget\":" << jn(scalingTarget)
        << ",\"inheritedScaling\":" << jn(inheritedScaling)
        << ",\"eclDisplay\":[" << jn(getDisplayEclipticPos()[0]) << ','
        << jn(getDisplayEclipticPos()[1]) << ',' << jn(getDisplayEclipticPos()[2]) << "]"
        << ",\"visible\":" << ((isVisible & isBodyVisible) ? "true" : "false")
        << ",\"screenSize\":" << jn(screenSize)
        << ",\"haloColor\":[" << jn(haloColor[0]) << ',' << jn(haloColor[1]) << ','
        << jn(haloColor[2]) << "]"
        << ",\"relation\":" << static_cast<int>(relation)
        << ",\"supplemental\":" << (supplemental ? "true" : "false")
        // Preload seam counter (B34, S11.132) - see the member's comment.
        << ",\"preloadCount\":" << preloadCount
        << ",\"evalCount\":" << evalCount
        << ",\"trail\":[";
    {
        const char *sep = "";
        for (const BodyModule *m : trailComponents) {
            out << sep;
            m->dumpState(out);
            sep = ",";
        }
    }
    out << "],\"near\":[";
    {
        const char *sep = "";
        for (const BodyModule *m : nearComponents) {
            out << sep;
            m->dumpState(out);
            sep = ",";
        }
    }
    out << "],\"eclRoot\":[" << jn(matLocalToBodyPos.r[12]) << ','
        << jn(matLocalToBodyPos.r[13]) << ',' << jn(matLocalToBodyPos.r[14]) << "]";
    out << ",\"modules\":[";
    {
        const char *sep = "";
        for (uint32_t i = 0; i < components.size(); ++i) {
            if (components[i]) {
                out << sep << '"' << slotID.nameOf(i) << '"';
                sep = ",";
            }
        }
    }
    out << "],\"routing\":{\"far\":" << farComponents.size()
        << ",\"near\":" << nearComponents.size()
        << ",\"grounded\":" << groundedComponents.size()
        << ",\"in\":" << inComponents.size()
        << ",\"orbit\":" << orbitComponents.size()
        << ",\"trail\":" << trailComponents.size()
        << ",\"tail\":" << tailComponents.size()
        << "},\"lastJD\":" << std::setprecision(17) << jn(lastJD) << '}';
}

void ModularBody::dumpHops(std::ostream &out) const
{
    out << '[';
    for (const ModularBody *b = this; b; b = b->parent) {
        if (b != this)
            out << ',';
        Mat4f up = Mat4f::identity(), down = Mat4f::identity();
        b->transformBodyToParent(up);
        b->transformParentToBody(down);
        const Mat4f tilt = b->computeBodyPosToBody(b->lastJD);
        const Mat4f spin = Mat4f::zrotation(b->computeAxisRotation(b->lastJD) + M_PI_2);
        out << std::setprecision(9) << "{\"name\":\"" << b->englishName
            << "\",\"ecl\":[" << jn(b->eclipticPos[0]) << ',' << jn(b->eclipticPos[1]) << ',' << jn(b->eclipticPos[2])
            << "],\"lastJD\":" << std::setprecision(17) << jn(b->lastJD) << std::setprecision(9)
            << ",\"obliquity\":" << jn(b->re.obliquity)
            << ",\"ascendingNode\":" << jn(b->re.ascendingNode)
            << ",\"offset\":" << jn(b->re.offset)
            << ",\"period\":" << jn(b->re.period)
            << ",\"absoluteTiltFrame\":" << (b->re.absoluteTiltFrame ? "true" : "false");
        const char *names[4] = {"up", "down", "tilt", "spin"};
        const Mat4f *mats[4] = {&up, &down, &tilt, &spin};
        for (int m = 0; m < 4; ++m) {
            out << ",\"" << names[m] << "\":[";
            for (int i = 0; i < 16; ++i)
                out << jn(mats[m]->r[i]) << ((i < 15) ? "," : "");
            out << ']';
        }
        out << '}';
        if (!b->isNotIsolated)
            break;
    }
    out << ']';
}
