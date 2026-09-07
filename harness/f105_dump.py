#!/usr/bin/env python3
"""F105 -- IS A DUMP A USE FOR THE 30 NEW-ONLY RECORDS, AND IN WHOSE FRAME?

    cd claude/harness && DISPLAY=:2 python3 f105_dump.py <absOutdir> \
        --tag <name> --bin <path> [--stages dump,operator] [--jd 2461233.5]

WHAT IT MEASURES.  The dual dump's second loop (ssystem_factory.cpp:1223-1245)
emits the 30 records the old tree does not carry and does NOT call the D8
barrier, so a dump is a use for 90 of 120 (INTENT 11.220(j3)).  Ten of those 30
are HIDDEN anchor bodies; eight of the ten hang off `Universe`, which the update
walk never visits (dispatchUpdate's up-chain loop is
`while (body->isNotIsolated)` and ModularSystem's ctor sets that false), so
their parent has never published a `parkedChildFrame` and a use would refresh
them in the IDENTITY frame -- 11.220(j4).  This driver takes ONE launch per
binary at a PINNED date and reports, per record: the eye-frame position, the
distance, `ecl`, `evalCount` and `lastJD`, plus the guard's own log lines.

THE OPERATOR ARM is here because the SELECTION channel already calls the
barrier every frame (ModularSystem.cpp:269-270) and `select planet <name>`
resolves a new-only name through the ModularObject bridge, so the hazard is
reachable from a shipped command on the ROUND'S OWN BASELINE BINARY, with no
code change at all.  Each arm tracks the WALKED control (Mars) before and after
the subject, from the same camera state, so a wrecked aim cannot be blamed on
the move (F100's shape); the `dist 0` control (51PegSystem, a system record the
barrier returns on its first line for) is the zero-vector class the guard is
predicted to put the subject INTO.

BOUNDARY.  Nothing is written outside <absOutdir> and its farm; the real
~/.spacecrafter is md5'd in and out and never opened for writing.
"""
import argparse
import hashlib
import json
import os
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f96_offset import (App, build_farm, cam_fields, cam_of,                   # noqa: E402
                        no_instance, JD)
from f99_locguard import lock_state                                # noqa: E402
import dumpread                                                    # noqa: E402
import f100_partition as part                                      # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"

# The eight hidden anchor bodies whose parent the walk never visits, plus the
# two whose parent it does.  Named here from the TREE (anchor.ini + the parent
# chain), never from a dump: the driver must be able to report a name it
# expected and did not find.
CLASS_C = ["big_dipper", "center_sun", "galaxy_center", "pleiades",
           "sun_in_gal", "un_point", "orbit_autour_point (orbit centre)",
           "orbit_autour_point"]
CLASS_B = ["baryEarthMoon", "orbit_autour_lune"]
PARENTS = {"baryEarthMoon": "Earth", "orbit_autour_lune": "Moon"}

# The guard's own line, matched on the part of the message that names the
# MECHANISM and not the body.  0 on any binary without the guard.  Measured
# gotcha (S11.226): the first version of this marker quoted the message
# verbatim ("has never published"), and when the message itself was corrected to
# "has not published" the count silently read 0 on a leg whose log carried 8 -
# a marker is a copy of a string that lives somewhere else (I2), so it matches
# the stable clause only, and the count is re-derivable from the log file.
D12_MARK = "published a position frame for its parked children"

SUBJECT = "pleiades"        # one of the eight: the largest identity-frame value
ZERO_CTL = "51PegSystem"    # a `dist 0` record the barrier returns early for
WALK_CTL = "Mars"           # the walked control, tracked before AND after


def md5_home():
    return {n: hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
            for n in ("config.ini", "ssystem.ini")}


