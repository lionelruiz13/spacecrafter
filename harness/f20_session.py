#!/usr/bin/env python3
"""B31 slice 3 gate: the session file (b31-design §3.2/§6.2/§6.3, INTENT §11.128).

The checks the design named, each with what it can catch that the others cannot.

  T2  DUMP, FIELD BY FIELD. `Camera::dumpTrace` is the instrument §6.2 names, and
      this task extended it with the two gaps §6.2 itself lists — the held
      sky-lock matrix and the in-flight plans — plus the rest of §2 group A in
      the dump header (date, rate, pause).  A scene is built by commands, saved,
      the app is QUIT, a fresh one is launched and the session restored: every
      field the session carries must come back.
  T2-CONTROL  the same fresh launch is dumped BEFORE the restore.  Without it,
      T2 proves nothing: a field that happens to match the default would pass.
  T1  SCREEN, the terminal observable.  The restored scene is shot and compared
      against the saved one, against an A/A floor measured IN THIS SCENE by a
      second launch of the SAME binary that rebuilds it by commands (§11.80(a):
      a floor is measured in-scene, never inherited).
  T4  FIXED POINT, traversed TWICE.  save -> restore -> save -> restore, the
      second entry starting from the state the first exit produced: the dumps
      must agree at both exits AND the second file must be byte-identical to the
      first.  A save that is not a fixed point is not a save.
  T5a DISCRIMINATION.  Restoring a session saved from a DIFFERENT scene must
      move the dump.
  T5b DISCRIMINATION, one key at a time.  Exactly one key is edited in the file
      by hand and exactly the corresponding dump field must move — and nothing
      else.  This is the leg that shows the restore is reading the file rather
      than reproducing a scene by luck.
  T10 THE D8 USE-SITE (§6.1).  A body is FROZEN (hidden — the B39/D23 mechanism,
      and a hidden body is a legal observer reference, §11.113(b)(vi)), jd is
      advanced while it is frozen, and the session is saved and restored.  Its
      position must equal the same scene where it was never frozen.  A
      divergence proportional to the freeze duration is the failure signature.
  §6.3 THE D9 SWEEP.  Across the save command itself, the md5 of every file
      under ~/.spacecrafter except the session must be unchanged.

Protocol: this script owns the app lifecycle and removes the sessions it wrote.
No other spacecrafter process may run (§11.121(m)).

    cd claude/harness && DISPLAY=:2 ./f20_session.py [outdir] [--mutate]

`--mutate` breaks the SAVED file between the save and the relaunch (one value
changed): that run is EXPECTED to fail, on the corresponding field and nowhere
else.  Exit 0 = every leg green; 1 = any failure, each named on stdout.
"""

import hashlib
import json
import math
import os
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

import numpy as np
from PIL import Image

HOME = Path.home()
USERDIR = HOME / ".spacecrafter"
SESSIONS = USERDIR / "sessions"
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))

MUTATE = "--mutate" in sys.argv[1:]
_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f20sess").resolve()
OUT.mkdir(parents=True, exist_ok=True)

FROZEN = ["config.ini", "ssystem.ini", "galactic.ini", "anchor.ini"]
JD = 2461233.5
FAILS = []

# The fields the session carries, and therefore the fields T2 asserts. Split by
# what equality means for each: a structural field is exact, a float field rides
# the cross-launch floor B30 puts under every one of them (§11.53(e)).
EXACT = ["reference", "tracked", "freeMode", "boundToSurface", "mount", "skyLocked",
         "selected"]
FLOAT = ["longitude", "latitude", "distance", "alt", "az", "halfFov", "viewOffset",
         "viewOffsetTransition", "viewOffsetEff"]
VEC = ["position", "lockedSkyRot"]
PLANS = ["viewT", "hdgT", "zoomDuration", "moveDuration"]
# THE OLD PATH'S OWN VIEW STATE (INTENT §5.63 / §11.130), read off the same dump.
# Derived from the MECHANISM, not from §5.63's symptom: the star field, the milky
# way and the nebulae are drawn from `Navigator`, which holds its own view
# direction and its own copy of the view offset. A restore that reproduces every
# camera field and none of these puts the right body in front of the wrong sky —
# which is precisely what it did (107.634 deg apart, 392 stars drawn against
# 689), invisible to every camera-side check. Vectors are compared by DIRECTION
# and by norm, both of which the old path can and does change independently.
OLDNAV_VEC = ["localVision", "equVision", "precEquVision"]
OLDNAV_FLOAT = ["viewOffset", "viewOffsetTransition", "heading"]


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def tree_md5():
    out = {}
    for p in sorted(USERDIR.rglob("*")):
        if not p.is_file():
            continue
        rel = p.relative_to(USERDIR).as_posix()
        if rel.startswith("log/") or rel.endswith(".tmp"):
            continue
        out[rel] = md5(p)
    return out


