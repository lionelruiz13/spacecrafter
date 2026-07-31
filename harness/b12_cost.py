#!/usr/bin/env python3
# B12 D11 COST CLAIM (§2.0 D11, denominator 1 ms/frame). A new shader family
# must state and measure its budget claim; this measures it in BOTH regimes,
# with the SAME instrument in both binaries.
#
# THE INSTRUMENT is b39_cost.py's, unchanged: `evalCount` on the observer's
# reference body increments exactly once per frame, so its delta over an
# undisturbed wall-clock dwell (frozen sim time, no commands, no dumps) IS the
# frame count, measured inside the process.
#
# THREE SCENES, chosen so the family's engagement is the only thing that moves:
#   FAR   fisheye 180 from Earth: the Sun is ~3 px - the depth-less mid band.
#         (Below screenSize 0.0015 no surface module is invoked AT ALL, which is
#         a code-level zero, not a measurement - ModularBody::draw.)
#   NEAR  zoom fov 2 on the tracked Sun: the disc is 537 px across and the
#         big-texture branch is engaged - the family's most expensive shipped
#         case at 1 AU.
#   NOSUN the same near framing with the Sun HIDDEN: the family is gated off
#         while everything else about the frame is held, so (NEAR - NOSUN) is
#         what the star's surface costs, per binary.
#
# usage (one fresh launch per binary):
#   ./b12_run.sh ... is the capture runner; for cost use b39_run.sh's shape:
#   SC_BIN=<pre> ./b12_cost_run.sh artifacts/b12_cost_pre
#                ./b12_cost_run.sh artifacts/b12_cost_post
import socket, time, json, sys, os

DWELL = 20.0
COUNTER = "Earth"

OUT = os.path.abspath(sys.argv[1])
os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def load(tag):
    out = {}
    with open(os.path.join(OUT, f"c_{tag}.json")) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            d = json.loads(line)
            if d.get("type") == "body" and isinstance(d.get("new"), dict):
                out[d["name"]] = d["new"]
    return out


def measure(tag):
    send(f"body action dual_dump filename {OUT}/c_{tag}_t0.json", 2.2)
    t0 = time.time()
    time.sleep(DWELL)
    dt = time.time() - t0
    send(f"body action dual_dump filename {OUT}/c_{tag}_t1.json", 2.2)
    A, B = load(f"{tag}_t0"), load(f"{tag}_t1")
    frames = B[COUNTER]["evalCount"] - A[COUNTER]["evalCount"]
    return {"tag": tag, "frames": frames, "dwell_s": round(dt, 3),
            "fps": round(frames / dt, 3),
            "ms_per_frame": round(1000.0 * dt / frames, 5),
            "sun_screenSize": B.get("Sun", {}).get("screenSize")}


send("flag experimental_path on", 1)
send("timerate rate 0", 1)
send("date jday 2461233.5", 1)
send("flag landscape off", 1)
send("flag atmosphere off", 1)
send("flag show_fps off", 1)
send("flag planet_names off", 1)
send("set home_planet Earth", 3)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
send("select planet Sun pointer off", 1)
send("flag track_object on", 6)

res = {"dwell_target_s": DWELL, "counter_body": COUNTER}
send("zoom fov 180 duration 0", 3)
res["FAR"] = measure("far")
send("zoom fov 2 duration 0", 3)
res["NEAR"] = measure("near")
send("body name Sun hidden true", 2)
res["NOSUN"] = measure("nosun")
send("body name Sun hidden false", 2)
res["NEAR2"] = measure("near2")     # reversible pair, second entry
send("zoom fov 180 duration 0", 2)
sock.close()

res["near_minus_nosun_ms"] = round(res["NEAR"]["ms_per_frame"] - res["NOSUN"]["ms_per_frame"], 5)
res["near2_minus_nosun_ms"] = round(res["NEAR2"]["ms_per_frame"] - res["NOSUN"]["ms_per_frame"], 5)
print(json.dumps(res, indent=2))
with open(os.path.join(OUT, "b12_cost.json"), "w") as f:
    json.dump(res, f, indent=2)
