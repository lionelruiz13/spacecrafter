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
  4. Commit shas -- any backticked 7-to-8 hex token -- by REACHABILITY, not by
     shape: `git merge-base --is-ancestor <sha> <branch>` in the repository the
     sentence names.  This class was added 2026-09-12 (F110) after a rehearsal
     of this document from a plain clone found three citations of a commit
     that had been amended away on 2026-09-05 and rewritten again on
     2026-09-12, while this checker reported 0 dangling: a sha is the one
     citation form that can go wrong without anything on disk changing, which
     is exactly why it needs a checker rather than a reading.
     Repository selection: HARNESS by default, because a sha cited in this
     document is a harness sha unless the sentence names a code file; a
     sentence naming a path OUTSIDE `claude/` selects the code repository.
     A sha is reported dangling when the object is absent (a fresh clone
     transfers reachable objects only, so an amended-away commit is not even a
     name there) and when it exists but is not an ancestor of the branch.

WHAT IS DELIBERATELY NOT CHECKED
  Absolute paths (`/usr/local`), home paths (`~/.spacecrafter/...`),
  placeholders (`<spacecrafter>/claude/`), remotes and shell fragments: none
  of them is a claim about this tree, so a checker that verified them would
  be measuring the host instead of the document.  They are counted and listed
  under --verbose so the exclusion is visible rather than silent.

Usage:
    python3 f85_links.py [doc] [--root DIR] [--verbose] [--no-sha]
