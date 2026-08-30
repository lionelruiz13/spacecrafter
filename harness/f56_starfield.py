#!/usr/bin/env python3
"""F56 — the star-field channel across the dim boundary (P7), and the A/A floor
re-measured on the healthy stack (P8).

F45's phase C writes a deterministic star-field frame: `deselect`, atmosphere /
landscape / milky-way / nebula / constellation-drawing / star-names / planets OFF,
`flag stars on`, `date utc 2020-01-01T22:00:00`, `timerate rate 0`, then the app's own
2048^2 readback.  Nine of those frames were captured on 2026-08-29 (the dim era) and
this task captured two more today from the SAME unmodified script.

Why the channel is worth a comparison: §11.159(a) established that the star pixels are
coloured from `HipStarMgr::color_table` (`zone_array.cpp:293…`), i.e. they do not pass
through the `s_texture` entry the Moon's two render paths share.  So a dim state
confined to textured-body rendering leaves this frame alone, while a global output
transform darkens it.  §11.174's chase never had a non-Moon photometric channel from
the dim day.

Statistics, per pair, over the full 2048x2048 RGB frame:
  n_diff        pixels differing at all
  n_diff_gt3    pixels whose MAX CHANNEL delta exceeds 3/255  (§11.159(k)(4)'s form)
  max_delta     the largest single-channel difference
  lit_gt8       lit-pixel count per frame (L > 8) — the member a global dimming moves
  lit_mean      mean L over the union of the two frames' lit pixels
  lit_ratio     lit_mean(b) / lit_mean(a)

usage: f56_starfield.py <out.json> <label=path> [<label=path> ...]
"""
import hashlib
import itertools
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

LMIN = 8


def load(p):
    im = Image.open(p).convert("RGB")
    rgb = np.asarray(im, dtype=np.int16)
    L = np.asarray(im.convert("L"), dtype=np.float64)
    return rgb, L


def main():
    argv = sys.argv[1:]
    if len(argv) < 3:
        print(__doc__)
        return 2
    out = Path(argv[0])
    frames = {}
    for spec in argv[1:]:
        label, path = spec.split("=", 1)
        rgb, L = load(path)
        frames[label] = {"path": path, "rgb": rgb, "L": L,
                         "md5": hashlib.md5(Path(path).read_bytes()).hexdigest(),
                         "shape": list(rgb.shape)}
    res = {"frames": {}, "pairs": []}
    for lab, f in frames.items():
        lit = f["L"] > LMIN
        res["frames"][lab] = {
            "path": f["path"], "md5": f["md5"], "shape": f["shape"],
            "frame_mean_L": round(float(f["L"].mean()), 4),
            "lit_gt8": int(lit.sum()),
            "lit_mean_L": round(float(f["L"][lit].mean()), 4) if lit.any() else None,
            "max_L": int(f["L"].max())}
        print(f"{lab:<22} md5 {f['md5']}  lit>8 {int(lit.sum()):>7}  "
              f"lit_mean {res['frames'][lab]['lit_mean_L']}  "
              f"frame_mean {res['frames'][lab]['frame_mean_L']}")
    print()
    for a, b in itertools.combinations(frames, 2):
        fa, fb = frames[a], frames[b]
        if fa["shape"] != fb["shape"]:
            res["pairs"].append({"a": a, "b": b, "error": "shape"})
            continue
        d = np.abs(fb["rgb"] - fa["rgb"])
        dmax = d.max(axis=2)
        lit = (fa["L"] > LMIN) | (fb["L"] > LMIN)
        rec = {"a": a, "b": b,
               "identical": fa["md5"] == fb["md5"],
               "n_diff": int((dmax > 0).sum()),
               "n_diff_gt3": int((dmax > 3).sum()),
               "max_delta": int(dmax.max()),
               "lit_union": int(lit.sum()),
               "lit_mean_a": round(float(fa["L"][lit].mean()), 4),
               "lit_mean_b": round(float(fb["L"][lit].mean()), 4)}
        rec["lit_ratio_b_over_a"] = round(rec["lit_mean_b"] / rec["lit_mean_a"], 6) \
            if rec["lit_mean_a"] else None
        res["pairs"].append(rec)
        print(f"{a:<22} vs {b:<22} identical={rec['identical']!s:<5} "
              f"n_diff {rec['n_diff']:>7} n_diff>3 {rec['n_diff_gt3']:>7} "
              f"max_delta {rec['max_delta']:>3} lit_ratio {rec['lit_ratio_b_over_a']}")
    out.write_text(json.dumps(res, indent=1))
    print(f"\n-> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
