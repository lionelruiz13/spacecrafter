#include "ModularBody.hpp"
#include <cstring>
#include <iomanip>
#include <ostream>
#include "ModularBodyPtr.hpp"
#include "ModularSystem.hpp"
#include "EnvironmentManager.hpp"
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
float ModularBody::halfFov = M_PI_2;
// Projection transfer (INTENT 11.33): FISHEYE default matches the config
// default; SSystemFactory ctor mirrors Context::projectionType (post-config).
// cullHalfFov init == halfFov init (edgeAngleNorm(FISHEYE) == 1).
int ModularBody::projectionMode = ProjectionTransfer::FISHEYE;
float ModularBody::cullHalfFov = M_PI_2;
bool ModularBody::flagLightTravelTime = false; // set from config via SSystemFactory
Vec3f ModularBody::defaultHaloColor{};
float ModularBody::haloScale = 1;
float ModularBody::haloSizeLimit = 9;
// System-collapse cross-fade brightness multiplier (B22, INTENT 11.64). 1.0
// everywhere except INSIDE ModularSystem::drawNested's transition band, where
// it ramps a nested system's halo output resolved<->dot. Default 1.0 makes
// every EXERCISED halo (drawHalo/drawStarProxy) byte-identical (x1.0f is an
// exact IEEE-754 identity): the collapse path itself is runtime-unexercised
// until the executor dissolution (§6.9), so in every shipped scene this stays 1.
float ModularBody::drawAlpha = 1.f;
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

// BodyModule's default caster descriptor (BodyModule.hpp hook 2b) - defined
// here because it needs the ModularBody definition: a solid whole-body caster
// (the G1 mesh case). radius = body radius, NOT the module's boundingRadius:
// a shell-inflated bounding radius would shrink the silhouette in its layer
// for no coverage gain. Clip: degenerate (always applies) - selection's
// light-corridor test already carries a solid caster's z-order.
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
    englishName(std::move(info.englishName)), parent(parent), orbit(std::move(info.orbit)), re(info.re), haloColor(info.haloColor), albedo(info.albedo), shadowAbsorbtion(info.shadowAbsorbtion), scaling(1), radius(info.radius), datumRadius(info.datumRadius), groundRadius(info.groundRadius), one_minus_oblateness(1-info.oblateness), solLocalDay(info.solLocalDay), bodyType(info.bodyType), siderealTimeModel(info.siderealTimeModel), surfaceModel(info.surfaceModel), trailLength(info.trailLength), composedDeclaration(info.composedDeclaration), isHaloEnabled(info.isHaloEnabled)
{
    // Nav-radius class default (B10-datum0, §11.75(a)): an UNSET (sentinel)
    // datum/ground resolves to `radius` here - the plain-body default (altitude
    // measured from the surface, free descent stopped at it), bit-identical to a
    // single-reference body. A ModularSystem OVERRIDES this to 0 in its own ctor
    // (a system is navigated INTO). An explicit value (>= 0, from a data key or a
    // runtime command) is never the sentinel and is kept unchanged. The loader
    // already defaults the key to `radius`, so this branch never fires for a
    // scripted/shipped body; it is the type-level completion so no future
    // omitting caller can silently leave a body at the negative sentinel.
    if (datumRadius < 0.f) datumRadius = radius;
    if (groundRadius < 0.f) groundRadius = radius;
    if (translator)
        nameI18 = translator->translateUTF8(englishName);
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
    return ret;
}

void ModularBody::registerToSystem(ModularBody *child)
{
    auto p = this;
    while (p->isNotIsolated)
        p = p->parent;
    static_cast<ModularSystem *>(p)->addBody(child);
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
    // Deregistration symmetric with createChild's registerToSystem: every
    // PARENTED body was registered in the parent-side owning system, whatever
    // its OWN isolation - the previous isNotIsolated gate skipped nested
    // ModularSystem children, dangling their sorted-list entry (INTENT 5.22).
    // Parentless roots (universe) were never registered.
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
    // I5: every cross-frame NON-OWNING holder of this body is either a
    // ModularBodyPtr (redirected below) or destruction-notified here. The
    // environment aggregation caches the previous frame's reference chain as
    // raw pointers to compute enter/leave edges - it is the second kind.
    EnvironmentManager::notifyBodyDestroyed(this);
    if (pointerCount)
        ModularBodyPtr::redirect(this, parent);
}

