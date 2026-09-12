# F77 predictions -- committed BEFORE the launch (INTENT 11.159(c) template)

Task F77, 2026-09-01, code `master-beta @ 8d41fbe3`, harness `5016b09`, host
TravellingFoxDev, `DISPLAY=:0`. Driver: `claude/harness/f77_badscript.py`
(written after this file; this file's md5 is asserted unchanged at delivery).

Every prediction below is a relation between things this run measures, and every
one of them can fail. Where a prediction is DERIVED from source read at
`8d41fbe3`, the read is cited: the launch is what turns it into a measurement,
and a disagreement is the finding, not a bug in the run.

## The four channels, and what each is

| channel | what it is | how it is read here |
|---|---|---|
| LOG | `LOG_FILE::SCRIPT`, i.e. `<farm>/.spacecrafter/log/script-YY.MM.DD.log` | `f27_reply.Session.lognew(mark)` per leg |
| WIRE | the `$DIAGON` dedicated feedback link (F69) | a second client that sends ONLY `$DIAGON`; raw bytes recorded |
| TAIL | the `#!` annotation the engine writes INTO the played file (F63/11.184) | md5 of the played file before/after + its bytes |
| STATE | what the line left behind | recorder (success), record+toggle (flag), `configuration action save` (config write-back), `body action dual_dump` (the dome), the two saved star-line files |
| CONSOLE | `print_log = true` (shipped default, `config.ini:309`) -> `cLog::writeConsole` | the child's stdout and stderr captured to SEPARATE files |

Plus a fifth thing that is not a channel but the claim itself: **SUCCESS**, read
through the engine's own recorder --- `executeCommandStatus` calls
`scriptInterface->recordCommand(commandline)` ONLY on the success branch
`[observed: app_command_interface.cpp:1310-1312]`, so a script recorded while
the bad script plays contains exactly the lines the engine reported success for.

## The script (`f77_bad.sts`, in the temp-HOME only, played by ABSOLUTE path)

```
 1  # F77 - one deliberately bad line per mechanism, neutral neighbours between.
 2  wait duration 0.2
 3  flag atmosphere yes
 4  flag atmosphere toggle
 5  wait duration 0.2
 6  flag atmosphere on
 7  flag atmosphere toggle
 8  wait duration 0.2
 9  flag atmosphere Off
10  flag atmosphere toggle
11  wait duration 0.2
12  set moon_scale big
13  wait duration 0.2
14  set stall_radius_unit = 5.0
15  wait duration 0.2
16  configuration module star_lines action load_star name f77seed star_name 1000 star_pos 1,2,3
17  configuration module star_lines action save name <OUT>/f77_sl_on.txt binary_mode on
18  configuration module star_lines action save name <OUT>/f77_sl_true.bin binary_mode true
19  configuration module star_lines action save name <OUT>/f77_sl_off.txt binary_mode off
20  wait duration 0.2
21  flag nosuchflag on
22  wait duration 0.2
```

Line 21 is the POSITIVE CONTROL on the file channel. Lines 6-7 are the accepted
control for lines 3-4 and 9-10. Lines 17/18/19 are the SS-26 triple.

## P1 -- `flag atmosphere yes` (line 3), S5.116(a)

- **LOG**: exactly ONE line for it, the `Execute_command flag atmosphere yes`
  echo at `L_INFO`. NO `Error executing`, NO `Could not execute`, NO blank
  `(Debug): ` line, NO `is unknown. Did you mean`.
- **WIRE**: 0 new bytes on the `$DIAGON` client. (Also 0 by origin: a FILE line
  never routes, `sendFeedback` returns at `!tcp || channel != TCP`
  `[observed: app_command_interface.cpp:275-289]`.)
- **TAIL**: `f77_bad.sts` byte-identical before and after the whole play.
- **SUCCESS**: `flag atmosphere yes` appears VERBATIM in the recording.
- **STATE**: the flag is **OFF**. Read: line 4's toggle is recorded as
  `flag atmosphere 1` (`commandFlag` rewrites its own commandline with the value
  the flag ENDS at, so `1` means it was 0 before the toggle)
  `[observed: :1343-1356]`.
- *Derivation*: `isTrue("yes")` is size 3, and the switch has cases for 4, 2 and
  1 only `[observed: src/tools/utility.hpp:160-171]` -> false -> `FV_OFF`.

## P2 -- `flag atmosphere on` (line 6), THE ACCEPTED CONTROL

Same four channels silent, same recording, but **STATE = ON**: line 7's toggle
recorded as `flag atmosphere 0`. If P1 and P2 record the SAME digit this leg
cannot discriminate and neither P1 nor P3's state half is evidence.

## P3 -- `flag atmosphere Off` (line 9), S5.116(a), the `& 0x5f` fold

Identical to P1 in every channel and in state: **OFF**, line 10's toggle
recorded as `flag atmosphere 1`. `isTrue("Off")` is size 3 -> false; the fold
never runs. (`isFalse` DOES have a size-3 case for `OFF`
`[observed: utility.hpp:172-181]` and has nothing to do with this path.)
So `Off` and `off` are indistinguishable HERE by luck, exactly as F76 found for
`ofn`; the prediction is that the coincidence holds and nothing is said.

## P4 -- `set moon_scale big` (line 12), S5.116(b)

- **LOG/WIRE/TAIL**: silent, as P1. **SUCCESS**: recorded verbatim.
- **STATE, channel 1 (the value)**: `configuration action save` after the play
  writes `[viewing] moon_scale` = **0** into the farm's `config.ini`
  `[observed: core.cpp:1704]`, against the shipped `5`
  `[observed: ~/.spacecrafter/config.ini:240]`. Control: a later
  `set moon_scale 5` + save restores `5`.
- **STATE, channel 2 (the dome) -- THE UNKNOWN THIS LAUNCH EXISTS TO SEE**:
  `flag_moon_scaled = true` in the shipped config `[observed: config.ini:221]`,
  so `setMoonScale(0)` reaches `Moon::setSphereScale(0)` and
  `radius = initialRadius * 0` `[observed: body.cpp:484-489]`. Prediction: in
  `body action dual_dump` the Moon's OLD-path `screenSz` is **0 (or within 1e-6
  of it)** and its `visible` is **false**; with `moon_scale 5` restored,
  `screenSz` is **> 0**. REACH: this is a readout of the draw-path's own
  computed screen size, not a photometric measurement of pixels; it says the
  body is drawn at zero angular size, not what a viewer sees.
  I do NOT predict the new-path (`scaling`/`scalingTarget`) fields: whether the
  runtime `set moon_scale` reaches ModularBody is unread at dispatch time --
  both values are RECORDED, and whichever way they come out is a finding about
  where the display-scale authority sits (F41's subject), stated not scored.

## P5 -- `set stall_radius_unit = 5.0` (line 14), S5.118 o S5.116

- **LOG/WIRE/TAIL**: silent. **SUCCESS**: recorded verbatim.
- *Parse, derived*: `parseCommand` reads `key value` PAIRS
  `[observed: :338-378]`, so the line yields exactly `{stall_radius_unit: "="}`
  and the trailing `5.0` is a dangling key that the `>>` pair loop never
  inserts -- F76 leg B's shape. Then `evalDouble("=")` -> `strToDouble` ->
  `std::stod` throws -> **0** `[observed: app_command_eval.cpp:116-128,
  utility.cpp:399-406]`, and `setRotationMultiplierCondition(0)` drops it
  (`if (v>1.0)`) and returns `true` into a `void` wrapper
  `[observed: anchor_manager.hpp:201-206, ssystem_factory.hpp:964-966]`.
- **STATE**: **UNREADABLE, and that is the prediction.** A census over the tree
  finds `rotationMultiplierCondition` read in exactly two places, both inside
  `AnchorManager::updateAnchor`'s follow-rotation test
  `[observed: anchor_manager.cpp:323, :329]`; there is no getter, the value is
  in no dump, and `saveCurrentConfig` does NOT write it back (the config half is
  read-only, `ssystem_factory.hpp:977`). The run VERIFIES the negative the only
  way it can be verified from outside: `configuration action save` is issued and
  the farm config is searched for a `stall_radius_unit` line that differs from
  the shipped `5.0`; the prediction is that the key is written unchanged or not
  at all, i.e. no shipped channel carries the dropped value. A key that DOES
  move would refute the census and would be the finding.

## P6 -- SS-26, one word and two meanings, on the same launch (lines 17/18/19)

`Utility::strToBool` lowercases and accepts only `true` and `1`
`[observed: utility.cpp:430-436]`; the two-argument overload only adds an
empty-string default `[:439-443]`. `binary_mode` is its shipped call site
(`W_BINARY` = `"binary_mode"`, `[observed: base_command_interface.hpp:147]`,
used at `:2425` and `:2463`).

- `binary_mode on` (17) -> **TEXT** file: `f77_sl_on.txt` begins with
  `# Created by SC StarLines::saveHipCatalogue` `[observed: starLines.cpp:139]`.
- `binary_mode true` (18) -> **BINARY** file: `f77_sl_true.bin` has NO `#`
  header and is exactly 16 bytes for one seeded entry (int + 3 floats)
  `[observed: starLines.cpp:155-172]`.
- `binary_mode off` (19) -> **TEXT**, byte-identical to (17).
- All three lines: LOG/WIRE/TAIL silent, all three recorded (SUCCESS).
- Composed with P2 on the same launch: `on` = YES for `flag` (isTrue) and
  `on` = NO for `binary_mode` (strToBool). If (17) and (18) come out
  byte-identical, SS-26's live confirmation FAILS and that is the finding.
- **Substitution, stated**: the dispatch names `media ... pause on` as the
  strToBool site. `media`'s `paused` is handed to the video player and has no
  readout on any shipped channel, so that site can only be read at source;
  `binary_mode` is the same function, the same word, and leaves a FILE. The
  media site is therefore reported as source-read, not measured.

## P7 -- `flag nosuchflag on` (line 21), THE POSITIVE CONTROL, FILE origin

- **LOG**, three lines and in this order:
  1. a BLANK `(Debug): ` line -- `setFlag(name,...)` writes `debug_message`
     while it is still empty, the assignment above it being commented out
     `[observed: :450-457]`, and this emitter does NOT go through `originTag()`,
     so the line carries no file:line either;
  2. the suggestion line `nosuchflag is unknown. Did you mean <x> ?` at
     `L_DEBUG` `[observed: app_command_init.cpp:404-405]`, also untagged;
  3. the funnel's F73-shape line, at `L_DEBUG`:
     `Error executing <abs>/f77_bad.sts:21: flag nosuchflag on #! Unrecognized or malformed flag argument`
     -- `where()` for a FILE origin is the FULL PATH plus `:line`
     `[observed: script_origin.hpp:120-125]`, and `withAnnotation` joins with
     ` #! ` `[observed: script_annotator.cpp:62-65]`.
- **WIRE**: **0 bytes** -- this is a FILE origin and `sendFeedback` routes only
  TCP. This is NOT a failure of the control; it is S11.187(d)'s asymmetry,
  which is precisely what this task owes S5.117. The wire's positive control is
  P8.
- **TAIL**: `f77_bad.sts` still byte-identical. The `#!` in the log line is the
  AS-IF line: `executeCommandStatus` never calls `scriptInterface->annotate`,
  only `reportScriptError` does `[observed: :215-240]`. A funnel refusal is
  shown as if annotated and the file is never touched -- the sharpest of these
  predictions, and the one whose refutation would matter most.
- **SUCCESS**: `flag nosuchflag on` is **ABSENT** from the recording.

## P8 -- the same control over TCP, the WIRE's positive control

`flag nosuchflag on` sent by the driving client:
- **WIRE**: > 0 bytes, exactly one record beginning
  `$DIAG|tcp#<id>|Unrecognized or malformed flag argument|flag nosuchflag on`
  `[observed: :1337, io.cpp:392]`.
- **LOG**: `Error executing tcp#<id>: flag nosuchflag on #! Unrecognized or
  malformed flag argument`.
- If the wire is empty here, NOTHING in this run is evidence about the wire.

## P9 -- the `#!` writer is armed (a SEPARATE file, the ruled class)

`f77_end_without_if.sts` = one `struct if end` closing nothing, played by
absolute path. Predictions: an `L_ERROR` log line containing
`closes nothing`, AND a ` #! ` tail written on that line OF THAT FILE (the file
changes; `f77_bad.sts` does not). Without this leg, "no tail on the bad script"
is consistent with a dead annotator.

## P10 -- S5.117's CONSOLE half, both ways

`print_log = true` is the shipped default `[observed: config.ini:309,
checkConfig.cpp:68]` and `main.cpp:262` hands it to `cLog::setDebug`. Reading
`cLog::write` `[observed: log.cpp:122-124]` and `writeConsole`
`[observed: log.cpp:165-211]`: **every** log line is echoed to the console when
`isDebug`, and the SEVERITY chooses the stream --- `L_ERROR`/`L_WARNING` to
`std::cerr`, everything else to `std::cout`.

- **PREDICTION**: the refusal lines of P7/P8 appear on **stdout** and NOT on
  **stderr**; the `L_ERROR` block-structure line of P9 appears on **stderr**.
  Both halves are asserted, so a run where the child's console is empty (or
  where the two streams were merged) cannot pass.
