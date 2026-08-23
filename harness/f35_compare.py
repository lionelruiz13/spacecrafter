#!/usr/bin/env python3
"""F35 — the INERTNESS half of §5.81: the distance-0 guard changed EXACTLY one
field and nothing else.

    cd claude/harness && ./f35_compare.py <pre_result.json> <post_result.json>

Leg A of `f35_degenerate.py` records, for each binary, the FULL per-body dump of
two scenes driven identically on a stepped clock (`timerate rate 0` + an explicit
`date jday`): the non-degenerate baseline and the post-`transition_to point`
scene in which one body sits at distance 0. This script diffs the two runs
field by field.

THE ANSWER IT GAVE IS THAT IT IS THE WRONG INSTRUMENT, and that is why it stays
in the tree with this note rather than being deleted. It reports **278**
differing fields between the two runs' base scenes, and the control that proves
the instrument rather than the fix is at fault is that the **UNTOUCHED old path**
moves the same way: `old.ecl` on 22 bodies, `old.matLocalToParent` on 22,
`old.mat` on 19, `old.dist` on 10. `evalCount` differed 2015 vs 1991 — the two
launches ran a different number of frames before the date was set — and the
iterative position solvers carry their Newton seed across evaluations (§11.117),
so a body's cached state is a function of its evaluation history. A cross-launch
dump diff therefore cannot decide inertness at field granularity.

D8 (as-if) is instead carried by two WITHIN-launch instruments: `b24_equivalence`
(old-vs-new on 120 bodies, same launch) and `f35_branch.py` (the guard branch
counted 0 times over a non-degenerate scene). Read this script's output as a
census of cross-launch jitter, never as a verdict on a change.
"""

import json, sys
from pathlib import Path

def flatten(o, prefix=""):
    """`dual_dump` nests each path's state under `old`/`new`; the comparison is
    per LEAF field, so a single differing number is reported as that number and
    not as the whole sub-object it sits in."""
    out = {}
    for k, v in o.items():
        if k in ("type", "name"):
            continue
        key = f"{prefix}{k}"
        if isinstance(v, dict):
            out.update(flatten(v, key + "."))
        else:
            out[key] = v
    return out


def norm(v):
    """Exact-string comparison (the dump is printed at round-trip precision, so
    equal strings == equal floats). `sanitize_nonfinite` maps a non-finite to a
    string sentinel, so a NaN reads as itself rather than comparing unequal."""
    return json.dumps(v, sort_keys=True)


def diff_scene(a, b, tag, report):
    names = sorted(set(a) | set(b))
    only_a = [n for n in names if n not in b]
    only_b = [n for n in names if n not in a]
    n_fields = n_diff = 0
    for n in names:
        if n not in a or n not in b:
            continue
        ka, kb = flatten(a[n]), flatten(b[n])
        for f in sorted(set(ka) | set(kb)):
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
