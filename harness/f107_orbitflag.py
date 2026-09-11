#!/usr/bin/env python3
"""F107 -- IS THE ORBIT-LINE SAMPLER A CHANNEL INTO THE POSITION SEED, IN THE
ENGINE?  (INTENT 11.225(j2))

    cd claude/harness && DISPLAY=:2 python3 f107_orbitflag.py <absOutdir> \
        --tag <name> --bin <path> [--flag satellites_orbits|planets_orbits|none]
        [--jd 2461233.5] [--jd2 2461234.5]

WHAT IT MEASURES.  `OrbitModule::sampleOrbit` walks 180 dates through the SAME
`iterativeLastE` the position solver uses (OrbitModule.cpp:105-110 ->
orbit.cpp:466 -> :577 -> :582) and leaves it at the LAST sample's date,
`date + 89*visPeriod/180`.  So drawing a body's orbit line perturbs its position
seed, and the next frame's evaluation starts from there.  Whether that is
OBSERVABLE is a separate question from whether it happens, and this driver asks
the observable one on the DELIVERED binary, with no code change:

  D0, D0b   two dumps with the flags OFF -- the WITHIN-LAUNCH control.  Every
            comparison below is dump-to-dump inside ONE launch, which removes
            the cross-launch float noise 11.225(e) measured (90 of 120 records
            move in `altaz_old` between two launches of one binary).  If D0 and
            D0b already differ, nothing else here means anything.
  D1        the flag turned on and the dump sent IMMEDIATELY: the transient
            lives ~1 frame (7 ms at the 144 fps cap), so this arm can only
            catch it by luck, and saying so in advance is the point.
  D2        the same flag, settled: the PREDICTION is bit-identical to D0,
            because at a pinned clock `sampleOrbit` runs ONCE (the
            `|date - lastSampleJD| >= visPeriod/180` test never fires again at a
            frozen date, OrbitModule.cpp:132) and the walked body is
            re-evaluated every frame.
  S0, S1    one screenshot per flag state -- NOT photometric: the count of
            differing pixels only, so that a null in D2 cannot be read as "the
            sampler never ran".
  D3        the date moved to --jd2 and dumped: the CONTROL that shows the
            comparison able to fail.

WHICH FLAG REACHES A SEED.  `wantShown` routes
`body->isSatellite() ? showSatellites : showPlanets` (OrbitModule.cpp:91) and
`isSatellite()` is "the parent is neither primary nor a system"
(ModularBody.hpp:1128-1130).  Every walked ITERATING record of F104's census is
a satellite, and every iterating Sun-child is PARKED (hence erased from
`sortedSystemBodies` by `unregisterBody` and never sampled) -- so
`flag planets_orbits on` samples only closed-form orbits and reaches NO seed.
Both flags are run; the satellite one is the one that can show anything.

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

from f96_offset import App, build_farm, cam_fields, no_instance, JD   # noqa: E402
from f99_locguard import lock_state                                  # noqa: E402
import f100_partition as part                                        # noqa: E402
import f105_dump                                                     # noqa: E402
import f107_model as M                                               # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"


def md5_home():
    return {n: hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
            for n in ("config.ini", "ssystem.ini")}


def take(app, tag):
    app.dump(tag, pause=1.0, keep=True)
    p = app.out / "dumps" / ("%s_%s.json" % (app.name, tag))
    h, b = f105_dump.parse(p)
    return h, b, p


def compare(a, b, names):
    """Per-record: does the new half's position differ, and by how much."""
    moved, maxd, worst = [], 0.0, None
    for n in names:
        ra = (a.get(n) or {}).get("new") or {}
        rb = (b.get(n) or {}).get("new") or {}
        ea, eb = ra.get("ecl"), rb.get("ecl")
        if ea is None or eb is None:
            continue
        if ea != eb:
            d = M.norm(M.sub(ea, eb))
            moved.append(n)
            if d > maxd:
                maxd, worst = d, n
    return moved, maxd, worst


