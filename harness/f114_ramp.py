#!/usr/bin/env python3
"""F114 -- the RAMP leg of S5.101 and the two branches of Core::autoZoomOut.

`zoom auto initial` re-aims BOTH paths now, and the two paths ride different
laws to the same place:
  OLD  navigator.cpp:71-73, zooming_mode == -1 : c = coef^4, coef linear in
       t/duration, slerped at fraction c  ->  progress(u) = A*u^4
  NEW  Camera::lookTo -> solvePlan (Camera.cpp:365-372), v0 = 0: two quadratic
       phases, t1 = T/2, a = 4A/T^2  ->  progress(u) = 2A*u^2   (u <= 1/2)
                                                     = A - 2A*(1-u)^2 (u > 1/2)
so the divergence is MEASURED and STATED, never gated (B34 / S11.92: the END
state is the parity target, the ramp is the perceptual class).

THE CLOCK IS THE APP'S OWN. Each sample's time comes from the dump header's
`jd` at `timerate rate 1`, where 1 day of simulated time = 1 s of wall clock:
t = (jd - jd_start)*86400. The ramp advances on the same real deltaTime, so jd
IS the ramp's clock and the send/write latency never enters the time axis. Cost
of that choice, stated: the init direction is a LOCAL-frame vector, so at
timerate 1 both paths' targets drift sidereally by 0.0042 deg/s -- common to
both paths, so the mutual angle (the parity observable) is untouched.

Legs, in one launch:
  ramp1    `zoom auto initial duration 1`  sampled as fast as the socket allows
  ramp10   `zoom auto initial duration 10` sampled ~0.35 s apart (the shape)
  manual   `zoom auto out manual true` repeated until the fov reaches InitFov:
           the OTHER autoZoomOut branch (core.cpp, the manual block), which
           `zoom auto initial` can never reach -- it is the `full` branch.
           Attribution is clean: with an object selected and manual_zoom on,
           autoZoomOut returns from the manual block every time.

Usage: f114_ramp.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
JD0 = 2461233.5


def send(sock, cmd, pause=0.6, drain=0.2):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(drain); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print("%.3f >> %s" % (time.time(), cmd), flush=True)


def dump(sock, name, pause=2.5, drain=0.2):
    send(sock, "body action dual_dump filename %s/%s.json" % (OUT, name),
         pause, drain)


def fast_dump(sock, name, pause):
    # 10 ms drain instead of 200: inside a 1 s ramp the drain IS the sampling
    # period. Nothing replies on this socket, so the drain is pure dead time.
    dump(sock, name, pause, 0.01)


def set_date(sock, jd):
    send(sock, "timerate rate 1", 0.3)
    send(sock, "date jday %.6f" % jd, 1.0)
    send(sock, "timerate rate 0", 2.0)


def settle(sock, secs=7.0):
    send(sock, "timerate rate 1", 0.3)
    time.sleep(secs)
    send(sock, "timerate rate 0", 1.5)


def track_mars(sock):
    """Leave both paths tracking Mars through `zoom auto in` (the S5.100 site)."""
    send(sock, "flag track_object off")
    send(sock, "flag lock_sky_position off")
    send(sock, "zoom auto initial duration 0", 1.5)
    settle(sock, 3.0)
    set_date(sock, JD0)
    send(sock, "select planet Mars", 1)
    send(sock, "zoom auto in duration 0", 1.5)
    settle(sock, 7.0)


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
send(s, "flag manual_zoom off")
set_date(s, JD0)

# ---- ramp1: duration 1, the mandated leg ---------------------------------
track_mars(s)
dump(s, "r1_before")
send(s, "timerate rate 1", 0.3)          # the clock for the samples
send(s, "zoom auto initial duration 1", 0.0, 0.01)
for i in range(12):
    fast_dump(s, "r1_%02d" % i, 0.06)
send(s, "timerate rate 0", 1.5)
settle(s, 3.0)
dump(s, "r1_after")

# ---- ramp10: duration 10, the shape --------------------------------------
track_mars(s)
dump(s, "r10_before")
send(s, "timerate rate 1", 0.3)
send(s, "zoom auto initial duration 10", 0.0, 0.01)
for i in range(24):
    fast_dump(s, "r10_%02d" % i, 0.30)
send(s, "timerate rate 0", 1.5)
settle(s, 3.0)
dump(s, "r10_after")

# ---- manual: the OTHER autoZoomOut branch --------------------------------
track_mars(s)
send(s, "flag manual_zoom on", 0.5)
dump(s, "m_before")
for i in range(20):
    send(s, "zoom auto out manual true duration 0", 0.25, 0.01)
settle(s, 3.0)
dump(s, "m_after")
send(s, "flag manual_zoom off", 0.5)

print("f114 ramp done", flush=True)
s.close()
