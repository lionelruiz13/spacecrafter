#!/usr/bin/env python3
"""F109 - measure the APP's own collapse threshold against the offline px
formula, by using the dot-suppression arm's edge as the detector
(INTENT 11.231). Runs under b22_live_run.sh like b22_live.py.

WHY. b22_live_analyze.py's aRes axis is computed OFFLINE as
    px = atan(S/sqrt(d^2-S^2))/halfFov*2*viewportRadius
with S transcribed from ModularBody::updateReach as
    S = 1.1*(1.1*(|Neptune.ecl| + 1.1*Neptune.boundingRadius)).
The app's own reach is NOT that expression: updateReach takes
    subsystem = max(boundingRadius, max over VISIBLE children of
                    (|child.ecl| + child.subsystemRadius)),  S = subsystem*1.1
(ModularBody.cpp:652-665) - a different structure, so the transcription can be
off by a percent or so, and a percent of px is 2% of the band's t at its lower
edge. F109 measured an affine term in the interior-only arm,
addI = G*(t_offline + 0.0217) fitted to 0.022 counts per channel per pixel,
which is EXACTLY what a px calibration offset of 0.174 px looks like - and
also exactly what a genuine alpha-independent floor emission would look like.
This driver separates them.

HOW. With SC_F109_G4=1 (the in-band proxy dot suppressed) the dot is drawn if
and only if the app computes px < SYSTEM_VISIBILITY_SUBSYSTEM_SIZE. So the
dot's presence is a STEP FUNCTION of the app's own px, worth ~45000 counts in
the crop - and sampling refDist finely across it brackets the app's threshold
in offline-px units to the sweep's own spacing. No differencing is needed: the
new frame's own crop sum carries the step.

  edge at offline px 16.000  => the offline axis is exact and the 0.0217 is an
                                alpha-INDEPENDENT EMISSION (F109 class A)
  edge at offline px 15.826  => the offline axis is 1.06% low and the 0.0217 is
                                the CALIBRATION (F109 class B, no emission)

Usage (under the runner): b22_live_run.sh f109_edge.py <outdir>
"""
import socket, time, json, math, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "."
AU_M = 149597870700.0
R = 45          # same crop half-size as b22_live_analyze.py

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
send(s, "zoom fov 340 duration 0", 2)
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
send(s, f"moveto altitude {int(sys_aoi*1.2*AU_M)} duration 0", 4)
send(s, "flag track_object off", 2)
send(s, "moveto altitude 12000000000000000 duration 0", 4)
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.005", 3)
send(s, "moveto altitude 185700000000000 duration 0", 4)
send(s, "moveto altitude 185700000000000 duration 0", 4)
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.005", 3)
send(s, "camera action descend coef 0.01675", 3)
dump(s, "band0"); c0 = cam(f"{OUT}/band0.json")
print(f"AFTER APPROACH ref={c0.get('reference')} refDist={c0.get('refDist')} "
      f"selDist={c0.get('selDist')}", flush=True)

def goto(sock, target, tol=0.0005, tries=10):
    """as b22_live.py's, with a 5e-4 tolerance: the edge is being bracketed to
    ~0.01 offline px, i.e. ~1 AU of refDist at this distance."""
    for _ in range(tries):
        dump(sock, "_g"); sel = cam(f"{OUT}/_g.json").get("selDist") or target
        if abs(sel - target) / target < tol:
            return True
        send(sock, f"camera action descend coef {target/sel:.8f}", 2.5)
    dump(sock, "_g"); c = cam(f"{OUT}/_g.json")
    print(f"  !! goto({target}) NOT CONVERGED: selDist={c.get('selDist')}", flush=True)
    return False

# refDist ladder across the offline-px threshold. offline px = K/refDist with
# K = 690.4*S; at S = 36.1553 AU px 16 is refDist 1560.0 AU. The ladder spans
# offline px 15.54 .. 16.32, i.e. +-2.5% of px around T - five times the
# calibration offset under test.
SEQ = [1607, 1597, 1587, 1580, 1575, 1570, 1565, 1560, 1555, 1548, 1530]
rows = []
for i, t in enumerate(SEQ):
    goto(s, t)
    tag = f"e{i:02d}"
    dump(s, tag); cc = cam(f"{OUT}/{tag}.json")
    shot(s, f"{tag}_new")
    rows.append({"tag": tag, "target": t, "reference": cc.get("reference"),
                 "refDist": cc.get("refDist"), "selDist": cc.get("selDist"),
                 "halfFov": cc.get("halfFov"),
                 "solsys_screen": solsys_screen(f"{OUT}/{tag}.json"),
                 "shots": {"new": f"{tag}_new.png"}})
    print(f"  {tag} target={t} ref={cc.get('reference')} refDist={cc.get('refDist')}", flush=True)

json.dump({"sys_sub": sys_sub, "band0": c0, "rows": rows},
          open(f"{OUT}/sweep.json", "w"), indent=1)
print("DONE", flush=True)
