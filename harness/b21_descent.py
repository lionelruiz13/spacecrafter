#!/usr/bin/env python3
# B21 view-directed free descent verification driver (INTENT 11.72).
# Numeric/vector layer (float-exact), per dispatch §2: descent geometry is
# measurable at the numeric layer (which direction "down" points), preferred
# over composed-screen A/B for the discriminating check.
#
# Driver: `camera action descend coef <c>` (new command routed this row - the
# view-directed descent geometry was UI-key-only before, B10 §11.71). coef<1
# descends, coef>1 ascends.
#
# Observables (from `body action dual_dump`):
#  - header["camera"]: reference, freeMode, position[3], distance, alt, az
#  - per-body "new"["dist"]: distance from the OBSERVER to that body
#    (= |eye-frame mat translation|) - the far-case metric.
import socket, time, json, math, sys, os

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b21"
os.makedirs(OUT, exist_ok=True)
AU_KM = 149597870.7

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def _clean(ln):
    return ln.strip().replace('-nan', 'null').replace('nan', 'null').replace('-inf','-1e308').replace('inf', '1e308')

def dump(sock, tag, pause=1.2):
    path = f"{OUT}/b21_{tag}.json"
    send(sock, f"body action dual_dump filename {path}", pause)
    return path

def load(tag):
    hdr = None; bodies = {}
    with open(f"{OUT}/b21_{tag}.json") as f:
        for ln in f:
            ln = _clean(ln)
            if not ln:
                continue
            try:
                o = json.loads(ln)
            except Exception:
                continue
            if o.get("type") == "header":
                hdr = o["camera"]
            elif o.get("type") == "body":
                bodies[o["name"]] = o
    return hdr, bodies

def cam(tag):
    return load(tag)[0]

def plen(c):
    p = c["position"]
    return math.sqrt(sum(x*x for x in p))

def bodydist(bodies, name):
    b = bodies.get(name)
    if not b or not b.get("new"):
        return None
    return b["new"]["dist"]

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)

COMMON = ("parent Sun type Planet oblateness 0.0 albedo 0.3 halo false "
          "color 0.6,0.6,0.9 tex_map bodies/moon.png coord_func still_orbit")
send(s, f'body action load name DescB radius 6000 {COMMON} '
        f'orbit_x 1200 orbit_y 0 orbit_z 0', 2)
send(s, f'body action load name DescClear radius 6000 datum_radius 6000 ground_radius 6012 '
        f'{COMMON} orbit_x 0 orbit_y 1200 orbit_z 0', 2)
send(s, f'body action load name DescEnter radius 6000 datum_radius 0 ground_radius 0 '
        f'{COMMON} orbit_x 0 orbit_y 0 orbit_z 1200', 2)
# Far-case targets: PERPENDICULAR (+y, +z), FAR from the system centre (12000 AU)
# so that from a just-above-system-AoI observer they are ANGULARLY SEPARATED
# (real planets sit ~1 AU from centre => <0.01deg apart from that distance,
# swamped by float noise; §11.72 far discriminator).
send(s, f'body action load name FarA radius 6000 {COMMON} orbit_x 0 orbit_y 12000 orbit_z 0', 2)
send(s, f'body action load name FarB radius 6000 {COMMON} orbit_x 0 orbit_y 0 orbit_z 20000', 3)

# =====================================================================
# PART 1 - NEAR SIGN (radial, tracked): descend must LOWER |pos| toward ground.
# =====================================================================
send(s, "set home_planet DescB", 3)
send(s, "camera action free_mode state on", 1.5)
send(s, "select planet DescB", 1)
send(s, "flag track_object on", 1)
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 2.5); dump(s, "near_sign_0")
for i in range(1, 6):
    send(s, "camera action descend coef 0.6", 1.0); dump(s, f"near_sign_{i}")

# =====================================================================
# PART 2 - VIEW-DIRECTED DISCRIMINATOR
#   From the tracked (centre-pointing) view, read alt0/az0, untrack, then
#   descend along two views tilted +/-DAZ in azimuth: view-directed => the two
#   landings sit at DIFFERENT surface points. The moveto-altitude control lands
#   at the SAME sub-observer point for both views (centre-directed).
# =====================================================================
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 2.5); dump(s, "p2_center")
c0 = cam("p2_center")
az0 = math.degrees(c0["az"]); alt0 = math.degrees(c0["alt"])
print(f"# tracked centre view: az0={az0:.3f} alt0={alt0:.3f} deg", flush=True)
send(s, "flag track_object off", 1)          # freeze the view (no re-aim)
DAZ = 35; NDESC = 10

