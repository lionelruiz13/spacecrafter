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
>
> **[2026-08-30] The two gaps above are closed**: the checker was rebuilt
> and re-run on your new file, and the 267 added lines are now examined.
> The fixes hold — nothing listed as fixed above re-fired. The ADDED lines
> carry a crop of their own: **section 4 below (SS-20…24)**, plus one
> half-fix recorded in SS-3's status.

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
  deleted, which is the answer as well as the fix. **[2026-08-30] One of the
  three is still broken**: the checker now ran on the new file, and the
  `flag datetime_display_number` line (373) still does nothing — there IS no
  on/off switch of that name, under either spelling; only the two `set …`
  lines exist. If switching the multi-date display on and off matters to
  your shows, that switch is missing (or spelled something we don't know).
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
  corpus gold. Status: FYI. **[2026-08-30] Your rewrite exercises three of
  them** — `transition`, `dso2d` and `domemasters` now appear in the file.
  Still never exercised: flyto, galaxy_stars, get, modulo, search, session,
  shutdown, sub, suntrace (nine — and note line 1507 writes `mod`, which
  misses `modulo`: SS-22).

## 4. The 267 added lines — first full pass, 2026-08-30

The checker was rebuilt and run over your rewritten file (the pass the
2026-08-26 note said could not be done yet). The good news dominates:
nearly all the new material reads exactly the way the engine will read it —
the new sections (transition, `wait loading`, preload, DSO2D, `landing`,
media loop/speed/subtitles, the newly documented flags) are clean. Five
things are not:

- **SS-20** — Lines 37–46, the new "Usage example": comments at the END of
  command lines (`media action pause   # Stop video & sound`). A comment
  only exists when `#` starts the line — mid-line, the `#` and every word
  after it are read as more parameters. On these 8 lines the extra words
  happen to be ignored, so the example WORKS today — but it teaches a
  syntax the engine does not have, and a copied trailing comment can
  silently change a line's meaning elsewhere. Fix: move each comment to its
  own line? Status: **[2026-08-30] RESOLVED ENGINE-SIDE by Vixy's ruling** —
  mid-line `#` will become a real comment in the engine, so your inline
  comments become correct as written; no need to move them. Until that
  engine change ships, these 8 lines keep working by accident (the junk
  words are ignored), which is now understood as the reason the pattern
  survived. [2026-08-31] scedit briefly named these eight lines with ONE
  finding each (`inline-comment`); **later the same day the engine change
  LANDED in the code tree (`3d9179d2`): a `#` outside quotes is a comment,
  to the end of the line, on every channel — your eight lines are correct
  as written and scedit reports nothing on them.** Two things to know:
  a `#` inside `"…"` stays text (so a quoted title may contain one), and an
  indented `#` is now a whole-line comment too (98 lines across the shipped
  scripts used to execute as unknown commands and now do nothing, as their
  authors meant). Status: **RESOLVED in the tree** — live confirmation on
  a running engine still owed (no display on the build machine).
- **SS-21** — Lines 940/945: the two `(Warning! Don't forget …)` advice
  lines have no `#`, so each one is executed as a command called
  `(Warning!` and fails. The advice itself is kept and valuable; the lines
  just need the `#`. Status: OPEN.
- **SS-22** — Line 1507 `mod a 2`: the engine's name for this command is
  `modulo` — `mod` does nothing here (worse, the engine's own suggestion
  answers "did you mean mode?", pointing away from the intent). Question,
  same class as SS-2: does YOUR current engine accept `mod` (an alias newer
  than this tree), or is it a slip? Status: **[2026-08-30] ANSWERED by
  Vixy's decision** — `mod` becomes a real alias of `modulo` (with `div`
  and `mul` for divide/multiply); your line is correct as written once
  that engine change lands. Until then it still does nothing.
  **[2026-08-31] RESOLVED IN-TREE: `mod`, `div`, `mul` are registered as
  aliases of `modulo`, `divide`, `multiply` (app_command_init.cpp:116-118);
  your line is correct as written. scedit no longer reports it (corpus
  record 16 → 15).**
- **SS-23** — Lines 1536/1539, the language examples:
  `text "behobachter"` / `text "observateur"`. `text` needs its named parts
  (`name … string … action load` — your own line 1444 shows the full form);
  a lone quoted word is dropped silently, and these two lines draw nothing.
  Same question as SS-22 — newer-engine shorthand, or a compressed example?
  Status: OPEN.
