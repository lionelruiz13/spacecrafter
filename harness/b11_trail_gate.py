#!/usr/bin/env python3
# B11 - the TRAIL RECORDING GATE.  Regression lock for the Vixy-ratified
# semantics (USER_QUESTIONS Q14 / INTENT 11.48(a) A1 -> 11.56):
#   "display gates recording: `flag object_trails off` STOPS accumulation, and
#    re-enabling starts FRESH"  (stated reason: the cost of accumulating a
#    trail nobody sees).
#
# AND for the fact that this gate is INDEPENDENT of the hidden/visible gate
# (Q13 / A10 / INTENT 11.54): a HIDDEN body still updates, therefore its trail
# still records.  Two conditions, two observables - reading them as one gate
# produces a wrong implementation (INTENT 13.B B11 says so in those words).
#
# OBSERVABLES (all from the running process, via `body action dual_dump` ->
# ModularBody::dumpTrace -> BodyModule::dumpState, INTENT 11.56):
#   new.trail[0].points          - buffer length
#   new.trail[0].recording       - THE recording gate's state
#   new.trail[0].fader           - THE display gate's state (dumped next to
#                                  `recording` so the two can be read apart)
#   new.trail[0].accumulateCount - entries into TrailModule::accumulate since
#                                  construction.  A FROZEN counter over an
#                                  interval in which simulated time advanced is
#                                  direct evidence that the accumulation code
#                                  did not run - the difference between "the
#                                  gate stopped the WORK" (what Vixy asked for)
#                                  and "the gate only stopped the DRAWING".
#   new.trail[0].head / headJD   - the newest recorded point + its sim time.
#                                  Compared against new.ecl / the header jd,
#                                  this proves a re-enable starts at the body's
#                                  CURRENT position with NO pre-off history.
#   new.relation                 - hide/show membership authority (<3 hidden)
#
# METHOD.  Time is FROZEN (`timerate rate 0`) and advanced only by explicit
# `date jday` jumps.  TrailModule::accumulate appends exactly ONE point per
# frame in which >= deltaTrail (1 sim-day) elapsed since the last point, so
# K jumps of 15 days == EXACTLY +K points on a recording body and +0 on a
# gated one.  The point count is therefore an exact, not statistical, count.
#
# SUBJECTS.  Both must carry a TRAIL module, which the deduce rule gives to
# non-satellite, non-Artificial bodies with orbit_visualization_period>0 (so
# NOT the Moon and NOT Phobos - B19's subjects have no trail):
#   Mars  - HIDDEN for the whole matrix   -> the "hidden" column
#   Venus - never hidden                  -> the "visible" column
#   Jupiter - never hidden, second visible witness
# The selection is pinned to the SUN because `setFlagTrails(true)` with a
# NON-star selection engages the old focus filter (ssystem_factory.hpp:327),
# which would override every other body's trail off and silently empty the
# matrix.  isStar() -> global, old getCenterObject parity.
#
# PRECONDITION: FRESH app launch (dirty instrument state invalidates the run),
# enable_tcp = true.  No init_fov requirement - the assertions below read
# dump-layer state; the screen A/B at the end is a self-referential px diff.
#
#   ./b11_run.sh b11_trail_gate.py <outdir>      (fresh launch under gdb)
#
# Exit 0 = every assertion passed.  <outdir>/b11_result.json = machine-readable.

import socket, time, json, math, sys, os

AU_KM = 149597870.7
J0 = 2461233.5      # same epoch as drive_scenes.py / b19_hidden_tick.py
STEP = 15.0         # sim-days per jump (>> deltaTrail=1, << maxTrail=1460)
K = 6               # jumps per measured interval
FADE = 4.0          # > LinearFader default duration (2000 ms) - "settled"
INFADE = 0.5        # < 2000 ms - a re-enable INSIDE the fade window

HIDDEN_SUBJ = "Mars"
VISIBLE_SUBJ = ["Venus", "Jupiter"]
ALL_SUBJ = [HIDDEN_SUBJ] + VISIBLE_SUBJ

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b11")
os.makedirs(OUT, exist_ok=True)

sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
jd_now = J0
n_flag_cmds = 0     # every `flag object_trails ...` sent - cross-checked
                    # against the gdb PROBE count (instrument chain)


def send(cmd, pause=0.5):
    global n_flag_cmds
    if cmd.startswith("flag object_trails"):
        n_flag_cmds += 1
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def dump(tag, pause=2.5):
    send(f"body action dual_dump filename {OUT}/b11_{tag}.json", pause)
    return tag


