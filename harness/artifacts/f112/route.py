#!/usr/bin/env python3
"""F112 -- the routing pass, done mechanically so it can be read and re-run.

Every LIVE caller of the concurrent-instance probe is rewritten to call the one
home (sc_instances.sh for shell, sc_instances for python); every FROZEN one gets
exactly ONE comment line naming the home and keeps its bytes otherwise.

The partition rule and the per-file evidence are in partition.tsv beside this file.

  python3 route.py --check    report what each file needs, change nothing
  python3 route.py --apply    do it
"""
import re
import sys
from pathlib import Path

H = Path(__file__).resolve().parents[2]          # harness/

NOTE_SH = (
    '# ROUTED 2026-09-12 through the ONE home of the instance criterion (F112,\n'
    '# Sec.11.238): comm | /proc/<pid>/exe | TCP 7805, union.  The inline\n'
    '# `comm == "spacecrafter"` form this replaced was measured BLIND to a renamed or\n'
    '# copied engine (Sec.11.231(j2): it read 0 with two staging instances live and\n'
    '# holding port 7805), and it was copy-pasted into 44 files, so no single edit\n'
    '# could fix it.  sc_instances.sh --assert prints pid . uid . comm . exe . port\n'
    '# per hit and exits 0 clear / 2 engine live / 4 the probe could not run.\n')

FROZEN_NOTE = (
    "FROZEN 2026-09-12 (F112, Sec.11.238): this driver's artifacts are landed, so its\n"
    "bytes stay as they were when they were produced.  The instance probe below is the\n"
    "pre-F112 `comm == \"spacecrafter\"` form and is BLIND to a renamed or copied engine\n"
    "(Sec.11.231(j2)).  If you re-run this driver, assert with the one home instead:\n"
    "bash harness/sc_instances.sh --assert <label>  (or `from sc_instances import\n"
    "no_instance`), which also covers /proc/<pid>/exe and TCP 7805.")

# ---- the LIVE shell files whose probe is the HITS="" loop, and their labels
SHAPE_A = {
    "f90_rehearsal_run.sh": "f90-smoke",
    "f91_run.sh": "f91",
    "f94_run.sh": "f94",
    "f96_run.sh": "f96",
    "f97_run.sh": "f97",
    "f99_run.sh": "f99",
    "f100_run.sh": "f100",
}
RE_A = re.compile(
    r'(?:# --- concurrent instance[^\n]*\n)?'
    r'HITS=""\n'
    r'for p in /proc/\[0-9\]\*/comm; do\n'
    r'    \[ -r "\$p" \] \|\| continue\n'
    r'    if \[ "\$\(cat "\$p" 2>/dev/null\)" = "spacecrafter" \]; then '
    r'HITS="\$HITS \$\{p%/comm\}"; fi\n'
    r'done\n'
    r'if \[ -n "\$HITS" \]; then\n'
    r'    echo "ABORT: another spacecrafter process exists:\$HITS"; exit 2\n'
    r'fi\n'
    r'echo "    /proc comm assert: no spacecrafter running"\n')

# ---- the LIVE python files: replace the function BODY, keep its name and
#      return shape, so no caller has to change (the F108 logread precedent).
PY_LIVE = {
    "f96_offset.py": ("no_instance", "list"),
    "f90_rehearsal.py": ("no_instance", "list"),
    "f91_parity.py": ("no_instance", "list"),
    "f94_bodyselect.py": ("no_instance", "list"),
    "f95_soak.py": ("no_instance", "list"),
    "f27_reply.py": ("concurrent_instances", "list"),
}

FROZEN = [
    "f29_run.sh", "f30_alloc.sh", "f38_config.sh", "f38_run.sh",
    "f38_scenes.sh", "f39_asmooth_probe.sh", "f39_farm.sh", "f39_scenes.sh",
    "f40_scenes.sh", "f41_scenes.sh",
    "f39_d21.py", "f41_ownership.py", "f55_probe.py", "f55_probe2.py",
    "f55_sampler.py", "f67_tcp_live.py", "f68_provenance.py",
    "f69_feedback.py", "f81_offset.py", "f84_coldhome.py",
    "f86_startup.py",
]
# the line each frozen file's note is anchored ABOVE: its own probe's first line
FROZEN_ANCHOR = re.compile(
    r'^(?P<indent>[ \t]*)(?P<line>.*(?:n=0; for p in /proc|^n=0$'
    r'|CONC=\$\(/usr/bin/grep -l -x'
    r"|def concurrent\(|def no_instance\(|def comm_probe\(|def running_instances\("
    r'|def concurrent_instances\(|def instance_pids\().*)$', re.M)