- **SS-24** — Line 1547 `struct if current_mode equal 0` has **no
  `struct if end`**. Everywhere else in the file the pair is closed; here,
  whenever the mode test fails (any mode other than the solar system), the
  engine skips EVERY line after 1547, to the end of the script. In this
  file that is the last ~60 lines (the wait/media/zoom examples); in a
  show, the same shape silently cuts the whole tail. Fix: add the `end`.
  Status: OPEN. **[2026-08-31] AMENDED — it is worse, and it starts earlier:**
  the syntax catalogue under `# STRUCT`, lines **1404-1409** (`struct if a
  inf b` … `struct if a diff b`, the six comparison forms shown one per
  line), has no `end` either — six more open blocks. And one of the first
  two is ALWAYS taken as "skip": `a inf b` skips when a ≥ b, `a sup b` skips
  when a ≤ b, and one of those is true whatever a and b are (they are not
  even defined in the file, so both read as 0). Consequence: **every line
  after 1405 is skipped every time this file runs** — the 200-line tail
  (`struct comment`, `struct loop`, the variables, the languages, the wait /
  media / zoom examples) never executes, not only when the mode test fails.
  Fix: close each catalogue line with `struct if end` (or turn the six lines
  into comments — they document syntax, they were not meant to run).
  scedit now reports all seven (`unclosed-struct`, at the opener line).
- **SS-25** — `fscripts/panorama5.sts` line 102 (and the identical copy in
  `navigation/fscripts/panorama5.sts`): the last block, lines 96-97, opens
  TWO `struct if` and lines 100-102 close THREE — one `struct if end` too
  many, copied from the earlier blocks that open three. The engine reports it
  and ignores the extra line, so the script works; that report is the only
  trace. Fix: delete line 102 (in both copies — the
  two files are byte-identical, so a fix in one and not the other leaves
  them diverging). Status: OPEN. **[2026-09-01] Confirmed on a running
  engine, and this entry had one detail wrong.** The checker now finds
  these two lines by itself (they were found by hand in August), and a
  test run reproduced exactly what happens: the engine writes an ERROR
  line naming the file and the line, AND it writes the explanation into
  the script itself, at the end of line 102, after a `#!` marker — so
  after a run the file tells you what is wrong with it. What this entry
  used to say, that the engine logs the words "end without if", stopped
  being true on 2026-08-31 when that message was replaced by the fuller
  sentence; the checker was saying the same outdated thing and has been
  corrected too. Nothing about the fix changes: delete line 102, in both
  copies.

(Minor, no decision needed: the new comet-tails demo line 181 repeats
`halo true` twice, like the old Wirtanen line — harmless, the engine keeps
the last value.)

## 5. The added lines read for CONTENT — 2026-08-31

Section 4 ran the CHECKER over your rewrite: it asked whether each new line
PARSES the way you meant. This pass asks the other question, which no checker
can: does the new prose TELL THE TRUTH about what the engine does? Every line
added since the previous revision was read against the handler that serves it.

**The headline is good.** 48 of the new comments answer something the project
had open, and several answer it exactly right — the 16 comet-tail keys with
their defaults (lines 180-200) all match the loader; `$language` = first letter
x 100 + second letter (de=405, fr=618, en=514, es=519) is correct on all four;
`moon_brightness` 0.5 and `sun_brightness` 200 are the real defaults; the whole
new TRANSITION section and `wait loading` document two commands that had no
documentation at all anywhere. **Six divergences banked in earlier passes are
FIXED by this revision**, including the invisible character on the old line 94
(SS-1) — a byte census confirms no command line in the file now carries one.
`movetocity` and `wait action reset_timer` were correctly deleted.

Six new things. Two of them are worth your attention beyond a text fix.

