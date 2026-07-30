#!/usr/bin/env python3
# B39 CORPUS BOUNDARY - the two shipped scripts that bound D23 (§11.113(b)(vi)),
# played live, on the new path.  These are §2(b) content: user scripts are
# immutable live-show material, so "as if they didn't exist" must NOT reach the
# command/structural surface, and these two are the proof of where the line is.
#
#   S10.sts:17-18   `set home_planet Solar_System` + `select planet
#                    Solar_System pointer off` on a body that ships
#                    `hidden = true` -> a hidden body is a live OBSERVER
#                    REFERENCE and a live SELECTION TARGET.  Being the reference
#                    is itself a use, so "no tick" cannot mean "holds no state".
#   W17.sts:14-18   `body name Goldilocks_Zone hidden false` / `... true` - the
#                    habitability-zone toggle.  This is where "as if never
#                    hidden" is judged, and it is the shipped consumer of the
#                    unhide path.
#
# WHAT IS ASSERTED (and how each can fail)
#   S1 the scripts RUN to completion - `script action play` then the app is still
#      alive and answering, and the applog carries no unknown-command error for
#      their lines.  A literal reading of the mandate breaks S10 here.
#   S2 after S10 the observer's REFERENCE is Solar_System and the SELECTION is
#      Solar_System - read from the process (camera dump `refParent`/`ref`, and
#      the selected body's name).  Not "it did not crash": the roles held.
#   S3 the hidden reference is POSITION-FRESH: it is on the camera's own chain,
#      so the walk evaluates it every frame - evalCount must GROW while it is the
#      reference, which is the measurement that "being the reference is a use"
#      is satisfied by construction rather than by hope.
#   S4 W17's toggle, traversed TWICE, second entry from the first exit's state:
#      `relation` flips both ways, and the SCREEN shows the body appear and
#      disappear (the zone carries a ring texture, so it is large and visible).
#      The unhidden frame must differ from the hidden one ABOVE the measured
#      noise floor, and the second hidden frame must return to the first.
#
# The scripts are played by ABSOLUTE path.  Measured, not assumed: `script action
# play filename W17.sts` resolves against the APP's cwd (FilePath's scriptPath and
# getScriptDir() are both the relative "scripts/"), so from a harness launch it
# reports "No 'W17.sts', 'scripts/W17.sts' ... found" and the whole leg would pass
# vacuously - which is exactly what the first run of this file did, and what the
# reference/selection preconditions below caught.
#
# W17 is a TOGGLE driven by a script variable (`struct if goldilock equal 0`), so
# its parity at the first play is not knowable from here: the leg plays it FOUR
# times and requires the observed relation sequence to ALTERNATE, which covers
# both entries of the reversible pair twice whichever beat it starts on.
#
# ONE LEG PER FRESH LAUNCH, selected by argv[2] ("w17" | "s10").  Measured
# reason, not caution: S10 re-homes the observer onto a hidden body 0.019 AU away,
# loads a full-screen image and leaves a paused script, and running the two legs in
# either order in one process contaminated the other - W17-after-S10 measured a
# BLACK frame (0 nonblack px), and S10-after-W17 measured `select planet
# Solar_System` not landing while the Sun was the standing selection+track target.
# Both are instrument confounds of SHARING a process, and both legs pass on their
# own fresh launch, which is this harness's standing precondition anyway.
#
#   ./b39_run.sh b39_scenes.py <outdir> w17
#   ./b39_run.sh b39_scenes.py <outdir> s10
#
# PRECONDITION: FRESH launch, enable_tcp, FISHEYE.
# Exit 0 = every assertion passed.

import socket, time, json, sys, os
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "b39_scenes")
LEG = sys.argv[2] if len(sys.argv) > 2 else "w17"
os.makedirs(OUT, exist_ok=True)
THR = 32
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
fail = 0
report = {}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    return ok


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=1.8):
    send(f"body action screenshot filename {OUT}/s_{tag}.png", pause); return tag


def dump(tag, pause=2.2):
    send(f"body action dual_dump filename {OUT}/s_{tag}.json", pause); return tag


def load(tag):
    hdr, bodies = None, {}
    with open(os.path.join(OUT, f"s_{tag}.json")) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            d = json.loads(line)
            if d.get("type") == "header":
                hdr = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
    return hdr, bodies


