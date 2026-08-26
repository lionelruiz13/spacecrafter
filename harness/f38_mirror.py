#!/usr/bin/env python3
"""F38 — D15(c): the four `flag_lock_equ_pos` bypass sites, driven both ways.

Task: mirror the four OLD-ONLY sky-lock write sites so ENABLE and DISABLE both
reach the new-path Camera. Acceptance is SERVO-grade (§11.149(a3)): not "the
two flags agree" but "the DRAWN view behaves like old's across the transition".
So every scene here records the flags AND a t0/t1 sidereal pair whose held /
released character is read off the composed screen and the two paths' own view
matrices.

The four sites (re-verified at dispatch on `master-beta @ f0c8ef83`):
  S2  core.cpp:2313  selectObject(Object)          ENABLE  (select while tracking)
  S3  core.cpp:1083  selectObject(type,id)         ENABLE  (RE-select the tracked
                                                    body: the inner Object
                                                    overload early-returns on the
                                                    same object, so old tracking
                                                    is still 1 when this runs)
  S1  core.cpp:1410  autoZoomOut(full)             DISABLE (`zoom auto initial`)
  S4  core.cpp:1372  autoZoomOut(manual branch)    DISABLE (`flag manual_zoom on`
                                                    + `zoom auto out manual 1`)

Every reversible pair is traversed TWICE, the second entry starting from the
state the first exit produced (rare-path rule) — S1 and S2 each run a second
round from the state their first round left behind.

Usage: f38_mirror.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

JD0 = 2461233.5
DJD = 0.05          # ~18.05 deg of sidereal rotation (360.9856 deg/day)


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def dump(sock, name, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)


def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)


def sample(sock, name):
    shot(sock, name)
    dump(sock, name)


def set_date(sock, jd):
    # b18 pattern: a brief rate-1 tick so the frame loop re-dispatches, the
    # absolute date, then freeze; the settle wait lets the sky-lock hold
    # converge (placement reflects the new jd only after the next dispatch).
    send(sock, "timerate rate 1", 0.3)
    send(sock, f"date jday {jd:.6f}", 1.0)
    send(sock, "timerate rate 0", 2.0)


def settle(sock, secs=7.0):
    """Drain in-flight view plans. Both paths suppress the sky-lock while a
    view move is in flight (old: auto_move > tracking > lock; new:
    Camera::update's `viewT <= 0` guard), and the tracking lookTo re-plans
    every frame with a 5 s max-duration, so a measurement taken right after a
    tracking release would read the PLAN, not the lock."""
    send(sock, "timerate rate 1", 0.3)
    time.sleep(secs)
    send(sock, "timerate rate 0", 1.5)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# --- scene ----------------------------------------------------------------
send(s, "timerate rate 0", 1)
send(s, "flag atmosphere off")
send(s, "flag fog off")
send(s, "flag landscape off")
send(s, "flag show_fps off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "flag stars on")
send(s, "flag star_names off")
send(s, "flag constellation_drawing on")
send(s, "flag constellation_art off")
send(s, "flag equatorial_grid on")
send(s, "flag planet_names off")
send(s, "flag object_trails off")
set_date(s, JD0)

# --- noise floor: the same state twice (B30) ------------------------------
sample(s, "ctrl_a")
sample(s, "ctrl_b")


def release_all(sock):
    """Return to the common start state: nothing selected, not tracking, lock
    off, fov at init. Sent through the OPERATOR surface only."""
    send(sock, "flag track_object off")
    send(sock, "flag lock_sky_position off")
    send(sock, "deselect")
    send(sock, "zoom auto initial duration 0", 1.5)
    settle(sock, 3.0)


# ==========================================================================
# S1 — DISABLE site core.cpp:1410 (autoZoomOut, full).  SHIPPED sequence from
#      §11.112(b): `flag lock_sky_position on` -> unzoom-to-init.  Old drifts
#      (its flag was cleared); the new path must drift too.
# ==========================================================================
def scene_s1(sock, rnd):
    tag = f"s1r{rnd}"
    release_all(sock)
    set_date(sock, JD0)
    send(sock, "flag lock_sky_position on", 1)
    settle(sock, 3.0)
    dump(sock, f"{tag}_locked")            # both flags must read 1 (mirror is dual)
    # held pair: with the lock ON both paths hold the equatorial frame
    set_date(sock, JD0);       sample(sock, f"{tag}_held_t0")
    set_date(sock, JD0 + DJD); sample(sock, f"{tag}_held_t1")
    # the transition under test
    set_date(sock, JD0)
    send(sock, "zoom auto initial duration 0", 1.5)
    settle(sock, 5.0)
    dump(sock, f"{tag}_after")             # PRE: old=0 new=1 ; POST: old=0 new=0
    set_date(sock, JD0);       sample(sock, f"{tag}_rel_t0")
    set_date(sock, JD0 + DJD); sample(sock, f"{tag}_rel_t1")


# ==========================================================================
# S2 — ENABLE site core.cpp:2313 (selectObject(Object)).  Select a DIFFERENT
#      body while tracking: old turns the lock on and stops tracking, so the
#      view must HOLD on the sky; the new path must hold too.
# ==========================================================================
def scene_s2(sock, rnd):
    tag = f"s2r{rnd}"
    release_all(sock)
    set_date(sock, JD0)
    send(sock, "select planet Mars", 1)
    dump(sock, f"{tag}_sel1")
    send(sock, "flag track_object on", 1)
    settle(sock, 7.0)
    dump(sock, f"{tag}_tracking")           # old flagTraking=1, camera.tracked=Mars
    send(sock, "select planet Jupiter", 1)  # site 2313 fires
    dump(sock, f"{tag}_after")              # PRE: old=1 new=false ; POST: both 1
    settle(sock, 7.0)                       # drain the released tracking plan
    set_date(sock, JD0);       sample(sock, f"{tag}_t0")
    set_date(sock, JD0 + DJD); sample(sock, f"{tag}_t1")


# ==========================================================================
# S3 — ENABLE site core.cpp:1083 (selectObject(type,id)).  Only reachable by
#      RE-selecting the body already selected AND tracked: the inner Object
#      overload returns early on the same object, leaving old tracking at 1
#      when the outer block runs.  This site's companion `setFlagTraking(0)`
#      (core.cpp:1085) is OLD-ONLY, so the new path keeps `Camera::target`
#      and its sky-lock stays DORMANT — the discriminator for the coupling.
# ==========================================================================
def scene_s3(sock):
    tag = "s3"
    release_all(sock)
    set_date(sock, JD0)
    send(sock, "select planet Mars", 1)
    send(sock, "flag track_object on", 1)
    settle(sock, 7.0)
    dump(sock, f"{tag}_tracking")
    send(sock, "select planet Mars", 1)      # RE-select: site 1083 fires
    dump(sock, f"{tag}_after")               # camera.tracked: PRE "Mars", POST ""
    settle(sock, 7.0)
    set_date(sock, JD0);       sample(sock, f"{tag}_t0")
    set_date(sock, JD0 + DJD); sample(sock, f"{tag}_t1")


# ==========================================================================
# S4 — DISABLE site core.cpp:1372 (autoZoomOut, manual branch).  Needs
#      FlagManualZoom (shipped config has it FALSE) and fov*2 >= init_fov.
# ==========================================================================
def scene_s4(sock):
    tag = "s4"
    release_all(sock)
    set_date(sock, JD0)
    send(sock, "flag manual_zoom on", 1)
    send(sock, "select planet Mars", 1)
    send(sock, "flag lock_sky_position on", 1)
    settle(sock, 3.0)
    dump(sock, f"{tag}_locked")
    send(sock, "zoom auto out manual 1 duration 0", 1.5)
    settle(sock, 5.0)
    dump(sock, f"{tag}_after")               # PRE: old=0 new=1 ; POST: old=0 new=0
    set_date(sock, JD0);       sample(sock, f"{tag}_rel_t0")
    set_date(sock, JD0 + DJD); sample(sock, f"{tag}_rel_t1")
    send(sock, "flag manual_zoom off", 1)


scene_s1(s, 1)
scene_s2(s, 1)
scene_s3(s)
scene_s4(s)
# Rare-path rule: both reversible scenes traversed a SECOND time, each entry
# starting from the state the previous exit produced.
scene_s1(s, 2)
scene_s2(s, 2)

print("f38 done", flush=True)
s.close()
