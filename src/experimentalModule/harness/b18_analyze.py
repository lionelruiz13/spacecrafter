#!/usr/bin/env python3
"""B18 analyzer - equatorial-mount sky-lock (old flag_lock_equ_pos).

Reads the dual_dump headers and screenshots the driver produced and reports,
per t0/t1 pair:
  * new alt-az delta   = angular change of Camera (alt,az) direction  [deg]
  * new equ delta      = orientation change of the new-path mat (body->eye) [deg]
  * old equ delta      = orientation change of old helioToEye (helio->eye) [deg]
  * old localVision delta = angular change of old local_vision          [deg]
  * screen px>32       = composed-screen diff of the two frames (px)
  * skyLocked flag as dumped for each frame

Expected (sidereal advance DJD~18.05 deg):
  OFF : new alt-az ~0, new equ ~18, old equ ~18   (default local lock)
  ON  : new alt-az ~18, new equ ~0,  old equ ~0    (sky held in equ frame)

Usage: b18_analyze.py [dir]
"""
import sys, json, os
import numpy as np
from PIL import Image

D = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "artifacts", "b18")


def header(name):
    with open(os.path.join(D, f"{name}.json")) as f:
        return json.loads(f.readline())


def rot3(mat):  # column-major r[j*4+i] -> 3x3 rotational part
    return np.array([[mat[0], mat[4], mat[8]],
                     [mat[1], mat[5], mat[9]],
                     [mat[2], mat[6], mat[10]]])


def rot_angle(R0, R1):
    c = (np.trace(R0.T @ R1) - 1.0) / 2.0
    return float(np.degrees(np.arccos(max(-1.0, min(1.0, c)))))


def altaz_dir(alt, az):
    ca = np.cos(alt)
    return np.array([np.cos(az) * ca, -np.sin(az) * ca, -np.sin(alt)])


def vec_angle(u, v):
    c = np.dot(u, v) / (np.linalg.norm(u) * np.linalg.norm(v))
    return float(np.degrees(np.arccos(max(-1.0, min(1.0, c)))))


def load(name):
    return np.asarray(Image.open(os.path.join(D, f"{name}.png")).convert("RGB"),
                      dtype=np.int16)


def px(a, b):
    d = np.abs(a - b)
    return int((d > 32).any(axis=2).sum()), int(d.max())


def pair(tag, n0, n1):
    h0, h1 = header(n0), header(n1)
    c0, c1 = h0["camera"], h1["camera"]
    daltaz = vec_angle(altaz_dir(c0["alt"], c0["az"]), altaz_dir(c1["alt"], c1["az"]))
    dnew = rot_angle(rot3(c0["mat"]), rot3(c1["mat"]))
    dold = rot_angle(rot3(h0["helioToEye"]), rot3(h1["helioToEye"]))
    dlv = vec_angle(np.array(h0["oldLocalVision"]), np.array(h1["oldLocalVision"]))
    try:
        p32, mx = px(load(n0), load(n1))
        scr = f"px>32={p32:6d} max|d|={mx:3d}"
    except FileNotFoundError:
        scr = "screenshot missing"
    print(f"  {tag:<10} lock[{c0['skyLocked']!s:>5}/{c1['skyLocked']!s:>5}]  "
          f"new_altaz={daltaz:7.4f}  new_equ={dnew:7.4f}  old_equ={dold:7.4f}  "
          f"old_lv={dlv:7.4f}  {scr}")
    return dict(daltaz=daltaz, dnew=dnew, dold=dold, dlv=dlv,
                lock0=c0["skyLocked"], lock1=c1["skyLocked"])


print(f"=== B18 sky-lock analysis === ({D})")
try:
    b = header("bogus")["camera"]["skyLocked"]
    print(f"  bogus-spelling  skyLocked={b}  (must be False -> no-op)")
except FileNotFoundError:
    print("  bogus dump missing")

# noise floor: same state twice
try:
    p32, mx = px(load("ctrl_a"), load("ctrl_b"))
    print(f"  screen noise floor (ctrl_a/ctrl_b): px>32={p32} max|d|={mx}")
except FileNotFoundError:
    print("  ctrl screenshots missing")

print("  -- pairs (t0 vs t1, sidereal advance ~18.05 deg) --")
res = {}
for tag, a, b in [("OFF", "off_t0", "off_t1"),
                  ("ON", "on_t0", "on_t1"),
                  ("OFF2", "off2_t0", "off2_t1"),
                  ("ON2", "on2_t0", "on2_t1"),
                  ("OFF3", "off3_t0", "off3_t1")]:
    res[tag] = pair(tag, a, b)
