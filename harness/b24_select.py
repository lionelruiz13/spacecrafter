#!/usr/bin/env python3
"""B24-select - composed/new-only bodies must be SELECTABLE from BOTH channels.

WHAT THIS INSTRUMENT DISCRIMINATES (INTENT 11.106, task F2)
----------------------------------------------------------
11.97(d) measured the gap: `select planet <composed-name>` searches the OLD
system only, so tracked/selected stay EMPTY for a composed body - a 2(c)-class
capability-reachability defect (both the command channel and the pointer
channel are product surface).  This script measures the SAME two fields on the
same scene and asserts them POPULATED, plus the pointer channel, plus R5
(BIGGEST wins inside the pick tolerance, A17/11.70(d)), plus old-body parity.

Every leg is shown able to fail: run the whole script with SC_BIN pointing at
a pre-fix binary and the composed legs go red while the old-body legs stay
green (that is the red/green pair the entry records).

THE TWO OBSERVABLES, and why they are BOTH needed
  camera dump `selected`  = ModularBody::getSelected() - the NEW-path selection
                            aggregate (SSystemFactory::newSelectedBody).
  camera dump `tracked`   = Camera::target, written by Core::setFlagTracking,
                            which returns early unless **Core::selected_object**
                            is truthy (core.cpp:2113).  So `tracked` after a
                            `flag track_object on` is the witness that the
                            OLD-side selection object (the ObjectBase bridge)
                            is populated, which `selected` alone cannot show.
                            A wiring that set only newSelectedBody would pass
                            on `selected` and fail on `tracked`.

THE SCENE (all geometry derived from source constants, asserted from the dump)
  Moon site of b3_ladder (jd 2461234, observer lat 0 lon 39.7 alt 8000 km,
  fov 30 deg).  Sub-observer surface point = longitude 180-39.7 (11.104(g)).
  Four exact-unit-sphere (Star_White) composed bodies are grounded at
  psi = 60 deg from the sub-observer point, altitude 500 km, which puts them at
      alpha = atan2((R+A) sin psi, (R+H) - (R+A) cos psi) = 12.67 deg
  from the nadir axis while the Moon's limb is at asin(R/(R+H)) = 10.28 deg:
  they sit OFF THE DISC, against black sky.  That is the point - the OLD
  picker must find NOTHING there (no old body's disc contains the click, no
  old body's centre is within its 30 px tolerance, stars/nebulae off), so the
  new route is what the click exercises, and old picking is untouched by
  construction.  A control click at the Moon's own dumped screen centre must
  still select the Moon: the old channel, live, in the same run.

  BigA (100 km) and SmallB (25 km) are placed ~0.02 NDC apart so that ONE
  click point has BOTH centres inside the pick tolerance (30 render px =
  0.0293 NDC): that is the R5 cluster.  A second launch swaps their radii and
  the same click must follow the size - a mutation the ranking cannot fake.

  Far C (60 km) sits on the opposite side of the screen; clicking each body's
  own dumped position must select THAT body.  This is also the y-convention
  witness: ModularBody::screenPos has +1 at the TOP [derived: b3_ladder.py's
  validated py = (1-sy)/2*RENDER], while VulkanMgr::screenToRect maps window y
  (0 at the top) linearly onto [-1,+1], so rect_y = -screenPos_y.  If that
  derivation were wrong the C leg would select the wrong body or nothing.

THE POINTER CHANNEL IS DRIVEN BY A REAL X CLICK (xclick.c, XTEST): there is no
command-interface entry to UI::handleClic, and asserting the pick function
directly would sit one layer below the terminal observable.

PRECONDITIONS
  - temp-HOME symlink farm (11.103(a)) - the real ~/.spacecrafter is never
    written; the composed scene lives in the farm's modularSystem/.
  - fresh launch per scene; DISPLAY must be a real X server with XTEST.
  - `flag moon_scaled off` (5.27 instrument precondition, 11.100).

usage: b24_select.py <outdir> [--skip-parity]
       SC_BIN=<binary> to point at the pre-fix binary for the red run.
"""
import json, math, os, re, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
REAL_HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
FARM = Path(os.environ.get("B24SEL_FARM", "/tmp/b24_select_farm"))
WIN_NAME = "spacecrafter"          # SDL window title prefix [src/config.h APP_NAME]
AU_KM = 149597870.0

# ---- scene constants -----------------------------------------------------
JD = 2461234.0
PARENT = "Moon"
R_KM = 1737.4          # [observed: ~/.spacecrafter/ssystem.ini [moon] radius]
OBS_LON = 39.7         # b3_ladder moon site
OBS_LAT = 0.0
OBS_ALT_M = 8000000
FOV = 30.0             # halfFov 15 deg
ALT_KM = 500.0         # composed bodies' altitude above the Moon datum
PSI = 60.0             # great-circle offset of the cluster from the sub-observer point
MODEL = "Star_White"   # exact unit sphere (verified at runtime, b3_ladder method)
PICK_TOL_PX = 30       # the OLD path's own pick tolerance (core.cpp:1049)
RENDER = 2048          # render side [measured: existing artifacts]

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m):   print(f"ok:   {m}", flush=True)


