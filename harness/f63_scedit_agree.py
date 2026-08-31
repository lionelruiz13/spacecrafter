#!/usr/bin/env python3
"""F63 x scedit: does scedit's reading of a line agree with the `#!` verdict the
ENGINE wrote on it?  (scedit INTENT §5 item 15 measured on engine output, not on
fixtures; constraint C1 on the file rather than on the parser.)

Input: the scripts F63 left annotated (`artifacts/f63/*.sts`, written by the
engine's ScriptAnnotator, code 2b8ec034).  Both readings come from ONE call per
file, `scedit --history` (F65): its `spacecrafter` rows are the `#!` tails
scedit located in the file, its `scedit` rows are what scedit's own checker
finds there now.  Each engine sentence is mapped to the lint id whose
`engine_tail` data in `grammar/sc-grammar.json` says so — the same shared data
the editor's MachineTail::relation uses, not a second copy of a rule — and
scedit must report exactly that id on exactly that line.  The reverse direction
too: every finding scedit reports on those files must sit on a line the engine
annotated with the same class (scedit must not out-claim the engine on ITS OWN
classes), EXCEPT where the engine's write was REFUSED by the leg's design
(legs E and G below); there scedit's findings are expected, and are printed as
such rather than counted as disagreements.

WHY THIS SCRIPT NO LONGER READS THE `#!` RULE ITSELF (I2).  Until 2026-08-31 it
carried a `tail_of()` that re-implemented ScriptAnnotator::annotationBegin in
Python — a THIRD reading of the rule, after the engine's and scedit's.  Its
first version lacked the executes-only clause (a comment-only line is dropped at
script.cpp:117 before executeCommand, so the annotator never holds a note for it
and neither writes nor clears there) and it mis-read leg F's line 1,
`# F: a #! inside quotes is text`, as an engine tail.  The rule was then written
down where it belongs — `parse_model.comments.machine_tail`, EXECUTES-ONLY
clause — and this copy was deleted: the tails below are the ones scedit reports,
so a divergence between scedit and the engine shows up as a DISAGREEMENT here
instead of being hidden by a third opinion that happens to match one of them.

    cd claude/harness && ./f63_scedit_agree.py [artifacts/f63] [scedit-binary]

Exit 0 = every annotated line agrees both ways; 1 = a disagreement (printed), or
no tail was found at all (an empty run must not look like a pass); 2 = inputs
missing.  First measured 2026-08-31 (11:45 run of F63 on 2b8ec034, scedit
4a00cf31): 12 tails / 12 agree / 0 disagreements / 6 expected findings in the
refused files (E: 5, G: 1).  Same numbers after this rewrite, on the same
artifacts, scedit at the F65 commits.
"""
import json, subprocess, sys
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
ART = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f63"
SCEDIT = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else REPO / "util/scedit/build-lovely/scedit"
GRAMMAR = REPO / "util/scedit/grammar/sc-grammar.json"
for p in (ART, SCEDIT, GRAMMAR):
    if not p.exists(): print("missing:", p); sys.exit(2)

# The ten legs F63 wrote, and what each one puts in front of this script. The
# two REFUSED legs are the reason a scedit finding without a tail is not always
# a disagreement: the engine HAD the diagnosis and could not place it.
LEGS = {
    "A.sts": ("one of each block-structure fault", "annotated"),
    "B.sts": ("an unclosed 'struct if', alone; the run that fixed it cleared the tail", "annotated then cleared"),
    "C.sts": ("a 'struct if end' with nothing open, closed correctly in the file", "annotated then cleared"),
    "D.sts": ("A again, CRLF: the endings must survive the rewrite", "annotated"),
    "E.sts": ("A again in a READ-ONLY DIRECTORY: a sibling-temp write cannot land", "REFUSED"),
    "F.sts": ("a `#!` inside a quoted value, and one in a column-0 comment: text, not tails", "nothing to write"),
    "G.sts": ("the line was changed while the script ran: the annotator skips it", "REFUSED"),
    "H.sts": ("plays I: a fault in a script played BY another lands in ITS file", "no fault of its own"),
    "I.sts": ("H's target, whose 'struct if' is never closed", "annotated"),
    "J.sts": ("a fault inside a loop body, replayed: two log lines, one tail", "annotated"),
}
REFUSED = {"E.sts": "read-only directory: the engine logged, could not write (leg E)",
           "G.sts": "line changed since the script was loaded: skipped by design (leg G)"}

g = json.loads(GRAMMAR.read_text(encoding="utf-8"))
tails = {}                        # engine clause prefix -> lint id
for seed in g["lint_seeds"]:
    for clause in seed.get("engine_tail", []): tails[clause] = seed["id"]
if not tails: print("no engine_tail data in the grammar"); sys.exit(2)


def history(path):
    """scedit's list for one file: (engine rows, scedit rows), each line -> ids.

    Seven TAB-separated fields per row (scedit --history, README § --history):
    file, line, source, id, severity, message, relation.
    """
    out = subprocess.run([str(SCEDIT), "--grammar", str(GRAMMAR), "--history", str(path)],
                         capture_output=True, text=True, encoding="latin-1")
    if out.returncode > 1:
        print("scedit --history failed on %s: %s" % (path.name, out.stderr.strip())); sys.exit(2)
    engine, found = {}, {}
    for row in out.stdout.splitlines():
        f = row.split("\t")
        if len(f) != 7:
            print("unexpected --history shape (%d fields): %r" % (len(f), row[:100])); sys.exit(2)
        n, source, ident, message = int(f[1]), f[2], f[3], f[5]
        if source == "scedit":
            found.setdefault(n, set()).add(ident)
            continue
        # An engine row: one sentence per diagnostic, joined with "; ".
        ids = set()
        for part in message.split("; "):
            hit = [i for c, i in tails.items() if part.startswith(c)]
            if hit: ids.add(hit[0])
            else: bad.append("%s:%d: tail clause maps to no scedit id: %r" % (path.name, n, part[:60]))
        if ids: engine[n] = ids
    return engine, found


diags = 0; agree = 0; bad = []; expected = []
files = sorted(ART.glob("*.sts"))
for f in files:
    if f.name not in LEGS:
        bad.append("%s: not one of F63's ten legs — this artifact directory is not the one this "
                   "instrument describes" % f.name)
        continue
    engine, found = history(f)
    for n, ids in engine.items():
        diags += len(ids)
        for i in ids:
            if i in found.get(n, ()): agree += 1
            else: bad.append("%s:%d: engine wrote %s, scedit reports %s"
                             % (f.name, n, i, sorted(found.get(n, ())) or "nothing"))
    for n, ids in found.items():
        for i in ids:
            if i in tails.values() and i not in engine.get(n, ()):
                if f.name in REFUSED:
                    expected.append("%s:%d: scedit reports %s; no tail because %s" % (f.name, n, i, REFUSED[f.name]))
                else:
                    bad.append("%s:%d: scedit reports %s, the engine wrote %s"
                               % (f.name, n, i, sorted(engine.get(n, ())) or "no tail"))

print("engine tails %d / agree %d / disagreements %d / expected findings in refused files %d  (files: %s)"
      % (diags, agree, len(bad), len(expected), ", ".join(p.name for p in files)))
for b in bad: print("  DISAGREE", b)
for e in expected: print("  expected", e)
sys.exit(1 if bad or diags == 0 else 0)
