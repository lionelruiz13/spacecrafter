#!/usr/bin/env python3
"""F43 - WHAT `moveto ... alt` COUNTS ALTITUDE FROM, AND WHEN IT READS IT.

WHY THIS PROBE EXISTS (INTENT 11.153(j)(3) -> 11.157, task F43)
---------------------------------------------------------------
`b24_select.py` carries four pre-existing failures whose whole content is that
its observer sits 2013.40 km too far from the Moon in the FIRST dump of the run
and exactly where predicted in the SECOND.  The candidate mechanism, read at
source:

  Camera::moveTo's free branch   -> -posePart(lon, lat, reference->getAltitudeReference() + alt)
                                    [observed: Camera.cpp:713-714]
  Camera::moveTo's anchored branch -> distance = reference->getAltitudeReference() + alt
                                    [observed: Camera.cpp:729]
  ModularBody::getAltitudeReference() { return scaledDatumRadius; }
                                    [observed: ModularBody.hpp:1506-1508]

so a commanded altitude is counted from the DISPLAY-SCALED datum, and
`scaling` is "a 5 s ASmooth ramp" [observed: SurfacePointOrbitLoader.hpp:70-71].
`moveTo` is an ABSOLUTE SNAP by construction ("absolute by meaning",
Camera.cpp:723) - it is evaluated once - so a `moveto` issued while a scale ramp
is running binds the observer to an INSTANTANEOUS radius that the ramp then
leaves behind, and nothing re-converges it.

THE TWO CLAIMS, each with its number predicted before the run
  M1 (the timing half): the radius the observer lands at equals the reference's
     scaledDatumRadius AT THE INSTANT OF THE COMMAND, plus the commanded
     altitude.  Bracketed, because a dump is not instantaneous: a dump taken
     just BEFORE and just AFTER the moveto brackets that instant, and the ramp
     is monotone, so
         landing_radius - 8000 km  must lie between them.
  M2 (the layer half): in the SETTLED state the same command lands
         scale 1 -> 1737.40 + 8000 = 9737.40 km
         scale 5 -> 8687.00 + 8000 = 16687.00 km
     i.e. 6949.60 km apart = datum x (scale - 1), which is exactly the magnitude
     11.101(f) recorded for the composed-rover load-time latch that D21 closed
     in the ORBIT layer.  The camera's altitude reference is the same two-layer
     question, still answered the display way.

Every leg states what it expects; a leg that cannot fail is not a measurement.
The scale flag is traversed on->off->on->off (twice in each direction, the
second entry from the state the first exit produced).

PRECONDITIONS: temp-HOME symlink farm (11.103(a)); fresh launch; the real
~/.spacecrafter is never written; DISPLAY must be a real X server.

usage: f43_ramp.py <outdir> [--bin /abs/binary]
"""
import json, math, os, socket, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REAL_HOME = Path.home()
AU_KM = 149597870.0
JD = 2461234.0
PARENT = "Moon"
R_KM = 1737.4          # [observed: ~/.spacecrafter/ssystem.ini [moon] radius]
OBS_LON, OBS_LAT = 39.7, 0.0
OBS_ALT_M = 8000000    # 8000 km, b24_select's own altitude
ROVER = "RampRover"    # a grounded child, b24_select's own authoring
ROVER_ALT_KM = 500.0
SCALE = 5              # [observed: ~/.spacecrafter/config.ini:240 moon_scale = 5
                       #  and modularSystem twin [Moon] display_scale = 5]
FARM = Path("/tmp/f43_ramp_farm")

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m):   print(f"ok:   {m}", flush=True)


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def load_dump(path):
    bodies = {}; cam = None
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            cam = o.get("camera")
        if o.get("type") == "body":
            bodies[o["name"]] = o
    return bodies, cam


SEQ = [0]
LAST_BODIES = {}   # every body of the last probe, for the legs that need more
                   # than the parent (R7); the 3-tuple return stays as it is so
                   # the ramp legs read one thing and read it the same way.
