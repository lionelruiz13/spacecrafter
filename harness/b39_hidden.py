#!/usr/bin/env python3
# B39 - `hidden` means AS-IF-NONEXISTENT in the RENDERED universe.
# Regression lock for the Vixy-decided semantics (D23 -> INTENT §11.113(b),
# verbatim: "hidden body should behave as if they didn't exist while hidden, and
# behave as if they never were hidden when unhidden. For performance reason,
# hidden bodies shouldn't tick. Hiding a body implicitly hide his child body as
# a side effect of nesting, but mustn't change the exposed hidden attribute/
# flag."), implemented as §13.B B39 / INTENT §11.117.  §5.31 is the trigger.
#
# WHAT EACH LEG ASSERTS, and how it can FAIL
# ------------------------------------------
# P1 INSTRUMENT CHAIN.  `relation` (BodyRelation: <3 hidden, >=3 shown) actually
#    flipped on every hide/show, and the DECLARED value of a body hidden only by
#    NESTING is UNCHANGED (D23's last clause: implicit child hiding must not
#    touch the exposed flag).  Without this every other leg could pass
#    vacuously on a swallowed command (the §11.54(j) class).
#
# P2 ORBIT LINE - THE §5.31 DISCRIMINATOR, red -> green.
#    ONE body's orbit line is armed through the per-name override
#    (`body name Mars orbit true` -> OrbitModule::setShown), the GLOBAL
#    planet_orbits flag stays off, so the orbit pass carries exactly one line
#    and the px diff is that line.  Two states, same pair of shots:
#      shown  : diff(orbit armed, orbit disarmed) = CONTROL, must be >> floor
#               in EVERY binary - it is what proves the instrument sees orbit
#               lines at all (a fix that killed all orbit lines fails here).
#      hidden : the same diff = SUBJECT.  Pre-fix >> floor (the defect: the line
#               sweep lacks the visibility test its TRACE sibling has,
#               ModularSystem.cpp), post-fix == floor.
#
# P3 TRAIL - the RECORDING half (B39(2), §11.114; supersedes §11.56's hidden
#    half).  `flag object_trails on`, then simulated time is jumped: a SHOWN
#    body's TrailModule::accumulateCount grows, a HIDDEN body's must be FROZEN.
#    Pre-fix both grow identically (§11.56's measured 2x2 matrix) - this leg is
#    the inversion, and `b11_trail_gate.py`'s hidden-Mars leg is re-pointed in
#    the same commit (never loosened, the P6 pattern).
#
# P4 SELECTION POINTER on a body hidden BY NESTING (Io under a hidden Jupiter) -
#    the ancestor case, which `relation` alone cannot see.  diff(pointer on,
#    pointer off) in each state; the parent's disc is absent from BOTH shots of
#    a state, so it cancels.  Precondition asserted from the dump: the subject
#    was `visible` at the moment its parent was hidden (otherwise the leg is
#    vacuous).  Same authority (ModularBody::operator bool) as click-picking.
#
# P5 NO TICK (D23's performance clause) - `evalCount` = entries into
#    ModularBody::transformParentToBodyPos/transformBodyToParent, the orbit
#    evaluation counter (the `accumulateCount` instrument class, §11.56).  Over
#    an interval of hundreds of rendered frames a SHOWN body's counter grows by
#    ~one per frame; a HIDDEN body's must grow by ONLY the use-site barrier's
#    own evaluations (5 per use: 1 + the §11.76(b) +4 extra iterations).
#
# P6 REVERSIBLE PAIR, traversed TWICE, the second entry starting from the state
#    the first exit produced (hide -> show -> hide -> show), with the position
#    freshness of `b19_hidden_tick.py`'s criterion re-checked at every entry:
#    |ecl_new - ecl_old| stays inside a tolerance calibrated IN THE RUN from
#    never-hidden control bodies.
#
# PRECONDITION: FRESH launch, enable_tcp, projection FISHEYE, the NEW path
# pinned.  Run through ./b39_run.sh (which asserts the field-data md5 in==out).
# Exit 0 = every assertion passed.  <outdir>/b39_result.json = machine-readable.

import socket, time, json, math, sys, os
import numpy as np
from PIL import Image

