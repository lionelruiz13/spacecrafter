# Script-surface findings & decisions — for the script-surface owner

**Audience**: the main user/tester — the person who writes most scripts,
owns the script surface, and drives the project's development
[vixy 2026-08-04]. Implementation knowledge NOT assumed (per the
USER_QUESTIONS conventions): every entry names the observable, never the
engine's internals. Relay: Vixy, as for USER_QUESTIONS rounds.

**Why this file exists**: `doc/superscript.sts` served for years as the
documentation and the only functional test [vixy 2026-08-04]. A new tool
(scedit, a script editor/checker) now compares that file and the whole
script surface against what the engine actually does — and the
divergences it finds belong to you, not to Vixy: for each one, only you
can say whether the script was right (the engine drifted or misread) or
the script was wrong (a writing-time slip), and what your shows rely on.

**Conventions**: each entry has a stable ID (SS-n), a `(ref:)` back to
the tracking ledger, and a Status. Answers update the Status here and
propagate to the ledger row — the ledger (`claude/INTENT.md`) wins on
divergence. Nothing here is a reproach of the file: its errors survived
because nothing checked them; now something does, in both directions.

---

## 1. Lines in superscript.sts that do not do what they say

Each: what the line looks like it means → what actually happens today →
the decision owed (fix the line / the engine should honor it / keep and
document).

> **THE FILE CHANGED on 2026-08-26 — you rewrote it (`f0c8ef83`, +267/−67,
> 1407 → 1606 lines), and eleven of the thirteen lines listed below are
> already gone.** We checked every one, byte by byte, against the version
> before your edit; nothing was changed by us. Line numbers in the entries
> below are the OLD ones — the pre-edit file is still readable in the
> project's history if you ever want it (`70dee810:doc/superscript.sts`),
> which matters because it is the record of what the script taught for
> years. Per-entry results are marked **[2026-08-26]** in each Status.
> **Still standing:** the landscape `spacecraft on` line (SS-9), which is
> unchanged, and the selection-number table (SS-17). **Two things we could
> NOT check**: the 267 lines you ADDED are unexamined, and the checker that
> found all of this cannot be re-run from this working copy — so this is a
> "did the old problems go away" check, not a fresh pass over the new file.
> **And a deletion is not an answer**: where an entry asks *did this ever
> work / what was it for* (SS-5, SS-8, SS-9, SS-10), removing the line tells
> us you judged it dead or a slip — it does not tell us the history, and for
> three of them a 2020 reference document says the spelling was real. Those
> questions stay open and are still worth a sentence each.
> *(Recorded in the tracking ledger at INTENT §11.149(e) / §5.97.)*

- **SS-1** (ref: §5.97a) — Line 94 contains an INVISIBLE wrong character:
  a "no-break space" (one byte, 0xA0) where a normal space belongs. The
  engine only splits words on real spaces/tabs, so everything after it on
  that line shifts: **`albedo 1` is silently ignored**. Almost certainly
  a typing accident (Alt-Space or copy-paste). Decision: fix the byte?
  Status: **[2026-08-26] DONE by you** — line 125 of the new file has a
  normal space and `albedo 1` now applies. (Three no-break spaces remain in
  the file, all inside `#` comment lines, where they are harmless.)
- **SS-2** (ref: §5.97b) — Line 1167 `set home_planet Mars duration 5`:
  `duration` is not something `set` understands, and because of how the
  engine processes the pairs, **the whole line does nothing** — Mars
  never becomes the home planet there. Did you mean a gradual change
  (which `set` never supported), or is this a leftover? Status:
  **[2026-08-26] line FIXED by you** (now `set home_planet Mars`, line
  1314) — **the question is not**: a 2020 reference document attests
  `set … duration`, so whether it once worked is still worth knowing.
- **SS-3** (ref: §5.97c) — Lines 303/306/309 use `date_display_number` /
  `date_display_position`; the engine's names are `datetime_display_…`
  (renamed at some point — the script predates the rename). The lines do
  nothing today. Fix spelling? Status: **[2026-08-26] DONE by you** — all
  three respelled `datetime_display_*` (lines 373/376/379). Respelled, not
  deleted, which is the answer as well as the fix.
- **SS-4** (ref: §5.97c) — Line 930 `movetocity` no longer exists (the
  engine ignores the line), and line 912's `moveto … name marseille`
  carries the city name as decoration the engine never reads. Status:
  **[2026-08-26] both lines REMOVED by you** — question still open, same
  reason as SS-2 (the 2020 reference attests `moveto … name`).
