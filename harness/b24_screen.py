#!/usr/bin/env python3
"""B24 SCREEN-LAYER grounded scene (INTENT 11.78 / shadow-paths H4(b) / 13.B B3).

Sibling of b24_compose.py (the numeric layer). Takes the proven composed grounded
rover to the SCREEN: a DRAWN OJM rover grounded on the Moon, authored through the
REAL adoption workflow (twin + appended composed `type=` sections, 11.78(d)/(f)).
Measures the two H4(b) observables with scene-present / scene-absent discrimination
(the 11.80 method: two fresh launches, identical camera, diff isolates the rovers;
tracking OFF before every shot per B30/11.94(e)).

FINDINGS THIS SCENE ESTABLISHES (2026-07-24, code c0181da9; interpretation CORRECTED
same day by the F1 root-cause chain, INTENT 11.99 -> 11.100 - the original (3) below
attributed a depth-precision collapse; REFUTED):
  (1) SCREEN-LAYER DRAW: the grounded OJM rover renders as a full mesh (drawInternal
      executes; OJM loads synchronously at construction). Proven by BODYpx>0 in the
      CLOSE view.
  (2) SHADOW observable WORKS via the EXISTING jobs-as-data machinery (OPAQUE_OJM
      cast, shadow-paths G1): a lit grounded rover casts a SILHOUETTE shadow onto the
      parent Moon surface. Measured as darkened px in the present-vs-absent diff.
  (3) OCCLUSION [corrected, INTENT 11.100]: there is NO demonstrated depth defect.
      The original far-suppression was the `moon_scale=5` display scaling: grounded
      children do not ride parent display scaling (defect 5.27, decision D21) - the
      rover sat ~7000 km under the DISPLAYED surface, CORRECTLY occluded by CORRECT
      depth; the close-view "draw" was the scaled radius pulling the parent into the
      empty-groundedComponents surface regime (11.97(e)) - parent absent, nothing to
      occlude against. THIS HARNESS therefore runs `flag moon_scaled off` (the 5.27
      instrument precondition) and asserts the REAL-geometry scene: the rover renders
      in BOTH views with working merged-bucket occlusion (the 11.100(b) causal lever,
      locked as a regression gate).
      THE `flag moon_scaled off` LINE IS NO LONGER A WORKAROUND (F39, 11.152 -
      5.27 CLOSED, D21 landed): a grounded child now inherits its parent's display
      scaling as a uniform dilation, and `f39_d21.py` runs THIS composition under
      the SHIPPED `moon_scale = 5` with the rover drawn ON the displayed surface.
      The line STAYS, re-read as a SCENE DECLARATION: gate A4's own subject is the
      REAL geometry, every committed baseline of this harness was measured with
      scaling off, and `f39_d21.py` is the scaled twin. Removing it would silently
      change what A4 measures - the opposite of retiring a workaround. Same reading
      for every other harness carrying the line (b3_ladder, b24_select,
      f23_b33_control, f24_b34_seams, f25_ramp, f29_upchain).
      The D1(b) grounded-slice prefill remains an
      unimplemented capability for FINE grounded content (header-forced, B3), but no
      scene here demonstrates wrong depth ordering.

Instrument-model notes (11.99(e)/11.100(f)): assert the PARENT's presence and the
config levers (moon_scale!), never only the present-vs-absent diff - the diff cancels
a missing parent; a "buried" probe body may legitimately protrude past the limb in
projection (visible correctly), so buried!=0 px is NOT by itself a depth defect.
"""
import json, math, os, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
USERDIR = HOME / ".spacecrafter"
TWIN = USERDIR / "modularSystem/SolarSystem.ini.disabled"
ENABLED = USERDIR / "modularSystem/SolarSystem.ini"
# ABSOLUTE (11.106): the app is launched with cwd = ~/.spacecrafter, so a
# RELATIVE outdir makes it write every dump/screenshot somewhere that does not
# exist - and the failure is SILENT, because the reader then finds the file
# left by a PREVIOUS run and verifies stale artifacts. Measured: a relative
# outdir here re-read 2.5-hour-old dumps and only the one leg asking for a
# file that had never existed crashed. Same rule as b3_ladder.py.
OUT = (Path(sys.argv[1]) if len(sys.argv) > 1
       else Path(__file__).resolve().parent / "artifacts/b24_screen").resolve()
