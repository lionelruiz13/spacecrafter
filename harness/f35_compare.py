#!/usr/bin/env python3
"""F35 — the INERTNESS half of §5.81: the distance-0 guard changed EXACTLY one
field and nothing else.

    cd claude/harness && ./f35_compare.py <pre_result.json> <post_result.json>

Leg A of `f35_degenerate.py` records, for each binary, the FULL per-body dump of
two scenes driven identically on a stepped clock (`timerate rate 0` + an explicit
`date jday`): the non-degenerate baseline and the post-`transition_to point`
scene in which one body sits at distance 0. This script diffs the two runs
field by field.

D8 (as-if): a guard on a degenerate input must be invisible off that input. The
discriminating statement is therefore not "the NaN is gone" but "the NaN is gone
AND every other field of every other body is bit-identical". Reported both ways:
the count of fields compared, the count that differ, and the differing ones in
full.
"""

import json, sys
from pathlib import Path

FIELDS = ("parent", "ecl", "mat", "dist", "screen", "axisRot", "lastJD",
          "attitude", "radius", "boundingRadius", "screenSize", "visible")


def norm(v):
    """Exact-string comparison for numbers (the dump is printed at round-trip
    precision, so equal strings == equal floats); NaN compares equal to NaN
    here, which is what makes the PRE run's own self-consistency visible."""
    return json.dumps(v, sort_keys=True)


def diff_scene(a, b, tag, report):
    names = sorted(set(a) | set(b))
    only_a = [n for n in names if n not in b]
    only_b = [n for n in names if n not in a]
    n_fields = n_diff = 0
    for n in names:
        if n not in a or n not in b:
            continue
        ka, kb = a[n], b[n]
        for f in sorted(set(ka) | set(kb)):
            if f in ("type", "name"):
                continue
            n_fields += 1
            if norm(ka.get(f)) != norm(kb.get(f)):
                n_diff += 1
                report.append({"scene": tag, "body": n, "field": f,
                               "pre": ka.get(f), "post": kb.get(f)})
    return {"scene": tag, "bodies_pre": len(a), "bodies_post": len(b),
            "only_pre": only_a, "only_post": only_b,
            "fields_compared": n_fields, "fields_differing": n_diff}


def main():
    pre = json.loads(Path(sys.argv[1]).read_text())
    post = json.loads(Path(sys.argv[2]).read_text())
    report = []
    out = {"pre_binary_md5": pre["binary_md5"], "post_binary_md5": post["binary_md5"],
           "scenes": [
               diff_scene(pre["A_base_bodies"], post["A_base_bodies"], "base", report),
               diff_scene(pre["A_point_bodies"], post["A_point_bodies"], "point", report),
           ],
           "differences": report}
    print(json.dumps(out, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
