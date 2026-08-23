# scedit per-handler extraction sweep — dispatch spec (INTENT §5 item 1)

Supervisor-maintained. Mirror ledger: `claude/util/scedit/INTENT.md` (authority
for scedit); parent ledger `claude/INTENT.md` wins on spacecrafter facts.
Code at dispatch: `master-beta @ b12c8cdd`. Executor: opus-xhigh, one unit per
dispatch, DoD-gated by the supervisor before the next unit launches.

## Objective (per unit)

For every command in the unit: extract from `src/interfaceModule/
app_command_interface.cpp` the argument keys read, their value domains,
defaults / absent-key behavior, REQUIRED/optional structure, and one
zero-knowledge doc line per command AND per key (C6). Output = one JSON
fragment file; nothing else is edited.

## Fragment schema

Write to `util/scedit/grammar/args/unit-<N>.json` (code repo):

```json
{
  "_meta": {
    "unit": 1,
    "code": "master-beta @ b12c8cdd",
    "handler_range": "app_command_interface.cpp:1180-2109",
    "args_bracket_expected": 68,
    "args_bracket_accounted": 68,
    "other_args_uses": [ {"line": 0, "form": "args.begin()", "role": "..."} ]
  },
  "commands": {
    "<registered name>": {
      "handler": "commandX",
      "lines": "1180-1200",
      "aliases": [],
      "doc": "one plain-language line, zero-knowledge bar — or null + flagged",
      "args": {
        "<key>": {
          "doc": "zero-knowledge one-liner — or null + flagged",
          "value": "domain: enumerated | number(evalDouble) | int(evalInt) | string(evalString) | boolean(flag grammar) | ...",
          "values": ["exact", "spellings", "when", "enumerated"],
          "default": "value, or the ABSENT-key behavior stated explicitly",
          "required": true,
          "source": "app_command_interface.cpp:NNNN",
          "notes": "sharp edges, aliases, deprecation"
        }
      },
      "exclusive_groups": [["keyA", "keyB"]],
      "notes": [],
      "flagged": ["C2 gaps the code cannot answer — for Vixy, never invented"]
    }
  }
}
```

Schema deltas are allowed where the code demands them (record the reason in
`notes`); the six per-key facts (doc, value, values-if-enumerated, default,
required, source) are the invariant core.

## DoD (all items, verifiable)

1. Fragment parses as JSON, conforms to the schema above.
2. **Count gate**: every `args[` occurrence in the unit's line range maps to
   exactly one key entry or one `other_args_uses` row;
   `args_bracket_accounted == args_bracket_expected`. Mismatch = not done.
3. Every command of the unit present — including no-arg commands (empty
   `args` + command doc still required). Registration aliases recorded
   (map: `app_command_init.cpp:37-109`).
4. **C2**: every doc/domain/default carries a `source` anchor (file:line).
   Nothing from recall or invention. Untraceable content = `null` + entry in
   `flagged`. `UNEXTRACTED`/`null` are the honest states.
5. **C6**: doc lines readable by someone with zero script knowledge
   (`_meta.documentation_requirement` in `sc-grammar.json`).
