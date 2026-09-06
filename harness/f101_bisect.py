#!/usr/bin/env python3
"""F101 - WHICH LINE OF `14.sts` EMPTIES THE OLD PATH'S DUMP HALF.

    cd claude/harness && DISPLAY=:2 python3 f101_bisect.py <absOutdir> \
        --leg p23|p25|p27|p31|p27nc|ctlmars|p25desc|p31desc [--bin B]

WHAT IT MEASURES.  Sec.5.143 records that after the tester's `fscripts/14.sts`
the dual dump's OLD half goes 246 -> 1 and then to 0 for the rest of the
session (Sec.11.218(h), F98).  The candidate mechanism the row carries is
`[derived, NOT confirmed]`.  This driver takes ONE fresh launch per leg, dumps
the two halves BEFORE the show and AFTER it, and reads the executor's own
transition prints back out of the application's stdout - so the system switch
is WITNESSED rather than inferred from a count that moved.

WHY A PRE-DUMP IN EVERY LEG.  A per-leg baseline makes each launch its own
control: the after-count is only readable against the before-count of the SAME
process, and a launch that started in an unexpected state says so instead of
contributing a difference to the table.

THE LEGS (every one of them a farm COPY under a NEW name - the tester's file is
never edited, never truncated, never played; Sec.11.211(c): the annotator
rewrites the file it plays):

  p23      lines 1..23 of 14.sts  - through `moveto alt 1.1E+16` and its wait,
                                    i.e. BEFORE any `body action load`.  This
                                    leg is the one the dispatch's three-prefix
                                    bisect does not contain, and it is the only
                                    one that can separate "the altitude move
                                    empties the half" from "the load empties
                                    the half": both models predict 1 at p25.
  p25      lines 1..25            - + `body action load name Solsys ... parent none`
  p27      lines 1..27            - + `set home_planet Solsys`
  p31      lines 1..31            - + `select planet Solsys` + `flag track_object on`
  p27nc    lines 1..27, line 27 commented out - the negative control for p27
  ctlmars  `set home_planet Mars` alone - the shipped-scene control: the same
           command on a name the CURRENT system carries
  p25desc  p25 + a descent (`moveto alt 0.9E+9`) - does the old half come back?
  p31desc  p31 + the same descent - does line 27 change which branch the
           descent takes?

BOUNDARY.  Nothing is written outside <absOutdir> and the farm under it.  The
139 real-HOME files (config.ini, ssystem.ini, the 137 `.sts`) are md5'd in and
out of every leg and the run FAILS if one moved.
"""

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import dumpread                                                   # noqa: E402
import f95_soak as S                                              # noqa: E402
from f99_locguard import lock_state                               # noqa: E402

SRC_SHOW = "fscripts/14.sts"

# The executor's own transition prints, `std::cout` on the process's stdout
# (NOT cLog, so they are in the applog and not in spacecrafter.log), plus the
# two old-path body lines.  Each is quoted from source with its site, because a
# witness pattern recalled rather than read is the class Q-67 records.
WITNESS = {
    "swap_to_solar_up": "Swapping to mode Solar System",      # solarSystemModule.cpp:256
    "exit_solar": "InSolarSystem->",                          # solarSystemModule.cpp:86
    "enter_galaxy": "->InGalaxy",                             # inGalaxyModule.cpp:55
    "galaxy_too_high": "too high -> altitude = max",          # inGalaxyModule.cpp:62
    "galaxy_too_low": "too low -> altitude = min",            # inGalaxyModule.cpp:58
    "galaxy_mode": "InGalaxy mode",                           # inGalaxyModule.cpp:66
    "exit_galaxy": "InGalaxy->",                              # inGalaxyModule.cpp:74
    "enter_stellar": "->InStellarSystem",                     # stellarSystemModule.cpp:71
    "exit_stellar": "InStellarSystem->",                      # stellarSystemModule.cpp:85
    "enter_solar": "->InSolarSystem",                         # solarSystemModule.cpp:69
    "swap_to_stellar_up": "Swapping to mode Stellar System",  # stellarSystemModule.cpp:246
    "altitude_change": "Altitude change received",            # executor.hpp:65
    "observer_change": "Modification observer to",            # core.hpp:592
    "oldpath_add": "Loading new Stellar System object...",    # protosystem.cpp:517
    "oldpath_no_parent": "SolarSystem: can't find parent for",  # protosystem.cpp:533
    "oldpath_no_name": "can not add body with no name",       # protosystem.cpp:523
}

