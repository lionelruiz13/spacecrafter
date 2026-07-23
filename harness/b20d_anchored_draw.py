#!/usr/bin/env python3
"""B20 draw-half: the ANCHORED galactic solar-system-from-afar view DRAWS.

INTENT 11.59 proved the ANCHOR is KEPT at galactic distance (mat layer, no
pixels).  INTENT 11.80/B5 landed drawExperimental mode-independently but
exercised it only in FREE MODE (reference escalated to MilkyWay -> drawNested).
This driver closes the missing quadrant: at galactic altitude WHILE ANCHORED
(freeMode off, reference kept = Earth), does the new draw surface DRAW the
solar system through whichever executor mode (InGalaxy / InUniverse) the OLD
observer altitude selects?

ARCHITECTURE (verified in source + measured, INTENT 11.83)
---------------------------------------------------------
Anchored to Earth the reference STAYS Earth (Camera.cpp:411 freeMode guard).
camera->system = ModularSystem::systemOf(Earth) = SolarSystem, and Camera::draw
draws camera->system->drawSystem (Camera.cpp:434) - the SolarSystem interior
DIRECTLY.  It is NEVER drawNested: the collapsed-dot / resolved-nested regime
(drawStarProxy) fires only when SolarSystem is a NESTED child of a HIGHER
current system (reference=MilkyWay/Universe = free flight only).  So the "dot
per px regime" is UNREACHABLE anchored - the anchor keeps SolarSystem current.
What draws the solar-system-from-afar while anchored is the Sun's far
representation StarModule::drawBigHalo (rmag floored to 32 px), reached through
ModularBody::draw when the Sun is IN THE VIEW CONE.

AIM IS LOAD-BEARING (measured, gdb-free instrumentation INTENT 11.83): flying
straight up (moveto altitude) from the anchored surface points the view
radially OUTWARD - the whole solar system lands directly BEHIND the observer
(Sun mat.r[14] = +dist), and ModularBody::preUpdate culls it as behind-the-
observer (isVisible=0) => NOTHING solar draws (measured new==old BIT-IDENTICAL,
maxdiff 0).  The view must FACE the solar system.  `flag track_object` needs the
OLD selected_object, which the executor CLEARS on every mode transition
(executor.cpp:75/102) - so tracking CANNOT be (re)established in galactic mode.
It CAN be established WHILE NEAR: Camera::target is a persisting ModularBodyPtr
and galaxy/universe onEnter never call trackBody(nullptr), so the aim rides the
fly-up (measured: tracked=Sun and Sun r14<0 at the galactic placement).  This is
a NAVIGATION property, not a draw gate; whether tracking SHOULD be re-
establishable in galactic mode is mode-transition/escalation policy = R13/B31/
6.9 territory, EXCLUDED here.

DISCRIMINATION (mandatory) = AIMED vs NOT-AIMED, same binary, in one run:
  * NOT-AIMED (moveout with NO tracking): the solar system is behind the
    observer, culled by preUpdate => the draw PROVABLY cannot reach the frame:
    new-vs-old cross px>32 == 0 and maxdiff == 0 (bit-identical), floor 0.
  * AIMED (track the Sun from near, kept through the fly-up): the Sun big halo
    is in the view cone => new-vs-old cross px>32 >> 0 at the image centre,
    maxdiff saturates, drawBigHalo fires (INTENT 11.83).
The ENTIRE centre signal is gated on the solar system being in the cone => it is
the solar-system-from-afar content reaching the frame through the galactic
executor draw hook, not a global new-vs-old offset.  (The INTENT 11.80 pre-fix-
binary discriminator transfers too: the anchored legs use the SAME
drawExperimental hook that measured 0 px>32 pre-fix.)

MEASUREMENT NOTE (recorded, not hidden): the new-path Sun big halo carries a
slow re-entry transient (~1-2 min texture-streaming settle; floor_new 278->35
across successive quads) AND the OLD catalog "Sol" star draws at the same centre
in BOTH phases (INTENT 11.80 composition note) - so floor_new and the hide-Sun
mutation are confounded at the centre.  The AIMED-vs-NOT-AIMED contrast sidesteps
both: NOT-AIMED has NO centre content at all (cross 0 / maxdiff 0 / floor 0), so
the whole centre halo+transient is proven solar-gated.

The AIM legs are moveto-outward only; the one DESCENT (to re-establish the near
selection for the aim leg) re-anchors Earth->SolarSystem (INTENT 11.59(d),
R13/B31 DEFERRED, EXCLUDED) - `set home_planet Earth` restores the anchor before
the aim fly-up.

PRECONDITION: fresh launch, FISHEYE, init_fov=340 (runner owns config + md5).
Pass an ABSOLUTE outdir.  Runner: b22_live_run.sh (stale-instance + retry).
"""
import socket, time, json, sys, os
import numpy as np
from PIL import Image

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "artifacts", "b20d")
os.makedirs(OUT, exist_ok=True)
ANCHOR = "Earth"
LATLON = "moveto lat 48.85 lon 2.35 alt 100 duration 0"
GAL_M = "12000000000000000"          # 1.2e16 m -> InGalaxy (Sun ~8e4 AU, mag ~ -2.2)
UNI_M = "185700000000000"            # 1.857e14 m x2 -> InUniverse (INTENT 11.80 ladder)

