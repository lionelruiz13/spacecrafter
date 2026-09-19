# SHAPE.md - how this code must be shaped, how that is checked, and how to rebuild the understanding

Derived view, written 2026-09-19 (session 33, Claude Fable 5.1) on the owner's request that the
understanding be reconstructible by any later session or dispatched agent; tested the same day by a
cold reader given nothing else (`INTENT/11.257.md`), its gaps fixed here. The ledger wins on
divergence: rules = `INTENT.md` section 2.1 **G12 (S1)-(S9)**; derivations and the owner's words
verbatim = `INTENT/11.249.md` ... `INTENT/11.255.md`; open work = section 13.B rows **B42-B46**.
This file is the index and the operating manual; it states nothing those do not.

## 1. What happened, in five lines
- The owner wrote `src/experimentalModule` alone (reference: code `4dfe7bb3`, the parent of the first
  `Claude*` commit). Agents then grew it 5x in code and 45x in comments. He read it on 2026-09-19.
- Cause, measured (11.249): every model added without reworking (deleted/added code 0.03-0.17, his
  4.02). Tasks were accepted on BEHAVIOUR; an added function cannot break a green check, a reworked
  one can; nothing read the SHAPE, so its decay was silent. The supervisor never read it either.
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
optimal-path + general-fallback pair (`draw` / `drawLoaded`) keeps both paths.

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
| his, 22 files | 9.0 | 21 % | 107 / 2 / 4 | 43 / 86 |
| code `57102bf4`, 68 files | 9.2 | 25 % | 369 / 10 / 2 | 47 / 77 |
One line, verb first ("Return ...", "Update ...", "Draw ..."), on about one member in five; what it
carries is a unit, a precondition, a caller responsibility, an ownership or thread rule. Module
comment/code: 0.07 his, 0.67 on the morning of 2026-09-19, 0.07 at `57102bf4`.

## 5. Instruments (`harness/shape/`, run from the CODE repo root) - what each proves and what it cannot
- `review.sh` - the owner's REVIEW QUEUE: `status` / `show` / `added` / `commits` / `done [commit]` /
  `log`. The mark is the code repo's ref `refs/review/headers`; its reflog is the history of his
  reviews. Headers by default, `ALL=1` for everything. Sessions never move the mark: only he does.
- `ratio.py <rev> [v]` - module code / comment lines at a revision (trailing comments count as code).
- `authors.py [path]` - added / deleted code and comment lines per author over the history.
- `writers.py <rev> <Class> <hpp> <cpp>` - which member functions WRITE which field, same-writer
  groups, one-statement forwarders: the detector for his sense of duplication and the S7 count.
  Regex, no compiler: blind to writes through a non-member call, an alias, another class. A
  forwarder is usually the HEALTHY form (S2); intent is in the name, the tool cannot see it.
- `trio_census.py [-v]` - every call into Projector / Navigator / Observer outside the module (668
  at `57102bf4`, 124 methods): the query surface of the wiring class (B42).
- `form.py` - the table of section 4. `clones.py` - literal text clones (measures TEXT only).
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
- BEFORE WRITING CODE: S4, from a current read. Then S5 (is there a declared name or value?), S9
  (does something that looks redundant have a reason?), S8 (which mode is responsible?).
- A COMMENT-ONLY PASS: `strip_prose.py` or hand edits -> `codeident.py <rev>` 0 FAIL, run by the
  supervisor and not only by the agent -> build -> `f70_ascii.py gate` -> commit. An intended FAIL
  (restoring his wording on a line an agent had extended) is named in the commit message.
- A DEDUPLICATION COMMIT: one information per commit; `writers.py` (or the manager list) before and
  after in the message; net lines negative; the harness gate that covers it named, or "not reached".
- ACCEPTING A DELIVERY: behaviour evidence AND shape - net code lines, deleted/added, comment form,
  manager count. A delivery that only adds is a finding to explain, not a default.
- AT EVERY SESSION CLOSE: `review.sh status` in the report to him (what waits, shape at both ends);
  its last line counts the source files changed OUTSIDE the header scope - when it is not zero, say
  which `.cpp` carry the change (`ALL=1 review.sh status` lists them).
- EXECUTORS: Fable only (owner, 2026-09-19): agent type `claude`, model `fable`. Disjoint file sets
  may run in parallel in one tree if none commits, builds or stashes; the supervisor verifies,
  builds and commits. Every task prompt binds G12 and names `spacecrafter.owner-shape` as the form.

## 7. State at the close of session 33 (code `a594b31a`, not pushed)
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
No task section is minted yet in `fable-dispatch.md` for B42-B46: mint it, with its PREMISES block,
before any dispatch (section 0b step 2). Sessions COMMIT in their own name and NEVER PUSH.

## 8. Terms a cold reader will meet
- **Vixy = the owner = Calvin Ruiz** (git author); `[vixy: date]` tags his words in the ledger.
- **The two paths and how the running one is decided**: old path = `src/bodyModule` + Navigator /
  Observer / Projector; new path = `src/experimentalModule` + `Camera`. The MODE is
  `SSystemFactory::drawModularSystem` (`ssystem_factory.hpp`: `setRenderPathMode`, `pathPinned`,
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
`CLAUDE.md` (auto-loaded) -> this file -> `INTENT.md` G12 and rows B42-B46 -> the entry files
11.249-11.255 for whatever the task touches -> `s33-second-managers.md` for the work map ->
`harness/shape/review.sh status` to see what the owner has not read yet.