def assert_no_other_instance():
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
        self.log = OUT / f"f20s_{tag}.applog"
        self.proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                     stdout=open(self.log, "w"), stderr=subprocess.STDOUT,
                                     env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock = wait_port()
        time.sleep(10)

    def cmd(self, c, pause=0.7):
        self.sock.sendall((c + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def dump(self, name, pause=2.0):
        p = OUT / f"f20s_{name}.json"
        p.unlink(missing_ok=True)
        self.cmd(f"body action dual_dump filename {p}", pause)
        if not p.exists():
            raise RuntimeError(f"dump {p} not written")
        return read_dump(p)

    def shot(self, name, pause=2.5):
        p = OUT / f"f20s_{name}.png"
        p.unlink(missing_ok=True)
        self.cmd(f"body action screenshot filename {p}", pause)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.3)
        return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)

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


def read_dump(path):
    """-> {'hdr': header, 'cam': camera, 'bodies': {name: new-path record}}"""
    hdr, cam, bodies, oldnav = None, None, {}, None
    with open(path) as f:
        for ln in f:
            ln = _clean(ln)
            if not ln:
                continue
            o = json.loads(ln)
            if o.get("type") == "header":
                hdr, cam = o, o["camera"]
                oldnav = o.get("oldView", {}).get("nav")
            elif o.get("type") == "body" and o.get("new"):
                bodies[o["name"]] = o["new"]
    return {"hdr": hdr, "cam": cam, "bodies": bodies, "oldnav": oldnav}


def close(a, b, tol=1e-5):
    if a is None or b is None:
        return a == b
    if isinstance(a, list):
        return len(a) == len(b) and all(close(x, y, tol) for x, y in zip(a, b))
    if isinstance(a, bool) or isinstance(b, bool) or isinstance(a, str):
        return a == b
    if a == b:
        return True
    scale = max(abs(a), abs(b), 1e-9)
    return abs(a - b) / scale <= tol


def compare(d0, d1, tag, tol=1e-5):
    """Every field the session carries. Returns the list of names that moved."""
    moved = []
    c0, c1 = d0["cam"], d1["cam"]
    for f in EXACT:
        if c0.get(f) != c1.get(f):
            moved.append(f)
    for f in FLOAT + VEC:
        if not close(c0.get(f), c1.get(f), tol):
            moved.append(f)
    for f in PLANS:
        if not close(c0["plans"].get(f), c1["plans"].get(f), tol):
            moved.append("plans." + f)
    o0, o1 = d0.get("oldnav"), d1.get("oldnav")
    if o0 and o1:
        for f in OLDNAV_FLOAT:
            if not close(o0.get(f), o1.get(f), tol):
                moved.append("oldnav." + f)
        for f in OLDNAV_VEC:
            if not close(o0.get(f), o1.get(f), tol):
                moved.append("oldnav." + f)
    elif o0 is None and o1 is None:
        pass
    else:
        moved.append("oldnav.<present on one side only>")
    # jd gets an ABSOLUTE tolerance: a relative one on a 2.46e6 magnitude is
    # +-246 days at 1e-4, i.e. no check at all. 1e-9 d = 86 us.
    if abs(d0["hdr"].get("jd", 0) - d1["hdr"].get("jd", 0)) > 1e-9:
        moved.append("hdr.jd")
    for f in ("timeSpeed", "timePaused"):
        if not close(d0["hdr"].get(f), d1["hdr"].get(f), tol):
            moved.append("hdr." + f)
    if moved:
        for f in moved:
            if f.startswith("oldnav."):
                src, dst = d0["oldnav"], d1["oldnav"]
            elif f.startswith("hdr."):
                src, dst = d0["hdr"], d1["hdr"]
            elif f in EXACT + FLOAT + VEC:
                src, dst = c0, c1
            else:
                src, dst = c0["plans"], c1["plans"]
            k = f.split(".")[-1]
            print(f"      {tag}: {f} {src.get(k)} -> {dst.get(k)}", flush=True)
    return moved


