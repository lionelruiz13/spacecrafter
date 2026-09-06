#!/usr/bin/env python3
"""F100 -- IS THE FROZEN READOUT THE D8 BARRIER'S MEMO?  (INTENT 5.139 / 11.216(j1))

The measurement, in one sentence: take F96's camera move twice on the SAME
binary, once with the clock PINNED and once with it RUNNING, and ask which
bodies' eye-frame position did not move.  If the freeze is the update walk's
visibility gate, the running clock changes nothing -- the walk still never
reaches a parked body.  If it is `useNow()`'s memo `evaluatedJD == currentJD`
[ModularBody.cpp:459], the running clock defeats it every frame and the parked
set un-freezes on a binary that was not rebuilt.

Every partition is scored against `f100_partition.py`, which sorts the bodies
by `relation` + the `parent` chain and by `dist == 0` and never sees a second
dump -- so the key cannot have been fitted to the answer.

  stages
    freeze    P0 -> camera move -> A0, freshness + evalCount + old-vs-new alt/az
    operator  `select planet Ceres` / `get status object` before and after the
              same move, against the dump's own OLD-inf / NEW-inf blocks; then
              `flag track_object on` on the hidden dwarf and on Mars
    cost      evalCount deltas per FRAME (Mars's own evalCount is the frame
              counter, 11.117(f)) with the camera held and with it moving --
              the D11 number and the no-memo mutation's discriminator

  usage
    DISPALY=:2 ./f100_freeze.py <absOutdir> --tag pre|post|nomemo --bin PATH
                    [--clock pinned,running] [--stages freeze,operator,cost]
    ./f100_freeze.py <absOutdir> --score          # re-score on disk, no launch
"""
import argparse
import json
import math
import os
import re
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import (App, build_farm, parse_dump, cam_of, nav_of, cam_fields,  # noqa: E402
                        dir_from_altaz, angle_between, no_instance, freshness,
                        JD, DEG)
import f100_partition as part                                        # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
FARM_ROOT = Path(os.environ.get("F100_FARM", "/home/claude/sc-f100/farm"))
MOVE_TARGET = "Jupiter"       # F96/F81's own move: `select` + `flag track_object on`
DWARF = "Ceres"               # a hidden dwarf, `hidden = true` in the field ini
CONTROL = "Mars"              # a walked body: the control on every arm
COST_BODY = "Pluto"           # a parked SUBTREE (5 moons): the cost worst case
F91_FLOOR = 3e-5              # 11.213(h2): F91's own alt/az control floor, deg

FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def note(m):
    NOTES.append(m)
    print("NOTE: " + m, flush=True)


# ------------------------------------------------------------------ readouts
def nh(rec):
    return (rec or {}).get("new") or {}


def evalcounts(bodies):
    return {n: nh(r).get("evalCount") for n, r in bodies.items()}


def altaz_sep(rec):
    """OLD vs NEW alt/az for one record, in degrees, or None.  Both halves are
    dumped in OLD's reported convention (F96's README note), so the same
    direction builder serves both."""
    a, b = rec.get("altaz_old"), rec.get("altaz_new")
    if not a or not b:
        return None
    return angle_between(dir_from_altaz(*a), dir_from_altaz(*b))


def lock_state():
    """F99's precedent: a crashing or killed leg leaves /tmp/spacecrafter.lock.
    Record it per leg; never remove it while its pid is alive."""
    p = Path("/tmp/spacecrafter.lock")
    if not p.exists():
        return {"present": False}
    try:
        pid = int(p.read_text().split()[0])
    except Exception:                                            # noqa: BLE001
        return {"present": True, "pid": None, "alive": None}
    alive = Path("/proc/%d" % pid).exists()
    return {"present": True, "pid": pid, "alive": alive}


def md5_home():
    import hashlib
    out = {}
    for n in ("config.ini", "ssystem.ini"):
        out[n] = hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
    return out


# ------------------------------------------------------- the navstr sidecar
INF_END = re.compile(r"^Distance\s*:")
MARK = re.compile(r"^  (OLD|NEW) (nav|inf): (.*)$")