def nadir_lon():
    return 180.0 - OBS_LON


def alpha_deg(psi_deg, alt_km):
    """Angle from the nadir axis of a body `alt_km` above the surface at
    great-circle distance psi from the sub-observer point."""
    psi = math.radians(psi_deg)
    a = R_KM + alt_km
    h = R_KM + OBS_ALT_M / 1000.0
    return math.degrees(math.atan2(a * math.sin(psi), h - a * math.cos(psi)))


def obs_dist_km(psi_deg, alt_km):
    psi = math.radians(psi_deg)
    a = R_KM + alt_km
    h = R_KM + OBS_ALT_M / 1000.0
    return math.hypot(a * math.sin(psi), h - a * math.cos(psi))


def model_radius(name):
    r2 = 0.0; rmin = 1e18
    for line in open(REAL_HOME / f".spacecrafter/model3D/{name}/{name}.ojm", errors="replace"):
        if line.startswith("v "):
            p = line[2:].split()
            d = float(p[0])**2 + float(p[1])**2 + float(p[2])**2
            r2 = max(r2, d); rmin = min(rmin, d)
    return math.sqrt(r2), math.sqrt(rmin)


def section(name, lon, lat, alt, authored_radius):
    return (f"\n[{name}]\nname = {name}\nparent = {PARENT}\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {lon:.6f}\norbit_lat = {lat:.6f}\norbit_alt = {alt}\n"
            f"radius = {authored_radius:.9f}\nmodel_name = {MODEL}\nhalo = false\n"
            f"[{name}:OJM]\nbody = {name}\ntype = OJM\n")


# psi offset of SmallB from BigA: dalpha/dpsi ~ 0.073 deg/deg at psi=60, and we
# want the two centres ~0.02 NDC = 0.3 deg apart (both inside one 0.0293 NDC
# pick tolerance around their midpoint).
DPSI_SMALL = 4.1
BODIES = [
    # (name, radius km, dlat, dlon)
    ("BigA",   100.0, PSI,               0.0),
    ("SmallB",  25.0, PSI + DPSI_SMALL,  0.0),
    ("FarC",    60.0, -PSI,             12.0),
    # ON the parent's DISC (psi 20 deg, alt 0) - once the recorded BOUNDARY of
    # B24-select (§11.106(e): the old picker owned every click inside a drawn
    # body's disc through its `default_object` tier, protosystem.cpp:344-355 +
    # core.cpp:1142, so a composed body drawn on its parent was NOT clickable),
    # now the SUBJECT of D26 (§11.113(e), landed §11.118): the child takes the
    # click on its own pixels and the parent keeps the rest of its disc. P6
    # asserts the first half, P6b the second.
    ("OnDisc", 150.0, 20.0,              0.0),
]
SWAP = {"BigA": 25.0, "SmallB": 100.0, "FarC": 60.0, "OnDisc": 150.0}  # R5 size mutation


def scene_sections(radii):
    mr, mrmin = model_radius(MODEL)
    if abs(mrmin / mr - 1.0) > 1e-4:
        fail(f"{MODEL} is not an exact sphere (min/max |v| = {mrmin/mr}) - scene invalid")
    return "".join(section(n, nadir_lon() + dlon, dlat, ALT_KM, radii[n] / mr)
                   for n, _r, dlat, dlon in BODIES), mr


# ---- app driving ---------------------------------------------------------
def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def shot(sock, out, name, pause=2.0):
    p = out / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(25):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def dump(sock, out, name, pause=1.6, tries=3):
    """One dual_dump, waited for.  RETRIED rather than crashed: the app's own
    stall watchdog (Fps::sigstacktrace) fires under a fast command stream and
    a dump can take seconds - a missing file is a slow app, not a result."""
    p = out / f"{name}.json"
    for attempt in range(tries):
        if p.exists():
            p.unlink()
        send(sock, f"body action dual_dump filename {p}", pause)
        for _ in range(40):
            if p.exists() and p.stat().st_size > 0:
                return load_dump(p)
            time.sleep(0.3)
        print(f"  (dump '{name}' not written after {pause + 12:.0f}s, retry {attempt + 1})", flush=True)
    raise RuntimeError(f"dump '{name}' never written - the app is not responding")


def load_dump(path):
    bodies = {}; cam = None
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            cam = o.get("camera")
        if o.get("type") == "body":
            bodies[o["name"]] = o
    return bodies, cam


XCLICK = None
CLIENT = None      # (W, H) of the SDL client window - the app's own Swapchain line
MOUSENORM = None   # (scaleX, offsetX, scaleY, offsetY) - VulkanMgr::mouseNorm


