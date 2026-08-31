# External feature requests

Single place for feature requests coming from outside the development
process itself (users, testers, anyone not already tracking work through
`claude/INTENT.md`). This file exists so a request made
today isn't lost before someone with context gets to triage it — it does
not imply a request will be accepted, only that it's recorded and will be
looked at.

See `INTENT.md` §13.D for how entries here connect to the internal tracker
once triaged.

## How to add a request

Append an entry at the bottom of the log below, in this form:

```
### [YYYY-MM-DD] Short title
- **From:** who's asking (name, or "anonymous" is fine)
- **Request:** what you want, in your own words — the more concrete the
  use case, the easier it is to evaluate (what were you trying to do when
  you noticed this was missing?)
- **Status:** new
```

Don't edit or remove existing entries other than to update their `Status`
field once triaged (`new` → `under consideration` / `accepted — tracked as
<INTENT.md ref>` / `declined — <reason>` / `duplicate of <entry>`).

## Log

### [2026-07-21] Fly to another star and arrive in its system
- **From:** tester (answering `USER_QUESTIONS.md` Q22, 2026-07-21 round)
- **Request:** *"We would like the possibility to select a star and get there
  by switching to the 'galaxy' mode and then to the 'stellar_system' mode
  (if existing or created from scratch like it is if there is no specific
  one)."* — i.e. a star should be a destination: select it, travel out
  through the galactic view, and arrive in a system around it, with a
  default/generated system when no authored one exists. Recorded because it
  is larger than the question that produced it (which only asked whether a
  star should have a visible surface — that half is answered and tracked
  separately).
- **Status:** accepted — tracked as `INTENT.md` §13.A **A30** (2026-07-21:
  it turned out this was already planned but never written down; the design
  is now stated — stars promote to a real system on demand, authored ones
  come from per-system files. What you asked for is in; the remaining
  decisions are internal)

### [2026-07-21] Command to save the current camera position into anchor.ini
- **From:** tester (answering `USER_QUESTIONS.md` Q5, 2026-07-21 round)
- **Request:** anchors are edited by hand and created from scripts, and
  cross-session persistence is *not* needed — but *"it could be useful to
  have a command that saves it in the anchor.ini"*, i.e. capture the
  camera where it currently is and write it out as a named anchor instead
  of hand-computing the coordinates.
- **Status:** under consideration — folded into `INTENT.md` §13.B **B4**
  (CameraAnchors port design); recorded here so it is not silently absorbed
  into that row's scope without a decision.
  **NOT DELIVERED by B4's implementation wave (2026-07-25, §11.111(j))** — and
  the reason is worth the request's own record: writing an anchor section is a
  SERIALIZATION path, and the B31 design pass has already ruled that the one
  existing "save my camera to a file" surface (`camera action save`, defective
  as shipped — §5.41) *"must be re-expressed on the session serializer rather
  than extended"* (`b31-design.md` §3.4(d)), under the §11.66(b) preserve-the-
  file writer contract. Minting a second serialization path to save nine lines
  would be exactly the duplication I2 forbids, and would pre-empt D30–D36. So
  the request is ALIVE and now has a home: it lands with B31's writer, as one
  more output format of the one serializer. Everything it needs already exists
  on the new path — the runtime anchor carries its own declaration (kind,
  parent/body, orbit or coordinates), which IS the anchor.ini section to write.

### [2026-07-29] Script-editor TUI: autocomplete, direct TCP, static analysis, inline key documentation with defaults
- **From:** Vixy (side-note carried by the `DECISIONS_PENDING.md` D31 answer,
  2026-07-26; recorded here at the §11.113 propagation pass)
- **Request:** *"a side project, writing a tui with mouse support - a script
  editor with autocomplete, direct tcp mode, static analysis to report errors
  early and showing documentation of the currently edited call + attribute key
  documentation corresppnding to the key/value the cursor is on with default
  value shown greyed out and candidate for autocomplete when the value field is
  empty"*. Not a request against spacecrafter itself — an external tool — but it
  consumes three surfaces this project owns, which is why it is recorded rather
  than left in a decision file.