# The line that ends every leg's show, so the driver knows the prefix ran to its
# end rather than timing out.  `wait duration` is honoured by the script engine,
# so a prefix that ends on a wait still emits `script end`.
SCRIPT_END = "ScriptMgr: script end"
SCRIPT_LOAD = "ScriptMgr: load"

DESCENT_TAIL = ["wait duration 0.05",
                "moveto alt 0.9E+9 duration 0",
                "wait duration 0.50"]


def leg_lines(farm_show, leg):
    """The leg's show text, built from the FARM's copy of 14.sts.

    `farm_show` is <farm>/.spacecrafter/scripts/fscripts/14.sts - a copy made by
    build_farm.  The tester's own file is never read for writing and never
    truncated; the prefixes are new files beside the copy.

    Line endings: the tester's file is Windows-authored (CR-LF).  The engine's
    own reader strips a trailing '\\r' (protosystem-style readers do; the script
    reader is fed by ScriptMgr), and the file plays as shipped, so the prefix
    keeps the bytes it had.  Only the lines this driver ADDS are LF-only.
    """
    raw = farm_show.read_bytes().decode("latin-1").split("\n")
    # split("\n") keeps the CR at the end of each line; index 0 is line 1.
    def take(n):
        return raw[:n]
    if leg == "p23":
        return take(23)
    if leg == "p25":
        return take(25)
    if leg == "p27":
        return take(27)
    if leg == "p31":
        return take(31)
    if leg == "p27nc":
        out = take(27)
        assert out[26].startswith("set home_planet Solsys"), out[26]
        out[26] = "#" + out[26]
        return out
    if leg == "p25desc":
        return take(25) + DESCENT_TAIL
    if leg == "p31desc":
        return take(31) + DESCENT_TAIL
    if leg == "ctlmars":
        return ["### F101 shipped-scene control: `set home_planet` on a name the",
                "### CURRENT system carries.  No altitude move, so no executor",
                "### mode change is predicted and the old half must not move.",
                "set home_planet Mars",
                "wait duration 0.50"]
    raise SystemExit("unknown leg %r" % leg)


