#!/usr/bin/env python3
"""Dual-path projection trace analyzer (INTENT.md 11.14).

Input: JSON-lines file from 'body action dual_dump' (header + one line/body).
Both paths store matrices column-major (translation at r[12..14]); both
translations are the body origin in the observer frame (old: eye_planet =
mat.getTranslation [body.cpp:943]; new: getObservedPosition = mat.getTranslation
[ModularBody.hpp:460]). Orientation CONVENTIONS may differ between paths
(direction of the mapping); a systematic identical R_rel across all bodies is
itself the E3 signature, not noise - measured, never assumed.

Outputs, per body: dEcl (parent-relative position delta -> E1/E6), dPos
(observer-frame position delta), dDist, relative rotation old->new (angle,
axis). Classification: R_rel identical for ALL bodies => observer/eye frame
convention (E3); divergence localized to one subtree => per-hop element error
(E2) at that branch. Missing new-path bodies are listed (INTENT 11.3 class).
"""
import json, math, sys
import numpy as np

def mat3(m):  # column-major 16 -> rows of the 3x3 linear part
    return [[m[0], m[4], m[8]], [m[1], m[5], m[9]], [m[2], m[6], m[10]]]

def mat3_mul_t(a, b):  # a^T * b
    return [[sum(a[k][i]*b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]

def rot_angle_axis(r):
    tr = r[0][0] + r[1][1] + r[2][2]
    c = max(-1.0, min(1.0, (tr - 1) / 2))
    ang = math.acos(c)
    if ang < 1e-9:
        return 0.0, (0, 0, 0)
    ax = (r[2][1]-r[1][2], r[0][2]-r[2][0], r[1][0]-r[0][1])
    n = math.sqrt(sum(v*v for v in ax)) or 1.0
    return ang, tuple(v/n for v in ax)

def sub(a, b): return [x-y for x, y in zip(a, b)]
def norm(v): return math.sqrt(sum(x*x for x in v))

def main(path):
    header, bodies, missing = None, [], []
    for line in open(path):
        line = line.strip()
        if not line: continue
        rec = json.loads(line)
        if rec["type"] == "header":
            header = rec
        elif rec["new"] is None:
            missing.append(rec["name"])
        else:
            bodies.append(rec)
    print(f"jd={header['jd']}\ncamera={json.dumps(header['camera'])}")
    if missing:
        print(f"\nMISSING in new path ({len(missing)}) [INTENT 11.3 class]: {', '.join(sorted(missing))}")
    stale = [r["name"] for r in bodies if not (r["old"].get("visible", True) and r["new"].get("visible", True))]
    fresh = [r for r in bodies if r["old"].get("visible", True) and r["new"].get("visible", True)]
    if stale:
        print(f"\nSTALE (not visible in both paths - eye-state comparison invalid, dEcl still valid) [{len(stale)}]: "
              + ", ".join(sorted(stale)))
    # Rigid decomposition (Kabsch over fresh-body unit directions):
    # the best single rotation old->new = the E3 (common frame) component;
    # per-body residuals after the fit = the E2 (per-hop) component.
    # zero-translation records (anchors) cannot contribute directions
    fresh = [r for r in fresh
             if np.linalg.norm(r["old"]["mat"][12:15]) > 1e-12
             and np.linalg.norm(r["new"]["mat"][12:15]) > 1e-12]
    if len(fresh) >= 2:
        O = np.array([r["old"]["mat"][12:15] for r in fresh], dtype=float)
        N = np.array([r["new"]["mat"][12:15] for r in fresh], dtype=float)
        O /= np.linalg.norm(O, axis=1, keepdims=True)
        N /= np.linalg.norm(N, axis=1, keepdims=True)
        U, _, Vt = np.linalg.svd(O.T @ N)
        d = np.sign(np.linalg.det(Vt.T @ U.T))
        R = Vt.T @ np.diag([1, 1, d]) @ U.T   # maps old dir -> new dir
        ang = math.degrees(math.acos(max(-1, min(1, (np.trace(R) - 1) / 2))))
        ax = np.array([R[2,1]-R[1,2], R[0,2]-R[2,0], R[1,0]-R[0,1]])
        ax = ax / (np.linalg.norm(ax) or 1)
        print(f"\nBest rigid rotation old->new over {len(fresh)} fresh bodies (E3 component): "
              f"{ang:.3f} deg about [{ax[0]:+.3f},{ax[1]:+.3f},{ax[2]:+.3f}]")
        print("Per-body residual after rigid fit (E2 component - the per-hop part):")
        for r_, o_, n_ in zip(fresh, O, N):
            res = math.degrees(math.acos(max(-1, min(1, float((R @ o_) @ n_)))))
            print(f"  {r_['name']:<14}{res:>9.3f} deg")
    rows = []
    for rec in bodies:
        o, n = rec["old"], rec["new"]
        t_o, t_n = o["mat"][12:15], n["mat"][12:15]
        ang, axis = rot_angle_axis(mat3_mul_t(mat3(o["mat"]), mat3(n["mat"])))
        rows.append({
            "name": rec["name"], "parent_old": o["parent"], "parent_new": n["parent"],
            "dEcl": norm(sub(o["ecl"], n["ecl"])),
            "dPos": norm(sub(t_o, t_n)),
            "dDist": n["dist"] - o["dist"],
            "distOld": o["dist"],
            "rotAngleDeg": math.degrees(ang), "rotAxis": axis,
            "dAxisRotDeg": math.degrees(n["axisRot"] - o["axisRot"]),
            "dScreen": norm(sub(o["screen"], n["screen"])),
        })
    rows.sort(key=lambda r: -r["dPos"])
    hdr = f"{'body':<14}{'dEcl':>12}{'dPos':>12}{'dDist':>12}{'rotAng°':>9}  rotAxis{'':<20}{'dScreen':>9}"
    print("\n" + hdr + "\n" + "-" * len(hdr))
    for r in rows:
        ax = "[{:+.2f},{:+.2f},{:+.2f}]".format(*r["rotAxis"])
        print(f"{r['name']:<14}{r['dEcl']:>12.3e}{r['dPos']:>12.3e}{r['dDist']:>12.3e}"
              f"{r['rotAngleDeg']:>9.3f}  {ax:<27}{r['dScreen']:>9.4f}")
        if r["parent_old"] != r["parent_new"]:
            print(f"  ! parent mismatch: old={r['parent_old']} new={r['parent_new']}")
    # E3 signature: identical relative rotation across all bodies.
    if rows:
        angs = [r["rotAngleDeg"] for r in rows]
        spread = max(angs) - min(angs)
        print(f"\nR_rel angle spread over bodies: {spread:.4f} deg "
              f"(near-zero + nonzero common angle => E3 observer-frame convention; "
              f"branch-dependent => E2 per-hop)")
    # E1 signature: parent-relative positions must agree if ephemeris inputs equal.
    bad_ecl = [r["name"] for r in rows if r["dEcl"] > 1e-6 * max(1e-12, r["distOld"])]
    if bad_ecl:
        print(f"dEcl significant (E1/E6 - ephemeris/data input mismatch): {', '.join(bad_ecl)}")

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/dual_trace.json")
