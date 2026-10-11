#!/usr/bin/env python3
# F148 - the Camera's reference decider read at its crossings, one PASS / FAIL / NOTE line per criterion.
#
#   bash f148_run.sh <label> ladder|control|switch before|after [on|off]
#   (the driver of f124_run.sh: f148_cross.py <out> <gdbpid> <fifo> --scene S --expect E --pin P)
#
# ladder: landed on Earth at the field's place, timerate 0, every scene from a re-landed Camera, in this order:
#   P12 8.0E+13 m, P11 9.0E+13, P13 8.5E+13 (position / view across the switch), P14 set home_planet Mars at 601.6 AU,
#   P25 set home_planet SolarSystem, P26 9.0E+13 over 3 s, P29 two 300-frame parks, P28 two bisections (the second with
#   Pluto shown), P23 LAST (1.5E+17, 1.5E+19: the old executor climbs out of the solar level and nothing comes down after).
# control: free mode, Earth's influence crossed by the free-move walk (the same switchToBody), position and view before / after.
# switch: on a binary WITHOUT the decider, the decider's two calls made by hand through gdb (main thread, Core::updateMove's
#   entry) in P11 / P13's scene: switchToBody(system) + the bind kept at 601.6 AU, then switchToBody(Earth) at 568.3 AU;
#   --hold switch|warp makes each crossing with that entry composed with placeAt(positionRelativeTo(dst), true).
# night: initial_night.sts lines 28-40 as commands, a dump in each line's packet and one after; Camera::relocateToBody's callers.
# --expect names the readings the run is judged against: `before` = no decider (the reference stays where the explicit
# commands put it), `after` = F144's predictions (INTENT 11.313 (h)). AoI(SolarSystem) is computed from each dump's bodies
# by ModularBody::updateReach's law, never pasted; the engine's AU is read from src/tools/sc_const.hpp.
# A line: PASS|FAIL|NOTE <P> <criterion> observed=<v> expected=<v> tol=<t>. Exit 1 on a FAIL, 3 on a scene not reached.
import argparse, ast, json, math, os, re, shutil, signal, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import b3_ladder as B

ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--scene", required=True, choices=["ladder", "control", "switch", "night"])
ap.add_argument("--expect", required=True, choices=["before", "after"])
ap.add_argument("--pin", default="on", choices=["on", "off"])
ap.add_argument("--hold", default="none", choices=["none", "switch", "warp"], help="switch scene: the entry each crossing is made with, re-placed by placeAt (none = switchToBody alone)")
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
GDBLOG = OUT + "/gdb.log"
AFTER = ARGS.expect == "after"
HOLD = ARGS.hold != "none"
ENTRY = "warpToBody" if ARGS.hold == "warp" else "switchToBody"
PLACE_TOL = 2e-4                # AU: 3 float32 ulps at 600 AU, the re-predicted P11 / P13 (the structure said 1e-6)
VIEW_TOL = 1e-5                 # rad
FAILS = []
LOGDIR = os.path.expanduser("~/.spacecrafter/log")
SC_CONST = os.path.normpath(os.path.join(HERE, "..", "..", "src", "tools", "sc_const.hpp"))
AU_KM = float(re.search(r"constexpr double AU\s*=\s*([0-9.]+)", open(SC_CONST).read()).group(1))
AU_M = AU_KM * 1000.0
ULP_600 = 2.0 ** -14            # float32 spacing at 512..1024 AU
FLOAT_FLOOR = 16 * ULP_600      # a continuous switch at 600 AU, through posePart / posePartToPose / the bind toggles
MOVETO_FLOOR = 5e-3             # AU: the radial-step model without any switch (the moveto re-sends lat / lon in degrees), 2 x before_ladder's 2.6e-3
WARP = 0.1                      # AU: the least structural jump at a body -> system switch (the centre offset is ~1 AU)

_src = HERE + "/f124_legs.py"
_node = next(n for n in ast.parse(open(_src).read()).body if isinstance(n, ast.FunctionDef) and n.name == "reuse")
exec(compile(ast.Module([_node], []), _src, "exec"), globals())
reuse(HERE + "/b21_keypath.py", ["_clean", "gdb_marker_count", "gdb_inject"])
reuse(_src, ["gdb_errors", "header_and_bodies"])


class SceneNotReached(Exception):
    pass


