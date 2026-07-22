#!/usr/bin/env python3
# OJM wave smoke scene (row 3, 2026-07-16): station (ISS2021) around the Moon
# at the established lunar-eclipse jd. Verifies: OjmLoader deduction + load,
# OJM color rows (plain first), receive (station inside Earth's umbra - the
# whole 2037 km orbit is inside the ~4700 km umbra during totality, so the
# orbital phase does not matter), and the A/B channel on both paths.
import socket, time, sys

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
send(s, "date jday 2461102.98", 1)   # lunar eclipse totality (established scene)
send(s, "timerate rate 0", 1)
# Station: S05.sts idiom, re-parented to the Moon, LLO at 300 km.
send(s, "body action load name station parent Moon type Artificial model_name ISS2021 "
        "radius 10 oblateness 0.0 halo false coord_func ell_orbit albedo 0.9 "
        "rot_periode -1.5 rot_rotation_offset 90.0 rot_obliquity 90.0 rot_equator_ascending_node 0.0 "
        "orbit_epoch 2461102.98 orbit_period 0.0833 orbit_semimajoraxis 2037. orbit_eccentricity 0.0 "
        "orbit_inclination 0.0 orbit_ascendingnode 0.0 orbit_longofpericenter 0.0 orbit_meanlongitude 0.0", 3)
send(s, "set home_planet Moon", 5)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 2)
send(s, "select planet station", 1)  # old-path select syntax (S05.sts idiom; "body" is not a select type)
send(s, "flag track_object on", 15)  # settle (INTENT 11.19c)
send(s, "flag experimental_path on", 3)   # pin NEW path
send(s, "body action screenshot filename /tmp/ojm_new.png", 3)
send(s, "flag experimental_path off", 3)  # pin OLD path
send(s, "body action screenshot filename /tmp/ojm_old.png", 3)
send(s, "flag experimental_path on", 2)
s.close()
print("smoke done", flush=True)