def probe(sock, out, tag):
    """One dump, returned as (wall-clock at request, moon-new-record, camera)."""
    SEQ[0] += 1
    p = out / f"{tag}_{SEQ[0]:03d}.json"
    if p.exists():
        p.unlink()
    t = time.time()
    send(sock, f"body action dual_dump filename {p}", 0.9)
    for _ in range(40):
        if p.exists() and p.stat().st_size > 0:
            b, c = load_dump(p)
            LAST_BODIES.clear(); LAST_BODIES.update(b)
            return t, (b.get(PARENT, {}).get("new") or {}), c
        time.sleep(0.2)
    raise RuntimeError(f"dump {tag} never written")


def radius_km(bodies, name, key):
    """|position| of a body's `key` vector, in km.  `ecl`/`eclDisplay` are
    PARENT-RELATIVE (checked: the Moon's own `ecl` has magnitude 359 850 km =
    its distance from Earth, its parent), so for a body grounded on PARENT this
    IS its radius from the parent's centre - no subtraction, which would mix two
    frames (it did, and reported 360 328 km before this was read off the dump)."""
    p = (bodies.get(name, {}).get("new") or {})[key]
    return math.sqrt(sum(x * x for x in p)) * AU_KM


def sdr_km(moon):
    return moon["scaledDatumRadius"] * AU_KM


def refdist_km(cam):
    return cam["refDist"] * AU_KM


def settle(sock, out, tag, want, tol=1e-4, tries=25):
    """Poll until scaling reaches its target and stops moving - a MEASURED
    precondition, not a sleep.  Returns (elapsed, samples)."""
    t0 = time.time(); samples = []; prev = None
    for _ in range(tries):
        t, m, _c = probe(sock, out, tag)
        samples.append((t - t0, m["scaling"], sdr_km(m)))
        if abs(m["scaling"] - want) < tol and prev is not None and abs(m["scaling"] - prev) < tol:
            return t - t0, samples
        prev = m["scaling"]
    fail(f"settle({tag}): scaling never reached {want} (last {samples[-1] if samples else None})")
    return time.time() - t0, samples


