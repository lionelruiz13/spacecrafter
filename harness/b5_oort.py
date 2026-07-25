#!/usr/bin/env python3
"""B5 §6.9 OORT content-migration PILOT verification (INTENT 6.9 / 13.B B5).

Proves the oort point cloud, migrated to a MODULAR BODY at the SolarSystem floor
(OortModule, CUSTOM/OORT slot), reproduces the OLD altitude-gated draw
(coreModule/oort.cpp) through the NEW path's distance/visibility REGIME machinery
(G4 floor gating) - NOT a hardcoded altitude test. Runs with the pilot ENABLED
(flag_experimental_oort=true; b5_oort_run.sh sets it) - the DEFAULT tree keeps
the oort OFF, so this is a SEPARATE run from b5_drawhalf.py (which stays 24/24,
unperturbed - the pilot's node-reach coupling, below, is exactly why it is gated).

Method: in each render phase, isolate that path's oort by toggling `flag oort`
on/off (the command routes to BOTH clouds through one CoreLink choke point). Only
the oort changes, so px32(on,off) = that path's oort ALONE and the rest of the
frame (bodies = the WITNESSES) is byte-identical in on/off (discrimination by
construction). Solar mode, ref=Sun (the clean regime-gated regime). The far
ref=Oort hijack - the node-reach coupling - is an ASSERTED leg since F0
(§11.102(e1) -> §11.103): it is the ONLY check here that fails when the pilot
flag silently does not apply, because every other leg is satisfied by the OLD
cloud drawing in both phases.

Gate, measured (see INTENT §11.<this>): the OLD cloud turns on ~1e13 m (~67 AU)
and off ~1e16 m; the NEW cloud is a NEAR component whose body scaledRadius (50
AU) puts the low edge at scaledRadius*2 = 100 AU (in/grounded->near regime). Test
points sit CLEARLY inside the hidden / shown bands, away from both soft edges.

Usage: b5_oort_run.sh b5_oort.py [outdir]  (fresh launch, init_fov=340, fisheye,
flag_experimental_oort=true; the runner owns config backup/restore + md5 assert).
"""
import socket, time, json, math, sys, os
import numpy as np
from PIL import Image

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "artifacts", "b5_oort")
os.makedirs(OUT, exist_ok=True)
AU_M = 149597870700.0

results = []
def check(name, ok, detail):
    results.append({"name": name, "ok": bool(ok), "detail": detail})
    print(f"{'PASS' if ok else 'FAIL'}  {name}: {detail}", flush=True)

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout: pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)

def img(name):
    return np.asarray(Image.open(f"{OUT}/{name}.png").convert("RGB"), dtype=np.int16)

def px32(a, b):
    d = np.abs(a - b); mask = (d > 32).any(axis=2); n = int(mask.sum())
    bbox = None
    if n:
        ys, xs = np.nonzero(mask); bbox = [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]
    return n, bbox

def notblack(name):
    return int((img(name).max(axis=2) > 16).sum())

def dump(sock, name):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", 2)
    cam = None
    with open(f"{OUT}/{name}.json") as f:
        for line in f:
            d = json.loads(line)
            if "camera" in d: cam = d["camera"]
    return cam

def oort_px(sock, phase_on, tag):
    """In the given render phase, isolate the oort via flag on/off (px32) and
    keep the oort-OFF shot to witness the rest of the scene."""
    send(sock, f"flag experimental_path {'on' if phase_on else 'off'}", 2)
    send(sock, "flag oort on", 2);  shot(sock, f"{tag}_on")
    send(sock, "flag oort off", 2); shot(sock, f"{tag}_off")
    px, bb = px32(img(f"{tag}_on"), img(f"{tag}_off"))
    witness = notblack(f"{tag}_off")   # scene WITHOUT the oort - the witnesses
    return px, bb, witness

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path off", 1)
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)
send(s, "flag atmosphere off", 1)
send(s, "flag landscape off", 1)
send(s, "flag star off", 1)
send(s, "flag milky_way off", 1)
send(s, "flag constellation_art off", 1)
send(s, "select planet Sun", 1)
send(s, "flag track_object on", 3)
send(s, "camera action free_mode state on", 1)

HIDE, SHOW = 800, 8000   # px32 thresholds (measured hidden ~250, shown ~39000)

def leg(tag, alt_m, want_shown):
    send(s, f"moveto altitude {int(alt_m)} duration 0", 4)
    send(s, "flag track_object off", 2)
    cam = dump(s, f"d_{tag}")
    rd = cam.get("refDist") or 0
    ref = cam.get("reference")
    npx, nbb, nwit = oort_px(s, True,  f"n_{tag}")
    opx, obb, owit = oort_px(s, False, f"o_{tag}")
    send(s, "flag track_object on", 2)
    print(f"  [{tag}] refDist={rd:.1f} ref={ref} NEW={npx}@{nbb} OLD={opx}@{obb} "
          f"wit(new/old)={nwit}/{owit}", flush=True)
    return rd, ref, npx, nbb, nwit, opx, owit

