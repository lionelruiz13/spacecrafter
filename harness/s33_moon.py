#!/usr/bin/env python3
# Session 33 diagnosis driver: the state AFTER a reference change (set home_planet Moon), both paths.
import socket, sys, time
OUT = sys.argv[1]
def send(s, cmd, pause=0.8):
    s.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        s.settimeout(0.3); r = s.recv(65536).decode('utf-8', 'replace')
    except socket.timeout:
        r = ''
    s.settimeout(None); print("%.3f >> %s  << %s" % (time.time(), cmd, r.strip()[:160]), flush=True); return r
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0"); send(s, "deselect"); time.sleep(2)
send(s, "set home_planet Moon", 4)
send(s, "body action screenshot filename %s/moon_new.png" % OUT, 3)
send(s, "flag experimental_path off", 2.5)
send(s, "body action screenshot filename %s/moon_old.png" % OUT, 3)
send(s, "flag experimental_path on", 2.5)
send(s, "body action dual_dump filename %s/final.json" % OUT, 4)
print("done", flush=True)