def main():
    argv = sys.argv[1:]
    binp = HERE.parents[1] / "build-claude/src/spacecrafter"
    if "--bin" in argv:
        binp = Path(argv[argv.index("--bin") + 1])
    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)

    pred = {
        "settled_scale1_radius_km": R_KM + OBS_ALT_M / 1000.0,
        "settled_scale5_radius_km": R_KM * SCALE + OBS_ALT_M / 1000.0,
        "layer_gap_km": R_KM * (SCALE - 1),
        "b24_select_observed_radius_km": 11750.80,
        # R7, the D21 acceptance question for the OBSERVER's own convention.
        # A grounded child at orbit_alt 500 km, drawn radius from the parent's
        # centre, at scaling 5:
        "rover_scale1_drawn_km": R_KM + ROVER_ALT_KM,               # 2237.40
        "rover_if_uniform_dilation_km": (R_KM + ROVER_ALT_KM) * SCALE,   # 11187.00 (D21)
        "rover_if_camera_convention_km": R_KM * SCALE + ROVER_ALT_KM,    #  9187.00
    }
    print(json.dumps(pred, indent=1), flush=True)

    subprocess.run(["bash", str(HERE / "b3_farm.sh"), str(FARM)], check=True)
    twin = REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled"
    farmdir = FARM / ".spacecrafter"
    section = (f"\n[{ROVER}]\nname = {ROVER}\nparent = {PARENT}\nrelation = grounded\n"
               f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
               f"orbit_lon = {OBS_LON - 90.0:.6f}\norbit_lat = 60.000000\n"
               f"orbit_alt = {ROVER_ALT_KM}\nradius = 100.0\n"
               f"model_name = Star_White\nhalo = false\n"
               f"[{ROVER}:OJM]\nbody = {ROVER}\ntype = OJM\n")
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(
        twin.read_bytes() + section.encode("latin-1"))

    env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([str(binp)], cwd=str(farmdir),
                            stdout=open(out / "ramp.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    rep = {"bin": str(binp), "pred": pred, "legs": {}}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "flag experimental_path on"); send(s, "timerate rate 0")
        send(s, "meteors zhr 0"); send(s, f"date jday {JD}")
        send(s, f"set home_planet {PARENT}", 2)
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off"); send(s, "flag landscape off")
        send(s, "flag stars off"); send(s, "flag nebulae off", 1)

        # ---- R0: the shipped state is scale 5 (flag_moon_scaled = true) ----
        _t, m0, _c = probe(s, out, "r0")
        rep["legs"]["R0_shipped"] = dict(scaling=m0["scaling"], sdr_km=sdr_km(m0))
        if abs(sdr_km(m0) - R_KM * SCALE) < 1.0:
            ok(f"R0: the shipped state draws the Moon at scaling {m0['scaling']:.5f}, "
               f"scaledDatumRadius {sdr_km(m0):.2f} km (= {R_KM} x {SCALE})")
        else:
            fail(f"R0: scaledDatumRadius {sdr_km(m0):.2f} km, expected {R_KM*SCALE:.2f} "
                 f"- the scene is not the one this probe reasons about")

        # ---- R1: the RAMP PROFILE of `flag moon_scaled off` (5 -> 1) ----
        t0 = time.time()
        send(s, "flag moon_scaled off", 0.0)
        prof = []
        for _ in range(12):
            t, m, _c = probe(s, out, "r1")
            prof.append(dict(t=t - t0, scaling=m["scaling"], sdr_km=sdr_km(m)))
            if abs(m["scaling"] - 1.0) < 1e-4 and len(prof) > 1 and abs(prof[-2]["scaling"] - 1.0) < 1e-4:
                break
        rep["legs"]["R1_ramp_profile"] = prof
        span = [p for p in prof if p["scaling"] > 1.0001]
        print("  ramp: " + ", ".join(f"t={p['t']:.2f}s scaling={p['scaling']:.4f} "
                                     f"sdr={p['sdr_km']:.1f}km" for p in prof), flush=True)
        if span:
            ok(f"R1: `flag moon_scaled off` RAMPS - {len(span)} sample(s) caught between "
               f"5 and 1, last moving sample scaling={span[-1]['scaling']:.4f} at "
               f"t={span[-1]['t']:.2f}s; settled by t={prof[-1]['t']:.2f}s")
        else:
            fail("R1: no sample caught mid-ramp - this probe cannot discriminate "
                 "(sampling too slow, or the scale change is instantaneous)")

        # ---- R2: SETTLED at scale 1 - what a `moveto alt` lands at ----
        settle(s, out, "r2s", 1.0)
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 3)
        _t, m, c = probe(s, out, "r2")
        r2 = refdist_km(c)
        rep["legs"]["R2_settled_scale1"] = dict(radius_km=r2, sdr_km=sdr_km(m),
                                                pred=pred["settled_scale1_radius_km"])
        if abs(r2 - pred["settled_scale1_radius_km"]) < 1.0:
            ok(f"R2 settled scale 1: `moveto alt {OBS_ALT_M/1000:.0f} km` lands the observer "
               f"{r2:.2f} km from the centre (predicted {pred['settled_scale1_radius_km']:.2f} "
               f"= {R_KM} + {OBS_ALT_M/1000:.0f})")
        else:
            fail(f"R2 settled scale 1: landed {r2:.2f} km, predicted "
                 f"{pred['settled_scale1_radius_km']:.2f}")

        # ---- R3: SETTLED at scale 5 - the LAYER half (M2) ----
        send(s, "flag moon_scaled on", 0.0)
        settle(s, out, "r3s", float(SCALE))
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 3)
        _t, m, c = probe(s, out, "r3")
        r3 = refdist_km(c)
        rep["legs"]["R3_settled_scale5"] = dict(radius_km=r3, sdr_km=sdr_km(m),
                                                pred=pred["settled_scale5_radius_km"])
        if abs(r3 - pred["settled_scale5_radius_km"]) < 2.0:
            ok(f"R3 settled scale 5: the SAME command lands {r3:.2f} km from the centre "
               f"(predicted {pred['settled_scale5_radius_km']:.2f} = {R_KM}x{SCALE} + "
               f"{OBS_ALT_M/1000:.0f}) - a gap of {r3-r2:.2f} km against R2, predicted "
               f"{pred['layer_gap_km']:.2f} = datum x (scale-1)")
        else:
            fail(f"R3 settled scale 5: landed {r3:.2f} km, predicted "
                 f"{pred['settled_scale5_radius_km']:.2f}")

        # ---- R4: MID-RAMP - the TIMING half (M1), b24_select's own delay ----
        # b24_select sends `flag moon_scaled off` (pause 2), `select planet Moon`
        # (pause 0.6), then the moveto; each send also costs up to 0.25 s of recv
        # timeout, so the command lands ~2.9-3.2 s into the ramp.
        send(s, "flag moon_scaled off", 0.0)
        time.sleep(1.6)
        tb, mb, _c = probe(s, out, "r4pre")     # bracket, before
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 0.0)
        ta, ma, ca = probe(s, out, "r4post")    # bracket, after
        r4 = refdist_km(ca)
        lo, hi = sorted((sdr_km(ma), sdr_km(mb)))
        rep["legs"]["R4_mid_ramp"] = dict(radius_km=r4, alt_ref_implied_km=r4 - OBS_ALT_M / 1000.0,
                                          sdr_before_km=sdr_km(mb), sdr_after_km=sdr_km(ma))
        implied = r4 - OBS_ALT_M / 1000.0
        if lo - 1.0 <= implied <= hi + 1.0 and hi - lo > 1.0:
            ok(f"R4 mid-ramp: the observer landed {r4:.2f} km out, i.e. altitude counted "
               f"from {implied:.2f} km - inside the bracket the ramp swept across the "
               f"command [{lo:.2f}, {hi:.2f}] km, and NOT the settled {R_KM} km")
        else:
            fail(f"R4 mid-ramp: implied altitude reference {implied:.2f} km is outside the "
                 f"bracket [{lo:.2f}, {hi:.2f}] km (bracket width {hi-lo:.2f})")

        # ---- R5: the ramp finishes and the observer does NOT follow it ----
        settle(s, out, "r5s", 1.0)
        _t, m5, c5 = probe(s, out, "r5")
        r5 = refdist_km(c5)
        rep["legs"]["R5_after_settle"] = dict(radius_km=r5, sdr_km=sdr_km(m5),
                                              altitude_km=r5 - sdr_km(m5))
        if abs(r5 - r4) < 1.0 and abs(sdr_km(m5) - R_KM) < 1.0:
            ok(f"R5: the ramp completed (scaledDatumRadius {sdr_km(m5):.2f} km) and the "
               f"observer did NOT follow - still {r5:.2f} km out, i.e. "
               f"{r5-sdr_km(m5):.2f} km above the datum where "
               f"{OBS_ALT_M/1000:.0f} km was commanded. Nothing re-converges it.")
        else:
            fail(f"R5: observer moved from {r4:.2f} to {r5:.2f} km, or the scale did not "
                 f"settle (sdr {sdr_km(m5):.2f} km)")

        # ---- R6: the reversible pair a SECOND time, entered from R5's state ----
        send(s, "flag moon_scaled on", 0.0)
        settle(s, out, "r6a", float(SCALE))
        send(s, "flag moon_scaled off", 0.0)
        el, _sm = settle(s, out, "r6b", 1.0)
        send(s, f"moveto lat {OBS_LAT} lon {OBS_LON} alt {OBS_ALT_M} duration 0", 3)
        _t, m6, c6 = probe(s, out, "r6")
        r6 = refdist_km(c6)
        rep["legs"]["R6_second_traverse"] = dict(radius_km=r6, sdr_km=sdr_km(m6),
                                                 settle_s=el, pred=pred["settled_scale1_radius_km"])
        if abs(r6 - pred["settled_scale1_radius_km"]) < 1.0:
            ok(f"R6 second traverse (on->off again, entered from R5's state): a moveto "
               f"issued AFTER a measured settle lands {r6:.2f} km - the predicted "
               f"{pred['settled_scale1_radius_km']:.2f}. The settle took {el:.2f} s of polling.")
        else:
            fail(f"R6 second traverse: landed {r6:.2f} km, predicted "
                 f"{pred['settled_scale1_radius_km']:.2f}")

        # ---- R7: is the OBSERVER's convention the odd one out? ----
        # D21 [vixy 2026-08-22]: "grounded children inherit scaling (to be
        # visually identical to unscaled ...)". A grounded child at orbit_alt
        # 500 km therefore has a MODEL radius of datum+500 at every scale (the
        # physics layer, D21's first clause) and a DRAWN radius that is the
        # UNIFORM dilation of it. The camera's altitude reference is neither:
        # it is scaledDatum + altitude. R7 measures both layers at both scales,
        # so the three conventions are separated by measurement, not argued.
        r7 = {}
        for tag, want in (("scale1", 1.0), ("scale5", float(SCALE))):
            if want != 1.0:
                send(s, "flag moon_scaled on", 0.0)
            settle(s, out, f"r7{tag}s", want)
            _t, m7, _c = probe(s, out, f"r7{tag}")
            rov = (LAST_BODIES.get(ROVER, {}).get("new") or {})
            if not rov:
                fail(f"R7 {tag}: the grounded child {ROVER} is absent from the dump")
                continue
            r7[tag] = dict(scaling=m7["scaling"], sdr_km=sdr_km(m7),
                           visible=rov.get("visible"),
                           model_km=radius_km(LAST_BODIES, ROVER, "ecl"),
                           drawn_km=radius_km(LAST_BODIES, ROVER, "eclDisplay"))
            print(f"  R7 {tag}: scaling={r7[tag]['scaling']:.5f} visible={r7[tag]['visible']} "
                  f"model={r7[tag]['model_km']:.2f} km  drawn={r7[tag]['drawn_km']:.2f} km",
                  flush=True)
        rep["legs"]["R7_layers"] = r7
        if "scale1" in r7 and "scale5" in r7:
            if not (r7["scale1"]["visible"] and r7["scale5"]["visible"]):
                fail("R7: the grounded child was not evaluated in one of the states - "
                     "its cached position may be stale (§5.107), so the layers are not read")
            elif abs(r7["scale5"]["model_km"] - pred["rover_scale1_drawn_km"]) > 2.0:
                fail(f"R7 physics layer: the child's MODEL radius moved with the display "
                     f"scale ({r7['scale1']['model_km']:.2f} -> {r7['scale5']['model_km']:.2f} km) "
                     f"- D21's first clause")
            elif abs(r7["scale5"]["drawn_km"] - pred["rover_if_uniform_dilation_km"]) < 5.0:
                ok(f"R7: the grounded child rides the scale by UNIFORM DILATION "
                   f"(model {r7['scale5']['model_km']:.2f} km unchanged, drawn "
                   f"{r7['scale5']['drawn_km']:.2f} km = predicted "
                   f"{pred['rover_if_uniform_dilation_km']:.2f}), while the OBSERVER "
                   f"lands at {r3:.2f} km = scaledDatum + altitude; uniform dilation of "
                   f"the observer would be {pred['settled_scale1_radius_km']*SCALE:.2f} km. "
                   f"The camera's altitude reference is a THIRD convention.")
            elif abs(r7["scale5"]["drawn_km"] - pred["rover_if_camera_convention_km"]) < 5.0:
                ok(f"R7: the grounded child uses the CAMERA's convention "
                   f"(drawn {r7['scale5']['drawn_km']:.2f} km = scaledDatum + alt) - the "
                   f"two agree and D21's uniform-dilation criterion is what is not met")
            else:
                fail(f"R7: the child's drawn radius {r7['scale5']['drawn_km']:.2f} km matches "
                     f"NEITHER uniform dilation ({pred['rover_if_uniform_dilation_km']:.2f}) "
                     f"nor the camera convention ({pred['rover_if_camera_convention_km']:.2f})")
    finally:
        try:
            send(s, "shutdown action now", 1); s.close(); proc.wait(timeout=40)
        except Exception:
            pass
        if proc.poll() is None:
            proc.kill()

    rep["fails"] = FAILS
    (out / "f43_ramp_result.json").write_text(json.dumps(rep, indent=1, default=str))
    print(f"\n{'F43-RAMP GREEN' if not FAILS else f'{len(FAILS)} FAILURES'} -> "
          f"{out}/f43_ramp_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
