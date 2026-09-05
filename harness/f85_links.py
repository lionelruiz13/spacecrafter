#!/usr/bin/env python3
"""f85_links.py -- every path and every ledger id cited by the developer entry
document must resolve at the current HEAD.

The document (`doc/developer-entry.md`) is derived from the ledger and points
back into it, so its citations are its whole load-bearing surface: a citation
that no longer resolves is the document silently becoming wrong.  This checker
is what makes "it points at real things" a measurement rather than a reading.

WHAT IS CHECKED
  1. Repo-relative paths in backticks, e.g. `src/coreModule/core.hpp:120`.
     Existence on disk, and -- where the citation names a line or a range --
     that the line exists in the file.  A citation past the end of its own
     file is a broken citation even though the path resolves.
  2. Bare continuations, e.g. a `[`:8-20`]` following a full path in the same
     paragraph.  These resolve against the most recent full path, which is
     what a reader does; leaving them unchecked would leave a hole exactly
     where the document is densest.
  3. Ledger ids written `Sec.A.B`, resolved over INTENT.md LIVE UNION
     INTENT/archive/ -- an entry file, an archived entry file, a numbered stub
     inside section A, or a header.  Archival never rewrites references
     (INTENT.md header), so a checker that looked only at the live surface
     would report false dangles on every archived entry.

WHAT IS DELIBERATELY NOT CHECKED
  Absolute paths (`/usr/local`), home paths (`~/.spacecrafter/...`),
  placeholders (`<spacecrafter>/claude/`), remotes and shell fragments: none
  of them is a claim about this tree, so a checker that verified them would
  be measuring the host instead of the document.  They are counted and listed
  under --verbose so the exclusion is visible rather than silent.

Usage:
    python3 f85_links.py [doc] [--root DIR] [--verbose]
Exit 0 = every citation resolves; 1 = at least one dangling; 2 = usage.
"""

import os
import re
import sys

DEFAULT_DOC = "doc/developer-entry.md"

# A backticked token.  The document's citation form is always backticked, so
# this is the complete candidate set by construction.
TICK = re.compile(r"`([^`\n]+)`")
# Sec.A.B, optionally followed by a clause letter that is not part of the id.
SECID = re.compile(r"\bSec\.(\d+)\.(\d+)\b")
# A trailing :N or :N-M line specification.
LINESPEC = re.compile(r":(\d+)(?:-(\d+))?$")

# Root-level files cited without a slash.
ROOT_FILES = {"INSTALL", "README", ".gitignore", ".gitmodules"}


def is_repo_path(tok):
    """Repo-relative path candidate, or one of the root files."""
    if tok in ROOT_FILES:
        return True
    if "/" not in tok:
        return False
    # Not a claim about this tree: absolute, home, parent-relative, a
    # placeholder, a remote, or anything with whitespace.
    if tok[0] in "~/<" or tok.startswith("../") or tok.startswith("./"):
        return False
    if any(c in tok for c in "@<> \t"):
        return False
    return True


def split_linespec(tok):
    m = LINESPEC.search(tok)
    if not m:
        return tok, None, None
    lo = int(m.group(1))
    hi = int(m.group(2)) if m.group(2) else lo
    return tok[: m.start()], lo, hi


def count_lines(path):
    with open(path, "rb") as fh:
        return sum(1 for _ in fh)


def section_range(text, major):
    """Line range (0-based, end-exclusive) of INTENT.md's '## <major>.' section."""
    lines = text.split("\n")
    start = None
    for i, l in enumerate(lines):
        if re.match(r"^## %d\." % major, l):
            start = i
            break
    if start is None:
        return None
    for j in range(start + 1, len(lines)):
        if lines[j].startswith("## "):
            return (start, j)
    return (start, len(lines))


