#!/usr/bin/env python3
# Scene E - hierarchy-spine ladder (INTENT 11.36 verification) +
# view-continuity assertions (INTENT 11.61, B13): the ladder asserts REFERENCE
# IDENTITY across transitions; the appended block asserts the ABSOLUTE SKY
# DIRECTION is held across a reference switch (set home_planet) and free-mode
# entry/exit (absFwd fixed, alt/az moved by the inter-frame rotation).
# Predictive validation (11.15 method): AoI thresholds COMPUTED offline from
# a baseline dump (repaired updateCache/updateReach formulas transcribed below),
# then the reference-switch sequence asserted at bracketing altitudes. Brackets
# stay WIDE (x0.7 / x1.5) around the Earth boundary as historical margin, but
# the launch-jd AoI LATCH they tolerated is FIXED (INTENT 11.62, B15):
# areaOfInfluence is now recomputed every frame from the current eclipticPos,
# so it tracks the jd instead of freezing at the first cache (which used to
# differ ~10% from the scene jd -> the first run's e_in flap). The B15 block at
# the tail asserts that tracking deterministically (season-differ + live
# ref-switch) - a re-latch reintroduces the miss.
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

# ---- B13 view-continuity (INTENT 11.61): absolute sky direction across a
# reference switch (set home_planet = warpToBody) and free-mode entry/exit.
# absFwd (Camera dump) is the ROOT-aligned look direction (frame-independent);
# alt/az is frame-relative. Keeping the ABSOLUTE direction leaves absFwd fixed
# while alt/az MOVES by the inter-frame rotation (the discriminator). Pre-B13
# warpToBody held alt/az fixed and JUMPED absFwd ~78 deg -> these asserts FAIL
# on that behavior (proven by temporary revert), PASS after.
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "look_at azimuth 60 altitude 30 duration 0", 1.5)          # non-degenerate aim
send(s, "body action dual_dump filename /tmp/spine_vc0.json", 2)   # Earth, aimed
send(s, "set home_planet Mars", 2)
send(s, "body action dual_dump filename /tmp/spine_vc1.json", 2)   # Mars (warpToBody A->B)
send(s, "set home_planet Earth", 2)
send(s, "body action dual_dump filename /tmp/spine_vc2.json", 2)   # Earth back (B->A)
send(s, "camera action free_mode state on", 1.5)
send(s, "body action dual_dump filename /tmp/spine_vc3.json", 2)   # free enter
send(s, "camera action free_mode state off", 1.5)
send(s, "body action dual_dump filename /tmp/spine_vc4.json", 2)   # free exit
send(s, "camera action free_mode state on", 1.5)
send(s, "body action dual_dump filename /tmp/spine_vc5.json", 2)   # free enter #2
send(s, "camera action free_mode state off", 1.5)
send(s, "body action dual_dump filename /tmp/spine_vc6.json", 2)   # free exit  #2

# ---- B15 AoI-tracks-jd block (INTENT 11.62): Earth's area of influence is
# Moon-distance dominated, so it MUST change when the date is jumped. Before
# the fix it froze at the launch-jd cache (the "e_in first-run flap" cause);
# these dates + asserts turn that latch into a deterministic, launch-INDEPENDENT
# failure (the AoI at two fixed ephemeris dates is fixed; a re-latch makes them
# equal -> the season-differ assert fails). D1/D2 are half a Julian year apart.
D1 = 2461233.5            # scene jd
D2 = 2461233.5 + 182.62   # +1/2 year
def earth_aoi_pred(bb):   # transcription of updateReach() (scaling=1)
    moon_sub  = 1.1 * bb["Moon"]["boundingRadius"]
    earth_sub = 1.1 * max(bb["Earth"]["boundingRadius"], n(bb["Moon"]["ecl"]) + moon_sub)
    aoi = max(bb["Earth"]["boundingRadius"]*128, earth_sub*16)
    cap = n(bb["Earth"]["ecl"])*0.6
    return min(aoi, cap) if cap > 0 else aoi
send(s, "camera action free_mode state off", 1)
send(s, "set home_planet Earth", 2)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1.5)
send(s, f"date jday {D1}", 1.5)
send(s, "body action dual_dump filename /tmp/spine_aoi_d1.json", 2)
send(s, f"date jday {D2}", 1.5)
send(s, "body action dual_dump filename /tmp/spine_aoi_d2.json", 2)
send(s, f"date jday {D1}", 1.5)   # reversible: back to D1, AoI must return
send(s, "body action dual_dump filename /tmp/spine_aoi_d1b.json", 2)
# Live reference transition at a FIXED altitude inside the season window: the
# lower-AoI date must escalate (Sun), the higher-AoI date must stay (Earth).
# A frozen AoI gives the SAME reference at both -> the discriminator.
pA = earth_aoi_pred(bodies("/tmp/spine_aoi_d1.json"))
pB = earth_aoi_pred(bodies("/tmp/spine_aoi_d2.json"))
alt_test = (pA + pB) / 2.0
def live_rung(date, tag):
    send(s, "camera action free_mode state off", 1)
    send(s, "set home_planet Earth", 2)            # force reference = Earth
    send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1.5)
    send(s, "camera action free_mode state on", 1)
    send(s, f"date jday {date}", 1.5)
    send(s, f"moveto altitude {int(alt_test*AU_M)} duration 0", 3)
    send(s, f"body action dual_dump filename /tmp/spine_{tag}.json", 2)
