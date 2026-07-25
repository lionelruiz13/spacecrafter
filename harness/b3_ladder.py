#!/usr/bin/env python3
"""B3 DEPTH LADDER - the discriminating instrument for defect 5.29 (INTENT
11.101(c), task F1-P1).  Grounded bodies of known size on the Moon, seen from
nadir, against the parent's ray-march depth.

WHAT IT DISCRIMINATES
---------------------
`bodyRayMarch.frag` never writes gl_FragDepth (:147 discard-only), so every
surviving ray fragment writes the depth of the PROXY SHELL it was rasterized on
(`LayeredMesh.cpp:239`, radius = min(scaledRadius*(1+0.01*altimetryLevel),
distance - scaledRadius/64)) while showing the terrain beneath it.  On the Moon
at altimetry level 2 the shell stands 0.02*1737.4 = 34.748 km above the datum:
a DEPTH WALL at constant altitude, independent of the observer's distance, over
the whole distance < 64*scaledRadius ray regime.  Anything grounded whose top
is below that wall is depth-killed while still being loaded, lit, placed and
CASTING ITS SHADOW (the shadow pass is offscreen and does not depth-test
against the parent) - the signature 11.101(c) recorded.

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
the literal-reproduction attempt.

THE MODEL (all lengths km; everything below is derived from source and
measured, never fitted)
  R      = 1737.4                     Moon radius [ssystem.ini [moon] radius]
  A      = 0.02                       altimetryFactor, config level 2 [11.100(a)]
  Rs     = R*(1+A) = 1772.148         proxy shell (unclamped: distance 9737.4
                                      >> R*(1+A), LayeredMesh.cpp:239 min())
  wall_pre  = Rs - R = 34.748         PRE-FIX depth wall altitude, terrain-blind
  wall_post = A*R*h = 34.748*h        POST-FIX wall = the true terrain, h =
                                      heightmap value at the site (bodyRayMarch
                                      .frag:59-62 maps tex 0..1 to R..R*(1+A))
  A grounded sphere of drawn radius r, centre at datum+alt, is cut by the wall
  sphere of radius R+w at
      q = (w*(2R+w) - r*r + 2R*alt + alt*alt) / (2*(R+alt))   [exact, two-sphere
  intersection: the cut plane sits q above the body centre]           ]
  visible cap radius = sqrt(r*r - q*q)   (q >= r  =>  INVISIBLE, 0 px)
  projected px radius = cap / dist * 1024 / halfFov   (fisheye: screen radius =
      theta/halfFov * viewportRadius; viewportRadius = 2048/2 [render 2048])

PRE-STATED PREDICTIONS -> see b3_ladder_predict.json, written by --predict
BEFORE any run of the state it predicts (the 11.99->11.100 FM-4 rule: the
expected values and their derivation are recorded before the measurement).

PRECONDITIONS (violations are asserted, not assumed)
  - `flag moon_scaled off`: 5.27 - grounded children do not ride the parent's
    display scaling, and the shipped moon_scale=5 swallows them (11.100).
  - fresh launch per scene; temp-HOME farm (11.103(a)) so the real
    ~/.spacecrafter is never written (md5 asserted by the runner).
  - the ray regime: 2*R < distance < 64*R  (below 2*R the parent enters the
    empty-groundedComponents surface-regime hole, 11.97(e)/11.100(g)(ii)).
  - each site must be LIT (asserted on the baseline shot).

usage: b3_ladder.py <outdir> <pre|post> [--families sph,cur]
       b3_ladder.py <outdir> --predict          (write the prediction file only)
"""
import json, math, os, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
REAL_HOME = Path.home()
FARM = Path(os.environ.get("B3_FARM", "/tmp/b3_farm"))
AU_KM = 149597870.0

# ---- scene ---------------------------------------------------------------
JD = 2461234.0            # b24_screen's calibrated date (lon ~60 lit)
OBS_LON = 39.7            # `moveto lon` value; see NADIR_LON
OBS_LAT = 0.0
# MEASURED CONVENTION (F1-P1 calibration run, 5 bodies, jd 2461234): the
# sub-observer point of `moveto lat 0 lon L` sits at surface_point longitude
# 180 - L.  Derived twice, independently: (i) cos psi from each body's dumped
# distance gives nadir_lon = 127.000 +- 0.001 for L = 53 over 5 bodies at
# lon 41.5..67.2; (ii) the camera dump's own `position` (reference-relative-to-
# observer, the point-reflected convention) has longitude -53.000000 exactly,
# and the observer direction is its negation => 180 - L.  `moveto lon` and
# `orbit_lon` are therefore NOT the same longitude authority - recorded as an
# out-of-scope find; here it is a calibration, ASSERTED per leg through the
# dumped observer->body distance (A0dist), so a convention change fails loudly.
NADIR_LON = 180.0 - OBS_LON
OBS_ALT_M = 8000000       # 8000 km  => distance 9737.4 km (ray regime, 11.101(c))
RENDER = 2048             # screenshot side [measured: existing artifacts]
FOV_WIDE = 20.0           # 11.101(c)/b24_screen geometry
FOV_ZOOM = 10.0           # 2x finer (0.68 km/px at 8000 km); half-field 5 deg
                          # still covers the whole ladder line (max 3.03 deg
                          # off-nadir at the +430 km site)