results = []
def check(name, ok, detail):
    results.append({"name": name, "ok": bool(ok), "detail": detail})
    print(f"{'PASS' if ok else 'FAIL'}  {name}: {detail}", flush=True)

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)

def cam(path):
    with open(path) as f:
        return json.loads(f.readline())["camera"]

def dump(sock, tag, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/b20d_{tag}.json", pause)
    return cam(f"{OUT}/b20d_{tag}.json")

def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)

def img(name):
    return np.asarray(Image.open(f"{OUT}/{name}.png").convert("RGB"), dtype=np.int16)

def diff(a, b):
    d = np.abs(a - b)
    m = (d > 32).any(axis=2)
    n = int(m.sum())
    bbox = None
    if n:
        ys, xs = np.nonzero(m); bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
    return n, int(d.max()), bbox

def phase_quad(sock, tag):
    """new/old/new/old (reversible pair)."""
    send(sock, "flag experimental_path on", 2);  shot(sock, f"{tag}_n1")
    send(sock, "flag experimental_path off", 2); shot(sock, f"{tag}_o1")
    send(sock, "flag experimental_path on", 2);  shot(sock, f"{tag}_n2")
    send(sock, "flag experimental_path off", 2); shot(sock, f"{tag}_o2")
    send(sock, "flag experimental_path on", 2)
    n1, o1, n2, o2 = (img(f"{tag}_{x}") for x in ("n1", "o1", "n2", "o2"))
    cross1, mx1, bb1 = diff(n1, o1)
    cross2, mx2, bb2 = diff(n2, o2)
    fl_n, flmx, _ = diff(n1, n2)
    notblack = int((n1.max(axis=2) > 16).sum())
    print(f"  [{tag}] cross1={cross1}(mx{mx1})@{bb1} cross2={cross2}(mx{mx2})@{bb2}"
          f" floor_new={fl_n}(mx{flmx}) notblack={notblack}", flush=True)
    return {"cross1": cross1, "cross2": cross2, "maxdiff1": mx1, "maxdiff2": mx2,
            "floor_new": fl_n, "bb1": bb1, "bb2": bb2, "notblack": notblack}

applog = os.path.join(OUT, "app.log")
def log_has(needle):
    try:
        with open(applog, errors="replace") as f:
            return needle in f.read()
    except FileNotFoundError:
        return False

report = {"legs": {}, "quads": {}}
s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "zoom fov 340 duration 0", 1)
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)
send(s, "flag atmosphere off", 1)
send(s, f"set home_planet {ANCHOR}", 2)
send(s, "camera action free_mode state off", 1)
send(s, LATLON, 2)

