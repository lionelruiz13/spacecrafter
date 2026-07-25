#!/usr/bin/env python3
"""B3 DEPTH LADDER - the discriminating instrument for the ray-regime depth
defects: 5.29 (Moon site, INTENT 11.101(c)/11.104, task F1-P1) and 5.30 +
5.33 (Earth site, task F1-P2).  Grounded spheres of known size and known
ALTITUDE on a parent body, seen from nadir, against the parent's depth.

WHAT IT DISCRIMINATES
---------------------
A grounded body is drawn AFTER its parent, in the parent's MERGED depth
bucket (Renderer.hpp partitioning contract), so whatever the parent left in
the depth buffer is a WALL: everything below it is depth-killed while still
being loaded, lit, placed and CASTING ITS SHADOW (the shadow pass is
offscreen and does not depth-test against the parent) - the signature
11.101(c) recorded.  Four wall hypotheses, each a STATE of this instrument,
each predicted from source arithmetic BEFORE the run it predicts:

  none     no depth written at all  => nothing is ever occluded, every leg
           renders its FULL silhouette however deep it is buried.
           [Earth at HEAD 922701c9 with the ATMOSPHERE module removed:
            LayeredMesh.cpp:229 ORs VARIANT_NO_DEPTH for the NIGHT row =>
            depthTest = depthWrite = false - defect 5.30]
  shell    wall = the PROXY SHELL the ray-march is rasterized on,
           scaledRadius*(1+0.01*altimetryLevel) (LayeredMesh.cpp:239),
           terrain-blind and observer-independent.
           [Moon before 922701c9 - defect 5.29; and the failure mode the
            11.104(c) rider warns about for Earth: "no depth" -> "shell
            depth" if the row flag is cleared without the frag depth write]
  terrain  wall = the TRUE surface, 0.01*altimetryLevel*R*h with h the
           heightmap value at the site (bodyRayMarch*.frag maps tex 0..1
           onto R..R*(1+0.01*level)).  The fixed state.
  atm      wall = the ATMOSPHERE shell, scaledRadius*atmosphere_radius_factor
           (AtmExtModule: BMT_TRANSLUCENT, depth test+write ON, drawn LAST in
           the parent's near list) - a SECOND wall, taller than both the proxy
           shell and any terrain on Earth (1.03 vs 1.02 vs 1.02*h).
           [Earth at HEAD, and Earth with 5.30 fixed but 5.33 not - defect 5.33]

Why THIS ladder body: an exact unit-sphere OJM (Star_White, 2143 vertices, all
at |v| = 1.000001 - measured from the model file at runtime, printed).  A
sphere's extent above its placement point is its drawn radius in EVERY
direction, so no leg depends on the model's orientation.  That matters: a
grounded body's frame is the parent's SURFACE frame (ModularBody.hpp:535 folds
parent->computeBodyToSurface(); the child adds only zrot(axisRotation+pi/2) at
draw), i.e. its model axes are NOT aligned with the local vertical, and the
extent of an asymmetric mesh above the ground therefore depends on the site
LONGITUDE.  11.101(c)'s ladder used Curiosity, whose extent above the placement
point is f*r with f in [0.0015, 0.7533] depending on that orientation - so its
size thresholds are not reproducible without its scene file.  The CUR family
below runs the same sizes with Curiosity anyway, reported (not asserted), as
the literal-reproduction attempt (moon site only).

THE MODEL (all lengths km; everything below is derived from source and
measured, never fitted)
  R      = parent radius              [ssystem.ini <parent> radius]
  A      = 0.01 * altimetry_level     config moon_/planet_altimetry_level = 2
  Rs     = R*(1+A)                    proxy shell (unclamped: the clamp
                                      min(..., distance - scaledRadius/64) of
                                      LayeredMesh.cpp:239 needs alt < 0.0356*R,
                                      unreachable in the steady-state draw path
                                      - 11.104(c))
  wall_shell   = A*R                  terrain-blind
  wall_terrain = A*R*h                h = heightmap value at the site
  wall_atm     = (atm_factor-1)*R     atmosphere_radius_factor - R, 0 if the
                                      body has no ATMOSPHERE module
  A grounded sphere of drawn radius r, centre at datum+alt, is cut by the wall
  sphere of radius R+w at
      q = (w*(2R+w) - r*r + 2R*alt + alt*alt) / (2*(R+alt))   [exact, two-sphere
  intersection: the cut plane sits q above the body centre]           ]
  visible cap radius = sqrt(r*r - q*q) for 0 < q < r; r for q <= 0 (the cut
      plane is at or below the body centre, so the silhouette is the body's
      own equator, not the cut circle); 0 for q >= r (INVISIBLE)
  projected px radius = cap / dist * 1024 / halfFov   (fisheye: screen radius =
      theta/halfFov * viewportRadius; viewportRadius = 2048/2 [render 2048])

KNOWN INSTRUMENT PROPERTIES (11.104(d2), inherited; do not re-derive)
  * cap radius carries a -2% SYSTEMATIC (the |delta| > 16 diff threshold eats
    the antialiased rim): transferable bias 0.9770 +- 0.20%, measured on a
    state whose wall is exact.  It is MULTIPLICATIVE (the lost annulus scales
    with the radius), so a ratio transfers and a px offset does not.
  * reading resolution +-1 px; the wall inversion steepens as cap -> the full
    silhouette: dwall/dpx = (cap/q0) * (km per px).  A leg METERS the wall
    when cap ~ q0 and carries NO wall information when cap >> q0 (q0 -> 0).
  * sqrt(foreshortening) is applied to EVERY leg's prediction including the
    fully-clear ones, where the true silhouette is orientation-free (a sphere
    projects to a circle from any direction).  Bounded error: 1-sqrt(cos|psi-
    theta|) <= 0.8% on the moon site's lifted control, <= 0.04% on the earth
    ladder.  Kept as-is so 11.104(b)/(d)'s numbers stay reproducible.

PRE-STATED PREDICTIONS -> see b3_ladder_predict.json, written by --predict
BEFORE any run of the state it predicts (the 11.99->11.100 FM-4 rule: the
expected values and their derivation are recorded before the measurement).

F1-P2 EARTH PREDICTIONS, COMMITTED BEFORE THE RUNS THEY PREDICT
---------------------------------------------------------------
Site earth / earth_noatm, jd 2461234, `moveto lat 0 lon 270` (measured lit:
centre luma 177 vs 10-11 elsewhere), alt 10000 km => distance 16378.14 km,
inside the ray band [12756.28, 408200.96] km.  R = 6378.14 km, A = 0.02,
atm_factor = 1.03 => wall_shell = 127.5628 km, wall_atm = 191.3442 km,
wall_terrain = 127.5628*h with h = 0.043-0.047 at this OCEAN site under ALL
FOUR candidate texture-u conventions (window 0.0431-0.0471 => 5.50-6.01 km),
so the convention ambiguity of 11.104(g) does not reach this prediction.
Cap radius in px at fov 10 (equivalent-area radius of the present-vs-absent
diff), predicted per state:

  leg   alt     none    shell   terrain    atm     role
  n150  -150    51.7      0        0         0     GUARD: top at -105 km, dark
                                                   under ANY wall >= 0
  n60    -60    52.3      0        0         0
  n30    -30    52.6      0     31.9-32.3    0     terrain metering leg
                                                   (dwall/dpx 0.65-0.67)
  n10    -10    52.7      0     49.0-49.5    0
  a0       0    52.8      0     52.4-52.4    0     shell-vs-terrain: 0 vs 52 px
  a30     30    52.9      0       52.9       0
  a100   100    53.3    42.2      53.3       0
  a160   160    53.5    53.5      53.5     38.5    atm metering leg
                                                   (dwall/dpx 0.87)
  a400   400    54.7    54.7      54.7     54.7    invariant control

RUN PLAN and what each run decides (each prediction committed before its run):
  R1  earth_noatm @ code 922701c9   expect state `none`  - 5.30 RED: Earth
      writes no depth at all, so even a body buried 105 km below the datum
      renders whole.
  R2  earth       @ code 922701c9   expect state `atm`   - the SECOND wall,
      191.34 km, present at the same code with the same ladder: the delta
      between R1 and R2 IS the atmosphere shell's depth write (5.33).
  R3  earth_noatm @ 5.30 fixed      expect state `terrain` - the FLIP.  The
      rider's failure mode (row flag cleared without the frag depth write)
      would read state `shell` instead: a0 0 px vs 52 px, a100 42 vs 53.
  R4  earth       @ 5.30 fixed      expect state `atm`, UNCHANGED from R2 -
      the row fix alone does NOT deliver grounded occlusion on stock Earth.
  R5  earth       @ 5.33 also fixed expect state `terrain` = R3.

PRECONDITIONS (violations are asserted, not assumed)
  - `flag moon_scaled off`: 5.27 - grounded children do not ride the parent's
    display scaling, and the shipped moon_scale=5 swallows them (11.100).
    The dumped scaledDatumRadius is asserted against the authored radius, so
    ANY live scaling on the parent fails the run (the earth site has no
    shipped scale flag; the assert is what makes that a measurement).
  - fresh launch per scene; temp-HOME farm (11.103(a)) so the real
    ~/.spacecrafter is never written (md5 asserted by the runner).
  - the ray regime: 2*R < distance < 64*R  (below 2*R the parent enters the
    empty-groundedComponents surface-regime hole, 11.97(e)/11.100(g)(ii)).
  - each site must be LIT (asserted on the baseline shot).

usage: b3_ladder.py <outdir> <state> [--site moon|earth|earth_noatm]
                                     [--families sph,cur]
       b3_ladder.py <outdir> --predict [--site ...]
  state = none | shell | terrain | atm   (aliases: pre=shell, post=terrain)
"""
import json, math, os, re, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
REAL_HOME = Path.home()
FARM = Path(os.environ.get("B3_FARM", "/tmp/b3_farm"))
AU_KM = 149597870.0