bool ModularBody::remove(bool recursive)
{
    // Non-recursive removal refuses while ANY child is owned, hidden included
    // (hidden children are parent-owned now - destroying them silently on a
    // non-recursive remove would be an unasked cascade; old-path removeBody
    // refuses on satellites, same class).
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

// Receives this body's POSITION frame (root-aligned - frame contract in the
// header): the ACCUMULATED equatorial frame goes into `mat` only, children
// receive the flat frame (bound children `mat`, their offsets live in the
// surface frame = accumulated + spin). Single authority (I2): this is the
// same frame the camera declares the observer in - routing the render
// through the own tilt instead was the 23.44deg Moon render/observer
// contradiction (INTENT 11.34; measured live, orientation_check.py P-d).
// For the reference body the fold cancels the dispatchUpdate exit exactly:
// mat = C . A^-1 . A = C (same uniform jd both sides).
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
    // Hidden children keep ticking (translation-only) - see updateHiddenBodies.
    // `mat` IS the accumulated surface frame the grounded loop above uses.
    updateHiddenBodies(jd, matLocalToBodyPos, &mat);
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
        for (auto &c : body->groundedBodies)
            c->recursiveTranslationUpdate(jd, mat_local_to_body);
        for (auto &c : body->orbitingBodies)
            c->recursiveTranslationUpdate(jd, flat);
        for (auto &c : body->innerBodies)
            c->recursiveTranslationUpdate(jd, flat);
        // mat_local_to_body == flat . accumulatedBodyPosToBody(jd) exactly
        // (flat was built as its inverse fold above) - the surface frame the
        // grounded loop uses, handed over instead of recomputed.
        body->updateHiddenBodies(jd, flat, &mat_local_to_body);
    }
    while (body->isNotIsolated) {
        body->transformBodyToParent(jd, flat);
        ModularBody *parent = body->parent;
        // Accumulated, not own tilt (I2 single authority): the up-chain
        // ancestors' `mat` and their bound children's surface frame follow
        // the same convention as the descent - an own-tilt fold here would
        // desynchronize a moon-referenced camera's ancestor orientations.
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
        // Same exclusion as the three loops above: `body` is the node the walk
        // came up from and is already updated - and it CAN be hidden (nothing
        // forbids hiding the camera reference).
        parent->updateHiddenBodies(jd, flat, &parentTilted, body);
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
    // Navigation radii scale with the same visual scaling as the render radius
    // (B10 §5.2): datum defaults to radius ⇒ scaledDatumRadius == scaledRadius
    // for every shipped body, bit-identical.
    scaledDatumRadius = datumRadius * scaling;
    scaledGroundRadius = groundRadius * scaling;
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
    // Position-derived reach (subsystemRadius + areaOfInfluence) is computed
    // here for the first-cache / load-time path (ModularSystem.cpp:807), AND
    // recomputed every frame from update() (INTENT 11.62, B15): it tracks jd
    // because eclipticPos moves, whereas the module/radius part above only
    // changes on a radius/scaling/module event. Splitting it out is what stops
    // the AoI freezing at the launch-jd cache (stale transition thresholds
    // after a date jump - measured 10.28% at the scene jd).
    updateReach();
    if (cached)
        uncached = false;
}

