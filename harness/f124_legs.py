#!/usr/bin/env python3
# F124 - the input path's instrument: gdb legs on the b21 pattern, both pinned paths.
#
#   f124_run.sh <label> f124_legs.py --leg Z|L|A|M [--pin on|off] [--perturb withhold-new]
#
# Each leg calls the function the INPUT PATH calls (UI::raiseHeight, UI::zoomIn, UI::moveLon ...
# then Core::updateMove), never the Camera, and reads both engines through `body action
# dual_dump` (header: camera / control / oldView / ramp). One application per real frame.
# A check named RESID_* asserts a PREDICTED RESIDUAL of INTENT 11.261 (P): green = the
# residual is there with its cause; it turns red the day the code under it is reshaped.
# --perturb withhold-new gives the step to the OLD engine alone: the leg's new == old check
# must go red (exit 0 = it did, the leg is able to fail; exit 1 = the leg is blind).
# What is reused: check / send / _clean / gdb_marker_count / gdb_inject are b21_keypath.py's
# own functions, taken from that file (it runs its scene at import, so it cannot be imported).
import argparse, ast, json, math, os, signal, socket, struct, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--leg", required=True, choices=["Z", "L", "A", "M"])
ap.add_argument("--pin", default="on", choices=["on", "off"])
ap.add_argument("--perturb", default="none", choices=["none", "withhold-new"])
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
GDBLOG = OUT + "/gdb.log"
FAILS = []
PERT = ARGS.perturb == "withhold-new"


def reuse(path, names):
    names = set(names)
    for node in ast.parse(open(path).read()).body:
        if isinstance(node, ast.FunctionDef) and node.name in names:
            exec(compile(ast.Module([node], []), path, "exec"), globals())
            names.discard(node.name)
    if names:
        sys.exit("cannot reuse %s from %s" % (sorted(names), path))


reuse(HERE + "/b21_keypath.py", ["check", "send", "_clean", "gdb_marker_count", "gdb_inject"])

AU_KM = 149597870.7
R_KM = 6000.0                       # DescB, as loaded below
FLOOR_KM = 0.125 * R_KM             # MIN_MOVEMENT_SPEED x scaled radius
F32 = lambda x: struct.unpack("f", struct.pack("f", x))[0]
UP, DN = F32(1.01) - 1.0, F32(0.99) - 1.0
ZOOM_SPEED = 0.0001                 # config.ini zoom_speed, asserted against the ring below
TOL_KM = 0.005                      # 10x the float32 quantum of `distance` at 8000 km
RESULTS = {"leg": ARGS.leg, "pin": ARGS.pin, "perturb": ARGS.perturb, "states": [], "notes": []}
STEP = [0]


def note(txt):
    print("NOTE " + txt, flush=True)
    RESULTS["notes"].append(txt)


def state(tag, pause=1.5):
    path = "%s/%s.json" % (OUT, tag)
    send(S, "body action dual_dump filename " + path, pause)
    h = None
    with open(path) as f:
        for ln in f:
            ln = _clean(ln)
            if ln.startswith('{"type":"header"'):
                h = json.loads(ln)
                break
    cam, ctl, prj = h["camera"], h["control"], h["oldView"]["projector"]
    st = dict(tag=tag, ref=cam["reference"], free=cam["freeMode"],
              alt_new=ctl["altitude"]["new"] / 1000.0, alt_old=ctl["altitude"]["old"] / 1000.0,
              alt_reported=ctl["altitude"]["reported"] / 1000.0,
              lon_new=ctl["longitude"]["new"], lon_old=ctl["longitude"]["old"],
              lat_new=ctl["latitude"]["new"], lat_old=ctl["latitude"]["old"],
              dist_member_km=cam["distance"] * AU_KM, pos_km=[x * AU_KM for x in cam["position"]],
              fov_old=prj["fov"], fov_new=2.0 * math.degrees(cam["halfFov"]), aim_old=prj["aimFov"],
              max_fov=prj["maxFov"], auto_zoom=prj["flagAutoZoom"], zoom_duration=cam["plans"]["zoomDuration"],
              drawn=ctl["drawnPath"])
    print("-- %-16s drawn=%s free=%s alt new %.4f old %.4f km | lon new %.5f old %.5f | lat new %.5f old %.5f"
          " | fov new %.6f old %.6f" % (tag, st["drawn"], st["free"], st["alt_new"], st["alt_old"], st["lon_new"],
                                         st["lon_old"], st["lat_new"], st["lat_old"], st["fov_new"], st["fov_old"]),
          flush=True)
    RESULTS["states"].append(dict(st))
    st["ramp"] = h["ramp"]["steps"]
    return st


