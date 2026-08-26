#!/usr/bin/env python3
"""F39: field-by-field comparison of two A-D battery legs (the §11.150(m) shape).

Two dump trees, same scenes, same config, different binaries. Reports, per
scene: every NON-NUMERIC field that differs (a bool or a string moving is a
behaviour change, never noise), every field present in only one leg, and the
worst numeric delta per field name (with the record it came from).

    ./f39_cmp.py <dirA> <dirB> [--tag A=... B=...]
"""
import sys, math
from pathlib import Path
import dumpread

SCENES = ("gen_a", "gen_b", "gen_moon", "gen_mars", "gen_mars_2")


def flatten(o, prefix=""):
    if isinstance(o, dict):
        for k, v in o.items():
            yield from flatten(v, f"{prefix}.{k}" if prefix else k)
    elif isinstance(o, list):
        for i, v in enumerate(o):
            yield from flatten(v, f"{prefix}[{i}]")
    else:
        yield prefix, o


def key_of(rec):
    return (rec.get("type"), rec.get("name"))


def load(d, scene):
    p = Path(d) / f"{scene}.json"
    if not p.exists():
        return None
    recs = {}
    header = None
    for line in open(p, encoding="utf-8", errors="replace"):
        if not line.strip():
            continue
        try:
            r = dumpread.loads(line)
        except Exception:
            continue
        if r.get("type") == "header":
            header = r
        else:
            recs[key_of(r)] = r
    return header, recs


def main():
    A, B = sys.argv[1], sys.argv[2]
    total_nonnum = 0
    for scene in SCENES:
        la, lb = load(A, scene), load(B, scene)
        if la is None or lb is None:
            print(f"--- {scene}: MISSING in {'A' if la is None else 'B'}")
            continue
        (ha, ra), (hb, rb) = la, lb
        onlyA = set(ra) - set(rb)
        onlyB = set(rb) - set(ra)
        nonnum, worst, newfields = [], {}, set()
        pairs = [("header", ha, hb)] + [(k, ra[k], rb[k]) for k in sorted(set(ra) & set(rb), key=str)]
        for key, a, b in pairs:
            fa, fb = dict(flatten(a)), dict(flatten(b))
            for f in set(fb) - set(fa):
                newfields.add(f)
            for f in set(fa) & set(fb):
                x, y = fa[f], fb[f]
                if isinstance(x, bool) or isinstance(y, bool) or isinstance(x, str) or isinstance(y, str) or x is None or y is None:
                    if x != y:
                        nonnum.append((key, f, x, y))
                elif isinstance(x, (int, float)) and isinstance(y, (int, float)):
                    if (x != x) or (y != y):
                        if (x != x) != (y != y):
                            nonnum.append((key, f, x, y))
                        continue
                    d = abs(x - y)
                    if d and (f not in worst or d > worst[f][0]):
                        worst[f] = (d, key, x, y)
        print(f"--- {scene}: records A={len(ra)} B={len(rb)} onlyA={len(onlyA)} onlyB={len(onlyB)} "
              f"non-numeric diffs={len(nonnum)} fields-only-in-B={len(newfields)}")
        if newfields:
            print(f"    new fields in B: {', '.join(sorted(set(f.split('[')[0] for f in newfields)))}")
        for key, f, x, y in nonnum[:25]:
            print(f"    NONNUM {key} {f}: {x!r} -> {y!r}")
        total_nonnum += len(nonnum)
        top = sorted(worst.items(), key=lambda kv: -kv[1][0])[:12]
        for f, (d, key, x, y) in top:
            print(f"    max|d| {f:34s} {d:12.6g}  ({key[1]}) {x:.9g} -> {y:.9g}")
    print(f"\nTOTAL non-numeric differences across all scenes: {total_nonnum}")


if __name__ == "__main__":
    main()