def px_diff(a, b, thr=8):
    if a.shape != b.shape:
        return -1
    return int((np.abs(a - b).max(axis=2) > thr).sum())


# ---------------------------------------------------------------------------
# The scene. Every command is one of the §2 rows the slice carries, and the
# scene is built the SAME way on both sides of the A/A pair, so a difference
# between them is the launch and nothing else.
# ---------------------------------------------------------------------------
# THE SCENES, and why they are three rather than one.
#
# The first run of this gate built ONE scene carrying every row at once -
# tracking AND a sky lock AND a selection - and three legs failed on the same
# thing: under tracking, and under a live sky lock, alt/az/heading and
# lockedSkyRot are DERIVED every frame (Camera::update: tracking re-aims,
# `recoverParams(lockedSkyRot)` re-derives, and the lock is dormant while
# something is tracked - the old precedence auto_move > tracking > lock). A
# restored scene converges to the same values, which is correct, but a leg that
# needs "exactly one field moves" cannot be run on parameters the engine
# derives from each other. So: one scene per question.
def build_scene_a(app):
    """T1/T2/T4: a sky-LOCKED, untracked scene. The lock is live here (nothing
    is tracked), so it holds the view - which is also what makes the screen leg
    possible in spite of the `heading` carve-out: under a live lock the heading
    is re-derived from the HELD matrix, which the session does carry."""
    app.cmd("timerate rate 0", 1.0)
    app.cmd(f"date jday {JD}", 1.5)
    app.cmd("set home_planet Mars", 3.0)          # B1 reference
    app.cmd("select planet Mars", 1.0)            # C1 selection
    app.cmd("moveto lat 12 lon 34 alt 500000 duration 0", 2.5)  # B3
    app.cmd("zoom fov 45 duration 0", 2.0)        # B11 half_fov
    # B10, and it took two tasks to get here. F21 found the line inert -
    # `view_offset` is NOT a registered `set` name and the app rejects it
    # ("Did you mean zoom_offset ?", INTENT §11.129(b)) - and left it, because
    # changing the scene would have moved the §5.63 baseline it was hunting.
    # §5.63 is attributed and closed (§11.130), so the line is repointed at the
    # registered §2(c) channel and row B10 is exercised for the first time.
    app.cmd("set zoom_offset 0.25", 1.5)          # B10 offset
    app.cmd("flag lock_sky_position on", 2.0)     # B9 lock + its held matrix


def build_scene_b(app):
    """T5a + the TRACKED round trip (B2/C5), which scene A cannot carry: a lock
    and a tracked body cannot both be live."""
    app.cmd("timerate rate 0", 1.0)
    app.cmd("flag lock_sky_position off", 1.5)   # a lock and a tracked body
    app.cmd(f"date jday {JD + 40}", 1.5)         # cannot both be live
    app.cmd("set home_planet Earth", 3.0)
    app.cmd("select planet Moon", 1.0)
    app.cmd("flag track_object on", 1.5)
    app.cmd("moveto lat -20 lon 100 alt 900000 duration 0", 2.5)
    app.cmd("zoom fov 120 duration 0", 2.0)


def build_scene_c(app):
    """T5b: nothing derives anything. No tracking, no lock - so every parameter
    the file carries is independent and 'exactly one field moves' is a
    statement about the restore rather than about the scene."""
    app.cmd("timerate rate 0", 1.0)
    app.cmd("flag lock_sky_position off", 1.5)
    app.cmd("flag track_object off", 1.5)
    app.cmd(f"date jday {JD}", 1.5)
    app.cmd("set home_planet Mars", 3.0)
    app.cmd("moveto lat 12 lon 34 alt 500000 duration 0", 2.5)
    app.cmd("zoom fov 45 duration 0", 2.0)


