#!/usr/bin/env python3
"""B16 - the two 2(c) channels, both exercised on one fresh launch.

  channel 1 (scriptless / live): `body action reload` over TCP
  channel 2 (script):            `script action play filename
                                  b16_reload_check.sts`, whose single
                                  non-comment line is `body action reload`
Both must reach SSystemFactory::reloadCurrentSystem - the gdb breakpoint
counts the entries (2 expected), independently of the app's own log.
PRECONDITION: copy b16_reload_check.sts (this directory) into the script dir
(~/.spacecrafter/scripts/) before the run, and remove it afterwards - the
user data directory is left as found.
Usage: b16_channels.py <outdir>
"""
import socket, sys, time, os

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)


def send(sock, cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "date jday 2461233.5")
send(s, "timerate rate 0")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "body action dual_dump filename %s/ch_pre.json" % OUT, 3)
print("--- channel 1: live command (TCP) ---", flush=True)
send(s, "body action reload", 5)
send(s, "body action dual_dump filename %s/ch_live.json" % OUT, 3)
print("--- channel 2: script ---", flush=True)
send(s, "script action play filename b16_reload_check.sts", 8)
send(s, "body action dual_dump filename %s/ch_script.json" % OUT, 3)
print("b16 channels done", flush=True)
s.close()
