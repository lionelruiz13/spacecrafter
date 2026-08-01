#include "CameraAnchors.hpp"
#include "Camera.hpp"
#include "ModularSystem.hpp"
#include "ModuleLoaderMgr.hpp"
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT §5.39/D29)
#include "tools/log.hpp"
#include <fstream>
#include <ostream>

CameraAnchors::~CameraAnchors() = default;

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
    // §2(f): what fired, the valid states, and the fix action. No fallback: an
    // unnamed kind has no defensible default (an `orbit` needs orbit elements,
    // a `point` needs coordinates), so the anchor is refused rather than
    // silently made into something the author did not ask for.
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
        // Default rotation elements (period 1 day, no tilt, J2000 epoch): an
        // anchor point has no surface, and switchTo() drops the surface bind on
        // it, so the spin phase is never consumed. Kept at the type's default
        // rather than zeroed because a 0 period is a division by zero in
        // computeAxisRotation - a finite value that nothing reads is the safe
        // expression of "this body has no rotation semantics".
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = 0,
        // Altitude is measured FROM the point (datum 0) and a point is not
        // landable (ground 0 - no free-descent floor). Explicit, not the
        // sentinel: an anchor is not "a body whose radius happens to be 0".
        .datumRadius = 0,
        .groundRadius = 0,
        .oblateness = 0,
        .solLocalDay = 0,
        // BodyType::ANCHOR is the enum's own value for exactly this ("Simplest
        // type, just an anchor", ModularBody.hpp:44). It has no behavioural
        // consumer (verified §11.109-era; re-verified 2026-07-25), so it acts
        // only as the dump's is-this-an-anchor observable.
        .bodyType = BodyType::ANCHOR,
        .isHaloEnabled = false,
    };
    ModularBody *body = parent->createChild(info, BodyRelation::ORBITING);
    // Hidden: parent-owned, position-ticking, outside every draw/pick walk.
    body->hide();
    // Hidden BY CONSTRUCTION, not by an operator: an anchor's body is never
    // shown. Re-take its baseline so the override ledger does not read the
    // engine's own decision as somebody's change (INTENT §11.129).
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
        // Read in the ROOT (Universe) frame - the frame in which a fixed point
        // is fixed (R3). still_orbit is the engine's own "stay at this offset
        // from the parent" provider: one position authority, no anchor-local
        // arithmetic (I2).
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
    // ON_ORBIT: the orbit is declared exactly as a body's is (coord_func +
    // orbit_* keys), so the anchor's own params ARE the orbit params - the same
    // keys the loaders read for ssystem.ini bodies.
    ModularBody *parent = nullptr;
    if (anchor.params["parent"].empty()) {
        // The parentless form of the old grammar: an orbit around an arbitrary
        // CENTER given in AU (anchor.ini's `orbit_autour_point`). Preserved, not
        // retired: the requirement it serves is "an orbit centred on a place
        // rather than on a body". Expressed with the machinery that already
        // exists - the centre is a FIXED_POINT anchor body and the orbiting
        // anchor is its child - so there is no second notion of orbit centre.
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
    // Liveness by the NAME REGISTRY, never by "the pointer is not null": a
    // destroyed body's remnant ModularBodyPtrs are redirected to its PARENT
    // (ModularBodyPtr::redirect), so a system reload leaves this entry pointing
    // at a live body that is not the anchor.
    const std::string &key = (anchor.kind == AnchorKind::ATTACHED)
        ? anchor.params["body_name"] : anchor.name;
    if (anchor.body && ModularBody::findBody(key) == static_cast<ModularBody *>(anchor.body))
        return true;
    anchor.body = nullptr;
    if (anchor.ownsBody) {
        // An owned body the tree destroyed (system reload) is rebuilt from the
        // declaration - the params map is the authority, exactly as the data
        // file is the authority for a reloaded system.
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
    Anchor *anchor = find(name);
    if (!anchor) {
        // Not a declared anchor: the old path auto-creates one anchor per body,
        // and the dual-path seam resolves anchor names against the body registry
        // - so a body name IS an implicit body anchor. No bind change: an
        // implicit anchor declares nothing about following the rotation.
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
        // A point has no surface to be bound to - the same rule Camera applies
        // to a system reference (Camera::warpToBody), asked of the KIND here
        // because only the anchor layer knows this body is a place.
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
        << "\",\"count\":" << anchors.size() << ",\"declared\":[";
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
    // The shipped grammar, through the ONE authority that owns it
    // (tools/ini_line.hpp - INTENT §5.39/D29): `[` starts a new block, `#`
    // comments, `key = value` with any amount of blank around the `=`. This
    // reader used to carry a copy of the legacy loader's substr arithmetic,
    // which is the defect class D29 unified away.
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
