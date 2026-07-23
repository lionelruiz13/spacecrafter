#!/usr/bin/env python3
"""B22 deliverable 1 - LIVE band-crossing reversible pair + monotone alphas
(INTENT 11.64 / 11.80 / 13.B B22). Fresh launch, FISHEYE, init_fov=340.

The universe-executor collapse band renders live since §11.80 (b5 uniband leg).
This driver navigates there, DESCEND-sweeps a fixed set of refDist points that
span below-band -> band [16,24)px -> above-band (capturing NEW and OLD phase at
each), then ASCEND-sweeps the SAME points (new phase) - the reversible pair.

Geometry (measured, offline-reproducible): the SolarSystem node's own screenSize
is 0 (bare system node, §11.80 defect 1); drawNested classifies px from the
SUBSYSTEM: px = atan(S/sqrt(d^2-S^2))/halfFov*2*viewportRadius, S=subsystemRadius
(b5 transcription 1.1*(1.1*(|Neptune ecl|+1.1*Neptune.boundingRadius))), d=node
distance == the dump's refDist here, viewportRadius=render_size/2=1024. At fov 340
band [16,24)px maps to refDist ~ (1040,1560) AU (1284 AU = px 19.4).

Isolation: the composed screen is dominated by fixed old-path galactic content
(milkyway/dso3d/tully/catalog Sol) that draws in BOTH phases; NEW-vs-OLD
differencing cancels it and isolates the drawNested contribution (interior/dot),
the b5 method. analyze with b22_live_analyze.py.

Reference stays MilkyWay throughout the whole sweep (the band is well above the
SolarSystem AoI ~578 AU, so no capture into SolarSystem) - asserted per step.
"""
import socket, time, json, math, sys, os
OUT = sys.argv[1] if len(sys.argv) > 1 else "."
AU_M = 149597870700.0

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout: pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)
def cam(p):
    with open(p) as f: return json.loads(f.readline())["camera"]
def bodies(p):
    out = {}
    for line in open(p):
        d = json.loads(line)
        if d.get("type") == "body" and d.get("new"): out[d["name"]] = d["new"]
    return out
def solsys_screen(p):
    for line in open(p):
        d = json.loads(line)
        if d.get("type") == "body" and d.get("name") == "SolarSystem" and d.get("new"):
            return d["new"].get("screen")
    return None
def shot(sock, nm, pause=2.5): send(sock, f"body action screenshot filename {OUT}/{nm}.png", pause)
def dump(sock, nm, pause=2.0): send(sock, f"body action dual_dump filename {OUT}/{nm}.json", pause)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "zoom fov 340 duration 0", 2)          # force the harness fov (config may drift)
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
dump(s, "base")
b = bodies(f"{OUT}/base.json")
n = lambda v: math.sqrt(sum(x*x for x in v))
sys_sub = 1.1 * (1.1 * (n(b["Neptune"]["ecl"]) + 1.1*b["Neptune"]["boundingRadius"]))
sys_aoi = sys_sub * 16
send(s, "select planet Sun", 1)
send(s, "flag track_object on", 4)
send(s, "flag atmosphere off", 2)
send(s, "camera action free_mode state on", 1)
send(s, f"moveto altitude {int(sys_aoi*1.2*AU_M)} duration 0", 4)   # gal (solar exec)
send(s, "flag track_object off", 2)
send(s, "moveto altitude 12000000000000000 duration 0", 4)          # ->InGalaxy
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.005", 3)
send(s, "moveto altitude 185700000000000 duration 0", 4)            # ->InUniverse
send(s, "moveto altitude 185700000000000 duration 0", 4)            # clear entry fade
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.01675", 3)
dump(s, "band0"); c0 = cam(f"{OUT}/band0.json")
print(f"AT BAND ref={c0.get('reference')} refDist={c0.get('refDist')} selDist={c0.get('selDist')}", flush=True)

# below-band -> band -> above-band. px=K/refDist ~ px16@1560AU px24@1040AU at fov340.
SEQ = [1750, 1650, 1591, 1520, 1450, 1400, 1340, 1280, 1200, 1120, 1061, 1010, 960, 910]

def goto(sock, target):                          # descend/ascend aims Sun; selDist scales *coef exactly
    dump(sock, "_g"); sel = cam(f"{OUT}/_g.json").get("selDist") or target
    send(sock, f"camera action descend coef {target/sel:.6f}", 2.5)

rows = []
def capture(sock, tag, direction, target, phases):
    dump(sock, tag); cc = cam(f"{OUT}/{tag}.json")
    rec = {"tag": tag, "dir": direction, "target": target,
           "reference": cc.get("reference"), "refDist": cc.get("refDist"),
           "selDist": cc.get("selDist"), "halfFov": cc.get("halfFov"),
           "solsys_screen": solsys_screen(f"{OUT}/{tag}.json"), "shots": {}}
    for ph in phases:
        send(sock, f"flag experimental_path {'on' if ph=='new' else 'off'}", 2.0)
        shot(sock, f"{tag}_{ph}"); rec["shots"][ph] = f"{tag}_{ph}.png"
    send(sock, "flag experimental_path on", 1.0)
    rows.append(rec)
    print(f"  {tag}[{direction}] target={target} ref={rec['reference']} refDist={rec['refDist']}", flush=True)

for i, t in enumerate(SEQ):                       # DESCEND: px increasing, new+old
    goto(s, t); capture(s, f"dn{i:02d}", "descend", t, ("new", "old"))
for i, t in enumerate(reversed(SEQ)):             # ASCEND: revisit same points, new (reversible pair)
    j = len(SEQ) - 1 - i
    goto(s, t); capture(s, f"up{j:02d}", "ascend", t, ("new",))

json.dump({"sys_sub": sys_sub, "band0": c0, "rows": rows},
          open(f"{OUT}/sweep.json", "w"), indent=1)
print("DONE", flush=True)
