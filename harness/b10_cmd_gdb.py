#!/usr/bin/env python3
# B10-cmd swallow-guard driver (INTENT 11.79(e) D9key spelling proof). Sends a
# known set of datum_radius/ground_radius commands over TCP; the gdb probe
# (b10_cmd_probe.gdb) counts entries into the routing seam. Expected:
#   3 valid `datum_radius` commands  -> 3 setBodyDatumRadius PROBE lines
#   3 valid `ground_radius` commands -> 3 setBodyGroundRadius PROBE lines
#   4 BOGUS-spelling commands        -> 0 additional PROBE lines (swallowed)
# The bogus set includes Q12's REJECTED word order (`radius_datum`/`radius_ground`)
# - proving the D9key data-word-order spelling lands and the rejected one does not.
import socket, sys, time, os

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b10cmdgdb"
os.makedirs(OUT, exist_ok=True)

def send(sock, cmd, pause=1.2):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "body action load name GdbTest radius 6000 parent Sun type Planet "
        "halo false coord_func still_orbit orbit_x 900 orbit_y 0 orbit_z 0", 2)

print("--- 3 VALID datum_radius (expect 3 datum PROBE) ---", flush=True)
send(s, "body name GdbTest datum_radius 0")
send(s, "body name GdbTest datum_radius 6000")
send(s, "body name GdbTest datum_radius 3000")

print("--- 3 VALID ground_radius (expect 3 ground PROBE) ---", flush=True)
send(s, "body name GdbTest ground_radius 0")
send(s, "body name GdbTest ground_radius 6000")
send(s, "body name GdbTest ground_radius 6012")

print("--- 4 BOGUS spellings (expect 0 PROBE) ---", flush=True)
send(s, "body name GdbTest radius_datum 6000")    # Q12's REJECTED word order
send(s, "body name GdbTest radius_ground 6000")   # Q12's REJECTED word order
send(s, "body name GdbTest datumradius 6000")     # missing underscore
send(s, "body name GdbTest datum 6000")           # partial word

print("b10-cmd swallow-guard driver done", flush=True)
s.close()
