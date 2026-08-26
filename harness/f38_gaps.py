#!/usr/bin/env python3
"""F38 — the two ADJACENT old-only write sites found while mirroring the four.

Neither is a `flag_lock_equ_pos` site, so neither is in F38's mandate; both are
measured here so the rows that record them carry a datum instead of a source
read (record-don't-fix, §11.150).

G1 — `zoom auto in` starts OLD tracking only [core.cpp:1324,
     `navigation->setFlagTraking(true)` with no Camera::trackBody beside it,
     while the B17 offset mirror IS there two lines below]. Consequence: the
     path that DRAWS does not follow the body, which is D15(c)'s own use case
     (*"hard to impossible to properly observe a body while the time continue
     to tick"*) reached through a different command. Observable: the body's
     SCREEN POSITION on each path across a sidereal advance.

G2 — `autoZoomOut` re-aims only the OLD view to InitViewPos [core.cpp:1430/1478,
     `navigation->moveTo(InitViewPos, ...)`, no Camera lookTo], so
     `zoom auto initial` returns old to the init direction and leaves the drawn
     view where it was. Observable: each path's own forward direction across the
     command, with the date frozen so nothing else can move it.

Usage: f38_gaps.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
JD0 = 2461233.5
DJD = 0.05


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
    shot(sock, name); dump(sock, name)


def set_date(sock, jd):
    send(sock, "timerate rate 1", 0.3)
    send(sock, f"date jday {jd:.6f}", 1.0)
    send(sock, "timerate rate 0", 2.0)


def settle(sock, secs=7.0):
    send(sock, "timerate rate 1", 0.3)
    time.sleep(secs)
    send(sock, "timerate rate 0", 1.5)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0", 1)
send(s, "flag atmosphere off")
send(s, "flag fog off")
send(s, "flag landscape off")
send(s, "flag show_fps off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "flag stars on")
send(s, "flag star_names off")
send(s, "flag planet_names off")
set_date(s, JD0)

# ---- G1 -----------------------------------------------------------------
send(s, "flag track_object off")
send(s, "flag lock_sky_position off")
send(s, "deselect")
send(s, "zoom auto initial duration 0", 1.5)
settle(s, 3.0)
set_date(s, JD0)
send(s, "select planet Mars", 1)
dump(s, "g1_selected")
send(s, "zoom auto in duration 0", 1.5)   # core.cpp:1324 fires
settle(s, 7.0)
dump(s, "g1_after_zoom_in")               # old flagTraking=1, camera.tracked=?
set_date(s, JD0);       sample(s, "g1_t0")
set_date(s, JD0 + DJD); sample(s, "g1_t1")

# ---- G2 -----------------------------------------------------------------
send(s, "flag track_object off")
send(s, "flag lock_sky_position off")
send(s, "zoom auto initial duration 0", 1.5)
settle(s, 3.0)
set_date(s, JD0)
send(s, "select planet Mars", 1)
send(s, "flag track_object on", 1)        # BOTH paths slew to Mars (dual setter)
settle(s, 8.0)
send(s, "flag track_object off", 1)       # both stop; both views stay on Mars
settle(s, 3.0)
sample(s, "g2_before")                    # both away from the init direction
send(s, "zoom auto initial duration 0", 1.5)
settle(s, 5.0)
sample(s, "g2_after")                     # old re-aimed; new?

print("f38 gaps done", flush=True)
s.close()
