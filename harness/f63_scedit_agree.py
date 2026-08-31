#!/usr/bin/env python3
"""F63 x scedit: does scedit's reading of a line agree with the `#!` verdict the
ENGINE wrote on it?  (scedit INTENT §5 item 15(a-i) measured on engine output,
not on fixtures; constraint C1 on the file rather than on the parser.)

Input: the scripts F63 left annotated (`artifacts/f63/*.sts`, written by the
engine's ScriptAnnotator, code 2b8ec034).  For every line carrying a `#!` tail
the tail's first clause is mapped to the lint id whose `engine_tail` data in
`grammar/sc-grammar.json` starts with it (the same mapping the editor's
MachineTail::relation uses), and `scedit --check` must report exactly that id on
exactly that line; every `#!` inside a quoted value (leg F) must map to NOTHING
and scedit must report nothing there.  The reverse direction too: every
finding scedit reports on those files must sit on a line the engine annotated
with the same class (scedit must not out-claim the engine on ITS OWN classes) -
EXCEPT in the files where the engine's write was REFUSED by the leg's design
(`REFUSED` below: E = read-only directory, G = the line changed since load);
there scedit's findings are expected and are reported as such, not as
disagreements.

Which lines can carry a tail: a `#!` at or after the first '#' outside quotes
(ScriptAnnotator::annotationBegin) ON A LINE THAT EXECUTES - a comment-only
line is dropped at script.cpp:114 before executeCommand, so the annotator never
holds a note for it and neither writes nor clears there; scedit's
EditCore::machineTail mirrors that (sc_editcore.cpp:252-255: no tail unless the
line has a command AND a comment).  The reader below is this script's own
copy of that rule - a THIRD reading of it, tolerated until scedit exposes its
list (`--history`, dispatch task F65), at which point this copy is deleted.
Its first version lacked the executes-only clause and mis-read leg F's line 1
(`# F: a #! inside quotes is text`) as an engine tail - the very gap F65's
grammar clause closes.

    cd claude/harness && ./f63_scedit_agree.py [artifacts/f63] [scedit-binary]

Exit 0 = every annotated line agrees both ways; 1 = a disagreement (printed);
2 = inputs missing.  First measured 2026-08-31 (11:45 run of F63 on 2b8ec034,
scedit 4a00cf31): 12 tails / 12 agree / 0 disagreements / 6 expected findings
in the refused files (E: 5, G: 1).
"""
import json, re, subprocess, sys
from pathlib import Path

HARNESS = Path(__file__).resolve().parent
REPO = HARNESS.parents[1]
ART = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else HARNESS / "artifacts/f63"
SCEDIT = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else REPO / "util/scedit/build-lovely/scedit"
GRAMMAR = REPO / "util/scedit/grammar/sc-grammar.json"
for p in (ART, SCEDIT, GRAMMAR):
    if not p.exists(): print("missing:", p); sys.exit(2)

g = json.loads(GRAMMAR.read_text(encoding="utf-8"))
tails = {}                        # engine clause prefix -> lint id
for seed in g["lint_seeds"]:
    for clause in seed.get("engine_tail", []): tails[clause] = seed["id"]
if not tails: print("no engine_tail data in the grammar"); sys.exit(2)

REFUSED = {"E.sts": "read-only directory: the engine logged, could not write (leg E)",
           "G.sts": "line changed since the script was loaded: skipped by design (leg G)"}

def tail_of(line):
    """annotationBegin's rule, restricted to lines that execute (see the header)."""
    if not line.strip() or line.lstrip().startswith("#"):
        return None                       # comment-only or blank: never dispatched
    inq = False; hash_at = None
    for i, ch in enumerate(line):
        if ch == '"': inq = not inq
        elif ch == '#' and not inq: hash_at = i; break
    if hash_at is None: return None
    j = line.find("#!", hash_at)
    return line[j + 2:].strip() if j >= 0 else None

diags = 0; agree = 0; bad = []; expected = []
for f in sorted(ART.glob("*.sts")):
    text = f.read_bytes().decode("latin-1")
    lines = text.split("\n")
    engine = {}                   # 1-based line -> set of ids the engine's tail names
    for n, ln in enumerate(lines, 1):
        t = tail_of(ln.rstrip("\r"))
        if t is None: continue
        ids = set()
        for part in t.split("; "):
            hit = [i for c, i in tails.items() if part.startswith(c)]
            if hit: ids.add(hit[0])
            else: bad.append("%s:%d: tail clause maps to no scedit id: %r" % (f.name, n, part[:60]))
        if ids: engine[n] = ids
    out = subprocess.run([str(SCEDIT), "--grammar", str(GRAMMAR), "--check", str(f)],
                         capture_output=True, text=True, encoding="latin-1")
    found = {}
    for m in re.finditer(r"^.*:(\d+): (\w+): .*\[-W([\w-]+)\]$", out.stdout, re.M):
        found.setdefault(int(m.group(1)), set()).add(m.group(3))
    for n, ids in engine.items():
        diags += len(ids)
        for i in ids:
            if i in found.get(n, ()): agree += 1
            else: bad.append("%s:%d: engine wrote %s, scedit reports %s" % (f.name, n, i, sorted(found.get(n, ())) or "nothing"))
    for n, ids in found.items():
        for i in ids:
            if i in tails.values() and i not in engine.get(n, ()):
                if f.name in REFUSED: expected.append("%s:%d: scedit reports %s; no tail because %s" % (f.name, n, i, REFUSED[f.name]))
                else: bad.append("%s:%d: scedit reports %s, the engine wrote %s" % (f.name, n, i, sorted(engine.get(n, ())) or "no tail"))
print("engine tails %d / agree %d / disagreements %d / expected findings in refused files %d  (files: %s)"
      % (diags, agree, len(bad), len(expected), ", ".join(p.name for p in sorted(ART.glob("*.sts")))))
for b in bad: print("  DISAGREE", b)
for e in expected: print("  expected", e)
sys.exit(1 if bad or diags == 0 else 0)
