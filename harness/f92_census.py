#!/usr/bin/env python3
"""f92_census.py -- classify every `master-beta` occurrence in BOTH repositories,
and emit the patch that covers exactly the live-pointer class.

WHY A PARTITION AND NOT A LIST
------------------------------
The branch is to be renamed `main` [vixy 2026-09-05: "The master-beta will became
the reference and get renamed main once ready."].  A rename is one act; what it
stales is every place the old name is written down -- and those places are NOT of
one kind.  Sorting them into four classes is what turns "226 hits" into "four
edits and 222 things that must NOT be touched":

  LIVE POINTER      a reader or a tool consumes it as CURRENT state; the rename
                    makes the sentence FALSE.  -> the patch, and nothing else.
  PIN               `master-beta @ <sha>`: resolves by SHA whatever the branch is
                    called, so the rename cannot invalidate it.  Never touched.
                    (The class the scedit grammars call `_meta.anchor_pin`.)
  HISTORICAL RECORD ledger prose, archived notes, dated session updates, commit
                    trailers already written.  Still TRUE after the rename,
                    because each states what was so at its date.  The ledger's
                    maintenance invariant is supersession-with-record: these are
                    ANNOTATED at the rename, never rewritten -- a patch that
                    edited them would destroy the record it claims to maintain.
  CONVENTION        the `Code: <branch> @ <short-sha>` trailer GRAMMAR.  Stated
                    with a placeholder wherever it is stated authoritatively, so
                    it holds no occurrence of the branch name at all; the two
                    tools that WRITE trailers derive the name from
                    `git branch --show-current` and follow the rename by
                    themselves.  Token count 0 is the measurement, not a gap.

THE DISCRIMINATOR, stated so it can be applied to a hit this table does not know:
    does the rename make this sentence FALSE?
      yes                                  -> LIVE POINTER
      no, it resolves by sha               -> PIN
      no, it describes a past state        -> HISTORICAL RECORD
      no, it is branch-agnostic by wording -> CONVENTION

TWO CONSTANTS, AND WHY THEY ARE NOT ONE
---------------------------------------
SEARCH_TOKEN is what is being audited: the old branch name.  It does not change
at the rename -- afterwards the interesting question is still "where is the old
name", and the answer must be "nowhere live".
SITE_BRANCH is the branch name AS WRITTEN AT THE LIVE SITES.  The rename changes
it, and changing this one line is what makes every SITES entry below locate its
site again in the renamed world.  `patch` emits exactly that edit for this file,
so the instrument is inside its own patch rather than beside it (I2: one place
states the branch name for this tool).

THIS FILE'S OWN OCCURRENCES.  It defines the classes by quoting them, and it
quotes the owner's sentence verbatim, so it holds the token in prose.  Those are
declared HISTORICAL RECORD: a quotation that is edited is no longer a quotation,
and a class definition that is "renamed" stops describing the corpus it sorts.
Only the SITE_BRANCH line above is live here.  Stated limit: the declaration is
by PATH, so a live pointer added to this file later would be classed historical
by default -- the one file in the corpus where the census trusts its author.

USAGE
    python3 f92_census.py census [--code-root R] [--harness-root R] [--verbose]
    python3 f92_census.py patch  [--out FILE]
    python3 f92_census.py verify --code-root W1 --harness-root W2
Exit 0 = the partition is complete (census) / LIVE is empty (verify);
1 = an unclassified occurrence, a stale site, or a failed expectation; 2 = usage.
"""

import os
import re
import subprocess
import sys

SEARCH_TOKEN = "master-beta"          # audited; NEVER changes (see the header)
# SITE_BRANCH is the name as written at the live sites.  The rename changes this
# ONE line of this file and no other; the emitted patch carries that edit, so the
# instrument goes over with the corpus it measures.
SITE_BRANCH = "master-beta"

CODE_ROOT_DEFAULT = "/home/claude/spacecrafter"
HARNESS_ROOT_DEFAULT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