AU_KM = 149597870.7
J0 = 2461233.5            # same epoch as drive_scenes.py / b19_hidden_tick.py
DT = 20.0 / 1440.0        # 20 simulated minutes, B19's own interval
THR = 32                  # px threshold; the floor is MEASURED below
BARRIER_EVALS = 5         # 1 + the §11.76(b) "+4 extra iterations"

HERE = os.path.dirname(os.path.abspath(__file__))
# ABSOLUTE: the app writes screenshots/dumps from ITS OWN cwd, not the driver's
# (a relative outdir silently lands elsewhere - measured on the first run).
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "b39")
os.makedirs(OUT, exist_ok=True)
sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)

fail = 0
report = {"legs": {}}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    return ok


def send(cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=1.8):
    send(f"body action screenshot filename {OUT}/b39_{tag}.png", pause)
    return tag


def dump(tag, pause=2.2):
    send(f"body action dual_dump filename {OUT}/b39_{tag}.json", pause)
    return tag


def bodies(tag):
    out, hdr = {}, None
    with open(os.path.join(OUT, f"b39_{tag}.json")) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            d = json.loads(line)
            if d.get("type") == "header":
                hdr = d
            elif d.get("type") == "body":
                out[d["name"]] = d
    return hdr, out


def img(tag):
    return np.asarray(Image.open(os.path.join(OUT, f"b39_{tag}.png"))
                      .convert("RGB")).astype(np.int32)


def dpx(a, b, thr=THR):
    d = np.abs(img(a) - img(b)).max(axis=2)
    return int((d > thr).sum()), int(d.max())


def nonblack(tag):
    return int((img(tag).max(axis=2) > 8).sum())


def ecl_err_km(rec):
    """|ecl_new - ecl_old| in km - b19's criterion D, re-used here."""
    if rec.get("new") is None or rec.get("old") is None:
        return None
    a, b = rec["new"]["ecl"], rec["old"]["ecl"]
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3))) * AU_KM


def trail_of(rec):
    t = rec["new"].get("trail") or []
    return t[0] if t else None


# =========================================================== scene ==========
send("flag experimental_path on", 1)      # pin the NEW path (no A/B alternation)
send("timerate rate 0", 1)                # freeze BEFORE the epoch (b19's note)
send(f"date jday {J0:.9f}", 1)
send("flag landscape off", 1)             # never compare masked content
send("flag atmosphere off", 1)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
# AIM. Calibration probe (§11.117): at the DEFAULT view direction the whole
# orbit pass lands 0 px - `flag planet_orbits on` measured (0, 0) there and
# 15995 px once the view was aimed, i.e. an un-aimed orbit leg would have
# "passed" on a scene that never showed the subject (§11.111(i2)'s trap).
# Aim = select the Sun, track, RELEASE (tracking is the aim, not the state to
# shoot in - §11.80(c)/§11.101(g3)); the Sun stays selected, which is also what
# §11.56(b) requires for the trail matrix (the star-selected global path).
send("select planet Sun pointer off", 1)
send("flag track_object on", 5)
send("flag track_object off", 1.5)

# ------------------------------------------------ P0: measured noise floor --
shot("floor_a"); shot("floor_b")
floor_px, floor_max = dpx("floor_a", "floor_b")
nb = nonblack("floor_a")
print(f"\nnoise floor: px>{THR} = {floor_px}, max|d| = {floor_max}, "
      f"nonblack px = {nb}")
check(nb > 20000, f"P0 scene carries content ({nb} nonblack px) - "
                  f"the diffs below are content-vs-content, not masked fiction")
report["legs"]["P0_floor"] = {"px": floor_px, "max": floor_max, "nonblack": nb}

# ================================================= P2: the orbit line =======
# Positive control FIRST, in the shown state.
send("body name Mars orbit true", 3.5)     # per-name arm; global flag stays off
shot("orb_shown_on")
send("body name Mars orbit false", 3.5)    # fade out
shot("orb_shown_off")
ctl_px, ctl_max = dpx("orb_shown_on", "orb_shown_off")

# Subject: the same pair with Mars HIDDEN.
send("body name Mars hidden true", 1.5)
d_hid_arm = dump("orb_hidden_pre")
send("body name Mars orbit true", 3.5)
shot("orb_hidden_on")
send("body name Mars orbit false", 3.5)
shot("orb_hidden_off")
sub_px, sub_max = dpx("orb_hidden_on", "orb_hidden_off")
send("body name Mars hidden false", 1.5)
send("body name Mars orbit false", 2.5)

