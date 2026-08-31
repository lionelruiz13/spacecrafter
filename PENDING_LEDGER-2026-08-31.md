# PENDING_LEDGER — 2026-08-31 (round-3 compilation session, Claude Fable 5)

**Contract** (per the §11.116 precedent, `PENDING_LEDGER-2026-07-30.md`):
this is a holding buffer written while the F68/F69 executor session is
live on this tree — INTENT.md and its entry files are that session's to
touch. The supervising session MERGES this into the ledger at a
quiescent point and DELETES this file in the merge commit. Nothing here
is new work; it is in-conversation Vixy testimony + its derived
consequences, received by the round-3 compilation session (commits
`626c4b1` → `59052d1` → `ef6adcc` → `7285dfc` → `0b5378a` → `3d95c82` →
this one), recorded before they can go stale. Every [derived] carries
its veto point.

## 1. Routing directives [vixy 2026-08-31, in-conversation]

*"Parts of the last part shall land to him, in case it hold defects
(heading is not used by many)"* + *"Implicitly resolvable where
astronomical grounding resolves it, otherwise route to him."* — the
round-3 "Not included" section's routing filter, partially exercised by
the owner. Enacted in `USER_QUESTIONS_ROUND3.md` (still DRAFT, NOT
SENT): §11.4(2) → R27, §11.92(d) → R28, NEW R29 (see §3). The final-pass
list (ledger-owned per §11.116(c)) grows by these three; the ledger's
copy of the list should be updated at merge.

## 2. §11.4's two decisions — one resolved, one routed

- **(1) RA zero point: RESOLVED by astronomical definition** [derived,
  under the owner's explicit delegation above; veto point: if the
  readout was ever meant as something other than standard RA]. RA = 0 at
  the vernal equinox is definitional — certainty class (i),
  dependency-free, no convention freedom in any catalog or epoch frame.
  The new path's constant −90.0003° offset (§11.158(f)) is therefore a
  frame-construction defect (RA measured from an axis 90° away), not a
  choice. Root the correcting gate at CITED CATALOG values of known
  bright stars (§11.51(d) red line: never from recall), not at old-path
  parity alone (verification height: the terminal observable is
  catalog agreement; old is cross-check). The ~1 arcsec residual beyond
  the 90° must be ATTRIBUTED during the fix, never absorbed (residuals
  are axes).
- **(2) origin (observer- vs body-centred): routed to the tester = R27**,
  framed as topocentric (pointing number) vs geocentric (catalog
  number). Cross-check recorded: the ledger's measured "~1° on the
  Moon, invisible elsewhere" IS the Moon's parallax (~57′) — confirming
  the two halves separate exactly on the definitional/conventional
  line. Old = observer-centred = inherited, no recallable intent
  (§11.161(d)).

## 3. A43's premise REFRAMED by owner testimony — fix gated on R29

[vixy 2026-08-31]: the main tester deliberately authors **distinct
normal/miniature skins** to give a body two appearances (far vs close)
— a misuse of the LoD/miniature system (*"intended for miniature, not
for redesign"*), unreliable (keys on apparent size: far-but-zoomed
shows the wrong face), and Vixy has already warned him it can break at
any time. Consequences:
- **A43** ("Sun/Moon previews don't match their full maps — regenerate")
  assumed the mismatch is a slip. The tester is the data's principal
  author (§11.161(c1)) ⇒ the mismatch may be his DESIGN; regenerating
  would destroy it. **A43's fix direction is now GATED on round-3 R29**
  (slip-or-design, per body). A42's swap-distance answer feeds from the
  same answer. Row annotations owed at A43/A42.
- **[script-trigger]'s use case materialized**: a trigger on
  distance/visibility change + the existing `body … skin_tex`/`skin_use`
  swap (grammar-verified present) is the supported replacement for the
  hack. Recorded at FEATURE_REQUESTS (commit `3d95c82`) with the `*`
  wildcard refinement (special name `*` matches every running script)
  and the [parallel-script] prerequisite chain (commit `59052d1`).

## 4. §11.92(d)/B17 — premise corrected, old-as-spec weakened, a lead

