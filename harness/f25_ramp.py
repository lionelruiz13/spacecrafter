#!/usr/bin/env python3
"""B34's LAST MEMBER — the interactive VIEW and ZOOM ramps, measured per STEP
(INTENT §11.108(b2)/§11.133).

    cd claude/harness && DISPLAY=:2 ./f25_ramp.py <out> [--bin B] [--pre]
                                                 [--phase frames|turn|diag|fov|zoom|all]

The row's finding (F4, §11.108(b2)): holding an arrow key turns the OLD
navigator and leaves the drawn camera exactly where it was. The fix mirrors the
turn at `Core::updateMove`, and the claim it has to support is a PER-STEP one —
same law, same step, same number of steps, same stop. Two absolute dumps either
side of a key hold cannot say that, so every leg here reads the `ramp` object
the instrument commit added to `body action dual_dump`: one row per frame in
which a ramp is active, carrying the frame's inputs and BOTH paths' view
parameters before and after the step.

THE FRAME RELATION IS MEASURED, NOT ASSUMED (phase `frames`). Old holds the view
as a VECTOR whose spherical coordinates are (azVision, altVision) in the active
mount's frame; the camera holds two PARAMETERS whose convention is
az = −lng, alt = −lat of the forward direction (`Camera::paramForward`, and
`viewRotation·paramForward == (0,0,−1)` is the check by derivation). If that is
right, then with both paths aimed at the same body:
        alt_cam + altVision_old == 0      and      az_cam + azVision_old == const
and the ramp mirror must apply +deltaAz to `az` and −deltaAlt to `alt`. The
phase measures both on several bodies at different sky positions, because a
sign taken from one direction is a coin toss.

Every phase is run on BOTH binaries. `--pre` inverts the expectations rather
than skipping the legs: on the pre-fix binary (a build of the instrument
commit, which HAS the readout) the new path's parameters must be BIT-IDENTICAL
across every step while the old path's move — an absent EFFECT, not an absent
field (§11.132(a)).
"""

import argparse, json, math, os, re, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
JD = 2461233.5
WIN = "1024x1024"

# ---------------------------------------------------------------- predictions
# Committed before the runs (the wave's discipline). Magnitudes DERIVED, not
# fitted:
#  * per-step parity bar: the camera holds alt/az as float32 of magnitude O(1),
#    so one step's before/after difference carries <= 2 ulp = 2.4e-7 rad, and
#    the azimuth wrap adds one more ulp of pi. Bar 1e-6 rad (5.7e-5 deg).
#  * cumulative bar over a 2.5 s hold (~360 steps at the shipped 144 fps cap):
#    the step ITSELF is rounded when added to a float32 of magnitude O(1),
#    <= 0.5 ulp = 6e-8 rad per step; random walk over 360 steps = 1.1e-6 rad,
#    worst case systematic 2.2e-5 rad = 1.2e-3 deg = 0.014 px on a 1024-px-
#    radius 180 deg dome. Bar 5e-5 rad.
#  * pole clamp: old pins the view altitude at pi/2 - 1e-6 (double). The camera
#    holds it as float32 -> pi/2 - 9.5367e-7, i.e. 4.6e-8 rad = 2.7e-6 deg from
#    old's value. Bar 1e-6 rad.
STEP_BAR = 1e-6          # rad, per step, view-space
CUM_BAR = 5e-5           # rad, over one hold
POLE_BAR = 1e-6          # rad, clamped altitude, old vs new
FRAME_BAR = 2e-4         # rad, the alt_cam + altVision_old == 0 relation
                         # (the tracked-aim residual: two independent aiming
                         # laws settling on the same body, not a bit relation)

FAILS = []


def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok:
        FAILS.append(name)


def note(msg):
    print("--   " + msg, flush=True)


def wrap_pi(x):
    return (x + math.pi) % (2 * math.pi) - math.pi


