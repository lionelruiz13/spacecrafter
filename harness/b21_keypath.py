#!/usr/bin/env python3
# B21 UNIFICATION check (INTENT §11.71 finding / §11.72(c) residual): the
# interactive altitude ramp (`Core::raiseHeight`/`lowerHeight` -> `vzm.deltaHeight`
# -> `Core::updateMove`) must drive the new path through its ONE descent
# authority (`Camera::descend`), so that a KEY/joypad-driven descent and a
# COMMAND-driven descent produce the SAME trajectory.
#
# Why gdb: the ramp's only UI entry is a joypad BUTTON binding
# (`joypad_controller.cpp:414-417` -> `UI::raiseHeight()` -> `core->raiseHeight(1)`,
# ui.cpp:629-631); this host cannot inject joystick events (/dev/uinput is
# ACL-denied, no evdev), so the driver calls `Core::raiseHeight/lowerHeight` and
# `Core::updateMove` in the live process - the exact functions the UI calls, one
# layer below SDL. The layer above is source-verified, not injected: stated, not
# hidden.
#
# Usage: b21_keypath.py <outdir> <gdb_pid> <gdb_fifo>
import socket, sys, time, os, json, math, signal

OUT, GDBPID, FIFO = sys.argv[1], int(sys.argv[2]), sys.argv[3]
os.makedirs(OUT, exist_ok=True)
GDBLOG = f"{OUT}/gdb.log"
FAILS = []
GROUND_KM = 6000.0          # DescB radius, no datum/ground override
AU_KM = 149597870.7
COEF, NSTEP = 0.99, 5
PRED_ALT = 200.0 * COEF**NSTEP
# ONE application per gdb stop, with real frames in between: `Camera::descend`
# reads the reference's CACHED eye-frame matrix (`getObservedPosition()` =
# `mat.getTranslation()`, ModularBody.hpp:1075-1077), refreshed by the per-frame
# body update -- so N applications inside ONE frame all see the SAME geometry
# and compound LINEARLY instead of geometrically (measured: 10x0.99 with no
# intervening frame gave alt x0.9000 instead of x0.9044 -- INTENT §5.32's stale-
# reference class, annotated there). The real key path runs exactly one
# application per frame, so one-per-stop is the faithful cadence.

def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok: FAILS.append(name)

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192)
    except socket.timeout: pass
    sock.settimeout(None); print(">>", cmd, flush=True)

def _clean(ln):
    return ln.strip().replace('-nan', 'null').replace('nan', 'null') \
             .replace('-inf', '-1e308').replace('inf', '1e308')

def dump(sock, tag, pause=1.5):
    send(sock, f"body action dual_dump filename {OUT}/{tag}.json", pause)
    hdr, bodies = None, {}
    with open(f"{OUT}/{tag}.json") as f:
        for ln in f:
            ln = _clean(ln)
            if not ln: continue
            try: o = json.loads(ln)
            except Exception: continue
            if o.get("type") == "header": hdr = o["camera"]
            elif o.get("type") == "body": bodies[o["name"]] = o
    p = hdr["position"]
    newpos_km = math.sqrt(sum(x*x for x in p)) * AU_KM
    b = bodies.get("DescB", {})
    olddist_km = (b.get("old") or {}).get("dist")
    olddist_km = olddist_km * AU_KM if olddist_km is not None else None
    st = dict(tag=tag, newpos=newpos_km, newalt=newpos_km - GROUND_KM,
              olddist=olddist_km, ref=hdr.get("reference"), free=hdr.get("freeMode"))
    print(f"-- {tag}: ref={st['ref']} free={st['free']} |pos|={newpos_km:.6f} km "
          f"alt={st['newalt']:.6f} km  old_dist={olddist_km}", flush=True)
    return st

def gdb_marker_count(mark):
    try:
        with open(GDBLOG, "rb") as f: return f.read().count(mark.encode())
    except FileNotFoundError:
        return 0

def gdb_inject(cmds, mark, timeout=120):
    """Stop the inferior, run cmds, resume. Handshake on `mark` in gdb.log."""
    before = gdb_marker_count(mark)
    os.kill(GDBPID, signal.SIGINT)
    time.sleep(3)
    with open(FIFO, "w") as f:
        for c in cmds: f.write(c + "\n")
        f.write(f'printf "{mark}\\n"\n')
        f.write("continue\n")
        f.flush()
    t0 = time.time()
    while time.time() - t0 < timeout:
        if gdb_marker_count(mark) > before:
            time.sleep(2); return True
        time.sleep(1)
    return False

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
for f in ("atmosphere", "fog", "landscape", "show_fps"):
    send(s, f"flag {f} off")
