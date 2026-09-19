#include "CameraAnchors.hpp"
#include "Camera.hpp"
#include "ModularSystem.hpp"
#include "ModuleLoaderMgr.hpp"
#include "bodyModule/orbit.hpp" // the travel is a motion law (B4(iv), S11.141)
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT S5.39/D29)
#include "tools/log.hpp"
#include "tools/sc_const.hpp" // AU, in km - the unit `altitude` is given in
#include <iomanip>
#include <sstream>
#include <cmath>
#include <fstream>
#include <ostream>

CameraAnchors::~CameraAnchors() = default;

namespace {
class TravelOrbit : public Orbit {
public:
    TravelOrbit(const ModularBody *parent, const Vec3d &startRoot, const Vec3d &direction,
                double distance, double startTime, double travelTime,
                std::unique_ptr<Orbit> resume)
        : parent(parent), startRoot(startRoot), direction(direction), distance(distance),
          startTime(startTime), travelTime(travelTime), arrivalTime(startTime + travelTime),
          resume(std::move(resume)) {}

    void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const override {
        if (resume && JD >= arrivalTime) {
            resume->positionAtTimevInVSOP87Coordinates(JD0, JD, v);
            return;
        }
        const Vec3d p = travelPosition(JD) - parent->getCachedRootPosition();
        v[0] = p[0];
        v[1] = p[1];
        v[2] = p[2];
    }
    // A travelling place is not on a Keplerian path: nothing may precess it.
    bool useParentPrecession(double) const override {
        return false;
    }
    //! Hand the resume law over: a NEW travel replaces this one, and the chain
    //! must not nest one wrapper per travel a show performs.
    std::unique_ptr<Orbit> takeResume() {
        return std::move(resume);
    }
    std::string saveOrbit() const override {
        return "coord_func = still_orbit\norbit_x = " + std::to_string(startRoot[0] + direction[0]*distance)
             + "\norbit_y = " + std::to_string(startRoot[1] + direction[1]*distance)
             + "\norbit_z = " + std::to_string(startRoot[2] + direction[2]*distance) + "\n";
    }
private:
    //! anchor_manager.cpp:406 verbatim, in root coordinates.
    Vec3d travelPosition(double JD) const {
        if (travelTime <= 0)
            return startRoot + direction * distance;
        if (JD > arrivalTime)
            JD = arrivalTime;
        if (JD < startTime)
            JD = startTime;
        constexpr double s = 1;
        constexpr double u = 2;
        const double x = (((JD - startTime) / travelTime) * 25) - 5;
        const double logistic = 1 / (1 + exp(-(x - u) / s));
        return startRoot + direction * (distance * logistic);
    }
    const ModularBody *parent;
    Vec3d startRoot;
    Vec3d direction;
    double distance;
    double startTime;
    double travelTime;
    double arrivalTime;
    std::unique_ptr<Orbit> resume;
};
} // namespace

// The `type` vocabulary of anchor.ini, mapped ONCE (I2/I4: no consumer below
// re-reads the string). `observatory` folds into FIXED_POINT - see the header.
bool CameraAnchors::parseKind(const stringHash_t &params, const std::string &name, AnchorKind &out)
{
    const auto it = params.find("type");
    const std::string type = (it == params.end()) ? std::string() : it->second;
    if (type == "orbit") {
        out = AnchorKind::ON_ORBIT;
        return true;
    }
    if (type == "body") {
        out = AnchorKind::ATTACHED;
        return true;
    }
    if (type == "point" || type == "observatory") {
        out = AnchorKind::FIXED_POINT;
        return true;
    }
    cLog::get()->write("Anchor '" + name + "': " + (type.empty()
            ? std::string("no type declared")
            : ("unknown type = '" + type + "'"))
        + ". Valid values are 'orbit' (a point on an orbit around parent = <body>), "
        "'body' (attached to body_name = <body>, with follow_rotation = true|false), "
        "and 'point'/'observatory' (a fixed point x/y/z in AU, read in the Universe "
        "frame). This anchor is NOT created. To fix: add type = <one of those> to "
        "the anchor declaration.", LOG_TYPE::L_ERROR);
    return false;
}