AU_KM = 149597870.0
JD = 2461234.0          # jd where observer lon 60 on the Moon is the LIT hemisphere
OBS_LON = 60            # lit-side sub-observer longitude (calibrated, findlit)
# Two observer distances via moveto altitude (metres). CLOSE resolves the child in
# the parent's depth bucket; FAR collapses the precision -> body suppressed.
ALT_CLOSE_M = 4000000
ALT_FAR_M = 22000000

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)


def rover(name, lon, lat, alt, radius):
    # composed `type=` grammar (11.78(d)/(j)): node type=Artificial marks a node;
    # module section type=OJM is the family. surface_point + relation=grounded.
    return (f"\n[{name}]\nname = {name}\nparent = Moon\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {lon}\norbit_lat = {lat}\norbit_alt = {alt}\nradius = {radius}\n"
            f"model_name = Curiosity\nhalo = false\n[{name}:OJM]\nbody = {name}\ntype = OJM\n")

# ShadowCaster: big lit surface rover -> clear cast shadow on the parent.
# Behind: buried far-side rover -> MUST be occluded (occlusion ordering probe).
SECTIONS = (rover("ScreenRover", OBS_LON, 0, 0, 800)
            + rover("Behind", OBS_LON, 12, -9000, 500))


def wait_port(timeout=90):
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

def shot(sock, name, pause=2.2):
    p = OUT / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(25):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)

def load_new(path):
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
        if o.get("type") == "body" and o.get("new") is not None:
            bodies[o["name"]] = o["new"]
    return bodies, cam


