#!/usr/bin/env python3
# F29 / INTENT §5.46 - the UP-CHAIN walk publishes a flat frame it never writes.
#
# ---------------------------------------------------------------------------
# THE MECHANISM, re-derived from source at code master-beta @ 5b86be0f
# (this header is the PREDICTION and was committed BEFORE the first run,
# harness 5df7d17; the only later edit is the P3' paragraph, whose reason is
# a MEASUREMENT stated there and whose substance is unchanged).
# ---------------------------------------------------------------------------
# `ModularBody::dispatchUpdate` (ModularBody.cpp:323-386) walks DOWN from the
# camera reference and then UP to the system centre.  Every DOWNWARD site sets
# the body's flat position frame explicitly:
#   recursiveUpdate            ModularBody.cpp:307   this->matLocalToBodyPos = matLocalToBodyPos
#   transformParentToBodyPos   ModularBody.hpp:711   matLocalToBodyPos = mat_local_to_body
#   selectiveUpdate else       ModularBody.hpp:897   matLocalToBodyPos = mat_local_to_parent
#   recursiveTranslationUpdate ModularBody.hpp:929   matLocalToBodyPos = frame
# The UP-CHAIN loop (ModularBody.cpp:355-384) assigns only
#   ModularBody.cpp:381   body->mat = parentTilted
# with `parentTilted = flat.multiplyFast(parent->accumulatedBodyPosToBody(jd))`
# (ModularBody.cpp:362) - the SAME product recursiveUpdate forms at :308 - and
# never writes `matLocalToBodyPos`, whose member contract (ModularBody.hpp:1927)
# says "Set on EVERY position update (visible or not)".
#
# `flat` at :381 IS the parent's flat position frame: `transformBodyToParent`
# (:356, ModularBody.hpp:829-859) is the exact inverse of the descent hop, so
# after it `flat` has climbed one level.  Hence the fix shape recorded in the
# row - `body->matLocalToBodyPos = flat` beside `body->mat = parentTilted` -
# makes the climb write exactly what the descent writes.
#
# CONSUMERS (whole-`src` enumeration of getMatLocalToBodyPos):
#   ModularSystem.cpp:632  drawOrbits    parentFrame = matLocalToBodyPos . translate(-ecl)
#   ModularSystem.cpp:673  drawTrails    same expression
#   ModularSystem.cpp:707  drawTails     same expression
# all three guarded by `if (!body.getParent()) continue`, so the defect is only
# OBSERVABLE on an up-chain ancestor that HAS a parent - i.e. the reference must
# be at least two levels below the system centre.  On today's corpus:
# observer on the Moon (ssystem.ini: moon.parent = Earth, earth.parent = Sun)
# => Earth is the up-chain ancestor with a parent.
#
# ---------------------------------------------------------------------------
# PREDICTIONS (derived, committed pre-run)
# ---------------------------------------------------------------------------
# P1 - THE MATRIX-LAYER INVARIANT.  `accumulatedBodyPosToBody` is a product of
#      `computeBodyPosToBody` = Mat4f::xzrotation (ModularBody.hpp:717-722,
#      788-801) => a PURE rotation, zero translation.  `multiplyFast`
#      (vecmath.hpp:1927-1939) gives translation(A.B) = A.translation when
#      B.translation == 0.  Therefore, for every body the walk DESCENDS through,
#          eclRoot == mat[12:15]     EXACTLY (same float bits)
#      because both are the translation of the same Mat4f.  Pre-fix this
#      invariant must FAIL for every up-chain ancestor (`mat` refreshed, frame
#      frozen) and HOLD for every other evaluated body.  Post-fix it must hold
#      for EVERY body in the dump.
#      Sharpened: with the observer on Earth (shipped default) the Sun is
#      already an up-chain ancestor, so P1 must fail for the Sun there too -
#      with NO rendered consequence, the Sun being parentless.
#
# P2 - THE FREEZE, named exactly.  With jd frozen and the observer stationary,
#      after `set home_planet Moon` the walk never descends through Earth again
#      (the up-chain sibling loops at ModularBody.cpp:363-374 skip the node the
#      walk came from, and Earth is that node at the second iteration).  So
#      pre-fix Earth's eclRoot in the MOON scene must equal, bit for bit at the
#      dump's precision-9 float printing, its eclRoot in the EARTH scene.
#      Post-fix the two must differ (Earth's eye position from Paris ~6.4e3 km
#      vs from the Moon ~3.8e5 km).
#
# P3 - THE RENDER, terminal observable.  A body's own line - orbit or trail -
#      is drawn in parentFrame = matLocalToBodyPos . translate(-ecl), and both
#      lines PASS THROUGH THE BODY by construction: the ORBIT threads a notch
#      through the centre (OrbitModule.cpp:170-178, center = ecl - (r/10,r/10,0))
#      and the TRAIL's newest point IS the body's current ecl
#      (TrailModule.cpp accumulate: points.insert(begin, {getEclipticPos(),date})).
#      Feeding `ecl` through `translate(-ecl)` cancels it, so that end of the
#      line is drawn at matLocalToBodyPos's TRANSLATION - the body's own eye
#      position for every descent body, and the FROZEN one for an up-chain
#      ancestor.  Therefore:
#        (a) POST-FIX the subject's line ends at the subject's own dumped screen
#            position - the coincidence a DESCENT body (Mars, the in-frame
#            positive control) shows on BOTH binaries;
#        (b) PRE-FIX it ends at the screen position of the FROZEN eclRoot
#            instead, which is a different point;
#        (c) in both cases the line end sits at project(eclRoot) - that IS the
#            mechanism, and it is the same statement on both binaries.
#
# P3' - WHICH CONSUMER CARRIES P3 [measured 2026-08-09, harness artifacts
#      f29/probe]: the ORBIT pass is DEPTH-BUCKETED (Renderer.hpp:156-167:
#      beginOrbitTrace clears + sets the orbit depth range, beginOrbitLines
#      depth-tests the lines against it), so from the Moon's surface - where
#      the Moon reserves the bucket - NO planet orbit line reaches the frame at
#      all: measured 0 px added by `flag planets_orbits on`, by
#      `flag satellites_orbits on` it is 4963 px, and on Earth only Earth's own
#      orbit draws (2317 px) because only its notch falls inside Earth's own
#      slice.  The TRAIL pass is DEPTH-FREE by contract (TrailModule.cpp:67-68
#      depthTest/depthWrite false; Renderer.hpp:168-175 "no depth clear/range").
#      So the RENDER half of P3 is measured on the TRAIL - the consumer §5.46's
#      own row names ("Earth is up-chain and carries a TRAIL module") - and the
#      ORBIT/TAIL passes ride the identical expression, one and two lines away
#      in the same file.  The substance of P3 is unchanged.
#
# P3'' - WHERE THE FREEZE STATE MUST BE PUT, and why [measured 2026-08-09,
#      artifacts f29/pre run 1]: the frozen frame is used VERBATIM as eye
#      coordinates, so the pre-fix line is drawn at a fixed place on screen no
#      matter where the observer then looks.  With the freeze taken from a
#      surface observer on Earth itself, that place is Earth's own centre =
#      the exact NADIR = 180 deg from the zenith view axis, and at fov 340
#      (halfFov 170 deg) it is OFF SCREEN: measured |screen| = 1.0588 > 1 for
#      the subject in that state, so the pre-fix half of P3 had nothing to
#      land on.  The freeze is therefore taken from a leg that CENTRES the
#      subject (`select planet Earth` + `flag track_object on`, from Mars),
#      which makes the prediction SHARPER rather than weaker:
#        PRE-FIX the subject's trail head lands at the FRAME CENTRE
#        (1024, 1024) +- the tracking residual, because a centred subject has
#        eye vector (0, 0, -d) and project((0,0,-d)) = (0,0) = the centre.
#      The general statement head == project(eclRoot) is unchanged and is
#      checked on both binaries.  Nothing else about P3 moves; the pre-fix
#      matrix-layer halves P1/P2 were already CONFIRMED on that first run
#      (violations exactly {Earth, Sun, SolarSystem} in the subject scene and
#      exactly {Sun, SolarSystem} in the others; subject eclRoot bit-identical
#      across the switch).
#
# P4 - THE AS-IF CONTROL.  Every downward-path body already had the write, so a
#      scene whose up-chain carries no parented ancestor must be unchanged.
#      Two controls, in increasing strength:
#        C1 the shipped EARTH scene (up-chain = Sun + SolarSystem; SolarSystem
#           is parentless-for-this-purpose and the Sun draws no orbit/trail on
#           the shipped corpus): screenshots bit-identical pre/post.
#        C2 `set home_planet Solar_System` (reference IS the system centre =>
#           `isNotIsolated` false => the up-chain loop body never executes):
#           bit-identical by construction.
#
# ---------------------------------------------------------------------------
# THE SCENE is DETERMINISTIC BY CONSTRUCTION: the clock is frozen
# (`timerate rate 0`) and the trail is accumulated by N discrete `date jday`
# steps of exactly DeltaTrail = 1.0 sim-day (TrailLoader.cpp:28), which
# TrailModule::accumulate turns into exactly one point per step.  No wall-clock
# term enters the sampled geometry, so two binaries driven by this script
# produce comparable - and on the control scenes bit-identical - frames.
#
#   ./f29_run.sh [outdir]                 # fresh launch + this driver
#   SC_BIN=<pre-fix binary> ./f29_run.sh <outdir>
# Artifacts: t_<tag>.png / t_<tag>.json per state + f29_report.json.
# f29_compare.py reads two report dirs and evaluates the cross-binary halves.