TOK = re.compile(SEARCH_TOKEN.encode())
# A pin: the branch name followed by a sha, in any of the spellings the corpus
# actually uses -- "master-beta @ d6aec251", "`master-beta` @ `76ee38c7`",
# "master-beta 423cbe23" (F68/F69's prose form).  Backticks optional on either
# side because the ledger writes both.
PIN = re.compile(
    rb"`?" + SEARCH_TOKEN.encode() + rb"`?[ \t]*@[ \t]*`?[0-9a-f]{7,40}`?"
    rb"|" + SEARCH_TOKEN.encode() + rb"[ \t]+[0-9a-f]{7,40}"
)

# --------------------------------------------------------------------------
# THE LIVE SITES.  Each is located by a snippet that must appear EXACTLY ONCE in
# its file -- not by a line number, which drifts under every append.  `{B}` is
# SITE_BRANCH.  A site whose snippet is missing or duplicated is a STALE TABLE
# and fails the run: the alternative is a census that silently stops covering
# the one class the patch is made of.
# --------------------------------------------------------------------------
SITES = [
    # (repo, path, snippet-template, what a reader does with it)
    ("code", "doc/developer-entry.md",
     "The working branch is\n`{B}`; pull requests target `2023-master`",
     "the newcomer's one sentence naming the branch he must be on"),
    ("harness", "CLAUDE.md",
     "branch `{B}` (PRs → `2023-master`)",
     "the auto-loaded map: which branch this tree is"),
    ("harness", "agents/opus-xhigh.md",
     "code `{B}` at\n  `/home/claude/spacecrafter`",
     "the executor definition's map of the two repos"),
    ("harness", "agents/opus-xhigh.md",
     "`Code: {B} @ <sha>` trailer",
     "the executor's commit-discipline instruction: the trailer it must write"),
    ("harness", "harness/f92_census.py",
     'SITE_BRANCH = "{B}"',
     "this instrument's own record of the name written at the sites"),
]

# Everything in THIS file except the SITE_BRANCH site: the audited constant, the
# verbatim owner quote, the class definitions and the example spellings.  All
# records (see the header); the rename leaves every one of them alone.
SELF = "harness/f92_census.py"

# --------------------------------------------------------------------------
# RECORD SURFACES: append-only or dated-correction documents.  A hit here is
# HISTORICAL RECORD by construction -- INTENT.md's header forbids rewriting a
# record, DEPLOYMENT-MAP.md's own maintenance line is "correct in place with
# dated strikes", and `fable-dispatch/archive/` + `INTENT/archive/` are
# byte-exact moves ("References are NEVER rewritten").  Anything NOT matched
# here and not a pin and not a site is reported UNCLASSIFIED and fails the run.
# --------------------------------------------------------------------------
RECORD_PREFIXES = (
    "INTENT.md", "INTENT/", "DEPLOYMENT-MAP.md", "HOST-EVENTS.md", "README.md",
    "b12-design.md", "dispatch-2026-", "fable-dispatch.md", "fable-dispatch/",
    "harness/README.md", "harness/artifacts/", "util/scedit/INTENT.md",
    "util/scedit/SWEEP_DISPATCH.md",
    # byte-exact archived predecessors of the executor definition (§11.161(g),
    # §11.175(d)) -- records of what the definition SAID, not instructions
    "agents/opus-xhigh.2026-07-19.md",
    "agents/opus-xhigh.2026-07-19.user-level.md",
)

# Sites that build a pin string from parts, so the sha is not adjacent to the
# name in the source text.  Semantically pins: what they WRITE is
# `master-beta @ <that constant>`, which resolves by sha.
PIN_BY_CONSTRUCTION = {
    ("harness", "harness/f75_anchors.py"): 1,   # ANCHOR_PIN = "master-beta @ " + ANCHOR_PIN_SHA (= 54a2b844)
}

CLASSES = ("LIVE POINTER", "PIN", "HISTORICAL RECORD", "CONVENTION")


def tracked(root):
    out = subprocess.run(["git", "-C", root, "ls-files", "-z"],
                         capture_output=True).stdout
    return [p.decode() for p in out.split(b"\0") if p]