def header_and_bodies(tag):
    hdr, bodies = None, {}
    with open("%s/%s.json" % (OUT, tag)) as f:
        for ln in f:
            try:
                o = json.loads(_clean(ln))
            except Exception:
                continue
            if o.get("type") == "header":
                hdr = o
            elif o.get("type") == "body":
                bodies[o["name"]] = o
    return hdr, bodies


def radii(tag, name):
    """Scaled ground and datum radius of a body as the new path holds them, in km, from a dump."""
    new = header_and_bodies(tag)[1][name]["new"]
    return new["scaledGroundRadius"] * AU_KM, new["scaledDatumRadius"] * AU_KM


def refcached(tag):
    return header_and_bodies(tag)[0]["camera"]["refCached"]


def gdb_errors(since):
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        txt = f.read().decode("latin-1")
    bad = [l for l in txt.splitlines() if any(k in l for k in (
        "No symbol", "Cannot ", "Couldn't", "There is no member", "Invalid ", "not defined", "SIGSEGV", "SIGABRT"))]
    return bad


def inject(frames):
    """frames = list of call lists; each list runs at the entry of one real Core::updateMove."""
    since = os.path.getsize(GDBLOG)
    cmds = []
    for calls in frames:    # the breakpoint goes before the calls: updateMove is one of them
        cmds += ["break Core::updateMove", "continue", "set $core = this", "delete breakpoints"] + calls
    ok = gdb_inject(cmds, "F124STEP%d" % STEP[0])
    STEP[0] += 1
    bad = gdb_errors(since)
    if not ok or bad:
        check("gdb_step_%d_ran_clean" % (STEP[0] - 1), False, "handshake=%s gdb said: %s" % (ok, bad[:3]))
    time.sleep(1.0)
    return ok and not bad


def capture_ui():
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(["break UI::updateTimeouts", "continue", "set $ui = this",
                     'printf "F124UI=%p\\n", $ui', "delete breakpoints"], "F124UICAP")
    check("gdb_reaches_the_UI_object", ok and not gdb_errors(since), "`this` of UI::updateTimeouts, the per-frame UI call")


def raise_calls(dt):
    if PERT:
        return ["call $core->observatory._M_ptr->multAltitude(1.01)"]
    return ["call $ui->raiseHeight()", "call $core->updateMove(%d)" % dt, "call $ui->stopRaiseHeight()"]


def lower_calls(dt):   # Core::lowerHeight(1) has no caller in src/: driven directly, said in the map
    return ["call $core->lowerHeight(1)", "call $core->updateMove(%d)" % dt, "call $core->lowerHeight(0)"]


def lateral_calls(axis):   # the UI writes a joystick sample; one updateMove(10) applies and clears it
    return ["call $ui->move%s(1.0)" % axis, "call $core->updateMove(10)"]


def zoom_calls(way, dt):
    if PERT:
        return ["call $core->projection->changeFov(-1.0)"]
    return ["call $ui->zoom%s()" % way, "call $core->updateMove(%d)" % dt, "call $ui->stopZoom%s()" % way]


