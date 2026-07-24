#!/usr/bin/env python3
# B17 - view_offset SCREEN A/B: the new-path Camera offset vs the OLD path, on
# flat fisheye, at offsets {0, +0.3, -0.3} (INTENT 11.63/11.79(c), D6 option 1).
#
# Discriminators (the row's own):
#   * NEW pixels SHIFT with offset (the port acts): new@0 vs new@0.3 is large.
#   * new@V == old@V at each offset to the cross-path parity CLASS (px>32, B30):
#     the offset shifts BOTH paths identically, so new-vs-old is ~its offset-0
#     baseline at every offset (NOT +2x, which a wrong sign/axis would give).
#   * fov-INDEPENDENCE (11.63 signature): the angular shift / halfFov == the
#     offset fraction at TWO fovs (dump-measured, the "percent of fov radius").
#
# Two content classes: a TRACKED BODY (the Moon - Camera-drawn, the port's own
# surface) and the SKY GRID+STARS (old-navigator-drawn - rides old-nav's offset
# in BOTH paths). Both must shift and match.
#
# Usage: b17_screen.py <outdir>   (app fresh-launched, FISHEYE, enable_tcp)
import socket, sys, time, os, json
import numpy as np
from PIL import Image

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
FAILS = []
def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok: FAILS.append(name)

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192)
    except socket.timeout: pass
    sock.settimeout(None); print(">>", cmd, flush=True)
def shot(sock, name, pause=2.5): send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)
def dump(sock, name, pause=2.0):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return json.loads(open(f"{OUT}/{name}.json").readline())["camera"]
def img(n): return np.asarray(Image.open(f"{OUT}/{n}.png").convert("RGB"), dtype=np.int16)
def px32(a, b): return int((np.abs(a - b) > 32).any(axis=2).sum())

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
for f in ("atmosphere", "fog", "landscape", "show_fps", "planet_names", "cardinal_points"):
    send(s, f"flag {f} off")
send(s, "set home_planet Earth", 3)
send(s, "date jday 2461233.5", 1)

def sweep(prefix, want_dump=False):
    d = {}
    for V, tag in [("0", "0"), ("0.3", "p3"), ("-0.3", "m3")]:
        send(s, f"set zoom_offset {V}", 2); time.sleep(1.5)
        send(s, "flag experimental_path on", 2); shot(s, f"{prefix}_new_{tag}")
        if want_dump: d[tag] = dump(s, f"{prefix}_new_{tag}")
        send(s, "flag experimental_path off", 2); shot(s, f"{prefix}_old_{tag}")
        send(s, "flag experimental_path on", 1)
    return d

# --- TRACKED BODY (Camera-drawn) -----------------------------------------
send(s, "flag equatorial_grid off"); send(s, "flag stars off")
send(s, "flag milky_way off"); send(s, "flag constellation_drawing off")
send(s, "select planet Moon pointer off", 2)
send(s, "flag track_object on", 4)                 # arms the offset (setFlagTracking)
send(s, "zoom fov 10 duration 0", 3); send(s, "timerate rate 0", 1)
dt = sweep("trk", want_dump=True)
tn0, tnp, tnm = img("trk_new_0"), img("trk_new_p3"), img("trk_new_m3")
to0, top, tom = img("trk_old_0"), img("trk_old_p3"), img("trk_old_m3")
base = px32(tn0, to0)
check("trk_armed", abs(dt["p3"]["viewOffsetEff"] - 0.3) < 1e-3 and abs(dt["m3"]["viewOffsetEff"] + 0.3) < 1e-3,
      f"eff +0.3={dt['p3']['viewOffsetEff']:.4f} / -0.3={dt['m3']['viewOffsetEff']:.4f}")
check("trk_new_shifts", px32(tn0, tnp) > 2000 and px32(tn0, tnm) > 2000,
      f"new shift +0.3={px32(tn0,tnp)} -0.3={px32(tn0,tnm)} px")
check("trk_matches_old", px32(tnp, top) < base + 400 and px32(tnm, tom) < base + 400,
      f"cross @0={base} @+0.3={px32(tnp,top)} @-0.3={px32(tnm,tom)} (offset adds ~0 divergence)")

# --- SKY GRID + STARS (old-navigator-drawn) ------------------------------
send(s, "deselect", 1); send(s, "flag track_object off", 1)
send(s, "flag equatorial_grid on"); send(s, "flag stars on"); send(s, "flag star_twinkle off")
send(s, "zoom fov 120 duration 0", 2)
send(s, "look_at azimuth 180 altitude 45 duration 1", 3); send(s, "timerate rate 0", 1)
sweep("grid")
gn0, gnp = img("grid_new_0"), img("grid_new_p3")
go0, gop, gom = img("grid_old_0"), img("grid_old_p3"), img("grid_new_m3")
gbase = px32(gn0, go0)
check("grid_new_shifts", px32(gn0, gnp) > 5000, f"new grid shift={px32(gn0,gnp)} px")
check("grid_matches_old", px32(gnp, gop) < gbase + 500,
      f"cross @0={gbase} @+0.3={px32(gnp,gop)} (old-nav content, both paths shift identically)")

# --- fov-INDEPENDENCE (11.63 signature) ----------------------------------
send(s, "select planet Moon pointer off", 2); send(s, "flag track_object on", 4)
send(s, "flag equatorial_grid off"); send(s, "flag stars off")
def eff_absfwd(V, fov):
    send(s, f"zoom fov {fov} duration 0", 3)
    send(s, f"set zoom_offset {V}", 2); time.sleep(1.5)
    c = dump(s, f"fov{fov}_{V}")
    return np.array(c["absFwd"]), c["halfFov"]
import math
def angdeg(a, b):
    d = float(np.clip(np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b)), -1, 1))
    return math.degrees(math.acos(d))
f0a, hf90 = eff_absfwd("0", 20)      # halfFov ~90 deg
f3a, _    = eff_absfwd("0.3", 20)
frac90 = math.radians(angdeg(f0a, f3a)) / hf90
f0b, hf30 = eff_absfwd("0", 60)      # halfFov ~30 deg
f3b, _    = eff_absfwd("0.3", 60)
frac30 = math.radians(angdeg(f0b, f3b)) / hf30
check("fov_independent_fraction", abs(frac90 - 0.3) < 0.01 and abs(frac30 - 0.3) < 0.01,
      f"shift/halfFov = {frac90:.5f} @halfFov{math.degrees(hf90):.0f} / {frac30:.5f} @halfFov{math.degrees(hf30):.0f} (want 0.3)")

print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
