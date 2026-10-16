# spacecrafter — session-start context (auto-loaded)

Authoritative detail lives in `claude/README.md` (repo contract) and `claude/INTENT.md`
(the live ledger). This file is the minimal map to reach them; on divergence they win.

## What the software is for (INTENT §2 — above every constraint; read it before minting or designing anything)

- **R1 potential expansion · R2 the use case: small interactive planetariums.** One person does the session with fewer than
  a hundred people and follows what exactly these people know and are curious about: no two sessions are predictable, the
  software must let him adapt to the unknown at any time. IN session only predefined things are usable — typing breaks
  immersion; OFF session is when content is added, things are modified and what would have been needed is looked at
  [vixy 2026-09-21, §11.272].
- **A need's ORIGIN is derived from the SHAPE of that space and from what you know of such places — never asked per user**:
  predict what the need will end up covering, then place it by the responsibility anchor, so that the structure is ready
  when the need shows up. It is G12 (S10), owner-ruled 2026-09-21, and it comes BEFORE the name pass (S4); a capability enters
  as a COMMAND the operator binds (one-line `.sts`), never as a new hardcoded key: his keyboard is fully allocated (§11.273).

## Two repositories, one tree

- **Code**: `/home/claude/spacecrafter` — branch **`main`** [vixy 2026-09-19: *"main branch is
  the new normal"*, §11.260]. `master-beta` was the working branch until that day (PRs →
  `2023-master`) and stays where it stood; harness trailers read `Code: main @ <sha>` from
  then on, older `Code: master-beta @ …` trailers stay valid (`supervised-by.sh
  --branch-alias=master-beta=main` judges them against `main`).
- **Two OS users share both trees** (`claude` = the sessions, `foxy` = the owner, who is in
  group `claude`): everything is group `claude`, group-writable, directories setgid, git
  `core.sharedRepository=group` (§11.260). A file that is not group-writable is a defect to
  fix at its cause, never to work around; a permission boundary is reported, not bypassed.
- **Reasoning/harness**: `/home/claude/spacecrafter/claude` — its OWN git repo, orphan
  branch `CC-harness`, same GitHub remote. Nested; the code repo ignores `claude/`.
- **Conventions** (full form in `claude/README.md`): every harness commit carries a
  `Code: <branch> @ <short-sha>` trailer; when a change spans both repos, commit code
  first; always `git -C <explicit path>` (cwd ambiguity = wrong-repo commits); never
  merge `CC-harness` into a code branch.

## Where things are

- `claude/INTENT.md` — single authority: §2.0 domain constraints D1–D14 (D8 as-if rule,
  D9 data-is-the-product/frozen-field, D10 optimize-the-potential, D11 1 ms/frame
  soft-realtime, D12 acting-defaults-logged, D13 downgrade-must-stay-possible, D14 pure-ASCII source), §5 defects, §11
  append-only journal, §13 open-item ledger (13.A = suspended for Vixy — blocked by
  protocol; 13.B = Fable territory). Header carries the maintenance invariant
  (supersession-with-record) + provenance tag grammar (`[stated:]` `[observed:]`
  `[measured]` `[derived]` `[vixy:]` …). Expanded §5/§11 entries live in
  `claude/INTENT/<id>.md` (2026-07-23 split); the in-file stub is a derived label —
  the entry file wins.
- `claude/DECISIONS_PENDING.md`, `claude/USER_QUESTIONS*.md`, `claude/FEATURE_REQUESTS.md`,
  `claude/SCRIPT_SURFACE.md` (script-surface owner's channel: witness/code
  divergences + surface decisions, SS-n ids) — derived views/channels; the
  ledger wins on divergence.
- `claude/harness/` — verification scripts (`b*_run.sh`, `SC_BIN` overridable,
  default `../../build-claude/src/spacecrafter`); `claude/harness/README.md` for use.
- `claude/SHAPE.md` — READ IT BEFORE TOUCHING CODE: how the code must be shaped (INTENT §2.1
  G12 (S1)–(S11), owner-ruled 2026-09-19 / 09-21 / 09-23), the owner's form measured, the shape instruments
  (`claude/harness/shape/`), the procedures, the state and the order of the open work
  (§13 B42–B46). The owner's own module, as the form to write in, is checked out read-only at
  `/home/claude/spacecrafter.owner-shape` (code `4dfe7bb3`). Work map of the deduplication:
  `claude/s33-second-managers.md` + `claude/INTENT/11.254.md`.
- `claude/harness/shape/review.sh` — the OWNER'S REVIEW QUEUE (code-repo ref
  `refs/review/headers`): what he has not read yet. Sessions never move the mark; every
  session close reports `review.sh status` to him.

## Standing rules (reasons in INTENT.md / README.md)

