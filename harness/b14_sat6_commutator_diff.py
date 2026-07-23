#!/usr/bin/env python3
"""B14-sat6 commutator A/B (INTENT §11.75(b)): full per-body planet-moon
commutator table for TWO dumps (before vs after the 6-moon pole edit) and the
per-body delta. The discriminator: ONLY the 6 widened Saturn moons (Telesto,
Pandora, Janus, Helene, Epimetheus, Prometheus) may move; every other body's
commutator must be bit-identical (the §11.69(d) "75 others unchanged" pattern,
extended to the whole spectrum). Hyperion stays unwritten => unchanged too.

Reuses b14_analyze.py's commutator construction verbatim (imported), so this is
the same metric, not a re-derivation.

Usage: b14_sat6_commutator_diff.py <before.json> <after.json>
Exit 0 iff exactly the 6 targets moved and all others are ULP-0 identical.
"""
import sys, math
import numpy as np
import b14_analyze as B

TARGETS = {"Telesto","Pandora","Janus","Helene","Epimetheus","Prometheus"}

def spectrum_map(path):
    _, bodies, _ = B.load(path)
    out = {}
    for n in bodies:
        o = bodies[n]["old"]
        if o is None: continue
        p = o["parent"]
        if p not in bodies or p in ("Sun","") or bodies[p]["old"] is None \
           or bodies[p]["old"]["parent"] != "Sun":
            continue
        if n in ("Moon","Charon","Sun","Mars","Earth","Pluto"): continue
        Rm = B.rot(B.M(o["rotLocalToParent"]))
        po = bodies[p]["old"]
        Rvar = B.rot(B.M(po["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                         else po["rotLocalToParent"]))
        Rp = B.rot(B.M(po["rotLocalToParent"]))
        out[n] = (B.ang(Rp@Rm, Rm@Rvar), p)
    return out

def main(before, after):
    a = spectrum_map(before)
    b = spectrum_map(after)
    names = sorted(set(a) | set(b))
    print(f"{'moon':<12}{'parent':<9}{'before':>12}{'after':>12}{'|delta|':>12}  verdict")
    moved, bad = set(), []
    for n in names:
        pa = a.get(n); pb = b.get(n)
        if pa is None or pb is None:
            print(f"{n:<12}{'?':<9}  MISSING in one dump"); bad.append(n); continue
        fa, par = pa; fb, _ = pb
        d = abs(fb - fa)
        expect_move = n in TARGETS
        if d == 0.0:
            verdict = "unchanged"
            if expect_move: verdict = "FAIL(target did not move)"; bad.append(n)
        else:
            moved.add(n)
            verdict = "MOVED" if expect_move else "FAIL(unexpected mover)"
            if not expect_move: bad.append(n)
        # print targets, movers, and a compact tail otherwise
        if expect_move or d != 0.0:
            print(f"{n:<12}{par:<9}{fa:>12.4f}{fb:>12.4f}{d:>12.3e}  {verdict}")
    unchanged = [n for n in names if n not in moved and n in a and n in b]
    print(f"\nmoved: {sorted(moved)}")
    print(f"unchanged (ULP-0 identical): {len(unchanged)} bodies")
    print(f"expected movers (the 6): {sorted(TARGETS)}")
    ok = (moved == TARGETS) and not bad
    print("RESULT:", "OK - exactly the 6 moved, all others bit-identical" if ok else f"FAIL {bad}")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2]))