- **SS-26** — Line 988, `media action play … pause on`, documented as starting
  the video paused. It does not: the video plays. **`on` is not a word this
  particular key understands.** Everywhere in `flag`, the words `on`, `true`
  and `1` all mean yes. But `media`'s `pause` key, `configuration`'s
  `binary_mode`, and the true/false keys of the stellar-system files
  (`halo`, `rings`, `close_orbit`, `has_atmosphere`, `hidden`) go through a
  DIFFERENT reader that accepts only `true` and `1` — `on` falls through to
  "no", silently, and the command reports success. Write `pause true` and it
  works. We checked the installed `ssystem.ini`: every one of those keys is
  written `true` or `false` there, so no shipped body data is affected today.
  Two questions, and only you can answer the first: (a) is there any show of
  yours writing `on`/`off` for those keys? (b) Should the engine be made to
  accept the same words everywhere — this is a real inconsistency and Vixy
  would have to rule on it, but it is your surface that would change.
  **CONFIRMED ON A RUNNING ENGINE 2026-09-01** (F77, INTENT §11.196(f)): we put
  the two readers to the engine on the SAME launch, one after the other, and
  they disagreed about the same word. `configuration module star_lines action
  save … binary_mode on` and the same line with `binary_mode off` wrote
  **byte-identical text files** (157 573 B each), while `binary_mode true`
  wrote a binary one (80 720 B) — so `on` was read as NO. In the same run,
  `flag atmosphere on` left the flag ON. All of these lines were echoed by the
  engine, none was refused, and every one of them was written into the engine's
  own recording as a success. We drove `binary_mode` rather than `media …
  pause` because `binary_mode` leaves a file we can look at and `pause` leaves
  nothing any channel reports; it is the same reader, so the finding is the
  same one — but if you want the `media` line itself driven, say so and it will
  need a video and a way to ask the player whether it is paused.
  Status: OPEN (question (a) is still yours, and it is the one that decides
  whether this is a documentation fix or a compatibility problem).

- **SS-27** — Lines 1478-1479, `transition action skip duration 1`, documented
  as "All faders to off in 1 second". The duration is in **MILLISECONDS**, so
  that line advances the faders by one thousandth of a second. To get the
  second you describe, write `duration 1000`. (For reference, when you leave
  `duration` off entirely the engine uses 3600 — i.e. 3.6 s — with the comment
  "ensure transitions complete now".) Fix: change the number, or the comment,
  whichever you meant. Status: OPEN.

- **SS-28** — Lines 127-131, `body action load mode in_stellar_system …`. The
  spelling `in_stellar_system` appears **nowhere in the engine**; the engine
  knows `in_stellarsystem` and `instellarsystem` (no underscore before
  "system"). And on a `body` line the `mode` key is only consulted for the
  three 3D-model modes, so on these two lines it is dropped either way and the
  new planet and moon are created in whatever system you are currently in.
  That may well be what you want — but the comment says the mode chooses, and
  it does not. Question: did you intend these bodies to land in a NAMED system?
  If so that is a missing capability, not a typo. Status: OPEN.

- **SS-29** — Line 373, `flag datetime_display_number`. There is no flag by
  that name (the line is accepted and does nothing). Same family as SS-3, which
  was about the `date` command's own spellings; this one is on `flag`.
  Status: OPEN.

- **SS-30** — a REMOVAL to double-check. The old lines 1142-1144 documented
  `set line_width 1.5`, and the string `line_width` now appears **zero** times
  in the file. The setting is still live and still registered in the engine, so
  the effect of the removal is that a real control is now documented nowhere at
  all. Deliberate (it is being retired) or lost in the rewrite?
  Status: OPEN.

- **SS-31** — **the missing-documentation list, as a proposal.** Not a
  divergence: a map of what the engine accepts that your file never
  demonstrates, so you can decide what deserves a line. Machine-readable at
  `claude/harness/artifacts/f71/missing_doc.json`; the criterion is
  deliberately generous (a name counts as covered if it appears anywhere at
  all), so everything listed is a genuine gap and the real gap is larger.

  | | never in the file | driven, but never this way | in comments only | exercised |
  |---|---|---|---|---|
  | commands (65) | 7 | - | 4 | 54 |
  | argument keys (324) | 15 | 33 | 4 | 272 |
  | flags (97) | 6 | - | - | 91 |
  | colour names (46) | 5 | 23 | - | 18 |
  | `set` names (43) | 8 | 3 | 1 | 31 |
  | font targets (10) | - | - | - | 10 |

  The colour surface is the striking one: **only 18 of the 46 colour names are
  ever driven as colours**. Many of the other 28 appear in the file as FLAG
  names (`flag analemma on`) but never as `color property analemma …`, so an
  author reading your file learns the thing exists and not that its colour can
  be set. Never in the file at all: commands `session`, `galaxy_stars`, `sub`,
  `suntrace`, `flyto`, `div`, `mul`; flags `navigation`, `astronomical`,
  `loxodromy`, `orthodromy`, `experimental_shadows`, `experimental_path`;
  `set` names `line_width` (see SS-30), `srt_locale`, `ui_locale`,
  `star_size_limit`, `planet_size_limit`, `star_scale`,
  `text_fading_duration`, `zodiacal_intensity`. `loxodromy` and `orthodromy`
  are absent as both flag and colour — a whole feature with no line anywhere.
  Status: OPEN, no action required — this is a menu, not a bug list.