RENDER = 2048             # screenshot side [measured: existing artifacts]
FOV_WIDE = 20.0           # 11.101(c)/b24_screen geometry
FOV_ZOOM = 10.0           # 2x finer; half-field 5 deg still covers the ladder

SPHERE_MODEL = "Star_White"   # exact unit sphere (verified at runtime)
ROVER_MODEL = "Curiosity"     # 11.101(c)'s body; model radius 3.009647

# ---- sites ---------------------------------------------------------------
# A site is a parent body + an observer station + a ladder.  `nadir_lon` uses
# the MEASURED convention (F1-P1 calibration, 5 bodies, jd 2461234): the
# sub-observer point of `moveto lat 0 lon L` sits at surface_point longitude
# 180 - L.  Derived twice, independently: (i) cos psi from each body's dumped
# distance gives nadir_lon = 127.000 +- 0.001 for L = 53 over 5 bodies at
# lon 41.5..67.2; (ii) the camera dump's own `position` (reference-relative-to-
# observer, the point-reflected convention) has longitude -53.000000 exactly,
# and the observer direction is its negation => 180 - L.  `moveto lon` and
# `orbit_lon` are therefore NOT the same longitude authority (11.104(g),
# recorded not fixed); here it is a calibration, ASSERTED per leg through the
# dumped observer->body distance (A0dist), so a convention change fails loudly.
#
# ladder legs: (tag, drawn radius km, altitude km, lateral offset km from nadir)

