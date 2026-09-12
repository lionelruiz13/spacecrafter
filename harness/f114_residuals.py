#!/usr/bin/env python3
"""F114 -- the two RESIDUALS of the fix, measured rather than read off the source.

Neither is introduced by F114 and neither is fixed by it; both are recorded with
a datum instead of a source read (the S11.150(k) standard).

R1  TRACKING IS BODY-ONLY ON THE DRAWN PATH. Camera::trackBody takes a
    ModularBody*, and the only expression that produces one from a selection is
    ModularBody::findBody(name) -- which answers nullptr for a star. So
    `select star X` + `zoom auto in` starts old's tracking and leaves the drawn
    path's target empty, exactly as the already-dual Core::setFlagTracking does
    at the same expression. Measured here on a star, against the same command
    pair on a planet.

R2  THE 0.2 s FLOOR ON THE CAMERA'S VIEW PLAN (Camera.cpp, `if (T < 0.2f) T =
    0.2f`, whose stated reason is finite retarget accelerations). Old's move
    takes exactly the commanded duration (navigator.cpp: speed = 1/(duration*
    1000), angle-independent), so for a commanded duration in (0, 0.2) s the
    drawn view is still moving after old has landed. duration 0 snaps on both
    paths, so the residual is an interval, not an edge.

Usage: f114_residuals.py <outdir>
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


def settle(sock, secs=7.0):
    send(sock, "timerate rate 1", 0.3)
    time.sleep(secs)
    send(sock, "timerate rate 0", 1.5)


def set_date(sock, jd):
    send(sock, "timerate rate 1", 0.3)
    send(sock, "date jday %.6f" % jd, 1.0)
    send(sock, "timerate rate 0", 2.0)


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

# ---- R1: the same command pair on a STAR and on a PLANET -----------------
send(s, "flag track_object off")
send(s, "deselect")
send(s, "zoom auto initial duration 0", 1.5)
settle(s, 3.0)
set_date(s, JD0)
send(s, "select star Sirius", 1)
dump(s, "star_selected")
send(s, "zoom auto in duration 0", 1.5)
settle(s, 7.0)
dump(s, "star_zoomed")            # old tracks the star; camera.tracked = ?

send(s, "zoom auto initial duration 0", 1.5)
settle(s, 3.0)
set_date(s, JD0)
send(s, "select planet Mars", 1)
send(s, "zoom auto in duration 0", 1.5)
settle(s, 7.0)
dump(s, "planet_zoomed")          # the control: the same pair on a body

# ---- R2: a commanded duration under the camera's 0.2 s plan floor --------
send(s, "timerate rate 1", 0.3)
send(s, "zoom auto initial duration 0.1", 0.0, 0.01)
for i in range(10):
    dump(s, "d01_%02d" % i, 0.04, 0.01)
send(s, "timerate rate 0", 1.5)
settle(s, 3.0)
dump(s, "d01_after")

print("f114 residuals done", flush=True)
s.close()