Eight further divergences were found by the same pass and are recorded with
their code anchors in `claude/harness/f71/superscript_delta.json`
(`divergences`), but are NOT written up as SS rows here because they were not
independently re-verified at source: `illuminate … size` (line 747),
meteor ZHR (1021), `init_fov` and the [Home] key (1316), what `transition
action skip` settles faders TO (1475), the SRT-only `media` line (992),
`mode jump in_sandbox` (1062), the lift-off demo's landscape line (898), and
`dso3d`'s default rate (403). Each carries a file:line so you can judge it
directly. Also noted there: three comment lines still hold invisible
characters (619, 1245, 1257) — harmless where they are, since comments are
never parsed.

## 6. The installed shows, checked for the first time — 2026-09-01

Everything above came from `doc/superscript.sts`, the reference script. This
pass is different: the checker was run over **all 408 scripts installed on this
machine** — your shows, the navigation scripts, the internal ones — and every
single thing it said was then checked against the engine's own code, and eight
of them against a running engine.

**The headline is that the checker was right 1661 times out of 1661** — it
never once complained about something that is in fact fine. And those 1661
complaints are only **thirteen actual slips**, because a few of them are on
lines that were copied a great many times.

**Nine of the thirteen do nothing visible.** They are lines, or parts of lines,
that quietly have no effect. That is the reason for this pass: the engine tells
you nothing about them — of the 1661 complaints, the engine says a word about
**twelve**, and even those twelve go into the script log marked `(Debug)`,
next to the ordinary trace. Nothing here is urgent. Some of it may be worth an
afternoon.

Two general notes before the list. **Files come in copies**: 43 groups of the
installed scripts are byte-for-byte identical (usually `navigation/fscripts/X`
being a copy of `fscripts/X`, but also six trios under `iphases_eclipses`), so
where a defect is in one it is in the others and a fix has to land in each —
the counts below already say how many lines. And **`fscripts/W17.sts` and
`navigation/fscripts/W17.sts` are NOT copies of each other** (18 lines against
173): same name, different scripts, which is worth knowing on its own.

- **SS-32** — **Five deep-sky drawings are dimmer than the file asks, and lose
  their credit.** `internal/deepsky_drawings.sts` has 60 `dso action load`
  lines; 55 of them write the photographer as one word, `credit
  Laurent_Ferrero`. Five do not: lines **36, 40, 45** write the name with a
  SPACE (`credit Nicolas Biver`, `credit Laurent Ferrero`, `credit Jere
  Kahampaa`) and lines **8 and 41** have no `credit` word at all, just the name
  after the filename. Because the engine reads a line as alternating
  name/value, a two-word name pushes everything after it out of step: on all
  five lines the `texture_luminance_adjust 1` at the end **never arrives**, and
  the engine uses 0 instead of 1. The credit is lost or cut in half as well.
  Fix: an underscore instead of the space on the three, and `credit ` in front
  of the name on the other two — exactly what the file's other 55 lines do.
  This is the one on the list with a visible consequence. Status: OPEN.

- **SS-33** — **`fscripts/06old.sts` has three lines that are not what they
  look like.** Line **104** is just `LS`, and line **299** is
  `==> e_sats-tle-new.sts <==` — both look like the leftovers of pasting
  several files together, and both are run as commands and fail. Line **286**
  is subtler and worse: it reads `... lighting false color0.5,0.5,0.5
  tex_map ...` with **no space after `color`**, and from that point on every
  name/value pair on the line is out of step, so the satellite "TDRS 3" is
  loaded with a scrambled set of parameters — no texture, no orbit. Compare it
  with the line above it to see the difference. Question: is `06old.sts` still
  used, or is it superseded by another file? If it is live, line 286 is worth
  fixing. Status: OPEN.

