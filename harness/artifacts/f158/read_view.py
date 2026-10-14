#!/usr/bin/env python3
"""read_view.py <gone.json> <repo-substring> -- the referent's lines at its pin
beside the region of the target where its SURVIVING lines landed (linemap, the
gate's own content map). A reading aid: what the construct looks like now.
"""
import collections, difflib, json, sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

TO = "9d461bd5"
gone = json.load(open(sys.argv[1]))
flt = sys.argv[2]
by = collections.OrderedDict()
for g in gone:
    by.setdefault((g["repo"], g["elem"], g["pin"]), []).append(g)
for (repo, elem, pin), rows in sorted(by.items(), key=lambda kv: (kv[0][0] or "", int(kv[0][1].split("-")[0]))):
    if not repo or flt not in repo:
        continue
    a = int(elem.split("-")[0]); b = int(elem.split("-")[1]) if "-" in elem else a
    m, lp, lt = ag.linemap(pin, repo, head=TO)
    print("=" * 100)
    print("%s:%s @ %s x%d" % (repo, elem, pin[:8], len(rows)))
    surv = [(n, m[n]) for n in range(a, b + 1) if n in m]
    for n in range(a, b + 1):
        print("  P%5d%s %s" % (n, (" ->%5d" % m[n]) if n in m else "   GONE", lp[n - 1][:110]))
    if surv:
        lo, hi = surv[0][1], surv[-1][1]
        print("  ---- at %s, around the surviving lines %d..%d" % (TO, lo, hi))
        for j in range(max(1, lo - 4), min(len(lt), hi + 3) + 1):
            print("  T%5d  %s" % (j, lt[j - 1][:110]))
    else:
        for n in (a, b):
            src = lp[n - 1].strip()
            c = sorted(((difflib.SequenceMatcher(a=src, b=l.strip(), autojunk=False).ratio(), j)
                        for j, l in enumerate(lt, 1) if l.strip()), reverse=True)[:3]
            print("  ~P%d: %s" % (n, "  ".join("%d(%.2f) %s" % (j, r, lt[j - 1].strip()[:60]) for r, j in c)))