_, b = bodies(d_hid_arm)
check(b["Mars"]["new"]["relation"] < 3,
      f"P2 instrument: Mars relation={b['Mars']['new']['relation']} (hidden) "
      f"while the subject pair was shot")
print(f"\nP2 orbit line: CONTROL(shown) px>{THR} = {ctl_px} (max {ctl_max})  |  "
      f"SUBJECT(hidden) px>{THR} = {sub_px} (max {sub_max})  |  floor {floor_px}")
check(ctl_px > 200, f"P2 control: an ARMED orbit line on a SHOWN body is "
                    f"{ctl_px} px > 200 (the instrument sees orbit lines; the "
                    f"calibration probe measured 779 px for Mars in this scene)")
check(sub_px <= floor_px, f"P2 SUBJECT: a HIDDEN body contributes {sub_px} px "
                          f"<= floor {floor_px} (as-if-nonexistent)")
report["legs"]["P2_orbit"] = {"control_px": ctl_px, "control_max": ctl_max,
                              "subject_px": sub_px, "subject_max": sub_max,
                              "floor_px": floor_px}

# ============================================ P3: trail RECORDING gate ======
# The jump step is 15 SIMULATED DAYS - §11.56(b)'s own step, and the step the
# instrument requires: TrailModule's shipped `deltaTrail` is 1 DAY, so a 20 min
# jump appends NO point (measured: dpoints 0 on every body, the first version of
# this leg) and only `accumulateCount` would have moved.  With 15 d both
# observables are live: `accumulateCount` = "the code ran", `points` = "a sample
# landed in the history".
DT_TRAIL = 15.0
send("flag object_trails on", 2.0)
t0 = dump("trail_t0")
for k in range(1, 4):                      # let both bodies record
    send(f"date jday {J0 + k * DT_TRAIL:.9f}", 1.4)
t1 = dump("trail_t1")
send("body name Mars hidden true", 1.5)
# The measured interval starts HERE, after the hide: between the t1 dump and the
# hide command the app renders ~350 more frames, and the subject records in all
# of them (measured 346 - correctly, it was still shown). Both ends of the
# interval below are dumps taken while the subject is hidden, so any nonzero
# delta is a real leak, not a boundary artifact.
t1b = dump("trail_t1b")
base = J0 + 3 * DT_TRAIL
for k in range(1, 5):                      # the hidden interval
    send(f"date jday {base + k * DT_TRAIL:.9f}", 1.4)
t2 = dump("trail_t2")
send("body name Mars hidden false", 2.5)   # unhide IS a use (D23 clause iv)
t3 = dump("trail_t3")
send("flag object_trails off", 1.5)

_, B0 = bodies(t0); _, B1 = bodies(t1); _, B1b = bodies(t1b)
_, B2 = bodies(t2); _, B3 = bodies(t3)
rec = {}
for name, state in (("Mars", "hidden"), ("Venus", "shown"), ("Jupiter", "shown")):
    a, b_, c = trail_of(B0[name]), trail_of(B1b[name]), trail_of(B2[name])
    a0 = trail_of(B1[name])
    if not (a and b_ and c and a0):
        check(False, f"P3 {name}: no TRAIL module in the dump (leg vacuous)")
        continue
    grow_shown = a0["accumulateCount"] - a["accumulateCount"]
    grow_test = c["accumulateCount"] - b_["accumulateCount"]
    dpoints = c["points"] - b_["points"]
    print(f"  P3 {name:8s} ({state:6s}) dAcc[shown interval]={grow_shown:6d}  "
          f"dAcc[test interval]={grow_test:6d}  points {b_['points']}->"
          f"{c['points']} (d={dpoints:+d})")
    rec[name] = {"state": state, "dAcc_shown": grow_shown,
                 "dAcc_test": grow_test, "dpoints_test": dpoints,
                 "points_t1": b_["points"], "points_t2": c["points"]}
    check(grow_shown > 0, f"P3 {name}: recorded while shown "
                          f"(dAcc={grow_shown} > 0) - the gate is armed")
    if state == "hidden":
        check(grow_test == 0, f"P3 SUBJECT {name}: HIDDEN interval dAcc="
                              f"{grow_test} == 0 (the work stopped)")
        check(dpoints == 0, f"P3 SUBJECT {name}: HIDDEN interval dpoints="
                            f"{dpoints} == 0 (no sample entered the history)")
    else:
        check(grow_test > 0, f"P3 control {name}: kept recording "
                             f"(dAcc={grow_test} > 0) - only the hidden body stopped")
        check(dpoints > 0, f"P3 control {name}: history grew by {dpoints} "
                           f"samples over the same interval")