def run(tag, with_scene, dumps):
    if with_scene:
        ENABLED.write_bytes(TWIN.read_bytes() + SECTIONS.encode("latin-1"))
    else:
        ENABLED.unlink(missing_ok=True)
    proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                            stdout=open(OUT / f"{tag}.applog", "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    imgs = {}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "flag experimental_path on"); send(s, "timerate rate 0")
        send(s, "meteors zhr 0"); send(s, f"date jday {JD}")
        send(s, "set home_planet Moon", 2)
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off"); send(s, "flag landscape off")
        # SCENE DECLARATION, not a workaround, since F39/11.152 (see the header):
        # this harness's subject is the REAL geometry, so it pins scaling OFF.
        send(s, "flag moon_scaled off", 2)
        send(s, "select planet Moon")
        for view, alt in (("far", ALT_FAR_M), ("close", ALT_CLOSE_M)):
            send(s, f"moveto lat 0 lon {OBS_LON} alt {alt} duration 0", 4)
            send(s, "flag track_object on", 2)
            send(s, "zoom fov 20 duration 0", 2)
            send(s, "flag track_object off", 2)               # B30 determinism
            if with_scene:
                p = OUT / f"{tag}_{view}.json"
                send(s, f"body action dual_dump filename {p}", 2)
                dumps[view] = load_new(p)
            imgs[view] = shot(s, f"{tag}_{view}")
        # SELECTION-AIM CROSS-CHECK of the geometric aim (B24-select, INTENT
        # 11.106).  Every measurement above aims at the rover GEOMETRICALLY -
        # its dumped screenPos converted to pixels - which is a claim about
        # which body the harness thinks it is looking at, taken from the same
        # dump it then measures.  Selecting the rover BY NAME and letting the
        # camera track it is an independent route to the same body: it comes
        # from the selection resolver, not from the screen arithmetic.  Run
        # LAST, after every shot, so the tracking move disturbs nothing.
        if with_scene:
            send(s, "select planet ScreenRover", 1)
            send(s, "flag track_object on", 3)
            p = OUT / f"{tag}_selaim.json"
            send(s, f"body action dual_dump filename {p}", 2)
            dumps["selaim"], dumps["selaim_cam"] = load_new(p)
            send(s, "flag track_object off", 2)
        send(s, "shutdown action now", 1); s.close()
        try:
            proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        pass
    return imgs


def bright_dark(a, b):
    la = a.max(axis=2); lb = b.max(axis=2); d = la - lb
    return (d > 16), (d < -16)


def box(mask, cx, cy, r=200):
    H = mask.shape[0]
    x0, x1 = max(0, cx - r), min(H, cx + r); y0, y1 = max(0, cy - r), min(H, cy + r)
    return int(mask[y0:y1, x0:x1].sum())


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if not TWIN.exists():
        raise RuntimeError("twin absent - launch once on the shipped state first")
    dumps = {}
    try:
        A = run("A", True, dumps)     # with rovers
        B = run("B", False, {})       # baseline
    finally:
        ENABLED.unlink(missing_ok=True)

    report = {}
    H = 2048
    for view in ("far", "close"):
        bodies, cam = dumps.get(view, ({}, None))
        moon = bodies.get("Moon", {})
        rov = bodies.get("ScreenRover", {})
        beh = bodies.get("Behind", {})
        moon_km = moon.get("dist", 0) * AU_KM
        bright, dark = bright_dark(A[view], B[view])
        tot_b = int(bright.sum()); tot_d = int(dark.sum())
        # rover body box
        rb = bd = None
        if rov:
            sx, sy = rov["screen"]; px = int((sx + 1) / 2 * H); py = int((1 - sy) / 2 * H)
            rb = box(bright, px, py); rbd = box(dark, px, py)
        behb = None
        if beh:
            sx, sy = beh["screen"]; px = int((sx + 1) / 2 * H); py = int((1 - sy) / 2 * H)
            behb = box(bright, px, py)
        report[view] = dict(moon_km=moon_km, tot_bright=tot_b, tot_dark=tot_d,
                            rover_body=rb, behind_body=behb,
                            rover_screenSize=rov.get("screenSize"), rover_dist_km=(rov.get("dist", 0) * AU_KM),
                            rover_vis=rov.get("visible"), behind_dist_km=(beh.get("dist", 0) * AU_KM))
        print(f"[{view}] Moon dist={moon_km:.0f}km  totBright={tot_b} totDark={tot_d}  "
              f"ScreenRover(dist={rov.get('dist',0)*AU_KM:.0f}km ss={rov.get('screenSize')}) BODYpx={rb}  "
              f"Behind BODYpx={behb}", flush=True)

    # ---- A1: composition present ----
    b0, _ = dumps.get("far", ({}, None))
    if all(n in b0 for n in ("ScreenRover", "Behind")) and \
       b0["ScreenRover"].get("relation") == 3 and "OJM" in b0["ScreenRover"].get("modules", []):
        ok("A1 compose: grounded ScreenRover present (parent Moon, relation grounded, OJM module)")
    else:
        fail("A1 compose: grounded rover not composed correctly")

    # ---- A2: screen-layer DRAW (close view, rover renders body px) ----
    rc = report["close"]["rover_body"]
    if rc and rc > 500:
        ok(f"A2 screen-draw: grounded OJM rover renders {rc} body px in the CLOSE view "
           f"(dist {report['close']['moon_km']:.0f}km) - drawn-rover confirmed")
    else:
        fail(f"A2 screen-draw: rover body px={rc} in close view (expected >500)")

    # ---- A3: SHADOW observable (cast on the parent, existing machinery) ----
    # the shadow is darkening on the lit surface, present in the diff (either view
    # where the caster is lit); take the max over views.
    dmax = max(report["far"]["tot_dark"], report["close"]["tot_dark"])
    if dmax > 2000:
        ok(f"A3 shadow: grounded rover casts a shadow on the parent Moon - {dmax} darkened "
           f"px (present/absent diff) via the existing OPAQUE_OJM machinery")
    else:
        fail(f"A3 shadow: cast-shadow darkened px={dmax} (expected >2000)")

    # ---- A4: REAL occlusion scene (corrected model, INTENT 11.100) ----
    # With moon_scaled OFF the far view is a real-geometry occlusion scene: the
    # surface rover must RENDER (the 11.100(b) causal lever as a regression gate -
    # a regression back to 0 px here = the scaling-swallow class returning, or a
    # genuine new depth defect; both must fail loudly). Behind (9000 km buried) is
    # REPORTED, softly bounded: mostly occluded, but limb protrusion in projection
    # is legitimate (see header note), so no hard zero-assert.
    rf = report["far"]["rover_body"]
    if rf is not None and rf > 500:
        ok(f"A4 occlusion (real geometry): far-view surface rover RENDERS {rf} body px "
           f"over the unscaled Moon disc - merged-bucket parent-vs-grounded depth "
           f"compare works at this scale (11.100(b) gate)")
    else:
        fail(f"A4 occlusion: far-view rover body px={rf} (expected >500 with "
             f"moon_scaled off - scaling-swallow regression or new depth defect)")
    for view in ("far", "close"):
        bb, rb2 = report[view]["behind_body"], report[view]["rover_body"]
        if bb is not None and rb2 and bb > rb2:
            fail(f"A4b buried-vs-surface inversion in {view} view: Behind px={bb} > rover px={rb2}")
    print(f"note: Behind(9000km-buried) px far={report['far']['behind_body']} "
          f"close={report['close']['behind_body']} (soft-bounded; limb protrusion legitimate)", flush=True)

    # ---- A5: selection-aim cross-check of the geometric aim (11.106) ----
    selaim = dumps.get("selaim", {})
    selcam = dumps.get("selaim_cam") or {}
    rov = selaim.get("ScreenRover", {})
    sel, trk = selcam.get("selected", ""), selcam.get("tracked", "")
    off = math.hypot(*rov["screen"]) if rov else None
    seldist_km = selcam.get("selDist", 0) * AU_KM
    rov_km = rov.get("dist", 0) * AU_KM if rov else 0
    report["selaim"] = dict(selected=sel, tracked=trk, screen=rov.get("screen"),
                            offset_ndc=off, selDist_km=seldist_km, rover_dist_km=rov_km)
    if sel == "ScreenRover" and trk == "ScreenRover":
        ok(f"A5 selection-aim: `select planet ScreenRover` -> selected='{sel}' tracked='{trk}' "
           f"(the composed body is reachable from the command channel, 11.106)")
        # the tracked body must be centred, and the selection's own distance must
        # be the rover's dumped distance - the two routes agree on ONE body
        if off is not None and off < 0.02 and abs(seldist_km - rov_km) < 1.0:
            ok(f"A5 cross-check: the tracked rover sits at |screenPos| = {off:.5f} NDC of centre "
               f"and selDist {seldist_km:.1f} km == the dumped rover distance {rov_km:.1f} km "
               f"- the geometric aim and the selection resolve the SAME body")
        else:
            fail(f"A5 cross-check: offset {off} NDC, selDist {seldist_km:.1f} km vs dumped "
                 f"{rov_km:.1f} km - geometric aim and selection disagree")
    else:
        fail(f"A5 selection-aim: `select planet ScreenRover` -> selected='{sel}' tracked='{trk}' "
             f"(expected both 'ScreenRover' - 11.97(d)/11.106)")

    (OUT / "b24_screen_result.json").write_text(json.dumps({"fails": FAILS, "report": report}, indent=1))
    print(f"\n{'SCREEN SCENE GREEN' if not FAILS else f'{len(FAILS)} FAILURES'} -> b24_screen_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