def navstr_blocks(path):
    """{body: {"OLD inf": "...", "NEW inf": "...", "OLD nav": ..., "NEW nav": ...}}

    The info block is MULTI-LINE and its continuation lines are unindented, so a
    parser keyed on indentation alone silently swallows the next body's name.
    The block's own terminator is its `Distance :` line (ModularObject.cpp /
    body.cpp both end there), which is what closes it here."""
    out, name, cur, mode, buf = {}, None, {}, None, []

    def flush():
        if mode and buf:
            cur[mode] = "\n".join(buf)

    for line in open(path, errors="replace"):
        line = line.rstrip("\n")
        m = MARK.match(line)
        if m:
            flush()
            mode, buf = "%s %s" % (m.group(1), m.group(2)), [m.group(3)]
            continue
        if mode and mode.endswith("inf"):
            buf.append(line)
            if INF_END.match(line):
                flush()
                mode, buf = None, []
            continue
        if mode and mode.endswith("nav"):
            buf.append(line)
            continue
        if line.strip():                      # an unindented line, no mode: a name
            if name:
                out[name] = cur
            name, cur = line.strip(), {}
    flush()
    if name:
        out[name] = cur
    return out


def normalise(s):
    return re.sub(r"\s+", " ", (s or "")).strip()


# ------------------------------------------------------------------- stages
def setup(app, rate):
    app.send("flag experimental_path on")
    app.send("timerate rate 0")
    app.send("meteors zhr 0")
    app.send("date jday %.9f" % JD, 1.5)
    if rate:
        app.send("timerate rate %d" % rate, 1.0)


def settle_aim(app, tag, tries=25, pause=0.8):
    """Bounded, and it RECORDS instead of failing: whether the aim converges is
    itself an observable of this task (a tracked frozen body cannot be centred,
    so its plan may never finish)."""
    for i in range(tries):
        h, b, _ = app.dump("%s_poll%02d" % (tag, i), pause=0.6, keep=False)
        if h and nav_of(h)["plans"]["flagAutoMove"] == 0:
            return {"settled": True, "polls": i + 1}
        time.sleep(pause)
    return {"settled": False, "polls": tries}


def camera_move(app, tag):
    """F96's own move, offset 0 throughout: `select planet Jupiter` +
    `flag track_object on`, let the aim converge, then release."""
    app.send("select planet %s" % MOVE_TARGET, 1.0)
    app.send("flag track_object on", 2.0)
    s = settle_aim(app, tag + "_arm")
    app.send("flag track_object off", 1.5)
    s2 = settle_aim(app, tag + "_rel")
    return {"arm": s, "release": s2}


def stage_freeze(app, leg, rate):
    hP, bP, pP = app.dump("p0_launch")
    leg["camera_p0"] = cam_fields(hP)
    leg["jd_p0"] = hP.get("jd")
    leg["timeSpeed_p0"] = hP.get("timeSpeed")
    leg["dump_p0"] = str(pP)
    leg["move"] = camera_move(app, "mv")
    hA, bA, pA = app.dump("a0_moved")
    leg["camera_a0"] = cam_fields(hA)
    leg["jd_a0"] = hA.get("jd")
    leg["dump_a0"] = str(pA)

    fr = freshness(bP, bA)
    leg["freshness"] = fr
    pt = part.partition({n: r for n, r in bA.items()})
    leg["partition"] = pt
    frozen, union, I = set(fr["frozen"]), set(pt["union"]), set(pt["I"])
    leg["expect"] = "P u I" if not rate else "I"
    expected = union if not rate else I
    leg["frozen_minus_expected"] = sorted(frozen - expected)
    leg["expected_minus_frozen"] = sorted(expected - frozen)
    (ok if frozen == expected else fail)(
        "[%s clock] frozen = %d of %d; expected %s = %d; frozen\\expected %s ; "
        "expected\\frozen %s"
        % ("pinned" if not rate else "running", len(frozen), fr["n"],
           leg["expect"], len(expected),
           leg["frozen_minus_expected"] or "(none)",
           leg["expected_minus_frozen"] or "(none)"))

    eP, eA = evalcounts(bP), evalcounts(bA)
    leg["evalcount"] = {n: [eP.get(n), eA.get(n)] for n in sorted(bA)}
    frames = (eA.get(CONTROL) or 0) - (eP.get(CONTROL) or 0)
    leg["frames_between_dumps"] = frames
    pmi = [n for n in pt["P_only"]]
    leg["parked_eval_delta"] = {n: (eA.get(n) or 0) - (eP.get(n) or 0) for n in pmi}
    stuck = [n for n, d in leg["parked_eval_delta"].items() if d == 0]
    leg["parked_eval_unchanged"] = sorted(stuck)
    ok("[%s clock] frames between the dumps (Mars evalCount delta) = %d; "
       "parked bodies whose evalCount did NOT move: %d of %d"
       % ("pinned" if not rate else "running", frames, len(stuck), len(pmi)))

    rows = {}
    for n in sorted(set(bP) & set(bA)):
        rows[n] = {"sep_p0_deg": altaz_sep(bP[n]), "sep_a0_deg": altaz_sep(bA[n]),
                   "class": ("P" if n in set(pt["P"]) else "") +
                            ("I" if n in set(pt["I"]) else "") or "walked",
                   "frozen": n in frozen}
    leg["altaz"] = rows
    walked = [n for n, r in rows.items()
              if r["class"] == "walked" and r["sep_a0_deg"] is not None]
    pk = [n for n in pt["P_only"] if rows.get(n, {}).get("sep_a0_deg") is not None]
    leg["walked_sep_a0_max"] = max(((rows[n]["sep_a0_deg"], n) for n in walked),
                                   default=(None, None))
    leg["parked_sep_a0_max"] = max(((rows[n]["sep_a0_deg"], n) for n in pk),
                                   default=(None, None))
    leg["parked_sep_p0_max"] = max(((rows[n]["sep_p0_deg"], n) for n in pk
                                    if rows[n]["sep_p0_deg"] is not None),
                                   default=(None, None))
    ok("[%s clock] old-vs-new alt/az after the move: walked max %s ; parked max %s "
       "(parked max at the LAUNCH state, before the move: %s)"
       % ("pinned" if not rate else "running",
          leg["walked_sep_a0_max"], leg["parked_sep_a0_max"],
          leg["parked_sep_p0_max"]))
    return leg


