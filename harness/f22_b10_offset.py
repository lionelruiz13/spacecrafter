#!/usr/bin/env python3
"""§2 row B10 (the view offset) — the scene hole, and the row's two authorities.

TWO THINGS, one run.

(1) THE SCENE HOLE (INTENT §5.63 exclusion 6, F21).  `f20_session.py`'s scene A
    carried `set view_offset 0.25` for row B10 and the app REJECTED it —
    `view_offset` is not a registered `set` name — so the row was never
    exercised at all.  This checks the repointing BOTH WAYS in ONE launch: the
    old spelling must still be refused (the did-you-mean line appears and the
    offset does not move) and the registered spelling `set zoom_offset` must
    ACT (no refusal, and the offset moves on BOTH paths).

(2) THE ROW HAS TWO AUTHORITIES AND ONLY ONE OF THEM DRAWS THE SKY.  The camera
    holds the offset for the new path; `Navigator` holds its own, and the old
    navigator's is what pitches the star field, the milky way and the nebulae.
    D32 makes the ARMING LATCH a saved condition and its ramp a motion that
    snaps.  So the checks are stated in terms of the mechanism: after a restore,
    the scalar AND the latch must agree with the save on BOTH paths — and the
    ARMED case is the one that matters, because an unarmed offset is inert and
    an inert difference proves nothing (§11.130).

The scene arms the latch with a `look_at`, which also aims the OLD navigator
away from its launch default — so this run is simultaneously the strongest form
of the §5.63 check: a session whose old-path view direction is NOT the one a
fresh launch happens to start at.

    cd claude/harness && DISPLAY=:2 ./f22_b10_offset.py [outdir]

Exit 0 = every leg green; 1 = any failure, each named on stdout.
"""

import json
import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f21_s563
from f21_s563 import (App, JD, SESSIONS, USERDIR, FROZEN, md5,
                      assert_no_other_instance)

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f22b10").resolve()
OUT.mkdir(parents=True, exist_ok=True)
f21_s563.OUT = OUT

FAILS = []
DIDYOUMEAN = "Did you mean zoom_offset"


def fail(m):
    FAILS.append(m)
    print(f"FAIL: {m}", flush=True)


def ok(m):
    print(f"ok:   {m}", flush=True)


def header(app, tag):
    p = OUT / f"f22b10_{tag}.json"
    p.unlink(missing_ok=True)
    app.cmd(f"body action dual_dump filename {p}", 2.0)
    for _ in range(30):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    with open(p) as f:
        for ln in f:
            ln = ln.strip().replace("inf", "1e308").replace("nan", "null")
            if ln:
                return json.loads(ln)
    raise RuntimeError(f"{p}: empty dump")


def offsets(h):
    """(camera scalar, camera transition, camera effective, old scalar, old transition)"""
    c, n = h["camera"], h["oldView"]["nav"]
    return (c["viewOffset"], c["viewOffsetTransition"], c["viewOffsetEff"],
            n["viewOffset"], n["viewOffsetTransition"])


def near(a, b, tol=1e-6):
    return abs(a - b) <= tol


def logtext(app):
    return Path(app.log).read_text(errors="replace")


