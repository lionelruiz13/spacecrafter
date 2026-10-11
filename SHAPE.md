# SHAPE.md - how this code must be shaped, how that is checked, and how to rebuild the understanding

Derived view, written 2026-09-19 (session 33, Claude Fable 5.1) on the owner's request that the
understanding be reconstructible by any later session or dispatched agent; tested the same day by a
cold reader given nothing else (`INTENT/11.257.md`), its gaps fixed here. The ledger wins on
divergence: rules = `INTENT.md` section 2.1 **G12 (S1)-(S11)**; derivations and the owner's words
verbatim = `INTENT/11.249.md` ... `INTENT/11.255.md`; open work = section 13.B rows **B42-B46**.
This file is the index and the operating manual; it states nothing those do not.

## 1. What happened, in five lines
- The owner wrote `src/experimentalModule` alone (reference: code `4dfe7bb3`, the parent of the first
  `Claude*` commit). Agents then grew it 5x in code and 45x in comments. He read it on 2026-09-19.
- Cause (11.249, dated and corrected by 11.265): every model added beside the structure - Fable 5
  wrote 49 % of it, mostly with the owner present; 98 % was in place by 2026-08-09. Tasks were
  accepted on BEHAVIOUR; an added function cannot break a green check, a reworked one can; and the
  code's SHAPE was no layer's input: those positioned to see it read a text ABOUT the code (report,
  ledger entry, comment), never the code - 82 deliveries accepted with 18 Reads under `src/`.
  (11.249's "deleted/added 0.03-0.17 against his 4.02" was wrong twice: it counted the ledger
  files that once lived in the module - 4.02 is one file-move commit - and del/add does not
  separate his new-code work from ours. Writer count, comment form and volume do.)
- The duplication was not text (literal clones 3 %): it was SECOND MANAGERS - the new Camera
  mirroring the old navigator / observer / projector member by member, laws ported beside the
  still-compiled originals, parallel constants built beside the ones he had declared.
- The prose was the ledger's conventions (append-only, provenance, measurements) leaking into code.
- His review time is the scarcest resource and its cost is the DISTANCE between the informations he
  needs: code volume, comments and spacing are all that distance. That is why G12 is a goal.

## 2. The rules (index - the text is G12 in `INTENT.md`)
S1 duplication = same EFFECT, not same text · S2 one implementation per effect, one entry point per
INTENT (two intents over one law stay two functions, one forwards; an alias goes) · S3 a raw
primitive + ONE composed operation; tell, never ask · S4 never implement directly: every method
NAME against the intent -> the functions manipulating the members concerned -> read those; then use
/ extend minimally / build; cross-boundary duplication becomes a class unless it costs more than it
saves · S5 his declared names and values ARE the ruling · S6 a comment is a what-for, one line, only
where the name does not carry it; no history, tags, measurements or ledger references in code ·
S7 manager count of each touched information, before and after · S8 one responsible per MODE
(old path = Projector / Navigator / Observer, new path = Camera - no third status), the gating in a
WIRING class of its own · S9 every duplication has a reason: recover it before touching it; an
optimal-path + general-fallback pair (`draw` / `drawLoaded`) keeps both paths · S10 the ORIGIN of the need
first, before S4: what it will end up covering, read from the shape of the space of use; the supervisor's step.
S11 a check either fails LOUD or does not exist, and a failed check's information is routed to the RESPONSIBLE of
it in the form decided at its anchor (owner-ruled necessary 2026-09-23, 11.289): a validate-and-continue, a swallowing
catch, an unlogged default is a silent catch that breaks the propagation his acceptance relies on.

## 3. The owner, as far as it bears on the code (11.249(g), 11.250, 11.255; cross-project Q-86, Q-87)
- He compares everything by EFFECT; words are a projection at his boundary. Resolve his words by
  the STRUCTURAL reading unless he writes "verbatim" (owner-ratified). Copy his words verbatim,
  interpret them structurally; test a binding before building an instrument on it.
- He reads code for its shape, by names first. A header that grows makes his exists-already pass
  dearer, it gets skipped, and the next addition lands beside the structure: growth that feeds itself.