def query_status(app, name, pause=1.2):
    app.send("select planet %s" % name, 1.0)
    return app.query("get status object", pause)


def stage_operator(app, leg, rate):
    """WHICH PATH ANSWERS, and does its answer go stale?  `select planet <name>`
    is OLD-FIRST (core.cpp:1091-1097 -> ssystem_factory.cpp:909-915), so the
    string a shipped command prints is measured against BOTH halves of the
    dump's own sidecar rather than assumed."""
    out = {"clock": "running" if rate else "pinned"}
    out["before"] = {}
    for name in (DWARF, CONTROL):
        out["before"][name] = query_status(app, name)
    h1, b1, p1 = app.dump("op_before")
    out["dump_before"] = str(p1)
    out["move"] = camera_move(app, "op_mv")
    out["after"] = {}
    for name in (DWARF, CONTROL):
        out["after"][name] = query_status(app, name)
    h2, b2, p2 = app.dump("op_after")
    out["dump_after"] = str(p2)

    for tag, dump, answers in (("before", p1, out["before"]),
                               ("after", p2, out["after"])):
        blocks = navstr_blocks(str(dump) + ".navstr")
        out.setdefault("routing", {})[tag] = {}
        for name in (DWARF, CONTROL):
            got = normalise(" ".join(answers[name]))
            o = normalise(blocks.get(name, {}).get("OLD inf"))
            n = normalise(blocks.get(name, {}).get("NEW inf"))
            which = ("OLD" if got and got == o else
                     "NEW" if got and got == n else "neither")
            out["routing"][tag][name] = {"answer": got, "old_inf": o,
                                         "new_inf": n, "matches": which}
            ok("[%s clock/%s] get status object %-6s -> matches %s half"
               % (out["clock"], tag, name, which))

    # the tracking arm: `flag track_object on` hands the NEW path the body
    # (core.cpp:2494) and the aim consumes its position every frame
    # (Camera.cpp:653-664) -- a frozen position cannot be centred.
    out["track"] = {}
    for name in (DWARF, CONTROL):
        app.send("flag track_object off", 1.0)
        app.send("select planet %s" % name, 1.0)
        app.send("flag track_object on", 2.0)
        s = settle_aim(app, "trk_%s" % name)
        h, b, p = app.dump("track_%s" % name)
        rec = b.get(name, {})
        out["track"][name] = {
            "settle": s,
            "new_screen": nh(rec).get("screen"),
            "old_screen": (rec.get("old") or {}).get("screen"),
            "camera_tracked": cam_of(h).get("tracked"),
            "cam_alt_az": [cam_of(h).get("alt"), cam_of(h).get("az")],
            "dump": str(p)}
        sc = out["track"][name]["new_screen"] or [None, None]
        mag = (math.hypot(*sc) if sc[0] is not None else None)
        out["track"][name]["new_screen_radius"] = mag
        ok("[%s clock] track %-6s -> new screen %s (|screen| = %s), settled %s"
           % (out["clock"], name, sc, mag, s["settled"]))
    app.send("flag track_object off", 1.0)
    leg["operator"] = out
    return out


