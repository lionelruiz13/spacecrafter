#!/usr/bin/env python3
# INTENT §11.108 (task F4, audit part i) - LIVE probe of the interactive
# navigation ramp: press a real arrow key and look at what moves.
#
# The claim under test, read from source and NOT trusted until pressed:
#   arrow key -> UI (ui.cpp:1091-1109) -> Core::turnLeft/Right/Up/Down
#             -> vzm.deltaAz/deltaAlt -> Core::updateMove (core.cpp:1817-1826)
#             -> navigation->updateMove(...)          <-- OLD path only
# with NO `Camera::` mirror anywhere (`Camera::lookRel`'s only caller is
# `Core::dragView`, core.cpp:1754). If that is right, holding an arrow key
# turns the OLD path's view and leaves the NEW path's camera where it was -
# i.e. under the new render path the basic navigation keys do nothing.
#
# Instrument honesty, two ways:
#  * POSITIVE CONTROL - the same keystroke must visibly move the OLD phase. If
#    it does not, the key never reached the app and "nothing moved" is a dead
#    probe, not a finding.
#  * VERIFICATION HEIGHT - leg 2 puts BODY content on screen with the sky off,
#    so the new-phase screenshot shows what the new path itself draws. (In
#    leg 1 the grid/stars are drawn by the OLD navigator in BOTH phases, so a
#    new-phase delta there says nothing about the camera - stated, not hidden.)
#
# Usage: f4_keyprobe.py <outdir>   (app fresh-launched, FISHEYE, enable_tcp)
import socket, sys, time, os, json, math, subprocess
import numpy as np
from PIL import Image

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
HERE = os.path.dirname(os.path.abspath(__file__))
XKEY = os.path.join(OUT, "xkey")
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

def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)

def dump(sock, name, pause=1.8):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return json.loads(open(f"{OUT}/{name}.json").readline())["camera"]

def body_screens(name, body="Moon"):
    """Per-PATH screen position of `body` from a dual dump: (new NDC, old px).
    This is the observable that discriminates - a screenshot cannot, whenever
    the lit content of the frame happens to be path-independent (measured: 48
    px>32 cross-path in leg 2, i.e. what is on screen there is drawn the same
    way in both phases, so its motion says nothing about which camera moved)."""
    new = old = None
    for ln in open(f"{OUT}/{name}.json"):
        ln = ln.strip().replace('-nan', 'null').replace('nan', 'null') \
               .replace('-inf', '-1e308').replace('inf', '1e308')
        if not ln: continue
        try: o = json.loads(ln)
        except Exception: continue
        if o.get("type") == "body" and o.get("name") == body:
            if o.get("new"): new = o["new"].get("screen")
            if o.get("old"): old = o["old"].get("screen")
    return new, old

def img(n): return np.asarray(Image.open(f"{OUT}/{n}.png").convert("RGB"), dtype=np.int16)
def px32(a, b): return int((np.abs(a - b) > 32).any(axis=2).sum())

subprocess.run(["gcc", "-O1", "-o", XKEY, os.path.join(HERE, "xkey.c"), "-lX11",
                "/usr/lib/x86_64-linux-gnu/libXtst.so.6"], check=True)

def hold(key, ms):
    r = subprocess.run([XKEY, "spacecrafter", key, str(ms), "1024x1024"],
                       capture_output=True, text=True)
    print(f"   xkey {key} {ms}ms -> rc={r.returncode} {r.stdout.strip()}{r.stderr.strip()}", flush=True)
    return r.returncode == 0

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
for f in ("atmosphere", "fog", "landscape", "show_fps", "planet_names", "cardinal_points"):
    send(s, f"flag {f} off")
send(s, "set home_planet Earth", 3)
send(s, "date jday 2461233.5", 1)

def snap(tag):
    send(s, "flag experimental_path on", 2)
    c = dump(s, f"{tag}_cam"); shot(s, f"{tag}_new")
    send(s, "flag experimental_path off", 2); shot(s, f"{tag}_old")
    send(s, "flag experimental_path on", 1)
    return c