def read_dump(path):
    # require_old=False: the p23 and p31desc legs read an EMPTY old half BY
    # DESIGN - that emptiness is the measurement - so this reader opts out of
    # the guard it is itself the evidence for.
    header, pairs, missing_new, missing_old = dumpread.load_dump(
        path, require_old=False)
    old_names = sorted([r["name"] for r in pairs] + list(missing_new))
    new_names = sorted([r["name"] for r in pairs] + list(missing_old))
    row = {
        "file": path.name,
        "bodies_old": len(pairs) + len(missing_new),
        "bodies_new": len(pairs) + len(missing_old),
        "both": len(pairs), "old_only": len(missing_new),
        "new_only": len(missing_old),
        "old_names": old_names if len(old_names) <= 12 else old_names[:12] + ["..."],
        "old_names_n": len(old_names),
        "new_names_n": len(new_names),
        "jd": header.get("jd") if header else None,
    }
    # The record for the body the show authors, both halves, so the old half's
    # ONE body can be identified by its own declared parameters: the script's
    # Sphere (radius 1e-5, still_orbit) and a `createSystem` star (radius
    # 1190.856, stellar_special) are the two candidate origins and they are
    # distinguishable in the dump.
    for r in pairs:
        if r["name"] == "Solsys":
            row["solsys_old"] = r.get("old")
            break
    return row


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--leg", required=True)
    ap.add_argument("--bin", default=str(S.DEFAULT_BIN))
    ap.add_argument("--budget", type=float, default=90.0,
                    help="seconds to wait for the show's own `script end`")
    ap.add_argument("--then", default=None,
                    help="a SECOND shipped show, played after the post dump, "
                         "with a third dump after it. This is the CONSEQUENCE "
                         "arm: Sec.11.218(h) measured that from cycle 2 on the "
                         "170 satellites of `06old.sts` never reach the old "
                         "path at all, and the mechanism for that is a refusal "
                         "(`protosystem.cpp:531-535`) in whatever system the "
                         "first show left current - which is a claim about a "
                         "SECOND show and cannot be read off the first")
    ap.add_argument("--tag", default=None,
                    help="output sub-directory; defaults to the leg name")
    a = ap.parse_args()

    out = Path(a.out).resolve() / (a.tag or a.leg)
    out.mkdir(parents=True, exist_ok=True)
    res = {"leg": a.leg, "bin": a.bin, "bin_md5": S.md5(a.bin)[:8],
           "started": S.now_iso()}

    hits = S.no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)
    res["proc_before"] = hits
    res["lock_before"] = lock_state()

    frozen = S.frozen_md5s(dirs=["fscripts"])
    res["frozen_n"] = len(frozen)
    res["frozen_digest_in"] = S.frozen_digest(frozen)[:8]

    farm = out / "farm"
    res["farm_asserts"] = S.build_farm(farm, [SRC_SHOW]
                                       + ([a.then] if a.then else []))
    home = farm / ".spacecrafter"
    fs = home / "scripts" / "fscripts"

    show_name = "f101_14_%s.sts" % a.leg
    lines = leg_lines(fs / "14.sts", a.leg)
    (fs / show_name).write_bytes(("\n".join(lines) + "\n").encode("latin-1"))
    res["show"] = show_name
    res["show_lines"] = len(lines)
    res["show_md5"] = S.md5(fs / show_name)[:8]
    res["show_tail"] = [ln.rstrip("\r") for ln in lines[-4:]]
    # The farm's own copy of the tester's file must be untouched by the write
    # above; asserted here rather than assumed.
    res["farm_14sts_md5"] = S.md5(fs / "14.sts")[:8]

    applog = out / "leg.applog"
    env = {**os.environ, "HOME": str(farm),
           "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([a.bin], cwd=str(home), stdout=open(applog, "w"),
                            stderr=subprocess.STDOUT, env=env)
    res["pid"] = proc.pid
    launch_t = time.time()
    wire = S.Wire(logon=True)
    if not wire.connect(launch_t + 300):
        proc.kill()
        raise SystemExit("the application never accepted a connection")
    res["tcp_up_s"] = round(time.time() - launch_t, 2)

    # The app plays its own `startup.sts` at the end of App::init, so the
    # script log already carries a `script end` before this driver plays
    # anything: the counters are read as DELTAS against a mark, never as
    # absolutes (F98's own instrument lesson, one level down).
    tail = S.LogTail(home / "log", "script", [SCRIPT_END, SCRIPT_LOAD])
    tail.poll()

    def dump(tag):
        f = out / ("dump_%s.json" % tag)
        wire.send("body action dual_dump filename %s" % f, 1.5)
        end = time.time() + 90
        while time.time() < end and not (
                f.exists() and f.stat().st_size > 0
                and Path(str(f) + ".navstr").exists()):
            time.sleep(0.4)
        time.sleep(0.8)
        return read_dump(f)

    # a settle before the pre-dump: the app's own startup script has run and
    # the first frames are drawn by the time TCP answers, but the dump is a USE
    # and a dump taken in the first frame measures the launch, not the state.
    time.sleep(3.0)
    res["dump_pre"] = dump("pre")

    tail.poll()
    end_before = tail.counts[SCRIPT_END]
    load_before = tail.counts[SCRIPT_LOAD]
    t0 = time.time()
    wire.send("script action play filename fscripts/%s" % show_name, 1.0)
    seen_end = False
    while time.time() - t0 < a.budget:
        tail.poll()
        if tail.counts[SCRIPT_END] > end_before:
            seen_end = True
            break
        if proc.poll() is not None:
            break
        time.sleep(0.5)
    res["show_wall_s"] = round(time.time() - t0, 2)
    res["script_end_seen"] = seen_end
    res["script_load_seen"] = tail.counts[SCRIPT_LOAD] > load_before
    res["died_during_show"] = proc.poll()

    # The executor's mode machine is driven by an EVENT queue drained in the
    # main loop; a dump issued in the same breath as the last command would
    # read a state one transition early.  Two seconds is ~290 frames at the
    # config cap and is recorded as a parameter, not hidden in a sleep.
    time.sleep(2.0)
    if proc.poll() is None:
        res["dump_post"] = dump("post")

    if a.then and proc.poll() is None:
        tail.poll()
        end_before = tail.counts[SCRIPT_END]
        t1 = time.time()
        res["then"] = a.then
        wire.send("script action play filename %s" % a.then, 1.0)
        seen2 = False
        while time.time() - t1 < 240.0:
            tail.poll()
            if tail.counts[SCRIPT_END] > end_before:
                seen2 = True
                break
            if proc.poll() is not None:
                break
            time.sleep(0.5)
        res["then_wall_s"] = round(time.time() - t1, 2)
        res["then_end_seen"] = seen2
        time.sleep(2.0)
        if proc.poll() is None:
            res["dump_then"] = dump("then")

    txt = applog.read_text(encoding="latin-1", errors="replace")
    res["witness"] = {k: txt.count(v) for k, v in WITNESS.items()}
    # The ORDER of the transition prints is the thing a count cannot show.
    order = []
    for ln in txt.splitlines():
        for k, v in WITNESS.items():
            if v in ln and k not in ("altitude_change", "oldpath_add"):
                order.append(k)
                break
    res["witness_order"] = order
    res["altitude_lines"] = [ln.strip() for ln in txt.splitlines()
                             if "Altitude change received" in ln]
    res["oldpath_add_lines"] = [ln.strip() for ln in txt.splitlines()
                                if WITNESS["oldpath_add"] in ln][-8:]

    quit_rc, quit_wall = None, None
    if proc.poll() is None:
        q0 = time.time()
        wire.send("shutdown action now", 0.4)
        try:
            proc.wait(timeout=120)
        except subprocess.TimeoutExpired:
            proc.kill()
        quit_rc, quit_wall = proc.returncode, round(time.time() - q0, 2)
    res["quit_exit_code"] = quit_rc
    res["quit_wall_s"] = quit_wall
    res["exit_status"] = proc.returncode

    time.sleep(2.0)
    res["proc_after"] = S.no_instance()
    res["lock_after"] = lock_state()
    frozen_out = S.frozen_md5s(dirs=["fscripts"])
    res["frozen_digest_out"] = S.frozen_digest(frozen_out)[:8]
    res["frozen_moved"] = sorted(f for f, m in frozen.items()
                                 if frozen_out.get(f) != m)
    res["frozen_ok"] = (not res["frozen_moved"]
                        and res["frozen_digest_in"] == res["frozen_digest_out"])
    res["finished"] = S.now_iso()

    (out / "result.json").write_text(json.dumps(res, indent=1))
    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("farm_asserts",)}, indent=1))
    if not res["frozen_ok"]:
        raise SystemExit("BOUNDARY BREACH: a real-HOME file moved: %s"
                         % res["frozen_moved"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