- **Status:** accepted — tracked as `claude/util/scedit/INTENT.md` (2026-08-03:
  work started as **scedit**, home `util/scedit` [vixy]; slice 1 landed — the
  command surface is now a machine-readable contract
  (`util/scedit/grammar/sc-grammar.json`) with a count-gated validator; scope
  extended by Vixy to stellar-system files; zero-knowledge documentation bar
  and GNU-style diagnostic ids added by Vixy in-session).
  **STATUS 2026-08-31 — THE D31 SPEC IS COMPLETE BUT FOR ONE HALF OF ONE
  CLAUSE.** *tui with mouse support* (the FTXUI shell, click-to-warp),
  *autocomplete* (ghost text = exactly what Tab inserts), *static analysis to
  report errors early* (`--check`, 15 armed rules, C3 zero false positives),
  *documentation of the currently edited call + attribute key documentation
  corresponding to the key/value the cursor is on* (the four-line doc bar, with
  an honest blank where the contract has none) and now *direct tcp mode*
  (**F67**, scedit journal `2026-08-31h`: `--tcp`, the `$LOGON` feed pane, the
  engine's `#!` write-back handled so that neither the author's edits nor the
  engine's findings are lost without a choice, and an MCP `run_command` over the
  same client) are ALL LANDED. What is not: *"with default value shown greyed out
  and candidate for autocomplete when the value field is empty"* — the MECHANISM
  is built and arms itself from the data, but all 324 argument specs state their
  default as an English SENTENCE, so there is no literal a machine may type
  without guessing (C2 forbids guessing). It waits on a DOC PASS writing
  `default_value` as data, source-anchored, never regexed out of prose — scedit
  INTENT §5 items 11 and 12. Count of literals at HEAD: **0**; 35 of the 324
  reduce to a bare token by inspection and are the obvious first batch. An empty
  value slot with an ENUMERATED domain does complete today, from `values`; that
  half is live.
  **Consequence (2) below is discharged by measurement**: the TCP channel HAS
  become an editor-facing API and its error reporting was judged against that.
  The answer is **§11.185**, routed to Vixy — the engine tells a client nothing
  about a script it plays (no start, no end, no diagnostic; only `get`/`search`
  are ever answered), so an editor has to watch the FILE.
  Original triage note
  kept below — the three consequences became scedit constraints. Three consequences
  worth carrying into the work that touches those surfaces:
  (1) the **command grammar** and the **data-key vocabulary** become
  MACHINE-consumed, not only human-read — B38's command census
  (`capability-surface.md`) and B24/B25/B27's key inventory are the raw material,
  and §2.0 D10's documentation gate acquires a consumer that cannot infer intent
  from prose;
  (2) the **TCP channel** becomes an editor-facing API (completion/validation
  round-trips), a use case beyond running a show — its stability and error
  reporting are judged against that too;
  (3) *"default value shown greyed out"* wants **defaults to be declared data
  rather than code constants** — the same authority question §2.0 D12's
  acting-default logging raises from the other end (a default that must be
  displayed and a default that must be logged both need a single place that
  knows what it is).

### [2026-08-26] Parallel scripts — run and control more than one script at a time
- **From:** Vixy (`claude/vixy-side-ideas.txt` `[parallel-script]`, written
  with the D15 answer, harness `2b24a1b`)
- **Request:** POINTER ENTRY — **`claude/vixy-side-ideas.txt` is the authority
  for the request's own text and stays so**; this entry exists only so the idea
  is reachable from the triage channel (I2: one authority, one index). In one
  line, without restating the spec: a named-script model with an execution
  policy (`exec` = `stack` / `parallel` / `detached`), script-local resources
  bound to the name, script operations addressing the named script *and the
  ones it tracks*, and termination cascading down the tracked set.
- **Status:** new — untriaged. Three surfaces it would touch, flagged so triage
  starts from the right rows rather than from scratch [derived]: the script
  engine's play/pause/queue state, which **§11.113(o)/D36** deliberately
  classified as *time-bearing ⇒ OUT of the session file* (a multi-script world
  makes "the running show" plural, so D36's boundary is re-read against it, not
  re-opened by it); the command grammar, which is now a machine-consumed
  contract (`util/scedit/grammar/sc-grammar.json`) ⇒ new words are B28-protocol
  spellings; and **§2(b)** (user scripts are immutable live-show content), which
  bounds what a policy may do to a script already playing.

### [2026-08-26] Script-bound key/joystick bindings — override a binding to launch a script
- **From:** Vixy (`claude/vixy-side-ideas.txt` `[script-binding]`, harness
  `2b24a1b`)
- **Request:** POINTER ENTRY — text authority stays `vixy-side-ideas.txt`.
  Shape only: `script action bind on <keybind> launch <script_path>`, with the
  binding's lifetime defaulting to global and settable to script-lifetime.
- **Status:** new — untriaged. Adjacency worth carrying into triage [derived]:
  **B37** already tracks the UI-only capabilities (reachable by key or mouse and
  therefore unusable in a show); this request runs the arrow the other way —
  keys become script-reachable — so the two together decide whether the key map
  is a *surface* with one authority or two half-surfaces. A lifetime that is
  "script-lifetime" is also state a session save must classify (**B31** §2,
  declarative-in / time-bearing-out per D36).
