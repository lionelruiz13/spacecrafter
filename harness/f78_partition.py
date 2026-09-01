#!/usr/bin/env python3
"""F78 scope 4 -- the RE-PARTITION of strict-credit v2's unmarked residual.

The LIST is the baseline and the counters are its derived summary (the §11.156(e) /
§11.165(c) / §11.179(k)(2) form).  Every pair in the v2 unmarked set gets ONE class.
The machine columns below are an INPUT TO ADJUDICATION, NEVER A VERDICT -- F59's rule,
applied here to my own instrument as well as to the frozen one.

Machine columns, each decidable:
  nohome    the target has no home anywhere (entry file, register stub, INTENT/archive)
  invmech   the producing line lies in the SOURCE's own home AND is itself a marker span
            naming the TARGET -- i.e. the line IS the back-marker, read backwards
            (§11.165(b)'s mechanism, made mechanical; this is candidate C2, USED HERE AS
            EVIDENCE ONLY and deliberately NOT enacted in the instrument)
  srcreg    the source is a §5 row (F59's OFF-AXIS shape: no §11 entry to ask about)
  fromstub  the producing line came from a register row rather than an entry file
            (member 6's own contribution, so its share of the residual is visible)
  quoted    the target citation sits inside a QUOTED run on the producing line -- the
            entry is reporting another node's text, not asserting about the target
            (§11.156(b)'s rule N5: adjudicated at the other node, never here)
  tablerow  the producing line is a table row (a catalogue entry's own inventory)

Usage: python3 harness/f78_partition.py <root> [--tsv <path>]
"""
import os, re, sys

ROOT = os.path.abspath(sys.argv[1])
TSV = sys.argv[sys.argv.index("--tsv") + 1] if "--tsv" in sys.argv else None
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# re-run the v2 instrument in-process, so the partition cannot drift from the counter
import importlib.util
_argv = sys.argv
sys.argv = ["intent_backmarker_scan.py", ROOT]
_spec = importlib.util.spec_from_file_location(
    "sc", os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                       "intent_backmarker_scan.py"))
import io, contextlib
sc = importlib.util.module_from_spec(_spec)
with contextlib.redirect_stdout(io.StringIO()):
    _spec.loader.exec_module(sc)
sys.argv = _argv

UNMARKED = sorted(p for p in sc.pairs if not sc.has_backmarker(p[1], p[0]))


def producing_lines(src, tgt):
    """every (home, line) of SRC that produced this pair under the v2 rule."""
    out = []
    for home, text in [("entry", open(sc.FILES[src], encoding="utf-8").read())
                       if src in sc.FILES else (None, None),
                       ("stub", sc.stub(src))]:
        if not text:
            continue
        for line in text.splitlines():
            ks = [m.start() for m in sc.KEYRE.finditer(line)]
            if not ks:
                continue
            for m in sc.CITE.finditer(line):
                if m.group(1) == tgt and any(abs(k - m.start()) <= sc.W for k in ks):
                    out.append((home, line))
                    break
    return out


QUOTE = re.compile(r'\*"[^"]{0,600}"\*|"[^"]{0,600}"|\u201c[^\u201d]{0,600}\u201d')

def in_quote(line, cited):
    pat = re.compile(r"§" + re.escape(cited) + r"\b")
    return any(pat.search(q.group(0)) for q in QUOTE.finditer(line))


DATE = re.compile(r"\b20\d\d-\d\d-\d\d\b")

def is_marker_line(line, cited):
    """the line, taken ALONE, is a §11.113(p) BACK-MARKER span naming `cited`.

    THE DATE IS LOAD-BEARING AND WAS ADDED BY MEASUREMENT.  Without it the rule accepted
    "**(c) SUPERSEDES the §11.75(b)/§2.0 tension reconciliation**" -- which is §11.76
    ASSERTING a supersession, the exact opposite of a back-marker -- and filed the pair as
    an inversion, i.e. it made a candidate ARREAR disappear into the safe class.  A
    §11.113(p) marker is dated by rule (§11.161(g)); an assertion in running prose is not.
    Same bound as test M's in intent_pair_check.py, for the same reason.
    """
    pat = re.compile(r"§" + re.escape(cited) + r"\b")
    return any(pat.search(s) and sc.MARKRE.search(s) and DATE.search(s)
               for s in sc.spans(line))


