#!/usr/bin/env python3
"""f76_corpus.py - the disposition table for scedit's findings over the SHIPPED
scripts (scedit INTENT S5 item 16; constraint C3).

C3 says a lint rule ships only when the shipped corpus produces zero FALSE
positives, and that the TRUE findings are recorded upstream instead of being
silenced.  The 408 installed scripts met the thirteen shipped rules for the
first time on 2026-08-30 and produced 1661 findings that nobody had judged:
"by inspection plausibly all TRUE, but plausibly is not a disposition"
(journal 2026-08-30e).  This script is the judgment, made machine-checkable.

SHAPE (f70_dispositions.py's, deliberately): a MECHANICAL census joined to
HAND-MADE dispositions.  The census comes from `scedit --check` itself - never
from a second reading of the scripts - and each disposition carries the GROUND
it was decided on (an engine file:line that was read, or a live observation).
A finding that matches no disposition row is an ERROR, not a default: an
unjudged finding is exactly the one that would ship unnoticed.

    cd claude/harness && ./f76_corpus.py > artifacts/f76/dispositions.tsv
    ./f76_corpus.py --strict        # exit 1 if anything is UNADJUDICATED

THE FOUR DISPOSITIONS (the task's taxonomy, F76):

  TRUE-shipped    the script is wrong; the engine does what the message says.
                  Routes to the script-surface owner (SCRIPT_SURFACE.md, SS-n)
                  - it is his file and his intent, never ours to edit (D9).
  TRUE-generator  the file is generated and the defect is the generator's.
  FALSE-POSITIVE  scedit is wrong: a C1 defect of a rule or of the grammar.
                  Fixed in the task that finds it - a rule that fires falsely
                  on shipped content does not ship.
  ENGINE          the engine's behaviour is the defect and the script is
                  reasonable.  Routes to the parent ledger (S5.79), recorded
                  and not fixed.

THE CORPUS IS NEVER DECODED.  It is untracked field data in ISO-8859 or
anything else; only scedit reads its bytes.  This script reads scedit's OUTPUT,
and writes the table as pure ASCII (any high byte is escaped), so the artifact
is greppable and D14-clean whatever the scripts hold.

THE TWIN RULE.  43 md5-identical groups exist under ~/.spacecrafter/scripts
(mostly `navigation/fscripts/X` mirroring `fscripts/X`), so one authored defect
can appear on several rows and one fix must land in every member.  The TWINS
column names the other members, measured by md5 at run time, so the table
itself carries the fact instead of a sentence somewhere else remembering it.
"""

import argparse
import collections
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))          # /home/claude/spacecrafter
DEFAULT_SCEDIT = os.path.join(REPO, "util/scedit/build-f75/scedit")
DEFAULT_GRAMMAR = os.path.join(REPO, "util/scedit/grammar/sc-grammar.json")
DEFAULT_CORPUS = os.path.expanduser("~/.spacecrafter/scripts")

LINE_RE = re.compile(rb"^(.*?):(\d+): (warning|error): (.*) \[-W([a-z-]+)\]$")


def ascii_only(b):
    """Bytes -> a pure-ASCII string, high bytes shown as \\xNN.  The corpus is
    not decoded: an escape is a faithful record of a byte, a decode is a
    guess about an encoding nobody declared."""
    if isinstance(b, str):
        b = b.encode("utf-8")
    return b.decode("ascii", "backslashreplace")