def scene():
    send(S, "flag experimental_path " + ARGS.pin, 1)
    send(S, "date jday 2461233.5", 1)
    send(S, "timerate rate 0", 1)
    for f in ("atmosphere", "fog", "landscape", "show_fps"):
        send(S, "flag %s off" % f)
    send(S, "body action load name DescB radius 6000 parent Sun type Planet oblateness 0.0 "
            "albedo 0.3 halo false color 0.6,0.6,0.9 tex_map bodies/moon.png "
            "coord_func still_orbit orbit_x 1200 orbit_y 0 orbit_z 0", 3)
    send(S, "set home_planet DescB", 3)


def place(alt_km, lat=0.0, lon=0.0):
    send(S, "moveto lat %s lon %s alt %d duration 0" % (lat, lon, round(alt_km * 1000)), 2.5)


def free_scene():
    send(S, "camera action free_mode state on", 1.5)
    send(S, "select planet DescB", 1)
    send(S, "flag track_object on", 2)


class SceneNotReached(Exception):
    pass


def placed(tag, alt_km, free):
    """Place both engines, then refuse to go on if the scene is not the one the sub-leg judges."""
    place(alt_km)
    st = state(tag)
    drawn = "new" if ARGS.pin == "on" else "old"
    ok = (abs(st["alt_new"] - alt_km) < 0.5 and abs(st["alt_old"] - alt_km) < 0.5
          and st["free"] is free and st["ref"] == "DescB" and st["drawn"] == drawn)
    check(tag + "_scene", ok, "want %.1f km on both, free=%s, drawn=%s: new %.4f old %.4f free=%s drawn=%s ref=%s"
          % (alt_km, free, drawn, st["alt_new"], st["alt_old"], st["free"], st["drawn"], st["ref"]))
    if not ok:
        raise SceneNotReached(tag)
    return st


def climb(tag, st, calls, n, law_new, law_old, equal, resid=""):
    """n one-frame steps; after each, new / old against their law, and new == old where predicted."""
    for i in range(n):
        want_new, want_old = law_new(st["alt_new"]), law_old(st["alt_old"])
        inject([calls])
        nx = state("%s_s%d" % (tag, i + 1))
        name = "%s%s_s%d" % (resid, tag, i + 1)
        check(name + "_new_follows_its_law", abs(nx["alt_new"] - want_new) < TOL_KM,
              "new %.4f -> %.4f km, law says %.4f (|d| %.2e)" % (st["alt_new"], nx["alt_new"], want_new, abs(nx["alt_new"] - want_new)))
        if not PERT:
            check(name + "_old_follows_its_law", abs(nx["alt_old"] - want_old) < TOL_KM,
                  "old %.4f -> %.4f km, law says %.4f" % (st["alt_old"], nx["alt_old"], want_old))
        if equal:
            check("%s_s%d_new_equals_old" % (tag, i + 1), abs(nx["alt_new"] - nx["alt_old"]) < TOL_KM,
                  "new %.4f old %.4f km |d| %.2e" % (nx["alt_new"], nx["alt_old"], abs(nx["alt_new"] - nx["alt_old"])))
        else:
            note("%s_s%d residual new-old = %+.4f km" % (tag, i + 1, nx["alt_new"] - nx["alt_old"]))
        st = nx
    return st


anch_up = lambda a: a + max(a, FLOOR_KM) * UP        # datum-relative `distance`, floored both ways
anch_dn = lambda a: a + max(a, FLOOR_KM) * DN
free_up = lambda a: a + max(a, FLOOR_KM) * UP        # live ground altitude, floored outward only
geo = lambda c: (lambda a: a * c)
# B44 (F137, INTENT 11.299): one law through the drawn engine, both engines, the step follows dt
free_dn = lambda a: a + max(a, FLOOR_KM) * DN        # the free descent is floored too
law40 = lambda a: a + max(a, FLOOR_KM) * 0.04        # dt = 40 ms
old_dn = lambda a: max(anch_dn(a), 0.0001)           # Observer::setAltitude clamps at 0.1 m
STEP_DEG = math.degrees(FLOOR_KM * 0.01 / (R_KM + 200.0))   # lateral at 200 km, dt 10: 0.069309 deg
FREE_STEP_KM = 5.0 * FLOOR_KM * 0.01                 # free lateral at the floor, dt 10: 37.5 km


