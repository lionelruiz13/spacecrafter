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

- **SS-1** (ref: §5.80a) — Line 94 contains an INVISIBLE wrong character:
  a "no-break space" (one byte, 0xA0) where a normal space belongs. The
  engine only splits words on real spaces/tabs, so everything after it on
  that line shifts: **`albedo 1` is silently ignored**. Almost certainly
  a typing accident (Alt-Space or copy-paste). Decision: fix the byte?
  Status: OPEN.
- **SS-2** (ref: §5.80b) — Line 1167 `set home_planet Mars duration 5`:
  `duration` is not something `set` understands, and because of how the
  engine processes the pairs, **the whole line does nothing** — Mars
  never becomes the home planet there. Did you mean a gradual change
  (which `set` never supported), or is this a leftover? Status: OPEN.
- **SS-3** (ref: §5.80c) — Lines 303/306/309 use `date_display_number` /
  `date_display_position`; the engine's names are `datetime_display_…`
  (renamed at some point — the script predates the rename). The lines do
  nothing today. Fix spelling? Status: OPEN.
- **SS-4** (ref: §5.80c) — Line 930 `movetocity` no longer exists (the
  engine ignores the line), and line 912's `moveto … name marseille`
  carries the city name as decoration the engine never reads. Status:
  OPEN.
- **SS-5** (ref: §5.80c) — Lines 1133/1153 `set mode …` are accepted and
  report success but **do nothing at all** (the engine's handler for it
  is empty). Did `set mode` ever work, and what did it do? Your answer
  decides whether this is a lost feature or dead vocabulary. Status:
  OPEN.
- **SS-6** (ref: §5.80c) — Line 1205 `set stall_radius_unit 5.0` — and a
  quirk: values ≤ 1.0 are silently ignored by the engine. Worth knowing
  when you write it. Status: FYI unless your shows set small values.
- **SS-7** (ref: §5.80c) — Lines 1269/1271: `#` comments that are
  INDENTED. The engine only treats `#` as a comment at the very start of
  a line — indented ones are executed as (failing) commands. The lines
  were meant as comments. Fix indentation? Status: OPEN.
- **SS-8** (ref: §5.80 EXTENDED, :333) — `dso3d` lines use `zrot` /
  `yrot` for rotations; the engine's names are `yaw`/`pitch`/`roll`.
  **Both rotations do nothing.** Old names from an earlier engine, or a
  misunderstanding at writing time? Status: OPEN.
- **SS-9** (ref: §5.80 EXTENDED, :681/:769) — `image … spacecraft on` and
  `landscape … spacecraft on`: no part of the engine reads a `spacecraft`
  key on either command (we searched the whole source). What was it meant
  to do? Status: OPEN.
- **SS-10** (ref: §5.80 EXTENDED, :1366) — **`wait action reset_timer`
  does not wait and internally reports a failure** (the engine's `wait`
  only understands a duration, a `loading` form, and a video form). Was
  reset_timer a feature of an older engine? If your shows use it, they
  have a timing hole today. Status: OPEN — highest value answer in this
  section.

## 2. Script-surface behaviors that need YOUR intent, not a code fix

- **SS-11** (ref: §5.74) — `suntrace pen on`: today it does NOT aim the
  trace at the Sun — it starts drawing with whatever body was last
  traced. The code reads an (undocumented) `sun <name>` option in that
  spot instead of the Sun itself. Two opposite fixes exist; which
  behavior do your scripts expect? Status: OPEN.
- **SS-12** (ref: §5.79) — When the engine RECORDS a session, several
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
- **SS-14** (ref: §5.77) — Does any show write `configuration module …`?
  Today a star-catalogue save through it ALSO silently rewrites
  config.ini (destroying comments), and an unknown module re-runs the
  app's init mid-show. If nobody uses the spelling, the fix is free.
  Status: OPEN.
- **SS-15** (ref: §5.78) — Is the startup date mode ever written `Preset`
  (capital P) in configs? If so: the program starts on the preset date,
  but `date load preset` in a script jumps to the computer's date
  instead. Status: OPEN.
- **SS-16** (ref: §5.80 owed) — superscript.sts itself: fix the wrong
  lines in place (it stays the reference), or keep it untouched as a
  historical document and start a corrected copy? scedit can verify
  either against the engine from now on. Status: OPEN.

## 3. Pending — full divergence table

A systematic pass over superscript.sts's comment layer and usage (what
the file *teaches* vs what the engine *does*, both directions, every
command it never exercises listed) is running; its table lands here
when verified. Status: IN PROGRESS (2026-08-04).
