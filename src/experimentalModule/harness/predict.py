#!/usr/bin/env python3
"""Predictive-model validation, triplet(+Mars) - INTENT 11.14 successor (2026-07-11).

Contract being validated (Vixy): the predicted matrix must match the observed
matrix in BOTH paths, relative positions must be IDENTICAL (reference = old
path), and any rotation difference must have an exactly named cause.

Models are transcriptions of the code as fixed (flat chain - frame contract in
ModularBody.hpp). Acceptance:
  P1 old-path predictions exact (double eps).
  P2 new-path predictions exact (float eps ~1e-6 relative) for fresh bodies.
  P3 relative geometry (eye->body distances, inter-body angles) old==new
     within float eps of the quantities involved.
  P4 observer position in root frame old==new (topocentric parity).
  P5 per-body rotation differential D = R_new.R_old^T equals the named model:
     D(body) = D_common for Earth/Sun/Mars; D(Moon) = D_common conjugated by
     old's parent-rot accumulation (rot_m . rot_e_unprec^T . rot_m^T), because
     the old path post-multiplies the parent's rotation into satellite
     orientation (two-level composition, body.cpp) while the new path's
     orientation is the body's own tilt only. D_common itself = view-state
     difference (camera alt/az/tracking vs old navigator view) - not a defect,
     both are self-consistent view frames.
"""
import json, math, sys
import numpy as np

def M(a): return np.array(a, dtype=float).reshape(4, 4).T
def T(v):
    m = np.eye(4); m[:3, 3] = v; return m
def rot(m): return m[:3, :3]
def tr(m): return m[:3, 3]

def ang_axis(R):
    c = max(-1.0, min(1.0, (np.trace(R) - 1) / 2))
    a = math.degrees(math.acos(c))
    ax = np.array([R[2,1]-R[1,2], R[0,2]-R[2,0], R[1,0]-R[0,1]])
    n = np.linalg.norm(ax)
    return a, (ax / n if n > 1e-12 else ax)

def res(pred, obs):
    return np.linalg.norm(pred - obs) / max(1.0, np.linalg.norm(obs))

def load(path):
    header, bodies, hops = None, {}, {}
    for line in open(path):
        line = line.strip()
        if not line: continue
        r = json.loads(line)
        if r["type"] == "header": header = r
        elif r["type"] == "body": bodies[r["name"]] = r
        elif r["type"] == "hops": hops[r["name"]] = r["new"]
    return header, bodies, hops

QUAD = ("Earth", "Moon", "Sun", "Mars")