def line(word, p, name, observed, expected, tol):
    """Print one verdict line; a FAIL counts against the exit."""
    print("%s %s %s observed=%s expected=%s tol=%s" % (word, p, name, fmt(observed), fmt(expected), tol), flush=True)
    if word == "FAIL":
        FAILS.append("%s:%s" % (p, name))


def judge(p, name, ok, observed, expected, tol="-"):
    line("PASS" if ok else "FAIL", p, name, observed, expected, tol)


def note(p, name, observed, expected="-", tol="-"):
    line("NOTE", p, name, observed, expected, tol)


def fmt(v):
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, float):
        return "%.9g" % v
    if isinstance(v, (list, tuple)):
        return "[" + ",".join(fmt(x) for x in v) + "]"
    return str(v)


def send(cmd, pause=0.7):
    print("%.3f >> %s" % (time.time(), cmd), flush=True)
    B.send(S, cmd, pause)


def raw(cmds, pause):
    """Send several commands in ONE packet: the engine reads them before the next frame's update can run."""
    print("%.3f >> one packet: %s" % (time.time(), " | ".join(cmds)), flush=True)
    S.sendall(("\n".join(cmds) + "\n").encode())
    time.sleep(pause)


def read_dump(tag):
    for _ in range(40):
        try:
            hdr, bodies = header_and_bodies(tag)
            if hdr is not None:
                return hdr, bodies
        except (OSError, ValueError, KeyError):
            pass
        time.sleep(0.5)
    raise SceneNotReached("dump %s never readable" % tag)


def dump(tag, pause=1.5):
    send("body action dual_dump filename %s/%s.json" % (OUT, tag), pause)
    return state(tag, *read_dump(tag))


def state(tag, hdr, bodies):
    """The reading of one dump: the Camera's fields, the system derived, AoI(SolarSystem) computed."""
    c = hdr["camera"]
    st = {"tag": tag, "hdr": hdr, "bodies": bodies, "ref": c["reference"], "aimed": c.get("aimed", "absent"),
          "bound": c["boundToSurface"], "free": c["freeMode"], "root": [float(x) for x in c["rootPos"]],
          "fwd": [float(x) for x in c["absFwd"]], "refAoI": c["refAoI"], "refDist": c["refDist"], "dist": c["distance"],
          "frames": hdr["seam"]["frames"], "old_home": hdr["oldView"]["observer"]["homePlanet"],
          "alt_rep": hdr["control"]["altitude"]["reported"]}
    st["system"] = system_of(bodies, st["ref"])
    st["aoi_ss"] = aoi(bodies, "SolarSystem")
    st["dist_ss"] = new(bodies, "SolarSystem")["dist"]
    print("-- dump %-14s ref=%s aimed=%s system=%s bound=%s free=%s |root|=%.9g AoI(SS)=%.6g dist(SS)=%.9g old_home=%s frames=%s"
          % (tag, st["ref"], st["aimed"], st["system"], st["bound"], st["free"], norm(st["root"]), st["aoi_ss"],
             st["dist_ss"], st["old_home"], st["frames"]), flush=True)
    return st


def new(bodies, name):
    return bodies[name]["new"]


def system_of(bodies, name):
    """Return the first node on name's parent chain built as a ModularSystem (bodyType SYSTEM 2 or GALAXY 3)."""
    while name:
        n = new(bodies, name)
        if n["bodyType"] in (2, 3):
            return name
        name = n["parent"]
    return ""


def aoi(bodies, name):
    """Transcribe ModularBody::updateReach (visible children only: relation >= GROUNDED = 3)."""
    kids = {}
    for k, b in bodies.items():
        n = b.get("new")
        if n is not None and n["relation"] >= 3:
            kids.setdefault(n["parent"], []).append(k)
    memo = {}

    def sub(k):
        if k not in memo:
            n = new(bodies, k)
            s = n["boundingRadius"]
            for c in kids.get(k, []):
                s = max(s, norm(new(bodies, c)["ecl"]) + sub(c))
            memo[k] = s * 1.1
        return memo[k]

    n = new(bodies, name)
    a = max(n["boundingRadius"] * 128 / (n["scaling"] * n["inheritedScaling"]), sub(name) * 16)
    cap = norm(n["ecl"]) * 0.6
    return min(a, cap) if cap > 0 else a


def norm(v):
    return math.sqrt(sum(x * x for x in v))


def sub3(a, b):
    return [x - y for x, y in zip(a, b)]


