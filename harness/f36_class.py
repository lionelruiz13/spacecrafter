#!/usr/bin/env python3
"""F36 / INTENT §5.77 — classify the enumerated console-report sites, and size the row.

§5.77's owed item is an enumeration of *"the other startup paths that report only
on stderr"*, so that the fix can be judged at the class rather than one line at a
time. `f36_enum.py` produces the sites; `f36_reach.py` measures which are on the
startup path; this file carries the CLASSIFICATION and prints the sizing.

The verdicts are judgments, so each one is written down with the observation that
supports it rather than left implicit in a number. Three axes:

  klass   FAILURE  — reports a condition the app could not carry out
          CHATTER  — progress / state / parameter trace; nothing failed
          DEVTOOL  — an explicitly-invoked dump helper (`print()`, `display_all()`)
          NOTASITE — not an output statement at all (a macro definition, a stream
                     manipulator, or cLog's own console writer)

  logged  NO       — nothing about this condition reaches `spacecrafter.log`
          YES      — a `cLog` call reports the SAME condition at the same place
          PARTIAL  — the log records the ATTEMPT and never its outcome. This is
                     §5.77's exact shape and the reason the row matters: the log
                     does not merely omit, it implies success.
          NA       — not applicable (NOTASITE / DEVTOOL)

  blocker why this site does NOT admit the one uniform additive `cLog` routing
          the task was authorised to apply. Empty = no blocker.
          PRELOG    executes before `main.cpp:209` opens the log; `cLog::write`
                    would call `logFile.at(INTERNAL)` on an empty map and throw
          FRAGMENT  the message is a sentence fragment with no newline, composed
                    on the console with its neighbours; alone it is not a
                    diagnostic, so routing it needs new WORDING (a decision)
          DUPCONSOLE the site's own channel is stderr AND `cLog` at L_ERROR
                    already writes stderr whenever `print_log` is set (the
                    installed default) — adding the call prints the SAME line
                    TWICE on a non-log observable
          CONSOLEADD the site writes stdout / SDL_Log, so a `cLog` call is not a
                    verbatim duplicate — but under `print_log` it still ADDS a
                    console line, so the routing is not purely additive on the
                    console for this site either
          ALREADY   a `cLog` call for the same condition is already there
          PLATFORM  compiled out on this platform (`#else` of `#ifdef __linux__`)
          SIGNAL    async-signal handler; `cLog::write` takes a mutex
          SELF      the report IS the logging subsystem failing
          VENDORED  third-party file; editing it is a decision of its own

Usage:  ./f36_class.py [--sites artifacts/f36/f36_sites.json]
                       [--reach artifacts/f36/f36_result.json]
                       [-o artifacts/f36/f36_class.json]
"""
import argparse
import json
import sys
from pathlib import Path