def stage_track(app, leg, rate):
    """THE ONE OPERATOR SURFACE THE NEW PATH ACTUALLY DRIVES.  `flag
    track_object on` hands the NEW body to the camera (core.cpp:2494) and the
    aim reads its position every frame (Camera.cpp:653-664
    `lookTo(observedToLocalPos(target->getObservedPosition()))`), so a body
    whose eye-frame position is frozen aims the camera at where it was.

    The order is the control: the WALKED body is tracked FIRST, from the same
    moved camera state, so a wrecked aim after the hidden body cannot be blamed
    on the move.  A hidden body's `screen` is never written (it is not visible,
    ModularBody dumps 0), so the observable is the CAMERA's own state across the
    polls -- does it converge, and where."""
    out = {"clock": "running" if rate else "pinned", "arms": []}
    app.send("flag track_object off", 1.0)
    out["move"] = camera_move(app, "tk_mv")

    def arm(name, label):
        app.send("flag track_object off", 1.0)
        app.send("select planet %s" % name, 1.0)
        app.send("flag track_object on", 1.5)
        polls = []
        for i in range(7):
            h, b, _p = app.dump("tk_%s_%02d" % (label, i), pause=0.7, keep=False)
            c = cam_of(h)
            rec = b.get(name, {})
            polls.append({"alt": c.get("alt"), "az": c.get("az"),
                          "heading": c.get("heading"),
                          "tracked": c.get("tracked"),
                          "new_screen": nh(rec).get("screen"),
                          "old_screen": (rec.get("old") or {}).get("screen"),
                          "altaz_old": rec.get("altaz_old"),
                          "altaz_new": rec.get("altaz_new"),
                          "sep_old_new_deg": altaz_sep(rec)})
            time.sleep(0.5)
        last, prev = polls[-1], polls[-2]
        conv = (abs((last["alt"] or 0) - (prev["alt"] or 0)) < 1e-6
                and abs((last["az"] or 0) - (prev["az"] or 0)) < 1e-6)
        at_clamp = abs(abs(last["alt"] or 0) - math.pi / 2) < 1e-5
        a = {"body": name, "label": label, "polls": polls, "converged": conv,
             "alt_at_clamp": at_clamp, "final": last}
        out["arms"].append(a)
        ok("[%s clock] track %-6s (%s): final alt %.6f az %.6f, converged %s, "
           "alt at +-pi/2 %s, new screen %s, old-vs-new alt/az %s deg"
           % (out["clock"], name, label, last["alt"] or 0, last["az"] or 0,
              conv, at_clamp, last["new_screen"], last["sep_old_new_deg"]))
        return a

    arm(CONTROL, "control_before")
    arm(DWARF, "hidden")
    arm(CONTROL, "control_after")
    app.send("flag track_object off", 1.0)
    leg["track"] = out
    return out


def stage_cost(app, leg, rate):
    """THE D11 NUMBER, and the mutation's discriminator.  Mars's own evalCount
    IS the frame counter for a body the walk always evaluates (11.117(f)), so
    `delta(parked) / delta(Mars)` is refreshes per frame -- comparable across
    binaries without a clock."""
    out = {"clock": "running" if rate else "pinned", "arms": {}}
    app.send("flag track_object off", 1.0)
    app.send("select planet %s" % COST_BODY, 1.5)

    def sample(tag, driver=None, seconds=10.0):
        _h, b0, _p = app.dump("cost_%s_a" % tag, keep=False)
        t0 = time.time()
        if driver:
            driver()
        while time.time() - t0 < seconds:
            time.sleep(0.5)
        _h, b1, _p = app.dump("cost_%s_b" % tag, keep=False)
        e0, e1 = evalcounts(b0), evalcounts(b1)
        frames = (e1.get(CONTROL) or 0) - (e0.get(CONTROL) or 0)
        d = {n: (e1.get(n) or 0) - (e0.get(n) or 0)
             for n in (COST_BODY, "Charon", CONTROL, "Sun")}
        per = {n: (v / frames if frames else None) for n, v in d.items()}
        out["arms"][tag] = {"frames": frames, "delta": d, "per_frame": per,
                            "seconds": round(time.time() - t0, 1)}
        ok("[%s clock] cost/%s: %d frames, %s eval delta %s -> %.4f per frame"
           % (out["clock"], tag, frames, COST_BODY, d[COST_BODY],
              per[COST_BODY] if per[COST_BODY] is not None else float("nan")))

    sample("held")

    def move():
        # a CONTINUOUS observer move: the eye frame changes every frame while
        # the clock does not.  lat/lon only -- 5.109 forbids a datum-relative
        # `alt` here.
        app.send("moveto lat 20 lon 20 duration 12", 0.3)
    sample("moving", driver=move, seconds=13.0)
    leg["cost"] = out
    return out


