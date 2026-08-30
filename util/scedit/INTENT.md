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
  **[AMENDED 2026-08-04, vixy]** Fourth anchor class: `doc/superscript.sts`
  as USAGE-WITNESS — it "served as documentation (and functional test,
  the only one) for years" [vixy, this session]. Tier: below code, above
  nothing; reliability precondition stated by Vixy himself: "it might
  have (rarely) been wrong if it wasn't understood when documented."
  Consequently a witness claim is citable as `[superscript-attested:
  line]` ONLY cross-checked against code; code-vs-witness divergence is
  never silently resolved — it is a finding either way (doc-error at
  writing time, or engine drift since; the sweep already holds instances
  of the drift mode: `date_display_*` → `datetime_display_*`,
  `movetocity`).
  **[AMENDED 2026-08-04 (2nd), vixy]** Flag ROUTING splits by ownership:
  witness-vs-code divergences and script-surface semantics belong to the
  MAIN USER/TESTER — "who own the script surface (who is the one writing
  the most scripts by a good margin)" and drives the project's
  development — not to Vixy; they are written to `claude/SCRIPT_SURFACE.md`
  (SS-n ids, relay via Vixy, USER_QUESTIONS conventions: observables,
  never internals). Vixy remains the design authority for engine
  internals and scedit's own design (the round-2 lesson, §11.70 era:
  route questions to the party who can answer them).
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

*Items 1–3 DONE (journal 2026-08-04b/c/e; code `73cc7b80`): sweep
merged (324 arg specs, schema v2 partial), tokenizer + `--check` live
(oracle 53 058/0, C3 armed and holding at zero false positives), the
second `'#'` site resolved. Struck bodies kept below for their scoping
notes.*

1. ~~**Per-handler extraction sweep** — `app_command_interface.cpp` (4747
   lines): per command, the arg keys read, value domains, defaults,
   REQUIRED/optional structure, and the C6 doc line for each; reconcile
   the `args[KEY]` occurrence count at HEAD against the census's dated
   366; assign the `argument_token_vocabulary` roles (keys vs enumerated
   values); confirm/retire the census's never-referenced `W_*` set.
   Dispatchable in per-handler units with count gates.~~
2. ~~**Tokenizer + `--check`** (C1-faithful; GNU-style output per D6);
   then arm C3's corpus gate harness.~~
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
7. ~~**FTXUI shell** — editor + cursor-driven doc panel (C6) + completion
   (defaults greyed, D31 spec).~~ **DONE 2026-08-04 (journal
   2026-08-04h; gated; code `0745dc34`). D1 discharged: FTXUI v5.0.0
   vendored, sha256 `a2991cb2…`, verbatim upstream source lists.
   Default-greyed ghosts DORMANT BY DATA (see item 11).**
10. **`Diagnostic` gains a `Span`** [flagged by the shell slice] —
    sc_check's finding struct carries no column, so no consumer can
    underline the exact byte; the editor reaches invisible-separator's
    column from the bytes (display fact, not a rule copy). Small,
    contract-level, next sc_check touch.
11. **`default_value` backfill** [flagged by the shell slice] — all 324
    arg-spec defaults are prose; D31's default-greyed ghost arms per
    entry when a literal `default_value` lands. 35 candidates reduce to
    bare tokens by inspection (enumerated in the slice report) — a doc
    pass writes them as DATA, each source-anchored, never regexed from
    prose (C2).
12. **`completable` marker on values[]** [flagged by the shell slice] —
    values mix literals with prose; the editor's bare-token filter has
    one known false positive (`xRRGGBB`). Schema marker + validator
    check at next grammar touch.
