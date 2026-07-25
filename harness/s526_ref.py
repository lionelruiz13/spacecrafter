#!/usr/bin/env python3
# INTENT §5.26, scene 2 - does a REFERENCE SWITCH desynchronise the two heading
# authorities? (scene 1 = s526_heading.py, which found `set heading X` CLEAN at
# offset 0 and divergent only at heading!=0 AND offset!=0 = §11.92(d)'s
# suspended coupling.)
#
# Mechanism under test: `Camera::recoverParams` (Camera.cpp:183-207) REWRITES
# the new path's `heading` param at every view-preserving transition (a
# reference switch is one, `warpToBody`), while old's `Navigator::heading`
# (navigator.cpp:314) is a persistent scalar nothing but the operator touches.
# If the two drift apart, ONE command lands two different rolls.
#
# The purest probe: `heading delta_azimuth 0` computes `evalDouble(0) +
# coreLink->getHeading()` (app_command_interface.cpp:2125) - getHeading() reads
# the OLD authority (coreLink.hpp:990) - and writes it to BOTH. A ZERO delta is
# a semantic no-op; if the new-path view MOVES, the two authorities disagree by
# construction and the pixels say by how much.
#
# Usage: s526_ref.py <outdir>   (app fresh-launched, FISHEYE, enable_tcp)
import socket, sys, time, os, json, math
import numpy as np
from PIL import Image

OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
RES = {}; FAILS = []

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
def lit_centroid(a):
    m = (a.sum(axis=2) > 60)
    if m.sum() == 0: return (float('nan'), float('nan'), 0)
    ys, xs = np.nonzero(m)
    return (float(xs.mean()), float(ys.mean()), int(m.sum()))

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
for f in ("atmosphere", "fog", "landscape", "show_fps", "planet_names", "cardinal_points",
          "equatorial_grid", "stars", "milky_way", "constellation_drawing"):
    send(s, f"flag {f} off")
send(s, "set home_planet Earth", 3)
send(s, "date jday 2461233.5", 1)

def leg(tag, cmds=(), pause=1.5):
    for c in cmds:
        send(s, c, 2)
    time.sleep(pause)
    send(s, "flag experimental_path on", 2)
    c = dump(s, f"{tag}_cam")
    shot(s, f"{tag}_new")
    send(s, "flag experimental_path off", 2)
    shot(s, f"{tag}_old")
    send(s, "flag experimental_path on", 1)
    n, o = img(f"{tag}_new"), img(f"{tag}_old")
    cn, co = lit_centroid(n), lit_centroid(o)
    hmod = math.degrees(c["heading"]) % 360.0
    if hmod > 180: hmod -= 360.0
    RES[tag] = dict(heading=c["heading"], headingDeg=math.degrees(c["heading"]),
                    headingMod360=hmod, ref=c.get("reference"), alt=c["alt"], az=c["az"],
                    eff=c.get("viewOffsetEff"), cross32=px32(n, o), cross8=px8(n, o),
                    cnew=cn, cold=co,
                    dcentroid=float(math.hypot(cn[0]-co[0], cn[1]-co[1])) if cn[2] and co[2] else None)
    r = RES[tag]
    print(f"-- {tag}: ref={r['ref']} heading={r['headingDeg']:.4f}deg (mod360 {hmod:.4f}) "
          f"cross32={r['cross32']} cross8={r['cross8']} "
          f"lit new={cn[2]} old={co[2]} dcentroid={r['dcentroid']}", flush=True)
    return r

# --- Earth reference, Moon tracked (the B17 trk scene, offset OFF) ---------
e_nat = leg("e_nat", ["select planet Moon pointer off", "flag track_object on",
                      "zoom fov 10 duration 0", "timerate rate 0"])
# `heading delta_azimuth 0` = a semantic no-op that writes old's heading to BOTH
e_d0  = leg("e_d0",  ["heading delta_azimuth 0"])

# --- REFERENCE SWITCH Earth -> Moon (warpToBody => recoverParams) ----------
m_nat = leg("m_nat", ["deselect", "flag track_object off", "set home_planet Moon",
                      "select planet Earth pointer off", "flag track_object on",
                      "zoom fov 10 duration 0", "timerate rate 0"], pause=3)
m_d0  = leg("m_d0",  ["heading delta_azimuth 0"])
m_h30 = leg("m_h30", ["set heading 30"])
m_h0  = leg("m_h0",  ["set heading 0"])

# --- SECOND ENTRY of the reversible pair (Moon -> Earth), from the state the
#     first exit produced --------------------------------------------------
e2_nat = leg("e2_nat", ["deselect", "flag track_object off", "set home_planet Earth",
                        "select planet Moon pointer off", "flag track_object on",
                        "zoom fov 10 duration 0", "timerate rate 0"], pause=3)
e2_d0  = leg("e2_d0",  ["heading delta_azimuth 0"])

json.dump(RES, open(f"{OUT}/s526_ref_results.json", "w"), indent=1)

base = e_nat["cross32"]
check("A_earth_baseline", base < 600, f"Earth/Moon-tracked cross px>32 = {base}")
check("B_delta0_is_noop_on_earth", abs(e_d0["cross32"] - base) < max(0.5*base, 100),
      f"`heading delta_azimuth 0`: {base} -> {e_d0['cross32']} px>32")
check("C_moon_ref_param_drift",
      abs(m_nat["headingMod360"]) < 0.1,
      f"after Earth->Moon switch the new-path heading param (mod 360) = "
      f"{m_nat['headingMod360']:.4f} deg  [<0.1 => authorities still agree; "
      f">0.1 => §5.26 has a fixable heading-zero-point half]")
check("D_delta0_is_noop_on_moon",
      abs(m_d0["cross32"] - m_nat["cross32"]) < max(0.5*max(m_nat["cross32"],1), 100),
      f"`heading delta_azimuth 0` after the switch: {m_nat['cross32']} -> {m_d0['cross32']} px>32")
check("E_second_entry",
      abs(e2_d0["cross32"] - e2_nat["cross32"]) < max(0.5*max(e2_nat["cross32"],1), 100),
      f"2nd entry Earth: nat {e2_nat['cross32']} -> delta0 {e2_d0['cross32']} px>32")

print("\n--- SUMMARY ---", flush=True)
for k, v in RES.items():
    print(f"  {k:7s} ref={str(v['ref']):12s} hdg={v['headingDeg']:10.4f} (mod {v['headingMod360']:8.4f}) "
          f"cross32={v['cross32']:7d} cross8={v['cross8']:7d} litN={v['cnew'][2]:7d} litO={v['cold'][2]:7d} "
          f"dcent={v['dcentroid']}", flush=True)
print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
