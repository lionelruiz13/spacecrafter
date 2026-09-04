#!/usr/bin/env python3
"""C3's STELLAR-SYSTEM half, field side: `scedit --check` over the field data,
against a record that was dispositioned finding by finding (F80, 2026-09-04).

C3 (scedit/INTENT.md S2) asks for zero FALSE positives over the shipped corpus
before a rule ships, with the true findings recorded upstream rather than
silenced. It names two corpora, and only one of them was ever armed: the script
half landed 2026-09-01 (F76). The other clause -- "and the field
`~/.spacecrafter/ssystem.ini`" -- was still item 4's. This is that clause.

TWO GATES FOR THE STELLAR-SYSTEM CORPUS, AND THE SPLIT IS ABOUT OWNERSHIP.
`data/default_ssystem.ini` is tracked in this repository, so it gets the
line-for-line record shape (`ss_corpus_gate`, tests/ss-corpus-expected.txt) --
the strongest form available, and available precisely because that file is
ours. THIS gate covers what is not ours: `~/.spacecrafter/ssystem.ini` and the
machine-owned composed twins beside it, which are untracked, host-varying field
data. It records per-file per-id COUNTS plus each file's own md5, and never a
line of their content -- the same reasoning shipped_corpus_gate records counts
for another person's shows (README S Verification, veto-open), plus one reason
of its own: the field file is READ-ONLY to every task that touches it, and a
gate that copied it into the repository would create a second copy of data
whose whole point is that there is one (I2, and INTENT S2.0 D9).

The md5 row earns its place separately: when this gate fails it says whether
the DATA moved or the CHECKER did, which is the first question anyone asks.

ABSENT FIELD DATA IS A SKIP, AND A LOUD ONE. These files exist on a machine
with spacecrafter installed and nowhere else. A gate that greened on missing
input would report "checked" for a machine where nothing was, which is the
vacuous green C3 exists to prevent, so the absent case exits 77 -- CMake's
SKIP_RETURN_CODE turns that into a REPORTED skip in the ctest summary, never a
pass. Field data that is PRESENT but different FAILS on purpose: these
expectations describe one corpus, named by its md5, and a different one has not
been dispositioned.

    python3 tests/field_corpus_gate.py <scedit> <grammar> <expected>
    python3 tests/field_corpus_gate.py ... --record     # rewrite the record

Exit 0 clean / 1 the record and the corpus disagree / 2 cannot run / 77 no
field data on this machine.
"""

import glob
import hashlib
import os
import re
import subprocess
import sys

SKIP = 77

# NOTE the severity alternation carries `info`, which the script gate's twin
# does not need: the stellar-system rules report inert-but-true facts (a key no
# loader reads) at info, and a regex that silently dropped them would under-count
# by 187 on the shipped field file -- a gate passing because it could not see
# most of its subject.
LINE_RE = re.compile(rb"^(.*?):(\d+): (info|warning|error): (.*) \[-W([a-z-]+)\]$")


def corpus_files(home=None):
    """The field stellar-system corpus: the legacy file and the composed twins.

    Returned in a stable order so the record does not depend on readdir.
    """
    home = home or os.path.expanduser("~")
    sc = os.path.join(home, ".spacecrafter")
    out = []
    legacy = os.path.join(sc, "ssystem.ini")
    if os.path.exists(legacy):
        out.append(legacy)
    out += sorted(glob.glob(os.path.join(sc, "modularSystem", "*.ini")))
    out += sorted(glob.glob(os.path.join(sc, "modularSystem", "*.ini.disabled")))
    return out


def render(scedit, grammar, files):
    """Run the checker and reduce its output to the recorded shape."""
    if not files:
        return None, "no field stellar-system file under ~/.spacecrafter"
    rows = {}
    md5s = []
    for f in files:
        with open(f, "rb") as fh:
            md5s.append((os.path.basename(f), hashlib.md5(fh.read()).hexdigest()))
    p = subprocess.run([scedit, "--grammar", grammar, "--check"] + files,
                       capture_output=True)
    if p.returncode > 1:
        return None, "scedit exited %d: %s" % (p.returncode,
                                               p.stderr.decode("latin-1", "replace"))
    if p.stderr.strip():
        return None, "scedit wrote to stderr: %s" % p.stderr.decode("latin-1", "replace")
    unparsed = 0
    for line in p.stdout.split(b"\n"):
        if not line.strip():
            continue
        m = LINE_RE.match(line)
        if not m:
            unparsed += 1
            continue
        path, _n, _sev, _msg, lid = m.groups()
        key = (os.path.basename(path.decode("latin-1")), lid.decode("ascii"))
        rows[key] = rows.get(key, 0) + 1
    if unparsed:
        # A finding the record cannot represent must fail rather than vanish.
        return None, "%d diagnostic line(s) did not match the recorded shape" % unparsed

    out = []
    out.append("# field_corpus_gate record -- C3's stellar-system half, field side.")
    out.append("# COUNTS and md5s, never the lines: this is another person's data and")
    out.append("# this repository does not carry it (see the module docstring).")
    out.append("# Regenerate with: python3 tests/field_corpus_gate.py <scedit> <grammar>"
               " <expected> --record")
    out.append("")
    out.append("[files]")
    for name, h in md5s:
        out.append("%-34s %s" % (name, h))
    out.append("")
    out.append("[findings]")
    total = 0
    for (name, lid) in sorted(rows):
        out.append("%-34s %-28s %d" % (name, lid, rows[(name, lid)]))
        total += rows[(name, lid)]
    out.append("")
    out.append("[totals]")
    out.append("files                              %d" % len(files))
    out.append("findings                           %d" % total)
    return "\n".join(out) + "\n", None


def main():
    args = [a for a in sys.argv[1:] if a != "--record"]
    record = "--record" in sys.argv
    if len(args) < 3:
        sys.stderr.write(__doc__)
        return 2
    scedit, grammar, expected = args[0], args[1], args[2]
    files = corpus_files()
    if not files:
        sys.stderr.write("field_corpus_gate: SKIP -- no ~/.spacecrafter stellar-system "
                         "file on this machine; nothing was checked.\n")
        return SKIP
    got, err = render(scedit, grammar, files)
    if err:
        sys.stderr.write("field_corpus_gate: %s\n" % err)
        return 2
    if record:
        with open(expected, "w", encoding="ascii") as fh:
            fh.write(got)
        sys.stderr.write("field_corpus_gate: recorded %s\n" % expected)
        return 0
    try:
        with open(expected, encoding="ascii") as fh:
            want = fh.read()
    except OSError as e:
        sys.stderr.write("field_corpus_gate: cannot read %s: %s\n" % (expected, e))
        return 2
    if got.strip() != want.strip():
        sys.stderr.write("field_corpus_gate: the record and the field data disagree.\n")
        wl, gl = want.strip().split("\n"), got.strip().split("\n")
        import difflib
        for d in difflib.unified_diff(wl, gl, "recorded", "got", lineterm="", n=1):
            sys.stderr.write(d + "\n")
        return 1
    print("field_corpus_gate: %d file(s), record matches" % len(files))
    return 0


if __name__ == "__main__":
    sys.exit(main())
