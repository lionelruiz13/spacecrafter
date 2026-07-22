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
