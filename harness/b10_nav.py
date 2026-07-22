#!/usr/bin/env python3
# B10 datum_radius/ground_radius verification driver (INTENT 11.71).
# Numeric altitude/clamp layer (float-exact), per the row: the landing scenes
# are the TRAP; the discriminating cases are where the two radii DIFFER.
#
# Run the SAME driver at HEAD (baseline, keys inert) and post-change:
#  - shipped/default bodies: distances IDENTICAL before/after (regression gate)
#  - test bodies with datum!=ground!=radius: distances DIFFER in the predicted
#    way (enterable lands at centre; clearance descent stops at ground_radius).
#
# Camera dump gives `distance` (AU, anchored) and `position` (AU, free mode).
import socket, time, json, math, sys, os

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b10"
os.makedirs(OUT, exist_ok=True)

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def dump(sock, tag, pause=1.5):
    path = f"{OUT}/b10_{tag}.json"
    send(sock, f"body action dual_dump filename {path}", pause)
    return path

def cam(tag):
    with open(f"{OUT}/b10_{tag}.json") as f:
        return json.loads(f.readline())["camera"]

def plen(c):
    p = c["position"]
    return math.sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2])

AU_M = 149597870700.0
AU_KM = 149597870.7

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)   # pin new path
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)

# ---- REGRESSION: shipped body (Earth), defaults = radius -------------------
send(s, "set home_planet Earth", 2)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1.5);  dump(s, "earth_a100")
send(s, "moveto altitude 50000 duration 0", 1.5);              dump(s, "earth_a50k")
send(s, "moveto altitude 0 duration 0", 1.5);                  dump(s, "earth_a0")

# ---- Create test bodies (children of Sun, far & isolated so their AoI is the
# local reference; anchored, then free where the clamp is exercised) ---------
COMMON = ("parent Sun type Planet oblateness 0.0 albedo 0.3 halo false "
          "color 0.6,0.6,0.9 tex_map bodies/moon.png coord_func still_orbit")
# EnterTest: datum=0, ground=0  (enterable / transparent-body case)
send(s, f'body action load name EnterTest radius 6000 datum_radius 0 ground_radius 0 '
        f'{COMMON} orbit_x 900 orbit_y 0 orbit_z 0', 2)
# ClearTest: datum=radius, ground=radius*1.002 (terrain clearance)
send(s, f'body action load name ClearTest radius 6000 datum_radius 6000 ground_radius 6012 '
        f'{COMMON} orbit_x 0 orbit_y 900 orbit_z 0', 2)
# DefaultTest: no datum/ground keys -> both default to radius (inert)
send(s, f'body action load name DefaultTest radius 6000 '
        f'{COMMON} orbit_x 0 orbit_y 0 orbit_z 900', 3)

# ---- DefaultTest: defaults inert (distance = radius + altitude) -------------
send(s, "set home_planet DefaultTest", 3)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5);  dump(s, "def_a100")
send(s, "moveto altitude 0 duration 0", 1.5);           dump(s, "def_a0")

# ---- EnterTest: datum=ground=0 -> moveto altitude 0 lands at CENTRE ---------
send(s, "set home_planet EnterTest", 3)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5);  dump(s, "enter_a100")
send(s, "moveto altitude 0 duration 0", 1.5);           dump(s, "enter_a0")
send(s, "moveto altitude 100000 duration 0", 1.5);      dump(s, "enter_a100k")

# ---- ClearTest: datum=radius (altitude legacy-exact), ground=radius*1.002 ---
#      free-mode descent STOPS (holds) at ground_radius (12 km above datum).
send(s, "set home_planet ClearTest", 3)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5);  dump(s, "clear_a100")   # anchored altitude
send(s, "moveto altitude 0 duration 0", 1.5);           dump(s, "clear_a0")     # anchored: no clamp
# free-mode reversible pair x2: descend -> hold -> ascend -> escape
send(s, "camera action free_mode state on", 1.5)
send(s, "moveto altitude 0 duration 0", 2);             dump(s, "clear_f_desc1") # -> clamp @ ground
send(s, "moveto altitude 100000 duration 0", 2);        dump(s, "clear_f_asc1")  # escape (100 km)
send(s, "moveto altitude 0 duration 0", 2);             dump(s, "clear_f_desc2") # -> clamp @ ground (2nd)
send(s, "moveto altitude 100000 duration 0", 2);        dump(s, "clear_f_asc2")  # escape (2nd)
send(s, "camera action free_mode state off", 1)
s.close()

# ---- report ----------------------------------------------------------------
def line(tag):
    c = cam(tag)
    return c, c["distance"], plen(c), c["reference"], c["freeMode"]

print("\n==== B10 numeric layer ====", flush=True)
rows = ["earth_a100","earth_a50k","earth_a0","def_a100","def_a0",
        "enter_a100","enter_a0","enter_a100k","clear_a100","clear_a0",
        "clear_f_desc1","clear_f_asc1","clear_f_desc2","clear_f_asc2"]
for t in rows:
    try:
        c, d, pl, ref, fm = line(t)
        print(f"{t:16s} ref={ref:12s} free={str(fm):5s} distance={d:.9e} |pos|={pl:.9e}  "
              f"dist_km={d*AU_KM:.3f} pos_km={pl*AU_KM:.3f}", flush=True)
    except Exception as e:
        print(f"{t:16s} ERROR {e}", flush=True)
print("done", flush=True)
