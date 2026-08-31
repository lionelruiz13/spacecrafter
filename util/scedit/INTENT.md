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
  **[AMENDED 2026-08-31, vixy]** For a behaviour Vixy has RULED, the
  reference is the ruled engine, ahead of HEAD: *"make scedit track what
  the HEAD would be after the behavior get corrected, then we correct
  spacecrafter to be in face"*. Mechanism: the oracle test's copy of
  `parseCommand` becomes the TARGET (the ruled block written there in the
  exact form the engine receives), scedit is measured against it, and the
  engine gets the identical code — the copy is verbatim again the moment
  the engine commit lands (first instance: the comment rule, code
  `3d9179d2`, same day). Target = HEAD only, no version ranges [vixy
  2026-08-31, "scedit target HEAD only, yes"].
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
- ~~**Tokenizer gate** (pending): derivation-diff against
  `parseCommand` + differential corpus runs; the corpus gate C3 arms when
  `--check` exists.~~ **LIVE since 2026-08-04d (journal): `parse_oracle` —
  scedit vs a verbatim copy of `parseCommand` (the ruled TARGET under amended
  C1) — 119 337 comparisons / 0 mismatches at 2026-08-31; `tests/
  derivation-diff.md` is the derivation-diff; C3's corpus gate armed the same
  day (witness + harness corpus; the 408 shipped scripts await item 16).**
- ~~Future live legs (TCP) inherit the parent harness disciplines
  (fresh-launch, md5 in==out); no build exists on this laptop yet —
  rebuild is a prerequisite recorded, not done.~~ **SUPERSEDED 2026-08-31: the
  engine builds (GCC 11, `a3437670`) and RUNS here (display session for
  `claude`; F61–F63 over port 7805). Live legs keep the parent disciplines
  (`f27_reply.Session`: fresh temp-HOME launch, concurrent-instance probe,
  md5 in==out) plus this host's own precondition — a LOCKED screen throttles
  the engine to 1 Hz (`HOST-EVENTS.md` 2026-08-31).**
- **Gate inventory at 2026-08-31 (F67 delivered; `ctest` in `build-lovely` and in
  a fresh `build-f67`): 13 gates — tokenizer 189 · parse_oracle 119 337/0 ·
  editcore 266 · roundtrip · ui_selftest 20 frames · seed_gate · lint_rules 27 ·
  history_list 36 · corpus_gate 15 · check_json · doc_queries 10 ·
  mcp_protocol 77 · tcp_client 23/65. `-Wall -Wextra` on scedit's own targets,
  0 warnings. Live: `claude/harness/f67_tcp_live.py` 28/28, twice.**
- ~~**Gate inventory at 2026-08-31 (F66 delivered; `ctest` in `build-lovely`):
  12** — tokenizer 189 · parse_oracle 119 337/0 · editcore 223 · roundtrip
  (md5) · ui_selftest 17 frames · seed_gate · lint_rules 27 · history_list 36 ·
  corpus_gate 15 · check_json · doc_queries 10 · mcp_protocol 55; `-Wall
  -Wextra` on scedit's own targets, 0 warnings (item 20). Each record gate is
  edited deliberately and shown able to fail at the touch that adds it (README
  § Verification is the per-gate description; this line is the count of record).~~
  **[SUPERSEDED 2026-08-31 by the F67 line above; kept as the F66 state of
  record, per the maintenance invariant.]**

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
6. ~~**TCP client mode** — line protocol + `$LOGON` feed pane~~ **DONE
   2026-08-31 (F67; journal 2026-08-31h; code `584edade` -> `51cfc24d`).**
   `--tcp [[host:]port]`, `sc_tcpclient` (connect/$LOGON/send/$LOGOFF, bounded
   500-line feed, latin-1 both ways), the feed pane with its five keys, the
   `#!` write-back rule that loses neither the author's edits nor the engine's
   tails without an explicit choice, and MCP `run_command` over the same
   client. Gates 12 -> 13 (`tcp_client`), editcore 223 -> 266, ui 17 -> 20,
   mcp 55 -> 77; live 28/28 twice (`claude/harness/f67_tcp_live.py`).
   **THREE THINGS LEFT, none of them scedit's to decide:** the engine sends a
   client NOTHING about a script it plays (parent **§11.185**, routed to Vixy
   with the workaround's bounds); the write-back UX (second Ctrl-S takes the
   destructive branch) is a scedit call with the veto open; and no gate drives
   the editor's KEYS through a terminal — what is drawn is pinned by 20 frames
   and what the actions do by the same calls run headlessly, and the seam
   between them is read, not measured (README § What the editor cannot do yet).
   *(prerequisite MET 2026-08-31: the engine builds and RUNS here — display
   session for the `claude` user; F61/F62 drove it over port 7805.)*
7. ~~**FTXUI shell** — editor + cursor-driven doc panel (C6) + completion
   (defaults greyed, D31 spec).~~ **DONE 2026-08-04 (journal
   2026-08-04h; gated; code `0745dc34`). D1 discharged: FTXUI v5.0.0
   vendored, sha256 `a2991cb2…`, verbatim upstream source lists.
   Default-greyed ghosts DORMANT BY DATA (see item 11).**