ModularBody *CameraAnchors::createAnchorBody(const std::string &name, ModularBody *parent, stringHash_t &orbitParams)
{
    auto orbit = ModuleLoaderMgr::instance.loadOrbit(orbitParams);
    if (!orbit) {
        cLog::get()->write("Anchor '" + name + "': could not build an orbit from coord_func = '"
            + orbitParams["coord_func"] + "'. This anchor is NOT created. To fix: use a "
            "coord_func the engine provides (ell_orbit, still_orbit, barycenter, comet_orbit, "
            "location_orbit, surface_point, or a *_special function) and give it the keys it "
            "needs.", LOG_TYPE::L_ERROR);
        return nullptr;
    }
    ModularBodyCreateInfo info {
        .orbit = std::move(orbit),
        .englishName = name,
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = 0,
        .datumRadius = 0,
        .groundRadius = 0,
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::ANCHOR,
        .isHaloEnabled = false,
    };
    ModularBody *body = parent->createChild(info, BodyRelation::ORBITING);
    // Hidden: parent-owned, position-ticking, outside every draw/pick walk.
    body->hide();
    body->captureAuthoredState();
    return body;
}

bool CameraAnchors::buildBody(Anchor &anchor)
{
    if (anchor.kind == AnchorKind::ATTACHED) {
        const std::string &target = anchor.params["body_name"];
        ModularBody *body = ModularBody::findBody(target);
        if (!body) {
            cLog::get()->write("Anchor '" + anchor.name + "': body_name = '" + target
                + "' is not a loaded body, so there is nothing to attach to. This anchor is "
                "NOT created. To fix: name a body of the loaded system (its english name), "
                "or declare the anchor after the body's system is loaded.", LOG_TYPE::L_ERROR);
            return false;
        }
        anchor.body = body;
        anchor.ownsBody = false;
        return true;
    }
    // Owned kinds: the anchor body carries the anchor's NAME, so a collision
    // with a real body would REPLACE that body (ModularBody ctor) - refused.
    if (ModularBody::exists(anchor.name)) {
        cLog::get()->write("Anchor '" + anchor.name + "': a body with that name already "
            "exists, and creating the anchor would replace it. This anchor is NOT created. "
            "To fix: give the anchor a name no body uses.", LOG_TYPE::L_ERROR);
        return false;
    }
    if (anchor.kind == AnchorKind::FIXED_POINT) {
        if (anchor.params["x"].empty() || anchor.params["y"].empty() || anchor.params["z"].empty()) {
            cLog::get()->write("Anchor '" + anchor.name + "': a fixed point needs all three "
                "coordinates x, y and z (in AU). This anchor is NOT created. To fix: add the "
                "missing coordinate(s).", LOG_TYPE::L_ERROR);
            return false;
        }
        stringHash_t orbitParams;
        orbitParams["coord_func"] = "still_orbit";
        orbitParams["orbit_x"] = anchor.params["x"];
        orbitParams["orbit_y"] = anchor.params["y"];
        orbitParams["orbit_z"] = anchor.params["z"];
        ModularBody *body = createAnchorBody(anchor.name, root, orbitParams);
        if (!body)
            return false;
        anchor.body = body;
        anchor.ownsBody = true;
        return true;
    }
    ModularBody *parent = nullptr;
    if (anchor.params["parent"].empty()) {
        if (anchor.params["orbit_center_x"].empty() || anchor.params["orbit_center_y"].empty()
                || anchor.params["orbit_center_z"].empty()) {
            cLog::get()->write("Anchor '" + anchor.name + "': an orbit anchor needs either "
                "parent = <body> (orbit around that body) or the three orbit_center_x/y/z "
                "coordinates in AU (orbit around a fixed place). This anchor is NOT created. "
                "To fix: add one of the two.", LOG_TYPE::L_ERROR);
            return false;
        }
        const std::string centerName = anchor.name + " (orbit centre)";
        if (ModularBody::exists(centerName)) {
            cLog::get()->write("Anchor '" + anchor.name + "': the implicit orbit-centre body '"
                + centerName + "' name is already taken. This anchor is NOT created. To fix: "
                "rename the anchor.", LOG_TYPE::L_ERROR);
            return false;
        }
        stringHash_t centerParams;
        centerParams["coord_func"] = "still_orbit";
        centerParams["orbit_x"] = anchor.params["orbit_center_x"];
        centerParams["orbit_y"] = anchor.params["orbit_center_y"];
        centerParams["orbit_z"] = anchor.params["orbit_center_z"];
        parent = createAnchorBody(centerName, root, centerParams);
        if (!parent)
            return false;
    } else {
        parent = ModularBody::findBody(anchor.params["parent"]);
        if (!parent) {
            cLog::get()->write("Anchor '" + anchor.name + "': parent = '" + anchor.params["parent"]
                + "' is not a loaded body, so the orbit has nothing to orbit around. This anchor "
                "is NOT created. To fix: name a loaded body, or use orbit_center_x/y/z to orbit "
                "a fixed place instead.", LOG_TYPE::L_ERROR);
            return false;
        }
    }
    ModularBody *body = createAnchorBody(anchor.name, parent, anchor.params);
    if (!body) {
        // The implicit centre, if we just made one, has no anchor left to serve.
        if (anchor.params["parent"].empty())
            parent->remove(true);
        return false;
    }
    anchor.body = body;
    anchor.ownsBody = true;
    return true;
}

