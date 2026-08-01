#include "SessionFile.hpp"
#include "Camera.hpp"
#include "ModularBody.hpp"
#include "ModularSystem.hpp"
#include "ModularSystemFormat.hpp"
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

// The file a name denotes, and the refusals that stand between a name and a
// path. Both live HERE because this is the level that owns where sessions go -
// the same argument that put `body action save`'s refusals at the SSystemFactory
// seam (§11.121(e)).
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

// The D8 use-site barrier at this save's own use sites (§6.1). A body the
// engine froze (hidden - and a hidden body is a legal observer reference,
// §11.113(b)(vi)) holds its position at its freeze date until something asks;
// a save that named it without asking would describe a scene at a date that has
// passed, and the restore would make that permanent. `useNow` is a no-op for a
// body the walks still evaluate, so this costs nothing in the ordinary case.
// SCOPE, stated rather than implied: this slice names three bodies (the
// reference, the tracked one and the selection). The obligation §6.1 calls
// "the first consumer that touches EVERY body" arrives with the per-body
// override ledger, which walks the whole tree.
void useNow(const std::string &name)
{
    if (name.empty())
        return;
    if (ModularBody *b = ModularBody::findBodyOnce(name))
        b->useNow();
}

// Every system in this tree that came from a file. `ModularBody::forEach` walks
// the global registry, which is the one place that knows every body there is -
// asking the render lists instead would miss a hidden one, and a hidden system
// is still loaded content (§11.121(b)'s lesson, from the other side).
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

} // namespace

bool save(Host &host, const std::string &name)
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
    sections.push_back(std::move(observer));

    Section selection;
    selection.setHeader("selection");
    selection.appendEntry("selected", selected);
    selection.appendEntry("track_object", host.getTracking() ? "true" : "false");
    if (selected.empty()) {
        // §2 row C4 / D34's own unanswered half, said where it is missing.
        selection.annotate("selected", "non-body-selection-unkeyed",
            "A selected STAR, nebula or DSO is not carried by this session. Its identity key "
            "(a catalogue identifier) is the one part of DECISIONS_PENDING D34 that was not "
            "answered, and inventing one here would field a key nobody chose. Only a BODY "
            "selection round-trips today.");
    }
    sections.push_back(std::move(selection));

    // THE MANIFEST (§3.3): what this session assumed was loaded. It is what
    // makes the file readable on another install - D32 turned the artifact into
    // a diagnostic one, and a diagnostic that does not say what it assumed is a
    // guess.
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

    // The banner carries NO date and NO run identity, and that is a
    // requirement rather than an omission: a save of an unchanged state must be
    // byte-identical to the one before it (b31-design §6.2 T4), and a
    // timestamp would give the file no fixed point at all.
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

bool load(Host &host, const std::string &name)
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

    // §9(1): a file from a version this build does not know is REFUSED whole.
    // Half-applying a future file is the worst failure available here - the
    // operator gets a scene that is neither the saved one nor the running one.
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

    // The manifest is CHECKED, not enacted. Loading a system this session names
    // and this app does not have is the in-session load/unload half of §3.3 and
    // it needs more than the manifest carries (where the system node sits, and
    // the galactic load surface, which is §5.37's own suspended question). So a
    // divergence is REPORTED and the rest of the restore proceeds - the
    // operator is told which file the session expected.
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

    // ORDER IS THE CONTRACT, and it was MEASURED rather than assumed.
    // TIME FIRST: warping to a body computes a compensation from the two
    // bodies' positions AT THE CURRENT DATE, so a warp performed before the
    // date is restored lands on a view derived from whatever date the app
    // happened to be showing. (Measured as a restored scene whose whole view
    // was rolled, because the warp's recoverParams ran at the launch date.)
    if (time) {
        if (const std::string *v = time->find("jday"))
            host.setJDay(Utility::strToDouble(*v, host.getJDay()));
        if (const std::string *v = time->find("time_speed"))
            host.setTimeSpeed(Utility::strToDouble(*v, host.getTimeSpeed()));
        if (const std::string *v = time->find("paused"))
            host.setTimePaused(*v == "true" || *v == "1");
    }

    // THEN the reference body, because the warp recomputes the camera's own
    // parameters to hold the view across the change (recoverParams) - so it
    // must not run after the values that describe the restored view have been
    // assigned. Everything after it is an assignment, which is what makes a
    // second load land in the same place.
    if (observer) {
        if (const std::string *ref = observer->find("reference")) {
            if (!ref->empty() && !host.warpToBody(*ref))
                cLog::get()->write("Session restore: the observer's reference body '" + *ref +
                    "' is not in this tree, so the observer stayed where it was and every "
                    "position in this session is relative to the wrong body. The rest was "
                    "applied. To fix: load the system that declares '" + *ref +
                    "' before restoring.", LOG_TYPE::L_WARNING);
        }
        // THE PLACE, through the dual seam, BEFORE the camera's own members:
        // it moves the old Observer too, and the old Observer is what the star
        // field, the milky way and the nebulae are still drawn from. Restoring
        // the camera alone put the right body in front of the wrong sky
        // (measured: every camera field identical, 111001 px>8 of dome still
        // differing against an in-scene A/A floor of 0).
        const std::string *lonS = observer->find("longitude");
        const std::string *latS = observer->find("latitude");
        const std::string *altS = observer->find("altitude");
        if (lonS && latS && altS) {
            host.moveObserverTo(Utility::strToDouble(*latS, 0) * (180.0 / M_PI),
                                Utility::strToDouble(*lonS, 0) * (180.0 / M_PI),
                                Utility::strToDouble(*altS, 0));
        }
        // The other two DUAL values, before the camera's own members. The sky
        // lock is set FIRST because engaging it CAPTURES the current view -
        // restoreSession then overwrites that capture with the matrix the
        // session actually held, which is the value §2 row B9 calls state.
        if (const std::string *v = observer->find("fov"))
            host.setFov(Utility::strToDouble(*v, 0));
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

    cLog::get()->write("Session restored from " + path, LOG_TYPE::L_INFO);
    return true;
}

} // namespace SessionFile