def pixels_differ(p0, p1):
    from PIL import Image
    import numpy as np
    a = np.asarray(Image.open(p0).convert("RGB"), dtype=np.int16)
    b = np.asarray(Image.open(p1).convert("RGB"), dtype=np.int16)
    if a.shape != b.shape:
        return {"shape_a": a.shape, "shape_b": b.shape, "differ": None}
    d = np.abs(a - b).sum(axis=2)
    return {"shape": list(a.shape), "pixels_differing": int((d > 0).sum()),
            "max_abs_channel_delta": int(np.abs(a - b).max())}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--flag", default="satellites_orbits",
                    choices=["satellites_orbits", "planets_orbits", "none"])
    ap.add_argument("--jd", type=float, default=JD)
    ap.add_argument("--jd2", type=float, default=JD + 1.0)
    ap.add_argument("--census", default=str(M.DEFAULT_CENSUS))
    a = ap.parse_args()

    out = Path(a.out).resolve() / a.tag
    out.mkdir(parents=True, exist_ok=True)
    census = json.load(open(a.census))
    walked_iter = M.walked_iterating(census)

    res = {"tag": a.tag, "bin": a.bin, "flag": a.flag, "jd": a.jd, "jd2": a.jd2,
           "bin_md5": hashlib.md5(open(a.bin, "rb").read()).hexdigest()[:8],
           "started": time.strftime("%Y-%m-%dT%H:%M:%S"),
           "walked_iterating_n": len(walked_iter)}
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
        app.send("date jday %.9f" % a.jd, 1.5)
        app.settle_scale()
        h0, b0, p0 = take(app, "d0_flags_off")
        res["jd_d0"], res["camera_d0"] = h0.get("jd"), cam_fields(h0)
        res["n_records"] = len(b0)
        pt = part.partition(b0)
        res["partition"] = {"P": len(pt["P"]), "I": len(pt["I"]), "n": pt["n"]}
        h0b, b0b, _ = take(app, "d0b_flags_off_again")
        s0 = app.shot("s0_flags_off")
        # the within-launch control: nothing changed between D0 and D0b
        mv, mx, w = compare(b0, b0b, list(b0))
        res["ctl_d0_vs_d0b"] = {"moved": mv, "max_AU": mx, "worst": w,
                                "n": len(b0)}
        print("  control D0 vs D0b: %d of %d records moved (max %.3g AU)"
              % (len(mv), len(b0), mx), flush=True)
        if a.flag != "none":
            app.send("flag %s on" % a.flag, 0.0)     # NO settle: the P5b arm
            h1, b1, _ = take(app, "d1_flag_on_immediate")
            mv1, mx1, w1 = compare(b0b, b1, list(b0))
            res["d1_immediate"] = {"moved": mv1, "max_AU": mx1, "worst": w1,
                                   "moved_walked_iter":
                                   [n for n in mv1 if n in walked_iter]}
            print("  D1 (flag on, immediate): %d moved (max %.3g AU, %s)"
                  % (len(mv1), mx1, w1), flush=True)
            h2, b2, _ = take(app, "d2_flag_on_settled")
            mv2, mx2, w2 = compare(b0b, b2, list(b0))
            res["d2_settled"] = {"moved": mv2, "max_AU": mx2, "worst": w2,
                                 "moved_walked_iter":
                                 [n for n in mv2 if n in walked_iter]}
            print("  D2 (flag on, settled):   %d moved (max %.3g AU, %s)"
                  % (len(mv2), mx2, w2), flush=True)
            s1 = app.shot("s1_flag_on")
            if s0 and s1:
                res["pixels"] = pixels_differ(s0, s1)
                print("  screenshot delta: %s" % res["pixels"], flush=True)
        # the control that shows the comparison able to fail
        app.send("date jday %.9f" % a.jd2, 1.5)
        h3, b3, _ = take(app, "d3_other_date")
        mv3, mx3, w3 = compare(b0b, b3, list(b0))
        res["ctl_other_date"] = {"jd": h3.get("jd"), "moved_n": len(mv3),
                                 "max_AU": mx3, "worst": w3, "n": len(b0)}
        print("  control other date: %d of %d records moved (max %.3g AU)"
              % (len(mv3), len(b0), mx3), flush=True)
        res["log_lines"] = log.lines()
    finally:
        res["exit_code"] = app.stop()
        res["lock_after"] = lock_state()
        res["md5_out"] = md5_home()
        res["proc_after"] = no_instance()
        (out / "f107_orbitflag.json").write_text(json.dumps(res, indent=1))

    if res["md5_in"] != res["md5_out"]:
        raise SystemExit("BOUNDARY BREACH: the real ~/.spacecrafter moved: %s -> %s"
                         % (res["md5_in"], res["md5_out"]))
    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("log_lines",)}, indent=1)[:4000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