import socket, time, json, sys, os, math
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "f29")
os.makedirs(OUT, exist_ok=True)

JD0 = 2461233.5
FOV = 340.0
LAT, LON, ALT = 48.85, 2.35, 100.0
NSTEP = 30          # trail points, one per 1.0-sim-day `date jday` step
SUBJECT = "Earth"   # the up-chain ancestor WITH a parent, observer on the Moon
CONTROL = "Mars"    # a descent body in the same frame, same pass, same code

sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
rep = {"jd0": JD0, "fov": FOV, "nstep": NSTEP,
       "subject": SUBJECT, "control": CONTROL}
jd = JD0


def send(cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=1.8):
    send(f"body action screenshot filename {OUT}/t_{tag}.png", pause); return tag


def dump(tag, pause=2.2):
    send(f"body action dual_dump filename {OUT}/t_{tag}.json", pause); return tag


def load(tag):
    hdr, out = None, {}
    for line in open(os.path.join(OUT, f"t_{tag}.json")):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "header":
            hdr = d
        elif d.get("type") == "body" and d.get("new"):
            out[d["name"]] = d["new"]
    return hdr, out


def img(tag):
    return np.asarray(Image.open(os.path.join(OUT, f"t_{tag}.png"))
                      .convert("RGB")).astype(np.int32)