def add3(a, b):
    return [x + y for x, y in zip(a, b)]


def scale3(a, k):
    return [x * k for x in a]


def angle(u, v):
    d = sum(x * y for x, y in zip(u, v)) / (norm(u) * norm(v))
    return math.acos(max(-1.0, min(1.0, d)))


def gdb_system(mark):
    """Print the Camera's system member through gdb (the engine stopped once), return its name."""
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(['printf "F148BEGIN %s\\n"' % mark, "print Camera::instance->system->englishName._M_dataplus._M_p"],
                    "F148END_" + mark)
    time.sleep(1)
    with open(GDBLOG, "rb") as f:   # since is a byte offset: the log holds multi-byte lines
        f.seek(since)
        txt = f.read().decode("latin-1")
    m = re.search(r'F148BEGIN %s.*?"([^"]*)"' % mark, txt, re.S)
    if not ok or gdb_errors(since) or not m:
        return "gdb-error"
    return m.group(1)


def instrument_checks(st):
    """Fail loud when the reading's own premises do not hold: the AoI transcription, the Sun at the root's origin."""
    p, ref, bodies = "INSTR", st["ref"], st["bodies"]
    if ref in ("Earth", "SolarSystem"):
        mine = aoi(bodies, ref)
        judge(p, "aoi_transcription_%s_%s" % (ref, st["tag"]), abs(mine - st["refAoI"]) <= 1e-5 * st["refAoI"],
              mine, st["refAoI"], "1e-5 rel")
    if st["dist_ss"] > 0:
        judge(p, "sun_at_root_origin_%s" % st["tag"], abs(norm(st["root"]) - st["dist_ss"]) <= 1e-5 * st["dist_ss"],
              norm(st["root"]), st["dist_ss"], "1e-5 rel")


def expect_ref(p, st, after_ref, before_ref, after_aimed=None, before_aimed="absent"):
    ref, aimed = (after_ref, after_aimed) if AFTER else (before_ref, before_aimed)
    judge(p, "reference_" + st["tag"], st["ref"] == ref, st["ref"], ref)
    if aimed is not None:
        judge(p, "aimed_" + st["tag"], st["aimed"] == aimed, st["aimed"], aimed)


def reland(tag):
    """Re-land on Earth at the field's place, from inside the system's influence: no crossing on the way."""
    send("moveto altitude 8.5E+13 duration 0", 1.0)
    send("set home_planet Earth", 1.5)
    send("moveto lat default lon default alt default", 1.0)
    st = dump(tag)
    if st["ref"] != "Earth" or abs(st["alt_rep"] - 75.0) > 5:
        raise SceneNotReached("re-landing %s: ref %s, altitude %s m" % (tag, st["ref"], st["alt_rep"]))
    return st


def flips(st_a, st_b):
    """Return the frames where the ring's flag 32 (the Camera's reference == the old home planet) changes, a..b."""
    seam = st_b["hdr"]["seam"]
    if not seam["armed"] or seam["dropped"] and seam["steps"][0]["frame"] > st_a["frames"]:
        raise SceneNotReached("the seam ring does not cover frames %s..%s (armed=%s)" % (st_a["frames"], st_b["frames"], seam["armed"]))
    steps = [s for s in seam["steps"] if st_a["frames"] <= s["frame"] <= st_b["frames"]]
    out, prev = [], None
    for s in steps:
        f = bool(s["flags"] & 32)
        if prev is not None and f != prev:
            out.append((s["frame"], f))
        prev = f
    return out, steps


def m2au(m):
    return m / AU_M