# --------------------------------------------------------------------- main
def run_clock(binp, out, rep, rate, stages):
    name = "running" if rate else "pinned"
    farm = FARM_ROOT / name
    build_farm(farm)
    rep[name] = leg = {"farm": str(farm), "rate": rate,
                       "lock_before": lock_state(), "md5_in": md5_home()}
    hits = no_instance()
    if hits:
        raise RuntimeError("another spacecrafter is running: %s" % hits)
    app = App(binp, farm, out, name)
    try:
        leg["tcp_seconds"] = app.start()
        setup(app, rate)
        app.settle_scale()
        if "freeze" in stages:
            stage_freeze(app, leg, rate)
        if "operator" in stages:
            stage_operator(app, leg, rate)
        if "track" in stages:
            stage_track(app, leg, rate)
        if "cost" in stages:
            stage_cost(app, leg, rate)
    finally:
        leg["exit_code"] = app.stop()
        leg["lock_after"] = lock_state()
        leg["md5_out"] = md5_home()
        if leg["md5_in"] != leg["md5_out"]:
            fail("the real ~/.spacecrafter moved during leg %s: %s -> %s"
                 % (name, leg["md5_in"], leg["md5_out"]))
    return leg


def report(rep, out):
    lines = []
    for clock in ("pinned", "running"):
        leg = rep.get(clock)
        if not leg:
            continue
        lines.append("=== clock %s (timerate rate %s)" % (clock, leg["rate"]))
        if "freshness" in leg:
            fr, pt = leg["freshness"], leg["partition"]
            lines.append("  freshness: %d frozen / %d re-evaluated of %d"
                         % (len(fr["frozen"]), len(fr["moved"]), fr["n"]))
            lines.append("  partition: |P| %d  |I| %d  |PnI| %d  |PuI| %d  |P\\I| %d"
                         % (len(pt["P"]), len(pt["I"]), len(pt["both"]),
                            len(pt["union"]), len(pt["P_only"])))
            lines.append("  expected frozen = %s ; frozen\\expected %s ; "
                         "expected\\frozen %s"
                         % (leg["expect"], leg["frozen_minus_expected"] or "(none)",
                            leg["expected_minus_frozen"] or "(none)"))
            lines.append("  frames between the dumps = %d ; parked (P\\I) with a "
                         "STILL evalCount: %d of %d  %s"
                         % (leg["frames_between_dumps"],
                            len(leg["parked_eval_unchanged"]),
                            len(leg["parked_eval_delta"]),
                            leg["parked_eval_unchanged"]))
            lines.append("  old-vs-new alt/az after the move: walked max %s ; "
                         "parked max %s ; parked max at P0 %s"
                         % (leg["walked_sep_a0_max"], leg["parked_sep_a0_max"],
                            leg["parked_sep_p0_max"]))
            lines.append("  frozen: %s" % ", ".join(sorted(fr["frozen"])))
        if "operator" in leg:
            o = leg["operator"]
            for tag in ("before", "after"):
                for nm, r in o["routing"][tag].items():
                    lines.append("  get status object %-6s %-6s -> %s half"
                                 % (nm, tag, r["matches"]))
                    lines.append("      answer : %s" % r["answer"])
                    lines.append("      OLD    : %s" % r["old_inf"])
                    lines.append("      NEW    : %s" % r["new_inf"])
            for nm, t in o["track"].items():
                lines.append("  track %-6s: new screen %s |r| %s settled %s polls %d"
                             % (nm, t["new_screen"], t["new_screen_radius"],
                                t["settle"]["settled"], t["settle"]["polls"]))
        if "track" in leg:
            for a in leg["track"]["arms"]:
                f = a["final"]
                lines.append("  track %-6s (%-14s): alt %.6f az %.6f heading %s ; "
                             "converged %s ; alt at +-pi/2 %s ; new screen %s ; "
                             "old-vs-new alt/az %s deg"
                             % (a["body"], a["label"], f["alt"] or 0, f["az"] or 0,
                                f["heading"], a["converged"], a["alt_at_clamp"],
                                f["new_screen"], f["sep_old_new_deg"]))
                lines.append("      alt track: %s"
                             % " ".join("%.5f" % (p["alt"] or 0) for p in a["polls"]))
        if "cost" in leg:
            for tag, a in leg["cost"]["arms"].items():
                lines.append("  cost/%s: %d frames, %s +%d (%.4f/frame), "
                             "Charon +%d, Sun +%d"
                             % (tag, a["frames"], COST_BODY, a["delta"][COST_BODY],
                                a["per_frame"][COST_BODY] or 0,
                                a["delta"]["Charon"], a["delta"]["Sun"]))
        lines.append("  md5 in==out: %s ; lock before %s after %s ; exit %s"
                     % (leg["md5_in"] == leg["md5_out"], leg["lock_before"],
                        leg["lock_after"], leg["exit_code"]))
    lines.append("FAILS: %d  NOTES: %d" % (len(FAILS), len(NOTES)))
    for m in FAILS:
        lines.append("  FAIL: " + m)
    for m in NOTES:
        lines.append("  NOTE: " + m)
    txt = "\n".join(lines) + "\n"
    (Path(out) / "f100_report.txt").write_text(txt)
    print(txt)


