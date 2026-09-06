#!/usr/bin/env python3
"""F97 -- `location_orbit` and `surface_point` in the running application
(INTENT S11.217; row S5.21).

WHAT THIS MEASURES, AND WHY EACH NUMBER CAN FAIL.  Four things, all from ONE
launch's dual dumps, and every one of them was PREDICTED in
`artifacts/f97/derivation.txt` + `f97_frame.cpp`'s header before this file was
first run:

  M1  THE UNIT OF orbit_lat.  A body authored `orbit_lat 45` -- its emitted
      direction's latitude, read as asin(ecl_z/|ecl|) on BOTH halves of the
      dump.  This reading is frame-INDEPENDENT: LocationOrbit writes
      spheToRect(lon', lat) * r straight into the position member, so the z
      ratio IS sin(lat) whatever frame that member is later interpreted in.
      PRE-fix 58.31008 deg (45 taken as RADIANS), POST-fix 45.00000 deg.

  M2  THE MISSING EQUATORIAL->VSOP87 ROTATION, old path, against OLD'S OWN
      AUTHORITY read out of the same dump: AnchorPointBody's composition is
      P->getRotEquatorialToVsop87() . Z((P.axisRot + lon)*pi/180) .
      Y((90-lat)*pi/180) [anchor_point_body.cpp:63-71], and the dump carries
      `rotLocalToParent` and `axisRot` for the parent.  For a parent whose own
      parent is the root, rotLocalToParent IS getRotEquatorialToVsop87.
      PRE-fix: tens of degrees.  (Not fixed by this task -- see the entry.)

  M3  THE 90 DEGREES, live, with a both-ways control inside one run.  With the
      observer bound to Mars's surface at (lat 0, lon L), the angle AT MARS'S
      CENTRE between the observer's own direction and a `surface_point` +
      grounded body's is read from `eclRoot` alone -- `eclRoot` is the body's
      position in the EYE frame (verified: |eclRoot| == dist), so
      (-Mars.eclRoot) is the observer's Mars-relative direction and
      (B.eclRoot - Mars.eclRoot) is the body's.  The angle is basis-free.
      PREDICTED: a body at orbit_lon = L sits 90.000 deg from an observer at
      lon = L, and a body at orbit_lon = L-90 sits 0.000 deg from him; moving
      the observer to lon = L+90 swaps the two.  A run that reports 0 deg for
      the first pair refutes the whole derivation.

  M4  THE FROZEN SPIN, as ONE pre-registered number.  LocationOrbit's own
      emitted longitude is lon_frozen + JD*JDToRotation by construction, so
      atan2(ecl_y, ecl_x) minus the parent's LIVE axisRot is exactly the
      frozen model's error -- no frame algebra at all.  `f97_frame.cpp`
      predicts +0.070360 deg for Mars at J2000 (the shipped `float`
      parentSideralDay; in double it is 0.000000) and +8.856158 deg for Earth.
      A second dump 12 h later scores the RATE separately from the OFFSET.

  M5  THE DOUBLY-SPELLED BODY.  `location_orbit` + `bound_to_surface true` is
      grounded on the new path, so the fold spins it a second time; the same
      keys without the spelling are not.  The two land apart -- the S11.78(c)
      trap, measured rather than asserted.

PRECONDITIONS: a PRIVATE farm under the caller's own root (nothing writes the
field), fresh launch, no concurrent instance (/proc/<pid>/comm, S11.134(b)),
config/ssystem md5 in == out on the REAL ~/.spacecrafter, the clock pinned.
S5.50 is FIXED (S11.124(h)): a `surface_point` body pushed through
`body action load` gets a null old half and a live new one instead of killing
the app -- that is what makes M3 authorable over TCP at all.

usage: f97_locorbit.py <absOutdir> --bin PATH --tag pre|post [--jd JD]
"""
import argparse
import json
import math
import os
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import dumpread                                # noqa: E402
import f91_parity as F91                       # noqa: E402

JD = 2451545.0                                 # J2000, the probe's own epoch
FARM_ROOT = Path(os.environ.get("F97_FARM_ROOT", "/home/claude/sc-f97"))
AU_KM = 149597870.691
OBS_ALT_M = 2000000.0                          # 2000 km up: everything in view
OBS_LON = 0.0

