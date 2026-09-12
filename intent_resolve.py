#!/usr/bin/env python3
"""intent_resolve.py -- turn a ledger id into the one place that holds it.

WHY THIS EXISTS
  The owner asked about four of his own tracker's rows on 2026-09-12 and could
  not find them: "many others are genuinely in the INTENT folder, but those one
  sound like broken pointers, or pointer I failed to track reliably"
  (INTENT/11.233.md (a)).  They were not broken.  A large entry lives in its own
  file `INTENT/<id>.md`; a small one stays as a numbered row INSIDE `INTENT.md`,
  and such a row is written

      142. **THE UNIFORM POOL ...**

  with the section number nowhere on its own line.  So a reader who searches for
  the printed form of the id -- `5.142` -- finds every sentence that CITES the
  row and never the row, and a reader who opens `INTENT/` finds no file.  The id
  is correct, the reader's reconstruction of where it lives is not, and nothing
  told him which.  That is the same failure the entry document's `Sec.N.M`
  sentences hand to a newcomer (doc/developer-entry.md section 6), which is why
  this tool is referenced from there.

WHAT IT RESOLVES, in the order the ledger's own rules put them
  1. `INTENT/<id>.md`          -- the entry file.  The INTENT.md header's rule is
                                  that the entry file WINS over its in-file stub,
                                  so when one exists it is the answer and the
                                  stub is a derived label.
  2. `INTENT/archive/<id>.md`  -- the archived entry file.  Archival never
                                  rewrites references, so a live-surface-only
                                  lookup reports a false miss on every closed id.
  3. `INTENT.md:<line>`        -- the inline row, scoped to its own section:
                                  section 5 rows between `## 5.` and `## 6.`,
                                  section 11 stubs between `## 11.` and the
                                  maintenance marker, section 13 rows in a table.
                                  The scoping is not decoration: `142. ` matches
                                  a line in section 5 AND a line in section 11,
                                  and an unscoped grep hands back both.

Usage:
    python3 claude/intent_resolve.py 5.142
    python3 claude/intent_resolve.py Sec.5.142      # the entry document's form
    python3 claude/intent_resolve.py 11.233         # section 11
    python3 claude/intent_resolve.py --self-test
    python3 claude/intent_resolve.py --self-test --break-scoping   # must FAIL

  The id may be written `5.142`, `Sec.5.142`, `sec.5.142`, `S5.142` or with the
  section sign.  Options: `--root DIR` (default: this file's directory),
  `--quiet` (print the location and nothing else).

Exit 0 = resolved; 1 = resolves nowhere; 2 = usage.

By hand, without this tool:   grep -n '^142\\. ' claude/INTENT.md
"""

import os
import re
import sys

# The section sign, written as an escape because every source file in this
# project is pure ASCII bytes (D14; INTENT/11.189.md clause (c) is the rule for
# a literal that must carry a non-ASCII character).
SECTION_SIGN = chr(0xA7)

# `Sec.5.142`, `5.142`, `S5.142`, and the section-sign form.
IDPAT = re.compile(
    r"^(?:sec\.|s|" + SECTION_SIGN + r")?\s*(\d+)\.(\d+)[a-z]?$", re.IGNORECASE
)

# The end of section 11's numbered list.  It is a marker and not `## 12.`
# because stubs 128-138 once accreted BELOW the list and inside the section.
MAINT_MARKER = re.compile(r"^\*Maintenance marker\b")


def parse_id(raw):
    """'Sec.5.142' -> (5, 142).  Returns None when it is not an id."""
    m = IDPAT.match(raw.strip().strip("`'\","))
    if not m:
        return None
    return int(m.group(1)), int(m.group(2))


def section_bounds(lines, major, scoped=True):
    """[start, end) line indices of section `major`'s row list.

    `scoped=False` is the WRONG behaviour on purpose -- --self-test uses it to
    show the scoping leg can fail rather than asserting that it works.
    """
    if not scoped:
        return 0, len(lines)
    start = None
    for i, l in enumerate(lines):
        if re.match(r"^## %d\." % major, l):
            start = i
            break
    if start is None:
        return None
    for j in range(start + 1, len(lines)):
        if lines[j].startswith("## ") or MAINT_MARKER.match(lines[j]):
            return start, j
    return start, len(lines)


