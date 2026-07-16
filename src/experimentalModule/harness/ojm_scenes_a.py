#!/usr/bin/env python3
# OJM wave - launch A (init_fov 60): self-shadow first light + plain-row A/B.
# Geometry: two stations parked 8000/8300 km sunward of the Moon at jd
# 2461103.30 (post-eclipse, phase angle 3.9 deg - sunlit by construction);
# observer ON station (S05 home_planet idiom). Deterministic: frozen time,
# fresh launch, placements from the measured Moon position (lon 166.699,
# lat -0.745, r 383867 km) via the calibrated ell_orbit mapping
# (meanlongitude == geocentric ecliptic longitude at i=0).
import socket, time

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

ORBIT = ("rot_periode 1000 rot_rotation_offset 0 rot_obliquity 0 rot_equator_ascending_node 0 "
         "orbit_epoch 2461103.30 orbit_period 27.3 orbit_eccentricity 0.0 "
         "orbit_inclination 0.76 orbit_ascendingnode 256.70 orbit_longofpericenter 0.0 ")

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "date jday 2461103.30", 1)
send(s, "timerate rate 0", 1)
send(s, "body action load name station parent Earth type Artificial model_name ISS2021 "
        "radius 10 oblateness 0.0 halo false coord_func ell_orbit albedo 0.9 "
        + ORBIT + "orbit_semimajoraxis 375900. orbit_meanlongitude 166.70", 3)
send(s, "body action load name sidekick parent Earth type Artificial model_name ISS2021 "
        "radius 10 oblateness 0.0 halo false coord_func ell_orbit albedo 0.9 "
        + ORBIT + "orbit_semimajoraxis 376200. orbit_meanlongitude 166.75", 3)
send(s, "set home_planet station", 5)
send(s, "moveto lat 0 lon 0 alt 60000 duration 0", 3)
# --- self-shadow (new): shadows on -> off -> on differential ---
send(s, "flag experimental_path on", 3)
send(s, "body action screenshot filename /tmp/self_new_on.png", 3)
send(s, "flag experimental_shadows off", 3)
send(s, "body action screenshot filename /tmp/self_new_off.png", 3)
send(s, "flag experimental_shadows on", 3)
send(s, "body action screenshot filename /tmp/self_new_on2.png", 3)
# --- self-shadow (old CoI machinery) ---
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/self_old.png", 3)
send(s, "flag experimental_path on", 2)
# --- plain-row A/B: sidekick tracked from the station ---
send(s, "select planet sidekick pointer off", 1)
send(s, "flag track_object on", 12)
send(s, "body action screenshot filename /tmp/plain_new.png", 3)
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/plain_old.png", 3)
send(s, "flag experimental_path on", 2)
s.close()
print("scenes A done", flush=True)