10. ~~**`Diagnostic` gains a `Span`**~~ **DONE 2026-08-31 (journal
    2026-08-31): `Diagnostic::span`, filled by every rule; the editor
    underlines it and the look-alike marker now comes from the rule's span
    (the byte-derived second copy is gone); printed shape unchanged (D6,
    no column — a one-line decision left open).** [flagged by the shell slice] —
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
13. ~~**`inline-comment` seed**~~ **DONE 2026-08-31, in two steps the same
    day: MINTED for the HEAD defect (code `2fe14699`), then RETIRED when the
    rule flipped to the ruled behaviour under the amended C1 — tokenizer
    step 0 + oracle target copy + `indented-comment` retired too + the
    engine's parseCommand given the identical block (code `3d9179d2`;
    journal 2026-08-31b; derivation-diff §1 new row, §5.10 superseded,
    §7.5). Witness :37-46 correct as written; SS-20 resolved in-tree.**
    [flagged by the 2026-08-30 corpus run] — a
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
14. ~~**`unclosed-struct` seed**~~ **DONE 2026-08-31 (journal 2026-08-31;
    derivation-diff §5.11): `unclosed-struct` (at the OPENER) +
    `end-without-if` / `else-without-if` / `loop-end-without-loop`; the
    engine-side log + `#!` ruling below stays engine work.** [same run] — a `struct if` (or loop) left
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
    **(c) DONE 2026-08-31: sc_tui already drew the caret with FTXUI's
    `inverted` (SGR 7); the ui_selftest now prints an `inv` mask per frame
    and the record pins one inverted cell at the caret in all 10 frames.
    ~~(a)/(b) wait for the `#!` format to exist engine-side.~~ ENGINE HALF
    LANDED 2026-08-31 (code `2b8ec034`, parent §11.184, gate F63 34/34). The
    format scedit must recognise: the tail from the first `#!` at or after
    the first `#` outside quotes (the parser's own toggle — ScriptAnnotator::
    annotationBegin), written as ` #! <one ASCII sentence>` after the
    command or the author's own comment; several diagnostics on one line join
    with `; `. (a)/(b) are now unblocked; (b) holds by construction today
    (the editor writes bytes back and never composes a `#!`).**
    **(a-i) DONE 2026-08-31 (code `4a00cf31`): the tail is recognised
    (`EditCore::machineTail`, `Context::MachineTail`), shown on the bar's
    findings row with the caret anywhere on the line, and RELATED to scedit's
    own finding — agree / finds-nothing-now (fixed or disagree) / not a class
    scedit checks — the mapping carried as data (`engine_tail` on the four
    block seeds, anchored to the engine's MSG_* constants);
    `parse_model.comments.machine_tail` written; two selftest frames + E4f.
    ~~(a-ii) OPEN: the error-history pane in sc_tui listing every `#!` line
    (and scedit's findings) with click-to-warp-cursor.~~** ~~**(a-ii) MINTED as
    dispatch task F65 (2026-08-31e); the agreement scedit↔engine measured on
    F63's files: 12/12 (`harness/f63_scedit_agree.py`).**~~
    **ITEM 15 CLOSED WHOLE 2026-08-31 (F65; journal 2026-08-31f). (a-ii) DONE:
    `EditCore::errorHistory()` + `warpTo()` (code `cdda1ee4`), the pane with
    F5/F3/F4 and click-to-warp (`4c70f2aa`), the `--history` CLI twin over the
    same reader (`99260e28`). (a) and (b) therefore both discharged; (c) was
    done 2026-08-31 above. ONE READING FLAGGED FOR VIXY, NOT A DECISION I OWN:
    "history" is taken as the CURRENT BUFFER'S SET, not a log of past editing
    sessions — the argument is that the engine's `#!` channel already IS the
    log (a tail stays in the file until the fault is fixed and a run reaches a
    natural end), so the file carries the history and a second store could only
    go stale (I2). If the word meant the other thing — every diagnostic this
    session has seen, kept after it was fixed — it is a different feature with
    a store of its own: say the word. Stated in `util/scedit/README.md` § The
    error pane, with the keys, the placement, the toggle and the
    default-closed named as scedit's own calls, all veto-open.**
16. **Shipped-corpus dispositioning sweep** [measured 2026-08-30e] — the 13
    older seeds over the 408 shipped scripts: **1757 findings in 35 files**
    (duplicate-key 1596 — 1500 in the generated `internal/
    comet-particles.sts`, indented-comment 98, unknown-parameter 51,
    dangling-key 6, unknown-command 5, silent-off-value 1). C3 requires
    every one dispositioned before the shipped half joins the corpus gate
    (`SCEDIT_CORPUS` still carries the superseded "EMPTY" note). By
    inspection plausibly all TRUE, but "plausibly" is not a disposition.
    Dispatchable (fragment pattern, per-category units); true findings
    route to SCRIPT_SURFACE (SS-n) per C2's ownership split; a data-package
    fact to carry: 43 md5-identical script pairs (`navigation/fscripts/`
    mirrors `fscripts/`), so every fix lands twice.
17. ~~**SUSPENDED FOR VIXY — engine-version targeting.**~~ **RESOLVED
    [vixy 2026-08-31, verbatim: "scedit target HEAD only, yes."] — C1
    stands as written; a retiring seed simply retires (no `since:`/
    `until:` schema, no `--target`); the field-divergence consequence is
    accepted by the owner. Original question kept for the record.** C1 says scedit tracks
    the engine at HEAD; D9 says the installed field is frozen. Once the
    ruled changes land (mid-line `#` comments, `mod`/`div`/`mul`), a script
    written for the new engine is silently wrong on every older one (an
    inline comment becomes junk pairs again; `mod` becomes a dead line).
    Does scedit target ONE engine version (HEAD, current behaviour), or a
    declared version range (the grammar gains `since:`/`until:` on clauses
    and seeds, `--check --target <version>` selects; the retiring seeds
    become "portability" warnings instead of disappearing)? Not decided
    here: it changes the contract's shape, and the field's version spread
    is information only Vixy holds.
18. ~~**IN FLIGHT 2026-08-31 00:35 — engine change, uncommitted in the code
    tree until its build result is read:**~~ **DISCHARGED 2026-08-31: build
    log `EXIT=0` read (208 objects, 0 errors, the 2 warnings pre-existing
    in Camera.cpp:1009/1171), binary answers `--version`; committed
    `a3437670`; parent §11.181 written; journal 2026-08-31b.** `std::stacktrace` guarded by a
    CMake feature probe (`SPACECRAFTER_HAVE_STACKTRACE` + the matching
    link library: `stdc++exp` / `stdc++_libbacktrace` / none) instead of
    `__linux__` [vixy 2026-08-31, verbatim: "test a macro which tell if
    stacktrace is there, instead"]; `src/appModule/fps.cpp` + top-level
    `CMakeLists.txt`. Verified so far: probe → NOT available on GCC 11
    (all three variants fail, message printed), `fps.cpp.o` compiles
    clean; the GCC-15 path is identical by construction (first variant
    succeeds → same link line, same code) — desktop confirmation owed.
    Full engine build running (`/tmp/sc-build-2026-08-31.log`, `nice -j6`,
    RAM 8 GiB free at 24 objects). On completion: read the log's `EXIT=`
    line (never the notification's), commit the code change, write parent
    §11.181 (id verified free over live ∪ archive), journal 2026-08-31b
    here, then strike this item. If this item is still here in a later
    session, the build result was never read: re-run the build, do not
    assume.
8. **Engine emitter** — the grammar file becomes a build/runtime
   artifact (D5 seams); propose upstream once the contract shape has
   survived slices 1–4.
19. **LLM assistance over the documentation — the scedit-side surface**
    [vixy 2026-08-31, FEATURE_REQUESTS "LLM assistance over the command
    documentation"; untriaged, direction given: two roles, documentation
    helper = a router, coding helper delegated to any harness through an
    MCP binding]. What would land here: a `--doc <cmd> [member]` /
    `--search <words>` JSON surface on scedit's existing readers (one
    authority, I2) and an MCP stdio adapter over it (+ `--check`, + item 6's
    TCP line as tools); the router measured by `harness/f64_doc_router.py`
    (gemma4:latest, CPU-only: 45.0% one-level, 58.5% with family members as
    pages, baseline 23.5%); its next lever is the flags/colours per-name doc
    pass (items 11/12's neighbour). Waits for Vixy's triage (which mode
    first, local-only default, checker as a hard gate in agent mode).
    **Scedit-side surface DONE 2026-08-31 (F66, journal 2026-08-31g):
    `--doc` / `--search` / `--check --json` and a dual-era MCP stdio server over
    the same C++ readers, three tools (`doc_lookup`, `doc_search`,
    `check_script`) in one registry, 3 new gates, `--search` measured identical
    to the f64 baseline on all 340 witness questions. What is NOT done and is
    not scedit's to decide — the router/LLM half — with its three questions
    unchanged and unanswered: WHICH MODE FIRST (natural language → script,
    script → explanation, or NL → live command over TCP); LOCAL-ONLY DEFAULT or
    not; and IS THE CHECKER'S VERDICT A HARD GATE on execution in agent mode
    (rec: yes — C3's zero-false-positive discipline is what makes a hard gate
    acceptable). The MCP path needs none of them answered to be used today; the
    standalone router mode needs all three.**
20. ~~**The build sets no warning flags**~~ **DONE 2026-08-31 (F66 scope 7,
    journal 2026-08-31g): `-Wall -Wextra` on scedit's seven own targets only
    (the vendored trees compile as upstream ships them — the build-shape call,
    taken with its argument in `CMakeLists.txt`, veto open), `cmdSpan` deleted,
    0 warnings on a fresh build, and the flag shown live by a throwaway that
    warned. "0 warnings" can now be false.** [measured 2026-08-31, F65] — so
    "clean build, 0 warnings" has been a criterion that cannot fail. Under
    `-Wall -Wextra` the whole tree yields exactly ONE: `cmdSpan` set but not
    used, `src/sc_check.cpp:341` (there since `7fd5ea75`; no file F65 touched
    warns). The fix is two lines — an `add_compile_options(-Wall -Wextra)` in
    `CMakeLists.txt` and that variable's deletion — and it is left undone
    deliberately: it belongs to whoever can also decide whether the vendored
    FTXUI target should be excluded from the flags, which is a build-shape
    call rather than a defect. S.
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

- **[2026-08-31h] The engine at the other end of the editor: `--tcp`, the feed,
  the `#!` write-back that loses neither side — and the measured fact that the
  engine says nothing about a script it plays.** Dispatch task F67
  (`claude/fable-dispatch.md`), executor run, all six scopes delivered.
  Preconditions verified live before anything moved (§0.7): both HEADs and clean
  trees, **12/12** gates with every stated count re-measured from the gate's own
  output (tokenizer 189, oracle 119 337/0, editcore 223, ui 17 frames, lint 27,
  history 36, corpus 15, doc_queries 10, mcp 55), the engine binary at code HEAD,
  the display answering 1920x1080 on `:2`, the canary failing by construction and
  recorded rather than re-banked, no concurrent instance, the frozen md5 pair,
  §11.185 free, journal letter `h` free. No abort. One dispatcher path gloss
  reported: the section's sources name `INTENT/5.47.md`, which does not exist —
  §5.47 is CLOSED and its record is `INTENT/11.135.md` plus the live §5.72 row.
  **THE DESIGN IS DECIDED BY A MEASUREMENT, NOT BY A PREFERENCE.** Before writing
  a line of the client I asked the tree who can hear the engine: `setOutput` has
  exactly **two** callers, `get status …` and `search name …`
  (app_command_interface.cpp:1284-1301, :1415), plus the inline
  `$NOTICE`/`$LOGON`/`$LOGOFF` replies (io.cpp:640-663). **Everything else is
  silence** — a refused command, a script's start, a script's end, all of it goes
  to `debug_message` and the log (§5.117). Confirmed live: an entire play that
  produced findings put **zero bytes** on the wire after the subscription's own
  confirmation. So: the feed pane cannot be a success report, the `#!` reload
  cannot wait on an event, and `run_command`'s description has to say all of this
  to the model reading it. Parent **§11.185** records the gap with both readings
  and routes it; §5.72 and §5.117 carry back-markers to it in the same commit.
  **The client (`584edade`).** `sc_tcpclient`: connect + `$LOGON`, one line out,
  non-blocking read, `$LOGOFF` + close, state as data, ISO-8859 bytes untouched
  in both directions, a 500-line feed that COUNTS what its bound discards. The
  framing is the server's own — `ServerSocket::send` writes `strlen+1`, so a
  record ends in a NUL and `deliver` puts the `\n` inside it, and `$NOTICE`'s
  reply has no newline at all. Gate 13 `tcp_client`: `tests/fake_engine.py` is a
  stand-in whose every framing rule is read from a named line of `io.cpp`, and
  `tests/tcp_gate.py` runs one leg per process and then asserts what the stand-in
  RECEIVED — a leg cannot pass by agreeing with itself. 23 gate checks over 65
  leg checks.
  **The write-back, as a rule rather than a feature (`8a5c7ce3`).**
  `EditCore::diskState()` compares the file with `diskImage()`, the bytes this
  buffer was read from or last wrote — byte comparison, not a digest: it is a
  script, the cost is one read, and there is then no collision to reason about.
  `save()` runs it ALWAYS and REFUSES on Changed, naming both losses and taking
  neither; `reloadFromDisk()` and `saveOverwriting()` are the two ways out. A
  clean buffer is refused too, because a clean buffer holds the bytes from
  BEFORE the run. editcore **223 → 266**.
  **The editor (`917b0e2b`).** `--tcp [[host:]port]`, F6 connect, F7 send the
  caret's line, F8 play the file, F9 the feed, F11/F12 and the wheel to scroll
  it, Ctrl-U reload; control twins for all of them; **bound only when `--tcp` was
  given**, and the way that claim is checked rather than asserted is that all
  **seventeen** frames recorded before this commit are byte-identical after it.
  Three new frames carry a fifth mask (`feed LELEL`) read off the pixels: which
  lines scedit wrote and which the engine did is visible on the screen. ONE
  clock, with both numbers in the header and the README: the socket is drained
  4×/s while connected (reading what the peer pushed), and the played FILE is
  re-read **≤1 Hz, for ≤5 minutes, only after a play** — I3's admitted
  external-writer case, a convenience whose guarantee lives in `save()`.
  **`run_command` (`7b78229d`), and F66's seam held to the letter**: one entry in
  `registeredTools()` and one field on `ToolContext`, **no protocol code
  touched**. Its description carries the three things a model cannot work out —
  live dome, no undo; silence is neither success nor failure; a reply may be
  another client's. `wait_ms` bounded 100–10000 (asking for 1 ms and calling the
  absence a result is not a measurement). mcp **55 → 77**.
  **Live, 28/28, twice** (`claude/harness/f67_tcp_live.py`, predictions printed
  and written into the JSON before any leg ran): a command scedit sent is
  EXECUTED — `session action save` writes `stars = false` then `stars = true`, a
  TRANSITION read through a channel the client never touches; a `get` answered on
  scedit's own connection **exactly once** although it is also a subscriber
  (§5.47 live, by an implementation that is not the harness); a second client's
  answer arriving on the feed (§5.72 live); the write-back into a clean buffer
  with the tail on line 3, every other byte identical, and a separate
  `scedit --history` process reporting the same row and relating it to scedit's
  own (*agrees with scedit's `end-without-if`* — the C1 signal, agreeing);
  the dirty buffer REFUSED with the file byte-exactly what the engine left; and
  **the same driver FORCED**, where the tail is gone — the control that makes the
  refusal a fact about the code rather than a hope about the situation.
  **[measured 2026-08-31] Three reds on a stand-in engine, and two of them were
  Linux, not Python.** `close()` on a socket another thread is blocked in
  `recv()` or `accept()` on tears nothing down: the blocked call holds the open
  file description, no FIN goes out, and the port goes on listening. So the
  stand-in's "the engine went away" did not go away and its "nothing is
  listening" was still listening — and scedit was right both times to report
  nothing. `shutdown()` first, in both places.
  **[measured 2026-08-31] Four of my own checks were wrong before any code was.**
  The `$LOGOFF` count (the leg disconnects twice), the sent-lines count (the
  subscription is not a command), the close event asserted against ENGINE lines
  when it is by design a local one, and — twice, live — the `$LOGON`
  confirmation counted among the replies to the commands, which turned a true
  "0 replies" into a red "1". Each is corrected in place with the miscount
  written down beside it, and the refined count keeps its teeth because leg B is
  the positive map where a reply does arrive.
  **[measured 2026-08-31] An instrument that overwrote its own evidence, again.**
  F66 banked that class from `f64_doc_router.py`; my own live instrument met it
  on its second run — the first run's host record, the one that captured the
  LOCKED session, was overwritten by the second. Each run now writes a
  timestamped copy beside the stable name. The lost values are in the delivery.
  **[measured 2026-08-31] A label had been standing in for a path.** `openBytes`
  stored its LABEL where `open` stores a path, so a buffer with no file answered
  a question about a file that never existed ("gone"). Found by the new
  write-back check asking it. `hasFile()` now says which kind of buffer this is.
  **Host, disclosed (§11.174(h), owner veto item):** the session's screen was
  LOCKED at the first live run; the instrument read that, recorded it, woke the
  session and recorded that too. 1 and 2 frame stalls per run against 105/run
  locked (§11.183). No claim here is a timing claim.
  **State:** 13/13 gates green in `build-lovely` AND in a fresh `build-f67`
  (Release), **0 warnings** with the flags on scedit's own targets and absent
  from ftxui's. Item 6 struck; item 19's router half and items 11/12 unchanged.

- **[2026-08-31g] The documentation answers a machine: `--doc`, `--search`,
  `--check --json`, an MCP server over the same readers — and the warning bar
  that could not fail.** Dispatch task F66 (`claude/fable-dispatch.md`), executor
  run, all seven scopes delivered. Preconditions verified live before anything
  moved (§0.7): both HEADs and clean trees, 9/9 gates with the stated counts
  (tokenizer 189, parse_oracle 119337/0, editcore 223, lint 27, history 36,
  corpus 15), lint_seeds 15, `--history`'s seven fields, and — by measurement,
  not by reading — f64_doc_router.py's recorded baseline **80/340 reproduced**.
  No abort.
  **The surface (`170ce615`):** `--doc <cmd> [<key>|<family name>]` prints one
  page and `--doc` alone the two-level catalogue; `--search <words>` ranks pages;
  `--check --json` prints the findings as objects beside the untouched D6 text;
  `--mcp` serves all three as MCP tools on stdio. New module `sc_docjson`
  (serialisation only) and `sc_mcp` (the adapter); no second reader of the
  grammar, no model call, no network. **The honest-null rule survives
  serialisation**: `dso3d z_reflection` and `suntrace sun` answer `"doc": null`,
  `flag stars` answers `"present": true, "doc": null`, and all three are pinned
  by two gates. A name outside the vocabulary exits 2 with an object naming the
  vocabulary AND the nearest name — the CHECKER's `cappedSuggestion`, moved out
  of sc_check.cpp's anonymous namespace rather than written a second time, so
  `--doc` and `--check` cannot disagree about the engine's nearest name.
  **The ranking is F64's baseline, ported and measured (`3e28c92`):** the score
  is published in the header and in the README
  (`|words(Q) & words(P)| / (1 + sqrt(|words(P)|))`), and
  `harness/f66_search_parity.py` asks scedit all 340 witness questions and
  compares its top-1 against the baseline **question by question: 340/340 agree,
  both sides 80/340 = 23.5%**. Two things had to be right for that. (1) TIES:
  measured, 6 of the 340 tie at the top score and file order and alphabetical
  order disagree on all 6 — so `DocIndex` now parses with `ordered_json` and
  records `commandFileOrder()`, and enumeration order (the contract file's own)
  is the stated tie-break. (2) THE DEGENERATE CASE: 21 questions share no word
  with any command, where the baseline returns whichever command the file lists
  first; scedit returns nothing, the parity check reads
  `score > 0 ⇒ same pick; score == 0 ⇒ no answer`, and those 21 were hits zero
  times, which is why the recorded hit rate is untouched. `f64_doc_router.py`
  was refactored minimally to be importable without side effects (argv, prints,
  the model loop and the artifact writing moved into `main()`); its script path
  was re-run against ollama to prove it still works.
  **The MCP server is DUAL-ERA, and that is a fetched fact, not a recalled one.**
  `https://modelcontextprotocol.io/specification/`'s own "latest" pointer
  resolved to revision **2026-07-28** (fetched 2026-08-31). That revision has NO
  `initialize` handshake: it is stateless, carries the protocol version and the
  client capabilities in each request's `_meta`, and requires `server/discover`.
  The dispatch section assumed the handshake — it is the *legacy* era there
  (2025-11-25 and earlier), and it is what deployed clients still speak:
  **Claude Code 2.1.251 opens with `"protocolVersion": "2025-11-25"`, measured on
  this host** (`harness/artifacts/f66/claude_code_binding.log`: initialize →
  initialized → tools/list, the three tools returned; the model turn itself never
  ran, "Credit balance is too low", which the binding does not depend on). So the
  server answers per request in whichever era the request is in. A tool is
  declared in ONE place (`registeredTools()`); the protocol code names no tool,
  which is the seam item 6's `run_command` needs. What is not implemented is a
  LIST in the README, not a silence (resources, prompts, logging, completions,
  pagination, caching, subscriptions/list-changed, progress, cancellation,
  multi-round-trip + the client features it needs, outputSchema, icons, batches,
  extensions, authorization).
  **Gates 9 → 12 (`fbdf1d48`):** `check_json` (the same comparator, `--json` in
  its MODE — one finding set, two printers), `doc_queries` (ten recorded
  arguments → stdout + exit code, through a new comparator that says why it is
  not `check_gate.cmake`: many invocations, arguments that are not files, and an
  exit code that is part of the record), and `mcp_protocol` (**55 checks** from a
  stdlib-only Python client — a second implementation on purpose). Every new
  criterion shown able to fail, four tampers at once, all restored and
  md5-verified: an accept-any-tool-name and a wrong error code → exactly two red
  checks in `mcp_protocol`; one recorded `did_you_mean` and one recorded severity
  → `doc_queries` and `check_json` red.
  **The warning bar (`ac3e752e`, item 20 struck):** `-Wall -Wextra` on scedit's
  seven own targets only, with the argument in the file (the vendored trees
  compile as their authors ship them; measured after configuring: our objects
  carry the flags, ftxui's carry `-O3 -DNDEBUG -fPIC -std=gnu++17` and no more).
  The one warning the tree then yielded — `cmdSpan` set-but-unused in
  `LineChecker::rules`, nothing reading it — is deleted. Shown able to fail: a
  throwaway `int f66_throwaway = 0;` compiled with `-Wunused-variable` and the
  compile line carrying the flags; removed again. **From here "0 warnings" is a
  claim that can be false.**
  **[measured 2026-08-31] A hand-written copy of a contract fact was wrong, and
  the port found it.** F64's Python catalogue maps three commands to families
  (`{flag, set, color}`); the contract says **four** — `font` names
  `families.font_targets` (10 names). scedit's catalogue reads `subfamily` from
  the Grammar and lists all four, so the first mcp_protocol run failed on MY
  assertion, not on the code. Consequence for F64's numbers, recorded not fixed:
  its two-level run (199/340) showed the model 97 + 43 + 46 names and NOT the 10
  font targets, so any `font <target>` question was unanswerable at member level
  there. The instrument keeps its measured shape; the divergence is the point of
  this task — one authority, read, instead of a copy, retyped.
  **[measured 2026-08-31] A tamper must be shown to reach the criterion.** The
  parity check went green under two successive falsification attempts before the
  third worked: the first flipped a verdict for a question that is not in the
  set, the second patched `baseline()` while the check reads `baseline_scored()`.
  A green run whose tamper never fired is not evidence of anything.
  **[measured 2026-08-31] An instrument that overwrites its own evidence.**
  `f64_doc_router.py` names its artifact by model alone, so the `--limit 5`
  re-run that proved its script path still works overwrote the ROWS of F64's
  full one-level run (`artifacts/f64/gemma4_latest.json`, `summary.json`;
  untracked, so nothing in git was lost — the numbers stay in FEATURE_REQUESTS
  and the two-level `*_members.json` is intact). Recorded in
  `harness/README.md`; re-derivable in ~5 minutes. Left unfixed on purpose: the
  artifact paths are cited by name in the FR entry.
  **State:** 12/12 gates green in `build-lovely` AND in a fresh dir (`build-f66`,
  Release), **0 warnings** under the new flags. Item 19's scedit half is done;
  its three triage questions stay Vixy's. Item 20 struck.

- **[2026-08-31f] Item 15 closed whole: the error pane, the `--history` twin,
  the rule that says which lines can carry a `#!` — and the third reader of
  that rule deleted.** Dispatch task F65 (`claude/fable-dispatch.md`), executor
  run, all six scopes delivered. Preconditions verified live before anything
  moved (§0.7): both HEADs, the nine stated counts, F63's ten artifacts in
  their stated shapes — all as dispatched, no abort.
  **Headless first (`cdda1ee4`):** `EditCore::errorHistory()` lists every `#!`
  tail and every finding in line order, rebuilt with the diagnostics after each
  edit; `warpTo()` moves the caret to one, at the byte its span begins on.
  A line carrying BOTH gives TWO entries, the engine's first — they are two
  claims by two authors about one line (what happened when it RAN vs what the
  bytes say NOW) and the case where they differ IS the C1 signal item 15 names;
  merging them would hide it. editcore **193 → 223** checks.
  **`--history` (`99260e28`):** the same list printed, seven TAB-separated
  fields (file, line, source, id, severity, message, relation), so a harness
  reads what an author sees. New gate `history_list`, **8 → 9**, through the
  same comparator as the two `--check` records (`check_gate.cmake` gained a
  MODE rather than being copied a third time). Its fixture
  `tests/history_cases.sts` is F63-SHAPED — every `#!` in it is a sentence the
  engine wrote — and three of its ten cases are deliberate ABSENCES.
  **The pane (`4c70f2aa`):** F5/Ctrl-E show-hide, F3/Ctrl-N next, F4/Ctrl-P
  previous, click a row to go there; `>` + inversion mark the row the caret is
  on; ui_selftest **13 → 17** frames. Off by default with the COUNT always on
  the status line — the count is what makes it findable. **A design fault
  caught by its own record, worth keeping:** the first version stepped in BYTE
  order, and on a line carrying both, the engine's `#!` sits to the RIGHT of
  the finding's span — so F3 skipped that finding going down AND on the wrap,
  an entry the keyboard could never reach. Seen in a rendered frame
  (`pane-warp-twice` landed on line 3 instead of line 2), not by inspection.
  The stepper now walks the LIST, and the pane keeps exactly one piece of
  state — the row the last warp landed on — trusted only while the caret still
  stands where that row begins.
  **The rule, written where it belongs (`a3e44b64`):**
  `parse_model.comments.machine_tail` gains its EXECUTES-ONLY clause with three
  anchors — the script layer drops a line whose first byte is `#` before it is
  dispatched (`script.cpp:117`), so `ScriptAnnotator::saw`/`note` are never
  called for it (sole call site `script_mgr.cpp:337`) and `flush` iterates ONLY
  the notes it holds (`script_annotator.cpp:130`), so such a line is neither
  written NOR CLEARED; scedit reads it that way by construction
  (`sc_editcore.cpp:306`). Families, counts, seeds and the argument-token
  vocabulary md5-identical; the whole diff is 7 prose lines.
  **The third reader deleted (harness `1bd49f6`):** `f63_scedit_agree.py`
  carried its own copy of the `#!` locating rule — written from the grammar
  sentence, which lacked the clause, so the copy lacked it too and mis-read
  F.sts:1. It now consumes `--history`; result unchanged on the same artifacts,
  **12 tails / 12 agree / 0 disagreements / 6 expected**, and it gained the leg
  table for all ten of F63's files (an unknown artifact is now a loud failure,
  not a smaller pass).
  **Every new record gate shown able to fail** (each restored after): inverting
  the entry order → 9 red editcore checks; dropping `machineTail`'s
  executes-only guard → the three F4 checks red, which is EXACTLY the harness's
  original bug reproduced as a test; one recorded `--history` relation changed
  agrees→disagrees → `history_list` red; warping to column 0 → `ui_selftest`
  AND editcore red; one `>` removed from a recorded pane row → `ui_selftest`
  red; a tail rewritten to another fault class in a COPY of A.sts (the
  committed artifacts md5-untouched) → agree 11 / disagreements 2, one per
  direction.
  **[measured 2026-08-31] A citation-staleness instance, three copies of one
  line number:** engine `2b8ec034` inserted the annotator's line-provenance
  counter above `script.cpp`'s drop filter and moved it **114 → 117**. Three
  scedit-side citations of `:114` went stale in that one commit and none was
  updated with it — `parse_model.comments.script_layer`, `derivation-diff.md`
  §inner_script_channel, and the harness copy. All three corrected here. The
  shape is the lesson: a line-number anchor is a cached conclusion with no
  gate, and the commit that invalidates it is the one moment its author holds
  both ends.
  **[measured 2026-08-31] The "0 warnings" bar was not discriminating.** The
  project sets NO warning flags, so a green build proves only that it compiles.
  Rebuilt with `-Wall -Wextra`: **one** warning in the whole tree, a
  pre-existing `cmdSpan` set-but-unused in `sc_check.cpp:341` (last touched at
  `7fd5ea75`, before this task) — **none** in any file F65 changed. Left as
  found, out of this task's scope; a `-Wall -Wextra` line in `CMakeLists.txt`
  plus that one deletion is the whole fix, and is worth a future item.
  **State:** 9/9 gates green in a FRESH build dir (`build-f65`, Release) and in
  `build-lovely`. tokenizer 189 · parse_oracle 119 337/0 · editcore 223 ·
  roundtrip byte-exact · ui_selftest 17 frames · seed gate self-consistent ·
  lint 27 · history 36 · corpus 15. Item 15 closed; the "history" reading is
  the one thing flagged for Vixy.
- **[2026-08-31e] The previous changes verified live on this host, the `#!`
  agreement measured on engine-written files, one criterion corrected at its
  root, and the round minted (F65–F67).** Session 18, Vixy's dispatch line:
  *"Continue the work on scedit … test the previous changes landed properly
  before working on the next FEATURE_REQUEST.md entries related to scedit and
  script engine. Use one agent per feature to implement."* Verified (every
  number read from a log or a tool result, never from a notification): engine
  binary at HEAD (`cmake --build -n` lists nothing; last engine commit
  `2b8ec034`); scedit **8/8** on a rebuild (tokenizer 189, editcore 193, oracle
  119 337/0, 13 selftest frames); **F62 11/11, F63 34/34** on `2b8ec034`;
  **F61 15/16** at first — the one RED leg an INSTRUMENT defect, traced rather
  than explained away: the watchdog sends the process its own SIGUSR1 on every
  frame stall (fps.cpp:150-156), so "the WARNING exactly once" presupposed a
  stall-free window; the host was stalling at exactly 1000 ms for whole runs
  (105/run) on HEAD AND on the pre-fix control `a3437670` (which reproduced its
  8/16, the eight comment-rule legs) — cause: the claude session's SCREEN LOCK
  (compositor throttling; `HOST-EVENTS.md` 2026-08-31); criterion refined
  (quiescence wait + watchdog pairing, degrades LOUDLY when attribution is
  impossible); control run awake → **16/16, 1 stall/run**, prediction stated
  first, held. NEW instrument `harness/f63_scedit_agree.py`: scedit's reading vs
  the engine's `#!` verdict on F63's real artifacts — **12 tails / 12 agree / 0
  disagreements**, 6 expected findings in the refused-write files (E read-only
  directory, G changed-since-load); its first version mis-read F.sts:1
  (`# F: a #! …`, a column-0 comment) as a tail because the grammar sentence
  omits that only EXECUTED lines carry one (script.cpp:114 drops comment lines
  before executeCommand; `sc_editcore.cpp:252-255` already requires a command
  + a comment) → data-precision item folded into F65 scope 4; the script's own
  reader is a third reading, to be replaced by `--history` (F65 scope 3).
  Canary `--no-scene`: 2 FAIL on this host by construction (the desktop's bank)
  — reported, not re-banked. Remotes: local CONTAINS origin on both repos (push
  = fast-forward; `git fetch` itself refused — auth); the 2026-08-30b non-ff
  note is superseded by measurement. FEATURE_REQUESTS `#!` entry: the stale
  "(2) still owed" annotated (landed `4a00cf31`). Round minted in
  `claude/fable-dispatch.md` (§0b: a task without a section is not
  dispatchable; scedit tasks deliver to THIS ledger, stated in F65's header):
  **F65** = item 15(a-ii) error-history pane + `--history` CLI twin + the
  grammar clause; **F66** = item 19's scedit-side surface (`--doc` / `--search`
  / `--check --json` + an MCP stdio server over the C++ readers; NO LLM call —
  the router/triage half stays Vixy's); **F67** = item 6 direct TCP mode +
  `$LOGON` feed pane + the `#!` write-back handled by requirement + MCP
  `run_command`. NOT dispatched: the three untriaged engine requests
  (`[parallel-script]` → tester resubmission; `[script-binding]` /
  `[script-trigger]` → design decisions named at record time) — listed for
  Vixy in fable-dispatch §3. Order F65 → F66 → F67 (smallest and ruled first;
  the display-needing one last, while the display exists).
- **[2026-08-31d] The `#!` channel exists — engine half of item 15 landed
  (code `2b8ec034`, parent §11.184, harness `f63_annotations.py` 34/34).**
  Design, from the session's reading: provenance threaded (Token → LoopStep →
  executeCommand's origin overload with an RAII scope → IfSwap openers; loop
  opener tracked in the interface), the natural end (`terminateScript`, sole
  caller = the queue running out) audits both stacks before `script action
  end` discards them; `ScriptAnnotator` writes per file at script end
  (byte-compare, sibling temp + rename, clear stale tails at natural end
  only, skip a line changed since load, log-only when unwritable, CRLF kept).
  Producers = the ruled block-structure class; the generic `debug_message`
  channel deliberately NOT wired — ~1661 tails into 35 shipped scripts on
  first run — disclosed to Vixy in FEATURE_REQUESTS with the count. Gate legs
  incl. the splice (a fault in a script played by another lands in ITS file),
  the loop replay (two log lines, one tail), a read-only DIRECTORY (a file's
  own mode does not stop a rename — the directory is what the sibling-temp
  write needs). Two checker slips before green, both mine: a `flyto` sub-leg
  in F62 that could not succeed (dropped; the claim rides on `div`), and an
  F63 check that searched the log for a path the `Execute_command` line
  legitimately contains. **Item 15's scedit half, part one, same evening
  (code `4a00cf31`)**: the tail recognised and related to scedit's own finding
  (the C1 signal as a sentence on the bar: agree / finds-nothing-now /
  unknown class), `parse_model.comments.machine_tail`, `engine_tail` data on
  the four block seeds, two selftest frames, E4f; 8/8. Left for the next
  session: the error-history/click-to-warp pane (15 a-ii), and Vixy's
  decision on the generic channel.
- **[2026-08-31c] The engine runs here: both rulings confirmed live, the
  aliases land, and a claim three records carried turns out to name a map
  nothing reads.** Display session available for `claude` (`DISPLAY=:2`,
  XAUTHORITY under the user's own runtime dir — the harness README's F28
  recipe). (1) **F61** (`harness/f61_live_rulings.py`, parent §11.183): one
  fresh launch on a temp-HOME farm pays the confirmation §11.181/§11.182 owed
  — the comment rule on BOTH external channels (script file + TCP; trailing,
  glued, indented ×2, quoted, unquoted twin), flag state read back as
  TRANSITIONS through `session action save`, quoted `#` proven by the engine
  echoing the value it parsed, comment-only lines by the count of
  "Unrecognized" equalling the positive controls; SIGUSR1 → the watchdog's
  WARNING once, process alive. **16/16 on `3d9179d2`**; RED control on a
  staging build of `a3437670` (worktree; EntityCore submodule copied in, then
  RE-configured — the first link failed on `Set::~Set()` because CMake had
  globbed the empty dir): **8/16, the eight comment legs exactly**, SIGUSR1
  and quoted legs green on both. Driver slips kept as record: the
  "X is unknown" regex matched the command-lookup's did-you-mean too (fixed
  to the truthful shape); the session files lived only in the farm (copied to
  the artifact dir now); the foreign asan/tsan binaries do not load here
  (`libavcodec.so.61`). (2) **Aliases** (code `7fd5ea75`; F62 11/11): engine
  `div`/`mul`/`mod`, reverse map canonical-first with aliases added after;
  scedit alias entries resolved ONCE at load in both readers (Grammar,
  DocIndex), validator rule, C4b test, commands 60 → 63, corpus 16 → 15
  (`mod a 2` correct — SS-22 resolved in-tree). (3) **The correction**: the
  "recording trap" (m_commands_ToString first-name-wins flips recorded
  spellings) — carried by B38's row, §11.182, FEATURE_REQUESTS and scedit's
  own `alias-respelled` seed — named a map with NO consumer at HEAD;
  `recordCommand` writes the raw line and only `flag` toggles are
  re-serialised (measured: F62's recording holds `div y 2` verbatim and
  `flag stars 0`). Seed RETIRED (16 → 15); `recording_alias_loss` rewritten
  to the measured engine; every carrier annotated. Class: a claim about a
  data structure's construction propagated as a claim about behaviour, four
  records deep, without one grep for a consumer — the check cost one command.
  (4) Latent UB in editcore_test D5 (pointer into a temporary `EditCore`'s
  DocIndex) exposed when `CommandInfo` grew; fixed by binding the host. (5)
  Driver design lesson, F62: a recorded command must SUCCEED to be written;
  three `flyto` forms guessed blind all failed on camera state — the grammar
  scedit carries (`camera.args.target`) would have answered first; the claim
  rides on `div y 2` alone and the docstring says so. Next: the `#!`
  annotation (FEATURE_REQUESTS 2026-08-30; item 15's engine half) — design
  from the reading done this session: tokens carry no provenance (Token =
  text + script DIR), IfSwap holds booleans only, the natural end is
  `ScriptMgr::update`'s "script done" branch → `terminateScript()` →
  `ifSwap->reset()` silently, loop bodies replay from bare strings.
- **[2026-08-31b] The engine builds here; the comment rule flips to the
  ruled behaviour, scedit first, engine in phase — C1 amended.** Two
  engine commits, both compiled on GCC 11 into the probe-built tree, not
  run (no display): (1) `a3437670` — `std::stacktrace` becomes a CMake
  feature probe (`stdc++exp` → `stdc++_libbacktrace` → none; one answer
  drives the compile guard AND the link line; `__linux__` keeps the signal
  half; without stack support the SIGUSR1 handler logs WHY no stack
  follows) [vixy: "test a macro which tell if stacktrace is there,
  instead"]; full build `EXIT=0`, 208 objects, 0 errors — the tree had NO
  other GCC-15-only dependency, so the toolchain blocker of 2026-08-30e is
  gone for BUILDING (running still needs the display session). (2)
  `3d9179d2` — the comment rule. Vixy on the morning's `inline-comment`
  seed: *"# loop on is a comment, not to be parsed as syntax"*, then the
  order ruling: *"make scedit track what the HEAD would be after the
  behavior get corrected, then we correct spacecrafter to be in face"* →
  C1 amended (ruled engine > oracle > scedit; the oracle's parseCommand
  copy is the TARGET). Rule as written, both sides identical (diffed): a
  `#` outside a `"…"` run ends the command, quotes counted by a plain
  toggle from byte 0, the cut running BEFORE the leading-blank strip and
  the ` " ` normalisation; placed in parseCommand so every channel has it
  (alternative externalised: script-layer-only — rejected: two classifier
  sites already diverge, parseCommand is the one choke point incl. the
  nested executeCommand calls; observable difference only on live channels
  with an unquoted `#`, and the sweep shows none: no ini command string
  carries `#`, HTTP never delivers a raw `#`). Sweep before deciding the
  shape [measured, 408 + witness + harness]: `#` glued outside quotes 0,
  `#` inside quotes 0, word-start `#` 106 = 98 indented whole-line comments
  + the witness's 8 — so "outside quotes, `#` starts a comment" (simplest
  statement, zero-knowledge bar) and the shell-style word-start rule agree
  on the whole corpus; the simplest won, veto open. scedit: comment cut =
  tokenizer normalisation step 0 on (s, off) (erased/spans follow for
  free), `Line::comment_begin`, editor `Context::Comment` (no completion,
  bar says `comment` with the `mid_line` sentence), tail drawn dim;
  seeds `inline-comment` AND `indented-comment` retired (18 → 16); oracle
  alphabet gains `#`: 119 337 comparisons / 0 mismatches; gates 8/8;
  tokenizer 189, editcore 175. Corpus record 24 → 16 (the 8 witness lines
  are CORRECT as written — SS-20 resolved in-tree); 408 shipped 1759 →
  1661, diff vs previous run minus its `indented-comment` lines EMPTY:
  98 shipped indented comments stop executing as unknown commands. Ledger
  effects: item 13 done (minted then retired the same day — the minting
  was not wasted: it produced the sweep, the fixture shapes, and the
  finding that `# loop on`'s "collision" was a parse of a comment, which
  is what prompted the order ruling); item 17 RESOLVED (HEAD only); item
  18 discharged; FEATURE_REQUESTS `#!` ordering clause (1) satisfied; the
  alias entry gains an identified trap (m_commands_ToString first-name-
  wins would flip recorded spellings to `div`/`mod`/`mul` — the recorder's
  canonical name must be chosen, not inherited). Parent §11.181 (probe)
  and §11.182 (comment rule) written. Pre-existing warnings noted, not
  touched: Camera.cpp:1009 unused `decelerationDuration`, :1171 unused
  `readD`. Open: live confirmation of both engine changes on a running
  engine (display session; desktop build for the GCC-15 path of the
  probe); item 16 (shipped sweep, now 1661); the D6 column micro-decision;
  the mod/div/mul aliases (engine, with the trap above); item 4 next.
- **[2026-08-31] The sc_check touch: spans, five seeds, the block structure
  read — items 10, 13 (HEAD half), 14, 15(c) closed.** Code `2fe14699`.
  Gates 8/8 on a clean build (0 warnings); tokenizer 150 → 171 and editcore
  154 → 171 checks; both record gates falsification-tested (a tampered
  record line → FATAL, lint and frames). What landed: (a) item 10 —
  `Diagnostic::span`, filled by every rule; the editor underlines it and
  the look-alike marker now comes from `invisible-separator`'s span;
  `EditCore::lookalikeSpaceColumns` deleted — it marked 0xA0 only while the
  rule knows six byte shapes and the quoted-value exclusion, i.e. the two
  copies had ALREADY diverged silently (I2's failure mode, found by
  removing the copy, not by noticing the drift). Printed shape untouched
  (D6, no column) — adding gcc's `:col:` is a one-line decision, left
  open. (b) item 13 — `inline-comment`, PREFIX policy: one message per
  line stating what the tail does (computed from the engine's own `args`:
  a tail key that IS a key → "CHANGES"; `#` pair sorting first on a
  single-pair command → the intended pair "never applied"; first on `set`
  → "nothing applied"; else inert → "happens to work"; args_complete:false
  → "cannot tell"), the other rules reading the prefix the author meant.
  Deliberate departure from §5.9's co-firing, reason recorded
  (derivation-diff §5.10): separately-actionable is the test, and here one
  action removes every consequence while the consequence messages point
  away from it; precedent = indented-comment. The fixture caught MY
  mis-designed case: `media action pause # loop on` is not a collision —
  `loop` is the VALUE of key `#`, `on` dangles; the checker's reading was
  the engine's, mine was not; fixture corrected to `# set loop on`. (c)
  item 14 — `BlockSkipState` tracks the if-stack and loop pairs with line
  numbers and spans, mirroring IfSwap (push/flip/pop, ignore-and-log on
  empty, nothing counted inside a `comment` block per the :4605 guard);
  `unclosed-struct` reported at the OPENER (the root — matches Vixy's
  ruling for the engine log and the future `#!`), `end-without-if`,
  `else-without-if`, `loop-end-without-loop`; `checkBuffer` stable-sorts
  by line. §5.1/§5.2's questions ANSWERED as "structure yes, arms no".
  (d) item 15(c) — the caret was already FTXUI `inverted` (SGR 7); now an
  `inv` mask per frame pins one inverted cell at the caret in all 10
  frames; an `under` mask pins every span at its bytes. Corpus record 33 →
  24 (−24 generic on :37-46, +8 inline-comment, +7 unclosed-struct: the
  six catalogue lines :1404-1409 and :1547); 408 shipped: old-id output
  byte-identical (1757 lines, diff empty), new ids exactly the two TRUE
  `end-without-if` at panorama5.sts:102 (+ md5 twin) → SS-25; SS-24
  amended with the deterministic-tail-death argument; SS-20 noted. Grammar
  18 seeds, +3 parse_model clauses (comments.mid_line, if_structure,
  loop_structure), `_meta.amended`; families and vocabulary verified
  byte-identical against HEAD. **Incident, named per house honesty:** the
  first fixture write truncated `tests/lint_cases.sts` to 0 bytes —
  `open(p,'wb')` truncates BEFORE `.encode('latin-1')` raised on an em-dash
  in the new comment text; the loss was invisible in that script's own
  traceback and surfaced only through the NEXT edit's exact-anchor assert;
  restored from HEAD (md5 checked), re-applied encode-before-open. Second
  instance of 2026-08-04c's class (destructive step before a fallible one)
  → cross-project Q-56 (`~/shared/QUEUE.md`). Open from this slice: item
  13's retirement half (engine change), item 16 (shipped-corpus sweep,
  1757 undispositioned), item 17 (SUSPENDED for Vixy: engine-version
  targeting — C1 vs D9), the D6 column micro-decision, the CMake corpus
  note still reading "EMPTY" (item 16 owns it). Engine blockers unchanged:
  toolchain (GCC ≥ 14; Vixy's system-upgrade offer awaits the
  desktop-toolchain answer), display session for the `claude` user.
- **[2026-08-30e] The rebuild fails at the toolchain, not at RAM; C3's
  shipped half measured for the first time; two seeds grounded.** Session
  resumed cold; 2026-08-30d's `nice -j6` incremental build died with its
  session (46 fresh objects, last written 23:39, no process left).
  Relaunched → FAILS: `src/appModule/fps.cpp:33: fatal error: stacktrace:
  No such file or directory` (`<stacktrace>` added `e6fc73c5b`,
  2025-11-22) + `CMakeLists.txt:147 link_libraries(stdc++exp)` — both
  GCC ≥ 14 facilities; this laptop = GCC 11.4.0 (Ubuntu 22.04 stock,
  measured); the desktop tree's `build-claude/` cache records GCC 15.2.0.
  2026-08-30c's "configure probe CLEAN" was a FALSE GREEN — configure
  compiles no fps.cpp (verification height: the terminal observable is the
  binary, not the cache). Measured without root: jammy's `libstdc++-12-dev`
  .deb ships the `<stacktrace>` header and NO backtrace library; no
  g++-13/14 exist in stock jammy. `build.stale-cmake322-gcc11/` (CMakeCache
  2026-08-04 23:28) is this finding's earlier, unrecorded fossil. Own
  instrument slip: the background wrapper `cmd; echo EXIT=$?` reported
  exit 0 to the harness (the echo's status), `EXIT=2` in the log — read
  the log, never the notification. Vixy in-conversation: "gcc-12?" →
  answered no (facts above); "I can upgrade the system" → answered: aim
  the upgrade at the DESKTOP's toolchain (parity removes a compiler
  variable from every future measurement; GCC ≥ 14 is the hard floor; the
  NVIDIA 580 driver must survive it; the `claude`-user display session of
  2026-08-30d is a separate blocker). Engine build, item 6 and the live
  verification of the 2026-08-30b rulings stay BLOCKED on it; scedit
  itself builds on GCC 11 (8/8 re-established on `build-lovely`).
  **C3 shipped half, first measurement** (CMakeLists still carries the
  superseded "EMPTY" note): the 13 shipped seeds over the 408 shipped
  scripts → **1757 findings in 35 files** — duplicate-key 1596 (1500 in
  one generated file, `internal/comet-particles.sts`), indented-comment 98,
  unknown-parameter 51, dangling-key 6, unknown-command 5,
  silent-off-value 1. By inspection plausibly all TRUE (`lanscape`, `ofn`,
  `key_color`, `output_rate` ×9, `deselect pointer` ×27, `(warning`-class
  prose …) but UNDISPOSITIONED → queued as §5 item 16; not a gate on this
  slice (C3 binds a NEW rule to zero false positives; these are shipped
  rules meeting the shipped corpus for the first time). Data-package
  note: 43 md5-identical script pairs (`navigation/fscripts/` mirrors
  `fscripts/`) — a script fix must land twice. **Pre-scan for the two
  candidate seeds** over 408 + witness + harness (python approximation of
  the tokenizer; the checker confirms at gate time): inline `#` in KEY
  position = exactly the witness's 8 lines, 0 shipped; unclosed
  `struct if` at EOF = **7** in the witness — :1547 (recorded) AND
  :1404-1409, the six-comparison syntax catalogue, unrecorded until now;
  `struct if end` without an opener = shipped `fscripts/panorama5.sts:102`
  (+ its identical `navigation/` copy): block :96-97 opens two, :100-102
  closes three. Consequence sharper than SS-24 stated: `struct if a inf
  b` pushes skip iff a ≥ b (:4630) and `struct if a sup b` iff a ≤ b
  (:4644); one of the two holds for ANY a, b (undefined names read as 0:
  `evalDouble` → `strToDouble`, app_command_eval.cpp:116-128), so **every
  line of the witness after :1405 is skipped at runtime, deterministically**
  — the 200-line tail (:1406-1606) is dead, not conditionally dead. SS-24
  amended, SS-25 opened (SCRIPT_SURFACE). Parent-ledger routing gap noted,
  not fixed: the three 2026-08-30b engine rulings (mid-line `#`,
  unclosed-if log + `#!`, mod/div/mul aliases) have no §13.B row —
  FEATURE_REQUESTS "accepted — pending" is their only home. Slice chosen
  and stated to Vixy (no objection at write time): items 14 + 13 + 10 +
  15(c) in one sc_check touch, item 4 scoped after.
- **[2026-08-30d] The build that killed the session: -j from nproc, RAM
  never consulted.** 2026-08-30c's background build ran `-j12` (from
  `nproc`) with **9.9 GiB available** — 12 × ~1.5 GiB/gcc = 18 GiB
  requested against 15 GiB RAM + 2 GiB swap → the MACHINE FROZE
  (thrash, before the OOM killer ever fired) and Vixy force-rebooted
  [vixy, corrected in-session: not a process kill — a whole-machine
  freeze]; the Claude Code session resumed cold. Both figures had sat in the
  session's own env line for an hour — the decision step "derive -j
  from RAM" did not exist, so in-window information stayed inert (the
  attention-shedding class, [021]). Mitigations, both harness-side:
  Vixy's launch script `taskset`s the session to half the cores (nproc
  now reports 6 — measured on resume); the SessionStart env line now
  prints `safe -j = min(nproc, floor(avail/1.5))`. Findings on inspection:
  `build-claude/` is the DESKTOP's tree carried by the migration (binary
  dated 2026-08-26, 272 objects) — the laptop never lacked a built
  engine either, only the reconfigure re-dirtied it; incremental rebuild
  relaunched at `nice -j6` — measured under load: 6 × cc1plus ≈ 1.2 GiB
  each ≈ 7 GiB, 6.1 GiB still available, swap untouched (the 1.5 GiB
  planning figure holds, conservative side). Open for RUNNING the engine
  from a Claude session: the `claude` user has **no Wayland session**
  [vixy] — no display surface for Vulkan, and Wayland offers no
  cross-user sharing (no xhost equivalent) — build and `--check`
  need none; engine launch, harness `b*_run.sh`, and item 6 (TCP client
  vs a live engine) do. Vixy's proposal: log in graphically as `claude`
  and launch the session there (real GPU, real surface) — cleaner than
  a headless Xvfb+lavapipe stand-in, which would serve protocol tests
  only and no measurement.
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
