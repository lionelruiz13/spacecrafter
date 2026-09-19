#!/usr/bin/env python3
# Session 33 diagnosis driver: what the new path shows once the Camera stands INSIDE its reference.
import socket, sys, time
OUT = sys.argv[1]
def send(s, cmd, pause=0.8):
    s.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        s.settimeout(0.3); s.recv(65536)
    except socket.timeout:
        pass
    s.settimeout(None); print("%.3f >> %s" % (time.time(), cmd), flush=True)
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0"); send(s, "deselect"); time.sleep(2)
send(s, "set home_planet Jupiter", 5)
send(s, "body action screenshot filename %s/jupiter_new.png" % OUT, 3)
send(s, "flag experimental_path off", 2.5)
send(s, "body action screenshot filename %s/jupiter_old.png" % OUT, 3)
send(s, "flag experimental_path on", 2.5)
print("done", flush=True)