# MOON (F1-P1, 11.104): a SIZE ladder at alt 0 - the pre-fix wall (34.748 km)
# and the post-fix relief (20-24 km) bracket differently by SIZE there.
# offsets keep every pair apart by >= r_i + r_j + 30 km on the surface, and the
# whole line inside the fov-10 half-field (max 3.70 deg off-axis at +530 km).
LADDER_MOON = [
    ("lift20", 20.0, 150.0, -400.0),   # the LIFTED CONTROL (11.101(c))
    ("b20",    20.0,   0.0, -250.0),
    ("b30",    30.0,   0.0, -160.0),
    ("b45",    45.0,   0.0,  -40.0),
    ("b90",    90.0,   0.0,  140.0),
    ("b250",  250.0,   0.0,  530.0),
]

# EARTH (F1-P2, 5.30/5.33): an ALTITUDE ladder at fixed radius.  Earth carries
# THREE candidate walls at once - atm 191.34 km (1.03), shell 127.56 km (1.02),
# terrain 127.56*h km - so a fixed radius r = 45 km with the altitude swept
# from -150 to +400 km separates all four states at several legs each, and the
# separation does NOT depend on knowing the heightmap convention:
#   n150 is buried below the DATUM itself (top at -105 km), so it must be dark
#        under ANY wall >= 0 - the guard against a "fix" that merely stops
#        depth-testing, and the REVERSE-flip witness (visible -> dark);
#   a0   discriminates shell (dark) from terrain (44.7 km, near-full) by 45 px;
#   a160 meters the atm wall (cap ~ q0 there);
#   n30  meters the terrain wall at this ocean site (cap ~ q0 there);
#   a400 is the invariant control (clear of every candidate wall).
LADDER_EARTH = [
    ("n150", 45.0, -150.0, -640.0),   # buried below the datum - GUARD
    ("n60",  45.0,  -60.0, -480.0),
    ("n30",  45.0,  -30.0, -320.0),   # terrain-wall metering leg (ocean site)
    ("n10",  45.0,  -10.0, -160.0),
    ("a0",   45.0,    0.0,    0.0),   # shell-vs-terrain discriminator (nadir)
    ("a30",  45.0,   30.0,  160.0),
    ("a100", 45.0,  100.0,  320.0),
    ("a160", 45.0,  160.0,  480.0),   # atm-wall metering leg
    ("a400", 45.0,  400.0,  640.0),   # the LIFTED CONTROL (clear of every wall)
]

SITES = {
    "moon": dict(
        parent="Moon", radius=1737.4,       # [observed: ~/.spacecrafter/ssystem.ini [moon] radius]
        altimetry=0.02,                     # 0.01 * moon_altimetry_level(2) [config]
        dem="bodies/moon_alti.jpg",
        atm_factor=None,                    # no ATMOSPHERE module on the Moon
        jd=2461234.0,                       # b24_screen's calibrated date
        obs_lon=39.7, obs_lat=0.0, obs_alt_m=8000000,
        ladder=LADDER_MOON, drop_sections=(),
        scale_off=("flag moon_scaled off",),   # 5.27 precondition
    ),
    "earth": dict(
        parent="Earth", radius=6378.14,     # [observed: ~/.spacecrafter/ssystem.ini [earth] radius]
        altimetry=0.02,                     # 0.01 * planet_altimetry_level(2) [config]
        dem="bodies/earth_alti.png",
        atm_factor=1.03,                    # [observed: [earth] atmosphere_radius_factor]
        jd=2461234.0,
        obs_lon=270.0,                      # MEASURED lit: centre luma 177 vs 10-11
        obs_lat=0.0, obs_alt_m=10000000,    # distance 16378 km, ray band [2R, 64R]
        ladder=LADDER_EARTH, drop_sections=(),
        scale_off=("flag moon_scaled off",),
    ),
    # Counterfactual: the SAME Earth with its ATMOSPHERE module removed from
    # the composed twin.  Isolates the MESH row's own depth (5.30) from the
    # atmosphere shell's (5.33) - the two walls are independent and the stock
    # body carries both.
    "earth_noatm": dict(
        parent="Earth", radius=6378.14, altimetry=0.02,
        dem="bodies/earth_alti.png", atm_factor=None,
        jd=2461234.0, obs_lon=270.0, obs_lat=0.0, obs_alt_m=10000000,
        ladder=LADDER_EARTH, drop_sections=("Earth:ATMOSPHERE",),
        scale_off=("flag moon_scaled off",),
    ),
}