- **Dependency added [vixy 2026-08-31, in-conversation; recorded by fable]:**
  `[parallel-script]` **is a prerequisite of this request** — *"dynamic binding
  is where it may interest him [the tester], because joystick button are
  severely limited and it's a pre-requisite"*. Structural reading [derived]: a
  bound script fires *while a show is playing*, which is the concurrent case
  the serial model never defined — binding is only meaningful once concurrent
  execution has semantics (names, policies, tracking, termination). The chain
  is therefore `[parallel-script]` → `[script-binding]` → `[script-trigger]`
  (the txt's own `@requires` covering the last link), which reorders triage:
  parallel-script is the root, not a sibling. Same statement carried the
  framing rule now in `USER_QUESTIONS_ROUND3.md`'s header (a proposal leads
  with what it enables/simplifies/removes, never with its own defense) — R25
  reframed under it the same day.

### [2026-08-26] Script triggers — run a script when a condition is met
- **From:** Vixy (`claude/vixy-side-ideas.txt` `[script-trigger]`, harness
  `2b24a1b`; the file marks it `@requires [script-binding]`)
- **Request:** POINTER ENTRY — text authority stays `vixy-side-ideas.txt`.
  Shape only: bind a script to a condition — camera attach/detach to a named
  body, crossing an altitude threshold (optionally while attached to a named
  body), and a named body's visibility class changing (halo / far / near /
  surface).
- **Status:** new — untriaged. The one fact triage should not have to
  rediscover [observed]: every condition listed already exists as engine state
  with a named owner — camera attachment is the reference/`freeMode` pair
  (`Camera`, **B18**/D15(b) territory), altitude is `ground_radius`-relative
  (**B10**, §11.71), and the visibility classes are the G4/G5 regime ladder
  whose thresholds are now px-authored with one authority (§11.127, **A41**).
  So the request is a *notification* surface over existing state, which is I3
  (the owner of state notifies dependents) rather than a new state machine —
  and that is also why it wants deciding alongside **A41/A42** (a trigger that
  fires on a regime boundary makes those constants user-visible in a new way).

