#include "ModularBody.hpp"
#include "ModularBodyPtr.hpp"
#include "ModularSystem.hpp"
#include "tools/log.hpp"
#include "tools/translator.hpp"

Vec3f ModularBody::lightPosition;
float ModularBody::lightDistance;
float ModularBody::lightSize;
ModularBody *ModularBody::lastFit = nullptr;
std::map<std::string, ModularBody *> ModularBody::bodyReference;
std::list<ModularBody> ModularBody::hidden;
float ModularBody::halfFov = M_PI_2;
Vec3f ModularBody::defaultHaloColor{};
float ModularBody::haloScale = 1;
float ModularBody::haloSizeLimit = 9;
std::vector<ModularBody *> ModularBody::notableBody;
Translator *ModularBody::translator = nullptr;

ModularBody::ModularBody(ModularBody *parent, ModularBodyCreateInfo &info) :
    englishName(std::move(info.englishName)), parent(parent), orbit(std::move(info.orbit)), re(info.re), haloColor(info.haloColor), albedo(info.albedo), radius(info.radius), innerRadius(info.innerRadius), one_minus_oblateness(1-info.oblateness), solLocalDay(info.solLocalDay), bodyType(info.bodyType), isHaloEnabled(info.isHaloEnabled)
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
        computedJD = parent->lastJD;
        Vec3d tmp;
        if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
            (*oscFunc)(computedJD,computedJD,tmp);
        } else {
            orbit->positionAtTimevInVSOP87Coordinates(computedJD,computedJD,tmp);
        }
        computedEclipticPos = tmp;
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
    if (isNotIsolated) {
        auto p = parent;
        while (p->isNotIsolated)
            p = p->parent; // Search for system center
        static_cast<ModularSystem *>(p)->removeBody(this);
    }
    childs.clear();
    bodyReference.erase(englishName);
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

void ModularBody::updateEclipticPos(Vec3f eclipticPos, double jd, double targetJD)
{
   Vec3d tmp;
   if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
       (*oscFunc)(targetJD,targetJD,tmp);
   } else {
       orbit->positionAtTimevInVSOP87Coordinates(targetJD,targetJD,tmp);
   }
   computedEclipticPos = tmp;
   deltaEclipticPos = (computedEclipticPos - eclipticPos) / (targetJD - jd);
   computedJD = jd;
}

void ModularBody::recursiveUpdate(double jd, const Mat4f &matLocalToBody)
{
    mat = matLocalToBody;
    update(jd, matLocalToBody);
    for (auto &c : childs)
        c.selectiveUpdate(jd, matLocalToBody);
}

ModularSystem *ModularBody::dispatchUpdate(ModularBody *body, double jd, Mat4f mat_local_to_body)
{
    body->preUpdate(jd, mat_local_to_body);
    if (body->isVisible)
        body->recursiveUpdate(jd, mat_local_to_body);
    while (body->isNotIsolated) {
        body->transformBodyToParent(jd, mat_local_to_body);
        ModularBody *parent = body->parent;
        for (auto &b : parent->childs) {
            if (&b != body)
                b.selectiveUpdate(jd, mat_local_to_body);
        }
        body = parent;
        body->preUpdate(jd, mat_local_to_body);
        body->update(jd, mat_local_to_body);
        body->mat = mat_local_to_body;
    }
    return static_cast<ModularSystem *>(body);
}

void ModularBody::select()
{
    isSelected = true;
}

void ModularBody::deselect()
{
    isSelected = false;
}

void ModularBody::updateCache()
{
    bool cached = true;
    scaledRadius = radius * scaling;
    scaledInnerRadius = innerRadius * scaling;
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
        const float tmp = c.computedEclipticPos.lengthSquared();
        if (squaredSubsystemRadius < tmp)
            squaredSubsystemRadius = tmp;
    }
    // Take some extra margin for the system radius
    subsystemRadius = sqrt(squaredSubsystemRadius) * 1.1f;
    // The area of influence is an heuristic
    areaOfInfluence = std::min(std::max(boundingRadius * 128 / scaling, subsystemRadius * 16), computedEclipticPos.length() * 0.6f);
    if (cached)
        uncached = false;
}

void ModularBody::drawLoaded(Renderer &renderer)
{
    loaded = true;
    const auto matrix = mat.multiplyFast(computeBodyToSurface()); // TODO Fix ojml ?
    if (screenSize > 0.008) {
        renderer.clearDepth(distance, boundingRadius);
        if (screenSize < 0.2) {
            for (auto &module : farComponents)
                module->draw(renderer, this, mat);
            for (auto &module : nearComponents) {
                if (module->isLoaded()) {
                    module->draw(renderer, this, matrix);
                } else
                    loaded = false;
            }
        } else {
            for (auto &module : farComponents)
                module->draw(renderer, this, mat);
            if (distance < scaledInnerRadius) {
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
    Mat4f diff(Mat4f::identity());
    auto common = findCommonParent(to);
    // Start from target body
    for (auto body = to; body != common; body = body->parent)
        body->transformBodyToParent(diff);
    // Find dependency chain from the common body to this body
    std::vector<const ModularBody *> travel;
    for (auto body = this; body != common; body = body->parent)
        travel.push_back(body);
    // Go to this body
    while (travel.size()) {
        travel.back()->transformParentToBody(diff);
        travel.pop_back();
    }
    return diff;
}

void ModularBody::setTranslator(Translator &_translator)
{
    translator = &_translator;
    for (auto &ref : bodyReference)
        ref.second->nameI18 = _translator.translateUTF8(ref.second->englishName);
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
    return ret;
}
