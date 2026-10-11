#!/usr/bin/env python3
# F131 - the tester's five observations of 2026-09-26, one scene per observation, per pinned path.
#
#   umask 0002; F124_OUT_ROOT=$PWD/artifacts/f131 SC_SEAM_RECORD=1 \
#     bash f124_run.sh <label> f131_obs.py --obs 1|2|3|4|5|6 --pin on|off
#
# Plays the tester's scripts as his keys play them (`script action play`, the same ScriptMgr::playScript
# as ui.cpp:2429 / 2536), the wheel as the command that makes the wheel's call (CoreLink::zoomTo), the
# lat / lon keys as the command their handler runs. Writes shots, dumps (the last one carries the
# per-frame seam ring armed by SC_SEAM_RECORD) and the launch's logs; judges only that each scene was
# reached. The criteria of INTENT 11.292 P5 are judged offline on these records.
import argparse, ast, json, os, shutil, signal, subprocess, sys, time
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import b3_ladder as B

ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--obs", required=True, type=int, choices=[1, 2, 3, 4, 5, 6])
ap.add_argument("--pin", required=True, choices=["on", "off"])
ap.add_argument("--refuse", action="store_true", help="send the script's lines, line 38 inside the travel (the lost race)")
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
OUTP = Path(OUT)
GDBLOG = OUT + "/gdb.log"
FAILS = []

INITIAL_NIGHT = os.path.expanduser("~/.spacecrafter/scripts/internal/initial_night.sts")
FLY_TO_SELECTED = os.path.normpath(os.path.join(HERE, "..", "tester_notes", "fly_to_selected.sts"))
NIGHT_JD = 2461233.5        # f128_legs.py: night at the default site, the band overhead
FADE = 5.0                  # s after a milky way switch (config fader 2 s)
MOUSE_ZOOM = 30             # config.ini mouse_zoom, read by ui.cpp:124
WHEEL_S = 0.2               # the wheel's duration, ui.cpp:450 / 453
O6_VIEW = (180, 25)         # obs 6 look_at azimuth, altitude (deg): the equator in the field from fov 180 to 60
LOGDIR = os.path.expanduser("~/.spacecrafter/log")

_src = HERE + "/f124_legs.py"
_node = next(n for n in ast.parse(open(_src).read()).body if isinstance(n, ast.FunctionDef) and n.name == "reuse")
exec(compile(ast.Module([_node], []), _src, "exec"), globals())
reuse(HERE + "/b21_keypath.py", ["check", "_clean", "gdb_marker_count", "gdb_inject"])
reuse(_src, ["gdb_errors", "header_and_bodies"])


class SceneNotReached(Exception):
    pass


def send(cmd, pause=0.7):
    print("%.3f >> %s" % (time.time(), cmd), flush=True)
    B.send(S, cmd, pause)


def dump(tag, pause=3.0):
    """Write a dual dump, wait for its header, return (header, bodies)."""
    p = "%s/%s.json" % (OUT, tag)
    send("body action dual_dump filename %s" % p, pause)
    for _ in range(40):
        try:
            hdr, bodies = header_and_bodies(tag)
            if hdr is not None:
                c, ctl = hdr["camera"], hdr["control"]
                print("-- dump %-12s drawn=%s ref=%s free=%s bound=%s old_home=%s jd=%.6f frames=%s" % (
                    tag, ctl["drawnPath"], c["reference"], c["freeMode"], c["boundToSurface"],
                    hdr["oldView"]["observer"]["homePlanet"], hdr["jd"], hdr["seam"]["frames"]), flush=True)
                return hdr, bodies
        except (OSError, ValueError, KeyError):
            pass
        time.sleep(0.5)
    raise SceneNotReached("dump %s never readable" % tag)


def shot(tag):
    B.shot(S, OUTP, tag)
    ok = (OUTP / (tag + ".png")).exists()
    check("capture_%s_written" % tag, ok, str(OUTP / (tag + ".png")))
    return ok


