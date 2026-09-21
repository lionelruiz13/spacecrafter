#!/usr/bin/env python3
# F128 - the identity instrument of row B47: one LAYER alone, per LEVEL, per PINNED path.
#
#   F124_OUT_ROOT=artifacts/f128 f124_run.sh <label> f128_legs.py --leg SOL|GAL|UNI|TILT --pin on|off
#        [--layer milky_way] [--perturb none|tilt-system|intensity|withhold-poke] [--expect system|owner]
#   (f128_leg.sh is the one command per leg: it runs the launches and the comparisons.)
#
# SOL / GAL / UNI write ONE capture of the layer at that level under that pin: <tag>_on.png,
# <tag>_off.png (the layer's own switch, nothing else touched: ON - OFF is the layer alone, the
# b5_oort.py pattern) and <tag>_state.json (what the dump says was DRAWN: control.drawnPath,
# the Camera's reference, the executor mode seen in the engine's output). Pairs of captures are
# compared by f128_compare.py - across the pins, or against a stored capture of the same pin.
# TILT pokes a non-zero obliquity into a node of the body tree under gdb and answers WHOSE
# tilt the layer follows: --expect system (the current system's) or --expect owner (the
# galaxy's, the ruling). --perturb withhold-poke pokes 0: the leg must then FAIL.
# Reused: wait_port / send / shot = b3_ladder.py; check / _clean / gdb_marker_count /
# gdb_inject = b21_keypath.py and reuse / gdb_errors / header_and_bodies = f124_legs.py, taken
# by AST (those two files run their scene at import); the pixel metrics = f128_compare.py.
import argparse, ast, json, os, signal, sys, time
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import b3_ladder as B
import f128_compare as C

LAYERS = {   # layer -> the flag that switches it; every OTHER one is switched off for the capture
    "milky_way": "milky_way", "zodiacal_light": "zodiacal_light", "stars": "stars", "nebulae": "nebulae",
    "oort": "oort", "tully": "tully", "star_lines": "star_lines", "constellation_drawing": "constellation_drawing",
    "equatorial_grid": "equatorial_grid", "cardinal_points": "cardinal_points", "atmosphere": "atmosphere",
    "landscape": "landscape"}
ALSO_OFF = ["fog", "star_twinkle", "star_names", "planet_names", "nebula_names", "nebula_hints", "bright_nebulae",
            "constellation_art", "constellation_names", "constellation_boundaries", "planet_orbits", "object_trails",
            "body_trace", "dso_pictograms", "show_tui_datetime", "show_tui_short_obj_info", "show_latlon",
            "azimuthal_grid", "ecliptic_grid", "galactic_grid", "equator_line", "ecliptic_line", "meridian_line"]
ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--leg", required=True, choices=["SOL", "GAL", "UNI", "TILT"])
ap.add_argument("--pin", required=True, choices=["on", "off"])
ap.add_argument("--layer", default="milky_way", choices=sorted(LAYERS))
ap.add_argument("--perturb", default="none", choices=["none", "tilt-system", "intensity", "withhold-poke"])
ap.add_argument("--expect", default="system", choices=["system", "owner"])
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
OUTP = Path(OUT)
GDBLOG = OUT + "/gdb.log"
FAILS = []
JD = 2461233.5              # 2026-07-12 00:00 UT: night at the default site, the band overhead
FADE = 5.0                  # s after a switch: config milky_way_fader_duration = 2
TILT_RAD = 0.5              # poked obliquity, 28.6 deg
WITNESS_MIN_PX = 10         # the layer-off frame must hold bodies, or "nothing else moved" is vacuous
INTENSITY = {"none": None, "intensity": 0.25}   # `set milky_way_intensity` of the perturbed pin (config: 0.5)
GALAXY_M = 12000000000000000                    # > 1e16 m: the solar executor goes up (b5_drawhalf.py:196)
UNIVERSE_M = 200459146738000                    # > 1e14 in galaxy units: up again (b5_drawhalf.py:198)
GAL_LON = 150.0             # deg, written after the climb: above a system the old observer's longitude otherwise carries the app's START clock
OLD_LON = "CoreLink::instance->core._M_ptr->observatory._M_ptr->longitude"   # by gdb: `moveto lon` re-sends the path-dependent altitude readout
SYSTEM, GALAXY = "Camera::instance->system", "Camera::instance->system->parent"
RESULTS = {"leg": ARGS.leg, "pin": ARGS.pin, "layer": ARGS.layer, "perturb": ARGS.perturb, "captures": {}, "notes": []}