- **SS-5** (ref: §5.97c) — Lines 1133/1153 `set mode …` are accepted and
  report success but **do nothing at all** (the engine's handler for it
  is empty). Did `set mode` ever work, and what did it do? Your answer
  decides whether this is a lost feature or dead vocabulary. Status:
  **[2026-08-26] both lines REMOVED by you — the QUESTION is still the
  open one**, and it is the whole entry: removing them is consistent with
  "dead vocabulary" but does not say so, and the engine's empty handler is
  still there accepting the spelling and reporting success.
- **SS-6** (ref: §5.97c) — Line 1205 actually reads
  `set stall_radius_unit = 5.0` — **the `=` sign breaks it**: the engine
  has no `=` syntax, so `=` is taken as the value and `5.0` is dropped
  entirely. The line sets nothing (and the engine also silently ignores
  values ≤ 1.0, worth knowing). *(Corrected 2026-08-04: an earlier
  version of this entry quoted the line without the `=`.)* Status:
  **[2026-08-26] the `=` is GONE** (line 1358 now reads
  `set stall_radius_unit 5.0`), so the value reaches the engine. The
  second half of the entry is untouched and we did not re-test it: the
  engine silently ignores values ≤ 1.0, and 5.0 is above that, so this
  should now do something — worth one run to confirm it does what you
  expect.
- **SS-7** (ref: §5.97c) — Lines 1269/1271: `#` comments that are
  INDENTED. The engine only treats `#` as a comment at the very start of
  a line — indented ones are executed as (failing) commands. The lines
  were meant as comments. Fix indentation? Status: **[2026-08-26] DONE by
  you** — the new file has no indented `#` at all.
- **SS-8** (ref: §5.97 EXTENDED, :333) — `dso3d` lines use `zrot` /
  `yrot` for rotations; the engine's names are `yaw`/`pitch`/`roll`.
  **Both rotations do nothing.** Old names from an earlier engine, or a
  misunderstanding at writing time? Status: **[2026-08-26] line FIXED by
  you** — it now reads `yaw 0 pitch 0 roll 0` (line 405), so the rotations
  work. The history question is answered by implication and needs no reply.
- **SS-9** (ref: §5.97 EXTENDED, :681/:769) — `image … spacecraft on` and
  `landscape … spacecraft on`: no part of the engine reads a `spacecraft`
  key on either command (we searched the whole source). What was it meant
  to do? Status: **[2026-08-26] HALF and HALF — this is now the sharpest
  entry in the section.** The `image … spacecraft on` line is gone; the
  `landscape … spacecraft on` line is **still there, byte for byte** (line
  875), still with the comment above it that says *"Draw a landscape that
  will resist to take-off (as a spacecraft cockpit)"*. So the file still
  teaches a key the engine does not read, for a behaviour the comment
  describes precisely. **What was the cockpit-that-survives-take-off meant
  to do, and do your shows rely on it?**
- **SS-10** (ref: §5.97 EXTENDED, :1366) — **`wait action reset_timer`
  does not wait and internally reports a failure** (the engine's `wait`
  only understands a duration, a `loading` form, and a video form). Was
  reset_timer a feature of an older engine? If your shows use it, they
  have a timing hole today. Status: **[2026-08-26] line REMOVED by you**
  (every `wait` in the new file is `wait duration …`) — **the question is
  still the valuable one**: the 2020 reference attests the spelling, so if
  any of YOUR other shows still use it they have a silent timing hole that
  removing it from this one file does not touch.

## 2. Script-surface behaviors that need YOUR intent, not a code fix

- **SS-11** (ref: §5.91) — `suntrace pen on`: today it does NOT aim the
  trace at the Sun — it starts drawing with whatever body was last
  traced. The code reads an (undocumented) `sun <name>` option in that
  spot instead of the Sun itself. Two opposite fixes exist; which
  behavior do your scripts expect? Status: OPEN.
- **SS-12** (ref: §5.96) — When the engine RECORDS a session, several
  commands are written back transformed: `set a 1 b 2` is recorded
  twice; `clear` is recorded as its ~35 internal commands; a
  media-audio line is recorded rebuilt (a filename with spaces breaks);
  `flag`/`timerate` record computed results instead of what you typed.
  **What should a recording contain — what you wrote, or what the engine
  did?** Both answers currently coexist. Status: OPEN — this decides an
  entire defect class.
- **SS-13** (ref: §5.36) — `media subtitle toggle` today turns subtitles
  OFF (a one-word bug, known); the working spelling is
  `media action toggle subtitle <anything>`. The editor must document ONE
  as the way to write it — which? Status: OPEN.