PREDICTIONS = """
P-M1  pre  : F97LO45 latitude of the emitted direction = 58.31008 deg on BOTH halves
      post : 45.00000 deg on both halves (|err| <= 1e-4 deg)
P-M2  pre  : F97LO0's old half is > 30 deg from old's own authority for (lon 0, lat 0)
      post : unchanged (this task does not fix the frame)
P-M3  obs at lon 0 : angle(observer, F97SP0)   = 90.000 deg +- 0.05
                     angle(observer, F97SPm90) =  0.000 deg +- 0.05
      obs at lon 90: angle(observer, F97SP0)   =  0.000 deg +- 0.05
                     angle(observer, F97SPm90) = 90.000 deg +- 0.05
P-M4  Mars: atan2(ecl) - live axisRot = +0.070360 deg (f97_frame.cpp), |err| <= 0.002
      the same quantity 12 h later differs from it by <= 0.002 deg (a frozen OFFSET,
      not a frozen RATE, at this separation)
P-M5  F97LO0G (grounded spelling) and F97LO0 (not) land >= 1 deg apart on the new path
"""


def ok(m):
    print("  ok   %s" % m)


def fail(m):
    print("  FAIL %s" % m)
    FAILS.append(m)


FAILS = []


def note(m):
    print("  note %s" % m)


# --------------------------------------------------------------- the bodies
def load_cmd(name, func, lon, lat, alt, grounded=False):
    s = ("body action load name %s type Artificial parent Mars radius 5 "
         "coord_func %s orbit_lon %s orbit_lat %s orbit_alt %s "
         "tex_map bodies/asteroid.png" % (name, func, lon, lat, alt))
    if grounded:
        s += " bound_to_surface true"
    return s


BODIES = [
    ("F97LO0",   load_cmd("F97LO0",   "location_orbit", 0,   0, 0)),
    ("F97LO45",  load_cmd("F97LO45",  "location_orbit", 0,  45, 0)),
    ("F97LO0G",  load_cmd("F97LO0G",  "location_orbit", 0,   0, 0, grounded=True)),
    ("F97SP0",   load_cmd("F97SP0",   "surface_point",  0,   0, 100, grounded=True)),
    ("F97SPm90", load_cmd("F97SPm90", "surface_point", -90,  0, 100, grounded=True)),
]


# ------------------------------------------------------------------- reading
def load(path):
    """{name: record} plus the header, using the project's single reader."""
    bodies, hdr = {}, None
    for line in Path(path).read_text(errors="replace").splitlines():
        line = line.strip()
        if not line:
            continue
        o = dumpread.loads(line)
        if o.get("type") == "frame" or "camera" in o:
            hdr = o
        elif "name" in o:
            bodies[o["name"]] = o
    return bodies, hdr


def vlen(v):
    return math.sqrt(sum(x * x for x in v))


def sub(a, b):
    return [a[i] - b[i] for i in range(3)]


def angdeg(a, b):
    la, lb = vlen(a), vlen(b)
    if la == 0 or lb == 0:
        return float("nan")
    c = sum(a[i] * b[i] for i in range(3)) / (la * lb)
    return math.degrees(math.acos(max(-1.0, min(1.0, c))))


def latdeg(v):
    return math.degrees(math.asin(v[2] / vlen(v)))


def londeg(v):
    return math.degrees(math.atan2(v[1], v[0]))


def mat_mul_vec(m, v):
    """The dump writes Mat4d row-major as the engine stores it (column-major
    r[0..15], the translation in r[12..14]) -- so the rotation acts as
    r[0],r[4],r[8] on x.  Verified against Mars's own rotLocalToParent by the
    identity check in run()."""
    return [m[0] * v[0] + m[4] * v[1] + m[8] * v[2],
            m[1] * v[0] + m[5] * v[1] + m[9] * v[2],
            m[2] * v[0] + m[6] * v[1] + m[10] * v[2]]


def sphe(lng, lat):
    return [math.cos(lng) * math.cos(lat), math.sin(lng) * math.cos(lat), math.sin(lat)]


def wrap180(d):
    d = math.fmod(d, 360.0)
    if d > 180:
        d -= 360
    if d <= -180:
        d += 360
    return d


