#!/usr/bin/env python3
"""F38 analyzer — D15(c) sky-lock write-site mirroring, both ways.

Reads the dumps + screenshots f38_mirror.py produced and reports, per scene:

  FLAGS at the transition   old = oldView.nav.flagLockEquPos (the OLD path's own
                            flag, unmediated), new = camera.skyLocked (the path
                            that DRAWS), reported = control.skyLock.reported
                            (B33's readout), plus the tracking pair
                            (oldView.nav.flagTraking / camera.tracked).
  SERVO pair (t0 -> t1, sidereal advance ~18.05 deg)
      new_equ  = orientation change of the new-path view matrix (body->eye)
      old_equ  = orientation change of old helioToEye (helio->eye)
      new_altaz= angular change of Camera (alt,az)
      old_lv   = angular change of old local_vision
      px>32    = composed-screen difference between the two frames

HELD (lock engaged)  : *_equ ~ 0, altaz/lv ~ 18, screen ~ frozen
RELEASED (lock off)  : *_equ ~ 18, altaz/lv ~ 0, screen moves

The verdict a scene must satisfy is the AGREEMENT of the two paths' characters
(both held, or both released) — a flag readback is explicitly not the criterion
(§11.149(a3): the acceptance is that the drawn view behaves like old's).

Usage: f38_analyze.py <dir>
"""
import sys, json, os
import numpy as np
from PIL import Image

D = sys.argv[1]
HELD, RELEASED = 1.0, 10.0   # deg thresholds; the observable is ~18.05 or ~0


def header(name):
    p = os.path.join(D, f"{name}.json")
    with open(p) as f:
        return json.loads(f.readline())


def rot3(m):
    return np.array([[m[0], m[4], m[8]], [m[1], m[5], m[9]], [m[2], m[6], m[10]]])


def rot_angle(R0, R1):
    c = (np.trace(R0.T @ R1) - 1.0) / 2.0
    return float(np.degrees(np.arccos(max(-1.0, min(1.0, c)))))


def altaz_dir(alt, az):
    ca = np.cos(alt)
    return np.array([np.cos(az) * ca, -np.sin(az) * ca, -np.sin(alt)])


def vec_angle(u, v):
    c = np.dot(u, v) / (np.linalg.norm(u) * np.linalg.norm(v))
    return float(np.degrees(np.arccos(max(-1.0, min(1.0, c)))))


def px(a, b):
    d = np.abs(a - b)
    return int((d > 32).any(axis=2).sum()), int(d.max())


def img(name):
    return np.asarray(Image.open(os.path.join(D, f"{name}.png")).convert("RGB"),
                      dtype=np.int16)


def flags(name):
    h = header(name)
    nav = h["oldView"]["nav"]
    ctl = h["control"]["skyLock"]
    c = h["camera"]
    return dict(old=int(nav["flagLockEquPos"]), new=bool(c["skyLocked"]),
                reported=bool(ctl["reported"]), ctl_old=bool(ctl["old"]),
                ctl_new=bool(ctl["new"]), trk_old=int(nav["flagTraking"]),
                trk_new=c["tracked"], sel=c["selected"], jd=h["jd"])


def show_flags(tag, name):
    try:
        f = flags(name)
    except FileNotFoundError:
        print(f"  {tag:<22} (dump missing: {name})")
        return None
    agree = "AGREE" if (bool(f["old"]) == f["new"]) else "DESYNC"
    print(f"  {tag:<22} lock old={f['old']} new={str(f['new']):<5} "
          f"reported={str(f['reported']):<5} [{agree}]   "
          f"track old={f['trk_old']} new='{f['trk_new']}'  sel='{f['sel']}'")
    return f


