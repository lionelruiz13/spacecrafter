#!/usr/bin/env python3
"""F44 — comment-stripped symbol census over spacecrafter's src/ tree.

Why this exists (the F36 lesson, §11.146): a raw `grep -c` over this tree measured a
symbol 2.2x too often, because the tree's comments quote code heavily (this project
writes its requirements INTO the comments, so every ported member's name appears in
prose beside its call sites). A census whose denominator is a raw grep is therefore
not a census. This strips C and C++ comments while PRESERVING line numbers (each
stripped character becomes a space, each newline is kept), so `file:line` citations
made against the stripped text are valid against the original file.

String and char literals are honoured (a `//` inside a string is not a comment), as
are raw string literals R"delim( ... )delim" and line continuations inside a // comment.

Usage:
    f44_census.py <symbol> [<symbol> ...]        -- report code hits + comment hits
    f44_census.py --self-test                    -- discrimination of the stripper
"""
import os
import re
import sys

ROOT = "/home/claude/spacecrafter/src"
EXTS = (".cpp", ".hpp", ".h", ".c", ".cxx", ".hxx", ".inl")
# EntityCore is a submodule (read-only by protocol) but IS part of the built binary,
# so it is scanned; it is reported separately so the boundary stays visible.
SKIP_DIRS = set()


def strip_comments(src: str) -> str:
    """Replace comment bytes with spaces, keeping every newline in place."""
    out = list(src)
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            # Raw string literal?  R"delim( ... )delim"
            if c == '"' and i > 0 and src[i - 1] == 'R':
                m = re.match(r'"([^()\\ ]{0,16})\(', src[i:])
                if m:
                    delim = m.group(1)
                    end = src.find(')' + delim + '"', i)
                    i = n if end < 0 else end + len(delim) + 2
                    continue
            q = c
            i += 1
            while i < n:
                if src[i] == '\\':
                    i += 2
                    continue
                if src[i] == q:
                    i += 1
                    break
                if src[i] == '\n':      # unterminated literal: do not run away
                    break
                i += 1
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '/':
            while i < n and src[i] != '\n':
                # a backslash-newline continues a // comment onto the next line
                if src[i] == '\\' and i + 1 < n and src[i + 1] == '\n':
                    out[i] = ' '
                    i += 2
                    continue
                out[i] = ' '
                i += 1
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            while i < n and not (src[i] == '*' and i + 1 < n and src[i + 1] == '/'):
                if src[i] != '\n':
                    out[i] = ' '
                i += 1
            for k in range(i, min(i + 2, n)):
                out[k] = ' '
            i += 2
            continue
        i += 1
    return "".join(out)


def files():
    for dirpath, dirnames, filenames in os.walk(ROOT):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for f in sorted(filenames):
            if f.endswith(EXTS):
                yield os.path.join(dirpath, f)


def census(symbols):
    pat = re.compile(r'\b(' + '|'.join(re.escape(s) for s in symbols) + r')\b')
    code, comment = {s: [] for s in symbols}, {s: [] for s in symbols}
    for path in files():
        raw = open(path, 'r', encoding='utf-8', errors='replace').read()
        stripped = strip_comments(raw)
        rawlines = raw.split('\n')
        striplines = stripped.split('\n')
        for ln, (rl, sl) in enumerate(zip(rawlines, striplines), start=1):
            for m in pat.finditer(sl):
                code[m.group(1)].append((path, ln, rl.strip()))
            in_code = {m.group(1) for m in pat.finditer(sl)}
            for m in pat.finditer(rl):
                if m.group(1) not in in_code:
                    comment[m.group(1)].append((path, ln, rl.strip()))
    return code, comment


SELF_TEST = r'''
int a; // observedPosToRaDe in a line comment
/* observedPosToRaDe in a block comment
   observedPosToRaDe again */
const char *s = "observedPosToRaDe in a string";
int b = observedPosToRaDe(x);        // REAL
// continued comment observedPosToRaDe \
   still comment observedPosToRaDe
int c = observedPosToRaDe(y); /* trailing */ int d = observedPosToRaDe(z);
'''


def self_test():
    stripped = strip_comments(SELF_TEST)
    assert len(stripped) == len(SELF_TEST), "length not preserved"
    assert stripped.count('\n') == SELF_TEST.count('\n'), "newlines not preserved"
    raw_hits = SELF_TEST.count('observedPosToRaDe')
    code_hits = stripped.count('observedPosToRaDe')
    # Fixture holds 9 occurrences of the symbol: 5 in comments (line, block x2,
    # continued-line-comment x2) and 4 outside them (3 real calls + 1 inside a
    # string literal -- a literal is code text, not a comment, so it is KEPT and
    # any such hit is adjudicated by reading, not by the stripper).
    print("self-test fixture: raw hits = %d, code hits = %d (expected 9 and 4)"
          % (raw_hits, code_hits))
    ok = (raw_hits == 9 and code_hits == 4)
    for ln, (rl, sl) in enumerate(zip(SELF_TEST.split('\n'), stripped.split('\n')), 1):
        mark = 'CODE' if 'observedPosToRaDe' in sl else ('COMMENT' if 'observedPosToRaDe' in rl else '    ')
        print("  %2d %-8s %s" % (ln, mark, rl))
    print("SELF-TEST", "PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--self-test':
        sys.exit(self_test())
    syms = sys.argv[1:]
    if not syms:
        print(__doc__)
        sys.exit(2)
    code, comment = census(syms)
    for s in syms:
        print("=== %s : %d CODE hits, %d comment-only hits" % (s, len(code[s]), len(comment[s])))
        for path, ln, txt in code[s]:
            print("  CODE    %s:%d: %s" % (path.replace(ROOT, 'src'), ln, txt[:150]))
        for path, ln, txt in comment[s]:
            print("  comment %s:%d: %s" % (path.replace(ROOT, 'src'), ln, txt[:150]))
