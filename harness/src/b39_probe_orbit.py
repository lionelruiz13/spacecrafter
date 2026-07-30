#!/usr/bin/env python3
# Throwaway instrument-calibration probe for the B39 orbit-line leg: which
# channel actually puts an orbit line on the composed screen?
import socket, time, sys, os
import numpy as np
from PIL import Image
OUT = os.path.abspath(sys.argv[1])
os.makedirs(OUT, exist_ok=True)
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
def send(c, p=0.8):
    s.sendall((c + "\n").encode()); time.sleep(p)
    try:
        s.settimeout(0.2); s.recv(4096)
    except socket.timeout:
        pass
    s.settimeout(None); print(">>", c, flush=True)
def shot(t, p=1.8):
    send(f"body action screenshot filename {OUT}/p_{t}.png", p); return t
def im(t):
    return np.asarray(Image.open(f"{OUT}/p_{t}.png").convert("RGB")).astype(np.int32)
def d(a, b):
    x = np.abs(im(a) - im(b)).max(axis=2); return int((x > 32).sum()), int(x.max())

send("flag experimental_path on", 1)
send("timerate rate 0", 1)
send("date jday 2461233.5", 1)
send("flag landscape off", 1)
send("flag atmosphere off", 1)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
shot("base")
send("body name Mars orbit true", 4)
shot("pername")
print("pername vs base:", d("pername", "base"))
send("flag planet_orbits on", 4)
shot("global")
print("global vs pername:", d("global", "pername"))
print("global vs base:", d("global", "base"))
send("flag planet_orbits off", 4)
shot("globaloff")
print("globaloff vs base:", d("globaloff", "base"))
send("body name Mars orbit true", 4)
shot("pername2")
print("pername2 vs globaloff:", d("pername2", "globaloff"))
# zoom out / different aim: put the observer in free flight above the ecliptic
send("select planet Sun pointer off", 1)
send("flag track_object on", 4)
send("flag track_object off", 1)
shot("aimsun")
send("flag planet_orbits on", 4)
shot("aimsun_orb")
print("aimsun_orb vs aimsun:", d("aimsun_orb", "aimsun"))
s.close()