live_rung(D1, "aoi_live1")
live_rung(D2, "aoi_live2")
send(s, "camera action free_mode state off", 1)
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

# ---- B13 view-continuity assertions (INTENT 11.61) -------------------------
def _absfwd(tag): return cam(f"/tmp/spine_{tag}.json")["absFwd"]
def _altaz(tag):
    c = cam(f"/tmp/spine_{tag}.json"); al, az = c["alt"], c["az"]; ca = math.cos(al)
    return (math.cos(az)*ca, -math.sin(az)*ca, -math.sin(al))
def _ang(a, b):
    na = math.sqrt(sum(x*x for x in a)); nb = math.sqrt(sum(x*x for x in b))
    return math.degrees(math.acos(max(-1.0, min(1.0, sum(x*y for x, y in zip(a, b))/(na*nb)))))
ABS_TOL = 0.05    # absolute sky direction HELD (deg): Euler(~6e-6)+B30(~5e-3)
                  # floor << 0.05; pre-B13 warpToBody jumps ~78 deg -> discriminates.
DISC_MIN = 10.0   # alt/az MUST move across a ref switch = proof the ABSOLUTE (not
                  # frame-relative) direction was held. Pre-B13 alt/az delta = 0.
vc = [
    ("vc ref-switch E->M abs held", _ang(_absfwd("vc0"), _absfwd("vc1")), "<", ABS_TOL),
    ("vc ref-switch M->E abs held", _ang(_absfwd("vc1"), _absfwd("vc2")), "<", ABS_TOL),
    ("vc ref-switch E->M discrim ", _ang(_altaz("vc0"),  _altaz("vc1")),  ">", DISC_MIN),
    ("vc free enter abs held     ", _ang(_absfwd("vc2"), _absfwd("vc3")), "<", ABS_TOL),
    ("vc free exit  abs held     ", _ang(_absfwd("vc3"), _absfwd("vc4")), "<", ABS_TOL),
    ("vc free enter#2 abs held   ", _ang(_absfwd("vc4"), _absfwd("vc5")), "<", ABS_TOL),
    ("vc free exit#2  abs held   ", _ang(_absfwd("vc5"), _absfwd("vc6")), "<", ABS_TOL),
    # switchToBody (free-flight auto-transition) continuity: the ladder's first
    # escalation Earth->Sun keeps the absolute direction too (moveto altitude
    # moves position, not the look rotation; switchToBody re-derives alt/az).
    ("vc auto-switch e_in->e_out ", _ang(_absfwd("e_in"), _absfwd("e_out")), "<", ABS_TOL),
]
for name, val, op, thr in vc:
    ok = (val < thr) if op == "<" else (val > thr)
    print(f"{'OK ' if ok else 'FAIL'} {name} {val:9.5f} deg {op} {thr}", flush=True)
    fail += not ok

# ---- B15 AoI-tracks-jd assertions (INTENT 11.62) -------------------------
cd1  = cam("/tmp/spine_aoi_d1.json");  pD1  = earth_aoi_pred(bodies("/tmp/spine_aoi_d1.json"))
cd2  = cam("/tmp/spine_aoi_d2.json");  pD2  = earth_aoi_pred(bodies("/tmp/spine_aoi_d2.json"))
cd1b = cam("/tmp/spine_aoi_d1b.json")
aD1, aD2, aD1b = cd1["refAoI"], cd2["refAoI"], cd1b["refAoI"]
errD1   = abs(aD1 - pD1)/pD1 * 100.0
errD2   = abs(aD2 - pD2)/pD2 * 100.0
season  = abs(aD2 - aD1)/aD1 * 100.0     # frozen -> 0; tracking -> ~11%
revert  = abs(aD1b - aD1)/aD1 * 100.0
liveA   = cam("/tmp/spine_aoi_live1.json")["reference"]   # date D1
liveB   = cam("/tmp/spine_aoi_live2.json")["reference"]   # date D2
lo_ref, hi_ref = (liveA, liveB) if pD1 < pD2 else (liveB, liveA)
b15 = [
    ("b15 AoI tracks jd D1 (<2%)  ",   errD1,  "<", 2.0),
    ("b15 AoI tracks jd D2 (<2%)  ",   errD2,  "<", 2.0),
    ("b15 AoI season-differs (>5%)",   season, ">", 5.0),   # latch discriminator
    ("b15 AoI reversible D1 (<0.5%)",  revert, "<", 0.5),
]
for name, val, op, thr in b15:
    ok = (val < thr) if op == "<" else (val > thr)
    print(f"{'OK ' if ok else 'FAIL'} {name} {val:9.5f}% {op} {thr}", flush=True)
    fail += not ok
ok = (lo_ref == "Sun" and hi_ref == "Earth")   # live season-dependent transition
print(f"{'OK ' if ok else 'FAIL'} b15 live ref season-switch loAoI={lo_ref} hiAoI={hi_ref} "
      f"(want Sun/Earth) appAoI[D1={aD1:.4e} D2={aD2:.4e}]", flush=True)
fail += not ok
sys.exit(1 if fail else 0)