- Every piece of his code has a reason for landing. A question whose answer is already in his
  header is not a question: consume the header.

## 4. His form, measured (11.255(e)(f)) - re-measure with `harness/shape/form.py`
Reference checkout, read-only, module only: `/home/claude/spacecrafter.owner-shape` (git worktree of
the code repo at `4dfe7bb3`; recreate: `git worktree add --detach --no-checkout <dir> 4dfe7bb3`, then
`git sparse-checkout set --no-cone src/experimentalModule/` and `git checkout 4dfe7bb3` inside it).
| headers | code lines per full-line comment | functions commented | blocks of 1 / 2 / 3+ lines | length median / p90 |
|---|---|---|---|---|
| his, 22 files | 9.1 | 21 % | 106 / 2 / 4 | 43 / 86 |
| code `57102bf4`, 68 files | 9.2 | 25 % | 368 / 10 / 2 | 47 / 77 |
| his LIBRARY, `src/EntityCore` at `c9bef5a` (11.279(e)), 52 files | 5.2 | 37 % | 328 / 33 / 52 | 44 / 92 |
One line, verb first ("Return ...", "Update ...", "Draw ..."), on about one member in five; what it
carries is a unit, a precondition, a caller responsibility, an ownership or thread rule. Module
comment/code: 0.07 his, 0.67 on the morning of 2026-09-19, 0.07 at `57102bf4`.
HIS FORM IS TWO STATISTICS WITH TWO DRIVERS (11.279(e), 11.280(e), three corpora): the SHARE of
functions that get a comment follows who reads the header (39-41 % on the library other codebases
consume, 21-25 % on what he alone consumes); the DENSITY peaks at the first library he shipped to
others and falls since (1 per ~16 in 2018, his first program - 3.6 in 2021 - 4.8-4.9 in 2023 - 9.0 in
2026; 11.281(f)) - a learning gradient in form after the peak, so the module row above is his CURRENT form. The
library carries class-top blocks of 6+ lines wherever an invariant the declarations cannot carry must
be stated (18 in 52 headers). A task touching `src/EntityCore` writes in the library's row. Every row
is ONE-line, verb-first comments for the most part, median 43 characters in every corpus and year;
the difference is how many members get one, never the form of each.
[Re-measured 2026-10-11, F150 §11.321 (F3): `form.py` read every `*`-leading CODE line as a comment - 34 `*ptr = ...`
lines of the library's inline code; its row moved 4.8 / 39 % -> 5.2 / 37 %, his row 9.0 -> 9.1 and the module's 369 -> 368
blocks by one line each; the other corpora of 11.279-11.281 ("39-41 %", "4.8-4.9 in 2023", 2018 / 2021) are NOT re-measured:
the gradient stands as a direction, its numbers as pre-fix.]

## 5. Instruments (`harness/shape/`, run from the CODE repo root) - what each proves and what it cannot
- `review.sh` - the owner's REVIEW QUEUE: `status` / `show` / `added` / `commits` / `done [commit]` /
  `log`. The mark is the code repo's ref `refs/review/headers`; its reflog is the history of his
  reviews. Headers by default, `ALL=1` for everything. Sessions never move the mark: only he does.
- `delivery.py <base> [<head>|WORKTREE] [--class C:hpp:cpp]` - the SHAPE block an acceptance and an executor's shape
  section paste, composed from ratio.py's classify, names.py, writers.py, form.py's measure, review.sh's scope line and
  the D14 gate; `--self-test` replays the recorded deliveries of INTENT 11.316 / 11.317 from quotes of the records (11.321).
- `decomment.py` - the ONE C++ comment scanner (source order: literals, `//`, `/* */`) names.py, writers.py and
  codeident.py read through since 2026-10-11; before it a `//*` comment or a `'"'` char literal swallowed code
  (`ssystem_factory.hpp` 78 of 162 names, `AppCommandInterface` 5 of 91 member functions): counts recorded before
  11.321 are undercounts for those files.
