#!/usr/bin/env python3
"""Orientation-convention checker - INTENT 11.34 predictive confirmation.

Gate for the 6.8 implementation (predictive-model method, 11.15): every
magnitude 11.34 derived by arithmetic must be reproduced from a LIVE dump's
transcribed pieces before the code is touched, and the observed matrices must
confirm the transcriptions wherever freshness allows.

Conventions transcribed (rotation parts only; translations are the settled
P1-P4 layer):
  old_observer(b) = R_prec(a_n)...R_prec(a_1) . R_prec(b)
      [getRotEquatorialToVsop87, body.cpp:552-559 - full precessed ancestor
       walk including self; the Sun contributes identity via the parentless
       guard, so planets are ecliptic-referenced]
  old_render(b)   = R_prec(b) . R_variant(parent)
      [computeDraw one-hop right-multiply, body.cpp:1000-1017; variant =
       unprecessed iff !useParentPrecession(b); parentless bodies: R = I]
  new_current(b)  = tilt(b)
      [recursiveUpdate, ModularBody.cpp:150 - own tilt only]
  fix_A(b)        = R(a_n)...R(a_1) . tilt(b)
      [6.8 decision (A): accumulated + precessed at both consumers; ancestors
       skip system-centered nodes (ecl==0 proxy), SELF IS UNCONDITIONAL -
       derived: the mesh is drawn tilted (2(a) honors the Sun's 7.25), and
       the observer stands on the mesh, so the self tilt cannot be skipped
       even at system center. Old is self-consistent on the Sun (I at both
       consumers); new today is contradictory there too (render tilted,
       observer walk skips self) - same class as the Moon gap.]

Predictions being gated (11.34 arithmetic):
  P-a angle(new_current, old_render):  Moon ~23.44, Charon 115.60, Sun 7.25,
      controls 0  [the CURRENT divergence set - closed at 7 bodies]
  P-b angle(fix_A, old_render):        Moon ~0.363 (Earth precession, growing
      ~0.014/yr), Charon ~0, Sun 7.25 (named win), controls 0
  P-c angle(fix_A, old_observer):      0 everywhere EXCEPT Sun-class (7.25 -
      the self-unconditional consequence; old observer is ecliptic there)
  P-d observed cross-checks: old_render must equal rot(old mat) up to the
      view+spin decoration (checked via the P1-style full composition);
      new_current must equal rot(new mat) up to the same for fresh bodies;
      when the reference is the Moon (scene C), angle(rot(C), rot(new_mat))
      of the reference ~23.44 = the live internal contradiction.
"""
import json, math, sys
import numpy as np

def M(a): return np.array(a, dtype=float).reshape(4, 4).T
def rot(m): return np.array(m)[:3, :3]

def ang(Ra, Rb):
    """Angle (deg) between two rotations.

    Near-identity uses the Frobenius small-angle form: acos((tr-1)/2) is
    ill-conditioned at identity and turns float-ulp matrix noise (~1e-8)
    into ~0.014 deg phantom rows (traced 2026-07-17: |Rp - tilt| = 4e-8
    read as 0.0141 deg - the P4 1.4-ulp class wearing an angle unit)."""
    R = Ra @ Rb.T
    f = np.linalg.norm(R - np.eye(3))
    if f < 1e-3:
        return math.degrees(f / math.sqrt(2))
    c = max(-1.0, min(1.0, (np.trace(R) - 1) / 2))
    return math.degrees(math.acos(c))

def load(path):
    header, bodies, hops = None, {}, {}
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        r = json.loads(line)
        if r["type"] == "header":
            header = r
        elif r["type"] == "body":
            bodies[r["name"]] = r
        elif r["type"] == "hops":
            hops[r["name"]] = r["new"]
    return header, bodies, hops

def old_chain(bodies, name):
    """Ancestor names root-last for the OLD path (parent links in the dump)."""
    chain = []
    while name and name in bodies:
        chain.append(name)
        name = bodies[name]["old"]["parent"]
    return chain  # [self, parent, ..., root]