def main(path):
    header, bodies, hops = load(path)
    jd = header["jd"]
    H = M(header["helioToEye"])
    C = M(header["camera"]["mat"])
    ref = header["camera"]["reference"]
    print(f"jd={jd} reference={ref}")
    hopm = {n: {h["name"]: h for h in hops[n]} for n in hops}

    # -- consistency of the hop pair (must be exact inverses after the rework)
    print("\n== up/down mutual inverse per node (|U.D - I|, full 4x4)")
    seen = set()
    for n, chain in hops.items():
        for h in chain:
            if h["name"] in seen: continue
            seen.add(h["name"])
            r_ud = np.linalg.norm(M(h["up"]) @ M(h["down"]) - np.eye(4))
            print(f"  {h['name']:<14} {r_ud:.2e}" + ("" if r_ud < 1e-5 else "  <-- NOT inverse"))

    # -- P1: old model (validated composition, unchanged)
    print("\n== P1 OLD predictions")
    for n in QUAD:
        if n not in bodies: continue
        o = bodies[n]["old"]
        MLP = M(o["matLocalToParent"])
        pname = o["parent"]
        if pname in bodies:
            p = bodies[pname]["old"]
            prot = M(p["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                     else p["rotLocalToParent"])
            pred = H @ T(np.array(p["ecl"])) @ MLP @ prot
        else:
            pred = H @ MLP
        print(f"  {n:<6} mat-residual={res(pred, M(o['mat'])):.2e}")

    # -- P2: new model (fixed flat chain, +ecl = child position in parent frame)
    # reference: mat = C ; flat = C.tilt(ref)^-1
    # descent: frame .= T(+ecl) ; mat(body) = frame.tilt(body)
    # walk-up: flat .= T(-ecl(body)) ; ancestor mat = flat.tilt(ancestor)
    print("\n== P2 NEW predictions (fixed model)")
    ref_chain = hops[ref]
    ref_names = [h["name"] for h in ref_chain]
    # camera frame = the reference's ACCUMULATED equatorial frame
    # (accumulatedBodyToBodyPos): product of inverse tilts up the chain,
    # skipping system-centered nodes (ecl == 0 - their coordinates are
    # ecliptic by definition; matches old getRotEquatorialToVsop87).
    acc = np.eye(4)
    for h in ref_chain[:-1]:
        if np.linalg.norm(np.array(h["ecl"])) > 0:
            acc = acc @ np.linalg.inv(M(h["tilt"]))
    flat0 = C @ acc
    new_pred = {}
    for n in QUAD:
        if n not in bodies or bodies[n]["new"] is None: continue
        w = bodies[n]["new"]
        if n == ref:
            # reference mat = flat . tilt(ref) (== C only when the accumulated
            # frame reduces to the single tilt, i.e. Earth-like references)
            pred = flat0 @ M(hopm[ref][ref]["tilt"])
        else:
            chain = hops[n]
            names = [h["name"] for h in chain]
            common = next(nm for nm in ref_names if nm in names)
            base = flat0.copy()
            for h in ref_chain[:ref_names.index(common)]:
                base = base @ T(-np.array(h["ecl"]))         # flat up-hop
            down = chain[:names.index(common)]
            pred = base
            for h in reversed(down):
                pred = pred @ T(np.array(h["ecl"]))          # flat down-hop
            pred = pred @ M(hopm[n][n]["tilt"])              # own tilt into mat
        new_pred[n] = pred
        obs = M(w["mat"])
        fresh = w["visible"] or n == ref
        tag = "" if fresh else "  (not visible: rot gated, t-only meaningful)"
        tr_res = np.linalg.norm(tr(pred)-tr(obs)) / max(np.linalg.norm(tr(obs)), 1e-12)
        print(f"  {n:<6} mat-residual={res(pred, obs):.2e}  t-residual={tr_res:.2e}{tag}")

    # -- P3: relative geometry (THE promise)
    print("\n== P3 relative geometry old vs new (must be identical)")
    names = [n for n in QUAD if n in bodies and bodies[n]["new"] is not None]
    for i, a in enumerate(names):
        to = np.array(bodies[a]["old"]["mat"][12:15]); tn = np.array(bodies[a]["new"]["mat"][12:15])
        do, dn = np.linalg.norm(to), np.linalg.norm(tn)
        print(f"  |eye->{a:<5}| old={do:.9e} new={dn:.9e} rel-d={(dn-do)/do:+.2e}")
        for b in names[i+1:]:
            uo = np.array(bodies[b]["old"]["mat"][12:15]); un = np.array(bodies[b]["new"]["mat"][12:15])
            ao = math.degrees(math.acos(max(-1, min(1, float(to@uo)/(do*np.linalg.norm(uo))))))
            an = math.degrees(math.acos(max(-1, min(1, float(tn@un)/(dn*np.linalg.norm(un))))))
            print(f"    angle({a},{b}): old={ao:9.4f} new={an:9.4f} d={an-ao:+.2e} deg")

    # -- P4: observer in root frame
    up_all = flat0.copy()
    for h in ref_chain[:-1]:                                  # exclude isolated root's own hop
        up_all = up_all @ T(-np.array(h["ecl"]))
    eye_new = -rot(up_all).T @ tr(up_all)
    eye_old = -rot(H).T @ tr(H)
    d = np.linalg.norm(eye_new - eye_old)
    print(f"\n== P4 eye in root frame: |old-new| = {d:.3e} AU ({d*1.496e8:.2f} km)")

    # -- P5: rotation differential model (anchor-free, generalizes to any reference)
    # D(body) = R_new.R_old^T factorizes as Dc . H . A(body) . H^T with
    #   Dc = rot(flat0).H^T           : the view-state term (camera vs navigator)
    #   A  = tilt_self . oldRoot^T    : orientation surplus in the root frame
    # Named A models: I (parity: tilt==old rot, parent rot = I);
    # satellite: tilt.(rot_self.rot_parent[unprec flag])^T (old accumulates the
    # parent's rotation into satellite orientation); parentless: tilt (old skips
    # rotation elements for parentless bodies - the "solar equator" comment).
    print("\n== P5 rotation differential (view term + named per-body surplus)")
    Dc = rot(flat0) @ rot(H).T
    a, ax = ang_axis(Dc)
    print(f"  view term Dc: {a:8.4f} deg about [{ax[0]:+.3f},{ax[1]:+.3f},{ax[2]:+.3f}]  (camera view state vs old navigator - E3 view layer)")
    for n in names:
        w, o = bodies[n]["new"], bodies[n]["old"]
        if not (w["visible"] or n == ref): continue
        D = rot(M(w["mat"])) @ rot(M(o["mat"])).T
        tilt_self = rot(M(hopm[n][n]["tilt"]))
        cands = {"parity (A=I)": np.eye(3)}
        if o["parent"] in bodies:
            p = bodies[o["parent"]]["old"]
            prot = rot(M(p["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                         else p["rotLocalToParent"]))
            cands["old parent-rot accumulation"] = tilt_self @ (rot(M(o["rotLocalToParent"])) @ prot).T
        else:
            cands["old skips parentless rot"] = tilt_self
        best, dev = None, 1e9
        for tag, A in cands.items():
            d = np.linalg.norm(D - Dc @ rot(H) @ A @ rot(H).T)
            if d < dev: best, dev = tag, d
        print(f"  {n:<6} |D - D_model| = {dev:.2e}  cause: {best}"
              + ("" if dev < 1e-4 else "  <-- UNMODELED rotation component"))

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/dual_trace.json")