# --------------------------------------------------------------------------
# Per-FUNCTION defaults. Key: (file, func). Value: (klass, logged, blocker, note)
GROUPS = {
 ("src/appModule/save_screen_interface.cpp", "SaveScreenInterface::write_png_image"):
    ("FAILURE", "NO", "", "screenshot writer; 10 printf on the PNG failure paths"),
 ("src/bodyModule/body_artificial.cpp", "Artificial::Artificial"):
    ("FAILURE", "NO", "", "model load failed; the body is then zero-radius so it is not drawn"),
 ("src/bodyModule/planet_grid.cpp", "PlanetGrid::computeGridVertices"):
    ("FAILURE", "NO", "", "meridian vertex index overflow"),
 ("src/bodyModule/protosystem.cpp", "ProtoSystem::addBody"):
    ("FAILURE", "YES", "ALREADY", "cLog L_ERROR on the next line (protosystem.cpp:616)"),
 ("src/bodyModule/ring.cpp", "Ring::createAsteroidRing"):
    ("CHATTER", "NO", "", "ring density, ANSI-coloured"),
 ("src/bodyModule/solarsystem_display.cpp", "SolarSystemDisplay::computePreDraw"):
    ("CHATTER", "NO", "", "'X is now the Center of Interest'"),
 ("src/bodyModule/ssystem_factory.cpp", "SSystemFactory::loadSystem"):
    ("CHATTER", "NO", "", "raw parameter dump of every stellar system loaded"),
 ("src/coreModule/core.cpp", "Core::getSelectedObjectInfoColor"):
    ("FAILURE", "NO", "DUPCONSOLE", "defensive; returns white. Callable per frame"),
 ("src/coreModule/core.cpp", "Core::init"):
    ("FAILURE", "NO", "DUPCONSOLE", "unknown viewing_mode; assert(0) follows and is a NO-OP under NDEBUG"),
 ("src/coreModule/core.cpp", "Core::selectObject"):
    ("FAILURE", "NO", "DUPCONSOLE", "SAME string to cerr AND cout (F35 handoff); command path, not startup"),
 ("src/coreModule/core.cpp", "Core::setLandscapeToBody"):
    ("CHATTER", "NO", "", "'Body : X Landscape : Y'"),
 ("src/coreModule/core.cpp", "Core::setSkyCultureDir"):
    ("FAILURE", "PARTIAL", "DUPCONSOLE", "THE §5.77 SITE; the log says 'Check sky_cultures subdirectory ok'"),
 ("src/coreModule/core.hpp", "?"):
    ("CHATTER", "NO", "", "Core::onObserverChange, 'Modification observer to X'"),
 ("src/coreModule/coreLink.hpp", "?"):
    ("DEVTOOL", "NA", "", "CoreLink::observerDisplayPos, an explicit dump"),
 ("src/coreModule/time_mgr.cpp", "TimeMgr::dateSunMeridian"):
    ("NOTASITE", "NA", "", "std::cout.precision(15): a manipulator, no output — but it mutates the GLOBAL cout precision"),
 ("src/eventModule/event_handler.cpp", "EventHandler::handle"):
    ("FAILURE", "NO", "DUPCONSOLE", "no handler registered for an event type"),
 ("src/executorModule/executor.hpp", "?"):
    ("CHATTER", "NO", "", "Executor::onAltitudeChange"),
 ("src/inGalaxyModule/starNavigator.cpp", "StarNavigator::loadCommonNames"):
    ("FAILURE", "PARTIAL", "DUPCONSOLE", "log carries 'Loading star names from X' and never the failure"),
 ("src/interfaceModule/app_command_eval.cpp", "AppCommandEval::define"):
    ("FAILURE", "NO", "", "script-time; the script channel LOG_FILE::SCRIPT exists and is unused here"),
 ("src/main.cpp", "usage"):
    ("CHATTER", "NA", "PRELOG", "--help text; the console IS the destination"),
 ("src/main.cpp", "check_command_line"):
    ("CHATTER", "NO", "PRELOG", "runs at main.cpp:188, before openLog at 209"),
 ("src/main.cpp", "main"):
    ("FAILURE", "NO", "CONSOLEADD", "SDL_Log: desktop display mode query failed, then exit(EXIT_FAILURE)"),
 ("src/mainModule/signals.cpp", "ISignals::NSSigTERM"):
    ("CHATTER", "NO", "SIGNAL", "signal handler; cLog::write locks a mutex"),
 ("src/mediaModule/subtitle.cpp", "Subtitle::writeToConsole"):
    ("DEVTOOL", "NA", "", "the console is this function's stated purpose"),
 ("src/mediaModule/video_player.cpp", "VideoPlayer::restartCurrentVideo"):
    ("FAILURE", "NO", "", "av_seek_frame failed"),
 ("src/mediaModule/video_player.cpp", "VideoPlayer::seekVideo"):
    ("FAILURE", "NO", "", "av_seek_frame failed"),
 ("src/navModule/anchor_manager.cpp", "AnchorManager::displayAnchor"):
    ("DEVTOOL", "NA", "", "an explicit dump"),
 ("src/ojmModule/LazyOjmL.cpp", "LazyOjmL::loadCache"):
    ("CHATTER", "NO", "", "'Statistics v<n> i<n>' per cached model"),
 ("src/ojmModule/ojm.cpp", "Ojm::print"):
    ("DEVTOOL", "NA", "", "an explicit dump"),
 ("src/ojmModule/ojm.cpp", "Ojm::readOJM"):
    ("FAILURE", "NO", "", "see per-line override for :387"),
 ("src/ojmModule/ojm.cpp", "Ojm::testIndices"):
    ("FAILURE", "NO", "", "vertex/normal/uv count mismatch; the model is rejected"),
 ("src/starModule/geodesic_grid.cpp", "GeodesicGrid::searchZone"):
    ("FAILURE", "NO", "", "'software error: not found' then exit(-1)"),
 ("src/starModule/hip_star.cpp", "Star1::print"): ("DEVTOOL", "NA", "", "dump helper"),
 ("src/starModule/hip_star.cpp", "Star2::print"): ("DEVTOOL", "NA", "", "dump helper"),
 ("src/starModule/hip_star.cpp", "Star3::print"): ("DEVTOOL", "NA", "", "dump helper"),
 ("src/starModule/hip_star_mgr.cpp", "HipStarMgr::HipStarMgr"):
    ("FAILURE", "NO", "DUPCONSOLE", "allocation failure, then exit(1)"),
 ("src/starModule/hip_star_mgr.cpp", "HipStarMgr::load_data"):
    ("FAILURE", "PARTIAL", "DUPCONSOLE", "log carries 'Loading catalog X' per catalogue and never an outcome"),
 ("src/starModule/hip_star_mgr.cpp", "HipStarMgr::loadCommonNames"):
    ("FAILURE", "PARTIAL", "DUPCONSOLE", "log carries 'Loading star names from X' first (hip_star_mgr.cpp:522)"),
 ("src/starModule/hip_star_mgr.cpp", "HipStarMgr::loadSciNames"):
    ("FAILURE", "PARTIAL", "DUPCONSOLE", "same shape; the function has no caller at all (§5.78)"),
 ("src/starModule/string_array.cpp", "StringArray::initFromFile"):
    ("FAILURE", "NO", "DUPCONSOLE", "OOM only: a MISSING FILE returns silently, reporting nowhere"),
 ("src/starModule/zone_array.cpp", "ZoneArray::create"):
    ("FAILURE", "PARTIAL", "FRAGMENT", "13 of 15 are newline-less fragments; log has the version line and the star count on SUCCESS only"),
 ("src/starModule/zone_array.cpp", "ZoneArray1::updateHipIndex"):
    ("FAILURE", "NO", "DUPCONSOLE", "invalid HIP number, then exit(1)"),
 ("src/starModule/zone_array.cpp", "SpecialZoneArray"):
    ("FAILURE", "NO", "DUPCONSOLE", "allocation / mmap failures, then exit(1)"),
 ("src/tiny_jpeg.h", "tje_encode_with_func"):
    ("NOTASITE", "NA", "VENDORED", "'#define tje_log(msg) puts(msg)' is a macro DEFINITION, and NDEBUG makes it empty in this build"),
 ("src/tools/app_settings.cpp", "AppSettings::display_all"):
    ("DEVTOOL", "NA", "", "an explicit dump of every configured path"),
 ("src/tools/call_system.cpp", "CallSystem::checkUserSubDirectory"):
    ("FAILURE", "NO", "PRELOG", "runs at main.cpp:199, before openLog at 209; its SUCCESS messages go to the `out` accumulator that main.cpp:223 logs"),
 ("src/tools/file_path.cpp", "FilePath::FilePath"):
    ("FAILURE", "NO", "CONSOLEADD", "'No X, Y or Z found' — the file-resolution failure of the whole app"),
 ("src/tools/init_parser.cpp", "InitParser::getStr"):
    ("CHATTER", "YES", "ALREADY", "printf is `if (DEBUG)` trace; the missing-key report is cLog L_WARNING"),
 ("src/tools/init_parser.cpp", "InitParser::getInt"):
    ("CHATTER", "YES", "ALREADY", "as getStr"),
 ("src/tools/init_parser.cpp", "InitParser::getDouble"):
    ("CHATTER", "YES", "ALREADY", "as getStr"),
 ("src/tools/init_parser.cpp", "InitParser::getBoolean"):
    ("CHATTER", "YES", "ALREADY", "as getStr"),
 ("src/tools/io.cpp", "ServerSocket::computeHttp"):
    ("CHATTER", "NO", "", "HTTP request trace, marked //Debug in the source"),
 ("src/tools/log.cpp", "cLog::openLog"):
    ("FAILURE", "NO", "SELF", "the log file itself could not be opened"),
 ("src/tools/log.cpp", "cLog::writeConsole"):
    ("NOTASITE", "NA", "SELF", "cLog's OWN console writer — the mechanism, not a report"),
 ("src/tools/translator.cpp", "Translator::getAvailableLanguagesCodes"):
    ("FAILURE", "NO", "DUPCONSOLE", "locale directory unreadable; returns {}"),
 ("src/tools/vecmath.hpp", "print"):
    ("DEVTOOL", "NA", "", "Matrix4<T>::print"),
 ("src/uiModule/ui_tui.cpp", "s_tui::TimeZoneitem::TimeZoneitem"):
    ("FAILURE", "YES", "ALREADY", "cLog L_ERROR on the next line (ui_tui.cpp:813), then exit(0)"),
}