_NONFINITE = re.compile(r'([:\[,]\s*)(-?(?:nan|inf))')


def _jload(line):
    return json.loads(_NONFINITE.sub(
        lambda m: m.group(1) + ("NaN" if "nan" in m.group(2) else
                                ("-Infinity" if m.group(2)[0] == "-" else "Infinity")), line))


class App:
    """One fresh launch on the installed HOME (the ramp is a config-driven law:
    move_speed / zoom_speed / viewing_mode / init_fov are the shipped values and
    a farm would substitute them). Nothing here writes into the installed data —
    dumps and screenshots go to the out dir — and the frozen md5s are asserted
    by the runner around the whole run."""

    def __init__(self, binary, out, applog):
        self.out = Path(out)
        stale = subprocess.run(["pgrep", "-x", "spacecrafter"], capture_output=True, text=True)
        if stale.stdout.strip():
            raise RuntimeError("§11.121(m): another spacecrafter is running: " + stale.stdout)
        self.proc = subprocess.Popen(
            [binary], stdout=open(applog, "w"), stderr=subprocess.STDOUT,
            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock, t0 = None, time.time()
        while time.time() - t0 < 120:
            if self.proc.poll() is not None:
                raise RuntimeError("app died before opening its port")
            try:
                self.sock = socket.create_connection(("127.0.0.1", 7805), timeout=1)
                break
            except OSError:
                time.sleep(1)
        if self.sock is None:
            self.proc.kill()
            raise RuntimeError("port 7805 never opened")
        time.sleep(10)

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def dump(self, tag, pause=3.0):
        p = self.out / f"{tag}.json"
        p.unlink(missing_ok=True)
        self.send(f"body action dual_dump filename {p}", pause)
        for _ in range(30):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.4)
        if not p.exists():
            raise RuntimeError(f"dump {p} was not written")
        header, bodies = None, {}
        for line in open(p, encoding="utf-8", errors="replace"):
            line = line.strip()
            if not line:
                continue
            d = _jload(line)
            if d.get("type") == "header":
                header = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
        return header, bodies

    def shot(self, tag):
        p = self.out / f"{tag}.png"
        p.unlink(missing_ok=True)
        self.send(f"body action screenshot filename {p}", 2.5)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                return p
            time.sleep(0.4)
        raise RuntimeError(f"screenshot {p} was not written")

    def stop(self):
        try:
            self.send("shutdown action now", 1)
            self.sock.close()
            self.proc.wait(timeout=40)
        except Exception:
            self.proc.kill()


def px32(a, b):
    ia = np.asarray(Image.open(a).convert("RGB")).astype(np.int32)
    ib = np.asarray(Image.open(b).convert("RGB")).astype(np.int32)
    return int((np.abs(ia - ib).max(axis=2) > 32).sum())


def lit(p):
    return int((np.asarray(Image.open(p).convert("RGB")).max(axis=2) > 8).sum())


XKEY = None


def hold(keys, ms):
    r = subprocess.run([XKEY, "spacecrafter", keys, str(ms), WIN],
                       capture_output=True, text=True)
    print(f"   xkey {keys} {ms}ms -> rc={r.returncode} {r.stdout.strip()}{r.stderr.strip()}",
          flush=True)
    if r.returncode != 0:
        raise RuntimeError("xkey failed - the keystroke never reached the app")


# ------------------------------------------------------------- ramp analysis
def steps_of(header):
    return header["ramp"]["steps"]


def new_run(header, prev_total):
    """The rows written since `prev_total` — one hold's worth."""
    ramp = header["ramp"]
    n = ramp["total"] - prev_total
    if ramp["dropped"]:
        note(f"ring dropped {ramp['dropped']} rows (capacity {ramp['capacity']})")
    return ramp["steps"][-n:] if n > 0 else [], ramp["total"]