S = SITES["moon"]     # selected in main()
FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)

STATE_ALIAS = {"pre": "shell", "post": "terrain"}
STATES = ("none", "shell", "terrain", "atm")


# ---- model geometry ------------------------------------------------------
def model_radius(name):
    """Ojm::radius = max |v| over the .ojm vertices (ojm.cpp:355/492); the
    authored `radius` is a SCALE FACTOR on it (OjmLoader.cpp:42)."""
    r2 = 0.0; rmin = 1e18; n = 0
    for line in open(REAL_HOME / f".spacecrafter/model3D/{name}/{name}.ojm", errors="replace"):
        if line.startswith("v "):
            p = line[2:].split()
            d = float(p[0])**2 + float(p[1])**2 + float(p[2])**2
            r2 = max(r2, d); rmin = min(rmin, d); n += 1
    return math.sqrt(r2), math.sqrt(rmin), n


def nadir_lon():
    return 180.0 - S["obs_lon"]


def site_lon(offset_km):
    return nadir_lon() + offset_km / (S["radius"] * math.pi / 180.0)


_DEM = {}
def dem_at(lon_deg, lat_deg=0.0, halfwin_deg=0.6):
    """Heightmap value at a surface point, over the FOUR candidate texture-u
    conventions.  The ray-march reads u = atan2(y,x)/2pi + 0.5 in the frame
    mat*zrot(axisRotation) (LayeredMesh.cpp:243 removes the +pi/2), while a
    surface_point sits in the frame mat*zrot(axisRotation+pi/2) - so
    u = 0.5 + (lon+90)/360 by derivation, but the chain crosses three sign
    conventions (spheToRect, Mat4f::zrotation, atan2), so all four of
    0.5 +- (lon+90)/360 and 0.5 +- lon/360 are carried as hypotheses and the
    fixed-state measurement discriminates them (the fix makes the rendered wall
    height BE the local terrain, so each leg measures its own site).
    Returns the four h values (MEAN over a +-halfwin window - the body covers a
    finite patch of ground and its cap rim samples it; the window's min/max are
    reported separately as the terrain-roughness diagnostic)."""
    key = S["dem"]
    if key not in _DEM:
        im = Image.open(REAL_HOME / ".spacecrafter/textures" / key).convert("L")
        _DEM[key] = np.asarray(im, dtype=np.float32) / 255.0
    a = _DEM[key]; H, W = a.shape
    out = []
    for base_lon in (lon_deg + 90.0, lon_deg):
        for sign in (1, -1):
            u = (0.5 + sign * base_lon / 360.0) % 1.0
            v = 0.5 - sign * lat_deg / 180.0
            x = int(u * W) % W; y = min(H - 1, max(0, int(v * H)))
            n = max(1, int(halfwin_deg / 360.0 * W))
            idx = [(x + i) % W for i in range(-n, n + 1)]
            w = a[y, idx]
            out.append((float(w.mean()), float(w.min()), float(w.max())))
    return out


def cap_radius(r, alt, w):
    """Visible cap radius of a sphere of radius r whose centre sits at
    datum+alt, cut by the wall sphere of radius R+w.  0 => invisible.
    Exact two-sphere intersection: a point c+q of the body sphere clears the
    wall iff |c+q|^2 > (R+w)^2 <=> q_radial > q0 with
        q0 = ((R+w)^2 - (R+alt)^2 - r^2) / (2*(R+alt)) .
    q0 <= 0 means the cut plane sits at or below the body CENTRE: the widest
    visible horizontal extent is then the body's own equator, r - NOT the cut
    circle sqrt(r^2-q0^2), which is smaller.  (F1-P2 correction; no F1-P1 leg
    was in the band -r < q0 < 0, so 11.104(b)/(d) are unaffected - the moon
    ladder's buried legs all had q0 > 0 and its lifted control q0 < -r.)"""
    R = S["radius"]
    Rc = R + alt
    q = (w * (2 * R + w) - 2 * R * alt - alt * alt - r * r) / (2 * Rc)
    if q >= r:
        return 0.0
    if q <= 0:
        return r          # clear of the wall at the body's own equator
    return math.sqrt(r * r - q * q)


def q0_of(r, alt, w):
    R = S["radius"]
    return (w * (2 * R + w) - 2 * R * alt - alt * alt - r * r) / (2 * (R + alt))


def observer_distance(alt, off):
    """Exact observer->body distance: the body sits at surface angle
    theta = off/R from the nadir, at radius R+alt; the observer at R+OBS_ALT."""
    R = S["radius"]
    th = off / R
    rb = R + alt
    ro = R + S["obs_alt_m"] / 1000.0
    return math.hypot(rb * math.sin(th), ro - rb * math.cos(th))