rows = []
for src, tgt in UNMARKED:
    pl = producing_lines(src, tgt)
    rows.append(dict(
        src=src, tgt=tgt,
        nohome=sc.homeless(tgt),
        invmech=any(is_marker_line(l, tgt) for _, l in pl),
        srcreg=src.startswith("5."),
        fromstub=all(h == "stub" for h, _ in pl) if pl else False,
        quoted=any(in_quote(l, tgt) for _, l in pl),
        tablerow=any(l.lstrip().startswith("|") for _, l in pl),
        excerpt=(pl[0][1][:160] if pl else ""),
    ))


# ---- ADJUDICATED PAIRS.  Read at their node, one by one, with the ground for each.
# F59's rule applied to my own instrument: the mechanical columns above are an INPUT,
# never a verdict.  "F59 twin #n" = the per-pair row of harness/artifacts/f59/
# TWIN_VERDICTS.md, cited rather than the label carried forward -- §11.179(a)'s "83 -> 82"
# is the precedent for a carried label being wrong.
ADJUDICATED = {
 # -- the §5 axis: every one has a committed per-pair verdict from F59's twin audit
 ("11.101", "5.31"):  ("MINT-ROUTE", "F59 twin #12: §11.101(j) MINTS §5.31; the nearby REFUTED belongs to §11.53's hypothesis"),
 ("11.149", "5.2"):   ("PROXIMITY",  "F59 twin #1: '§5.2 class' is a CLASS LABEL; the REFUTED in the window belongs to §11.101(f)'s claim at §5.27"),
 ("11.155", "5.107"): ("MINT-ROUTE", "F59 twin #39: 'NEW §5.107'; §5.107's stub cites §11.155(g2)(k) as provenance"),
 ("11.157", "5.80"):  ("PROXIMITY",  "F59 twin #29: the AFFECTED/SUPERSEDED verdict is about the HARNESS f34_convention.py, not the row"),
 ("11.164", "5.107"): ("PROXIMITY",  "F59 twin #41: the superseder says so itself -- '§5.107's own row is untouched'"),
 ("11.179", "5.28"):  ("CATALOGUE",  "§11.179(i)/M4's finding text names §5.28's two defects; F59 placed no §5-node annotation because none was owed, and the F59 acceptance disambiguated §5.28 at all three homes"),
 ("11.182", "5.10"):  ("NAMESPACE",  "NAMED EXCEPTION, re-verified at the node: this §5.10 is scedit's tests/derivation-diff.md §5.10 (session-18 §3(e)). An id that RESOLVES here (INTENT/archive/5.10.md) while meaning a document outside it -- no machine settles this, which is why member 5's NO-HOME rule deliberately does not claim it"),
 # -- §11.165's and §11.156's and §11.179's own catalogues (N5 by §11.156(b)'s rule)
 ("11.165", "11.100"):("CATALOGUE",  "§11.165(c) enumerating its own bucket 4"),
 ("11.165", "11.53"): ("CATALOGUE",  "§11.165(c) enumerating its own bucket 4"),
 ("11.165", "11.70"): ("CATALOGUE",  "§11.165(c) enumerating its own bucket 4"),
 ("11.165", "11.99"): ("CATALOGUE",  "§11.165's header/catalogue naming the §11.99 marker form"),
 ("11.165", "5.26"):  ("CATALOGUE",  "§11.165(c) enumerating its own bucket 3/4"),
 ("11.165", "5.32"):  ("CATALOGUE",  "§11.165's header/catalogue"),
 ("11.156", "11.117"):("CATALOGUE",  "§11.156(g)'s out-of-scope paragraph, a catalogue of the class it could not see"),
 ("11.156", "11.135"):("CATALOGUE",  "§11.156's N4 case/word-form enumeration"),
 ("11.156", "11.140"):("CATALOGUE",  "§11.156's N4 case/word-form enumeration"),
 ("11.156", "11.150"):("CATALOGUE",  "§11.156's catalogue of §11.112's marker"),
 ("11.156", "11.153"):("CATALOGUE",  "§11.156's catalogue of §11.144's ATTRIBUTION marker"),
 ("11.156", "11.99"): ("CATALOGUE",  "§11.156's N4 enumeration naming §11.99"),
 ("11.156", "5.106"): ("CATALOGUE",  "§11.156's catalogue of §11.144's marker, which routes to §5.106"),
 ("11.179", "11.6"):  ("CATALOGUE",  "NAMED EXCEPTION, re-verified: §11.179(e) QUOTES the back-marker at §11.6 to name the direction-inversion family -- §11.179(l) predicted this pair before it existed"),
 ("11.179", "11.144"):("CATALOGUE",  "NAMED EXCEPTION, re-verified: same sentence, quoting §11.144's own [ATTRIBUTION REFUTED ... -> new §5.106]"),
 # -- pure proximity: the keyword belongs to a neighbouring claim
 ("11.100", "11.97"): ("PROXIMITY",  "§11.165(c) bucket 4, verbatim: 'the §11.97(c) measurements stand on their own record' beside a RETRACTED about §11.100's OWN (b)"),
 ("11.101", "11.53"): ("PROXIMITY",  "§11.165(c) bucket 4, re-verified at §11.53(d): §11.101(g2) refutes its OWN suspicion, contradicting nothing §11.53 asserted"),
 ("11.70",  "11.52"): ("PROXIMITY",  "§11.165(c) bucket 4, verbatim: 'already reflected §11.52(c)'"),
 ("11.171", "11.165"):("PROXIMITY",  "NAMED EXCEPTION, re-verified at the node -- and the carried label 'benign-in' now has a sharper reason: the keywords in the window are the INSTRUMENT'S OWN LEXICON, quoted ('the event half fires on REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN'). The scan fires on a quotation of itself"),
 ("11.177", "11.161"):("PROXIMITY",  "NAMED EXCEPTION, re-verified at the node: §11.177(l) cites §11.161 as the rule it FOLLOWED, and kept the pair deliberately as this class's specimen"),
 # -- inversion the mechanical column misses
 ("11.165", "11.113"):("PROXIMITY",  "§11.165(b) cites §11.113(p) as the RULE the marker satisfies -- the same shape as §11.177 -> §11.161"),
 ("11.187", "11.113"):("PROXIMITY",  "§11.187's marker names §11.113(p) as the rule it FOLLOWS while superseding §11.184; the (§11.187, §11.184) pair on the same line comes out MARKED, which is the compliance"),
 ("11.165", "5.59"):  ("MINT-ROUTE", "§11.165(d) routes §11.122(i)'s surviving residual to the named defect §5.59 -- a route, not a supersession of §5.59's own claim; §11.165(c)'s bucket 3 shape"),
 ("11.179", "5.106"): ("CATALOGUE",  "NAMED EXCEPTION, re-verified: §11.179(e)'s sentence QUOTING §11.144's own [ATTRIBUTION REFUTED ... -> new §5.106] to name the direction-inversion family"),
 ("11.76",  "11.75"): ("AMBIGUOUS",  "CLASS ADDED DURING THE WALK, with its argument (F59's AMBIGUOUS, added the same way its OFF-AXIS was): SURFACED BY MEMBER 3(b) AND CANNOT BE HONESTLY RESOLVED BY ME. Reading 1, an arrear: §11.76(c) says it SUPERSEDES the §11.75(b)/§2.0 tension reconciliation, and INTENT/11.75.md contains the string 11.76 ZERO times, so §11.75's own node carries nothing. Reading 2, nothing owed: the superseded TEXT is §2.0's line 104, which does carry a dated marker (**SUPERSEDED [vixy 2026-07-22, §11.76(c)]**) -- the marker went where the claim lived, and §11.75(b) only points at §2.0. §2.0 is not a node this instrument can resolve (it is the NO-HOME member on the same line). PLACING OR NOT PLACING A MARKER AT §11.75 IS THE SUPERVISOR'S ACT; this task writes no ledger text to move a counter"),
 # -- §11.197's OWN pairs.  An entry ABOUT supersessions reproduces the catalogue
 # signature exactly as §11.165(c) measured for itself ("this entry adds 21 candidate
 # pairs of its own"); every one below is this entry QUOTING another node's marker or
 # another instrument's finding, never asserting about the target.  §11.172(l)'s
 # sentence-creates-the-event family, and these were expected before they were counted.
 ("11.197", "11.164"):("CATALOGUE",  "§11.197(j) quoting §11.172 -> §11.174's own description, in which the CORRECTED belongs to a neighbouring claim about §11.164/§11.167"),
 ("11.197", "11.167"):("CATALOGUE",  "same sentence, same quotation"),
 ("11.197", "11.180"):("CATALOGUE",  "§11.197(g) naming F60's three STALE ROUTING markers, i.e. quoting the pair §5.44 -> §11.180 that member 6 makes visible"),
 ("11.197", "5.44"):  ("CATALOGUE",  "same sentence"),
 ("11.197", "11.192"):("CATALOGUE",  "§11.197(h) quoting the F73 executor's §0.7 report about §11.192's SUPERSEDED marker"),
 ("11.197", "11.75"): ("CATALOGUE",  "§11.197(j) quoting §11.76(c)'s own assertion verbatim, to show the rule that accepted it -- the quotation manufactures the pair the quotation is about"),
 ("11.172", "11.174"):("INVERTED",   "NAMED EXCEPTION, re-verified at the node and the carried label HOLDS on both halves: the §11.174(c) citation sits inside an [ANNOTATION ... original preserved.] span AT §11.172 (the line IS the marker), and the CORRECTED in the window belongs to a neighbouring claim about §11.164/§11.167. invmech misses it, so 62 is a FLOOR"),
}