def parked_table(paths, out=None):
    """THE TABLE THE ENTRY CITES: one row per parked body with a meaningful
    position (P \\ I), old-vs-new alt/az at the launch state and after the move,
    with the evalCount pair beside it -- across as many result files as are
    given (`<label>=<path>/f100_result.json`).  The evalCount column is what
    turns a number into a mechanism: a body whose position code never ran and
    whose readout is 170 deg off is a memo, not an ephemeris."""
    legs = []
    for spec in paths:
        label, _, p = spec.partition("=")
        rep = json.load(open(p))
        for clock in ("pinned", "running"):
            if clock in rep and "altaz" in rep[clock]:
                legs.append(("%s/%s" % (label, clock), rep[clock]))
    if not legs:
        return ""
    names = sorted(legs[0][1]["partition"]["P_only"])
    ctrl = ["Mars", "Jupiter", "Moon", "Neptune", "Belinda"]
    w = 30
    head = "%-18s | " % "body" + " | ".join(("%*s" % (w, l)) for l, _ in legs)
    lines = [head,
             "%-18s | " % "" + " | ".join(("%*s" % (w, "P0sep / A0sep / eval0,eval1"))
                                          for _ in legs), "-" * len(head)]
    for group, title in ((names, "PARKED, meaningful position (P \\ I)"),
                         (ctrl, "WALKED controls")):
        lines.append("== " + title)
        for n in group:
            cells = []
            for _l, leg in legs:
                a = leg["altaz"].get(n, {})
                e = leg["evalcount"].get(n, [None, None])
                cells.append("%*s" % (w, "%.6g / %.6g / %s,%s"
                                      % (a.get("sep_p0_deg") or -1,
                                         a.get("sep_a0_deg") or -1, e[0], e[1])))
            lines.append("%-18s | " % n + " | ".join(cells))
    for _l, leg in legs:
        lines.append("%s: frames between the dumps = %d; frozen %d of %d; "
                     "expected %s" % (_l, leg["frames_between_dumps"],
                                      len(leg["freshness"]["frozen"]),
                                      leg["freshness"]["n"], leg["expect"]))
    txt = "\n".join(lines) + "\n"
    if out:
        Path(out).write_text(txt)
    print(txt)
    return txt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", default=str(HERE / "../../build-claude/src/spacecrafter"))
    ap.add_argument("--tag", default="pre")
    ap.add_argument("--clock", default="pinned,running")
    ap.add_argument("--stages", default="freeze,operator,cost")
    ap.add_argument("--score", action="store_true")
    ap.add_argument("--parked-table", nargs="*", default=None,
                    metavar="LABEL=RESULT.json")
    a = ap.parse_args()
    if a.parked_table:
        parked_table(a.parked_table, a.out)
        return 0
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    rep = {"tag": a.tag, "bin": str(Path(a.bin).resolve()),
           "started": time.strftime("%Y-%m-%d %H:%M:%S %Z")}
    if a.score:
        rep = json.load(open(out / "f100_result.json"))
        report(rep, out)
        return 0
    stages = set(a.stages.split(","))
    for clock in a.clock.split(","):
        run_clock(a.bin, out, rep, 1 if clock == "running" else 0, stages)
    rep["fails"], rep["notes"] = FAILS, NOTES
    json.dump(rep, open(out / "f100_result.json", "w"), indent=1)
    report(rep, out)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
