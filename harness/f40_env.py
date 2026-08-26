#!/usr/bin/env python3
"""F40 supplement — WHICH LAYER THE SHIPPED-FLAGS FRAME CHANGE BELONGS TO.

    cd claude/harness && DISPLAY=:2 ./f40_env.py <absOutdir> --bin B --tag pre|post

§11.144(i) attributed a 3 300 642 px>8 change of the composed frame across
`camera action free_mode state on`, at the shipped place with the shipped flags,
to the 11 300 km teleport ("day into night").  F40's fix removes the teleport
(the observer moves 0.3 mm) and the frame STILL changes by 3.3 Mpx — so that
attribution cannot be right, and this instrument says what the frame is actually
answering to.

The mechanism is at source and it is deliberate:
`EnvironmentManager::update` sets `onBody = !camera.isFreeMode() &&
!reference->isSystem()` [observed: EnvironmentManager.cpp:81, its own comment
"system references and free flight are the old anchor-point case"], and only the
`onBody` branch updates `reference->groundedEnvironment` — which is where
`LandscapeEnv` (landscape + fog) lives [observed: LandscapeEnv.hpp:8].

FOUR CELLS x TWO BINARIES, one A/A floor per cell, measured adjacent in time
(the module faders are per-FRAME and keep ramping under a frozen clock):

    atmosphere off/on  x  landscape off/on
      -> px>8 across ONE `free_mode` toggle, and the observer's own chord

Reading: a cell whose px collapses between the binaries was the CONVERTER; a
cell whose px survives is the environment gate.  Nothing is fixed here — the
gate is the old path's own `isOnBody()` semantics, so what free flight should
show is user-visible semantics.
"""

import argparse, json, math, sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f27_reply import Session
import dumpread

HERE = Path(__file__).resolve().parent
JD0 = 2461233.5
AU_KM = 149597870.691
LAM, PHI, ALT = 5.0 + 22.0 / 60.0, 43.0 + 18.0 / 60.0, 75.0   # §5.80's own place


def px(a, b, thr=8):
    A = np.asarray(Image.open(a).convert("L"), dtype=int)
    B = np.asarray(Image.open(b).convert("L"), dtype=int)
    return int((np.abs(A - B) > thr).sum())


def eye_of(m16):
    m = np.array(m16, dtype=float).reshape(4, 4).T
    return -m[:3, :3].T @ m[:3, 3]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    ap.add_argument("--tag", default="post")
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, f"f40env{a.tag}", a.bin)
    drv = sess.client("drv")
    n = [0]

    def cmd(c, p=0.8):
        drv.send(c, p)

    def shot(name):
        p = out / f"env_{name}.png"
        drv.send(f"body action screenshot filename {p}", 2.5)
        return p

    def dump(name):
        n[0] += 1
        p = out / f"envdump_{n[0]:03d}_{name}.json"
        drv.send(f"body action dual_dump filename {p}", 1.4)
        head, _, _, _ = dumpread.load_dump(p)
        return head["camera"]

    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        cmd(c, 0.5)
    cmd(f"date jday {JD0:.9f}", 1.2)
    cmd(f"moveto lat {PHI} lon {LAM} alt {ALT} duration 0", 1.5)

    cells = {}
    for atm in (0, 1):
        for land in (0, 1):
            name = f"a{atm}l{land}"
            cmd(f"flag atmosphere {'on' if atm else 'off'}", 2.0)
            cmd(f"flag landscape {'on' if land else 'off'}", 6.0)
            c1, c2 = shot(f"{name}_ctl1"), shot(f"{name}_ctl2")
            floor = px(c1, c2)
            s0, d0 = shot(f"{name}_anchored"), dump(f"{name}_anchored")
            cmd("camera action free_mode state on", 1.5)
            s1, d1 = shot(f"{name}_free"), dump(f"{name}_free")
            cmd("camera action free_mode state off", 1.5)
            s2 = shot(f"{name}_anchored_2")
            E0, E1 = eye_of(d0["mat"]), eye_of(d1["mat"])
            chord_m = float(np.linalg.norm(E1 - E0)) * AU_KM * 1000
            cells[name] = {"atmosphere": atm, "landscape": land,
                           "px_ab": px(s0, s1), "px_pair": px(s0, s2),
                           "px_floor": floor, "chord_m": chord_m,
                           "lit_anchored": int((np.asarray(
                               Image.open(s0).convert("L"), dtype=int) > 8).sum()),
                           "lit_free": int((np.asarray(
                               Image.open(s1).convert("L"), dtype=int) > 8).sum())}
            print(f"  {name}: atm={atm} land={land}  px_ab={cells[name]['px_ab']:>9} "
                  f"floor={floor:>7} pair={cells[name]['px_pair']:>7} "
                  f"chord={chord_m:.3f} m  lit {cells[name]['lit_anchored']} -> "
                  f"{cells[name]['lit_free']}", flush=True)

    (out / f"f40_env_{a.tag}.json").write_text(json.dumps(
        {"tag": a.tag, "binary": a.bin, "place": [LAM, PHI, ALT], "cells": cells},
        indent=1))
    sess.stop(drv)
    return 0


if __name__ == "__main__":
    sys.exit(main())
