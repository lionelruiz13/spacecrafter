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

def stub(idnum):
    pref = idnum.split(".", 1)[1] + ". "
    for line in INTENT_LINES:
        if line.startswith(pref):
            return line
    return ""

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
