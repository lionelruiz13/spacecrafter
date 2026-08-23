#!/usr/bin/env python3
# F29 / INTENT §5.46 - the reversible pair, entered TWICE.
#
# The pair this fix touches is a body's membership of the walk: DESCENT (the
# frame is written by transformParentToBodyPos / recursiveUpdate) <-> UP-CHAIN
# (the frame is written by the fixed site). A reference switch moves the subject
# across it in both directions, so the pair is driven Earth -> Moon -> Earth ->
# Moon -> Earth, each entry starting from the state the previous exit produced.
#
# At every state, two things are asserted, so a latch or a leftover would show:
#   (1) the P1 invariant - eclRoot == mat[12:15] EXACTLY - over every body;
#   (2) the subject's own frame is the observer-to-subject vector, i.e. its
#       magnitude tracks the state (~6.4e3 km standing on Earth, ~4e5 km from
#       the Moon) instead of being carried across the switch.
#
#   ./f29_run.sh <outdir> f29_reversible.py
import socket, time, json, sys, os, math

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "f29", "rev")
os.makedirs(OUT, exist_ok=True)
SUBJECT = "Earth"
AU_KM = 149597870.7
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def state(tag):
    send(f"body action dual_dump filename {OUT}/r_{tag}.json", 2.2)
    bodies = {}
    for line in open(os.path.join(OUT, f"r_{tag}.json")):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "body" and d.get("new"):
            bodies[d["name"]] = d["new"]
    bad = []
    for name, b in bodies.items():
        e, m = b.get("eclRoot"), b.get("mat")
        if e is None or m is None:
            continue
        if any(float(e[i]) != float(m[12 + i]) for i in range(3)):
            bad.append(name)
    s = bodies[SUBJECT]
    r = math.sqrt(sum(v * v for v in s["eclRoot"])) * AU_KM
    return {"tag": tag, "n_bodies": len(bodies), "violators": sorted(bad),
            "subject_frame_km": r}


send("flag experimental_path on", 1.0)
send("timerate rate 0", 1.0)
send("date jday 2461233.5", 1.5)
send("flag moon_scaled off", 1.0)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 3.0)

rows = [state("earth0")]
for entry in (1, 2):
    send("set home_planet Moon", 6.0);  rows.append(state(f"moon{entry}"))
    send("set home_planet Earth", 6.0); rows.append(state(f"earth{entry}"))

fail = []
for r in rows:
    if r["violators"]:
        fail.append(f"{r['tag']}: invariant violated by {r['violators']}")
    on_moon = r["tag"].startswith("moon")
    if on_moon and not (3.0e5 < r["subject_frame_km"] < 5.0e5):
        fail.append(f"{r['tag']}: subject frame {r['subject_frame_km']:.0f} km "
                    "is not an Earth-Moon distance")
    if not on_moon and not (6.0e3 < r["subject_frame_km"] < 7.0e3):
        fail.append(f"{r['tag']}: subject frame {r['subject_frame_km']:.0f} km "
                    "is not an Earth-surface distance")
# the two entries must AGREE - a latch would make the second differ
for a, b in (("moon1", "moon2"), ("earth1", "earth2")):
    ra = next(r for r in rows if r["tag"] == a)
    rb = next(r for r in rows if r["tag"] == b)
    if abs(ra["subject_frame_km"] - rb["subject_frame_km"]) > 1.0:
        fail.append(f"{a} vs {b}: subject frame differs by "
                    f"{abs(ra['subject_frame_km'] - rb['subject_frame_km']):.3f} km")

for r in rows:
    print(f"{r['tag']:8s} bodies={r['n_bodies']:3d} violators={r['violators'] or '[]'}  "
          f"subject frame = {r['subject_frame_km']:12.3f} km")
json.dump({"rows": rows, "fail": fail}, open(os.path.join(OUT, "f29_rev.json"), "w"), indent=1)
print(("ALL PASS" if not fail else "FAIL: " + "; ".join(fail)))
sys.exit(1 if fail else 0)