def main():
    print(f"SC_BIN = {SC_BIN}", flush=True)
    if not assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f20*.ini"):
        f.unlink()

    frozen_in = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}

    # ---------------- launch 1: build, sweep, save ----------------
    app = App("save")
    build_scene_a(app)
    d_live = app.dump("live")
    shot_live = app.shot("live")
    before = tree_md5()
    app.cmd("session action save filename f20a", 2.5)
    after = tree_md5()
    build_scene_b(app)
    d_live_b = app.dump("live_b")
    app.cmd("session action save filename f20b", 2.5)
    build_scene_c(app)
    d_live_c = app.dump("live_c")
    app.cmd("session action save filename f20c", 2.5)
    app.quit()

    fa = SESSIONS / "f20a.ini"
    fb = SESSIONS / "f20b.ini"
    fc = SESSIONS / "f20c.ini"
    if not fa.exists() or not fb.exists() or not fc.exists():
        fail("the save wrote no session file")
        return 1
    ok(f"save wrote {fa.name} ({fa.stat().st_size} B), {fb.name} and {fc.name}")
    shutil.copy(fa, OUT / "f20a.first.ini")
    shutil.copy(fc, OUT / "f20c.first.ini")

    # §6.3 - across the save COMMAND, not across the launch.
    touched = sorted(k for k in set(before) | set(after) if before.get(k) != after.get(k))
    expected = {"sessions/f20a.ini"}
    if set(touched) != expected:
        fail(f"§6.3: the save touched {touched}, expected exactly {sorted(expected)}")
    else:
        ok(f"§6.3: of {len(before)} files under ~/.spacecrafter, the save changed exactly "
           f"the session it was asked to write")

    if MUTATE:
        txt = fa.read_text()
        txt = txt.replace("bound_to_surface = true", "bound_to_surface = false")
        fa.write_text(txt)
        print("   [--mutate] bound_to_surface flipped in the saved file", flush=True)

    # ---------------- launch 2: the A/A partner (floor) ----------------
    app = App("aa")
    build_scene_a(app)
    d_aa = app.dump("aa")
    shot_aa = app.shot("aa")
    app.quit()
    floor = px_diff(shot_aa, shot_live)
    aa_moved = compare(d_live, d_aa, "A/A", tol=1e-4)
    print(f"\n== T1 floor: an A/A pair of launches of the same binary, same scene by "
          f"commands: {floor} px>8 ==", flush=True)
    if aa_moved:
        print(f"      (A/A dump fields that differ across a launch: {aa_moved})", flush=True)

    # ---------------- launch 3: restore ----------------
    app = App("restore")
    d_default = app.dump("default")             # T2-CONTROL, before any restore
    app.cmd("session action load filename f20a", 6.0)
    d_rest = app.dump("restored")
    shot_rest = app.shot("restored")

    # T4: save again from the restored state, then restore again.
    app.cmd("session action save filename f20a2", 2.5)
    d_rest2 = app.dump("restored2")
    app.cmd("session action load filename f20a2", 6.0)
    d_rest3 = app.dump("restored3")
    app.quit()

    print("\n== T2-CONTROL: the fresh launch BEFORE the restore ==", flush=True)
    ctrl_moved = compare(d_default, d_live, "control")
    if not ctrl_moved:
        fail("T2-CONTROL: the default scene already equals the saved one — T2 would pass "
             "on a restore that did nothing")
    else:
        ok(f"T2-CONTROL: {len(ctrl_moved)} of the session's fields differ before the restore "
           f"({', '.join(ctrl_moved[:8])}{' ...' if len(ctrl_moved) > 8 else ''})")

    print("\n== T2: restored vs saved, field by field ==", flush=True)
    moved = compare(d_live, d_rest, "T2", tol=1e-4)
    if moved:
        fail(f"T2: {len(moved)} field(s) did not come back: {moved}")
    else:
        ok(f"T2: every field the session carries came back "
           f"({len(EXACT + FLOAT + VEC + PLANS + OLDNAV_VEC + OLDNAV_FLOAT) + 3} fields, "
           f"{len(EXACT)} exact / {len(FLOAT + VEC)} float / {len(PLANS)} plans / "
           f"{len(OLDNAV_VEC + OLDNAV_FLOAT)} old-path view / 3 time)")

    print("\n== T4: the fixed point, entered twice ==", flush=True)
    f2 = SESSIONS / "f20a2.ini"
    ref = (OUT / "f20a.first.ini").read_bytes()
    if not f2.exists():
        fail("T4: the second save wrote nothing")
    elif f2.read_bytes() != ref:
        fail(f"T4: the second session file is not byte-identical to the first "
             f"({len(ref)} vs {f2.stat().st_size} bytes)")
        (OUT / "f20a2.ini").write_bytes(f2.read_bytes())
    else:
        ok(f"T4: the second session file is byte-identical to the first ({len(ref)} bytes)")
    m2 = compare(d_rest, d_rest2, "T4-exit1", tol=1e-4)
    m3 = compare(d_rest, d_rest3, "T4-exit2", tol=1e-4)
    if m2 or m3:
        fail(f"T4: the dump moved across the pair (exit1 {m2}, exit2 {m3})")
    else:
        ok("T4: the dump is identical at both exits — a restore of the state a restore "
           "produced lands in the same place (D33 idempotence)")

    print("\n== T1: the composed screen ==", flush=True)
    d_px = px_diff(shot_rest, shot_live)
    print(f"   restored vs saved: {d_px} px>8   (A/A floor for this scene: {floor} px>8)",
          flush=True)
    print(f"   heading (NOT carried, D28): saved {d_live['cam']['heading']:.9f} rad, "
          f"restored {d_rest['cam']['heading']:.9f} rad", flush=True)
    if floor < 0 or d_px < 0:
        fail("T1: screenshot sizes differ")
    elif d_px > floor:
        fail(f"T1: the restored screen differs from the saved one by {d_px} px>8, above "
             f"the in-scene A/A floor of {floor}")
    else:
        ok(f"T1: the restored screen is within the in-scene A/A floor "
           f"({d_px} <= {floor} px>8, same binary both sides)")

    # ---------------- launch 4: T5 ----------------
    app = App("t5")
    app.cmd("session action load filename f20b", 6.0)
    d_b = app.dump("t5_b")
    app.cmd("session action load filename f20a", 6.0)
    d_a = app.dump("t5_a")
    app.quit()
    print("\n== T5a: a DIFFERENT scene's session moves the dump ==", flush=True)
    ab = compare(d_b, d_a, "T5a")
    if not ab:
        fail("T5a: restoring a different session did not move the dump at all")
    else:
        ok(f"T5a: {len(ab)} field(s) moved between the two sessions "
           f"({', '.join(ab[:8])}{' ...' if len(ab) > 8 else ''})")
    if compare(d_live, d_a, "T5a-back", tol=1e-4):
        fail("T5a: restoring f20a after f20b did not land on the saved scene")
    else:
        ok("T5a: restoring f20a from f20b's state lands on the saved scene (order-free)")
    # The TRACKED round trip (§2 rows B2/C5), which scene A cannot carry.
    mb = compare(d_live_b, d_b, "T2-tracked", tol=1e-4)
    if mb:
        fail(f"T2-tracked: {len(mb)} field(s) of the tracked scene did not come back: {mb}")
    else:
        ok(f"T2-tracked: the tracked scene came back whole "
           f"(tracked={d_b['cam']['tracked']!r}, selected={d_b['cam']['selected']!r})")

    # T5b: exactly one key, by hand. Measured against a restore of the UNEDITED
    # file in the same launch shape, so the only difference between the two runs
    # is the edit.
    app = App("t5b_base")
    app.cmd("session action load filename f20c", 6.0)
    d_c = app.dump("t5b_base")
    app.quit()
    if compare(d_live_c, d_c, "T5b-base", tol=1e-4):
        fail("T5b: the unedited restore of scene C does not reproduce it — the base of "
             "every case below would then be wrong")
    else:
        ok("T5b: the unedited restore of scene C reproduces it (the base of the cases)")
    print("\n== T5b: one key edited by hand ==", flush=True)
    src = (OUT / "f20c.first.ini").read_text()
    # Three PARAMETERS of the file, chosen because nothing in scene C derives
    # one from another (no tracking, no lock). `distance` and `alt` were tried
    # and dropped: at 40x the distance the placement perturbs alt/az by ~6e-5
    # rad, which is a property of the scene, not of the restore.
    # `view_offset` is the fourth case and the first one with TWO expected
    # fields, which is the point of it: §2 row B10 lives on both paths (the
    # camera's scalar and the old navigator's, which is what pitches the star
    # field), so a restore that moves one and not the other is the §5.63 class
    # again. `viewOffsetEff` does not move because scene C's latch is unarmed -
    # the scalar is stored, the ramp is what applies it (D32).
    # Each case names EVERY field the edited key is an authority for, and the
    # extra names are DERIVED, not incidental — the gate's own §11.128(g)
    # lesson (a leg that needs "exactly one field moves" cannot be run on
    # parameters the engine derives from each other), applied to the old path:
    #   * the old navigator's equatorial and precessed vision vectors are
    #     M(lat,lon) . localVision, so a place edit MUST move them and a place
    #     edit that did not would mean the sky is being drawn from the old place;
    #   * `viewOffsetEff` is scalar x latch, so it moves with the scalar exactly
    #     when the latch is armed — which scene C's is.
    PLACE = ["oldnav.equVision", "oldnav.precEquVision"]
    cases = [("fov", None, "12.5", ["halfFov"]),
             ("latitude", None, "0.5", ["latitude"] + PLACE),
             ("longitude", None, "0.9", ["longitude"] + PLACE),
             ("view_offset", None, "0.375",
              ["viewOffset", "viewOffsetEff", "oldnav.viewOffset"])]
    for key, oldv, newv, fields in cases:
        edited = []
        hit = False
        for line in src.split("\n"):
            if line.strip().startswith(key + " ") or line.strip().startswith(key + "="):
                edited.append(f"{key} = {newv}")
                hit = True
            else:
                edited.append(line)
        if not hit:
            fail(f"T5b: the key '{key}' is not in the session file — the leg has no subject")
            continue
        (SESSIONS / "f20mut.ini").write_text("\n".join(edited))
        app = App("t5b_" + key)
        app.cmd("session action load filename f20mut", 6.0)
        d_mut = app.dump("t5b_" + key)
        app.quit()
        moved = compare(d_c, d_mut, "T5b:" + key, tol=1e-4)
        # `mat` is a composition of the parameters and is not in the compared
        # set; every field named here is an independent parameter of the file.
        if sorted(moved) == sorted(fields):
            ok(f"T5b: editing '{key}' moved exactly {fields} and nothing else")
        else:
            fail(f"T5b: editing '{key}' moved {moved}, expected exactly {fields}")

    # ---------------- launch 5: T10, the D8 use-site ----------------
    print("\n== T10: a FROZEN body's position survives the session ==", flush=True)
    # Two runs of the same scene, differing in ONE variable: whether the body is
    # hidden = frozen while jd advances.  Hidden is the B39/D23 freeze
    # mechanism, and a hidden body is a legal reference (§11.113(b)(vi)).
    d_t10 = {}
    for leg, freeze in (("frozen", True), ("live", False)):
        app = App("t10_" + leg)
        app.cmd("timerate rate 0", 1.0)
        app.cmd(f"date jday {JD}", 1.5)
        app.cmd("set home_planet Earth", 3.0)
        app.cmd("select planet Mars", 1.0)
        if freeze:
            app.cmd("body name Mars hidden true", 1.5)     # freeze: it stops ticking
        app.cmd(f"date jday {JD + 30}", 2.0)               # ... while 30 days pass
        app.cmd(f"session action save filename f20t10{leg}", 2.5)
        app.quit()
        app = App("t10r_" + leg)
        app.cmd(f"session action load filename f20t10{leg}", 3.0)
        if freeze:
            app.cmd("body name Mars hidden false", 2.0)    # unhide = a USE
        d = app.dump("t10_" + leg)
        app.quit()
        d_t10[leg] = d
    a, b = d_t10["frozen"]["bodies"].get("Mars"), d_t10["live"]["bodies"].get("Mars")
    if not a or not b:
        fail("T10: Mars is not in one of the dumps")
    else:
        ea, eb = a["ecl"], b["ecl"]
        d3 = math.sqrt(sum((x - y) ** 2 for x, y in zip(ea, eb)))
        rel = d3 / max(math.sqrt(sum(x * x for x in eb)), 1e-12)
        print(f"   frozen  ecl {ea}\n   live    ecl {eb}\n   |delta| {d3:.6e} AU "
              f"(rel {rel:.3e})", flush=True)
        if rel > 1e-6:
            fail(f"T10: the frozen body's position is {rel:.3e} off the never-frozen scene — "
                 f"a freeze-duration-proportional divergence is the §6.1 failure signature")
        else:
            ok(f"T10: a body frozen for 30 days comes back where the never-frozen scene puts "
               f"it ({rel:.3e} relative, recomputed at use)")

    frozen_out = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}
    if frozen_in != frozen_out:
        for k in frozen_in:
            if frozen_in[k] != frozen_out.get(k):
                fail(f"frozen data changed: {k} {frozen_in[k]} -> {frozen_out.get(k)}")
    else:
        ok("frozen md5 in == out (" +
           ", ".join(f"{k} {v[:8]}" for k, v in frozen_in.items()) + ")")

    for f in SESSIONS.glob("f20*.ini"):
        shutil.copy(f, OUT / f.name)
        f.unlink()
    print(f"\n{'FAILED: ' + str(len(FAILS)) if FAILS else 'ALL GREEN'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
