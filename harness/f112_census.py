#!/usr/bin/env python3
"""F112 -- the concurrent-instance census, counting PROBE SITES rather than files.

The task section's census pattern

  grep -lE '= "spacecrafter"|== "spacecrafter"|-x .spacecrafter. /proc' *.sh *.py

returns a FILE COUNT (44 on 2026-09-12), and it is not specific: it matches a
comment, an SDL window title, a log-channel name and a `scedit --history` row as
readily as a /proc probe.  It also cannot fall after the routing, because the
routed files carry a comment that quotes the very test they no longer perform.

This instrument answers the question the pattern was standing in for: WHICH FILES
STILL DECIDE, ON THEIR OWN, WHETHER AN ENGINE IS RUNNING, and by what.

  python3 f112_census.py            the table
  python3 f112_census.py --tsv      machine-readable, one row per file
  python3 f112_census.py --self-test

CLASSES
  ROUTED     the file asks the one home (sc_instances / sc_gpu) and decides nothing
  PROBE      the file has its own /proc test -- a site the home does not own
  MENTION    the pattern matches, but not a process probe (a comment, a window
             title, a channel name, a history row, a binary-name precondition)

PARTITION (live / frozen), the rule, applied in partition.tsv:
  LIVE    a future task can RUN it to take a NEW measurement.  Any of:
          (a) reached from a standing entry point -- the canary f56_canary.sh
              (fable-dispatch.md 0.5's enumerated precondition), the smoke suite
              f90_rehearsal_run.sh (doc/developer-entry.md 5's one command), the
              standing parity gate f91_run.sh (re-run by F104/F105/F108/F114/F116),
              or the shared python library f96_offset.py (8 importers);
          (b) a Sec.11 entry NUMBERED AFTER its own delivering entry records it run;
          (c) it belongs to the current era -- delivered at Sec.11.213 (F91) or
              later -- in which every task measurably re-runs its predecessors'
              drivers.
  FROZEN  everything else: a one-shot campaign driver whose artifacts are landed.
          Its bytes stay as they were when those artifacts were produced, so the
          landed record stays reproducible.  It gains exactly ONE comment block
          naming the home and nothing else.
"""

import re
import sys
from pathlib import Path

H = Path(__file__).resolve().parent

#: the criterion's own home and the two instruments that exist to measure it.
#: They read /proc on purpose -- sc_instances.py IS the probe, f112_decoymap.py
#: carries a verbatim copy of the PRE-F112 form so the blind spot can be mapped,
#: and this file's own regex literals match the census pattern.  Classing them
#: with the drivers would make the table say the opposite of the truth.
HOMES = {"sc_instances.py", "sc_instances.sh", "sc_gpu.py",
         "f112_census.py", "f112_decoymap.py"}

CENSUS_RE = re.compile(r'= "spacecrafter"|== "spacecrafter"|-x .spacecrafter. /proc')
ROUTED_RE = re.compile(r'sc_instances|sc_gpu')
# a line that actually reads /proc to decide, rather than talking about doing so
PROBE_RE = re.compile(
    r'(/proc/\[0-9\]\*|/proc")|Path\("/proc"\)|os\.listdir\("/proc"\)'
    r'|glob\.glob\("/proc|/proc/%s/comm')


def strip_comments(text, is_py):
    out = []
    in_doc = False
    for ln in text.splitlines():
        s = ln.strip()
        if is_py:
            q = s.count('"""') + s.count("'''")
            if in_doc:
                if q:
                    in_doc = False
                continue
            if s.startswith(('"""', "'''")):
                if q < 2:
                    in_doc = True
                continue
            if q and not s.startswith("#"):
                # a docstring opening mid-line; treat the rest as prose
                if q % 2:
                    in_doc = True
                continue
            if s.startswith("#"):
                continue
        else:
            if s.startswith("#"):
                continue
        out.append(ln)
    return "\n".join(out)