13. **`inline-comment` seed** [flagged by the 2026-08-30 corpus run] — a
    mid-line token whose KEY begins with `#` is an inline-comment
    attempt; the engine reads it and everything after it as parameters
    (script.cpp:114 is column-1-only). The rewritten witness holds 8
    TRUE instances producing 24 findings under generic ids
    (unknown-parameter / dangling-key) with actively confusing messages
    (`'#' … did you mean 'b'?`); a dedicated seed replaces them with 8
    messages naming the actual mistake. C3-groundable: no legitimate
    `#`-initial key exists anywhere in corpus or grammar. Touches
    lint_seeds + sc_check + lint_cases + both expected files.
    **RULED [vixy 2026-08-30]: the ENGINE will treat mid-line `#` as a
    real comment** — the legacy defect only "passes" because unrecognized
    commands/keys are inert, so make the commenting real. Sequencing per
    C1 (scedit tracks the engine at HEAD): the seed stays valid and
    mintable for TODAY's engine; when the engine change lands, parse_model
    + tokenizer + oracle update, the seed retires, and witness lines 37-46
    become CORRECT (SS-20 resolves engine-side, no script edit needed).
    Implementation precondition flagged for the engine change: a `#`
    inside a quoted value must NOT open a comment (quoting-aware scan),
    and the shipped corpus wants a `#`-in-values sweep at that moment.
14. **`unclosed-struct` seed** [same run] — a `struct if` (or loop) left
    unclosed at end of file: ifSwap is a stack, popped only by `end`
    (:4611), skipped-while-set at :225 — unclosed + false condition
    silently discards the file's whole tail. One TRUE instance in the
    witness (:1547 → SS-24); rule needs the skip-region/nesting care
    that was deliberately deferred at merge time (2026-08-04d). Veto
    open.
    **Context [vixy 2026-08-30]:** ifSwap was originally a boolean; the
    intent is to SCOPE if-state to the script — leaving a script shall
    leave its ifs (today the stack is global and leaks across the
    script-in-script splice), and nested `script action stop` is messy —
    cleaned up in the non-legacy exec policies while `legacy` preserves
    old behavior under old syntax (`[parallel-script]` refinement,
    harness `a501eb4`). The seed is UNAFFECTED: an unclosed `if` is a
    defect under every policy; scoping only bounds its blast radius to
    the script's own tail — which is exactly the witness's damage.
    **RULED [vixy 2026-08-30]: the engine must LOG the unclosed if**, per
    the three-part log schema (INTENT.md:126, §11.169 refinement): CAUSE
    (struct if opened at line L, never closed before script end) +
    CONTENT (lines after L skipped / if-state discarded) + self-contained
    ACTION (add `struct if end`; the unclosed opener's line quoted — the
    diagnostic points at the OPENER, the root, not at EOF where the
    damage surfaces). Both suspended micro-points RESOLVED same session
    [vixy]: (a) "inplace" = the ENGINE writes a `#!` trailing annotation
    on the faulty line, idempotently replaced — full mechanism and its
    flagged consequences in FEATURE_REQUESTS [2026-08-30] `#!` entry;
    (b) "bad idea regardless" referred to unclosed-if DIAGNOSIS under
    the inline policy (line provenance of spliced lines), not to the
    policy itself — no reversal.
15. **`#!` annotation surface in the editor** [vixy 2026-08-30, ruled
    with the `#!` engine feature]: scedit must (a) RECOGNIZE `#!`
    machine annotations as navigable errors — shown on the doc bar when
    the caret is on the line, otherwise listed in an error history with
    click-to-warp-cursor; (b) never WRITE or rewrite `#!` content (the
    engine owns that channel; roundtrip byte-exactness untouched);
    (c) the caret must use the standard escape-sequence
    foreground/background inversion so it is visible in a tui [vixy,
    verbatim requirement — check what sc_tui does today and gate it in
    ui_selftest]. Adjacent, not ruled: a `#!` whose content disagrees
    with what scedit itself derives for that line is a C1 divergence
    signal worth surfacing, not hiding.
8. **Engine emitter** — the grammar file becomes a build/runtime
   artifact (D5 seams); propose upstream once the contract shape has
   survived slices 1–4.