- **SS-14** (ref: §5.94) — Does any show write `configuration module …`?
  Today a star-catalogue save through it ALSO silently rewrites
  config.ini (destroying comments), and an unknown module re-runs the
  app's init mid-show. If nobody uses the spelling, the fix is free.
  Status: OPEN.
- **SS-15** (ref: §5.95) — Is the startup date mode ever written `Preset`
  (capital P) in configs? If so: the program starts on the preset date,
  but `date load preset` in a script jumps to the computer's date
  instead. Status: OPEN.
- **SS-16** (ref: §5.97 owed) — superscript.sts itself: fix the wrong
  lines in place (it stays the reference), or keep it untouched as a
  historical document and start a corrected copy? scedit can verify
  either against the engine from now on. Status: **[2026-08-26] ANSWERED
  BY ACTION — you fixed it in place**, which is a clear answer and needs no
  reply. One consequence recorded so nobody trips on it later: the "what
  the script taught for years" evidence now lives in the project's history
  rather than in the file, at `70dee810:doc/superscript.sts`. Any future
  entry citing an old line number means that version, not the current one.

## 3. Full divergence pass — landed 2026-08-04

The systematic pass is done (full detail:
`util/scedit/grammar/witness/` in the code repo — 477 comment lines and
1319 written pairs, all accounted). What changes the entries above:

- **SS-2, SS-4, SS-10 shift from "probable slip" toward "feature that
  went away"**: a SECOND, independent documentation (a French per-command
  reference from 3/06/2020 found in `util/new_parser_scripts/`) attests
  all three spellings — `set … duration`, `moveto … name`, and
  `wait action reset_timer` were documented behavior in 2020. Your
  memory of whether they once worked is now the deciding evidence.
- **SS-11 gains documentary evidence**: the same 2020 reference defines
  `sun_trace` as "un alias de la commande body_trace avec le soleil
  comme astre sélectionné" — i.e. `suntrace pen on` WAS meant to aim at
  the Sun. Today's engine does not do that. Fix direction now has a
  source; still yours to confirm.
- **SS-5 gains intent**: the witness's own comments describe `set mode`
  as what `mode jump` does today — likely superseded vocabulary, not a
  lost feature.

New entries:

- **SS-17** (ref: §5.98) — **Saturn and Ganymede are misspelled inside
  the engine's selection-number table** ("Satun", "Ganymed"): selecting
  either makes `$body_selected` answer 999 (= nothing special selected),
  so `struct if body_selected equal 600` (Saturn) or `503` (Ganymede)
  never fires. Every other body matches its documented number. Question:
  do any of your shows test those two numbers (or work around the 999)?
  The fix is two spellings once you confirm nothing relies on today's
  behavior. Status: **OPEN — untouched by the 2026-08-26 rewrite.** The
  number table is still in the file (line 1529) and still reads
  `Ganymed=503` / `Saturn=600`, so the mismatch is exactly as it was: the
  engine agrees with the table on `Ganymed` and disagrees on `Saturn`,
  while the actual body names are `Ganymede` and `Saturn`.
- **SS-18** — 15 more teach-vs-do divergences catalogued (full list:
  `witness/superscript-witness.json`), the sharpest for you:
  (a) lines 121-123 `*_altimetry_factor` — renamed engine-side
  (tesselation vocabulary unified), the old spellings do nothing;
  (b) lines 249/251/252 `configuration module …` — **running the
  reference script end-to-end rewrites your config.ini twice and
  re-runs the app's init mid-script** (SS-14's fall-through, live in
  the reference file itself); (c) 4 × `image … action twice` — not an
  image action (one line even labels itself "SC2020 and earlier");
  (d) the whole `dso3d` demo section uses `action reset`, which does
  not exist; (e) `set heading +15` is ABSOLUTE 15, not +15 relative.
  Status: OPEN (bulk answer fine — "fix all that match my intent").
  **[2026-08-26] partial, spot-checked while verifying section 1 — NOT a
  full re-pass:** (a) all three `*_altimetry_factor` lines are still there
  (162-164); (b) all five `configuration module` lines are still there
  (312-318), so running the reference script end-to-end **still rewrites
  your `config.ini` twice and re-runs the app's init mid-script** — this is
  the one worth acting on; (c) 3 → 2; (d) gone; (e) gone. The other ten of
  the fifteen were not re-checked.
- **SS-19** — The reference script **never exercises 12 of the 62
  commands**: domemasters, dso2d, flyto, galaxy_stars, get, modulo,
  search, session, shutdown, sub, suntrace, transition. Nine of those
  have no documentation anywhere in the tree except the code itself.
  FYI + invitation: if you have shows exercising them, they are
  corpus gold. Status: FYI.