void ModularBody::updateReach()
{
    // Subtree extent = |child offset| + the child's OWN subsystem reach.
    // Direct |ecl| alone collapsed nested systems to zero extent (a system
    // node's only direct child can sit at its center - the whole subsystem
    // was invisible to the AoI heuristic; INTENT 5.19).
    float subsystem = boundingRadius;
    forEachVisibleChild([&subsystem](ModularBody &c) {
        const float tmp = c.eclipticPos.length() + c.subsystemRadius;
        if (subsystem < tmp)
            subsystem = tmp;
    });
    // Take some extra margin for the system radius
    subsystemRadius = subsystem * 1.1f;
    // The area of influence is an heuristic. The sibling-separation cap
    // (|ecl|*0.6) only applies off-center: a system-centered or parentless
    // body's influence IS its system's space - at ecl==0 the cap collapsed
    // AoI to zero (the Sun, system nodes at their host's origin; INTENT 5.18),
    // making every reference transition escalate and none descend.
    float aoi = std::max(boundingRadius * 128 / scaling, subsystemRadius * 16);
    const float sibCap = eclipticPos.length() * 0.6f;
    if (sibCap > 0)
        aoi = std::min(aoi, sibCap);
    areaOfInfluence = aoi;
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
    forEachVisibleChild([](ModularBody &c) {
        if (c.isChildVisible)
            c.setChildNoLongerVisible();
        c.distance = 0;
        // Clear the visibility flag too: with the translation-only subtree
        // refresh, distance becomes non-zero again next frame, so a stale
        // isVisible=true would let drawSystem draw a chimera state.
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

// Hide = move ownership to the parent's hiddenBodies (still parent-owned -
// no global list, no parent-tracking workaround; the relation remembers the
// origin list so show() restores exactly). Return true iff it was shown.
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
    // rings gate = old parse parity (protosystem.cpp:777 strToBool(rings, 0)):
    // tex_ring alone doesn't create rings. Deliberate suppression lives HERE,
    // not in RingLoader::isLikely - a 0 bid on a deduced module fires the
    // missing-loader warning (the hint=false lesson, 11.19).
    if (param.count("tex_ring") && Utility::strToBool(param["rings"], false))
        ret.push_back(BodyModuleType::RING);
    if (param.count("tex_map"))
        ret.push_back(BodyModuleType::MESH);
    // model_name has TWO consumers in the old parse (protosystem.cpp:634-641
    // vs 727-739): type=artificial -> an Ojm model (materials, phong,
    // self-shadow - the OJM module); any other type -> a NAMED ObjL drawn by
    // the MESH family (Phobos/Deimos class - BasicMeshLoader carries it).
    // Gating here keeps the deliberate non-OJM case out of the missing-loader
    // warning channel (the rings/hint=false lesson above). Match = the old
    // parse's exact discriminator: the 4-byte prefix switch on "Artificial"
    // (protosystem.cpp:478-487 setBodyTypeFromString).
    if (param.count("model_name")) {
        const std::string &type = param["type"];
        if (type.size() >= 4 && std::memcmp(type.data(), "Arti", 4) == 0)
            ret.push_back(BodyModuleType::OJM);
    }
    // From-space atmosphere shell (row 13). The compound gate mirrors the old
    // parse precondition EXACTLY (protosystem.cpp:865-871): the params block
    // is only read when has_atmosphere or atmosphere_lim_landscape is
    // present, and the shell exists iff atmosphere_ext_model is non-empty.
    // Positioned after MESH/OJM: nearComponent order = record order, the
    // shell draws AFTER the disc (old same-command-buffer parity).
    if ((param.count("has_atmosphere") || param.count("atmosphere_lim_landscape"))
        && !param["atmosphere_ext_model"].empty())
        ret.push_back(BodyModuleType::ATMOSPHERE);
    // Rotation-axis line (row 10): default for every body with a mesh (the
    // landing-zone rule; the old path carried an Axis member on EVERY body -
    // model-only artificial bodies losing theirs is a documented divergence,
    // visible only under `flag axis on`). Positioned after MESH: record order
    // = near-list order, the depth test resolves axis-vs-disc either way.
    if (param.count("tex_map"))
        ret.push_back(BodyModuleType::AXIS);
    // Every named body gets a HINT module by default; hint=false suppresses it
    // HERE (not in HintLoader::isLikely - a 0 there would fire the missing-
    // loader warning for a deliberate suppression).
    if (!englishName.empty() && !Utility::isFalse(param["hint"]))
        ret.push_back(BodyModuleType::HINT);
    // Orbit line (row 8): default for a body with a NON-STILL orbit. The gate
    // is orbit_visualization_period > 0 - EXACTLY the old draw gate
    // (re.sidereal_period, OrbitPlot::doDraw): the Sun/anchors carry no such
    // key and get no orbit, planets/moons do. param orbit=false suppresses the
    // module entirely (deduce gate, not a runtime flag - the hint=false lesson,
    // 11.19: a 0 bid on a deduced module fires the missing-loader warning).
    if (Utility::strToDouble(param["orbit_visualization_period"], 0.0) > 0.0
        && !Utility::isFalse(param["orbit"]))
        ret.push_back(BodyModuleType::ORBIT);
    // Trail line (row 9): position accumulation over sim time. The old trail set
    // was the CLASS dispatch BigBody+SmallBody (types Planet/Dwarf/Asteroid/KBO/
    // Comet); Moon/Sun/Star/Center/Artificial carry NO Trail (Moon has no Trail
    // member + a no-op drawTrail; the others never construct one -
    // protosystem.cpp:645-801). Gate = a non-still orbit (the same "moving" gate
    // as ORBIT, orbit_visualization_period>0) AND not a satellite (Moon
    // exclusion, isSatellite) AND type != Artificial (the new BodyType enum
    // lumps Planet/Moon/Artificial as CUSTOM_BODY, so the discriminators are the
    // parent relation + the raw type string, same "Arti" 4-byte prefix as OJM
    // above). MaxTrail per class is set by the loader from the type.
    {
        const std::string &type = param["type"];
        const bool artificial = type.size() >= 4 && std::memcmp(type.data(), "Arti", 4) == 0;
        if (Utility::strToDouble(param["orbit_visualization_period"], 0.0) > 0.0
            && !isSatellite() && !artificial)
            ret.push_back(BodyModuleType::TRAIL);
    }
    // Comet tail (row 12): the old SmallBody bound tails ONLY for a comet
    // carrying an apparent_magnitude AND slope (protosystem.cpp:802) - the
    // coma/tail-size formula needs the absolute magnitude H + activity slope G.
    // Deduce for type=Comet with apparent_magnitude present (the "type=Comet"
    // landing-zone rule, gated by the old data precondition - a comet without
    // photometry drew no tail, so the module would draw nothing/garbage), OR an
    // explicit tail=true (the "or explicit" half). tail=false suppresses HERE
    // (deduce gate, not a runtime flag - a 0 bid on a deduced module fires the
    // missing-loader warning, the hint=false lesson §11.19). Magnitude coupling
    // (the landing zone's albedo/radius vs the old H/G) SUSPENDED for Vixy, §11.43.
    {
        const std::string &type = param["type"];
        const bool comet = type.size() >= 4 && std::memcmp(type.data(), "Come", 4) == 0;
        const bool hasMag = !param["apparent_magnitude"].empty();
        if (!Utility::isFalse(param["tail"])
            && (Utility::isTrue(param["tail"]) || (comet && hasMag)))
            ret.push_back(BodyModuleType::TAIL);
    }
    // Star big-halo glow (row 14): the old Sun/BodyStar bound a tex_big_halo
    // (protosystem.cpp:687-691) only on STAR-typed bodies. Gate = STAR bit
    // (strToBodyType maps type=Sun/Star -> STAR) AND a non-empty tex_big_halo -
    // exactly the old setBigHalo precondition. Deduced as CUSTOM: StarLoader
    // (co-registered with GridLoader) outbids on stars, so it lands in the
    // default "CUSTOM" slot; the Sun carries no planet_grid, so no slot clash
    // (INTENT §11.44). The disc itself stays MESH (§11.19a "bare disc"); this
    // module is only the additive screen-space halo.
    if (isStar() && !param["tex_big_halo"].empty())
        ret.push_back(BodyModuleType::CUSTOM);
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
        // B32 RECOMPUTE-AT-USE (D20 §11.79(n), the D8 §11.76 barrier): a dump is
        // a USE, so the spin phase is recomputed here from the ROOT-fresh lastJD
        // (translation tick keeps lastJD current on EVERY body, visible or not,
        // B19) through the ONE authority computeAxisRotation (I2) - NOT read from
        // the visibility-gated axisRotation cache. The cache is a per-frame
        // memoization the tick maintains ONLY for visible bodies (update()); it
        // is fresh-by-construction exactly where the draw path reads it (a body
        // is drawn iff visible iff updated this frame), and STALE everywhere the
        // tick genuinely froze the spin (invisible / frozen-under-invisible-
        // parent). Reading the cache in the dump leaked the LAUNCH wall-clock
        // spin phase of the last visible update - measured up to 4.49 rad of
        // cross-launch scatter on Moon/Deimos/Phobos/Mars/Mercury (§5.24; the
        // ~20 pole-bearing moons in a scene where they are invisible). The
        // recompute is a closed-form evaluation (computeAxisRotation is a
        // polynomial+fmod, or the finite nutation series for EARTH_APPARENT - no
        // convergence loop), so the §11.76 +4-iterations restoration does NOT
        // apply to spin (that clause is the ITERATIVE Kepler position solve,
        // §11.76 territory, untouched here). Deterministic: a function of the
        // bit-identical lastJD, so two fresh launches now agree exactly.
        << "],\"axisRot\":" << computeAxisRotation(lastJD)
        // `attitude` == axisRot since the B32 fix (both = computeAxisRotation
        // (lastJD)); retained as the B24-att-named channel the b24_compose /
        // b25 harnesses read (§11.90/§11.91) - not removed to avoid a dump-
        // format break in landed evidence.
        << ",\"attitude\":" << computeAxisRotation(lastJD)
        << ",\"surfaceLocked\":" << (surfaceLockedAttitude ? "true" : "false")
        // B27-tail capability instrument (§11.107): the capabilities that used
        // to be derived from the `type` data string, read at the body where they
        // now live. `bodyType` as its integer so the STAR (0x40) / MINOR_BODY (4)
        // / CUSTOM_BODY (7) distinction is exact-comparable - it is the ONLY
        // externally observable of the Tier-B resolution, and legacy-vs-composed
        // equality on it IS the co-delivery proof for `light_source` /
        // `shadow_exempt`. `composedDecl` reports WHICH format declared the body
        // (D14 scope), so a composed leg can be shown to have actually taken the
        // composed resolution rather than silently re-running the legacy one -
        // it is the one field here that legitimately DIFFERS between the two
        // legs, and is therefore deliberately excluded from the equality gate.
        << ",\"bodyType\":" << static_cast<int>(bodyType)
        << ",\"surfaceModel\":" << static_cast<int>(surfaceModel)
        << ",\"trailLength\":" << trailLength
        << ",\"composedDecl\":" << (composedDeclaration ? "true" : "false")
        << ",\"boundingRadius\":" << boundingRadius
        // Navigation radii, scaled, in AU (B10 §5.2 / B10-cmd instrument): the
        // ONLY numeric observable of the datum_radius/ground_radius scalars, so
        // the harness can read the runtime `body name X datum_radius|ground_radius`
        // command taking effect (the behavioral discriminators - moveto altitude
        // 0 -> centre, free-descent hold at ground - ride these two values).
        << ",\"scaledDatumRadius\":" << scaledDatumRadius
        << ",\"scaledGroundRadius\":" << scaledGroundRadius
        << ",\"visible\":" << ((isVisible & isBodyVisible) ? "true" : "false")
        << ",\"screenSize\":" << screenSize
        // Halo color (B29 runtime-color instrument, INTENT §11.65): the
        // body-owned color channel (haloColor, consumed by drawHalo). The
        // LABEL/ORBIT/TRAIL channels live on their modules; trail's is in its
        // dumpState below, orbit/label are measured on screen. Lets the harness
        // read the runtime recolor + its reload behaviour numerically.
        << ",\"haloColor\":[" << haloColor[0] << ',' << haloColor[1] << ','
        << haloColor[2] << "]"
        // relation = the membership authority (which parent list owns this
        // body); makes hide/show structurally observable from the harness -
        // dump PRESENCE never tracks it (the dump iterates the name registry,
        // which includes hidden bodies). INTENT 11.36 rare-path instrument.
        << ",\"relation\":" << static_cast<int>(relation)
        // Trail recording state (B11 instrument, INTENT 11.56): the ONLY
        // externally observable of the recording gate. Written by the module
        // itself (BodyModule::dumpState - no type sniffing here, I4). An
        // ARRAY because trailComponents is a list; empty list -> `[]`, which
        // is itself the finding "this body has no trail module".
        << ",\"trail\":[";
    {
        const char *sep = "";
        for (const BodyModule *m : trailComponents) {
            out << sep;
            m->dumpState(out);
            sep = ",";
        }
    }
    // Near-component state (B23 GRID instrument, INTENT §11.57): the planet-grid
    // module lives here (addNearComponent). Its dumpState reports the tropic/
    // polar latitudes actually baked for this body (obliquity readout, DoD-2);
    // every other near module keeps the "null" default, so the non-null entry
    // is the grid.
    out << "],\"near\":[";
    {
        const char *sep = "";
        for (const BodyModule *m : nearComponents) {
            out << sep;
            m->dumpState(out);
            sep = ",";
        }
    }
    // eclRoot (B24 grounded instrument, INTENT §11.78): the parent-relative
    // position AFTER the surface fold - matLocalToBodyPos's translation, the
    // offset actually composed into the world. For orbiting bodies == ecl;
    // for grounded bodies it exposes what the fold DID (ecl is the fold's
    // INPUT, constant in the surface frame, and cannot show co-rotation).
    // Kept fresh for every body by the translation tick (B19 mechanism) -
    // unlike `mat`, which is chimeric on invisible bodies (INTENT 11.14b);
    // spin freshness is the parent's (stale-spin finding, §11.78).
    out << "],\"eclRoot\":[" << matLocalToBodyPos.r[12] << ','
        << matLocalToBodyPos.r[13] << ',' << matLocalToBodyPos.r[14] << "]";
    // Slot inventory + routing counts (B24 equivalence instrument, INTENT
    // §11.78): `modules` = the filled slot names (module-set identity per
    // body - what the legacy-vs-composed equivalence compares); `routing` =
    // per-list module counts (the relation= override's observable: a reroute
    // moves a module between lists without changing the slot inventory).
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
        << "},\"lastJD\":" << std::setprecision(17) << lastJD << '}';
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
        // B32 recompute-at-use (D20 §11.79(n)): the dumped spin matrix is a USE,
        // recomputed from the fresh lastJD through the ONE authority (I2), not
        // read from the visibility-gated axisRotation cache computeBodyToSurface
        // reads. Fixes the §11.54(j) stale-spin: Pluto/Charon are hidden, so
        // their cached spin was frozen at JD 0 and every dumpHops spin matrix for
        // them was evaluated stale; now it tracks lastJD like `tilt` above. The
        // +M_PI_2 mirrors getAxisRotation()'s convention exactly.
        const Mat4f spin = Mat4f::zrotation(b->computeAxisRotation(b->lastJD) + M_PI_2);
        out << std::setprecision(9) << "{\"name\":\"" << b->englishName
            << "\",\"ecl\":[" << b->eclipticPos[0] << ',' << b->eclipticPos[1] << ',' << b->eclipticPos[2]
            << "],\"lastJD\":" << std::setprecision(17) << b->lastJD << std::setprecision(9)
            // Raw rotation-frame readout (B28 bit-identical gate, INTENT 11.67):
            // the loader-resolved obliquity/ascendingNode and the declared frame
            // flag. These are projection-free and jd-only-through-precession, so
            // the frame conversion is measurable to float-ulp independent of the
            // B30 render jitter that perturbs the composed `mat`.
            << ",\"obliquity\":" << b->re.obliquity
            << ",\"ascendingNode\":" << b->re.ascendingNode
            // re.offset (rot_rotation_offset, DEGREES) - the prime-meridian phase
            // at epoch. Projection-free, load-time readout of the rot_pole_w0 ->
            // offset conversion (B14-W0, §11.79(a)); the DIRECT observable the
            // W0-conversion discriminator reads (visibility-independent, unlike
            // the live axisRotation which only refreshes on visible bodies).
            << ",\"offset\":" << b->re.offset
            // re.period (rot_periode/24, DAYS, float32 as loaded) - the sidereal
            // rotation rate the spin formula (ModularBody.hpp:350) divides into.
            // Projection-free, load-time, deterministic (no jd/visibility jitter,
            // unlike the cached axisRotation/spin) => the DIRECT commutator-class
            // observable of the rot_periode data channel (B14-periode, §11.86(d)/
            // §11.87(e)): a body's row moves iff its rot_periode was edited. Sign
            // encodes spin direction (negative = retrograde, the Venus -5832 h /
            // Uranus-moon Ẇ<0 convention).
            << ",\"period\":" << b->re.period
            << ",\"absoluteTiltFrame\":" << (b->re.absoluteTiltFrame ? "true" : "false");
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
