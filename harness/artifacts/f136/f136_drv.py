#!/usr/bin/env python3
# F136 - the fov readouts through the gate, on one pin: R3 (`zoom delta_fov`), R5 (`zoom auto in manual`)
# and the gdb read of Core::drawnFov() (P22 re-pointed), each taken while the two engines are APART.
#
#   F124_OUT_ROOT=<harness>/artifacts/f136 bash f124_run.sh <label> artifacts/f136/f136_drv.py --pin on|off [--before]
#
# The engines are separated the only way an input can: one key step on a 60 -> 20 zoom plan (leg Z2 "same"),
# then rest: the projector lands on 20, the Camera rests near its own mid-plan value. Then:
#   R3  `zoom delta_fov 5`                 -> both engines at DRAWN + 5 (new pin: the Camera's rest; old pin: 25)
#   R5  `zoom auto in manual on duration 1` -> both engines at DRAWN x 0.5
#   R4  `position action save` apart, move both, `position action load` -> both engines at the DRAWN value saved
#   P22 gdb: Core::drawnFov() == ModularBody::halfFov * 360 / pi (new) / == projection->fov (old), bit for bit
# --before: the binary has no drawnFov (the BEFORE run); the R3 / R5 checks must then FAIL on the new pin.
# Helpers are F124's and b21's own, taken by AST as f124_legs.py takes them.
import argparse, ast, json, math, os, signal, socket, struct, sys, time

HARNESS = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--pin", default="on", choices=["on", "off"])
ap.add_argument("--before", action="store_true")
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
GDBLOG = OUT + "/gdb.log"
FAILS = []
PERT = False
RESULTS = {"driver": "f136_drv", "pin": ARGS.pin, "before": ARGS.before, "states": [], "notes": [], "gdb": []}
STEP = [0]


def reuse(path, names):
    names = set(names)
    for node in ast.parse(open(path).read()).body:
        if isinstance(node, ast.FunctionDef) and node.name in names:
            exec(compile(ast.Module([node], []), path, "exec"), globals())
            names.discard(node.name)
    if names:
        sys.exit("cannot reuse %s from %s" % (sorted(names), path))


reuse(HARNESS + "/b21_keypath.py", ["check", "send", "_clean", "gdb_marker_count", "gdb_inject"])
reuse(HARNESS + "/f124_legs.py", ["note", "state", "gdb_errors", "inject", "capture_ui", "zoom_calls", "scene", "fov_pair"])

AU_KM = 149597870.7                 # f124_legs.py's constant, read by its state()
NEW_EXPR = "(double)ModularBody::halfFov * (360.0 / 3.141592653589793)"


def gdb_read(tag):
    """One frame's read of both engines' fov and of the gated reader, at the entry of Core::updateMove."""
    since = os.path.getsize(GDBLOG)
    cmds = ["break Core::updateMove", "continue", "set $core = this", "delete breakpoints",
            'printf "F136G %s drawModularSystem=%%d projector=%%.17g newv=%%.17g\\n", '
            '$core->ssystemFactory->drawModularSystem, $core->projection->fov, %s' % (tag, NEW_EXPR)]
    if not ARGS.before:
        cmds.append('printf "F136D %s drawn=%%.17g eq_new=%%d eq_old=%%d\\n", $core->drawnFov(), '
                    '$core->drawnFov() == %s, $core->drawnFov() == $core->projection->fov' % (tag, NEW_EXPR))
    ok = gdb_inject(cmds, "F136READ%d" % STEP[0])
    STEP[0] += 1
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        txt = f.read().decode("latin-1")
    got = {}
    for ln in txt.splitlines():
        p = ln[max(ln.find("F136G"), ln.find("F136D"), 0):].split()   # after gdb's own prompts
        if len(p) >= 3 and p[0] in ("F136G", "F136D") and p[1] == tag:
            for kv in p[2:]:
                k, v = kv.split("=")
                got[k] = float(v)
    bad = gdb_errors(since)
    check("gdb_read_%s_ran_clean" % tag, ok and not bad and "projector" in got, "handshake=%s gdb said: %s" % (ok, bad[:3]))
    RESULTS["gdb"].append(dict(tag=tag, **got))
    print("GDB %-5s %s" % (tag, " ".join("%s=%.17g" % kv for kv in sorted(got.items()))), flush=True)
    if not ARGS.before and "drawn" in got:
        new_mode = got["drawModularSystem"] == 1
        want_field = "eq_new" if new_mode else "eq_old"
        check("P22_drawnFov_bit_exact_%s" % tag, got[want_field] == 1,
              "drawModularSystem=%d: drawnFov()=%.17g, halfFov x 360/pi=%.17g, projector fov=%.17g, %s=%d"
              % (got["drawModularSystem"], got["drawn"], got["newv"], got["projector"], want_field, got[want_field]))
    return got


