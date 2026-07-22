#!/usr/bin/env python3
# B15 - AoI re-derivation on date change (INTENT 11.62).
# Measures the launch-jd AoI latch at the FORMULA layer (B30-safe: AoI is a
# computed scalar, not a pixel). The app's live Earth AoI is dumped as
# camera.refAoI (reference=Earth); the PREDICTED AoI is recomputed offline from
# the SAME dump's fresh body positions (ecl updates every frame; only
# areaOfInfluence was latched). error% = |refAoI - predicted| / predicted.
#
# Pre-fix: refAoI is frozen at LAUNCH jd (startup_time_mode=Actual ~ real now)
# while the dumps are taken at jumped dates -> error grows with the jump.
# Post-fix: AoI tracks the current jd -> error ~ 0 at every jumped date.
#
# Precondition: app fresh-launched, init_fov=340, fisheye, enable_tcp, DISPLAY=:2.
import socket, time, json, math, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp"

def send(sock, cmd, pause=0.5):
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

def bodies(path):
    out = {}
    with open(path) as f:
        for line in f:
            d = json.loads(line)
            if d.get("type") == "body" and d.get("new"):
                out[d["name"]] = d["new"]
    return out

n = lambda v: math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])

def predict_earth_aoi(b):
    # Transcription of ModularBody::updateCache / updateReach (scaling=1):
    #   subsystem = max(bounding, max_child(|ecl_child| + subsys_child))
    #   subsystemRadius = subsystem * 1.1
    #   aoi = max(bounding*128, subsystemRadius*16) capped by |ecl|*0.6 iff |ecl|>0
    moon_sub  = 1.1 * b["Moon"]["boundingRadius"]
    earth_sub = 1.1 * max(b["Earth"]["boundingRadius"], n(b["Moon"]["ecl"]) + moon_sub)
    aoi = max(b["Earth"]["boundingRadius"]*128, earth_sub*16)
    cap = n(b["Earth"]["ecl"])*0.6
    if cap > 0:
        aoi = min(aoi, cap)
    return aoi

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "timerate rate 0", 1)

SCENE_JD = 2461233.5
HALF_YR  = 182.62   # ~half a Julian year

legs = [
    ("launch", None),        # dump BEFORE any date jump: positions at launch jd
    ("scene",  SCENE_JD),    # the scene-E jd
    ("half",   SCENE_JD + HALF_YR),   # +1/2 year (seasonal case)
    ("back",   SCENE_JD),    # reversible pair: back to scene jd
    ("fwd2",   SCENE_JD + HALF_YR),   # second forward jump from the back state
]
rows = []
for tag, jd in legs:
    if jd is not None:
        send(s, f"date jday {jd}", 1.5)
    send(s, f"body action dual_dump filename /tmp/b15_{tag}.json", 1.5)
s.close()

print("\n== AoI staleness (Earth reference) ==", flush=True)
for tag, jd in legs:
    c = cam(f"/tmp/b15_{tag}.json")
    b = bodies(f"/tmp/b15_{tag}.json")
    app_aoi = c["refAoI"]
    pred = predict_earth_aoi(b)
    moon_d = n(b["Moon"]["ecl"])
    err = abs(app_aoi - pred) / pred * 100.0 if pred else float('nan')
    print(f"{tag:8s} ref={c['reference']:6s} appAoI={app_aoi:.6e}  "
          f"pred={pred:.6e}  moonDist={moon_d:.6e}  err={err:6.3f}%", flush=True)
    rows.append((tag, jd, app_aoi, pred, moon_d, err))

with open(f"{OUT}/b15_stale.json", "w") as f:
    json.dump([{"tag": t, "jd": j, "appAoI": a, "predAoI": p,
                "moonDist": m, "errPct": e} for t, j, a, p, m, e in rows], f, indent=1)
print(f"\nwrote {OUT}/b15_stale.json", flush=True)
