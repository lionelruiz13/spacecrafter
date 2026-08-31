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
  and GNU-style diagnostic ids added by Vixy in-session). Original triage note
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
- **Status:** accepted — engine change pending. Consequences flagged at
  record time [derived]:
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