# --------------------------------------------------------------- the census
def census(scedit, grammar, corpus):
    """Every finding scedit reports over the corpus, in its own order."""
    files = []
    for root, _dirs, names in os.walk(corpus):
        for n in names:
            if n.endswith(".sts"):
                files.append(os.path.join(root, n))
    files.sort()
    if not files:
        print("no .sts file under %s - the shipped corpus is absent here; this "
              "table cannot be produced (and a silent empty one would be the "
              "vacuous pass C3 exists to prevent)" % corpus, file=sys.stderr)
        sys.exit(2)
    out = subprocess.run([scedit, "--grammar", grammar, "--check"] + files,
                         capture_output=True)
    if out.returncode > 1:
        print("scedit --check failed (rc %d): %s"
              % (out.returncode, ascii_only(out.stderr)[:400]), file=sys.stderr)
        sys.exit(2)
    if out.stderr.strip():
        print("scedit wrote to stderr: %s" % ascii_only(out.stderr)[:400], file=sys.stderr)
        sys.exit(2)
    rows = []
    for raw in out.stdout.splitlines():
        m = LINE_RE.match(raw)
        if not m:
            print("unparsed --check line: %s" % ascii_only(raw)[:200], file=sys.stderr)
            sys.exit(2)
        path = m.group(1).decode("ascii", "backslashreplace")
        rel = os.path.relpath(path, corpus)
        rows.append((rel, int(m.group(2)), m.group(5).decode("ascii"),
                     m.group(3).decode("ascii"), ascii_only(m.group(4))))
    return files, rows


def twins(files, corpus):
    """md5 -> the group of identical files, as corpus-relative paths."""
    by = collections.defaultdict(list)
    for f in files:
        with open(f, "rb") as fh:
            by[hashlib.md5(fh.read()).hexdigest()].append(os.path.relpath(f, corpus))
    return {p: sorted(set(g) - {p}) for g in by.values() for p in g if len(g) > 1}


# ---------------------------------------------------------- the dispositions
# Each row: (defect id, lint id, matcher, disposition, route, ground).
#
# The matcher is (rel-path, line, message) -> bool.  FIRST match wins, so the
# order below is part of the data: a narrow row must precede the broad one it
# carves out of.  `ground` names the engine text that was READ (post-F75
# anchors, code e3afca8f) or the live observation that settled it - never a
# plausibility.
DEFECTS = []


def disposition(rel, line, msg):
    for d in DEFECTS:
        if d[2](rel, line, msg):
            return d
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scedit", default=DEFAULT_SCEDIT)
    ap.add_argument("--grammar", default=DEFAULT_GRAMMAR)
    ap.add_argument("--corpus", default=DEFAULT_CORPUS)
    ap.add_argument("--strict", action="store_true",
                    help="exit 1 if any finding is UNADJUDICATED")
    a = ap.parse_args()
    for p in (a.scedit, a.grammar):
        if not os.path.exists(p):
            print("missing: " + p, file=sys.stderr)
            return 2
    if not os.path.isdir(a.corpus):
        print("the shipped corpus is not installed at %s - SKIPPING LOUDLY "
              "rather than reporting an empty table" % a.corpus, file=sys.stderr)
        return 2

    files, rows = census(a.scedit, a.grammar, a.corpus)
    tw = twins(files, a.corpus)

    print("\t".join(["file", "line", "id", "severity", "defect", "disposition",
                     "route", "twins", "ground", "message"]))
    by_disp = collections.Counter()
    by_id = collections.Counter()
    unjudged = 0
    for rel, line, ident, sev, msg in rows:
        d = disposition(rel, line, msg)
        if d is None:
            unjudged += 1
            defect, disp, route, ground = "-", "UNADJUDICATED", "-", "-"
        else:
            defect, disp, route, ground = d[0], d[3], d[4], d[5]
        by_disp[disp] += 1
        by_id[ident] += 1
        print("\t".join([rel, str(line), ident, sev, defect, disp, route,
                         ",".join(tw.get(rel, [])) or "-", ground, msg]))

    print("# %d findings over %d files in %d scripts; by id: %s; by disposition: %s"
          % (len(rows), len({r[0] for r in rows}), len(files),
             " ".join("%s=%d" % kv for kv in sorted(by_id.items())),
             " ".join("%s=%d" % kv for kv in sorted(by_disp.items()))),
          file=sys.stderr)
    if unjudged:
        print("# %d finding(s) carry no disposition row" % unjudged, file=sys.stderr)
        if a.strict:
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
