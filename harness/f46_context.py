#!/usr/bin/env python3
"""F46 -- context digest for the assert census (task F46, sec 5.89's owed sweep).

Takes the comment-stripped census produced by f44_census.py (reused verbatim, its
self-test re-run the same day) and prints, for every hit, N lines of context around
the site so each member can be CLASSIFIED FROM SOURCE rather than from the one-line
predicate. The site line is marked '>>'.

Usage:
    f46_context.py <before> <after> <file:line> [<file:line> ...]
    f46_context.py --from-census <before> <after>      # read file:line pairs on stdin
"""
import sys

ROOT = "/home/claude/spacecrafter/src"


def dump(path, line, before, after):
    full = path if path.startswith('/') else ROOT + '/' + path[len('src/'):] \
        if path.startswith('src/') else path
    try:
        lines = open(full, 'r', encoding='utf-8', errors='replace').read().split('\n')
    except OSError as e:
        print("!! %s: %s" % (full, e))
        return
    lo = max(1, line - before)
    hi = min(len(lines), line + after)
    print("======== %s:%d ========" % (path, line))
    for n in range(lo, hi + 1):
        print("%s %5d  %s" % ('>>' if n == line else '  ', n, lines[n - 1]))
    print()


def main(argv):
    if argv[0] == '--from-census':
        before, after = int(argv[1]), int(argv[2])
        sites = [s.strip() for s in sys.stdin if s.strip()]
    else:
        before, after = int(argv[0]), int(argv[1])
        sites = argv[2:]
    for s in sites:
        path, _, ln = s.rpartition(':')
        dump(path, int(ln), before, after)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