6. Scope boundaries:
   - `app_command_eval.cpp` NOT extracted (INTENT §5 item 5): record
     conversions as `evalDouble`/`evalInt`/`evalString` ("$-variable
     substitution possible"), semantics opaque here.
   - The 97-flag doc pass is NOT this task: `flag` records its key grammar
     shape only, cross-ref `families.flags`.
   - Reading anything is allowed; ACCOUNTING covers only the unit's range;
     WRITES touch only the fragment file. No commits — supervisor gates,
     merges, commits.
7. Handler behavior that contradicts or refines `parse_model` in
   `sc-grammar.json` (C1-relevant: e.g. a handler re-reading the raw line,
   order-dependence defeated by the alphabetical map) → recorded in `notes`
   AND in the final report.
8. Deprecated/obsolete token handling in-range → recorded, cross-ref
   `families.obsolete_tokens`.
9. Final text = report: per-command key counts table, flagged list,
   C1-relevant discoveries, `other_args_uses` summary. Raw data, no prose
   padding.

## Units

End line of a handler = next handler's start − 1. `args[` counts measured at
b12c8cdd; sum 68+101+93+77 = 339 = whole file (0 outside the four ranges).

**Unit 1 — :1180–2109, args[ = 68, 16 commands**
flag(commandFlag :1180) · get(commandGet :1201) · session(:1309) ·
search(:1328) · planet_scale(:1349) · wait(commandWait :1361, &wait param) ·
personal(:1407) · dso(:1436) · dso3d(:1513) · dso2d(:1532) ·
personeq(:1556) · body_trace(:1585) · suntrace(:1629) · color(:1862) ·
illuminate(:1891) · print(:1969–2109).
Shared context: executeCommandStatus :1163–1179 (error-reporting shape —
readable, out of accounting range).

**Unit 2 — :2110–3095, args[ = 101, 15 commands**
set(commandSet :2110 + evalCommandSet :2143 — same command, both accounted) ·
shutdown(:2234) · configuration(:2244) · constellation(:2353) ·
external_viewer(:2398) · clear(:2468) · heading(:2536) · meteors(:2568) ·
landscape(:2590) · screen_fader(:2625) · text(:2641) · sky_culture(:2786) ·
script(commandScript :2797, &wait) · audio(:2872) · image(:2933–3095).

**Unit 3 — :3096–3944, args[ = 93, 15 commands**
select(:3096) · deselect(:3149) · comment(:3160) · uncomment(:3167) ·
look_at(commandLook :3174) · star_lines(:3206) · galaxy_stars(:3224) ·
position(:3253) · zoom(commandZoom :3268, &wait) · timerate(:3303) ·
moveto(:3396) · mode(commandModeJump :3465) · media(:3490) ·
domemasters(:3780) · date(:3799–3944).

**Unit 4 — :3945–4747, args[ = 77 (+30 other args uses), 15 commands**
body(:3945) · font(:4149) · camera(commandCamera :4186, &wait; alias
**flyto**, app_command_init.cpp:45) · define(:4482; helper evalInt :4474 in
range) · add(:4495) · sub(:4508) · multiply(:4522) · divide(:4535) ·
modulo(:4548) · tangent(:4561) · trunc(:4574) · sinus(:4587) ·
struct(commandStruct :4600) · random(:4709) · transition(:4727–4747).

Coverage check: 16+15+15+15 = 61 registered names + flyto alias = the 62
entries in `families.commands`. ✓

## Merge protocol (supervisor) — updated 2026-08-04 post-unit-1

Per unit: gate DoD → independent re-derivation of the count gate + content
spot-checks on flagged claims → commit the FRAGMENT → journal append.
The `sc-grammar.json` merge happens ONCE, after the tokenizer task lands:
the tokenizer's corpus runs read that file at runtime, and holding it
stable during them is the isolation the dispatch protocol exists for.
(Deviation from the original per-unit-merge line, same gating intent.)
Whole-file census reconciliation (339 at HEAD vs census's dated 366) is a
merge-time item, not per-unit.

## Protocol update [vixy 2026-08-04]

The recorded sequential protocol's reason (previously unrecorded — Vixy:
"should have lived where the recorded protocol lives"): sequentiality lets
the executor TEST its work without interference from other instances,
catching errors when the most information is available. Consequence:
**parallel dispatch is allowed when testability is side-effect-free** and
write surfaces are disjoint. The sweep units + tokenizer task qualify by
construction: read-only engine source, disjoint writes (per-unit fragments;
tokenizer confined to util/scedit/src+tests), self-tests are greps/JSON
validation/own-build. Units 2–4 + tokenizer therefore run concurrently;
gates remain per-unit at merge.

## Unit-1 lessons — binding for units 2–4

- Helper functions inside a unit's range are IN-accounting: attribute their
  `args[` uses to the calling command or `other_args_uses` (precedent:
  `applyColor` sits between suntrace and color).
- Machine-verify every file:line anchor before declaring done (unit 1: 22
  anchors wrong on first write, caught only by programmatic self-check).
- Forwarded-map pattern: when a handler hands the whole `args` map
  downstream, record downstream keys with `downstream: true` + their own
  file:line anchors in the downstream module (precedent: dso3d →
  dsoNavigator.cpp:264-321).
- The unit table's handler line ranges are navigation aids; the code wins
  (suntrace actually ends :1655, the table said :1861).
- `args[KEY]` is `std::map::operator[]` — absent-key reads INSERT empty
  entries; keys-of-the-line must come from the tokenizer, never from
  handler behavior (unit-1 C1 refinement, material for forwarded maps).
- Absent-key defaults resolve in app_command_interface.cpp itself:
  `evalDouble("")=0.0`, `evalInt("")=0`, `evalString("")=""` (:4458-4479);
  only non-empty eval stays opaque (narrows the item-5 boundary).

## Gate record (supervisor)

- **Unit 1 GREEN** (2026-08-04, code `15927a11`): 68/68 lines, 84/84
  tokens; suntrace/dso3d/color claims verified at source → §5.91-76.
- **Unit 4 GREEN** (code `8e62cfd9`): 77/77, 83/83, 30/30 other-uses;
  struct dead-registered-path, follow_rotation exact-"true", rotate-target
  commented branch verified; camera double-prefix = §5.41 rediscovery.
- **Unit 3 GREEN** (code `091713cd`): 93/93, 97/97, zero non-bracket
  (matches supervisor pre-dispatch measure); media self-re-entry :3511,
  subtitle wrong-variable (= §5.36, drift :3329→:3764), date W_PRESET
  double-test :3898, date-sun no-effect statement :3937, media-speed
  fromString :3686 + timerate stod :3317 all verified verbatim.
- **Unit 2 GREEN** (code `091713cd`): 101/101, 105/105, 6/6; set loop,
  external_viewer silent success, args[W_PATH] write, textClear-before-
  error verified; configuration fall-through queued for FULL verification
  before registration (see batch).

## Merge-time upstream batch (queued; execute after tokenizer lands)

New §5 rows (verify-then-write, unit-1 discipline):
1. configuration star_navigator fall-through (:2330-2345 — save then
   overwrite settings file; VERIFY full extent first).
2. date pair: `W_PRESET || W_PRESET` double-test :3898 (Preset-cased
   config starts preset but loads system date) + `date sun <unknown>`
   silent success :3937 (no-effect `_()` statement).
3. Recorder-semantics cluster (R2 class, §5.70 precedent): `clear`
   records 35 nested lines + `zoom auto initial` twice; `set a 1 b 2`
   records twice (per-pair executeCommandStatus); media audio-only
   re-entry records the rebuilt line, spaces-in-name destroyed;
   flag/timerate rewrites (units 1/3) — probably ONE row for the class.
Annotations to existing rows:
4. §5.92: unguarded-conversion inventory now complete-for-the-sweep —
   dso3d stoi :1525; timerate stod :3317; media speed/speed_increment
   FixedPointI16_2::fromString :3686/:3754/:3759 (throws, sole catch in
   scope is app_command_eval.cpp:151, other path). Discharges the row's
   "owed".
5. §5.93: second reach — `body … color r/g/b` through the same by-value
   setClassicColor (unit-4 note).
6. §5.36: convergence note + line drift :3329→:3764 (unit-3 rediscovery).
7. §5.41: convergence note + line drift (unit-4 rediscovery,
   coreLink.cpp:65/:70 at HEAD).
Silent-success family (lint-seed grade, record in grammar not §5 unless
Vixy upgrades): external_viewer unknown extension; mode without jump;
domemasters diagnostic overwritten; text empty-string clears screen
first; set stall_radius_unit ≤1 ignored; landscape author path
overwritten :2597; camera rotate target required-but-unused.
Grammar merge riders: per-BRANCH value domains (unit 3); ≥3 boolean
grammars — flag-grammar lint must be per-key (units 2/3/4);
inner_script_channel comment model (2026-08-04c); set multi-pair vs
flag single-pair; W_TRUE live / W_RECORD live; unit-2's four lint-seed
candidates + unit-3 anchor convention (repo-relative paths, no bare
basenames) adopted for the merged file.

Unit-2 specific: `evalCommandSet` (:2143-2233) IS the set-family
semantics — the 43 `families.set_names` act as keys of `set`; record
per-name value domains/defaults/docs from the in-range code.
Unit-4 specific: reconcile `commandStruct` with
`parse_model.pre_table_commands` (struct is both pre-table intercepted
and registered).