9. ~~**superscript.sts doc-mining pass**~~ **DONE 2026-08-04 (journal
   2026-08-04g; gated). Residue: S-NP-1 suspended for Vixy; 6
   code-consistent doc answers queued for post-FTXUI grammar merge.**
   [added 2026-08-04 on Vixy's
   source revelation] — extract the witness layer: comment lines +
   per-command/per-key usage patterns (value examples, co-occurrence,
   ordering) → candidate answers for the flagged doc gaps
   (`[superscript-attested]`, code-cross-checked per amended C2) +
   divergence table (each row dispositioned doc-error vs engine-drift).
   Fragment-output pattern (`grammar/witness/…`, supervisor merges), so
   it can run PARALLEL to item 7 (FTXUI) under the testability
   criterion — disjoint writes, read-only corpus. Dispatchable after
   the args merge gates.

## 6. Journal (append-only)

- **[2026-08-30c] The blocker dissolves: the field was here all along.**
  Vixy traced "how to make spacecrafter work here" through dependencies
  IN MEMORY and hit his own stale cache: the `.spacecrafter` data had
  been brought to this laptop for **fable-free-session-01** (a free
  session, so that instance could see the sky from here) — a state
  change whose side-effect his planning model never registered
  ("definitely a stale state cache, not invalidated on state change"
  [vixy] — I3 at the human layer, writer and cache-holder the same
  person). Verified at filesystem, not taken from recall: `~/.
  spacecrafter` present, 11 GB; **config.ini/ssystem.ini md5 = the
  recorded pristine pair** (`03fbee59`/`545a51ef`, §11.149(d3)) — the
  laptop field IS the recorded field, every field-dependent conclusion
  transfers; **`scripts/` holds 408 `.sts`** in 8 categories — the
  2026-08-04b "EMPTY on this laptop" measurement is SUPERSEDED (it
  carried its date and tag, so it aged honestly; the data drop
  post-dates it). Consequences: (1) **C3's shipped-scripts half is
  armable** — 408 scripts into SCEDIT_CORPUS, every finding
  dispositioned (at witness density expect a real crop; dispatchable
  sweep, fragment pattern); (2) engine configure probe on this laptop:
  CLEAN (C++23, -Ofast, deps resolved), full build launched in
  background — if it completes, item 6's blocker and the five
  2026-08-30b rulings' verification blocker dissolve, and the session
  premise "can't work on spacecrafter [on this machine]" is retired.
  The blocker list shrank by memory-trace + one `ls` + one configure.
- **[2026-08-30b] Rulings land the same evening, and the remotes turn
  out diverged.** Vixy in-conversation: (1) item 13 RULED — mid-line `#`
  becomes a real engine comment (ruling + sequencing on the item); (2)
  item 14 gains its design context — script-scoped ifSwap intent, seed
  unaffected; (3) NEW feature idea recorded (FEATURE_REQUESTS
  2026-08-30): truncated keywords execute as-if expanded on unique
  match — measured against the grammar: `mod` is AMBIGUOUS
  (`mode`/`modulo`, the motivating line stays dead), and exact-match-
  wins is a mandatory clause (`body`→`body_trace`, `dso`→`dso2d/3d`);
  (4) `[parallel-script]` refined by Vixy (harness `a501eb4`, txt is
  authority): `legacy` default policy + `inline` policy — legacy
  nesting ≈ inline already (addScriptFirst splice + global ifSwap
  leak). **Repo-state finding, next push is NOT fast-forward:** both
  GitHub remotes hold desktop-side commits absent from this migrated
  tree — code `master-beta` @ `76ee38c7` (*"empty locale dir no longer
  detonates two calls later"*, translator.cpp pop_back guard, sits on
  `0745dc34` — so the remote also LACKS this tree's f0c8ef83..6a4d184b
  series), harness `CC-harness` @ `eb9af25c` (*"Reorganize files…"*).
  Reconciliation owed before any push, from whichever machine holds
  both lines. Probe-method note, honest: the grammar's three family
  shapes bit a second probe today (commands = name-keyed dict with
  `_meta` keys); accessor now verified against all three shapes.
