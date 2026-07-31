#!/usr/bin/env python3
"""§5.32 gate: the camera reads its reference LIVE, not from a per-frame cache.

Two legs, each measured on BOTH binaries (the pre-fix one is the counterfactual)
and each self-certifying — the pre-fix run is what proves the scene actually
puts the instrument in the regime the leg is about.

  DESCEND (§11.108(e), the measured observable of the row)
    Ten `camera action descend coef 0.99`, issued two ways:
      BURST   all ten in ONE socket write, no pause  -> they land in one frame
      SPREAD  one per send, a pause between          -> one per frame
    A descent is multiplicative on the live altitude, so the two cadences must
    give the SAME altitude: 200 km * 0.99^10.  Pre-fix `Camera::descend` reads
    `reference->getObservedPosition()` — the reference's `mat`, republished once
    per frame — so all ten members of the burst see the geometry of the eye
    BEFORE the first of them moved it, and the burst compounds LINEARLY
    (x(1 - 10*0.01) = x0.9).  The discriminator is the DIFFERENCE between the
    two cadences within one binary: it needs no claim about how many frames
    elapsed, because the spread leg is its own control.

  SPIN (the row's own half: stale spin in the bound placement, which is what the
        persistent longitude conversion of setBoundToSurface consumes)
    A synthetic body with `rot_periode 24` (period = 1 day, so the spin over
    dJD days is exactly 2*pi*dJD), anchored observer, bound to the surface,
    looking at the zenith with a NARROW fov so the reference is culled and
    `dispatchUpdate` skips its `update()` — the ONE node that can skip it.  The
    camera's own matrix carries `computeSurfaceToBody()`, so the rotation
    between two dumps taken at jd and jd+dJD IS the spin the camera composed
    with.  Pre-fix that rotation is 0 (the spin froze at the last frame the
    reference was drawn on: the null is what proves the reference is culled);
    post-fix it is 2*pi*dJD.

Protocol: this script owns the app lifecycle.  Frozen-data md5 asserted in ==
out around every launch (D35 / §2.0 D13).  No other spacecrafter process may be
running (§11.121(m)).

    cd claude/harness && DISPLAY=:2 ./f20_s532.py [outdir] [--tag NAME]
    SC_BIN=<other binary> ./f20_s532.py ...   # the counterfactual leg

Exit 0 = every leg green (post-fix expectations); 1 = any failure.
`--prefix` inverts the expectations: the run then asserts the DEFECT is present,
which is how the counterfactual binary is shown to be a real counterfactual and
not a dead instrument.
"""

import hashlib
import json
import math
import os
import socket
import subprocess
import sys
import time
from pathlib import Path

HOME = Path.home()
USERDIR = HOME / ".spacecrafter"
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
AU_KM = 149597870.7

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
PREFIX_MODE = "--prefix" in sys.argv[1:]
TAG = "pre" if PREFIX_MODE else "post"
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f20").resolve()
OUT.mkdir(parents=True, exist_ok=True)

# The frozen corpus: nothing this gate does may write any of it.
FROZEN = ["config.ini", "ssystem.ini", "galactic.ini", "anchor.ini"]

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def frozen_md5():
    return {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}


def assert_no_other_instance():
    """§11.121(m): a concurrent launcher shares ~/.spacecrafter and invalidates
    every md5 assertion below.  ANY account."""
    # The COMMAND must itself be the binary — a shell whose command line merely
    # mentions the path is not an instance (a `pgrep -f` match on the wrapper is
    # the self-confirming-instrument trap).
    r = subprocess.run(["ps", "-e", "-o", "pid=,args="], capture_output=True, text=True)
    live = [l.strip() for l in r.stdout.splitlines()
            if l.split(maxsplit=1)[1:] and
            l.split(maxsplit=1)[1].split()[0].endswith("/spacecrafter")]
    if live:
        fail("another spacecrafter process is running: " + " | ".join(live))
        return False
    ok("no other spacecrafter instance")
    return True


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


