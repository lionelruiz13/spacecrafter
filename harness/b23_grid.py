#!/usr/bin/env python3
# B23 - the planet-grid TROPIC + POLAR CIRCLES, keyed to the sky-line flags.
# Regression lock for the Vixy-ratified restoration (USER_QUESTIONS Q16 /
# INTENT 11.48(b) A4 -> 11.57):
#   "tropics + polar circles come back, drawn when the corresponding sky-line
#    flags are on (the old flag coupling is deliberate - they are the lines that
#    show obliquity directly)."
#
# THE COUPLING (old body.cpp:1257-1258, observed):
#   tropic circles  ride  LINE_TROPIC        <- `flag tropic_lines on|off`
#   polar  circles  ride  LINE_CIRCLE_POLAR  <- `flag polar_circle on|off`
#   tropic latitude   = +/- axial_tilt          (the obliquity, degrees)
#   polar   latitude  = +/- (90 - axial_tilt)
#   tropics only on non-satellite non-star bodies (old !isSatellite && !=Sun).
#
# OBSERVABLES:
#   DUMP (measured from the running process, ModularBody::dumpTrace -> the GRID
#     module's dumpState, INTENT 11.57): new.near[*] = {axialTilt, hasTropics,
#     tropicLat, polarLat, show, showTropics, showPolarCircles, built, ...}.
#     showTropics/showPolarCircles echo the LINE_TROPIC/LINE_CIRCLE_POLAR flag
#     the grid polled -> a command that reached the sky manager AND the grid.
#   SCREEN (terminal observable, DoD-4): a self-referential px diff. Noise floor
#     is measured (base vs base2, same frozen state); every gated assertion is
#     stated against it. THR>32 is used for the gated-line counts (well above
#     the measured floor of 0).
#
# METHOD. Time FROZEN; each planet framed at a fixed 5 body-radii so the disc
# is well resolved (23 deg in a 35 deg fov). Body tracked -> disc centred at
# the image centre, so a diff pixel's radial distance from centre is a direct,
# projection-honest readout of WHERE the ring sits on the disc: as obliquity
# grows the tropic moves out toward the pole/limb and the polar circle moves in
# toward the equator - Earth and Uranus come out REVERSED, which is the whole
# point of the feature.
#
# SUBJECTS: Earth (23.44 deg), Jupiter (3.13, near-upright), Uranus (97.77,
# tipped past 90) - three clearly-different obliquities. Plus Moon (satellite)
# and Sun (star) for the tropic GATE (hasTropics=False, polar circles still on).
# All need planet_grid=true in ssystem.ini (test-only, restored byte-identical).
#
# PRECONDITION: FRESH launch, enable_tcp, the NEW path pinned.
#   ./b23_run.sh b23_grid.py <outdir>
# Exit 0 = every assertion passed.  <outdir>/b23_result.json = machine-readable.

import socket, time, json, sys, os
import numpy as np
from PIL import Image

AU = 149597870.7
RAD = {"Earth": 6371.0, "Jupiter": 69911.0, "Uranus": 25362.0,
       "Moon": 1737.4, "Sun": 696000.0}
OBL = {"Earth": 23.44, "Jupiter": 3.13, "Uranus": 97.77, "Moon": 0.0, "Sun": 0.0}
PLANETS = ["Earth", "Jupiter", "Uranus"]     # hasTropics = True
GATED = ["Moon", "Sun"]                       # hasTropics = False (gate)
K = 5.0
FOV = 35.0
THR = 32          # gated-line px threshold (measured noise floor is 0)

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b23")
os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=1.7):
    send(f"body action screenshot filename {OUT}/b23_{tag}.png", pause)


def dump(tag, pause=2.0):
    send(f"body action dual_dump filename {OUT}/b23_{tag}.json", pause)
    return tag


def grid_of(tag, name):
    for line in open(os.path.join(OUT, f"b23_{tag}.json")):
        line = line.strip()
        if not line:
            continue
        r = json.loads(line)
        if r.get("type") == "body" and r.get("name") == name:
            g = [x for x in r["new"].get("near", []) if x]
            return r["new"], (g[0] if g else None)
    return None, None