bool CameraAnchors::ensureBody(Anchor &anchor)
{
    const std::string &key = (anchor.kind == AnchorKind::ATTACHED)
        ? anchor.params["body_name"] : anchor.name;
    if (anchor.body && ModularBody::findBody(key) == static_cast<ModularBody *>(anchor.body))
        return true;
    anchor.body = nullptr;
    if (anchor.ownsBody) {
        cLog::get()->write("Anchor '" + anchor.name + "': its body is gone (system reload?) - "
            "rebuilding it from the anchor declaration.", LOG_TYPE::L_INFO);
        anchor.ownsBody = false;
    }
    return buildBody(anchor);
}

CameraAnchors::Anchor *CameraAnchors::find(const std::string &name)
{
    for (auto &a : anchors) {
        if (a.name == name)
            return &a;
    }
    return nullptr;
}

bool CameraAnchors::add(stringHash_t params)
{
    const std::string name = params["name"];
    if (name.empty()) {
        cLog::get()->write("Anchor declaration without a name is not usable (an anchor is "
            "reached by name). This anchor is NOT created. To fix: add name = <anchor name>.",
            LOG_TYPE::L_ERROR);
        return false;
    }
    if (find(name)) {
        // Same rule as the old registry (AnchorManager::addAnchor:177-192):
        // a name already taken is not silently redefined.
        cLog::get()->write("Anchor '" + name + "' already exists - the new declaration is "
            "ignored. To fix: drop it first (camera action drop name " + name + ") or use "
            "another name.", LOG_TYPE::L_WARNING);
        return false;
    }
    Anchor anchor;
    anchor.name = name;
    if (!parseKind(params, name, anchor.kind))
        return false;
    anchor.params = std::move(params);
    {
        const auto it = anchor.params.find("follow_rotation");
        if (it != anchor.params.end() && !it->second.empty()) {
            anchor.declaresFollowRotation = true;
            anchor.followRotation = Utility::isTrue(it->second);
        }
    }
    if (!buildBody(anchor))
        return false;
    anchors.push_back(std::move(anchor));
    return true;
}

bool CameraAnchors::remove(const std::string &name)
{
    for (auto it = anchors.begin(); it != anchors.end(); ++it) {
        if (it->name != name)
            continue;
        if (currentName == name) {
            // The old path's own rule (AnchorManager::removeAnchor:199-200):
            // the anchor the camera is standing on is not dropped under it.
            cLog::get()->write("Anchor '" + name + "' is the current anchor and was NOT "
                "dropped. To fix: switch to another anchor first.", LOG_TYPE::L_WARNING);
            return false;
        }
        if (it->ownsBody && it->body) {
            ModularBody *body = it->body;
            ModularBody *parent = body->getParent();
            it->body = nullptr;
            body->remove(true);
            // The implicit orbit centre exists only to carry this anchor.
            if (it->kind == AnchorKind::ON_ORBIT && it->params["parent"].empty()
                    && parent && !parent->ownsAnyChild())
                parent->remove(true);
        }
        anchors.erase(it);
        return true;
    }
    return false;
}