def px_radius(cap_km, dist_km, fov_deg):
    return cap_km / dist_km / (fov_deg * math.pi / 360.0) * (RENDER / 2)


def walls():
    """The candidate walls at this site, in km above the datum, each derived
    from source (terrain is per-leg, so it is filled in predictions())."""
    R = S["radius"]
    return {"shell": R * S["altimetry"],
            "atm": (S["atm_factor"] - 1.0) * R if S["atm_factor"] else None,
            "none": 0.0}


def predictions():
    smod, smin, sn = model_radius(SPHERE_MODEL)
    rmod, rmin, rn = model_radius(ROVER_MODEL)
    R = S["radius"]
    W = walls()
    dist_nadir = R + S["obs_alt_m"] / 1000.0
    out = {"generated": "before any run of the state it predicts",
           "site": S["parent"], "site_key": S.get("key"),
           "model": {SPHERE_MODEL: {"radius": smod, "min_over_max": smin / smod, "verts": sn},
                     ROVER_MODEL: {"radius": rmod, "min_over_max": rmin / rmod, "verts": rn}},
           "wall_shell_km": W["shell"], "wall_atm_km": W["atm"],
           "dist_nadir_km": dist_nadir, "legs": {}}
    for tag, r, alt, off in S["ladder"]:
        lon = site_lon(off)
        stats = dem_at(lon, halfwin_deg=max(0.6, r / (R * math.pi / 180.0)))
        hs = [s[0] for s in stats]
        d = observer_distance(alt, off)
        # foreshortening: the cap's rim circle lies in the plane normal to the
        # LOCAL RADIAL, seen from a direction (psi - theta) off it.
        psi = off / R
        theta = math.asin(min(1.0, (R + alt) * math.sin(psi) / d)) if off else 0.0
        fore = math.cos(abs(psi) - abs(theta))
        leg = {"r_km": r, "alt_km": alt, "lateral_km": off, "lon": lon,
               "dem_h": hs, "dem_window_minmax": [[s[1], s[2]] for s in stats],
               "wall_terrain_km": [h * W["shell"] for h in hs],
               "dist_km": d, "psi_deg": math.degrees(psi), "theta_deg": math.degrees(theta),
               "foreshorten": fore,
               "km_per_px": {f"fov{int(f)}": d * (f * math.pi / 360.0) / (RENDER / 2)
                             for f in (FOV_WIDE, FOV_ZOOM)},
               "authored_radius_sphere": r / smod, "authored_radius_rover": r / rmod}
        # one entry per STATE: list of candidate caps (>1 only for terrain,
        # whose wall depends on the unresolved texture-u convention)
        caps = {"none": [r],
                "shell": [cap_radius(r, alt, W["shell"])],
                "terrain": [cap_radius(r, alt, h * W["shell"]) for h in hs]}
        q0s = {"none": [q0_of(r, alt, 0.0)],
               "shell": [q0_of(r, alt, W["shell"])],
               "terrain": [q0_of(r, alt, h * W["shell"]) for h in hs]}
        if W["atm"] is not None:
            caps["atm"] = [cap_radius(r, alt, W["atm"])]
            q0s["atm"] = [q0_of(r, alt, W["atm"])]
        leg["cap_km"] = caps
        leg["q0_km"] = q0s
        leg["px"] = {st: {f"fov{int(f)}": [px_radius(c, d, f) * math.sqrt(fore) for c in cs]
                          for f in (FOV_WIDE, FOV_ZOOM)}
                     for st, cs in caps.items()}
        # dwall/dpx = (cap/q0) * km_per_px - the metering sensitivity
        # (11.104(d2)).  DEFINED ONLY in the strict band 0 < q0 < r: outside it
        # the cap is saturated (q0 <= 0, silhouette = r) or the leg is dark
        # (q0 >= r), and it carries NO wall information at all - reported null,
        # never a finite number, so a saturated leg can never be read as metric.
        leg["dwall_dpx"] = {st: [(abs(c / q) * leg["km_per_px"]["fov10"]
                                  if 0 < q < r else None)
                                 for c, q in zip(cs, q0s[st])]
                            for st, cs in caps.items()}
        out["legs"][tag] = leg
    return out


# ---- app driving ---------------------------------------------------------
def wait_port(timeout=120):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def shot(sock, out, name, pause=2.5):
    p = out / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(30):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def section(name, model, lon, lat, alt, authored_radius):
    return (f"\n[{name}]\nname = {name}\nparent = {S['parent']}\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {lon:.6f}\norbit_lat = {lat}\norbit_alt = {alt}\n"
            f"radius = {authored_radius:.9f}\nmodel_name = {model}\nhalo = false\n"
            f"[{name}:OJM]\nbody = {name}\ntype = OJM\n")


def drop_sections(twin_bytes, names):
    """Remove whole [Name] sections from the composed twin (counterfactual
    module removal - the farm's copy only, 11.103(a))."""
    if not names:
        return twin_bytes
    text = twin_bytes.decode("latin-1")
    for n in names:
        new = re.sub(r"^\[" + re.escape(n) + r"\]\s*$.*?(?=^\[|\Z)", "", text,
                     flags=re.M | re.S)
        if new == text:
            fail(f"drop_sections: [{n}] not found in the composed twin - "
                 f"the counterfactual did NOT apply")
        text = new
    return text.encode("latin-1")


