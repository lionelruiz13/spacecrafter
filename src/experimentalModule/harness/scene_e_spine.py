#!/usr/bin/env python3
# Scene E - hierarchy-spine ladder (INTENT 11.36 verification).
# Predictive validation (11.15 method): AoI thresholds COMPUTED offline from
# a baseline dump (repaired updateCache formulas transcribed below), then the
# reference-switch sequence asserted at bracketing altitudes. Brackets are
# WIDE (x0.7 / x1.5) around the Earth boundary: areaOfInfluence freezes at
# the first full cache (uncached latch), i.e. at LAUNCH jd - |ecl_moon| there
# differs ~10% from the scene jd (measured, first run's e_in flap).
# Auto-transitions are FREE-FLIGHT-ONLY (11.36 policy): the ladder runs in
# freeMode; the anchored legs verify the LEGACY pattern stays race-free.
# Precondition: app fresh-launched, init_fov=340, fisheye, enable_tcp.
import socket, time, json, math, sys

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

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)   # pin new path (deterministic captures)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "body action dual_dump filename /tmp/spine_base.json", 2)

b = bodies("/tmp/spine_base.json")
n = lambda v: math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])
AU_M = 149597870700.0  # m per AU (moveto altitude is in meters)

# ---- Transcription of the repaired updateCache (ModularBody.cpp) ----------
# subsystemRadius = 1.1 * max(bounding, max_child(|ecl_child| + subsys_child))
# AoI = max(bounding*128/scaling, subsystem*16), capped |ecl|*0.6 iff |ecl|>0
moon_sub  = 1.1 * b["Moon"]["boundingRadius"]
earth_sub = 1.1 * max(b["Earth"]["boundingRadius"], n(b["Moon"]["ecl"]) + moon_sub)
earth_aoi = min(max(b["Earth"]["boundingRadius"]*128, earth_sub*16), n(b["Earth"]["ecl"])*0.6)
nep_sub   = 1.1 * b["Neptune"]["boundingRadius"]
sun_sub   = 1.1 * (n(b["Neptune"]["ecl"]) + nep_sub)
sun_aoi   = max(b["Sun"]["boundingRadius"]*128, sun_sub*16)      # ecl==0: uncapped
sys_sub   = 1.1 * sun_sub                                        # Sun at system center
sys_aoi   = sys_sub*16                                           # ecl==0: uncapped
mw_aoi    = 3.2e9*128                                            # bounding term dominates
print(f"predicted AoI [AU]: Earth {earth_aoi:.4e}  Sun {sun_aoi:.1f}  "
      f"SolarSystem {sys_aoi:.1f}  MilkyWay {mw_aoi:.3e}", flush=True)

expect = []
def rung(alt_au, tag, ref, pause=3):
    alt_m = int(alt_au * AU_M)
    send(s, f"moveto altitude {alt_m} duration 0", pause)
    send(s, f"body action dual_dump filename /tmp/spine_{tag}.json", 2)
    expect.append((tag, ref))

# ---- Free-flight ladder (auto-transitions live here) ----------------------
send(s, "camera action free_mode state on", 1)
rung(earth_aoi*0.7,  "e_in",   "Earth")        # inside Earth AoI (wide bracket)
rung(earth_aoi*1.5,  "e_out",  "Sun")          # escalate -> Sun
rung(sun_aoi*0.9,    "sun_in", "Sun")
rung(sun_aoi*1.05,   "sun_out","SolarSystem")  # above Sun, below system AoI
rung(sys_aoi*1.2,    "sys_out","MilkyWay")
send(s, "body action screenshot filename /tmp/spine_mw.jpg", 2)   # in-galaxy backdrop
rung(mw_aoi*1.2,     "mw_out", "Universe")
send(s, "body action screenshot filename /tmp/spine_uni.jpg", 2)  # outside: none
# ---- Free-flight DESCENT: capture cascade ---------------------------------
# freeMode moveto altitude is CENTER-RELATIVE to the CURRENT reference; from
# Universe the target direction lands in solar vicinity (universe, milkyway
# and solar system share center 0) -> the cascade captures MilkyWay ->
# SolarSystem -> Sun, and the SUN keeps the camera (Earth is 1 AU away from
# the target point - capturing it would be wrong). Earth-directed free
# descent needs a directional move surface (suspended note).
rung(earth_aoi*0.5,  "descend", "Sun", pause=5)  # one capture per frame
# ---- Second entry of BOTH cascades (reversible pair x2) -------------------
rung(mw_aoi*1.2,     "mw_out2", "Universe", pause=5)  # escalate cascade again
rung(earth_aoi*0.5,  "descend2","Sun", pause=5)       # capture cascade again
# ---- Anchored legacy legs (no auto-transitions by policy) -----------------
send(s, "camera action free_mode state off", 1)
send(s, "body action dual_dump filename /tmp/spine_unfree.json", 2)
expect.append(("unfree", "Sun"))               # mode off keeps the reference
rung(sun_aoi*1.05,   "anchor_high", "Sun")     # anchored: reference PINNED
                                               # (this altitude escalated in freeMode)
send(s, "set home_planet Mars", 3)             # the legacy warp+moveto pattern
send(s, "moveto lat 10 lon 30 alt 100 duration 0", 2)
send(s, "body action dual_dump filename /tmp/spine_mars.json", 2)
expect.append(("mars", "Mars"))
send(s, "set home_planet Earth", 3)            # restore
s.close()

fail = 0
for tag, want in expect:
    c = cam(f"/tmp/spine_{tag}.json")
    ok = c["reference"] == want
    print(f"{'OK ' if ok else 'FAIL'} {tag:12s} ref={c['reference']:12s} "
          f"dist={c['distance']:.4e} free={c['freeMode']}", flush=True)
    fail += not ok
mars = cam("/tmp/spine_mars.json")
ok = abs(mars["distance"] - 2.270821e-05) < 3e-9  # R_mars + 100 m (11.17 value)
print(f"{'OK ' if ok else 'FAIL'} mars landing dist {mars['distance']:.6e} vs Rmars+100m", flush=True)
fail += not ok
sys.exit(1 if fail else 0)