bool CameraAnchors::switchTo(const std::string &name, Camera &camera)
{
    if (moving) {
        // The old registry's own rule (AnchorManager::switchToAnchor:358): the
        // place under the camera is not swapped out mid-travel.
        cLog::get()->write("camera action switch '" + name + "': a scripted travel is in "
            "flight, and the anchor is NOT changed under it. To fix: wait for the travel to "
            "land (the move_to command's own duration) before switching.", LOG_TYPE::L_ERROR);
        return false;
    }
    Anchor *anchor = find(name);
    if (!anchor) {
        if (ModularBody *body = ModularBody::findBody(name)) {
            camera.warpToBody(body);
            currentName = name;
            return true;
        }
        return false;
    }
    if (!ensureBody(*anchor)) {
        cLog::get()->write("Anchor '" + name + "': the camera was NOT moved, because the "
            "anchor has no body to attach to (see the error above).", LOG_TYPE::L_WARNING);
        return false;
    }
    if (anchor->kind == AnchorKind::FIXED_POINT) {
        // D12: this switch ACTS on the reference frame, not just the position -
        // it says so, once, where it happens.
        cLog::get()->write("Anchor '" + name + "': fixed-point anchors are read in the "
            "Universe frame, so the camera's reference becomes this point at the root of the "
            "tree (its coordinates are AU from the universe centre). Attach to a body or an "
            "orbit anchor to navigate inside a system again.", LOG_TYPE::L_INFO);
    }
    camera.warpToBody(anchor->body);
    if (anchor->ownsBody) {
        camera.setBoundToSurface(false);
    } else if (anchor->declaresFollowRotation) {
        // The kind-(2) semantic: follow the body's spin, or hold the angle.
        camera.setBoundToSurface(anchor->followRotation);
    }
    currentName = name;
    return true;
}

bool CameraAnchors::setFollowRotation(const std::string &name, bool value, Camera &camera)
{
    Anchor *anchor = find(name);
    if (!anchor) {
        cLog::get()->write("camera action follow_rotation: '" + name + "' is not a declared "
            "anchor, so there is nothing to set. To fix: name an anchor declared in anchor.ini "
            "or created with camera action create.", LOG_TYPE::L_WARNING);
        return false;
    }
    anchor->declaresFollowRotation = true;
    anchor->followRotation = value;
    if (currentName == name && !anchor->ownsBody)
        camera.setBoundToSurface(value);
    return true;
}

//! An AU coordinate as a declaration value: round-trip-exact.
static std::string au(double v)
{
    std::ostringstream os;
    os << std::setprecision(17) << v;
    return os.str();
}

CameraAnchors::Anchor *CameraAnchors::currentPlace(const Camera &camera)
{
    Anchor *anchor = find(currentName);
    if (!anchor || !anchor->ownsBody || !anchor->body)
        return nullptr;
    if (camera.getReferenceBody() != static_cast<ModularBody *>(anchor->body))
        return nullptr; // the camera left this anchor by some other route
    return anchor;
}

static Vec3d positionAtDateReconverged(const ModularBody *body, double atJd, double frameJd)
{
    const Vec3d ret = body->getPositionAtDate(atJd);
    for (int i = 0; i < 5; ++i)
        (void)body->getPositionAtDate(frameJd);
    return ret;
}

bool CameraAnchors::installTravel(Anchor &place, const Vec3d &targetRoot, double travelDays, double jd)
{
    ModularBody *body = place.body;
    const Vec3d start = positionAtDateReconverged(body, jd, jd);
    Vec3d direction = targetRoot - start;
    const double distance = direction.length();
    if (distance > 0)
        direction.normalize();
    std::unique_ptr<Orbit> previous = body->setOrbit(nullptr);
    std::unique_ptr<Orbit> resume;
    if (place.kind == AnchorKind::ON_ORBIT) {
        // Un-nest: a second travel must not stack another wrapper on the first
        // one's own resume chain.
        if (auto *inFlight = dynamic_cast<TravelOrbit *>(previous.get()))
            resume = inFlight->takeResume();
        else
            resume = std::move(previous);
    }
    body->setOrbit(std::make_unique<TravelOrbit>(body->getParent(), start, direction, distance,
        jd, travelDays, std::move(resume)));
    body->useNow(); // publish this date's position under the new law
    if (place.kind == AnchorKind::FIXED_POINT) {
        const Vec3d landing = start + direction * distance;
        place.params["x"] = au(landing[0]);
        place.params["y"] = au(landing[1]);
        place.params["z"] = au(landing[2]);
    }
    travelStart = start;
    travelDirection = direction;
    travelDistance = distance;
    travelStartTime = jd;
    travelEndTime = jd + ((travelDays > 0) ? travelDays : 0);
    ++travelSeq;
    if (travelDays > 0) {
        moving = true;
        travelArrival = jd + travelDays;
    }
    return true;
}

void CameraAnchors::update(double jd)
{
    if (moving && jd >= travelArrival)
        moving = false;
}