def descent_run(tag, azoff):
    send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 1.2)
    send(s, f"look_at azimuth {az0+azoff} altitude {alt0} duration 0", 1.2)
    for _ in range(NDESC):
        send(s, "camera action descend coef 0.6", 0.7)
    dump(s, tag)

def moveto_run(tag, azoff):
    send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 1.2)
    send(s, f"look_at azimuth {az0+azoff} altitude {alt0} duration 0", 1.2)
    send(s, "moveto altitude 60000 duration 0", 1.5)   # ~60 km up, view-independent
    dump(s, tag)

descent_run("near_viewP", +DAZ)
descent_run("near_viewM", -DAZ)
moveto_run("near_ctrlP", +DAZ)
moveto_run("near_ctrlM", -DAZ)

# =====================================================================
# PART 3 - CLAMP COMPOSITION (R4 hold @ ground; reversible x2) + enter->centre
# =====================================================================
send(s, "set home_planet DescClear", 3)
send(s, "select planet DescClear", 1)
send(s, "flag track_object on", 1)
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 2.5)
for i in range(14): send(s, "camera action descend coef 0.4", 0.5)
dump(s, "clear_desc1")
for i in range(14): send(s, "camera action descend coef 2.0", 0.5)
dump(s, "clear_asc1")
for i in range(14): send(s, "camera action descend coef 0.4", 0.5)
dump(s, "clear_desc2")
for i in range(14): send(s, "camera action descend coef 2.0", 0.5)
dump(s, "clear_asc2")

send(s, "set home_planet DescEnter", 3)
send(s, "select planet DescEnter", 1)
send(s, "flag track_object on", 1)
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 2.5)
for i in range(18): send(s, "camera action descend coef 0.4", 0.5)
dump(s, "enter_desc")

# =====================================================================
# PART 4 - FAR CASE: descend from a SYSTEM reference aims at LAST SELECTED body.
#   Reach ref=SolarSystem (isSystem, and the solar bodies still iterate -> clean
#   obs->body distances; MilkyWay collapses them). AoI computed from a baseline
#   the scene_e way (max over Sun's children incl. the synthetic ones).
# =====================================================================
send(s, "flag track_object off", 1)
send(s, "camera action free_mode state off", 1)
send(s, "set home_planet Earth", 2)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
dump(s, "far_base")
_, bb = load("far_base")
def bnew(n): return bb.get(n, {}).get("new")
def necl(n):
    e = bnew(n)["ecl"]; return math.sqrt(sum(x*x for x in e))
# Sun.subsystem = 1.1 * max(Sun.bounding, max_child(|ecl_child| + 1.1*bounding_child))
sun_children_terms = []
for nm, o in bb.items():
    nw = o.get("new")
    if nw and nw.get("parent") == "Sun":
        e = nw["ecl"]; le = math.sqrt(sum(x*x for x in e))
        sun_children_terms.append(le + 1.1*nw["boundingRadius"])
sun_bounding = bnew("Sun")["boundingRadius"]
sun_sub = 1.1 * max([sun_bounding] + sun_children_terms)
sun_aoi = max(sun_bounding*128, sun_sub*16)   # Sun ecl==0 -> uncapped
sys_aoi = 1.1 * sun_sub * 16
FAR_AU = sun_aoi * 1.07                        # above Sun AoI, below system AoI (margin)
print(f"# sun_aoi={sun_aoi:.1f} AU  sys_aoi={sys_aoi:.1f} AU  fly_to={FAR_AU:.1f} AU", flush=True)
AU_M = 149597870700.0
# Small step -> the observer stays above sun_aoi so the reference stays
# SolarSystem across the before/after dumps (no de-escalation re-basing) and the
# selDist ratio is EXACTLY coef == proof the descent aims exactly at the target.
CF = "0.96"

def fly_to_system(seltarget):
    send(s, "camera action free_mode state off", 1)
    send(s, "set home_planet Earth", 2)
    send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.2)
    # SELECT while NEAR (searchByEnglishName finds the body only while its
    # system is the current old-path system; the selection PERSISTS through the
    # fly-out and reference escalation - it is reference-independent).
    send(s, f"select planet {seltarget}", 1.5)
    send(s, "camera action free_mode state on", 1)
    send(s, f"moveto altitude {int(FAR_AU*AU_M)} duration 0", 2.5)
    for _ in range(6):   # let the escalation cascade run frames
        send(s, "body action screenshot dummy_b21_esc", 0.4)