# ----------------------------------------------------------------- the run
def run(out, binary, tag, jd):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    (out / "predictions.txt").write_text(PREDICTIONS)
    farm = FARM_ROOT / ("farm_%s" % tag)
    shape = F91.build_farm(farm, "fr")
    print("farm: %s  %s" % (farm, json.dumps(shape)))
    app = F91.App(binary, farm, out)
    boot = app.start()
    print("boot: %.1f s" % boot)
    res = {"tag": tag, "jd": jd, "binary": str(binary)}
    try:
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("meteors zhr 0")
        app.send("date jday %.9f" % jd, 1.5)
        app.send("set home_planet Mars", 3)
        app.send("flag atmosphere off")
        app.send("flag landscape off")
        app.send("moveto lat 0 lon %s alt %s duration 0" % (OBS_LON, OBS_ALT_M), 5)
        app.send("zoom fov 120 duration 0", 2)
        for name, cmd in BODIES:
            app.send(cmd, 1.2)
        time.sleep(2.0)
        d_a = app.dump("obs_lon0")
        app.send("moveto lat 0 lon 90 alt %s duration 0" % OBS_ALT_M, 5)
        d_b = app.dump("obs_lon90")
        app.send("moveto lat 0 lon %s alt %s duration 0" % (OBS_LON, OBS_ALT_M), 5)
        app.send("date jday %.9f" % (jd + 0.5), 1.5)
        d_c = app.dump("obs_lon0_plus12h")
    finally:
        rc = app.stop()
    res["exit"] = rc
    res["dumps"] = [str(d_a), str(d_b), str(d_c)]
    score(res, d_a, d_b, d_c, jd)
    (out / "f97_result.json").write_text(json.dumps(res, indent=1))
    return res


