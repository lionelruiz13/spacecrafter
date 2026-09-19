#!/usr/bin/env python3
# GATE - a reference change keeps the place on BOTH engines (INTENT S11.258).
# Driver for f121_run.sh:   ./f121_run.sh <label> s33_refchange.py [binary]
# Line of execution validated: `set home_planet X` -> SSystemFactory::syncCameraReference ->
# Camera (new path) beside Observer (old path). At every step the altitude the new path reports
# must equal the old observer's, and the Camera must not stand inside its reference.
# It failed on code a594b31a by 2 308 785 m: warpToBody had gone back to the owner's raw primitive
# and its callers still relied on the altitude re-basing an agent had put inside it.
import socket, sys, time, json
OUT = sys.argv[1]
TOL_M = 1.0          # the two engines hold the altitude in float32 AU / double m: a few cm at these scales
AU_KM = 149597870.7
fails = []

def send(s, cmd, pause=0.8):
    s.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        s.settimeout(0.3); s.recv(65536)
    except socket.timeout:
        pass
    s.settimeout(None); print("%.3f >> %s" % (time.time(), cmd), flush=True)

def check(tag):
    p = "%s/%s.json" % (OUT, tag); send(s, "body action dual_dump filename " + p, 4)
    h = json.loads(open(p).readline()); c = h["camera"]; alt = h["control"]["altitude"]
    d = abs(alt["new"] - alt["old"])
    ok = d <= TOL_M and alt["new"] > -TOL_M
    print("%s %-14s ref=%-6s centre distance %.1f km | altitude new %.2f m, old %.2f m, |delta| %.3g m"
          % ("PASS" if ok else "FAIL", tag, c["reference"], c["distance"] * AU_KM, alt["new"], alt["old"], d), flush=True)
    if not ok:
        fails.append(tag)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "timerate rate 0"); send(s, "deselect"); time.sleep(2)
check("a_earth")
send(s, "set home_planet Moon", 4);      check("b_moon")
send(s, "moveto alt 100 duration 0", 3); check("c_moon_low")
send(s, "set home_planet Earth", 4);     check("d_earth_back")
send(s, "set home_planet Jupiter", 4);   check("e_jupiter")
send(s, "set home_planet Earth", 4);     check("f_earth_again")
print("=== ALL PASS ===" if not fails else "=== FAILURES: %s ===" % ",".join(fails), flush=True)
sys.exit(1 if fails else 0)