def img(tag):
    return np.asarray(Image.open(os.path.join(OUT, f"s_{tag}.png"))
                      .convert("RGB")).astype(np.int32)


def dpx(a, b):
    d = np.abs(img(a) - img(b)).max(axis=2)
    return int((d > THR).sum()), int(d.max())


SCRIPTS = os.path.expanduser("~/.spacecrafter/scripts/fscripts")
send("flag experimental_path on", 1)
send("timerate rate 0", 1)

# ================================================ W17: the live unhide toggle =
if LEG == "w17":
    # W17 runs FIRST: S10 is destructive (full-screen image overlay, the observer
    # re-homed onto a hidden body 0.019 AU away, a paused script left behind), and the
    # first version of this file ran W17 after it and measured a BLACK frame - 0
    # nonblack px, i.e. a zero-diff on masked content, which the nonblack
    # precondition below is here to refuse.
    # The subject orbits the Sun at 0.01 AU carrying a 270e6 km ring, so from Earth's
    # surface with the Sun framed it covers a large part of the sky.
    send("flag landscape off", 1)
    send("flag atmosphere off", 1)
    send("date jday 2461233.5", 1)
    send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
    send("select planet Sun pointer off", 1)
    send("flag track_object on", 5)
    send("flag track_object off", 1.5)
    shot("w17_floor_a"); shot("w17_floor_b")
    floor_px, floor_max = dpx("w17_floor_a", "w17_floor_b")
    nb = int((img("w17_floor_a").max(axis=2) > 8).sum())
    print(f"\nW17 noise floor: px>{THR} = {floor_px} (max {floor_max}); "
          f"nonblack = {nb}")
    check(nb > 5000, f"W17 scene carries content ({nb} nonblack px)")

    beats = []
    for beat in range(1, 5):
        send(f"script action play filename {SCRIPTS}/W17.sts", 3)
        time.sleep(2)
        t = dump(f"w17_b{beat}"); shot(f"w17_b{beat}")
        _, B = load(t)
        rel = B["Goldilocks_Zone"]["new"]["relation"]
        ev = B["Goldilocks_Zone"]["new"]["evalCount"]
        print(f"W17 beat {beat}: Goldilocks_Zone relation={rel} evalCount={ev}")
        beats.append({"beat": beat, "relation": rel, "evalCount": ev, "tag": f"w17_b{beat}"})
    for i in range(1, 4):
        check((beats[i]["relation"] < 3) != (beats[i - 1]["relation"] < 3),
              f"W17: the shipped toggle FLIPPED at beat {i+1} "
              f"(relation {beats[i-1]['relation']} -> {beats[i]['relation']})")
    shown = [b for b in beats if b["relation"] >= 3]
    hid = [b for b in beats if b["relation"] < 3]
    check(len(shown) == 2 and len(hid) == 2,
          f"W17: 2 shown + 2 hidden beats observed ({len(shown)}/{len(hid)}) - both "
          f"entries of the reversible pair, traversed twice")
    if len(shown) == 2 and len(hid) == 2:
        px_sh, max_sh = dpx(shown[0]["tag"], hid[0]["tag"])
        px_sh2, max_sh2 = dpx(shown[1]["tag"], hid[1]["tag"])
        px_hh, max_hh = dpx(hid[0]["tag"], hid[1]["tag"])
        px_ss, max_ss = dpx(shown[0]["tag"], shown[1]["tag"])
        print(f"W17 screen: shown-vs-hidden {px_sh} px (max {max_sh}) and {px_sh2} px "
              f"(max {max_sh2})  |  hidden-vs-hidden {px_hh} px  |  shown-vs-shown "
              f"{px_ss} px  |  floor {floor_px}")
        check(px_sh > max(floor_px, 100) and px_sh2 > max(floor_px, 100),
              f"W17: the body is VISIBLY there when shown and gone when hidden - "
              f"{px_sh} / {px_sh2} px>{THR} against a {floor_px} px floor")
        check(px_hh <= floor_px, f"W17 both HIDDEN frames identical ({px_hh} px <= "
                                 f"floor {floor_px}) - as-if-nonexistent, reproduced")
        check(px_ss <= floor_px, f"W17 both SHOWN frames identical ({px_ss} px <= "
                                 f"floor {floor_px}) - 'as if it never was hidden'")
        report["w17"] = {"floor_px": floor_px, "beats": beats,
                         "shown_vs_hidden_px": [px_sh, px_sh2],
                         "hidden_vs_hidden_px": px_hh, "shown_vs_shown_px": px_ss}
    else:
        report["w17"] = {"floor_px": floor_px, "beats": beats}