R_MOON = 1737.4           # [observed: ~/.spacecrafter/ssystem.ini [moon] radius]
ALTIMETRY = 0.02          # 0.01 * altimetryLevel(2) [11.100(a), config level 2]
WALL_PRE = R_MOON * ALTIMETRY

SPHERE_MODEL = "Star_White"   # exact unit sphere (verified at runtime)
ROVER_MODEL = "Curiosity"     # 11.101(c)'s body; model radius 3.009647

# ladder: (tag, drawn radius km, altitude km, lateral offset km from the nadir)
# offsets keep every pair apart by >= r_i + r_j + 30 km on the surface, and the
# whole line inside the fov-10 half-field (max 3.70 deg off-axis at +530 km).
# b30 exists because the POST-fix wall at this site is the real relief
# (~23-25 km), which brackets differently from the 34.748 km pre-fix wall:
# 20 km stays buried in BOTH states (predicted, and a guard against a "fix"
# that merely stops depth-testing), 30 km is the FLIP leg.
LADDER = [
    ("lift20", 20.0, 150.0, -400.0),   # the LIFTED CONTROL (11.101(c))
    ("b20",    20.0,   0.0, -250.0),
    ("b30",    30.0,   0.0, -160.0),
    ("b45",    45.0,   0.0,  -40.0),
    ("b90",    90.0,   0.0,  140.0),
    ("b250",  250.0,   0.0,  530.0),
]

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)


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


def site_lon(offset_km):
    return NADIR_LON + offset_km / (R_MOON * math.pi / 180.0)


