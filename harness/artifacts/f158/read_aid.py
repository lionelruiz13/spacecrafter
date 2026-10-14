#!/usr/bin/env python3
"""read_aid.py <gone.json> [repo-substring] -- one block per GONE referent, for READING.

Prints the referent's lines at its pin with context, the enclosing function
there, every sentence that cites it (the token in its string), and the closest
lines of the same file at the target by text similarity -- a READING AID only:
nothing here decides a row of the read table (F75's rule: never re-pointed by
arithmetic or by a score; each row is a reading with its reason).
"""
import collections, difflib, json, subprocess, sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

TO = "9d461bd5"
gone = json.load(open(sys.argv[1]))
flt = sys.argv[2] if len(sys.argv) > 2 else ""
by = collections.OrderedDict()
for g in gone:
    by.setdefault((g["repo"], g["elem"], g["pin"]), []).append(g)
strings = {}
for path in ag.TARGETS:
    for jp, s in ag.walk(ag.load(path)):
        strings[(path, jp)] = s
for (repo, elem, pin), rows in sorted(by.items(), key=lambda kv: (kv[0][0], int(kv[0][1].split("-")[0]))):
    if flt not in (repo or ""):
        continue
    print("=" * 100)
    print("REFERENT %s:%s @ %s   (%d references)" % (repo, elem, pin, len(rows)))
    if not repo:
        print("  (no file)"); continue
    a = int(elem.split("-")[0]); b = int(elem.split("-")[1]) if "-" in elem else a
    lp = ag.lines_at(pin, repo); lt = ag.lines_at(TO, repo)
    print("  enclosing at pin: %s" % ag.enclosing_symbol(pin, repo, a))
    for n in range(max(1, a - 2), min(len(lp), b + 2) + 1):
        print("  %s %5d| %s" % (">" if a <= n <= b else " ", n, lp[n - 1][:150]))
    for g in rows:
        s = strings[(g["contract"], g["jp"])]
        t = ag.tokenize(s)[g["ti"]]
        print("  CITED BY %s %s: ...%s..." % (g["contract"].split("/")[-1], g["jp"],
              s[max(0, t.start - 160):t.end + 60].replace("\n", " ")))
    if lt is None:
        print("  FILE ABSENT at %s" % TO); continue
    for n in range(a, b + 1):
        src = lp[n - 1].strip()
        if not src:
            continue
        cands = sorted(((difflib.SequenceMatcher(a=src, b=l.strip(), autojunk=False).ratio(), j)
                        for j, l in enumerate(lt, 1) if l.strip()), reverse=True)[:2]
        print("  pin %5d ~ %s" % (n, "  ".join("%d(%.2f): %s" % (j, r, lt[j - 1].strip()[:70]) for r, j in cands)))