def classify(repo, root, verbose=False):
    """-> (per-class counts, per-file rows, problems)."""
    counts = {c: 0 for c in CLASSES}
    rows, problems = [], []
    site_by_path = {}
    for r, path, tmpl, why in SITES:
        if r == repo:
            site_by_path.setdefault(path, []).append((tmpl, why))

    # Locate every site first: a missing or duplicated snippet is a stale table.
    live_spans = {}
    for path, entries in site_by_path.items():
        full = os.path.join(root, path)
        if not os.path.isfile(full):
            problems.append(f"STALE SITE: {repo}:{path} does not exist under {root}")
            continue
        blob = open(full, "rb").read()
        spans = []
        for tmpl, why in entries:
            snip = tmpl.replace("{B}", SITE_BRANCH).encode()
            n = blob.count(snip)
            if n != 1:
                problems.append(
                    f"STALE SITE: {repo}:{path} -- snippet {tmpl!r} occurs {n} times "
                    f"(expected exactly 1); re-read the file and update SITES")
                continue
            off = blob.find(snip)
            spans.append((off, off + len(snip), why))
        live_spans[path] = spans

    for path in tracked(root):
        full = os.path.join(root, path)
        if not os.path.isfile(full) or os.path.islink(full):
            continue
        blob = open(full, "rb").read()
        if SEARCH_TOKEN.encode() not in blob:
            continue
        pins = [(m.start(), m.end()) for m in PIN.finditer(blob)]
        spans = live_spans.get(path, [])
        per = {c: 0 for c in CLASSES}
        for m in TOK.finditer(blob):
            off = m.start()
            line = blob.count(b"\n", 0, off) + 1
            if any(a <= off < b for a, b, _ in spans):
                cls = "LIVE POINTER"
            elif any(a <= off < b for a, b in pins):
                cls = "PIN"
            elif repo == "harness" and path == SELF:
                cls = "HISTORICAL RECORD"      # declared; see the header
            elif PIN_BY_CONSTRUCTION.get((repo, path)):
                cls = "PIN"
            elif path.startswith(RECORD_PREFIXES):
                cls = "HISTORICAL RECORD"
            else:
                problems.append(
                    f"UNCLASSIFIED: {repo}:{path}:{line} -- not a site, not a pin, "
                    f"not a record surface. Read it and place it in one of the four "
                    f"classes before trusting this census.")
                continue
            per[cls] += 1
            counts[cls] += 1
            if verbose:
                print(f"    {cls:<17} {repo}:{path}:{line}")
        rows.append((path, per, sum(per.values())))
    rows.sort(key=lambda r: (-r[2], r[0]))
    return counts, rows, problems


PATCH_HEADER = """\
# rename-live-pointers.patch -- the LIVE POINTER class of the `{T}` -> `main`
# rename, BOTH repositories, code repo first (the direction of the dependency:
# harness records cite code commits, never the reverse).
#
# Generated by `claude/harness/f92_census.py patch`; regenerate it rather than
# editing it, and re-run `census` first -- a site the table can no longer locate
# fails the run instead of silently dropping out of the patch.
#
# The two halves live in two repositories, so they are applied separately.  The
# path sets are disjoint, which is what makes one file safe to hand to both:
#
#     git -C <code-repo>    apply [--check] --include='doc/*' <this file>
#     git -C <harness-repo> apply [--check] --include='CLAUDE.md' \\
#                                 --include='agents/*' --include='harness/*' <this file>
#
# NOT in this patch, deliberately (F92's census, {N} occurrences):
#   PIN               `{T} @ <sha>` -- resolves by sha, the rename cannot stale it
#   HISTORICAL RECORD ledger prose, archives, the {H} commit trailers already
#                     written -- annotated at the rename, never rewritten
#   CONVENTION        the `Code: <branch> @ <short-sha>` grammar -- stated with a
#                     placeholder, and the two tools that write trailers read
#                     `git branch --show-current`, so both follow by themselves
# ---------------------------------------------------------------------------
"""


