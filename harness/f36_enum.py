#!/usr/bin/env python3
"""F36 / INTENT §5.77 — enumerate the app's DIRECT-TO-CONSOLE report sites in `src/`.

§5.77 owes *"an enumeration of the other startup paths that report only on
stderr"*. This is that enumeration's producer. It is a source-level census, and
it is deliberately the ONLY authority for the site table: the reachability probe
(`f36_reach.py`) and the classification table (`f36_class.py`) both read the JSON
this writes, so a site cannot be measured that was never enumerated, and a
classification cannot name a site that does not exist (I2).

THREE THINGS IT DOES THAT A `grep` DOES NOT, each learned by getting it wrong:

 1. **It strips comments before matching.** A first pass over `src/` with plain
    `grep std::cerr\\|std::cout` returned 335 hits; 166 of them are commented-out
    debug prints, and 2 more (`checkConfig.cpp:505,508`) sit inside a `/* ... */`
    block that a per-line `//` test cannot see. Counting them would have inflated
    the class this row exists to SIZE by more than 2x. The stripper is a real
    character-state machine (code / line-comment / block-comment / string / char)
    because `std::cout << "// not a comment"` must survive and
    `// std::cout << "x"` must not.

 2. **It matches every console channel, not just iostreams.** The site that
    started this row is `std::cerr`, but the star-catalogue loader
    (`ZoneArray::create`) reports its failures with bare `printf` and one
    `fprintf(stderr, ...)`, and `main` reports a failed display-mode query with
    `SDL_Log`. An enumeration restricted to `std::cerr`/`std::cout` would have
    missed the single largest cluster in the class and mis-sized the row.
    `sprintf`/`snprintf`/`fprintf(<file>, ...)` are NOT console channels and are
    excluded by the negative lookbehind / explicit stream argument.

 3. **It records the enclosing function**, because the unit the reachability
    probe can measure is a function entry, not a line: a failure-report line does
    not execute on a healthy startup, so "did this line run" answers nothing,
    while "was the function that contains it entered during startup" is exactly
    the question §5.77 asks.

EntityCore sites are enumerated but tagged `entitycore`: that submodule is
read-only for this task, so they are part of the SIZING and not of any fix.

Usage:  ./f36_enum.py [-o artifacts/f36/f36_sites.json]
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path

SRC = Path("/home/claude/spacecrafter/src")
REPO = Path("/home/claude/spacecrafter")
EXT = (".cpp", ".hpp", ".h", ".inl")

# A console channel. `printf`/`puts`/`perror` carry a negative lookbehind so
# `sprintf`, `snprintf`, `vsnprintf`, `obj.printf` and `p->puts` do not match;
# `fprintf` is a channel only when its stream argument is stderr/stdout.
CHANNEL = re.compile(
    r'std::cerr'
    r'|std::cout'
    r'|fprintf\s*\(\s*stderr'
    r'|fprintf\s*\(\s*stdout'
    r'|(?<![\w.>:])printf\s*\('
    r'|(?<![\w.>:])perror\s*\('
    r'|(?<![\w.>:])puts\s*\('
    r'|SDL_Log\s*\('
)

FUNC = re.compile(r'^[^(]*?([A-Za-z_~][A-Za-z0-9_]*(?:::[A-Za-z0-9_~]+)*)\s*\(')


def strip_comments(text: str) -> str:
    """Blank out comments, preserving line count and column count.

    Character-state machine over code / `//` / `/* */` / string / char literal.
    Comments become spaces (not removed) so that every reported line number is
    the line number in the ORIGINAL file — an enumeration whose line numbers do
    not open in an editor is not evidence.
    """
    out = []
    i, n, state = 0, len(text), 0  # 0 code, 1 line, 2 block, 3 string, 4 char
    while i < n:
        c = text[i]
        d = text[i + 1] if i + 1 < n else ''
        if state == 0:
            if c == '/' and d == '/':
                state = 1; out.append('  '); i += 2; continue
            if c == '/' and d == '*':
                state = 2; out.append('  '); i += 2; continue
            if c == '"':
                state = 3; out.append(c); i += 1; continue
            if c == "'":
                state = 4; out.append(c); i += 1; continue
            out.append(c); i += 1; continue
        if state == 1:
            if c == '\n':
                state = 0; out.append('\n'); i += 1; continue
            out.append(' '); i += 1; continue
        if state == 2:
            if c == '*' and d == '/':
                state = 0; out.append('  '); i += 2; continue
            out.append('\n' if c == '\n' else ' '); i += 1; continue
        # string / char literal
        if c == '\\':
            out.append('  '); i += 2; continue
        if (state == 3 and c == '"') or (state == 4 and c == "'"):
            state = 0
        out.append(c); i += 1; continue
    return ''.join(out)


def enclosing_function(clean_lines, i):
    """Nearest preceding line that starts in column 0 and looks like a
    definition. Crude by design: it is a LABEL for the reader and the seed for
    the gdb spec, and every spec the probe actually uses is checked at
    resolution time (`f36_reach.py` reports pending breakpoints), so a wrong
    guess here shows up as an unresolved location rather than as a silent hole.
    """
    for j in range(i, -1, -1):
        lj = clean_lines[j]
        if lj and not lj[0].isspace() and "(" in lj and not lj.startswith("#"):
            m = FUNC.match(lj)
            if m:
                return m.group(1), j + 1
    return "?", -1


def enumerate_sites():
    rows = []
    for dirpath, _dirnames, filenames in os.walk(SRC):
        for fn in sorted(filenames):
            if not fn.endswith(EXT):
                continue
            p = Path(dirpath) / fn
            rel = str(p.relative_to(REPO))
            raw = p.read_text(encoding="utf-8", errors="replace")
            clean = strip_comments(raw)
            raw_lines = raw.split("\n")
            clean_lines = clean.split("\n")
            for i, line in enumerate(clean_lines):
                m = CHANNEL.search(line)
                if not m:
                    continue
                func, funcline = enclosing_function(clean_lines, i)
                # A site inside a class-body inline member has no column-0
                # definition above it, so the scan yields nothing. Fall back to
                # the site's own line and SAY SO: a breakpoint there measures
                # "this report fired", not "the function was entered", and the
                # two are different facts that must not be silently merged.
                kind = "definition"
                if funcline < 0:
                    funcline, kind = i + 1, "site-fallback"
                rows.append({
                    "file": rel,
                    "line": i + 1,
                    "func": func,
                    "func_line": funcline,
                    "func_line_kind": kind,
                    "chan": m.group(0).split("(")[0].strip(),
                    "text": raw_lines[i].strip(),
                    "entitycore": rel.startswith("src/EntityCore"),
                })
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--out", default="artifacts/f36/f36_sites.json")
    a = ap.parse_args()
    rows = enumerate_sites()
    out = Path(a.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(rows, indent=1))

    proj = [r for r in rows if not r["entitycore"]]
    ec = [r for r in rows if r["entitycore"]]
    print(f"live console-output sites: {len(rows)}  "
          f"(project {len(proj)}, EntityCore {len(ec)})")
    by_chan = {}
    for r in proj:
        by_chan[r["chan"]] = by_chan.get(r["chan"], 0) + 1
    for k in sorted(by_chan, key=lambda k: -by_chan[k]):
        print(f"  {by_chan[k]:4d}  {k}")
    funcs = sorted({(r["file"], r["func"]) for r in proj})
    print(f"distinct enclosing functions (project): {len(funcs)}")
    print(f"-> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