def main(path):
    header, bodies, hops = load(path)
    ref = header["camera"]["reference"]
    C = M(header["camera"]["mat"])
    print(f"jd={header['jd']} reference={ref}")

    # New-path tilt pieces per node, from the hops records (per-node lastJD -
    # current even for invisible branches).
    tilt = {}
    ecl_new = {}
    for chain in hops.values():
        for h in chain:
            tilt[h["name"]] = rot(M(h["tilt"]))
            ecl_new[h["name"]] = np.array(h["ecl"], dtype=float)

    targets = [n for n in ("Moon", "Charon", "Sun", "Mars", "Earth", "Pluto") if n in bodies]
    controls = [n for n in bodies
                if n not in targets and bodies[n]["new"] is not None
                and bodies[n]["old"] is not None  # new-path-only bodies (B24 "old":null sweep) have no old convention to compare
                and bodies[n]["old"]["parent"] in ("Jupiter", "Saturn", "Uranus", "Neptune", "Mars")]

    def conventions(n):
        o = bodies[n]["old"]
        Rp_self = rot(M(o["rotLocalToParent"]))
        # old_observer: full precessed walk, parents left of self
        acc = Rp_self.copy()
        for a in old_chain(bodies, o["parent"]):
            acc = rot(M(bodies[a]["old"]["rotLocalToParent"])) @ acc
        # old_render: one-hop right-multiply, variant per flag
        if o["parent"] in bodies:
            p = bodies[o["parent"]]["old"]
            Rvar = rot(M(p["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                         else p["rotLocalToParent"]))
            old_render = Rp_self @ Rvar
        else:
            old_render = Rp_self
        # new pieces: only bodies covered by a hops chain have exact tilts
        if n not in tilt:
            return acc, old_render, None, None
        new_current = tilt[n]
        # fix_A: ancestors (new chain = hops order), skip ecl==0 nodes
        # (system-centered proxy), self unconditional; stop at system root
        fix = new_current.copy()
        chain = next(c for c in hops.values() if any(h["name"] == n for h in c))
        names = [h["name"] for h in chain]
        for a in names[names.index(n) + 1:]:
            if a in tilt and np.linalg.norm(ecl_new[a]) > 0:
                fix = tilt[a] @ fix
        return acc, old_render, new_current, fix

    print(f"\n{'body':<10} {'P-a cur|old_r':>14} {'P-b fix|old_r':>14} "
          f"{'P-c fix|old_o':>14} {'old_o|old_r':>12}")
    for n in targets:
        old_o, old_r, new_c, fix = conventions(n)
        pa = f"{ang(new_c, old_r):14.4f}" if new_c is not None else f"{'no-hops':>14}"
        pb = f"{ang(fix, old_r):14.4f}" if fix is not None else f"{'':>14}"
        pc = f"{ang(fix, old_o):14.4f}" if fix is not None else f"{'':>14}"
        print(f"{n:<10} {pa} {pb} {pc} {ang(old_o, old_r):12.4f}")

    # Planet-moon divergence spectrum (2026-07-17 finding: the 11.34 "7-body
    # closure" was FALSE - it grepped rot_obliquity keys and missed the
    # pole-RA/DE-derived rotation elements both parse paths compute, so EVERY
    # Mars/Jupiter/Saturn/Uranus/Neptune moon carries a non-identity parent
    # factor). The divergence of the fix vs old render is the COMMUTATOR
    # [R_m, R_P] (old composes wrong-side, 11.34(iii)); it is zero for
    # zero-own-tilt moons - there the fix RESTORES parity the own-tilt-only
    # code lacks today - and large exactly where the moon's own declared tilt
    # is the 15.5/213.7 copy-paste cluster (garbage-class data either way).
    print("\n== planet-moon spectrum (cur = own-only vs old_r; fix = R_P.R_m vs old_r)")
    rows = []
    for n in bodies:
        o = bodies[n]["old"]
        if o is None:  # new-path-only body (B24 "old":null sweep) - no old convention exists
            continue
        p = o["parent"]
        if p not in bodies or p in ("Sun", "") or bodies[p]["old"]["parent"] != "Sun":
            continue
        if n in targets:
            continue
        Rm = rot(M(o["rotLocalToParent"]))
        po = bodies[p]["old"]
        Rvar = rot(M(po["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                     else po["rotLocalToParent"]))
        Rp = rot(M(po["rotLocalToParent"]))
        rows.append((ang(Rp @ Rm, Rm @ Rvar), ang(Rm, Rm @ Rvar), n, p))
    rows.sort(reverse=True)
    for fix, cur, n, p in rows[:8]:
        print(f"  {n:<12} {p:<9} cur={cur:9.4f} fix={fix:9.4f}")
    restored = sum(1 for f, c, _, _ in rows if c > 0.01 and f < 1e-3)
    diverging = sum(1 for f, _, _, _ in rows if f > 1e-3)
    print(f"  {len(rows)} moons: {restored} parity-RESTORED by fix, "
          f"{diverging} named-divergent (commutator class)")

    # P-d: live internal contradiction at the reference (scene C: Moon ref)
    if ref in bodies and bodies[ref]["new"] is not None:
        Rmat = rot(M(bodies[ref]["new"]["mat"]))
        print(f"\n== P-d reference '{ref}': angle(rot(new mat), rot(C)) = "
              f"{ang(Rmat, rot(C)):.4f} deg")
        print("   (current code predicts the accumulated-vs-own gap; after the"
              " fix this must be 0.0000)")

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/dual_trace.json")