- `ratio.py <rev> [v]` - module code / comment lines at a revision (trailing comments count as code).
- `authors.py [path]` - added / deleted code and comment lines per author over the history. PITFALL,
  paid for (11.265(d)): it counts EVERY file that ever lived under the path; give it C++ pathspecs
  (`':(glob)src/experimentalModule/**/*.cpp'` and `*.hpp`), and read del/add only on tasks that
  MODIFY existing code. `harness/f52_strata.py` (11.168) already had the exclusion and both confounds.
- `timeline.py [--by week|day|commit] [--top N] [paths]` - WHEN the shape moved, per period and author.
- `writers.py <rev|WORKTREE> <Class> <hpp> <cpp>` - which member functions WRITE which field, same-writer
  groups, one-statement forwarders: the detector for his sense of duplication and the S7 count.
  Regex, no compiler: blind to writes through a non-member call, an alias, another class. A
  forwarder is usually the HEALTHY form (S2); intent is in the name, the tool cannot see it.
- `trio_census.py [-v]` - every call into Projector / Navigator / Observer outside the module (668
  at `57102bf4`, 124 methods): the query surface of the wiring class (B42).
- `form.py` - the table of section 4. `clones.py` - literal text clones (measures TEXT only).
- `supersede.py <repo> <ref> <author-re> [--since D]` / `supersede2.py ... [--show K]` - the owner's ITERATION
  signature (11.291(j)): the first blames the lines commit N+1 removes to their origin (pairs where the successor
  supersedes its predecessor), the second measures the share of commit N's added lines the next commit on the
  file removes. Line-blame counts a moved law as killed: the numbers are UPPER bounds; on a history with merges
  or cherry-picks the dt column is noisy (use the aggregates).
- `names.py [--rev <rev>] <header>...` - the S4 name pass INJECTED (11.265(g)5): every function-like name a
  header declares, one line per header, pasted into a code task's PREMISES block by
  `harness/premise_check.py --fill <commands file>` (the mint's writer: an expectation is never typed). Regex:
  blind to macro-made names; overloads count once; never a substitute for reading the header.
- `strip_prose.py <rev> <files>` - deletes agent-written comment runs of >= MIN (3) lines.
  `codeident.py <rev> [files]` - proof of a comment-only change: code tokens and preprocessor
  lines identical, every comment line a human wrote kept in order. PITFALLS, each one paid for:
  `git blame` names the LAST TOUCHER (the ASCII pass `cb521cf1` re-touched legacy comments) and
  other humans wrote here too -> agent-written = `Claude*` author AND absent at `4dfe7bb3`; a
  verifier that shares the tool's classification rule verifies its error - give it an independent
  one; pass `EXEMPT=<commits>` so a rerun does not delete freshly written one-to-three-line
  contracts; a TODO or the words "fall through" inside prose used to shield a whole block; CRLF
  files must stay CRLF; read the OUTPUT distribution before trusting a green run.
- Environment: `harness/artifacts/` is git-ignored (put records elsewhere); `git stash` is a stack
  shared with the owner (copy files aside instead); the synthetic-key channel (xkey) delivers
  nothing on this host since the 26.04 upgrade - drive inputs through gdb (the `b21` pattern).

## 6. Procedures
- AT A MINT, BEFORE ANYTHING ELSE (S10, owner-ruled 2026-09-21): the need's ORIGIN (INTENT section 2: roots R1 / R2, consequence (g); 11.272, 11.273). Derive it from the
  SHAPE of the space of use - one operator, a public he adapts to, only predefined inputs in session, authoring off session -
  and from what is known of such places; write what the need will END UP COVERING; place for that coverage by the
  responsibility anchor. The owner is asked what the shape cannot answer, never what it can.
- BEFORE WRITING CODE: S4, from a current read. Then S5 (is there a declared name or value?), S9
  (does something that looks redundant have a reason?), S8 (which mode is responsible?).
- A COMMENT-ONLY PASS: `strip_prose.py` or hand edits -> `codeident.py <rev>` 0 FAIL, run by the
  supervisor and not only by the agent -> build -> `f70_ascii.py gate` -> commit. An intended FAIL
  (restoring his wording on a line an agent had extended) is named in the commit message.
- WHEN A FUNCTION'S BODY CHANGES (yours or the owner's): enumerate its CALLERS - they are the affected
  space, and no compiler error names them; for each line of execution name the gate that reaches it or
  write "not reached" (11.258: a primitive that silently carried a composition broke three callers).
  And for each FIELD whose write changes: every other function that reads or writes it (`writers.py` lists
  them), for the contract they assume - read with context, never `-U0` (11.276: an index taken N at a time
  broke a ring whose decoder protects exactly one slot).