def view_deltas(s):
    """Both paths' step in VIEW space (old's own convention: azimuth-lng and
    altitude of the vision direction). The camera's parameters are the negatives
    of that pair by construction (paramForward), so this is where the two are
    directly comparable — not in the camera's parameter space."""
    d_az_old = wrap_pi(s["oldAzAfter"] - s["oldAz"])
    d_alt_old = s["oldAltAfter"] - s["oldAlt"]
    d_az_new = -wrap_pi(s["newAzAfter"] - s["newAz"])
    d_alt_new = -(s["newAltAfter"] - s["newAlt"])
    return d_az_old, d_alt_old, d_az_new, d_alt_new


def analyse(rows, tag, pre, expect_az=None, expect_alt=None, clamped_ok=False):
    """expect_az/expect_alt: the sign the ramp law predicts for the VIEW-space
    step (-1, 0, +1). Returns (cum_az_old, cum_alt_old, cum_az_new, cum_alt_new)."""
    act = [s for s in rows if s["active"]]
    rel = [s for s in rows if not s["active"]]
    if not act:
        check(f"{tag}_rows", False, "no active ramp row was recorded — the key never reached updateMove")
        return (0, 0, 0, 0)
    worst_az = worst_alt = 0.0
    worst_law_old = worst_law_new = 0.0
    cum = [0.0, 0.0, 0.0, 0.0]
    clamp_rows = 0
    for s in act:
        daz_o, dalt_o, daz_n, dalt_n = view_deltas(s)
        cum[0] += daz_o; cum[1] += dalt_o; cum[2] += daz_n; cum[3] += dalt_n
        # the ramp law itself, on each path independently
        worst_law_old = max(worst_law_old, abs(daz_o - (-s["dAz"])))
        if not pre:
            worst_law_new = max(worst_law_new, abs(daz_n - (-s["dAz"])))
        # the altitude leg is skipped on a clamp row (old pins instead of stepping)
        if abs(dalt_o - s["dAlt"]) > STEP_BAR:
            clamp_rows += 1
            if not clamped_ok:
                worst_law_old = max(worst_law_old, abs(dalt_o - s["dAlt"]))
        worst_az = max(worst_az, abs(daz_o - daz_n))
        worst_alt = max(worst_alt, abs(dalt_o - dalt_n))
    n = len(act)
    note(f"{tag}: {n} active rows, {len(rel)} release row(s), {clamp_rows} clamp row(s); "
         f"dt {act[0]['dt']}..{act[-1]['dt']} ms, fov {act[0]['fov']:.4f} deg, "
         f"|dAz| {abs(act[0]['dAz']):.6e} rad/step")
    note(f"{tag}: cumulative VIEW step  old (az {cum[0]:+.6f}, alt {cum[1]:+.6f}) rad ; "
         f"new (az {cum[2]:+.6f}, alt {cum[3]:+.6f}) rad")
    check(f"{tag}_old_law", worst_law_old <= STEP_BAR,
          f"the OLD path takes exactly the step Core::updateMove computed "
          f"(worst |d_view - vzm.delta| = {worst_law_old:.3e} rad, bar {STEP_BAR:.0e})")
    if expect_az is not None:
        got = math.copysign(1, cum[0]) if abs(cum[0]) > 1e-6 else 0
        check(f"{tag}_old_direction", got == expect_az,
              f"OLD view azimuth moved {cum[0]:+.6f} rad (expected sign {expect_az:+d})")
    if expect_alt is not None:
        got = math.copysign(1, cum[1]) if abs(cum[1]) > 1e-6 else 0
        check(f"{tag}_old_direction_alt", got == expect_alt,
              f"OLD view altitude moved {cum[1]:+.6f} rad (expected sign {expect_alt:+d})")
    if pre:
        frozen = all(s["newAzAfter"] == s["newAz"] and s["newAltAfter"] == s["newAlt"]
                     for s in rows)
        check(f"{tag}_PRE_new_frozen", frozen and abs(cum[2]) + abs(cum[3]) == 0.0,
              f"pre-fix: the camera's parameters are BIT-IDENTICAL across every one of "
              f"the {n} steps (cumulative {cum[2]:.1e}/{cum[3]:.1e} rad) while the old "
              f"path moved {cum[0]:+.4f}/{cum[1]:+.4f} rad — an absent EFFECT, on a "
              f"binary that HAS the readout")
    else:
        check(f"{tag}_new_law", worst_law_new <= STEP_BAR,
              f"the NEW path takes exactly the same step (worst |d_view - vzm.delta| = "
              f"{worst_law_new:.3e} rad, bar {STEP_BAR:.0e})")
        check(f"{tag}_per_step_parity", worst_az <= STEP_BAR and worst_alt <= STEP_BAR,
              f"per-step VIEW deltas equal on both paths: worst az {worst_az:.3e}, "
              f"alt {worst_alt:.3e} rad (bar {STEP_BAR:.0e})")
        check(f"{tag}_cumulative_parity",
              abs(cum[0] - cum[2]) <= CUM_BAR and abs(cum[1] - cum[3]) <= CUM_BAR,
              f"over the whole hold: d(az) {cum[0]-cum[2]:+.3e}, d(alt) {cum[1]-cum[3]:+.3e} rad "
              f"(bar {CUM_BAR:.0e}; predicted <= 2.2e-5 worst case from the float32 step rounding)")
    # the release row: BOTH paths must stop dead in the same frame
    if rel:
        r = rel[-1]
        daz_o, dalt_o, daz_n, dalt_n = view_deltas(r)
        check(f"{tag}_release", (r["dAz"] == 0 and r["dAlt"] == 0
                                 and abs(daz_o) + abs(dalt_o) == 0.0
                                 and abs(daz_n) + abs(dalt_n) == 0.0),
              f"the frame after the key-up steps NEITHER path (old {daz_o:.1e}/{dalt_o:.1e}, "
              f"new {daz_n:.1e}/{dalt_n:.1e} rad) — old's ramp has no deceleration to "
              f"reproduce, and the mirror installs no smoothing plan that would add one")
    else:
        check(f"{tag}_release", False, "no release row was recorded")
    return tuple(cum)