def leg_M():
    scene(); capture_ui()
    st = placed("M_high", 2000.0, False)
    st = climb("M_high", st, raise_calls(10), 1 if PERT else 3, anch_up, geo(1.01), True)
    if PERT:
        return
    inject([["call $ui->lowerHeight()", "call $core->updateMove(10)", "call $ui->stopLowerHeight()"]])
    nx = state("M_button")
    check("M_lower_button_is_one_held_step_on_both", abs(nx["alt_new"] - anch_dn(st["alt_new"])) < TOL_KM
          and abs(nx["alt_old"] - anch_dn(st["alt_old"])) < TOL_KM,
          "press + updateMove(10) + release: new %.4f -> %.4f, old %.4f -> %.4f km (law %.4f); reported altitude was %.4f"
          % (st["alt_new"], nx["alt_new"], st["alt_old"], nx["alt_old"], anch_dn(st["alt_new"]), st["alt_reported"]))
    st = placed("M_lowup", 10.0, False)
    a0 = st["alt_new"]
    st = climb("M_lowup", st, raise_calls(10), 3, anch_up, anch_up, True)
    step10 = (st["alt_new"] - a0) / 3
    check("RESID_M_floor_is_0.125_R", abs(step10 / UP - FLOOR_KM) < 1.0,
          "step per call / (coef-1) = %.3f km, 0.125 x R = %.3f km" % (step10 / UP, FLOOR_KM))
    st = placed("M_lowdn", 10.0, False)
    st = climb("M_lowdn", st, lower_calls(10), 3, anch_dn, old_dn, False, "RESID_")
    check("RESID_M_descent_goes_under_the_datum", st["alt_new"] < 0.0 and st["alt_old"] > 0.0,
          "after 3 descents from 10 km: new %.4f km, old %.4f km" % (st["alt_new"], st["alt_old"]))
    st = placed("D_M_lowup40", 10.0, False)
    a0 = st["alt_new"]
    st = climb("D_M_lowup40", st, raise_calls(40), 2, law40, law40, True)
    step40 = (st["alt_new"] - a0) / 2
    check("D_M_step_per_call_follows_dt", abs(step40 / step10 - 4.0) < 1e-3,
          "per call %.4f km at dt=40 against %.4f at dt=10 (ratio %.5f): per SECOND %.1f km/s at 25 fps against %.1f at 100 fps"
          % (step40, step10, step40 / step10, step40 * 25, step10 * 100))
    send(S, "body name DescB ground_radius 6100", 4)
    state("M_refcmd")
    g, d = radii("M_refcmd", "DescB")
    check("radius_command_reaches_the_reference_body", abs(g - 6100.0) < 1.0,
          "`body name DescB ground_radius 6100` while DescB is the Camera's reference, 4 s later: scaled ground %.1f km,"
          " datum %.1f km, reference cache fresh = %s" % (g, d, refcached("M_refcmd")))
    send(S, "set home_planet Moon", 5)
    send(S, "moveto lat 0 lon 0 alt 10000 duration 0", 3)
    st = state("M_moon")
    datum = st["dist_member_km"] - st["alt_new"]
    inject([raise_calls(10)])
    nx = state("M_moon_s1")
    ratio = (nx["alt_new"] - st["alt_new"]) / UP / datum
    check("RESID_M_floor_scales_with_the_body", st["ref"] == "Moon" and abs(ratio - 0.125) < 1e-3,
          "Moon, scaled datum %.1f km, from %.4f km: step / (coef-1) / datum = %.5f (0.125 if the floor is R's)"
          % (datum, st["alt_new"], ratio))
    send(S, "set home_planet DescB", 5)
    st = placed("M_datum", 2000.0, False)
    g, d = radii("M_datum", "DescB")
    apart = abs(g - d) > 50.0
    check("M_datum_scene_separates_the_two_radii", apart, "measured in the dump: scaled ground %.1f km, datum %.1f km" % (g, d))
    if apart:
        inject([raise_calls(10)])
        nx = state("M_datum_s1")
        base = (nx["alt_new"] - st["alt_new"]) / UP
        over_ground = st["alt_new"] - (g - d)
        check("M_base_is_datum_relative", abs(base - st["alt_new"]) < 0.5 and abs(base - over_ground) > 50.0,
              "step / (coef-1) = %.3f km; altitude over the datum %.3f km, over the ground %.3f km"
              % (base, st["alt_new"], over_ground))