- **[2026-08-30] Third corpus run — the rewritten witness read by the
  checker; §11.149(e)'s stated residue discharged.** Code `6a4d184b`.
  Context: upstream rewrote `doc/superscript.sts` 2026-08-26 (`f0c8ef83`,
  1407 → 1606 lines, ISO-8859 + CRLF preserved); the desktop probe
  (parent §11.149(e)) verified the old findings byte-wise but named two
  gaps — 267 added lines unexamined, checker not re-runnable there. Both
  closed today on ~~LovelyFoxDev~~ **TravellingFoxDev** [CORRECTED
  2026-08-30, same session: `hostname` measured on Vixy's prompt — the
  LAPTOP is TravellingFoxDev, the desktop is LovelyFoxDev. The original
  name was reconstructed from a message label (itself inverted) instead
  of measured; recall-reliability class — empirical cells are copied
  from reads, never re-serialized. The `build-lovely/` dir name is the
  error's fossil, kept. A SessionStart env-identification hook
  (hostname/nproc/ram/gpu) was proposed to close the class]: clean
  rebuild (fresh `build-lovely/`;
  the migrated tree arrived foxy-owned — Vixy chowned mid-session),
  8/8 gates, roundtrip byte-exact on the new file. corpus-expected
  19 → 33, every row dispositioned (derivation-diff §7.3, zero false
  positives): 15 cleared TRUE by the rewrite; 2 line-drift survivors
  (:76 halo dup, :875 landscape spacecraft = SS-9); 1 half-fix — the
  `date_display_*` respell fixed the two `set` lines, not the `flag`
  line (:373 — no such flag exists; SS-3 amended); 30 new, of which 24
  are ONE root (trailing-`#` inline comments ×8, SS-20), plus
  `(Warning!` prose ×2 (SS-21), `mod` vs registered `modulo` (SS-22),
  `text "word"` shorthand ×2 dropped silently (SS-23). One
  rule-invisible defect recorded so silence ≠ health: unclosed
  `struct if current_mode equal 0` at :1547 — polarity verified at
  :4615-4621 (equal-false → push(true) → skip): any non-solar-system
  mode silently discards the file's tail (SS-24). Items 13/14 queued
  (two candidate seeds, veto open); SCRIPT_SURFACE §4 created,
  SS-19's never-exercised set 12 → 9 (transition/dso2d/domemasters now
  witnessed). Witness's new DOC layer noted as mining input, not
  merged (grammar byte-untouched this session): comments documenting
  flags `body_pick`/`dual_viewport`/`image_compression_loss`/
  `lunar_eclipse_*`/`skip_pause`, `illuminate` size-as-brightness
  semantics, `landscape landing` digit encoding, and the comet-tail
  key set + defaults (item 4 input). `z_reflection` witness claim
  checked at code: READ on the restart path (dsoNavigator.cpp:304) —
  code-consistent, the old "inert rider" note does not cover this
  site. Record notes: §7.0's "20 findings / +8" prose is off-by-one
  vs its own 19-line record (noted in §7.3, left in place).
