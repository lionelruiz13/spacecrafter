#!/usr/bin/env python3
"""F100 -- THE INERTNESS CONTROL: what did the fix leave alone, byte for byte?

`f100_identity.py <dumpA> <dumpB>` compares two dual dumps taken at the SAME
drive point on two binaries and reports, split by the freshness-blind partition
(`f100_partition.py`), how many bodies carry a byte-identical new-path `mat`
translation and a byte-identical `altaz_new` pair, plus whether the camera state
is identical.  It is the sibling of 11.216(d3)'s clean claim: byte-identity
belongs to an UN-MOVED view, because the camera state after a tracking
convergence is run-to-run variable at ~3.7e-04 rad on ONE binary (11.216(j4)).

The split is the point.  A fix that only reaches bodies the update walk does not
reach must leave the walked set identical and may move the parked one; reporting
one number for all 120 records would hide exactly that.
"""
import argparse
import json
import sys

import f100_partition as part

CAM_KEYS = ("reference", "tracked", "freeMode", "boundToSurface", "mount",
            "skyLocked", "viewOffset", "viewOffsetTransition", "viewOffsetEff",
            "longitude", "latitude", "distance", "alt", "az", "heading",
            "halfFov")


def load(path):
    hdr, bodies = None, {}
    op = __import__("gzip").open if str(path).endswith(".gz") else open
    with op(path, "rt", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                o = json.loads(line)
            except ValueError:
                continue
            if o.get("type") == "header":
                hdr = o
            elif o.get("type") == "body":
                bodies[o["name"]] = o
    return hdr, bodies


def mat_t(rec):
    m = ((rec or {}).get("new") or {}).get("mat")
    return None if not m or len(m) < 15 else tuple(m[12:15])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("--label", default="A vs B")
    ap.add_argument("--out")
    args = ap.parse_args()
    ha, ba = load(args.a)
    hb, bb = load(args.b)
    pt = part.partition(bb)
    P, I = set(pt["P"]), set(pt["I"])
    lines = ["=== %s" % args.label, "  A = %s" % args.a, "  B = %s" % args.b]
    ca = {k: ha["camera"].get(k) for k in CAM_KEYS}
    cb = {k: hb["camera"].get(k) for k in CAM_KEYS}
    diff = {k: (ca[k], cb[k]) for k in CAM_KEYS if ca[k] != cb[k]}
    lines.append("  camera state identical: %s%s"
                 % (not diff, "" if not diff else "  differing: %s" % diff))
    lines.append("  jd A %r  jd B %r" % (ha.get("jd"), hb.get("jd")))
    groups = {"walked": [n for n in bb if n not in P and n not in I],
              "parked (P)": sorted(P), "unreached (I)": sorted(I)}
    for g, names in groups.items():
        names = [n for n in names if n in ba]
        same_pos = [n for n in names if mat_t(ba[n]) == mat_t(bb[n])]
        both_aa = [n for n in names
                   if ba[n].get("altaz_new") and bb[n].get("altaz_new")]
        same_aa = [n for n in both_aa
                   if ba[n]["altaz_new"] == bb[n]["altaz_new"]]
        moved = sorted(set(names) - set(same_pos))
        lines.append("  %-14s n=%3d  identical mat translation %3d/%3d  "
                     "identical altaz_new %3d/%3d"
                     % (g, len(names), len(same_pos), len(names),
                        len(same_aa), len(both_aa)))
        if moved:
            lines.append("      moved: %s" % ", ".join(moved))
    txt = "\n".join(lines) + "\n"
    if args.out:
        open(args.out, "a").write(txt)
    print(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
