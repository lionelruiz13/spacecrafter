#!/usr/bin/env python3
"""F18 / INTENT §5.52 - reads what f18_midband.py captured.

The measurement is the row THROUGH THE DISC CENTRE on the sum-of-RGB scale
(max 765) - the same scale §11.44/§11.123(g) recorded, so the numbers here are
directly comparable to the row's `178 720 725 ... 270`.

The disc centre is the image centre by construction (the body is tracked, and
under FISHEYE the tracked object sits at the centre of the projection circle);
the brightest-pixel offset is reported as the cross-check, never assumed away.

Verdict per body:
  GREEN  disc-centre row non-zero on BOTH paths
  RED    non-zero on the old path, zero on the new one  (= §5.52 reproduced)

Usage: f18_disc.py <outdir> [radius]
"""
import sys, os, json
import numpy as np
from PIL import Image

D = sys.argv[1]
R = int(sys.argv[2]) if len(sys.argv) > 2 else 4      # inside the hint ring
G = json.load(open(os.path.join(D, "geometry.json")))


def img(n):
    p = os.path.join(D, n + ".png")
    if not os.path.exists(p):
        return None
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int64)


def report(tag, name, rdisc=None):
    a = img(f"{tag}_{name}")
    if a is None:
        return {"missing": f"{tag}_{name}"}
    lum = a.sum(axis=2)
    h, w = lum.shape
    cy, cx = h // 2, w // 2
    row = lum[cy, cx - R - 1:cx + R + 2]
    win = lum[cy - 8:cy + 9, cx - 8:cx + 9]
    disc = None
    if rdisc:
        yy, xx = np.mgrid[0:h, 0:w]
        m = ((xx - cx) ** 2 + (yy - cy) ** 2) <= rdisc ** 2
        disc = {"r_px": round(rdisc, 3), "sum": int(lum[m].sum()),
                "mean": round(float(lum[m].mean()), 2),
                "npx": int(m.sum()), "nonzero": int((lum[m] > 0).sum())}
    ys, xs = np.nonzero(lum > 0)
    peak = None
    if len(xs):
        i = int(np.argmax(lum[ys, xs]))
        peak = [int(xs[i] - cx), int(ys[i] - cy), int(lum[ys[i], xs[i]])]
    return {"shape": [h, w], "centre_px": [int(cx), int(cy)], "disc": disc,
            "centre_rgb": [int(v) for v in a[cy, cx]],
            "centre_row_sumRGB": [int(v) for v in row],
            "row_max": int(row.max()), "row_nonzero": int((row > 0).sum()),
            "win17_nonzero": int((win > 0).sum()),
            "win17_max": int(win.max()),
            "brightest_offset_xy_val": peak}


out = {}
for target, info in G.items():
    if not isinstance(info, dict) or not info.get("in_band"):
        continue
    tag = target.lower()
    # Predicted disc RADIUS in px: screenSize is the diameter as a fraction of
    # the render width under FISHEYE (r = theta/halfFov on a circle of radius
    # viewportRadius = width/2), so r_px = screenSize * width / 2. Taken from
    # the app's own screenSize, never from the fov command.
    rdisc = info["geom"]["screenSize"] * 2048 / 2.0
    new = report(tag, "new", rdisc)
    old = report(tag, "old", rdisc)
    newb = report(tag, "new_b", rdisc)
    floor = None
    an, ab = img(f"{tag}_new"), img(f"{tag}_new_b")
    if an is not None and ab is not None and an.shape == ab.shape:
        floor = int(np.abs(an - ab).max())
    # The verdict rides the DISC-INTEGRATED luminance, not "any non-zero
    # pixel": a body with satellites or a ring has other content inside the
    # centre window (Jupiter's moons at 12 px, its ring), and a per-pixel
    # non-zero test calls that a drawn disc. Ratio >= 0.5 = the disc is there
    # on both paths; <= 0.05 with a non-empty old disc = §5.52 reproduced.
    verdict, ratio = "?", None
    if new.get("disc") and old.get("disc"):
        ratio = (new["disc"]["sum"] / old["disc"]["sum"]) if old["disc"]["sum"] else None
        verdict = ("BOTH-EMPTY" if not old["disc"]["sum"] and not new["disc"]["sum"] else
                   "NEW-ONLY" if not old["disc"]["sum"] else
                   "RED" if ratio <= 0.05 else
                   "GREEN" if ratio >= 0.5 else "PARTIAL")
    out[target] = {"screenSize": info["geom"]["screenSize"], "fov": info["fov"],
                   "verdict": verdict, "disc_ratio_new_over_old": ratio,
                   "noise_floor_max": floor,
                   "new": new, "old": old, "new_b": newb}

print(json.dumps(out, indent=2))
print()
for t, v in out.items():
    print(f"{t:10s} ss={v['screenSize']:.6f} fov={v['fov']:.3f} "
          f"VERDICT={v['verdict']:10s} disc_ratio="
          f"{'n/a' if v['disc_ratio_new_over_old'] is None else '%.4f' % v['disc_ratio_new_over_old']} "
          f"new_disc={v['new']['disc']['sum']} old_disc={v['old']['disc']['sum']} "
          f"new_row_max={v['new'].get('row_max')} old_row_max={v['old'].get('row_max')} "
          f"floor={v['noise_floor_max']}")