# "as if they never were hidden when unhidden" on ACCUMULATED HISTORY
# (§11.113(b)(iv): "recoverable ... by evaluating the same sample times").
#
# The count is asserted against a PREDICTION, not against the control's count,
# and the difference is itself a measured fact worth stating: accumulate() appends
# AT MOST ONE sample per call, so a never-hidden body driven by 15-day date JUMPS
# records 4 samples over 60 days - it is UNDER-sampled relative to its own
# declared cadence, and the source says so ("detail lost on big jumps, by
# design"). The reconstruction uses the module's DECLARED cadence (deltaTrail),
# which is what the trail would have had under continuous time; the two agree
# wherever time flows continuously and diverge only in the jump regime.
# So: predicted = pre-hide samples + floor(missed span / deltaTrail).
mars1b = trail_of(B1b["Mars"])
mars3, ven3 = trail_of(B3["Mars"]), trail_of(B3["Venus"])
mars_now = B3["Mars"]["new"]["lastJD"]        # the body's own (retarded) sim date
missed = int((mars_now - mars1b["headJD"]) / mars1b["deltaTrail"])
predicted = min(mars1b["maxTrail"], mars1b["points"] + missed)
print(f"  P3 after unhide: Mars points={mars3['points']} (predicted {predicted} = "
      f"{mars1b['points']} pre-hide + {missed} missed at deltaTrail="
      f"{mars1b['deltaTrail']} d)  headJD={mars3['headJD']}  |  Venus "
      f"points={ven3['points']} headJD={ven3['headJD']}")
check(mars3["points"] == predicted,
      f"P3 as-if-never-hidden: Mars history {mars3['points']} samples == the "
      f"predicted {predicted} (the missed span reconstructed, D23 clause iv)")
check(mars3["points"] > mars1b["points"],
      f"P3 as-if-never-hidden: the history GREW ({mars1b['points']} -> "
      f"{mars3['points']}) - it was neither discarded nor left with a gap")
check(mars3["headJD"] is not None and ven3["headJD"] is not None
      and abs(mars3["headJD"] - ven3["headJD"]) < 0.2,
      f"P3 as-if-never-hidden: newest sample dates agree to "
      f"{abs((mars3['headJD'] or 0) - (ven3['headJD'] or 0)):.6f} d < 0.2 d "
      f"(light-time offsets differ per body)")
# ... and the reconstructed samples are ON THE ORBIT, not merely present. The
# polyline LENGTH over its own SPAN is the body's mean orbital speed, and the
# change in that mean between the pre-hide arc and the reconstructed one is
# PREDICTED, not banded: for a two-body orbit the transverse speed is h/r with h
# constant, so mean speed scales as mean(1/r) - and r is measured in this same run
# (|ecl| at the span ends). Everything here is measured in-run; no external or
# recalled datum enters (§11.51(d)).
# Second-order terms, both derived and both below the band:
#   - the radial speed component adds sqrt(1+(dr/dt / v)^2), 8.9 % vs 8.96 % of v
#     on the two spans, so it cancels to ~0.01 % in the RATIO;
#   - the pre-hide leg's chords span 15 d and under-measure the arc by
#     1-sin(x)/x with x = half the swept angle (360/686.98*15 deg from the
#     ini-declared orbit_visualization_period -> 0.079 %); the reconstructed
#     1-day chords by 3e-6.
#   - mean(1/r) is taken as the endpoint trapezoid, which is the crudest step.
# A reconstruction placed anywhere but on the orbit misses this by orders of
# magnitude; one that left a gap shows up as a chord shortcut.
def rad(rec):
    e = rec["new"]["ecl"]
    return math.sqrt(sum(x * x for x in e))