def load_dump(path):
    bodies = {}
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "body" and o.get("new") is not None:
            bodies[o["name"]] = o["new"]
    return bodies


def run_scene(out, tag, sections, twin):
    """One fresh launch against the temp HOME.  Returns (images, dump)."""
    farmdir = FARM / ".spacecrafter"
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(twin + sections.encode("latin-1"))
    for f in (farmdir / "log").glob("*.log"):
        f.unlink()
    env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([SC_BIN], cwd=str(farmdir),
                            stdout=open(out / f"{tag}.applog", "w"), stderr=subprocess.STDOUT, env=env)
    imgs = {}; dump = {}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "flag experimental_path on"); send(s, "timerate rate 0")
        send(s, "meteors zhr 0"); send(s, f"date jday {S['jd']}")
        send(s, f"set home_planet {S['parent']}", 2)
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off"); send(s, "flag landscape off")
        for c in S["scale_off"]:
            send(s, c, 2)
        send(s, f"select planet {S['parent']}")
        send(s, f"moveto lat {S['obs_lat']} lon {S['obs_lon']} alt {S['obs_alt_m']} duration 0", 5)
        send(s, "flag track_object on", 2)
        send(s, f"zoom fov {FOV_WIDE} duration 0", 2)
        send(s, "flag track_object off", 2)         # B30 determinism
        p = out / f"{tag}_dump.json"
        send(s, f"body action dual_dump filename {p}", 2)
        dump = load_dump(p)
        send(s, "flag experimental_shadows off", 2)
        imgs["wide"] = shot(s, out, f"{tag}_wide")
        send(s, f"zoom fov {FOV_ZOOM} duration 0", 2)
        imgs["zoom"] = shot(s, out, f"{tag}_zoom")
        send(s, f"zoom fov {FOV_WIDE} duration 0", 2)
        send(s, "flag experimental_shadows on", 2)
        imgs["shadow"] = shot(s, out, f"{tag}_shadow")
        send(s, "zoom fov 150 duration 0", 2)          # whole-disc witness (no assert)
        shot(s, out, f"{tag}_disc")
        send(s, f"zoom fov {FOV_WIDE} duration 0", 2)
        send(s, "shutdown action now", 1); s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
    return imgs, dump


# ---- measurement ---------------------------------------------------------
def screen_px(screen):
    return (screen[0] + 1) / 2 * RENDER, (1 - screen[1]) / 2 * RENDER


def box_counts(a, b, cx, cy, half):
    """changed / bright / dark px in the box around (cx, cy)."""
    x0, x1 = int(max(0, cx - half)), int(min(RENDER, cx + half))
    y0, y1 = int(max(0, cy - half)), int(min(RENDER, cy + half))
    if x1 <= x0 or y1 <= y0:
        return 0, 0, 0
    da = a[y0:y1, x0:x1].max(axis=2).astype(np.int32)
    db = b[y0:y1, x0:x1].max(axis=2).astype(np.int32)
    d = da - db
    return int((np.abs(d) > 16).sum()), int((d > 16).sum()), int((d < -16).sum())