[vixy 2026-08-31]: *"His dome doesn't use it, but someone else's dome
did and… there were already some long-standing fixes about it at some
point, not sure it's really clean now though."*
- **Premise correction**: the main tester's OWN dome does not use the
  view offset; ONE other installation's tilted dome drove the feature.
  R11's *"it can change during a show"* re-scopes to field knowledge,
  not his install. R28 rewritten accordingly (knows-or-expects basis,
  with the R13-style exit: "no dome I know uses it" downgrades the item
  to an engineering call).
- ~~**Old-as-spec WEAKENED for this surface** [derived]: §11.52(b)'s
  parity is conditioned on old being exact; here the owner testifies
  old's offset behavior is the end state of accumulated fixes whose
  cleanliness he doubts — the old behavior may be accident, not intent.
  Caveat lands on B17's shape-(1) "reproduce old exactly" (D6): the
  answer stands, but what "old exactly" is worth there is now
  qualified.~~ **SUPERSEDED same day by the owner's own bounding**
  [vixy 2026-08-31, third message]: *"The only two configurations which
  aren't tested much are (window height > window width) and heading.
  For the rest, I know one reported an issue about it, I don't know
  about those who reported nothing (worked around silently, didn't
  remark it or didn't take action to report it), though. But for the
  rest, old-as-spec is the safest default, precisely because I don't
  want to break what I don't know about."* The weakening is BOUNDED to
  exactly two configurations: **portrait aspect (h > w)** and
  **heading**. Everywhere else, old-as-spec binds INDEPENDENT of old's
  cleanliness, on a ground §11.52(b) does not state: the silent user
  population is unobservable (a workaround depends on the defect's
  exact shape), so reproduction bounds every value of that unknown at
  once — D9's frozen-field logic at the behavior layer. General rule
  candidate [derived, veto]: **divergence license concentrates exactly
  where defects do — in unexercised configurations — because silent
  dependents cannot exist where nobody goes**; the audit prior
  (§11.161(b), defects in rarely-exercised paths) and the divergence
  license are the same exercise-density variable, read from both
  sides. Consequences: B17/D6 shape (1) UN-qualified for the exercised
  region (the caveat above is retracted there); §11.92(d)'s routing to
  R28 CONFIRMED (heading is in the untested pair); R28 re-narrowed to
  the heading×offset combination, plain-offset parity stated to the
  tester as protected on principle.
- **NEW verification-surface hole, named by the owner**: portrait
  aspect (window height > width) is the second barely-tested
  configuration — no ledger row exists for it. Field-side probe added
  to R21's census (does portrait exist anywhere in the field?);
  harness-side portrait leg is a cheap candidate (supervisor's queue).
  Annotation owed wherever the verification-surface map lives.
- **Stratum datum** [vixy 2026-08-31, fourth message]: the owner asked
  *what the view offset is* (*"the fact I ask means it was even less
  exercised than heading"*) ⇒ the offset is confirmed
  inherited-stratum (consistent: it lives in `navigator.cpp`, one of
  the three §11.161(d) maximal-struggle headers). Remaining intent
  holders: the tester (R28), the 2020 French reference, git's fix
  history. Answered from source, same conversation: one scalar, draw
  pitches the view by `view_offset × fov/2` ramped by
  `view_offset_transition` [observed: navigator.cpp:324]; config
  `view_offset` + runtime `set zoom_offset` [observed: core.cpp:2531-49
  comments]; purpose = tilted-dome re-aim, fov-coupling makes zoom
  converge on the off-center sweet spot [tester Q3 + derived].
- **DEFECT CANDIDATE found while answering, NOT minted** [observed;
  consequence derived, callers untraced]: two application sites with
  DIFFERENT couplings — draw applies `fraction × fov/2`
  [navigator.cpp:324] while `setLocalVision`'s aim-compensation undoes
  `fraction × 90° FIXED` [navigator.cpp:162, feeding equ_vision and
  downstream]. They cancel exactly at fov = 180 (the standard dome) and
  nowhere else — offset 0.3 at fov 40 ⇒ ~21° aim-vs-draw mismatch. The
  hard-coded dome-case shape is exactly the testified fix-accretion.
  §5 mint + verification = supervisor's; also a concrete entry point
  for the R28/heading-corner work.
- **Named lead, RE-SCOPED**: the long-standing view-offset fixes in git
  — an archaeology pass (navigator/projector territory, §11.161(d)'s
  maximal-struggle cell) is now only worth running for the HEADING
  corner: the exercised behavior needs no reconstruction, the field
  ratified it; only the untested combination's intent is unrecoverable
  from use. Candidate task, supervisor's queue, priority accordingly
  lower.

## 5. Interface rule candidate (final-pass protocol)

[vixy 2026-08-31]: a proposal item must lead with its visible interest
— what it enables, simplifies, or removes from the reader's hands; a
change whose interest is not visible is RIGHTLY dismissed; the cost
must not look unbalanced; habits change hardly unless worth it.
Generalized into `USER_QUESTIONS_ROUND3.md`'s header as the eighth
format rule beside §11.177(i)'s seven. Candidate for the ledger home of
the final-pass question-shape rules (§11.173(d) neighborhood) at merge.

## 6. Owner self-model line (configuration data, D10 horizon)

[vixy 2026-08-31, in-conversation, off-work reflection]: *"My working
style is quite sequential by design, which transpose into sessions
being sequential dispatch orchestrate alternation. … The only aspect
which is not sequentialized is the one which is not code related, nor
work related. It contrast quite a lot with things like 'ultracode' in
claude code which is heavily parallel, but doesn't fit my workflow very
well."* — Owner self-report CORROBORATING the measured execution-mode
signature (INT-7; §11.168's anticipatory-commit finding; §11.173(b)'s
owner-confirmed intra-session resolution). Consequence for session
design [derived, veto]: the sequential dispatch-orchestrate topology is
RATIFIED by owner preference, not merely token-allowance-forced —
heavy fan-out (ultracode/Workflow-style) is a configuration mismatch
for this collaboration (depth-dominant work, single-authority ledger,
review-attention-bounded); parallelism stays licensed only below the
zero-coupling boundary (disjoint write surfaces, e.g. this session
beside F68/F69). Candidate home at merge: §11.173(e)'s owner self-model
lines.

**Refinement, same conversation** [vixy 2026-08-31, fifth message]:
prior self-model was *reactive* — *"When I write code, I use it to test
my ideas, so I can't delegate writing code without delegating designing
it"* (⇒ the CAUSE behind the recorded pre-Fable LLM disinterest, a fact
the ledger held without its cause); mechanism in his words: *"I avoid
merging and the resolution part by making resolution local. I do less
in the same time, but I correct more over feedback and end up reacting
faster - because what slow down have not been built"*; and the trade
re-evaluation: *"I thought I traded throughput for robustness. Maybe I
didn't, if my style cover the losses with long-term gains."*
Derived mechanism offered and accepted-pending-veto: feedback
evaluation is the NON-PARALLELIZABLE stage (one head, one model) ⇒
local-sequential is throughput-OPTIMAL given a serial validator, not a
trade; WIP≈1 ⇒ minimal reaction latency (Little's law) and defect cost
capped at ~zero inventory; integration debt deferred-superlinear vs
local resolution immediate-linear. Trade DISSOLVES at the D10 horizon
(robustness = the compounding term of integrated throughput), with the
stated BOUNDARY: ~~holds only for root-level local resolutions — the
two recorded counterexamples are proxy-level ones that returned with
interest (the 2020 log decision without a cost model → §5.115; the
serial script model never refined → [parallel-script])~~ **SUPERSEDED
same conversation [vixy 2026-08-31, sixth message] — the owner
corrects the shape: both specimens were ROOT-LEVEL, EARLY resolutions
BLOCKED AT A RESPECTED GATE, not shallow ones.** New facts, verbatim
where testimony: *"the main tester/user is the gate for work in
spacecrafter"* (governance statement — tester intent gates even the
owner's root-level fixes; sharpens §11.177(f)'s two-axis routing with
its historical dimension); the log hole was PREDICTED and WARNED
(*"could take too much room"*), disregarded because the observable was
absent; trigger = the infinite-loop feature (~1 year later), root = a
choice predating spacecrafter by decades — trigger/root correctly
separated by the owner at the time; countermeasure = **resolve and
hold armed**: EntityCore's crash-only-retention log system built in
OWNED territory, routed in advance for fast, non-conflicting
deployment, deployed when BOTH predicted consequences materialized
(valuable log erased; disk overloaded) — the tester's own >1 GB
disk-trace being the gate-moving event; the script model: 5+ years of
reject→refine→repropose (industry-standard-informed, aiming to close
silently-dropped resources, hook limitations, script-imposed
structural gates). **Boundary RELOCATED** [derived, veto]: the style's
leak is not resolution depth but the GATE's evidence threshold (a
correct prediction without an observable cannot move it — C3's
feedback-channel precondition at the governance layer); the owner's
standing fix is FORCED MATERIALIZATION of the evidence (*"I built
around this capability and not only the root, so that I could show
value, because I want it to land"*) — which grounds §5 below: the R25
framing rule is the owner's own long-practiced gate-crossing method,
now stated. Residual cost named: armed-solution inventory (converged,
so it does not rot like unvalidated stock, but "not in conflict with
spacecrafter" is a MAINTAINED property). Owner self-assessment line
for §11.173(e): *"I think I have learned to see where the structure
can break, so that I can prevent the materialisation of problems, not
merely correct them as they get diagnosed"* — corroborated by the
prevention shape of D9/D12/D13. Corollary UNCHANGED: the
dispatch harness is the owner's style transposed one level up (the
ledger/report contracts are the re-coupling machinery that made
design-fraction delegation possible at all).

**Availability profile, same conversation** [vixy 2026-08-31, seventh
message; configuration data for session/batch scheduling]: the
owner-side gain of the collaboration is PARALLELISM ACROSS HIS
UNAVAILABILITY, not speed (*"you are not much faster than myself, but
you can work in parallel of me"* — completed same conversation:
*"But you have more context window"* — the WORKING-SET asymmetry:
whole-corpus coupling sweeps are the task class his substrate prices
highest and the model's prices lowest, which is why aggregation
compiles land on sessions and judgment stays with him; the ledger is
the DUAL prosthesis — working-memory extension for the small-window
persistent head, long-term memory for the large-window volatile one);
decision latency is partly
DELIBERATE pipelining (*"I let you work while I can't"*); reliable
deep-work windows = weekdays + **Saturday morning**, the rest of the
weekend unreliable. The cause was disclosed in-conversation and is
DELIBERATELY NOT COMMITTED here (personal; this repo shares a public
remote and the horizon is decades — the operational profile carries
everything scheduling needs; adding the cause is the owner's override).
Consequences [derived, veto]: steering bandwidth is the system's
bottleneck resource ⇒ §11.177(i)'s question-format rules are
BOTTLENECK MANAGEMENT, not interface polish, and apply to both humans'
channels; decision batches target reliable windows, never the weekend's
remainder; weekend-spanning sessions plan for ZERO steering
(self-sufficient, gates respected, everything queued); the ledger's
function list gains, explicitly: continuity of state across the
owner's attention windows.

**Two comparison-table corrections** [vixy 2026-08-31, eighth
message]: (1) "persistent store: none" for the model is WRONG — the
`claude/` directory IS the model's persistent store and demonstrably
works (*"INTENT.md and QUEUE.md let you continue the work
transparently, without me ever needing to remember them all"*).
Corrected asymmetry: the owner's store is deep but its ACQUISITION
channel is lossy (vocal-dominant, unreplayable, origins fade — his
stated envy: *"the ability to reliably store everything at lowest
cost - and every sessions here can be retraced to the word"*); the
model's store is thin per-session but lossless and
retraceable-to-the-word. (2) "authority/judgment: none" conflated two
axes — authority is the owner's, gated; JUDGMENT is exercised on both
sides (*"you also have some judgment, otherwise you wouldn't be good
at orchestration"*) with a DESIGNED conversion channel: structural
arguments that hold (userPreferences' own line) convert judgment into
authority. Related architecture-genealogy disclosure routed
cross-project → `~/shared/QUEUE.md` Q-57 (error/attack as one failure
surface under two sampling distributions; integration owed to the
fork by the next architecture-touching session).

## 7. Provenance note

The three script requests ([parallel-script]/[script-binding]/
[script-trigger]) are VIXY's, not the tester's — FEATURE_REQUESTS.md
mixes both sources, and its header still scopes the file to requests
"from outside the development process itself", which no longer matches
its contents (flagged to Vixy 2026-08-31, his call: amend header or
split log by origin). R25 was corrected to attribute explicitly
(commit `ef6adcc`).
