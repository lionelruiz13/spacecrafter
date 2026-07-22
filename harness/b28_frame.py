#!/usr/bin/env python3
"""B28 loader frame-declaration driver (INTENT 11.67).

Freezes the deterministic dual-dump scene (same epoch/observer as
dual-dump.sts / drive_scenes.py), pins the NEW path, and captures the
rotation-frame state of the 7 rot_pole_ra planets TWICE:

  load1  = the state produced by the startup load
  load2  = the state produced by `body action reload` (the second entry of
           the load reversible pair - DoD 5; the same file, the same code)

The bit-identical gate (DoD 3) reads the per-hop `obliquity` / `ascendingNode`
(raw re fields, added to dumpHops for this row) and the `tilt` matrix for the
7 planets; the frame refactor must not move any of them by one ulp vs the
pre-refactor (instrument-only) binary.  The reversible pair (DoD 5) checks
load1==load2 for every body, declared-frame or defaulted.

Usage: b28_frame.py <outdir>
"""
import socket, time, json, sys, os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b28")
os.makedirs(OUT, exist_ok=True)

J0 = 2461233.5

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

sock = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(sock, "flag experimental_path on", 1)
send(sock, "timerate rate 0", 1)
send(sock, "date jday %.9f" % J0, 1)
send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(sock, "select planet Moon", 1)
send(sock, "flag track_object on", 1)
send(sock, "wait duration 3", 3)

send(sock, f"body action dual_dump filename {OUT}/b28_load1.json", 2)
# second entry of the reversible pair: reload rebuilds every body from the
# same file through the same loader (frame resolution reruns), keeping camera
# + date (B16).  Must produce the identical frame resolution in memory.
send(sock, "body action reload", 2)
send(sock, "wait duration 2", 2)
send(sock, f"body action dual_dump filename {OUT}/b28_load2.json", 2)
print("DRIVER OK", flush=True)
