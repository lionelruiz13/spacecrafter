#!/usr/bin/env python3
"""F118 -- THE TRAIL WALKER'S TRANSIENT, CAUGHT ON THE FRAME AFTER AN UNHIDE.
INTENT Sec.11.239(h) / Sec.5.150's rider.

    cd claude/harness && DISPLAY=:2 python3 f118_edge.py <absOutdir> \
        --tag <name> --bin <path> [--jd J] [--days N] [--days2 N2] \
        [--kwaits 1,2,3]

`--days` is the main jump (357.99 by default: the fractional part is the
STALENESS the walker leaves, and the integer part puts Ceres -- the one body of
the reachable corpus whose residual is first-order visible -- at its own maximum
sensitivity).  `--days2` adds a second, shorter jump so the three Asteroid-class
bodies, whose retention is 60 samples and not 1460, reconstruct instead of
resetting (`missed > maxTrail`, TrailModule.cpp:193-199).

THE PROBLEM THIS DRIVER SOLVES, and it is a LATENCY problem, not a rate one.
`TrailModule::resumeAfterHidden` fires on ONE EDGE (the unhide), and what it
leaves behind is a STALE SEED, not a wrong position: the body's next evaluation
gets ITERATIVE_STEPS_PER_CALL = 2 steps from it, the frame after that 2 more,
and at a pinned clock the residual contracts by about e^2 per frame.  So the
transient is gone within about three frames, and the ordinary tcp dump helper
needs 100-200 frames to answer (Sec.11.239(c): 167-168 frames measured between
two `App.dump()` calls).  THE TCP CHANNEL IS BLIND TO THIS BY CONSTRUCTION.

THE CHANNEL THAT IS NOT.  `ScriptMgr::update` (script_mgr.cpp:294-339) runs
script commands in a `while (wait_time == 0)` loop until one returns a non-zero
wait, and `commandWait` (app_command_interface.cpp:1562) turns
`wait duration 0.001` into 1 ms -- less than one frame at 144 fps, so the next
frame's `wait_time -= delta_time` clamps to 0 and runs the NEXT command.  One
command per frame, deterministically.  Each arm is therefore a generated .sts
script whose commands are separated by `wait duration 0.001`, and the number of
those separators between the unhide and the dump IS the frame latency k.

k IS MEASURED, NEVER ASSUMED.  `evalCount` (ModularBody.hpp:709) counts position
evaluations; the walk makes exactly one per frame for a shown body and NONE for
a hidden one, so for a body hidden across the jump

    k = evalCount(post-unhide dump) - evalCount(dump taken while hidden)

is exactly the number of walk evaluations the dumped position carries.  A body
that is never hidden (the `--frameref`, Jupiter by default) gives the frame
count over the same interval independently.

THE ARMS, each a full cycle from JD0 so that no arm inherits another's state:
  on_kN     trails ON,  hide -> jump N days -> unhide -> N wait-frames -> dump
  off_kN    trails OFF, the same commands (resumeAfterHidden returns at !want
            before its reconstruction loop, TrailModule.cpp:185) -- the control
            that isolates the WALK and nothing else
  off_kN_b  the same arm again: the A/A floor of the arm-to-arm statistic
  jump_kN   no hide at all, just `date jday` -- the mandate's "within-launch
            control", which is NOT a floor (useNow returns on its first line for
            a body the walk evaluates, Sec.11.239(l)2)
  tcp_kN    the same edge driven entirely through the tcp helper: the measured
            blindness of the channel F111 used
plus the two-date control (F111's shape: the comparison shown able to fail).

THE PRIMARY OBSERVABLE is |ecl(on arm) - ecl(off arm)| at the same k and the
same date: both are engine-dumped vectors in the same frame, so
`rotate_to_vsop87` cancels (a rotation preserves |a-b|) and no model frame is
needed.  The model-scored radial residual of F111 is reported beside it.

BOUNDARY.  Nothing is written outside <absOutdir> and its farm; the real
~/.spacecrafter is md5'd in and out and never opened for writing.
"""
import argparse
import hashlib
import json
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import App, build_farm, cam_fields, no_instance, JD  # noqa: E402
from f99_locguard import lock_state                                 # noqa: E402
import f105_dump                                                    # noqa: E402
import f107_model as M                                              # noqa: E402
import f118_predict as Q                                            # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
# THE CORPUS, CORRECTED BY LEG 1 AND BY THE ENGINE'S OWN DUMP.  Sec.11.239(h)'s
# slice priced the walker on 40 SATELLITES; `deduceBodyModuleList`
# (ModularBody.cpp:940-946) gives a TRAIL module only to a body that is NOT a
# satellite, so not one of those 40 has the module (measured: `"trail":[]` on
# every one of them, leg edge1).  The bodies below are the 18 that DO carry one,
# minus the seven planets whose orbit has no iterative seed at all, plus two of
# those kept as nulls (Mars shown, Pluto shipped-hidden).
BODIES = ["Ceres", "Arrokoth", "Eris",                      # ell_orbit
          "Juno", "Pallas", "Vesta", "Haumea", "Makemake", "Sedna",  # comet
          "Pluto", "Mars"]                                  # SpecialOrbit nulls
