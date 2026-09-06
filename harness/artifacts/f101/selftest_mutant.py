#!/usr/bin/env python3
"""SHOWN ABLE TO FAIL.  The guard's criterion is mutated from `bodies_old == 0`
to `bodies_old < 2` and the SAME self-test is re-run.  If the suite could not
discriminate, this would still print 17 PASS.  It does not: the cases that carry
exactly ONE old-path body go RED - the synthetic one-pair case, F98's own
post-`14.sts` bisect dump (1/277), and F101's p25 (1/121).  That is the evidence
that the threshold is a measured choice and not a number that happens to be in
the file.

    cd claude/harness && python3 artifacts/f101/selftest_mutant.py
    (expected: 13 PASS, 4 FAIL, exit 1 - recorded in selftest_mutant.txt)
"""
import json as _json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
import dumpread as D                                              # noqa: E402
print(__doc__)


def mutant(path, *, require_old=True):
    header, pairs, missing_new, missing_old = None, [], [], []
    for line in open(path, encoding='utf-8', errors='replace'):
        if not line.strip():
            continue
        try:
            rec = D.loads(line)
        except _json.JSONDecodeError:
            continue
        t = rec.get("type")
        if t == "header":
            header = rec
        elif t != "body":
            continue
        elif rec.get("new") is None:
            missing_new.append(rec["name"])
        elif rec.get("old") is None:
            missing_old.append(rec["name"])
        else:
            pairs.append(rec)
    bodies_old = len(pairs) + len(missing_new)
    records = bodies_old + len(missing_old)
    if require_old and records and bodies_old < 2:          # THE MUTATION
        raise D.EmptyOldHalf(path, bodies_old, len(pairs) + len(missing_old),
                             records)
    return header, pairs, missing_new, missing_old


D.load_dump = mutant
rc = D._selftest()
print("mutant exit = %d (non-zero is the point)" % rc)
sys.exit(0 if rc else 1)