r_start, r_mid, r_end = rad(B0["Mars"]), rad(B1b["Mars"]), rad(B3["Mars"])
sp_pre = mars1b["pathLength"] / (mars1b["headJD"] - mars1b["tailJD"])
sp_post = mars3["pathLength"] / (mars3["headJD"] - mars3["tailJD"])
ratio = sp_post / sp_pre
predicted = (2.0 / (r_start + r_end)) / (2.0 / (r_start + r_mid))  # mean(1/r) ratio
err = abs(ratio - predicted) / predicted
print(f"  P3 mean speed from the polyline: pre-hide {sp_pre:.6e} AU/d over "
      f"{mars1b['headJD'] - mars1b['tailJD']:.3f} d  |  after reconstruction "
      f"{sp_post:.6e} AU/d over {mars3['headJD'] - mars3['tailJD']:.3f} d")
print(f"  P3 ratio measured {ratio:.6f} vs PREDICTED {predicted:.6f} from "
      f"mean(1/r) with r = {r_start:.7f} / {r_mid:.7f} / {r_end:.7f} AU "
      f"-> {err:.4%}")
check(err < 0.005,
      f"P3 reconstruction is ON THE ORBIT: measured speed ratio {ratio:.6f} "
      f"matches the h/r prediction {predicted:.6f} to {err:.4%} < 0.5 %")
rec["after_unhide"] = {
    "mars_points": mars3["points"], "predicted_points": predicted,
    "missed": missed, "venus_points": ven3["points"],
    "mars_headJD": mars3["headJD"], "venus_headJD": ven3["headJD"],
    "speed_pre_AU_per_d": sp_pre, "speed_post_AU_per_d": sp_post,
    "speed_ratio_measured": ratio, "speed_ratio_predicted": predicted,
    "speed_ratio_err": err,
    "r_start_AU": r_start, "r_mid_AU": r_mid, "r_end_AU": r_end}
report["legs"]["P3_trail"] = rec

# ======================================== P4: pointer on a NESTED-hidden body
send("select planet Jupiter pointer off", 1.0)
send("flag track_object on", 4.0)
send("flag track_object off", 1.5)         # aim, then RELEASE (§11.80(c))
send("select planet Io pointer on", 1.5)
d_aim = dump("ptr_aim")
_, BA = bodies(d_aim)
io_visible = BA["Io"]["new"]["visible"]
io_rel = BA["Io"]["new"]["relation"]
check(io_visible, f"P4 precondition: Io visible={io_visible} before the parent "
                  f"is hidden (a false here makes the leg vacuous)")
shot("ptr_shown_on")
send("select planet Io pointer off", 1.5)
shot("ptr_shown_off")
p_ctl_px, p_ctl_max = dpx("ptr_shown_on", "ptr_shown_off")

send("body name Jupiter hidden true", 1.5)
d_nest = dump("ptr_nested")
_, BN = bodies(d_nest)
send("select planet Io pointer on", 1.5)
shot("ptr_hidden_on")
send("select planet Io pointer off", 1.5)
shot("ptr_hidden_off")
p_sub_px, p_sub_max = dpx("ptr_hidden_on", "ptr_hidden_off")
send("body name Jupiter hidden false", 1.5)

# D23's last clause, measured: the DECLARED flag of the nested child is untouched
check(BN["Io"]["new"]["relation"] == io_rel,
      f"P1 declared flag: Io relation stayed {BN['Io']['new']['relation']} "
      f"(== {io_rel}) while its parent was hidden - implicit child hiding does "
      f"NOT touch the exposed attribute (D23)")
check(BN["Jupiter"]["new"]["relation"] < 3,
      f"P1 instrument: Jupiter relation={BN['Jupiter']['new']['relation']} (hidden)")
print(f"\nP4 pointer: CONTROL(parent shown) px>{THR} = {p_ctl_px} (max "
      f"{p_ctl_max})  |  SUBJECT(parent hidden) px>{THR} = {p_sub_px} "
      f"(max {p_sub_max})")
check(p_ctl_px > 20, f"P4 control: the pointer on a SHOWN Io is {p_ctl_px} px "
                     f"> 20 (the instrument sees the pointer)")