def pin():
    """Pin the path and stop if the drawn path is not the pinned one."""
    send("flag experimental_path " + ARGS.pin, 2)
    hdr, _ = dump("pin_check")
    want = "new" if ARGS.pin == "on" else "old"
    check("drawn_path_is_the_pinned_one", hdr["control"]["drawnPath"] == want, "control.drawnPath = %s" % hdr["control"]["drawnPath"])
    if hdr["control"]["drawnPath"] != want:
        raise SceneNotReached("the pin was swallowed")


def milky_way_layer(tag):
    """Shoot the frame with the milky way on, then off, then switch it back on."""
    shot(tag + "_on")
    send("flag milky_way off", FADE)
    shot(tag + "_off")
    send("flag milky_way on", FADE)


def gdb_print(exprs, mark):
    """Print expressions of the stopped engine into gdb.log, between two markers."""
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(['printf "F131BEGIN %s\\n"' % mark] + ["print %s" % e for e in exprs], "F131END_" + mark)
    check("gdb_%s_ran_clean" % mark, ok and not gdb_errors(since), "; ".join(exprs))
    time.sleep(2)


def play(path, settle):
    send("script action play filename %s" % path, 0.5)
    time.sleep(settle)


def obs1():
    dump("pre")
    play(INITIAL_NIGHT, 14)
    shot("end"); dump("end")
    time.sleep(20)
    shot("late"); dump("late")


def fly_prefix():
    """Night at the default site, Saturn selected, the milky way captured, the tester's script played."""
    send("date jday %s" % NIGHT_JD, 1)
    send("select planet Saturn", 3)
    hdr, _ = dump("pre")
    check("saturn_selected", hdr["camera"]["selected"] == "Saturn", "Camera selected = %s" % hdr["camera"]["selected"])
    if hdr["camera"]["selected"] != "Saturn":
        raise SceneNotReached("Saturn not selected")
    milky_way_layer("pre_mw")
    if ARGS.refuse:
        fly_refused()
    else:
        play(FLY_TO_SELECTED, 22)


def fly_refused():
    """Send fly_to_selected.sts lines 13-38 as commands with its waits, line 38 at 2.9 s: inside the 3 s travel."""
    send("body action preload name selected", 0.3)
    send("timerate rate 1", 0.3)
    send("flag track_object on", 1.0)
    send("moveto altitude 20000000 duration 3", 3.0)
    print("%.3f >> lines 21 + 37 in one packet" % time.time(), flush=True)
    S.sendall(b"camera action transition_to target point name Space\n"
              b"camera action move_to target body body_name selected duration 3 altitude 200000\n")
    time.sleep(2.9)
    print("%.3f >> line 38 at +2.9 s" % time.time(), flush=True)
    S.sendall(b"camera action transition_to target body name selected\n")
    time.sleep(17)
    hdr, _ = dump("refused_check")
    check("new_registry_refused_line_38", hdr["camera"]["reference"] == "temp_point", "Camera reference = %s" % hdr["camera"]["reference"])
    if hdr["camera"]["reference"] != "temp_point":
        raise SceneNotReached("the refused state")


def obs2():
    fly_prefix()
    dump("end")
    milky_way_layer("end_mw")


def obs3():
    fly_prefix()
    shot("end_full"); dump("end")
    gdb_print(["ModularBody::lightPosition", "Camera::instance->system->englishName._M_dataplus._M_p",
               "ModularBody::halfFov"], "light")
    send("zoom fov 60 duration 0", 3)
    shot("end_z60"); dump("end_z60")


def obs4():
    fly_prefix()
    shot("before"); dump("before")
    for _ in range(20):
        send("moveto delta_lon 0.5 duration 0.1", 0.4)
    time.sleep(2)
    shot("after_lon"); dump("after_lon")
    for _ in range(20):
        send("moveto delta_lat 0.5 duration 0.1", 0.4)
    time.sleep(2)
    shot("after_lat"); dump("after_lat")