- **SS-34** — **Four spellings the command simply ignores, on 39 lines.** Each
  of these is read by nobody: the command runs, reports success, and the word
  does nothing at all.
  - `deselect ... pointer off` — **27 lines**, `navigation/fscripts/13.sts`.
    `select ... pointer off` IS real, which is surely where the spelling came
    from; `deselect` reads only `constellation`.
  - `audio ... output_rate 44100` — **9 lines**, in `fscripts/K9.sts`,
    `navigation/fscripts/K9.sts`, `internal/white_room.sts`,
    `internal/white_room_old.sts`, `internal/white_room_open_only.sts`. The
    sound plays; the rate request is dropped. There is no way to ask for a
    sample rate from a script.
  - `media ... key_color off` — **2 lines**, `fscripts/W06.sts` and its
    navigation copy. The engine's spelling is **`keycolor`**, one word.
  - `date utc ... duration 0` — **1 line**, `fscripts/08.sts:163`. The date IS
    set; only the word `duration` is dropped. (Same word, same question as
    SS-2 on `set`.)
  Question for all four: did you expect any of them to do something? For
  `key_color` the fix is one character. For the other three there is nothing to
  fix except deleting the words — unless you want the capability, which is a
  different conversation. Status: OPEN.

- **SS-35** — **`set home_planet Earth duration 0` does not set the home
  planet**, and this is the one place where an ignored word costs the whole
  line. `fscripts/S13.sts:2` and its navigation copy. The engine goes through
  the words of a `set` line in ALPHABETICAL order, and it stops at the first
  one it does not know: `duration` comes before `home_planet`, so it stops
  before ever reaching `home_planet`. Measured on a running engine, both ways:
  the same line with a bad word that sorts AFTER the real one works fine.
  The engine's own advice on this line is unhelpfully wrong, too — it says
  *"duration is unknown. Did you mean heading ?"*. Fix: delete ` duration 0`
  and the line does what it says. Status: OPEN.

- **SS-36** — **Four switch names that do not exist**, on 5 lines. `flag`
  refuses each of these outright (it is one of the twelve things the engine
  does mention, in the `(Debug)` log):
  - `flag suntrace off` — `internal/clear_mess.sts:39`,
    `internal/clearVR360.sts:14`. `suntrace` is a COMMAND, not a switch; if
    the intent is to stop the sun trace, the command form is what does it.
  - `flag ground off` — `navigation/fscripts/09.sts:22`. Did you mean
    `landscape`?
  - `flag show_selected_object_info off` — `shows/3d_sky.sts:12`. No switch of
    that name exists under any spelling we can find.
  - `flag lanscape off` — `shows/image_spherical.sts:13`. A missing `d`;
    the engine even suggests `landscape`.
  Both "clear" scripts and the 3d_sky one are presumably meant to turn
  something OFF and are not doing it. Status: OPEN.

- **SS-37** — **Three images that never appear: `nebula action load ...`**
  There is no `nebula` command. `internal/skypole.sts:12` (the pole star
  marker), `navigation/fscripts/16.sts:64 and :65` (a sun and a moon marker).
  The command that takes exactly these words — `ra`, `de`, `magnitude`,
  `angular_size`, `name`, `filename`, `credit`, `texture_luminance_adjust` —
  is **`dso`**. Changing the first word of each line is very likely the whole
  fix; the engine's own suggestion ("did you mean media?") points the wrong
  way. Worth checking whether `nebula` was the name in an older engine.
  Status: OPEN.

- **SS-38** — **`flag stars ofn` — a typo that happens to be harmless.**
  `fscripts/M17.sts:16`, between `flag bright_nebulae off` and `flag planets
  on`. `ofn` is obviously `off`, and the stars do go off — but not because
  the engine understood: **any word it does not recognise means OFF** on a
  `flag` line, silently and reporting success. So this line is right by luck.
  The reason it is worth a line here is the general rule behind it: a typo in
  the VALUE of a `flag` never fails, it just means off. `flag stars onn` would
  turn the stars off too. Status: OPEN (the typo is a one-character fix; the
  general behaviour is a question for Vixy and is recorded).

- **SS-39** — **`halo true` is written twice on the same line, 1595 times.**
  Harmless — both copies say `true` and the engine keeps the last — but it is
  one comet-body template that has been copied a great deal:
  `internal/comet-particles.sts` (1500 lines, `c1` to `c1500`),
  `navigation/fscripts/W17.sts` (94), `internal/comet.sts` (1). Nothing needs
  fixing. Two questions instead: **(a)** what produces
  `internal/comet-particles.sts`? It looks generated — 1500 mechanically
  identical bodies — but there is no generator anywhere in the program's
  source, and no script anywhere on this machine plays it. If a tool of yours
  writes it, that tool has the duplicate in it. **(b)** is that file still
  wanted? Status: OPEN, FYI.

