#!/usr/bin/env python3
"""F40 — the ANCHORED regression: two f40_inverse runs, field by field.

    cd claude/harness && ./f40_cmp.py <preOutdir> <postOutdir>

Reads the `data.L9` list of two runs' `f40_result.json` - the shape BOTH
`f40_anchored.py` (its whole itinerary) and `f40_inverse.py` (its tail leg)
write - and compares the dumped camera state, the `get status position` readout
and the composed screen of every place.

USE IT ON `f40_anchored.py` RUNS.  On `f40_inverse.py` runs it reports 15
failures and they are the FIX, not a regression: that itinerary runs after every
free-mode transition, and the deduce-identical-view rule holds the composed
body->eye rotation across each one, so the PRE binary's observer - teleported
eight times - arrives at the same lat/lon/alt with a different LOCAL orientation
(measured: identical pose members everywhere, heading 55.402987 vs -46.992559).
Two runs whose histories legitimately differ cannot be compared on the screen.
`f40_anchored.py` enters free mode nowhere and aims deterministically, so the
orientation is a function of the place; there, pre and post agree bit for bit.

Compared: the camera's whole dumped state (the drawn matrix included), the
`get status position` readout, and the composed SCREEN of each place.  Exact
equality is the gate for the pose members and the matrix, because an anchored
place passes through NO expression this task changed - `moveTo`'s anchored
branch and `viewMat`'s anchored branch are untouched, and `placeAt` was rerouted
through an expression the probe checked bit-identical.  A non-zero here is a
regression, not a tolerance question.
"""

import json, sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dumpread

FAILS = []


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def flat(o, p=""):
    if isinstance(o, dict):
        for k, v in o.items():
            yield from flat(v, f"{p}.{k}" if p else k)
    elif isinstance(o, list):
        for i, v in enumerate(o):
            yield from flat(v, f"{p}[{i}]")
    else:
        yield p, o


def main():
    A, B = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve()
    ra = json.loads((A / "f40_result.json").read_text())
    rb = json.loads((B / "f40_result.json").read_text())
    la, lb = ra["data"]["L9"], rb["data"]["L9"]
    if len(la) != len(lb):
        fail(f"itinerary length {len(la)} vs {len(lb)}")
        return 1
    # camera state, exact
    IGNORE = {"file", "tag"}
    for i, (x, y) in enumerate(zip(la, lb)):
        assert x["cmd"] == y["cmd"], "itineraries differ"
        da, db = dict(flat(x["dump"])), dict(flat(y["dump"]))
        diffs = [k for k in da
                 if k.split(".")[0] not in IGNORE and da[k] != db.get(k)]
        # `position` is the free-mode member: dead while anchored, rewritten on
        # entering free flight, and its value IS what this task changed - so it
        # is expected to differ and is reported rather than gated.
        pos = [k for k in diffs if k.startswith("position")]
        hard = [k for k in diffs if not k.startswith("position")]
        if hard:
            fail(f"place {i} {x['cmd']}: {len(hard)} field(s) differ: "
                 + ", ".join(f"{k} {da[k]!r} -> {db.get(k)!r}" for k in hard[:6]))
        else:
            ok(f"place {i} lat/lon/alt {x['cmd']}: every dumped camera field "
               f"identical ({len(da)} fields), except the dead free-mode member "
               f"({len(pos)} components: {[round(da[k],10) for k in sorted(pos)]} -> "
               f"{[round(db[k],10) for k in sorted(pos)]})")
        if x["place"] != y["place"]:
            fail(f"place {i}: `get status position` {x['place']} -> {y['place']}")
    # screens, exact
    for i in range(len(la)):
        pa, pb = A / f"shot_L9_anchored_{i}.png", B / f"shot_L9_anchored_{i}.png"
        if not (pa.exists() and pb.exists()):
            fail(f"screen {i}: missing artifact")
            continue
        u = np.asarray(Image.open(pa).convert("L"), dtype=int)
        v = np.asarray(Image.open(pb).convert("L"), dtype=int)
        d = int(np.abs(u - v).max())
        n8 = int((np.abs(u - v) > 8).sum())
        (ok if d == 0 else fail)(
            f"screen {i}: max abs pixel difference {d} ({n8} px>8) over "
            f"{u.shape[0]}x{u.shape[1]}")
    print(f"\nf40_cmp: {'GREEN' if not FAILS else str(len(FAILS)) + ' FAILURES'}")
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main())
