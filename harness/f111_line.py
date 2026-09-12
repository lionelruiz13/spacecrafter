#!/usr/bin/env python3
"""F111 -- IS THE DRAWN ORBIT LINE THE SAME LINE?  INTENT Sec.5.150 / Sec.11.239.

    cd claude/harness && DISPLAY=:2 python3 f111_line.py <absOutdir> \
        --tag <name> --bin <path> [--body Pasiphae] [--fov 2.0]

WHY A DEDICATED LEG.  The first attempt compared the default-camera screenshots
of the two binaries and could not discriminate: turning `satellites_orbits` on
changed 91505 pixels INSIDE one launch and two launches of the SAME binary
differed by 91533, so the A/A floor swallowed the signal whole.  A check that
cannot fail is not evidence (Sec.0.5), so this leg removes the floor instead of
reporting a number taken under it: the noisy layers are switched off, the camera
is put on the body whose line is being compared, and the field of view is
narrowed until that line is a large object on screen.

THE CHECK, AND WHAT IT IS AGAINST.  Four frames per launch:
    a0, a1   flag OFF, twice   -- the A/A floor without lines
    b0, b1   flag ON,  twice   -- the A/A floor WITH lines
and the signal is the PRE-vs-POST difference of b0, judged against
max(|a0-a1|, |b0-b1|) of BOTH launches.  `a0` vs `b0` says whether the line is
on screen at all -- without that the whole leg proves nothing.

WHAT IS PREDICTED (artifacts/f111/prediction.txt P5).  The pre-fix line's FIRST
sampled point is computed from a seed half a visualisation period away and gets
two Newton steps, so it is wrong by up to 0.587 AU (Neso) / 0.033 AU (Pasiphae);
the post-fix line is the converged one.  The prediction is therefore that the
PRE-vs-POST difference EXCEEDS the A/A floor -- the opposite of the mandate's
expectation, and registered before this leg ran.

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
from f96_offset import App, build_farm, no_instance, ini_set, JD       # noqa: E402
from f99_locguard import lock_state                                    # noqa: E402

REAL_HOME = Path.home() / ".spacecrafter"
QUIET = ["flag stars off", "flag star_lines off", "flag constellation_drawing off",
         "flag constellation_art off", "flag constellation_boundaries off",
         "flag constellation_names off", "flag milky_way off", "flag nebula off",
         "flag atmosphere off", "flag landscape off", "flag fog off",
         "flag cardinal_points off", "flag azimuthal_grid off",
         "flag equatorial_grid off", "flag meridian_line off",
         "flag ecliptic_line off", "flag planets_hints off",
         "flag planets_orbits off", "flag object_trails off",
         "meteors zhr 0"]


def md5_home():
    return {n: hashlib.md5((REAL_HOME / n).read_bytes()).hexdigest()[:8]
            for n in ("config.ini", "ssystem.ini")}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--bin", required=True)
    ap.add_argument("--body", default="Jupiter")
    ap.add_argument("--fov", type=float, default=2.0)
    ap.add_argument("--jd", type=float, default=JD)
    a = ap.parse_args()

    out = Path(a.out).resolve() / a.tag
    out.mkdir(parents=True, exist_ok=True)
    res = {"tag": a.tag, "bin": a.bin, "body": a.body, "fov": a.fov,
           "bin_md5": hashlib.md5(open(a.bin, "rb").read()).hexdigest()[:8],
           "started": time.strftime("%Y-%m-%dT%H:%M:%S")}
    hits = no_instance()
    if hits:
        raise SystemExit("REFUSED: another spacecrafter is running: %s" % hits)
    res["proc_before"], res["lock_before"] = hits, lock_state()
    res["md5_in"] = md5_home()

    farm = out / "farm"
    build_farm(farm)
    # THE LINE IS DRAWN AT `planet_orbits_color = 0.2,0.2,0.2` in the shipped
    # config, which at the fader's alpha reaches a MAX CHANNEL DELTA OF 6 -- the
    # A/A floor of the same scene is 449 differing pixels, so the shipped colour
    # cannot carry this check.  Raised to white IN THE FARM COPY ONLY (the real
    # ~/.spacecrafter is md5-asserted in == out and never opened for writing),
    # identically in both arms, so the pre/post comparison is unaffected by it.
    ini_set(farm / ".spacecrafter" / "config.ini", "color",
            "planet_orbits_color", "1.0,1.0,1.0")
    res["farm_orbit_color"] = "1.0,1.0,1.0 (farm copy only)"
    app = App(a.bin, farm, out, a.tag)
    shots = {}
    try:
        res["tcp_seconds"] = app.start()
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("date jday %.9f" % a.jd, 1.5)
        for c in QUIET:
            app.send(c, 0.2)
        app.settle_scale()
        # `pointer off`: the selection marker ANIMATES, and it was the whole of
        # the first attempt's 975-pixel A/A floor (b11_trail_gate.py:160 uses
        # the same spelling for the same reason).
        if a.body.lower() not in ("", "none"):
            app.send("select planet %s pointer off" % a.body, 1.0)
            app.send("flag track_object on", 12.0)
            app.send("zoom fov %g duration 0" % a.fov, 6.0)
        app.send("flag satellites_orbits off", 4.0)
        # `track_object` STAYS ON: released, the aim drifts back and the lines
        # left the frame entirely (measured: 0 differing pixels between the two
        # flag states, 210 non-black pixels in the whole frame).  With the
        # pointer off, tracking costs nothing in A/A noise at a pinned clock.
        shots["a0"] = app.shot("a0_off")
        shots["a1"] = app.shot("a1_off")
        app.send("flag satellites_orbits on", 6.0)
        shots["b0"] = app.shot("b0_on")
        shots["b1"] = app.shot("b1_on")
        app.dump("line_state", pause=0.8, keep=True)
    finally:
        res["shots"] = {k: str(v) for k, v in shots.items()}
        res["exit_code"] = app.stop()
        res["lock_after"] = lock_state()
        res["md5_out"] = md5_home()
        res["proc_after"] = no_instance()
        (out / "f111_line.json").write_text(json.dumps(res, indent=1))
    if res["md5_in"] != res["md5_out"]:
        raise SystemExit("BOUNDARY BREACH: the real ~/.spacecrafter moved: %s -> %s"
                         % (res["md5_in"], res["md5_out"]))
    print(json.dumps(res, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