def parse(path):
    """The dump, read through `dumpread`'s GRAMMAR (I2, S5.103).

    MEASURED HERE, and it is why this function exists rather than
    `f96_offset.parse_dump`: tracking a body whose eye-frame position is the
    ZERO VECTOR drives the camera state to `nan`, the header prints bare `nan`,
    and a plain `json.loads` raises on that line -- so the whole HEADER is
    silently skipped and the camera reads as absent exactly in the arm that
    matters.  `dumpread.loads` is the reader that already knows the C
    spellings; every line goes through it."""
    hdr, bodies = {}, {}
    with open(path, "rt", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            try:
                o = dumpread.loads(line)
            except ValueError:
                continue
            if o.get("type") == "header":
                hdr = o
                # Recorded, not inferred: whether THIS header line carries a
                # bare `nan`/`inf` (the token a strict json.loads dies on).
                hdr["_raw_nonfinite"] = bool(
                    dumpread._NONFINITE.search(line))
            elif o.get("type") == "body":
                bodies[o["name"]] = o
    return hdr, bodies


def take(app, tag, keep=True, pause=1.0):
    """app.dump() writes and returns f96_offset's own parse; this re-reads the
    file through `parse` above and optionally drops it (a poll dump is 2.5 MB
    and there are 28 of them in an operator leg)."""
    app.dump(tag, pause=pause, keep=True)
    p = app.out / "dumps" / ("%s_%s.json" % (app.name, tag))
    h, b = parse(p)
    if not keep:
        p.unlink()
        sc = Path(str(p) + ".navstr")
        if sc.exists():
            sc.unlink()
    return h, b, p


def rec_fields(rec):
    """The fields this task reads, for one dump record."""
    n = (rec or {}).get("new") or {}
    m = n.get("mat") or []
    return {"parent": n.get("parent"), "relation": n.get("relation"),
            "dist": n.get("dist"), "ecl": n.get("ecl"),
            "mat_t": m[12:15] if len(m) >= 15 else None,
            "evalCount": n.get("evalCount"), "lastJD": n.get("lastJD"),
            "axisRot": n.get("axisRot")}


class Log:
    """The ENGINE's own log file (cLog writes there, and to the console only
    when isDebug -- log.cpp), read by count at named moments.  F101's `applog`
    is the process's stdout and is a different channel; both are recorded."""

    def __init__(self, home):
        self.path = Path(home) / "log" / "spacecrafter.log"

    def lines(self, mark=D12_MARK):
        if not self.path.is_file():
            return []
        txt = self.path.read_text(encoding="latin-1", errors="replace")
        return [ln.strip() for ln in txt.splitlines() if mark in ln]


def stage_dump(app, res, log, tag):
    """Two dumps in ONE launch: the first is the measurement, the second is the
    once-per-body check (a guard that logs at every use would double)."""
    out = {}
    h0, b0, p0 = take(app, "p0_launch")
    out["dump_p0"] = str(p0)
    out["jd"] = h0.get("jd")
    out["timeSpeed"] = h0.get("timeSpeed")
    out["oldSystem"] = h0.get("oldSystem", "<absent>")
    out["inSystem"] = h0.get("inSystem", "<absent>")
    out["camera_p0"] = cam_fields(h0)
    out["n_records"] = len(b0)
    out["n_new_only"] = sum(1 for r in b0.values() if r.get("old") is None)
    out["log_after_p0"] = log.lines()
    out["records_p0"] = {n: rec_fields(b0.get(n)) for n in CLASS_C + CLASS_B}
    for n in ("Earth", "Moon", "Phobos", "SolarSystem", "Universe", "MilkyWay"):
        out["records_p0"][n] = rec_fields(b0.get(n))
    pt = part.partition(b0)
    out["partition_p0"] = {"P": len(pt["P"]), "I": len(pt["I"]),
                           "both": len(pt["both"]), "n": pt["n"],
                           "I_names": pt["I"]}
    h1, b1, p1 = take(app, "p1_second")
    out["dump_p1"] = str(p1)
    out["log_after_p1"] = log.lines()
    out["records_p1"] = {n: rec_fields(b1.get(n)) for n in CLASS_C + CLASS_B}
    res["dump"] = out
    print("  dump: %d records, %d new-only, oldSystem=%s inSystem=%s ; "
          "D12 lines after p0 = %d, after p1 = %d"
          % (out["n_records"], out["n_new_only"], out["oldSystem"],
             out["inSystem"], len(out["log_after_p0"]),
             len(out["log_after_p1"])), flush=True)
    return out


def track_arm(app, name, label, polls=7):
    """select + track + poll the CAMERA's own state.  A hidden body's `screen`
    is never written (F100), so the observable is where the camera ends up."""
    app.send("flag track_object off", 1.0)
    app.send("select planet %s" % name, 1.2)
    app.send("flag track_object on", 1.5)
    seq, rec = [], None
    for i in range(polls):
        keep = (i == polls - 1)
        h, b, _p = take(app, "tk_%s_%02d" % (label, i), keep=keep, pause=0.7)
        c = cam_of(h)
        seq.append({"alt": c.get("alt"), "az": c.get("az"),
                    "heading": c.get("heading"), "tracked": c.get("tracked"),
                    "dist": (rec_fields(b.get(name)) or {}).get("dist"),
                    "hdr_nonfinite": h.get("_raw_nonfinite")})
        if keep:
            rec = rec_fields(b.get(name))
        time.sleep(0.4)
    last, prev = seq[-1], seq[-2]
    def fin(x):
        return x is not None and x == x     # NaN != NaN: the zero-vector arm
    conv = (fin(last["alt"]) and fin(prev["alt"])
            and abs(last["alt"] - prev["alt"]) < 1e-6
            and abs(last["az"] - prev["az"]) < 1e-6)
    app.send("flag track_object off", 1.0)
    a = {"body": name, "label": label, "polls": seq, "converged": conv,
         "final": last, "record_while_selected": rec}
    print("  track %-24s (%-14s): alt %s az %s tracked=%s conv=%s dist=%s"
          % (name, label, last["alt"], last["az"], last["tracked"], conv,
             (rec or {}).get("dist")), flush=True)
    return a


def stage_operator(app, res, log):
    out = {"arms": []}
    for name, label in ((WALK_CTL, "walked_before"),
                        (SUBJECT, "subject"),
                        (ZERO_CTL, "zero_control"),
                        (WALK_CTL, "walked_after")):
        out["arms"].append(track_arm(app, name, label))
    out["log_after_operator"] = log.lines()
    res["operator"] = out
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--stages", default="dump")
    ap.add_argument("--jd", type=float, default=JD)
    a = ap.parse_args()

    out = Path(a.out).resolve() / a.tag
    out.mkdir(parents=True, exist_ok=True)
    stages = [s for s in a.stages.split(",") if s]

    res = {"tag": a.tag, "bin": a.bin, "stages": stages, "jd_asked": a.jd,
           "bin_md5": hashlib.md5(open(a.bin, "rb").read()).hexdigest()[:8],
           "started": time.strftime("%Y-%m-%dT%H:%M:%S")}
    hits = no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)
    res["proc_before"] = hits
    res["lock_before"] = lock_state()
    res["md5_in"] = md5_home()

    farm = out / "farm"
    build_farm(farm)
    home = farm / ".spacecrafter"
    log = Log(home)

    app = App(a.bin, farm, out, a.tag)
    try:
        res["tcp_seconds"] = app.start()
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("meteors zhr 0")
        app.send("date jday %.9f" % a.jd, 1.5)
        app.settle_scale()
        res["log_after_setup"] = log.lines()
        if "dump" in stages:
            stage_dump(app, res, log, a.tag)
        if "operator" in stages:
            stage_operator(app, res, log)
        res["log_all"] = log.lines()
    finally:
        res["exit_code"] = app.stop()
        res["lock_after"] = lock_state()
        res["md5_out"] = md5_home()
        res["proc_after"] = no_instance()
        applog = out / ("%s.applog" % a.tag)
        if applog.is_file():
            txt = applog.read_text(encoding="latin-1", errors="replace")
            res["applog_d12_lines"] = [ln.strip() for ln in txt.splitlines()
                                       if D12_MARK in ln]
        (out / "f105_result.json").write_text(json.dumps(res, indent=1))

    if res["md5_in"] != res["md5_out"]:
        raise SystemExit("BOUNDARY BREACH: the real ~/.spacecrafter moved: %s -> %s"
                         % (res["md5_in"], res["md5_out"]))
    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("dump", "operator")}, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