# --------------------------------------------------------------------- scene
SKY_OFF = ("atmosphere", "fog", "landscape", "milky_way", "nebulae", "nebula_names",
           "constellation_drawing", "constellation_lines", "constellation_names",
           "constellation_art", "cardinal_points", "planet_names", "planet_orbits",
           "planet_trails", "meridian_line", "zenith_line", "ecliptic_line",
           "equator_line", "azimuthal_grid", "show_fps")


def base_scene(app):
    app.send("flag experimental_path on", 1)
    app.send("timerate rate 0", 1)
    app.send(f"date jday {JD}", 1)
    app.send("meteors zhr 0", 0.6)
    for f in SKY_OFF:
        app.send(f"flag {f} off")
    app.send("flag moon_scaled off", 1)
    app.send("set home_planet Earth", 3)
    app.send("timerate rate 0", 1)


def aim_at(app, body, fov=None):
    """Place the view on BOTH paths without touching either path's private state:
    tracking is the one dual aim (Core::setFlagTracking mirrors to
    Camera::trackBody), and it is switched off again so the ramp starts from a
    still view."""
    app.send(f"select planet {body} pointer off", 2)
    app.send("flag track_object on", 7)
    if fov is not None:
        app.send(f"zoom fov {fov} duration 0", 3)
    app.send("flag track_object off", 2)
    app.send("timerate rate 0", 1)


