#!/usr/bin/env python3
"""f117_cites.py - resolve every file:line citation of a note against HEAD.

F117 discriminating check (a): "every file:line the note cites resolves at HEAD
to the text it claims". This script produces the resolution table that makes the
check checkable: for each citation it prints the note's own line number, the
citation as written, and the SOURCE LINE at that position in the working tree.
A reader (or the executor) then compares the claim with the text; a citation
whose file is missing or whose line is out of range FAILS here, mechanically.

Citation forms understood (they are the ones the ledger's grammar produces):
  path/file.cpp:123          an absolute-in-repo path with a line
  path/file.cpp:123-140      a range (both ends are resolved)
  [:123]  [:123, :140]       a bare line, resolved against the LAST file named
                             before it in the note (the ledger's shorthand)

Roots searched, in order: the code repo, then the harness repo.

Usage:  python3 f117_cites.py <note.md> [--code DIR] [--harness DIR]
Exit 0 = every citation resolved; exit 1 = at least one did not.
"""

import argparse
import os
import re
import sys

FILE_RE = re.compile(
    r"([A-Za-z0-9_][A-Za-z0-9_./+-]*\.(?:cpp|hpp|h|md|tsv|py|sh|dat|ini|geom|vert|frag|glsl|json|txt))"
    r":(\d+)(?:-(\d+))?")
BARE_RE = re.compile(r"[\[,]\s*:(\d+)(?:-(\d+))?")


SKIP_DIRS = {".git", "build-claude", "build", "__pycache__", "artifacts",
             "fable-dispatch", "INTENT", "sha-maps"}


def build_index(roots):
    """basename -> [absolute paths]. The ledger's citation grammar is a BASENAME
    plus a line (`core.cpp:366`), sometimes a tail path (`inGalaxyModule/dso3d.hpp`),
    so resolution is by suffix; an ambiguous basename is reported, never guessed."""
    index = {}
    for r in roots:
        for dirpath, dirs, files in os.walk(r):
            dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
            for name in files:
                index.setdefault(name, []).append(os.path.join(dirpath, name))
    return index


def resolve(path, roots, index):
    for r in roots:
        p = os.path.join(r, path)
        if os.path.isfile(p):
            return p, None
    cands = index.get(os.path.basename(path), [])
    if "/" in path:
        cands = [c for c in cands if c.endswith("/" + path)] or cands
    if len(cands) == 1:
        return cands[0], None
    if len(cands) > 1:
        return None, "AMBIGUOUS (%d): %s" % (len(cands), ", ".join(
            os.path.relpath(c, roots[0]) for c in sorted(cands)[:4]))
    return None, None


def readline_at(path, n):
    try:
        with open(path, "rb") as f:
            for i, line in enumerate(f, 1):
                if i == n:
                    return line.decode("iso-8859-1").rstrip("\n")
    except OSError:
        return None
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("note")
    ap.add_argument("--code", default="/home/claude/spacecrafter")
    ap.add_argument("--harness", default="/home/claude/spacecrafter/claude")
    a = ap.parse_args()
    roots = [a.code, a.harness]

    with open(a.note, "rb") as f:
        note = f.read().decode("iso-8859-1").split("\n")

    total = 0
    failed = 0
    last_file = None
    index = build_index(roots)
    print("== F117 citation resolution: %s ==" % a.note)
    print("roots: %s" % " | ".join(roots))
    print("")
    for lineno, text in enumerate(note, 1):
        hits = []
        for m in FILE_RE.finditer(text):
            hits.append((m.start(), m.group(1), int(m.group(2)),
                         int(m.group(3)) if m.group(3) else None))
        for m in BARE_RE.finditer(text):
            hits.append((m.start(), None, int(m.group(1)),
                         int(m.group(2)) if m.group(2) else None))
        hits.sort()
        for _pos, fname, n1, n2 in hits:
            if fname is None:
                if last_file is None:
                    continue
                fname = last_file
                shown = ":%d" % n1
            else:
                last_file = fname
                shown = "%s:%d" % (fname, n1)
            p, why = resolve(fname, roots, index)
            total += 1
            if p is None:
                failed += 1
                print("FAIL note:%-5d %-46s %s" % (lineno, shown,
                                                   why or "FILE NOT FOUND"))
                continue
            for n in ([n1] if n2 is None else [n1, n2]):
                src = readline_at(p, n)
                if src is None:
                    failed += 1
                    print("FAIL note:%-5d %-46s LINE %d OUT OF RANGE"
                          % (lineno, shown, n))
                else:
                    print("ok   note:%-5d %-46s %s" % (lineno, "%s:%d" % (fname, n),
                                                       src.strip()[:110]))
    print("")
    print("== %d citation(s), %d unresolved ==" % (total, failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