### [2026-08-30] Truncated keywords execute as-if expanded when the match is unique
- **From:** Vixy (in-conversation, this session's harness commit): *"what about
  (for execution, not autocomplete) making the truncated keyword working as-if
  expanded, when it is a single-match ?"* — engine-side; intuitive command
  truncations work as the user expected, and a whole class of
  did-you-mean-shaped failures dissolves.
- **Status:** new — untriaged. Facts triage should start from, all
  [measured] against the grammar at HEAD:
  (1) **the motivating instance fails the precondition**: `mod` prefixes BOTH
  `mode` and `modulo` — ambiguous, no expansion, SS-22's line stays dead; the
  rule answers the class, not that line;
  (2) **exact match must win before expansion is attempted**, stated as a
  clause or the rule breaks working commands: two full names are prefixes of
  other commands — `body` → `body_trace`, `dso` → `dso2d`/`dso3d`;
  (3) **temporal fragility**: a unique-today prefix becomes ambiguous when a
  new command registers, so a truncation-written script can stop working on
  engine upgrade — it fails LOUD (ambiguity = error, no silent retarget),
  but it is a new way for field scripts to age (D9-adjacent);
  (4) scedit consequences: C1 obliges the checker/tokenizer to mirror the
  expansion exactly (derivable from `families.commands`, no grammar change);
  `unknown-command` refines to no-match/ambiguous with the match set in the
  message; and if the recorder (SS-12) normalizes truncations to canonical
  spellings on write-back, that interacts with the what-should-a-recording-
  contain question — same decision, one more face.
  Scope question for triage: commands only, or every machine-listed family
  (keys, flags, set names) — the mechanism generalizes, each family carries
  its own ambiguity surface.
  **[2026-08-30, same evening] DECLINED — by the requester, on fact (3):**
  *"a script which worked and stop working is worse than a script which never
  worked - it's less intuitive to the user, not more, so my suggestion's
  precondition got invalidated"* [vixy]. The idea was cached from existing
  tooling practice (unique-prefix execution) whose unstated precondition is a
  STABLE command set — invalid in an evolving language. Superseded by the
  alias entry below.

### [2026-08-30] `#!` — the engine annotates the faulty script line in place
- **From:** Vixy (in-conversation, answering the unclosed-if logging
  question): the engine MODIFIES the script — inserts a comment at the END
  of the faulty line, starting with **`#!`** so it is added only once; a
  `#!` tail is REPLACED by spacecrafter on execution when different from
  what it would emit. Purpose: *"providing error feedback at the place the
  error happened - easier for a user to debug using a casual text editor,
  who doesn't want to ever try using scedit."* For the unclosed `struct if`,
  the annotation lands on the OPENER line; message content follows the
  three-part log schema (§11.169: cause + content + self-contained action).
- **Status:** ~~accepted — engine change pending~~ **LANDED 2026-08-31 (code
  `2b8ec034`, §11.184; gate `harness/f63_annotations.py` 34/34)** for the
  RULED class — unclosed `struct if`/`struct loop` annotated at their OPENER,
  `end`/`else` without `if` and `loop end` without `loop` at their line —
  through `ScriptAnnotator` (contract in `scriptModule/script_annotator.hpp`:
  batch per file at script end, sibling temp + rename, no write when the tails
  already say this, stale tails cleared at a natural end, unwritable file →
  log only, CRLF/ISO-8859 preserved). Of the consequences below: (1) met
  earlier by `3d9179d2`; (3) IMPLEMENTED as stated (fixed ⇒ cleared at the next
  natural end); (4) IMPLEMENTED (read-only degrades to the log with the count);
  (5) confined to `addScriptFirst`'s engine-synthesised lines — a script played
  BY another keeps its own file and line (gate leg H+I); (2) the parse_model
  clause is scedit's half, with item 15(a)/(b), ~~still owed~~ **[2026-08-31
  later: (2) LANDED — `parse_model.comments.machine_tail`, code `4a00cf31`,
  with 15(a-i) (the tail recognised on the bar and RELATED to scedit's own
  finding); 15(b) holds by construction (the editor writes bytes back, never
  composes a `#!`); 15(a-ii), the error-history pane, minted as dispatch task
  F65 the same day; scedit's reading agrees with the engine on all 12 lines the
  engine annotated in F63's artifacts (`harness/f63_scedit_agree.py`)]**.
  **[2026-08-31, later — SCEDIT HALF COMPLETE (F65): item 15 closed whole. The
  error-history pane lists every `#!` line and every scedit finding with
  click-to-warp; `scedit --history` prints the same list for a machine; the
  grammar's `machine_tail` clause now states that only a line that EXECUTED can
  carry a tail. scedit still writes none. The engine half's remaining open
  decision is the one below: the generic `debug_message` channel, yours to say
  yes / no / which subset.]** **DECISION FOR VIXY,
  disclosed with its measurement**: the generic channel — every failing
  command's `debug_message` written on its line — is three lines away and NOT
  wired: it would put ~1661 `#!` tails into 35 of the 408 shipped scripts on
  their first runs (scedit corpus count 2026-08-30; 1500 in the generated
  `internal/comet-particles.sts`), i.e. §2(b)/D9 at scale. Say yes, no, or
  which subset (unknown command / unknown flag are the two that dominate).
  Consequences flagged at record time [derived]:
  (1) **ordering**: a trailing `#!` comment is only a comment if mid-line
  `#` is real — this REQUIRES the same-day mid-line-# ruling to land first
  or together, else the engine would write junk args into scripts;
  **[2026-08-31] SATISFIED: mid-line `#` landed in the tree (`3d9179d2`,
  parseCommand: a `#` outside a `"…"` run ends the command); a `#!` tail is
  a comment by that rule.**
  (2) `#!` becomes RESERVED machine-owned syntax — a third comment class
  (column-1 `#`, future mid-line `#`, machine `#!`), grammar/parse_model
  entry when it lands;
  (3) idempotent-replacement wants its completion, unstated and flagged:
  fault fixed ⇒ stale `#!` removed on next run, else fixed scripts keep
  dead annotations;
  (4) §2(b) (user scripts immutable live-show content) is deliberately
  crossed — reconciled as: the write is semantic-neutral (a comment) and
  idempotent; a read-only file must degrade to log-only, never to failure;
  (5) under `[parallel-script]`'s `inline` policy the spliced lines have no
  backing file — line provenance is the open cost Vixy named (*"could make
  it slightly hard to diagnose"* — confirmed referent: the diagnosis, not a
  reversal of the policy).
  scedit's half is tracked as scedit INTENT §5 item 15 (recognize,
  navigate, error history, caret standard).

### [2026-08-30] Short aliases for the long math commands: mod, div, mul
- **From:** Vixy (in-conversation, replacing the declined expansion idea
  above): *"adding mod as an alias for modulo became cleaner, and if not
  already there, div for divide, sub for substract, mul for multiply, if
  those exists."*