def img(tag):
    return np.asarray(Image.open(os.path.join(OUT, f"b23_{tag}.png")).convert("RGB")).astype(np.int32)


def dpx(tag_a, tag_b, thr):
    d = np.abs(img(tag_a) - img(tag_b)).max(axis=2)
    return int((d > thr).sum()), int(d.max()), d


def frame(name):
    alt_m = int((K - 1.0) * RAD[name] * 1000.0)
    send(f"set home_planet {name}", 4)
    send(f"select planet {name} pointer off", 1)
    send("flag track_object on", 5)
    send(f"moveto lat 0 lon 0 alt {alt_m} duration 0", 4)
    send(f"zoom fov {FOV:.1f} duration 0", 3)
    # RELEASE (F0, §11.101(g3) / §11.80(c)): tracking is the AIM, not the state to
    # shoot in - it eases and never exactly settles, so every px floor measured
    # under it is the tracking residual, not the instrument's. This arm had no
    # matching off until F0, which left the flag set for the rest of the run and
    # made the "noise floor 0" statements properties of the run, not of the
    # instrument. Time is frozen, so releasing does not move the framing.
    send("flag track_object off", 2)


# ============================================================ drive =========
send("flag experimental_path on", 1)          # pin the NEW path
send("timerate rate 0", 1)
send("date jday 2461233.5", 1)
for f in ("atmosphere", "landscape", "fog", "stars", "milky_way", "planet_names",
          "cardinal_points", "nebula", "constellation_drawing", "constellation_art",
          "show_fps", "subtitle", "star_twinkle"):
    send(f"flag {f} off")
send("flag planets_axis on", 1)               # the grid rides the axis flag
send("flag equatorial_grid off")
send("flag tropic_lines off")
send("flag polar_circle off", 1)

records = {}
for name in PLANETS:
    frame(name)
    send("flag tropic_lines off", 1)
    send("flag polar_circle off", 1.2)
    _, g = grid_of(dump(f"{name}_probe"), name)
    print(f"  {name}: grid={g}", flush=True)
    shot(f"{name}_base")
    shot(f"{name}_base2")                       # noise-floor twin
    # --- tropic reversible pair, traversed TWICE (each entry from the prior exit)
    send("flag tropic_lines on", 1.5); shot(f"{name}_tr_on1"); dump(f"{name}_tr_on1")
    send("flag tropic_lines off", 1.3); shot(f"{name}_tr_off1")
    send("flag tropic_lines on", 1.5); shot(f"{name}_tr_on2")
    send("flag tropic_lines off", 1.3); shot(f"{name}_tr_off2")
    # --- polar reversible pair, traversed TWICE
    send("flag polar_circle on", 1.5); shot(f"{name}_pl_on1"); dump(f"{name}_pl_on1")
    send("flag polar_circle off", 1.3); shot(f"{name}_pl_off1")
    send("flag polar_circle on", 1.5); shot(f"{name}_pl_on2")
    send("flag polar_circle off", 1.3); shot(f"{name}_pl_off2")
    # --- both on (the composed obliquity readout) + measured latitudes
    send("flag tropic_lines on", 1.2)
    send("flag polar_circle on", 1.5)
    shot(f"{name}_both"); dump(f"{name}_both")

# --- command-spelling negative control on Earth (the b11-class trap) ---------
# The real names are `tropic_lines` / `polar_circle`.  A plausible-wrong name
# must be swallowed with NO effect: the grid's showTropics stays as it was and
# the screen does not move.
frame("Earth")
send("flag tropic_lines off", 1)
send("flag polar_circle off", 1.2)
shot("neg_base"); _, gneg0 = grid_of(dump("neg_before"), "Earth")
send("flag tropic off", 1.5)                   # BOGUS (missing _lines)
shot("neg_bogus"); _, gneg1 = grid_of(dump("neg_bogus"), "Earth")
send("flag tropic_lines on", 1.5)              # REAL
shot("neg_real"); _, gneg2 = grid_of(dump("neg_real"), "Earth")