- A DEDUPLICATION COMMIT: one STRUCTURAL change per commit [vixy 2026-09-22, 11.282(e): "one structural
  change per commit seems to fit my practice better" - small fixes and additions may ride along, a second
  structural change never does]; `writers.py` (or the manager list) before and
  after in the message; net lines negative; the harness gate that covers it named, or "not reached".
- ACCEPTING A DELIVERY: behaviour evidence AND shape - the block of `harness/shape/delivery.py <base> <head>`
  (`WORKTREE` before the commit) PASTED, never re-typed: net code lines, deleted/added, comment form, manager
  count, the ledger grep, D14, the scope. A delivery that only adds is a finding to explain, not a default.
- AT EVERY SESSION CLOSE: `review.sh status` in the report to him (what waits, shape at both ends);
  its last line counts the source files changed OUTSIDE the header scope - when it is not zero, say
  which `.cpp` carry the change (`ALL=1 review.sh status` lists them).
- EXECUTORS (owner, 2026-09-27, INTENT 11.293; "Fable only" from 2026-09-19 to 09-26): STRUCTURE = agent
  type `claude`, model `fable` (the S4 pass, the predicted shape, a PREDICTIONS block, no code written);
  IMPLEMENTATION = agent type `opus-xhigh` (Claude Opus 5.5), bound to report EVERY conflict with the
  predictions - the conflict is the signal of what was not as predicted, it drives the rework. Disjoint
  file sets may run in parallel in one tree if none commits, builds or stashes; an implementer alone in
  the tree may build; the supervisor verifies, builds and commits. Every task prompt binds G12 and names
  `spacecrafter.owner-shape` as the form. ROUTING (owner, 2026-10-10, INTENT 11.302; veto-open): by the
  task's STRUCTURAL-COUPLING LOAD at the mint, never by its label - FOCUSED (input set closed and named,
  one concern, criteria that can fail) -> `opus-xhigh`, under the owner's system prompt since that day
  (`claude/agents/deploy.sh`); OPEN-COUPLING (a family's S4 pass, a shape to choose, the owner to model,
  every mint and acceptance) -> `claude` / `fable`; the split above is the common case
  (`fable-dispatch.md` section 0b (a) has the rule and the record it was tested on).

