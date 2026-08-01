#!/usr/bin/env python3
"""§5.63 attribution, wave 2: bisect the MECHANISM, not the content.

Wave 1 (`f21_s563.py`) established, in one run and against an in-scene A/A floor
of 0 at every stage: the residual lives ENTIRELY in the old-path sky content
(stars 2418 px + milky way 744 px + nebulae 2493 px = the whole 5655), it is
PHOTOMETRIC (restored brighter, +10.8/8-bit, lit 400 vs 907) and it is
PERSISTENT (identical at t0, t+15 s and t+40 s, untouched).

Stars, the milky way and the nebulae have exactly one thing in common: they are
drawn through the old `ToneReproductor`, whose world adaptation luminance is
recomputed every frame from the ATMOSPHERE (`solarSystemModule.cpp:156` ->
`Atmosphere::getWorldAdaptationLuminance`, computed at `atmosphere.cpp:167,292`).

So this wave re-asserts ONE candidate state at a time, to the SAME value on both
sides, and reports where the difference collapses. A stage that collapses it
names a state that was divergent; a stage that does not, excludes its state.
The last stage is the both-ways half: having equalised the atmosphere flag by
turning it OFF on both sides, turn it back ON on both sides.  If the difference
stays at zero, the divergent state was THE FLAG ITSELF (a §2 row E3 flag - which
is what the F21 slice serializes).  If it comes back, the flag is not it and the
divergence is in something the flag does not equalise.

    cd claude/harness && DISPLAY=:2 ./f21_s563b.py [outdir]
"""

import json
import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f21_s563 import (App, JD, OUT as _OUT, SESSIONS, USERDIR, FROZEN, md5,
                      assert_no_other_instance, build_scene_a, stats)

import numpy as np

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f21s563c").resolve()
OUT.mkdir(parents=True, exist_ok=True)

# Each stage re-asserts ONE candidate state, to a fixed value, on both sides.
STAGES = [
    ("base", None),
    ("unlock", "flag lock_sky_position off"),
    ("relock", "flag lock_sky_position on"),
    ("heading0", "set heading 0"),
    ("look_up", "look_at altitude 60"),
]


def ladder(app, prefix):
    out = {}
    for name, cmd in STAGES:
        if cmd:
            app.cmd(cmd, 2.5)
        p = OUT / f"f21c_{prefix}_{name}.png"
        p.unlink(missing_ok=True)
        app.cmd(f"body action screenshot filename {p}", 2.5)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.3)
        from PIL import Image
        out[name] = np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)
    return out


def main():
    if not assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f21c*.ini"):
        f.unlink()
    frozen_in = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}

    import f21_s563
    f21_s563.OUT = OUT                      # App/shot write here

    app = App("b_saved")
    build_scene_a(app)
    app.cmd("session action save filename f21c", 2.5)
    saved = ladder(app, "saved")
    app.quit()

    app = App("b_aa")
    build_scene_a(app)
    aa = ladder(app, "aa")
    app.quit()

    app = App("b_rest")
    app.cmd("session action load filename f21c", 6.0)
    rest = ladder(app, "rest")
    app.quit()

    frozen_out = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}
    print(f"\nfrozen md5 in == out: {frozen_in == frozen_out}", flush=True)

    print("\n== mechanism ladder: each stage re-asserts ONE state on BOTH sides ==", flush=True)
    print(f"{'stage':<12} {'restored px>8':>14} {'floor px>8':>12} {'signed mean':>12} "
          f"{'lit saved':>10} {'lit rest':>9}", flush=True)
    res = {}
    for name, cmd in STAGES:
        s = stats(saved[name], rest[name])
        f = stats(saved[name], aa[name])
        res[name] = {"cmd": cmd, "restored": s, "floor": f}
        print(f"{name:<12} {s['px8']:>14} {f['px8']:>12} {s['signed_mean']:>12.3f} "
              f"{s['lit_a']:>10} {s['lit_b']:>9}", flush=True)
    (OUT / "f21_s563b.json").write_text(json.dumps(res, indent=1))
    print(f"\nartifacts in {OUT}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