# executorModule: mode transitions and altitude clamps, all state trace
for _f, _fn in [
    ("src/executorModule/inGalaxyModule.cpp", "InGalaxyModule::onEnter"),
    ("src/executorModule/inGalaxyModule.cpp", "InGalaxyModule::onExit"),
    ("src/executorModule/inSandBoxModule.cpp", "InSandBoxModule::onEnter"),
    ("src/executorModule/inSandBoxModule.cpp", "InSandBoxModule::onExit"),
    ("src/executorModule/inUniverseModule.cpp", "InUniverseModule::onEnter"),
    ("src/executorModule/inUniverseModule.cpp", "InUniverseModule::onExit"),
    ("src/executorModule/solarSystemModule.cpp", "SolarSystemModule::onEnter"),
    ("src/executorModule/solarSystemModule.cpp", "SolarSystemModule::onExit"),
    ("src/executorModule/solarSystemModule.cpp", "SolarSystemModule::testValidAltitude"),
    ("src/executorModule/stellarSystemModule.cpp", "StellarSystemModule::onEnter"),
    ("src/executorModule/stellarSystemModule.cpp", "StellarSystemModule::onExit"),
    ("src/executorModule/stellarSystemModule.cpp", "StellarSystemModule::testValidAltitude"),
]:
    GROUPS[(_f, _fn)] = ("CHATTER", "NO", "", "executor mode/altitude state trace")

