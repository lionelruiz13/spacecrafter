#!/usr/bin/env python3
"""F147: derive fd_archive_pass.py's unit arguments from the LIVE file, by content anchors.

usage: python3 harness/artifacts/f147/units.py [FILE]      (default: fable-dispatch.md; run from the harness root)
For every unit of plan-out/units.json: its first line must occur EXACTLY ONCE in FILE; the line
count-1 below must equal the recorded last line; the payload md5 must equal the recorded one
(forvixy / protocol units: byte-exact as planned; index-s25-s42 alone is open-ended - its end is the
last non-blank line before the next `---`, because each acceptance appends to that run-on line).
Seams are recomputed: a unit whose removal would leave two blank lines adjacent gets
`--seam-after <id>:<line>:1`. Any failure prints FAIL and exits 1 - nothing is guessed.
Prints the three commands, bottom-up (section 3, then section 1, then sections 0 / 0b), with
<pass> / <pre-commit> left for the supervisor. Read-only."""
import hashlib, json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
plan = json.load(open(os.path.join(HERE, "plan-out", "units.json"), encoding="utf-8"))
path = sys.argv[1] if len(sys.argv) > 1 else "fable-dispatch.md"
raw = open(path, "rb").read()
L = raw.decode("utf-8").split("\n")
n = len(L) - 1
md5 = lambda b: hashlib.md5(b).hexdigest()
ok, got = True, []
for u in plan["units"]:
    hits = [i + 1 for i in range(n) if L[i] == u["first"]]
    if len(hits) != 1:
        print(f"FAIL {u['id']}: first line found {len(hits)} times"); ok = False; continue
    s = hits[0]
    if u["id"] == "index-s25-s42":
        e = s
        while e < n and L[e] != "---":
            e += 1
        while L[e - 1] == "":
            e -= 1
        c = e - s + 1
    else:
        c = u["count"]
        if L[s + c - 2] != u["last"]:
            print(f"FAIL {u['id']}: line {s + c - 1} is not the recorded last line"); ok = False; continue
        if md5(("\n".join(L[s - 1:s - 1 + c]) + "\n").encode("utf-8")) != u["md5"]:
            print(f"FAIL {u['id']}: payload md5 differs from the plan (the text moved under it)"); ok = False; continue
    got.append((u, s, c))
if not ok:
    sys.exit(1)
removed = set()
for u, s, c in got:
    removed.update(range(s, s + c))
seams = []
for u, s, c in got:
    after = s + c
    while after in removed:
        after += 1
    if L[s - 2] == "" and after <= n and L[after - 1] == "" and (s + c) not in removed:
        seams.append((u["id"], s + c))
for kind, label, pre in (("forvixy", "pass A - section 3", "<pre-commit: after M1>"),
                         ("index", "pass B - section 1's derived index", "<pre-commit: after pass A>"),
                         ("protocol", "pass C - sections 0 / 0b", "<pre-commit: after pass B>")):
    us = [f"{u['id']}:{s}:{c}" for u, s, c in got if u["kind"] == kind]
    sm = [f"--seam-after {i}:{p}:1" for i, p in seams if any(u["id"] == i and u["kind"] == kind for u, _, _ in got)]
    print(f"# {label} ({len(us)} units{', ' + str(len(sm)) + ' seam' if sm else ''})")
    print(f"python3 harness/fd_archive_pass.py <pass> 2026-10-11 {pre} " + " ".join(us + sm))
    print()
print(f"# {len(got)} units located in {path} (md5 {md5(raw)[:8]}, {n} lines); every anchor unique, every payload as planned")
