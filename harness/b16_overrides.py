#!/usr/bin/env python3
"""B16 follow-up - what a reload does to RUNTIME PER-BODY OVERRIDES.

Vixy's decision on B16 names camera + date as the state to keep. Body-scoped
runtime state (scale seams, hide/show, per-body flags) is not named, and the
rebuild-from-file necessarily drops it: this run MEASURES which, instead of
assuming. Three probes, all read back from the live tree via dual_dump:
  * moon_scale 30 -> boundingRadius x30 -> reload -> ?
  * body name Mars hidden true -> relation = -orbiting -> reload -> ?
  * flag planet_orbit off/on is global (not per body) - not probed here.
Data file untouched (md5 asserted at both ends).

Usage: b16_overrides.py <outdir>
"""
import socket, sys, time, os, hashlib

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def dump(sock, name, pause=3):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)


M0 = md5(SSY)
print("ssystem.ini md5 in  =", M0, flush=True)
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "timerate rate 0", 2)

send(s, "flag moon_scaled on", 1)
send(s, "set moon_scale 30", 15)          # ASmooth ease - settle
send(s, "body name Mars hidden true", 3)
dump(s, "ovr_pre")
send(s, "body action reload", 5)
dump(s, "ovr_post")
print("ssystem.ini md5 out =", md5(SSY), " MATCH=", md5(SSY) == M0, flush=True)
print("b16 overrides done", flush=True)
s.close()
