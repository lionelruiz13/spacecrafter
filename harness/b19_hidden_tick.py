#!/usr/bin/env python3
# B19 - hidden bodies keep ticking.  Regression lock for the Vixy-ratified
# semantics (USER_QUESTIONS Q13 / INTENT 11.48(a) A10, verbatim answer:
# "It should be where it is now"; the deciding case Vixy named is "hide a body
# early in a show, run twenty minutes of simulated time, show it again").
# INTENT.md 11.54 / 13.B B19.  Supersedes the "suspended" state of 11.15b(b).
#
# WHAT IS ASSERTED, and why a freeze optimisation cannot pass it
# --------------------------------------------------------------
# The observable is `ecl` = ModularBody::eclipticPos [ModularBody.cpp:569-570],
# the body's PARENT-RELATIVE POSITION STATE.  It is written in exactly one
# place per direction - transformParentToBodyPos / transformBodyToParent
# (ModularBody.hpp:404-405 and 539-540), immediately after the orbit is
# evaluated at jd.  Any optimisation that stops ticking a hidden body stops
# calling that code, so `ecl` keeps the value it had at hide time.  It is
# therefore NOT a proxy: it IS the state a re-shown body would be drawn at.
# `lastJD` (same two write sites) is dumped alongside as a corroborating
# witness, never as the primary criterion.
#
# Four assertion families, per hidden interval:
#   A membership   - `relation` (BodyRelation: <3 hidden, >=3 visible) actually
#                    flipped.  This is the instrument-chain check: without it a
#                    mistyped hide command would make every other assert pass
#                    vacuously (the dump iterates the name registry, which
#                    contains hidden bodies - harness/README.md).
#   B time control - every dump's header jd equals the commanded value, so the
#                    only jd movement in the run is the commanded one
#                    (`timerate rate 0` is set AFTER the auto-played
#                    startup.sts sets `timerate rate 1`).
#   C advance      - |ecl(t1)-ecl(t0)| on the NEW path, compared against the
#                    OLD path's own advance over the same interval (the old
#                    path is the reference implementation of the ratified
#                    behavior: solarsystem_display.cpp:343-357 computes every
#                    body every frame, hidden or not).
#   D "where it is now" - |ecl_new(t1) - ecl_old(t1)|, i.e. the hidden body is
#                    at the position the ticking path says it should be at NOW,
#                    not where it was hidden.  Tolerance is calibrated IN THE
#                    RUN from never-hidden control bodies (their own old/new
#                    disagreement), never guessed.
#
# Both entries of the reversible pair are covered: hide -> show -> hide -> show,
# the second hide starting from the state the first show produced.
#
# PRECONDITION: app fresh-launched (dirty instrument state invalidates the run),
# enable_tcp = true, projection FISHEYE.  No init_fov requirement: this scene
# reads mat-layer position state only, never screen-layer px.
#
#   DISPLAY=:2 ./build-claude/src/spacecrafter &
#   ./b19_hidden_tick.py [outdir]        # default outdir: harness/artifacts/b19
#
# Exit 0 = all assertions pass, 1 = at least one failed (details on stdout,
# machine-readable summary in <outdir>/b19_result.json).

import socket, time, json, math, sys, os

AU_KM = 149597870.7
J0 = 2461233.5          # same epoch as drive_scenes.py / scene_e_spine.py
DT_MIN = 20.0           # Vixy's deciding case: "twenty minutes of simulated time"
DT = DT_MIN / 1440.0    # days

# Hidden subjects.  Moon = direct child of the camera reference (Earth).
# Phobos = grandchild reached only through the HIDDEN parent Mars: it proves
# the whole hidden SUBTREE keeps ticking, not just the hidden node.
SUBJECTS = ["Moon", "Phobos"]
HIDE = ["Moon", "Mars"]          # Mars is hidden to put Phobos under a hidden parent
CONTROL = ["Venus", "Jupiter", "Io"]   # never hidden - tolerance calibration

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b19")
os.makedirs(OUT, exist_ok=True)


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)


def load(tag):
    """-> (header, {name: {'old':..., 'new':...}})"""
    bodies = {}
    header = None
    with open(os.path.join(OUT, f"b19_{tag}.json")) as f:
        for line in f:
            d = json.loads(line)
            if d.get("type") == "header":
                header = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
    return header, bodies


def dist_km(a, b):
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3))) * AU_KM


# ---------------------------------------------------------------- drive ----
sock = socket.create_connection(("127.0.0.1", 7805), timeout=10)
# Pin the NEW path: it is the path under regression lock, and pinning also
# stops the 1 s A/B alternation (INTENT 11.53).  Both paths keep UPDATING
# either way (ssystem_factory.cpp:444-459 + updateExperimental) - pinning is
# draw-side only, so the old-path reference numbers stay valid.
send(sock, "flag experimental_path on", 1)
# `timerate rate 0` FIRST, then the epoch: the auto-played startup.sts has set
# `timerate rate 1` long before this script connects, so any gap between the
# epoch command and the freeze accumulates real seconds of simulated time
# (measured: +1.39e-05 d = 1.2 s over a 1 s gap, first run).  Freezing first
# makes assertion B exact for every dump, including the first.
send(sock, "timerate rate 0", 1)
send(sock, "date jday %.9f" % J0, 1)
send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)

steps = []          # (tag, expected_jd, {name: expected 'hidden'|'shown'})
shown = {n: "shown" for n in HIDE}


def dump(tag, jd):
    send(sock, f"body action dual_dump filename {OUT}/b19_{tag}.json", 2)
    steps.append((tag, jd, dict(shown)))


def set_hidden(name, hidden):
    send(sock, f"body name {name} hidden {'true' if hidden else 'false'}", 1)
    shown[name] = "hidden" if hidden else "shown"


