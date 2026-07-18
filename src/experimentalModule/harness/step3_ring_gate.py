#!/usr/bin/env python3
# Step-3 gate (row-4 ring COLOR + receive, 2026-07-18). Fresh launch.
# Old-path A/B rides vantages local to the Saturn system (D10: the old path
# wedges NVIDIA on distant small-fov CoI scenes - never zoom Earth->Saturn
# in an old phase here).
#  1. Titan vantage (F1 scene): ring color new vs old.
#  2. Iapetus 40000 km inclined vantage: ring color A/B + shadows off/on
#     (isolates ring->planet band AND planet->ring shadow together).
#  3. Ring restore (untracked): bit-class flag cycle.
import socket, time

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag landscape off", 1)
send(s, "flag atmosphere off", 1)
send(s, "flag star_twinkle off", 1)
send(s, "date jday 2462654", 2)
send(s, "timerate rate 0", 1)

# --- Scene 1: Titan vantage ---
send(s, "set home_planet Titan", 5)
send(s, "moveto lat 0 lon 0 alt 100000 duration 0", 2)
send(s, "select planet Saturn pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "flag track_object off", 2)
send(s, "zoom fov 2 duration 0", 3)
send(s, "flag experimental_path on", 3)
send(s, "body action screenshot filename /tmp/r_titan_new.png", 3)
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/r_titan_old.png", 3)
send(s, "flag experimental_path on", 2)

# --- Scene 2: Iapetus inclined vantage ---
send(s, "set home_planet Iapetus", 5)
send(s, "moveto lat 40 lon 0 alt 40000000 duration 0", 3)
send(s, "select planet Saturn pointer off", 1)
send(s, "flag track_object on", 15)
send(s, "flag track_object off", 2)
send(s, "zoom fov 0.6 duration 0", 3)
send(s, "body action screenshot filename /tmp/r_iap_new.png", 3)
send(s, "flag experimental_shadows off", 3)
send(s, "body action screenshot filename /tmp/r_iap_off.png", 3)
send(s, "flag experimental_shadows on", 3)
send(s, "body action screenshot filename /tmp/r_iap_on2.png", 3)
send(s, "flag experimental_path off", 3)
send(s, "body action screenshot filename /tmp/r_iap_old.png", 3)
send(s, "flag experimental_path on", 2)

send(s, "shutdown action now", 2)
s.close()
print("step3 gate scenes done", flush=True)
