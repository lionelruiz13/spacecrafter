#!/usr/bin/env python3
"""F109 - the B22 in-band decomposition, refined, plus the FLOOR GEOMETRY read
off the dump itself (INTENT 11.231; instrument of record for 11.82(b)'s C0).

Why this exists beside b22_live_analyze.py (which stays the verdict of record,
unmodified): that script's (C) reports the alpha-independent term as the
INTERCEPT of a straight line fitted to the 8 in-band points, minus the MEAN of
the below-band points. Two things ride in that number besides any disc floor:

  (i) the proxy dot's own brightening as the observer closes in - the
      below-band mean sits at px ~15.0 while the fit's intercept sits at
      px 16.0, and addI_dot is not flat in px;
  (ii) any curvature of addI(t) - a concave ramp fitted by a line lands its
      intercept ABOVE the true t->0 value.

So this script adds an ENDPOINT-ANCHORED model with no free parameters:
addI_model(px, t) = D(px)*(1-t) + I(px)*t, where D is a power law fitted to
the below-band (pure-dot) points and I a power law fitted to the above-band
(pure-resolved) points - each component evaluated at the point's OWN px, so
the dot's and the interior's distance dependence are removed rather than
absorbed. The residual measured - model is then what the linear fit calls C0
minus those two effects: a curvature hump if the ramp is nonlinear, a step at
t->0 if a floor really emits alpha-independently.

FLOOR GEOMETRY (the F109 attribution question): every candidate floor's firing
condition is decidable from the dump alone, so it is REPORTED PER SWEEP POINT
rather than assumed -

  StarModule.cpp:35  if (rmag < screenR * 2.f)   rmag_raw = sunHaloSize/2/sqrt(d)
  StarModule.cpp:39  if (rmag < 32.f)            screenR  = screenSize*2*viewportRadius
  ModularBody.hpp:1936 if (rmag < screen_r)      screen_r = the body's own disc px;
                                                 rmag >= 1.2 by construction in
                                                 BOTH branches above the test
                                                 (anti-blink sets 1.2; the
                                                 size-limit branch starts >= 1.2)

Usage: f109_decomp.py <artifacts_dir> [--label NAME] [--json OUT.json]
"""
import json, math, sys, os
import numpy as np
from PIL import Image

T, B, VR, R = 16.0, 8.0, 1024.0, 45
SUN_HALO_SIZE = 200.0          # StarModule::sunHaloSize default (old setFlagSunScale(false) base)
RMAG_MIN = 1.2                 # drawHaloCore: rmag >= 1.2 leaving both branches

args = [a for a in sys.argv[1:]]
OUT = args[0]
label = "run"
jout = None
if "--label" in args: label = args[args.index("--label") + 1]
if "--json" in args: jout = args[args.index("--json") + 1]

sw = json.load(open(f"{OUT}/sweep.json"))
SUB = sw["sys_sub"]
rows = {r["tag"]: r for r in sw["rows"]}
N = sum(1 for k in rows if k.startswith("dn"))

def img(nm): return np.asarray(Image.open(f"{OUT}/{nm}").convert("RGB"), dtype=np.int16)
def px_of(d, hf): return math.atan(SUB/math.sqrt(d*d-SUB*SUB))/hf*2*VR if d > SUB else 2*VR
def cen(s): nx, ny = s; return int((nx*0.5+0.5)*2048), int((0.5-ny*0.5)*2048)
def crop(a, cx, cy): return a[cy-R:cy+R, cx-R:cx+R]

def sun_of(tag):
    """the Sun's dump record at this sweep point (radius + distance + halfFov)."""
    for line in open(f"{OUT}/{tag}.json"):
        d = json.loads(line)
        if d.get("type") == "body" and d.get("name") == "Sun" and d.get("new"):
            return d["new"]
    return None

def max_disc_px(tag, hf):
    """largest body disc on screen (px) among the dumped new-path bodies:
    screen_r = screenSize*2*VR with screenSize = halfAngularSize/halfFov."""
    best, who = 0.0, None
    for line in open(f"{OUT}/{tag}.json"):
        d = json.loads(line)
        if d.get("type") != "body" or not d.get("new"): continue
        n = d["new"]; r = n.get("scaledDatumRadius") or 0.0; dist = n.get("dist") or 0.0
        if r <= 0 or dist <= r: continue
        px = math.atan(r/math.sqrt(dist*dist - r*r))/hf*2*VR
        if px > best: best, who = px, d["name"]
    return best, who

def addI(tag):
    r = rows[tag]; cx, cy = cen(r["solsys_screen"])
    n = crop(img(r["shots"]["new"]), cx, cy)
    o = crop(img(r["shots"]["old"]), cx, cy)
    return int(np.clip(n - o, 0, None).sum()), int((n >= 255).sum())

print(f"== F109 decomposition [{label}]  {OUT}")
print(f"subsystemRadius={SUB:.4f} AU   band px=[{T},{T+B})   crop {2*R}x{2*R}")

