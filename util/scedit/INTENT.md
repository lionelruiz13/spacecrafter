# scedit — intent, constraints, decisions, journal

Mirror ledger for `util/scedit` (code repo), per Vixy's home decision
[vixy 2026-08-03]: *"In mirror, an util/scedit directory in the
spacecrafter/claude folder, for the local INTENT.md, and possibly others."*
Maintenance conventions inherit the parent ledger (`claude/INTENT.md`
header): supersession-with-record, provenance tags (`[stated:]`
`[observed:]` `[measured]` `[derived]` `[vixy:]`), append-only journal.
On divergence about spacecrafter facts, the parent ledger and source win;
this file owns only scedit.

## 1. Mandate

Recorded origin — `FEATURE_REQUESTS.md` [2026-07-29], Vixy's D31 side-note
verbatim [vixy 2026-07-26]: *"a side project, writing a tui with mouse
support - a script editor with autocomplete, direct tcp mode, static
analysis to report errors early and showing documentation of the currently
edited call + attribute key documentation corresppnding to the key/value
the cursor is on with default value shown greyed out and candidate for
autocomplete when the value field is empty"*.

Scope additions [vixy 2026-08-03, session]:
- **stellar-system files** join scripts as first-class edited artifacts
  ("The TUI for editing scripts... And also stellar system files");
- transport confirmed TCP ("direct (TCP ?) mode" — resolved: the shipped
  channel, port `io:tcp_port_in` = 7805, line protocol; HTTP `GET
  ?command=` exists on the same socket as fallback);
- **documentation bar** [vixy, mid-session, verbatim]: *"each command AND
  each key should have his own line of documentation, shown interactively
  depending on the cursor position. Someone without any knowledge of
  script should be able to understand and modify any script through this
  editor."*
- **diagnostic ids** [vixy, mid-session]: GNU-style kebab-case
  (`unknown-command`, `unknown-parameter`, `deprecated`, ...).

The three consequences the FEATURE_REQUESTS entry pre-recorded stand as
design constraints: (1) command grammar + data-key vocabulary become
MACHINE-consumed; (2) the TCP channel becomes an editor-facing API (its
§5.47 reply-routing enabler landed 2026-08-02, code `d13681eb`, measured
0.002 s); (3) defaults want to be declared data, not code constants.

## 2. Constraints

- **C1 — engine fidelity.** scedit's tokenizer must reproduce
  `AppCommandInterface::parseCommand` exactly, sharp edges included
  (dangling-key drop, duplicate-key-last-wins, `"`-only quoting, the
  space-after-quote normalization, key lowercasing). Divergence between
  scedit's reading of a line and the engine's is a scedit defect class of
  its own — the editor must never claim a line means something the engine
  will read differently.
- **C2 — identified knowledge only.** No grammar entry, doc line or
  default without a source anchor (handler code, ledger, or Vixy). A doc
  gap the code cannot answer at the zero-knowledge bar is FLAGGED to Vixy,
  never invented. `UNEXTRACTED`/`null` are the honest states.
- **C3 — corpus gate (zero false positives).** Before any lint rule
  ships in `--check`: every shipped `.sts` script and the field
  `~/.spacecrafter/ssystem.ini` must pass with 0 false positives (true
  findings in shipped content are recorded upstream instead — they are
  engine/data findings, not lint noise).
- **C4 — imported from the parent ledger** (references, not copies):
  legacy data files are ISO-8859 (CLAUDE.md standing rule); writes into
  LEGACY files must never introduce new-format constructs — comments/new
  keys there break downgrade (§2.0 D13) → lint hard in legacy regime;
  composed/new-format files take the full B24 grammar (`type=`,
  `relation=`, comments legal); `englishName` is THE body identity,
  globally unique, duplicate silently drops the second body (D34,
  §11.109(c)) → cross-file lint; fresh-launch/md5 disciplines apply to any
  future live-measurement harness legs.
- **C5 — naming.** The tool is **scedit** everywhere; "TUI" unqualified
  means spacecrafter's in-app dome menu (channel 8, capability-surface.md)
  — the collision is why the name exists.
- **C6 — docs are product surface** [vixy 2026-08-03]. Per-command and
  per-key one-liners at the zero-knowledge bar, cursor-driven display,
  defaults greyed + completion-offered. Extraction DoD includes them.

## 3. Decisions

- **D1 — substrate: FTXUI** [vixy 2026-08-03]. C++, vendorable, mouse +
  layout built in. Not needed until the UI slice; the core stays headless.
- **D2 — home: `util/scedit` in the code repo, mirror ledger here**
  [vixy 2026-08-03]. util/ is where spacecrafter-related tools live
  (src_converter2, StarCatalog precedents). Kept in-tree with
  sts-extension so the one grammar contract serves both consumers
  [stated; interpretation of "it include it with extension" — flagged,
  unchallenged].