- **Code shape (G12) — fires at every code edit, every dispatch, every acceptance.** Never
  implement directly: every method NAME of the class against the intent, then the functions
  manipulating the members concerned, read from a current checkout; then use / extend
  minimally / build. One implementation per EFFECT, one entry point per INTENT (two intents
  over one law stay two functions; an alias goes). The owner's declared names and values ARE
  the ruling — consume them, never build beside them and ask. One responsible per MODE: old
  path = Projector / Navigator / Observer, new path = Camera, the gating in a wiring class of
  its own — never "inform both". Every duplication has a reason: recover it before touching it.
  A comment is a what-for, ONE line, verb first, only where the name does not carry it; no
  history, provenance, measurements or ledger references in code — git is code's archive, the
  ledger holds derivations. A check either fails LOUD or does not exist — a validate-and-continue, a
  swallowing catch, an unlogged default is a silent catch that breaks the propagation his acceptance
  relies on — and a failed check's information goes to the RESPONSIBLE of it, in the form decided at
  its anchor (S11, owner-ruled 2026-09-23, §11.289). Accept a delivery on behaviour AND shape (net code lines,
  deleted/added, manager count); one that only adds is a finding to explain. Executors
  [vixy 2026-09-27, §11.293; was "Fable only" from 2026-09-19 to 09-26]: STRUCTURE (the S4
  pass, the predicted shape, a PREDICTIONS block) = agent type `claude`, model `fable`;
  IMPLEMENTATION = agent type `opus-xhigh` (Claude Opus 5.5), bound to report EVERY conflict
  with the predictions — the conflict is the signal of what was not as predicted; it drives
  the rework. ROUTING [vixy 2026-10-10, §11.302; veto-open]: by the task's STRUCTURAL-COUPLING
  LOAD at the mint, never by its label — FOCUSED (input set closed and named in the section, one
  concern, criteria that can fail) → `opus-xhigh`, which runs under the OWNER's system prompt
  since that day (`claude/agents/deploy.sh` generates its projection; the warm-up asserts
  `deploy.sh --check`); OPEN-COUPLING (the S4 pass over a family, a shape still to choose, the
  owner to model, every mint and acceptance) → `claude` / `fable`; the structure / implementation
  split is the common case, not the rule (`fable-dispatch.md` §0b (a)).
- Old render path = comparison baseline, unchanged by construction **= everything in the old
  path is gated by the old path [vixy 2026-09-19, §11.253]**; parity per
  §11.52(b) (perceptual, conditioned on old being physically exact).
- Data values (poles, W0, physical constants): NEVER from recall — cited fetch only
  (§11.51(d) red line). Loaded data = `~/.spacecrafter/ssystem.ini` (ISO-8859).
- Encoding hazards, post-D14 state (records: §11.188(k), §11.189(a); rule rewritten
  at F70 acceptance — the pre-F70 wording lived here through 2026-08-31):
  (1) the Bash-tool `grep` wrapper is ugrep with `-I`: any file holding
  non-UTF-8-decodable bytes is classed BINARY and skipped SILENTLY. Since F70
  (code `cb521cf1`+`71ef30ac`) every tracked CONVERT-set file is pure ASCII —
  the hazard now lives ONLY in: `~/.spacecrafter/ssystem.ini` (untracked),
  `doc/superscript.sts`, and the EXCLUDE-listed files of
  `claude/harness/f70_partition.tsv` (witness fixtures, expected-output
  records, vendored trees, `data/`). `/usr/bin/grep` or Read on those.
  (2) `/usr/bin/grep -P '[\x80-\xff]'` under a UTF-8 locale matches CODE
  POINTS, not bytes — 0 hits on pure box-drawing. `LC_ALL=C` for byte classes.
  (3) The old "ISO-8859 file(s) in src/" diagnosis was WRONG: the
  `app_command_interface` pair were UTF-8 with one stray 0xA7 byte each —
  whole-file ISO-8859 decoding mojibakes such files; F70's greedy-UTF-8 +
  per-byte-fallback decoder is the reference method.
  (4) D14 standing gate: `python3 claude/harness/f70_ascii.py gate` — a new
  non-ASCII file in an unclassified path FAILS it; new source is pure ASCII,
  string literals by `\xNN` escape (§11.189(c), veto-open).
  (5) `/usr/bin/grep` does NOT parse `\xNN` inside a POSIX bracket expression: the
  literal `'[\x80-\xff]'` matches the characters `x 8 0 - f` and reports hundreds
  of false hits on a pure-ASCII file (measured 327/1162, §11.203(j)). Byte classes
  need `-P`, or bash `$'[\x80-\xff]'` quoting (the shell expands the bytes), or a
  Python byte census. The Bash-tool wrapper (ugrep) DOES parse the escape — which
  is why the literal form looks right there and lies under `/usr/bin/grep`.
  (6) This host's locale is FRENCH (`fr_FR`): under it `awk` parses `9.25e-08` as `9` with NO
  error — the decimal point is a comma, the parse stops at the `.` — and so do `sort -g` and
  `printf '%f'`; an executor's range check PASSED on truncated numbers (§11.334 (10), 2026-10-16).
  `LC_ALL=C` on every shell-side numeric read or comparison; Python's `float()` is locale-free
  (cross-project: `~/shared/QUEUE.md` Q-111).
- Suspended-for-Vixy items: work on them is blocked by protocol, not dependencies.
- Corrections propagate forward only (`spacecrafter-data` → future deliveries); the
  installed field is frozen — backward compatibility is forced (§2.0 D9).
- Fresh-launch precondition for measurements; config/ssystem md5 in==out asserted.
