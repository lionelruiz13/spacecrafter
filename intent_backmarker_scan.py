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
# EVENT lexicon.  v1's five words [F49].
# [F78 member 2, 2026-09-01] + INCOMPLETE: S11.177 measured that a marker can satisfy
# S11.161(g) completely and still be uncredited, because the lexicon has no term for
# THIS CLAIM WAS INCOMPLETE.  Its own specimen is the marker S11.177(m) placed at
# S11.48(b): "the A17 residual list is INCOMPLETE".  One word, one named specimen.
# BOUNDED by the session-16 scan-owner ruling: discharge vocabulary stays OUT of this
# lexicon BY DESIGN -- a discharge is not a supersession.  DISCHARGED / ANSWERED /
# DELIVERED / PAID are therefore absent here on purpose, and admissible only in
# MARKRE, where the question asked is a different one (see member 3).
# [F78 member 3 arm (b), 2026-09-01] + the PRESENT TENSE of the same five words.
# S11.179(i)/M2: "S11.130 -> S5.63"'s source line heads "THREE THINGS S5.63 SAID THAT
# THIS RETRACTS OR CORRECTS" -- a MARKED pair carrying three covered claims, invisible
# because the lexicon knew only past participles.  No new concept enters, only the
# other tense of words already ruled in.
# [F78 member 4, 2026-09-01] + STALE.  S11.180(l) committed a scan prediction, measured
# it wrong, and traced the failure: every refutation in that entry is written
# lowercase-bold and the keyword its three annotations open with is "STALE ROUTING",
# so a delivery that corrected three rows moved the scan by zero.  An assertion that a
# claim is STALE is a correction-class event, and "[STALE ROUTING <date>, S11.180 ...]"
# at a target is a back-marker naming its source -- so the word enters BOTH lexicons.
# NOT taken with it, and stated: S13 row ids (A<n>/B<n>) stay unreadable.  S11.180(i)
# measured that axis as not machine-decidable (15 of 73 references NOT-A-ROW from seven
# namespace collisions), and a test that emits them installs a permanent false-positive
# population -- exactly the debt this package is paying down.
KEYRE = re.compile("REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN|INCOMPLETE|"
                   "REFUTES|SUPERSEDES|CORRECTS|RETRACTS|WITHDRAWS|STALE ROUTING")
# MARKER lexicon (half 2, form A only -- see the asymmetry note below).
# [F78 member 3 arm (a), 2026-09-01] v1's eight words are the SUPERSESSION lexicon
# wearing a second hat, and S11.179(i)/M1 measured six genuine dated markers they
# cannot see.  Two changes, both traceable:
#   (1) the SIBLING instrument's list is adopted whole -- intent_pair_check.py:68-70's
#       MARK_RE is this ledger's own recorded answer to "what word opens a marker",
#       and keeping a second poorer copy here is the duplication I2 forbids;
#   (2) five words neither list had, each with a named specimen:
#       REFRESH   <- "[STUB REFRESHED 2026-08-29 -- F42/S11.156 ...]"        (S5.2)
#       ROOT-CAUSE<- "**(a) ROOT-CAUSED 2026-07-31 by F18, S11.127(c) ...**" (S5.53)
#       FIXED     <- "**FIXED AND CLOSED 2026-08-26 (F40, S11.153 ...)**"    (S5.104, S5.80)
#       TESTED    <- "[TESTED 2026-08-30, S11.177 (task F57) ...]"           (S11.173)
#       INCOMPLETE<- "[ANNOTATION 2026-08-30, S11.177 ... IS INCOMPLETE]"    (S11.48)
# THE ASYMMETRY, stated because it is the package's governing argument: a wide MARKER
# lexicon credits an arrear and makes it VANISH, which is the failure this instrument
# exists to prevent, while a wide EVENT lexicon only adds visible noise.  So this list
# is used ONLY inside form A, which already requires a BRACKETED-or-BOLD span holding
# the citation; the unbracketed proximity form B stays keyed on KEYRE.  Discharge
# vocabulary is admissible HERE and not in KEYRE (session-16 ruling): "[OWED SWEEP
# DISCHARGED 2026-08-29 (F46 -> S11.160)]" at a target IS a back-marker naming its
# source, and refusing to read it manufactures a false arrear.
MARKRE = re.compile("ANNOTAT|ADDENDUM|BACK-MARKER|SUPERSED|REFUT|CORRECT|RETRACT|WITHDRAW|"
                    "RATIFIED|REOPEN|UPDATED?|DELIVERED|PERFORMED|DISCHARG|UNBLOCK|ATTRIBUTED|"
                    "CLOSED|FIXED|REFRESH|ROOT-CAUSE|TESTED|INCOMPLETE|STALE", re.I)
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