- **D3 — first slice: grammar table + analysis** [vixy 2026-08-03],
  overriding the TCP-first rec. Headless core first; `--check` before any
  networking or UI.
- **D4 — contract format: JSON; nlohmann/json v3.11.3 vendored**
  [derived, veto open]. Arguments: TS consumer (sts-extension) reads it
  natively; engine-side future EMITTER needs no parser (string building);
  single-header vendoring is house style (`src/stb_image.h` precedent).
  Vendored sha256 prefix `9bea4c8066ef4a1c`, 919 975 bytes.
- **D5 — authority chain** [derived]. Until the engine emits the file,
  SOURCE WINS: `app_command_init.cpp` (registration),
  `base_command_interface.hpp` (spellings), `app_command_interface.cpp`
  (parse + handlers). Target state: engine-generated —
  `AppCommandInit` already keeps `commandList/flagList/colorList/setList`
  copies "to futur exploitation", and B31's
  `SessionFile::CommandSurface::forEach*` hooks enumerate flags/values/
  colors at runtime; an emitter has its seams waiting. The emitter also
  retires sts-extension's grammar-blindness (it carries NO token list —
  purely positional highlighting [observed extension.ts]).
- **D6 — diagnostic id style: GNU kebab-case** [vixy 2026-08-03];
  `--check` output gcc-shaped: `file:line: severity: message [-Wid]`.
- **D7 — doc model** [derived from C6]: schema v1 ships `doc` slots on
  command entries (filled only where traced: flag/set/color/session/
  comment/uncomment today); schema v2 converts every family names-array
  to `[{name, doc, default?}]` when the doc pass fills content.

## 4. Verification bar

- **Seed gate** (live, green 2026-08-03): `scedit` validates the contract
  — family counts re-derived from data vs `_meta.expected_counts`,
  in-family uniqueness, subfamily links, lint-id uniqueness. Counts at
  HEAD: commands 60 / flags 97 / set 43 / colors 46 / obsolete 7 /
  reserved vars 24 / font targets 10. Census (2026-07-25) cross-check:
  one delta, `session` (+1 command, B31) — explained, recorded in
  `_meta.count_deltas_vs_census_2026_07_25`.
- **Tokenizer gate** (pending): derivation-diff against
  `parseCommand` + differential corpus runs; the corpus gate C3 arms when
  `--check` exists.
- Future live legs (TCP) inherit the parent harness disciplines
  (fresh-launch, md5 in==out); no build exists on this laptop yet —
  rebuild is a prerequisite recorded, not done.

## 5. Open work (ordered)

1. **Per-handler extraction sweep** — `app_command_interface.cpp` (4747
   lines): per command, the arg keys read, value domains, defaults,
   REQUIRED/optional structure, and the C6 doc line for each; reconcile
   the `args[KEY]` occurrence count at HEAD against the census's dated
   366; assign the `argument_token_vocabulary` roles (keys vs enumerated
   values); confirm/retire the census's never-referenced `W_*` set.
   Dispatchable in per-handler units with count gates.
2. **Tokenizer + `--check`** (C1-faithful; GNU-style output per D6);
   then arm C3's corpus gate harness.
3. ~~**`script_mgr.cpp:94`** second `'#'` site~~ **RESOLVED 2026-08-04
   (journal 2026-08-04c): `ScriptMgr::addScriptFirst` — a SECOND,
   divergent line-classification model (whitespace-trim first, so an
   indented `#` IS a comment; command indentation stripped), fed today
   by exactly one engine-internal producer (`camera action lift_off`,
   app_command_interface.cpp:4404-4408, synthesized comment-free text)
   ⇒ not user-reachable, latent divergence recorded. parse_model update
   queued for merge time (grammar frozen while the tokenizer runs).**
4. **Stellar-system-file grammar** — second contract file: legacy
   `ssystem.ini` key set (loader grammar, base-D census residual) +
   composed/new-format B24 grammar (`type=`/`relation=`/`compose=`),
   two-regime lint per C4, ISO-8859 handling at the file boundary.
5. **`app_command_eval.cpp`** (376 lines) — `$`-variable substitution
   semantics for the reserved_variables family (currently UNEXTRACTED).
6. **TCP client mode** — line protocol + `$LOGON` feed pane;
   spacecrafter rebuild on this laptop is its prerequisite.
7. **FTXUI shell** — editor + cursor-driven doc panel (C6) + completion
   (defaults greyed, D31 spec).
8. **Engine emitter** — the grammar file becomes a build/runtime
   artifact (D5 seams); propose upstream once the contract shape has
   survived slices 1–4.

## 6. Journal (append-only)

