#!/usr/bin/env python3
# B39 D11 COST CLAIM, on the CPU stage the change actually touches (§2.0 D11,
# denominator 1 ms/frame).  Companion of b39_cost.py, which measured THROUGHPUT
# and hit its own floor: the shipped config limits the loop to 161.3 fps, so the
# frame time is quantized at 1/3226 of the dwell = 0.0019 ms and the whole effect
# sits inside one quantum.  This driver removes the limiter and measures the
# EXECUTOR_UPDATE segment directly - the CPU stage that contains the update walk,
# i.e. the tick the change retires.
#
# TEMP-HOME FARM (§11.103(a)): the field $HOME/.spacecrafter is copied once, and
# the two acting defaults this measurement needs are set IN THE COPY ONLY -
# `maximum_fps = 10000` (the limiter is what floors b39_cost.py) and
# `query_statistics = true` (the segment log is off by default).  Both are logged
# here rather than assumed (D12), and the field install is never written.
#
# INSTRUMENT: log/statistics.dat, {int32 type; float32 secondsSinceFrameStart}
# pairs, parsed by b22_parse_stats (FADER_UPDATE -> EXECUTOR_UPDATE = the update
# stage).  The file is a RING BUFFER, so it is snapshotted after each dwell and
# each snapshot's tail is analysed on its own.
#
# SCENES: as b39_cost.py - A default, B four outer planets hidden with their
# satellite subtrees, C plus the three other inner planets, A2 restored.  The
# bodies that actually stopped being evaluated are COUNTED from evalCount, so the
# x-axis of the claim is measured too.
#
#   ./b39_cost_farm.py <outdir>              # current build
#   SC_BIN=<pre-fix binary> ./b39_cost_farm.py <outdir>

import json, os, shutil, socket, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b22_parse_stats as PS

HERE = Path(__file__).resolve().parent
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
FARM = Path(os.environ.get("B39_FARM", "/tmp/b39_farm"))
REAL = Path.home() / ".spacecrafter"
DWELL = 25.0
COUNTER = "Earth"
HIDE_B = ["Jupiter", "Saturn", "Uranus", "Neptune"]
HIDE_C = ["Mercury", "Venus", "Mars"]

OUT = Path(sys.argv[1] if len(sys.argv) > 1 else HERE / "artifacts/b39_cost_farm").resolve()
OUT.mkdir(parents=True, exist_ok=True)
farm_home = FARM / ".spacecrafter"
if not farm_home.exists():
    farm_home.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(REAL, farm_home, symlinks=True)
    print(f"farm created from {REAL}", flush=True)
cfg = (farm_home / "config.ini").read_text(encoding="latin-1").splitlines(keepends=True)
cfg = ["query_statistics               = true\n" if l.startswith("query_statistics")
       else ("maximum_fps                    = 10000\n" if l.startswith("maximum_fps") else l)
       for l in cfg]
(farm_home / "config.ini").write_text("".join(cfg), encoding="latin-1")
print("ACTING DEFAULTS set in the FARM COPY only (D12): query_statistics = true, "
      "maximum_fps = 10000", flush=True)
stats = farm_home / "log/statistics.dat"
stats.unlink(missing_ok=True)

env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
proc = subprocess.Popen([SC_BIN], cwd=str(farm_home),
                        stdout=open(OUT / "app.log", "w"),
                        stderr=subprocess.STDOUT, env=env)
sock = None
res = {"binary": SC_BIN, "dwell_target_s": DWELL, "counter_body": COUNTER,
       "hidden_in_B": HIDE_B, "hidden_in_C": HIDE_B + HIDE_C}
try:
    for _ in range(60):
        time.sleep(2)
        try:
            sock = socket.create_connection(("127.0.0.1", 7805), timeout=5)
            break
        except OSError:
            if proc.poll() is not None:
                raise SystemExit("app died before tcp")
    time.sleep(10)

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
        for line in open(OUT / f"c_{tag}.json"):
            line = line.strip()
            if not line:
                continue
            d = json.loads(line)
            if d.get("type") == "body" and d.get("new"):
                out[d["name"]] = d["new"]
        return out

    def measure(tag):
        a = dump(f"{tag}_t0")
        t0 = time.time(); time.sleep(DWELL); dt = time.time() - t0
        shutil.copy(stats, OUT / f"stats_{tag}.dat")
        b = dump(f"{tag}_t1")
        A, B = load(a), load(b)
        frames = B[COUNTER]["evalCount"] - A[COUNTER]["evalCount"]
        frozen = [n for n in B if n in A and B[n]["evalCount"] == A[n]["evalCount"]]
        seg = PS.report(str(OUT / f"stats_{tag}.dat"), tail=2000)
        return {"tag": tag, "frames": frames, "dwell_s": dt, "fps": frames / dt,
                "ms_per_frame": 1000.0 * dt / frames, "bodies": len(B),
                "declared_hidden": sum(1 for v in B.values() if v["relation"] < 3),
                "not_evaluated": len(frozen), "stages": seg}

    send("flag experimental_path on", 1)
    send("timerate rate 0", 1)
    send("date jday 2461233.5", 1)
    send("flag landscape off", 1)
    send("flag atmosphere off", 1)
    send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
    send("select planet Sun pointer off", 1)
    send("flag track_object on", 5)
    send("flag track_object off", 1.5)
    time.sleep(8)

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
finally:
    if sock:
        sock.close()
    proc.terminate()
    try:
        proc.wait(10)
    except subprocess.TimeoutExpired:
        proc.kill()

print()
for k in ("A_default", "B_outer", "C_inner_too", "A2_restored"):
    r = res.get(k)
    if not r:
        continue
    upd = r["stages"].get("EXECUTOR_UPDATE", {})
    print(f"{k:12s} {r['fps']:8.1f} fps  EXECUTOR_UPDATE median={upd.get('median_us', 0):8.2f} us "
          f"p05={upd.get('p05_us', 0):8.2f} p95={upd.get('p95_us', 0):8.2f} (n={upd.get('n', 0)})  "
          f"NOT evaluated {r['not_evaluated']}/{r['bodies']}", flush=True)
base = res.get("A_default", {}).get("stages", {}).get("EXECUTOR_UPDATE", {}).get("median_us")
if base:
    for k in ("B_outer", "C_inner_too", "A2_restored"):
        m = res[k]["stages"].get("EXECUTOR_UPDATE", {}).get("median_us")
        if m is not None:
            res[f"delta_us_{k}"] = m - base
            print(f"{k:12s} - A_default = {m - base:+8.2f} us/frame "
                  f"({100.0 * (m - base) / 1000.0:+.2f} % of the 1 ms/frame budget)")
(OUT / "b39_cost_farm_result.json").write_text(json.dumps(res, indent=1))
print(f"\nwritten {OUT}/b39_cost_farm_result.json")
