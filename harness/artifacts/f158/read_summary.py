#!/usr/bin/env python3
"""read_summary.py <gone.json> [repo-substring] -- one compact block per GONE referent.

For each end of the referent: its text at the pin, its own verdict at the
target (an end of a range can survive while the other left), the function
enclosing it at the pin and, if a function of that name is defined in the
same file at the target, where it starts and closes there. A READING AID: the
read table's rows are decided by reading these blocks and the code, never by
this script's output alone.
"""
import collections, json, re, sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

TO = "9d461bd5"
gone = json.load(open(sys.argv[1]))
flt = sys.argv[2] if len(sys.argv) > 2 else ""
by = collections.OrderedDict()
for g in gone:
    by.setdefault((g["repo"], g["elem"], g["pin"]), []).append(g)

def defs_at(rev, repo, name):
    ls = ag.lines_at(rev, repo) or []
    out = []
    for i, l in enumerate(ls, 1):
        if re.match(r'^[A-Za-z_~][^;]*\b%s\s*\(' % re.escape(name), l) and not l.rstrip().endswith(";"):
            end = next((j for j in range(i, len(ls) + 1) if ls[j - 1].startswith("}")), None)
            out.append((i, end))
    return out

strings = {}
for path in ag.TARGETS:
    for jp, s in ag.walk(ag.load(path)):
        strings[(path, jp)] = s
for (repo, elem, pin), rows in sorted(by.items(), key=lambda kv: (kv[0][0] or "", int(kv[0][1].split("-")[0]))):
    if flt not in (repo or ""):
        continue
    a = int(elem.split("-")[0]); b = int(elem.split("-")[1]) if "-" in elem else None
    lp = ag.lines_at(pin, repo)
    print("-" * 110)
    print("%s:%s @ %s  x%d  [%s]" % (repo, elem, pin[:8], len(rows), ",".join(sorted(set(g["contract"].split("/")[-1].replace(".json", "") for g in rows)))))
    for n in [a] + ([b] if b else []):
        v, new, txt, _ = ag.classify(pin, repo, n, head=TO)
        sym = ag.enclosing_symbol(pin, repo, n)
        d = defs_at(TO, repo, sym) if sym else []
        dp = defs_at(pin, repo, sym) if sym else []
        print("   end %5d %-6s -> %-5s | %s" % (n, v, new, (txt or "").strip()[:90]))
        print("             enclosing %s; at pin def %s; at %s def %s" % (sym, dp, TO, d))
    for g in rows[:3]:
        s = strings[(g["contract"], g["jp"])]
        t = ag.tokenize(s)[g["ti"]]
        print("   cited %s%s: ...%s..." % (g["jp"], " (MARKED)" if g["marked"] else "",
              s[max(0, t.start - 110):t.end + 40].replace("\n", " ")))
    if len(rows) > 3:
        print("   (+%d more citing paths: %s)" % (len(rows) - 3, ", ".join(sorted(set(g["jp"].split(".")[-1] for g in rows[3:])))[:200]))