def shot(tag, pause=1.5):
    send(f"body action screenshot filename {OUT}/b11_{tag}.png", pause)


def jump(days=STEP, pause=0.6):
    global jd_now
    jd_now += days
    send("date jday %.9f" % jd_now, pause)


def advance(n=K):
    for _ in range(n):
        jump()


def load(tag):
    bodies, header = {}, None
    with open(os.path.join(OUT, f"b11_{tag}.json")) as f:
        for line in f:
            d = json.loads(line)
            if d.get("type") == "header":
                header = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
    return header, bodies


def tr(bodies, name):
    """The body's trail-module state (the dump carries a LIST; deduce makes
    exactly one, and a count != 1 is itself a finding, asserted below)."""
    t = bodies[name]["new"]["trail"]
    return t[0] if t else None


# ============================================================ drive =========
# --- phase 0: scene, frozen time, known flag state -------------------------
send("flag experimental_path on", 1)      # pin the NEW path (also stops A/B)
send("timerate rate 0", 1)                # FIRST, before the epoch (B19 note)
send("date jday %.9f" % J0, 1)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send("flag landscape off")
send("flag atmosphere off")
send("flag star_twinkle off")
send("flag show_fps off")
send("flag planet_names off")
send("flag subtitle off")
send("select planet Sun pointer off", 1)  # star -> NO focus filter (see header)
send("flag track_object on", 6)
send("zoom fov 26 duration 0", 3)
send("timerate rate 0", 1)                # re-assert after any startup.sts race
send("flag object_trails off", FADE)      # known baseline, faders settled

# --- phase 1: the OFF row of the 2x2 matrix -------------------------------
send(f"body name {HIDDEN_SUBJ} hidden true", 1.5)
dump("m_off_t0")
advance()
dump("m_off_t1")

# --- phase 2: the ON row of the same matrix (Mars still hidden) -----------
send("flag object_trails on", 2.5)
dump("m_on_t0")           # also the FRESH-start witness of cycle 0
advance()
dump("m_on_t1")

# --- phase 3: command-spelling negative control ---------------------------
# `flag trails off` does not exist.  If the interface swallows it silently the
# trail state is unchanged AND no PROBE planetsSetFlagTrails line appears.
send("flag trails off", 2.0)
dump("neg_spelling")
advance(2)
dump("neg_spelling_after")

# --- phase 4/5: the reversible pair, BOTH entries, twice ------------------
# on -> off -> on -> off -> on.  Cycle 2 starts from the state cycle 1's
# re-enable produced (no re-setup in between).
cycles = []
for c in (1, 2):
    dump(f"p{c}_pre_off")      # history that must be lost
    send("flag object_trails off", FADE)
    dump(f"p{c}_off")          # discarded: points == 0, recording == false
    advance(2)                 # sim time advances WHILE OFF - must add nothing
    dump(f"p{c}_off_advanced")
    send("flag object_trails on", 1.5)
    dump(f"p{c}_reon")         # FRESH: points == 1, head == current ecl
    advance()
    dump(f"p{c}_grown")        # and it grows again from there
    cycles.append(c)

# --- phase 6: re-enable INSIDE the fade window ----------------------------
# Q14 says a re-enable starts fresh.  A toggle 0.5 s after the off is still a
# re-enable, but the display fader has not reached 0 - so a fader-gated
# implementation keeps accumulating into the OLD buffer and this is the
# assertion that separates the two designs.
dump("fast_pre")
send("flag object_trails off", INFADE)
send("flag object_trails on", 1.5)
dump("fast_reon")

# --- phase 7: unhide, terminal observable (screen) ------------------------
send(f"body name {HIDDEN_SUBJ} hidden false", 1.5)
send("flag object_trails on", 1.0)
advance(30)                     # 450 sim-days of arc on every trailed planet
shot("screen_on_a")
shot("screen_on_b")             # same state, 1.5 s apart -> instrument floor
send("flag object_trails off", FADE)
shot("screen_off")
send("flag object_trails on", 2.0)
shot("screen_reon")             # fresh: 1 point, nothing drawable (n < 2)
advance(30)
shot("screen_regrown")
dump("screen_final")
sock.close()

# ========================================================== evaluate ========
fail = 0
report = {"step_days": STEP, "K": K, "matrix": {}, "checks": [],
          "flag_commands_sent": n_flag_cmds}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    report["checks"].append({"ok": bool(ok), "text": text})
    return ok


def d(a, b):
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3))) * AU_KM