def jump(jd):
    send(sock, "date jday %.9f" % jd, 2)


dump("base", J0)

intervals = []      # (label, tag_t0, tag_t1, state)
for entry in (1, 2):
    # ---- hidden leg (entry 2 starts from the state entry 1's show produced)
    for n in HIDE:
        set_hidden(n, True)
    t0, t1 = f"h{entry}_t0", f"h{entry}_t1"
    jd0 = J0 + (2 * entry - 2) * DT
    dump(t0, jd0)
    jump(jd0 + DT)
    dump(t1, jd0 + DT)
    intervals.append((f"hidden entry {entry}", t0, t1, "hidden"))
    # ---- shown leg (control: the same measurement with nothing hidden)
    for n in HIDE:
        set_hidden(n, False)
    s0, s1 = f"s{entry}_t0", f"s{entry}_t1"
    dump(s0, jd0 + DT)
    jump(jd0 + 2 * DT)
    dump(s1, jd0 + 2 * DT)
    intervals.append((f"shown  entry {entry}", s0, s1, "shown"))

sock.close()

# ------------------------------------------------------------- evaluate ----
fail = 0
report = {"dt_minutes": DT_MIN, "intervals": [], "membership": [], "time": [],
          "tolerance": {}}


def check(ok, text):
    global fail
    fail += not ok
    print(f"{'OK  ' if ok else 'FAIL'} {text}", flush=True)
    return ok


loaded = {tag: load(tag) for tag, _, _ in steps}

print("\n--- B: time control (header jd == commanded; timerate rate 0) ---")
for tag, jd, _ in steps:
    got = loaded[tag][0]["jd"]
    ok = abs(got - jd) < 1e-9
    check(ok, f"{tag:8s} jd={got:.9f} commanded={jd:.9f} d={got-jd:+.3e} d")
    report["time"].append({"tag": tag, "jd": got, "commanded": jd})

print("\n--- A: membership (relation flipped; <3 hidden, >=3 visible) ---")
for tag, _, state in steps:
    bodies = loaded[tag][1]
    for n in HIDE:
        rel = bodies[n]["new"]["relation"]
        want_hidden = state[n] == "hidden"
        ok = (rel < 3) == want_hidden
        check(ok, f"{tag:8s} {n:7s} relation={rel} expected {state[n]}")
        report["membership"].append({"tag": tag, "body": n, "relation": rel,
                                     "expected": state[n]})

# Tolerance calibration: how far apart do the two paths put a body that was
# NEVER hidden?  That is the floor of this instrument (ephemeris instance,
# light-travel, dump precision).  Measured, not assumed.
ctl = []
for tag, _, _ in steps:
    bodies = loaded[tag][1]
    for n in CONTROL:
        ctl.append(dist_km(bodies[n]["new"]["ecl"], bodies[n]["old"]["ecl"]))
ctl_max = max(ctl)
TOL_KM = max(10.0 * ctl_max, 1.0)
report["tolerance"] = {"control_bodies": CONTROL, "control_max_km": ctl_max,
                       "tol_km": TOL_KM}
print(f"\ncalibration: max old-vs-new disagreement on never-hidden "
      f"{CONTROL} over {len(steps)} dumps = {ctl_max:.4f} km "
      f"-> tolerance {TOL_KM:.4f} km")

print("\n--- C/D: advance and 'where it is now' ---")
for label, t0, t1, state in intervals:
    b0, b1 = loaded[t0][1], loaded[t1][1]
    for n in SUBJECTS:
        mv_old = dist_km(b1[n]["old"]["ecl"], b0[n]["old"]["ecl"])
        mv_new = dist_km(b1[n]["new"]["ecl"], b0[n]["new"]["ecl"])
        err = dist_km(b1[n]["new"]["ecl"], b1[n]["old"]["ecl"])
        djd = b1[n]["new"]["lastJD"] - b0[n]["new"]["lastJD"]
        tol = max(TOL_KM, 0.02 * mv_old)
        print(f"  {label} {n:7s} moved_old={mv_old:10.2f} km  "
              f"moved_new={mv_new:10.2f} km  |new-old|@t1={err:8.3f} km  "
              f"dlastJD={djd:.9f} d")
        # C: it advanced, and by the reference amount
        check(mv_new >= 0.5 * mv_old,
              f"C {label} {n}: new advance {mv_new:.2f} km >= 50% of old "
              f"{mv_old:.2f} km")
        check(abs(mv_new - mv_old) <= 0.05 * mv_old,
              f"C {label} {n}: |advance_new - advance_old| "
              f"{abs(mv_new-mv_old):.2f} km <= 5% of {mv_old:.2f} km")
        # D: and it is where it is NOW (the ratified wording)
        check(err <= tol,
              f"D {label} {n}: new-vs-old at t1 {err:.3f} km <= {tol:.3f} km "
              f"(a freeze would read ~{mv_old:.0f} km)")
        # E: corroborating witness (never the criterion)
        check(abs(djd - DT) < 1e-5,
              f"E {label} {n}: dlastJD {djd:.9f} d vs commanded {DT:.9f} d")
        report["intervals"].append(
            {"interval": label, "body": n, "state": state,
             "moved_old_km": mv_old, "moved_new_km": mv_new,
             "err_new_vs_old_at_t1_km": err, "tol_km": tol,
             "dlastJD_days": djd})

report["fail"] = fail
with open(os.path.join(OUT, "b19_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print(f"\n{'ALL PASS' if not fail else str(fail) + ' ASSERTION(S) FAILED'} "
      f"- {OUT}/b19_result.json")
sys.exit(1 if fail else 0)