# ---- LEG 1: sky content (old-navigator drawn) = the positive control -------
send(s, "flag equatorial_grid on"); send(s, "flag stars on"); send(s, "flag star_twinkle off")
send(s, "flag milky_way off"); send(s, "flag constellation_drawing off")
send(s, "deselect", 1); send(s, "flag track_object off", 1)
send(s, "zoom fov 120 duration 0", 2)
send(s, "look_at azimuth 180 altitude 45 duration 1", 3)
send(s, "timerate rate 0", 1)
a0 = snap("L1_before")
ok_key = hold("Left", 2500)
check("xkey_ran", ok_key, "XTEST key injection returned 0")
time.sleep(1.5)
a1 = snap("L1_after")
L1_old = px32(img("L1_before_old"), img("L1_after_old"))
L1_new = px32(img("L1_before_new"), img("L1_after_new"))
d1az = math.degrees(a1["az"] - a0["az"]); d1alt = math.degrees(a1["alt"] - a0["alt"])

# ---- LEG 2: BODY content only = the terminal observable -------------------
send(s, "flag equatorial_grid off"); send(s, "flag stars off")
send(s, "select planet Moon pointer off", 2)
send(s, "flag track_object on", 4)
send(s, "zoom fov 30 duration 0", 3)
send(s, "flag track_object off", 2)          # freeze the view, Moon centred
send(s, "timerate rate 0", 1)
b0 = snap("L2_before")
hold("Left", 2500)
time.sleep(1.5)
b1 = snap("L2_after")
L2_old = px32(img("L2_before_old"), img("L2_after_old"))
L2_new = px32(img("L2_before_new"), img("L2_after_new"))
d2az = math.degrees(b1["az"] - b0["az"]); d2alt = math.degrees(b1["alt"] - b0["alt"])

print(f"-- LEG1 (sky, old-nav drawn in BOTH phases): old {L1_old} px>32, new {L1_new} px>32 ; "
      f"new camera d(az,alt) = ({d1az:.6f}, {d1alt:.6f}) deg", flush=True)
print(f"-- LEG2 (body content, sky off):             old {L2_old} px>32, new {L2_new} px>32 ; "
      f"new camera d(az,alt) = ({d2az:.6f}, {d2alt:.6f}) deg", flush=True)

check("POSITIVE_CONTROL_key_reached_the_app", L1_old > 5000,
      f"LEG1 OLD-phase screen moved {L1_old} px>32 under the injected keystroke "
      f"(if ~0, the key never arrived and every null below is void)")
check("FINDING_new_camera_never_turned", abs(d1az) < 1e-4 and abs(d1alt) < 1e-4
      and abs(d2az) < 1e-4 and abs(d2alt) < 1e-4,
      f"new camera az/alt unchanged in both legs: L1 ({d1az:.6f},{d1alt:.6f}) "
      f"L2 ({d2az:.6f},{d2alt:.6f}) deg")
n0, o0 = body_screens("L2_before_cam")
n1, o1 = body_screens("L2_after_cam")
dn = math.hypot(n1[0] - n0[0], n1[1] - n0[1]) if (n0 and n1) else float('nan')
do = math.hypot(o1[0] - o0[0], o1[1] - o0[1]) if (o0 and o1) else float('nan')
print(f"-- per-path Moon screen: NEW {n0} -> {n1} (|d| {dn:.3e} NDC) ; "
      f"OLD {o0} -> {o1} (|d| {do:.1f} px)", flush=True)
check("FINDING_per_path_screen", dn == 0.0 and do > 100.0,
      f"the SAME body, in the SAME frame: the NEW path's screen position is "
      f"bit-identical across the keystroke (|d| {dn:.3e} NDC) while the OLD "
      f"path's moves {do:.1f} px - the ramp reaches one path only")
print(f"note: the LEG2 SCREENSHOT pair is reported, not asserted - its lit content is "
      f"path-independent (before-cross {px32(img('L2_before_new'), img('L2_before_old'))} px>32), "
      f"so its motion cannot attribute a camera; the dump above can.", flush=True)

json.dump(dict(L1=dict(old=L1_old, new=L1_new, daz=d1az, dalt=d1alt),
               L2=dict(old=L2_old, new=L2_new, daz=d2az, dalt=d2alt)),
          open(f"{OUT}/f4_keyprobe.json", "w"), indent=1)
print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
