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
- Suspended-for-Vixy items: work on them is blocked by protocol, not dependencies.
- Corrections propagate forward only (`spacecrafter-data` → future deliveries); the
  installed field is frozen — backward compatibility is forced (§2.0 D9).
- Fresh-launch precondition for measurements; config/ssystem md5 in==out asserted.