def click(win_x, win_y):
    r = subprocess.run([str(XCLICK), WIN_NAME, str(int(round(win_x))), str(int(round(win_y))),
                        f"{CLIENT[0]}x{CLIENT[1]}"],
                       capture_output=True, text=True,
                       env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    if r.returncode != 0:
        raise RuntimeError(f"xclick failed: {r.stderr.strip()}")
    return r.stdout.strip()


def win_geom():
    r = subprocess.run([str(XCLICK), WIN_NAME, "--geom", f"{CLIENT[0]}x{CLIENT[1]}"],
                       capture_output=True, text=True,
                       env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    if r.returncode != 0:
        raise RuntimeError(f"xclick --geom failed: {r.stderr.strip()}")
    return [int(v) for v in r.stdout.split()]


def read_geometry(applog):
    """The app prints its own render geometry at init:
         Scaling : <f> / Viewport : (w, -h) / Swapchain : (w, h) / Rect : (w, h) offset=(x, y)
    Rebuild VulkanMgr::mouseNorm from them EXACTLY as VulkanMgr.cpp:160-163
    does - the mapping window-pixel -> ScreenRect is the app's own authority
    (screenToRect), so the harness must not invent a second one (I2)."""
    txt = Path(applog).read_text(errors="replace")
    scaling = float(re.search(r"^Scaling : ([0-9.]+)", txt, re.M).group(1))
    sw, sh = (int(v) for v in re.search(r"^Swapchain : \((\d+), (\d+)\)", txt, re.M).groups())
    rw, rh = (int(v) for v in re.search(r"^Rect : \((\d+), (\d+)\)", txt, re.M).groups())
    scaledW, scaledH = scaling * rw, scaling * rh
    scaleX, scaleY = 0.5 * (scaledW - 1), 0.5 * (scaledH - 1)
    offsetX = (sw - scaledW) / 2 + scaleX
    offsetY = (sh - scaledH) + scaleY
    return (sw, sh), (scaleX, offsetX, scaleY, offsetY), (rw, rh)


def ndc_to_window(screen):
    """ModularBody screenPos -> client-window pixel.
    screenPos has +1 at the TOP [derived: b3_ladder.py's validated
    py = (1-sy)/2*RENDER]; screenToRect maps window y (0 at the top) linearly
    onto [-1,+1] => rect_y = -screenPos_y."""
    sx, ox, sy, oy = MOUSENORM
    return screen[0] * sx + ox, (-screen[1]) * sy + oy


def farm_setup():
    subprocess.run(["bash", str(HERE / "b3_farm.sh"), str(FARM)], check=True)


def launch(out, tag, sections):
    farmdir = FARM / ".spacecrafter"
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(TWIN + sections.encode("latin-1"))
    for f in (farmdir / "log").glob("*.log"):
        f.unlink()
    env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([SC_BIN], cwd=str(farmdir),
                            stdout=open(out / f"{tag}.applog", "w"), stderr=subprocess.STDOUT, env=env)
    s = wait_port(); time.sleep(10)
    send(s, "flag experimental_path on"); send(s, "timerate rate 0")
    send(s, "meteors zhr 0"); send(s, f"date jday {JD}")
    send(s, f"set home_planet {PARENT}", 2)
    send(s, "camera action free_mode state on")
    send(s, "flag atmosphere off"); send(s, "flag landscape off")
    send(s, "flag stars off"); send(s, "flag nebulae off")
    send(s, "flag moon_scaled off", 2)          # 5.27 precondition
    send(s, f"select planet {PARENT}")
    send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 5)
    send(s, "flag track_object on", 2)          # centre the parent -> deterministic aim
    send(s, f"zoom fov {FOV} duration 0", 2)
    send(s, "flag track_object off", 2)         # B30 determinism (11.94(e))
    send(s, "deselect", 1)
    return proc, s


def shutdown(proc, s):
    try:
        send(s, "shutdown action now", 1); s.close()
        proc.wait(timeout=40)
    except Exception:
        pass
    finally:
        if proc.poll() is None:
            proc.kill()


def sel_of(cam):
    return (cam or {}).get("selected", ""), (cam or {}).get("tracked", "")


def select_and_read(s, out, tag, name, track=True):
    """Command channel: select by name, optionally arm tracking (the
    Core::selected_object witness), dump, release tracking."""
    send(s, "deselect", 0.6)
    send(s, f"select planet {name}", 0.8)
    if track:
        send(s, "flag track_object on", 1.5)
    _b, cam = dump(s, out, f"{tag}")
    if track:
        send(s, "flag track_object off", 1.2)
    return cam


def click_and_read(s, out, tag, wx, wy):
    send(s, "deselect", 0.6)
    info = click(wx, wy)
    time.sleep(1.2)
    _b, cam = dump(s, out, f"{tag}")
    return cam, info


# ---- main ---------------------------------------------------------------
TWIN = b""


def navstr_blocks(path, names):
    """Split a dual_dump `.navstr` sidecar into per-body blocks, given the body
    NAMES from the matching .json dump.

    The name set is required, not a convenience: getShortInfoNavString and
    getInfoString both contain EMBEDDED newlines whose continuation lines start
    at column 0 ("SA ...", "Magnitude: ...", "Distance: ... AU"), so neither an
    indentation rule nor a "line before a label" rule delimits blocks - both
    truncate at the first embedded newline. Only the writer's own key does
    (ssystem_factory.cpp, dumpTracePaths: one bare english-name line per body)."""
    out, name, cur = {}, None, []
    for line in open(path, errors="replace"):
        if line.rstrip("\n") in names:
            if name is not None:
                out[name] = "".join(cur)
            name, cur = line.rstrip("\n"), []
        elif name is not None:
            cur.append(line)
    if name is not None:
        out[name] = "".join(cur)
    return out


def dump_names(json_path):
    names = set()
    for line in open(json_path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "body":
            names.add(o["name"])
    return names


def compare_parity(pre_json, post_json):
    """OLD-BODY PARITY A/B (check 3), pre-fix vs post-fix, from two full runs.

    STRICT (asserted): the name set, the selection OUTCOME of every
    old-resolvable body name, and its selDist.  Those are the parity claim -
    the new route must not touch a name the old resolver answers.

    MEASURED, not asserted: the dual_dump nav-string readouts and the rendered
    scene.  A byte comparison of float readouts across two BUILDS of this
    project is not a valid assert: `-Ofast -march=native` (CMakeLists.txt:99)
    makes code generation float-sensitive, and this wave MEASURED the floor -
    adding a provably-uncalled inline method to core.hpp moves 58/95 body
    distances by up to 2.4e-7 relative (2 float32 ULP) with zero semantic
    change (INTENT 11.106(g)).  So these are reported with their split
    (old-path half vs new-path half) and their magnitude, and the entry
    records which comparison pair each verdict came from.
    """
    a = json.load(open(pre_json)).get("parity", {})
    b = json.load(open(post_json)).get("parity", {})
    print(f"old-body parity: {len(a)} names pre, {len(b)} names post")
    if set(a) != set(b):
        fail(f"parity: name sets differ - only pre {sorted(set(a)-set(b))}, "
             f"only post {sorted(set(b)-set(a))}")
        return 1
    dsel = [n for n in a if a[n]["selected"] != b[n]["selected"]]
    dmax, dwho = 0.0, None
    for n in a:
        da, db = a[n].get("selDist"), b[n].get("selDist")
        if da is None or db is None:
            continue
        if abs(da - db) > dmax:
            dmax, dwho = abs(da - db), n
    if dsel:
        fail(f"parity: {len(dsel)} names select a DIFFERENT body post-fix: "
             + ", ".join(f"{n} {a[n]['selected']!r}->{b[n]['selected']!r}" for n in dsel[:10]))
    else:
        ok(f"parity: all {len(a)} old-resolvable body names select the identical body "
           f"pre and post")
    if dmax == 0.0:
        ok("parity: selDist bit-identical for every name (max |delta| = 0)")
    else:
        fail(f"parity: selDist differs, max |delta| = {dmax:.3e} AU on {dwho}")

    pa, pb = Path(pre_json).parent, Path(post_json).parent
    # ---- readouts, split by which path produced them ----
    na, nb = pa / "main_base.json.navstr", pb / "main_base.json.navstr"
    if na.exists() and nb.exists():
        ba = navstr_blocks(na, dump_names(pa / "main_base.json"))
        bb = navstr_blocks(nb, dump_names(pb / "main_base.json"))
        def split(block):
            old, new, cur = [], [], None
            for line in block.splitlines(keepends=True):
                if line.startswith("  OLD "):
                    cur = old
                elif line.startswith("  NEW "):
                    cur = new
                if cur is not None:
                    cur.append(line)
            return "".join(old), "".join(new)
        sh = sorted(set(ba) & set(bb))
        dold = [n for n in sh if split(ba[n])[0] != split(bb[n])[0]]
        dnew = [n for n in sh if split(ba[n])[1] != split(bb[n])[1]]
        print(f"readouts (measured, see the docstring): {len(sh)} shared bodies; "
              f"OLD-path half differs on {len(dold)} {dold[:6]}; "
              f"NEW-path half differs on {len(dnew)} {dnew[:6]}; "
              f"post adds {len(set(bb) - set(ba))} new-only blocks {sorted(set(bb) - set(ba))}")
    # ---- rendered scene ----
    ia, ib = pa / "main_scene.png", pb / "main_scene.png"
    if ia.exists() and ib.exists():
        A = np.asarray(Image.open(ia).convert("RGB"), dtype=np.int16)
        B = np.asarray(Image.open(ib).convert("RGB"), dtype=np.int16)
        d = np.abs(A.max(axis=2) - B.max(axis=2))
        print(f"rendered scene (measured): px>0 {int((d > 0).sum())} px>8 {int((d > 8).sum())} "
              f"px>32 {int((d > 32).sum())} max {int(d.max())} on "
              f"{int((A.max(axis=2) > 16).sum())} lit px")
    return 1 if FAILS else 0


def main():
    global TWIN, XCLICK
    argv = sys.argv[1:]
    if "--compare" in argv:
        i = argv.index("--compare")
        return compare_parity(argv[i + 1], argv[i + 2])
    skip_parity = "--skip-parity" in argv
    argv = [a for a in argv if not a.startswith("--")]
    out = Path(argv[0]).resolve() if argv else HERE / "artifacts/b24_select"
    out.mkdir(parents=True, exist_ok=True)

    XCLICK = out / "xclick"
    r = subprocess.run(["gcc", "-O1", "-o", str(XCLICK), str(HERE / "xclick.c"), "-lX11",
                        "/usr/lib/x86_64-linux-gnu/libXtst.so.6"], capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"xclick build failed: {r.stderr}")

    twin_path = REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled"
    if not twin_path.exists():
        raise RuntimeError("composed twin absent - launch once on the shipped state first")
    TWIN = twin_path.read_bytes()
    farm_setup()

    # ---- predictions, committed before the run ----
    pred = {
        "limb_deg": math.degrees(math.asin(R_KM / (R_KM + OBS_ALT_M / 1000.0))),
        "alpha_big_deg": alpha_deg(PSI, ALT_KM),
        "alpha_small_deg": alpha_deg(PSI + DPSI_SMALL, ALT_KM),
        "alpha_far_deg": alpha_deg(math.degrees(math.acos(
            math.cos(math.radians(PSI)) * math.cos(math.radians(12.0)))), ALT_KM),
        "dist_big_km": obs_dist_km(PSI, ALT_KM),
        "pick_tol_ndc": PICK_TOL_PX / (RENDER / 2),
        "screenSize_big": math.atan(100.0 / obs_dist_km(PSI, ALT_KM)) / math.radians(FOV / 2),
        "screenSize_small": math.atan(25.0 / obs_dist_km(PSI + DPSI_SMALL, ALT_KM)) / math.radians(FOV / 2),
    }
    pred["ndc_big"] = pred["alpha_big_deg"] / (FOV / 2)
    pred["ndc_small"] = pred["alpha_small_deg"] / (FOV / 2)
    pred["ndc_moon_limb"] = pred["limb_deg"] / (FOV / 2)
    pred["cluster_sep_ndc"] = pred["ndc_small"] - pred["ndc_big"]
    (out / "b24_select_predict.json").write_text(json.dumps(pred, indent=1))
    print(json.dumps(pred, indent=1), flush=True)

    report = {"bin": SC_BIN, "pred": pred, "legs": {}}

    # =====================================================================
    # LAUNCH 1 - main scene
    # =====================================================================
    sections, mr = scene_sections({n: r for n, r, _a, _b in BODIES})
    print(f"--- launch 1 (main scene), model radius {mr:.6f} ---", flush=True)
    proc, s = launch(out, "main", sections)
    try:
        global CLIENT, MOUSENORM
        CLIENT, MOUSENORM, RECT = read_geometry(out / "main.applog")
        wx0, wy0, W, H = win_geom()
        report["window"] = [wx0, wy0, W, H]
        report["client"] = CLIENT; report["rect"] = RECT; report["mouseNorm"] = MOUSENORM
        print(f"client window {CLIENT[0]}x{CLIENT[1]} at root +{wx0}+{wy0} ({W}x{H}); "
              f"render rect {RECT}; mouseNorm {MOUSENORM}", flush=True)
        if (W, H) != CLIENT:
            fail(f"client window {W}x{H} != the app's own Swapchain {CLIENT} - "
                 f"the click coordinate frame is not the one screenToRect inverts")

        bodies, cam = dump(s, out, "main_base")
        report["cam_base"] = cam
        moon = bodies.get(PARENT, {}).get("new", {}) or {}
        scr = {n: (bodies.get(n, {}).get("new") or {}) for n, _r, _a, _b in BODIES}

        # ---- L0 scene sanity: composed present, off the disc, geometry as predicted
        missing = [n for n in scr if not scr[n]]
        if missing:
            fail(f"L0 scene: composed bodies missing from the dump: {missing}")
        else:
            desc = ", ".join("%s ss=%.4f" % (n, scr[n]["screenSize"]) for n in scr)
            ok(f"L0 scene: 3 composed bodies present ({desc})")
        for n in scr:
            if not scr[n]:
                continue
            sp = scr[n]["screen"]
            rad = math.hypot(sp[0], sp[1])
            a_meas = rad * (FOV / 2)
            report["legs"].setdefault("geometry", {})[n] = dict(
                screen=sp, ndc_radius=rad, alpha_deg=a_meas,
                screenSize=scr[n]["screenSize"], dist_km=scr[n]["dist"] * AU_KM)
            if n == "OnDisc":     # deliberately inside the disc - see BODIES
                continue
            if rad <= pred["ndc_moon_limb"] + scr[n]["screenSize"]:
                fail(f"L0 {n}: ndc radius {rad:.4f} is not clear of the Moon's disc "
                     f"({pred['ndc_moon_limb']:.4f}) - the old picker would see the parent there")
        if scr["BigA"]:
            da = abs(math.hypot(*scr["BigA"]["screen"]) * (FOV / 2) - pred["alpha_big_deg"])
            if da < 0.25:
                ok(f"L0 geometry: BigA at alpha {math.hypot(*scr['BigA']['screen'])*(FOV/2):.3f} deg "
                   f"vs predicted {pred['alpha_big_deg']:.3f} deg (limb {pred['limb_deg']:.3f})")
            else:
                fail(f"L0 geometry: BigA alpha off prediction by {da:.3f} deg")

        # ---- cluster geometry (R5 precondition)
        sepx = scr["BigA"]["screen"][0] - scr["SmallB"]["screen"][0]
        sepy = scr["BigA"]["screen"][1] - scr["SmallB"]["screen"][1]
        sep = math.hypot(sepx, sepy)
        mid = ((scr["BigA"]["screen"][0] + scr["SmallB"]["screen"][0]) / 2,
               (scr["BigA"]["screen"][1] + scr["SmallB"]["screen"][1]) / 2)
        tol = pred["pick_tol_ndc"]
        report["legs"]["cluster"] = dict(sep_ndc=sep, tol_ndc=tol, mid=mid)
        if sep / 2 < tol:
            ok(f"L0 cluster: BigA/SmallB centres {sep:.4f} NDC apart, both within the "
               f"{tol:.4f} NDC pick tolerance of their midpoint (R5 precondition)")
        else:
            fail(f"L0 cluster: centres {sep:.4f} NDC apart - half-separation {sep/2:.4f} "
                 f"exceeds the pick tolerance {tol:.4f}; the R5 leg would not be a cluster")

        # =============== COMMAND CHANNEL ===============
        cam_c = select_and_read(s, out, "cmd_composed", "BigA")
        sel, trk = sel_of(cam_c)
        report["legs"]["C1_command_composed"] = dict(selected=sel, tracked=trk,
                                                     selDist=(cam_c or {}).get("selDist"))
        if sel == "BigA" and trk == "BigA":
            ok(f"C1 command/composed: `select planet BigA` -> selected='{sel}' tracked='{trk}' "
               f"selDist={cam_c['selDist']:.6e} (11.97(d) inverted)")
        else:
            fail(f"C1 command/composed: `select planet BigA` -> selected='{sel}' tracked='{trk}' "
                 f"(expected both 'BigA') - the 11.97(d) gap")

        cam_o = select_and_read(s, out, "cmd_old", PARENT)
        sel, trk = sel_of(cam_o)
        report["legs"]["C2_command_old"] = dict(selected=sel, tracked=trk)
        if sel == PARENT and trk == PARENT:
            ok(f"C2 command/old: `select planet {PARENT}` -> selected='{sel}' tracked='{trk}'")
        else:
            fail(f"C2 command/old: `select planet {PARENT}` -> selected='{sel}' tracked='{trk}'")

        cam_x = select_and_read(s, out, "cmd_absent", "NoSuchBody")
        sel, trk = sel_of(cam_x)
        report["legs"]["C3_command_absent"] = dict(selected=sel, tracked=trk)
        if sel == "" and trk == "":
            ok("C3 command/absent: an unknown name selects nothing (no false positive)")
        else:
            fail(f"C3 command/absent: 'NoSuchBody' selected='{sel}' tracked='{trk}'")

        # restore the aim: tracking BigA/Moon moved the camera
        send(s, "deselect", 0.5)
        send(s, f"select planet {PARENT}", 0.6)
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 4)
        send(s, "flag track_object on", 2)
        send(s, f"zoom fov {FOV} duration 0", 2)
        send(s, "flag track_object off", 2)
        send(s, "deselect", 1)
        bodies, cam = dump(s, out, "main_reaimed")
        moon = bodies.get(PARENT, {}).get("new", {}) or {}
        scr = {n: (bodies.get(n, {}).get("new") or {}) for n, _r, _a, _b in BODIES}
        report["cam_reaimed"] = cam
        img_scene = shot(s, out, "main_scene")

        # =============== POINTER CHANNEL ===============
        # P0 - old-body control: the Moon's own dumped centre must still pick the Moon
        mx, my = ndc_to_window(moon["screen"])
        cam_p0, info0 = click_and_read(s, out, "clk_moon", mx, my)
        sel, _ = sel_of(cam_p0)
        report["legs"]["P0_pointer_old"] = dict(win=[mx, my], selected=sel, xclick=info0)
        if sel == PARENT:
            ok(f"P0 pointer/old: click at the Moon's screen centre ({mx:.0f},{my:.0f}) "
               f"selects '{sel}' - the old picking path is live and untouched")
        else:
            fail(f"P0 pointer/old: click at the Moon centre selected '{sel}' (expected {PARENT}) "
                 f"- either the click plumbing or old picking is broken")

        # P0b - CLICK-SCALE calibration, discriminating: just inside the Moon's
        # limb must be the Moon, just outside it must be nothing.  The centre
        # click above cannot catch a coordinate-frame error (a wrong window
        # frame shifts by ~44 px and still lands on a 350 px disc - measured on
        # this host with the WM frame instead of the client window); this pair
        # is sensitive to ~20 px.
        mss = moon["screenSize"]
        for tag, k, want in (("P0b_in", 0.85, PARENT), ("P0b_out", 1.06, "")):
            bx, by = ndc_to_window((0.0, k * mss))
            cam_b, infob = click_and_read(s, out, f"clk_limb_{tag}", bx, by)
            selb, _ = sel_of(cam_b)
            report["legs"][tag] = dict(ndc=[0.0, k * mss], win=[bx, by], selected=selb, want=want)
            if selb == want:
                ok(f"{tag}: click at NDC (0,{k*mss:+.3f}) = {k:.2f}x the Moon's disc radius "
                   f"-> selected '{selb}' (expected '{want}') - click scale calibrated to ~20 px")
            else:
                fail(f"{tag}: click at {k:.2f}x the Moon's disc radius selected '{selb}' "
                     f"(expected '{want}') - click coordinate frame or scale is wrong")

        # P1/P2 - each composed body's own position selects THAT body
        for tag, name in (("P1", "FarC"), ("P2", "BigA")):
            cx, cy = ndc_to_window(scr[name]["screen"])
            cam_p, infop = click_and_read(s, out, f"clk_{name}", cx, cy)
            sel, _ = sel_of(cam_p)
            report["legs"][f"{tag}_pointer_{name}"] = dict(win=[cx, cy], selected=sel, xclick=infop)
            if sel == name:
                ok(f"{tag} pointer/composed: click at {name}'s screen position "
                   f"({cx:.0f},{cy:.0f}) selects '{sel}'")
            else:
                fail(f"{tag} pointer/composed: click at {name}'s position selected '{sel}' "
                     f"(expected '{name}')")

        # P3 - R5: click the cluster midpoint, the BIGGER must win
        gx, gy = ndc_to_window(mid)
        cam_p3, info3 = click_and_read(s, out, "clk_cluster", gx, gy)
        sel, _ = sel_of(cam_p3)
        report["legs"]["P3_R5_biggest"] = dict(win=[gx, gy], selected=sel,
                                               big="BigA", ss_big=scr["BigA"]["screenSize"],
                                               ss_small=scr["SmallB"]["screenSize"])
        if sel == "BigA":
            ok(f"P3 R5: both centres inside one pick tolerance -> selected '{sel}' "
               f"(screenSize {scr['BigA']['screenSize']:.4f} vs {scr['SmallB']['screenSize']:.4f}) "
               f"- BIGGEST wins")
        else:
            fail(f"P3 R5: cluster click selected '{sel}' (expected the bigger, 'BigA')")

        # P6 - the SEAM, RE-POINTED 2026-07-30 by D26 (§11.113(e), F12/§11.118):
        # this leg asserted 'Moon' while the decision was open, as the lock on a
        # measured residual. Vixy answered "the visible composed child TAKES the
        # click when it lands ON the child", so the expected value INVERTS to the
        # child's own name - re-pointed, never loosened, in the same commit as
        # the behaviour. It stays the discriminator: a later silent flip in
        # either direction fails here.
        ox, oy = ndc_to_window(scr["OnDisc"]["screen"])
        cam_p6, info6 = click_and_read(s, out, "clk_ondisc", ox, oy)
        sel, _ = sel_of(cam_p6)
        report["legs"]["P6_pointer_on_disc"] = dict(
            win=[ox, oy], selected=sel, ndc=scr["OnDisc"]["screen"],
            screenSize=scr["OnDisc"]["screenSize"], expect="OnDisc")
        if sel == "OnDisc":
            ok(f"P6 seam: a click ON the composed body drawn INSIDE the Moon's disc "
               f"(ndc {scr['OnDisc']['screen']}, ss {scr['OnDisc']['screenSize']:.4f}) selects "
               f"'{sel}' - the child takes its own pixels across the old/new seam (D26)")
        else:
            fail(f"P6 seam: on-disc composed click selected '{sel}' (expected 'OnDisc') - "
                 f"D26's rule is not in force")

        # P6b - THE BOUND on D26, and the leg that makes "confined to the
        # child's own pixels" a measurement rather than a claim: the SAME
        # parent disc, one child-radius away from the child, must still select
        # the PARENT. Without it, "the child wins" and "the child wins
        # everywhere on the parent" pass the same test.
        odn = scr["OnDisc"]["screen"]
        away = (odn[0] + 3.0 * scr["OnDisc"]["screenSize"], odn[1])
        ax, ay = ndc_to_window(away)
        cam_p6b, info6b = click_and_read(s, out, "clk_ondisc_off", ax, ay)
        selb, _ = sel_of(cam_p6b)
        report["legs"]["P6b_pointer_parent_disc"] = dict(
            win=[ax, ay], selected=selb, ndc=list(away), expect=PARENT)
        if selb == PARENT:
            ok(f"P6b bound: the same parent disc, {3.0 * scr['OnDisc']['screenSize']:.4f} NDC "
               f"off the child, still selects '{selb}' - the pre-emption is confined to the "
               f"child's own pixels")
        else:
            fail(f"P6b bound: a click on the parent's disc away from the child selected "
                 f"'{selb}' (expected '{PARENT}') - the new route is taking more than D26 allows")

        # P6c - the SEAM crossed a SECOND time, entering from the state the
        # first crossing left behind (a parent selection made by P6b, not the
        # clean state P6 started from). Count arithmetic is not a substitute:
        # the pre-emption runs before the old picker and both sides mutate the
        # selection, so the pair has to be traversed twice.
        cam_p6c, info6c = click_and_read(s, out, "clk_ondisc_again", ox, oy)
        selc, _ = sel_of(cam_p6c)
        report["legs"]["P6c_pointer_on_disc_again"] = dict(
            win=[ox, oy], selected=selc, expect="OnDisc", entered_from=selb)
        if selc == "OnDisc":
            ok(f"P6c seam re-entry: the same on-child click, now made with '{selb}' "
               f"selected, again selects '{selc}'")
        else:
            fail(f"P6c seam re-entry: the same click selected '{selc}' (expected 'OnDisc') "
                 f"after entering from '{selb}' - the seam is state-dependent")

        # P5 - control: empty sky selects nothing
        ex, ey = ndc_to_window((-0.85, -0.85))
        cam_p5, info5 = click_and_read(s, out, "clk_empty", ex, ey)
        sel, _ = sel_of(cam_p5)
        report["legs"]["P5_pointer_empty"] = dict(win=[ex, ey], selected=sel)
        if sel == "":
            ok("P5 pointer/empty: a click on empty sky selects nothing (no false positive)")
        else:
            fail(f"P5 pointer/empty: empty-sky click selected '{sel}'")

        # =============== INFO SURFACE (check 5) ===============
        send(s, "deselect", 0.5)
        send(s, "select planet BigA", 0.8)
        b_i, cam_i = dump(s, out, "info")
        blk = navstr_blocks(out / "info.json.navstr",
                            dump_names(out / "info.json")).get("BigA")
        report["legs"]["I1_info"] = dict(block=blk)
        if blk:
            dm = re.search(r"Distance: ([0-9.eE+-]+) AU", blk)
            dumped_au = (b_i.get("BigA", {}).get("new") or {}).get("dist")
            fields = [k for k in ("RA/DE:", "Alt/Az:", "Distance:", "Magnitude:") if k in blk]
            mag = re.search(r"Magnitude: (\S+)", blk)
            report["legs"]["I1_info"]["fields"] = fields
            report["legs"]["I1_info"]["magnitude"] = mag.group(1) if mag else None
            if len(fields) < 4:
                fail(f"I1 info: the composed body's info block is missing fields: {fields}")
            print(f"note: composed-body info magnitude reads "
                  f"'{mag.group(1) if mag else '?'}' (recorded, see 11.106)", flush=True)
            if dm and dumped_au and abs(float(dm.group(1)) - dumped_au) <= 1e-6 * max(1.0, dumped_au):
                ok(f"I1 info: the composed body's info readout is populated ({', '.join(fields)}) "
                   f"and its Distance {float(dm.group(1)):.8f} AU matches the dumped position "
                   f"{dumped_au:.8f} AU")
            else:
                fail(f"I1 info: distance mismatch (info={dm.group(1) if dm else None} "
                     f"dump={dumped_au})")
        else:
            fail("I1 info: no NEW info block emitted for the composed body")

        # =============== OLD-BODY PARITY SWEEP ===============
        if not skip_parity:
            old_names = sorted(n for n, o in bodies.items() if o.get("old") is not None)
            parity = {}
            for i, n in enumerate(old_names):
                if proc.poll() is not None:
                    fail(f"parity sweep: the app exited at body {i}/{len(old_names)} ({n})")
                    break
                send(s, "deselect", 0.3)
                send(s, f"select planet {n}", 0.5)
                _b, c = dump(s, out, "parity_tmp", pause=1.3)
                parity[n] = dict(selected=(c or {}).get("selected", ""),
                                 selDist=(c or {}).get("selDist"))
                if i % 20 == 0:
                    print(f"  parity sweep {i}/{len(old_names)} ...", flush=True)
            report["parity"] = parity
            ok(f"parity sweep: {len(old_names)} old-path body names selected and recorded")
    finally:
        shutdown(proc, s)

    # =====================================================================
    # LAUNCH 2 - R5 size mutation (radii swapped)
    # =====================================================================
    sections2, _ = scene_sections(SWAP)
    print("--- launch 2 (R5 size mutation: BigA 25 km / SmallB 100 km) ---", flush=True)
    proc, s = launch(out, "swap", sections2)
    try:
        wx0, wy0, W, H = win_geom()
        bodies, cam = dump(s, out, "swap_base")
        scr2 = {n: (bodies.get(n, {}).get("new") or {}) for n, _r, _a, _b in BODIES}
        mid2 = ((scr2["BigA"]["screen"][0] + scr2["SmallB"]["screen"][0]) / 2,
                (scr2["BigA"]["screen"][1] + scr2["SmallB"]["screen"][1]) / 2)
        gx, gy = ndc_to_window(mid2)
        cam_m, infom = click_and_read(s, out, "clk_cluster_swapped", gx, gy)
        sel, _ = sel_of(cam_m)
        report["legs"]["P4_R5_mutation"] = dict(
            win=[gx, gy], selected=sel,
            ss_BigA=scr2["BigA"]["screenSize"], ss_SmallB=scr2["SmallB"]["screenSize"])
        if sel == "SmallB":
            ok(f"P4 R5 mutation: radii swapped -> the SAME cluster click now selects '{sel}' "
               f"(screenSize {scr2['SmallB']['screenSize']:.4f} vs {scr2['BigA']['screenSize']:.4f}) "
               f"- selection FOLLOWS size")
        else:
            fail(f"P4 R5 mutation: swapped-size cluster click selected '{sel}' (expected 'SmallB')")
        shot(s, out, "swap_scene")
    finally:
        shutdown(proc, s)

    report["fails"] = FAILS
    (out / "b24_select_result.json").write_text(json.dumps(report, indent=1, default=str))
    print(f"\n{'B24-SELECT GREEN' if not FAILS else f'{len(FAILS)} FAILURES'} "
          f"-> {out}/b24_select_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