def main():
    if not assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f22b10*.ini"):
        f.unlink()
    frozen_in = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}

    # ---------------- launch 1: the scene hole, both ways -------------------
    app = App("b10_save")
    app.cmd("timerate rate 0", 1.0)
    app.cmd(f"date jday {JD}", 1.5)
    app.cmd("set home_planet Mars", 3.0)
    app.cmd("select planet Mars", 1.0)
    app.cmd("moveto lat 12 lon 34 alt 500000 duration 0", 2.5)
    app.cmd("zoom fov 45 duration 0", 2.0)

    base = offsets(header(app, "base"))
    print(f"   before anything: cam {base[0]} x{base[1]} = {base[2]} | old {base[3]} x{base[4]}",
          flush=True)

    # (1a) THE OLD SPELLING — must still be refused, and must not move anything.
    # The refusal is asserted from the log AFTER the app exits: the app's stdout
    # is block-buffered into that file, so a count taken while it is running
    # reads 0 whatever happened (measured — this leg failed on its own
    # instrument first, §11.130).
    app.cmd("set view_offset 0.25", 1.5)
    o = offsets(header(app, "old_spelling"))
    if not (near(o[0], base[0]) and near(o[3], base[3])):
        fail(f"`set view_offset` MOVED the offset ({base} -> {o}) — it is registered after all "
             f"and this leg's premise is wrong")
    else:
        ok("the old spelling moves nothing on either path — the scene hole was real")

    # (1b) THE REGISTERED SPELLING — must act, on BOTH paths.
    app.cmd("set zoom_offset 0.25", 1.5)
    o = offsets(header(app, "new_spelling"))
    if not near(o[0], 0.25):
        fail(f"`set zoom_offset 0.25` did not move the CAMERA's offset (got {o[0]})")
    elif not near(o[3], 0.25):
        fail(f"`set zoom_offset 0.25` did not move the OLD navigator's offset (got {o[3]}) — "
             f"the path that draws the sky did not get the value")
    else:
        ok(f"`set zoom_offset 0.25` acts on BOTH paths (cam {o[0]}, old {o[3]}), no refusal")

    # (2a) THE LATCH. Unarmed, the offset is stored and inert; a `look_at` arms it
    # on both paths and the effective offset becomes non-zero. An inert
    # difference proves nothing, which is why the saved scene is the ARMED one.
    if not (near(o[1], 0.0) and near(o[4], 0.0) and near(o[2], 0.0)):
        fail(f"the offset armed itself without a move ({o}) — D32's latch is not a latch")
    else:
        ok("unarmed: the scalar is stored and the EFFECTIVE offset is 0 on both paths (D32)")
    app.cmd("look_at azimuth 30 altitude 20 duration 0", 3.0)
    time.sleep(2.0)
    a = offsets(header(app, "armed"))
    if not (near(a[1], 1.0, 1e-3) and near(a[4], 1.0, 1e-3)):
        fail(f"the look_at did not arm the latch on both paths (cam {a[1]}, old {a[4]})")
    elif not near(a[2], 0.25, 1e-6):
        fail(f"armed, the camera's effective offset is {a[2]}, expected 0.25")
    else:
        ok(f"armed by a move: latch 1 on both paths, effective offset {a[2]:.6f}")

    saved = a
    saved_vision = header(app, "armed")["oldView"]["nav"]["localVision"]
    app.cmd("session action save filename f22b10", 2.5)
    app.quit()

    # The refusal, read off the completed log: EXACTLY ONE, and it names the
    # spelling the scene no longer uses.
    lt = logtext(app)
    n = lt.count(DIDYOUMEAN)
    if n != 1:
        fail(f"expected exactly one did-you-mean refusal in this launch (the old spelling), "
             f"found {n} — either the old spelling was accepted or the new one was refused")
    elif "Could not execute: set view_offset" not in lt:
        fail("the refusal is logged but not against `set view_offset` — the leg is not "
             "measuring what it says")
    elif "Could not execute: set zoom_offset" in lt:
        fail("`set zoom_offset` was refused — it is not the registered name after all")
    else:
        ok("exactly one refusal in the launch, against `set view_offset`, none against "
           "`set zoom_offset` (§2(f): the app says what it did not do)")

    # ---------------- launch 2: the restore, with a control ------------------
    app = App("b10_rest")
    ctl = offsets(header(app, "control"))
    ctl_vision = header(app, "control")["oldView"]["nav"]["localVision"]
    if near(ctl[0], saved[0]) and near(ctl[3], saved[3]):
        fail(f"the fresh launch already carries the saved offset {ctl} — the restore leg would "
             f"pass on a default")
    else:
        ok(f"control: the fresh launch is at cam {ctl[0]} x{ctl[1]} / old {ctl[3]} x{ctl[4]}, "
           f"not the saved value")
    app.cmd("session action load filename f22b10", 6.0)
    time.sleep(2.0)
    h = header(app, "restored")
    r = offsets(h)
    r_vision = h["oldView"]["nav"]["localVision"]
    app.quit()

    print(f"\n   saved:    cam {saved[0]} x{saved[1]} = {saved[2]} | old {saved[3]} x{saved[4]}",
          flush=True)
    print(f"   restored: cam {r[0]} x{r[1]} = {r[2]} | old {r[3]} x{r[4]}", flush=True)
    for i, name in enumerate(("camera scalar", "camera latch", "camera effective",
                              "old scalar", "old latch")):
        if not near(r[i], saved[i], 1e-5):
            fail(f"B10 restore: {name} came back {r[i]}, saved {saved[i]}")
    if not FAILS:
        ok("B10 restore: the scalar AND the latch came back on BOTH paths, armed (D32)")

    # The §5.63 half, on a direction a fresh launch does NOT start at.
    import numpy as np
    sv, rv, cv = (np.array(v) for v in (saved_vision, r_vision, ctl_vision))
    def ang(a, b):
        a, b = a / np.linalg.norm(a), b / np.linalg.norm(b)
        return float(np.degrees(np.arccos(np.clip(a.dot(b), -1, 1))))
    d_rest, d_ctl = ang(sv, rv), ang(sv, cv)
    print(f"\n   old-path view direction: restored {d_rest:.6f} deg from saved, "
          f"fresh launch {d_ctl:.6f} deg from saved", flush=True)
    if d_ctl < 1.0:
        fail(f"the fresh launch already looks where the session does ({d_ctl:.6f} deg) — "
             f"this leg would pass without restoring anything")
    elif d_rest > 1e-3:
        fail(f"the OLD path's view direction came back {d_rest:.6f} deg from the saved one")
    else:
        ok(f"the OLD path's view direction came back to {d_rest:.2e} deg, from a control "
           f"{d_ctl:.3f} deg away — the session carries an AIMED direction, not a default")

    frozen_out = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}
    if frozen_in != frozen_out:
        fail("frozen md5 in != out")
    else:
        ok("frozen md5 in == out")

    print(f"\n{'ALL GREEN' if not FAILS else str(len(FAILS)) + ' FAILURE(S)'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
