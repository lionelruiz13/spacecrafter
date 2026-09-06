#!/usr/bin/env python3
"""THE ONE HOME OF THE `.sts` DURATION MODEL (F98; INTENT Sec.11.218).

    from sts_duration import parse, show_own_duration

WHY THIS FILE EXISTS.  The model had TWO homes - `f90_rehearsal.py:573` and
`f95_soak.py:288`, the second a copy of the first - and both were blind to
`struct loop`, which is what made `custom/diaporama.sts` SHOW-TIMEOUT on every
cycle of both F95 legs (Sec.11.215(k)(2)).  Two copies of a model is the
duplication I2 forbids, and the F98 corpus makes the blindness fatal:
`fscripts/panorama0.sts` wraps its body in `struct loop 1000000`.  So the model
is fixed ONCE, here, and both instruments read this file.  It is a module of
its own rather than an import of one driver by the other, because
`f90_rehearsal.py` is the developer's entry-document smoke suite and must not
depend on the soak driver to run.

WHAT THE MODEL SEES.
  `wait duration X`        adds X seconds
  `script action pause`    counts one pause (the operator has to answer it)
  `struct loop N` ... `struct loop end`
                           multiplies everything lexically inside it by N;
                           nested loops MULTIPLY (the stack's product)
The grammar is the shipped one: `struct loop N` / `struct loop end` /
`struct loop break` [observed: doc/superscript.sts:1414-1424].

WHAT THE MODEL CANNOT SEE, stated because every number it returns is an
estimate and a user of it must know which way it is wrong:
  - `struct if` INSIDE a loop.  A branch that skips the wait makes the true
    duration SMALLER, so inside a conditional loop the expansion is an UPPER
    BOUND (`fscripts/05.sts` is the measured case: six 240-iteration loops
    whose body waits 0.05 s only on one branch).
  - `struct loop break`.  An early exit the static model cannot evaluate; the
    count is returned in `breaks` so a caller can say "upper bound" out loud
    (`fscripts/panorama0.sts` and `fscripts/S11.sts` carry one each).
  - Everything that takes time WITHOUT an authored wait: `moveto`/`zoom`/
    `look_at ... duration N` transitions, `audio action play`, media upload,
    `body action load` batches.  A show with no `wait duration` line at all
    models as 0.0 s and still takes real wall time; 89 of the 136 `fscripts/`
    shows are in that class, so a caller that turns this number into a timeout
    MUST carry a grace term and attribute what overruns it.
  - `script action play` of a nested script (the shipped doc says a script
    cannot be started inside a loop, `superscript.sts:1418`).

THE TWO PROJECTIONS.
  `parse(path)`             -> the whole model, expanded and unexpanded.
  `show_own_duration(path)` -> `(own, pauses, lines)`, F90's exact triple and
                               its exact semantics: the UNEXPANDED sum, so the
                               smoke suite's recorded baseline cannot move by
                               this refactor.  Verified equal to the deleted
                               copies on every `.sts` either instrument reads
                               (F98: 145 files, 0 differences).
"""

import re
from pathlib import Path

WAIT_RE = re.compile(r"^wait\s+duration\s+([0-9.]+)")
PAUSE_RE = re.compile(r"^script\s+action\s+pause\b")
LOOP_RE = re.compile(r"^struct\s+loop\s+(\d+)\s*$")
LOOP_END_RE = re.compile(r"^struct\s+loop\s+end\b")
LOOP_BREAK_RE = re.compile(r"^struct\s+loop\s+break\b")


def parse(path):
    """-> the duration model of one `.sts`, expanded and unexpanded.

    Keys: own (s, unexpanded), expanded (s, loops applied), pauses (declared),
    pauses_expanded, lines (non-blank non-comment, F90's count), loops (the N
    of every `struct loop` in file order), max_depth, breaks, unclosed (loops
    still open at EOF), stray_ends (a `struct loop end` with no opener).
    """
    own = expanded = 0.0
    pauses = pauses_expanded = lines = 0
    stack, loops, max_depth, breaks, stray_ends = [], [], 0, 0, 0
    mult = 1
    for raw in Path(path).read_text(encoding="latin-1").splitlines():
        s = raw.strip()
        if not s or s.startswith("#"):
            continue
        lines += 1
        m = LOOP_RE.match(s)
        if m:
            stack.append(int(m.group(1)))
            mult *= stack[-1]
            max_depth = max(max_depth, len(stack))
            loops.append(stack[-1])
            continue
        if LOOP_END_RE.match(s):
            if stack:
                mult //= stack.pop()
            else:
                stray_ends += 1
            continue
        if LOOP_BREAK_RE.match(s):
            breaks += 1
            continue
        m = WAIT_RE.match(s)
        if m:
            own += float(m.group(1))
            expanded += float(m.group(1)) * mult
        if PAUSE_RE.match(s):
            pauses += 1
            pauses_expanded += mult
    return {"own": round(own, 2), "expanded": round(expanded, 2),
            "pauses": pauses, "pauses_expanded": pauses_expanded,
            "lines": lines, "loops": loops, "max_depth": max_depth,
            "breaks": breaks, "unclosed": len(stack), "stray_ends": stray_ends}


def show_own_duration(path):
    """F90's triple, unchanged: (unexpanded wait total, pauses, command lines).

    Kept as a PROJECTION of `parse` rather than a second parser - one grammar,
    two readings.  `f90_rehearsal.py` imports this name and its numbers are a
    frozen baseline, so this function must keep returning what the copy it
    replaced returned."""
    m = parse(path)
    return m["own"], m["pauses"], m["lines"]