_DEM = None
def dem_at(lon_deg, lat_deg=0.0, halfwin_deg=0.6):
    """Heightmap value at a surface point, over the FOUR candidate texture-u
    conventions.  The ray-march reads u = atan2(y,x)/2pi + 0.5 in the frame
    mat*zrot(axisRotation) (LayeredMesh.cpp:243 removes the +pi/2), while a
    surface_point sits in the frame mat*zrot(axisRotation+pi/2) - so
    u = 0.5 + (lon+90)/360 by derivation, but the chain crosses three sign
    conventions (spheToRect, Mat4f::zrotation, atan2), so all four of
    0.5 +- (lon+90)/360 and 0.5 +- lon/360 are carried as hypotheses and the
    POST-fix measurement discriminates them (the fix makes the rendered wall
    height BE the local terrain, so each leg measures its own site).
    Returns the four h values (MEAN over a +-halfwin window - the body covers a
    finite patch of ground and its cap rim samples it; the window's min/max are
    reported separately as the terrain-roughness diagnostic)."""
    global _DEM
    if _DEM is None:
        im = Image.open(REAL_HOME / ".spacecrafter/textures/bodies/moon_alti.jpg").convert("L")
        _DEM = np.asarray(im, dtype=np.float32) / 255.0
    a = _DEM; H, W = a.shape
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
        q0 = ((R+w)^2 - (R+alt)^2 - r^2) / (2*(R+alt)) ."""
    Rc = R_MOON + alt
    q = (w * (2 * R_MOON + w) - 2 * R_MOON * alt - alt * alt - r * r) / (2 * Rc)
    if q >= r:
        return 0.0
    if q <= -r:
        return r          # fully clear of the wall
    return math.sqrt(r * r - q * q)


def observer_distance(alt, off):
    """Exact observer->body distance: the body sits at surface angle
    theta = off/R from the nadir, at radius R+alt; the observer at R+OBS_ALT."""
    th = off / R_MOON
    rb = R_MOON + alt
    ro = R_MOON + OBS_ALT_M / 1000.0
    return math.hypot(rb * math.sin(th), ro - rb * math.cos(th))


def px_radius(cap_km, dist_km, fov_deg):
    return cap_km / dist_km / (fov_deg * math.pi / 360.0) * (RENDER / 2)


def predictions():
    smod, smin, sn = model_radius(SPHERE_MODEL)
    rmod, rmin, rn = model_radius(ROVER_MODEL)
    dist_nadir = R_MOON + OBS_ALT_M / 1000.0
    out = {"generated": "before any run of the state it predicts",
           "model": {SPHERE_MODEL: {"radius": smod, "min_over_max": smin / smod, "verts": sn},
                     ROVER_MODEL: {"radius": rmod, "min_over_max": rmin / rmod, "verts": rn}},
           "wall_pre_km": WALL_PRE, "dist_nadir_km": dist_nadir, "legs": {}}
    for tag, r, alt, off in LADDER:
        lon = site_lon(off)
        stats = dem_at(lon, halfwin_deg=max(0.6, r / (R_MOON * math.pi / 180.0)))
        hs = [s[0] for s in stats]
        d = observer_distance(alt, off)
        # foreshortening: the cap's rim circle lies in the plane normal to the
        # LOCAL RADIAL, seen from a direction (psi - theta) off it.
        psi = off / R_MOON
        theta = math.asin(min(1.0, (R_MOON + alt) * math.sin(psi) / d)) if off else 0.0
        fore = math.cos(abs(psi) - abs(theta))
        leg = {"r_km": r, "alt_km": alt, "lateral_km": off, "lon": lon,
               "dem_h": hs, "dem_window_minmax": [[s[1], s[2]] for s in stats],
               "wall_post_km": [h * WALL_PRE for h in hs],
               "dist_km": d, "psi_deg": math.degrees(psi), "theta_deg": math.degrees(theta),
               "foreshorten": fore,
               "authored_radius_sphere": r / smod, "authored_radius_rover": r / rmod}
        # PRE: wall at 34.748 km everywhere (terrain-blind)
        cpre = cap_radius(r, alt, WALL_PRE)
        leg["cap_pre_km"] = cpre
        leg["px_pre"] = {f"fov{int(f)}": px_radius(cpre, d, f) * math.sqrt(fore)
                         for f in (FOV_WIDE, FOV_ZOOM)}
        # POST: wall = local terrain (four candidate DEM conventions)
        leg["cap_post_km"] = [cap_radius(r, alt, h * WALL_PRE) for h in hs]
        leg["px_post"] = {f"fov{int(f)}": [px_radius(c, d, f) * math.sqrt(fore)
                                           for c in leg["cap_post_km"]]
                          for f in (FOV_WIDE, FOV_ZOOM)}
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
    return (f"\n[{name}]\nname = {name}\nparent = Moon\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {lon:.6f}\norbit_lat = {lat}\norbit_alt = {alt}\n"
            f"radius = {authored_radius:.9f}\nmodel_name = {model}\nhalo = false\n"
            f"[{name}:OJM]\nbody = {name}\ntype = OJM\n")


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
        send(s, "meteors zhr 0"); send(s, f"date jday {JD}")
        send(s, "set home_planet Moon", 2)
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off"); send(s, "flag landscape off")
        send(s, "flag moon_scaled off", 2)          # 5.27 instrument precondition
        send(s, "select planet Moon")
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 5)
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
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    pred = predictions()
    (out / "b3_ladder_predict.json").write_text(json.dumps(pred, indent=1))
    if len(sys.argv) > 2 and sys.argv[2] == "--predict":
        print(json.dumps(pred, indent=1)); return 0
    state = sys.argv[2]
    families = (sys.argv[4].split(",") if len(sys.argv) > 4 and sys.argv[3] == "--families"
                else ["sph", "cur"])
    assert state in ("pre", "post")

    smod = pred["model"][SPHERE_MODEL]["radius"]
    rmod = pred["model"][ROVER_MODEL]["radius"]
    if abs(pred["model"][SPHERE_MODEL]["min_over_max"] - 1.0) > 1e-4:
        fail(f"{SPHERE_MODEL} is not an exact sphere "
             f"(min/max |v| = {pred['model'][SPHERE_MODEL]['min_over_max']}) - ladder model invalid")

    twin = (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes()
    scenes = {"base": ""}
    if "sph" in families:
        scenes["sph"] = "".join(section("S" + t, SPHERE_MODEL, site_lon(o), 0.0, alt, r / smod)
                                for t, r, alt, o in LADDER)
    if "cur" in families:
        scenes["cur"] = "".join(section("C" + t, ROVER_MODEL, site_lon(o), 0.0, alt, r / rmod)
                                for t, r, alt, o in LADDER)

    imgs = {}; dumps = {}
    for tag, sec in scenes.items():
        print(f"--- launch {tag} ({state}) ---", flush=True)
        imgs[tag], dumps[tag] = run_scene(out, f"{state}_{tag}", sec, twin)

    base = imgs["base"]
    moon = dumps["base"].get("Moon", {})
    dist_km = moon.get("dist", 0) * AU_KM
    # self-calibrated px scale: screenSize = scaledRadius / (dist * halfFov)
    half_fov_meas = (moon.get("scaledDatumRadius", 0) / (moon.get("dist", 1) * moon.get("screenSize", 1))
                     if moon.get("screenSize") else 0)
    report = {"state": state, "moon_dist_km": dist_km,
              "halfFov_measured_rad": half_fov_meas,
              "halfFov_nominal_rad": FOV_WIDE * math.pi / 360.0,
              "legs": {}}
    print(f"Moon dist {dist_km:.1f} km  halfFov meas {half_fov_meas:.6f} "
          f"nominal {FOV_WIDE*math.pi/360:.6f}", flush=True)

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
        for tag, r, alt, off in LADDER:
            name = prefix + tag
            b = dumps[fam].get(name)
            leg = {"family": fam, "present": b is not None}
            if b:
                leg.update(drawn_radius_km=b.get("boundingRadius", 0) * AU_KM,
                           relation=b.get("relation"), visible=b.get("visible"),
                           dist_km=b.get("dist", 0) * AU_KM, screen=b.get("screen"))
                # fisheye is radially linear in theta (custom_project.glsl), so a
                # fov change scales the offset from the view axis by the halfFov
                # ratio; the view axis = the tracked Moon centre (dumped).
                mcx, mcy = screen_px(moon.get("screen", [0, 0]))
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
            print(f"[{name}] r={r} alt={alt} lon={p['lon']:.3f} "
                  f"pred_cap_pre={p['cap_pre_km']:.2f}km ({p['px_pre']['fov20']:.1f}px) "
                  f"pred_cap_post={p['cap_post_km'][0]:.2f}/{p['cap_post_km'][1]:.2f}km  "
                  f"MEASURED wide.changed={leg.get('wide',{}).get('changed')} "
                  f"eqR={leg.get('wide',{}).get('eq_radius_px',0):.1f}px  "
                  f"zoom.changed={leg.get('zoom',{}).get('changed')} "
                  f"eqR={leg.get('zoom',{}).get('eq_radius_px',0):.1f}px  "
                  f"shadowbox.dark={leg.get('shadow',{}).get('wide_dark')}", flush=True)

    # ---------------- asserts ------------------------------------------------
    for fam, prefix in (("sph", "S"), ("cur", "C")):
        if fam not in scenes:
            continue
        for tag, r, alt, off in LADDER:
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

    (out / f"b3_ladder_{state}.json").write_text(json.dumps({"report": report, "pred": pred,
                                                             "fails": FAILS}, indent=1))
    print(f"\n{'LADDER GREEN' if not FAILS else str(len(FAILS)) + ' FAILURES'} "
          f"-> b3_ladder_{state}.json", flush=True)
    return 1 if FAILS else 0


def check_sphere_ladder(report, pred, state):
    """The ladder's own verdict.  PRE = 11.101(c)'s red signature on the exact
    sphere geometry (wall = 34.748 km, terrain-blind); POST = the same
    arithmetic with the wall at the LOCAL TERRAIN.  Both sides are predicted
    BEFORE the run (predictions() / b3_ladder_predict.json)."""
    res = []
    for tag, r, alt, off in LADDER:
        L = report["legs"]["S" + tag]; p = pred["legs"][tag]
        if not L.get("present"):
            continue
        meas = L["zoom"]["eq_radius_px"]; ch = L["zoom"]["changed"]
        want = ([p["px_pre"][f"fov{int(FOV_ZOOM)}"]] if state == "pre"
                else p["px_post"][f"fov{int(FOV_ZOOM)}"])
        lo, hi = min(want), max(want)
        if hi <= 1.0:
            res.append((ch < 40, f"{tag}: predicted INVISIBLE (cap "
                                 f"{p['cap_pre_km'] if state == 'pre' else p['cap_post_km']} km) "
                                 f"- measured {ch} changed px (bound 40)"))
        else:
            res.append((lo * 0.80 <= meas <= hi * 1.20,
                        f"{tag}: predicted cap radius {lo:.1f}-{hi:.1f} px, measured "
                        f"{meas:.1f} px ({ch} px) - 20% band"))
    # the depth-independent witness: a depth-killed body must still cast its
    # shadow (the shadow pass is offscreen and does not depth-test the parent)
    for tag in ("b20", "b30"):
        L = report["legs"].get("S" + tag, {})
        if L.get("present"):
            d = L["shadow"]["wide_dark"]
            res.append((d > 40, f"{tag}: shadow witness {d} dark px in the wide box "
                                f"(loaded/lit/placed while its colour draw is depth-killed)"))
    return res


if __name__ == "__main__":
    sys.exit(main())
