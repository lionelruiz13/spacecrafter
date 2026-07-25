#!/usr/bin/env python3
# INTENT §5.26 - `set heading` old/new SEMANTIC parity, measured at the SCREEN.
#
# The defect (recorded by B17, §11.92(e)): the ONE command `set heading X`
# drives old (`Navigator::changeHeading`, an absolute additive eye-frame roll,
# navigator.cpp:314) and new (`Camera::setHeading`, a ZXZ Euler PARAMETER of the
# composed view rotation that `recoverParams` rewrites at every view-preserving
# transition) - two authorities, two zero points, one command.
#
# Scene = the B17 `trk` scene verbatim (b17_screen.py) so the cross-path px are
# directly comparable to the recorded ~132 px class.
#
# Legs (each = dumped camera params + NEW screenshot + OLD screenshot + cross):
#   nat        offset +0.3, no heading command ever issued   -> the 132 px class
#   h0         `set heading 0`   , offset +0.3               -> §5.26's own leg
#   h0b        `set heading 0` again                         -> idempotence
#   h30        `set heading 30`  , offset +0.3               -> heading != 0
#   o0_h0      `set heading 0`   , offset 0                  -> baseline, no offset
#   o0_h30     `set heading 30`  , offset 0                  -> PURE heading parity
#                                                               (the discriminator:
#                                                                no offset => the
#                                                                §11.92(d) coupling
#                                                                question is moot)
#
# Usage: s526_heading.py <outdir>   (app fresh-launched, FISHEYE, enable_tcp)
import socket, sys, time, os, json, math
import numpy as np
from PIL import Image

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
FAILS = []
RES = {}

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

def dump(sock, name, pause=2.0):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return json.loads(open(f"{OUT}/{name}.json").readline())["camera"]

def img(n): return np.asarray(Image.open(f"{OUT}/{n}.png").convert("RGB"), dtype=np.int16)
def px32(a, b): return int((np.abs(a - b) > 32).any(axis=2).sum())
def px8(a, b): return int((np.abs(a - b) > 8).any(axis=2).sum())

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
for f in ("atmosphere", "fog", "landscape", "show_fps", "planet_names", "cardinal_points"):
    send(s, f"flag {f} off")
send(s, "set home_planet Earth", 3)
send(s, "date jday 2461233.5", 1)
send(s, "flag equatorial_grid off"); send(s, "flag stars off")
send(s, "flag milky_way off"); send(s, "flag constellation_drawing off")
send(s, "select planet Moon pointer off", 2)
send(s, "flag track_object on", 4)                 # arms the offset (setFlagTracking)
send(s, "zoom fov 10 duration 0", 3); send(s, "timerate rate 0", 1)

def leg(tag, offset=None, heading=None, pre_pause=1.5):
    """One measurement leg: optionally set offset / heading, then dump + A/B."""
    if offset is not None:
        send(s, f"set zoom_offset {offset}", 2)
    if heading is not None:
        send(s, f"set heading {heading}", 2)
    time.sleep(pre_pause)
    send(s, "flag experimental_path on", 2)
    c = dump(s, f"{tag}_cam")
    shot(s, f"{tag}_new")
    send(s, "flag experimental_path off", 2)
    shot(s, f"{tag}_old")
    send(s, "flag experimental_path on", 1)
    cross32 = px32(img(f"{tag}_new"), img(f"{tag}_old"))
    cross8 = px8(img(f"{tag}_new"), img(f"{tag}_old"))
    RES[tag] = dict(heading=c["heading"], headingDeg=math.degrees(c["heading"]),
                    alt=c["alt"], az=c["az"], eff=c.get("viewOffsetEff"),
                    cross32=cross32, cross8=cross8)
    print(f"-- leg {tag}: heading={c['heading']:.6f} rad ({math.degrees(c['heading']):.4f} deg) "
          f"alt={c['alt']:.6f} az={c['az']:.6f} eff={c.get('viewOffsetEff')} "
          f"cross px>32={cross32} px>8={cross8}", flush=True)
    return RES[tag]

nat    = leg("nat",    offset="0.3")
h0     = leg("h0",     heading="0")
h0b    = leg("h0b",    heading="0")
h30    = leg("h30",    heading="30")
o0_h0  = leg("o0_h0",  offset="0", heading="0")
o0_h30 = leg("o0_h30", heading="30")

json.dump(RES, open(f"{OUT}/s526_results.json", "w"), indent=1)

base = nat["cross32"]
check("PR1_natural_heading_nonzero", abs(nat["heading"]) > 0.01,
      f"natural new-path heading = {nat['heading']:.6f} rad = {nat['headingDeg']:.4f} deg "
      f"(old's is 0 by config)")
check("PR2_natural_cross_132_class", 50 <= base <= 500,
      f"natural cross px>32 = {base} (recorded class ~132)")
check("PR3_set_heading_0_diverges", h0["cross32"] > 10 * max(base, 1),
      f"cross after `set heading 0` = {h0['cross32']} px>32 vs natural {base} "
      f"(ratio {h0['cross32']/max(base,1):.1f}x)")
check("PR4_command_lands_on_new", abs(h0["heading"]) < 1e-6,
      f"dumped heading after `set heading 0` = {h0['heading']:.8f} rad")
check("PR5_pure_heading_no_offset", True,
      f"offset 0: cross(h=0) = {o0_h0['cross32']} px>32 ; cross(h=30) = {o0_h30['cross32']} px>32 "
      f"=> {'HEADING-ZERO-POINT desync (fixable here)' if o0_h30['cross32'] > 10*max(o0_h0['cross32'],1) else 'offset-coupling only (=> §11.92(d), suspend)'}")
check("PR6_idempotent", abs(h0b["cross32"] - h0["cross32"]) < 0.25 * max(h0["cross32"], 1),
      f"`set heading 0` twice: {h0['cross32']} then {h0b['cross32']} px>32")

print("\n--- SUMMARY (cross-path px>32) ---", flush=True)
for k, v in RES.items():
    print(f"  {k:8s} heading={v['headingDeg']:9.4f} deg  eff={v['eff']}  cross32={v['cross32']:7d}  cross8={v['cross8']:7d}", flush=True)
print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