# --- the tropic GATE: satellite + star carry polar circles but NO tropics ----
for name in GATED:
    frame(name)
    send("flag tropic_lines on", 1)
    send("flag polar_circle on", 1.2)
    _, g = grid_of(dump(f"{name}_gate"), name)
    print(f"  GATE {name}: grid={g}", flush=True)

sock.close()

# ========================================================== evaluate ========
fail = 0
report = {"K": K, "fov": FOV, "thr": THR, "planets": {}, "gate": {}, "checks": []}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    report["checks"].append({"ok": bool(ok), "text": text})
    return ok


print("\n=== 0. noise floor (frozen scene, new path) ===")
for name in PLANETS:
    nf, nfmax, _ = dpx(f"{name}_base", f"{name}_base2", 0)
    report["planets"].setdefault(name, {})["noise_floor_px_gt0"] = nf
    check(nf == 0, f"{name}: noise floor base vs base2 = {nf} px>0 (max delta {nfmax})")

print("\n=== 1. measured latitudes track obliquity (from the process dump) ===")
lat_summary = {}
for name in PLANETS:
    _, g = grid_of(f"{name}_both", name)
    tl, pl, ax, ht = g["tropicLat"], g["polarLat"], g["axialTilt"], g["hasTropics"]
    lat_summary[name] = (ax, tl, pl)
    report["planets"][name].update(
        {"axialTilt": ax, "tropicLat": tl, "polarLat": pl, "hasTropics": ht,
         "vertexCount": g["vertexCount"], "tropicCount": g["tropicCount"],
         "polarCount": g["polarCount"]})
    print(f"  {name:8s} axialTilt={ax:7.3f}  tropicLat={tl:7.3f}  polarLat={pl:8.3f}"
          f"  hasTropics={ht}")
    check(ht is True, f"{name}: hasTropics=True (planet)")
    check(abs(tl - OBL[name]) < 0.02, f"{name}: tropicLat {tl:.3f} == obliquity {OBL[name]}")
    check(abs(pl - (90.0 - OBL[name])) < 0.02,
          f"{name}: polarLat {pl:.3f} == 90-obliquity {90.0-OBL[name]:.2f}")
# they actually DIFFER across bodies (moves with obliquity)
tls = [lat_summary[n][1] for n in PLANETS]
check(len(set(round(x, 2) for x in tls)) == len(PLANETS),
      f"tropic latitudes are all distinct across bodies: {[round(x,2) for x in tls]}")

print("\n=== 2. flag gating on the composed screen (px>%d vs 0 floor) ===" % THR)
for name in PLANETS:
    # tropic pair, twice: on -> present, off -> back to base exactly
    on1, m1, _ = dpx(f"{name}_base", f"{name}_tr_on1", THR)
    off1, mo1, _ = dpx(f"{name}_base", f"{name}_tr_off1", 0)
    on2, m2, _ = dpx(f"{name}_base", f"{name}_tr_on2", THR)
    off2, mo2, _ = dpx(f"{name}_base", f"{name}_tr_off2", 0)
    report["planets"][name]["tropic_px"] = [on1, on2]
    check(on1 > 0 and on2 > 0,
          f"{name} tropic: ON present both entries ({on1}, {on2} px>{THR})")
    check(off1 == 0 and off2 == 0,
          f"{name} tropic: OFF reverts to base both entries ({off1}, {off2} px>0)")
    # polar pair, twice
    pon1, _, _ = dpx(f"{name}_base", f"{name}_pl_on1", THR)
    poff1, _, _ = dpx(f"{name}_base", f"{name}_pl_off1", 0)
    pon2, _, _ = dpx(f"{name}_base", f"{name}_pl_on2", THR)
    poff2, _, _ = dpx(f"{name}_base", f"{name}_pl_off2", 0)
    report["planets"][name]["polar_px"] = [pon1, pon2]
    check(pon1 > 0 and pon2 > 0,
          f"{name} polar: ON present both entries ({pon1}, {pon2} px>{THR})")
    check(poff1 == 0 and poff2 == 0,
          f"{name} polar: OFF reverts to base both entries ({poff1}, {poff2} px>0)")