# ============================ CONTROL: NOT AIMED (counterfactual) ============
# No tracking: the fly-up leaves the solar system BEHIND the observer -> culled.
send(s, f"moveto altitude {GAL_M} duration 0", 6)
time.sleep(4)
cN = dump(s, "gal_noaim")
report["legs"]["gal_noaim"] = cN
check("noaim_anchor_kept", cN.get("reference") == ANCHOR and cN.get("freeMode") is False,
      f"reference={cN.get('reference')!r} freeMode={cN.get('freeMode')} (anchored)")
check("noaim_executor", log_has("->InGalaxy"), "app.log '->InGalaxy'")
qN = phase_quad(s, "gal_noaim")
report["quads"]["gal_noaim"] = qN
check("noaim_inert", qN["cross1"] <= 8 and qN["maxdiff1"] <= 8,
      f"cross={qN['cross1']} maxdiff={qN['maxdiff1']} (draw PROVABLY absent: solar system "
      f"culled behind the observer - the counterfactual leg)")

# ============================ re-establish anchor + AIM the Sun ==============
send(s, LATLON, 3)                      # descend to near (re-anchors -> SolarSystem, witness)
send(s, f"set home_planet {ANCHOR}", 2) # restore the Earth anchor (R13 excluded)
send(s, "camera action free_mode state off", 1)
send(s, LATLON, 2)
send(s, "select planet Sun", 1.5)       # near: old system current -> selection persists
send(s, "flag track_object on", 6)      # aim at the Sun while near

def aim_leg(tag, moves, exec_needle, exec_name):
    for m in moves:
        send(s, f"moveto altitude {m} duration 0", 6)   # rise; tracking keeps the aim
    time.sleep(6)
    send(s, "flag track_object off", 2)  # FREEZE the Sun-facing aim (kills tracking jitter)
    time.sleep(8)
    c = dump(s, tag)
    report["legs"][tag] = c
    check(f"{tag}_anchor_kept", c.get("reference") == ANCHOR and c.get("freeMode") is False,
          f"reference={c.get('reference')!r} refParent={c.get('refParent')!r} "
          f"freeMode={c.get('freeMode')} (anchor KEPT at galactic altitude)")
    check(f"{tag}_executor", log_has(exec_needle), f"app.log {exec_needle!r} ({exec_name})")
    check(f"{tag}_distance", (c.get("distance") or 0) > 1e3,
          f"distance={c.get('distance')} AU (galactic-scale far view)")
    q = phase_quad(s, tag)
    report["quads"][tag] = q
    check(f"{tag}_draws", q["cross1"] > 64 and q["cross2"] > 64 and q["maxdiff1"] > 64,
          f"cross {q['cross1']}/{q['cross2']} (mx {q['maxdiff1']}) @{q['bb1']} "
          f"(new draws the solar-system-from-afar Sun big halo; old draws nothing solar)")
    check(f"{tag}_discriminates", q["cross1"] > 8 * max(qN['cross1'], 1),
          f"aimed cross {q['cross1']} >> not-aimed cross {qN['cross1']} "
          f"(the centre content is solar, gated on being in the {exec_name} view cone)")
    check(f"{tag}_notblack", q["notblack"] > 0, f"lit px={q['notblack']}")
    return q

aim_leg("gal_aim", [GAL_M], "->InGalaxy", "InGalaxy")
aim_leg("uni_aim", [UNI_M, UNI_M], "->InUniverse", "InUniverse")

s.close()
report["results"] = results
with open(f"{OUT}/b20d_result.json", "w") as f:
    json.dump(report, f, indent=1)
bad = [r for r in results if not r["ok"]]
print(f"=== {len(results)-len(bad)}/{len(results)} PASS ===", flush=True)
sys.exit(1 if bad else 0)
