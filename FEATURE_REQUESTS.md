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
