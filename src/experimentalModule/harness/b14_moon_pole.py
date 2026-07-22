#!/usr/bin/env python3
"""B14 moon-absolute-pole driver (INTENT 11.68).

Dumps the rotation-frame state of a MOON with a non-system-centered parent
(Iapetus / Saturn - B14's pilot) at TWO dates far enough apart that the parent
has moved a large arc in its orbit.  The measurement is projection-free (raw
`obliquity` / `ascendingNode` / `absoluteTiltFrame` + the per-hop `tilt`
matrix added to dumpHops), so no init_fov / fisheye setup is needed - the same
reason the B28 gate ran direct.

The data file (~/.spacecrafter/ssystem.ini) is mutated by the CALLER between
launches (baseline / absolute_pole / parent_relative) and restored
byte-identically afterwards - this driver never touches it (b26/b16 pattern:
the file's state IS the case).

Usage: b14_moon_pole.py <outdir> <tag>
Dumps <outdir>/b14_<tag>_d1.json and _d2.json (+ .navstr sidecars).
"""
import socket, time, sys, os

OUT = sys.argv[1]
TAG = sys.argv[2] if len(sys.argv) > 2 else "run"
os.makedirs(OUT, exist_ok=True)

# Two epochs ~10 Julian years apart: Saturn (P ~29.5 yr) sweeps ~122 deg of
# orbit between them.  An absolute pole must be IDENTICAL at both; a
# parent-relative one carries the (fixed) parent tilt either way - the
# discriminator is the parent-tilt factor, measured separately.
D1 = 2451545.0            # J2000.0
D2 = 2455197.0            # J2000 + 3652 d (~10.0 yr)

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.3); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

sock = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(sock, "flag experimental_path on", 1)
send(sock, "timerate rate 0", 1)
send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)

send(sock, "date jday %.9f" % D1, 1)
send(sock, "wait duration 2", 2)
send(sock, f"body action dual_dump filename {OUT}/b14_{TAG}_d1.json", 2)

send(sock, "date jday %.9f" % D2, 1)
send(sock, "wait duration 2", 2)
send(sock, f"body action dual_dump filename {OUT}/b14_{TAG}_d2.json", 2)
print("DRIVER OK", flush=True)