class App:
    def __init__(self, tag):
        self.tag = tag
        self.log = OUT / f"f20_{TAG}_{tag}.applog"
        self.proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                     stdout=open(self.log, "w"), stderr=subprocess.STDOUT,
                                     env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock = wait_port()
        time.sleep(10)
        self.cmd("timerate rate 0")

    def cmd(self, c, pause=0.7):
        self.sock.sendall((c + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def burst(self, c, n, pause=1.5):
        """n copies of one command in ONE write, no pause between them."""
        self.sock.sendall(((c + "\n") * n).encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(65536)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def dump(self, name, pause=2.0):
        p = OUT / f"f20_{TAG}_{name}.json"
        p.unlink(missing_ok=True)
        self.cmd(f"body action dual_dump filename {p}", pause)
        if not p.exists():
            raise RuntimeError(f"dump {p} not written")
        return p

    def quit(self):
        self.cmd("shutdown action now", 1.0)
        self.sock.close()
        try:
            self.proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            fail(f"{self.tag}: app did not exit within 30 s")


def _clean(ln):
    return (ln.strip().replace("-nan", "null").replace("nan", "null")
            .replace("-inf", "-1e308").replace("inf", "1e308"))


def camera(path):
    with open(path) as f:
        for ln in f:
            ln = _clean(ln)
            if not ln:
                continue
            o = json.loads(ln)
            if o.get("type") == "header":
                return o["camera"]
    raise RuntimeError(f"no header in {path}")


def plen(c):
    return math.sqrt(sum(x * x for x in c["position"]))


def rot3(m):
    """The 3x3 rotation part of the dumped column-major 4x4."""
    return [[m[0], m[4], m[8]],
            [m[1], m[5], m[9]],
            [m[2], m[6], m[10]]]


def rel_angle(m0, m1):
    """Angle of R0^T R1, in degrees — the rotation the camera matrix gained."""
    a, b = rot3(m0), rot3(m1)
    # R0^T R1 trace = sum_k (R0^T R1)_kk = sum_k sum_j R0[j][k] * R1[j][k]
    tr = sum(a[j][k] * b[j][k] for k in range(3) for j in range(3))
    c = max(-1.0, min(1.0, (tr - 1.0) / 2.0))
    return math.degrees(math.acos(c))


# ---------------------------------------------------------------------------
# LEG 1 - DESCEND: burst vs spread
# ---------------------------------------------------------------------------
BODY_RADIUS_KM = 6000.0
START_ALT_M = 200000          # metres, as `moveto altitude` takes them
COEF = 0.99
N = 10
GEOMETRIC = COEF ** N          # 0.9043821
LINEAR = 1.0 - N * (1.0 - COEF)  # 0.9 exactly


def altitude_km(c):
    return plen(c) * AU_KM - BODY_RADIUS_KM


def scene_descend(app):
    app.cmd("date jday 2461233.5", 1.0)
    app.cmd("timerate rate 0", 1.0)
    app.cmd("body action load name S532B radius 6000 parent Sun type Planet "
            "oblateness 0.0 albedo 0.3 halo false color 0.6,0.6,0.9 "
            "tex_map bodies/moon.png coord_func still_orbit "
            "orbit_x 1200 orbit_y 0 orbit_z 0", 2.5)
    app.cmd("set home_planet S532B", 3.0)
    app.cmd("camera action free_mode state on", 1.5)
    app.cmd("select planet S532B", 1.0)
    app.cmd("flag track_object on", 1.5)


def leg_descend(app):
    scene_descend(app)
    app.cmd(f"moveto lat 0 lon 0 alt {START_ALT_M} duration 0", 2.5)
    start = altitude_km(camera(app.dump("desc_start")))

    app.burst(f"camera action descend coef {COEF}", N)
    burst = altitude_km(camera(app.dump("desc_burst")))

    app.cmd(f"moveto lat 0 lon 0 alt {START_ALT_M} duration 0", 2.5)
    start2 = altitude_km(camera(app.dump("desc_reset")))
    for _ in range(N):
        app.cmd(f"camera action descend coef {COEF}", 0.6)
    spread = altitude_km(camera(app.dump("desc_spread")))

    pred_geo = start * GEOMETRIC
    pred_lin = start * LINEAR
    print(f"\n== DESCEND ({TAG}) ==", flush=True)
    print(f"  start        {start:.6f} km   (reset {start2:.6f})", flush=True)
    print(f"  burst  x{N}   {burst:.6f} km", flush=True)
    print(f"  spread x{N}   {spread:.6f} km", flush=True)
    print(f"  predicted geometric {pred_geo:.6f}  linear {pred_lin:.6f}", flush=True)
    print(f"  |burst - spread| = {abs(burst - spread) * 1000:.3f} m", flush=True)

    if abs(start - start2) > 1e-3:
        fail(f"descend: the reset did not restore the start altitude "
             f"({start:.6f} vs {start2:.6f} km)")
    # The spread leg is the reference cadence on BOTH binaries.
    if abs(spread - pred_geo) > 0.01:
        fail(f"descend: spread {spread:.6f} km is not the geometric "
             f"{pred_geo:.6f} km — the scene is not the one this leg describes")
    else:
        ok(f"descend: spread == start*{COEF}^{N} ({spread:.6f} vs {pred_geo:.6f} km)")

    delta_m = abs(burst - spread) * 1000
    if PREFIX_MODE:
        if abs(burst - pred_lin) > 0.01:
            fail(f"descend[pre]: burst {burst:.6f} km is not the LINEAR "
                 f"{pred_lin:.6f} km — the ten did not land in one frame, so this "
                 f"binary is not a counterfactual for the burst leg")
        else:
            ok(f"descend[pre]: burst compounds LINEARLY ({burst:.6f} km), "
               f"{delta_m:.1f} m off the spread cadence — the defect is present")
    else:
        if delta_m > 10.0:
            fail(f"descend: burst and spread differ by {delta_m:.1f} m — the "
                 f"descent is still cadence-dependent")
        else:
            ok(f"descend: burst == spread within {delta_m:.3f} m — the descent is "
               f"cadence-INDEPENDENT ({burst:.6f} vs {spread:.6f} km)")
    return {"start": start, "burst": burst, "spread": spread,
            "pred_geo": pred_geo, "pred_lin": pred_lin, "delta_m": delta_m}


# ---------------------------------------------------------------------------
# LEG 2 - SPIN: the bound placement of an observer whose reference is culled
# ---------------------------------------------------------------------------
DJD = 0.1           # days; period is 1 day, so the spin is 0.2*pi = 36 deg
JD0 = 2461233.5


def leg_spin(app):
    app.cmd("timerate rate 0", 1.0)
    app.cmd(f"date jday {JD0}", 1.5)
    # rot_periode 24 h => period 1 day => spin over DJD days is exactly 2*pi*DJD.
    app.cmd("body action load name S532Spin radius 6000 parent Sun type Planet "
            "oblateness 0.0 albedo 0.3 halo false color 0.6,0.6,0.9 "
            "tex_map bodies/moon.png coord_func still_orbit "
            "orbit_x 0 orbit_y 1200 orbit_z 0 rot_periode 24 "
            "rot_obliquity 0 rot_equator_ascending_node 0", 2.5)
    app.cmd("set home_planet S532Spin", 3.0)
    app.cmd("flag track_object off", 1.0)
    # Far enough that the body subtends almost nothing, and looking away from
    # it: BOTH are needed to put the reference outside the cull cone, which is
    # the regime in which dispatchUpdate skips its update().  `boundToSurface`
    # stays true out here — that is the row's own observation (Camera.hpp: it is
    # a persistent default, not an altitude invariant), and it is what keeps the
    # spin in the composition at a distance where the body is culled.
    app.cmd("moveto lat 0 lon 0 alt 1000000000 duration 0", 2.5)   # 1e6 km up
    app.cmd("zoom fov 30 duration 0", 1.5)                         # narrow the cone
    # alt = -89: at +90 the camera's eye-forward (-z) points AT the reference
    # centre (a tracked body dumps r14 = -distance).  Measured, not assumed —
    # the leg asserts the resulting angle against the cull cone below.
    app.cmd("look_at azimuth 0 altitude -89 duration 0", 2.0)
    c0 = camera(app.dump("spin_0"))
    # The leg's structural precondition, computed from the dump itself.
    t = c0["mat"][12:15]
    tn = math.sqrt(sum(x * x for x in t))
    ang_ref = math.degrees(math.acos(max(-1.0, min(1.0, -t[2] / tn))))
    bR = BODY_RADIUS_KM / AU_KM
    d = c0["distance"]
    half_ang = math.degrees(math.atan(bR / math.sqrt(d * d - bR * bR))) if d > bR else 180.0
    cull = math.degrees(c0["cullHalfFov"]) + half_ang
    print(f"  reference direction {ang_ref:.3f} deg off the view axis; "
          f"cull cone {cull:.3f} deg (cullHalfFov {math.degrees(c0['cullHalfFov']):.3f} "
          f"+ halfAngularSize {half_ang:.3f})", flush=True)
    if ang_ref <= cull:
        fail(f"spin: the reference is INSIDE the cull cone ({ang_ref:.3f} <= "
             f"{cull:.3f} deg) — it is being updated every frame, so neither "
             f"binary is in the regime this leg measures")
    app.cmd(f"date jday {JD0 + DJD}", 2.5)
    c1 = camera(app.dump("spin_1"))

    ang = rel_angle(c0["mat"], c1["mat"])
    pred = math.degrees(2 * math.pi * DJD) % 360.0
    print(f"\n== SPIN ({TAG}) ==", flush=True)
    print(f"  reference {c0['reference']!r} bound={c0['boundToSurface']} "
          f"free={c0['freeMode']}", flush=True)
    print(f"  camera-matrix rotation over dJD={DJD} d : {ang:.6f} deg "
          f"(predicted spin {pred:.6f} deg)", flush=True)

    for k in ("longitude", "latitude", "distance", "alt", "az", "heading"):
        if abs(c0[k] - c1[k]) > 1e-6:
            fail(f"spin: {k} moved across the date jump ({c0[k]} -> {c1[k]}) — "
                 f"the matrix rotation is then not attributable to the spin alone")
    if c0["reference"] != "S532Spin" or not c0["boundToSurface"] or c0["freeMode"]:
        fail(f"spin: the scene is not an anchored surface-bound observer on "
             f"S532Spin ({c0['reference']}, bound={c0['boundToSurface']}, "
             f"free={c0['freeMode']})")

    if PREFIX_MODE:
        if ang > 0.01:
            fail(f"spin[pre]: the camera matrix DID rotate ({ang:.6f} deg) — the "
                 f"reference was not culled, so this scene is not the regime the "
                 f"leg is about and the post-fix number proves nothing")
        else:
            ok(f"spin[pre]: the bound placement did NOT follow the spin "
               f"({ang:.6f} deg against {pred:.6f} predicted) — the reference is "
               f"culled and its cached spin is frozen: the defect is present")
    else:
        if abs(ang - pred) > 0.05:
            fail(f"spin: the bound placement rotated {ang:.6f} deg, predicted "
                 f"{pred:.6f} deg")
        else:
            ok(f"spin: the bound placement follows the spin of a CULLED reference "
               f"({ang:.6f} deg vs {pred:.6f} predicted)")
    return {"angle": ang, "pred": pred}


def main():
    print(f"SC_BIN = {SC_BIN}  ({TAG} expectations)", flush=True)
    if not assert_no_other_instance():
        return 1
    before = frozen_md5()

    results = {}
    app = App("desc")
    try:
        results["descend"] = leg_descend(app)
    finally:
        app.quit()

    app = App("spin")
    try:
        results["spin"] = leg_spin(app)
    finally:
        app.quit()

    after = frozen_md5()
    if before != after:
        for k in before:
            if before[k] != after.get(k):
                fail(f"frozen data changed across the run: {k} "
                     f"{before[k]} -> {after.get(k)}")
    else:
        ok(f"frozen md5 in == out ({', '.join(f'{k} {v[:8]}' for k, v in before.items())})")

    (OUT / f"f20_s532_{TAG}.json").write_text(json.dumps(results, indent=1))
    print(f"\nwrote {OUT}/f20_s532_{TAG}.json", flush=True)
    print(f"\n{'FAILED: ' + str(len(FAILS)) if FAILS else 'ALL GREEN'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