## 7. State at the close of session 33 (code branch `main` @ `6c3d0ed6`; `a594b31a` and below pushed by the owner, `6c3d0ed6` not pushed)
The code branch is `main` since 2026-09-19 (owner; `master-beta` stays where it stood). The reference-change fix of
11.258 is ON `main` (`6c3d0ed6`, gate `harness/s33_refchange.py` ALL PASS on the main-tree binary). Both trees and
`~/shared` are shared between OS users `claude` and `foxy` through group `claude`: group-writable, setgid directories,
`core.sharedRepository=group` (11.260) - if a file is not writable, fix the cause and report it, never work around it.
After any history rewrite by the owner (`supervised-by.sh`): re-read HEADs, resolve old shas through `sha-maps/`.
Done: compiles again at the five seams of his `8ce65acc` (`78455b88`); agent prose out (`3c5952ad`,
`b57d0642`); header what-for lines back and thinned to his form (`a49df0f1`, `57102bf4`).
Work map: `s33-second-managers.md` (about 280 second managers, raw) + `INTENT/11.254.md` (four
classes, the wiring class's request table). NOT targets: laws ported beside a still-compiled
original (they leave with the old path); `draw` / `drawLoaded` (his optimization pair).
Acting defaults in force, his to strike (11.250(d), 11.251): the two un-named regime gates are his
baseline literals 0.004 / 0.2; movers take `velocityScaling(1)`; interactive zoom feeds
`setHalfFov(x, 0)`; `setMountMode` informs both engines - the last two are stopgaps until B42.
Not verified: zoom feed; regime boundaries at his 2 / 16 / 256 px; gates predicted red are listed
in 11.251(b). Only `b21_keypath_run.sh` was run on the rebuilt binary (ALL PASS).
Next, in order, and why: **B45** instruments first (nothing run so far reaches the movers, the zoom
or the regime boundaries; a structural change needs its instruments before it) -> **B42** wiring
class (the request + query table goes to him BEFORE building) -> **B44** movers (its callers are
the dual-writer seams B42 reshapes; four points are NOT ruled: `DECISIONS_PENDING.md` section 14)
-> **B43** duplication inside the new path -> **B46** shape numbers assembled by a script.
Sessions COMMIT in their own name and NEVER PUSH.

### 7b. After session 34 (2026-09-20; code `main` @ `6c3d0ed6` UNCHANGED - record `INTENT/11.264.md`)
- **B45 has its instruments.** Item (1): `harness/f124_run.sh` (generic FIFO runner) + `f124_legs.py`, legs Z / L /
  A / M / D on both pinned paths, a 60-row coverage map in `INTENT/11.261.md`. Item (2): the predicted-red gates
  run and classified (`INTENT/11.263.md`); `f125_constants.py` is where a gate reads his constants from; NEVER
  paste an observed value back as an expectation. NOT built: leg S (`Core::findAndSelect`), the VIEW path.
- **B42: the page is with the owner - `claude/b42-wiring-table.md`. NOTHING IS BUILT until he answers it.**
  THE FACT TO CARRY: at HEAD the SKY is still drawn through Projector / Navigator in BOTH modes, ungated
  (`src/executorModule/solarSystemModule.cpp`); "inform both" is what keeps it right on the new path. His
  sentence (*"the Projector/Navigator/Observer are old path, the Camera is the new path"*) is the TARGET, not
  a description of the code - session 33 wrote it down as a fact. So: QUERIES BEFORE REQUESTS, frame matrices first.
- **What the instruments found** (rows 5.162-5.172, nothing fixed): the anchored descent goes under the datum
  (his floor applied inward, 5.163); one key tap on a timed zoom leaves sky and bodies 23 deg of fov apart at
  rest (5.166, the stopgap feed); a stale base and an unclamped pole in the lateral movers; the mount readout
  reports the navigator in both modes (5.170); a radius command deferred while the body is not `visible` (5.162).
- **Instruments that compare both engines inside ONE run** (F124's legs, `f125_fovlaw.py`, `f23`, `f18_disc`)
  hold only while both engines are informed: when B42 lands they become run-against-run across two pinned launches.
- DISPATCHING IN PARALLEL, learned the hard way: one writer per shared file (agents write only their own entry
  file, checkpoint state in its first lines); assert the ALLOCATED entry number, not the next free one; do not
  edit a tracked file after telling an agent the tree is clean; a boundary must not forbid what the mandate asks
  (that is how a fourth runner came to exist); re-read the prompt's reading order against the mandate's own order.
- Next without the owner, harness only: leg S and the VIEW legs; retire the three older FIFO runners onto
  `f124_run.sh` (and make a crashed gate print FAIL, 5.172); the dig of 5.162; B46's assembling script.

### 7c. After session 35 (2026-09-21; the post-mortem - record `INTENT/11.265.md`; code UNCHANGED)
- WHY IT SURVIVED: shape was no check's input; the checks placed to see it read the checked party's
  own account. So: accept
  from `git show` READ IN FULL, answer a question about code by opening the code, and ask a
  measurement what it says about US before scoring it (11.168 measured this failure and scored PASS).
- BEFORE BUILDING AN INSTRUMENT, search `harness/` and the ledger for one: S4 applies to us too.
- Proposed and NOT built (his to rule, 11.265(g)): a code-repo hook rejecting ledger grammar and
  2+-line comment runs in `src/`; RATCHETS (trio calls from the new path, (field, writer) pairs per
  class: a count may only go down, an increase needs a `Shape-Exception:` trailer); the method-name
  list of each touched class pasted into the task's PREMISES block by script.
- The asi-harness would NOT have helped, as built or as specified (11.265(h)): it accepts on the
  report too, and its provenance laws do not scope code out. Its hook layer is what could enforce
  "no acceptance without Reads of every changed hunk" and "no Edit before the header is read".
- Transcripts are deleted after 30 days by the CLI. Copies: `/home/claude/transcript-preserve/`;
  tools to read them: `transcript-preserve/work/tx_*.py` (`TX_ROOT` picks the store). July IS recovered
  (11.265(k)): three stores cover the first prompt to today.
- WHY SKY AND BODIES CAN DISAGREE (11.265(l)): in new mode ONE frame is drawn by TWO observers - the
  stars, grids, lines and constellations through Projector / Navigator / Observer (ungated), the
  bodies, backdrop, atmosphere and landscape through the Camera - each with its own place, fov and
  easing law, commands forwarded to both. His baseline already forwards (`coreLink.hpp`: zoomTo,
  lookAt, lat / lon / alt; 30 call sites then, 72 now): a scaffold of the bodies-first draft, never
  bound per mode, and our one-run parity instruments depend on it. Synchronising two easing laws
  never makes them one; the binding is B42, QUERIES first. S9 applies before EXTENDING a pattern.
- HIS INTENT FOR THE FRAME (11.265(m), row B47): what depends on WHERE we are - stars, milky way,
  landscape, atmosphere - is an `EnvironmentModule` member of the body or system it comes from, drawn
  by the reference-to-root chain with its OWNER's matrix; the executor modes dissolve into that
  nesting. Built: three wrapper members (milky way, atmosphere, landscape), called from the old mode
  module. NOT built: the STARS and the rest of the sky - and no row held them until B47. Nothing is
  dispatched on B47 before he rules who owns the STARS and its order against B42. RULED (11.265(n)):
  a member is oriented regarding its OWNER (milky way = the galaxy's inside view, landscape = the
  body's landed view). `EnvironmentManager` hands an ancestor the SYSTEM's matrix: right only while
  no system is tilted against its parent - and the capability EXISTS (`ModularSystem` inherits `re`,
  `dispatchUpdate` applies it): three `.re = {}` literals in `ssystem_factory.cpp` keep it zero.
  THE WORK MAP IS `claude/b47-sky-members.md` (11.266): 25 layers, three classes, nine questions his.
  STARS RULED (11.265(o)): the Solar system owns the 2D stars, the galaxy the 3D stars, the universe
  tully / what legacy inUniverse draws. The chain still needs a SUPERSESSION rule (nearest owner of
  a family draws; ancestors silent) - the old modes gave it for free. His to rule, with the rest.

### 7d. After the dispatch round of 2026-09-21 (record `INTENT/11.270.md`; code `main` @ `0bc9458d`, NOT pushed)
- FIRST CODE CHANGE MADE UNDER 11.265's LESSONS, one commit: `0bc9458d` - a member owned by a body above the current
  system receives its OWNER's matrix (B47 slice 1, his ruling 11.265(n)); code +11 -8, one header comment line made
  true again; identical while every `re` is zero, the tilt precondition GONE. Entry `INTENT/11.268.md`.
- THE IDENTITY INSTRUMENT EXISTS FOR ONE LAYER: `bash harness/f128_leg.sh SOL|GAL|TILT [--expect owner|system]`
  (`INTENT/11.267.md`): layer ON minus OFF per pin, gate = 4 x the MEASURED same-pin floor, stored references for
  before / after a code change (never `--rebank` to clear a red). SOL's cross-pin check is RED AT HEAD by 0.005283
  (5.175, a 0.03 deg difference between the two observers): judge a change on the two `*_vs_stored_*` lines. OWED, one
  line: SOL's able phase must tilt the GALAXY now (the system tilt no longer perturbs, so it prints BLIND).
- SLICE 2 STOPPED, MEASURED (`INTENT/11.269.md`): ABOVE A SYSTEM THE TWO OBSERVERS STAND 139.64 deg APART (5.178),
  invisible today because the Navigator draws the whole sky there in both modes. NO sky layer moves to the Camera
  above the system before he rules who places the observer there (page `b47-sky-members.md`, question 10). And no
  Camera-positioned layer is ever BIT-identical to a Navigator-positioned one: cross-pin checks are bounded ones.
- HOW THE ROUND WAS RUN, and what held: the mint is a code-design act - the supervisor reads the code AND THE RECORDS
  the task stands on (this round's defect: slice 2 was minted without opening F128's stored dumps, which already held
  both observers' matrices; the executor found the STOP in them in ten minutes); the method-name lists of every class
  a task may touch are pasted into its PREMISES block by script (`harness/shape/names.py` through
  `harness/premise_check.py --fill`), and the executor still reads the headers in full; the S4 pass and the prediction
  are written in the entry with the tree clean BEFORE the first Edit (clock of the write against the mtime of the
  source); acceptance = `git diff` read in full + the header re-read + shape numbers + a rebuild + every leg re-run by
  the supervisor; a STOP rule about one's OWN re-runs is written BEFORE they start; an S7 count is taken per
  INFORMATION and also per FRAME (how many observers draw this level's sky).
- LAUNCHES UNDER gdb: 1 of about 90 lost a capture (5.177, unattributed): count it at every TILT run.

## 8. Terms a cold reader will meet
- **Vixy = the owner = Calvin Ruiz** (git author); `[vixy: date]` tags his words in the ledger.
- **The two paths and how the running one is decided**: old path = `src/bodyModule` + Navigator /
  Observer / Projector; new path = `src/experimentalModule` + `Camera`. The MODE is
  `SSystemFactory::drawModularSystem` (`src/bodyModule/ssystem_factory.hpp` - a seam class living in
  the old path's directory; resolve any bare filename with `git ls-files '*name'`, never compose its
  directory: `solarSystemModule.cpp` is under `src/executorModule/`): `setRenderPathMode`, `pathPinned`,
  read through `getExperimentalPath()` / `Core::getExperimentalPath()`); default = new path, pinned;
  `flag experimental_path on|off` pins one; `~/.spacecrafter/beta_features.ini` `[dual_path]
  render_path = alternate` makes it alternate (harness README, top).
- **Manager / second manager**: whoever holds or writes an information; a discrepancy between two
  elements that CAN exist means a second manager (INTENT 11.247, `~/shared/QUEUE/Q-85.md`).
- **RA = responsibility anchor**: the place where the necessary information is, so where a thing is
  managed (`~/shared/asi-harness/requirements/RA-MODEL.md`). **I1-I9**: the owner's programming
  invariants (interface = what-for, single authority, push not poll, tell don't ask, non-owning
  references, rework at the earliest, as-if rule, no second artifact, enumerate at the anchor) -
  they arrive in the session's system prompt; origin `~/shared/others/Principes.txt`.
- **Seam**: a function in `Core` / `CoreLink` / `SSystemFactory` that drives both paths.
- **Q-nn**: cross-project queue, `~/shared/QUEUE.md` (heads) and `~/shared/QUEUE/Q-nn.md`.
- **xkey**: the harness's synthetic-key injector (dead on this host); **the b21 pattern**:
  `harness/b21_keypath_run.sh` - the app under gdb on a FIFO, the driver calls the functions the
  input path calls. **F<n> / B<n> / §5.n / §11.n**: task, open-item, defect and journal ids of the ledger.

## 9. Reading order for a cold session
`CLAUDE.md` (auto-loaded) -> this file -> `INTENT.md` section 2 (the two roots, consequences (a)-(g)), then G12 and rows B42-B46 -> the entry files
11.249-11.255 for whatever the task touches -> `s33-second-managers.md` for the work map ->
`harness/shape/review.sh status` to see what the owner has not read yet.
