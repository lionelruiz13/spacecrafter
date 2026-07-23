#!/usr/bin/env python3
"""B5 draw-half mode independence (INTENT 6.9 / 11.36 named limitation / 11.80).

Verifies that the NEW path DRAWS in the inGalaxy / inUniverse executor modes
(drawExperimental called from their draw), that a nested system RESOLVES
(drawNested px classification from the SUBSYSTEM geometry - the node's own
screenSize derives from boundingRadius = 0 for a bare system node), and that
the collapsed system is represented by the star-proxy dot (not gated on the
star's close-range halo channel; magnitude dims with distance, old-path
body_sun.cpp:86 parity).

Old-executor / camera facts this driver is built on (measured 2026-07-23):
  * solar executor mode reaches up to 1e16 m; entering inGalaxy clamps the
    OLD observer to 1e10 m and STAYS inGalaxy; a further dual-routed moveto
    above 1e14 m flips inGalaxy -> inUniverse, which re-bases the OLD
    observer to 1e9 m and needs one more moveto to clear the fade band.
  * free-mode `moveto altitude X` counts from the reference's
    getAltitudeReference() = datum_radius: at a MilkyWay reference every
    moveto lands at 3.2e9 AU + X - the not-yet-landed B10(c) system-node
    datum=0 default, confirmed live.  The NEW camera is therefore placed
    with `camera action descend coef <c>` (new-path-only, exact selDist
    scaling, B21) after each executor-steering moveto.
  * the camera keeps the surface view direction across the fly-out: aim by
    selecting + tracking the Sun once, then track OFF (the old tracking
    easing never settles and pollutes same-phase floors - measured 278 px).

Legs (all at init_fov=340, FISHEYE):
  * "earth" quad - RECORDED only (cross-path px at the day surface; at this
    fov the two paths measured 0 apart - the gal leg is the flag-liveness
    control instead: its OLD phase shows old bodies, its NEW phase the
    resolved nested interior, in EVERY build).
  * "gal" - solar executor, ref=MilkyWay @694 AU: nested SolarSystem ~36 px
    >= 16 -> RESOLVED interior.  cross>0, floors 0.
  * "galexec" - moveto 1.2e16 m ('->InGalaxy'), descend to ~8e4 AU:
    dot regime (px ~0.3, mag ~ -2.5).  PRE-FIX cross == 0 (InGalaxyModule
    never called the new path).  THE draw-half discriminator.
  * "uniband" - moveto 1.857e14 m x2 ('->InUniverse' + fade clear), descend
    to ~1340 AU: inside the B22 cross-fade band [16,24) px - interior AND
    dot compose.  PRE-FIX cross == 0.
Every leg: same-phase floors == 0, new-phase shot not all-black.

Usage: b5_run.sh b5_drawhalf.py [outdir]   (fresh launch, init_fov=340,
fisheye, enable_tcp; the runner owns config backup/restore)
"""
import socket, time, json, math, sys, os
import numpy as np
from PIL import Image

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "artifacts", "b5")
os.makedirs(OUT, exist_ok=True)
AU_M = 149597870700.0

results = []
def check(name, ok, detail):
    results.append({"name": name, "ok": bool(ok), "detail": detail})
    print(f"{'PASS' if ok else 'FAIL'}  {name}: {detail}", flush=True)

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def cam(path):
    with open(path) as f:
        return json.loads(f.readline())["camera"]

def bodies(path):
    out = {}
    with open(path) as f:
        for line in f:
            d = json.loads(line)
            if d.get("type") == "body" and d.get("new"):
                out[d["name"]] = d["new"]
    return out

def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)

def img(name):
    return np.asarray(Image.open(f"{OUT}/{name}.png").convert("RGB"), dtype=np.int16)

def px32(a, b):
    d = np.abs(a - b)
    mask = (d > 32).any(axis=2)
    n = int(mask.sum())
    if n:
        ys, xs = np.nonzero(mask)
        bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
    else:
        bbox = None
    return n, bbox

def phase_quad(sock, tag):
    """4 shots: n1, o1, n2, o2 (new/old alternating, reversible pair)."""
    send(sock, "flag experimental_path on", 2)
    shot(sock, f"{tag}_n1")
    send(sock, "flag experimental_path off", 2)
    shot(sock, f"{tag}_o1")
    send(sock, "flag experimental_path on", 2)
    shot(sock, f"{tag}_n2")
    send(sock, "flag experimental_path off", 2)
    shot(sock, f"{tag}_o2")
    send(sock, "flag experimental_path on", 2)   # leave pinned new
    n1, o1, n2, o2 = img(f"{tag}_n1"), img(f"{tag}_o1"), img(f"{tag}_n2"), img(f"{tag}_o2")
    cross1, bb1 = px32(n1, o1)
    cross2, bb2 = px32(n2, o2)
    fl_n, _ = px32(n1, n2)
    fl_o, _ = px32(o1, o2)
    notblack = int((n1.max(axis=2) > 16).sum())   # screen-fader guard
    print(f"  [{tag}] cross1={cross1} bbox={bb1}  cross2={cross2} bbox={bb2}"
          f"  floor_new={fl_n} floor_old={fl_o}  notblack={notblack}", flush=True)
    return cross1, cross2, fl_n, fl_o, bb1, bb2, notblack

applog = os.path.join(OUT, "app.log")
def log_has(needle):
    try:
        with open(applog, errors="replace") as f:
            return needle in f.read()
    except FileNotFoundError:
        return False

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461234.0", 1)          # noon UT (daylight surface baseline)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)                # kill the one random animator
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, f"body action dual_dump filename {OUT}/b5_base.json", 2)