# ---- reference gate, per point (an off-reference point is EXCLUDED, loudly) --
pts, excluded = [], []
for i in range(N):
    tag = f"dn{i:02d}"; r = rows[tag]
    px = px_of(r["refDist"], r["halfFov"])
    rec = {"tag": tag, "refDist": r["refDist"], "px": px, "halfFov": r["halfFov"],
           "reference": r["reference"]}
    if r["reference"] != "MilkyWay":
        excluded.append(rec); continue
    rec["addI"], rec["sat"] = addI(tag)
    pts.append(rec)
if excluded:
    print(f"\n!! EXCLUDED {len(excluded)} off-reference point(s) - the sweep must stay "
          f"referenced to MilkyWay (a capture into SolarSystem is a different scene):")
    for e in excluded:
        print(f"     {e['tag']} reference={e['reference']} refDist={e['refDist']:.3f} px={e['px']:.3f}")
print(f"   {len(pts)} of {N} descend points retained")

below = [p for p in pts if p["px"] < T]
band  = [p for p in pts if T <= p["px"] < T+B]
above = [p for p in pts if p["px"] >= T+B]

# ---- floor geometry, measured per point ------------------------------------
print("\n[floors] which candidate floor CAN fire, from the dump's own numbers")
print("  px      d(AU)    screenR(px)  rmag_raw  2*screenR  32f   maxDisc(px)      C:rmag>=1.2 vs screen_r")
fl = {"screenR2": 0, "hard32": 0, "halocore": 0, "n": 0}
for p in pts:
    s = sun_of(p["tag"])
    if not s: continue
    d = s["dist"]; r = s["scaledDatumRadius"]; hf = p["halfFov"]
    screenR = math.atan(r/math.sqrt(d*d - r*r))/hf*2*VR
    rmag_raw = SUN_HALO_SIZE/2.0/math.sqrt(d)
    f1 = rmag_raw < screenR*2.0
    f2 = (rmag_raw if not f1 else screenR*2.0) < 32.0
    md, who = max_disc_px(p["tag"], hf)
    f3 = md > RMAG_MIN
    fl["n"] += 1; fl["screenR2"] += f1; fl["hard32"] += f2; fl["halocore"] += f3
    p.update({"screenR_px": screenR, "rmag_raw": rmag_raw, "fires_screenR2": bool(f1),
              "fires_32": bool(f2), "max_disc_px": md, "max_disc_body": who,
              "fires_halocore": bool(f3)})
    print(f"  {p['px']:6.3f} {d:8.2f}  {screenR:10.3e}  {rmag_raw:8.4f}  "
          f"{'FIRES' if f1 else '  -  '}      {'FIRES' if f2 else '  -  '}  "
          f"{md:8.3e} ({who[:9] if who else '-':9s})  {'FIRES' if f3 else '  -  '}")
print(f"  -> firing counts over {fl['n']} points: screenR*2 {fl['screenR2']}   "
      f"32.f {fl['hard32']}   drawHaloCore screen_r {fl['halocore']}")

# ---- the b22_live_analyze metric, reproduced -------------------------------
print("\n[A] linear-fit metric (same form as b22_live_analyze.py (C))")
print("  px      aRes     addI      sat255")
for p in band:
    print(f"  {p['px']:6.3f}  {(p['px']-T)/B:.4f}  {p['addI']:8d}  {p['sat']:5d}")
seq = [p["addI"] for p in band]
mono = all(seq[k] <= seq[k+1]+1 for k in range(len(seq)-1))
A = np.array([[(p["px"]-T)/B, 1] for p in band]); y = np.array([p["addI"] for p in band], float)
(c1, c0), _, _, _ = np.linalg.lstsq(A, y, rcond=None)
mb = float(np.mean([p["addI"] for p in below])); ma = float(np.mean([p["addI"] for p in above]))
swing = ma - mb; pop = c0 - mb
print(f"  monotone non-decreasing: {mono}")
print(f"  pure-dot mean (px<T, n={len(below)})       = {mb:.0f}")
print(f"  pure-resolved mean (px>=T+B, n={len(above)}) = {ma:.0f}")
print(f"  IN-BAND fit: addI = {c0:.0f} + {c1:.0f}*aRes")
print(f"  swing = {swing:.0f};  ALPHA-INDEPENDENT pop at T = {pop:.0f} = {100*pop/swing:.1f}% of swing")
print(f"  alpha-linear share = {100*c1/swing:.1f}%")

# ---- the endpoint-anchored model -------------------------------------------
def plaw(ps):
    if len(ps) < 2: return None
    X = np.array([[math.log(p["px"]), 1.0] for p in ps])
    Y = np.array([math.log(p["addI"]) for p in ps])
    (k, b), _, _, _ = np.linalg.lstsq(X, Y, rcond=None)
    return float(k), float(math.exp(b))
