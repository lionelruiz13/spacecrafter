#!/usr/bin/env python3
"""B26 case analyzer - dual-path default flip (INTENT 11.53 / 11.50(c)).

    ./b26_analyze.py <tag> [dir]

Reports, per case:
  * freeze witness            jd of the two dual_dump headers (must be equal)
  * instrument noise floor    pin_new vs pin_new2 (same state, 3 s apart)
  * counterfactual            pin_new vs pin_old (a pair KNOWN to differ)
  * the lag sweep             fraction of pairs that differ at 1.0/2.0/2.5/3.0 s
  * per-shot path class       each burst shot against the pinned references

CRITERION (corrected at 11.53 - the 11.50(c) wording is unsafe):
  * discriminator = px>32 (count of pixels with any channel |delta| > 32).
    Measured: 0 for EVERY same-path pair (incl. the new-path micro-residual),
    133..136 for EVERY cross-path pair - a separation with no overlap, while
    max|delta| and px>8 overlap.
  * separation lag = an ODD multiple of 1.0 s.  The alternation is a 1000 ms
    square wave (ssystem_factory.cpp:454-459), i.e. period 2.0 s, so:
        1.0 s -> always opposite phase      (20/20 pairs differed)
        2.0 s -> always the SAME phase      ( 0/16 pairs differed)
        2.5 s -> quarter-period offset      ( 7/14 pairs differed - a coin flip)
        3.0 s -> always opposite phase      (12/12 pairs differed)
    "two shots >= 2.5 s apart must differ under alternate" is therefore a 50%
    test, not a test.
"""
import sys, json, os
import numpy as np
from PIL import Image

TAG = sys.argv[1]
D = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "artifacts", "b26")
N = 24
SPACING = 0.25   # s between burst shots


def load(n):
    return np.asarray(Image.open(os.path.join(D, f"{TAG}_{n}.png")).convert("RGB"), dtype=np.int16)


def st(a, b):
    d = np.abs(a - b)
    return (int(d.max()), float(d.mean()), int(d.any(axis=2).sum()),
            int((d > 8).any(axis=2).sum()), int((d > 32).any(axis=2).sum()))


def line(lbl, a, b):
    mx, mn, p0, p8, p32 = st(a, b)
    print(f"  {lbl:<36} max|d|={mx:3d}  mean|d|={mn:.7f}  px>0={p0:7d}  px>8={p8:7d}  px>32={p32:6d}")


burst = [load(f"stab{i:02d}") for i in range(N)]
pin_old, pin_new, pin_new2 = load("pin_old"), load("pin_new"), load("pin_new2")

print(f"=== case {TAG} ===  ({D})")
try:
    jd = [json.loads(open(os.path.join(D, f"{TAG}_{w}.json")).readline())["jd"] for w in ("jd1", "jd2")]
    print(f"  freeze witness: jd1={jd[0]!r} jd2={jd[1]!r} equal={jd[0] == jd[1]}")
except FileNotFoundError:
    print("  freeze witness: dumps not kept for this run")

print("  -- instrument --")
line("noise floor  pin_new/pin_new2", pin_new, pin_new2)
line("counterfactual pin_new/pin_old", pin_new, pin_old)
a = burst[0]
print(f"  content: stab00 mean RGB={a.mean(axis=(0, 1)).round(3)} max={int(a.max())} "
      f"nonzero px={int(a.any(axis=2).sum())}/{a.shape[0] * a.shape[1]} shape={a.shape}")

print("  -- lag sweep (a pair DIFFERS iff px>32 is nonzero) --")
for lag in (4, 8, 10, 12):
    res = [st(burst[k], burst[k + lag]) for k in range(N - lag)]
    nd = sum(1 for r in res if r[4] > 0)
    print(f"    lag {lag * SPACING:.2f} s : {nd}/{len(res)} pairs differ; "
          f"px>32 in [{min(r[4] for r in res)},{max(r[4] for r in res)}]  "
          f"max|d| in [{min(r[0] for r in res)},{max(r[0] for r in res)}]  "
          f"px>8 in [{min(r[3] for r in res)},{max(r[3] for r in res)}]")

print("  -- per-shot path class (pixel channel) --")
cls = []
for i, im in enumerate(burst):
    dn, do = st(im, pin_new), st(im, pin_old)
    cls.append("N" if dn[3] < do[3] else "O")
print("    " + "".join(cls) + f"   (shot spacing {SPACING} s; N=new path drawn, O=old)")
