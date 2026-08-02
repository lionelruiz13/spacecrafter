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

`Core::dragView`, the other caller of `Camera::lookRel`, is NOT here: the drag
channel is undrivable on this host (XTEST pointer motion does not move the
pointer — measured by `xdrag.c`'s own step report, root 0x0, `XQueryPointer` at
(0,0) after every fake motion while Button1Mask is held), so it is verified one
layer below SDL by `f25_drag.py` on §11.108(d)'s gdb-FIFO precedent.

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
#  * pole clamp: old pins the view altitude at the DOUBLE pi/2 - 1e-6 =
#    1.5707953267948966; the camera computes the same expression in float32,
#    float(float(pi/2) - float(1e-6)) = 1.5707954168319702. The two pins are
#    therefore 9.0037e-8 rad = 5.16e-6 deg apart BY CONSTRUCTION, and that is
#    the whole residual there. Computed exactly below, not estimated.
#  * a CLAMP row is not a step: its size is "whatever reaches the pin", so it
#    absorbs whatever float32 divergence the hold accumulated before it. Those
#    rows are excluded from the per-step comparison and checked by the pin
#    criterion instead - the accumulated divergence is REPORTED there, since
#    that is the one place the instrument can see it directly.
STEP_BAR = 1e-6          # rad, per step, view-space
CUM_BAR = 5e-5           # rad, over one hold
POLE_PIN_OLD = math.pi / 2 - 1e-6
POLE_PIN_NEW = float(np.float32(np.float32(math.pi / 2) - np.float32(1e-6)))
POLE_PIN_GAP = POLE_PIN_NEW - POLE_PIN_OLD          # 9.0037e-8 rad, derived
POLE_BAR = 2 * POLE_PIN_GAP
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
    if n > ramp["capacity"]:
        note(f"WARNING: this hold wrote {n} rows into a {ramp['capacity']} ring — truncated")
        n = ramp["capacity"]
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
    worst_clamp = 0.0
    for s in act:
        daz_o, dalt_o, daz_n, dalt_n = view_deltas(s)
        cum[0] += daz_o; cum[1] += dalt_o; cum[2] += daz_n; cum[3] += dalt_n
        # the ramp law itself, on each path independently
        worst_law_old = max(worst_law_old, abs(daz_o - (-s["dAz"])))
        if not pre:
            worst_law_new = max(worst_law_new, abs(daz_n - (-s["dAz"])))
        # A CLAMP row is not a step: old does not add deltaAlt there, it PINS.
        # Its size is therefore "whatever reaches the pin" and it absorbs the
        # float32 divergence the hold accumulated before it, so it belongs to
        # the pin check (below, in the caller) and not to the per-step one.
        clamped = abs(dalt_o - s["dAlt"]) > STEP_BAR
        if clamped:
            clamp_rows += 1
            worst_clamp = max(worst_clamp, abs(dalt_o - dalt_n))
            if not clamped_ok:
                worst_law_old = max(worst_law_old, abs(dalt_o - s["dAlt"]))
            continue
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
              f"alt {worst_alt:.3e} rad over {n - clamp_rows} un-clamped rows "
              f"(bar {STEP_BAR:.0e})"
              + (f"; the {clamp_rows} clamp row(s) differ by up to {worst_clamp:.3e} rad "
                 f"because a clamp is not a step — see the pin check" if clamp_rows else ""))
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
    # STARS OFF, and it is the whole reason the composed-screen leg can attribute
    # a camera at all: the star field is drawn by the OLD pipeline in BOTH
    # phases, so with it on the lit content moves with the old navigator whatever
    # `experimental_path` is pinned to — F4's recorded confound (its leg-1 delta
    # "says nothing about the camera"), and measured again here at 1861 px>32 of
    # 2217 lit on a PRE-FIX binary whose camera never moved a bit.
    app.send("flag stars off")
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
    alt_diffs = [r[4] - r[2] for r in rows]
    az_sums = [wrap_pi(r[3] + r[1]) for r in rows]
    az_diffs = [wrap_pi(r[3] - r[1]) for r in rows]
    worst_alt = max(abs(a) for a in alt_sums)
    # DISCRIMINATION, not an absolute bar: the residual here is the AIM residual
    # (two independent aiming laws settling on the same body, and the two trees'
    # positions for it are not bit-equal), so the question a bar cannot answer is
    # "+ or −". The competing hypothesis is measured beside the accepted one: if
    # the mapping were the other sign, the SUM would vary and the DIFFERENCE
    # would be constant. Both spreads are reported and the ratio is the finding.
    sp_alt_sum = max(alt_sums) - min(alt_sums)
    sp_alt_diff = max(alt_diffs) - min(alt_diffs)
    sp_az_sum = max(az_sums) - min(az_sums)
    sp_az_diff = max(az_diffs) - min(az_diffs)
    note(f"alt: spread of (cam+old) = {sp_alt_sum:.3e} rad vs (cam−old) = {sp_alt_diff:.3e} rad")
    note(f"az : spread of (cam+old) = {sp_az_sum:.3e} rad vs (cam−old) = {sp_az_diff:.3e} rad")
    check("frames_alt_is_negated", worst_alt <= FRAME_BAR and sp_alt_diff > 100 * sp_alt_sum,
          f"alt_cam + altVision_old == 0 on {len(rows)} bodies: worst {worst_alt:.3e} rad "
          f"({math.degrees(worst_alt):.5f} deg, bar {FRAME_BAR:.0e}); the opposite mapping "
          f"would spread by {sp_alt_diff:.3e} rad, i.e. {sp_alt_diff/max(sp_alt_sum,1e-12):.0f}× "
          f"more ⇒ the camera's `alt` is the NEGATIVE of the view altitude, so old's "
          f"+deltaAlt maps to −deltaAlt here")
    check("frames_az_offset_constant", sp_az_diff > 100 * sp_az_sum,
          f"az_cam + azVision_old is CONSTANT across {len(rows)} bodies "
          f"(mean {math.degrees(sum(az_sums)/len(az_sums)):+.5f} deg, spread {sp_az_sum:.3e} rad) "
          f"while az_cam − azVision_old spreads {sp_az_diff:.3e} rad "
          f"({sp_az_diff/max(sp_az_sum,1e-12):.0f}× more) ⇒ the two azimuth origins differ by a "
          f"FIXED rotation about the mount pole, and azimuth DIFFERENCES therefore map with a "
          f"+ sign onto `az`")
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

    # --- the row's own hold (Left, 2500 ms), for the per-step trace ----------
    h0, bod0 = app.dump("turn_before_cam")
    total = h0["ramp"]["total"]
    hold("Left", 2500)
    time.sleep(1.5)
    h1, bod1 = app.dump("turn_after_cam")
    rows, total = new_run(h1, total)
    analyse(rows, "left", pre, expect_az=+1, expect_alt=0)
    n0 = bod0["Moon"]["new"]["screen"]; n1 = bod1["Moon"]["new"]["screen"]
    o0 = bod0["Moon"]["old"]["screen"]; o1 = bod1["Moon"]["old"]["screen"]
    dn = math.hypot(n1[0] - n0[0], n1[1] - n0[1])
    do = math.hypot(o1[0] - o0[0], o1[1] - o0[1])
    note(f"the SAME body in the SAME dump: NEW |d| {dn:.4e} NDC, OLD |d| {do:.2f} px")
    if pre:
        check("left_per_path_screen", dn == 0.0 and do > 100.0,
              f"pre-fix: the NEW path's screen position is bit-identical across the hold "
              f"({dn:.3e} NDC) while the OLD path's moves {do:.1f} px — F4's asymmetry, reproduced")
    else:
        check("left_per_path_screen", dn > 0.05 and do > 100.0,
              f"delivered: BOTH paths moved — NEW |d| {dn:.4f} NDC, OLD |d| {do:.1f} px")

    # --- THE COMPOSED SCREEN, and it has to be attributable -----------------
    # fov 10 so the disc is large enough to be the frame's content, and a 1200 ms
    # hold so both positions stay inside the dome (2.3 deg of a 5 deg dome radius).
    # The comparator is checked before it is used: with the sky off the two phases
    # must draw the SAME frame at rest, or a cross-path delta after the hold is
    # not attributable to a camera.
    aim_at(app, "Moon", fov=10)
    b_new = app.shot("scr_before_new")
    app.send("flag experimental_path off", 2)
    b_old = app.shot("scr_before_old")
    app.send("flag experimental_path on", 2)
    h, _ = app.dump("scr_before")
    total = h["ramp"]["total"]
    hold("Left", 1200)
    time.sleep(1.2)
    h, _ = app.dump("scr_after")
    rows, total = new_run(h, total)
    a_new = app.shot("scr_after_new")
    app.send("flag experimental_path off", 2)
    a_old = app.shot("scr_after_old")
    app.send("flag experimental_path on", 2)
    analyse(rows, "screen", pre, expect_az=+1, expect_alt=0)
    lit_new, lit_old = lit(b_new), lit(b_old)
    old_px, new_px = px32(b_old, a_old), px32(b_new, a_new)
    cross0, cross1 = px32(b_new, b_old), px32(a_new, a_old)
    note(f"composed screen (fov 10, stars off): lit {lit_new} new-phase / {lit_old} old-phase ; "
         f"across the hold OLD {old_px} px>32, NEW {new_px} px>32 ; "
         f"cross-path {cross0} px>32 before, {cross1} after")
    check("screen_frame_carries_content", lit_new > 500 and lit_old > 500,
          f"both phases draw a real frame ({lit_new}/{lit_old} lit px — the Moon at this date "
          f"is a CRESCENT, so the lit set is a fraction of the ~114-px disc) — the F23 "
          f"black-frame guard: a px>32 of 0 on an empty frame is agreement about nothing")
    check("screen_positive_control", old_px > 0.5 * lit_old,
          f"the OLD phase moved {old_px} px>32 against its own {lit_old} lit px — the key "
          f"demonstrably arrived (if ~0 every null below is void)")
    # ATTRIBUTION comes from the BOTH-WAYS pair and not from a cross-path bar:
    # the cross-path residual at rest is §11.52(b)'s perceptual parity (here the
    # crescent's terminator, ~25 % of a thin lit set) and is not this row's claim.
    # What makes `new_px` a camera observable is that the SAME scene on the
    # pre-fix binary leaves it at ~0 while the old phase moves.
    if pre:
        check("screen_new_still", new_px < 0.1 * lit_new,
              f"pre-fix: the DRAWN frame does not move ({new_px} px>32 of {lit_new} lit) while "
              f"the old phase's moves {old_px} — the operator turned the universe nobody is "
              f"looking at")
        check("screen_paths_diverge", cross1 > 0.5 * lit_new,
              f"pre-fix: after the hold the two phases DISAGREE ({cross1} px>32 against "
              f"{cross0} at rest) — the divergence the key opened up, on the composed screen")
    else:
        check("screen_new_moves", new_px > 0.5 * lit_new,
              f"delivered: the DRAWN frame moved {new_px} px>32 against its own {lit_new} lit "
              f"px — the row's own px>32 bar, taken on the frame that draws")
        check("screen_paths_stay_together", cross1 < 2.0 * cross0,
              f"delivered: after the hold the two phases still agree as well as they did at "
              f"rest ({cross1} px>32 against {cross0}) — both universes turned together, and "
              f"the residual is the pre-existing cross-path one (§11.52(b)), not a new one")

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
        pinned = abs(abs(old_view_alt) - POLE_PIN_OLD) < 1e-9
        check("pole_old_pinned", pinned,
              f"the OLD path pinned at pi/2 - 1e-6 = {POLE_PIN_OLD:.13f} "
              f"(measured {abs(old_view_alt):.13f})")
        # the clamp row's own size: where the accumulated float32 divergence of
        # the hold becomes visible, because the clamp absorbs it in one row
        trans = [s for s in act
                 if abs((s["oldAltAfter"] - s["oldAlt"]) - s["dAlt"]) > STEP_BAR][:1]
        if trans:
            t = trans[0]
            pre_gap = (-t["newAlt"]) - t["oldAlt"]
            note(f"at the clamp transition the two paths' view altitudes were "
                 f"{pre_gap:+.3e} rad apart — the float32 divergence accumulated over the "
                 f"hold (predicted <= 2.2e-5 worst case), which the clamp then absorbs")
        if pre:
            check("pole_PRE_new_frozen", all(s["newAltAfter"] == s["newAlt"] for s in rows),
                  "pre-fix: the camera's altitude never moved, so it never reached a clamp")
        else:
            gap = abs(new_view_alt) - abs(old_view_alt)
            check("pole_new_pinned", abs(gap - POLE_PIN_GAP) <= 1e-10,
                  f"BOTH paths pinned, and the gap is the PREDICTED one: measured "
                  f"{gap:.6e} rad against {POLE_PIN_GAP:.6e} predicted exactly — old pins at "
                  f"the double pi/2-1e-6 and the camera at float(float(pi/2)-float(1e-6)); "
                  f"{math.degrees(POLE_PIN_GAP):.2e} deg, i.e. {math.degrees(POLE_PIN_GAP)/90*1024:.1e} px "
                  f"on a 1024-px-radius 180 deg dome")
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
