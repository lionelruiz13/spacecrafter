#!/usr/bin/env python3
"""B29 screen A/B analysis - the terminal observable (INTENT §11.65, §11.52(b)).

A color change is a pixel change. Per path we measure base->after under the
same recolor-all-halo-red command (both paths get it via the dual-write seam),
against the same-state noise floor. px>N with N calibrated over the floor.

    ./b29_screen.py [dir]
"""
import sys, os, json
import numpy as np
from PIL import Image

D = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "artifacts", "b29")


def load(n):
    return np.asarray(Image.open(os.path.join(D, n + ".png")).convert("RGB"), dtype=np.int16)


def diff(a, b):
    d = np.abs(load(a) - load(b)).max(axis=2)
    return {f"px>{t}": int((d > t).sum()) for t in (0, 2, 8, 16, 32, 64)} | {
        "max": int(d.max()), "mean": round(float(d.mean()), 5)}


def redshift(a, b):
    """Mean per-channel change base->after; a halo recolored white->red raises R,
    drops G/B where the halo sits."""
    da = load(b).astype(np.float32) - load(a).astype(np.float32)
    return [round(float(da[..., c].mean()), 4) for c in range(3)]


R = {}
R["noise_floor_new"] = diff("g_ctrl_new", "g_base_new")     # same state, twice
R["new_base_vs_after"] = diff("g_base_new", "g_after_new")  # new path responds
R["old_base_vs_after"] = diff("g_base_old", "g_after_old")  # old path responds
R["new_redshift_rgb"] = redshift("g_base_new", "g_after_new")
R["old_redshift_rgb"] = redshift("g_base_old", "g_after_old")
R["old_persist_after_vs_initial"] = diff("g_after_old", "h_initial_old")  # ~0 = persist

with open(os.path.join(D, "b29_screen.json"), "w") as f:
    json.dump(R, f, indent=2)
print(json.dumps(R, indent=2))
