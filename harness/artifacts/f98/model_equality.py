#!/usr/bin/env python3
"""CONTROL (F98): the ONE-HOME refactor must not move F90's or F95's numbers.

The two deleted copies of `show_own_duration` (identical to each other:
`f90_rehearsal.py:573` and `f95_soak.py:288` as they stood at harness
`68efa447`) are re-implemented here VERBATIM and compared with
`sts_duration.show_own_duration` on every `.sts` either instrument reads.

    cd claude/harness && python3 artifacts/f98/model_equality.py

Output committed beside this file as `model_equality.txt`.
"""
import glob
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from sts_duration import parse, show_own_duration          # noqa: E402


def old_copy(path):
    """VERBATIM the deleted function (both copies were byte-identical)."""
    total, pauses, lines = 0.0, 0, 0
    for line in Path(path).read_text(encoding="latin-1").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        lines += 1
        m = re.match(r"^wait\s+duration\s+([0-9.]+)", s)
        if m:
            total += float(m.group(1))
        if re.match(r"^script\s+action\s+pause\b", s):
            pauses += 1
    return round(total, 2), pauses, lines


H = os.path.expanduser("~/.spacecrafter/scripts")
files = []
for d in ("basis", "custom", "deepsky", "fscripts"):
    files += sorted(glob.glob(os.path.join(H, d, "*.sts")))
bad = 0
loopy = []
for f in files:
    a, b = old_copy(f), show_own_duration(f)
    if a != b:
        bad += 1
        print("DIFFER %s old=%s new=%s" % (f, a, b))
    m = parse(f)
    if m["expanded"] != m["own"]:
        loopy.append((os.path.relpath(f, H), m["own"], m["expanded"],
                      m["loops"], m["breaks"]))
print("files compared: %d   DIFFERENCES: %d" % (len(files), bad))
print()
print("the shows whose LOOP-EXPANDED duration differs from the unexpanded one")
print("(i.e. exactly what the two blind copies could not see):")
print("  %-28s %10s %14s  %-24s %s"
      % ("show", "own s", "expanded s", "loops", "breaks"))
for n, o, e, l, br in sorted(loopy, key=lambda x: -x[2]):
    print("  %-28s %10.2f %14.2f  %-24s %d" % (n, o, e, str(l[:5]), br))
print()
print("NEGATIVE CONTROL - the comparison CAN fail: a deliberately mutated")
print("expectation on one file, to show the loop above is not vacuous:")
f = os.path.join(H, "custom", "diaporama.sts")
if os.path.exists(f):
    got = show_own_duration(f)
    print("  diaporama.sts own triple %s ; mutated expectation (0.0, 0, 0) -> %s"
          % (str(got), "DIFFER" if got != (0.0, 0, 0) else "EQUAL"))
    print("  diaporama.sts loop-expanded: %s" % parse(f))
