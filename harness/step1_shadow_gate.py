#!/usr/bin/env python3
# Step-1 gate (per-entry-rows restructure, 2026-07-18): the change is
# algebraically identity (same row values, moved per entry), so the OLD path
# - untouched - is the within-run reference: old-vs-new relationships must
# reproduce the F1/G1 recorded classes on the same three scenes.
#   1. Lunar eclipse 2026-03-03 (jd 2461102.98), Moon tracked fov 0.5:
#      umbra means old-vs-new at the D2 composition class (ratio ~0.88),
#      R-dominant both.
#   2. Solar eclipse 2026-08-12 (jd 2461265.24), Earth from Moon fov 4
#      (ray-march receiver): spot present; shadows off -> on bit-restores.
#   3. Saturn band from ~40000 km above Iapetus (jd 2462654, G8 + clip):
#      winter-hemisphere band present, no summer false band.
# Fresh launch required (S5 harness precondition).
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

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
# Measurement preconditions (INTENT 11.19a): landscape/atmosphere mask the
# compared surface; twinkle/fps overlays are noise.
send(s, "flag landscape off", 1)
send(s, "flag atmosphere off", 1)
send(s, "flag star_twinkle off", 1)
send(s, "flag show_fps off", 1)

# --- Scene 1: lunar eclipse ---
send(s, "date jday 2461102.98", 2)
send(s, "timerate rate 0", 1)
send(s, "select planet Moon pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "zoom fov 0.5 duration 0", 3)
send(s, "flag experimental_path on", 3)
send(s, "body action screenshot filename /tmp/s1_lunar_new.png", 3)
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/s1_lunar_old.png", 3)
send(s, "flag experimental_path on", 2)

# --- Scene 2: solar eclipse from the Moon (ray-march receiver) ---
send(s, "date jday 2461265.24", 2)
send(s, "set home_planet Moon", 5)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 2)
send(s, "select planet Earth pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "zoom fov 4 duration 0", 3)
send(s, "body action screenshot filename /tmp/s1_solar_on.png", 3)
send(s, "flag experimental_shadows off", 3)
send(s, "body action screenshot filename /tmp/s1_solar_off.png", 3)
send(s, "flag experimental_shadows on", 3)
send(s, "body action screenshot filename /tmp/s1_solar_on2.png", 3)

# --- Scene 3: Saturn G8 band from above Iapetus ---
send(s, "date jday 2462654", 2)
send(s, "set home_planet Iapetus", 5)
send(s, "moveto lat 40 lon 0 alt 40000000 duration 0", 3)
send(s, "select planet Saturn pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "zoom fov 0.6 duration 0", 3)
send(s, "body action screenshot filename /tmp/s1_ring_new.png", 3)
send(s, "flag experimental_shadows off", 3)
send(s, "body action screenshot filename /tmp/s1_ring_off.png", 3)
send(s, "flag experimental_shadows on", 3)
send(s, "body action screenshot filename /tmp/s1_ring_on2.png", 3)

send(s, "shutdown action now", 2)
s.close()
print("step1 gate scenes done", flush=True)
