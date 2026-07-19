#!/usr/bin/env python3
# Numerically STABLE comet (moderate e) to isolate the port from the
# near-parabolic sungrazer instability. q=0.8 AU, e=0.6 -> a=2.0 AU.
import socket, time, sys

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try: sock.settimeout(0.3); sock.recv(4096)
    except socket.timeout: pass
    sock.settimeout(None); print(f">> {cmd[:55]}", flush=True)

PERIH = "2461200.0"
COMET = ("body action load name TestComet type Comet parent Sun radius 1000 albedo 1 "
    "halo true tex_map bodies/asteroid.png tex_halo bodies/comet.png coord_func comet_orbit "
    f"orbit_TimeAtPericenter {PERIH} orbit_PericenterDistance 0.8 orbit_Eccentricity 0.6 "
    "orbit_ArgOfPericenter 0 orbit_AscendingNode 0 orbit_Inclination 20 "
    "sidereal_period 1033 orbit_visualization_period 1033 close_orbit 1 "
    "apparent_magnitude 5.0 slope 4.0 "
    "dust_tail_trace_JD 1 dust_tail_ejection_force 8000 dust_tail_radius_xx_coef 30000 "
    "dust_tail_radius_x_coef 15000 dust_tail_radius_base_coef 8000 dust_tail_color_red 0.7 "
    "dust_tail_color_green 0.6 dust_tail_color_blue 0.4 "
    "gaz_tail_trace_JD 1 gaz_tail_ejection_force 16000 gaz_tail_radius_xx_coef 12000 "
    "gaz_tail_radius_x_coef 1500 gaz_tail_radius_base_coef 800 gaz_tail_color_red 0.3 "
    "gaz_tail_color_green 0.4 gaz_tail_color_blue 0.9")

jd  = sys.argv[1] if len(sys.argv) > 1 else PERIH
fov = sys.argv[2] if len(sys.argv) > 2 else "60"
tag = sys.argv[3] if len(sys.argv) > 3 else "stab"
noload = (len(sys.argv) > 4 and sys.argv[4] == "noload")

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
if not noload:
    send(s, "flag atmosphere off"); send(s, "flag landscape off")
    send(s, COMET, 2)
send(s, f"date jday {jd}", 1)
send(s, "timerate rate 0", 1)
send(s, "select planet TestComet", 1)
send(s, "flag track_object on", 10)
send(s, f"zoom fov {fov} duration 0", 3)
send(s, "flag experimental_path on", 2)
send(s, f"body action screenshot filename /tmp/{tag}_new.png", 2.5)
send(s, "flag experimental_path off", 2)
send(s, f"body action screenshot filename /tmp/{tag}_old.png", 2.5)
s.close(); print("done", flush=True)
