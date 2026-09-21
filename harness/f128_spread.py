#!/usr/bin/env python3
"""F128 - offline analyses behind INTENT 11.267's two findings (no engine, captures only).

  f128_spread.py pins  <capture dir> ...   rel of every pair of captures, grouped by the path each
                                           one says it DREW: same-pin spread, cross-pin spread
  f128_spread.py sense <dirA> <dirB>       what the metric resolves (dirB's layer turned / shifted
                                           against itself) and the best small transform of B towards A
  f128_spread.py clock <capture dir> ...   the OLD observer's longitude against the WALL clock of each
                                           dump (the galaxy-level start-clock dependence)
"""
import itertools
import json
import os
import sys
import time

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import f128_compare as C


def rel(x, y):
    return float(np.abs(x - y).sum() / (0.5 * (np.abs(x).sum() + np.abs(y).sum())))


def pins(dirs):
    caps = [(d, C.state(d, "base").get("drawnPath", "?")) for d in dirs]
    groups = {}
    for (a, pa), (b, pb) in itertools.combinations(caps, 2):
        m = C.compare(a, "base", b, "base")
        groups.setdefault("|".join(sorted((pa, pb))), []).append((m["rel"], m["max_delta"], m["px8"]))
    for k, v in sorted(groups.items()):
        rels = [x[0] for x in v]
        print("%-8s %3d pairs: rel min %.6f max %.6f, exactly 0 in %d, max delta %.0f, max px8 %d"
              % (k, len(v), min(rels), max(rels), sum(r == 0 for r in rels), max(x[1] for x in v), max(x[2] for x in v)))


def sense(dir_a, dir_b):
    from scipy import ndimage as ndi
    la, lb = C.layer(dir_a, "base")[1], C.layer(dir_b, "base")[1]
    d = lb - la
    print("A|B rel %.6f (luminance layers); pixels beyond half a level %d (%d up, %d down); mean %.5f"
          % (rel(la, lb), int((np.abs(d) > .5).sum()), int((d > .5).sum()), int((d < -.5).sum()), d.mean()))
    for deg in (0.01, 0.02, 0.05, 0.1, 0.5):
        print("B turned %.2f deg about the centre against itself: rel %.6f" % (deg, rel(lb, ndi.rotate(lb, deg, reshape=False, order=1))))
    for px in (0.05, 0.1, 0.5, 1.0):
        print("B shifted %.2f px against itself: rel %.6f" % (px, rel(lb, ndi.shift(lb, (px, 0), order=1))))
    best = (rel(la, lb), "none")
    for deg in np.arange(-0.2, 0.2001, 0.02):
        v = rel(la, ndi.rotate(lb, deg, reshape=False, order=1))
        if v < best[0]:
            best = (v, "turn %.2f deg" % deg)
    for dy in np.arange(-1, 1.01, 0.25):
        for dx in np.arange(-1, 1.01, 0.25):
            v = rel(la, ndi.shift(lb, (dy, dx), order=1))
            if v < best[0]:
                best = (v, "shift dy %.2f dx %.2f px" % (dy, dx))
    print("best small transform of B towards A: %s (rel %.6f)" % (best[1], best[0]))


def clock(dirs):
    rows = []
    for d in dirs:
        p = d + "/base.json"
        for ln in open(p):
            if ln.startswith('{"type":"header"'):
                h = json.loads(ln.replace("-nan", "null").replace("nan", "null").replace("inf", "1e308"))
                rows.append((os.path.getmtime(p), d, h["oldView"]["observer"]["longitude"], h["jd"], h["control"]["drawnPath"]))
                break
    rows.sort()
    t0, l0 = rows[0][0], rows[0][2]
    for t, d, lon, jd, path in rows:
        rate = (lon - l0) / (t - t0) * 3600 if t > t0 else 0.0
        print("%s drawn %s lon %.6f deg jd %.10f | +%.0f s wall, %+.5f deg, %.3f arcsec/s  %s"
              % (time.strftime("%H:%M:%S", time.localtime(t)), path, lon, jd, t - t0, lon - l0, rate, "/".join(d.split("/")[-2:])))


if __name__ == "__main__":
    if len(sys.argv) < 3 or sys.argv[1] not in ("pins", "sense", "clock"):
        print(__doc__)
        sys.exit(2)
    {"pins": pins, "clock": clock}.get(sys.argv[1], lambda a: sense(*a[:2]))(sys.argv[2:])