def find_inline(lines, major, minor, scoped=True):
    """1-based line number of the inline row, or None."""
    b = section_bounds(lines, major, scoped)
    if b is None:
        return None
    row = re.compile(r"^%d\.\s" % minor)
    for i in range(b[0], b[1]):
        if row.match(lines[i]):
            return i + 1
    # Section 13 is a table: `| A4 | ... |` / `| B17 | ... |`.  Its ids are
    # lettered, so a numeric minor never reaches here from 13 -- kept so the
    # section's shape is stated rather than silently unsupported.
    return None


def find_table_row(lines, major, ident):
    b = section_bounds(lines, major)
    if b is None:
        return None
    pat = re.compile(r"^\|\s*~*%s~*\s*\|" % re.escape(ident), re.IGNORECASE)
    for i in range(b[0], b[1]):
        if pat.match(lines[i]):
            return i + 1
    return None


def resolve(root, major, minor):
    """-> (kind, location, detail) or None."""
    sid = "%d.%d" % (major, minor)
    entry = os.path.join(root, "INTENT", sid + ".md")
    if os.path.isfile(entry):
        return ("entry file", "INTENT/%s.md" % sid, head_of(entry))
    arch = os.path.join(root, "INTENT", "archive", sid + ".md")
    if os.path.isfile(arch):
        return ("archived entry file", "INTENT/archive/%s.md" % sid, head_of(arch))
    intent = os.path.join(root, "INTENT.md")
    if not os.path.isfile(intent):
        return None
    lines = open(intent, encoding="utf-8").read().split("\n")
    n = find_inline(lines, major, minor)
    if n is not None:
        return ("inline row", "INTENT.md:%d" % n, lines[n - 1][:200])
    return None


def head_of(path, limit=200):
    for l in open(path, encoding="utf-8"):
        l = l.strip()
        if l and not l.startswith("# "):
            return l[:limit]
    return ""


# --- self-test --------------------------------------------------------------

# The six ids the owner could not resolve (INTENT/11.233.md (h)), with the line
# each one had when that clause was written.  They are NOT asserted by number
# here -- a line number is exactly the thing that moves -- but by the property
# that must hold whatever the numbers are.
OWNER_SIX = [(5, 142), (5, 100), (5, 101), (5, 149), (5, 146), (5, 140)]