# starManager: catalogue authoring and statistics tools, none on any launch path
for _fn in ["StarManager::saveStarBinCatalog", "StarManager::saveStarCatalog",
            "StarManager::HyperCubeStatistiques", "StarManager::MagStarStatistiques",
            "StarManager::verificationData", "StarManager::saveAsterismStarsPosition",
            "StarManager::loadOtherStar"]:
    GROUPS[("src/inGalaxyModule/starManager.cpp", _fn)] = (
        "DEVTOOL", "NA", "", "offline catalogue tool / statistics dump")

# --------------------------------------------------------------------------
# Per-LINE overrides, where sites inside one function do not share a verdict.
LINES = {
 ("src/ojmModule/ojm.cpp", 387):
    ("CHATTER", "NO", "", "'T ' << tmp — a leftover trace, not a failure"),
 ("src/starModule/zone_array.cpp", 134):
    ("CHATTER", "NO", "FRAGMENT", "'byteswap ' — a state note, not a failure"),
 ("src/starModule/zone_array.cpp", 505):
    ("FAILURE", "NO", "PLATFORM", "_get_osfhandle: the #else of #ifdef __linux__"),
 ("src/starModule/zone_array.cpp", 510):
    ("FAILURE", "NO", "PLATFORM", "CreateFileMapping: the #else of #ifdef __linux__"),
 ("src/starModule/zone_array.cpp", 518):
    ("FAILURE", "NO", "PLATFORM", "MapViewOfFile: the #else of #ifdef __linux__"),
 ("src/starModule/zone_array.cpp", 147):
    ("FAILURE", "NO", "PLATFORM", "mmap-without-gcc branch: the #if (!defined(__GNUC__))"),
 ("src/inGalaxyModule/starManager.cpp", 449):
    ("NOTASITE", "NA", "", "std::cout.precision(6): a manipulator, no output"),
 ("src/inGalaxyModule/starManager.cpp", 528):
    ("NOTASITE", "NA", "", "std::cout.precision(6): a manipulator, no output"),
}


def classify(site):
    key = (site["file"], site["line"])
    if key in LINES:
        return LINES[key]
    g = GROUPS.get((site["file"], site["func"]))
    if g is None:
        return ("UNCLASSIFIED", "?", "", "no verdict recorded")
    return g


