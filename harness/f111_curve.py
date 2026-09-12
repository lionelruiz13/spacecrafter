#!/usr/bin/env python3
"""F111 -- THE PROOF, ASSEMBLED FROM ARMS WHOSE PER-FRAME DATE JUMP MATCHES.
INTENT Sec.5.150 / Sec.11.239.

    ./f111_curve.py <leg_pre> <leg_post> [--out FILE]

WHY THIS EXISTS.  The obvious proof -- "flag on moves, flag off does not" -- is
not available at a time rate high enough to resample every frame, and the
pre-fix leg measured why: drawing 31 orbit lines costs frame rate (144 -> ~15.5
fps, from `evalCount` deltas), so the flag-ON arm advances the clock up to NINE
TIMES further per frame than the flag-OFF arm at the same `timerate`.  The walk
buys ONE solver call per frame (ModularBody.hpp:707) = two Newton steps
(iterative_orbits.hpp:31), so a larger per-frame jump leaves a larger residual
with the flag OFF too.  The flag-OFF arm is a measured FLOOR, not a zero.

TWO COMPARISONS, NEITHER OF WHICH EXTRAPOLATES.

  A.  SAME STAGE, PRE vs POST.  Both arms have the flag ON, both draw the same
      lines, so both run at the same frame rate -- measured, and reported here
      so the claim is checkable.  Everything except the sampler's seed walk is
      held fixed, so the ratio IS the sampler's contribution.

  B.  MATCHED JUMP, flag ON vs flag OFF, within one leg.  An ON arm is paired
      with the OFF arm of ANOTHER stage whose measured dJD is within `--tol` of
      it.  If the sampler contributes nothing, the ON residual sits at the OFF
      residual of the same jump.

An earlier version of this file interpolated the OFF curve in log-log at the ON
arm's jump.  It is gone: the OFF arms span 0.26-6.0 d/frame and the ON arms
1.3-55, so nearly every point needed EXTRAPOLATION, and a slope taken between
two values near the float floor projects to anything.  Reported as a defect of
this instrument rather than shipped with a clamp (Sec.11.229(i3)'s rule: a gate
that can only pass is not a gate, and a reference that can be anything is not
a reference).
"""
import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f111_predict as P               # noqa: E402


def arms(res):
    """(kind, stage, mean dJD, {body: resid}) for every arm that measured one."""
    out = []
    for k, v in res.items():
        if not isinstance(v, dict) or "per_body" not in v:
            continue
        d = v.get("dJD_per_frame") or []
        if not d:
            continue
        kind = "on" if k.endswith("_on") else "off"
        out.append((kind, k, sum(d) / len(d),
                    {b: r["resid"] for b, r in v["per_body"].items()}))
    return sorted(out, key=lambda t: t[2])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("legs", nargs=2)
    ap.add_argument("--tol", type=float, default=1.6,
                    help="max ratio between two arms' dJD for them to be paired")
    ap.add_argument("--floor", type=float, default=5.6e-7,
                    help="the pre-registered PERTURBED threshold, AU")
    ap.add_argument("--out")
    a = ap.parse_args()
    legs = [json.load(open(Path(d) / "f111_rate.json")) for d in a.legs]
    A = [arms(r) for r in legs]
    bodies = [n for n, el, vis, br, e in P.sampling_bodies() if vis > 0]

    L = ["F111 -- THE FIX'S PROOF, FROM ARMS WHOSE PER-FRAME JUMP MATCHES",
         "leg PRE  = %s   binary %s" % (legs[0]["tag"], legs[0]["bin_md5"]),
         "leg POST = %s   binary %s" % (legs[1]["tag"], legs[1]["bin_md5"]),
         "PERTURBED threshold (pre-registered) = %.3g AU" % a.floor, ""]

    # ---- A. same stage, flag ON, PRE vs POST -------------------------------
    L.append("A.  SAME STAGE, FLAG ON, PRE vs POST -- everything but the")
    L.append("    sampler's seed walk held fixed (the frame rate too: both arms")
    L.append("    draw the same 31 lines).  Ratio = the sampler's contribution.")
    L.append("")
    L.append("%-12s %-14s %9s %9s %13s %13s %11s"
             % ("body", "stage", "dJD_pre", "dJD_post", "PRE_on_AU",
                "POST_on_AU", "pre/post"))
    tot = {"improved": 0, "same": 0, "worse": 0}
    best = []
    for kind, k, d, p in A[0]:
        if kind != "on":
            continue
        m = [(dd, pp) for kk, kn, dd, pp in A[1] if kn == k and kk == "on"]
        if not m:
            continue
        d2, p2 = m[0]
        for b in bodies:
            if b not in p or b not in p2:
                continue
            x, y = p[b], p2[b]
            if x < a.floor and y < a.floor:
                continue                      # both at the floor: no claim
            r = x / y if y > 0 else float("inf")
            tot["improved" if r > 2 else ("worse" if r < 0.5 else "same")] += 1
            best.append((b, k, d, d2, x, y, r))
    for r in sorted(best, key=lambda t: -t[6]):
        L.append("%-12s %-14s %9.3f %9.3f %13.5g %13.5g %11.4g" % r)
    L.append("")
    L.append("    rows above the threshold in at least one arm: %d"
             " -- improved (>2x) %d, unchanged %d, worse (<0.5x) %d"
             % (len(best), tot["improved"], tot["same"], tot["worse"]))

    # ---- B. matched jump, ON vs OFF, within one leg ------------------------
    for name, arm_set in (("PRE", A[0]), ("POST", A[1])):
        L.append("")
        L.append("B-%s.  MATCHED JUMP: each flag-ON arm against the flag-OFF arm"
                 % name)
        L.append("       whose measured dJD is within %.2gx of it.  If the"
                 " sampler" % a.tol)
        L.append("       contributes nothing the ratio is ~1.")
        L.append("")
        L.append("%-12s %9s %9s %13s %13s %11s"
                 % ("body", "dJD_on", "dJD_off", "ON_AU", "OFF_AU", "ON/OFF"))
        rows = []
        for kind, k, d, p in arm_set:
            if kind != "on":
                continue
            cand = [(abs(dd / d - 1.0), i) for i, (kk, kn, dd, pp)
                    in enumerate(arm_set)
                    if kk == "off" and d > 0 and dd > 0
                    and 1.0 / a.tol <= dd / d <= a.tol]
            if not cand:
                continue
            doff, poff = arm_set[min(cand)[1]][2], arm_set[min(cand)[1]][3]
            for b in bodies:
                if b not in p or b not in poff:
                    continue
                x, y = p[b], poff[b]
                if x < a.floor and y < a.floor:
                    continue
                rows.append((b, d, doff, x, y, (x / y if y > 0 else float("inf"))))
        for r in sorted(rows, key=lambda t: -t[5]):
            L.append("%-12s %9.3f %9.3f %13.5g %13.5g %11.4g" % r)
        big = sum(1 for r in rows if r[5] > 10)
        L.append("")
        L.append("       %d matched rows, %d with ON/OFF > 10, worst %.4g"
                 % (len(rows), big, max([r[5] for r in rows], default=0.0)))
    txt = "\n".join(L) + "\n"
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