def leg_A():
    scene(); capture_ui(); free_scene()
    st = placed("A_high", 2000.0, True)
    st = climb("A_high", st, raise_calls(10), 1 if PERT else 2, free_up, geo(1.01), True)
    if PERT:
        return
    st = placed("A_below", 200.0, True)
    a0 = st["alt_new"]
    st = climb("A_below", st, raise_calls(10), 2, free_up, free_up, True)
    step10 = (st["alt_new"] - a0) / 2
    st = placed("A_upcross", 740.0, True)
    climb("A_upcross", st, raise_calls(10), 4, free_up, free_up, True)
    st = placed("A_dncross", 760.0, True)
    climb("A_dncross", st, lower_calls(10), 4, free_dn, free_dn, True)
    st = placed("A_asym", 200.0, True)
    st = climb("A_asym_up", st, raise_calls(10), 1, free_up, free_up, True)
    st = climb("A_asym_dn", st, lower_calls(10), 1, free_dn, free_dn, True)
    check("A_raise_then_lower_returns", abs(st["alt_new"] - 200.0) < 0.01 and abs(st["alt_old"] - 200.0) < 0.01,
          "200 -> raise -> lower = %.4f km (old %.4f); one law both ways" % (st["alt_new"], st["alt_old"]))
    st = placed("D_A_below40", 200.0, True)
    a0 = st["alt_new"]
    st = climb("D_A_below40", st, raise_calls(40), 2, law40, law40, True)
    step40 = (st["alt_new"] - a0) / 2
    check("D_A_step_per_call_follows_dt", abs(step40 / step10 - 4.0) < 1e-3,
          "per call %.4f km at dt=40 against %.4f at dt=10 (ratio %.5f)" % (step40, step10, step40 / step10))
    st = placed("A_button", 2000.0, True)
    inject([["call $ui->lowerHeight()", "call $core->updateMove(10)", "call $ui->stopLowerHeight()"]])
    nx = state("A_button_s1")
    check("A_lower_button_is_one_held_step_on_both", abs(nx["alt_new"] - free_dn(st["alt_new"])) < TOL_KM
          and abs(nx["alt_old"] - free_dn(st["alt_old"])) < TOL_KM,
          "free lower BUTTON, press + updateMove(10) + release: new %.4f -> %.4f km, old %.4f -> %.4f km (law %.4f)"
          % (st["alt_new"], nx["alt_new"], st["alt_old"], nx["alt_old"], free_dn(st["alt_new"])))


def dpos(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a["pos_km"], b["pos_km"])))