fly_to_system("FarA");                                     dump(s, "far0a")
send(s, f"camera action descend coef {CF}", 1.5);          dump(s, "far1a")
fly_to_system("FarB");                                     dump(s, "far0b")
send(s, f"camera action descend coef {CF}", 1.5);          dump(s, "far1b")
# BOGUS: missing coef -> no effect
fly_to_system("FarA");                                     dump(s, "far0z")
send(s, "camera action descend", 1.5);                     dump(s, "far1z")

send(s, "camera action free_mode state off", 1)
s.close()

# =====================================================================
# REPORT
# =====================================================================
print("\n==== B21 numeric/vector layer ====", flush=True)

print("\n-- PART 1: near sign (|pos| must DECREASE monotonically toward ground 6000) --")
for i in range(6):
    c = cam(f"near_sign_{i}")
    print(f"near_sign_{i} ref={c['reference']:10s} |pos|_km={plen(c)*AU_KM:.4f}", flush=True)

print("\n-- PART 2: view-directed discriminator --")
def endp(tag):
    c = cam(tag); return [x*AU_KM for x in c["position"]], plen(c)*AU_KM
def sep(t1, t2):
    a, La = endp(t1); b, Lb = endp(t2)
    dl = math.sqrt(sum((a[i]-b[i])**2 for i in range(3)))
    na = math.sqrt(sum(x*x for x in a)); nb = math.sqrt(sum(x*x for x in b))
    ca = max(-1, min(1, sum(a[i]*b[i] for i in range(3))/(na*nb))) if na>0 and nb>0 else 1
    return dl, math.degrees(math.acos(ca))
for t in ["near_viewP","near_viewM","near_ctrlP","near_ctrlM"]:
    p, L = endp(t)
    print(f"{t:11s} pos_km=({p[0]:.1f},{p[1]:.1f},{p[2]:.1f}) |pos|={L:.2f}", flush=True)
dv, av = sep("near_viewP", "near_viewM")
dc, ac = sep("near_ctrlP", "near_ctrlM")
print(f"  DESCEND +DAZ vs -DAZ: |ΔE|={dv:.2f} km, surface-angle={av:.3f} deg  (view-directed => LARGE)")
print(f"  CONTROL +DAZ vs -DAZ: |ΔE|={dc:.4f} km, surface-angle={ac:.5f} deg  (centre-directed => ~0)")

print("\n-- PART 3: clamp (hold@6012; enter->0) --")
for t in ["clear_desc1","clear_asc1","clear_desc2","clear_asc2","enter_desc"]:
    c = cam(t)
    print(f"{t:11s} ref={c['reference']:10s} |pos|_km={plen(c)*AU_KM:.4f}", flush=True)

print("\n-- PART 4: far case (descend aims at LAST SELECTED; selDist = obs->selected AU) --")
# Observable: the camera dump's selDist (= |getSelected()->getObservedPosition()|,
# the exact quantity the far-mode descent moves along). selDist FOLLOWS the
# selection, and descend multiplies it by EXACTLY coef -> the observer moves
# toward whatever is selected. A centre-directed descent would NOT hit coef for
# an off-centre body (counterfactual noted).
def cam4(tag):
    c = cam(tag); return c, c.get("selected", ""), c.get("selDist", float('nan'))
for t in ["far0a","far1a","far0b","far1b","far0z","far1z"]:
    c, sel, sd = cam4(t)
    print(f"{t:6s} ref={c['reference']:12s} selected={sel:8s} selDist={sd:.6e} AU", flush=True)
def sratio(t0, t1):
    _, _, s0 = cam4(t0); _, _, s1 = cam4(t1)
    return s1 / s0 if s0 else float('nan')
_, selA, sdA = cam4("far0a"); _, selB, sdB = cam4("far0b")
print(f"\n  selection drives target: FarA-selected selDist={sdA:.4e} vs FarB-selected selDist={sdB:.4e} "
      f"(DIFFER by {abs(sdA-sdB):.3e} AU => selDist follows the SELECTED body)")
print(f"  FarA SELECTED (coef {CF}): selDist ratio={sratio('far0a','far1a'):.6f} (== coef {CF})")
print(f"  FarB SELECTED (coef {CF}): selDist ratio={sratio('far0b','far1b'):.6f} (== coef {CF})")
print(f"  BOGUS (no coef):           selDist ratio={sratio('far0z','far1z'):.6f} (expect 1.0 = no move)")
print("done", flush=True)
