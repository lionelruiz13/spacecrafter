#!/usr/bin/env python3
"""Skin-texture seam verification (S6 Textures row, INTENT 11.46).

Contract under test (old Body::createTexSkin/switchMapSkin parity):
  - skin_tex creates/replaces a skin and NEVER activates it (drawn map unchanged);
  - skin_use on requires an existing skin (no-op otherwise);
  - skin_use on/off swaps the drawn color map on BOTH paths;
  - replacing a skin while active deactivates (drawn map reverts);
  - second entry of every reversible pair (on/off/on).
Scene: Mars (BasicMesh) then Earth from the Moon (LayeredMesh mid regime).
"""
import socket, time, sys

def send(sock, cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def shot(sock, tag, pause=3):
    send(sock, f"body action screenshot filename /tmp/skin_{tag}.png", pause)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1)
send(s, "flag atmosphere off", 1)
send(s, "landscape action rotate", 0.2)  # no-op warmup for the channel
send(s, "flag landscape off", 1)
send(s, "select planet Mars", 1)
send(s, "flag track_object on", 15)
send(s, "zoom auto in", 10)

# --- BasicMesh (Mars) ---
send(s, "flag experimental_path on", 2);  shot(s, "base_new")
send(s, "flag experimental_path off", 2); shot(s, "base_old")
# create must not activate (old parity)
send(s, "body name Mars skin_tex bodies/jupiter.png", 4)
shot(s, "created_old")
send(s, "flag experimental_path on", 2);  shot(s, "created_new")
# activate
send(s, "body name Mars skin_use on", 3); shot(s, "on_new")
send(s, "flag experimental_path off", 2); shot(s, "on_old")
# off (pair entry 1)
send(s, "body name Mars skin_use off", 3); shot(s, "off_old")
send(s, "flag experimental_path on", 2);  shot(s, "off_new")
# on/off/on (pair entry 2, ends active)
send(s, "body name Mars skin_use on", 2)
send(s, "body name Mars skin_use off", 2)
send(s, "body name Mars skin_use on", 3); shot(s, "on2_new")
# replace while active -> deactivates (old parity), then activate the new skin
send(s, "body name Mars skin_tex bodies/moon.png", 4); shot(s, "replaced_new")
send(s, "body name Mars skin_use on", 3); shot(s, "moon_new")
send(s, "flag experimental_path off", 2); shot(s, "moon_old")
# no-skin no-op (Venus never got a skin) - survives on both paths
send(s, "body name Venus skin_use on", 1)
# cleanup Mars
send(s, "body name Mars skin_use off", 2)

# --- LayeredMesh (Earth seen from the Moon) ---
send(s, "flag track_object off", 2)
send(s, "set home_planet Moon", 6)
send(s, "moveto lat 10 lon 30 alt 100 duration 0", 2)
send(s, "select planet Earth", 1)
send(s, "flag track_object on", 15)
send(s, "zoom auto in", 10)
send(s, "flag experimental_path on", 2);  shot(s, "earth_base_new")
send(s, "flag experimental_path off", 2); shot(s, "earth_base_old")
send(s, "body name Earth skin_tex bodies/mars.png", 4)
send(s, "body name Earth skin_use on", 3); shot(s, "earth_on_old")
send(s, "flag experimental_path on", 2);  shot(s, "earth_on_new")
send(s, "body name Earth skin_use off", 3); shot(s, "earth_off_new")
send(s, "body name Earth skin_use on", 2)
send(s, "body name Earth skin_use off", 2)  # end clean
s.close()
print("skin scenes done", flush=True)