def leg_L():
    scene(); capture_ui()
    st = placed("L_anch", 200.0, False)
    inject([["call $core->observatory._M_ptr->moveRelLon(1.0, 0)"] if PERT else lateral_calls("Lon")])
    nx = state("L_anch_lon")
    dn, do = nx["lon_new"] - st["lon_new"], nx["lon_old"] - st["lon_old"]
    check("L_anchored_lon_new_equals_old", abs(dn - do) < 1e-5 and abs(abs(do) - STEP_DEG) < 1e-5,
          "UI::moveLon(1) + updateMove(10): longitude new %+.6f deg, old %+.6f deg (law %.6f)" % (dn, do, STEP_DEG))
    if PERT:
        return
    inject([lateral_calls("Lat")])
    st, nx = nx, state("L_anch_lat")
    dn, do = nx["lat_new"] - st["lat_new"], nx["lat_old"] - st["lat_old"]
    check("L_anchored_lat_new_equals_old", abs(dn - do) < 1e-5 and abs(abs(do) - STEP_DEG) < 1e-5,
          "UI::moveLat(1) + updateMove(10): latitude new %+.6f deg, old %+.6f deg (law %.6f)" % (dn, do, STEP_DEG))
    sgn = 1.0 if dn > 0 else -1.0
    place(200.0, lat=89.95 * sgn); st = state("L_pole")
    inject([lateral_calls("Lat")])
    nx = state("L_pole_s1")
    check("RESID_L_new_latitude_passes_the_pole", abs(nx["lat_old"] - 90.0 * sgn) < 1e-3 and abs(nx["lat_new"] - (89.95 + STEP_DEG) * sgn) < 1e-3,
          "from %.2f: old %.5f (clamped), new %.5f (not clamped)" % (st["lat_new"], nx["lat_old"], nx["lat_new"]))
    place(200.0); free_scene(); st = placed("L_free", 200.0, True)
    inject([lateral_calls("Lon")])
    nx = state("L_free_s1")
    want = FREE_STEP_KM
    check("L_free_step_is_x_5_velocityScaling", abs(dpos(st, nx) - want) < 0.1,
          "UI::moveLon(1) + updateMove(10) free at 200 km: |d position| %.3f km, law 5 x velocityScaling(0.01) = %.3f km;"
          " old longitude %+.5f deg" % (dpos(st, nx), want, nx["lon_old"] - st["lon_old"]))
    send(S, "camera action free_mode state off", 1.5)
    place(2000.0)
    send(S, "camera action free_mode state on", 1.5)
    st = placed("L_stale", 200.0, True)
    inject([lateral_calls("Lon")])
    nx = state("L_stale_s1")
    stale, live = 5.0 * 2000.0 * 0.01, want
    check("L_free_base_is_the_live_distance", abs(dpos(st, nx) - live) < 0.5,
          "free at a LIVE 200 km after entering free mode at 2000 km: |d position| %.3f km; live base %.3f, stale base would give %.3f;"
          " `distance` member reads %.1f km" % (dpos(st, nx), live, stale, st["dist_member_km"]))


def ring_steps(st, dt):
    return [r for r in st["ramp"] if r["active"] and r["dt"] == dt and r["dFov"] != 0]


def fov_pair(tag, want):
    st = state(tag)
    check(tag + "_both_at_%g" % want, abs(st["fov_old"] - want) < 1e-3 and abs(st["fov_new"] - want) < 1e-3,
          "projector %.6f, camera %.6f deg" % (st["fov_old"], st["fov_new"]))
    return st


