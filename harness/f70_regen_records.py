#!/usr/bin/env python3
"""f70_regen_records.py - re-record scedit's pinned outputs, deliberately.

    python3 f70_regen_records.py --scedit BIN [--write]

WHY THIS EXISTS.  F70's sweep transliterates the GRAMMAR prose (authored English
documentation in util/scedit/grammar/*.json, which scedit displays and quotes)
while it leaves C++ string literals byte-identical behind \\xNN escapes.  Five
records pin scedit's output, and that output is a MIXTURE of the two: one lint
message can carry an em dash that came from the grammar (now "--") beside one
that came from a code literal (still an em dash).  So these records cannot be
updated by running the map over them - that would convert the code-literal half
too, and the gate would go red against unchanged code.  They have to be
RE-RECORDED from the binary, and the diff read.

ui_gate.cmake says of its record: "Changing the layout means editing it
deliberately, never regenerating it blind."  This script is the deliberate half:
it prints the diff for a human to read before --write is passed, and the diff it
produced for F70 is committed beside it.

Each gate's GOT string is reproduced EXACTLY as its own .cmake builds it -
including check_gate's two path rewrites and every gate's final STRIP - because
a record written by a different recipe than the one that reads it is a gate that
passes for the wrong reason.
"""

import argparse
import difflib
import os
import subprocess
import sys

SRC = "/home/claude/spacecrafter/util/scedit"
GRAMMAR = os.path.join(SRC, "grammar/sc-grammar.json")


def run(scedit, args):
    p = subprocess.run([scedit, "--grammar", GRAMMAR] + args,
                       capture_output=True, text=True)
    if p.stderr:
        raise SystemExit("scedit %s wrote to stderr: %s" % (args, p.stderr))
    return p.stdout, p.returncode


def check_gate(scedit, mode, files):
    """check_gate.cmake: one run, then its two path rewrites, then STRIP."""
    present = [f for f in files if os.path.exists(f)]
    out, rc = run(scedit, mode + present)
    if rc > 1:
        raise SystemExit("scedit %s exited %d" % (mode, rc))
    out = out.replace(SRC + "/../../", "").replace(SRC + "/", "")
    return out.strip()


def query_gate(scedit, queries):
    got = []
    for line in open(queries, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        out, rc = run(scedit, line.split())
        got.append("$ scedit %s\n%sexit %d\n" % (line, out, rc))
    return "".join(got).strip()


def ui_gate(scedit):
    out, rc = run(scedit, ["--ui-selftest"])
    if rc != 0:
        raise SystemExit("scedit --ui-selftest exited %d" % rc)
    return out.strip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scedit", required=True)
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    t = lambda n: os.path.join(SRC, "tests", n)

    records = [
        ("lint-expected.txt",
         lambda: check_gate(args.scedit, ["--check"],
                            [t("lint_cases.sts")])),
        ("history-expected.txt",
         lambda: check_gate(args.scedit, ["--history"],
                            [t("lint_cases.sts"), t("history_cases.sts")])),
        ("check-json-expected.txt",
         lambda: check_gate(args.scedit, ["--check", "--json"],
                            [t("lint_cases.sts")])),
        # corpus-expected.txt is NOT re-recorded here: its inputs are the
        # external SCEDIT_CORPUS scripts, not tree fixtures, and its gate is
        # green - a record that still matches must not be rewritten.
        ("doc-expected.txt",
         lambda: query_gate(args.scedit, t("doc-queries.txt"))),
        ("ui-selftest-expected.txt", lambda: ui_gate(args.scedit)),
    ]

    total = 0
    for name, producer in records:
        path = t(name)
        if not os.path.exists(path):
            print("skip %s (not present)" % name)
            continue
        try:
            got = producer()
        except SystemExit as e:
            print("SKIP %-28s %s" % (name, e))
            continue
        orig = open(path, encoding="utf-8").read()
        want = orig.strip()
        # Both comparators STRIP their two sides, so the record's leading and
        # trailing whitespace is outside what any gate compares - which is
        # exactly why it must be PRESERVED here rather than normalised away.
        # (Measured: a first pass wrote `got + "\n"` and silently dropped a
        # trailing tab from the last line of history-expected.txt.  The gate
        # could not see it; the diff could, and a diff with an unexplained
        # byte in it is not a reviewable diff.)
        lead = orig[:len(orig) - len(orig.lstrip())]
        tail = orig[len(orig.rstrip()):]
        if got == want:
            print("unchanged  %s" % name)
            continue
        d = list(difflib.unified_diff(want.splitlines(), got.splitlines(),
                                      "recorded/" + name, "produced/" + name,
                                      lineterm="", n=0))
        total += sum(1 for l in d if l.startswith("+") and not l.startswith("+++"))
        print("\n".join(d))
        if args.write:
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(lead + got + tail)
            print("*** rewritten: %s" % name)
    print("---\n%d changed line(s) across the records" % total)
    return 0


if __name__ == "__main__":
    sys.exit(main())