print("\n=== 0. instrument chain: the dump carries a trail module at all ===")
h0, b0 = load("m_on_t1")
for n in ALL_SUBJ:
    t = bodies_t = bodies_n = None
    lst = b0[n]["new"]["trail"]
    check(len(lst) == 1, f"{n}: dumped trail modules = {len(lst)} (expect 1)")

print("\n=== 1. THE 2x2 MATRIX  {flag on,off} x {body visible,hidden} ===")
print("    (deltas over %d jumps of %g sim-days)" % (K, STEP))
for flag, t0tag, t1tag in (("off", "m_off_t0", "m_off_t1"),
                           ("on", "m_on_t0", "m_on_t1")):
    ha, ba = load(t0tag)
    hb, bb = load(t1tag)
    djd = hb["jd"] - ha["jd"]
    check(abs(djd - K * STEP) < 1e-6,
          f"time control: flag {flag} interval djd={djd:.6f} d "
          f"(commanded {K*STEP:.6f})")
    for n in ALL_SUBJ:
        vis = "hidden" if n == HIDDEN_SUBJ else "visible"
        ta, tb = tr(ba, n), tr(bb, n)
        rel = bb[n]["new"]["relation"]
        dpts = tb["points"] - ta["points"]
        dacc = tb["accumulateCount"] - ta["accumulateCount"]
        cell = {"flag": flag, "visibility": vis, "body": n,
                "points_t0": ta["points"], "points_t1": tb["points"],
                "dpoints": dpts, "daccumulateCount": dacc,
                "recording_t1": tb["recording"], "fader_t1": tb["fader"],
                "relation_t1": rel, "djd_days": djd}
        report["matrix"][f"{flag}/{vis}/{n}"] = cell
        print(f"  [flag {flag:3s} | {vis:7s}] {n:8s} points {ta['points']:3d}"
              f" -> {tb['points']:3d} (d={dpts:+3d})  accumulateCount d="
              f"{dacc:+6d}  recording={tb['recording']}  fader={tb['fader']:.3f}"
              f"  relation={rel}")
        # membership: the hidden subject really is hidden (instrument chain -
        # without this a mistyped hide makes the whole column pass vacuously)
        check((rel < 3) == (vis == "hidden"),
              f"membership: {n} relation={rel} expected {vis}")
        if flag == "on":
            check(dpts == K, f"[on|{vis}] {n}: +{dpts} points over {K} jumps "
                             f"(expect +{K})")
            check(dacc > 0, f"[on|{vis}] {n}: accumulate() ran {dacc} times")
            check(tb["recording"] is True, f"[on|{vis}] {n}: recording=True")
        else:
            check(dpts == 0, f"[off|{vis}] {n}: {dpts:+d} points over {K} "
                             f"jumps (expect 0 - accumulation STOPPED)")
            check(dacc == 0, f"[off|{vis}] {n}: accumulate() ran {dacc} times "
                             f"(expect 0 - THE WORK stopped, not just the draw)")
            check(tb["recording"] is False, f"[off|{vis}] {n}: recording=False")
            check(tb["points"] == 0, f"[off|{vis}] {n}: buffer empty "
                                     f"({tb['points']} points)")

print("\n--- independence of the two axes (the row's claim) ---")
for flag in ("on", "off"):
    hv = report["matrix"][f"{flag}/hidden/{HIDDEN_SUBJ}"]
    for n in VISIBLE_SUBJ:
        vv = report["matrix"][f"{flag}/visible/{n}"]
        check(hv["dpoints"] == vv["dpoints"],
              f"flag {flag}: hidden {HIDDEN_SUBJ} dpoints={hv['dpoints']} == "
              f"visible {n} dpoints={vv['dpoints']} (visibility axis is inert)")
on_v = report["matrix"][f"on/visible/{VISIBLE_SUBJ[0]}"]["dpoints"]
off_v = report["matrix"][f"off/visible/{VISIBLE_SUBJ[0]}"]["dpoints"]
check(on_v != off_v, f"flag axis IS the gate: on {on_v:+d} vs off {off_v:+d} "
                     f"points over the same {K} jumps")

print("\n=== 2. command spelling (from the running process) ===")
hs, bs = load("neg_spelling")
ha2, ba2 = load("neg_spelling_after")
for n in ALL_SUBJ:
    t0, t1 = tr(bs, n), tr(ba2, n)
    check(t0["recording"] is True and t1["recording"] is True,
          f"bogus `flag trails off` did not stop {n} (recording still True)")
    check(t1["points"] - t0["points"] == 2,
          f"bogus `flag trails off` did not gate {n} "
          f"(+{t1['points']-t0['points']} points over 2 jumps)")