def leg_Z():
    scene(); capture_ui()
    send(S, "zoom fov 60 duration 1", 4); fov_pair("Z_start", 60.0)
    if PERT:
        inject([zoom_calls("In", 10)])
        st = state("Z_pert")
        check("Z_at_rest_new_equals_old", abs(st["fov_new"] - st["fov_old"]) < 1e-3,
              "projector %.6f camera %.6f deg" % (st["fov_old"], st["fov_new"]))
        return
    inject([zoom_calls("In", 10)] * 4)
    inject([zoom_calls("In", 40)] * 4)
    inject([zoom_calls("Out", 10)] * 4)
    st = state("Z_ramp")
    for dt, n in ((10, 8), (40, 4)):
        rs = ring_steps(st, dt)
        check("Z_ring_holds_the_dt%d_steps" % dt, len(rs) == n, "%d active steps at dt=%d in the ramp ring, want %d" % (len(rs), dt, n))
        worst = max([abs(2 * math.degrees(r["halfFovAfter"]) - r["fovAfter"]) / r["fovAfter"] for r in rs] or [9])
        check("Z_mid_ramp_camera_equals_projector_dt%d" % dt, worst < 2e-6,
              "worst relative |2 x halfFov - fov| over the steps: %.2e" % worst)
        k = 5 * ZOOM_SPEED * dt
        # The order's magnitude is the DRAWN fov (Core::drawnFov): the ring's halfFov on the new pin, the projector's on the old
        drawn = (lambda r: 2 * math.degrees(r["halfFov"])) if ARGS.pin == "on" else (lambda r: r["fov"])
        bad = [r for r in rs if abs(r["fovAfter"] / r["fov"] - (1 + (-k if r["dFov"] < 0 else k) * drawn(r) / r["fov"])) > 1e-9]
        check("Z_per_call_factor_is_5_zoom_speed_dt%d" % dt, bool(rs) and not bad,
              "per call x%.4f in / x%.4f out; off-law steps: %d" % (1 - k, 1 + k, len(bad)))
    check("Z_at_rest_new_equals_old", abs(st["fov_new"] - st["fov_old"]) < 1e-3,
          "projector %.6f camera %.6f deg" % (st["fov_old"], st["fov_new"]))
    note("max_fov reads %.4f deg in the dump (the 0.49 deg stop residual is predicted only if this is 350)" % st["max_fov"])
    for way in ("In", "Out"):
        tag = "Z2_" + ("same" if way == "In" else "opposite")
        send(S, "zoom fov 60 duration 1", 4); fov_pair(tag + "_reset", 60.0)
        send(S, "zoom fov 20 duration 20", 3)
        inject([zoom_calls(way, 10)])
        tap = state(tag + "_tap", 0.5)
        note("%s: at the tap projector %.4f camera %.4f deg, projector auto-zoom %s, camera plan %.3f s"
             % (tag, tap["fov_old"], tap["fov_new"], tap["auto_zoom"], tap["zoom_duration"]))
        time.sleep(24)
        rest = state(tag + "_rest")
        check("%s_engines_rest_together" % tag, abs(rest["fov_old"] - 20.0) < 1e-3 and abs(rest["fov_new"] - rest["fov_old"]) < 1e-4,
              "wheel-style plan 60 -> 20 with one key step %s: at rest projector %.4f, camera %.4f deg (apart by %.4f)"
              % (way, rest["fov_old"], rest["fov_new"], abs(rest["fov_new"] - rest["fov_old"])))


S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
SCENE_LOST = None
try:
    {"Z": leg_Z, "L": leg_L, "A": leg_A, "M": leg_M}[ARGS.leg]()
except SceneNotReached as e:
    SCENE_LOST = str(e)
finally:
    RESULTS["fails"] = FAILS
    json.dump(RESULTS, open("%s/f124_%s_results.json" % (OUT, ARGS.leg), "w"), indent=1)
    S.close()
if SCENE_LOST:
    print("\n=== leg %s STOPPED: scene '%s' not reached - nothing after it was judged ===" % (ARGS.leg, SCENE_LOST), flush=True)
    sys.exit(3)
if PERT:
    red = [f for f in FAILS if f.endswith("new_equals_old")]
    print("\n=== ABLE-TO-FAIL leg %s: %s ===" % (ARGS.leg, ("RED as it must be: " + ",".join(red)) if red
          else "STAYED GREEN - THE LEG IS BLIND"), flush=True)
    sys.exit(0 if red else 1)
print("\n=== leg %s pin %s: %s ===" % (ARGS.leg, ARGS.pin, "ALL PASS" if not FAILS else "FAILURES: " + ",".join(FAILS)), flush=True)
sys.exit(1 if FAILS else 0)