_src = HERE + "/f124_legs.py"
_node = next(n for n in ast.parse(open(_src).read()).body if isinstance(n, ast.FunctionDef) and n.name == "reuse")
exec(compile(ast.Module([_node], []), _src, "exec"), globals())
reuse(HERE + "/b21_keypath.py", ["check", "_clean", "gdb_marker_count", "gdb_inject"])
reuse(_src, ["gdb_errors", "header_and_bodies"])


def note(txt):
    print("NOTE " + txt, flush=True)
    RESULTS["notes"].append(txt)


def send(cmd, pause=0.7):
    print(">> " + cmd, flush=True)
    B.send(S, cmd, pause)


class SceneNotReached(Exception):
    pass


def engine_said(text):
    with open(GDBLOG, "rb") as f:
        return f.read().count(text.encode())


def capture(tag):
    """The layer on, then off, and what the dump says was drawn in between."""
    flag = LAYERS[ARGS.layer]
    send("flag %s on" % flag, FADE)
    B.shot(S, OUTP, tag + "_on")
    send("body action dual_dump filename %s/%s.json" % (OUT, tag), 2.0)
    send("flag %s off" % flag, FADE)
    B.shot(S, OUTP, tag + "_off")
    hdr = header_and_bodies(tag)[0]
    cam, ctl = hdr["camera"], hdr["control"]
    st = {"tag": tag, "drawnPath": ctl["drawnPath"], "reference": cam["reference"], "freeMode": cam["freeMode"],
          "old_longitude": hdr["oldView"]["observer"]["longitude"], "jd": hdr["jd"],
          "halfFov": cam["halfFov"], "fov_old": hdr["oldView"]["projector"]["fov"], "layer": ARGS.layer,
          "pin": ARGS.pin, "leg": ARGS.leg, "perturb": ARGS.perturb,
          "engine_said": {k: engine_said(k) for k in ("->InGalaxy", "->InUniverse", "InGalaxy->", "InUniverse->")},
          "on_md5": C.md5("%s/%s_on.png" % (OUT, tag)), "off_md5": C.md5("%s/%s_off.png" % (OUT, tag))}
    json.dump(st, open("%s/%s_state.json" % (OUT, tag), "w"), indent=1)
    RESULTS["captures"][tag] = st
    print("-- %-10s drawn=%s ref=%s free=%s fov old %.3f new %.3f deg said=%s" % (
        tag, st["drawnPath"], st["reference"], st["freeMode"], st["fov_old"],
        2 * 57.29577951308232 * st["halfFov"], st["engine_said"]), flush=True)
    want = "new" if ARGS.pin == "on" else "old"
    check(tag + "_drawn_path_is_the_pinned_one", st["drawnPath"] == want, "pin %s -> control.drawnPath = %s" % (ARGS.pin, st["drawnPath"]))
    if st["drawnPath"] != want:
        raise SceneNotReached(tag + ": the pin was swallowed")
    return st


def scene():
    send("flag experimental_path " + ARGS.pin, 2)
    send("date jday %s" % JD, 1)
    send("timerate rate 0", 1)
    send("meteors zhr 0", 1)
    send("deselect", 1)
    for f in sorted(set(LAYERS.values()) - {LAYERS[ARGS.layer]}) + ALSO_OFF:
        send("flag %s off" % f, 0.3)
    if ARGS.layer != "milky_way":
        return
    i = INTENSITY.get(ARGS.perturb)
    if i is not None:
        send("set milky_way_intensity %s" % i, 1)
        note("PERTURBED on this pin: set milky_way_intensity %s" % i)


