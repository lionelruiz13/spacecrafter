# spacecrafter — session-start context (auto-loaded)

Authoritative detail lives in `claude/README.md` (repo contract) and `claude/INTENT.md`
(the live ledger). This file is the minimal map to reach them; on divergence they win.

## Two repositories, one tree

- **Code**: `/home/claude/spacecrafter` — branch `master-beta` (PRs → `2023-master`).
- **Reasoning/harness**: `/home/claude/spacecrafter/claude` — its OWN git repo, orphan
  branch `CC-harness`, same GitHub remote. Nested; the code repo ignores `claude/`.
- **Conventions** (full form in `claude/README.md`): every harness commit carries a
  `Code: <branch> @ <short-sha>` trailer; when a change spans both repos, commit code
  first; always `git -C <explicit path>` (cwd ambiguity = wrong-repo commits); never
  merge `CC-harness` into a code branch.

## Where things are

- `claude/INTENT.md` — single authority: §2.0 domain constraints D1–D13 (D8 as-if rule,
  D9 data-is-the-product/frozen-field, D10 optimize-the-potential, D11 1 ms/frame
  soft-realtime, D12 acting-defaults-logged, D13 downgrade-must-stay-possible), §5 defects, §11
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

## Standing rules (reasons in INTENT.md / README.md)

- Old render path = comparison baseline, unchanged by construction; parity per
  §11.52(b) (perceptual, conditioned on old being physically exact).
- Data values (poles, W0, physical constants): NEVER from recall — cited fetch only
  (§11.51(d) red line). Loaded data = `~/.spacecrafter/ssystem.ini` (ISO-8859; the
  Bash-tool `grep` wrapper silently excludes untracked ini files — use `/usr/bin/grep`
  or Read).
- Suspended-for-Vixy items: work on them is blocked by protocol, not dependencies.
- Corrections propagate forward only (`spacecrafter-data` → future deliveries); the
  installed field is frozen — backward compatibility is forced (§2.0 D9).
- Fresh-launch precondition for measurements; config/ssystem md5 in==out asserted.