send(s, 'body action load name DescB radius 6000 parent Sun type Planet oblateness 0.0 '
        'albedo 0.3 halo false color 0.6,0.6,0.9 tex_map bodies/moon.png '
        'coord_func still_orbit orbit_x 1200 orbit_y 0 orbit_z 0', 3)
send(s, "set home_planet DescB", 3)
send(s, "camera action free_mode state on", 1.5)
send(s, "select planet DescB", 1)
send(s, "flag track_object on", 2)
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 3)

s0 = dump(s, "f4_s0")
check("scene_start", abs(s0["newalt"] - 200.0) < 0.5 and s0["free"] is True,
      f"start alt {s0['newalt']:.6f} km (want 200.000), freeMode={s0['free']}")

# ---- LEG C: the COMMAND channel ----------------------------------------
for _ in range(NSTEP):
    send(s, f"camera action descend coef {COEF}", 0.7)
c1 = dump(s, "f4_legC")

# ---- reset to the identical start state --------------------------------
send(s, "moveto lat 0 lon 0 alt 200000 duration 0", 3)
s1 = dump(s, "f4_s1")
check("reset_exact", abs(s1["newalt"] - s0["newalt"]) < 1e-3,
      f"reset alt {s1['newalt']:.6f} vs start {s0['newalt']:.6f} km")

# ---- LEG K: the KEY/joypad ramp, injected at Core::lowerHeight ----------
# One application per stop, real frames in between (see the cadence note above).
okall = True
for i in range(NSTEP):
    cmds = ["break Core::updateMove", "continue", "set $core = this",
            'printf "F4CORE=%p\\n", $core', "delete breakpoints",
            "call $core->lowerHeight(1)",
            "call $core->updateMove(30)",
            "call $core->lowerHeight(0)"]
    okall &= gdb_inject(cmds, f"F4KEYDONE{i}")
    time.sleep(1.5)
    dump(s, f"f4_legK_step{i}")
check("gdb_injection_ran", okall, f"handshake markers F4KEYDONE0..{NSTEP-1} seen in gdb.log")
k1 = dump(s, "f4_legK")

# ---- verdicts -----------------------------------------------------------
dC = c1["newalt"]; dK = k1["newalt"]
check("P1_key_equals_command", abs(dK - dC) < 1e-3,
      f"alt after 10x{COEF}: command {dC:.6f} km  key {dK:.6f} km  |diff| {abs(dK-dC):.2e} km "
      f"(predicted {PRED_ALT:.6f})")
check("P1b_command_matches_law", abs(dC - PRED_ALT) < 1e-3,
      f"command leg {dC:.6f} vs predicted {PRED_ALT:.6f} km")
check("P4_command_is_new_only",
      c1["olddist"] is not None and abs(c1["olddist"] - s0["olddist"]) < 1e-3,
      f"old dist across leg C: {s0['olddist']} -> {c1['olddist']} km (must be unchanged)")
PRED_OLD = GROUND_KM + 200.0 * COEF**NSTEP   # multAltitude scales the ALTITUDE
check("P3_injection_reached_the_sink",
      k1["olddist"] is not None and abs(k1["olddist"] - PRED_OLD) < 5e-2,
      f"old dist across leg K: {s1['olddist']} -> {k1['olddist']} km "
      f"(want {PRED_OLD:.4f} = the OLD ramp acting)")
check("P2_new_path_moved", abs(dK - s1["newalt"]) > 1.0,
      f"new-path alt across leg K: {s1['newalt']:.6f} -> {dK:.6f} km "
      f"(pre-fix binary must show NO movement here)")

json.dump(dict(s0=s0, legC=c1, s1=s1, legK=k1, coef=COEF, nstep=NSTEP,
               predicted_alt=PRED_ALT), open(f"{OUT}/b21_keypath_results.json", "w"), indent=1)
print(f"\n=== {'ALL PASS' if not FAILS else 'FAILURES: ' + ','.join(FAILS)} ===", flush=True)
s.close()
sys.exit(1 if FAILS else 0)
