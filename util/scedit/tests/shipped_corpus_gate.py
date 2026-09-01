#!/usr/bin/env python3
"""The SHIPPED half of C3's corpus gate: `scedit --check` over the installed
script package, against a record that was dispositioned finding by finding.

C3 says a lint rule ships only when the shipped corpus produces zero FALSE
positives, and that the true findings are recorded upstream rather than
silenced.  The tracked half of that gate (`corpus_gate`) has run since
2026-08-04 over doc/superscript.sts and the harness scripts.  The shipped half
could not: `SCEDIT_CORPUS` carried a note saying the script directory was empty
on the machine where the gate was written.  It is not empty any more, and every
one of its 1661 findings now has a disposition (F76, 2026-09-01).

WHAT IS RECORDED, AND WHY IT IS COUNTS AND NOT LINES.  Two shapes were on the
table.  (a) the 1661 output lines, the way corpus-expected.txt records its 15.
(b) per-file per-id COUNTS plus the totals and the package's own identity.
This is (b), and the reasons are in util/scedit/README.md so they can be
argued with; the short form is that (a) would copy the key names and values of
another person's shows into this repository, and that the line-level record
already exists where it belongs - the disposition table, which carries every
finding WITH the engine ground it was judged on.  What (b) cannot see is one
finding replaced by another of the same id in the same file; the table sees
that, and `f76_corpus.py --strict` is the check that fails on it.

ABSENT CORPUS IS A SKIP, AND A LOUD ONE.  The script package is untracked field
data: it exists on machines that have it installed and nowhere else.  A gate
that silently passes where its input is missing is the vacuous green C3 exists
to prevent (F70's exclude-list precedent), so the absent case exits 77 - which
CMake's SKIP_RETURN_CODE turns into a REPORTED skip, visible in the ctest
summary, never a pass.  A corpus that is PRESENT but different fails, on
purpose: these expectations describe one data package, named by its aggregate
md5, and a different one has not been dispositioned.

    python3 tests/shipped_corpus_gate.py <scedit> <grammar> <expected> [corpus]
    python3 tests/shipped_corpus_gate.py ... --record     # rewrite the record

Exit 0 clean / 1 the record and the corpus disagree / 2 cannot run / 77 no
corpus installed here.
"""

import hashlib
import os
import re
import subprocess
import sys

SKIP = 77
LINE_RE = re.compile(rb"^(.*?):(\d+): (warning|error): (.*) \[-W([a-z-]+)\]$")


def render(scedit, grammar, corpus):
    files = []
    for root, _d, names in os.walk(corpus):
        files += [os.path.join(root, n) for n in names if n.endswith(".sts")]
    files.sort()
    if not files:
        return None, "no .sts file under %s" % corpus
    agg = hashlib.md5()
    for f in files:
        with open(f, "rb") as fh:
            agg.update(("%s %s\n" % (os.path.relpath(f, corpus),
                                     hashlib.md5(fh.read()).hexdigest())).encode())
    out = subprocess.run([scedit, "--grammar", grammar, "--check"] + files,
                         capture_output=True)
    if out.returncode > 1 or out.stderr.strip():
        return None, ("scedit --check exited %d / stderr %r"
                      % (out.returncode, out.stderr[:200]))
    per, by_id, total = {}, {}, 0
    for raw in out.stdout.splitlines():
        m = LINE_RE.match(raw)
        if not m:
            return None, "unparsed --check line: %r" % raw[:160]
        rel = os.path.relpath(m.group(1).decode("ascii", "backslashreplace"), corpus)
        ident = m.group(5).decode("ascii")
        per[(rel, ident)] = per.get((rel, ident), 0) + 1
        by_id[ident] = by_id.get(ident, 0) + 1
        total += 1
    body = [
        "# shipped-corpus record -- counts, not a silencer (README, Verification).",
        "# Every one of these findings is dispositioned line by line, with the engine",
        "# code each judgment was read at, in the harness repository at",
        "#   claude/harness/artifacts/f76/dispositions.tsv.gz  (generator: f76_corpus.py)",
        "# and routed to the script-surface owner as SS-25 and SS-32..SS-39.",
        "# A count that moves means a shipped show changed or a rule did: look at which,",
        "# re-disposition, and edit this file deliberately.",
        "#   Regenerate with: python3 tests/shipped_corpus_gate.py <scedit> <grammar> "
        "<expected> [corpus] --record",
        "corpus.files            %d" % len(files),
        "corpus.aggregate_md5    %s" % agg.hexdigest(),
        "findings.total          %d" % total,
    ]
    body += ["findings.by-id          %-20s %d" % (k, by_id[k]) for k in sorted(by_id)]
    body += ["file %-46s %-20s %d" % (f, i, n) for (f, i), n in sorted(per.items())]
    return "\n".join(body) + "\n", None


def main(argv):
    record = "--record" in argv
    argv = [a for a in argv if a != "--record"]
    if not 3 <= len(argv) <= 4:
        print(__doc__)
        return 2
    scedit, grammar, expected = argv[0], argv[1], argv[2]
    corpus = argv[3] if len(argv) == 4 else os.path.expanduser("~/.spacecrafter/scripts")
    for p in (scedit, grammar):
        if not os.path.exists(p):
            print("missing: " + p)
            return 2
    if not os.path.isdir(corpus):
        print("SKIP: no shipped script package at %s. This gate checks the INSTALLED\n"
              "      scripts, which are field data and are not in this repository; on a\n"
              "      machine without them there is nothing to check and this is reported\n"
              "      as a skip rather than passed over in silence." % corpus)
        return SKIP
    got, err = render(scedit, grammar, corpus)
    if got is None:
        if err.startswith("no .sts file"):
            print("SKIP: %s - the directory exists but holds no script; same reasoning "
                  "as an absent package." % err)
            return SKIP
        print("FAIL: " + err)
        return 2
    if record:
        with open(expected, "w") as fh:
            fh.write(got)
        print(got, end="")
        return 0
    if not os.path.exists(expected):
        print("missing record: " + expected)
        return 2
    with open(expected) as fh:
        want = fh.read()
    if want.strip() == got.strip():
        print("shipped corpus gate: %d scripts match %s"
              % (int(re.search(r"corpus\.files\s+(\d+)", got).group(1)), expected))
        return 0
    w, g = want.strip().split("\n"), got.strip().split("\n")
    print("FAIL: the shipped corpus no longer matches %s" % expected)
    for line in sorted(set(w) ^ set(g)):
        print("  %s %s" % ("-" if line in w else "+", line))
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