check(p_sub_px <= floor_px, f"P4 SUBJECT: pointer on a NESTED-hidden Io is "
                            f"{p_sub_px} px <= floor {floor_px}")
report["legs"]["P4_pointer"] = {"io_visible": io_visible,
                                "control_px": p_ctl_px, "subject_px": p_sub_px,
                                "floor_px": floor_px}

# ================================================= P5: the tick is retired ==
# evalCount over an interval of ~hundreds of rendered frames at a FROZEN date:
# a shown body is evaluated once per frame, a body outside every walk only at a
# USE.  Subject = JUPITER, so the same measurement also carries D23's nesting
# clause: IO (declared shown, hidden only by its parent) must freeze too, and
# that is the half `relation` cannot see.
send("body name Jupiter hidden true", 1.5)
e1 = dump("eval_t1")
time.sleep(12.0)                            # real frames, no commands, no dumps
e2 = dump("eval_t2")
send("body name Jupiter hidden false", 1.5)
_, E1 = bodies(e1); _, E2 = bodies(e2)
delta = {n: E2[n]["new"]["evalCount"] - E1[n]["new"]["evalCount"]
         for n in ("Jupiter", "Io", "Venus", "Mars")}
print(f"\nP5 evalCount over a ~12 s no-command interval: {delta}")
check(delta["Venus"] > 100, f"P5 control: shown Venus was evaluated "
                            f"{delta['Venus']} times (> 100 rendered frames - "
                            f"the instrument counts)")
check(delta["Mars"] > 100, f"P5 control: shown Mars was evaluated "
                           f"{delta['Mars']} times (> 100)")
check(delta["Jupiter"] <= BARRIER_EVALS,
      f"P5 SUBJECT: HIDDEN Jupiter was evaluated {delta['Jupiter']} times "
      f"<= {BARRIER_EVALS} (only the use-site barrier of the one dump)")
check(delta["Io"] <= BARRIER_EVALS,
      f"P5 SUBJECT (nesting): Io - declared SHOWN, hidden only by its parent - "
      f"was evaluated {delta['Io']} times <= {BARRIER_EVALS}")
report["legs"]["P5_notick"] = {"delta": delta, "barrier_evals": BARRIER_EVALS}

# ============================== P6: reversible pair, twice, freshness at use =
CONTROL = ["Venus", "Jupiter", "Io"]
tags, states = [], []
jd = J0 + 8 * DT
send(f"date jday {jd:.9f}", 1.5)
for entry in (1, 2):
    send("body name Mars hidden true", 1.2)
    jd += DT; send(f"date jday {jd:.9f}", 1.5)
    tags.append(dump(f"rev{entry}_hidden")); states.append("hidden")
    send("body name Mars hidden false", 1.2)
    jd += DT; send(f"date jday {jd:.9f}", 1.5)
    tags.append(dump(f"rev{entry}_shown")); states.append("shown")

ctl = []
for t in tags:
    _, BB = bodies(t)
    for n in CONTROL:
        e = ecl_err_km(BB[n])
        if e is not None:
            ctl.append(e)
tol_km = max(10.0 * max(ctl), 1.0)
print(f"\nP6 tolerance calibrated in-run from never-hidden {CONTROL}: "
      f"max old-vs-new {max(ctl):.4f} km -> tol {tol_km:.4f} km")
rev = []
for t, st in zip(tags, states):
    _, BB = bodies(t)
    rel = BB["Mars"]["new"]["relation"]
    err = ecl_err_km(BB["Mars"])
    ok_rel = (rel < 3) == (st == "hidden")
    check(ok_rel, f"P6 {t}: Mars relation={rel} matches expected {st}")
    check(err <= tol_km, f"P6 {t}: Mars |ecl_new-ecl_old| = {err:.3f} km "
                         f"<= {tol_km:.3f} km (fresh AT THE USE, {st})")
    rev.append({"tag": t, "state": st, "relation": rel, "err_km": err})
report["legs"]["P6_reversible"] = {"tol_km": tol_km, "control_max_km": max(ctl),
                                   "entries": rev}

sock.close()
report["fail"] = fail
with open(os.path.join(OUT, "b39_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail) + ' ASSERTION(S) FAILED'} "
      f"- {OUT}/b39_result.json")
sys.exit(1 if fail else 0)