def main():
    global S
    argv = sys.argv[1:]
    site_key = "moon"
    if "--site" in argv:
        i = argv.index("--site"); site_key = argv[i + 1]; del argv[i:i + 2]
    families = ["sph", "cur"]
    if "--families" in argv:
        i = argv.index("--families"); families = argv[i + 1].split(","); del argv[i:i + 2]
    S = SITES[site_key]; S["key"] = site_key
    # ABSOLUTE: the app runs with cwd = the farm's .spacecrafter, so a relative
    # outdir would make it write the dump/screenshots somewhere else (silently -
    # the failure surfaces only as a missing dump).
    out = Path(argv[0]).resolve(); out.mkdir(parents=True, exist_ok=True)
    pred = predictions()
    (out / "b3_ladder_predict.json").write_text(json.dumps(pred, indent=1))
    if len(argv) > 1 and argv[1] == "--predict":
        print(json.dumps(pred, indent=1)); return 0
    state = STATE_ALIAS.get(argv[1], argv[1])
    assert state in STATES, f"state must be one of {STATES} (or pre/post)"
    if state == "atm" and S["atm_factor"] is None:
        print(f"FATAL: state 'atm' on site {site_key} which has no ATMOSPHERE module")
        return 2

    smod = pred["model"][SPHERE_MODEL]["radius"]
    rmod = pred["model"][ROVER_MODEL]["radius"]
    if abs(pred["model"][SPHERE_MODEL]["min_over_max"] - 1.0) > 1e-4:
        fail(f"{SPHERE_MODEL} is not an exact sphere "
             f"(min/max |v| = {pred['model'][SPHERE_MODEL]['min_over_max']}) - ladder model invalid")

    twin = drop_sections(
        (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes(),
        S["drop_sections"])
    scenes = {"base": ""}
    if "sph" in families:
        scenes["sph"] = "".join(section("S" + t, SPHERE_MODEL, site_lon(o), 0.0, alt, r / smod)
                                for t, r, alt, o in S["ladder"])
    if "cur" in families:
        scenes["cur"] = "".join(section("C" + t, ROVER_MODEL, site_lon(o), 0.0, alt, r / rmod)
                                for t, r, alt, o in S["ladder"])

    imgs = {}; dumps = {}
    for tag, sec in scenes.items():
        print(f"--- launch {tag} ({site_key}/{state}) ---", flush=True)
        imgs[tag], dumps[tag] = run_scene(out, f"{state}_{tag}", sec, twin)

    base = imgs["base"]
    parent = dumps["base"].get(S["parent"], {})
    dist_km = parent.get("dist", 0) * AU_KM
    # self-calibrated px scale: screenSize = scaledRadius / (dist * halfFov)
    half_fov_meas = (parent.get("scaledDatumRadius", 0) / (parent.get("dist", 1) * parent.get("screenSize", 1))
                     if parent.get("screenSize") else 0)
    report = {"site": site_key, "state": state, "parent_dist_km": dist_km,
              "halfFov_measured_rad": half_fov_meas,
              "halfFov_nominal_rad": FOV_WIDE * math.pi / 360.0,
              "parent_scaledDatumRadius_km": parent.get("scaledDatumRadius", 0) * AU_KM,
              "parent_boundingRadius_km": parent.get("boundingRadius", 0) * AU_KM,
              "parent_modules": parent.get("modules"), "parent_routing": parent.get("routing"),
              "legs": {}}
    print(f"{S['parent']} dist {dist_km:.1f} km  halfFov meas {half_fov_meas:.6f} "
          f"nominal {FOV_WIDE*math.pi/360:.6f}", flush=True)

    # PRECONDITION P0: the parent is drawn UNSCALED and in the ray regime, and
    # the counterfactual module set is the one this site declares.
    sdr = report["parent_scaledDatumRadius_km"]
    if abs(sdr - S["radius"]) / S["radius"] > 1e-3:
        fail(f"P0 {S['parent']} scaledDatumRadius {sdr:.2f} km != authored radius "
             f"{S['radius']} km - a display scaling is live; every prediction is void")
    else:
        ok(f"P0 {S['parent']} drawn unscaled: scaledDatumRadius {sdr:.2f} km")
    if dist_km and not (2 * S["radius"] < dist_km < 64 * S["radius"]):
        fail(f"P0 {S['parent']} distance {dist_km:.1f} km outside the ray band "
             f"[{2*S['radius']:.0f}, {64*S['radius']:.0f}] km")
    mods = report["parent_modules"] or []
    has_atm = "ATMOSPHERE" in mods
    if has_atm != (S["atm_factor"] is not None):
        fail(f"P0 {S['parent']} ATMOSPHERE module present={has_atm} but the site "
             f"declares atm_factor={S['atm_factor']} - counterfactual mismatch")
    else:
        ok(f"P0 module set as declared ({mods}), ATMOSPHERE present={has_atm}")

    # precondition: the parent is drawn and the ladder line is lit
    centre = base["wide"][RENDER//2-50:RENDER//2+50, RENDER//2-50:RENDER//2+50].max(axis=2)
    report["centre_luma"] = float(centre.mean())
    if centre.mean() > 30:
        ok(f"P1 parent present and lit at nadir: centre luma {centre.mean():.1f}")
    else:
        fail(f"P1 nadir centre luma {centre.mean():.1f} - parent absent or unlit")

    for fam, prefix in (("sph", "S"), ("cur", "C")):
        if fam not in scenes:
            continue
        for tag, r, alt, off in S["ladder"]:
            name = prefix + tag
            b = dumps[fam].get(name)
            leg = {"family": fam, "present": b is not None}
            if b:
                leg.update(drawn_radius_km=b.get("boundingRadius", 0) * AU_KM,
                           relation=b.get("relation"), visible=b.get("visible"),
                           dist_km=b.get("dist", 0) * AU_KM, screen=b.get("screen"))
                # fisheye is radially linear in theta (custom_project.glsl), so a
                # fov change scales the offset from the view axis by the halfFov
                # ratio; the view axis = the tracked parent centre (dumped).
                mcx, mcy = screen_px(parent.get("screen", [0, 0]))
                bcx, bcy = screen_px(b["screen"])
                for view, fov in (("wide", FOV_WIDE), ("zoom", FOV_ZOOM), ("shadow", FOV_WIDE)):
                    k = FOV_WIDE / fov
                    cx, cy = mcx + (bcx - mcx) * k, mcy + (bcy - mcy) * k
                    rp = px_radius(r, leg["dist_km"], fov)
                    ch, br, dk = box_counts(imgs[fam][view], base[view], cx, cy, max(12, 1.35 * rp))
                    wch, wbr, wdk = box_counts(imgs[fam][view], base[view], cx, cy, max(80, 4 * rp))
                    leg.setdefault("box", {})[view] = [cx, cy, max(12, 1.35 * rp)]
                    x0, x1 = int(max(0, cx - 12)), int(min(RENDER, cx + 12))
                    y0, y1 = int(max(0, cy - 12)), int(min(RENDER, cy + 12))
                    site_luma = float(base[view][y0:y1, x0:x1].max(axis=2).mean()) if x1 > x0 and y1 > y0 else 0.0
                    leg[view] = dict(full_px_radius=rp, changed=ch, bright=br, dark=dk,
                                     wide_changed=wch, wide_dark=wdk, site_luma=site_luma,
                                     eq_radius_px=math.sqrt(ch / math.pi) if ch else 0.0)
            report["legs"][name] = leg
            p = pred["legs"][tag]
            want = p["px"][state][f"fov{int(FOV_ZOOM)}"]
            print(f"[{name}] r={r} alt={alt} lon={p['lon']:.3f} "
                  f"pred_cap[{state}]={[round(c,2) for c in p['cap_km'][state]]}km "
                  f"({min(want):.1f}-{max(want):.1f}px@fov10)  "
                  f"MEASURED wide.changed={leg.get('wide',{}).get('changed')} "
                  f"eqR={leg.get('wide',{}).get('eq_radius_px',0):.1f}px  "
                  f"zoom.changed={leg.get('zoom',{}).get('changed')} "
                  f"eqR={leg.get('zoom',{}).get('eq_radius_px',0):.1f}px  "
                  f"shadowbox.dark={leg.get('shadow',{}).get('wide_dark')}", flush=True)

    # ---------------- asserts ------------------------------------------------
    for fam, prefix in (("sph", "S"), ("cur", "C")):
        if fam not in scenes:
            continue
        for tag, r, alt, off in S["ladder"]:
            L = report["legs"][prefix + tag]
            if not L["present"]:
                fail(f"A0 {prefix+tag}: absent from the dump (not composed)"); continue
            exp_r = r
            if abs(L["drawn_radius_km"] - exp_r) / exp_r > 0.02:
                fail(f"A0 {prefix+tag}: drawn radius {L['drawn_radius_km']:.2f} km != "
                     f"authored*model {exp_r} km (OjmLoader.cpp:42 arithmetic)")
            if L["relation"] != 3 or not L["visible"]:
                fail(f"A0 {prefix+tag}: relation={L['relation']} visible={L['visible']} "
                     f"(expected grounded=3, visible)")
            pdist = pred["legs"][tag]["dist_km"]
            if abs(L["dist_km"] - pdist) / pdist > 0.01:
                fail(f"A0dist {prefix+tag}: observer->body {L['dist_km']:.1f} km != predicted "
                     f"{pdist:.1f} km - the `moveto lon` <-> `orbit_lon` calibration "
                     f"(nadir = 180 - L) no longer holds; every geometric prediction is void")
            if L["wide"]["site_luma"] < 30:
                fail(f"A0 {prefix+tag}: site not lit (baseline luma "
                     f"{L['wide']['site_luma']:.1f} < 30) - the ladder needs a lit site")
    if "sph" in scenes:
        checks = check_sphere_ladder(report, pred, state)
        for good, msg in checks:
            (ok if good else fail)(msg)

    (out / f"b3_ladder_{site_key}_{state}.json").write_text(
        json.dumps({"report": report, "pred": pred, "fails": FAILS}, indent=1))
    print(f"\n{'LADDER GREEN' if not FAILS else str(len(FAILS)) + ' FAILURES'} "
          f"-> b3_ladder_{site_key}_{state}.json", flush=True)
    return 1 if FAILS else 0


def check_sphere_ladder(report, pred, state):
    """The ladder's own verdict: the measured cap radius of every leg against
    the band THIS STATE's wall hypothesis predicts.  Both sides are predicted
    BEFORE the run (predictions() / b3_ladder_predict.json)."""
    res = []
    for tag, r, alt, off in S["ladder"]:
        L = report["legs"]["S" + tag]; p = pred["legs"][tag]
        if not L.get("present"):
            continue
        meas = L["zoom"]["eq_radius_px"]; ch = L["zoom"]["changed"]
        want = p["px"][state][f"fov{int(FOV_ZOOM)}"]
        lo, hi = min(want), max(want)
        if hi <= 1.0:
            res.append((ch < 40, f"{tag}: predicted INVISIBLE (cap "
                                 f"{[round(c,2) for c in p['cap_km'][state]]} km) "
                                 f"- measured {ch} changed px (bound 40)"))
        else:
            res.append((lo * 0.80 <= meas <= hi * 1.20,
                        f"{tag}: predicted cap radius {lo:.1f}-{hi:.1f} px, measured "
                        f"{meas:.1f} px ({ch} px) - 20% band"))
    # the depth-independent witness: a depth-killed body must still cast its
    # shadow (the shadow pass is offscreen and does not depth-test the parent).
    # Applies to every leg this state predicts INVISIBLE.
    for tag, r, alt, off in S["ladder"]:
        p = pred["legs"][tag]
        if max(p["px"][state][f"fov{int(FOV_ZOOM)}"]) > 1.0:
            continue
        L = report["legs"].get("S" + tag, {})
        if L.get("present"):
            d = L["shadow"]["wide_dark"]
            res.append((d > 40, f"{tag}: shadow witness {d} dark px in the wide box "
                                f"(loaded/lit/placed while its colour draw is depth-killed)"))
    return res


if __name__ == "__main__":
    sys.exit(main())