def route_shape_a(text, name, label):
    call = (NOTE_SH +
            'bash "$HERE/sc_instances.sh" --assert %s || exit 2\n' % label)
    new, n = RE_A.subn(call, text)
    return new, n


def py_block(lines, start):
    """The extent of the def that STARTS at index `start`, by indentation --
    never by regex.  The first version of this file used a regex with a
    lookahead and swallowed 901 lines of f96_offset.py; the diffstat caught it
    before it was committed, and the lesson is that a python block is defined by
    its indentation and has to be read that way."""
    i = start + 1
    end = i
    while i < len(lines):
        ln = lines[i]
        if ln.strip() == "":
            i += 1
            continue
        if ln[0] in " \t":
            end = i + 1
            i += 1
            continue
        break                       # a line at column 0: the def is over
    return end


def route_python(text, fname, fn):
    """Replace the body of `fn` with a delegation to the home."""
    lines = text.splitlines(True)
    head = re.compile(r'^def %s\(\):\s*$' % re.escape(fn))
    idx = [i for i, l in enumerate(lines) if head.match(l)]
    if len(idx) != 1:
        return text, 0
    start = idx[0]
    end = py_block(lines, start)
    body = (
        'def %s():\n'
        '    """ROUTED 2026-09-12 to the ONE home of the instance criterion\n'
        '    (F112, Sec.11.238): comm | /proc/<pid>/exe | TCP 7805, union.\n\n'
        '    The inline `comm == "spacecrafter"` test this replaced was measured\n'
        '    BLIND to a renamed or copied engine (Sec.11.231(j2): 0 with two\n'
        '    staging instances live and holding the port).  The name and the\n'
        '    return shape are unchanged -- a list, empty when the host is clear --\n'
        '    so every caller keeps working and now gets a message that says which\n'
        '    channel fired.  The F108 `logread.py` precedent: one home, many\n'
        '    callers, and the callers do not have to know."""\n'
        '    return sc_instances.no_instance()\n' % fn)
    return "".join(lines[:start]) + body + "".join(lines[end:]), 1


IMPORT_BLOCK = (
    "# F112, Sec.11.238: the ONE home of the concurrent-instance criterion.  The\n"
    "# harness directory is inserted rather than assumed -- every importer of this\n"
    "# module already does the same (measured), and this makes the module work when\n"
    "# it is run directly too.\n"
    "import os as _f112o, sys as _f112s                                # noqa: E402\n"
    "_f112s.path.insert(0, _f112o.path.dirname(_f112o.path.abspath(__file__)))\n"
    "import sc_instances                                               # noqa: E402\n")


def add_import(text):
    if "import sc_instances" in text:
        return text, 0
    lines = text.splitlines(True)
    last = max(i for i, l in enumerate(lines)
               if re.match(r'^(import |from )\S', l))
    return ("".join(lines[:last + 1]) + IMPORT_BLOCK
            + "".join(lines[last + 1:])), 1


def annotate_frozen(text, name):
    if "FROZEN 2026-09-12 (F112" in text:
        return text, 0
    m = FROZEN_ANCHOR.search(text)
    if not m:
        return text, 0
    comment = "#" if name.endswith(".sh") else "#"
    block = "".join("%s%s %s\n" % (m.group("indent"), comment, ln)
                    for ln in FROZEN_NOTE.splitlines())
    return text[:m.start()] + block + text[m.start():], 1


def main(argv):
    apply = "--apply" in argv
    report = []
    for name, label in SHAPE_A.items():
        p = H / name
        t = p.read_text()
        new, n = route_shape_a(t, name, label)
        report.append(("LIVE-sh", name, n))
        if apply and n:
            p.write_text(new)
    for name, (fn, _shape) in PY_LIVE.items():
        p = H / name
        t = p.read_text()
        new, n = route_python(t, name, fn)
        if n:
            new, _ = add_import(new)
        report.append(("LIVE-py", name, n))
        if apply and n:
            p.write_text(new)
    for name in FROZEN:
        p = H / name
        t = p.read_text()
        new, n = annotate_frozen(t, name)
        report.append(("FROZEN", name, n))
        if apply and n:
            p.write_text(new)
    bad = 0
    for kind, name, n in report:
        print("%-8s %-24s %s" % (kind, name, "ok" if n else "NOT MATCHED"))
        bad += 0 if n else 1
    print("%d file(s), %d not matched" % (len(report), bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
