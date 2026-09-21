#!/usr/bin/env python3
"""F128 - compare ISOLATED LAYERS between two captures (row B47, slice 0).

A capture is a directory written by one fresh launch of f128_legs.py: for a tag T the
frames T_on.png / T_off.png (the layer switched on, then off, nothing else touched) and
T.json (the dual_dump taken between them). The LAYER of a capture is ON - OFF: what the
rest of the frame draws cancels, on either pinned path (the b5_oort.py pattern).

Metrics between two layers A and B (float, signed, per channel):
  energy_a/_b  sum |A|, sum |B|                  - 0 means the layer is not there
  rel          sum |A - B| / mean(energy_a, energy_b)   0 identical .. 2 disjoint
               (a pure intensity factor k gives 2|1-k|/(1+k); a turned band about 1)
  ratio        energy_b / energy_a               - the intensity member
  corr         Pearson correlation of the two luminance layers - the placement member
  px8, px32    pixels with a channel of A - B beyond 8 / 32 (px32 = the house metric)
The verdict of every pair reads `rel` against a threshold DERIVED from a measured floor
(same pin, two fresh launches, both pins): thr = K_FLOOR x the largest same-pin rel. Nothing
is pasted from another scene: `floor` writes the floor file, `pair` reads it. A floor of 0
demands a bit-identical layer, which is then what a relaunch gives. A second, named tier is
printed beside it and never replaces it: REL_BOUND = 2 % of the layer's energy (on the SOL
scene about 0.06 deg of turn or 0.7 px of shift: 0.01 deg reads 0.0038, 0.05 px 0.0014).
`--want differ` (a perturbation) must clear REL_BOUND, not only the floor.

usage:
  f128_compare.py floor <out.json> <scene> <dirA> <dirB> [<dirA2> <dirB2> ...]  [--tag T]
  f128_compare.py pair  <out.json> <floor.json> <dirA> <dirB> [--tag T] [--tagb T2]
                        [--want same|differ] [--name N]
Exit: 0 every check PASS, 1 a FAIL, 2 usage / missing capture.
"""
import hashlib
import json
import os
import sys
from pathlib import Path

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from f56_starfield import load          # rgb int16 + luminance: ONE loader

K_FLOOR = 4.0       # a pair is "the same" up to K_FLOOR x the measured same-pin floor
REL_BOUND = 0.02    # second tier, 2 % of the layer's energy: what a perturbation must clear
ENERGY_MIN = 1e5    # a layer under this energy is ABSENT: a pair of absent layers certifies nothing
FAILS = []


def check(name, ok, detail):
    print(("PASS " if ok else "FAIL ") + name + "  " + detail, flush=True)
    if not ok:
        FAILS.append(name)


def md5(path):
    return hashlib.md5(Path(path).read_bytes()).hexdigest()


def layer(cap, tag):
    """(signed rgb layer, signed luminance layer, record) of capture dir `cap`, tag `tag`."""
    on, off = "%s/%s_on.png" % (cap, tag), "%s/%s_off.png" % (cap, tag)
    rgb_on, l_on = load(on)
    rgb_off, l_off = load(off)
    rec = {"dir": str(cap), "tag": tag, "on_md5": md5(on), "off_md5": md5(off),
           "shape": list(rgb_on.shape), "off_lit_gt16": int((rgb_off.max(axis=2) > 16).sum()),
           "state": state(cap, tag)}
    return (rgb_on - rgb_off).astype(np.float64), l_on - l_off, rec


def state(cap, tag):
    """What the dump of the capture says was DRAWN: path, reference, level."""
    p = "%s/%s_state.json" % (cap, tag)
    return json.load(open(p)) if os.path.exists(p) else {}


def metrics(a, la, b, lb):
    ea, eb = float(np.abs(a).sum()), float(np.abs(b).sum())
    d = a - b
    mean_e = 0.5 * (ea + eb)
    sa, sb = la.std(), lb.std()
    corr = float(((la - la.mean()) * (lb - lb.mean())).mean() / (sa * sb)) if sa > 0 and sb > 0 else 0.0
    return {"energy_a": ea, "energy_b": eb, "rel": float(np.abs(d).sum() / mean_e) if mean_e > 0 else 0.0,
            "ratio": eb / ea if ea > 0 else 0.0, "corr": corr,
            "px8": int((np.abs(d) > 8).any(axis=2).sum()), "px32": int((np.abs(d) > 32).any(axis=2).sum()),
            "max_delta": float(np.abs(d).max())}