# ---- LOW: deep inside (refDist ~20 AU) -> BOTH HIDDEN (in/grounded regime) ---
rd, ref, npx, nbb, nwit, opx, owit = leg("low", 3e12, want_shown=False)
check("low_ref_sun", ref == "Sun", f"ref={ref} refDist={rd:.1f} (clean solar-mode regime)")
check("low_new_hidden", npx < HIDE, f"NEW modular oort px32={npx} (< {HIDE}: regime hides it)")
check("low_old_hidden", opx < HIDE, f"OLD oort px32={opx} (< {HIDE}: altitude gate hides it)")
check("low_witness", nwit > 0 and owit > 0, f"witness lit px new/old={nwit}/{owit} (bodies drawn without the oort)")

# ---- MID: shown band (refDist ~334 AU) -> BOTH SHOWN, parity ----------------
rd, ref, npx, nbb, nwit, opx, owit = leg("mid", 5e13, want_shown=True)
check("mid_ref_sun", ref == "Sun", f"ref={ref} refDist={rd:.1f} (clean solar-mode regime)")
check("mid_new_shown", npx > SHOW, f"NEW modular oort px32={npx} (> {SHOW}: regime shows it)")
check("mid_old_shown", opx > SHOW, f"OLD oort px32={opx} (> {SHOW})")
parity = abs(npx - opx) / max(opx, 1)
check("mid_parity", parity < 0.15, f"|NEW-OLD|/OLD={parity:.3f} (< 0.15: equivalent content)")
check("mid_witness", nwit > 0, f"witness lit px (oort OFF)={nwit} (bodies drawn without the oort)")

# ---- discrimination: SAME regime gate, shown/hidden by ALTITUDE -------------
# (the §11.80 counterfactual: gate mis-matched -> draw collapses to ~0 while the
#  witnesses stay green. Here the low-altitude gate IS the mis-match.)
lo_new = next(r for r in results if r["name"] == "low_new_hidden")
low_new_px = int(lo_new["detail"].split("px32=")[1].split()[0])
disc = npx / max(low_new_px, 1)   # npx here = the MID new-oort px (last leg)
check("discrimination", disc > 20,
      f"NEW oort shown/hidden ratio = {npx}/{low_new_px} = {disc:.0f}x (one regime gate, "
      f"witnesses drawn in both)")

# ---- PATH IDENTITY: the coupling leg, an ASSERT since F0 (INTENT §11.103) ---
# WHY THIS IS THE INSTRUMENT'S OWN LOAD-BEARING CHECK (§11.102(e1)): every leg
# above passes unchanged if the modular oort was never instantiated. The dual
# seam (solarSystemModule.cpp:202) suppresses the OLD cloud in the modular phase
# ONLY when hasExperimentalOort() - so a silently failed flag leaves the OLD
# cloud drawing in BOTH phases and shown/hidden/parity/discrimination/witness
# all stay green with the SUBJECT ABSENT. Until F0 the evidence that the subject
# exists was printed, never asserted.
# The witness: at refDist ~668 AU the camera reference becomes 'Oort' ONLY
# because the modular body's ~6398 AU extent inflates its AoI past MilkyWay's -
# a property of the LIVE TREE, not of a log line. Without the modular oort the
# reference stays MilkyWay. DISCRIMINATION MEASURED (F0, 2026-07-25): same
# binary, same driver, flag_experimental_oort OMITTED -> 10/11 with exactly this
# check failing (reference='MilkyWay'), which is §11.102(e1) reproduced live.
send(s, "moveto altitude 100000000000000 duration 0", 4)  # 1e14 m -> refDist ~668
send(s, "flag track_object off", 2)
cam = dump(s, "d_far")
coupling_ref = cam.get("reference")
check("path_identity", coupling_ref == "Oort",
      f"at refDist={cam.get('refDist'):.0f} AU reference={coupling_ref!r} (must be 'Oort': the "
      f"modular cloud's ~6400 AU extent inflated its AoI and HIJACKED the camera reference from "
      f"MilkyWay - the node-reach coupling, which ONLY the modular body can produce, so this is "
      f"the proof the pilot subject is in the tree and the legs above are not measuring the OLD "
      f"cloud twice)")

json.dump({"results": results, "coupling_ref": coupling_ref,
           "coupling_refDist": cam.get("refDist")},
          open(f"{OUT}/b5_oort_result.json", "w"), indent=1)
bad = [r for r in results if not r["ok"]]
print(f"=== {len(results)-len(bad)}/{len(results)} PASS ===", flush=True)
sys.exit(1 if bad else 0)
