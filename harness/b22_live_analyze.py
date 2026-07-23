#!/usr/bin/env python3
"""B22 deliverable-1 analysis (INTENT 11.64 / 13.B B22). Reads the b22_live.py
sweep (artifacts dir) and reports, on the LIVE composed screen:

 (A) NO HYSTERESIS: descend-new vs ascend-new at matched refDist. The cross-fade
     alpha is a pure function of px, so same px -> same screen regardless of
     approach direction. Discriminator: matched crops bit-identical (maxCh 0),
     while a T-crossing mismatch is well above the B30 floor.

 (B) MONOTONE cross-fade signal: the drawHaloCore component (dot + Sun point-halo)
     that drawAlpha controls fades in/out monotonically across the band, isolated
     by NEW-vs-OLD differencing (added intensity over a crop at the SolarSystem
     centre). Reversible pair confirms hysteresis-free.

 (C) BIG-HALO PARTICIPATION (INTENT 11.64/11.81 B22-fix): decomposing the in-band
     added intensity as addI = C0 + C1*aRes measures how much of the dot->resolved
     swing is a hard POP at T (the alpha-INDEPENDENT floor C0-pure_dot) vs a smooth
     cross-dissolve (the alpha-linear C1). Pre-fix the big-halo glow (dominant
     resolved visual) was UNFADED -> ~80% pop (11.64(c) 'endpoints pixel-exact'
     falsified live). The B22-fix scales the big-halo on `color` (not cmag:
     sun_big_halo.frag FLOORS cmag via max(1,cmag+0.1) and its procedural nearHalo
     disc ignores cmag) so it fades linearly -> pop drops to ~10% (a genuine
     cross-dissolve; the residual is the 11.64(a) small disc-floor/near-continuous
     step). This script prints the pop% so any binary re-validates.

Usage: b22_live_analyze.py <artifacts_dir>
"""
import json, math, sys
import numpy as np
from PIL import Image

OUT = sys.argv[1]
T, B = 16.0, 8.0                 # SYSTEM_VISIBILITY_SUBSYSTEM_SIZE, CROSSFADE_BAND
VR = 1024.0                      # viewportRadius = render_size/2
R = 45                           # crop half-size around the SolarSystem centre
sw = json.load(open(f"{OUT}/sweep.json")); SUBSYS = sw["sys_sub"]
rows = {r["tag"]: r for r in sw["rows"]}
def img(nm): return np.asarray(Image.open(f"{OUT}/{nm}").convert("RGB"), dtype=np.int16)
def px_of(d, hf): return math.atan(SUBSYS/math.sqrt(d*d-SUBSYS*SUBSYS))/hf*2*VR if d > SUBSYS else 2*VR
def cen(scr): nx, ny = scr; return int((nx*0.5+0.5)*2048), int((0.5-ny*0.5)*2048)
def crop(a, cx, cy): return a[cy-R:cy+R, cx-R:cx+R]
def addI(tag):       # NEW-vs-OLD added intensity in the crop (drawNested contribution)
    r = rows[tag]; cx, cy = cen(r["solsys_screen"])
    d = crop(img(r["shots"]["new"]), cx, cy) - crop(img(r["shots"]["old"]), cx, cy)
    return int(np.clip(d, 0, None).sum())

print(f"subsystemRadius={SUBSYS:.3f} AU  band px=[{T},{T+B})")

# reference-stability gate (no capture into SolarSystem anywhere in the sweep)
refs = set(r["reference"] for r in sw["rows"])
print(f"\n[gate] references across whole sweep: {refs}  {'OK (all MilkyWay)' if refs=={'MilkyWay'} else 'FAIL'}")

