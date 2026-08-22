# CC-harness — reasoning & verification stream for spacecrafter

This repository holds the reasoning base built around the spacecrafter code to
check it: the verification harness, INTENT.md (intent/decision record),
dispatch records, and collaboration files. The code lives in the spacecrafter
repository. Dependency is one-way: this repo reasons about a code state; the
code never depends on this repo.

**Filing criterion** — a file belongs here if it is reasoning-base (built
around the reasoning to check or steer the code); it belongs in the code repo
if it is consumed by the build/runtime or by code contributors (e.g. a test
other developers should run in CI is code-side; a probe script for one
investigation is here). Conclusions discovered here that constrain future code
edits (invariants, seam tables, accepted-divergence criteria) must be promoted
into the code repo (header comments, docs/) — this repo is the lab notebook,
not where other contributors will look.

## Layout precondition

This repo must be checked out **nested inside a spacecrafter worktree at
`<spacecrafter>/claude/`**. Harness run scripts resolve the binary at
`../../build-claude/src/spacecrafter` (i.e. `<spacecrafter>/build-claude/`);
each script accepts an `SC_BIN` override. The parent repo's `.gitignore`
excludes `claude/`, so the nesting is invisible to it.

## Remote

Same GitHub repository as the code, **orphan branch `CC-harness`** (unrelated
history). Never merge `CC-harness` into a code branch or vice versa — git will
not stop you, and the result is a mess.

## Commit convention

Every commit records the code state it was made against, as a trailer:

    Code: <branch> @ <short-sha>

When a logical change spans both repos, commit code first, then this repo —
the trailer then references an existing commit. This trailer is the only
binding between reasoning state and code state; without it the correspondence
exists nowhere.

## Nothing derived enters history

A file belongs here if it is an ORIGIN -- reasoning, a script, a record. A
DERIVATION -- a build output, a staging binary, a rendered artifact, a
measurement dump -- does not: it is either reconstructible (so redundant beside
the source commit + md5 the delivery entry already records) or not
reconstructible (so unreliable as a record, because nothing can ever check what
it claims). This is the "uncommitted per the artifacts convention" that delivery
entries have stated since 2026-07. What follows is that convention with an
enforcement, added after a 187 MiB staging binary sat in 44 commits for three
weeks while the entry recording that wave stated it was uncommitted (INTENT
11.137) -- prose is enforced by whoever is paying attention at `git add`.

Three layers, each assuming the previous one failed:

1. **`.gitignore`** -- the known staging-binary name shapes (`harness/sc_f*_pre`,
   `harness/sc_f*_child`, `harness/spacecrafter_pre_*`) plus `artifacts`. Names,
   so it is a proxy: a binary under a new name walks straight past it.
2. **`githooks/pre-commit`** -- content, so it is the criterion: any
   ELF/PE/Mach-O/ar blob refused at any size, and any blob >= 8 MiB
   (`hooks.maxBlobBytes`) refused whatever it holds. The size number is set from
   the corpus, not from taste: the largest legitimately tracked blob across both
   repos is 3.06 MB. Install with `githooks/install.sh`, which wires
   `core.hooksPath` in BOTH repos and then PROVES the hook fires -- ELF refused,
   9 MiB refused, ordinary file passed. The negative control is not decoration:
   a hook that refuses everything passes both positives.
3. **GitHub's own >100 MiB rejection** -- the last resort, and the expensive one.
   By the time it fires the commit exists, and only a history rewrite removes it.

**Limit, stated:** git config is per-clone and is not cloned, so a FRESH CLONE
runs no hook until `githooks/install.sh` is run. Nothing inside a repository can
close that -- it is a property of git. Run it at checkout, with the layout
precondition above.

**When one gets in anyway:** `purge-path.sh` removes a path from history and
keeps every pointer that cited the re-shaed commits pointing at them -- tracker
files and commit messages both, in ONE pass, because shas are load-bearing data
here and two passes re-sha whatever the first just repointed. It previews every
substitution with its line, gates the result (tree change == the purged path
only, identity/dates/parents preserved, path and blob gone, every citation
reachable), and rolls back on any failed gate. `--prune` additionally drops the
pre-rewrite objects, which is irreversible.

## History before this repo

Before `eb9af25` (2026-07-22), these files evolved **inside the code
repository** (branch `master-beta`, up to `c523e3b1`) under former paths:

- `src/experimentalModule/` — `INTENT.md`, `dispatch-*.md`,
  `projection-paths.md`, `shadow-paths.md`, `strategy.txt`, `harness/`
- repo root — `USER_QUESTIONS.md`, `USER_QUESTIONS_ROUND2.md`,
  `FEATURE_REQUESTS.md`

`git log` in this repo only reaches 2026-07-22; for anything older, use the
code repo's log against those paths. (`DECISIONS_PENDING.md` and
`supervised-by.sh` were never tracked code-side; their history starts here.)