def self_test(root, break_scoping=False):
    """Return (ok, report lines).

    The legs are chosen so that each can fail on its own:
      1  every one of the owner's six resolves somewhere
      2  each resolves to a line INSIDE section 5's bounds -- the leg that
         --break-scoping turns off, and the one that catches the real hazard
         (`142. ` also matches a section 11 stub)
      3  an id that exists in no section resolves NOWHERE (a resolver that
         answers everything answers nothing)
      4  the four spellings of one id agree
      5  an entry-file id resolves to the FILE and not to its stub -- the
         header's entry-file-wins rule
    """
    out = []
    ok = True
    intent = os.path.join(root, "INTENT.md")
    lines = open(intent, encoding="utf-8").read().split("\n")
    b5 = section_bounds(lines, 5)

    for major, minor in OWNER_SIX:
        n = find_inline(lines, major, minor, scoped=not break_scoping)
        if n is None:
            out.append("  FAIL  %d.%d resolves nowhere" % (major, minor))
            ok = False
            continue
        inside = b5[0] < n <= b5[1]
        out.append(
            "  %s  %s%d.%d -> INTENT.md:%d %s"
            % ("ok  " if inside else "FAIL", SECTION_SIGN, major, minor, n,
               "" if inside else "(OUTSIDE section 5 -- wrong row)")
        )
        if not inside:
            ok = False

    # leg 2b -- THE LEG SCOPING EXISTS FOR, and the only one --break-scoping can
    # actually break.  An unscoped search walks the file from the top, so for a
    # section 5 id it happens to find the right row first and proves nothing;
    # the case that discriminates is a section 11 id whose number ALSO names a
    # section 5 row, where the wrong answer sits EARLIER in the file.  11.7 and
    # 11.9 are the two such ids with no entry file of their own (measured
    # 2026-09-12); a future split of either would make this leg vacuous, so it
    # says so rather than passing quietly.
    b11 = section_bounds(lines, 11)
    twins = [(11, n) for n in (7, 9)
             if find_inline(lines, 5, n) and find_inline(lines, 11, n)]
    if not twins:
        out.append("  FAIL  no section-11 id with a section-5 twin is left "
                   "inline -- this leg no longer discriminates, pick another")
        ok = False
    for major, minor in twins:
        n = find_inline(lines, major, minor, scoped=not break_scoping)
        inside = n is not None and b11[0] < n <= b11[1]
        out.append(
            "  %s  %s%d.%d -> INTENT.md:%s %s"
            % ("ok  " if inside else "FAIL", SECTION_SIGN, major, minor, n,
               "" if inside else "(that is the section 5 row of the same number)")
        )
        if not inside:
            ok = False

    # leg 3 -- a number no section carries
    if resolve(root, 5, 99999) is None:
        out.append("  ok    5.99999 resolves nowhere, as it must")
    else:
        out.append("  FAIL  5.99999 resolved to something")
        ok = False

    # leg 4 -- the spellings
    forms = ["5.142", "Sec.5.142", "sec.5.142", SECTION_SIGN + "5.142", "S5.142"]
    parsed = {parse_id(f) for f in forms}
    if parsed == {(5, 142)}:
        out.append("  ok    five spellings of one id parse to one id")
    else:
        out.append("  FAIL  spellings disagree: %r" % (parsed,))
        ok = False

    # leg 5 -- entry file wins over its stub
    r = resolve(root, 11, 233)
    if r and r[0] == "entry file":
        out.append("  ok    11.233 -> %s (entry file wins over its stub)" % r[1])
    else:
        out.append("  FAIL  11.233 did not resolve to its entry file: %r" % (r,))
        ok = False

    return ok, out


def main(argv):
    root = os.path.dirname(os.path.abspath(__file__))
    args, ids, quiet, st, brk = argv[1:], [], False, False, False
    i = 0
    while i < len(args):
        a = args[i]
        if a == "--self-test":
            st = True
        elif a == "--break-scoping":
            brk = True
        elif a == "--quiet":
            quiet = True
        elif a == "--root":
            i += 1
            if i >= len(args):
                print("--root needs a value", file=sys.stderr)
                return 2
            root = os.path.abspath(args[i])
        elif a.startswith("-"):
            print("unknown option %s" % a, file=sys.stderr)
            return 2
        else:
            ids.append(a)
        i += 1

    if st:
        ok, report = self_test(root, brk)
        print("intent_resolve --self-test  (root %s)" % root)
        if brk:
            print("  [--break-scoping: section scoping DISABLED on purpose]")
        for l in report:
            print(l)
        print("  => %s" % ("PASS" if ok else "FAIL"))
        return 0 if ok else 1

    if not ids:
        print(__doc__.strip().split("\n\n")[0], file=sys.stderr)
        print("usage: intent_resolve.py <id> [<id>...] | --self-test", file=sys.stderr)
        return 2

    rc = 0
    for raw in ids:
        pid = parse_id(raw)
        if pid is None:
            print("%s: not a ledger id (expected 5.142, Sec.5.142, 11.233)" % raw,
                  file=sys.stderr)
            rc = 1
            continue
        r = resolve(root, *pid)
        if r is None:
            print("%s%d.%d: resolves nowhere -- not an entry file, not archived, "
                  "not a row in section %d" % (SECTION_SIGN, pid[0], pid[1], pid[0]),
                  file=sys.stderr)
            rc = 1
            continue
        kind, loc, detail = r
        if quiet:
            print(loc)
        else:
            print("%s%d.%d  %s  %s" % (SECTION_SIGN, pid[0], pid[1], kind, loc))
            if detail:
                print("    %s" % detail)
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv))