def go_up(metres, said, level, times):
    """Leave the level below by altitude, the way b5_drawhalf.py:143-146,203,213-214 does; stop if the executor did not follow."""
    before = engine_said(said)
    send("select planet Sun", 1)
    send("flag track_object on", 4)
    send("camera action free_mode state on", 1)
    for _ in range(times):      # galaxy: once (a second one would go up again); universe: flip, then place
        send("moveto altitude %d duration 0" % metres, 4)
    send("flag track_object off", 2)
    send("deselect", 1)
    ok = engine_said(said) > before
    check("%s_level_reached" % level, ok, "the engine printed '%s' %d time(s) after `moveto altitude %d`" % (said, engine_said(said) - before, metres))
    if not ok:
        raise SceneNotReached(level)


def poke(node, value, mark):
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(['printf "F128NODE %s=%%s\\n", %s->englishName._M_dataplus._M_p' % (mark, node),
                     "set var %s->re.obliquity = %s" % (node, value),
                     'printf "F128RE %s=%%f\\n", %s->re.obliquity' % (mark, node)], "F128POKE_" + mark)
    bad = gdb_errors(since)
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        said = [l[l.index("F128"):] for l in f.read().decode("latin-1").splitlines() if "F128" in l]    # after the "(gdb) " prompts
    check("gdb_poke_%s_ran_clean" % mark, ok and not bad, "`set var %s->re.obliquity = %s`; gdb said %s %s" % (node, value, said, bad[:3]))
    time.sleep(1.5)
    return said


def leg_level():
    scene()
    if ARGS.leg in ("GAL", "UNI"):
        go_up(GALAXY_M, "->InGalaxy", "galaxy", 1)
    if ARGS.leg == "UNI":
        go_up(UNIVERSE_M, "->InUniverse", "universe", 2)
    if ARGS.leg in ("GAL", "UNI"):
        since = os.path.getsize(GDBLOG)
        ok = gdb_inject(["set var %s = %s" % (OLD_LON, GAL_LON), 'printf "F128LON=%%f\\n", %s' % OLD_LON], "F128LONPIN")
        check("gdb_pins_the_old_observer_longitude", ok and not gdb_errors(since), "`set var %s = %s`" % (OLD_LON, GAL_LON))
        time.sleep(2)
    if ARGS.perturb == "tilt-system":
        said = poke(SYSTEM, TILT_RAD, "sys")
        note("PERTURBED on this pin: %s" % said)
    st = capture("base")
    if ARGS.leg == "SOL":
        check("SOL_reference_is_a_body", st["reference"] == "Earth" and st["freeMode"] is False,
              "reference %s free=%s (the tilt perturbation reads as a sky turn only from a body)" % (st["reference"], st["freeMode"]))
    else:
        check("%s_reference_is_the_galaxy" % ARGS.leg, st["reference"] == "MilkyWay", "Camera reference %s" % st["reference"])
        check("%s_old_observer_longitude_is_the_scene_s" % ARGS.leg, abs(st["old_longitude"] - GAL_LON) < 1e-6,
              "old observer longitude %.6f deg, scene %.1f (unpinned it reads 5.37 + the sidereal time of the app's start)" % (st["old_longitude"], GAL_LON))
    capture("again")
    m = C.compare(OUT, "base", OUT, "again")
    RESULTS["in_launch"] = {k: m[k] for k in ("rel", "ratio", "corr", "px8", "px32", "energy_a", "energy_b")}
    check("layer_present", m["energy_a"] > C.ENERGY_MIN, "layer energy %.3e (absent under %.0e)" % (m["energy_a"], C.ENERGY_MIN))
    note("in-launch repeat of the capture: rel %.5f ratio %.5f corr %.6f px8 %d px32 %d" % (m["rel"], m["ratio"], m["corr"], m["px8"], m["px32"]))


