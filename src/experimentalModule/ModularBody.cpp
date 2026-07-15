#include "ModularBody.hpp"
#include <iomanip>
#include <ostream>
#include "ModularBodyPtr.hpp"
#include "ModularSystem.hpp"
#include "RenderChain.hpp"
#include "tools/log.hpp"
#include "tools/translator.hpp"
#include "tools/utility.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"

Vec3f ModularBody::lightPosition;
float ModularBody::lightDistance;
float ModularBody::lightSize;
ModularBody *ModularBody::lastFit = nullptr;
std::map<std::string, ModularBody *> ModularBody::bodyReference;
std::list<ModularBody> ModularBody::hidden;
float ModularBody::halfFov = M_PI_2;
bool ModularBody::flagLightTravelTime = false; // set from config via SSystemFactory
Vec3f ModularBody::defaultHaloColor{};
float ModularBody::haloScale = 1;
float ModularBody::haloSizeLimit = 9;
float ModularBody::viewportRadius = 1;
std::vector<ModularBody *> ModularBody::notableBody;
float ModularBody::deltaTime = 0;
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

ModularBody::ModularBody(ModularBody *parent, ModularBodyCreateInfo &info) :
    englishName(std::move(info.englishName)), parent(parent), orbit(std::move(info.orbit)), re(info.re), haloColor(info.haloColor), albedo(info.albedo), shadowAbsorbtion(info.shadowAbsorbtion), scaling(1), radius(info.radius), one_minus_oblateness(1-info.oblateness), solLocalDay(info.solLocalDay), bodyType(info.bodyType), isHaloEnabled(info.isHaloEnabled)
{
    if (translator)
        nameI18 = translator->translateUTF8(englishName);
    auto &ref = bodyReference[englishName];
    if (ref && !englishName.empty()) {
        // It is too late to abort the body creation, so we replace it
        cLog::get()->write("Body with name '" + englishName + "' already exists, replacing it", LOG_TYPE::L_INFO);
        childs = std::move(ref->childs);
        for (auto &child : childs)
            child.parent = this;
        if (ref->pointerCount)
            ModularBodyPtr::redirect(ref, this);
        ref->remove(true);
        bodyReference[englishName] = this;
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

ModularBody *ModularBody::createChild(ModularBodyCreateInfo &info)
{
    childs.emplace_back(this, info);
    auto ret = &childs.back();
    auto p = this;
    while (p->isNotIsolated)
        p = p->parent;
    static_cast<ModularSystem *>(p)->addBody(ret);
    return ret;
}

ModularBody::~ModularBody()
{
    // I5 guard: if this body's orbit is wired as the secondary of the parent's
    // BinaryOrbit (EMB class, see ModularSystem::loadBody), unwire before the
    // orbit is destroyed - the BinaryOrbit references it without ownership.
    if (parent && orbit) {
        if (auto *binary = dynamic_cast<BinaryOrbit *>(parent->orbit.get()))
            binary->clearSecondaryOrbit(orbit.get());
    }
    if (isNotIsolated) {
        auto p = parent;
        while (p->isNotIsolated)
            p = p->parent; // Search for system center
        static_cast<ModularSystem *>(p)->removeBody(this);
    }
    childs.clear();
    groundedBodies.clear();
    orbitingBodies.clear();
    innerBodies.clear();
    components.clear();
    groundedEnvironment.clear();
    environment.clear();
    bodyReference.erase(englishName);
    components.clear();
    if (lastFit == this)
        lastFit = nullptr;
    if (pointerCount)
        ModularBodyPtr::redirect(this, parent);
}

bool ModularBody::remove(bool recursive)
{
    if (recursive || childs.empty()) {
        const auto end = hidden.end();
        for (auto it = hidden.begin(); it != end; ++it) {
            if (&*it == this) {
                ModularBodyPtr::rawUntrack(parent);
                hidden.erase(it);
                return true;
            }
        }
        parent->childs.remove(*this);
        return true;
    } else {
        return false;
    }
}

// Receives this body's POSITION frame (root-aligned - frame contract in the
// header): the tilt goes into `mat` only, children receive the flat frame
// (bound children the tilted one, their offsets live in the surface frame).
void ModularBody::recursiveUpdate(double jd, const Mat4f &matLocalToBodyPos)
{
    mat = matLocalToBodyPos.multiplyFast(computeBodyPosToBody(jd));
    update(jd, mat);
    for (auto &c : childs)
        c.selectiveUpdate(jd, c.boundToSurface ? mat : matLocalToBodyPos);
}

ModularSystem *ModularBody::dispatchUpdate(ModularBody *body, double jd, Mat4f mat_local_to_body)
{
    // Produce/consume boundary of notableBody: cleared at update start (runs
    // every frame, whichever path draws - the dual-path bridge made a
    // draw-side-only drain unbounded during old-path phases), refilled by
    // update(), read by the Renderer between this update and the next.
    notableBody.clear();
    body->preUpdate(jd, mat_local_to_body);
    // The camera mat is the reference's ACCUMULATED equatorial frame (its
    // surface/spin composition and the body's lat/lon grid are defined there -
    // see accumulatedBodyToBodyPos); the chain works in root-aligned frames -
    // leave the accumulated frame exactly once, here (frame contract above).
    Mat4f flat = mat_local_to_body.multiplyFast(body->accumulatedBodyToBodyPos(jd));
    if (body->isVisible) {
        body->recursiveUpdate(jd, flat);
    } else {
        body->mat = mat_local_to_body;
        for (auto &c : body->childs)
            c.recursiveTranslationUpdate(jd, c.boundToSurface ? mat_local_to_body : flat);
    }
    while (body->isNotIsolated) {
        body->transformBodyToParent(jd, flat);
        ModularBody *parent = body->parent;
        const Mat4f parentTilted = flat.multiplyFast(parent->computeBodyPosToBody(jd));
        for (auto &b : parent->childs) {
            if (&b != body)
                b.selectiveUpdate(jd, b.boundToSurface ? parentTilted : flat);
        }
        body = parent;
        body->mat = parentTilted; // assign BEFORE update: update() reads the member
        body->preUpdate(jd, flat);
        body->update(jd, parentTilted);
    }
    return static_cast<ModularSystem *>(body);
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
    scaledRadius = radius * scaling;
    boundingRadius = scaledRadius;
    for (auto &module : nearComponents) {
        cached &= module->update(this, scaledRadius);
        if (boundingRadius < module->getBoundingRadius())
            boundingRadius = module->getBoundingRadius();
    }
    for (auto &module : inComponents) {
        cached &= module->update(this, scaledRadius);
    }
    if (parent)
        parent->invalidateCachedState();
    float squaredSubsystemRadius = boundingRadius*boundingRadius;
    for (auto &c : childs) {
        const float tmp = c.eclipticPos.lengthSquared();
        if (squaredSubsystemRadius < tmp)
            squaredSubsystemRadius = tmp;
    }
    // Take some extra margin for the system radius
    subsystemRadius = sqrt(squaredSubsystemRadius) * 1.1f;
    // The area of influence is an heuristic
    areaOfInfluence = std::min(std::max(boundingRadius * 128 / scaling, subsystemRadius * 16), eclipticPos.length() * 0.6f);
    if (cached)
        uncached = false;
}

void ModularBody::drawLoaded(Renderer &renderer)
{
    loaded = true;
    const auto matrix = mat.multiplyFast(computeBodyToSurface()); // TODO Fix ojml ?
    if (screenSize > 0.008) {
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
            if (distance < scaledRadius) {
                for (auto &module : inComponents) {
                    if (module->isLoaded()) {
                        module->draw(renderer, this, matrix);
                    } else
                        loaded = false;
                }
                for (auto &module : nearComponents)
                    loaded &= module->isLoaded();
                return;
            } else {
                for (auto &module : nearComponents) {
                    if (module->isLoaded()) {
                        module->draw(renderer, this, matrix);
                    } else
                        loaded = false;
                }
            }
        }
    } else {
        for (auto &module : farComponents)
            module->draw(renderer, this, mat);
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
    for (auto &c : childs) {
        if (c.isChildVisible)
            c.setChildNoLongerVisible();
        c.distance = 0;
        // Clear the visibility flag too: with the translation-only subtree
        // refresh, distance becomes non-zero again next frame, so a stale
        // isVisible=true would let drawSystem draw a chimera state.
        c.isVisible = false;
    }
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
    const auto end = parent->childs.end();
    for (auto it = parent->childs.begin(); it != end; ++it) {
        if (&*it != this) // was missing: hide() used to hide the parent's FIRST
            continue;     // child instead of this one (show() had the test)
        isVisible = false;
        if (isChildVisible)
            setChildNoLongerVisible();
        hidden.splice(hidden.begin(), parent->childs, it);
        ModularBodyPtr::rawTrack(parent);
        parent->invalidateCachedState();
        return true;
    }
    return false;
}

bool ModularBody::show()
{
    const auto end = hidden.end();
    for (auto it = hidden.begin(); it != end; ++it) {
        if (&*it == this) {
            parent->childs.splice(parent->childs.begin(), hidden, it);
            ModularBodyPtr::rawUntrack(parent);
            parent->invalidateCachedState();
            return true;
        }
    }
    return false;
}

Mat4f ModularBody::calculateSwitchCompensation(const ModularBody *to) const
{
    // Maps `to`-equatorial coordinates to this-equatorial coordinates (both
    // references hold their ACCUMULATED equatorial frame - see
    // accumulatedBodyToBodyPos): comp = acc(this)^-1 . [flat translations via
    // the common parent] . acc(to).
    // Left-to-right build = reverse of right-to-left application order.
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

void ModularBody::setTranslator(Translator &_translator)
{
    translator = &_translator;
    for (auto &ref : bodyReference)
        ref.second->nameI18 = _translator.translateUTF8(ref.second->englishName);
    viewportRadius = VulkanMgr::instance->getScreenRect().extent.width/2;
}

std::vector<BodyModuleType> ModularBody::deduceBodyModuleList(std::map<std::string, std::string> &param)
{
    std::vector<BodyModuleType> ret;
    if (param.count("tex_ring"))
        ret.push_back(BodyModuleType::RING);
    if (param.count("tex_map"))
        ret.push_back(BodyModuleType::MESH);
    if (param.count("model_name"))
        ret.push_back(BodyModuleType::OJM);
    // From-space atmosphere shell (row 13). The compound gate mirrors the old
    // parse precondition EXACTLY (protosystem.cpp:865-871): the params block
    // is only read when has_atmosphere or atmosphere_lim_landscape is
    // present, and the shell exists iff atmosphere_ext_model is non-empty.
    // Positioned after MESH/OJM: nearComponent order = record order, the
    // shell draws AFTER the disc (old same-command-buffer parity).
    if ((param.count("has_atmosphere") || param.count("atmosphere_lim_landscape"))
        && !param["atmosphere_ext_model"].empty())
        ret.push_back(BodyModuleType::ATMOSPHERE);
    // Every named body gets a HINT module by default; hint=false suppresses it
    // HERE (not in HintLoader::isLikely - a 0 there would fire the missing-
    // loader warning for a deliberate suppression).
    if (!englishName.empty() && !Utility::isFalse(param["hint"]))
        ret.push_back(BodyModuleType::HINT);
    return ret;
}

// Dual-path trace harness (INTENT.md 11.14).
// One JSON object with the NEW-path per-body transform state: parent-relative
// position (ecl, float - the post-downcast value actually used), observer
// matrix, distance, screen position, axis rotation, last evaluation jd.
// Precision 9 = round-trip-exact float; lastJD at 17 (double).
void ModularBody::dumpTrace(std::ostream &out) const
{
    out << std::setprecision(9) << "{\"parent\":\""
        << (parent ? parent->englishName : "") << "\",\"ecl\":["
        << eclipticPos[0] << ',' << eclipticPos[1] << ',' << eclipticPos[2]
        << "],\"mat\":[";
    for (int i = 0; i < 16; ++i)
        out << mat.r[i] << ((i < 15) ? "," : "");
    out << "],\"dist\":" << distance
        << ",\"screen\":[" << screenPos.first << ',' << screenPos.second
        << "],\"axisRot\":" << axisRotation
        << ",\"boundingRadius\":" << boundingRadius
        << ",\"visible\":" << ((isVisible & isBodyVisible) ? "true" : "false")
        << ",\"screenSize\":" << screenSize
        << ",\"lastJD\":" << std::setprecision(17) << lastJD << '}';
}

// Harness (INTENT.md 11.14a): per-hop construction pieces, this body -> root.
// Since the flat-chain rework, up/down are pure-translation hops (+ bound
// folds); tilt/spin are dumped separately and exact mutual inverses hold.
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
        const Mat4f spin = b->computeBodyToSurface();
        out << std::setprecision(9) << "{\"name\":\"" << b->englishName
            << "\",\"ecl\":[" << b->eclipticPos[0] << ',' << b->eclipticPos[1] << ',' << b->eclipticPos[2]
            << "],\"lastJD\":" << std::setprecision(17) << b->lastJD << std::setprecision(9);
        const char *names[4] = {"up", "down", "tilt", "spin"};
        const Mat4f *mats[4] = {&up, &down, &tilt, &spin};
        for (int m = 0; m < 4; ++m) {
            out << ",\"" << names[m] << "\":[";
            for (int i = 0; i < 16; ++i)
                out << mats[m]->r[i] << ((i < 15) ? "," : "");
            out << ']';
        }
        out << '}';
        if (!b->isNotIsolated)
            break;
    }
    out << ']';
}
