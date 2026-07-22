#!/usr/bin/env python3
# B13 - reference-change / free-mode view-continuity measurement (INTENT 11.61).
# Measures the ABSOLUTE sky-direction delta (root-aligned "absFwd" from the
# Camera dump) and the alt/az-frame delta across:
#   (a) a reference switch between two bodies (set home_planet, warpToBody path),
#   (b) free-mode entry, (c) free-mode exit,  each traversed TWICE (reversible).
# Absolute delta ~0  == the sky direction did not move (Q2/A11).
# alt/az delta != 0   == the DISCRIMINATOR: proves it is the ABSOLUTE direction
#                        that was held, not the frame-relative (alt/az) one.
# Precondition: app fresh-launched, FISHEYE, enable_tcp. init_fov irrelevant
# (mat-layer only, no screen px).
import socket, time, json, math, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b13"
def dpath(tag): return f"{OUT}_{tag}.json"

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def cam(path):
    with open(path) as f:
        return json.loads(f.readline())["camera"]

def ang(a, b):
    na = math.sqrt(sum(x*x for x in a)); nb = math.sqrt(sum(x*x for x in b))
    d = sum(x*y for x, y in zip(a, b)) / (na*nb)
    d = max(-1.0, min(1.0, d))
    return math.degrees(math.acos(d))

def dump(s, tag):
    send(s, f"body action dual_dump filename {dpath(tag)}", 1.2)
    return cam(dpath(tag))

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "set home_planet Earth", 2)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "look_at azimuth 60 altitude 30 duration 0", 1.5)   # aim non-degenerate

seq = []
seq.append(("S0_earth",  dump(s, "S0_earth")))               # ref Earth
send(s, "set home_planet Mars", 2)
seq.append(("S1_mars",   dump(s, "S1_mars")))                # A->B  (ref switch)
send(s, "set home_planet Earth", 2)
seq.append(("S2_earth2", dump(s, "S2_earth2")))              # B->A  (ref switch back)
send(s, "camera action free_mode state on", 1.5)
seq.append(("S3_free_in", dump(s, "S3_free_in")))            # free entry
send(s, "camera action free_mode state off", 1.5)
seq.append(("S4_free_out", dump(s, "S4_free_out")))          # free exit
send(s, "camera action free_mode state on", 1.5)
seq.append(("S5_free_in2", dump(s, "S5_free_in2")))          # free entry 2
send(s, "camera action free_mode state off", 1.5)
seq.append(("S6_free_out2", dump(s, "S6_free_out2")))        # free exit 2
s.close()

print("\n== raw states ==", flush=True)
for tag, c in seq:
    print(f"{tag:12s} ref={c['reference']:10s} mount={c['mount']:10s} "
          f"free={str(c['freeMode']):5s} alt={math.degrees(c['alt']):8.3f} "
          f"az={math.degrees(c['az']):8.3f} hdg={math.degrees(c['heading']):8.3f} "
          f"absFwd=[{c['absFwd'][0]:+.5f},{c['absFwd'][1]:+.5f},{c['absFwd'][2]:+.5f}] "
          f"|absFwd|={math.sqrt(sum(x*x for x in c['absFwd'])):.5f}", flush=True)

def leg(name, a, b):
    ca = dict(seq)[a]; cb = dict(seq)[b]
    dabs = ang(ca['absFwd'], cb['absFwd'])
    dalt = math.degrees(cb['alt'] - ca['alt'])
    daz  = math.degrees(cb['az']  - ca['az'])
    # combined alt/az angular delta (great-circle on the param sphere)
    def fwd(c):
        al, az = c['alt'], c['az']; ca_ = math.cos(al)
        return (math.cos(az)*ca_, -math.sin(az)*ca_, -math.sin(al))
    daltaz = ang(fwd(ca), fwd(cb))
    print(f"{name:26s} absDelta={dabs:9.5f} deg   altazDelta={daltaz:9.5f} deg "
          f"(dAlt={dalt:+8.3f} dAz={daz:+8.3f})", flush=True)
    return dabs, daltaz

print("\n== transition deltas (deg) ==", flush=True)
print("  absDelta   -> should be ~0 after fix (ABSOLUTE sky direction held)", flush=True)
print("  altazDelta -> DISCRIMINATOR: !=0 on ref switch = absolute (not frame) held", flush=True)
r = {}
r['switch_A_to_B'] = leg("ref switch Earth->Mars", "S0_earth", "S1_mars")
r['switch_B_to_A'] = leg("ref switch Mars->Earth", "S1_mars", "S2_earth2")
r['free_enter']    = leg("free-mode enter", "S2_earth2", "S3_free_in")
r['free_exit']     = leg("free-mode exit",  "S3_free_in", "S4_free_out")
r['free_enter2']   = leg("free-mode enter#2", "S4_free_out", "S5_free_in2")
r['free_exit2']    = leg("free-mode exit#2",  "S5_free_in2", "S6_free_out2")

# machine-readable
with open(f"{OUT}_result.json", "w") as f:
    json.dump({k: {"absDelta": v[0], "altazDelta": v[1]} for k, v in r.items()}, f, indent=2)
print(f"\nwrote {OUT}_result.json", flush=True)