- **CONSEQUENCE if it holds, and it is a CORRECTION to S5.117**: the row's
  console clause reads *"promoting these 157 records also puts them on the
  console"*. Under the shipped `print_log = true` they are ALREADY on the
  console; a promotion to `L_ERROR` MOVES them from stdout to stderr (and
  colours them red). The price is a channel move, not an appearance.
- **The number**, cited from F76 and not re-derived: of the 1661 findings over
  the 408 shipped scripts, the SIGNAL column of
  `harness/artifacts/f76/dispositions.tsv.gz` reads **12 LOG-DEBUG**, **2
  LOG-ERROR+ANNOTATE**, **1647 SILENT**. So a promotion moves **12** lines per
  full corpus play from stdout to stderr; the 2 are already `L_ERROR`.

## P11 -- the run's own integrity

- `~/.spacecrafter/config.ini` and `ssystem.ini` md5 in == out
  (`03fbee59` / `545a51ef`).
- All 408 shipped scripts byte-identical before and after (F76's layer 3).
- No concurrent `spacecrafter` in `/proc/*/comm` at launch.
- `GetActive` recorded (expected `false`).
- The canary `--no-scene` exit RECORDED (expected **3**, fail-by-construction
  against the desktop bank on this host); never re-banked, never gated on.
- `timerate rate 0` freezes the clock: drift over ~3 s exactly 0.0 frozen,
  non-zero running (F76's hazard, measured both ways).

## What is NOT predicted, and why

- The exact text of the `Did you mean` suggestion for `nosuchflag` (a
  Levenshtein nearest over 97 names) -- recorded, not scored.
- The new-path Moon fields (P4 channel 2), as stated there.
- Anything about frame timing. No claim in this run is a timing claim.