- **[2026-08-04d] All four units + tokenizer gated GREEN; upstream batch
  executed; merge dispatched.** Units 2/3/4 landed and were independently
  re-gated (counts re-derived, sharp claims verified at source — full
  gate record in SWEEP_DISPATCH): 339/339 args[ occurrences accounted
  across the four fragments (68+101+93+77), code `8e62cfd9`+`091713cd`.
  §5 items 1 AND 2 of this ledger are thereby DONE pending merge.
  Tokenizer task (code `da513e3a`): C1 is MEASURED, not asserted — a
  verbatim copy of parseCommand vs scedit::tokenizeLine over 53 058
  enumerated+corpus lines, 0 mismatches; 12/12 lint seeds armed;
  unknown-parameter arms itself from args data presence (merge lights it
  with no code change); first C3 corpus run: 12 findings, all TRUE, 0
  false positives; exit codes 0/1/2 verified by supervisor on an
  independent clean build (5/5 ctest). Corpus found shipped-content
  defects → §5.80 upstream (0xA0 eats `albedo 1`; the dead
  `set home_planet … duration 5` line = §5.79(a) materialized).
  Upstream batch executed (harness `3b778fc`): NEW §5.77 (configuration
  fall-through — verified fully: star-catalogue save also runs §5.42's
  config writer; unknown module re-runs App::init before reporting),
  §5.78 (date W_PRESET||W_PRESET + no-effect `_()` statement), §5.79
  (recorder-rewrite class, 4 shapes), §5.80 (superscript.sts);
  annotations: §5.75 conversion inventory discharged, §5.76 body-color
  second reach, §5.36+§5.41 blind rediscoveries (convergence + line
  drift recorded so nobody re-registers them). Merge task dispatched
  (sequential — it writes the file everything reads): fragments →
  schema v2 partial, args_complete:false on body/camera,
  parse_model riders, invisible-separator seed mint, did-you-mean
  display cap (scedit surface, oracle untouched), census 339-vs-366
  attribution, full re-gate incl. arg-armed corpus at C3. Supervisor
  decisions taken, veto open: display cap is scedit-owned UX (C1
  governs parse, not lint message); struct/ifSwap skip-region lint
  semantics left to the merge with C3 as the constraint. Next after
  merge gate: FTXUI shell dispatch (D3 satisfied: --check exists,
  corpus-validated).
- **[2026-08-04c] §5 item 3 resolved; three defects registered upstream;
  ledger-maintenance incident.** The second `'#'` site (script_mgr.cpp:94,
  in `addScriptFirst`) is a divergent line-classifier — trims whitespace
  BEFORE the `#` test (indented `#` = comment, opposite of script.cpp:114)
  and strips command indentation — whose only caller is `camera action
  lift_off` (:4404-4408) injecting engine-synthesized text; not
  user-reachable today, latent if ever fed user text. parse_model gains
  an `inner_script_channel` clause at merge (grammar frozen till the
  tokenizer lands). Unit-1's three defect-grade findings registered in
  the parent ledger as **§5.74** (suntrace sun-as-key + error-less
  branches), **§5.75** (dso3d raw stoi, no catch on the chain — verified
  by grep, 'try' hits were directory_iterator substrings; z_reflection
  inert rider), **§5.76** (color by-value debug_message — full chain
  re-verified: the two range errors are the only messages, isOkay stays
  false, caller returns success on empty message); softer findings
  (get-no-TCP silent success, wait-loading fallthrough, dso2d naming
  trap, missing units) stay in the fragment's flagged/notes — lint-seed
  grade, not engine-defect grade. Incident, named per house honesty: a
  python slice reordering §5 rows TRUNCATED the parent ledger (515
  lines) and the post-edit check verified row order, not file integrity
  — committed truncated (d4820ea), caught by the diffstat, restored
  from HEAD~1 with an only-insertion diff gate (3f7e59b). Root: a
  relocation executed as a hand-rolled rebuild — the pure-move rule
  exists for exactly this; gate for any future ledger restructuring =
  diff-against-baseline, not spot-checks of the intended change.
- **[2026-08-04b] Unit 1 gated GREEN; protocol answers; full parallel
  dispatch.** Unit-1 fragment (grammar/args/unit-1.json, 84 317 B)
  independently re-gated by supervisor: count gate re-derived (68 lines,
  sets equal, 84/84 token multiplicity), schema fields complete, nulls =
  exactly the 2 flagged docs; content spot-checks on the sharpest claims
  all CONFIRMED in source — (1) suntrace pen branch reads `args[W_SUN]`
  (the *value of a key spelled "sun"*) where its other branches pass the
  literal, so `suntrace pen on` does not switch the trace to the Sun;
  (2) dso3d override path calls `std::stoi(args["depth"])` raw, no catch
  in the chain — abort on missing/non-numeric; (3) AppCommandColor::
  setClassicColor takes `debug_message` BY VALUE (hpp:81, cpp:58) vs the
  ctor's by-reference — r/g/b color errors vanish and the line records as
  success. → upstream registration owed to parent ledger (engine defects,
  not lint noise, per C3). Unit-1 C1 refinements adopted into
  SWEEP_DISPATCH "lessons" (binding for units 2–4): operator[] inserts on
  absent-key reads; absent-key defaults resolve in the interface file
  (evalDouble("")=0.0 etc. :4458-4479 — narrows §5 item 5's boundary);
  in-range helpers accounting; anchor machine-verification.
  Vixy [stated, this session]: (a) sequentiality's reason — previously
  unrecorded, "should have lived where the recorded protocol lives" —
  is executor testability without cross-instance interference; parallel
  dispatch ALLOWED when tests are side-effect-free; supervisor assessed
  the sweep units + tokenizer as qualifying by construction (read-only
  source, disjoint writes, no shared build) and dispatched units 2–4 +
  tokenizer concurrently. (b) TUI ordering: D3 CONFIRMED — tokenizer +
  --check as one task (C3 corpus gate is the tokenizer's own validation),
  FTXUI shell immediately after, content lighting up as fragments merge.
  Corpus boundary [measured]: ~/.spacecrafter/scripts/ EMPTY on this
  laptop; C3 corpus here = doc/superscript.sts + engine-exercised
  harness .sts; full shipped-scripts gate re-arms when the data package
  lands. Merge deferral recorded in SWEEP_DISPATCH (fragments committed
  per-unit; single sc-grammar.json merge after tokenizer lands, keeping
  its runtime input stable).
- **[2026-08-04] Sweep dispatched (units armed); TUI spec refinement.**
  Vixy [stated]: next load-bearing candidate = the TUI slice, deferring to
  the recorded plan if one exists — it does (§5 item 1, journal 2026-08-03),
  and the plans converge: the sweep produces the TUI's content (per-key doc,
  defaults, completion candidates), the tokenizer its cursor→token engine.
  TUI spec refinement [vixy, this session]: "auto-completion, greyed text
  when autocomplete with tab is possible to tell what would be completed,
  and showing the documentation related to each case" — generalizes D31's
  greyed-defaults to ghost-text preview for ANY tab completion, not only
  defaults in empty value fields. Dispatch protocol per Vixy: fully-specified
  tasks → opus-xhigh, clear DoD, pointed at this INTENT.
  Armed: `SWEEP_DISPATCH.md` (4 units partitioning all 339 `args[`
  occurrences at HEAD — 68/101/93/77, 0 outside the ranges [measured];
  fragment schema; 9-item DoD; merge protocol). Unit 1 (:1180–2109, 16
  commands) dispatched. Census reconciliation (339 vs dated 366) assigned to
  merge time. Open with Vixy: dispatch concurrency (recorded sequential
  protocol vs disjoint-write-surface parallel), TUI timing vs D3 ordering.
- **[2026-08-03] Slice 1 landed — contract seeded, gate green.**
  Post-migration session (memory files not migrated; graph-memory
  effectively empty on this laptop — 5 stray nodes; assessed low-impact
  by externalization discipline, residual: old-machine-only graph content
  unenumerable from here). Read: FEATURE_REQUESTS [2026-07-29] entry,
  D30–D36 + D31 answer (DECISIONS_PENDING), capability-surface.md
  (channel model §1, census §2, B38 §3.8), io.hpp (§5.47 routing),
  parseCommand + executeCommand + setFlag (app_command_interface.cpp
  :110-345), app_command_init.cpp (full), base_command_interface.hpp
  (full), extension.ts (full), script.cpp:114. Extracted → 
  `util/scedit/grammar/sc-grammar.json`: parse model (incl. dangling-key
  drop, duplicate-key-last-wins, `"`-only quoting, space-after-quote
  normalization, column-0-only `#` comments at the script layer, live
  channels never comment-stripped), 7 token families (60/97/43/46/7/24/
  10), W_* vocabulary (~220 spellings, roles unassigned), 12 lint seeds
  (GNU ids). Deltas vs census: `session` +1 (B31); `sky_draw` orphan and
  the five duplicate `#define`s confirmed at HEAD; sts-extension carries
  no grammar and mis-highlights three constructs the engine lacks
  (`'`-strings, `\"` escapes, any-position `#`). Scaffold: README,
  CMakeLists (standalone, util/ pattern), vendored nlohmann v3.11.3
  (sha256 `9bea4c8066ef4a1c…`), `src/main.cpp` validator; compiled and
  gate run green (all counts + uniqueness). Vixy in-session: GNU ids
  (D6), zero-knowledge doc bar (C6/D7). Slice order per D3; next is §5
  item 1 (per-handler sweep).
