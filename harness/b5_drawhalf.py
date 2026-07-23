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
    getAltitudeReference() = datum_radius.  B10-datum0 LANDED (INTENT
    11.75(a)): a MilkyWay reference (a ModularSystem) now has datum=0, so a
    moveto lands the new camera at X directly (was 3.2e9 AU + X, §11.80).
    The galexec/uniband legs therefore place the new camera by a DIRECT
    moveto (the new capability) AND still exercise `camera action descend
    coef <c>` (new-path-only exact selDist scaling, B21) - both routes green.
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

# =====================================================================
# B10-datum0 LANDED (INTENT 11.75(a)): at a MilkyWay reference free-mode
# `moveto altitude X` now lands at X (getAltitudeReference()==scaledDatumRadius
# ==0), so the executor-mode observation altitudes are reachable by a DIRECT
# moveto - no longer only via the descend workaround. This driver keeps BOTH
# placement routes green (the B10-datum0 regression the row asked for):
#   * DIRECT moveto (the NEW leg): one moveto lands the new camera at the target
#     altitude. The metre value is still dual-routed to the OLD observer, so it
#     ALSO steers the executor (>1e16 solar->galaxy; >1e14 galaxy->universe).
#   * DESCEND command (the KEPT workaround, B21): still scales the new camera's
#     selDist exactly - exercised downward from the direct position (a moveto to
#     a FARther altitude would re-cross the executor thresholds; the descend
#     command's placement, not the target, is what stays under test).
# Pre-datum0 both galexec/uniband REQUIRED the descend (moveto landed at 3.2e9
# AU + X); that path is the b5_datumbase baseline artifact.
def galactic_leg(tag, mode_str, lo, hi, want):
    c = cam(f"{OUT}/b5_{tag}.json")
    check(f"{tag}_mode", log_has(mode_str), f"app.log '{mode_str}'")
    rd = c.get("refDist") or 0
    check(f"{tag}_reference", c.get("reference") == "MilkyWay" and lo < rd < hi,
          f"reference={c.get('reference')!r} refDist={rd} ({want})")
    q1, q2, qf_n, qf_o, qb1, qb2, qnb = phase_quad(s, tag)
    check(f"{tag}_new_draws", q1 > 0 and q2 > 0,
          f"cross-phase px32 {q1}@{qb1} / {q2}@{qb2} (pre-fix: 0)")
    check(f"{tag}_floors", qf_n == 0, f"floor_new={qf_n} (old floor recorded: {qf_o})")
    check(f"{tag}_notblack", qnb > 0, f"lit px={qnb}")

GALEXEC_M = 12000000000000000            # 1.2e16 m: >1e16 -> InGalaxy;
                                         # datum 0 -> new cam ~8.02e4 AU (dot)
UNIBAND_M = int(round(1340 * AU_M))      # 2.005e14 m: >1e14 -> InUniverse;
                                         # datum 0 -> new cam ~1340 AU (band px~19)

# ---- "galexec": executor ->InGalaxy, dot regime ~8e4 AU -------------------
# DIRECT moveto route (new).
send(s, f"moveto altitude {GALEXEC_M} duration 0", 4)
send(s, f"body action dual_dump filename {OUT}/b5_galexec.json", 2)
galactic_leg("galexec", "->InGalaxy", 6e4, 1e5, "want ~8.0e4 AU, DIRECT moveto")
# DESCEND route (kept workaround): scale the new camera down, re-verify it draws.
send(s, "camera action descend coef 0.5", 3)                 # ~8.0e4 -> ~4.0e4 AU
send(s, f"body action dual_dump filename {OUT}/b5_galexec_desc.json", 2)
galactic_leg("galexec_desc", "->InGalaxy", 3e4, 6e4, "want ~4.0e4 AU, DESCEND route")

# ---- "uniband": executor ->InUniverse, cross-fade band ~1340 AU ----------
# DIRECT moveto route (new): flip to InUniverse and land in-band with moveto.
send(s, f"moveto altitude {UNIBAND_M} duration 0", 4)        # galaxy->universe flip + place
send(s, f"moveto altitude {UNIBAND_M} duration 0", 4)        # clear the entry fade band
send(s, f"body action dual_dump filename {OUT}/b5_uniband.json", 2)
galactic_leg("uniband", "->InUniverse", 1100, 1550, "want in-band ~1340 AU, DIRECT moveto")
# DESCEND route (kept): descend within InUniverse, re-verify it draws. Stay
# clearly above the SolarSystem AoI (~634 AU) so the reference holds at MilkyWay.
send(s, "camera action descend coef 0.7", 3)                 # ~1340 -> ~938 AU
send(s, f"body action dual_dump filename {OUT}/b5_uniband_desc.json", 2)
galactic_leg("uniband_desc", "->InUniverse", 800, 1100, "want ~938 AU, DESCEND route")

json.dump({"results": results,
           "earth_recorded": {"cross": [c1, c2], "floors": [cf_n, cf_o]}},
          open(f"{OUT}/b5_result.json", "w"), indent=1)
bad = [r for r in results if not r["ok"]]
print(f"=== {len(results)-len(bad)}/{len(results)} PASS ===", flush=True)
sys.exit(1 if bad else 0)
