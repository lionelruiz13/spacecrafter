#!/usr/bin/env python3
"""F59 -- enumeration of correcting relations whose TARGET is a §5 row (the §5-side
twin's universe).  THE BOUNDARY, committed BEFORE any verdict (the F58 form).

This is a task-local enumerator, NOT an edit to `intent_backmarker_scan.py`: that
instrument is FROZEN for this round (F56/F57/F58 acceptances) and every observation
about it made here is ROUTED to the queued strict-credit v2 package, never applied.

--------------------------------------------------------------------------------------
HALF 1 -- the candidate detector.  Deliberately the frozen scan's OWN rule
(`intent_backmarker_scan.py:30-33, 86-99`) so the twin is comparable to its §11<->§11
sibling, with ONE stated widening of the SOURCE set and one restriction of the target
set:

  lexicon = REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN   (uppercase, exact)
  window  = a §N.M citation within 160 characters of a keyword, ON ONE LINE
  target  = §5.N ONLY (this audit's axis);  target != source

  SOURCE TEXTS SCANNED -- the widening, stated so the frozen scan's own subset stays
  exactly recoverable (column `srcclass`):
    A-entry     INTENT/<id>.md            the frozen scan's own source set
    B-stub-5    INTENT.md, §5 register    } the instrument scans INTENT.md ONLY to
    B-stub-11   INTENT.md, §11 register   } resolve a TARGET's stub -- never as a
    B-stub-other INTENT.md, elsewhere     } SOURCE.  A correction asserted only in a
                                            stub line is invisible to it.
    C-archive   INTENT/archive/<id>.md    archived entry files, likewise unscanned.

  KNOWN AND ACCEPTED INCOMPLETENESS of half 1, restated so the bound is honest
  (§11.165(c) states it for the §11 axis; it holds identically here):
    - a correcting relation asserted across TWO lines, or in prose without one of the
      five uppercase words, is invisible BY DESIGN (the session-16 discharge-vocabulary
      ruling; the incompleteness-lexicon member of the strict-credit v2 package);
    - the detector over-generates: a keyword may belong to a neighbouring claim.
  A SECONDARY widened-lexicon probe is run separately (`--widen`) to SIZE that miss;
  it is reported, never merged into the primary verdict table.

--------------------------------------------------------------------------------------
HALF 2 -- the twin's question ("does the §5 row carry the marker naming that §11
entry?") is NOT delegated to the instrument's `stub()`.  That function resolves a §-id
by the first INTENT.md line starting with the number after the dot (`:40-45`) -- the
stub-collision defect recorded at §11.165's fourth-property note.  For a §5.N target
whose register row has been RETIRED (23 numbers are absent from the live §5 list,
measured here) it returns §11.N's stub: a wrong-but-plausible home.  So every §5
target's homes are resolved EXPLICITLY:

  homes(§5.N) = { the live register stub line, located INSIDE the §5 numbered list }
              u { INTENT/5.N.md }  u  { INTENT/archive/5.N.md }
  A target with NO home is reported as such: it cannot carry a marker.

The `mech` column applies the scan's own two marker forms (A = citation inside a
bracketed marker span; B = citation within 160 chars of an uppercase keyword) to those
CORRECTLY resolved homes.  It is an INPUT TO ADJUDICATION, not a verdict: every pair in
the delivered table is adjudicated by reading both texts.
"""
import os, re, sys, json
from collections import Counter

ROOT = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("-") \
       else "/home/claude/spacecrafter/claude"
WIDEN = "--widen" in sys.argv

KEYRE  = re.compile("REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN")
# secondary probe only -- NEVER used for the primary table
WIDERE = re.compile("REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN|REFUTES|SUPERSEDES|"
                    "CORRECTS|OVERTURNED|INVALIDATED|FALSIFIED|NARROWED|REOPENED|"
                    "REPLACED|REVISED|AMENDED|RECLASSIFIED|RESCINDED")
MARKRE = re.compile("ANNOTATION|ADDENDUM|BACK-MARKER|SUPERSED|REFUT|CORRECT|RETRACT|WITHDRAW", re.I)
CITE   = re.compile(r"§(\d+\.\d+)")
W = 160

IDIR = os.path.join(ROOT, "INTENT")
ADIR = os.path.join(IDIR, "archive")
LINES = open(os.path.join(ROOT, "INTENT.md"), encoding="utf-8").read().splitlines()


def hdr(n):
    for i, l in enumerate(LINES):
        if l.startswith("## %d. " % n):
            return i
    raise SystemExit("header '## %d.' not found" % n)


R5, R6, R11, R12 = hdr(5), hdr(6), hdr(11), hdr(12)


def register_of(i):
    if R5 <= i < R6:
        return "5"
    if R11 <= i < R12:
        return "11"
    return "other"


STUB5, STUB11 = {}, {}
for i, l in enumerate(LINES):
    m = re.match(r"^(\d+)\. ", l)
    if not m:
        continue
    r = register_of(i)
    if r == "5" and m.group(1) not in STUB5:
        STUB5[m.group(1)] = (i + 1, l)
    if r == "11" and m.group(1) not in STUB11:
        STUB11[m.group(1)] = (i + 1, l)