pd_, pi_ = plaw(below), plaw(above)
print("\n[B] endpoint-anchored model  addI = D(px)*(1-t) + I(px)*t   (no free parameter)")
if pd_ and pi_:
    kd, ad = pd_; ki, ai = pi_
    print(f"  D(px) = {ad:.1f}*px^{kd:.4f}   (pure dot, fitted below band)")
    print(f"  I(px) = {ai:.1f}*px^{ki:.4f}   (pure resolved, fitted above band)")
    print("  px      t       measured   model     resid    resid/swing")
    res = []
    for p in band:
        t = (p["px"]-T)/B
        D = ad*p["px"]**kd; I = ai*p["px"]**ki
        m = D*(1-t) + I*t
        res.append((t, p["addI"]-m))
        print(f"  {p['px']:6.3f}  {t:.4f}  {p['addI']:9d}  {m:8.0f}  {p['addI']-m:+7.0f}   "
              f"{100*(p['addI']-m)/swing:+6.1f}%")
    # the model's own reading of the three effects inside `pop`
    t0 = 0.0
    D0 = ad*T**kd
    print(f"  D(T)={D0:.0f} vs pure-dot mean {mb:.0f}  -> the dot's own brightening "
          f"below->T accounts for {D0-mb:+.0f} = {100*(D0-mb)/swing:+.1f}% of the swing")
    # extrapolate the residual hump to t=0 with a quadratic through the band residuals
    ts = np.array([r[0] for r in res]); rs = np.array([r[1] for r in res])
    q = np.polyfit(ts, rs, 2)
    print(f"  residual hump, quadratic in t: peak {rs.max():+.0f} at t={ts[int(np.argmax(rs))]:.3f}; "
          f"extrapolated to t=0: {np.polyval(q, 0.0):+.0f} = {100*np.polyval(q,0.0)/swing:+.1f}% of swing")
    print(f"  => LINEAR-FIT pop {pop:.0f} = dot-brightening {D0-mb:+.0f} + curvature/step "
          f"{pop-(D0-mb):+.0f} (the part a floor could own)")
else:
    print("  (need >=2 below-band and >=2 above-band points)")

# ---- [C] the metric's own 8-bit clipping --------------------------------------
# addI sums 8-bit channel values. A channel whose unclipped value exceeds 255 is
# recorded as 255, so S(a) = sum(min(a*V,255)) is CONCAVE in a: at small a
# nothing clips and S(a) = a*sum(V), at a=1 the clipped core loses
# sum(V)-sum(min(V,255)). Fitting a straight line to a concave ramp puts the
# intercept ABOVE the true a->0 value - i.e. clipping alone manufactures an
# "alpha-independent" term. Discriminator with no launch: recompute every
# quantity with the channels that EVER clip in this sweep masked out. If the pop
# collapses, the pop was the metric's, not the render's.
clipmask = None
for p in pts:
    r = rows[p["tag"]]; cx, cy = cen(r["solsys_screen"])
    n = crop(img(r["shots"]["new"]), cx, cy)
    m = (n >= 255)
    clipmask = m if clipmask is None else (clipmask | m)
print(f"\n[C] clipping-robust re-read: {int(clipmask.sum())} of {clipmask.size} channels reach "
      f"255 somewhere in the sweep ({100.0*clipmask.sum()/clipmask.size:.2f}%)")
def addI_masked(tag):
    r = rows[tag]; cx, cy = cen(r["solsys_screen"])
    n = crop(img(r["shots"]["new"]), cx, cy); o = crop(img(r["shots"]["old"]), cx, cy)
    d = np.clip(n - o, 0, None)
    return int(np.where(clipmask, 0, d).sum())
mpts = {p["tag"]: addI_masked(p["tag"]) for p in pts}
mb2 = float(np.mean([mpts[p["tag"]] for p in below]))
ma2 = float(np.mean([mpts[p["tag"]] for p in above]))
Am = np.array([[(p["px"]-T)/B, 1] for p in band]); ym = np.array([mpts[p["tag"]] for p in band], float)
(c1m, c0m), _, _, _ = np.linalg.lstsq(Am, ym, rcond=None)
sw2 = ma2 - mb2; pop2 = c0m - mb2
print(f"  unclipped-only: dot={mb2:.0f} resolved={ma2:.0f} swing={sw2:.0f} "
      f"fit={c0m:.0f}+{c1m:.0f}*aRes")
print(f"  unclipped-only pop = {pop2:.0f} = {100*pop2/sw2:.1f}% of swing "
      f"(all-channel pop was {pop:.0f} = {100*pop/swing:.1f}%)")
print(f"  => {100*(pop - pop2*swing/sw2)/pop:.0f}% of the linear-fit pop is carried by the "
      f"{int(clipmask.sum())} clipped channels")

if jout:
    json.dump({"label": label, "dir": OUT, "sub": SUB, "excluded": excluded,
               "unclipped": {"below_mean": mb2, "above_mean": ma2, "swing": sw2,
                             "c0": float(c0m), "c1": float(c1m), "pop": float(pop2),
                             "pop_pct": float(100*pop2/sw2),
                             "clipped_channels": int(clipmask.sum()),
                             "per_point": mpts},
               "points": pts, "fit": {"c0": float(c0), "c1": float(c1)},
               "below_mean": mb, "above_mean": ma, "swing": swing, "pop": float(pop),
               "pop_pct": float(100*pop/swing), "monotone": bool(mono),
               "floor_firings": fl},
              open(jout, "w"), indent=1)
    print(f"\njson -> {jout}")
