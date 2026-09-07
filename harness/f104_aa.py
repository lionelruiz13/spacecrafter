#!/usr/bin/env python3
"""F104 -- THE A/A FLOOR OF THE `altaz_old` CHANNEL, per body, in ulps.

INTENT 5.145 / 11.215(g) / 11.218(l).  F100's inertness control compares
`mat` translations (Mat4f -- floats) and `altaz_new`; it deliberately does NOT
compare `altaz_old`, because the OLD half is not run-to-run reproducible
(11.215(g): four bodies answered two different doubles for one pinned instant,
separations 1.4e-16 to 9.4e-16 AU, OLD half only).

F104 changes a solver both paths share, so `altaz_old` IS a channel this fix can
reach and the question "did the old half move" has to be asked -- and answered
against the channel's OWN floor rather than against zero.  This module takes
three dumps at the same drive point: two from the SAME binary (the A/A control,
which measures the floor) and one from the other (the A/B comparison).  A body
whose A/B difference sits inside the A/A floor is NOT attributable to the
binary; a body outside it is.

  usage
    ./f104_aa.py <dumpA1> <dumpA2> <dumpB> [--key altaz_old] [--out FILE]
"""
import argparse
import json
import math
import struct
import sys


def load(path):
    op = __import__("gzip").open if str(path).endswith(".gz") else open
    out = {}
    with op(path, "rt", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except ValueError:
                continue
            if rec.get("type") == "body":
                out[rec["name"]] = rec
    return out


def ulps(x, y):
    ix = struct.unpack("<q", struct.pack("<d", x))[0]
    iy = struct.unpack("<q", struct.pack("<d", y))[0]
    return abs(ix - iy)


def diff(a, b):
    """(max abs difference, max ulps) over a pair of angles, or None."""
    if not a or not b or len(a) != len(b):
        return None
    return (max(abs(a[i] - b[i]) for i in range(len(a))),
            max(ulps(a[i], b[i]) for i in range(len(a))))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a1")
    ap.add_argument("a2")
    ap.add_argument("b")
    ap.add_argument("--key", default="altaz_old")
    ap.add_argument("--out")
    args = ap.parse_args()
    A1, A2, B = load(args.a1), load(args.a2), load(args.b)
    names = sorted(set(A1) & set(A2) & set(B))
    lines = ["=== F104 A/A floor vs A/B difference on `%s`" % args.key,
             "  A1 = %s" % args.a1, "  A2 = %s (SAME binary as A1)" % args.a2,
             "  B  = %s" % args.b]
    rows, aa_worst, attributable = [], (0.0, 0, "-"), []
    for n in names:
        aa = diff(A1[n].get(args.key), A2[n].get(args.key))
        ab = diff(A1[n].get(args.key), B[n].get(args.key))
        if aa is None or ab is None:
            continue
        if aa[0] > aa_worst[0]:
            aa_worst = (aa[0], aa[1], n)
        if ab[0] > 0 or aa[0] > 0:
            rows.append((ab[0], aa[0], ab[1], aa[1], n))
    rows.sort(reverse=True)
    lines.append("  bodies with any movement in either comparison: %d of %d"
                 % (len(rows), len(names)))
    lines.append("  %-14s %-12s %-12s %-8s %-8s %s"
                 % ("body", "A/B |d|", "A/A |d|", "A/B ulp", "A/A ulp",
                    "attributable to the binary?"))
    for ab_d, aa_d, ab_u, aa_u, n in rows:
        att = ab_d > aa_d
        if att:
            attributable.append((ab_d, n))
        lines.append("  %-14s %-12.4g %-12.4g %-8d %-8d %s"
                     % (n, ab_d, aa_d, ab_u, aa_u,
                        "YES -- outside the A/A floor" if att else "no"))
    lines.append("  A/A floor, worst body: %s at %.4g rad = %.4g deg (%d ulps)"
                 % (aa_worst[2], aa_worst[0], math.degrees(aa_worst[0]),
                    aa_worst[1]))
    lines.append("  bodies whose A/B difference EXCEEDS their own A/A: %d %s"
                 % (len(attributable),
                    [n for _d, n in sorted(attributable, reverse=True)]))
    txt = "\n".join(lines) + "\n"
    print(txt)
    if args.out:
        open(args.out, "w").write(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
