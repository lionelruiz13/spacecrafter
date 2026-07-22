#!/usr/bin/env python3
"""B18 - equatorial-mount sky-lock (old flag_lock_equ_pos) observable check.

One fresh launch. Anchored on Earth, EQUATORIAL mount is the SHIPPED config
(viewing_mode=equator); the new-path Camera runs ALTAZ internally (the mount
command is not wired to Camera - pre-existing, INTENT 11.58) but the sky-lock
holds the WHOLE composed rotation, so the terminal observable is mount-
independent.

Discriminator (DoD item 2): advance sidereal time by DJD day and measure the
view-direction change in TWO frames:
  * equatorial frame  = the reference-body frame (mat rotational part for the
    new path, helioToEye rotational part for the old) - held ~0 under lock.
  * alt-az frame       = Camera (alt,az) - non-zero under lock (drifts to hold
    the sky), ~0 when unlocked. A no-op "lock" would leave BOTH non-zero.

Channels: the live `flag lock_sky_position` command (TCP). A bogus spelling
must be a no-op (dump skyLocked stays false; gdb sees 0 fires). Reversible
pair: off -> on -> off -> on, the second `on` starting from the state the
first `off` produced.

Usage: b18_skylock.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

JD0 = 2461233.5
DJD = 0.05   # ~18.05 deg of sidereal rotation (360.9856 deg/day * 0.05)


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


def set_date(sock, jd):
    # proven pattern (baseline probe): brief rate-1 tick so the frame loop
    # re-dispatches, then the absolute date, then freeze; a settle wait lets
    # the sky-lock hold converge (placement reflects the new jd only after the
    # next dispatch - the inherent one-frame lag).
    send(sock, "timerate rate 1", 0.3)
    send(sock, f"date jday {jd:.6f}", 1.0)
    send(sock, "timerate rate 0", 2.0)


def sample(sock, name):
    shot(sock, name)
    dump(sock, name)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# --- scene ---------------------------------------------------------------
send(s, "timerate rate 0", 1)
send(s, "flag atmosphere off")
send(s, "flag fog off")
send(s, "flag landscape off")
send(s, "flag show_fps off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
# rich, sky-anchored content so the drift is visible on the composed screen
send(s, "flag stars on")
send(s, "flag star_names off")
send(s, "flag constellation_drawing on")
send(s, "flag constellation_art off")
send(s, "flag equatorial_grid on")
send(s, "flag planet_names off")
set_date(s, JD0)

# --- noise floor control (same state twice, B30) -------------------------
sample(s, "ctrl_a")
sample(s, "ctrl_b")

# --- PHASE OFF (default local/alt-az lock) -------------------------------
set_date(s, JD0);        sample(s, "off_t0")
set_date(s, JD0 + DJD);  sample(s, "off_t1")

# --- bogus spelling: must be a no-op -------------------------------------
set_date(s, JD0)
send(s, "flag lock_sky_positionX on", 1)   # misspelled -> unknown flag
dump(s, "bogus")                            # skyLocked must stay false

# --- PHASE ON (first entry): lock captured at JD0 ------------------------
send(s, "flag lock_sky_position on", 1)     # real command -> gdb fires
sample(s, "on_t0")
set_date(s, JD0 + DJD);  sample(s, "on_t1")

# --- reversible OFF (first exit), from the on-state at JD1 ---------------
send(s, "flag lock_sky_position off", 1)
set_date(s, JD0);        sample(s, "off2_t0")
set_date(s, JD0 + DJD);  sample(s, "off2_t1")

# --- reversible ON (second entry), from the off-state --------------------
set_date(s, JD0)
send(s, "flag lock_sky_position on", 1)
sample(s, "on2_t0")
set_date(s, JD0 + DJD);  sample(s, "on2_t1")

# --- reversible OFF (second exit) ----------------------------------------
send(s, "flag lock_sky_position off", 1)
set_date(s, JD0);        sample(s, "off3_t0")
set_date(s, JD0 + DJD);  sample(s, "off3_t1")

print("b18 done", flush=True)
s.close()