- **Status:** ~~accepted — engine change pending (registration-table addition;
  no build on this laptop to verify, so recorded not implemented)~~ **LANDED
  2026-08-31 (code `7fd5ea75`; live: harness `f62_aliases.py` 11/11 — `mod x 3`
  → 1, `div`/`mul` → 12, canonical forms beside them, no "Unrecognized"; scedit
  models the aliases with `alias_of` resolved once at load, corpus record
  `mod a 2` cleared, SS-22 resolved in-tree).** Facts
  [measured, grammar at HEAD]: `add` and `sub` are ALREADY the short forms;
  `divide`/`multiply`/`modulo` are the long three; `div`/`mul`/`mod` are all
  free as exact names — exact-match dispatch, no ambiguity mechanism, nothing
  ages when new commands register (the property that killed the expansion
  idea is absent here). Consequences: SS-22's witness line `mod a 2` becomes
  CORRECT when the alias lands; scedit grammar wants an `alias_of` field on
  the new names rather than duplicate entries (I2 — args/docs live once, on
  the canonical name).
  **[2026-08-31, later — CORRECTION, §11.183]: the "trap" below was in a map
  NOTHING READS. `m_commands_ToString` has no consumer at HEAD (grep over the
  tree; measured live: F62's recording carries `div y 2` as typed, and the one
  re-serialised command is `flag <name> toggle` → `flag <name> 0|1`, from
  `m_flags_ToString`). The canonical-first construction landed anyway — it is
  the contract for the "futur exploitation" the member was kept for — but no
  recorded spelling was ever at stake. The claim came from B38's reading of the
  map's construction and was propagated (§11.182, scedit's `alias-respelled`
  seed, this note) without anyone checking for a consumer; the seed is retired.**
  **Implementation trap, identified 2026-08-31 [fable, from parse_model
  `recording_alias_loss`]:** `m_commands_ToString` is built by
  `emplace(enum, name)` over the ALPHABETICAL name map, first name per enum
  wins — `div` < `divide`, `mod` < `modulo`, `mul` < `multiply`, so registering
  the aliases naively flips the RECORDED spelling of every `divide`/`modulo`/
  `multiply` line to the short form (the `flyto` → `camera` class, inverted).
  The recorder's canonical name must be chosen explicitly (long form), not
  inherited from map order — a one-line ordering decision, but a D9-grade
  one (recordings are shipped artefacts).

### [2026-08-31] LLM assistance over the command documentation (ollama / OpenAI-compatible)
- **From:** Vixy (in-conversation, mid-turn, verbatim): *"New idea, as feature :
  To make spacecrafter script more accessible, is there a way to integrate
  ollama/OpenAI LLM support ? It would require to interface the command
  documentation."*