# -------------------------------------------------------------------- phases
def phase_frames(app, pre):
    """The convention measurement: is the camera's (alt, az) the negative of the
    old vision's (altVision, azVision), and is the azimuth offset a constant?"""
    print("\n===== PHASE frames — the old/new view-parameter relation =====", flush=True)
    base_scene(app)
    rows = []
    for body in ("Moon", "Mars", "Jupiter", "Venus", "Saturn"):
        aim_at(app, body)
        h, _ = app.dump(f"frames_{body}")
        cam = h["camera"]
        nav = h["oldView"]["nav"]
        mount = cam["mount"]
        v = nav["equVision"] if mount == "equatorial" else nav["localVision"]
        old_az = math.atan2(v[1], v[0])
        old_alt = math.asin(v[2] / math.sqrt(sum(c * c for c in v)))
        rows.append((body, old_az, old_alt, cam["az"], cam["alt"], mount))
        note(f"{body:8s} mount={mount}  old(az {math.degrees(old_az):+10.5f}, "
             f"alt {math.degrees(old_alt):+10.5f})  cam(az {math.degrees(cam['az']):+10.5f}, "
             f"alt {math.degrees(cam['alt']):+10.5f}) deg")
    alt_sums = [r[4] + r[2] for r in rows]
    az_sums = [wrap_pi(r[3] + r[1]) for r in rows]
    worst_alt = max(abs(a) for a in alt_sums)
    spread_az = max(az_sums) - min(az_sums)
    check("frames_alt_is_negated", worst_alt <= FRAME_BAR,
          f"alt_cam + altVision_old == 0 on {len(rows)} bodies: worst {worst_alt:.3e} rad "
          f"({math.degrees(worst_alt):.5f} deg, bar {FRAME_BAR:.0e}) ⇒ the camera's `alt` is "
          f"the NEGATIVE of the view altitude, so a +deltaAlt of old maps to −deltaAlt here")
    check("frames_az_offset_constant", spread_az <= FRAME_BAR,
          f"az_cam + azVision_old is CONSTANT across {len(rows)} bodies "
          f"(mean {math.degrees(sum(az_sums)/len(az_sums)):+.5f} deg, spread {spread_az:.3e} rad, "
          f"bar {FRAME_BAR:.0e}) ⇒ the two azimuth origins differ by a fixed rotation about "
          f"the mount pole and azimuth DIFFERENCES map with a + sign onto `az`")
    json.dump([dict(body=r[0], oldAz=r[1], oldAlt=r[2], camAz=r[3], camAlt=r[4], mount=r[5])
               for r in rows], open(app.out / "frames.json", "w"), indent=1)


