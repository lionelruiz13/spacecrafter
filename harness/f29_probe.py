#!/usr/bin/env python3
# F29 instrument bring-up probe: which orbit-line control surface actually
# draws, before and after a reference switch. Not a delivery instrument - it
# exists so the F29 scene is built on measured command behaviour instead of
# assumed behaviour (§0.5: a silent no-op probe converts observation into
# fiction).
import socket, time, sys, os, json
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "f29", "probe")
os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.3); r = sock.recv(8192)
    except socket.timeout:
        r = b""
    sock.settimeout(None)
    print(f">> {cmd}   <- {r[:120]!r}", flush=True)


def shot(tag, pause=1.6):
    send(f"body action screenshot filename {OUT}/p_{tag}.png", pause)
    a = np.asarray(Image.open(os.path.join(OUT, f"p_{tag}.png")).convert("RGB")).astype(int)
    m = a.max(axis=2) > 16
    red = int((m & (a[:, :, 0] > a[:, :, 1] + 8) & (a[:, :, 0] > a[:, :, 2] + 8)).sum())
    grn = int((m & (a[:, :, 1] > a[:, :, 0] + 8) & (a[:, :, 1] > a[:, :, 2] + 8)).sum())
    print(f"   [{tag}] n={int(m.sum())} red={red} green={grn} other={int(m.sum())-red-grn}", flush=True)
    return {"tag": tag, "n": int(m.sum()), "red": red, "green": grn}


rep = []
send("flag experimental_path on", 1.0)
send("timerate rate 0", 1.0)
send("date jday 2461233.5", 1.5)
send("flag moon_scaled off", 1.0)
for f in ("landscape", "atmosphere", "fog", "milky_way", "stars", "star_lines",
          "constellation_drawing", "constellation_art", "constellation_boundaries",
          "constellation_names", "nebula_hints", "nebula_names", "planets_hints",
          "planets_labels", "object_trails", "planets_orbits", "satellites_orbits",
          "star_names", "zodiacal_light", "atmospheric_refraction"):
    send(f"flag {f} off", 0.3)
send("zoom fov 340 duration 0", 2.0)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 3.0)
rep.append(shot("00_base"))
send("flag planets_orbits on", 3.0);            rep.append(shot("01_glob_on"))
send("flag planets_orbits off", 3.0);           rep.append(shot("02_glob_off"))
send("body name Earth orbit true", 3.0);        rep.append(shot("03_earth_on"))
send("body name Mars orbit true", 3.0);         rep.append(shot("04_mars_on"))
send("body name Earth color orbit r 1 g 0 b 0", 2.0); rep.append(shot("05_earth_red"))
send("body name Mars color orbit r 0 g 1 b 0", 2.0);  rep.append(shot("06_mars_green"))
send("body name Mars orbit false", 3.0);        rep.append(shot("07_mars_off"))
send("body name Earth orbit false", 3.0);       rep.append(shot("08_earth_off"))
send("body name Earth orbit true", 3.0);        rep.append(shot("09_earth_on2"))
send("set home_planet Moon", 6.0);              rep.append(shot("10_on_moon"))
send("body name Earth orbit true", 3.0);        rep.append(shot("11_earth_on_moon"))
send("flag planets_orbits on", 3.0);            rep.append(shot("12_glob_on_moon"))
send("flag satellites_orbits on", 3.0);         rep.append(shot("13_sat_on_moon"))
send(f"body action dual_dump filename {OUT}/p_moon.json", 2.5)
send("zoom fov 60 duration 0", 3.0);            rep.append(shot("14_fov60_moon"))
send("flag planets_orbits off", 1.0)
send("flag satellites_orbits off", 1.0)
send("body name Earth orbit true", 3.0);        rep.append(shot("15_pername_fov60"))
json.dump(rep, open(os.path.join(OUT, "probe.json"), "w"), indent=1)
print("done")