def main():
    ap = argparse.ArgumentParser()
    here = Path(__file__).resolve().parent
    ap.add_argument("--sites", default=str(here / "artifacts/f36/f36_sites.json"))
    ap.add_argument("--reach", default=str(here / "artifacts/f36/f36_result.json"))
    ap.add_argument("-o", "--out", default=str(here / "artifacts/f36/f36_class.json"))
    a = ap.parse_args()

    sites = json.load(open(a.sites))
    reach = json.load(open(a.reach))
    startup, post = set(), set()
    for lbl in reach["startup_reached"]:
        startup.add(lbl.split()[0].rsplit(":", 1)[0])
    for lbl in reach["post_reached"]:
        post.add(lbl.split()[0].rsplit(":", 1)[0])
    # reachability is measured per FUNCTION, and the label carries file:line func
    reach_by_func = {}
    for phase, labels in (("STARTUP", reach["startup_reached"]),
                          ("POST", reach["post_reached"])):
        for lbl in labels:
            fileline, func = lbl.split(" ", 1)
            f = fileline.rsplit(":", 1)[0]
            reach_by_func[(f, func)] = phase
    unresolved = {(u[2].split(" ", 1)[0].rsplit(":", 1)[0],
                   u[2].split(" ", 1)[1] if " " in u[2] else "?")
                  for u in reach.get("unresolved", []) if len(u) > 2}

    rows = []
    for s in sites:
        if s["entitycore"]:
            k, lg, bl, note = ("ENTITYCORE", "NA", "", "read-only submodule")
        else:
            k, lg, bl, note = classify(s)
        key = (s["file"], s["func"])
        ph = reach_by_func.get(key, "not-reached")
        if key in unresolved and ph == "not-reached":
            ph = "unprobed"
        rows.append({**s, "klass": k, "logged": lg, "blocker": bl,
                     "note": note, "phase": ph})
    Path(a.out).write_text(json.dumps(rows, indent=1))

    proj = [r for r in rows if not r["entitycore"]]
    unc = [r for r in proj if r["klass"] == "UNCLASSIFIED"]

    def n(pred):
        return len([r for r in proj if pred(r)])

    print(f"sites: {len(rows)}  (project {len(proj)}, EntityCore {len(rows)-len(proj)})")
    print(f"UNCLASSIFIED: {len(unc)}")
    for r in unc:
        print("   ", r["file"], r["line"], r["func"])
    print("\n-- by class (project) --")
    for k in ("FAILURE", "CHATTER", "DEVTOOL", "NOTASITE"):
        print(f"  {k:10s} {n(lambda r, k=k: r['klass'] == k):4d}")
    print("\n-- FAILURE sites by startup phase --")
    for ph in ("STARTUP", "POST", "not-reached", "unprobed"):
        print(f"  {ph:12s} {n(lambda r, p=ph: r['klass']=='FAILURE' and r['phase']==p):4d}")
    print("\n-- FAILURE + STARTUP, by 'is it in the app log' --")
    for lg in ("NO", "PARTIAL", "YES"):
        print(f"  {lg:8s} {n(lambda r, l=lg: r['klass']=='FAILURE' and r['phase']=='STARTUP' and r['logged']==l):4d}")
    print("\n-- FAILURE + STARTUP + not-fully-logged, by fix blocker --")
    tgt = [r for r in proj if r["klass"] == "FAILURE" and r["phase"] == "STARTUP"
           and r["logged"] in ("NO", "PARTIAL")]
    byb = {}
    for r in tgt:
        byb.setdefault(r["blocker"] or "(none)", []).append(r)
    for b in sorted(byb, key=lambda b: -len(byb[b])):
        print(f"  {b:11s} {len(byb[b]):4d}")
    print(f"\nTHE ROW'S CLASS (startup failure reports the app log does not carry): {len(tgt)}")
    for r in sorted(tgt, key=lambda r: (r["file"], r["line"])):
        print(f"  {r['file']}:{r['line']:5d} [{r['chan']:14s}] {r['logged']:8s} "
              f"{r['blocker'] or '-':11s} {r['text'][:70]}")
    print(f"\n-> {a.out}")
    return 1 if unc else 0


if __name__ == "__main__":
    sys.exit(main())