def phase_turn(app, pre):
    """The four directions, the per-step parity, the release, and the pole."""
    print("\n===== PHASE turn — the four arrow keys, per step =====", flush=True)
    base_scene(app)
    aim_at(app, "Moon", fov=30)
    total = 0
    h, _ = app.dump("turn_zero")
    total = h["ramp"]["total"]
    note(f"projection={h['oldView']['projector']['projectionType']}, "
         f"fov={h['oldView']['projector']['fov']:.4f} deg, "
         f"mount={h['camera']['mount']}, maximum_fps cap per config")

    # --- the row's own bar, on the SAME hold F4 measured (Left, 2500 ms) -----
    b_new = app.shot("turn_before_new")
    app.send("flag experimental_path off", 2)
    b_old = app.shot("turn_before_old")
    app.send("flag experimental_path on", 2)
    h0, bod0 = app.dump("turn_before_cam")
    total = h0["ramp"]["total"]
    hold("Left", 2500)
    time.sleep(1.5)
    h1, bod1 = app.dump("turn_after_cam")
    rows, total = new_run(h1, total)
    a_new = app.shot("turn_after_new")
    app.send("flag experimental_path off", 2)
    a_old = app.shot("turn_after_old")
    app.send("flag experimental_path on", 2)

    analyse(rows, "left", pre, expect_az=+1, expect_alt=0)

    old_px, new_px = px32(b_old, a_old), px32(b_new, a_new)
    note(f"composed screen across the hold: OLD phase {old_px} px>32 (lit {lit(b_old)}), "
         f"NEW phase {new_px} px>32 (lit {lit(b_new)})")
    check("left_positive_control", old_px > 5000,
          f"the OLD phase moved {old_px} px>32 under the injected keystroke — the key "
          f"demonstrably arrived (if ~0 every null below is void)")
    n0 = bod0.get("Moon", {}).get("new", {})
    n1 = bod1.get("Moon", {}).get("new", {})
    o0 = bod0.get("Moon", {}).get("old", {})
    o1 = bod1.get("Moon", {}).get("old", {})
    dn = math.hypot(n1["screen"][0] - n0["screen"][0], n1["screen"][1] - n0["screen"][1])
    do = math.hypot(o1["screen"][0] - o0["screen"][0], o1["screen"][1] - o0["screen"][1])
    note(f"the SAME body in the SAME dump: NEW |d| {dn:.4e} NDC, OLD |d| {do:.2f} px")
    if pre:
        check("left_per_path_screen", dn == 0.0 and do > 100.0,
              f"pre-fix: the NEW path's screen position is bit-identical across the hold "
              f"({dn:.3e} NDC) while the OLD path's moves {do:.1f} px — F4's asymmetry, reproduced")
        check("left_new_screen_still", new_px == 0 or new_px < old_px / 50,
              f"pre-fix: the NEW phase's composed screen barely moves ({new_px} px>32 against "
              f"the old phase's {old_px})")
    else:
        check("left_per_path_screen", dn > 0.05 and do > 100.0,
              f"delivered: BOTH paths moved — NEW |d| {dn:.4f} NDC, OLD |d| {do:.1f} px")
        check("left_new_screen_moves", new_px > 5000,
              f"delivered: the NEW phase's composed screen moved {new_px} px>32 (the row's "
              f"own bar is px>32 on the drawn frame)")

    # --- the other three directions ----------------------------------------
    for key, eaz, ealt in (("Right", -1, 0), ("Up", 0, +1), ("Down", 0, -1)):
        aim_at(app, "Moon", fov=30)
        h, _ = app.dump(f"turn_{key}_before")
        total = h["ramp"]["total"]
        hold(key, 1200)
        time.sleep(1.2)
        h, _ = app.dump(f"turn_{key}_after")
        rows, total = new_run(h, total)
        analyse(rows, key.lower(), pre, expect_az=eaz, expect_alt=ealt)

    # --- SECOND ENTRY of the reversible pair, from the state the first left --
    h, _ = app.dump("turn_second_before")
    total = h["ramp"]["total"]
    hold("Left", 1200)
    time.sleep(1.0)
    h, _ = app.dump("turn_second_mid")
    rows, total = new_run(h, total)
    analyse(rows, "left2", pre, expect_az=+1, expect_alt=0)
    hold("Right", 1200)
    time.sleep(1.0)
    h, _ = app.dump("turn_second_after")
    rows, total = new_run(h, total)
    analyse(rows, "right2", pre, expect_az=-1, expect_alt=0)

    # --- the POLE clamp -----------------------------------------------------
    aim_at(app, "Moon", fov=180)
    h, _ = app.dump("pole_before")
    total = h["ramp"]["total"]
    hold("Up", 6000)
    time.sleep(1.5)
    h, _ = app.dump("pole_after")
    rows, total = new_run(h, total)
    analyse(rows, "pole", pre, expect_alt=+1, clamped_ok=True)
    act = [s for s in rows if s["active"]]
    if act:
        last = act[-1]
        old_view_alt = last["oldAltAfter"]
        new_view_alt = -last["newAltAfter"]
        note(f"after {len(act)} up-steps: OLD view altitude {old_view_alt:.9f} rad "
             f"({math.degrees(old_view_alt):.6f} deg), NEW {new_view_alt:.9f} rad")
        pinned = abs(abs(old_view_alt) - (math.pi / 2 - 1e-6)) < 1e-7
        check("pole_old_pinned", pinned,
              f"the OLD path pinned at pi/2 - 1e-6 = {math.pi/2-1e-6:.9f} "
              f"(measured {abs(old_view_alt):.9f})")
        if pre:
            check("pole_PRE_new_frozen", all(s["newAltAfter"] == s["newAlt"] for s in rows),
                  "pre-fix: the camera's altitude never moved, so it never reached a clamp")
        else:
            check("pole_new_pinned", abs(old_view_alt - new_view_alt) <= POLE_BAR,
                  f"BOTH paths pinned at the same altitude: |old - new| = "
                  f"{abs(old_view_alt-new_view_alt):.3e} rad (bar {POLE_BAR:.0e}; predicted "
                  f"4.6e-8 rad — float32 quantization of pi/2 - 1e-6)")
            # the pole is where the parameter round trip used to flip the azimuth
            azs = [s["newAzAfter"] for s in act[-20:]]
            flips = sum(1 for i in range(1, len(azs)) if abs(wrap_pi(azs[i] - azs[i-1])) > 1.0)
            check("pole_no_az_flip", flips == 0,
                  f"no azimuth flip in the last {len(azs)} clamped steps ({flips} found): the "
                  f"snap assigns the parameters instead of round-tripping them through a "
                  f"direction, whose float32 form collapses pi/2-1e-6 onto the pole and then "
                  f"flips az by pi every frame (measured threshold 3.45e-4 rad)")