def homes(tgt):
    n = tgt.split(".", 1)[1]
    out = []
    if n in STUB5:
        out.append(("register-stub", "INTENT.md:%d" % STUB5[n][0], STUB5[n][1]))
    p = os.path.join(IDIR, tgt + ".md")
    if os.path.exists(p):
        out.append(("entry-file", "INTENT/%s.md" % tgt, open(p, encoding="utf-8").read()))
    p = os.path.join(ADIR, tgt + ".md")
    if os.path.exists(p):
        out.append(("archived-entry", "INTENT/archive/%s.md" % tgt,
                    open(p, encoding="utf-8").read()))
    return out


def spans(line):
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


def probe(tgt, src):
    pat = re.compile(r"§" + re.escape(src) + r"\b")
    hits = []
    for kind, where, blob in homes(tgt):
        # a register-stub home IS one line and `where` already carries its INTENT.md
        # line number; an entry-file home is a whole file and needs the offset.
        base = where if kind == "register-stub" else None
        for ln, line in enumerate(blob.splitlines(), 1):
            at = base or ("%s:%d" % (where, ln))
            for sp in spans(line):
                if pat.search(sp) and MARKRE.search(sp):
                    hits.append((kind, "A", at))
                    break
            ks = [m.start() for m in KEYRE.finditer(line)]
            if ks:
                for m in pat.finditer(line):
                    if any(abs(k - m.start()) <= W for k in ks):
                        hits.append((kind, "B", at))
                        break
    return hits


def scan_line(srcid, srcclass, where, line, keyre):
    out = []
    ks = [m.start() for m in keyre.finditer(line)]
    if not ks:
        return out
    for m in CITE.finditer(line):
        tgt = m.group(1)
        if not tgt.startswith("5.") or tgt == srcid:
            continue
        if any(abs(k - m.start()) <= W for k in ks):
            lo = max(0, m.start() - W)
            out.append(dict(src=srcid, srcclass=srcclass, where=where, tgt=tgt,
                            window=line[lo:m.start() + W]))
    return out


def collect(keyre):
    cands = []
    for fn in sorted(os.listdir(IDIR)):
        if not re.fullmatch(r"\d+\.\d+\.md", fn):
            continue
        sid = fn[:-3]
        for ln, line in enumerate(open(os.path.join(IDIR, fn), encoding="utf-8")
                                  .read().splitlines(), 1):
            cands += scan_line(sid, "A-entry", "INTENT/%s:%d" % (fn, ln), line, keyre)
    for i, l in enumerate(LINES):
        m = re.match(r"^(\d+)\. ", l)
        reg = register_of(i)
        sid = "%s.%s" % (reg, m.group(1)) if (m and reg in ("5", "11")) else "INTENT.md"
        cands += scan_line(sid, "B-stub-" + reg, "INTENT.md:%d" % (i + 1), l, keyre)
    for fn in sorted(os.listdir(ADIR)):
        if not re.fullmatch(r"\d+\.\d+\.md", fn):
            continue
        sid = fn[:-3]
        for ln, line in enumerate(open(os.path.join(ADIR, fn), encoding="utf-8")
                                  .read().splitlines(), 1):
            cands += scan_line(sid, "C-archive", "INTENT/archive/%s:%d" % (fn, ln),
                               line, keyre)
    return cands


cands = collect(WIDERE if WIDEN else KEYRE)
pairs = {}
for c in cands:
    pairs.setdefault((c["src"], c["tgt"]), []).append(c)

sys.stderr.write("LEXICON            : %s\n" % ("WIDENED (secondary probe)" if WIDEN
                                                else "the frozen scan's five"))
sys.stderr.write("CANDIDATE LINES    : %d  %s\n"
                 % (len(cands), json.dumps(Counter(c["srcclass"] for c in cands),
                                           sort_keys=True)))
sys.stderr.write("DISTINCT (src,tgt) PAIRS WITH A §5 TARGET: %d\n" % len(pairs))
sys.stderr.write("LIVE §5 REGISTER ROWS: %d ; RETIRED NUMBERS BELOW MAX: %s\n"
                 % (len(STUB5), sorted(set(range(1, max(int(k) for k in STUB5) + 1))
                                       - {int(k) for k in STUB5})))

print("\t".join(["src", "tgt", "srcclass", "homes", "mech", "sites", "window"]))
for (s, t), cs in sorted(pairs.items(),
                         key=lambda kv: (int(kv[0][1].split(".")[1]), kv[0][0])):
    h = homes(t)
    p = probe(t, s)
    print("\t".join([
        s, t,
        "|".join(sorted({c["srcclass"] for c in cs})),
        ",".join(k for k, _, _ in h) or "NONE",
        ";".join("%s/%s@%s" % (k, f, w) for k, f, w in p) or "no-marker",
        ";".join(sorted({c["where"] for c in cs})),
        " || ".join(c["window"].replace("\t", " ") for c in cs)[:1200],
    ]))
