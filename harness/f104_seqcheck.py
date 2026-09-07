#!/usr/bin/env python3
"""F104 -- THE STRUCTURAL CHECK: does doubling the step change the SEQUENCE?

INTENT 5.145 / 11.223(b).  The fix takes two Newton steps where the solvers
took one.  At a pinned clock the mean anomaly handed to a call does not change
between calls, so each call applies the SAME map to the seed and the claim is
exact rather than approximate:

    post-fix state after k calls  ==  pre-fix state after 2k calls, bit for bit.

Everything F104 predicts about the engine rests on it -- it is what turns
11.220(j1)'s measured "9 evaluations -> 1.09669e-05 deg" into a statement about
what FIVE post-fix evaluations will read, without re-measuring.  It is also the
only check that can catch the loop being put in the wrong place: a loop that
enclosed the `if (lastE == 0)` seeding, or that left a Laguerre-Conway branch's
s/c outside itself, would still halve the convergence count and would NOT
reproduce the sequence.

Both harnesses are built by f104_solver.py from their own tree, and each prints
the md5 of the slice it was built from, so a PASS names the two source texts it
compared.

  usage
    ./f104_seqcheck.py <preOutdir> <postOutdir> [--calls 10] [--out FILE]
"""
import argparse
import subprocess
import sys
from pathlib import Path

# one eccentricity per branch of eccentricAnomaly that iterates, including the
# two no corpus body reaches -- a structural claim is not scoped to the corpus
ECCS = [("0.1", "ell<0.2 fixed point"),
        ("0.43737099231635668", "ell0.2-0.9 Newton (Eris's own e)"),
        ("0.95", "ell0.9-1 Laguerre-Conway elliptic"),
        ("1.5", "ell>1 Laguerre-Conway hyperbolic")]
# a converged seed, a badly stale one, and one many revolutions away
OFFSETS = ["3.0", "-8", "32"]


def values(exe, e, calls, off):
    r = subprocess.run([str(exe), "--converge", e, str(calls), off],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise SystemExit("f104_solver failed: %s%s" % (r.stdout, r.stderr))
    slice_md5, steps = None, None
    out = []
    for line in r.stdout.splitlines():
        if line.startswith("# f104_solver"):
            for tok in line.split():
                if tok.startswith("slice="):
                    slice_md5 = tok.split("=", 1)[1]
                if tok.startswith("steps_per_call="):
                    steps = int(tok.split("=", 1)[1])
        elif line.startswith("call"):
            out.append(line.split()[3])          # the %.17g of lastE
    return slice_md5, steps, out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pre")
    ap.add_argument("post")
    ap.add_argument("--calls", type=int, default=10)
    ap.add_argument("--out")
    a = ap.parse_args()
    pre_exe = Path(a.pre) / "f104_solver"
    post_exe = Path(a.post) / "f104_solver"
    lines, fails, n_ok, n_tot = [], 0, 0, 0
    for e, label in ECCS:
        for off in OFFSETS:
            m0, s0, v0 = values(pre_exe, e, 2 * a.calls, off)
            m1, s1, v1 = values(post_exe, e, a.calls, off)
            bad = [(k + 1, v1[k], v0[2 * k + 1]) for k in range(len(v1))
                   if v1[k] != v0[2 * k + 1]]
            n_ok += len(v1) - len(bad)
            n_tot += len(v1)
            fails += len(bad)
            lines.append(
                "  e=%-22s off=%-5s  pre(slice %s, %d step/call) vs "
                "post(slice %s, %d step/call): %d/%d post[k]==pre[2k]%s   # %s"
                % (e, off, m0, s0, m1, s1, len(v1) - len(bad), len(v1),
                   "" if not bad else "  MISMATCH " + repr(bad[:3]), label))
    head = ["=== F104 sequence-identity check (5.145 / 11.223(b))",
            "  pre  = %s" % pre_exe, "  post = %s" % post_exe]
    tail = ["  %s: %d of %d values bit-identical across %d arms"
            % ("PASS" if not fails else "FAIL", n_ok, n_tot, len(ECCS) * len(OFFSETS))]
    txt = "\n".join(head + lines + tail) + "\n"
    print(txt)
    if a.out:
        open(a.out, "w").write(txt)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
