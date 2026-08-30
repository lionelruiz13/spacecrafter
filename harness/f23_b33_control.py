#!/usr/bin/env python3
"""B33 — the CONTROL SURFACE must answer for the path that DRAWS
(INTENT §11.108(f); the F12 template §11.118(f); delivered §11.131).

    cd claude/harness && DISPLAY=:2 ./f23_b33_control.py <out> [--bin B] [--prebin B]

THE INSTRUMENT. Until this wave no member of the class had an observable
channel — `get status position` never replies (§5.47; FIXED 2026-08-02, F27
§11.135 — it answers the connection that asked, in 0.002 s, and `f27_reply.py`
checks its content against this very `control` object. The dump stays the
per-member instrument: `{reported, old, new}` is a comparison one number on a
socket cannot make), the view-offset readout's
one live reader is the TUI, the mount readout has no live reader at all — so
"the getter reports the path that does not draw" could be read at source and not
MEASURED. `body action dual_dump` therefore carries a `control` object:
per member `{reported, old, new}` in the getter's own units. One dump
discriminates: a binary that reads the old authority has `reported == old`
whatever draws; a fixed one has `reported == new` while the new path draws, and
`reported == old` under `flag experimental_path off`.

The two members this script covers have a REAL divergence channel, i.e. one
shipped command sequence splits their two authorities with nothing injected:

  ALTITUDE — `camera action descend` is new-path-only BY DESIGN (the old path's
    free navigation is the anchor-point observatory; there is nothing to
    mirror, `coreLink.hpp`). Two of them at coef 0.5 leave the old observer
    where it was and the camera 4x lower. Every consumer that DERIVES an
    absolute target from the readout — `moveto multiply_alt`/`delta_alt`,
    `moveto` with any absent component, `mode jump ... altitude ±x`, the joypad
    height axis, the TUI location callback — then computes it from a place
    nothing is drawing from and writes it to BOTH paths.
  SKY LOCK — four shipped sites write the OLD flag alone (select-while-tracking
    x2, autoZoomOut x2), so `flag lock_sky_position toggle` read 1 while
    nothing held the sky and wrote 0 to both: a toggle that does nothing in one
    direction. That is §11.129's `flag satellites` defect one layer up.

The two LATENT members (view offset, mount) are in `f23_b33_inject.py`: no
shipped channel splits their authorities, so their divergence is injected.

Legs (each fresh launch; RED legs re-run the same scene on the pre-fix binary):
  A0  agreed state ......... reported == the drawn path's value, and the old
                             authority's own value, which differ only by the
                             camera's float grid (the regression half)
  A1  diverged by descend .. reported == new (pre-fix: == old)
  A2  path pin, twice ...... `flag experimental_path off` flips the answer back
                             to the old authority and on again, entered TWICE,
                             the second time from the state the first exit left
  A3  multiply_alt 1 ....... a SEMANTIC NO-OP: post-fix the drawn observer does
                             not move; pre-fix it is teleported to old's
                             altitude (dump AND composed screen)
  A4  multiply_alt 2 ....... the target derives from the drawn path (both bins)
  S1  select-while-tracking  old lock 1 / camera lock 0, reported == camera
  S2  the toggle ........... post-fix it LOCKS (both paths, camera alt/az then
                             drift under sidereal time); pre-fix it unlocks a
                             lock the drawn path never had, and nothing holds
"""

import json, math, os, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24   # the NaN-tolerant dump reader (I2)
import dumpread              # the dump's NaN grammar lives HERE (I2)

HERE = Path(__file__).resolve().parent
JD = 2461233.5
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def px8(a, b):
    ia = np.asarray(Image.open(a).convert("RGB")).astype(np.int32)
    ib = np.asarray(Image.open(b).convert("RGB")).astype(np.int32)
    return int((np.abs(ia - ib).max(axis=2) > 8).sum())


def lit(p):
    """Lit pixels — the guard that a px8 of 0 means AGREEMENT and not an empty
    frame. The first version of this scene had 28 lit pixels out of 4.2 M and
    every screen leg in it read 0 whatever happened."""
    return int((np.asarray(Image.open(p).convert("RGB")).max(axis=2) > 8).sum())


