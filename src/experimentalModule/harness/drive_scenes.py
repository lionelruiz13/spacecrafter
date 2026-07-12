#!/usr/bin/env python3
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
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1)
send(s, "select planet Moon", 1)
send(s, "flag track_object on", 3)
send(s, "body action dual_dump filename /tmp/gen_a.json", 2)
send(s, "moveto lat 48.85 lon 2.35 alt 50000 duration 0", 3)
send(s, "body action dual_dump filename /tmp/gen_b.json", 2)
send(s, "set home_planet Moon", 5)
send(s, "moveto lat 10 lon 30 alt 100 duration 0", 2)
send(s, "select planet Earth", 1)
send(s, "flag track_object on", 3)
send(s, "body action dual_dump filename /tmp/gen_moon.json", 3)
# Scene D [vixy 2026-07-12]: observer ON Mars, view tracking Earth's Moon -
# quadruplet (Mars, Sun, Earth, Moon). First CROSS-BRANCH reference scenario:
# ref chain Mars->Sun, target chain Moon->Earth->Sun, common parent = Sun
# (up-hop then two down-hops - never composed from the reference side before).
# Two dates, 88 days apart, to separate time-dependent from constant errors.
send(s, "set home_planet Mars", 5)
send(s, "moveto lat 10 lon 30 alt 100 duration 0", 2)
send(s, "select planet Moon", 1)
send(s, "flag track_object on", 3)
send(s, "body action dual_dump filename /tmp/gen_mars.json", 3)
send(s, "date jday 2461321.5", 3)
send(s, "body action dual_dump filename /tmp/gen_mars_2.json", 3)
s.close()
print("scenes done", flush=True)