b = bodies(f"{OUT}/b5_base.json")
n = lambda v: math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])
# scene_e_spine transcription of the repaired updateCache/updateReach formulas
nep_sub = 1.1 * b["Neptune"]["boundingRadius"]
sun_sub = 1.1 * (n(b["Neptune"]["ecl"]) + nep_sub)
sys_sub = 1.1 * sun_sub
sys_aoi = sys_sub * 16
print(f"predicted AoI [AU]: SolarSystem {sys_aoi:.1f}", flush=True)

# ---- surface quad, RECORDED only (see docstring) --------------------------
c1, c2, cf_n, cf_o, _, _, _ = phase_quad(s, "earth")
print(f"  [earth, recorded] cross={c1}/{c2} floors={cf_n}/{cf_o}", flush=True)

# ---- aim once, then release tracking --------------------------------------
send(s, "select planet Sun", 1)
send(s, "flag track_object on", 4)         # centre the system on-axis
send(s, "flag atmosphere off", 2)          # kills star scintillation (floors)
send(s, "camera action free_mode state on", 1)

# ---- "gal" leg: solar executor, ref=MilkyWay, resolved interior -----------
send(s, f"moveto altitude {int(sys_aoi*1.2*AU_M)} duration 0", 4)
send(s, "flag track_object off", 2)        # freeze the view (floor hygiene)
send(s, f"body action dual_dump filename {OUT}/b5_gal.json", 2)
c = cam(f"{OUT}/b5_gal.json")
check("gal_reference", c.get("reference") == "MilkyWay",
      f"reference={c.get('reference')!r} refDist={c.get('refDist')}")
g1, g2, gf_n, gf_o, gbb1, gbb2, gnb = phase_quad(s, "gal")
check("gal_resolved_draws", g1 > 0 and g2 > 0,
      f"cross-phase px32 {g1}@{gbb1} / {g2}@{gbb2} (= flag-liveness control, >0 in EVERY build)")
check("gal_floors", gf_n == 0,
      f"floor_new={gf_n} (old-phase floor RECORDED: {gf_o} px - old big-halo\n      re-entry easing at the view centre, old-path-only artifact)")
check("gal_notblack", gnb > 0, f"lit px={gnb}")

# ---- "galexec" leg: executor ->InGalaxy, dot regime ~8e4 AU ---------------
send(s, "moveto altitude 12000000000000000 duration 0", 4)   # 1.2e16 m: solar->galaxy flip
send(s, "camera action descend coef 0.005", 3)               # 3.2e9 -> 1.6e7 AU
send(s, "camera action descend coef 0.005", 3)               # -> 8.0e4 AU
send(s, f"body action dual_dump filename {OUT}/b5_galexec.json", 2)
c = cam(f"{OUT}/b5_galexec.json")
check("galexec_mode", log_has("->InGalaxy"), "app.log '->InGalaxy'")
check("galexec_reference", c.get("reference") == "MilkyWay"
      and 6e4 < (c.get("refDist") or 0) < 1e5,
      f"reference={c.get('reference')!r} refDist={c.get('refDist')} (want ~8.0e4 AU)")
x1, x2, xf_n, xf_o, xbb1, xbb2, xnb = phase_quad(s, "galexec")
check("galexec_new_draws", x1 > 0 and x2 > 0,
      f"cross-phase px32 {x1}@{xbb1} / {x2}@{xbb2} (pre-fix: 0)")
check("galexec_floors", xf_n == 0, f"floor_new={xf_n} (old floor recorded: {xf_o})")
check("galexec_notblack", xnb > 0, f"lit px={xnb}")

# ---- "uniband" leg: executor ->InUniverse, cross-fade band ~1340 AU -------
send(s, "moveto altitude 185700000000000 duration 0", 4)     # galaxy->universe flip
send(s, "moveto altitude 185700000000000 duration 0", 4)     # clear the entry fade band
send(s, "camera action descend coef 0.005", 3)               # 3.2e9 -> 1.6e7 AU
send(s, "camera action descend coef 0.005", 3)               # -> 8.0e4 AU
send(s, "camera action descend coef 0.01675", 3)             # -> ~1.34e3 AU (band px~19)
send(s, f"body action dual_dump filename {OUT}/b5_uniband.json", 2)
c = cam(f"{OUT}/b5_uniband.json")
check("uniband_mode", log_has("->InUniverse"), "app.log '->InUniverse'")
check("uniband_reference", c.get("reference") == "MilkyWay"
      and 1100 < (c.get("refDist") or 0) < 1550,
      f"reference={c.get('reference')!r} refDist={c.get('refDist')} (want in-band ~1340 AU)")
u1, u2, uf_n, uf_o, ubb1, ubb2, unb = phase_quad(s, "uniband")
check("uniband_new_draws", u1 > 0 and u2 > 0,
      f"cross-phase px32 {u1}@{ubb1} / {u2}@{ubb2} (pre-fix: 0)")
check("uniband_floors", uf_n == 0, f"floor_new={uf_n} (old floor recorded: {uf_o})")
check("uniband_notblack", unb > 0, f"lit px={unb}")

json.dump({"results": results,
           "earth_recorded": {"cross": [c1, c2], "floors": [cf_n, cf_o]}},
          open(f"{OUT}/b5_result.json", "w"), indent=1)
bad = [r for r in results if not r["ok"]]
print(f"=== {len(results)-len(bad)}/{len(results)} PASS ===", flush=True)
sys.exit(1 if bad else 0)