class App:
    def __init__(self, farm, binary, applog):
        self.dst = b25g.build_farm(farm=farm, dotted=False, corpus=None)
        self.proc = subprocess.Popen(
            [binary], cwd=str(self.dst),
            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
            env={**os.environ, "HOME": str(farm),
                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock, t0 = None, time.time()
        while time.time() - t0 < 90:
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
        self.out = None

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def header(self, tag):
        """The dump header: `control` (what the control surface answers) and
        `camera` (what the drawn path holds), from the SAME frame."""
        p = self.out / f"{tag}.json"
        p.unlink(missing_ok=True)
        self.send(f"body action dual_dump filename {p}", 2.5)
        line = dumpread.sanitize_nonfinite(open(p).readline())
        return json.loads(line)

    def control(self, tag):
        return self.header(tag)["control"]

    def shot(self, tag):
        p = self.out / f"{tag}.png"
        self.send(f"body action screenshot filename {p}", 2.2)
        return p

    def stop(self):
        self.send("shutdown action now", 1)
        self.sock.close()
        try:
            self.proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            self.proc.kill()


# The layers that would mask the body whose limb is the altitude witness, and
# the sky content that has nothing to do with it. `landscape off` is not
# cosmetic: it draws over exactly the part of the frame the limb moves through.
SKY_OFF = ("atmosphere", "fog", "landscape", "milky_way", "nebulae",
           "nebula_names", "constellation_drawing", "constellation_lines",
           "constellation_names", "constellation_art", "cardinal_points",
           "azimuthal_grid", "equatorial_grid", "planet_names", "planet_orbits",
           "planet_trails", "meridian_line", "zenith_line", "ecliptic_line",
           "equator_line")


def altitude_scene(app):
    """b3_ladder's earth site, and the reason it is that one and not a low
    orbit: below distance 2*scaledRadius the parent draws its (empty)
    groundedComponents and therefore draws NOTHING — the recorded
    §11.97(e)/§11.100(g)(ii) regime hole (`b3_ladder.py`'s `earth_surface`
    site exists to measure it). A first version of this scene sat at 200 km
    and every screenshot in it was black, so its screen legs measured a masked
    frame: zero-diff on no content. Both ends of the ladder here are OUTSIDE
    that boundary — distance 46 378 km and 16 378 km against 2R = 12 756 km —
    so the disc is drawn at both, and its angular radius (7.9 deg -> 22.9 deg)
    IS the altitude."""
    app.send("flag experimental_path on", 1)
    app.send("timerate rate 0", 1)
    app.send(f"date jday {JD}", 1)
    app.send("meteors zhr 0", 0.6)
    for f in SKY_OFF:
        app.send(f"flag {f} off")
    app.send("flag stars off")
    app.send("flag moon_scaled off", 1)     # §5.27 precondition, as b3 does
    app.send("set home_planet Earth", 3)


def run_altitude(out, tag, binary):
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    r = {}
    try:
        altitude_scene(app)

        # --- ANCHORED branch of Camera::getPlace(), state level only -------
        # (this altitude is inside the surface-regime hole, so it has no
        # screen witness — that is what the free-mode ladder below is for)
        app.send("moveto lat 0 lon 0 alt 200000 duration 0", 3)
        r["Anch0"] = app.control(f"{tag}_Anch0")
        app.send("camera action descend coef 0.5", 1.5)
        app.send("camera action descend coef 0.5", 1.5)
        r["Anch1"] = app.control(f"{tag}_Anch1")

        # --- the lit ladder: free flight, b3's earth site ------------------
        app.send("camera action free_mode state on", 1.5)
        app.send("select planet Earth pointer off", 1)
        # 40 000 km: descending twice by 0.5 lands at 10 000 km, and BOTH are
        # outside the 2R surface boundary.
        app.send("moveto lat 0 lon 270 alt 40000000 duration 0", 4)
        app.send("flag track_object on", 3)
        app.send("zoom fov 90 duration 0", 2)
        app.send("flag track_object off", 2)   # B30 determinism, as b3 does
        r["A0"] = app.control(f"{tag}_A0")
        r["A0_shot"] = str(app.shot(f"{tag}_A0"))

        # --- A1: the REAL divergence channel, two shipped commands ----------
        app.send("camera action descend coef 0.5", 1.5)
        app.send("camera action descend coef 0.5", 1.5)
        r["A1"] = app.control(f"{tag}_A1")
        r["A1_shot"] = str(app.shot(f"{tag}_A1"))
        r["A1_lit"] = lit(r["A1_shot"])
        r["A0_lit"] = lit(r["A0_shot"])

        # --- A2: the path pin, entered TWICE (reversible pair, second entry
        #         from the state the first exit produced) --------------------
        r["A2"] = []
        for _ in range(2):
            app.send("flag experimental_path off", 2.0)
            r["A2"].append(("off", app.control(f"{tag}_A2_off{len(r['A2'])}")))
            app.send("flag experimental_path on", 2.0)
            r["A2"].append(("on", app.control(f"{tag}_A2_on{len(r['A2'])}")))

        # --- A3: the semantic no-op --------------------------------------
        before = app.shot(f"{tag}_A3_before")
        r["A3_lit"] = lit(before)
        app.send("moveto multiply_alt 1 duration 0", 3)
        after = app.shot(f"{tag}_A3_after")
        r["A3"] = app.control(f"{tag}_A3")
        r["A3_px"] = px8(before, after)
        # The A/A floor of this scene, measured in-scene (§11.80(a)): two
        # screenshots with nothing between them.
        r["A3_floor"] = px8(after, app.shot(f"{tag}_A3_after2"))

        # --- A4: the multiply_alt target itself ---------------------------
        app.send("moveto multiply_alt 2 duration 0", 3)
        r["A4"] = app.control(f"{tag}_A4")
    finally:
        app.stop()
    return r


def run_skylock(out, tag, binary):
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    r = {}
    try:
        app.send("timerate rate 0", 1)
        app.send(f"date jday {JD}", 1)
        for f in ("atmosphere", "fog", "landscape"):
            app.send(f"flag {f} off")
        app.send("set home_planet Earth", 3)
        app.send("moveto lat 43.3 lon 5.36 alt 75 duration 0", 2)
        r["S0"] = app.control(f"{tag}_S0")

        # --- S1: select WHILE TRACKING. `Core::selectObject` sets the OLD
        # flag alone ("keep the earth following") and turns tracking off in the
        # same call, so the two authorities part company with two shipped
        # commands and nothing is left dormant afterwards.
        app.send("select planet Moon pointer off", 1.5)
        app.send("flag track_object on", 4)
        app.send("select planet Mars pointer off", 4)
        r["S1"] = app.control(f"{tag}_S1")

        # --- S2: the consumer. `flag lock_sky_position toggle` READS the
        # getter and writes its negation to both paths.
        app.send("flag lock_sky_position toggle", 3)
        r["S2"] = app.control(f"{tag}_S2")
        h0 = app.header(f"{tag}_S2_h0")
        s0 = app.shot(f"{tag}_S2_t0")
        # A sidereal hour later: a camera that is holding the sky must
        # re-derive alt/az against the advanced placement; one that is not
        # keeps them. Same date jump on both binaries.
        app.send(f"date jday {JD + 1/24.0}", 4)
        h1 = app.header(f"{tag}_S2_h1")
        s1 = app.shot(f"{tag}_S2_t1")
        c0, c1 = h0["camera"], h1["camera"]
        r["S2_drift_deg"] = math.degrees(
            abs(c1["alt"] - c0["alt"]) + abs(c1["az"] - c0["az"]))
        r["S2_screen_px"] = px8(s0, s1)
        r["S2_floor"] = px8(s1, app.shot(f"{tag}_S2_t1b"))
    finally:
        app.stop()
    return r


def run_default(out, tag, binary):
    """THE REGRESSION HALF, at the composed screen. The shipped scene, nothing
    commanded: every layer the old path draws in the observer frame is ON, and
    the only thing this wave can change here is the place the OLD observer ends
    up at. It DOES change it: `UI::init` re-applies the observer place through
    the dual seam (`ui.cpp`, the "initial.sts commands" block), so post-fix it
    writes the DRAWN place to both authorities instead of writing old's to
    both — which puts the old observer ON the camera's float grid (0.104 m of
    altitude and 1.3e-6 deg of latitude at the shipped Marseille place)
    instead of that far away from it. The two paths therefore AGREE at startup
    where before they did not; this leg measures what that costs on screen."""
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    try:
        app.send("timerate rate 0", 1)
        app.send(f"date jday {JD}", 2)
        c = app.control(f"{tag}_D0")
        a = app.shot(f"{tag}_D0")
        b = app.shot(f"{tag}_D1")     # same launch, nothing between: the floor
    finally:
        app.stop()
    return {"control": c, "shot": str(a), "floor": px8(a, b), "lit": lit(a)}


def close(a, b, tol):
    return abs(a - b) <= tol


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = Path(args[0]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    binary, prebin = DEFAULT_BIN, None
    for a in sys.argv[1:]:
        if a.startswith("--bin="):
            binary = a.split("=", 1)[1]
        if a.startswith("--prebin="):
            prebin = a.split("=", 1)[1]

    src_md5 = b25g.real_tree_md5()
    rep = {"alt_post": run_altitude(out, "alt_post", binary),
           "lock_post": run_skylock(out, "lock_post", binary),
           "def_post": run_default(out, "def_post", binary)}
    if prebin:
        rep["alt_pre"] = run_altitude(out, "alt_pre", prebin)
        rep["lock_pre"] = run_skylock(out, "lock_pre", prebin)
        rep["def_pre"] = run_default(out, "def_pre", prebin)

    P, L = rep["alt_post"], rep["lock_post"]

    # ---------------- the ANCHORED branch of getPlace() --------------------
    n0, n1 = P["Anch0"]["altitude"], P["Anch1"]["altitude"]
    if close(n0["reported"], n0["new"], 1e-9) and close(n1["reported"], n1["new"], 1e-9) \
            and n1["old"] / max(n1["new"], 1e-9) > 3:
        ok(f"ANCHORED branch: agreed {n0['reported']:.3f} m (old "
           f"{n0['old']:.3f}); after two descends the old authority holds "
           f"{n1['old']:.3f} m and the readout follows the drawn "
           f"{n1['new']:.3f} m")
    else:
        fail(f"ANCHORED branch: {json.dumps(n0)} / {json.dumps(n1)}")

    # ---------------- the regression half ---------------------------------
    a0 = P["A0"]["altitude"]
    q = abs(a0["new"] - a0["old"])
    if close(a0["reported"], a0["new"], 1e-9):
        ok(f"A0 agreed state: reported {a0['reported']:.6f} m == the drawn "
           f"path's {a0['new']:.6f} m; the old authority says {a0['old']:.6f} m, "
           f"{q:.6f} m away — the camera's float grid, which is the resolution "
           f"the drawn observer actually has")
    else:
        fail(f"A0: reported {a0['reported']} != drawn {a0['new']}")
    lat0 = P["A0"]["latitude"]
    if close(lat0["reported"], lat0["new"], 1e-12):
        ok(f"A0 latitude: reported == drawn ({lat0['new']:.9f} deg, old "
           f"{lat0['old']:.9f})")
    else:
        fail(f"A0 latitude: reported {lat0['reported']} != drawn {lat0['new']}")

    # ---------------- the diverged half -----------------------------------
    a1 = P["A1"]["altitude"]
    ratio = a1["old"] / a1["new"] if a1["new"] else float("inf")
    if P["A0_lit"] < 20000 or P["A1_lit"] < 20000:
        fail(f"the ladder's frames are EMPTY ({P['A0_lit']} / {P['A1_lit']} lit "
             f"px) — every screen leg below would be a zero-diff on no content")
    else:
        ok(f"the ladder is LIT and altitude-sensitive: the drawn disc covers "
           f"{P['A0_lit']} px at the top of it and {P['A1_lit']} px at the "
           f"bottom ({P['A1_lit'] / max(P['A0_lit'], 1):.2f}x)")
    if ratio > 3:
        ok(f"the scene DOES diverge with shipped commands only: two "
           f"`camera action descend coef 0.5` leave the old observer at "
           f"{a1['old']:.3f} m and the drawn one at {a1['new']:.3f} m "
           f"({ratio:.2f}x)")
    else:
        fail(f"A1: the two authorities did not part ({a1['old']} vs {a1['new']}) "
             f"— every leg below proves nothing")
    if close(a1["reported"], a1["new"], 1e-9):
        ok(f"A1: the readout follows the path that draws — {a1['reported']:.3f} m")
    else:
        fail(f"A1: reported {a1['reported']} != drawn {a1['new']}")

    # ---------------- the pin, twice --------------------------------------
    bad = []
    for i, (pin, c) in enumerate(P["A2"]):
        a = c["altitude"]
        want = a["old"] if pin == "off" else a["new"]
        if not close(a["reported"], want, 1e-9):
            bad.append(f"entry {i} pin={pin}: reported {a['reported']} != {want}")
    if not bad:
        ok(f"A2: the pin is a QUESTION, not a preference — `flag "
           f"experimental_path off` returns the old authority "
           f"({P['A2'][0][1]['altitude']['reported']:.3f} m) and `on` the drawn "
           f"one ({P['A2'][1][1]['altitude']['reported']:.3f} m), on BOTH "
           f"entries of the pair, the second from the state the first left")
    else:
        fail("A2: " + "; ".join(bad))

    # ---------------- the semantic no-op ----------------------------------
    a3 = P["A3"]["altitude"]
    pre3 = rep.get("alt_pre", {}).get("A3", {}).get("altitude")
    if close(a3["new"], a1["new"], 0.002 * a1["new"]) \
            and P["A3_px"] <= max(3 * P["A3_floor"], 0.01 * P["A3_lit"]):
        ok(f"A3 NO VISUAL JUMP: `moveto multiply_alt 1` — a semantic no-op — "
           f"leaves the drawn observer at {a3['new']:.3f} m (was "
           f"{a1['new']:.3f}) and moves the composed screen {P['A3_px']} px>8 "
           f"against an in-scene floor of {P['A3_floor']}")
    else:
        fail(f"A3: the no-op moved the drawn observer {a1['new']:.3f} -> "
             f"{a3['new']:.3f} m and the screen {P['A3_px']} px>8 "
             f"(floor {P['A3_floor']})")

    # ---------------- the target itself -----------------------------------
    a4 = P["A4"]["altitude"]
    pre4 = rep.get("alt_pre", {}).get("A4", {}).get("altitude")
    if close(a4["new"], 2 * a3["new"], max(0.02 * a3["new"], 1.0)):
        ok(f"A4: `moveto multiply_alt 2` builds its ABSOLUTE target from the "
           f"drawn altitude — {a3['new']:.3f} -> {a4['new']:.3f} m"
           + (f" (pre-fix binary, same scene: {pre4['new']:.3f} m, built from "
              f"old's {pre3['new'] if pre3 else '?'})" if pre4 else ""))
    else:
        fail(f"A4: target {a4['new']:.3f} m is not 2x the drawn "
             f"{a3['new']:.3f} m")

    # ---------------- sky lock --------------------------------------------
    s1 = L["S1"]["skyLock"]
    if s1["old"] and not s1["new"]:
        ok(f"S1: select-while-tracking splits the two authorities with two "
           f"shipped commands — old lock {s1['old']}, drawn lock {s1['new']}")
    else:
        fail(f"S1: the authorities did not part (old={s1['old']} new={s1['new']})")
    if s1["reported"] == s1["new"]:
        ok(f"S1: the readout reports the drawn path ({s1['reported']})")
    else:
        fail(f"S1: reported {s1['reported']} != drawn {s1['new']}")
    s2 = L["S2"]["skyLock"]
    if s2["new"] and s2["old"]:
        ok(f"S2: and the consumer works — `flag lock_sky_position toggle` "
           f"LOCKS the sky on both paths (old={s2['old']}, drawn={s2['new']}), "
           f"because it negated the value the drawn path actually had")
    else:
        fail(f"S2: after the toggle old={s2['old']} drawn={s2['new']} — the "
             f"toggle did not lock the drawn path")
    if L["S2_drift_deg"] > 1.0:
        ok(f"S2 CONSEQUENCE: a sidereal hour later the locked camera has "
           f"re-derived its params by {L['S2_drift_deg']:.4f} deg to hold the "
           f"same sky, and the composed screen moved {L['S2_screen_px']} px>8 "
           f"(floor {L['S2_floor']})")
    else:
        fail(f"S2: the camera drifted {L['S2_drift_deg']:.6f} deg over a "
             f"sidereal hour — nothing is holding the sky")

    # ---------------- the RED controls ------------------------------------
    if prebin:
        Q, M = rep["alt_pre"], rep["lock_pre"]
        q1 = Q["A1"]["altitude"]
        if close(q1["reported"], q1["old"], 1e-9) and not close(q1["reported"], q1["new"], 1.0):
            ok(f"RED: the pre-fix binary, same scene, reports {q1['reported']:.3f} m "
               f"while DRAWING from {q1['new']:.3f} m — the defect")
        else:
            fail(f"RED: the pre-fix binary already agreed "
                 f"({q1['reported']} vs new {q1['new']}) — this leg cannot fail")
        if Q["A3_px"] > 10 * max(P["A3_px"], 1):
            ok(f"RED: and on it the SAME no-op command teleports the drawn "
               f"observer {q1['new']:.3f} -> {Q['A3']['altitude']['new']:.3f} m, "
               f"moving the composed screen {Q['A3_px']} px>8 against the fixed "
               f"binary's {P['A3_px']} (its own floor {Q['A3_floor']})")
        else:
            fail(f"RED: the no-op moved the pre-fix screen only {Q['A3_px']} px>8 "
                 f"vs {P['A3_px']} — not discriminating")
        m1 = M["S1"]["skyLock"]
        if m1["reported"] == m1["old"] and m1["old"] != m1["new"]:
            ok(f"RED: the pre-fix binary reports the old lock ({m1['reported']}) "
               f"while the drawn path holds {m1['new']}")
        else:
            fail(f"RED: pre-fix skyLock reported {m1['reported']}, old "
                 f"{m1['old']}, new {m1['new']}")
        m2 = M["S2"]["skyLock"]
        if not m2["new"]:
            ok(f"RED: and its toggle turns OFF a lock the drawn path never had "
               f"— after `flag lock_sky_position toggle` old={m2['old']}, "
               f"drawn={m2['new']}, and the sky rotates freely: camera drift "
               f"{M['S2_drift_deg']:.4f} deg against the fixed binary's "
               f"{L['S2_drift_deg']:.4f}")
        else:
            fail(f"RED: the pre-fix toggle locked the drawn path "
                 f"(new={m2['new']}) — not discriminating")

        # --- the regression half at the composed screen -------------------
        D, E = rep["def_post"], rep["def_pre"]
        cross = px8(D["shot"], E["shot"])
        dp, de = D["control"]["altitude"], E["control"]["altitude"]
        if D["lit"] < 100000:
            fail(f"the default scene has only {D['lit']} lit px — the cross-binary "
                 f"screen comparison below would be a zero-diff on no content")
        elif cross <= max(3 * max(D["floor"], E["floor"]), 200):
            ok(f"REGRESSION HALF at the composed screen: the shipped scene with "
               f"every old-path layer ON, {D['lit']} lit px, pre-fix vs delivered "
               f"= {cross} px>8 against in-run floors of {D['floor']}/{E['floor']} "
               f"— and the old observer's own altitude moved "
               f"{abs(dp['old'] - de['old']):.6f} m (pre-fix {de['old']:.6f} -> "
               f"delivered {dp['old']:.6f}), because `UI::init` now re-applies the "
               f"DRAWN place through the dual seam and so lands the two "
               f"authorities on the same value instead of "
               f"{abs(de['new'] - de['old']):.6f} m apart")
        else:
            fail(f"REGRESSION HALF: the default scene moved {cross} px>8 pre vs "
                 f"post (floors {D['floor']}/{E['floor']})")

    if b25g.real_tree_md5() != src_md5:
        fail("the real ~/.spacecrafter tree was written by this run")
    else:
        ok("real tree md5 in == out")

    (out / "f23_b33.json").write_text(json.dumps(rep, indent=1, default=str))
    print(f"\n{'OK' if not FAILS else str(len(FAILS)) + ' FAILS'} -> "
          f"{out}/f23_b33.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