# ---- (A) NO HYSTERESIS ------------------------------------------------------
print("\n(A) NO HYSTERESIS  (descend-new vs ascend-new at matched refDist)")
def full_px32(a, b): return int((np.abs(img(a)-img(b)) > 32).any(axis=2).sum())
def crop_max(a_tag, b_tag):
    ra, rb = rows[a_tag], rows[b_tag]
    ca = cen(ra["solsys_screen"]); cb = cen(rb["solsys_screen"])
    d = np.abs(crop(img(ra["shots"]["new"]), *ca) - crop(img(rb["shots"]["new"]), *cb))
    return int(d.max())
match, mism = [], []
for i in range(len(sw["rows"])//3 if False else 14):
    dn, up = rows[f"dn{i:02d}"], rows[f"up{i:02d}"]
    m = full_px32(dn["shots"]["new"], up["shots"]["new"]); match.append(m)
    cm = crop_max(f"dn{i:02d}", f"up{i:02d}")
    print(f"  refDist {dn['refDist']:7.1f} px {px_of(dn['refDist'],dn['halfFov']):6.2f}"
          f"  full-image px32(dn,up)={m}  crop maxCh={cm}")
# discriminating mismatch: compare across the T boundary
tcross = crop_max("dn02", "up03")   # px 15.7 vs 16.4 - straddles T, the one big screen change
print(f"  MATCHED (opp dir, same refDist): max px32={max(match)}  -> {'NO HYSTERESIS' if max(match)==0 else 'HYSTERESIS'}")
print(f"  DISCRIMINATOR mismatch dn02(px15.7) vs up03(px16.4) across T: crop maxCh={tcross} (>> 0 -> instrument can fail)")

# ---- (B) MONOTONE + (C) BIG-HALO decomposition ------------------------------
print("\n(B) MONOTONE cross-fade signal + (C) BIG-HALO POP decomposition")
band = []
for i in range(14):
    r = rows[f"dn{i:02d}"]; px = px_of(r["refDist"], r["halfFov"])
    if T <= px < T+B: band.append((px, (px-T)/B, addI(f"dn{i:02d}")))
band.sort()
print("  px      aRes    addI(new-old)")
for px, a, ai in band: print(f"  {px:6.3f}  {a:.3f}   {ai}")
seq = [ai for _, _, ai in band]
mono = all(seq[k] <= seq[k+1]+1 for k in range(len(seq)-1))
print(f"  in-band addI monotone non-decreasing: {mono}")
A = np.array([[a, 1] for _, a, _ in band]); y = np.array([ai for _, _, ai in band], float)
(c1, c0), _, _, _ = np.linalg.lstsq(A, y, rcond=None)
below = [addI(f"dn{i:02d}") for i in range(14) if px_of(rows[f'dn{i:02d}']['refDist'], rows[f'dn{i:02d}']['halfFov']) < T]
above = [addI(f"dn{i:02d}") for i in range(14) if px_of(rows[f'dn{i:02d}']['refDist'], rows[f'dn{i:02d}']['halfFov']) >= T+B]
print(f"\n  pure-dot addI (px<T)        mean={np.mean(below):.0f}")
print(f"  pure-resolved addI (px>=T+B) mean={np.mean(above):.0f}")
print(f"  IN-BAND fit: addI = {c0:.0f} + {c1:.0f}*aRes")
swing = np.mean(above) - np.mean(below)
pop = c0 - np.mean(below)
poppct = 100*pop/swing
print(f"  dot->resolved swing = {swing:.0f};  ALPHA-INDEPENDENT pop at T = {pop:.0f} "
      f"= {poppct:.0f}% of the swing")
print(f"  cross-fade (alpha-linear) share = {100*c1/swing:.0f}% of the swing")
if poppct <= 20:
    print(f"\n  => (C) VERDICT: big-halo PARTICIPATES in the fade (pop {poppct:.0f}% <= 20%) -")
    print("     the collapse is a genuine cross-dissolve; residual = 11.64(a) disc-floor step.")
else:
    print(f"\n  => (C) VERDICT: cross-fade INCOMPLETE (pop {poppct:.0f}% > 20%) - a dominant")
    print("     resolved component is UNFADED (pre-B22-fix: the StarModule big-halo, cmag-floored).")