# ---- CLASS ASSIGNMENT.  The rules are stated here so the partition is reproducible and
# challengeable; every pair NOT reached by a rule is read individually and listed by name.
for r in rows:
    if (r["src"], r["tgt"]) in ADJUDICATED:
        r["cls"], r["why"] = ADJUDICATED[(r["src"], r["tgt"])]
        continue
    r["why"] = "machine rule"
    if r["nohome"]:
        r["cls"] = "NO-HOME"
    elif r["invmech"]:
        r["cls"] = "INVERTED"
    elif r["quoted"] or r["tablerow"]:
        r["cls"] = "CATALOGUE"
    elif r["srcreg"]:
        r["cls"] = "OFF-AXIS"
    else:
        r["cls"] = "READ"

from collections import Counter
print("CLASS TOTALS")
for k, v in sorted(Counter(r["cls"] for r in rows).items()):
    print("   %-10s %d" % (k, v))
print("   %-10s %d  (sum, must equal the v2 unmarked counter)" % ("TOTAL", len(rows)))
print()
if TSV:
    with open(TSV, "w", encoding="utf-8") as f:
        f.write("src\ttgt\tCLASS\tGROUND\tnohome\tinvmech\tsrcreg\tquoted\ttablerow\tfromstub\texcerpt\n")
        for r in rows:
            f.write("%s\t%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n" % (
                r["src"], r["tgt"], r["cls"], r["why"], r["nohome"], r["invmech"],
                r["srcreg"], r["quoted"], r["tablerow"], r["fromstub"],
                r["excerpt"].replace("\t", " ")))

print("v2 unmarked residual : %d" % len(UNMARKED))
print("  nohome             : %d" % sum(r["nohome"] for r in rows))
print("  invmech (the line IS the marker, read backwards)  : %d"
      % sum(r["invmech"] and not r["nohome"] for r in rows))
print("  srcreg  (source is a §5 row, F59 OFF-AXIS shape)  : %d"
      % sum(r["srcreg"] and not r["invmech"] and not r["nohome"] for r in rows))
print("  fromstub (member 6's own contribution)            : %d"
      % sum(r["fromstub"] for r in rows))
print()
print("  by ADJUDICATION (read at the node) : %d" % sum(r["why"] != "machine rule" for r in rows))
print("  by MACHINE RULE                    : %d" % sum(r["why"] == "machine rule" for r in rows))
left = [r for r in rows if r["cls"] == "READ"]
print("PAIRS NO RULE AND NO ADJUDICATION REACHES : %d" % len(left))
for r in left:
    print("    §%-8s -> §%-8s %s" % (r["src"], r["tgt"], r["excerpt"][:120]))
print()
print("REAL ARREARS (a genuine entry-to-entry supersession whose target lacks its "
      "back-marker) : %d" % sum(r["cls"] == "REAL" for r in rows))
