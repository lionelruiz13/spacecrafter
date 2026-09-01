#!/usr/bin/env python3
"""F49 -- F42-(g)-style bounded cross-entry supersession scan, both halves.

HALF 1 (candidate EVENT detector, F42's own form): a `S<N.M>` citation within 160
characters of an uppercase supersession keyword (REFUTED/SUPERSEDED/CORRECTED/
RETRACTED/WITHDRAWN), on ONE line, target != source.  Over-generates by construction.

HALF 2 (the S11.113(p) question): does the CITED node carry a back-marker naming the
source?  A back-marker = a bracketed span in the target's entry file or its live stub
that contains the citation `S<source>` AND a marker word (case-insensitive:
ANNOTATION/ADDENDUM/BACK-MARKER/SUPERSED/REFUT/CORRECT/RETRACT/WITHDRAW) in the same
span -- the S11.99 form.  Case-insensitive on purpose: S11.164's markers at S11.157
say "are refuted" in lowercase inside an [ANNOTATION ...] span.

Output = the candidate pairs whose target carries no such marker.  ADJUDICATION IS BY
READING: a line may itself BE the back-marker (target/source inverted by the detector),
or the keyword may belong to a neighbouring claim.
"""
import os, re, sys

# Root is REQUIRED [fable 2026-08-30, F52 acceptance — §11.168(m)]: the previous
# absolute-path default made a cd-into-pre-tree run silently measure the LIVE tree
# (second silent-wrong-tree incident this session; instrument-chain rule: a probe
# that can silently measure the wrong target converts observation into fiction).
# Measurement logic untouched; input contract only.
if len(sys.argv) < 2:
    sys.exit("usage: intent_backmarker_scan.py <root>  (root is required; "
             "pass the tree to scan explicitly — no default)")
ROOT = sys.argv[1]
KEYRE = re.compile("REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN")
MARKRE = re.compile("ANNOTATION|ADDENDUM|BACK-MARKER|SUPERSED|REFUT|CORRECT|RETRACT|WITHDRAW", re.I)
CITE = re.compile(r"§(\d+\.\d+)")
W = 160

d = os.path.join(ROOT, "INTENT")
FILES = {fn[:-3]: os.path.join(d, fn) for fn in sorted(os.listdir(d))
         if re.fullmatch(r"\d+\.\d+\.md", fn)}
INTENT_LINES = open(os.path.join(ROOT, "INTENT.md"), encoding="utf-8").read().splitlines()

# --- v2 MEMBER 1 [F78 2026-09-01]: stub resolution, wrong in two ways -------------
# (1-A) the REGISTER COLLISION (S11.165 fourth-instrument-property note): the v1
#       lookup matched the first INTENT.md line starting with the number after the
#       dot, and S5's register precedes S11's in the file, so stub("11.104")
#       returned S5.104's stub -- 79 S11 entry files affected, and F59 measured the
#       mirror (stub("5.31") -> S11.31's line; 23 retired S5 numbers).
# (1-B) the ONE-LINE TRUNCATION: a register row's later content lives in indented
#       continuation lines, and v1 returned only the numbered line.  Live specimen,
#       and on today's tree the ONLY one: S5.117's "MEASURED AND PARTLY CORRECTED
#       2026-09-01 (F77, S11.196)" block, 7 lines.  A continuation is a blank line
#       or an INDENTED line (markdown's own list-item rule) -- a non-indented line
#       ends the row, which is what keeps S11.14's "---" and S11.196's trailing
#       maintenance-marker paragraph out of their stubs.
# Both are the same fact resolved wrongly: "the stub of id X".  Resolution is now by
# REGISTER SPAN + BLOCK, and a missing home answers "" -- never a wrong-but-plausible
# stub.  Half 1 does not call this; the change is confined to half 2's credit.
SEC_HEAD = re.compile(r"^## (\d+)\.")
ROW_HEAD = re.compile(r"^(\d+[a-z]?)\.\s")

def _register_spans(lines):
    heads = [(i, m.group(1)) for i, l in enumerate(lines) for m in [SEC_HEAD.match(l)] if m]
    return {name: (start, heads[k + 1][0] if k + 1 < len(heads) else len(lines))
            for k, (start, name) in enumerate(heads)}

def _build_stubs(lines):
    spans, out = _register_spans(lines), {}
    for sec in ("5", "11"):
        if sec not in spans:
            continue
        a, b = spans[sec]
        idx = [i for i in range(a, b) if ROW_HEAD.match(lines[i])]
        for k, i in enumerate(idx):
            end = idx[k + 1] if k + 1 < len(idx) else b
            j = i + 1
            while j < end and (lines[j].strip() == "" or lines[j][:1].isspace()):
                j += 1
            while j - 1 > i and lines[j - 1].strip() == "":
                j -= 1
            out["%s.%s" % (sec, ROW_HEAD.match(lines[i]).group(1))] = "\n".join(lines[i:j])
    return out

STUBS = _build_stubs(INTENT_LINES)

def stub(idnum):
    return STUBS.get(idnum, "")

def spans(line):
    """bracketed [...] spans, non-nested, plus the whole line as a fallback span."""
    out, depth, start = [], 0, None
    for i, ch in enumerate(line):
        if ch == "[":
            if depth == 0:
                start = i
            depth += 1
        elif ch == "]" and depth:
            depth -= 1
            if depth == 0:
                out.append(line[start:i + 1])
    return out

def has_backmarker(tgt, src):
    pat = re.compile(r"§" + re.escape(src) + r"\b")
    blobs = []
    if tgt in FILES:
        blobs.append(open(FILES[tgt], encoding="utf-8").read())
    s = stub(tgt)
    if s:
        blobs.append(s)
    for b in blobs:
        for line in b.splitlines():
            # form A: the citation sits inside a bracketed marker span (the S11.99 form)
            for sp in spans(line):
                if pat.search(sp) and MARKRE.search(sp):
                    return True
            # form B: the citation sits within W chars of an UPPERCASE keyword, same line
            #         (the unbracketed marker form, e.g. S11.15's "**CORRECTED + CLOSED
            #          (2026-07-19, S11.40)**")
            ks = [m.start() for m in KEYRE.finditer(line)]
            if ks:
                for m in pat.finditer(line):
                    if any(abs(k - m.start()) <= W for k in ks):
                        return True
    return False

events, pairs = 0, set()
for src, path in FILES.items():
    for line in open(path, encoding="utf-8").read().splitlines():
        ks = [m.start() for m in KEYRE.finditer(line)]
        if not ks:
            continue
        hit = False
        for m in CITE.finditer(line):
            tgt = m.group(1)
            if tgt == src:
                continue
            if any(abs(k - m.start()) <= W for k in ks):
                pairs.add((src, tgt)); hit = True
        if hit:
            events += 1

unmarked = sorted(p for p in pairs if not has_backmarker(p[1], p[0]))
print("RAW EVENT LINES                     : %d" % events)
print("DISTINCT CANDIDATE (src, tgt) PAIRS : %d" % len(pairs))
print("CANDIDATES WITH NO BACK-MARKER AT THE TARGET NAMING THE SOURCE : %d" % len(unmarked))
for s, t in unmarked:
    print("   §%-8s -> §%-8s" % (s, t))