def wheel(factor, clicks, gap):
    """Send the wheel's own call: aim -/+ MOUSE_ZOOM x aim / 60 over 0.2 s, chained on the projector's aim."""
    hdr, _ = dump("aim_%d" % len(WHEEL_LOG), 1.5)
    aim = hdr["oldView"]["projector"]["aimFov"]
    for _ in range(clicks):
        aim = aim + factor * MOUSE_ZOOM * aim / 60.0
        WHEEL_LOG.append(aim)
        send("zoom fov %.9f duration %s" % (aim, WHEEL_S), gap)
    time.sleep(2.0)


def obs5():
    send("date jday %s" % NIGHT_JD, 1)
    send("timerate rate 0", 1)
    send("deselect", 1)
    time.sleep(2)
    dump("pre")
    wheel(-1, 1, 0.07)
    wheel(+1, 1, 0.07)
    wheel(-1, 3, 0.07)
    wheel(+1, 3, 0.07)
    dump("wheel_done")
    send("zoom fov 180 duration 0", 3)
    shot("cap_rest180")
    send("zoom fov 60 duration 3", 1.2)
    shot("cap_mid"); dump("cap_mid", 1.0)
    time.sleep(4)
    shot("cap_rest60"); dump("final")
    json.dump({"wheel_aims": WHEEL_LOG}, open(OUT + "/wheel_aims.json", "w"), indent=1)


def obs6():
    """Draw the equator line red over the equatorial grid blue, looking south, and capture the zoom 180 -> 60 at rest, mid-ramp and rest."""
    send("date jday %s" % NIGHT_JD, 1)
    send("timerate rate 0", 1)
    send("deselect", 1)
    send("flag landscape off", 1)
    send("color property equator_line r 1 g 0 b 0", 0.5)
    send("color property equator_grid r 0 g 0 b 1", 0.5)
    send("flag equatorial_grid on", 0.5)
    send("flag equator_line on", 0.5)
    send("look_at azimuth %d altitude %d duration 0" % O6_VIEW, 2)
    send("zoom fov 180 duration 0", 3)
    hdr, _ = dump("pre")
    json.dump({"look_at_azimuth_deg": O6_VIEW[0], "look_at_altitude_deg": O6_VIEW[1],
               "latitude_deg": hdr["control"]["latitude"]["reported"]}, open(OUT + "/o6_scene.json", "w"), indent=1)
    shot("cap_rest180")
    send("zoom fov 60 duration 3", 1.2)
    shot("cap_mid"); dump("cap_mid", 1.0)
    time.sleep(4)
    shot("cap_rest60"); dump("final")


def copy_logs():
    """Copy the running launch's log channels into the record."""
    for ch in ("spacecrafter", "script"):
        p = subprocess.run([sys.executable, HERE + "/logread.py", "--live", LOGDIR, ch], capture_output=True, text=True).stdout.strip()
        if p and os.path.exists(p):
            shutil.copyfile(p, "%s/log_%s.txt" % (OUT, ch))


WHEEL_LOG = []
S = B.wait_port(30)
LOST = None
try:
    pin()
    {1: obs1, 2: obs2, 3: obs3, 4: obs4, 5: obs5, 6: obs6}[ARGS.obs]()
    if ARGS.obs in (2, 3, 4):
        dump("final")
except SceneNotReached as e:
    LOST = str(e)
finally:
    copy_logs()
    S.close()
if LOST:
    print("\n=== obs %d pin %s STOPPED: scene '%s' not reached ===" % (ARGS.obs, ARGS.pin, LOST), flush=True)
    sys.exit(3)
print("\n=== obs %d pin %s: %s ===" % (ARGS.obs, ARGS.pin, "SCENE REACHED" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
sys.exit(1 if FAILS else 0)