- **[2026-08-04h] FTXUI shell gated GREEN — the D31 editor exists.**
  Code `0745dc34`. Supervisor re-gate: clean build 0 warnings, 8/8
  gates (5 inherited + editcore 154 checks + md5 round-trip +
  ui-selftest, all three new ones falsification-tested by the
  executor), grammar/tokenizer byte-untouched, write surface exact.
  Architecture: sc_document (byte-exact buffer; CRLF lives in the line
  TERMINATOR — mid-task correction found by pty drive: End/Enter were
  interacting with a text-resident `\r`) → sc_editcore (headless
  interaction: cursor→token through the normalization map, completion
  whose ghost is BY CONSTRUCTION the bytes Tab inserts, doc bar that
  labels what its sentence documents, findings recomputed whole-buffer
  per edit — comment blocks make lines non-local; 2.0 ms on the
  reference script) → sc_tui (renders, forwards, decides nothing).
  New reader sc_docindex = second READING of the grammar, not second
  authority (I2 stated). FTXUI v5.0.0 vendored (110 files, sha
  recorded, upstream source lists verbatim, PROVENANCE file). Ghost
  semantics: armed only where inserting a suffix produces the
  candidate; args_complete:false renders as an OPEN key list. Honest
  blanks for the 184 undocumented family names. Slice flags →
  items 10 (Diagnostic Span), 11 (default_value backfill, 35
  candidates), 12 (completable marker); ISO-8859-1-vs-15 display
  assumption recorded, undecided. Two scedit-owned UX calls taken in
  the display-cap precedent, both README-stated (offer filter;
  selected-candidate ghost). Item 7 struck; D1 discharged.
  **Remaining open surface: item 4 (stellar grammar — unlocks body/
  camera args_complete + SS-answers), item 5 (eval), item 6 (TCP,
  blocked on engine rebuild), item 8 (emitter), items 10-12, doc
  passes (flags/colors families), S-NP-1, and the SS-1…19 answers.**
- **[2026-08-04g] Witness mining gated GREEN — the witness answers, and
  it also indicts.** Coverage closes: 477/477 comment lines accounted,
  27/27 flagged gaps visited (8 answered code-consistent → mergeable as
  docs; 4 divergent → findings; 17 no-witness-evidence, 7 of those by
  total absence), 25 divergences (10 re-derive §5.97's catalogue
  independently, 15 new), 12/62 commands never exercised. Supervisor
  spot-verified at source: core.cpp "Satun":2177/"Ganymed":2173 →
  **§5.98** + SS-17; line-1205 bytes carry `=` → §5.97c + SS-6 QUOTE
  CORRECTED (supervisor's recalled-anchor slip: quoted a report, not
  the file); drift commit db7415d7 confirmed. SECOND WITNESS FOUND:
  `util/new_parser_scripts/input_fr.txt` — a declared-format French
  per-command/key/value reference (Nicolas Barile, 2020-06-03) with its
  own text→HTML toolchain; fully extracted (256 slots, 0 unparsed),
  tagged `[new-parser-fr-attested]`, kept STRICTLY separate from
  superscript attestations. It second-attests SS-2/SS-4/SS-10
  (→ "feature that went away" leaning) and documents `sun_trace` as a
  body_trace-with-Sun alias (SS-11 evidence) + `sky_draw` (the orphan
  define) as a dome point-plotter. One of its claims decided AGAINST it
  by code (print's keys), one off-by-one (audio default 84 not 85) —
  the reliability bound is live. **SUSPENDED FOR VIXY: S-NP-1** — does
  input_fr.txt join C2's anchor classes as a second usage-witness, with
  what reliability statement? Nothing from it merged pending the answer.
  QUEUED post-FTXUI (grammar frozen while that agent reads it): merge of
  the 6 code-consistent witness answers as docs with attestation tags.
  SCRIPT_SURFACE.md §3 filled (SS-17/18/19 added, SS-2/4/5/6/10/11
  updated). Item 9 struck.
- **[2026-08-04f] Ownership re-route: the script surface has an owner,
  and it is not Vixy.** Vixy [stated]: witness-vs-code divergences and
  script-surface semantics belong to the MAIN USER/TESTER, who owns the
  script surface (writes the most scripts by a good margin) and drives
  development (Vixy also pushes own ideas — e.g. experimental
  path-traced shadows — and remains design authority for engine
  internals and scedit itself). Created `claude/SCRIPT_SURFACE.md`
  (SS-n ids, USER_QUESTIONS conventions, relay via Vixy): SS-1…10 the
  known superscript divergences incl. §5.97's, SS-11…16 the
  surface-owned decisions previously mis-addressed to Vixy (suntrace
  §5.91, recording contract §5.96, subtitle spelling §5.36,
  configuration-module usage §5.94, Preset casing §5.95, superscript
  fix-vs-annotate). C2 amended (2nd): flag routing splits by ownership.
  Parent rows 74/79/80 annotated; CLAUDE.md channel list updated. This
  repeats round 1's recorded lesson (questions must go to the party who
  can answer them) — the mis-addressing is named, not silently fixed.
  Mining agent's divergence table appends to SCRIPT_SURFACE §3 when
  gated.