def main(argv):
    doc = DEFAULT_DOC
    root = None
    verbose = False
    args = argv[1:]
    i = 0
    while i < len(args):
        a = args[i]
        if a == "--verbose":
            verbose = True
        elif a == "--root":
            i += 1
            if i >= len(args):
                print("--root needs a value", file=sys.stderr)
                return 2
            root = args[i]
        elif a.startswith("-"):
            print("unknown option %s" % a, file=sys.stderr)
            return 2
        else:
            doc = a
        i += 1

    if root is None:
        # harness/ -> claude/ -> <spacecrafter>
        root = os.path.abspath(
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
        )
    root = os.path.abspath(root)
    docpath = doc if os.path.isabs(doc) else os.path.join(root, doc)
    if not os.path.isfile(docpath):
        print("no such document: %s" % docpath, file=sys.stderr)
        return 2

    text = open(docpath, encoding="utf-8").read()

    intent_path = os.path.join(root, "claude", "INTENT.md")
    intent = open(intent_path, encoding="utf-8").read() if os.path.isfile(intent_path) else ""
    intent_lines = intent.split("\n")
    entry_dir = os.path.join(root, "claude", "INTENT")
    archive_dir = os.path.join(entry_dir, "archive")

    dangling = []
    checked_paths = 0
    checked_conts = 0
    checked_secs = 0
    skipped = []
    last_path = None

    for m in TICK.finditer(text):
        tok = m.group(1)

        # (2) bare continuation -- resolve against the last full path.
        if re.fullmatch(r":\d+(?:-\d+)?", tok):
            if last_path is None:
                dangling.append("continuation %s with no preceding path" % tok)
                continue
            _, lo, hi = split_linespec("x" + tok)
            n = count_lines(os.path.join(root, last_path))
            checked_conts += 1
            if hi > n:
                dangling.append(
                    "%s%s -> %s has only %d lines" % (last_path, tok, last_path, n)
                )
            continue

        if not is_repo_path(tok):
            if "/" in tok or tok in ROOT_FILES:
                skipped.append(tok)
            continue

        # (1) full repo-relative path.
        path, lo, hi = split_linespec(tok)
        path = path.rstrip("/")
        full = os.path.join(root, path)
        if not os.path.exists(full):
            dangling.append("%s -> no such path" % tok)
            continue
        checked_paths += 1
        last_path = path
        if lo is not None:
            if not os.path.isfile(full):
                dangling.append("%s -> line cited on a directory" % tok)
                continue
            n = count_lines(full)
            if hi > n:
                dangling.append("%s -> file has only %d lines" % (tok, n))

    # (3) ledger ids.
    seen_sec = set()
    for m in SECID.finditer(text):
        major, minor = int(m.group(1)), int(m.group(2))
        key = (major, minor)
        if key in seen_sec:
            continue
        seen_sec.add(key)
        checked_secs += 1
        sid = "%d.%d" % (major, minor)
        if os.path.isfile(os.path.join(entry_dir, sid + ".md")):
            continue
        if os.path.isfile(os.path.join(archive_dir, sid + ".md")):
            continue
        rng = section_range(intent, major)
        found = False
        if rng:
            stub = re.compile(r"^%d\.\s" % minor)
            for l in intent_lines[rng[0]: rng[1]]:
                if stub.match(l):
                    found = True
                    break
        if not found:
            hdr = re.compile(r"^#{2,4} %s\b" % re.escape(sid))
            found = any(hdr.match(l) for l in intent_lines)
        if not found:
            dangling.append("Sec.%s -> not in INTENT.md live or INTENT/archive/" % sid)

    print("document : %s" % os.path.relpath(docpath, root))
    print("root     : %s" % root)
    print(
        "checked  : %d paths, %d continuations, %d ledger ids"
        % (checked_paths, checked_conts, checked_secs)
    )
    print("skipped  : %d non-repo tokens (absolute/home/placeholder/remote)" % len(skipped))
    if verbose:
        for s in sorted(set(skipped)):
            print("    skip %s" % s)
    if dangling:
        print("DANGLING : %d" % len(dangling))
        for d in dangling:
            print("    %s" % d)
        return 1
    print("DANGLING : 0")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
