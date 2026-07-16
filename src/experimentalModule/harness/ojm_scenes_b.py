#!/usr/bin/env python3
# OJM wave - launch B (init_fov 2): OPAQUE_OJM cast A/B. Two station shadows
# on the near-full Moon, tracked from Earth: old Gen-2 (CoI=Moon receiver,
# stations as casters) vs new typed OPAQUE_OJM word - PLUS the two-caster
# composition on one receiver (products commute). Same placements as launch A.
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
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "select planet Moon pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "flag experimental_path on", 3)
send(s, "body action screenshot filename /tmp/cast_new_on.png", 3)
send(s, "flag experimental_shadows off", 3)
send(s, "body action screenshot filename /tmp/cast_new_off.png", 3)
send(s, "flag experimental_shadows on", 3)
send(s, "body action screenshot filename /tmp/cast_new_on2.png", 3)
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/cast_old.png", 3)
send(s, "flag experimental_path on", 2)
s.close()
print("scenes B done", flush=True)