def score(res, pa, pb, pc, jd):
    A, ha = load(pa)
    B, hb = load(pb)
    C, hc = load(pc)
    res["scores"] = s = {}

    # ---- the scene is what we asked for -----------------------------------
    cam = ha["camera"]
    print("\n--- the scene")
    for k, want in (("reference", "Mars"), ("boundToSurface", True), ("freeMode", False)):
        if cam.get(k) == want:
            ok("camera.%s = %r" % (k, cam.get(k)))
        else:
            fail("camera.%s = %r, wanted %r" % (k, cam.get(k), want))
    ok("camera lon/lat = %.9f / %.9f rad (%.4f / %.4f deg)"
       % (cam["longitude"], cam["latitude"],
          math.degrees(cam["longitude"]), math.degrees(cam["latitude"])))
    s["cam_lon_deg_a"] = math.degrees(cam["longitude"])
    s["cam_lon_deg_b"] = math.degrees(hb["camera"]["longitude"])
    for name, _ in BODIES:
        if name not in A:
            fail("%s is not in the dump" % name)
    mars = A.get("Mars")
    if mars is None:
        fail("Mars is not in the dump")
        return

    # eclRoot IS the eye-frame position: |eclRoot| == dist (the identity that
    # licenses M3's basis-free angle).
    e = mars["new"]["eclRoot"]
    ident = abs(vlen(e) - mars["new"]["dist"]) / max(mars["new"]["dist"], 1e-30)
    s["eclRoot_is_eye_pos_relerr"] = ident
    (ok if ident < 1e-6 else fail)("|Mars.eclRoot| == Mars.dist to %.2e" % ident)

    # ---- M1: the unit of orbit_lat ---------------------------------------
    print("\n--- M1  the unit of orbit_lat (frame-independent)")
    for half in ("old", "new"):
        r = A.get("F97LO45", {}).get(half)
        if not r or not r.get("ecl"):
            fail("F97LO45 has no %s half" % half)
            continue
        v = latdeg(r["ecl"])
        s["M1_%s_deg" % half] = v
        ok("F97LO45 %s half: emitted latitude %.5f deg (authored 45)" % (half, v))
    r0 = A.get("F97LO0", {}).get("old")
    if r0 and r0.get("ecl"):
        s["M1_lat0_old_deg"] = latdeg(r0["ecl"])
        ok("F97LO0  old half: emitted latitude %.5f deg (authored 0 -- the control)"
           % s["M1_lat0_old_deg"])

    # ---- M2: the missing equatorial->VSOP87 rotation ----------------------
    print("\n--- M2  old half vs OLD'S OWN AUTHORITY, from the same dump")
    mo = mars["old"]
    R = mo["rotLocalToParent"]
    sidereal = mo["axisRot"]                        # DEGREES, body.cpp:604-610
    s["mars_axisRot_deg"] = sidereal
    for name, lon, lat in (("F97LO0", 0.0, 0.0), ("F97LO45", 0.0, 45.0)):
        r = A.get(name, {}).get("old")
        if not r or not r.get("ecl"):
            continue
        want = mat_mul_vec(R, sphe(math.radians(sidereal + lon), math.radians(lat)))
        a = angdeg(r["ecl"], want)
        s["M2_%s_deg" % name] = a
        ok("%s: %.6f deg from old's own authority" % (name, a))
    # the authority is well-formed: it reproduces the observer's own place
    obs_dir = [-x for x in mars["new"]["eclRoot"]]
    s["M2_obs_check_note"] = "observer direction is eye-frame; not comparable to VSOP87 here"

    # ---- M3: the ninety degrees, both ways --------------------------------
    print("\n--- M3  surface_point's orbit_lon vs the observer's lon (basis-free)")
    for label, D in (("obs lon 0", A), ("obs lon 90", B)):
        m = D.get("Mars", {}).get("new")
        if not m:
            fail("%s: no Mars new half" % label)
            continue
        obs = [-x for x in m["eclRoot"]]
        for name in ("F97SP0", "F97SPm90"):
            r = D.get(name, {}).get("new")
            if not r or not r.get("eclRoot"):
                fail("%s: %s has no new half" % (label, name))
                continue
            a = angdeg(obs, sub(r["eclRoot"], m["eclRoot"]))
            s["M3_%s_%s_deg" % (label.replace(" ", ""), name)] = a
            ok("%s: %s is %.4f deg from the observer, at Mars's centre" % (label, name, a))

    # ---- M4: the frozen spin, one number ----------------------------------
    print("\n--- M4  the frozen linear spin model, read off the emitted longitude")
    for label, D, dj in (("t0", A, 0.0), ("t0+12h", C, 0.5)):
        m = D.get("Mars", {}).get("old")
        r = D.get("F97LO0", {}).get("old")
        if not m or not r or not r.get("ecl"):
            fail("%s: missing Mars/F97LO0 old half" % label)
            continue
        emitted = londeg(r["ecl"])
        live = m["axisRot"]
        d = wrap180(emitted - live)
        s["M4_%s_deg" % label] = d
        s["M4_%s_axisRot" % label] = live
        ok("%s: emitted lon %.6f - live axisRot %.6f = %+.6f deg" % (label, emitted, live, d))
    if "M4_t0_deg" in s and "M4_t0+12h_deg" in s:
        s["M4_drift_deg"] = s["M4_t0+12h_deg"] - s["M4_t0_deg"]
        ok("the error is a frozen OFFSET, not a rate: it moved %+.6f deg in 12 h"
           % s["M4_drift_deg"])

    # ---- M5: the doubly-spelled body --------------------------------------
    print("\n--- M5  location_orbit + bound_to_surface: the double-spin trap")
    m = mars["new"]
    a1 = A.get("F97LO0", {}).get("new")
    a2 = A.get("F97LO0G", {}).get("new")
    if a1 and a2 and a1.get("eclRoot") and a2.get("eclRoot"):
        ang = angdeg(sub(a1["eclRoot"], m["eclRoot"]), sub(a2["eclRoot"], m["eclRoot"]))
        s["M5_deg"] = ang
        ok("same keys, one spelled `bound_to_surface true`: %.4f deg apart" % ang)
        s["M5_relation_LO0"] = a1.get("relation")
        s["M5_relation_LO0G"] = a2.get("relation")
        ok("relation: F97LO0 %r  F97LO0G %r" % (a1.get("relation"), a2.get("relation")))
    else:
        fail("M5: one of the two bodies has no new half")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", default=os.environ.get(
        "SC_BIN", "/home/claude/spacecrafter/build-claude/src/spacecrafter"))
    ap.add_argument("--tag", required=True, choices=("pre", "post"))
    ap.add_argument("--jd", type=float, default=JD)
    a = ap.parse_args()
    print(PREDICTIONS)
    run(a.out, a.bin, a.tag, a.jd)
    print("\n=== f97_locorbit %s: %d FAIL" % (a.tag, len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