def accumulate(n):
    """n discrete 1.0-day date steps == exactly n trail points (DeltaTrail=1)."""
    global jd
    for _ in range(n):
        jd += 1.0
        send(f"date jday {jd}", 0.45)
    time.sleep(1.5)


# ---------------------------------------------------------------- P1 invariant
def invariant(bodies):
    """eclRoot == mat[12:15] exactly, for every body. Returns the violators."""
    bad = {}
    for name, b in bodies.items():
        e, m = b.get("eclRoot"), b.get("mat")
        if e is None or m is None:
            continue
        t = m[12:15]
        if any(float(e[i]) != float(t[i]) for i in range(3)):
            bad[name] = {"eclRoot": e, "matT": t,
                         "delta": [float(t[i]) - float(e[i]) for i in range(3)],
                         "dist": b.get("dist"), "parent": b.get("parent")}
    return bad


# ------------------------------------------- the projection, mapped not assumed
def half_fov_from(bodies):
    """Solve halfFov out of the dump itself: |screen| = acos(-z/d)/halfFov
    (ModularBody::update, the FISHEYE branch). Agreement across bodies is the
    positive map of this reconstruction."""
    vals = []
    for b in bodies.values():
        m, s, d = b.get("mat"), b.get("screen"), b.get("dist")
        if not m or not s or not d:
            continue
        rq = math.hypot(s[0], s[1])
        if rq < 1e-6 or d <= 0:
            continue
        th = math.acos(max(-1.0, min(1.0, -m[14] / d)))
        vals.append(th / rq)
    vals.sort()
    return (vals[len(vals) // 2], vals[0], vals[-1]) if vals else (None, None, None)


def project(v, half_fov):
    """Eye-space vector -> screen NDC, the ModularBody::update FISHEYE form."""
    d = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
    if d == 0:
        return None
    rq = math.hypot(v[0], v[1])
    f = (math.acos(max(-1.0, min(1.0, -v[2] / d))) / (rq * half_fov)
         if rq > d * 1e-5 else 1.0 / (d * half_fov))
    return [v[0] * f, v[1] * f]


def ndc_px(s, w, h, flip):
    return ((s[0] * 0.5 + 0.5) * w,
            (1.0 - (s[1] * 0.5 + 0.5)) * h if flip else (s[1] * 0.5 + 0.5) * h)


def masks(off_tag, on_tag):
    """Isolate each subject's trail by a COLOUR differential, not by hue alone:
    the same frame is shot with the two trails black (invisible against the
    black sky) and then coloured. Recolouring does not touch the recorded
    points (TrailModule::setColor), so the two shots differ ONLY where those
    two polylines are - body discs, which are themselves red-dominant, cancel.
    Hue then says which of the two a changed pixel belongs to."""
    a, b = img(off_tag), img(on_tag)
    changed = np.abs(b - a).max(axis=2) > 12
    red = changed & (b[:, :, 0] > b[:, :, 1] + 8) & (b[:, :, 0] > b[:, :, 2] + 8)
    grn = changed & (b[:, :, 1] > b[:, :, 0] + 8) & (b[:, :, 1] > b[:, :, 2] + 8)
    return b, red, grn


def head_px(a, mask, chan):
    """The trail HEAD is its brightest end (vert shader: alpha falls 1 -> 0.1
    from newest to oldest, TrailModule::draw)."""
    if mask.sum() == 0:
        return None, 0
    v = np.where(mask, a[:, :, chan], -1)
    idx = int(np.argmax(v))
    return (int(idx % v.shape[1]), int(idx // v.shape[1])), int(v.max())


def sig(mask):
    ys, xs = np.nonzero(mask)
    if not len(xs):
        return {"n": 0, "bbox": None}
    return {"n": int(len(xs)),
            "bbox": [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())]}


def scene(tag):
    send(f"body name {SUBJECT} color trail r 0 g 0 b 0", 0.5)
    send(f"body name {CONTROL} color trail r 0 g 0 b 0", 1.5)
    shot(f"{tag}_off")
    send(f"body name {SUBJECT} color trail r 1 g 0 b 0", 0.5)
    send(f"body name {CONTROL} color trail r 0 g 1 b 0", 1.5)
    shot(f"{tag}_a"); shot(f"{tag}_b")
    dump(tag)
    _, bodies = load(tag)
    hf, hf_lo, hf_hi = half_fov_from(bodies)
    a, red, grn = masks(f"{tag}_off", f"{tag}_a")
    aa = np.abs(img(f"{tag}_a") - img(f"{tag}_b")).max(axis=2)
    h, w = red.shape
    out = {"aa_floor_px": int((aa > 12).sum()), "aa_max": int(aa.max()),
           "half_fov": hf, "half_fov_spread": [hf_lo, hf_hi],
           "mask_subject": sig(red), "mask_control": sig(grn),
           "invariant_violations": invariant(bodies), "shape": [w, h]}
    for who, mask, chan in ((SUBJECT, red, 0), (CONTROL, grn, 1)):
        b = bodies.get(who, {})
        m, s, e = b.get("mat"), b.get("screen"), b.get("eclRoot")
        hp, hv = head_px(a, mask, chan)
        pr = {"screen_dumped": s, "eclRoot": e,
              "matT": (m or [None] * 16)[12:15], "dist": b.get("dist"),
              "routing": b.get("routing"), "head_px": hp, "head_val": hv,
              "n_px": int(mask.sum())}
        if s and e and hf:
            pr["proj_matT_ndc"] = project(m[12:15], hf)     # == screen_dumped
            pr["proj_eclRoot_ndc"] = project(e, hf)
            for flip in (True, False):
                key = "flipY" if flip else "noflip"
                pm = ndc_px(pr["proj_matT_ndc"], w, h, flip)
                pe = ndc_px(pr["proj_eclRoot_ndc"], w, h, flip)
                pr[f"px_matT_{key}"] = pm
                pr[f"px_eclRoot_{key}"] = pe
                if hp:
                    pr[f"head_to_matT_{key}"] = math.dist(hp, pm)
                    pr[f"head_to_eclRoot_{key}"] = math.dist(hp, pe)
        out[who] = pr
    return out


# ------------------------------------------------------------------- the drive
send("flag experimental_path on", 1.0)
send("timerate rate 0", 1.0)
send(f"date jday {JD0}", 1.5)
send("flag moon_scaled off", 1.0)     # standing harness rule (§5.27 / D21)
for f in ("landscape", "atmosphere", "fog", "milky_way", "stars", "star_lines",
          "constellation_drawing", "constellation_art",
          "constellation_boundaries", "constellation_names",
          "nebula_hints", "nebula_names", "planets_hints", "planets_labels",
          "object_trails", "planets_orbits", "satellites_orbits",
          "star_names", "zodiacal_light", "atmospheric_refraction"):
    send(f"flag {f} off", 0.35)
send(f"zoom fov {FOV} duration 0", 2.0)
send(f"moveto lat {LAT} lon {LON} alt {ALT} duration 0", 3.0)
# Isolate the two subjects by COLOUR: every other trail is black, i.e. invisible
# against the black sky the flags above leave (BLEND_SRC_ALPHA over 0 = 0).
send("body name all color trail r 0 g 0 b 0", 1.2)
send("flag object_trails on", 2.0)

accumulate(NSTEP)
rep["scene_E"] = scene("E")                      # observer on Earth (control C1)
send("set home_planet Solar_System", 6.0)
accumulate(NSTEP)
rep["scene_S"] = scene("S")                      # control C2: loop never runs
# THE FREEZE SOURCE (P3''): the last frame the walk DESCENDS through the
# subject, with the subject CENTRED so the frozen frame's image of it is on
# screen. Tracking is released before the switch - releasing does not move the
# camera, so the frame that freezes is the centred one.
send("set home_planet Mars", 6.0)
send(f"select planet {SUBJECT} pointer off", 1.5)
send("flag track_object on", 8.0)
accumulate(NSTEP)          # tracking HELD: the subject stays centred while the
                           # world advances 30 days under it
rep["scene_X"] = scene("X")                      # freeze source; subject centred
send("flag track_object off", 2.0)   # releasing does not move the camera, so
                                     # the frame that freezes is the centred one
send("set home_planet Moon", 6.0)
accumulate(NSTEP)
rep["scene_M"] = scene("M")                      # observer on the Moon (subject)

# P2: the freeze, named. The subject's cached frame in the SUBJECT scene must
# equal the one left by the last DESCENT through it - scene X - pre-fix, and
# differ post-fix.
eX = rep["scene_X"][SUBJECT]["eclRoot"]
eM = rep["scene_M"][SUBJECT]["eclRoot"]
rep["P2_subject_eclRoot_X"] = eX
rep["P2_subject_eclRoot_M"] = eM
rep["P2_frozen"] = (eX == eM)
rep["X_subject_centred_ndc"] = rep["scene_X"][SUBJECT]["screen_dumped"]

with open(os.path.join(OUT, "f29_report.json"), "w") as fh:
    json.dump(rep, fh, indent=1, sort_keys=True)

print("\n=== F29 single-run report ===")
for s in ("E", "S", "X", "M"):
    d = rep[f"scene_{s}"]
    print(f"scene {s}: A/A floor {d['aa_floor_px']} px (max {d['aa_max']}); "
          f"halfFov {d['half_fov']} spread {d['half_fov_spread']}")
    print(f"  P1 violations ({len(d['invariant_violations'])}): "
          f"{sorted(d['invariant_violations'])}")
    for who in (SUBJECT, CONTROL):
        b = d[who]
        print(f"  {who}: trail {b['n_px']} px head={b['head_px']} "
              f"head->matT {b.get('head_to_matT_flipY')} / "
              f"head->eclRoot {b.get('head_to_eclRoot_flipY')}  (flipY)")
        print(f"        noflip: head->matT {b.get('head_to_matT_noflip')} / "
              f"head->eclRoot {b.get('head_to_eclRoot_noflip')}")
print(f"X centred subject ndc: {rep['X_subject_centred_ndc']}")
print(f"P2 subject eclRoot frozen across the switch: {rep['P2_frozen']}")
print(f"   X {eX}\n   M {eM}")
print(f"report -> {OUT}/f29_report.json")
