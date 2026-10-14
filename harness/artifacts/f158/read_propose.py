#!/usr/bin/env python3
"""read_propose.py <gone.json> <out.json> -- PROPOSALS for the read table, never the table.

One proposal per GONE referent, of one kind only: the referent is a WHOLE
FUNCTION at its pin (line a is a definition line whose qualified name is
recognisable, line b is that function's closing brace) and a function of the
SAME qualified name is defined exactly once in the same file at the target.
Everything else is left for reading ("manual"). Each proposal names the
commit that removed the referent's first line text (git log -S over
pin..target), so the reading can check what moved it. The read table is
written by hand from these blocks and the code; this file only shortens the
reading of the mechanical class.
"""
import collections, json, re, subprocess, sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

TO = "9d461bd5"
DEF = re.compile(r'^[A-Za-z_~][\w:<>,*&\s]*?\b((?:[A-Za-z_]\w*::)*~?[A-Za-z_]\w*)\s*\([^;]*$')

def qname(line):
    m = DEF.match(line)
    return m.group(1) if m and "::" in m.group(1) else (m.group(1) if m else None)

def closing(ls, i):
    return next((j for j in range(i, len(ls) + 1) if ls[j - 1].startswith("}")), None)

def defs(ls, q):
    return [i for i, l in enumerate(ls, 1) if qname(l) == q]

def remover(pin, repo, text):
    r = subprocess.run(["git", "-C", ag.REPO, "log", "--format=%h", "-S" + text, "%s..%s" % (pin, TO), "--", repo],
                       capture_output=True)
    return r.stdout.decode().split()

gone = json.load(open(sys.argv[1]))
by = collections.OrderedDict()
for g in gone:
    by.setdefault((g["repo"], g["elem"], g["pin"]), []).append(g)
out = []
for (repo, elem, pin), rows in by.items():
    a = int(elem.split("-")[0]); b = int(elem.split("-")[1]) if "-" in elem else None
    prop = dict(repo=repo, elem=elem, pin=pin, refs=len(rows), kind="manual")
    lp = ag.lines_at(pin, repo) if repo else None
    lt = ag.lines_at(TO, repo) if repo else None
    if lp and lt and b:
        q = qname(lp[a - 1])
        if q and closing(lp, a) == b:
            d = defs(lt, q)
            if len(d) == 1:
                e = closing(lt, d[0])
                prop.update(kind="function", qname=q, new="%d-%d" % (d[0], e),
                            first_line_removed_by=remover(pin, repo, lp[a - 1].strip()[:200]),
                            at_pin=lp[a - 1].strip()[:160], at_to=lt[d[0] - 1].strip()[:160])
    out.append(prop)
json.dump(out, open(sys.argv[2], "w"), indent=1)
c = collections.Counter(p["kind"] for p in out)
print(dict(c), "references:", dict(collections.Counter({k: sum(p["refs"] for p in out if p["kind"] == k) for k in c})))
for p in out:
    if p["kind"] == "function":
        print("  FUNC %-48s %-10s -> %-10s %-40s %s" % (p["repo"].split("/")[-1], p["elem"], p["new"], p["qname"], p["first_line_removed_by"]))
