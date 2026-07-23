#!/usr/bin/env python3
# B5 diagnosis driver - reproduce the dot-only regime (ref=MilkyWay at
# 3.2e9 AU) with the gdb probe attached; not a pass/fail harness.
import socket, time, sys, os, json, math

OUT = sys.argv[1]
AU_M = 149597870700.0

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "select planet Sun", 1)
send(s, "flag track_object on", 4)
send(s, "flag atmosphere off", 2)
send(s, "camera action free_mode state on", 1)
send(s, "moveto altitude 103848116760596 duration 0", 4)      # ~694 AU, ref->MilkyWay (resolved)
send(s, f"body action dual_dump filename {OUT}/diag_gal.json", 2)
send(s, "moveto altitude 185700000000000 duration 0", 4)      # +MilkyWay datum -> 3.2e9 AU (dot-only)
send(s, f"body action dual_dump filename {OUT}/diag_dot.json", 2)
time.sleep(3)
print("diag done", flush=True)