bool CameraAnchors::placeCurrentAt(const Vec3d &posRoot, Camera &camera, double jd)
{
    Anchor *place = currentPlace(camera);
    if (!place) {
        // Old: "error can't move when on a body" (anchor_manager.cpp:493).
        cLog::get()->write("camera action move_to target point: the camera is on a BODY, and a "
            "body is not moved by a camera command. This command does nothing. To fix: "
            "'camera action transition_to target point' first (it turns the observer's current "
            "place into a movable point), then move_to.", LOG_TYPE::L_ERROR);
        return false;
    }
    return installTravel(*place, posRoot, 0, jd);
}

bool CameraAnchors::travelToPoint(const Vec3d &posRoot, double seconds, Camera &camera, double jd)
{
    if (moving) {
        cLog::get()->write("camera action move_to: a travel is already in flight and this one is "
            "IGNORED. To fix: wait for the running travel's duration to elapse before issuing "
            "the next one.", LOG_TYPE::L_ERROR);
        return false;
    }
    if (!currentPlace(camera)) {
        cLog::get()->write("camera action move_to target point: the camera is on a BODY, so there "
            "is no place to travel. This command does nothing. To fix: 'camera action "
            "transition_to target point' first, then move_to.", LOG_TYPE::L_ERROR);
        return false;
    }
    if (seconds < 0) {
        cLog::get()->write("camera action move_to: duration is negative, so there is no travel to "
            "run. This command does nothing. To fix: give a duration >= 0 (0 = arrive now).",
            LOG_TYPE::L_ERROR);
        return false;
    }
    if (seconds == 0)
        return placeCurrentAt(posRoot, camera, jd);
    // Seconds of SIMULATION time, as old's: the arrival is a DATE
    // (anchor_manager.cpp:433), so the travel runs at the show's time rate.
    return installTravel(*currentPlace(camera), posRoot, seconds / (24 * 60 * 60), jd);
}

bool CameraAnchors::travelToBody(const std::string &bodyName, double seconds, double altitudeKm,
                                 Camera &camera, double jd)
{
    if (moving) {
        cLog::get()->write("camera action move_to target body: a travel is already in flight and "
            "this one is IGNORED. To fix: wait for the running travel's duration to elapse.",
            LOG_TYPE::L_ERROR);
        return false;
    }
    Anchor *place = currentPlace(camera);
    if (!place) {
        cLog::get()->write("camera action move_to target body: the camera is on a BODY, so there "
            "is no place to travel. This command does nothing. To fix: 'camera action "
            "transition_to target point' first, then move_to.", LOG_TYPE::L_ERROR);
        return false;
    }
    if (seconds < 0) {
        cLog::get()->write("camera action move_to target body: duration is negative, so there is "
            "no travel to run. To fix: give a duration >= 0.", LOG_TYPE::L_ERROR);
        return false;
    }
    ModularBody *target = ModularBody::findBody(bodyName);
    if (!target) {
        cLog::get()->write("camera action move_to target body: '" + bodyName + "' is not a loaded "
            "body, so there is nothing to travel to. This command does nothing. To fix: name a "
            "body of the loaded system (its english name).", LOG_TYPE::L_ERROR);
        return false;
    }
    const double travelDays = seconds / (24 * 60 * 60);
    // Aim at where the body WILL BE when the travel lands (old :477).
    const Vec3d targetPos = positionAtDateReconverged(target, jd + travelDays, jd);
    const Vec3d start = positionAtDateReconverged(place->body, jd, jd);
    Vec3d direction = targetPos - start;
    const double gap = direction.length();
    if (gap > 0)
        direction.normalize();
    const double standoff = (altitudeKm < 0)
        ? target->getScaledRadius() * 5.0
        : target->getScaledRadius() + altitudeKm / AU;
    return installTravel(*place, start + direction * (gap - standoff), travelDays, jd);
}

bool CameraAnchors::transitionToPoint(const std::string &name, Camera &camera, double jd)
{
    Anchor *anchor = find(name);
    if (anchor) {
        if (anchor->kind != AnchorKind::FIXED_POINT || !anchor->ownsBody) {
            // Old: "transitionToPoint: not an AnchorPoint" (:544).
            cLog::get()->write("camera action transition_to target point: '" + name + "' is "
                "already declared as an anchor that is not a point, so it cannot be turned into "
                "the observer's place. This command does nothing. To fix: drop it (camera action "
                "drop name " + name + ") or let the command use its own name.", LOG_TYPE::L_ERROR);
            return false;
        }
    } else {
        stringHash_t params;
        params["name"] = name;
        params["type"] = "point";
        params["x"] = "0";
        params["y"] = "0";
        params["z"] = "0"; // placed on the observer immediately below
        if (!add(std::move(params)))
            return false;
        anchor = find(name);
    }
    if (!ensureBody(*anchor)) {
        cLog::get()->write("camera action transition_to target point: '" + name + "' has no body "
            "to place (see the error above), so the camera was NOT moved.", LOG_TYPE::L_WARNING);
        return false;
    }
    if (!installTravel(*anchor, camera.getRootPosition(), 0, jd))
        return false;
    if (!switchTo(name, camera))
        return false;
    camera.placeAt(Vec3f(0, 0, 0), false);
    return true;
}