def emit_patch(code_root, harness_root):
    """The live-pointer edits, both repos, code first, as one unified diff."""
    import difflib
    out = []
    for repo, root in (("code", code_root), ("harness", harness_root)):
        paths = []
        for r, path, tmpl, why in SITES:
            if r == repo and path not in paths:
                paths.append(path)
        for path in paths:
            full = os.path.join(root, path)
            old = open(full, "r", encoding="utf-8").read()
            new = old
            for r, p, tmpl, why in SITES:
                if r != repo or p != path:
                    continue
                snip = tmpl.replace("{B}", SITE_BRANCH)
                assert new.count(snip) == 1, (path, tmpl)
                new = new.replace(snip, tmpl.replace("{B}", "main"))
            diff = difflib.unified_diff(
                old.splitlines(keepends=True), new.splitlines(keepends=True),
                fromfile=f"a/{path}", tofile=f"b/{path}", n=3)
            out.append(f"diff --git a/{path} b/{path}\n")
            out.extend(diff)
    return "".join(out)


def main(argv):
    cmd = argv[1] if len(argv) > 1 else "census"
    def opt(name, default):
        return argv[argv.index(name) + 1] if name in argv else default
    code_root = opt("--code-root", CODE_ROOT_DEFAULT)
    harness_root = opt("--harness-root", HARNESS_ROOT_DEFAULT)
    verbose = "--verbose" in argv

    if cmd == "patch":
        body = emit_patch(code_root, harness_root)
        n_other = n_trailers = 0
        for repo, root in (("code", code_root), ("harness", harness_root)):
            counts, _, _ = classify(repo, root)
            n_other += counts["PIN"] + counts["HISTORICAL RECORD"] + counts["CONVENTION"]
        n_trailers = len([l for l in subprocess.run(
            ["git", "-C", harness_root, "log", "--format=%B"],
            capture_output=True, text=True).stdout.splitlines()
            if l.startswith("Code: " + SEARCH_TOKEN + " @")])
        text = PATCH_HEADER.format(T=SEARCH_TOKEN, N=n_other, H=n_trailers) + body
        dest = opt("--out", "")
        if dest:
            open(dest, "w", encoding="utf-8").write(text)
            print(f"wrote {dest} ({len(text.splitlines())} lines)")
        else:
            sys.stdout.write(text)
        return 0

    if cmd not in ("census", "verify"):
        print(__doc__)
        return 2

    bad = 0
    grand = {c: 0 for c in CLASSES}
    for repo, root in (("code", code_root), ("harness", harness_root)):
        counts, rows, problems = classify(repo, root, verbose)
        total = sum(counts.values())
        head = subprocess.run(["git", "-C", root, "rev-parse", "--short", "HEAD"],
                              capture_output=True, text=True).stdout.strip()
        print(f"\n== {repo.upper()}  {root}  @ {head}")
        print(f"   files with a hit: {len(rows)}   occurrences: {total}")
        for c in CLASSES:
            print(f"     {c:<17} {counts[c]:>5}")
        if verbose:
            for path, per, n in rows:
                if n:
                    detail = " ".join(f"{c.split()[0][:4]}={per[c]}" for c in CLASSES if per[c])
                    print(f"     {n:>3}  {path}   [{detail}]")
        for p in problems:
            print(f"   ! {p}")
            bad += 1
        # The independent check: git's own count, so a bug in this file's walk
        # cannot report a complete partition over a corpus it never read.
        g = subprocess.run(["git", "-C", root, "grep", "-o", SEARCH_TOKEN],
                           capture_output=True, text=True).stdout
        gn = len([l for l in g.splitlines() if l])
        if gn != total:
            print(f"   ! DISAGREEMENT: git grep -o counts {gn}, this walk {total}")
            bad += 1
        else:
            print(f"   git grep -o agrees: {gn}")
        if cmd == "verify" and counts["LIVE POINTER"] != 0:
            print(f"   ! LIVE POINTER is {counts['LIVE POINTER']}, expected 0 after the patch")
            bad += 1
        for c in CLASSES:
            grand[c] += counts[c]
    print(f"\n== BOTH REPOS: " + "  ".join(f"{c}={grand[c]}" for c in CLASSES)
          + f"  total={sum(grand.values())}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