def leg_TILT():
    scene()
    value = 0.0 if ARGS.perturb == "withhold-poke" else TILT_RAD
    st = capture("base")
    if st["reference"] != "Earth" or st["freeMode"]:
        check("TILT_reference_is_a_body", False, "reference %s free=%s" % (st["reference"], st["freeMode"]))
        raise SceneNotReached("TILT scene")
    names = poke(SYSTEM, value, "sys")
    capture("sys")
    poke(SYSTEM, 0.0, "sys0")
    capture("rest1")
    names += poke(GALAXY, value, "gal")
    capture("gal")
    poke(GALAXY, 0.0, "gal0")
    capture("rest2")
    check("poked_nodes_are_the_system_and_the_galaxy", any("sys=SolarSystem" in n for n in names) and any("gal=MilkyWay" in n for n in names), str(names))
    rel = {t: C.compare(OUT, "base", OUT, t) for t in ("sys", "rest1", "gal", "rest2")}
    wit = {t: C.frames(OUT, "base", OUT, t) for t in ("sys", "gal")}
    floor = max(rel["rest1"]["rel"], rel["rest2"]["rel"])
    thr = C.threshold(floor)
    RESULTS["tilt"] = {"value_rad": value, "floor_rel": floor, "threshold_rel": thr, "turned_above": C.REL_BOUND,
                       "rule": "STILL = rel <= %g x the in-launch floor (base against the restored captures); TURNED = rel > %g" % (C.K_FLOOR, C.REL_BOUND),
                       "layer": {t: {k: rel[t][k] for k in ("rel", "ratio", "corr", "px8", "px32", "energy_a", "energy_b")} for t in rel},
                       "frame_without_the_layer": {t: {k: wit[t][k] for k in ("rel", "px8", "px32", "energy_a")} for t in wit}}
    for t in ("sys", "rest1", "gal", "rest2"):
        print("-- layer base vs %-5s rel %.5f corr %.6f ratio %.5f px8 %d px32 %d" % (t, rel[t]["rel"], rel[t]["corr"], rel[t]["ratio"], rel[t]["px8"], rel[t]["px32"]), flush=True)
    check("layer_present", rel["sys"]["energy_a"] > C.ENERGY_MIN, "layer energy %.3e" % rel["sys"]["energy_a"])
    check("restoring_the_poke_restores_the_layer", floor <= C.REL_BOUND, "in-launch floor: base vs restored rel %.6f / %.6f" % (rel["rest1"]["rel"], rel["rest2"]["rel"]))
    turned = {t: rel[t]["rel"] > C.REL_BOUND for t in ("sys", "gal")}
    still = {t: rel[t]["rel"] <= thr for t in ("sys", "gal")}
    detail = "system poked rel %.6f, galaxy poked rel %.6f; %s (here %.6f)" % (rel["sys"]["rel"], rel["gal"]["rel"], RESULTS["tilt"]["rule"], thr)
    if ARGS.pin == "off":
        check("TILT_old_pin_follows_no_node", still["sys"] and still["gal"], detail)
    elif ARGS.expect == "system":
        check("TILT_layer_follows_the_SYSTEM_tilt", turned["sys"] and still["gal"], detail)
    else:
        check("TILT_layer_follows_its_OWNER_the_galaxy", turned["gal"] and still["sys"], detail)
    lit = C.layer(OUT, "base")[2]["off_lit_gt16"]
    check("TILT_witnesses_present", lit >= WITNESS_MIN_PX, "frame with the layer off = the bodies, which ride the flat chain: %d px over 16 (want >= %d)" % (lit, WITNESS_MIN_PX))
    for t in ("sys", "gal"):
        w = wit[t]
        check("TILT_%s_poke_moves_nothing_else" % t, w["px32"] == 0,
              "frame with the layer off, base vs %s poked: px32 %d px8 %d rel %.5f (witness energy %.3e)" % (t, w["px32"], w["px8"], w["rel"], w["energy_a"]))


S = B.wait_port(30)
LOST = None
try:
    leg_TILT() if ARGS.leg == "TILT" else leg_level()
except SceneNotReached as e:
    LOST = str(e)
finally:
    RESULTS["fails"] = FAILS
    json.dump(RESULTS, open("%s/f128_%s_results.json" % (OUT, ARGS.leg), "w"), indent=1)
    S.close()
if LOST:
    print("\n=== leg %s STOPPED: scene '%s' not reached - nothing after it was judged ===" % (ARGS.leg, LOST), flush=True)
    sys.exit(3)
print("\n=== leg %s pin %s layer %s perturb %s expect %s: %s ===" % (ARGS.leg, ARGS.pin, ARGS.layer, ARGS.perturb, ARGS.expect,
      "ALL PASS" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
sys.exit(1 if FAILS else 0)