report["spelling"] = {"working": "flag object_trails on|off",
                      "bogus_tested": "flag trails off"}

print("\n=== 3. FRESH restart, both entries of the pair, twice ===")
for c in cycles:
    hpre, bpre = load(f"p{c}_pre_off")
    hoff, boff = load(f"p{c}_off")
    hadv, badv = load(f"p{c}_off_advanced")
    hre, bre = load(f"p{c}_reon")
    hgr, bgr = load(f"p{c}_grown")
    print(f"  -- cycle {c} --")
    for n in ALL_SUBJ:
        tp, to, tadv, trn, tg = (tr(bpre, n), tr(boff, n), tr(badv, n),
                                 tr(bre, n), tr(bgr, n))
        print(f"    {n:8s} pre_off={tp['points']:3d} off={to['points']:3d} "
              f"off_advanced={tadv['points']:3d} reon={trn['points']:3d} "
              f"grown={tg['points']:3d}  accCount off->adv "
              f"{tadv['accumulateCount']-to['accumulateCount']:+d}")
        check(tp["points"] >= 2,
              f"c{c} {n}: there WAS history to lose ({tp['points']} points)")
        check(to["points"] == 0 and to["recording"] is False,
              f"c{c} {n}: off -> buffer DISCARDED (points={to['points']}, "
              f"recording={to['recording']})")
        check(tadv["accumulateCount"] == to["accumulateCount"],
              f"c{c} {n}: no accumulate() call while off over 2 jumps")
        check(trn["points"] == 1,
              f"c{c} {n}: re-enable -> buffer length {trn['points']} (expect 1 "
              f"- starts from 0, not from {tp['points']})")
        # the first post-re-enable point is at the CURRENT position...
        err = d(trn["head"], bre[n]["new"]["ecl"])
        check(err < 1e-3, f"c{c} {n}: first point is at the body's CURRENT "
                          f"position (|head-ecl| = {err:.6f} km)")
        # ...and at the body's CURRENT evaluation time.  The reference is the
        # body's OWN lastJD, not the observer header jd: flag_light_travel_time
        # is on (config default), so the trail samples at the light-time-
        # corrected date (getLastJD, TrailModule.cpp:121) - the head jd equals
        # the body's lastJD exactly, offset from the header clock by the body's
        # light-travel time (measured: Mars ~6 min, Venus ~9.5 min, Jupiter
        # ~37 min - scales with distance, INTENT 11.56).
        body_jd = bre[n]["new"]["lastJD"]
        check(abs(trn["headJD"] - body_jd) < 1e-6,
              f"c{c} {n}: first point jd {trn['headJD']:.9f} == body eval time "
              f"{body_jd:.9f} (header jd {hre['jd']}, light-time offset)")
        check(trn["headJD"] > tp["headJD"] + 1e-6,
              f"c{c} {n}: no pre-off history survived (head jd advanced "
              f"{trn['headJD']:.6f} > pre-off {tp['headJD']:.6f})")
        check(tg["points"] == 1 + K,
              f"c{c} {n}: regrows from the fresh start ({tg['points']} points "
              f"after {K} jumps)")
        report["checks"].append({"cycle": c, "body": n,
                                 "pre_off": tp["points"], "off": to["points"],
                                 "reon": trn["points"], "grown": tg["points"]})

print("\n=== 4. re-enable INSIDE the fade window (still fresh) ===")
hfp, bfp = load("fast_pre")
hfr, bfr = load("fast_reon")
for n in ALL_SUBJ:
    tp, trn = tr(bfp, n), tr(bfr, n)
    print(f"    {n:8s} pre={tp['points']:3d} -> after off+{INFADE}s+on: "
          f"{trn['points']:3d}  fader={trn['fader']:.3f}")
    check(trn["points"] == 1,
          f"fast toggle {n}: {trn['points']} points (expect 1 - a re-enable "
          f"inside the fade is still a re-enable; was {tp['points']})")

report["fail"] = fail
with open(os.path.join(OUT, "b11_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail) + ' ASSERTION(S) FAILED'} "
      f"- {OUT}/b11_result.json")
print(f"flag object_trails commands sent: {n_flag_cmds} "
      f"(cross-check against PROBE planetsSetFlagTrails lines in gdb.log)")
sys.exit(1 if fail else 0)