- **Status:** new — untriaged **for the two decisions it needs** (below); the
  TOOL half is DELIVERED. **[2026-08-31, F66]** scedit now exposes the
  documentation and the checker as `--doc` / `--search` / `--check --json` and as
  an MCP stdio server (`scedit --mcp`, three tools over the same C++ readers) —
  so any harness with an MCP client, Claude Code included (binding measured on
  this host), has the grammar in its hands today with no model call anywhere in
  scedit. What still waits on Vixy is unchanged: which mode first, local-only
  default or not, and whether the checker's verdict is a hard gate in agent mode.
  Detail: `util/scedit/INTENT.md` journal 2026-08-31g + §5 item 19;
  `util/scedit/README.md` § For machines. Facts triage starts from [fable,
  recorded at receipt]:
  (1) **the interface it needs already exists as data**: `util/scedit/grammar/
  sc-grammar.json` is the machine-readable command contract — per-command and
  per-key one-liners written to the zero-knowledge bar (C6, "someone without any
  knowledge of script should be able to understand and modify any script"),
  value domains, defaults, required-ness, the parse model's sharp edges, and
  the lint seeds; scedit's `DocIndex` is a reader of it. An LLM needs exactly
  that: a grammar it can be given (as context or as tool definitions) and
  cannot invent. "Interface the command documentation" = expose this file, not
  write a second one.
  (2) **the red line is C2 in the other direction**: the LLM is a CONSUMER of
  grammar facts, never a SOURCE — every line it produces goes through scedit's
  `--check` (engine-fidelity tokenizer, C3-gated rules) before the engine sees
  it, and the doc bar / `#!` annotations are what it reads back. A model that
  hallucinates a key gets the same `unknown-parameter` an author does.
  (3) **modes, in order of value**: natural language → script (generate, then
  `--check`, then show — the editor's completion and doc bar already know the
  vocabulary); script → explanation (the doc lines are the material);
  NL → live command over TCP (item 6's channel) as an agent mode — the last is
  the one with a live engine at the other end and wants the checker in the
  loop as a gate, not a hint.
  (4) **placement**: outside the engine. A model call is seconds of latency and
  a network dependency; spacecrafter is a soft-realtime Vulkan process (D11).
  scedit (or a sibling tool) talking to the engine over TCP is the shape the
  architecture already has; the engine's part is the EMITTER of the grammar
  (D5's target state — the file becomes a build/runtime artefact) so the
  documentation the LLM sees is the engine's own, never a stale copy.
  (5) **what limits the quality today is the same doc debt scedit has**: 184
  of 227 family names undocumented (flags, colours, obsolete, reserved
  variables, font targets still v1 arrays), all 324 arg defaults as prose, no
  `completable` marker on values (scedit INTENT §5 items 11/12 and the doc
  passes). An LLM over thin docs answers thinly; the doc passes are the
  prerequisite, and they serve both consumers.
  (6) **provider surface**: ollama exposes an OpenAI-compatible HTTP API, so one
  client shape (base URL + model name, no key for local) covers both named
  providers and a local-first default — no network, no cost, and a dome's
  offline setting stays possible.
  Triage questions for Vixy: which mode first; local-only default or not;
  whether the checker's verdict is a hard gate on execution in agent mode
  (rec: yes — C3's zero-false-positive discipline is what makes a hard gate
  acceptable).
  **Refinement [vixy 2026-08-31, verbatim]:** *"The first test can be done
  with gemma4:4b (not sure that's enough, but this laptop, TravellingFoxDev,
  has only betwee 16 GB and 16 GiB of total RAM and 6 GiB of VRAM). For the
  LLM, two sides. The documentation-helper LLM gate which documentation page
  and command to show given what the user asked - the coding-helper LLM write
  or edit spacecrafter script (or just bind the documentation-helper as MCP -
  for instance, to claude code - which can be run with ollama launch using
  local LLM - or another harness), vibe scripting is probably the easiest
  path (it dissolves the cost to entry) for those who don't want of it.
  Creating script is already encouraged, with contest trying to bring
  engagement. Maybe this would helps further."*
  Read back [fable, same day] — two roles, and the second may be delegated:
  (1) the **documentation helper** = a ROUTER: request → the command/key page
  to show. Small-model territory, measurable as a hit rate. (2) the **coding
  helper** = writes/edits scripts — OR is not built at all: expose the
  documentation helper (plus the checker, plus the TCP line) as an **MCP
  server**, and any harness (Claude Code, Claude Code on a local model via
  ollama, another) does the "vibe scripting" with our tools in its hands.
  Structural read of (2): the MCP path is the higher-leverage one — one
  server, every harness, and the two roles collapse into tools the outside
  model calls (`doc_lookup`, `doc_search`, `check_script`, `run_command`),
  so the router LLM is only needed for the standalone scedit path. Placement
  rec, veto open: the tools' AUTHORITY stays scedit's C++ readers (Grammar,
  DocIndex, checker) — a `scedit --doc <cmd> [key]` / `--search <words>`
  JSON surface — and the MCP layer is a thin stdio JSON-RPC adapter over it,
  either inside scedit (`--mcp`, nlohmann/json is vendored, one binary for a
  dome operator to configure) or a small Python adapter shelling out;
  either way no second reader of the grammar (I2). Protocol details are
  fetched from the MCP spec at implementation time, never recalled.
  Facts measured at receipt [2026-08-31]: ollama 0.33.1 is installed and
  serving on this laptop; **the tag `gemma4:4b` does not exist in the
  registry** (`pull model manifest: file does not exist`) — present
  candidates of that size: `gemma3:4b`, `hf.co/unsloth/gemma-3-4b-it-GGUF:
  Q8_K_XL`, `llama3.2:3b`, `gemma3:1b` (about 100 models on disk in total);
  RAM 15 GiB, VRAM 6144 MiB (a 4B Q4 fits whole, a 12B does not); one query
  with a 65-command catalogue in the system prompt ≈ 0.8 s on gemma3:4b.
  **Experiment armed: `harness/f64_doc_router.py`** — the router role
  scored on 340 real (comment → command) pairs mined from
  `doc/superscript.sts` (the author's own line-above descriptions, not
  written for the test), against a no-model bag-of-words baseline on the
  same pairs; results appended below when the run lands.
  **[vixy 2026-08-31, interrupting the first run]:** *"No, not gemma3, he
  doesn't meet the low bar. Let try with gemma4:latest instead even if he is
  bigger."* — the run I had started on gemma3:4b / gemma3:1b / llama3.2:3b /
  gemma-3-4b Q8 after the `gemma4:4b` pull failed was MY substitution of the
  model, a decision that was Vixy's (which model sets the bar); stopped, its
  partial numbers void as a bar. Only the bag-of-words baseline (model-free)
  stands from it. Candidate under test: `gemma4:latest`.
  **F64 results, `gemma4:latest`** (8.0B, Q4_K_M, 9.6 GB on disk; the tag
  pulled in seconds — its blobs were already local — and ran **entirely on
  CPU**: `size_vram 0` for it AND for a 2.4 GB llama3.2:3b, so the snap ollama
  on this laptop is not using the GTX 1660 Ti at all — a machine/snap
  configuration fact, root-side, not a model fact; median 785 ms per query
  anyway, prefix cache doing the work) — one-level catalogue (65 commands +
  their one-liners), 340 witness pairs: **153/340 = 45.0% vs the model-free
  baseline 80/340 = 23.5%**. Decomposition [measured]: `date` 13/13, `audio`
  10/12, `media` 14/21, `image` 13/22, `body` 13/25, `landscape` 6/8 — the
  verb-shaped commands route; **`flag` 13/91, `set` 7/31, `struct` 1/9** —
  and the flag misses are STRUCTURAL: "Draw constellation line drawings." →
  `star_lines` (a command) where the page is the flag NAME
  `constellation_drawing`, which a command-level catalogue never shows the
  model (97 flag names, 43 set names, 46 colour names absent from the
  prompt). Second factor, question noise: 51 misses are ≤3-word comments that
  are not requests ("Usage example", "Press key K"); hit rate 26/77 on those
  vs 70/160 (44%) at 4–8 words and 57/103 (55%) beyond. Format followed:
  1 empty answer, 4 off-catalogue, in 340. **Two-level catalogue** (the 97
  flag / 43 set / 46 colour names listed as pages under their command, doc
  line where the file has one — `set_names` only today): **199/340 = 58.5%**
  (median 1.06 s, p90 1.75 s); `flag` 13 → 53/91, `set` 7 → 18/31; on
  requests longer than three words 154/263 = 58.6%. Member level (command
  AND member right) 54/124 of the pairs naming one; where the command is
  right and the member wrong, the names are near-synonyms the file does not
  yet tell apart (`orbits`/`planets_orbits`, `object_trails`/`planet_trails`,
  `atmosphere`/`atmospheric_refraction`) — the flags family is still the v1
  shape with no per-name doc, i.e. exactly scedit's doc debt (INTENT §5 items
  11/12 and the family doc passes). New confusion with the second level:
  `flag` → `color` ×14 ("Draw X" reads as either). Reading [fable]: as a
  top-1 "show this page" helper on a CPU-only 8B, gemma4:latest is USABLE
  (≈3 in 5, ≈4 in 5 on the verb-shaped commands) and not a gate; top-3
  presentation not measured yet; the cheapest known gain is the per-name doc
  pass on flags/colours, which serves scedit's bar and the router alike.
  Rows: `harness/artifacts/f64/gemma4_latest*.json` (not tracked).
  **[2026-08-31, F66 — two corrections to the run above, measured, recorded not
  re-run]** (1) the two-level catalogue showed the model 97 flag + 43 set + 46
  colour names and NOT the **10 `font` targets**: `font` names
  `families.font_targets` in the contract, and the instrument's hand-written
  `fam_of` map lists only three commands. A `font <target>` question was
  therefore unanswerable at member level in the 199/340 run — the exact defect
  class the new surface removes, since scedit reads `subfamily` from the
  contract and its catalogue lists all four families. (2) the one-level run's
  ROWS were lost: `f64_doc_router.py` names its artifact by model alone, so the
  `--limit 5` re-run that verified the script path after F66's refactor
  overwrote `gemma4_latest.json` and `summary.json` (untracked; the numbers here
  stand, `*_members.json` is intact, re-derivable in ~5 min).

**Provenance update to the three 2026-08-26 entries above [fable 2026-08-30,
owner testimony in-conversation → §11.173]:** the file's text is the
**RE-REFINED version** — what the main tester saw and objected to was an
EARLIER draft (no naming, weaker integration; unrecoverable — the owner's own
reasoning engine has been refined since, *"I can't regenerate the
lesser-quality version I used to hold"*). The two recovered objections —
global-control composition (`script action stop/pause`, resume, speedup:
target ambiguity) and *"hard to track"* — are **answered by the current
text** (default name `""` preserves legacy global-control semantics;
named-tree targeting; tracked/detached + termination cascade), with ONE named
gap: resume/speedup composition is answered structurally but not yet written
as clauses. **Consequence for triage**: `[parallel-script]` routes to the
final tester pass as a RESUBMISSION — *"your two objections, addressed —
re-evaluate"* — not as a reconsideration of a rejected idea; per §11.173's
interface model it should arrive decision-shaped with the objection→answer
mapping explicit. Two ledger hits recorded the same day, session 16:
script-local resource scoping attacks the stale-slot defect class at its root
(§5.110/§5.113 riders) and is valuable single-script, separable from
parallelism; `[script-trigger]` is the root-level answer to the poll-loop
log-storm amplifier §5.115 records at the symptom layer.

**Refinement [vixy 2026-08-30, harness `a501eb4` — the txt stays the
authority]:** `[parallel-script]` gains two policies and changes its default —
`exec` now defaults to **`legacy`** (names today's nesting behavior as a
contract: whatever `script action stop` and `struct if` do when nested, even
if inconsistent; logged-undefined-behavior when mixed with other policies),
and **`inline`** (called script's content spliced in place, untracked). The
observation that motivates them [derived, fable]: legacy nesting ALREADY IS
approximately `inline` — `ScriptMgr::addScriptFirst` splices the called
script's lines into the caller's queue (the divergent second line-classifier,
scedit journal 2026-08-04c), and `ifSwap` is one global stack so a called
script's open `if` leaks into the caller. Vixy's stated intent alongside
[in-conversation]: scope if-state to the script — leaving a script leaves its
ifs — cleaning the non-legacy policies while `legacy` preserves the old
semantics under the old syntax. History that explains the shape [vixy,
in-conversation]: *"the script system had always been thought at serial
level, but evolved with things which made the question more complicated,
without the model being refined, ever"* — features accreted on a serial
model that was never revisited; `[parallel-script]` is that first model
refinement, and `legacy` is the honest name for the accretion.

**Refinement [vixy 2026-08-31, in-conversation — the txt stays the
authority]:** the special name `*` matches every running script —
`script action <verb> name *` addresses all of them at once, so the
one-word whole-system stop survives into the named-script world
(strengthens the answer to the tester's global-control objection; carried
into `USER_QUESTIONS_ROUND3.md` R25 the same day). Same conversation, two
concrete instances for the binding half (a button cycling modes;
script-run navigation — close to the tester's existing warp-to-selected
practice) and one for the trigger half: the tester's two-appearance
bodies (distinct normal/miniature skins — the LoD misuse Vixy has warned
him is fragile, breaking e.g. far-but-zoomed), which a trigger firing on
distance/visibility plus the existing `skin_tex`/`skin_use` swap replaces
with a supported mechanism. That same testimony REFRAMES ledger row
A43's premise (the Sun/Moon preview↔full mismatch may be the tester's
authored design, not a data slip — regenerating would destroy it):
routed as round-3 question **R29**, A43's fix direction gated on its
answer; the ledger-row flip itself is the supervising session's.

### [2026-08-31] Command provenance (file/tcp + line) carried by the engine, and TCP feedback on a link dedicated to scedit
- **From:** Vixy (in-conversation, session-19 trigger line; full record §11.186(b)(c))
- **Request:** verbatim: *"spacecrafter script engine must carry the provenance
  (file/tcp + line), feedback about tcp sent back (note: an existing tcp path
  exists, used by masterput (which is closed-source), do not modify this
  channel) - and sent it back through the tcp link dedicated for scedit."*
  Two halves: (1) every command the engine executes knows where it came from —
  a script file (path + line, §11.184's existing half) or TCP — so a diagnostic
  can name its origin; (2) feedback about TCP-origin commands goes back over a
  TCP link DEDICATED to scedit (opt-in), never over the existing channel
  masterput speaks — that channel's wire-visible behaviour is FROZEN
  (closed-source client, tolerance unknowable: an unsubscribed connection must
  see byte-identical traffic, proven by a control leg). This is the
  §11.185(d)(1) ruling in specific form; (d)(2)/(d)(3) stay open.
- **Status:** accepted — tracked as §11.186(b)(c); dispatched 2026-08-31 as
  tasks F68 (provenance) and F69 (dedicated feedback link).
  **Half (1) DELIVERED 2026-08-31 → §11.187** (engine `423cbe23`): a command
  read on the control socket carries `tcp#<connection id>` and its
  diagnostics say so; the file half is §11.184's, unchanged; HTTP/mkfifo/UI
  are mapped and reported, not wired. Half (2) is F69, still open — and the frozen-wire
  requirement it must meet is now MEASURED as a baseline (an unsubscribed
  connection's bytes, pre and post, byte-identical).