- **[2026-08-04e] Merge gated GREEN — the contract is filled and armed;
  Vixy elevates superscript.sts.** Merge (code `73cc7b80`): 324 per-key
  arg specs across 62 command entries; fragments folded FIELD-IDENTICAL
  under a falsification-tested equality gate that stays in the validator
  (I2 closure for the two-copies exposure); set's 43 facts single-sourced
  in `families.set_names` (I2, ratified); `pretable` explicit data (I6,
  ratified — presence-inference would have promoted all 62 commands);
  `args_complete:false` on body/camera/flyto with lint suppression
  demonstrated; 8 parse_model riders anchored; `invisible-separator`
  minted (veto open) with co-firing policy documented; did-you-mean
  display cap max(2, len/3) — oracle untouched, 53 058/0 re-run;
  **census −27 DISSOLVED: unit mismatch** (census counted tokens = 366,
  sweep counted lines = 336@census-era; like-for-like drift +3,
  attributed per-commit, residual zero — supervisor's own dispatch
  framing had propagated the confusion). Supervisor re-gate: clean
  build 5/5, probes (body-silent / flyto-alias-only / :94 names 0xA0 /
  :306 keeps close suggestion), tokenizer+oracle byte-untouched,
  fragment diffs = exactly the `_merged` marker, census token count
  verified at `0453f75e`. Arg-armed corpus: six MORE dead superscript
  lines (all dispositioned TRUE) → §5.97 extended; sharpest: `wait
  action reset_timer` does not wait and reports failure. Residual gaps,
  stated: `font`'s family half unarmed (FontFactory unread); five
  families still v1 arrays awaiting their doc passes; `body nmae Earth`
  silent BY DESIGN until item 4's grammar lands (args_complete cost).
  Items 1–3 struck. Vixy [stated, this session]: superscript.sts =
  years of documentation + the ONLY functional test → C2 amended
  (usage-witness tier), item 9 queued (witness mining). NEXT: FTXUI
  shell (item 7) ∥ witness mining (item 9) — disjoint surfaces.
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
  defects → §5.97 upstream (0xA0 eats `albedo 1`; the dead
  `set home_planet … duration 5` line = §5.96(a) materialized).
  Upstream batch executed (harness `46ee672`): NEW §5.94 (configuration
  fall-through — verified fully: star-catalogue save also runs §5.42's
  config writer; unknown module re-runs App::init before reporting),
  §5.95 (date W_PRESET||W_PRESET + no-effect `_()` statement), §5.96
  (recorder-rewrite class, 4 shapes), §5.97 (superscript.sts);
  annotations: §5.92 conversion inventory discharged, §5.93 body-color
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
  the parent ledger as **§5.91** (suntrace sun-as-key + error-less
  branches), **§5.92** (dso3d raw stoi, no catch on the chain — verified
  by grep, 'try' hits were directory_iterator substrings; z_reflection
  inert rider), **§5.93** (color by-value debug_message — full chain
  re-verified: the two range errors are the only messages, isOkay stays
  false, caller returns success on empty message); softer findings
  (get-no-TCP silent success, wait-loading fallthrough, dso2d naming
  trap, missing units) stay in the fragment's flagged/notes — lint-seed
  grade, not engine-defect grade. Incident, named per house honesty: a
  python slice reordering §5 rows TRUNCATED the parent ledger (515
  lines) and the post-edit check verified row order, not file integrity
  — committed truncated (a105e22), caught by the diffstat, restored
  from HEAD~1 with an only-insertion diff gate (231a149). Root: a
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