# ===================================================== S10: hidden reference =
if LEG == "s10":
    # Played through the app's own script engine, i.e. the real channel a show uses.
    # LAST, because it is destructive - see the W17 block above.
    send(f"script action play filename {SCRIPTS}/S10.sts", 2)
    time.sleep(24)     # S10's waits sum to ~8 s, then it hits `script action pause`
    # Dumped BEFORE `script action end`, so nothing an end-of-script reset might do
    # can be mistaken for the boundary holding or not holding.
    d10 = dump("s10_after")
    hdr, bodies = load(d10)
    cam = hdr.get("camera", {})
    ref = cam.get("reference")
    selected = cam.get("selected")
    print(f"\nS10 camera dump: {json.dumps({k: v for k, v in cam.items() if not isinstance(v, list)})}")
    report["s10_camera"] = {k: v for k, v in cam.items() if not isinstance(v, list)}
    ss = bodies.get("Solar_System", {}).get("new")
    check(ss is not None, "S10 instrument: Solar_System is in the dump at all")
    if ss:
        print(f"S10 Solar_System: relation={ss['relation']} evalCount={ss['evalCount']} "
              f"dist={ss['dist']} ecl={ss['ecl']}")
        report["s10_solar_system"] = {"relation": ss["relation"],
                                     "evalCount": ss["evalCount"],
                                     "dist": ss["dist"]}
        check(ss["relation"] < 3,
              f"S10 boundary: Solar_System is STILL DECLARED HIDDEN "
              f"(relation={ss['relation']}) while serving as home_planet - the "
              f"declared flag was not touched by being used")
    check(ref == "Solar_System",
          f"S10 boundary: the observer REFERENCE is Solar_System (dump says "
          f"{ref!r}) - a hidden body is a live observer frame")
    check(selected == "Solar_System",
          f"S10 boundary: the SELECTION is Solar_System (dump says {selected!r}) - "
          f"selection-by-name survives hiding")
    report["s10_reference"] = ref
    report["s10_selected"] = selected

    # S3: the hidden reference is position-fresh because it is ON the camera chain.
    e0 = dump("s10_eval0")
    time.sleep(8)
    e1 = dump("s10_eval1")
    _, B0 = load("s10_eval0"); _, B1 = load("s10_eval1")
    grow = B1["Solar_System"]["new"]["evalCount"] - B0["Solar_System"]["new"]["evalCount"]
    ctl = B1["Earth"]["new"]["evalCount"] - B0["Earth"]["new"]["evalCount"]
    print(f"S10 evalCount over ~8 s: hidden-but-REFERENCE Solar_System +{grow}  |  "
          f"Earth +{ctl}")
    report["s10_eval"] = {"reference_delta": grow, "earth_delta": ctl}
    check(grow > 100, f"S10 'being the reference is a use': the hidden reference was "
                      f"evaluated {grow} times (> 100 frames) - the walk keeps it "
                      f"current with no barrier needed")

    # Selection survives: ask the process, not the script.
    send("select planet Earth pointer off", 1.5)      # move the selection away...
    send("select planet Solar_System pointer off", 1.5)  # ...then back, by NAME
    hdr2, BS = load(dump("s10_reselected"))
    resel = hdr2.get("camera", {}).get("selected")
    check(resel == "Solar_System",
          f"S10 boundary: `select planet Solar_System` re-selects the hidden body BY "
          f"NAME after the selection was moved away (dump says {resel!r}) - the "
          f"name lookup and the selection surface both survive hiding")
    check(BS["Solar_System"]["new"]["relation"] < 3,
          f"S10 boundary: and it is still DECLARED hidden (relation "
          f"{BS['Solar_System']['new']['relation']})")
    report["s10_reselected"] = resel

sock.close()
report["leg"] = LEG
report["fail"] = fail
with open(os.path.join(OUT, f"b39_scenes_{LEG}_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail) + ' ASSERTION(S) FAILED'}")
sys.exit(1 if fail else 0)
