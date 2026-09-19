#include "SessionFile.hpp"
#include <cstdio>
#include "Camera.hpp"
#include "ModularBody.hpp"
#include "ModularSystem.hpp"
#include "ModularSystemFormat.hpp"
#include "bodyModules/TrailModule.hpp"
#include "bodyModules/HintModule.hpp"
#include "bodyModules/OrbitModule.hpp"
#include "bodyModule/body_tesselation.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <vector>

namespace SessionFile {

namespace {

using ModularSystemFormat::Section;

std::string d2s(double v)
{
    std::ostringstream s;
    s << std::setprecision(17) << v;
    return s.str();
}

std::string f2s(float v)
{
    std::ostringstream s;
    s << std::setprecision(9) << v;
    return s.str();
}

bool resolve(const std::string &name, std::string &path, const char *verb)
{
    const std::string n = name.empty() ? DEFAULT_NAME : name;
    if (n.find('/') != std::string::npos || n.find('\\') != std::string::npos) {
        cLog::get()->write(std::string("Command 'session action ") + verb + "': '" + n +
            "' is a path, and this command takes a session NAME - a session is written in " +
            DIRECTORY + "/ and nowhere else. Nothing was " +
            (std::string(verb) == "save" ? "written" : "loaded") +
            ". (The system data files of this install are never written to by this command, "
            "by design.) To fix: pass a plain name, e.g. 'session action " + verb +
            " filename show1'.", LOG_TYPE::L_ERROR);
        return false;
    }
    path = std::string(DIRECTORY) + "/" + n +
           ((n.find('.') == std::string::npos) ? ".ini" : "");
    return true;
}

void useNow(const std::string &name)
{
    if (name.empty())
        return;
    if (ModularBody *b = ModularBody::findBodyOnce(name))
        b->useNow();
}

void collectSystems(std::vector<ModularSystem *> &out)
{
    ModularBody::forEach([&out](ModularBody &b) {
        if (!b.isSystem())
            return;
        ModularSystem *sys = static_cast<ModularSystem *>(&b);
        if (sys->hasSystemFile())
            out.push_back(sys);
    });
}

std::string v2s(const Vec3f &c)
{
    return d2s(c[0]) + "," + d2s(c[1]) + "," + d2s(c[2]);
}

std::string qualifiedPath(ModularBody *b)
{
    std::string chain = b->getEnglishName();
    std::string file;
    for (ModularBody *p = b->getParent(); p; p = p->getParent()) {
        if (p->isSystem() && file.empty()) {
            ModularSystem *sys = static_cast<ModularSystem *>(p);
            if (sys->hasSystemFile())
                file = sys->getSystemFilename();
        }
        chain = p->getEnglishName() + "/" + chain;
    }
    return (file.empty() ? std::string("<no file>") : file) + "::" + chain;
}

bool excludedFlag(const std::string &n)
{
    return n == "track_object"          // S2 row C5 - [selection] owns it
        || n == "lock_sky_position"     // S2 row B9  - [observer] owns it
        || n == "experimental_path"     // S2 row E6  - a dev gate, not show state
        || n == "experimental_shadows"; // S2 row E6
}

bool excludedValue(const std::string &n)
{
    return n == "home_planet"           // S2 row B1  - [observer] owns it
        || n == "zoom_offset"           // S2 row B10 - [observer] owns it
        || n == "heading"               // S2 row B6  - D28's mandated carve-out
        || n == "landscape_name"        // S2 row F1  - needs the pin, not the name
        || n == "time_zone"             // S2 row A6  - the config channel persists it
        || n == "date_display_format"   // S2 row A6
        || n == "time_display_format"   // S2 row A6
        || n == "startup_time_mode";    // S2 row A6
}

} // namespace

bool save(Host &host, CommandSurface *cmds, const std::string &name)
{
    std::string path;
    if (!resolve(name, path, "save"))
        return false;

    Camera *camera = Camera::instance;
    if (!camera) {
        cLog::get()->write("Command 'session action save': there is no camera yet, so there is "
            "no session to save. Nothing was written.", LOG_TYPE::L_ERROR);
        return false;
    }

    // Bring every body this session is about to name to the current date,
    // BEFORE anything is read off it.
    const std::string selected = host.getSelectedName();
    ModularBody *ref = camera->getReferenceBody();
    if (ref)
        ref->useNow();
    useNow(selected);
    if (ModularBody *tracked = camera->getTrackedBody())
        tracked->useNow();

    std::vector<Section> sections;

    Section header;
    header.setHeader("session");
    header.appendEntry("format", std::to_string(FORMAT_VERSION));
    sections.push_back(std::move(header));

    Section time;
    time.setHeader("time");
    time.appendEntry("jday", d2s(host.getJDay()));
    time.appendEntry("time_speed", d2s(host.getTimeSpeed()));
    time.appendEntry("paused", host.getTimePaused() ? "true" : "false");
    sections.push_back(std::move(time));

    Section observer;
    observer.setHeader("observer");
    camera->saveSession(observer);
    {
        double sx = 0, sy = 0, sz = 0;
        host.getSkyVision(sx, sy, sz);
        observer.appendEntry("sky_vision",
                             f2s(static_cast<float>(sx)) + "," +
                             f2s(static_cast<float>(sy)) + "," +
                             f2s(static_cast<float>(sz)));
    }
    sections.push_back(std::move(observer));

    Section selection;
    selection.setHeader("selection");
    selection.appendEntry("selected", selected);
    selection.appendEntry("track_object", host.getTracking() ? "true" : "false");
    if (selected.empty()) {
        // S2 row C4 / D34's own unanswered half, said where it is missing.
        selection.annotate("selected", "non-body-selection-unkeyed",
            "A selected STAR, nebula or DSO is not carried by this session. Its identity key "
            "(a catalogue identifier) is the one part of DECISIONS_PENDING D34 that was not "
            "answered, and inventing one here would field a key nobody chose. Only a BODY "
            "selection round-trips today.");
    }
    sections.push_back(std::move(selection));

    if (cmds) {
        int nFlags = 0, nValues = 0, nColors = 0;
        cmds->countNames(nFlags, nValues, nColors);

        Section flags;
        flags.setHeader("flags");
        int wroteFlags = 0;
        cmds->forEachFlag([&](const std::string &n, bool v) {
            if (excludedFlag(n))
                return;
            flags.appendEntry(n, v ? "true" : "false");
            ++wroteFlags;
        });
        flags.annotate("", "flags-not-carried", std::string("Of ") + std::to_string(nFlags) +
            " flags this build knows, this session carries " + std::to_string(wroteFlags) +
            ". `track_object` and `lock_sky_position` are carried by [selection] and "
            "[observer] instead - they are the same state, and one file must not hold two "
            "answers to one question. `experimental_path` and `experimental_shadows` are "
            "excluded by b31-design \xc2\xa7" "2 row E6: they are development gates that retire with "
            "the old render path, and a session is a show artefact.");
        sections.push_back(std::move(flags));

        Section values;
        values.setHeader("values");
        int wroteValues = 0;
        cmds->forEachValue([&](const std::string &n, const std::string &v) {
            if (excludedValue(n))
                return;
            values.appendEntry(n, v);
            ++wroteValues;
        });
        values.annotate("", "values-not-carried", std::string("Of ") + std::to_string(nValues) +
            " `set` values this build knows, this session carries " + std::to_string(wroteValues) +
            ". Left out on purpose: `home_planet` and `zoom_offset` belong to [observer]; "
            "`heading` is DECISIONS_PENDING D28's mandated carve-out (what it means across a "
            "reference change is unanswered, and a saved number would bake in an answer "
            "nobody gave); `landscape_name` needs the pin-vs-auto distinction its own row "
            "(\xc2\xa7" "2 F1) carries, and the name alone would silently turn a pin into a "
            "coincidence; the timezone and the date/time display formats are \xc2\xa7" "2 row A6 "
            "preferences the config channel already persists. Left out because they cannot "
            "be READ - there is no getter anywhere in the tree, so nothing here can be "
            "written honestly: moon_brightness, sun_brightness, milky_way_fader_duration, "
            "zodiacal_intensity, milky_way_texture, star_fader_duration, "
            "text_fading_duration, screen_fader (\xc2\xa7" "2 row H6 wants it), stall_radius_unit, "
            "datetime_display_position, datetime_display_number, init_fov - and `mode`, "
            "which writes nothing at all (\xc2\xa7" "2 row K3).");
        sections.push_back(std::move(values));

        Section colors;
        colors.setHeader("colors");
        int wroteColors = 0;
        cmds->forEachColor([&](const std::string &n, const Vec3f &c) {
            colors.appendEntry(n, d2s(c[0]) + "," + d2s(c[1]) + "," + d2s(c[2]));
            ++wroteColors;
        });
        colors.annotate("", "colors-not-carried", std::string("Of ") + std::to_string(nColors) +
            " `color` names this build knows, this session carries " + std::to_string(wroteColors) +
            ". The rest have no read half at any level - the on-dome text colour and the star "
            "colour table - so they are named here instead of being guessed. The PER-BODY "
            "colours are not these: they are \xc2\xa7" "2 rows D3/D4 and live in [body:*].");
        sections.push_back(std::move(colors));
    }

    {
        int ledgerBodies = 0;
        std::vector<Section> bodySections;
        ModularBody::forEach([&](ModularBody &b) {
            Section s;
            bool any = false;
            const ModularBody::AuthoredState &a = b.getAuthored();
            auto put = [&](const char *k, const std::string &v) {
                s.appendEntry(k, v);
                any = true;
            };
            if (b.isHiddenDeclared() != a.hidden)                       // D1
                put("hidden", b.isHiddenDeclared() ? "true" : "false");
            if (b.getScalingTarget() != 1.f)                            // D2
                put("scale", d2s(b.getScalingTarget()));
            Vec3f live, auth;
            for (auto ch : {BodyColorType::HALO, BodyColorType::LABEL,      // D3/D4
                            BodyColorType::ORBIT, BodyColorType::TRAIL}) {
                if (!b.getColor(ch, live) || !b.getAuthoredColor(ch, auth) || live == auth)
                    continue;
                put(ch == BodyColorType::HALO ? "halo_color"
                    : ch == BodyColorType::LABEL ? "label_color"
                    : ch == BodyColorType::ORBIT ? "orbit_color" : "trail_color", v2s(live));
            }
            if (b.getDatumRadiusRaw() != a.datumRadius)                 // D6, in km
                put("datum_radius", d2s(b.getDatumRadiusRaw() * AU));
            if (b.getGroundRadiusRaw() != a.groundRadius)               // D6
                put("ground_radius", d2s(b.getGroundRadiusRaw() * AU));
            if (b.getSkinUse())                                         // D7 (scalar half)
                put("skin_use", "true");
            if (b.getOrbitOverride() >= 0)                              // D8
                put("orbit", b.getOrbitOverride() ? "true" : "false");
            if (b.getTrailOverride() >= 0)                              // D8
                put("trail", b.getTrailOverride() ? "true" : "false");
            for (BodyModule *m : b.getTrailComponents()) {
                TrailModule *t = static_cast<TrailModule *>(m);
                const auto &pts = t->getPoints();
                if (pts.empty())
                    continue;
                std::string packed;
                packed.reserve(pts.size() * 48);
                for (const auto &p : pts) {
                    if (!packed.empty())
                        packed += ";";
                    packed += d2s(p.jd) + ":" + d2s(p.pos[0]) + "," + d2s(p.pos[1]) +
                              "," + d2s(p.pos[2]);
                }
                put("trail_points", packed);
                break;
            }
            if (!any)
                return;
            s.setHeader("body:" + b.getEnglishName());
            // Context for the miss report, never a key (D34).
            s.appendEntry("path", qualifiedPath(&b));
            bodySections.push_back(std::move(s));
            ++ledgerBodies;
        });
        for (Section &s : bodySections)
            sections.push_back(std::move(s));

        Section defs;
        defs.setHeader("body_defaults");
        defs.appendEntry("halo_color", v2s(ModularBody::getDefaultHaloColor()));
        defs.appendEntry("label_color", v2s(HintModule::defaultLabelColor));
        defs.appendEntry("orbit_color", v2s(OrbitModule::defaultColor));
        defs.appendEntry("trail_color", v2s(TrailModule::defaultColor));
        defs.annotate("", "d5-defaults-are-not-per-body",
            "These four are the colours a body gets when its declaration names none. "
            "Changing one recolours NO existing body - only the ones loaded afterwards - "
            "which is why they cannot be recovered from the [body:*] sections and have "
            "their own place here (b31-design \xc2\xa7" "2 row D5).");
        sections.push_back(std::move(defs));

        if (const auto &tes = ModularBody::getTesselation()) {
            Section ts;
            ts.setHeader("tesselation");
            ts.appendEntry("min_tes_level", std::to_string(tes->getMinTesLevel()));
            ts.appendEntry("max_tes_level", std::to_string(tes->getMaxTesLevel()));
            ts.appendEntry("planet_altimetry_level", std::to_string(tes->getPlanetAltimetryFactor()));
            ts.appendEntry("moon_altimetry_level", std::to_string(tes->getMoonAltimetryFactor()));
            ts.appendEntry("earth_altimetry_level", std::to_string(tes->getEarthAltimetryFactor()));
            sections.push_back(std::move(ts));
        }
        cLog::get()->write("Session save: the override ledger carries " +
            std::to_string(ledgerBodies) + " body/bodies", LOG_TYPE::L_INFO);
    }

    std::vector<ModularSystem *> systems;
    collectSystems(systems);
    for (ModularSystem *sys : systems) {
        Section s;
        s.setHeader("system:" + sys->getEnglishName());
        s.appendEntry("name", sys->getEnglishName());
        s.appendEntry("file", sys->getSystemFilename());
        s.appendEntry("reader", sys->isComposedFile() ? "composed" : "legacy");
        sections.push_back(std::move(s));
    }

    std::error_code ec;
    std::filesystem::create_directories(DIRECTORY, ec);
    if (ec) {
        cLog::get()->write(std::string("Command 'session action save': can't create the ") +
            DIRECTORY + " directory (" + ec.message() + ") - nothing was written.",
            LOG_TYPE::L_ERROR);
        return false;
    }

    const std::vector<std::string> banner = {
        " spacecrafter SESSION - machine-owned, disposable.",
        " Written by `session action save`; read by `session action load filename <name>`.",
        " Nothing here is data: this file REFERENCES the system files it needs (see the",
        " [system:*] sections) and never copies what is in them, so a correction that lands",
        " in the data underneath a session still reaches a session restored on top of it.",
        " Delete it and you lose a bookmark, nothing else.",
    };
    if (!ModularSystemFormat::write(path, sections, banner))
        return false;   // the writer said why, and left any previous file intact
    cLog::get()->write("Session saved to " + path + " (" + std::to_string(systems.size()) +
        " system(s) in the manifest)", LOG_TYPE::L_INFO);
    return true;
}

bool load(Host &host, CommandSurface *cmds, const std::string &name)
{
    std::string path;
    if (!resolve(name, path, "load"))
        return false;

    std::vector<Section> sections;
    if (!ModularSystemFormat::parse(path, sections)) {
        cLog::get()->write("Command 'session action load': there is no session file at " + path +
            ". Nothing changed. To fix: save one first with 'session action save filename " +
            (name.empty() ? std::string(DEFAULT_NAME) : name) + "'.", LOG_TYPE::L_ERROR);
        return false;
    }

    const Section *header = nullptr, *time = nullptr, *observer = nullptr, *selection = nullptr;
    std::vector<const Section *> systems;
    for (const Section &s : sections) {
        const std::string &h = s.getHeader();
        if (h == "session") header = &s;
        else if (h == "time") time = &s;
        else if (h == "observer") observer = &s;
        else if (h == "selection") selection = &s;
        else if (h.compare(0, 7, "system:") == 0) systems.push_back(&s);
    }

    if (!header || !header->find("format")) {
        cLog::get()->write("Command 'session action load': " + path + " carries no "
            "[session] format key, so it is not a session file this build wrote. Nothing "
            "changed.", LOG_TYPE::L_ERROR);
        return false;
    }
    const int version = Utility::strToInt(*header->find("format"), -1);
    if (version != FORMAT_VERSION) {
        cLog::get()->write("Command 'session action load': " + path + " is format version " +
            std::to_string(version) + " and this build reads version " +
            std::to_string(FORMAT_VERSION) + " only. Nothing changed - a session applied by "
            "halves is neither the saved scene nor the running one. To fix: save a fresh "
            "session with this build.", LOG_TYPE::L_ERROR);
        return false;
    }

    for (const Section *s : systems) {
        const std::string *n = s->find("name");
        const std::string *f = s->find("file");
        if (!n || !f)
            continue;
        ModularBody *b = ModularBody::findBodyOnce(*n);
        if (!b || !b->isSystem()) {
            cLog::get()->write("Session restore: this session was saved with the system '" + *n +
                "' loaded (from '" + *f + "') and it is not loaded now. The rest of the session "
                "was applied. To fix: load that system, then restore again.",
                LOG_TYPE::L_WARNING);
            continue;
        }
        const std::string live = static_cast<ModularSystem *>(b)->getSystemFilename();
        if (live != *f)
            cLog::get()->write("Session restore: the system '" + *n + "' is loaded from '" + live +
                "' but this session was saved with it loaded from '" + *f + "'. The session was "
                "applied to the file that is loaded. To fix: reload that system from the file the "
                "session names if the scene does not look right.", LOG_TYPE::L_WARNING);
    }

    if (time) {
        if (const std::string *v = time->find("jday"))
            host.setJDay(Utility::strToDouble(*v, host.getJDay()));
        if (const std::string *v = time->find("time_speed"))
            host.setTimeSpeed(Utility::strToDouble(*v, host.getTimeSpeed()));
        if (const std::string *v = time->find("paused"))
            host.setTimePaused(*v == "true" || *v == "1");
    }

    if (observer) {
        if (const std::string *ref = observer->find("reference")) {
            if (!ref->empty() && !host.warpToBody(*ref))
                cLog::get()->write("Session restore: the observer's reference body '" + *ref +
                    "' is not in this tree, so the observer stayed where it was and every "
                    "position in this session is relative to the wrong body. The rest was "
                    "applied. To fix: load the system that declares '" + *ref +
                    "' before restoring.", LOG_TYPE::L_WARNING);
        }
        const std::string *lonS = observer->find("longitude");
        const std::string *latS = observer->find("latitude");
        const std::string *altS = observer->find("altitude");
        if (lonS && latS && altS) {
            host.moveObserverTo(Utility::strToDouble(*latS, 0) * (180.0 / M_PI),
                                Utility::strToDouble(*lonS, 0) * (180.0 / M_PI),
                                Utility::strToDouble(*altS, 0));
        }
        if (const std::string *v = observer->find("fov"))
            host.setFov(Utility::strToDouble(*v, 0));
        if (const std::string *v = observer->find("view_offset")) {
            const std::string *a = observer->find("view_offset_armed");
            host.setViewOffset(Utility::strToDouble(*v, 0),
                               a && (*a == "true" || *a == "1"));
        }
        {
            const std::string *v = observer->find("sky_vision");
            double sx = 0, sy = 0, sz = 0;
            if (v && std::sscanf(v->c_str(), "%lf,%lf,%lf", &sx, &sy, &sz) == 3)
                host.setSkyVision(sx, sy, sz);
        }
        if (const std::string *v = observer->find("sky_locked"))
            host.setSkyLock(*v == "true" || *v == "1");
        if (Camera::instance)
            Camera::instance->restoreSession(*observer);
    }

    if (selection) {
        if (const std::string *sel = selection->find("selected")) {
            if (sel->empty())
                host.deselect();
            else if (!host.selectByName(*sel))
                cLog::get()->write("Session restore: the selected body '" + *sel + "' is not in "
                    "this tree, so nothing is selected. The rest of the session was applied. "
                    "To fix: load the system that declares it before restoring.",
                    LOG_TYPE::L_WARNING);
        }
        if (const std::string *t = selection->find("track_object"))
            host.setTracking(*t == "true" || *t == "1");
    }

    if (cmds) {
        int unknown = 0, applied = 0;
        for (const Section &s : sections) {
            const std::string &h = s.getHeader();
            const bool isFlags = (h == "flags"), isValues = (h == "values"),
                       isColors = (h == "colors");
            if (!isFlags && !isValues && !isColors)
                continue;
            for (const auto &e : s.params()) {
                bool ok = false;
                if (isFlags)
                    ok = cmds->applyFlagByName(e.first, e.second == "true" || e.second == "1");
                else if (isValues)
                    ok = cmds->applyValueByName(e.first, e.second);
                else
                    ok = cmds->applyColorByName(e.first, Utility::strToVec3f(e.second));
                if (ok)
                    ++applied;
                else if (++unknown <= 8)
                    cLog::get()->write("Session restore: this build has no " +
                        std::string(isFlags ? "flag" : isValues ? "`set` value" : "colour") +
                        " called '" + e.first + "', so that line was left alone. The rest of "
                        "the session was applied. (A session written by a different build "
                        "carries names this one may not have; the file is kept as it is.)",
                        LOG_TYPE::L_WARNING);
            }
        }
        if (unknown)
            cLog::get()->write("Session restore: " + std::to_string(applied) + " value(s) "
                "applied, " + std::to_string(unknown) + " name(s) unknown to this build.",
                LOG_TYPE::L_WARNING);
    }

    bool ledgerAnnotated = false;
    for (Section &s : sections) {
        const std::string &h = s.getHeader();
        if (h.compare(0, 5, "body:") != 0)
            continue;
        const std::string name = h.substr(5);
        ModularBody *b = ModularBody::findBodyOnce(name);
        if (!b) {
            const std::string *where = s.find("path");
            const std::string reason =
                "This session recorded an override for a body called '" + name +
                "', and no body of that name is in the tree now" +
                (where ? " (when it was saved it was " + *where + ")" : "") +
                ". The override was NOT applied and NOT applied to anything else - a "
                "same-named body from another system would be the wrong body. It is kept "
                "here so nothing is lost: if the data renamed it, rename this section to "
                "match; if the system it belongs to is not loaded, load it and restore "
                "again.";
            cLog::get()->write("Session restore: " + reason, LOG_TYPE::L_WARNING);
            s.annotate("", "override-key-unresolved", reason);
            ledgerAnnotated = true;
            continue;
        }
        if (const std::string *v = s.find("hidden")) {
            const bool want = (*v == "true" || *v == "1");
            if (want != b->isHiddenDeclared()) {
                if (want) b->hide(); else b->show();
            }
        }
        if (const std::string *v = s.find("scale"))
            b->setScaling(Utility::strToFloat(*v, 1.f));
        struct { const char *key; BodyColorType channel; } channels[] = {
            {"halo_color", BodyColorType::HALO}, {"label_color", BodyColorType::LABEL},
            {"orbit_color", BodyColorType::ORBIT}, {"trail_color", BodyColorType::TRAIL}};
        for (const auto &c : channels)
            if (const std::string *v = s.find(c.key))
                b->setColor(c.channel, Utility::strToVec3f(*v));
        if (const std::string *v = s.find("datum_radius"))
            b->setDatumRadius(Utility::strToFloat(*v, 0.f) / static_cast<float>(AU));
        if (const std::string *v = s.find("ground_radius"))
            b->setGroundRadius(Utility::strToFloat(*v, 0.f) / static_cast<float>(AU));
        if (const std::string *v = s.find("skin_use"))
            b->switchTexSkin(*v == "true" || *v == "1");
        if (const std::string *v = s.find("orbit"))
            b->setFlagOrbit(*v == "true" || *v == "1");
        if (const std::string *v = s.find("trail"))
            b->setFlagTrail(*v == "true" || *v == "1");
        if (const std::string *v = s.find("trail_points")) {
            std::vector<TrailModule::TrailPoint> pts;
            std::size_t i = 0;
            while (i < v->size()) {
                std::size_t end = v->find(';', i);
                if (end == std::string::npos)
                    end = v->size();
                const std::string one = v->substr(i, end - i);
                const std::size_t colon = one.find(':');
                if (colon != std::string::npos) {
                    TrailModule::TrailPoint p;
                    p.jd = Utility::strToDouble(one.substr(0, colon), 0);
                    p.pos = Utility::strToVec3f(one.substr(colon + 1));
                    pts.push_back(p);
                }
                i = end + 1;
            }
            for (BodyModule *m : b->getTrailComponents()) {
                static_cast<TrailModule *>(m)->restorePoints(std::move(pts));
                break;
            }
        }
    }
    if (ledgerAnnotated)
        ModularSystemFormat::write(path, sections, {});

    // The runtime colour DEFAULTS (S2 row D5) and tesselation (D9).
    for (const Section &s : sections) {
        if (s.getHeader() == "body_defaults") {
            if (const std::string *v = s.find("halo_color"))
                ModularBody::setDefaultHaloColor(Utility::strToVec3f(*v));
            if (const std::string *v = s.find("label_color"))
                HintModule::defaultLabelColor = Utility::strToVec3f(*v);
            if (const std::string *v = s.find("orbit_color"))
                OrbitModule::defaultColor = Utility::strToVec3f(*v);
            if (const std::string *v = s.find("trail_color"))
                TrailModule::defaultColor = Utility::strToVec3f(*v);
        } else if (s.getHeader() == "tesselation") {
            if (const auto &tes = ModularBody::getTesselation()) {
                if (const std::string *v = s.find("min_tes_level"))
                    tes->setMinTes(Utility::strToInt(*v, tes->getMinTesLevel()));
                if (const std::string *v = s.find("max_tes_level"))
                    tes->setMaxTes(Utility::strToInt(*v, tes->getMaxTesLevel()));
                if (const std::string *v = s.find("planet_altimetry_level"))
                    tes->setPlanetTes(Utility::strToInt(*v, tes->getPlanetAltimetryFactor()));
                if (const std::string *v = s.find("moon_altimetry_level"))
                    tes->setMoonTes(Utility::strToInt(*v, tes->getMoonAltimetryFactor()));
                if (const std::string *v = s.find("earth_altimetry_level"))
                    tes->setEarthTes(Utility::strToInt(*v, tes->getEarthAltimetryFactor()));
            }
        }
    }

    cLog::get()->write("Session restored from " + path, LOG_TYPE::L_INFO);
    return true;
}

} // namespace SessionFile