def classify(path):
    text = path.read_text(errors="replace")
    is_py = path.suffix == ".py"
    code = strip_comments(text, is_py)
    hits = [(i + 1, l) for i, l in enumerate(text.splitlines())
            if CENSUS_RE.search(l)]
    code_hits = [l for l in code.splitlines() if CENSUS_RE.search(l)]
    probe = [l for l in code.splitlines() if PROBE_RE.search(l)]
    routed = bool(ROUTED_RE.search(code))
    # a PROBE site = a /proc read in CODE that decides on the name
    own_probe = bool(code_hits) and bool(probe)
    if path.name in HOMES:
        cls = "HOME"
    elif own_probe:
        cls = "PROBE"
    elif routed:
        cls = "ROUTED"
    else:
        cls = "MENTION"
    return cls, len(hits), len(code_hits), hits


def main(argv):
    if "--self-test" in argv:
        return self_test()
    tsv = "--tsv" in argv
    files = sorted([p for p in H.glob("*.sh")] + [p for p in H.glob("*.py")])
    rows = []
    for p in files:
        text = p.read_text(errors="replace")
        if not CENSUS_RE.search(text) and not ROUTED_RE.search(text):
            continue
        cls, nhits, ncode, hits = classify(p)
        if not CENSUS_RE.search(text) and cls == "ROUTED":
            cls = "ROUTED"          # routed and no longer quoting the old test
        rows.append((cls, p.name, nhits, ncode, hits))
    counts = {}
    for cls, name, nhits, ncode, hits in rows:
        counts[cls] = counts.get(cls, 0) + 1
    if tsv:
        print("class\tfile\tpattern_hits\tcode_hits")
        for cls, name, nhits, ncode, _ in rows:
            print("%s\t%s\t%d\t%d" % (cls, name, nhits, ncode))
    else:
        for cls in ("PROBE", "ROUTED", "MENTION", "HOME"):
            print("=== %s: %d file(s)" % (cls, counts.get(cls, 0)))
            for c, name, nhits, ncode, hits in rows:
                if c != cls:
                    continue
                print("  %-24s pattern hits %d, of which in code %d"
                      % (name, nhits, ncode))
                if cls == "PROBE":
                    for ln, txt in hits:
                        print("      :%-5d %s" % (ln, txt.strip()[:88]))
    print("--- %d file(s) matched by the census pattern or naming the home; "
          "PROBE %d / ROUTED %d / MENTION %d / HOME %d"
          % (len(rows), counts.get("PROBE", 0), counts.get("ROUTED", 0),
             counts.get("MENTION", 0), counts.get("HOME", 0)))
    return 0


def self_test():
    """The classifier on fixtures, so a wrong class is visible without /proc."""
    import tempfile
    cases = [
        ("probe_sh", ".sh",
         'n=0\nfor p in /proc/[0-9]*; do [ "$(cat "$p/comm")" = "spacecrafter" ] '
         '&& n=$((n+1)); done\n', "PROBE"),
        ("routed_sh", ".sh",
         '# the old form was `comm == "spacecrafter"`\n'
         'bash "$HERE/sc_instances.sh" --assert x || exit 2\n', "ROUTED"),
        ("mention_sh", ".sh", '# a comment saying comm == "spacecrafter"\n'
         'echo hi\n', "MENTION"),
        ("probe_py", ".py",
         'import glob\ndef f():\n    for c in glob.glob("/proc/[0-9]*/comm"):\n'
         '        if open(c).read().strip() == "spacecrafter":\n'
         '            return c\n', "PROBE"),
        ("routed_py", ".py",
         'import sc_instances\ndef f():\n    """the old test was\n'
         '    == "spacecrafter" """\n    return sc_instances.no_instance()\n',
         "ROUTED"),
        ("title_py", ".py", 'WIN_NAME = "spacecrafter"   # SDL window title\n',
         "MENTION"),
    ]
    P = F = 0
    with tempfile.TemporaryDirectory() as d:
        for name, ext, body, want in cases:
            p = Path(d) / (name + ext)
            p.write_text(body)
            got, _, _, _ = classify(p)
            if got == want:
                P += 1
                print("  PASS %-12s -> %s" % (name, got))
            else:
                F += 1
                print("  FAIL %-12s -> %s, expected %s" % (name, got, want))
    print("  %d PASS %d FAIL" % (P, F))
    return 0 if F == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
