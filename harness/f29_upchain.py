#!/usr/bin/env python3
# F29 / INTENT §5.46 - the UP-CHAIN walk publishes a flat frame it never writes.
#
# ---------------------------------------------------------------------------
# THE MECHANISM, re-derived from source at code master-beta @ 5b86be0f
# (this header is the PREDICTION and is committed BEFORE the first run).
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
# P3 - THE RENDER, terminal observable.  The ORBIT module threads a NOTCH
#      through the body centre (OrbitModule.cpp:170-178:
#      center = ecl - (r/10, r/10, 0)), and drawOrbits hands it
#      parentFrame = matLocalToBodyPos . translate(-ecl), so the notch is drawn
#      at matLocalToBodyPos . (-r/10, -r/10, 0) - i.e. AT THE BODY, offset by
#      r*sqrt(2)/10.  For Earth seen from the Moon that is 902 km at 384400 km
#      = 2.35e-3 rad; under FISHEYE at fov 340 (halfFov 2.967 rad) on a 1024-px
#      shot (viewportRadius 512 px) that is 0.41 px.  Therefore:
#        (a) POST-FIX the Earth orbit line passes within ~1 px of Earth's own
#            dumped screen position - the same coincidence a DESCENT body (Mars,
#            the in-frame positive control) shows on BOTH binaries.
#        (b) PRE-FIX it does not: the line is drawn in the frozen Earth-scene
#            frame, and since jd is frozen the drawn geometry is the SAME
#            EYE-SPACE geometry as in the Earth scene (`ecl` cancels exactly in
#            the notch, and the sampled points move by <= the light-travel
#            retardation delta, 1.3 s of Earth motion = 38 km on a 1 AU radius
#            = 2.7e-7 rad << 1 px).  So pre-fix the Earth-orbit pixel mask in
#            the MOON scene equals the mask in the EARTH scene.
#        (c) POST-FIX those two masks differ.
#
# P4 - THE AS-IF CONTROL.  Every downward-path body already had the write, so a
#      scene whose up-chain carries no parented ancestor must be unchanged.
#      Two controls, in increasing strength:
#        C1 the shipped EARTH scene (up-chain = Sun alone, parentless => no
#           consumer reads its frame): screenshots bit-identical pre/post.
#        C2 `set home_planet Solar_System` (reference IS the system centre =>
#           `isNotIsolated` false => the up-chain loop body never executes):
#           bit-identical by construction.
#
# ---------------------------------------------------------------------------
#   ./f29_run.sh [outdir]                 # fresh launch + this driver
#   SC_BIN=<pre-fix binary> ./f29_run.sh <outdir>
# Artifacts: t_<tag>.png / t_<tag>.json per state + f29_report.json.
# f29_compare.py reads two report dirs and evaluates the cross-binary halves.

import socket, time, json, sys, os
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 \
    else os.path.join(HERE, "artifacts", "f29")
os.makedirs(OUT, exist_ok=True)

JD = 2461233.5
FOV = 340.0
LAT, LON, ALT = 48.85, 2.35, 100.0
FADE = 3.0          # fader settle wait (orbit/trail faders ramp in << 1 s)
SUBJECT = "Earth"   # the up-chain ancestor WITH a parent, observer on the Moon
CONTROL = "Mars"    # a descent body in the same frame, same pass, same code

sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)
rep = {"jd": JD, "fov": FOV, "subject": SUBJECT, "control": CONTROL}


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


# ---------------------------------------------------------------- P1 invariant
def invariant(bodies):
    """eclRoot == mat[12:15] exactly, for every body. Returns the violators."""
    bad = {}
    for name, b in bodies.items():
        e = b.get("eclRoot"); m = b.get("mat")
        if e is None or m is None:
            continue
        t = m[12:15]
        if any(float(e[i]) != float(t[i]) for i in range(3)):
            bad[name] = {"eclRoot": e, "matT": t,
                         "delta": [float(t[i]) - float(e[i]) for i in range(3)],
                         "dist": b.get("dist"), "parent": b.get("parent")}
    return bad


# --------------------------------------------------------------- P3 pixel work
def line_mask(off_tag, on_tag, thr=12):
    """Pixels the orbit lines added: |on-off| over a threshold, split by hue."""
    a, b = img(off_tag), img(on_tag)
    d = np.abs(b - a)
    changed = d.max(axis=2) > thr
    # the two lines are pushed as pure red / pure green fragment colors over a
    # black sky, so channel dominance separates them without any geometry
    red = changed & (b[:, :, 0] > b[:, :, 1] + 8)
    green = changed & (b[:, :, 1] > b[:, :, 0] + 8)
    return changed, red, green


