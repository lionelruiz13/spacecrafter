#!/usr/bin/env python3
# B39 D11 COST CLAIM (§2.0 D11, denominator 1 ms/frame).  D23's stated reason for
# retiring the hidden-body tick is PERFORMANCE, so the claim is mandatory - in
# both directions: the shipped default must not regress, and whatever the
# retirement buys must be measured rather than asserted.
#
# THE INSTRUMENT is the frame counter already in the dump: `evalCount` on a body
# the walks always evaluate increments exactly ONCE PER FRAME.  Over a fixed
# wall-clock dwell with NO commands and NO dumps in between, its delta over the
# dwell IS the frame rate, measured inside the process, with the same instrument
# in both binaries.  Simulated time is FROZEN, so nothing but the frame loop moves.
# The counter is EARTH: it is the observer's reference, so it is evaluated once
# per frame by dispatchUpdate's up-chain and cannot be parked by any command here.
#
# The same dump also counts, exactly, HOW MANY bodies left the tick: a body whose
# evalCount is unchanged across the dwell was not evaluated in ~3000 frames.  That
# is the x-axis of the cost claim, and it is measured, not assumed - hiding one
# planet parks its whole satellite subtree while leaving those satellites' DECLARED
# relation untouched, so the parked-body count cannot be read off the commands.
#
# THREE SCENES:
#   A DEFAULT - nothing hidden beyond what the install ships.  The no-regression
#     leg: it is what every shipped show pays.
#   B four outer planets hidden, with their satellite subtrees.
#   C B + the three other inner planets (Earth stays: it is the reference).
#   A2 everything unhidden again - the repeatability control, and the second entry
#     of the reversible pair at frame-cost level.
# The DRAW saving of hiding a body is identical in both binaries (both stop
# drawing it), so the pre/post difference of (B - A) and (C - A) isolates the tick
# and the sweeps.
#
# usage (one fresh launch per binary):
#   ./b39_run.sh b39_cost.py artifacts/b39_cost_post
#   SC_BIN=<pre-fix binary> ./b39_run.sh b39_cost.py artifacts/b39_cost_pre

import socket, time, json, sys, os

DWELL = 20.0            # seconds of undisturbed rendering per measurement
COUNTER = "Earth"       # the observer's reference: evaluated once per frame
HIDE_B = ["Jupiter", "Saturn", "Uranus", "Neptune"]
HIDE_C = ["Mercury", "Venus", "Mars"]

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "b39_cost")
os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def dump(tag, pause=2.2):
    send(f"body action dual_dump filename {OUT}/c_{tag}.json", pause); return tag


def load(tag):
    out = {}
    with open(os.path.join(OUT, f"c_{tag}.json")) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            d = json.loads(line)
            if d.get("type") == "body" and d.get("new"):
                out[d["name"]] = d["new"]
    return out


def measure(tag):
    a = dump(f"{tag}_t0")
    t0 = time.time()
    time.sleep(DWELL)                       # no commands, no dumps: pure frames
    dt = time.time() - t0
    b = dump(f"{tag}_t1")
    A, B = load(a), load(b)
    frames = B[COUNTER]["evalCount"] - A[COUNTER]["evalCount"]
    frozen = [n for n in B if n in A
              and B[n]["evalCount"] == A[n]["evalCount"]]
    declared = sum(1 for v in B.values() if v["relation"] < 3)
    return {"tag": tag, "frames": frames, "dwell_s": dt,
            "fps": frames / dt, "ms_per_frame": 1000.0 * dt / frames,
            "bodies": len(B), "declared_hidden": declared,
            "not_evaluated": len(frozen)}


send("flag experimental_path on", 1)
send("timerate rate 0", 1)
send("date jday 2461233.5", 1)
send("flag landscape off", 1)
send("flag atmosphere off", 1)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
send("select planet Sun pointer off", 1)
send("flag track_object on", 5)
send("flag track_object off", 1.5)

res = {"dwell_target_s": DWELL, "counter_body": COUNTER,
       "hidden_in_B": HIDE_B, "hidden_in_C": HIDE_B + HIDE_C}
res["A_default"] = measure("A")
for n in HIDE_B:
    send(f"body name {n} hidden true", 1.0)
res["B_outer"] = measure("B")
for n in HIDE_C:
    send(f"body name {n} hidden true", 1.0)
res["C_inner_too"] = measure("C")
for n in HIDE_B + HIDE_C:
    send(f"body name {n} hidden false", 1.0)
res["A2_restored"] = measure("A2")
sock.close()

for k in ("A_default", "B_outer", "C_inner_too", "A2_restored"):
    r = res[k]
    print(f"{k:12s} frames={r['frames']:6d} / {r['dwell_s']:.2f} s -> "
          f"{r['fps']:8.2f} fps = {r['ms_per_frame']:.4f} ms/frame   "
          f"(bodies {r['bodies']}, declared-hidden {r['declared_hidden']}, "
          f"NOT evaluated {r['not_evaluated']})", flush=True)
base = res["A_default"]["ms_per_frame"]
for k in ("B_outer", "C_inner_too", "A2_restored"):
    res[f"delta_ms_{k}"] = res[k]["ms_per_frame"] - base
    print(f"{k:12s} - A_default = {res[f'delta_ms_{k}']:+.4f} ms/frame "
          f"({100.0 * res[f'delta_ms_{k}']:+.2f} % of the 1 ms/frame budget)")
with open(os.path.join(OUT, "b39_cost_result.json"), "w") as f:
    json.dump(res, f, indent=1)