def phase_diag(app, pre):
    print("\n===== PHASE diag — two keys in the same frame =====", flush=True)
    base_scene(app)
    aim_at(app, "Moon", fov=60)
    h, _ = app.dump("diag_before")
    total = h["ramp"]["total"]
    hold("Left,Up", 1500)
    time.sleep(1.2)
    h, _ = app.dump("diag_after")
    rows, total = new_run(h, total)
    act = [s for s in rows if s["active"]]
    both = sum(1 for s in act if s["dAz"] != 0 and s["dAlt"] != 0)
    check("diag_both_components", both > 10,
          f"{both} of {len(act)} rows carry BOTH a dAz and a dAlt — the diagonal really is "
          f"two components in one frame, not two ramps in sequence")
    analyse(rows, "diag", pre, expect_az=+1, expect_alt=+1)


def phase_fov(app, pre):
    """The fov-scaling term: the step is proportional to the fov, so the same
    key must produce proportionally different steps at two fovs — and the two
    paths must scale together, because the mirror consumes the SAME scalar."""
    print("\n===== PHASE fov — the cadence at two fovs =====", flush=True)
    base_scene(app)
    res = {}
    for fov in (30.0, 180.0):
        aim_at(app, "Moon", fov=fov)
        h, _ = app.dump(f"fov_{int(fov)}_before")
        total = h["ramp"]["total"]
        hold("Left", 1500)
        time.sleep(1.2)
        h, _ = app.dump(f"fov_{int(fov)}_after")
        rows, total = new_run(h, total)
        act = [s for s in rows if s["active"]]
        cum = analyse(rows, f"fov{int(fov)}", pre, expect_az=+1, expect_alt=0)
        # per-step magnitude normalised by dt: the law is |dAz| = move_speed*dt*fov/30
        per_ms = [abs(s["dAz"]) / s["dt"] for s in act if s["dt"] > 0]
        res[fov] = (sum(per_ms) / len(per_ms), cum, len(act), h["oldView"]["projector"]["fov"])
        note(f"fov {fov}: mean |dAz|/dt = {res[fov][0]:.6e} rad/ms over {len(act)} steps "
             f"(projector fov {res[fov][3]:.4f})")
    ratio = res[180.0][0] / res[30.0][0]
    predicted = res[180.0][3] / res[30.0][3]
    check("fov_scaling_law", abs(ratio - predicted) / predicted < 0.02,
          f"the per-ms step scales with the fov: measured ratio {ratio:.4f} against the "
          f"fov ratio {predicted:.4f} (move_speed*dt*fov/30, core.cpp) — within 2%")
    if not pre:
        for fov in (30.0, 180.0):
            c = res[fov][1]
            check(f"fov{int(fov)}_paths_scale_together", abs(c[0] - c[2]) <= CUM_BAR,
                  f"at fov {fov} both paths turned the same amount: old {c[0]:+.6f} vs "
                  f"new {c[2]:+.6f} rad (d {c[0]-c[2]:+.2e})")