- **SS-40** — **Two keys on EVERY body of the stellar-system files do nothing,
  and 189 lines of your `ssystem.ini` are in the same state.** `tex_halo` and
  `lighting` appear once per body in both `~/.spacecrafter/ssystem.ini` (90 each)
  and the shipped `data/default_ssystem.ini` (77 and 78) — and no part of the
  program reads either one. The only places those words appear in the source at
  all are two blocks that WRITE them onto a star built from the star catalogue;
  nothing ever reads them back. Same for `model3D` (4 lines), `ring_shadow` (2),
  `sidereal_period` (1), and in the shipped file `tex_cloud` and
  `tex_cloud_normal`. Altogether **189 lines of your file (8.2% of its key
  lines) and 164 of the shipped one (9.4%) have no effect**. Nothing is broken
  by them — they cost nothing at run time — so this is not a bug report. Two
  questions: **(a)** were these keys meant to do something that was never
  implemented, or did their feature go away? **(b)** would you rather they were
  removed from future data, or left as documentation of intent? Nothing has been
  changed. Status: OPEN, needs your word. [Measured 2026-09-04, F80; parent
  §5.124.]

- **SS-41** — **Three misspelled keys account for 3365 lines of the shipped
  show package.** On `body action load` lines: **`big_halo` on 2779 lines** (the
  keys the engine reads are `tex_big_halo`, which switches the big halo on by
  being set at all, and `big_halo_size`); **`orbit_visualisation_period` on 496**
  (the engine spells it `orbit_visualiZation_period` — one letter); and
  **`sideral_period` on 90** (missing an `e`; and `sidereal_period`, the spelling
  it is reaching for, is not read either — see SS-40). Every one of those lines
  is silently ignored: an unrecognised key on a `body` line produces no message
  of any kind. What that means in practice is that the halo settings on 2779
  lines and the orbit-drawing period on 496 are not reaching the program at all,
  and whatever they were meant to look like, the program is drawing the default
  instead. Worth knowing which of the three you would want corrected in a future
  data release — nothing has been changed here. Status: OPEN, needs your word.
  [Measured 2026-09-04, F80 over the 408 installed scripts; parent §5.124.]

- **SS-42** — **`[Sedna]` in your `ssystem.ini` loses three of its orbital
  elements, and the reason is a rule that differs between scripts and data
  files.** Lines 2420, 2426 and 2428 write `orbit_MeanLongitude`,
  `orbit_LongOfPericenter` (with no `=` at all) and `orbit_Period`. None of the
  three reaches the program: **in a SCRIPT, capitals do not matter — the command
  reader lowercases every key — but in a stellar-system FILE they do**, and no
  file reader lowercases anything. So `orbit_Eccentricity` works on a `body
  action load` line (3128 shipped lines rely on exactly that) and the same
  spelling is dead in `ssystem.ini`. Sedna therefore gets the default mean
  longitude, no period, and no longitude of pericentre, silently. The missing
  `=` on line 2426 was already known (parent §11.109(i)); the two capital-letter
  ones are new. Question: is the capitals-matter-here-but-not-there rule
  something you want the program to stop enforcing (i.e. should data files
  lowercase keys too), or would you rather the data was corrected? That is a
  design call, not a fix, so nothing has been changed. Status: OPEN, needs your
  word. [Measured 2026-09-04, F80; parent §5.126.]

- **SS-43** — **If you adopt the new composed format for a system, four keys
  stop working, and the file still contains them.** The machine-written
  `.ini.disabled` twin copies every key of your legacy file exactly — including
  `big_halo_size`, `tex_skin`, `halo_alpha_override` and `halo_scale_override`,
  which only the OLD loader reads. Adopt the twin (drop the `.disabled`) and a
  star's big-halo size and a body's skin texture quietly stop being applied,
  while the keys sit there in the file looking correct. Measured on the shipped
  `SolarSystem.ini.disabled`: 4 such lines. This is a heads-up rather than a
  question — it is the program's to fix (parent §5.125) — but if you were
  planning to adopt a composed file, this is what would change. Status: OPEN,
  FYI. [Measured 2026-09-04, F80.]
