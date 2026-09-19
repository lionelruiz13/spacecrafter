#!/usr/bin/env python3
# Session 33 diagnosis driver (run through f121_run.sh): the tester's two symptoms, measured on both
# paths from one launch - the Milky Way (screenshots, night sky forced) and selection by command.
import socket, sys, time
OUT = sys.argv[1]
def send(s, cmd, pause=0.8):
    s.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        s.settimeout(0.3); r = s.recv(65536).decode('utf-8', 'replace')
    except socket.timeout:
        r = ''
    s.settimeout(None); print("%.3f >> %s  << %s" % (time.time(), cmd, r.strip()[:200]), flush=True); return r
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0"); send(s, "deselect"); time.sleep(2)
send(s, "body action screenshot filename %s/default_new.png" % OUT, 3)
for f in ("atmosphere off", "fog off", "landscape off", "milky_way on"):
    send(s, "flag " + f)
send(s, "milky_way intensity 8"); time.sleep(3)
send(s, "body action screenshot filename %s/night_new.png" % OUT, 3)
send(s, "flag experimental_path off", 2.5)
send(s, "body action screenshot filename %s/night_old.png" % OUT, 3)
send(s, "flag experimental_path on", 2.5)
send(s, "select planet Moon pointer on", 2)
send(s, "body action screenshot filename %s/selected_new.png" % OUT, 3)
send(s, "body action dual_dump filename %s/final.json" % OUT, 4)
print("done", flush=True)