BOLD = re.compile(r"\*\*(.+?)\*\*")

def spans(line):
    """marker-span candidates on one line.

    v1: bracketed [...] spans, non-nested.
    [F78 member 3 arm (a), 2026-09-01] PLUS **bold** spans.  S11.179(i)/M1 measured six
    genuine, dated, correctly-placed S11.113(p) markers that half 2 cannot see because
    they are "neither bracketed nor uppercase-keyworded" -- and FOUR of the six are
    bold, not bracketed: "**(a) ROOT-CAUSED 2026-07-31 by F18, S11.127(c) ...**",
    "**OWED SWEEP DISCHARGED 2026-08-29 (F46 -> S11.160)**", "**FIXED AND CLOSED
    2026-08-26 (F40, S11.153 ...)**", "**EXTENSION 2026-08-09 (F34, S11.144 ...): ...
    the row's own text is corrected ...**".  A vocabulary widening alone would have
    credited none of them.  The sibling instrument already reads both shapes as one
    class (`SPAN_RE` in intent_pair_check.py:71) -- this is that notion of a span,
    held once (I2).
    """
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
    out += BOLD.findall(line)
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

ARCHDIR = os.path.join(ROOT, "INTENT", "archive")

def homeless(tgt):
    """[F78 member 5, 2026-09-01] a target with NO home ANYWHERE in this ledger -- no
    entry file, no register stub, no archived entry -- CANNOT carry a marker, so filing
    it under "no back-marker at the target" asserts something the corpus cannot make
    true (F59's NO-LIVE-NODE verdict class).

    THE ARCHIVE CLAUSE IS LOAD-BEARING AND WAS ADDED BY MEASUREMENT: the first cut asked
    only for an entry file and a stub, and it labelled S5.31 and S5.10 homeless.  Both
    have `INTENT/archive/5.N.md` (F59's homes() reads exactly those three homes), so the
    class was over-claiming on two of its three members.  In particular it did NOT
    resolve session-18 S3(e)'s named exception: S11.182's `S5.10` means scedit's
    tests/derivation-diff.md S5.10, and an id that RESOLVES in this ledger while meaning
    a document outside it is a namespace collision no machine can settle -- it stays a
    read exception in the partition, which is the honest place for it.

    Today the class has exactly one member, S2.0 -- the domain-constraint block, outside
    both registers.  REPORTING only: the headline counter's definition does not move, so
    the supervisor's gate keeps its meaning and one line reverses the split.

    NOT taken, recorded as candidate C4: half 2 does not read ARCHIVED homes for credit,
    so a marker at an archived S5 row is invisible.  F59 re-implemented half 2 to read
    them and found zero arrears there; widening credit is the dangerous direction and
    this would be an eighth member."""
    return (tgt not in FILES and not stub(tgt)
            and not os.path.exists(os.path.join(ARCHDIR, tgt + ".md")))

unmarked = sorted(p for p in pairs if not has_backmarker(p[1], p[0]))
nohome = [p for p in unmarked if homeless(p[1])]
print("RAW EVENT LINES                     : %d" % events)
print("DISTINCT CANDIDATE (src, tgt) PAIRS : %d" % len(pairs))
print("CANDIDATES WITH NO BACK-MARKER AT THE TARGET NAMING THE SOURCE : %d" % len(unmarked))
print("   of which THE TARGET HAS NO HOME IN THIS LEDGER (cannot carry one) : %d" % len(nohome))
for s, t in unmarked:
    print("   §%-8s -> §%-8s%s" % (s, t, "   [NO HOME]" if homeless(t) else ""))