Exit 0 = every citation resolves; 1 = at least one dangling; 2 = usage.
`--no-sha` drops class 4 (for a tree with no git, or to reproduce the
pre-2026-09-12 behaviour).
"""

import os
import re
import subprocess
import sys

DEFAULT_DOC = "doc/developer-entry.md"

# A backticked token.  The document's citation form is always backticked, so
# this is the complete candidate set by construction.
TICK = re.compile(r"`([^`\n]+)`")
# Sec.A.B, optionally followed by a clause letter that is not part of the id.
SECID = re.compile(r"\bSec\.(\d+)\.(\d+)\b")
# A trailing :N or :N-M line specification.
LINESPEC = re.compile(r":(\d+)(?:-(\d+))?$")
# A commit sha: 7 or 8 hex digits, and NOT all decimal -- "12345678" is a
# number in prose, not a sha, and treating it as one would invent dangles.
SHA = re.compile(r"^[0-9a-f]{7,8}$")

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


# --- class 4: commit shas ---------------------------------------------------

# Which repository a sha sentence means.  The document cites two repositories
# and says which by naming a file: `claude/...` is the harness, anything else
# is the code tree.  Default harness, because that is where this document's
# shas have always pointed and a wrong default here would be silent.
HARNESS_REPO = ("claude", "CC-harness")
CODE_REPO = ("", "master-beta")


def sha_context(text, pos, window=400):
    """The sentence around a sha, used only to pick the repository."""
    lo = text.rfind("\n\n", 0, pos)
    lo = 0 if lo < 0 else lo
    hi = text.find("\n\n", pos)
    hi = len(text) if hi < 0 else hi
    frag = text[max(lo, pos - window): min(hi, pos + window)]
    return frag


def pick_repo(frag, lead=""):
    """Which repository a sha citation means.

    Two signals, local first.  `lead` is the text immediately before the sha,
    and a sentence that names both repositories disambiguates them exactly
    there -- "code `X` / harness `Y`" is how a human writes it, and reading the
    paragraph instead would pick one repository for both.  (Measured 2026-09-12:
    the paragraph rule alone sent a code sha to the harness repo and reported
    it absent, which is how this signal got added.)  Falling back to the
    paragraph: harness unless it names a path outside claude/.
    """
    tail = lead[-40:].lower()
    icode, iharn = tail.rfind("code"), tail.rfind("harness")
    if icode >= 0 or iharn >= 0:
        return CODE_REPO if icode > iharn else HARNESS_REPO
    for m in TICK.finditer(frag):
        tok = m.group(1)
        if "/" not in tok:
            continue
        if tok.startswith("claude/"):
            return HARNESS_REPO
    for m in TICK.finditer(frag):
        tok = m.group(1)
        if is_repo_path(tok) and not tok.startswith("claude/"):
            return CODE_REPO
    return HARNESS_REPO


def sha_state(root, subdir, branch, sha):
    """'ok' | 'absent' | 'unreachable' | 'norepo'."""
    repo = os.path.join(root, subdir) if subdir else root
    if not os.path.isdir(os.path.join(repo, ".git")):
        return "norepo"
    r = subprocess.run(
        ["git", "-C", repo, "cat-file", "-e", sha + "^{commit}"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if r.returncode != 0:
        return "absent"
    r = subprocess.run(
        ["git", "-C", repo, "merge-base", "--is-ancestor", sha, branch],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    return "ok" if r.returncode == 0 else "unreachable"


# --- where an id lives: ONE implementation, not this file's -----------------
#
# This file used to carry its own copy of "the line range of INTENT.md section
# N" and "the row that starts with `<minor>. `".  claude/intent_resolve.py was
# written for the same question on 2026-09-12 (F110), and the two copies had
# ALREADY diverged before either was used in anger: this one ended a section at
# the next `## `, the resolver also ends section 11 at its maintenance marker,
# which is where section 11's numbered list actually stops.  That is I2's
# failure exactly -- two answers to one question, no way to notice.  So the
# resolver is the authority and this file asks it.
#
# What is NOT delegated, deliberately: this checker also accepts a HEADER match
# (`## 5.142`) as a resolution, because it asks "does this citation resolve to
# something" while the resolver asks "where is this id", and a header is an
# answer to the first and not to the second.
def _load_resolver():
    import importlib.util
    p = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..",
                     "intent_resolve.py")
    p = os.path.abspath(p)
    if not os.path.isfile(p):
        return None
    spec = importlib.util.spec_from_file_location("intent_resolve", p)
    mod = importlib.util.module_from_spec(spec)
    try:
        spec.loader.exec_module(mod)
    except Exception:
        return None
    return mod


RESOLVER = _load_resolver()


def main(argv):
    doc = DEFAULT_DOC
    root = None
    verbose = False
    check_sha = True
    args = argv[1:]
    i = 0
    while i < len(args):
        a = args[i]
        if a == "--verbose":
            verbose = True
        elif a == "--no-sha":
            check_sha = False
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
            full_last = os.path.join(root, last_path)
            # A continuation after a DIRECTORY path is a broken citation, not a
            # crash.  (It was a crash until 2026-09-12: IsADirectoryError out of
            # count_lines, which reports nothing and exits non-zero for the
            # wrong reason.  The full-path branch below already had this guard;
            # the continuation branch did not.)
            if not os.path.isfile(full_last):
                dangling.append(
                    "%s%s -> continuation resolves against %s, which is not a file"
                    % (last_path, tok, last_path)
                )
                continue
            n = count_lines(full_last)
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
        found = False
        if RESOLVER is not None:
            found = RESOLVER.resolve(os.path.join(root, "claude"), major, minor) is not None
        else:
            # No resolver on disk (an older checkout): the legs it owns, inline.
            if os.path.isfile(os.path.join(entry_dir, sid + ".md")):
                found = True
            elif os.path.isfile(os.path.join(archive_dir, sid + ".md")):
                found = True
        if not found:
            hdr = re.compile(r"^#{2,4} %s\b" % re.escape(sid))
            found = any(hdr.match(l) for l in intent_lines)
        if not found:
            dangling.append("Sec.%s -> not in INTENT.md live or INTENT/archive/" % sid)

    # (4) commit shas.
    checked_shas = 0
    sha_rows = []
    if check_sha:
        seen_sha = {}
        for m in TICK.finditer(text):
            tok = m.group(1)
            if not SHA.match(tok):
                continue
            subdir, branch = pick_repo(
                sha_context(text, m.start()), text[max(0, m.start() - 40): m.start()]
            )
            key = (tok, subdir)
            if key in seen_sha:
                seen_sha[key] += 1
                continue
            seen_sha[key] = 1
            checked_shas += 1
            state = sha_state(root, subdir, branch, tok)
            where = (subdir or "<code>") + " @ " + branch
            sha_rows.append((tok, where, state))
            if state == "absent":
                dangling.append(
                    "`%s` -> no such commit in %s (a fresh clone would not have "
                    "it at all); look it up in claude/sha-maps/" % (tok, where)
                )
            elif state == "unreachable":
                dangling.append(
                    "`%s` -> exists but is not an ancestor of %s (amended or "
                    "rewritten away); look it up in claude/sha-maps/"
                    % (tok, where)
                )
            elif state == "norepo":
                dangling.append("`%s` -> no git repository at %s" % (tok, where))

    print("document : %s" % os.path.relpath(docpath, root))
    print("root     : %s" % root)
    print(
        "checked  : %d paths, %d continuations, %d ledger ids, %d commit shas"
        % (checked_paths, checked_conts, checked_secs, checked_shas)
    )
    print("skipped  : %d non-repo tokens (absolute/home/placeholder/remote)" % len(skipped))
    if verbose:
        for s in sorted(set(skipped)):
            print("    skip %s" % s)
        for tok, where, state in sha_rows:
            print("    sha  %s  %-22s %s" % (tok, where, state))
    if dangling:
        print("DANGLING : %d" % len(dangling))
        for d in dangling:
            print("    %s" % d)
        return 1
    print("DANGLING : 0")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