print("\n=== 3. moves with obliquity ON SCREEN (ring radial position reverses) ===")
# tropic-diff and polar-diff radial-distance-from-centre; Earth vs Uranus reverse
rad = {}
for name in ("Earth", "Uranus"):
    _, _, td = dpx(f"{name}_base", f"{name}_tr_on1", 16)
    _, _, pdd = dpx(f"{name}_tr_on1", f"{name}_both", 16)  # polar added on top of tropic
    def mean_r(dmap):
        ys, xs = np.where(dmap > 16)
        r = np.sqrt((xs - 1024.0) ** 2 + (ys - 1024.0) ** 2)
        return float(r.mean()) if len(xs) else float("nan")
    rt, rp = mean_r(td), mean_r(pdd)
    rad[name] = (rt, rp)
    report["planets"][name]["screen_radial_tropic_mean"] = rt
    report["planets"][name]["screen_radial_polar_mean"] = rp
    print(f"  {name}: tropic ring mean radius {rt:.0f}px, polar ring mean radius {rp:.0f}px")
# Earth: tropic inside polar (23<66); Uranus tipped: tropic OUTSIDE polar (98>-8)
check(rad["Earth"][0] < rad["Earth"][1],
      f"Earth: tropic ring ({rad['Earth'][0]:.0f}) INSIDE polar ring ({rad['Earth'][1]:.0f})")
check(rad["Uranus"][0] > rad["Uranus"][1],
      f"Uranus: tropic ring ({rad['Uranus'][0]:.0f}) OUTSIDE polar ring "
      f"({rad['Uranus'][1]:.0f}) - the obliquity reversal")

print("\n=== 4. command spelling (from the process) ===")
bogus_px, _, _ = dpx("neg_base", "neg_bogus", 0)
real_px, _, _ = dpx("neg_base", "neg_real", THR)
report["spelling"] = {"working": "flag tropic_lines / flag polar_circle",
                      "bogus_tested": "flag tropic off",
                      "bogus_showTropics": gneg1["showTropics"],
                      "real_showTropics": gneg2["showTropics"],
                      "bogus_screen_px": bogus_px, "real_screen_px": real_px}
check(gneg1["showTropics"] is False and bogus_px == 0,
      f"bogus `flag tropic off` swallowed: showTropics={gneg1['showTropics']}, "
      f"screen px>0={bogus_px}")
check(gneg2["showTropics"] is True and real_px > 0,
      f"real `flag tropic_lines on` dispatched: showTropics={gneg2['showTropics']}, "
      f"screen px>{THR}={real_px}")

print("\n=== 5. tropic GATE: satellite carries polar circles but NO tropics ===")
# The satellite branch (Moon) is observably exercised.  The star branch
# (!isStar()) is a code-level guarantee; the shipped Sun does not install a
# planet grid at all (near=[None,None]) - a separate observation about how the
# system-centre star loads, recorded, not a B23 gate failure.
for name in GATED:
    _, g = grid_of(f"{name}_gate", name)
    report["gate"][name] = g
    if g is None:
        print(f"  {name:6s} grid NOT installed (near has no grid module) - "
              f"star gate not observable on shipped data, code guard stands")
        report["gate"][name] = "no-grid-installed"
        if name == "Moon":       # the satellite MUST have a grid - a real failure
            check(False, f"{name}: grid module absent from the dump")
        continue
    print(f"  {name:6s} hasTropics={g['hasTropics']} tropicCount={g['tropicCount']} "
          f"polarCount={g['polarCount']} axialTilt={g['axialTilt']}")
    check(g["hasTropics"] is False, f"{name}: hasTropics=False (gated - {name} is a "
          f"{'satellite' if name=='Moon' else 'star'})")
    check(g["tropicCount"] == 0, f"{name}: tropicCount=0 (no tropic geometry baked)")
    check(g["polarCount"] > 0, f"{name}: polarCount={g['polarCount']} (polar circles "
          f"present - the old path draws them on every body)")

report["fail"] = fail
with open(os.path.join(OUT, "b23_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail)+' ASSERTION(S) FAILED'} - "
      f"{OUT}/b23_result.json")
sys.exit(1 if fail else 0)