def compare(cap_a, tag_a, cap_b, tag_b):
    a, la, ra = layer(cap_a, tag_a)
    b, lb, rb = layer(cap_b, tag_b)
    m = metrics(a, la, b, lb)
    m["a"], m["b"] = ra, rb
    return m


def frames(cap_a, tag_a, cap_b, tag_b, which="off"):
    """The same metrics on the FRAMES with the layer off: what else moved (the witnesses)."""
    a, la = load("%s/%s_%s.png" % (cap_a, tag_a, which))
    b, lb = load("%s/%s_%s.png" % (cap_b, tag_b, which))
    return metrics(a.astype(np.float64), la, b.astype(np.float64), lb)


def threshold(floor_rel):
    return K_FLOOR * floor_rel


def drawn(m):
    return "drawn %s | %s" % (m["a"]["state"].get("drawnPath", "?"), m["b"]["state"].get("drawnPath", "?"))


def opt(argv, name, default):
    if name in argv:
        i = argv.index(name)
        v = argv[i + 1]
        del argv[i:i + 2]
        return v
    return default


def main():
    argv = sys.argv[1:]
    if len(argv) < 4 or argv[0] not in ("floor", "pair"):
        print(__doc__)
        return 2
    tag = opt(argv, "--tag", "base")
    tagb = opt(argv, "--tagb", tag)
    want = opt(argv, "--want", "same")
    name = opt(argv, "--name", "pair")
    mode, out = argv[0], argv[1]
    if mode == "floor":
        scene, dirs = argv[2], argv[3:]
        pairs = []
        for cap_a, cap_b in zip(dirs[0::2], dirs[1::2]):
            m = compare(cap_a, tag, cap_b, tag)
            same_path = m["a"]["state"].get("drawnPath") == m["b"]["state"].get("drawnPath")
            check("floor_%s_same_pin_both_sides" % scene, same_path and "drawnPath" in m["a"]["state"], drawn(m))
            check("floor_%s_layer_present" % scene, min(m["energy_a"], m["energy_b"]) > ENERGY_MIN,
                  "energy %.3e | %.3e (absent under %.0e)" % (m["energy_a"], m["energy_b"], ENERGY_MIN))
            print("-- floor %s %s: rel %.5f ratio %.5f corr %.6f px8 %d px32 %d"
                  % (scene, drawn(m), m["rel"], m["ratio"], m["corr"], m["px8"], m["px32"]), flush=True)
            pairs.append(m)
        floor_rel = max(p["rel"] for p in pairs)
        rec = {"scene": scene, "tag": tag, "pairs": pairs, "floor_rel": floor_rel, "K_FLOOR": K_FLOOR,
               "REL_BOUND": REL_BOUND, "threshold_rel": threshold(floor_rel),
               "rule": "threshold_rel = K_FLOOR x max same-pin rel over the pairs; REL_BOUND is the second tier"}
        print("== floor %s: rel %.5f -> threshold %.5f ==" % (scene, floor_rel, rec["threshold_rel"]), flush=True)
    else:
        floor = json.load(open(argv[2]))
        thr = threshold(floor["floor_rel"])
        m = compare(argv[3], tag, argv[4], tagb)
        present = min(m["energy_a"], m["energy_b"]) > ENERGY_MIN
        check(name + "_layer_present", present, "energy %.3e | %.3e" % (m["energy_a"], m["energy_b"]))
        check(name + "_path_read_on_both_sides", "drawnPath" in m["a"]["state"] and "drawnPath" in m["b"]["state"], drawn(m))
        detail = ("%s: rel %.6f against %.6f = %g x the floor %.6f of scene %s (second tier %.2f); ratio %.5f corr %.6f px8 %d px32 %d max delta %.0f"
                  % (drawn(m), m["rel"], thr, K_FLOOR, floor["floor_rel"], floor["scene"], REL_BOUND, m["ratio"], m["corr"], m["px8"], m["px32"], m["max_delta"]))
        if want == "same":
            check(name + "_identical_at_the_floor", present and m["rel"] <= thr, detail)
            check(name + "_within_the_2pct_bound", present and m["rel"] <= REL_BOUND, "rel %.6f against %.2f" % (m["rel"], REL_BOUND))
        else:
            check(name + "_differs", present and m["rel"] > REL_BOUND, detail)
        rec = {"name": name, "want": want, "threshold_rel": thr, "floor_file": argv[2], "metrics": m}
    rec["fails"] = FAILS
    json.dump(rec, open(out, "w"), indent=1)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