def phase_zoom(app, pre):
    """The ZOOM ramp — probed before anything is mirrored (the dispatch's own
    order): does the drawn fov authority (ModularBody::halfFov) follow the old
    projector's fov under a held zoom key?"""
    print("\n===== PHASE zoom — the continuous zoom ramp =====", flush=True)
    base_scene(app)
    aim_at(app, "Moon", fov=60)
    for key, tag in (("Prior", "in"), ("Next", "out")):
        h, _ = app.dump(f"zoom_{tag}_before")
        total = h["ramp"]["total"]
        fov0 = h["oldView"]["projector"]["fov"]
        half0 = h["camera"]["halfFov"]
        hold(key, 1500)
        time.sleep(1.2)
        h, _ = app.dump(f"zoom_{tag}_after")
        rows, total = new_run(h, total)
        fov1 = h["oldView"]["projector"]["fov"]
        half1 = h["camera"]["halfFov"]
        act = [s for s in rows if s["active"] and s["dFov"] != 0]
        note(f"zoom {tag} ({key}): {len(act)} steps; OLD fov {fov0:.5f} -> {fov1:.5f} deg ; "
             f"NEW halfFov {half0:.7f} -> {half1:.7f} rad "
             f"(= fov {math.degrees(half0)*2:.5f} -> {math.degrees(half1)*2:.5f} deg)")
        check(f"zoom_{tag}_old_moved", abs(fov1 - fov0) > 1e-3,
              f"the OLD projector's fov moved {fov1-fov0:+.5f} deg — the key arrived")
        # per-step: does halfFov track fov?
        worst = 0.0
        for s in act:
            worst = max(worst, abs(math.degrees(s["halfFovAfter"]) * 2 - s["fovAfter"]))
        if pre:
            frozen = all(s["halfFovAfter"] == s["halfFov"] for s in rows)
            check(f"zoom_{tag}_PRE_new_frozen", frozen,
                  f"pre-fix: the DRAWN fov authority is bit-identical across every one of "
                  f"the {len(act)} zoom steps while the old projector's fov moves "
                  f"{fov1-fov0:+.5f} deg — the ramp reaches one path only")
        else:
            check(f"zoom_{tag}_tracks", worst <= 1e-4,
                  f"the drawn fov equals the old fov at every step (worst |2*halfFov - fov| "
                  f"= {worst:.3e} deg)")


PHASES = dict(frames=phase_frames, turn=phase_turn, diag=phase_diag,
              fov=phase_fov, zoom=phase_zoom)


def main():
    global XKEY
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--pre", action="store_true",
                    help="expect the PRE-FIX behaviour (the mirror absent)")
    ap.add_argument("--phase", default="all")
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    XKEY = str(out / "xkey")
    subprocess.run(["gcc", "-O1", "-o", XKEY, str(HERE / "xkey.c"), "-lX11",
                    "/usr/lib/x86_64-linux-gnu/libXtst.so.6"], check=True)
    print(f"binary   = {a.bin}")
    print(f"mode     = {'PRE-FIX (defect expected)' if a.pre else 'DELIVERED'}")
    wanted = list(PHASES) if a.phase == "all" else a.phase.split(",")
    app = App(a.bin, out, out / "app.log")
    try:
        for name in wanted:
            PHASES[name](app, a.pre)
    finally:
        app.stop()
    print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