bool CameraAnchors::transitionToBody(const std::string &name, Camera &camera)
{
    Anchor *anchor = find(name);
    if (anchor && anchor->kind != AnchorKind::ATTACHED) {
        // Old: "transitionToPoint: not a body" (:685, the message's own typo).
        cLog::get()->write("camera action transition_to target body: '" + name + "' is a declared "
            "anchor that is not a body, so the camera cannot land on it. This command does "
            "nothing. To fix: name a body, or switch to the anchor with 'camera action switch'.",
            LOG_TYPE::L_ERROR);
        return false;
    }
    ModularBody *body = nullptr;
    if (anchor) {
        if (!ensureBody(*anchor))
            return false;
        body = anchor->body;
    } else {
        body = ModularBody::findBody(name);
    }
    if (!body) {
        cLog::get()->write("camera action transition_to target body: '" + name + "' is not a "
            "loaded body. This command does nothing. To fix: name a body of the loaded system.",
            LOG_TYPE::L_ERROR);
        return false;
    }
    body->useNow(); // the compensation below reads its position
    const Vec3f place = camera.positionRelativeTo(body);
    if (!switchTo(name, camera))
        return false;
    camera.placeAt(place, true);
    return true;
}

void CameraAnchors::dumpState(std::ostream &out) const
{
    const Anchor *cur = nullptr;
    for (auto &a : anchors) {
        if (a.name == currentName) {
            cur = &a;
            break;
        }
    }
    out << "{\"current\":\"" << currentName << "\",\"kind\":\"";
    if (cur) {
        switch (cur->kind) {
            case AnchorKind::ON_ORBIT: out << "orbit"; break;
            case AnchorKind::ATTACHED: out << "body"; break;
            case AnchorKind::FIXED_POINT: out << "point"; break;
        }
    }
    out << "\",\"declaresFollowRotation\":" << ((cur && cur->declaresFollowRotation) ? "true" : "false")
        << ",\"followRotation\":" << ((cur && cur->followRotation) ? "true" : "false")
        << ",\"body\":\"" << ((cur && cur->body) ? cur->body->getEnglishName() : "")
        << "\",\"moving\":" << (moving ? "true" : "false")
        << ",\"travelArrival\":" << travelArrival
        << ",\"count\":" << anchors.size() << ",\"declared\":[";
    bool first = true;
    for (auto &a : anchors) {
        if (!first)
            out << ',';
        first = false;
        out << '"' << a.name << '"';
    }
    out << "]}";
}

void CameraAnchors::load(const std::string &path)
{
    std::ifstream file(path);
    if (!file) {
        // Not an error: an install without authored anchors is legitimate (the
        // runtime channel still works). Same severity as the old loader's miss.
        cLog::get()->write("CameraAnchors: no anchor file '" + path + "' to read", LOG_TYPE::L_WARNING);
        return;
    }
    cLog::get()->write("CameraAnchors: reading anchor file " + path, LOG_TYPE::L_INFO);
    stringHash_t params;
    std::string line, key, value;
    unsigned int loaded = 0;
    while (getline(file, line)) {
        switch (IniLine::read(line, key, value)) {
            case IniLine::Kind::SECTION:
                if (!params.empty()) {
                    loaded += add(std::move(params));
                    params.clear();
                }
                break;
            case IniLine::Kind::ENTRY:
                params[key] = value;
                break;
            case IniLine::Kind::MALFORMED:
                cLog::get()->write("CameraAnchors: ignoring line without '=' in " + path
                    + ": '" + key + "' - a key/value line needs 'key = value'; write '#' "
                    "first to make it a comment.", LOG_TYPE::L_WARNING);
                break;
            case IniLine::Kind::EMPTY:
                break;
        }
    }
    if (!params.empty())
        loaded += add(std::move(params));
    cLog::get()->write("CameraAnchors: " + std::to_string(loaded) + " anchor(s) declared from "
        + path, LOG_TYPE::L_INFO);
}
