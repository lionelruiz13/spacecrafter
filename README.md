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
DERIVATION does not, and the reason is an asymmetry rather than a size: a
compiled artifact is the DETERMINED end of a one-way transformation. The source
determines the binary; the binary does not determine the source, because
compilation destroys exactly the information that would let you go back. Of the
two ends, one carries what the other cannot be recovered from -- and that is the
one a record keeps. So:

    reconstructible from the sources  ->  redundant beside them
    NOT reconstructible               ->  unreliable, nothing can check it

Either way it does not belong. Size was never the criterion; it is what made
this visible, once, at 187 MiB (INTENT 11.137). A 12 KiB `.pyc` is exactly as
much a derivation as a 187 MiB executable, and a size gate never sees it.

Four layers, ordered by how directly each one encodes that reason:

1. **Type -- `githooks/pre-commit`, the criterion.** `file(1)` reads the staged
   blob's CONTENT (names lie) and anything it identifies as compiled output --
   ELF, Mach-O, PE, ar archive, Java class, `.pyc`, WebAssembly, core dump,
   object file -- is refused **unconditionally**: no threshold, no exemption,
   because there is no size at which a compiled object becomes an origin. A
   second channel checks magic bytes directly, so the gate still holds where
   `file(1)` is absent or has reworded its output; the two are OR'd, so drift
   between them can only widen the net.
2. **Path -- `.gitignore`, the convention.** `artifacts`, and the staging-binary
   name shapes (`harness/sc_f*_pre`, `harness/sc_f*_child`,
   `harness/spacecrafter_pre_*`). This is what covers the harness's own commonest
   derivation -- measurement output, PNG and logs -- which no type detector can
   tell from source data. Names, so it is a proxy: a new name walks past it.
3. **Size -- `githooks/pre-commit`, the residue.** Blobs >= 8 MiB
   (`hooks.maxBlobBytes`). This layer makes a *different claim* from layer 1: it
   has decided nothing. Reconstructibility is semantic -- a 40 MiB capture log
   and a 40 MiB star catalogue are the same bytes to any detector -- so it stops
   on "large, and unclassifiable" and puts the question where the answer lives.
   The number is read off the corpus, not chosen: the largest legitimately
   tracked blob across both repos is 3.06 MB. It refuses rather than warns only
   because a warning printed under `git add -A` is scrollback.
4. **GitHub's >100 MiB rejection** -- the last resort, and the expensive one. By
   the time it fires the commit exists, and only a history rewrite removes it.

Install with `githooks/install.sh`: it wires `core.hooksPath` in BOTH repos and
then PROVES the hook fires, against a throwaway index so neither working tree is
touched -- 8-byte ELF refused (size-independence, the point layer 3 cannot make),
Java class refused, `.pyc` refused (that one has no magic-byte fallback, so it is
the leg that dies if the `file(1)` channel dies), 9 MiB text refused by the
residue layer, and an ordinary file PASSED. The negative control is not
decoration: a hook that refuses everything passes every positive.

Measured, both repos, every tracked file staged as new: **0 refusals across
3 175 files**. Known blind spots, named rather than left to be discovered: an
archive *containing* binaries (`.jar`, `.whl`, a build tarball) reads as "Zip
archive data" and is indistinguishable from packaged source data, so it falls to
layer 3; and layer 2 is the only thing standing between measurement artifacts and
the history.

**Limits, stated.** `--no-verify` bypasses every hook, including the type
refusal, and git offers no way to prevent that; the only unbypassable layer is
server-side, which GitHub does not offer outside Enterprise. And git config is
per-clone and is not cloned, so a FRESH CLONE runs no hook until
`githooks/install.sh` is run. Neither is closeable from inside a repository. Run
the installer at checkout, with the layout precondition above.

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

## sha-maps/ -- resolving a commit sha a rewrite invalidated

`supervised-by.sh` re-hashes every commit it touches and repoints citations only
in tracked `*.md` of this repository **[CORRECTED 2026-09-07 at F103's acceptance
(supervisor): this sentence first named `purge-path.sh` beside it — that script
rewrites the same way and does NOT write a map (0 mentions of `sha-maps` in it);
its map is OWED as an instrument residue, not claimed here]**. Everything else that
cites a commit by sha -- the git history of those same files, notes outside this
pair, the archive drawer, the owner's own trees -- cannot be reached by any
rewrite. So every run that changes a sha now writes its old->new map to

    sha-maps/<UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>/
        code.tsv  harness.tsv  repair.tsv        # <old-full-sha> TAB <new-full-sha>

and the closing commit carries it. The directory is created by the first run that
needs it; `sha-maps/README.md` is written once beside the maps and carries the
contract. A sha that resolves to nothing reachable is looked up as a PREFIX in
those files, newest directory first -- resolution by convention, the same shape
the archive drawer uses for moved documents. The maps are append-only and are
never edited: a map that has been corrected has stopped being evidence of what a
particular run did, which is the only thing it is for. (Landed F103, 2026-09-07;
reasons and the measurements behind them in INTENT 11.224.)

**What is IN a map, corrected 2026-09-07 (F106, INTENT 11.228).** The rule used to
be read as *"a commit that rebuilt byte-identical kept its sha and appears in no
file"*. Since F106 nothing rebuilds byte-identical, because an unselected commit
whose parents did not change **is not rebuilt at all**: `supervised-by.sh` runs a
`--commit-filter` that emits the ORIGINAL commit id when the five fields a
signature signs (tree, parents, author, committer, message) are unchanged, so
that commit keeps its object -- its sha AND its `gpgsig` -- and appears in no
file. Measured on the F103 clone pair: 79 of 94 code commits in the map, the
15-commit side chain untouched, `cebebf44` still signed. The one commit a map
CANNOT keep whole is a signed one whose parent this run rewrote; the run names it
in a section-2(f) block and again in its closing summary.
