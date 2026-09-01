#!/usr/bin/env python3
"""F78 member 1 -- arm attribution and extent, measured on the delivery tree.

Arm 1-A = register collision (v1 matched the first INTENT.md line starting with the
number after the dot; S5's register precedes S11's).
Arm 1-B = one-line truncation (v1 returned the numbered line only).

Usage: f78_member1_probe.py <root>
"""
import os, re, sys
ROOT = sys.argv[1]
L = open(os.path.join(ROOT, "INTENT.md"), encoding="utf-8").read().split("\n")
SEC = re.compile(r"^## (\d+)\."); ROW = re.compile(r"^(\d+[a-z]?)\.\s")
heads = [(i, m.group(1)) for i, l in enumerate(L) for m in [SEC.match(l)] if m]
spans = {n: (s, heads[k+1][0] if k+1 < len(heads) else len(L)) for k, (s, n) in enumerate(heads)}

def v1_stub(idn):                       # the v1 resolver, verbatim
    pref = idn.split(".", 1)[1] + ". "
    for line in L:
        if line.startswith(pref):
            return line
    return ""

def v2_line(idn):                       # register-correct, ONE line (arm 1-A only)
    sec, num = idn.split(".", 1)
    if sec not in spans: return ""
    a, b = spans[sec]
    for i in range(a, b):
        m = ROW.match(L[i])
        if m and m.group(1) == num: return L[i]
    return ""

def v2_block(idn):                      # register-correct + block (1-A + 1-B)
    sec, num = idn.split(".", 1)
    if sec not in spans: return ""
    a, b = spans[sec]
    idx = [i for i in range(a, b) if ROW.match(L[i])]
    for k, i in enumerate(idx):
        if ROW.match(L[i]).group(1) == num:
            end = idx[k+1] if k+1 < len(idx) else b
            j = i + 1
            while j < end and (L[j].strip() == "" or L[j][:1].isspace()):
                j += 1
            while j - 1 > i and L[j-1].strip() == "":
                j -= 1
            return "\n".join(L[i:j])
    return ""

d = os.path.join(ROOT, "INTENT")
ids = sorted(fn[:-3] for fn in os.listdir(d) if re.fullmatch(r"\d+\.\d+\.md", fn))
coll = [i for i in ids if v1_stub(i) != v2_line(i)]
coll11 = [i for i in coll if i.startswith("11.")]
wrongreg = [i for i in coll if v1_stub(i) and v2_line(i) == "" or (v1_stub(i) and v2_line(i) and v1_stub(i) != v2_line(i))]
multi = [i for i in ids if v2_block(i).count("\n") > 0]
print("entry ids                                  : %d" % len(ids))
print("ARM 1-A  v1 stub != register-correct stub  : %d  (of which S11.N: %d)" % (len(coll), len(coll11)))
print("         v1 returned a WRONG-BUT-PLAUSIBLE : %d" % len([i for i in coll if v1_stub(i)]))
print("         v1 returned '' where a stub exists: %d" % len([i for i in coll if not v1_stub(i) and v2_line(i)]))
print("ARM 1-B  entry ids whose stub is MULTI-LINE : %d of %d ids with a live stub"
      % (len(multi), len([i for i in ids if v2_block(i)])))
# every INTENT.md register row, not only those with an entry file
allrows = []
for sec in ("5", "11"):
    a, b = spans[sec]
    allrows += [("%s.%s" % (sec, ROW.match(L[i]).group(1))) for i in range(a, b) if ROW.match(L[i])]
print("register rows total (S5 + S11)             : %d" % len(allrows))
cont = [r for r in allrows if v2_block(r).count("\n") > 0]
print("         with a REAL continuation block     : %d  %s" % (len(cont), cont))
print("         longest, in extra lines            : %d"
      % max([v2_block(r).count("\n") for r in allrows] or [0]))
print()
print("NOTE [F78, 2026-09-01]: an earlier run of this probe reported 232 of 282 rows")
print("carrying a continuation block, up to 11 lines.  That was WRONG and the wrongness")
print("is instructive: the block ran to the next numbered row, so every BLANK SEPARATOR")
print("line counted as content.  Under markdown's own list-continuation rule (blank or")
print("indented) the true figure is the line above.  The error was caught by the")
print("measurement it was supposed to explain -- member 1b moved no counter at all, and")
print("232 affected rows could not have done that.")