def ring_tap(st, tag):
    rows = [r for r in st["ramp"] if r["active"] and r["dFov"] != 0]
    if rows:
        r = rows[-1]
        note("%s ring tap row: projector %.4f -> %.4f, camera %.4f -> %.4f deg (dFov %+.6f, dt %d)"
             % (tag, r["fov"], r["fovAfter"], 2 * math.degrees(r["halfFov"]), 2 * math.degrees(r["halfFovAfter"]),
                r["dFov"], r["dt"]))


def separate(tag):
    """Leg Z2 'same': one key step IN on a 60 -> 20 plan, then rest. Returns the rest state."""
    send(S, "zoom fov 60 duration 1", 4); fov_pair(tag + "_reset", 60.0)
    send(S, "zoom fov 20 duration 20", 3)
    inject([zoom_calls("In", 10)])
    tap = state(tag + "_tap", 0.5)
    ring_tap(tap, tag)
    note("%s: at the tap projector %.4f camera %.4f deg, projector auto-zoom %s, camera plan %.3f s"
         % (tag, tap["fov_old"], tap["fov_new"], tap["auto_zoom"], tap["zoom_duration"]))
    gdb_read(tag + "_tap")
    time.sleep(24)
    rest = state(tag + "_rest")
    apart = abs(rest["fov_new"] - rest["fov_old"])
    check(tag + "_engines_apart_at_rest", abs(rest["fov_old"] - 20.0) < 1e-3 and apart > 1.0,
          "projector %.4f camera %.4f deg (apart %.4f): the scene the readout checks need" % (rest["fov_old"], rest["fov_new"], apart))
    return rest


def drawn_of(st):
    return st["fov_new"] if ARGS.pin == "on" else st["fov_old"]


def leg():
    scene(); capture_ui()
    send(S, "zoom fov 60 duration 1", 4); fov_pair("G_start", 60.0)
    gdb_read("rest60")
    rest = separate("R3")
    gdb_read("R3_rest")
    d = drawn_of(rest)
    send(S, "zoom delta_fov 5", 2.5)
    st = state("R3_after")
    want = d + 5.0
    check("R3_delta_fov_aims_at_the_drawn_fov_plus_delta",
          abs(st["fov_old"] - want) < 1e-3 and abs(st["fov_new"] - want) < 1e-3,
          "drawn %.4f (pin %s) + 5 = %.4f; after: projector %.4f, camera %.4f deg"
          % (d, ARGS.pin, want, st["fov_old"], st["fov_new"]))
    rest = separate("R5")
    d = drawn_of(rest)
    send(S, "flag manual_zoom on", 1)
    send(S, "select planet Sun", 1.5)
    send(S, "zoom auto in manual on duration 1", 4)
    st = state("R5_after")
    want = d * 0.5
    check("R5_auto_zoom_in_manual_aims_at_half_the_drawn_fov",
          abs(st["fov_old"] - want) < 1e-3 and abs(st["fov_new"] - want) < 1e-3,
          "drawn %.4f (pin %s) x 0.5 = %.4f; after: projector %.4f, camera %.4f deg"
          % (d, ARGS.pin, want, st["fov_old"], st["fov_new"]))
    rest = separate("R4")
    d = drawn_of(rest)
    send(S, "position action save", 1)
    send(S, "zoom fov 90 duration 0", 2)
    fov_pair("R4_moved", 90.0)
    send(S, "position action load", 3)
    st = state("R4_after")
    check("R4_saved_fov_is_the_drawn_fov",
          abs(st["fov_old"] - d) < 1e-3 and abs(st["fov_new"] - d) < 1e-3,
          "saved while drawn %.4f (pin %s), projector %.4f, camera %.4f; after `position action load`: projector %.4f, camera %.4f deg"
          % (d, ARGS.pin, rest["fov_old"], rest["fov_new"], st["fov_old"], st["fov_new"]))


S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
try:
    leg()
finally:
    RESULTS["fails"] = FAILS
    json.dump(RESULTS, open("%s/f136_drv_results.json" % OUT, "w"), indent=1)
    S.close()
print("\n=== f136_drv pin %s%s: %s ===" % (ARGS.pin, " BEFORE" if ARGS.before else "",
      "ALL PASS" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
sys.exit(1 if FAILS else 0)
