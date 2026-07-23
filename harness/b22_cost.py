#!/usr/bin/env python3
# Deliverable 2: live per-frame engine cost at the band vs out-of-band.
# Navigate to a COST_TARGET refDist (in-band / pure-resolved / pure-dot), then
# HOLD still and dwell so query_statistics captures many settled frames.
# The EXECUTOR_DRAW stage (app.cpp: DRAW_RESOURCE_READY -> EXECUTOR_DRAW brackets
# executor->draw -> drawExperimental -> drawNested -> drawStarProxy) is the cost
# window for the crossfade's marginal work (one drawStarProxy in-band). Verdict
# denominator = 1 ms/frame (D11), NOT the 16.7 ms display frame.
import socket, time, json, math, sys, os
OUT = sys.argv[1] if len(sys.argv) > 1 else "."
AU_M = 149597870700.0
TARGET_PX = float(os.environ.get("COST_PX", "19"))   # 19=in-band, 30=resolved, 12=dot
DWELL = float(os.environ.get("COST_DWELL", "30"))
def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout: pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)
def cam(p):
    with open(p) as f: return json.loads(f.readline())["camera"]
def bodies(p):
    out={}
    for line in open(p):
        d=json.loads(line)
        if d.get("type")=="body" and d.get("new"): out[d["name"]]=d["new"]
    return out
def dump(sock,nm,pause=2.0): send(sock,f"body action dual_dump filename {OUT}/{nm}.json",pause)

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)
send(s, "zoom fov 340 duration 0", 2)   # force the harness fov (config may drift)
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
dump(s, "base"); b = bodies(f"{OUT}/base.json")
n = lambda v: math.sqrt(sum(x*x for x in v))
sys_aoi = 1.1*(1.1*(n(b["Neptune"]["ecl"])+1.1*b["Neptune"]["boundingRadius"]))*16
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
dump(s, "band0"); cb = cam(f"{OUT}/band0.json")
sel = cb.get("selDist") or 1340.0; hf = cb.get("halfFov")
SUBSYS = 1.1*(1.1*(n(b["Neptune"]["ecl"])+1.1*b["Neptune"]["boundingRadius"]))
# refDist for a desired px at the ACTUAL halfFov: px=atan(S/sqrt(d^2-S^2))/hf*2048
#  => d = S / sin(px*hf/2048)
target_ref = SUBSYS / math.sin(TARGET_PX * hf / 2048.0)
send(s, f"camera action descend coef {target_ref/sel:.6f}", 3)
dump(s, "attarget"); c = cam(f"{OUT}/attarget.json")
d = c.get("refDist"); hf = c.get("halfFov")
px = math.atan(SUBSYS/math.sqrt(d*d-SUBSYS*SUBSYS))/hf*2*1024 if d>SUBSYS else 2048
print(f"AT TARGET target_px={TARGET_PX} refDist={d} px={px:.3f} halfFov={hf:.4f} reference={c.get('reference')}", flush=True)
json.dump({"target_px":TARGET_PX,"refDist":d,"px":px,"halfFov":hf,
           "reference":c.get("reference"),"subsys":SUBSYS,"dwell":DWELL,
           "inband": bool(16.0 <= px < 24.0)}, open(f"{OUT}/cost_meta.json","w"), indent=1)
# HOLD & dwell: no commands, let query_statistics capture settled frames.
print(f"dwelling {DWELL}s at px={px:.2f} ...", flush=True)
time.sleep(DWELL)
# a couple of dumps at the end to timestamp the settled tail
dump(s, "attarget2")
print("DONE", flush=True)
