#!/usr/bin/env python3
# B20 - anchored galactic display.  Regression lock for the Vixy-ratified
# semantics (USER_QUESTIONS Q8 / INTENT 11.48(a) A14, verbatim answer:
# "The solar-system view seen from very far, with my anchor kept (in case we
# go back afterward)"; Q7/A13: "you stay anchored to that body WHATEVER your
# altitude").  INTENT.md 11.59 / 13.B B20.
#
# WHAT IS ASSERTED, and why the counterfactual would break it
# -----------------------------------------------------------
# The ONE altitude-driven reference switch on the new path is Camera::update's
# escalation call, GUARDED by `if (freeMode)` [Camera.cpp:345-350]:
#     if (freeMode)
#         if (auto n = reference->findBetterReference(position.length()))
#             switchToBody(n);
# Anchored (freeMode==false) that call never runs, so no altitude can move the
# reference.  The observable is the camera dump `reference`.  The INVARIANT
# under test is body-agnostic: an OUTWARD anchored move does not change the
# reference (ref_after == ref_before), at ANY altitude including galactic
# (4.9e11 AU) - the same move that in FREE FLIGHT auto-transitions all the way
# to Universe.  The free-flight leg is the DISCRIMINATOR: it proves the anchor
# is what suppresses the switch, not that the switch simply never fires at that
# distance.  A counterfactual that ungates the escalation while anchored
# (drop `if (freeMode)`, feed the anchored `distance`) escalates the outward
# move -> ref_after != ref_before -> family B FAILS (proven, not assumed).
#
# The discriminator INPUT is dumped too: at galactic distance the camera-to-
# reference distance (dump `distance`) is many orders above the reference's
# `refAoI`, i.e. findBetterReference WOULD return the parent if it were called.
# The switch is SUPPRESSED, not absent.
#
# Reversible pair x2: near -> out1 -> back1 -> out2 -> back2, the second
# outward leg starting from the state the first round trip produced.  The
# OUTWARD legs are the assertion (anchor kept).  The BACK (descent) legs are
# recorded as WITNESSES only, NOT a pass/fail gate: on the current dual-path
# build the OLD executor re-anchors the new camera on the downward mode
# crossing (SolarSystemModule::onEnter -> enterSystem -> changeSystem ->
# Camera::switchToBody(SolarSystem), gdb-confirmed) - the distinct
# "anchored-mode descent" item SUSPENDED at INTENT 11.36 (line 615), NOT
# closed by A13/A14.  Gating it would touch the old-executor/new-camera seam
# and 6.9 mode dissolution (escalation policy = Vixy's).  Locking that value
# here would enshrine a defect; leaving it a witness reports it without gating.
#
# Instrument-chain: every leg asserts freeMode (False anchored / True free).
# Without it a mistyped `camera action free_mode state on` would leave the
# camera anchored and make the "free flight escalates" contrast pass vacuously
# (the flag-spelling trap, INTENT 11.53/11.58).
#
# PRECONDITION: app fresh-launched (dirty instrument state invalidates the run),
# enable_tcp = true, projection FISHEYE.  No init_fov requirement: this scene
# reads camera-dump reference/distance (mat layer), never screen-layer px.
#
#   DISPLAY=:2 ./build-claude/src/spacecrafter &      # wait for port 7805
#   ./b20_anchored_galactic.py [outdir]               # default artifacts/b20g
#
# Exit 0 = all gated assertions pass, 1 = at least one failed (details on
# stdout, machine-readable summary in <outdir>/b20_result.json).

import socket, time, json, sys, os

AU_M = 149597870700.0            # m per AU (moveto altitude command is metres)
J0 = 2461233.5                   # same epoch as scene_e_spine.py / drive_scenes
MW_AOI = 3.2e9 * 128             # scene-E MilkyWay AoI (AU): the galactic shell
GAL_AU = MW_AOI * 1.2            # ~4.9e11 AU: free flight => Universe (scene E)
GAL_M = int(GAL_AU * AU_M)       # the metres the moveto command takes
ANCHOR = "Earth"                 # the anchor under test (default home planet)
ANCHOR_PARENT = "Sun"            # Earth's parent (anchor-semantics check)

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b20g")
os.makedirs(OUT, exist_ok=True)


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.3); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)