FRAMEREF = "Jupiter"


def md5_home():
    return {n: hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
            for n in ("config.ini", "ssystem.ini")}


def elements_for(names):
    secs = M.by_name_sections(M.DEFAULT_INI)
    out = {}
    for n in names:
        sec = secs.get(n)
        if sec:
            el = M.elements(sec)
            if el:
                out[n] = el
    return out


def gen_script(path, dumps, tag, bodies, jd0, jd1, trail_on, do_hide, nwait,
               settle=1.2):
    """One complete cycle as a .sts file.  Every `wait duration 0.001` is
    exactly one frame (ScriptMgr::update's `while (wait_time == 0)` loop +
    commandWait's 1 ms floor), so `nwait` IS the intended frame latency and the
    dumped `evalCount` delta is its measurement.

    THE EVENT is the last command before the nwait separators: the unhide for a
    hide arm, the `date jday` for the no-hide control.  `mid` is therefore the
    dump taken one frame BEFORE the event in both shapes.
    """
    L = ["# F118 arm %s - generated by f118_edge.py, not authored by hand" % tag,
         "timerate rate 0",
         "date jday %.9f" % jd0,
         "wait duration %.3f" % settle]
    # TEN of the eighteen trail-carrying bodies ship DECLARED HIDDEN
    # (`hidden = true` in ssystem.ini -> ModularSystem.cpp:1400 `body->hide()`),
    # so they are parked, the trail sweep never reaches them and nothing is
    # recorded.  Every arm starts by showing them: this first unhide cannot fire
    # the walker (recording is false and points is empty, TrailModule.cpp:185).
    L += ["body name %s hidden false" % b for b in bodies]
    L += ["wait duration %.3f" % settle,
         "flag object_trails off",
         "wait duration %.3f" % settle,
         "flag object_trails %s" % ("on" if trail_on else "off"),
         "wait duration %.3f" % settle,
         "body action dual_dump filename %s/%s_pre.json" % (dumps, tag),
         "wait duration 0.001"]
    if do_hide:
        # hide, then jump while hidden, then the dump that fixes the reference
        L += ["body name %s hidden true" % b for b in bodies]
        L += ["wait duration 0.001",
              "date jday %.9f" % jd1,
              "wait duration %.3f" % settle,
              "body action dual_dump filename %s/%s_mid.json" % (dumps, tag),
              "wait duration 0.001"]
        # every unhide in ONE frame: no separator between them
        L += ["body name %s hidden false" % b for b in bodies]
    else:
        # no hide at all: the reference dump first, then the jump IS the event
        L += ["body action dual_dump filename %s/%s_mid.json" % (dumps, tag),
              "wait duration 0.001",
              "date jday %.9f" % jd1]
    for _ in range(nwait):
        L.append("wait duration 0.001")
    L += ["body action dual_dump filename %s/%s_post.json" % (dumps, tag),
          "wait duration 0.5",
          "body action dual_dump filename %s/%s_settled.json" % (dumps, tag),
          "wait duration 0.2"]
    Path(path).write_text("\n".join(L) + "\n")
    return L


def drain(app, seconds=0.3):
    """Read whatever the engine replied.  A script arm sends ~30 commands
    without the helper's own recv, and an unread socket is a main-loop hazard."""
    import socket as _s
    try:
        app.sock.settimeout(seconds)
        while True:
            if not app.sock.recv(65536):
                break
    except _s.timeout:
        pass
    finally:
        app.sock.settimeout(None)


def run_script(app, script, marker, timeout=180.0):
    """Play a generated script and wait for its LAST dump to land."""
    marker = Path(marker)
    if marker.exists():
        marker.unlink()
    t0 = time.time()
    app.sock.sendall(("script action play filename %s\n" % script).encode())
    last = -1
    while time.time() - t0 < timeout:
        if marker.exists():
            sz = marker.stat().st_size
            if sz > 0 and sz == last:
                time.sleep(1.0)
                drain(app)
                return round(time.time() - t0, 2)
            last = sz
        time.sleep(0.2)
    drain(app)
    raise RuntimeError("script %s never produced %s" % (script, marker))