def ladder():
    send("timerate rate 0", 1)
    send("deselect", 1)
    l0 = dump("L0_landed")
    instrument_checks(l0)
    judge("L0", "landed_reference", l0["ref"] == "Earth", l0["ref"], "Earth")
    judge("L0", "landed_bound", l0["bound"] is True, l0["bound"], True)
    judge("P24", "aimed_field", (l0["aimed"] != "absent") == AFTER, l0["aimed"], "Earth" if AFTER else "absent")
    note("P28", "aoi_solar_system_au_landed", l0["aoi_ss"])

    # P12 / P11 / P13: the leave, the stay inside, the return - position and view across each switch
    send("moveto altitude 8.0E+13 duration 0", 1.0)
    p12 = dump("P12_534au")
    instrument_checks(p12)
    expect_ref("P12", p12, "Earth", "Earth", "Earth")
    note("P12", "distance_to_ss_au", p12["dist_ss"], "< %.6g" % p12["aoi_ss"])
    note("P12", "system_derived", p12["system"], "SolarSystem")
    note("P12", "system_gdb", gdb_system("p12"), "SolarSystem")

    u = sub3(p12["root"], l0["root"])
    u = scale3(u, 1.0 / norm(u))
    send("moveto altitude 9.0E+13 duration 0", 1.0)
    p11 = dump("P11_601au")
    instrument_checks(p11)
    expect_ref("P11", p11, "SolarSystem", "Earth", "Earth")
    judge("P11", "free_mode", p11["free"] is False, p11["free"], False)
    judge("P11", "bound_unchanged", p11["bound"] == p12["bound"], p11["bound"], p12["bound"])
    note("P11", "distance_to_ss_au_after", p11["dist_ss"], "> %.6g" % p12["aoi_ss"])
    note("P11", "system_derived", p11["system"], "SolarSystem")
    note("P11", "system_gdb", gdb_system("p11"), "SolarSystem")
    pred = add3(p12["root"], scale3(u, m2au(9.0e13 - 8.0e13)))
    d11 = norm(sub3(p11["root"], pred))
    judge("P11", "rootpos_radial_step_au", d11 <= MOVETO_FLOOR, d11, 0.0, "%g (the no-switch floor; structure: 1e-6)" % MOVETO_FLOOR)
    note("P11", "rootpos_class", "moveto-floor" if d11 <= MOVETO_FLOOR else ("WARP" if d11 >= WARP else "between"),
         "moveto-floor <= %.3g, warp >= %.3g" % (MOVETO_FLOOR, WARP))
    t11 = angle(p11["fwd"], p12["fwd"])
    judge("P11", "absfwd_turn_rad", t11 <= 1e-5, t11, 0.0, "1e-5")

    send("moveto altitude 8.5E+13 duration 0", 1.0)
    p13 = dump("P13_568au")
    instrument_checks(p13)
    expect_ref("P13", p13, "Earth", "Earth", "Earth")
    judge("P13", "bound_unchanged", p13["bound"] == p12["bound"], p13["bound"], p12["bound"])
    if AFTER:   # the moveto ran on the node: 568.3 AU from its centre, along the Camera's direction from it
        pred = scale3(p11["root"], (m2au(8.5e13) + new(p11["bodies"], "SolarSystem")["scaledDatumRadius"]) / norm(p11["root"]))
    else:       # the moveto ran on Earth: the radial step back from P11
        pred = add3(p11["root"], scale3(u, m2au(8.5e13 - 9.0e13)))
    d13 = norm(sub3(p13["root"], pred))
    judge("P13", "rootpos_radial_step_au", d13 <= MOVETO_FLOOR, d13, 0.0, "%g (the no-switch floor; structure: 1e-6)" % MOVETO_FLOOR)
    note("P13", "rootpos_class", "moveto-floor" if d13 <= MOVETO_FLOOR else ("WARP" if d13 >= WARP else "between"),
         "moveto-floor <= %.3g, warp >= %.3g" % (MOVETO_FLOOR, WARP))
    t13 = angle(p13["fwd"], p11["fwd"])
    judge("P13", "absfwd_turn_rad", t13 <= 1e-5, t13, 0.0, "1e-5")

    # P11' / P13': the BOUNDARY moved across a parked Camera (Pluto shown: AoI(SS) ~690 AU), so the decider's own
    # leave and return run with no command between the two dumps that judge them
    send("body name Pluto hidden false", 2)
    send("moveto altitude 9.0E+13 duration 0", 1.0)
    b0 = dump("B0_parked_inside")
    expect_ref("P11'", b0, "Earth", "Earth", "Earth")
    note("P11'", "aoi_ss_pluto_shown_au", b0["aoi_ss"], "> %.6g" % b0["dist_ss"])
    send("body name Pluto hidden true", 2)
    b1 = dump("B1_boundary_in")
    instrument_checks(b1)
    expect_ref("P11'", b1, "SolarSystem", "Earth", "Earth")
    judge("P11'", "bound_unchanged", b1["bound"] == b0["bound"], b1["bound"], b0["bound"])
    d = norm(sub3(b1["root"], b0["root"]))
    judge("P11'", "place_continuous_au", d <= PLACE_TOL, d, 0.0, "%g (structure: 1e-6)" % PLACE_TOL)
    t = angle(b1["fwd"], b0["fwd"])
    judge("P11'", "view_turn_rad", t <= VIEW_TOL, t, 0.0, VIEW_TOL)
    send("body name Pluto hidden false", 2)
    b2 = dump("B2_boundary_out")
    instrument_checks(b2)
    expect_ref("P13'", b2, "Earth", "Earth", "Earth")
    d = norm(sub3(b2["root"], b1["root"]))
    judge("P13'", "place_continuous_au", d <= PLACE_TOL, d, 0.0, "%g (structure: 1e-6)" % PLACE_TOL)
    t = angle(b2["fwd"], b1["fwd"])
    judge("P13'", "view_turn_rad", t <= VIEW_TOL, t, 0.0, VIEW_TOL)
    fl, _ = flips(b0, b2)
    judge("P11'", "ring_reference_changes", len(fl) == (2 if AFTER else 0), len(fl), 2 if AFTER else 0)
    send("body name Pluto hidden true", 2)

    # P14: an explicit re-aim on the system node
    send("moveto altitude 9.0E+13 duration 0", 1.0)
    p14o = dump("P14_0_out")
    raw(["set home_planet Mars", "body action dual_dump filename %s/P14_1_same_packet.json" % OUT], 0.5)
    raw(["body action dual_dump filename %s/P14_2_two_frames.json" % OUT], 1.5)
    p14a = state("P14_1_same_packet", *read_dump("P14_1_same_packet"))
    p14b = state("P14_2_two_frames", *read_dump("P14_2_two_frames"))
    judge("INSTR", "p14_dumps_two_frames_apart", p14b["frames"] >= p14a["frames"] + 2, p14b["frames"] - p14a["frames"], ">= 2")
    note("P14", "reference_before", p14o["ref"], "SolarSystem" if AFTER else "Earth")
    note("P14", "reference_same_packet", p14a["ref"], "Mars or SolarSystem" if AFTER else "Mars")
    judge("P14", "aimed_within_two_frames", p14b["aimed"] == ("Mars" if AFTER else "absent"), p14b["aimed"],
          "Mars" if AFTER else "absent")
    expect_ref("P14", p14b, "SolarSystem", "Mars")
    fl, _ = flips(p14o, p14b)
    note("P14", "ring_flag32_changes", fl)
    send("moveto altitude 8.5E+13 duration 0", 1.0)
    p14c = dump("P14_3_return")
    expect_ref("P14", p14c, "Mars", "Mars", "Mars")

    # P25: a name the old registry refuses but the tree holds
    reland("P25_0_landed")
    send("set home_planet SolarSystem", 1.5)
    p25 = dump("P25_1_set")
    expect_ref("P25", p25, "SolarSystem", "Earth", "SolarSystem")
    time.sleep(1)
    p25b = dump("P25_2_later")
    expect_ref("P25", p25b, "SolarSystem", "Earth", "SolarSystem")
    judge("P25", "old_home_unchanged", p25b["old_home"] == "Earth", p25b["old_home"], "Earth")

    # P26: a timed crossing
    p26a = reland("P26_0_landed")
    send("moveto altitude 9.0E+13 duration 3", 1.5)
    p26m = dump("P26_1_mid", 0.5)
    time.sleep(3)
    p26b = dump("P26_2_after")
    expect_ref("P26", p26b, "SolarSystem", "Earth", "Earth")
    fl, steps = flips(p26a, p26b)
    judge("P26", "ring_reference_changes", len(fl) == (1 if AFTER else 0), len(fl), 1 if AFTER else 0)
    pd = [s["posDelta"] for s in steps]
    jumps = [abs(b - a) for a, b in zip(pd, pd[1:])]
    if jumps:
        k = max(range(len(jumps)), key=lambda i: jumps[i])
        typical = sorted(jumps)[len(jumps) // 2]
        note("P26", "posdelta_largest_step_au", jumps[k], "frame %s; median step %.6g" % (steps[k + 1]["frame"], typical))
    for frame, _ in fl:     # the step INTO the switch frame against the five steps before it
        i = next(j for j, s in enumerate(steps) if s["frame"] == frame) - 1
        before = max(jumps[max(0, i - 5):i] or [0.0])
        judge("P26", "posdelta_step_at_switch_au", jumps[i] <= 3 * before + PLACE_TOL, jumps[i], "<= 3 x %.6g + %g" % (before, PLACE_TOL))
    note("P26", "switch_frames", fl)
    note("P26", "camera_distance_after_au", p26b["dist"])

    # P29: parks just outside, just inside
    for tag, alt, after_n in (("P29_out", "8.9E+13", 1), ("P29_in", "8.7E+13", 0)):
        a = reland(tag + "_0_landed")
        send("moveto altitude %s duration 0" % alt, 0.7)
        time.sleep(6)
        b = dump(tag + "_1_parked")
        while b["frames"] - a["frames"] < 300:
            time.sleep(2)
            b = dump(tag + "_1_parked")
        fl, _ = flips(a, b)
        want = after_n if AFTER else 0
        judge("P29", "reference_changes_" + tag, len(fl) == want, len(fl), want, "frames %d" % (b["frames"] - a["frames"]))

    # P28: the boundary by bisection, then with Pluto shown
    for tag, lo, hi in (("P28_hidden", 8.5e13, 9.0e13), ("P28_pluto", 1.0e14, 1.1e14)):
        if tag == "P28_pluto":
            send("body name Pluto hidden false", 2)
        probes = []

        def probe(alt):
            reland("%s_l%d" % (tag, len(probes)))
            send("moveto altitude %.6E duration 0" % alt, 1.0)
            st = dump("%s_p%d" % (tag, len(probes)))
            probes.append((alt, st))
            return st["ref"] != "Earth"

        out_lo, out_hi = probe(lo), probe(hi)
        aoi_now = probes[-1][1]["aoi_ss"]
        note("P28", tag + "_aoi_ss_au", aoi_now)
        if out_lo or not out_hi:
            judge("P28", tag + "_bracket", not AFTER, "lo out=%s hi out=%s" % (out_lo, out_hi),
                  "lo in, hi out" if AFTER else "no leave (no decider)")
            continue
        for _ in range(6):
            mid = (lo + hi) / 2
            if probe(mid):
                hi = mid
            else:
                lo = mid
        inside = [st for alt, st in probes if alt == lo][-1]
        d_lo = inside["dist_ss"]
        note("P28", tag + "_bracket_m", [lo, hi], "six halvings of the start bracket")
        judge("P28", tag + "_aoi_between_lo_and_hi", d_lo <= aoi_now <= d_lo + m2au(hi - lo), aoi_now,
              "[%.6g, %.6g]" % (d_lo, d_lo + m2au(hi - lo)))
        note("P28", tag + "_leave_at_m", (lo + hi) / 2, "+-%.3g m" % ((hi - lo) / 2))
    send("body name Pluto hidden true", 2)

    # P23 LAST: above the first system, explicit only
    reland("P23_0_landed")
    send("moveto altitude 9.0E+13 duration 0", 1.0)
    p23a = dump("P23_1_601au")
    expect_ref("P23", p23a, "SolarSystem", "Earth", "Earth")
    send("moveto altitude 1.5E+17 duration 0", 2.0)
    p23b = dump("P23_2_1e6au")
    expect_ref("P23", p23b, "SolarSystem", "Earth", "Earth")
    send("moveto altitude 1.5E+19 duration 0", 2.0)
    p23c = dump("P23_3_1e8au")
    expect_ref("P23", p23c, "SolarSystem", "Earth", "Earth")
    note("P23", "milkyway_distance_maintained", new(p23c["bodies"], "MilkyWay")["dist"], 0)


def gdb_calls(calls, mark):
    """Run calls in the engine's main thread at the entry of one Core::updateMove (f124_legs.inject's safe point)."""
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(["break Core::updateMove", "continue", "delete breakpoints"] + calls, "F148CALL_" + mark)
    bad = gdb_errors(since)
    judge("INSTR", "gdb_calls_%s_ran_clean" % mark, ok and not bad, "handshake=%s errors=%s" % (ok, bad[:2]), "clean")
    time.sleep(1.0)
    return ok and not bad


def switch():
    """Make the decider's two calls by hand on a binary without the decider, in P11 / P13's scene."""
    send("timerate rate 0", 1)
    send("deselect", 1)
    l0 = dump("S0_landed")
    instrument_checks(l0)
    send("moveto altitude 9.0E+13 duration 0", 1.0)
    pre = dump("S11_0_pre")
    instrument_checks(pre)
    if pre["ref"] != "Earth":
        raise SceneNotReached("the reference left Earth by itself at 601.6 AU: this binary has a decider")
    note("P11", "distance_to_ss_au_pre", pre["dist_ss"], "> %.6g" % pre["aoi_ss"])
    hold = ["set $p = (Vec3f *) malloc(sizeof(Vec3f))",
            "set *$p = Camera::instance->positionRelativeTo((ModularBody *) Camera::instance->system)"] if HOLD else []
    gdb_calls(["set $earth = Camera::instance->reference.ptr"] + hold +
              ["call Camera::instance->%s((ModularBody *) Camera::instance->system)" % ENTRY,
               "call Camera::instance->setBoundToSurface(true)"] +
              (["call Camera::instance->placeAt(*$p, true)"] if HOLD else []), "leave")
    post = dump("S11_1_switched")
    instrument_checks(post)
    judge("P11", "reference_switched_by_hand", post["ref"] == "SolarSystem", post["ref"], "SolarSystem")
    note("P11", "system_gdb_after_switch", gdb_system("s11"), "SolarSystem (systemOf(SolarSystem) = itself)")
    judge("P11", "bound_kept", post["bound"] is True, post["bound"], True)
    d = norm(sub3(post["root"], pre["root"]))
    judge("P11", "rootpos_continuous_au", d <= PLACE_TOL, d, 0.0, "%g (structure: 1e-6)" % PLACE_TOL)
    note("P11", "rootpos_class", "float-floor" if d <= FLOAT_FLOOR else ("WARP" if d >= WARP else "between"),
         "float-floor <= %.3g, warp >= %.3g" % (FLOAT_FLOOR, WARP))
    note("P11", "distance_to_ss_au_post", post["dist_ss"], "continuous: %.9g" % pre["dist_ss"])
    note("P11", "camera_numbers_pre", [pre["hdr"]["camera"][k] for k in ("longitude", "latitude", "distance")])
    note("P11", "camera_numbers_post", [post["hdr"]["camera"][k] for k in ("longitude", "latitude", "distance")])
    t = angle(post["fwd"], pre["fwd"])
    judge("P11", "absfwd_turn_rad", t <= VIEW_TOL, t, 0.0, VIEW_TOL)
    send("moveto altitude 8.5E+13 duration 0", 1.0)
    pre13 = dump("S13_0_pre")
    instrument_checks(pre13)
    gdb_calls((["set *$p = Camera::instance->positionRelativeTo($earth)"] if HOLD else []) +
              ["call Camera::instance->%s($earth)" % ENTRY] +
              (["call Camera::instance->placeAt(*$p, true)"] if HOLD else []), "return")
    post13 = dump("S13_1_switched")
    instrument_checks(post13)
    judge("P13", "reference_switched_by_hand", post13["ref"] == "Earth", post13["ref"], "Earth")
    d = norm(sub3(post13["root"], pre13["root"]))
    judge("P13", "rootpos_continuous_au", d <= PLACE_TOL, d, 0.0, "%g (structure: 1e-6)" % PLACE_TOL)
    note("P13", "rootpos_class", "float-floor" if d <= FLOAT_FLOOR else ("WARP" if d >= WARP else "between"),
         "float-floor <= %.3g, warp >= %.3g" % (FLOAT_FLOOR, WARP))
    note("P13", "distance_to_ss_au", [pre13["dist_ss"], post13["dist_ss"]], "continuous: equal")
    t = angle(post13["fwd"], pre13["fwd"])
    judge("P13", "absfwd_turn_rad", t <= VIEW_TOL, t, 0.0, VIEW_TOL)


NIGHT_LINES = [(28, "moveto alt 0.9E+10 duration 0"), (30, "moveto alt 1.1E+16 duration 0"), (32, "set home_planet Sun"),
               (34, "moveto alt 0.9E+9 duration 0"), (36, "moveto alt 0.9E+10 duration 0"),
               (38, "mode jump in_solarsystem body Sun altitude 1.0E+5"), (39, "set home_planet Earth"),
               (40, "moveto lat default lon default alt default")]


def night():
    """Play initial_night.sts lines 28-40 as commands: a dump in each line's packet (before any frame) and one after it;
    every Camera::relocateToBody call's backtrace printed in gdb.log."""
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(["break Camera::relocateToBody", "commands", "silent", 'printf "F148RELOCATE\\n"', "bt 6", "continue", "end"],
                    "F148BREAK")
    judge("INSTR", "relocate_breakpoint_set", ok and not gdb_errors(since), ok, True)
    send("timerate rate 0", 1)
    send("deselect", 1)
    dump("N00_pre")
    for n, cmd in NIGHT_LINES:
        raw([cmd, "body action dual_dump filename %s/N%02d_a_same_packet.json" % (OUT, n)], 1.5)
        a = state("N%02d_a_same_packet" % n, *read_dump("N%02d_a_same_packet" % n))
        b = dump("N%02d_b_after" % n)
        for st in (a, b):
            note("NIGHT", "line%02d_%s" % (n, st["tag"][4:]), "ref=%s aimed=%s bound=%s dist=%.9g old_home=%s alt_old=%.6g" % (
                st["ref"], st["aimed"], st["bound"], st["dist"], st["old_home"], st["hdr"]["control"]["altitude"]["old"]),
                 "frame %s" % st["frames"])
    txt = open(GDBLOG, "rb").read()[since:].decode("latin-1")
    calls = []
    for blk in txt.split("F148RELOCATE")[1:]:
        frames = re.findall(r"#\d+\s+(?:0x[0-9a-f]+ in )?([\w:~<>]+)\s*\(", blk)
        calls.append(" <- ".join(frames[:6]))
    for i, c in enumerate(calls):
        note("NIGHT", "relocate_call_%d" % i, c)


def control():
    send("timerate rate 0", 1)
    send("deselect", 1)
    c0 = dump("C0_landed")
    instrument_checks(c0)
    send("camera action free_mode state on", 2)
    send("moveto altitude 6.0E+9 duration 0", 1.0)
    c1 = dump("C1_inside_a")
    send("moveto altitude 6.5E+9 duration 0", 1.0)
    c2 = dump("C2_inside_b")
    instrument_checks(c2)
    judge("CTRL", "free_mode_on", c2["free"] is True, c2["free"], True)
    judge("CTRL", "reference_inside", c2["ref"] == "Earth", c2["ref"], "Earth")
    note("CTRL", "earth_aoi_au", c2["refAoI"])
    u = sub3(c2["root"], c1["root"])
    u = scale3(u, 1.0 / norm(u))
    send("moveto altitude 7.5E+9 duration 0", 1.0)
    c3 = dump("C3_outside")
    judge("CTRL", "free_walk_left_earth", c3["ref"] == "Sun", c3["ref"], "Sun")
    pred = add3(c2["root"], scale3(u, m2au(7.5e9 - 6.5e9)))
    d = norm(sub3(c3["root"], pred))
    note("CTRL", "switchtobody_rootpos_jump_au", d, "continuous: < 1e-6 (float floor at 1 AU ~1e-7)")
    note("CTRL", "camera_distance_to_sun_after_au", norm(c3["root"]), "continuous: ~1; warp: |position| = %.6g" % norm(c3["root"]))
    note("CTRL", "switchtobody_absfwd_turn_rad", angle(c3["fwd"], c2["fwd"]), "< 1e-5")


def copy_logs():
    for ch in ("spacecrafter", "script"):
        p = subprocess.run([sys.executable, HERE + "/logread.py", "--live", LOGDIR, ch], capture_output=True, text=True).stdout.strip()
        if p and os.path.exists(p):
            shutil.copyfile(p, "%s/log_%s.txt" % (OUT, ch))


S = B.wait_port(30)
LOST = None
try:
    send("flag experimental_path " + ARGS.pin, 2)
    {"ladder": ladder, "control": control, "switch": switch, "night": night}[ARGS.scene]()
except SceneNotReached as e:
    LOST = str(e)
finally:
    copy_logs()
    S.close()
log = OUT + "/log_spacecrafter.txt"
if os.path.exists(log):
    warn = [l.strip() for l in open(log, errors="replace") if "re-reference the camera on" in l]
    judge("P25", "no_sync_warning", not warn, len(warn), 0)
if LOST:
    print("\n=== f148 %s %s STOPPED: scene '%s' not reached ===" % (ARGS.scene, ARGS.expect, LOST), flush=True)
    sys.exit(3)
print("\n=== f148 %s expect %s pin %s: %s ===" % (ARGS.scene, ARGS.expect, ARGS.pin,
      "ALL PASS" if not FAILS else "FAILS: " + ",".join(FAILS)), flush=True)
sys.exit(1 if FAILS else 0)