def pair(tag, n0, n1):
    try:
        h0, h1 = header(n0), header(n1)
    except FileNotFoundError:
        print(f"  {tag:<22} (pair missing: {n0}/{n1})")
        return None
    c0, c1 = h0["camera"], h1["camera"]
    r = dict(
        altaz=vec_angle(altaz_dir(c0["alt"], c0["az"]), altaz_dir(c1["alt"], c1["az"])),
        new_equ=rot_angle(rot3(c0["mat"]), rot3(c1["mat"])),
        old_equ=rot_angle(rot3(h0["helioToEye"]), rot3(h1["helioToEye"])),
        old_lv=vec_angle(np.array(h0["oldLocalVision"]), np.array(h1["oldLocalVision"])),
        djd=h1["jd"] - h0["jd"])
    try:
        r["px"], r["mx"] = px(img(n0), img(n1))
        scr = f"px>32={r['px']:7d}"
    except FileNotFoundError:
        scr = "no shots"
    r["new_state"] = ("held" if r["new_equ"] < HELD else
                      "released" if r["new_equ"] > RELEASED else "?")
    r["old_state"] = ("held" if r["old_equ"] < HELD else
                      "released" if r["old_equ"] > RELEASED else "?")
    ok = "OK" if r["new_state"] == r["old_state"] != "?" else "MISMATCH"
    print(f"  {tag:<22} new_equ={r['new_equ']:8.4f}({r['new_state']:>8})  "
          f"old_equ={r['old_equ']:8.4f}({r['old_state']:>8})  "
          f"altaz={r['altaz']:7.4f}  old_lv={r['old_lv']:7.4f}  {scr}  [{ok}]")
    return r


print(f"=== F38 sky-lock mirror analysis === ({D})")
try:
    p32, mx = px(img("ctrl_a"), img("ctrl_b"))
    print(f"  screen noise floor (ctrl_a/ctrl_b): px>32={p32} max|d|={mx}")
except FileNotFoundError:
    print("  ctrl screenshots missing")

verdicts = {}

for rnd in (1, 2):
    t = f"s1r{rnd}"
    print(f"\n-- S1 round {rnd}: DISABLE @ core.cpp:1410 (`zoom auto initial`) --")
    show_flags("locked (before)", f"{t}_locked")
    h = pair("held pair", f"{t}_held_t0", f"{t}_held_t1")
    f = show_flags("after unzoom-to-init", f"{t}_after")
    r = pair("released pair", f"{t}_rel_t0", f"{t}_rel_t1")
    verdicts[f"S1r{rnd}"] = (f and bool(f["old"]) == f["new"] and not f["new"]
                             and h and h["new_state"] == h["old_state"] == "held"
                             and r and r["new_state"] == r["old_state"] == "released")

def precondition_tracking(name):
    """VACUITY GUARD (the §5.99 lesson: a criterion that cannot report the
    absence of its own precondition is not a criterion). The ENABLE sites only
    fire when tracking is ON when the select runs — and an unknown flag
    spelling is a SILENT no-op on this command surface, which would leave both
    paths agreeing at 0 and read as a pass. So the precondition is asserted
    positively, on BOTH paths, before the scene's verdict counts."""
    f = flags(name)
    ok = f["trk_old"] == 1 and f["trk_new"] != ""
    print(f"    precondition tracking-on: old={f['trk_old']} new='{f['trk_new']}' "
          f"-> {'OK' if ok else 'VACUOUS (scene did not exercise the site)'}")
    return ok


for rnd in (1, 2):
    t = f"s2r{rnd}"
    print(f"\n-- S2 round {rnd}: ENABLE @ core.cpp:2313 (select while tracking) --")
    show_flags("tracking", f"{t}_tracking")
    pre = precondition_tracking(f"{t}_tracking")
    f = show_flags("after select", f"{t}_after")
    r = pair("hold pair", f"{t}_t0", f"{t}_t1")
    verdicts[f"S2r{rnd}"] = (pre and f and bool(f["old"]) == f["new"] and f["new"]
                             and r and r["new_state"] == r["old_state"] == "held")

print("\n-- S3: ENABLE @ core.cpp:1083 (re-select the tracked body) --")
show_flags("tracking", "s3_tracking")
pre3 = precondition_tracking("s3_tracking")
f = show_flags("after re-select", "s3_after")
r = pair("hold pair", "s3_t0", "s3_t1")
verdicts["S3"] = (pre3 and f and bool(f["old"]) == f["new"] and f["new"]
                  and f["trk_new"] == "" and f["trk_old"] == 0
                  and r and r["new_state"] == r["old_state"] == "held")

print("\n-- S4: DISABLE @ core.cpp:1372 (`zoom auto out manual`) --")
show_flags("locked (before)", "s4_locked")
f = show_flags("after manual unzoom", "s4_after")
r = pair("released pair", "s4_rel_t0", "s4_rel_t1")
verdicts["S4"] = (f and bool(f["old"]) == f["new"] and not f["new"]
                  and r and r["new_state"] == r["old_state"] == "released")

print("\n=== verdicts (post-change expectation: all PASS) ===")
for k, v in verdicts.items():
    print(f"  {k:<6} {'PASS' if v else 'FAIL'}")
print("ALL PASS" if all(verdicts.values()) else "NOT ALL PASS")