def read(path):
    h, b = f105_dump.parse(Path(path))
    return h, b


def rec(b, name):
    r = (b.get(name) or {}).get("new") or {}
    return r


def trail_of(r):
    t = r.get("trail") or []
    return t[0] if t else None


def arm_stats(dumps, tag, els, frameref):
    """Everything this arm measured, from its own three dumps."""
    out = {"tag": tag}
    for phase in ("pre", "mid", "post", "settled"):
        p = Path(dumps) / ("%s_%s.json" % (tag, phase))
        if not p.is_file():
            out[phase] = None
            continue
        h, b = read(p)
        d = {"jd": h.get("jd"), "timeSpeed": h.get("timeSpeed"),
             "frameref_eval": rec(b, frameref).get("evalCount"), "bodies": {}}
        for n in els:
            r = rec(b, n)
            if not r:
                continue
            t = trail_of(r)
            d["bodies"][n] = {
                "ecl": r.get("ecl"), "lastJD": r.get("lastJD"),
                "evalCount": r.get("evalCount"), "dist": r.get("dist"),
                "relation": r.get("relation"),
                "trail_points": (t or {}).get("points"),
                "trail_headJD": (t or {}).get("headJD"),
                "trail_recording": (t or {}).get("recording"),
                "trail_fader": (t or {}).get("fader"),
                "trail_accumulateCount": (t or {}).get("accumulateCount"),
            }
        out[phase] = d
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--jd", type=float, default=JD)
    ap.add_argument("--days", type=float, default=357.99)
    ap.add_argument("--days2", type=float, default=30.99)
    ap.add_argument("--kwaits", default="1,2,3")
    ap.add_argument("--frameref", default=FRAMEREF)
    ap.add_argument("--settle", type=float, default=1.2)
    a = ap.parse_args()

    out = Path(a.out).resolve() / a.tag
    dumps = out / "dumps"
    scripts = out / "scripts"
    for d in (out, dumps, scripts):
        d.mkdir(parents=True, exist_ok=True)
    jd0, jd1 = a.jd, a.jd + a.days
    els = elements_for(BODIES)
    kwaits = [int(x) for x in a.kwaits.split(",")]

    res = {"tag": a.tag, "bin": a.bin, "jd0": jd0, "jd1": jd1, "days": a.days,
           "kwaits": kwaits, "bodies": BODIES, "frameref": a.frameref,
           "days2": a.days2,
           "bin_md5": hashlib.md5(open(a.bin, "rb").read()).hexdigest()[:8],
           "started": time.strftime("%Y-%m-%dT%H:%M:%S"),
           "elements_found": sorted(els)}
    hits = no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)
    res["proc_before"], res["lock_before"] = hits, lock_state()
    res["md5_in"] = md5_home()

    farm = out / "farm"
    build_farm(farm)
    log = f105_dump.Log(farm / ".spacecrafter")
    app = App(a.bin, farm, out, a.tag)
    try:
        res["tcp_seconds"] = app.start()
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("meteors zhr 0")
        app.send("date jday %.9f" % jd0, 1.5)
        app.settle_scale()

        # --- the tcp channel's own latency, measured (Sec.11.239(l)1's number
        # --- re-taken on THIS leg rather than quoted) -------------------------
        h0, b0, _ = app.dump("lat0", pause=0.6)
        h1, b1, _ = app.dump("lat1", pause=0.6)
        res["camera"] = cam_fields(h0)
        res["n_records"] = len(b0)
        res["tcp_latency_frames"] = {
            n: (rec(b1, n).get("evalCount", 0) - rec(b0, n).get("evalCount", 0))
            for n in [a.frameref] + BODIES}
        print("  tcp two-dump frame gap: %s" % res["tcp_latency_frames"],
              flush=True)

        # --- the two-date control that must fail (F111's shape) --------------
        app.send("date jday %.9f" % (jd0 + 1.0), 1.5)
        h2, b2, _ = app.dump("ctl_other_date", pause=0.6)
        moved = []
        for n in els:
            r0, r2 = rec(b0, n), rec(b2, n)
            if r0.get("ecl") and r2.get("ecl"):
                if M.norm(M.sub(r2["ecl"], r0["ecl"])) > 1e-9:
                    moved.append(n)
        res["ctl_two_dates"] = {"jd0": h0.get("jd"), "jd1": h2.get("jd"),
                                "moved": moved, "of": len(els)}
        print("  control two dates: %d of %d target records moved"
              % (len(moved), len(els)), flush=True)
        app.send("date jday %.9f" % jd0, 1.5)

        # --- the scripted arms ----------------------------------------------
        plan = ([("on_k%d" % k, True, True, k, a.days) for k in kwaits]
                + [("off_k%d" % kwaits[0], False, True, kwaits[0], a.days),
                   ("off_k%d_b" % kwaits[0], False, True, kwaits[0], a.days),
                   ("jump_k%d" % kwaits[0], True, False, kwaits[0], a.days)])
        if a.days2 > 0:
            plan += [("on2_k%d" % kwaits[0], True, True, kwaits[0], a.days2),
                     ("off2_k%d" % kwaits[0], False, True, kwaits[0], a.days2)]
        res["arms"] = {}
        res["plan"] = [list(p) for p in plan]
        for tag, trail_on, do_hide, nwait, days in plan:
            sc = scripts / ("%s.sts" % tag)
            lines = gen_script(sc, dumps, tag, BODIES, jd0, jd0 + days, trail_on,
                               do_hide, nwait, a.settle)
            secs = run_script(app, sc, dumps / ("%s_settled.json" % tag))
            st = arm_stats(dumps, tag, els, a.frameref)
            st["nwait"], st["trail_on"], st["hide"] = nwait, trail_on, do_hide
            st["days"], st["jd1"] = days, jd0 + days
            st["script_seconds"], st["script_lines"] = secs, len(lines)
            res["arms"][tag] = st
            mid, post = st.get("mid"), st.get("post")
            ks = {}
            if mid and post:
                for n in els:
                    m_, p_ = mid["bodies"].get(n), post["bodies"].get(n)
                    if m_ and p_ and m_["evalCount"] is not None:
                        ks[n] = p_["evalCount"] - m_["evalCount"]
                st["k_by_evalcount"] = ks
                st["frames_mid_to_post"] = (post["frameref_eval"]
                                            - mid["frameref_eval"])
            print("  arm %-12s N=%-8.2f nwait=%d trail=%-5s hide=%-5s %5.1fs  k=%s frames=%s"
                  % (tag, days, nwait, trail_on, do_hide, secs,
                     ks.get("Ceres"), st.get("frames_mid_to_post")), flush=True)

        # --- the same edge over TCP, to measure the blind channel -------------
        app.send("date jday %.9f" % jd0, 1.5)
        app.send("flag object_trails off", 1.2)
        app.send("flag object_trails on", 1.5)
        ht0, bt0, _ = app.dump("tcp_pre", pause=0.6)
        for n in BODIES:
            app.send("body name %s hidden true" % n, 0.2)
        app.send("date jday %.9f" % jd1, 1.5)
        ht1, bt1, _ = app.dump("tcp_mid", pause=0.6)
        for n in BODIES:
            app.send("body name %s hidden false" % n, 0.2)
        ht2, bt2, _ = app.dump("tcp_post", pause=0.6)
        res["tcp_arm"] = {
            "k_by_evalcount": {n: (rec(bt2, n).get("evalCount", 0)
                                   - rec(bt1, n).get("evalCount", 0))
                               for n in els},
            "jd": [ht0.get("jd"), ht1.get("jd"), ht2.get("jd")],
            "bodies": {n: {"ecl": rec(bt2, n).get("ecl"),
                           "lastJD": rec(bt2, n).get("lastJD"),
                           "dist": rec(bt2, n).get("dist"),
                           "trail": trail_of(rec(bt2, n))} for n in els}}
        print("  tcp arm k = %s" % res["tcp_arm"]["k_by_evalcount"], flush=True)
        for n in BODIES:
            app.send("body name %s hidden false" % n, 0.2)
        res["log_lines"] = log.lines()
    finally:
        res["exit_code"] = app.stop()
        res["lock_after"] = lock_state()
        res["md5_out"] = md5_home()
        res["proc_after"] = no_instance()
        (out / "f118_edge.json").write_text(json.dumps(res, indent=1))

    if res["md5_in"] != res["md5_out"]:
        raise SystemExit("BOUNDARY BREACH: the real ~/.spacecrafter moved: %s -> %s"
                         % (res["md5_in"], res["md5_out"]))
    print("wrote %s" % (out / "f118_edge.json"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