def ndc_to_px(sx, sy, w, h, flip):
    x = (sx * 0.5 + 0.5) * w
    y = (1.0 - (sy * 0.5 + 0.5)) * h if flip else (sy * 0.5 + 0.5) * h
    return x, y


def min_dist(mask, sx, sy, flip):
    ys, xs = np.nonzero(mask)
    if len(xs) == 0:
        return None
    h, w = mask.shape
    px, py = ndc_to_px(sx, sy, w, h, flip)
    return float(np.sqrt((xs - px) ** 2 + (ys - py) ** 2).min())


def mask_sig(mask):
    ys, xs = np.nonzero(mask)
    return {"n": int(len(xs)),
            "bbox": [int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max())] if len(xs) else None,
            "cx": float(xs.mean()) if len(xs) else None,
            "cy": float(ys.mean()) if len(xs) else None}


def scene(tag):
    """off shot -> orbits on -> on shot (x2, for the A/A floor) -> dump."""
    send(f"body name {SUBJECT} orbit false", 0.5)
    send(f"body name {CONTROL} orbit false", FADE)
    shot(f"{tag}_off")
    send(f"body name {SUBJECT} orbit true", 0.5)
    send(f"body name {CONTROL} orbit true", FADE)
    shot(f"{tag}_on_a"); shot(f"{tag}_on_b")
    dump(tag)
    _, bodies = load(tag)
    changed, red, green = line_mask(f"{tag}_off", f"{tag}_on_a")
    aa = np.abs(img(f"{tag}_on_a") - img(f"{tag}_on_b")).max(axis=2)
    out = {"aa_floor_px": int((aa > 12).sum()), "aa_max": int(aa.max()),
           "mask_all": mask_sig(changed),
           "mask_subject": mask_sig(red), "mask_control": mask_sig(green),
           "invariant_violations": invariant(bodies)}
    for who, mask in ((SUBJECT, red), (CONTROL, green)):
        b = bodies.get(who, {})
        s = b.get("screen")
        out[who] = {"screen": s, "dist": b.get("dist"), "ecl": b.get("ecl"),
                    "eclRoot": b.get("eclRoot"), "matT": (b.get("mat") or [None]*16)[12:15],
                    "routing": b.get("routing"),
                    "notch_px_flipY": min_dist(mask, s[0], s[1], True) if s else None,
                    "notch_px_noflip": min_dist(mask, s[0], s[1], False) if s else None}
    return out


# ------------------------------------------------------------------- the drive
send("flag experimental_path on", 1.0)
send("timerate rate 0", 1.0)
send(f"date jday {JD}", 1.5)
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
send(f"body name {SUBJECT} color orbit r 1 g 0 b 0", 0.6)
send(f"body name {CONTROL} color orbit r 0 g 1 b 0", 1.5)

rep["scene_E"] = scene("E")                      # observer on Earth  (control C1)
send("set home_planet Moon", 5.0)
rep["scene_M"] = scene("M")                      # observer on the Moon (subject)
send("set home_planet Solar_System", 5.0)
rep["scene_S"] = scene("S")                      # control C2: loop never runs

# P2: the freeze, named. Earth's frame in M vs in E.
eE = rep["scene_E"][SUBJECT]["eclRoot"]
eM = rep["scene_M"][SUBJECT]["eclRoot"]
rep["P2_subject_eclRoot_E"] = eE
rep["P2_subject_eclRoot_M"] = eM
rep["P2_frozen"] = (eE == eM)

# P3(b/c): does the MOON-scene subject mask sit where the EARTH-scene one sat?
mE, mM = rep["scene_E"]["mask_subject"], rep["scene_M"]["mask_subject"]
rep["P3_mask_E_eq_M"] = (mE == mM)

with open(os.path.join(OUT, "f29_report.json"), "w") as fh:
    json.dump(rep, fh, indent=1, sort_keys=True)

print("\n=== F29 single-run report ===")
for s in ("E", "M", "S"):
    d = rep[f"scene_{s}"]
    v = d["invariant_violations"]
    print(f"scene {s}: A/A floor {d['aa_floor_px']} px (max {d['aa_max']}); "
          f"mask subject {d['mask_subject']['n']} px, control {d['mask_control']['n']} px")
    print(f"  P1 violations ({len(v)}): {sorted(v)}")
    for who in (SUBJECT, CONTROL):
        b = d[who]
        print(f"  {who}: screen={b['screen']} notch flipY={b['notch_px_flipY']} "
              f"noflip={b['notch_px_noflip']} routing={b['routing']}")
print(f"P2 subject eclRoot frozen across the switch: {rep['P2_frozen']}")
print(f"   E {eE}\n   M {eM}")
print(f"P3 subject mask identical E vs M: {rep['P3_mask_E_eq_M']}")
print(f"report -> {OUT}/f29_report.json")