def dump(sock, tag, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/b20_{tag}.json", pause)


def cam(tag):
    with open(os.path.join(OUT, f"b20_{tag}.json")) as f:
        return json.loads(f.readline())["camera"]


# ---------------------------------------------------------------- drive ----
sock = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(sock, "flag experimental_path on", 1)
send(sock, "timerate rate 0", 1)                 # freeze BEFORE the epoch
send(sock, "date jday %.9f" % J0, 1)
send(sock, "set home_planet %s" % ANCHOR, 2)     # explicit anchor (Q7)
send(sock, "camera action free_mode state off", 1)   # ensure anchored
send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
dump(sock, "near")

# ---- anchored reversible pair x2 (the CORE: outward keeps the anchor) ------
pairs = []               # (entry, pre_tag, out_tag, back_tag)
prev = "near"
for entry in (1, 2):
    out_tag, back_tag = f"out{entry}", f"back{entry}"
    send(sock, f"moveto altitude {GAL_M} duration 0", 4)   # OUT to galactic
    dump(sock, out_tag)
    send(sock, "moveto altitude 100 duration 0", 4)        # BACK toward surface
    dump(sock, back_tag)
    pairs.append((entry, prev, out_tag, back_tag))
    prev = back_tag      # entry 2 starts from entry 1's end state

# ---- free-flight contrast (the discriminator): same move DOES escalate -----
send(sock, "set home_planet %s" % ANCHOR, 2)     # re-anchor cleanly to Earth
send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
dump(sock, "fpre")
send(sock, "camera action free_mode state on", 1)
send(sock, f"moveto altitude {GAL_M} duration 0", 5)
dump(sock, "fout")
send(sock, "camera action free_mode state off", 1)
sock.close()

# ------------------------------------------------------------- evaluate ----
fail = 0
report = {"gal_au": GAL_AU, "gal_m": GAL_M, "anchor": ANCHOR,
          "legs": {}, "pairs": [], "free": {}}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    return ok


def rec(tag):
    c = cam(tag)
    report["legs"][tag] = {k: c[k] for k in
        ("reference", "freeMode", "boundToSurface", "distance",
         "refAoI", "refDist", "refCached", "refParent")}
    return c

print("\n--- A: instrument-chain + baseline anchor ---")
near = rec("near")
check(near["freeMode"] is False,
      f"near freeMode={near['freeMode']} (anchored precondition)")
check(near["reference"] == ANCHOR,
      f"near reference={near['reference']} == {ANCHOR}")

print("\n--- B: anchored OUTWARD to galactic keeps the reference (x2) ---")
for entry, pre_tag, out_tag, back_tag in pairs:
    pre = rec(pre_tag) if pre_tag not in report["legs"] else \
          {k: report["legs"][pre_tag][k] for k in report["legs"][pre_tag]}
    out = rec(out_tag)
    back = rec(back_tag)
    # the invariant: an outward anchored move does not change the reference
    check(out["reference"] == pre["reference"],
          f"entry{entry} OUT ref={out['reference']} == pre ref={pre['reference']} "
          f"(no altitude switch while anchored)")
    check(out["freeMode"] is False,
          f"entry{entry} OUT freeMode={out['freeMode']} (still anchored)")
    # reached galactic distance (this is a galactic-scale test, not a near one)
    check(out["distance"] >= 0.5 * GAL_AU,
          f"entry{entry} OUT distance={out['distance']:.4e} AU >= "
          f"{0.5*GAL_AU:.4e} (galactic)")
    # discriminator INPUT: the switch WOULD fire (obs dist >> refAoI) but is
    # suppressed by the anchor - "solar-system from afar", not recentered
    check(out["distance"] > out["refAoI"],
          f"entry{entry} OUT distance={out['distance']:.4e} > refAoI="
          f"{out['refAoI']:.4e} (switch WOULD fire in free flight; suppressed)")
    print(f"     entry{entry} BACK (descent) ref={back['reference']:12s} "
          f"refParent={back['refParent']:9s} dist={back['distance']:.4e}  "
          f"[WITNESS - suspended anchored-mode-descent, INTENT 11.36]")
    report["pairs"].append({"entry": entry, "pre_ref": pre["reference"],
        "out_ref": out["reference"], "out_dist": out["distance"],
        "out_refAoI": out["refAoI"], "back_ref": back["reference"],
        "back_refParent": back["refParent"], "back_dist": back["distance"]})

print("\n--- E: anchor semantics at galactic (Earth kept, parent Sun, resolves) ---")
o1 = rec("out1")
check(o1["reference"] == ANCHOR and o1["refParent"] == ANCHOR_PARENT
      and o1["refCached"] is True,
      f"out1 reference={o1['reference']} refParent={o1['refParent']} "
      f"refCached={o1['refCached']} (anchor resolves to its body, "
      f"solar-system-from-afar not recentred)")

print("\n--- C: free-flight contrast (same move DOES auto-transition) ---")
fpre = rec("fpre")
fout = rec("fout")
report["free"] = {"pre": report["legs"]["fpre"], "out": report["legs"]["fout"]}
check(fout["freeMode"] is True,
      f"fout freeMode={fout['freeMode']} (free-mode command took - "
      f"instrument-chain; else the contrast is vacuous)")
check(fout["reference"] != ANCHOR,
      f"fout reference={fout['reference']} != {ANCHOR} "
      f"(free flight auto-transitioned away from the anchor)")
check(fout["reference"] == "Universe",
      f"fout reference={fout['reference']} == Universe "
      f"(escalated to the top, matching scene-E mw_out)")

report["fail"] = fail
with open(os.path.join(OUT, "b20_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail) + ' ASSERTION(S) FAILED'} "
      f"- {OUT}/b20_result.json")
sys.exit(1 if fail else 0)
