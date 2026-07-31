# fable-dispatch.md — unblocked, load-bearing tasks (2026-07-24, Claude Fable 5)

**Purpose [vixy 2026-07-24]:** a dispatch list for token-allowance-sized runs. Claude Code
has no clean suspend/resume on token depletion — only abort — so each task here is
(i) self-contained, (ii) sized, and (iii) carries an abort-tolerance structure
(checkpoint commits + WIP line) so an aborted run loses at most one checkpoint.
Dispatch one task per fresh session, whenever the allowance permits it to run
uninterrupted.

**Authority note (I2):** this file is a *dispatch view* over `INTENT.md` §13 — it owns
no row. On any divergence, §13 + the cited `INTENT/<id>.md` entries win, and the
divergence is a staleness bug HERE. Before acting on any task, re-read its §13 row and
every recording entry it cites (§5.2 class: cached conclusions re-verify against
source, never recall). Compiled at code `master-beta @ ae3a218d`, harness `6b0189f`.

**Mode change [vixy 2026-07-25, via Fable]:** tasks are now OPERATED by `opus-xhigh`
executors (Opus 5) dispatched and supervised by Claude Fable 5 from a supervising
session — one task per executor run, sequential, checkpoint discipline unchanged;
Fable reviews each delivery against the ledger and escalates to a Fable-xhigh
re-analysis agent on doubt. NB: the executor's standing definition
(`.claude/agents/opus-xhigh.md`) carries STALE pre-move paths
(`src/experimentalModule/INTENT.md`, `§12`, `dispatch-2026-07-19.md` — none exist);
until Vixy resyncs it, every dispatch prompt carries a binding supersession block.
De-staled against §11.101/§11.102 (2026-07-24 audits): F0 added, F1 spec revised.

**Update [Fable 2026-07-30, supervising session 3]:** the §11.113 propagation (D22–D36
answered by Vixy 2026-07-26, propagated 2026-07-29) restructured the dispatchable set
after this file's last de-stale: seven §13.A rows closed (A33–A39), rows **B39/B40**
opened, **B27/B24/B33/B14** reopened-or-annotated, **B31 implementation UNBLOCKED**
(D30–D36 = the review F6 waited on). **F10–F12 added below, in dispatch priority AHEAD
of F8/F9** (numbering is by creation; file position = priority). Deferred with reasons:
B31-impl (behind F10 per §11.110's own sequencing: §5.39 parse → writer rework →
persistent-body serialization → session file — next session's headline), B14/D22
conversion fix (decided, queued next session). DECISIONS_PENDING open set: **D15 + D21
only**.

**Update [Fable 2026-07-30, supervising session 4]:** round of 3 per the sizing lesson
[vixy 2026-07-30: 6 hard tasks ≈ 80% supervisor context; 3 reuses the derivation cost
without depletion risk]. This session: **F12 → F13 → F14** — F13/F14 minted below from
session 3's own deferral note (B31-impl now unblocked: its §11.110 sequencing gate
§5.39 closed with F10/§11.115; B14/D22 decided §11.113(a), queued). F8/F9 stay after.
DECISIONS_PENDING open set at session start: **D15, D21 (both SCHEDULED last week of
August, §11.116(a)), D37**.
**Round outcome (session 4 close):** F12 → §11.118, F13 → §11.119, F14 → §11.120 — all
three delivered AND supervisor-verified same day; §5.28/§5.45 closed, §5.47/§5.48/§5.49
opened, D26/D27/D28 implemented; no new Vixy decision minted (three veto points
recorded in-entry: B27 hint-gate on `primary`, `primary`-as-own-member, §5.49's owed
test). One host OOM killed the first F12 executor mid-task (successor verified its
surviving diff rather than inheriting it); supervising protocol extracted → §0b.
Remaining dispatchable: **F8** (opportunistic), **F9** (design-first), **next B31
slice** (persistent-body serialization §4.1 — completes B25's half; section to mint
at its dispatch).

**Update [Fable 2026-07-30, supervising session 5]:** round of 3 per the sizing lesson:
**F15 → F8 → F9**. F15 minted below (B31 slice 2 = persistent-body serialization §4.1 +
live annotation wiring — the section session 4's close said to mint at its dispatch),
positioned ahead of F8/F9 like F10–F14 were (mandated product line beats opportunistic
hunt + design-first). Warm-up: both trees clean (code `f11f6a4e`, harness `2068afb`);
binary present, mtime 16 min before the F14 delivery commit — consistent with F14's own
build-verify-commit order, and the first executor rebuilds regardless. DECISIONS_PENDING
open set at session start: **D15, D21 (both SCHEDULED last week of August, §11.116(a)),
D37 (new, F11 — rec (1) keep, awaiting Vixy)**.
**Round outcome (session 5 close, 2026-07-31):** F15 → §11.121, F8 → §11.122,
F9 → §11.123 — all three delivered AND supervisor-verified. **B25 CLOSED** (F15:
live-tree save == machine twin line-for-line, T6 both ways); **B31** open on the
session file + ledger only; **B7** open at ≤ 2.8 % pooled with
contention-as-saturation EXCLUDED and its FIRST root-caused teardown defect FIXED
(shutdown heap-UAF, `1e44b639` — found by re-measuring §11.95(b)'s false ASan-cost
premise); **B12** open for CONTENT only — the dark-Sun-disc divergence is RETIRED
(§11.44's named residual closed; design note `b12-design.md`, F6 discipline held).
New defect rows, all record-only: **§5.50** (push-channel `surface_point` kills the
app — old-path bail-out named), **§5.51** (no virtual `~BodyModule`, 373 dtors
never run — masks all teardown measurement, fix needs its own dispatch),
**§5.52/§5.53/§5.54** (G4 mid-band/texture-level/threshold-spelling cluster, F9's
finds). No new DECISIONS_PENDING row (open set unchanged: D15, D21, D37); session-5
veto points + B12 content decisions + host note → §3. One executor deviation class
recurred and is worth naming: BOTH F9's inertness DoD and F3's A8/A9 line were MY
paraphrase drift in this file, and both executors correctly followed the ledger
over the view — the §0b.3 rule ("mandate verbatim or by exact ledger ref") exists
for exactly this; prompts should quote rows, not restate them. Remaining
dispatchable, next round: **F16 to mint** (teardown-integrity batch: §5.51
virtual-dtor + its regression battery, §5.50 bail-out — F16 BEFORE any further
hunt, §5.51 masks the instrument), **B7-hunt-4** (ASan mix N ≥ 50 + TSan + the
§11.122(i) HUNG re-measure pre-warmed, AFTER F16), **G4-coherence batch to mint**
(§5.52/§5.53/§5.54 — interacts with the B12 content slice's thresholds),
**next B31 slice** (session file §3.2 — its mint must state the D28/D21 carve-outs
explicitly: `heading` and T3 are sequenced behind Vixy decisions scheduled last
week of August).

**Update [Fable 2026-07-31, supervising session 6]:** round of 3 per the sizing lesson:
**F16 → F17 → F18**, minted below from session 5's own close queue (F16 before any
hunt — §5.51 masks the instrument; B7-hunt-4 after F16; G4 batch). Third slot kept on
the G4 batch over the next B31 slice: the session file carries two carve-outs behind
late-August decisions (D28/D21) and takes a revisit wave regardless, while §5.52 is a
user-visible hole in D3's common case. Warm-up: both trees clean (code `a958c05e`,
harness `fe7e2dc` — the harness moved past session-5 close: cadence corrections
§11.122(o)/§11.123(o)(o2) + INTENT archival pass 1, all verified, no code change);
binary present (mtime 03:22, consistent with F9's build order; first executor rebuilds
regardless). DECISIONS_PENDING open set at session start: **D15, D21 (both SCHEDULED
last week of August, §11.116(a)), D37 (awaiting Vixy)**.
**Round outcome (session 6 close, 2026-07-31):** F16 → §11.124, F17 → §11.125,
**F19** → §11.126 — all three delivered AND supervisor-verified same day. **The round
restructured itself mid-flight**: F17's hunt caught the row's first reproducible
teardown crash (reload + composed OJM + quit, ~96 % under load) and attributed it
single-variable to F16's own §5.51 fix having made two long-standing lifetime
violations REACHABLE; the supervisor decision (recorded at F17's WIP + §3): fix-first
— F19 minted into slot 3, F18 deferred with its section intact. Net: **§5.50, §5.51,
§5.55, §5.57, §5.58 CLOSED** (the teardown-order class fixed at the class, audit
in-entry); **B7's crash class CLOSED** (27/30 → 0/30; the row is now the HUNG class
alone = §5.59, reproducible on demand, its fork Vixy's as **A40**); §5.56 re-derived
and open; §5.59 open with its fix BUILT, MEASURED, WITHDRAWN (I7 bar not met — it
converts the hang into a crash); NEW §5.52–§5.54 (F9, pre-existing), §5.60 (device
limit, Vixy), §5.61 (EntityCore lost wakeup, recorded read-only). TSan is UNUSABLE on
this driver (§11.125(e), N=0 stated). Executor quality this round: two corrections of
predecessor records at source (§11.124(c) LeakSan, §5.55's signal face), one
fix-refused-on-its-own-bar — all three the discipline operating, all endorsed.
Remaining dispatchable, next round: **F18** (minted, head of queue), **next B31
slice** (session file §3.2 — mint must state D28/D21 carve-outs), **B7-hunt-5 only
after A40** (and its mix gains the line-family flags — the §11.124(k) hole is now
closable), **F18's G4/B12-content interaction** waits on Vixy's b12-design §7 set.

---

## 0. Cold-session warm-up protocol (run this first, every dispatch)

1. `CLAUDE.md` auto-loads (the map). Read THIS file; locate your task's section; read
   its **WIP line** — if non-empty, a prior run aborted mid-task: `git -C` log both
   repos since the noted checkpoint and resume from there, do NOT restart from zero.
   After any discontinuity, re-read sources before editing them (a summary/WIP note is
   unidentified knowledge until re-extracted).
2. Re-read the task's §13 row in `INTENT.md` + the `INTENT/<id>.md` entries it names.
3. `git -C /home/claude/spacecrafter status` and `git -C /home/claude/spacecrafter/claude
   status` — both clean expected; note both HEADs.
4. Build: `build-claude/src/spacecrafter` must exist (harness default `SC_BIN`);
   rebuild if the code HEAD moved (`claude/harness/README.md`).
5. Standing constraints (reasons in INTENT.md; violations are delivery-blocking):
   - Old render path = comparison baseline, unchanged by construction (§11.52(b)).
   - Data values NEVER from recall — cited fetch only (§11.51(d) red line).
   - Fresh-launch precondition for measurements; config/ssystem md5 in==out asserted
     (pristine pair as of compile date: `03fbee59` / `545a51ef`).
   - Loaded data `~/.spacecrafter/ssystem.ini` is ISO-8859 and untracked — the Bash
     `grep` wrapper silently excludes it; use `/usr/bin/grep` or Read.
   - Cost claims use the **1 ms/frame** denominator (§2.0 D11). Acting defaults are
     LOGGED (§2.0 D12). Actionable diagnostics per §2(f).
   - A green build is not coverage — every task names its discriminating check; if a
     check needs a display, say so instead of substituting a weaker one.
   - **No `run_in_background` for long campaigns** — foreground within-turn batches
     only (§11.98(h) template fix; three executor stalls root-caused to this).
   - **Memory-bounded builds (2026-07-30 host-OOM incident — killed an executor
     mid-task):** session affinity is now 12 cores, so `-j$(nproc)` self-caps at 12;
     additionally check `free -g` BEFORE each build — available < 16 GiB ⇒ use `-j6`.
     Other sessions share this host's RAM; memory, not cores, is the binding
     constraint.
   - **Concurrent-instance assert (2026-07-31, §11.121(m)):** before each measurement
     launch, assert no other spacecrafter process exists — ANY account: the observed
     confound is INTRA-account (concurrent claude sessions/agents share
     `~/.spacecrafter`), so md5 re-asserts alone do not cover a concurrent launcher.
     On hit: record it, wait it out, launch fresh. F8's `b7h3_host.log`
     batch-boundary check is the precedent instrument.
   - **Display architecture is part of the instrument (2026-07-31, §11.122(o) +
     §11.123(o)/(o2)):** claude renders on his OWN headless GNOME/Xwayland `:2`
     (GPU-real; the harness default); Vixy's remmina/RDP relay is view-only and its
     CPU is coupled to what WE draw. The "161.3 fps" cadence label is UNATTRIBUTED
     among three clocks that all fail to match it exactly (config `maximum_fps = 144`,
     `:2` virtual monitor 59.96, panel 164.5) — sharpest hypothesis: the dwell's
     "20 s" denominator was nominal and the true cadence is EXACTLY the config cap
     (§11.123(o2), H1; discriminating check owed by the next cadence-touching task:
     wall-clock-bracketed counter reads). Until settled: trust counter RATIOS and
     in-run A/B only; never absolute fps labels, never cross-session cadence; a
     stack change (compositor, streamer, headless X, screen power state) ⇒ report +
     re-baseline.
6. Abort-tolerance discipline (the reason this file exists):
   - Commit code + harness at **every green checkpoint** (small commits, normal
     trailer discipline: code first, harness carries `Code: <branch> @ <short-sha>`).
   - Update the task's **WIP line** in this file at each checkpoint (one line: date,
     checkpoint reached, next step); clear it at delivery. Commit the WIP update with
     the checkpoint — an uncommitted WIP line protects nothing.
   - Never start a long verification campaign with uncommitted work.
   - Delivery = INTENT §11 entry (file + stub) + §13 row flip + harness commit, as
     usual. An abort before recording ⇒ the successor resumes at the last checkpoint.

Sizes: **S** ≈ short focused run · **M** ≈ one full session · **L** ≈ full session at
high effort, mandatory checkpoints. Estimates are mine [derived], not measured.

---

## 0b. Supervising-session protocol (Fable) — the user triggers a round with ONE line

**Trigger phrasing (reuse verbatim):** *"Dispatch round: run a supervised dispatch
session per claude/fable-dispatch.md §0b."*

1. **Warm-up**: both trees clean + note HEADs; binary exists at code HEAD; re-read this
   file's update notes AND the §13 rows of the candidate tasks (this file is a view —
   §13 + the cited entries win). After any discontinuity, re-verify state before
   relying on it (the 2026-07-30 OOM left an executor's uncommitted diff in the tree;
   "clean expected" is an expectation, not knowledge).
2. **Pick the next 3 dispatchable tasks** by file position, honoring deferral/queue
   notes in the update block; a task without a section here is not dispatchable —
   mint the section first, commit, then dispatch. Sizing is measured
   [vixy 2026-07-30]: 6 hard tasks ≈ 80% supervisor context; fewer than 3 repays the
   session's derivation cost too often; 3 is the sweet spot.
3. **Dispatch sequentially** — one `opus-xhigh` executor per task, synchronous, never
   parallel (deliveries are each other's baselines). Every prompt carries: the binding
   SUPERSESSION BLOCK (template below, values refreshed), the task-section pointer +
   warm-up order, the mandate verbatim or by exact ledger ref, stop boundaries,
   discriminating checks, checkpoint discipline, and the report format (per-item DoD
   state + evidence pointers + deviations + suspensions + what the next task must know).
4. **Verify each delivery BEFORE the next dispatch**: read the §11 entry IN FULL;
   check trees/commits/authors; check every claimed ledger flip (§5, §13,
   DECISIONS_PENDING) at the ledger; judge every deviation and judgment call — endorse
   with the argument, or escalate to a re-analysis agent on doubt. Harnesses are NOT
   re-run when committed artifacts + both-ways discrimination records suffice; a
   claim without such a record IS a reason to re-run or escalate. Record acceptance in
   the task's WIP line; commit the acceptance. **After ANY WIP-tail edit, verify the
   NEXT `###` header still exists before committing** (`grep -c '^### F'` vs expected)
   — two headers have been destroyed by acceptance Edits whose old_string swallowed
   the following header as anchor context (F13, F18); anchor inside the WIP block,
   never across the section boundary.
5. **Close**: refresh section 3 (For Vixy) with new veto points/decisions, append the
   round outcome to the session update note, commit, and report to Vixy: deliveries,
   endorsements, anything newly Vixy's, the remaining dispatchable set.

**Supersession-block TEMPLATE** (the executor's standing definition is stale — every
prompt carries this, values refreshed): intent authority =
`/home/claude/spacecrafter/claude/INTENT.md`, expanded entries `claude/INTENT/<id>.md`
(NO `src/experimentalModule/INTENT.md`, NO §12, NO `dispatch-2026-07-19.md`); delivery
record = §11 entry at the next free number ⟨N⟩ + §13/§5 flips + this file's WIP line at
every checkpoint; today's date ⟨date⟩; `claude/` is its own repo — code committed
first, harness carries `Code: master-beta @ <sha>`; memory-bounded builds per §0.5;
no `run_in_background`; both HEADs stated ⟨code, harness⟩.

---

## 1. Dispatch order (load-bearing first; each task states why, so the order is challengeable)

### F0 — audit-residuals batch: decision-free fixes from §11.101/§11.102  [S–M]  — DELIVERED 2026-07-25
- **Row / recorded:** B5 residuals (§11.102(e1)–(e4)) · B25 (§11.102(c)) · §11.101(g)(g3)(h) · §11.102(g) ("all decision-free implementation residuals" except (b2)).
- **Why first:** (i) instrument debt bites every future task (`b5_oort.py` vacuous
  under flag-failure; `ab_orientation.py` cannot fail; runner md5 echo-not-assert);
  (ii) every item is precisely source-located with a stated fix shape — ideal
  calibration for the new operator model (bounded blast radius, high diagnostic
  value on protocol compliance); (iii) B17(b2) EXCLUDED (interacts with suspended
  §11.92(d)); B17(b1) deferred to its own dispatch (live commanded-channel
  verification burden).
- **Task:** B25 sidereal precedence guard; §11.101(h) zero-init; b5_oort
  path-identity assert; runner md5 exit codes; ab_orientation discriminating
  criterion; oort color-seam mirror; createExperimentalOort no-op logged (D12);
  b23_grid tracking release; b5_ladder header-comment bound fix.
- **Discriminating check:** per item (each must be shown able to FAIL); battery
  subset green (b24_equivalence, b5_oort, b5_ladder, b23_grid); md5 pristine.
- **WIP: DELIVERED 2026-07-25 → §11.103 (code `2600ca47`/`b87412b3`/`d006ee92`,
  harness `2ee877c`/`cdb33d9`/`2940b71`). 9/9 met; supervisor-verified directly
  (diffs, ledger discipline, mutation-residue, md5, tree state — harnesses NOT
  re-run by supervisor; committed artifacts + both-ways discrimination records
  accepted as evidence). Executor deviations all principled + flagged (item-1
  operator[]-insertion trap caught by its own mandated counterfactual; item-5
  root rework per I6). Out-of-scope finds recorded in §11.103(j)(k): drawLoaded
  vs draw regime split (I2), §11.35 scene-P non-reproducibility at HEAD, 3×
  §11.15d fires (→ B7 row annotated with the contention datum). Calibration
  verdict on the Opus 5 operator model: POSITIVE.**

### F1 — B3: D1(b) grounded-slice parent-depth prefill  [L]
- **Row / recorded:** B3 (§13.B) · §11.97(b)(e) · shadow-paths.md H4(b) · §3.1/§3.6 · §11.30 (S3 consumer landed).
- **Why most load-bearing:** it is the located, characterized missing capability of the
  §2.0 **D1** domain constraint (grounded child cannot occlude against a parent whose
  scale dwarfs its depth slice) and the STOP that bounds the whole grounded-composition
  mandate (§11.78(a): rover/rocket scenes). Everything on that line queues behind it.
- **State:** §11.97(b) source-located it — `computeShadows` nominates only the top
  body's own self-shadow [ModularSystem.cpp:206-253]; mesh/OJM modules route to
  `nearComponents` [OjmLoader.cpp:49, LayeredMeshLoader.cpp:86]; grounded children
  merge into the parent's ONE coarse D24 bucket (±1737 km at AU-scale); the header's
  dual-purpose (b) is unimplemented [BodyModule.hpp:212-213; Renderer.hpp:75-80].
  Measured defect, bidirectional: in-front-of-surface rover HIDDEN (0 px);
  3000 km-buried rover SHOWN (21377 px).
- **Task:** grounded bodies get their own fine depth slice, prefilled with the
  parent's depth trace (the D1(b) design stated in ModularBody.hpp:120-129 + §3.6).
- **Discriminating check:** the §11.97(b) probes FLIP — in-front rover shows, 3000 km-
  buried hides; far-separation and 9000 km-buried cases stay correct; the §11.97(a)
  shadow observable stays; `b24_screen.py` + full battery green; old path untouched.
- **Stop boundaries (NOT yours):** H4(a) SECONDARY ladder; H4(c) RGBA8 SELF_COLOR;
  the row-16/D4 surface module (the second bound on the FINE observable — §11.97(e);
  out of scope, the coarse capability is the deliverable).
- **WIP (2026-07-25, Fable, SPEC REVISED by §11.101(c)+§11.102 — supersedes the
  2026-07-24 WIP, whose "no measured defect" claim was falsified by the audit):**
  D1(b)'s MEASURED-defect justification RESTORED — §5.29 (ray-march proxy shell
  writes shell depth, never `gl_FragDepth`; Δ 34.75→~349 km on the Moon, whole
  `<64·scaledRadius` band) + §5.30 (Earth NIGHT row writes NO depth — grounded
  occlusion vs Earth impossible by construction). b24_screen occlusion asserts are
  NON-DISCRIMINATING (painter's order) — replace with the §11.101(c) size ladder
  (20/45/90/250 km at alt 0, Moon, `flag moon_scaled off`, observer 8000 km) +
  lifted-20 km control. Mandate scenes under shipped config STILL blocked on D21.
  **PART 1 DELIVERED 2026-07-25 → §11.104 (code `922701c9`, harness
  `8a294d8`/`c1ec7b8`/`d3343f1`): §5.29 CLOSED — wall inverted from render moved
  from 34.77/34.93 km two-site-constant (shell) to 21.33/19.74/22.78 km
  site-dependent (terrain); D11 |Δ| ≤ 0.06 ms/frame, null predicted from
  mechanism; content-free scenes byte-identical; supervisor-verified, one
  residual-closure amendment requested and delivered ((d2): all three
  out-of-band readings closed by quantified bias+quantization propagation,
  b250 left OPEN with criterion — not a metric leg). New instrument knowledge
  for part 2: cap ±0.2% systematic + ±1 px; wall meter sharp at r ≈ w·√2;
  `moveto lon L` ↔ `orbit_lon` differ by 180−L (§11.104(g)).**
  **PART 2 DELIVERED 2026-07-25 → §11.105 (code `d28e67d3`/`ded43e5a`, harness
  `db4207f`..`a11ae3d`): §5.30 CLOSED (WHY-derivation from git archaeology —
  the depthless requirement was named and had been retired at source twice;
  NIGHT frag depth write same commit; rider discharged BY MEASUREMENT — shell
  hypothesis predicts a0=0/a100=42.2, measured 53.00/53.25; wall 5.53 km vs DEM
  5.50–6.01) + FORCED EXPANSION §5.33 CLOSED (atmosphere shell wrote depth by
  inherited ctor default — 191.47 km wall measured vs 191.3442 derived,
  counterfactually attributed, one-bool fix, stock byte-identical) + D1(b)
  NOT BUILT — STOPPED on a measured derivation (no producer in Surface regime,
  ~2800× over-resolved in Outer) → suspended as A35/D24; axis-occlusion
  divergence → A36/D25. Old path measured unchanged (max|Δ|=0 on 379k lit px).
  Supervisor-verified (entry, diffs, design clause at source, ladder tables);
  DECISIONS_PENDING propagated (D24/D25). **F1 IS COMPLETE AS DISPATCHABLE** —
  B3 residue = excluded pieces + D21/D24/D25.**

### F2 — B24-select: composed bodies selectable  [M]
- **Row / recorded:** B24 (§13.B) · §11.97(d) · §11.60 (ModularObject uninstantiated) · A17/R5 (§11.70: visibility is the selection domain; BIGGEST wins in-tolerance).
- **Why load-bearing:** §2(c)-class capability gap — a visible body that cannot be
  selected violates the resolved A17 criterion, and every screen harness must aim
  geometrically until fixed (instrument debt compounding across future tasks).
- **Task:** route selection to the new tree for composed/new-only bodies (the
  ModularObject/ObjectBase bridge is built and idle — §11.60 readied it); old-path
  selection behavior unchanged (parity); disambiguation per R5 where it bites.
- **Discriminating check:** `select planet <composed-name>` lands (tracked/selected
  populated — today measured empty); old-body selection bit-identical to before;
  b24_screen re-run using selection-aim as a cross-check of the geometric aim.
- **WIP: DELIVERED 2026-07-25 → §11.106 (code `2c611b02`, harness `3361ecc`..
  `e4eaee6`). Both channels land (command red→green on §11.97(d)'s own pair;
  pointer via real XTEST clicks — new `xclick.c`, WM-frame trap caught with a
  20 px-sensitive calibration pair); R5 two-tier in the NEW tree's findBodyAt
  only (old picking untouched by construction — the route runs only where the
  old picker declines); old-body parity 90/90 identical, selDist Δ=0, screen
  below its own floor, readout ULP wobble attributed to -Ofast header codegen
  by three-probe isolation (banked: cross-build float byte-compares unsound
  here). ONE suspension → A37/D26 (child inside parent's disc — rec added by
  supervisor); §5.34 minted (Object::operator= leak, pre-existing class).
  Stale-artifact hazard in b24 gates fixed at root; earlier tasks' gate runs
  checked fresh-by-evidence (measured values moved across tasks). Supervisor-
  verified (entry, diffs, R5 locus, D26 propagation).**

### F3 — B27-tail: hardcode retirement A5–A9 + Tier-B (new format)  [M]
- **Row / recorded:** B27 (§13.B) · §11.73 (site map, file:line) · §11.79(e)(h) (D10key spellings RATIFIED; D14 Tier-B format-scoped) · §11.89(c) · §11.91 (A1–A4 precedent, co-delivery pattern).
- **Why load-bearing:** G1/G6 substrate — name-sniffing is the anti-pattern the
  composition mandate retires; A6 (`surface_model`: Moon shader lineage still
  `type=Moon`) blocks arbitrary-body surface models, a composition capability.
- **Task:** A5 Sun day-length → `isStar()`; A6 moonClass → `surface_model` key;
  A7 trail_length → key; A8/A9 G6 heuristics → declared keys; Tier-B `type`-identity
  retirement in the NEW format only (D9 keeps legacy `type`-driven forever). Emit
  (B25 twin) + consume co-delivery per §11.73(g) — never one without the other.
- **Discriminating check:** per-site counterfactual (key absent ⇒ legacy behavior;
  key present ⇒ drives) as §11.91 did for A1/A2; b24_equivalence green +
  discriminating post-change; grep-clean on the retired literals in new-path code.
- **WIP: DELIVERED 2026-07-25 → §11.107 (code `384f57b4`, harness `fdc89ac`..
  `46ca3cf`). A5/A6/A7 + Tier-B landed on the D14 boundary; 9-leg counterfactual
  table all predicted-then-matched; A6 verified at screen height (0-px A/B
  explained from a config coincidence, then predicted away: 562k/2.28M px when
  altimetry levels differ); D9 legacy parity 0 diffs on 93 bodies; co-delivery
  hole closed at the instrument (b24_equivalence field list + composedDecl
  assert); `authored()` = the one absent-or-empty authority (§11.103(b) trap
  closed as a class). OjmLoader duplicated-authority veto fixed (§11.89(c)
  blocker, two-binary discriminated). NOTE: this section's "A8/A9 → declared
  keys" line was MY paraphrase drift — §11.73's own verdicts are "keep";
  executor followed the authority (deduced-mode retirement suspended
  §11.107(g2)). Suspensions → D27 (Tier-B spellings, rec ratify-as-is,
  endorsed) + §11.89(c)'s type=BODY move (unblocked, still Vixy's).
  Supervisor-verified.**

### F4 — S6-sweep: capability audit + §5.26 heading parity + B21 keyboard-descent  [L]
- **Row / recorded:** spine item 7 (§ dependency spine) · §2(c) (bar: enumerate CAPABILITIES, not commands) · §5.26 · §11.92(e) · B21 residual (§11.72) · §11.97(d) note.
- **Why load-bearing:** the §2(c) criterion is a product-intent root (R1 on the
  control surface); §11.33/§11.36 showed exactly this blind-spot class recurs until
  audited at the capability level. The two named seams ride along per §5.26's own note.
- **Task:** (i) exhaustive AppCommandInterface capability audit against §9 — output =
  inventory + gap rows minted in §13 (do not fix-in-place beyond S class); (ii) fix
  §5.26 `set heading` old/new semantic desync (parity meta-rule §11.52(b): old's
  observable is the spec, heading≠0 regime; leave B17's suspended heading-coupling
  question untouched — §11.92(d) is Vixy's); (iii) B21 keyboard-descent unification
  (I2: one descent authority, `Camera::descend`).
- **Discriminating check:** (§5.26) cross-path px at `set heading X` with offset
  active collapses to the ~132 px class measured at natural heading; (B21) keyboard
  and command descent produce identical trajectories; (audit) every capability row
  carries its reachability evidence or a minted gap row.
- **WIP: DELIVERED 2026-07-25 → §11.108 + `capability-surface.md` (code
  `f4dd61f9`, harness `83d0e65`..`dbf095d`). Audit: channel model enumerated
  first (4 live command channels ⇒ "command-but-no-key" is not a gap); rows
  minted B33–B37 + A38/D28 + §5.35; biggest find MEASURED — arrow keys turn
  NOTHING under the new path (xkey positive-controlled). §5.26's recorded
  observable REFUTED at HEAD (129 vs 130 px baseline; the 11k-px leg is
  §11.92(d)'s suspended coupling, attributed to 0.09%); third divergence found
  (reference-switch roll) → A38/D28 (rec (a) hold-orientation + B33 readout
  fix, endorsed). B21 ramp unified — key vs command descent BIT-IDENTICAL,
  real-null proven on pre-fix binary; joypad-axis route recorded (step-feel,
  Vixy). §5.32 first measured observable (same-frame descents compound).
  SUPERVISOR NOTE: the two "died mid-stream" delegated sweeps (bases B & F)
  actually COMPLETED and bubbled to the supervisor — full results handed back;
  **integration LANDED (code `e3a5b19c` = the script-speed S-class one-liner
  red→green on the operator channel; harness `dd36a4b`): bases B/F closed,
  B38 + §5.36 minted, B34 self-corrected (trail seam IS dual; the dead thing
  is the CoreLink wrapper), the handed-back census itself re-verified and
  corrected at source (sky_draw), and the reporting failure's rule extracted
  to §11.108(g): a delegated result that does not arrive is an UNOBSERVED
  state, not a negative one — chase the channel before writing the residual.
  media-subtitle-toggle one-worder correctly NOT taken (counterfactual not
  observable on this host — no video asset, no readout).** Supervisor-verified.**

### F5 — B25-galactic: galactic-corpus twin verification  [S–M]
- **Row / recorded:** B25 (§13.B) · §11.78(f) · §11.52(a) writer contract.
- **Why load-bearing:** the twin is the migration vehicle for the paid data surface
  (§2.0 D9) — an untested generation path on galactic corpora is unverified product
  surface, exactly where D9 makes corruption a product-destruction class.
- **Task:** exercise twin generation on addSystem/galactic corpora (same code path,
  never exercised); verify order/byte preservation + equivalence per the shipped-
  corpus precedent; fix what is found within the writer's existing contract.
- **Discriminating check:** an equivalence gate on a galactic corpus, green +
  discriminating (mutation caught), md5-pristine originals.
- **WIP: DELIVERED 2026-07-25 → §11.109 (code `14bb627f`/`770ef38a`, harness
  `cd1289d`..`73f8f6c`). HEADLINE: the galactic surface is DEAD on every
  install — `.galactic.ini` path-concat regression (`da858612c`, 2025-09-20),
  measured by T1/T2/T3 single-variable isolation + 0/66 applogs; repair
  SUSPENDED → §5.37/D29 (rec (1) parse-first-in-one-commit, endorsed + empty-
  system-suppression refinement offered) with the §5.38 rider (7 shipped
  coordinates corrupted by the galactic parser — must fix WITH the path, never
  after). Writer exercised anyway on an authored corpus: gate green 120 bodies/
  17 systems, discriminating both directions; §5.40 FIXED (global-registry
  membership → isInSubtreeOf; 4 foreign-content twins → banner-only); forced-
  scope zero-init FINISHES §11.103(c) one member short of its own comment AND
  ATTRIBUTES §11.89(e)(ii) (uninitialized read, not B30). §5.39 two-parsers
  desync recorded (7 inert divergences). New B30-adjacent signal: solar-moon
  ecl flake 1-in-6 runs at 9.5e-6 rel — gate floor now calibrated in-run with
  a 1e-3 cap (can't silently widen). Supervisor-verified.**

### F6 — B31-design: exhaustive-save design pass (DESIGN ONLY)  [M]
- **Row / recorded:** B31 (§13.B — "needs a design pass before dispatch, NOT
  dispatched blind") · §11.66(b)(d) · §11.55(i) · §11.65 (measurement) · R8 (§11.70) · A29 identity-key hazard.
- **Why load-bearing:** mandated product feature (save→quit→reload as-if continued,
  §2.0 D8 applied to persistence); ONE serialization authority (I2) that B25's
  remaining half and B29's reload semantics both wait on.
- **Task:** produce the design pass the row demands: state inventory (what "ALL
  session state" enumerates), save format + trigger, persistent-body cross-session
  identity key (A29 hazard), write-back under the §11.66(b) contract (preserve
  malformed/comments, inline annotation, atomic rename). Output = design doc in the
  harness repo + enumerated decision points appended to `DECISIONS_PENDING.md`.
  **NO implementation** — implementation dispatch happens only after Vixy reviews.
- **Discriminating check:** n/a (design) — completeness check instead: every §11.66(d)
  clause + R8 + the B16/B29 measured behaviors appear in the inventory with a design
  answer or an explicit decision point. Nothing silently dropped.
- **WIP: DELIVERED 2026-07-25 → §11.110 + `b31-design.md` (harness `e0126a7`;
  code UNTOUCHED at `770ef38a` — design-only honored). 75-row/11-group state
  inventory (39 MUST-SAVE / 8 DERIVED / 12 EXCLUDED / 16 DECISION-NEEDED);
  one-serialization-authority format proposal (4 alternatives rejected, 2 by
  measurement); A29 dissolved for persistent bodies (they become authored data
  per §11.51(a) — the hazard moves to the override ledger, D34); as-if
  operationalized as T1–T10 incl. the §11.101(f) latch prediction. D30–D36
  opened (D31 = a genuine recorded-answer conflict §11.66(a) vs R8, surfaced
  not picked, third reading offered). TWO defects found by measurement on the
  EXISTING save surfaces: §5.41 (`camera action save` cannot succeed as
  shipped — double `anchors/` prepend + 10-day JD round-trip precision) and
  §5.42 (`configuration action save` destroys comments, materializes 9
  unauthored keys, truncates in place). §11.101(i)(3) saveOrbit claim
  corrected (one live old-path caller, unreachable only via §5.41).
  Sequencing recorded: §5.39 → writer rework → persistent-body serialization
  (completes B25's remaining half) → session file; T3 waits on D21; heading
  waits on D28. All recs endorsed (D30(3)/D31(c) with added arguments).
  Supervisor-verified. IMPLEMENTATION STAYS UNDISPATCHED until Vixy reviews.**

### F7 — B4: CameraAnchors implementation per R3  [M–L]
- **Row / recorded:** B4 (§13.B) · R3 (§11.70: on-orbit PRIMARY; body-attached-keeping-angle NEEDED; fixed-point Universe-only) · Q5 (both channels; no cross-session persistence) · FEATURE_REQUESTS 2026-07-21-02 (save-to-anchor.ini command — folded, not promised).
- **Why load-bearing:** S7 spine gate; the design-deciding half is closed, purely
  Fable territory now.
- **Task:** the three anchor kinds per R3, both §2(c) channels (anchor.ini +
  script/command creation); cross-session persistence explicitly NOT needed.
- **Discriminating check:** per-kind live scenes (on-orbit anchor holds through body
  motion; body-attached keeps angle; fixed-point in Universe mode), reversible;
  battery green.
- **WIP: DELIVERED 2026-07-25 → §11.111 (code `2d1387b5`/`3949cf22`/`606d6b87`,
  harness `b81a938`). All three R3 kinds MET with sharp discrimination (orbit
  chord predicted to 2.5e-08 rel while the Moon moved 5886.7 km; frame
  bit-identical vs 87.723° spin, both entries; fixed point bit-identical under
  the same instrument). Both channels dump-identical. TWO class fixes en route:
  §5.43 ModularBodyPtr copy-ctor SIGSEGV (measured, 6th anchor) + hidden-
  reference re-entering draw/pick sweeps (NaN halo). Default tree +10 hidden
  ANCHOR bodies (93→103) with the D9 story recorded (no reach coupling, no
  draw/pick, no-file installs get none; b5_oort path_identity still fires).
  save-to-anchor.ini correctly NOT built (defers to B31's serializer per I2);
  §5.41 untouched (path not built upon); D28 dependency inherited-not-decided;
  `follow_rotation` anchor.ini key spelling awaits sign-off (B28 protocol).
  Supervisor: §11.107's ANCHOR zero-consumer claim annotated (F7 is now ONE
  producer; zero consumers survives re-grep; composed no-key decision
  unaffected). Supervisor-verified.**

### F10 — B40: galactic path+parse repair, one commit  [M]  — DELIVERED 2026-07-30
- **Row / recorded:** B40 (§13.B, opened §11.113(h)) · §11.109 (T1/T2/T3 isolation,
  0/66 applogs) · §5.37/§5.38 (unsuspended, fix TOGETHER, never sequentially) ·
  §5.39 (parse half unblocked) · D29 answer verbatim (DECISIONS_PENDING §11 +
  §11.113(h)).
- **Why first:** dead product surface on every install (17 systems); the §5.38 rider
  is D9-critical (shipped-coordinate corruption must be fixed WITH the path); fully
  decided, bounded blast radius.
- **Task:** path-concat repair + hardened whitespace-tolerant parse in ONE code
  commit, parse fixed first inside it; ONE hardened parse shared by both parsers
  (galactic factory + §5.39 legacy class — I2); comment support in the READERS,
  never emitted into legacy (§2.0 D13); §5.38's 7 coordinates with it. NOT adopted
  (recorded): empty-system anchor suppression — 17 ghost anchors accepted as benign
  and self-explaining.
- **Discriminating check:** §11.109 T1/T2/T3 re-run FLIPS (systems live, applog
  evidence); galactic gate green + discriminating; §5.39's seven keys RE-MEASURED
  before/after (never inherited — §11.113(h)'s own instruction); D9 legacy parity
  93 bodies 0 diffs; md5 pristine; gate-count interactions checked (F7 precedent
  93→103; `b5_oort` path_identity must still fire).
- **WIP: DELIVERED 2026-07-30 → §11.115 (code `2e54be6f` — ONE commit, parse
  first inside it; harness `84f745c` + the delivery commit). §5.37/§5.38/§5.39
  ALL CLOSED. `src/tools/ini_line.hpp` is the family's one line grammar (`#`
  comments anywhere, blanks around `=` insignificant, `=`-less line NAMED per
  §2(f)); four readers on it, the OLD path's `ProtoSystem::load` deliberately
  not, with the reason at the site. Real install: `Params :` **0→18**, nodes
  **0→17**, twins **1→18**, **17/17** at galactic.ini's own coordinates —
  against a path-only counterfactual binary measured at **6/17 WRONG**, exactly
  as §5.38 predicted. Parse half BUILT AND MEASURED ALONE first: 103 bodies,
  **0 divergent fields**, `Params :` still 0 (§11.113(h)'s derived D9 claim
  re-measured, not inherited); nine shipped keys parse differently, **zero
  leading numbers change**; only artifact delta = 2 of 2304 twin entries.
  `b25_galactic` re-pointed to the PRODUCTION names, GREEN (130 bodies/17
  systems) and discriminating THREE ways (pre-binary 18 divergences, `--dotted`
  2, `--mutate` exactly 2); ISO-8859 leg re-pointed STRICTER, not dropped.
  Battery: b24_equivalence 120, b5_oort 11/11 incl. `path_identity`,
  b4_anchors 0 failures, b5_drawhalf 24/24, b24_select GREEN, scenes A–E green,
  frozen md5 in==out on every launch. Anchor suppression NOT adopted (D29's own
  veto), 18 galactic anchors accepted. NEW **§5.45** recorded not fixed and it
  is the successor's first item: a galactic section missing x/y/z ABORTS the app
  at startup (`stod("")`, SIGABRT measured) — reachable BECAUSE of this commit.
  **Supervisor-verified 2026-07-30** (entry, one-commit shape, `protosystem.cpp`
  hunk confirmed comment-only by reading the diff, ledger flips, D29 marked
  implemented, both trees clean; harnesses NOT re-run — committed artifacts +
  three-way discrimination records accepted as evidence). §5.45's
  record-don't-fix call ENDORSED; its fix re-homed as **F12 item (iv)** (batch
  shape, no urgency: reachable only via malformed galactic data no install has).**

### F11 — B39: hidden = as-if-nonexistent (rendered universe only)  [M–L]  — DELIVERED 2026-07-30
- **Row / recorded:** B39 (§13.B, opened §11.113(b)) · D23 verbatim (§11.113(b) +
  DECISIONS_PENDING §6) · §11.76 barrier (its (c) anticipated exactly this) · B32
  precedent (§11.93, cross-launch 0 ULP) · §5.31 · §5.44 (old-path, record-only) ·
  §11.114 (B11 trail obligation + `b11_trail_gate.py` hidden-leg inversion) ·
  §11.56 hidden-half SUPERSEDED (markers at the nodes).
- **Task:** (i) every annotation/derived contribution of a hidden body leaves the
  frame (orbit line, trail DRAWING, hints/labels, axis, grid, selection pointer,
  shadow cast+receive, occlusion, click-pick); (ii) hidden bodies don't tick —
  scope: HIDDEN only (merely-invisible bodies keep their current mechanism; the
  B19/B32 migration is NOT this task); unhide is a USE under the D8 barrier;
  unreconstructible history → resume fresh + LOG (D12); (iii) exposed `hidden` =
  DECLARED value (readout + twin emit declared; render consults effective =
  declared ∨ ancestor); (iv) boundary: command/structural surfaces survive —
  S10.sts (hidden home_planet + select) and W17.sts (live toggle) are the
  discriminators. Old path untouched (§5.44 recorded, never reproduced).
- **Discriminating check:** orbit-line gate red→green (visibility test like its
  TRACE sibling); `b19_hidden_tick.py` stays green through a use channel —
  RE-VERIFIED, not inherited (§11.113(b)(iii)); `b11_trail_gate.py` hidden-leg
  INVERTS in the same commit; S10/W17 scenes live before and after; battery green.
- **WIP: DELIVERED 2026-07-30 → §11.117 (code `5b488e84` instrument +
  `a62ad5d7` behaviour; harness `83b4c0a` RED baseline, `6521c40` green, +
  the delivery commit). §5.31 CLOSED on its own discriminator (hidden orbit
  line 779 → 0 px, control 779 px). Implemented STRUCTURALLY: `hide()` takes
  the parked subtree out of `sortedSystemBodies`, so ten contributions leave
  the frame in one edit (I6). DECLARED (`relation`) vs EFFECTIVE
  (`renderHidden`), one writer for flag + membership ⇒ §5.44 impossible here;
  nested-hidden pointer 137 → 0 px with the child's declared flag untouched.
  Tick RETIRED and MEASURED (2074 → 0 evaluations / ~2070 frames, new
  `evalCount`), replaced by the §11.76 barrier with the use channels
  enumerated; "no tick" bounded by the reference chain, which IS
  §11.113(b)(vi). Trail reconstruction from the orbit: 64 samples ==
  predicted, on-orbit to 0.0315 % of an h/r prediction from the run's own
  radii. D11 measured BOTH ways against the pre-fix binary: default 42.26 →
  42.01 µs (inside the ±1.9 µs repeatability), 0.217 µs/parked body/frame ⇒
  ~2.2 ms/frame at D10's 10 000. S10/W17 live BEFORE and AFTER, one fresh
  launch per leg. b11 hidden column INVERTED same commit (95/95), b19 RE-RUN
  59/59 and sharper. Battery green; md5 pristine. Residue: **D37** (hidden
  star still illuminates — measured, Vixy's), **§5.46** (up-chain never
  writes `matLocalToBodyPos`), pre-existing reload loss of 2 hidden bodies
  (attributed on the pre-fix binary). Deviations flagged in §11.117(l).
  **Supervisor-verified 2026-07-30** (entry read in full; the structural
  class-proof ACCEPTED for the un-measured contributions — one list, all
  sweeps observed to walk it at source, the measured observables verify the
  removal itself; trees clean; harnesses not re-run — red→green artifacts +
  in-run controls + pre-fix-binary counterfactuals accepted as evidence).
  Deviation (2) ENDORSED (Reason>Rule: §5.31's literal shape overshoots into
  out-of-cone orbit-line deletion); deviation (1) noted, verification intact.
  D37 endorsed (1) with the shadow-asymmetry datum appended; §5.46 and the
  reload-loss attribution accepted as recorded.**

### F12 — decision-implementation batch: B27-split + B24-click + B33 readout  [S–M]  — DELIVERED 2026-07-30
- **Row / recorded:** B27 (§11.113(f) — D27 OVERRIDES the ratify-as-is rec+endorsement:
  split `light_source` + `primary`) · B24 (§11.113(e)/D26 — visible child takes the
  click on its own pixels) · B33 (§11.113(g)/D28 rider — `CoreLink::getHeading` reads
  the old path).
- **Task:** (i) B27: per-site assignment by the derived dark-primary test (listed
  §11.113(f); the two flagged for a second look: hint suppression, `system_star`);
  legacy-star twin emits BOTH keys (value-for-value); `b24_equivalence.py` field
  list carries the split; (ii) B24: D26 rule across the old/new seam — child takes
  the click when it lands ON the child; `b24_select.py` P6 INVERTS in the same
  commit, re-pointed never loosened; (iii) B33: heading readout reads the drawn
  roll (`set heading 0` remedy stands); (iv) [added 2026-07-30 from §11.115(i)]
  §5.45: guard `name`/`x`/`y`/`z` in `SSystemFactory::loadSystem` — warn naming
  the section and the missing key, skip the section (the same function's own
  anchor call is the precedent; §2(f)/D12); (v) [added 2026-07-30 from
  §11.115(i)] `b10_cmd_battery_run.sh` stale md5 echo (`62239656…` vs pristine
  `545a51ef…`) → assert like the F0-fixed runners (§11.101(g) class).
- **Stop boundaries (NOT yours):** A17 residual (i) (old candidate set waits for old
  picker retirement); §11.92(d) heading-offset coupling (suspended, Vixy's);
  `instanced` key (waits row 5/S4).
- **Discriminating check:** (i) counterfactual table per moved consumer (§11.107's
  9-leg pattern) + D9 legacy parity; (ii) P6 inversion + old-body parity legs stay
  identical outside the child's pixels; (iii) readout matches drawn roll where the
  paths diverge (§5.26/`s526_ref.py` instrument, 39 090 px case); (iv) the §5.45
  repro (one `z` line removed) flips SIGABRT → warn+skip+start, and the
  well-formed corpus is byte-inert; (v) the runner FAILS on a mutated ssystem md5.
- **WIP: DELIVERED 2026-07-30 → §11.118 (code `41f68121` / `62b860d1` / `68967c01` /
  `f1151c63`; harness `e09837c` / `a1f9796` / `cbaf115` / `260af49` + the delivery commit).
  5/5 items met, each committed at its own green checkpoint.** (i) D27's split landed:
  `light_source` = illumination, `primary` = the structural remainder as its OWN MEMBER
  (a second `BodyType` bit would break `isMinorBody()`'s exact-equality test and the
  value-for-value tie to legacy `strToBodyType`). Both mandated second looks taken and they
  disagree: the **HINT gate MOVES to `primary`** (the skip exists because the body sits at its
  parent's ORIGIN — structural; a dark primary would lose its hint forever under a luminosity
  gate), `system_star` **CONFIRMED** on `light_source`; the map's "two shadow-sweep exclusions"
  is **three** at HEAD. 10-leg counterfactual with the halves moving independently, a dark
  primary on Earth un-satelliting the Moon, a `compose = deduced` pair (Earth deduces NO TRAIL),
  the same bytes answered differently by the pre-split binary; screen legs hint **545 px>8 /
  0 px floor** and orbit master flag **1183 px>8 == its own positive control**; D9 parity **120
  bodies / 0 divergent fields**, twin delta **exactly one line**; `b24_equivalence` 120 GREEN and
  RED on the new `--strip Sun:primary`. (ii) D26: the new-only route is asked BEFORE `cleverFind`;
  P6 INVERTED in the same commit + **P6b** (the bound — same disc 0.224 NDC off the child ⇒
  parent) + **P6c** (second traversal of the seam); pre-binary RED on exactly one leg; 90-name
  parity identical; residual 52 px>8 in a pre-click frame attributed to §11.106(g)'s az
  bistability (same-binary A/A 0 px>8). (iii) D28 rider: `getHeading` asks WHICH PATH DRAWS;
  drawn **−6.160074°**, fixed reports **−6.160070**, pre-fix **0.000000**; the semantic no-op
  `heading delta_azimuth 0` moves the drawn view **0 px>32** against **39 114**; `set heading 0`
  remedy intact (39 090 → 12). (iv) **§5.45 CLOSED** — predecessor code VERIFIED not inherited
  (its `stod("1,5")` claim measured FALSE and corrected); rc **−6** → warn+skip+start; NEW datum:
  a missing `name` never aborted, it built a system node called `System`; byte-inert on the
  shipped corpus. (v) runner asserts with exit codes (live **0** pristine / **4** wrong corpus),
  7/7 on the verbatim-extracted tail, and its byte-for-byte clone collapsed to a wrapper (I2).
  Two shared-instrument defects fixed at the root: `load_dump` silently DROPPED any body carrying
  a `nan`, and screen legs ignored the app REJECTING a flag name. Battery green (scenes A–E,
  b5_oort 11/11, b25_galactic 130/17, b4_anchors 0, b24_equivalence 120, b24_select incl. P6c);
  md5 pristine throughout. NEW **§5.47** (`get status position` never replies) + **§5.48**
  (§11.18 ASmooth cold-launch NaN, attributed to neither binary by a 0/4-vs-0/4 repeat).
  Deviations flagged in §11.118(j); nothing suspended for Vixy.
  **Supervisor-verified 2026-07-30** (11.118 read in full; commits/authors/both trees
  checked; §5.45 FIXED flip + §5.47/§5.48 stubs + D26/D27/D28 implementation marks
  verified in the ledger; harnesses NOT re-run — committed artifacts + both-ways
  discrimination records accepted as evidence). Deviations (1)–(4) ENDORSED: (1)
  applies §11.113(f)'s own mandated second look with the argument at the site and a
  cheap veto (twin-only key); (2)(3) are I2/I6 operating; (4) closes
  pass-for-wrong-reason paths. Supervisor fixed two artifacts of this session en
  route: the stray `---` splitting INTENT §5 between rows 46/47, and the F13 header
  (root = supervisor edit truncation, see F13's note).**

### F13 — B31-impl slice 1: the §11.66(b) writer rework  [M]  — DELIVERED 2026-07-30
*[Header restored 2026-07-30 by the supervisor. Root cause of the loss: a supervisor Edit
whose old_string swallowed the authored `###` line while its replacement dropped it
(edit-truncation; caught by F12's executor, who reconstructed a provisional header and
correctly refused to invent the lost size tag). Original title + size **[M]** restored from
the authoring context — the F12 report's "size estimate was never written" is hereby
corrected: it was written, then destroyed by the same edit.]*
- **Row / recorded:** B31 (§13.B — DISPATCHABLE since §11.113(i)–(o)) · `b31-design.md`
  §5.2/§5.3/§5.4 (the line-level design — the authority for this slice) · §11.66(b)
  (the contract) · §11.113(n)/D35 (composed + session files ONLY; legacy `ssystem.ini`
  READ-ONLY forever) · §2.0 D13 (nothing written into a file an older parser reads) ·
  §11.115 (F10's `ini_line.hpp` = the one line grammar this builds on) · §5.42
  (counterexample surface: what a writer must never do) · §11.109(e)(h2).
- **Why now:** §11.110's own sequencing (§5.39 parse → **writer rework** →
  persistent-body serialization → session file); the writer is also the dependency of
  B25's remaining half (script-triggered system save needs the same writer).
- **Task:** implement b31-design §5.2 exactly — `Section` promoted to an ordered line
  list (COMMENT/BLANK/KEY/RAW), parse keeps EVERY input line in order, write emits in
  order; changed values rewritten in place preserving key text/spacing verbatim; new
  keys append at section end; removals comment-out-with-reason, never delete; side
  index derived, the line list is the authority (I2). Plus §5.3's annotation half:
  loader-produced annotation set keyed (section, key, reason), written ABOVE the datum
  (source-forced — trailing comments are swallowed into legacy values), `#!sc:` stable
  marker, idempotent. Verify at HEAD what F10 already unified vs what §5.3's
  "loader adopts the format parser" still requires — re-measure, never inherit.
- **Stop boundaries (NOT yours):** the session file (§3.2), the ledger (§2 group D),
  persistent-body serialization (§4.1) — later slices; §11.109(h2) whitespace-key
  format semantics stays SUSPENDED; §5.41/§5.42's own surfaces are NOT reworked here
  (they re-express on the session serializer later — record, don't fix); anything
  D21/D28-dependent.
- **Discriminating check:** T9 (b31-design §6.2): rewrite-twice byte-identical; a
  round-trip corpus leg with comments + malformed (RAW) + unknown keys + irregular
  spacing — byte-identical where unchanged; a value-change leg preserving layout; an
  annotation leg showing the annotation lands ABOVE the right datum and REPLACES its
  own previous instance while human comments survive; legacy `ssystem.ini` md5 in==out
  asserted (D13); `b24_equivalence` + `b25_galactic` green (the twin still generates)
  and discriminating.
- **WIP:** — *(delivered: §11.119; code `9f60d3a1` + `99685172`, harness `a8a198f`
  + the entry commit. 12 gate legs green, discriminating two ways measured;
  18/18 twins byte-identical; b24 120 + RED, b25 130/17 + RED, b40_parity 17/17,
  b4_anchors 0. **The next B31 slice** is persistent-body serialization (§4.1),
  which also carries the LIVE loader→writer annotation wiring — it needs a write
  trigger to exist before a loader may retain its parsed sections — and completes
  B25's remaining half. **Supervisor-verified 2026-07-30** — 11.119 read in full;
  trees/commits checked; harnesses not re-run, committed artifacts + three-way
  discrimination records accepted as evidence. Both judgment calls ENDORSED: the
  annotation-producer deferral MATCHES D33 (explicit-save only — a load-time
  rewrite is decided-against, not undecided; the producer wires into the slice
  owning the explicit trigger), and the unmarked removal-reason lines are the
  correct fixed point (a machine-marked removal record would delete itself on the
  next write; the second-pass leg proves it). cLog pre-openLog trap accepted as
  entry-recorded (unreachable at HEAD).)*

### F14 — B14/D22: the 90° meridian conversion fix  [S–M]
- **Row / recorded:** B14 (§13.B, REOPENED §11.101(b)) · §11.113(a)/D22 verbatim (A33
  closed) · §5.28 · §11.86/§11.88 (the W0 machinery being corrected) ·
  DECISIONS_PENDING §6 D22.
- **Why:** decided + queued by session 3; user-visible on 3 shipped bodies (Iapetus,
  Amalthea, Proteus render 90° off); bounded blast radius (one conversion site).
- **Task:** fix in the CONVERSION (`ModularSystem.cpp:852-858` locus — re-locate at
  HEAD), NEVER the 20 bodies' data; `u = 0.5` (image centre) IS the convention,
  textures NOT re-registered; the `+π/2` fudge STAYS at `getAxisRotation()` and the
  conversion accounts for it; planet rows untouched. Code-only — both data files'
  md5 in==out.
- **Discriminating check:** MUST anchor on the TEXTURE (dark-centroid `u`), never the
  conversion's own axis — §11.113(a)'s own instruction (the existing gate is blind by
  construction). Predict the post-fix dark-centroid `u` from texture + IAU meridian
  BEFORE the fix run, then measure, on all three textured bodies; the 17
  placeholder-textured moons + all planets unchanged; the D22 Mercury control
  (`rot_pole_w0` added in a TEST corpus — surface rotates 90° pre-fix, 0° post-fix)
  is available as the cheap counterfactual; old path untouched; battery green.
  Record-only, NOT yours: the Jupiter `rot_rotation_offset` 107-shipped-vs-60-loaded
  divergence (§11.101(b)).
- **WIP: DELIVERED 2026-07-30 → §11.120 (code `f11f6a4e`; harness `df80f15` / `cc90444` /
  this commit).** All DoD items met. The fix is one operand swap in `resolveRotationFrame`
  (`atan2(-(pm·ex),pm·ey)` → `atan2(pm·ey,pm·ex)`), code-only, both data files' md5
  unchanged (`c4b426df` / `545a51ef`). Texture-anchored gate `f14_meridian.py` (RED on the
  pre-fix binary, `expect=` selects which): IAU-meridian texture column
  0.75000/0.75002/0.75000 → 0.50000/0.50002/0.50000, pixels selecting that model over
  ±90/180 by 0.461/0.873/0.527 and matching the image predicted BEFORE the fix
  (`f14_predict.py`, committed at checkpoint 1). Only-movers: exactly the 20 keyed bodies at
  +90.000000°±1.6e-5, 12 hopped controls bit-identical, obliquity/ascNode/period ULP-0 on 32.
  Mercury counterfactual (`f14_mercury.py`, temp-HOME farm): −90.080° pre, −0.080° post,
  binaries pixel-identical on the unmutated corpus. §11.69 Iapetus 0.000008°/0.000000°/
  19.6174°. Battery green (b24 120 + `--strip` RED, b25 130/17, b40 17/17, b4 0 failures,
  b10_cmd battery exit 0, scene E 26 OK / 0 FAIL). **Deviations/finds:** the task's
  "placeholder textures are featureless" premise is MEASURABLY FALSE (σ 8.15/13.33 vs
  Iapetus 51.51) — the 17 DO move pixels and that is correct, `f14_placeholder.py` measures
  it; two harness instruments carried the same wrong axis and were corrected (scope
  expansion, recorded §11.120(g)); NEW **§5.49** opened, not fixed — the observer's
  longitude origin is the mesh x̂ column, 90° from the map's centre column
  (derived-from-source, NOT measured at the render, veto point recorded).
  **Supervisor-verified 2026-07-30** — 11.120 read in full; trees/commits checked;
  harnesses not re-run, committed pre-fix predictions + both-ways-RED gate + corpus
  anchor accepted as evidence. All deviations ENDORSED: the premise correction is
  measurement doing its job (unregistered ≠ featureless — the corrected word is now
  the record's); the instrument corrections are I2 operating on the §5.28 root; the
  Mercury counterfactual finally run is D22's own named check. §5.49's
  record-don't-fix call ENDORSED (channel not given, veto + owed test stated).

### F15 — B31-impl slice 2: persistent-body serialization (§4.1) + live annotation wiring  [M–L]
- **Row / recorded:** B31 (§13.B — slice 1 landed §11.119; the row's own text names THIS
  slice next) · `b31-design.md` §4.1 (the authority: a persistent body becomes AUTHORED
  DATA — no new identity key; D34's ledger key is NOT this slice) · §11.119(b)(i)
  (annotation mechanism complete with ZERO producers; the producer wires into the slice
  owning the explicit write trigger — supervisor endorsement: load-time rewrite is
  decided-AGAINST per D33, not undecided) · B25 remaining half (§11.109(b) + §11.119
  rider: the writer is ready; the LIVE-TREE SERIALIZATION SOURCE is what is missing) ·
  §11.51(a) verbatim route (*"save a system on-the-fly as well by targeting without the
  .disabled or under a different name from scripts"*) · D33 (explicit-only triggers) ·
  D35 + §2.0 D13 (composed + session files ONLY; legacy read-only forever) · A32 (the
  target file is USER-owned ⇒ write-back governed by b31-design §5 + D35).
- **Why now:** §11.110's own sequencing (§5.39 ✓ → writer rework ✓ §11.119 → **this** →
  session file → ledger); it completes B25's remaining half and takes the annotation
  layer from mechanism to product.
- **Task:** (i) the live-tree serialization source: walk the live tree, emit each body's
  declared parameters + live capability keys through the F13 writer;
  `generateComposedTwin` is 80 % of it (§4.1); the gap = bodies with NO legacy section
  (runtime-created, no `stringHash_t`) — parameters come from the loaders that consumed
  them (the `saveOrbit()` pattern: DATA KEYS, never derived state —
  `SurfacePointOrbitLoader.hpp:47-60` is the exemplar; re-locate at HEAD); (ii) the
  explicit save trigger, BOTH §2(c) channels (script/command), per §11.51(a): target a
  composed file without `.disabled` or under a different name; spelling landed + recorded
  as a veto point (B28 protocol; F7 `follow_rotation` precedent); (iii) script-pushed
  bodies thereby survive sessions as ordinary authored data — if the push channel itself
  does not reach the NEW tree at HEAD, minting the minimal one is IN scope (the mandate's
  own words: *"script-pushed persistent bodies that survive sessions"*), measured first;
  (iv) LIVE annotation wiring: composed loads RETAIN parsed sections, loader diagnoses
  annotate them, annotations are WRITTEN only when an explicit save targets that file
  (never at load — D33).
- **Stop boundaries (NOT yours):** the session file (§3.2) + override ledger (§2 group D)
  — later slices; §5.41/§5.42 surfaces (re-express on the session serializer — record,
  don't fix); anything D21/D28-dependent (T3, heading); §11.109(h3) same-name push
  collision (record, never decide); §11.109(h2) whitespace-key semantics (SUSPENDED);
  legacy `ssystem.ini` never written — md5 in==out asserted every launch.
- **Discriminating check:** T6 (b31-design §6.2): a script-pushed body EXISTS after
  quit + fresh relaunch with same parent/relation/module set/routing — comparator
  IMPORTED from `b24_equivalence`, never copied (I2, §11.109(b)); no-runtime-delta
  control: the live-tree save of the shipped solar tree loads back b24-equivalent, its
  delta vs the twin explained line-by-line or zero; save-over-existing preserves author
  content (the F13 gate class on the LIVE path; T4's fixed-point rule: second save
  byte-identical); annotation leg: a diagnosed datum gains its `#!sc:` line ABOVE the
  datum only AFTER an explicit save, idempotent on re-save; §6.3 D9 assert: md5 of every
  data file the save did NOT target unchanged; battery green (b24_equivalence,
  b25_galactic, b40_parity, b4_anchors, scene E) + twin byte-stability (18/18).
- **WIP: DELIVERED 2026-07-30 → §11.121 (code `5a8881ff`; harness `121b238` /
  `c676cf3` / this commit). All DoD items met.** The live tree is a serialization
  source: every body keeps the map that DECLARED it (`declaredParams`, snapshotted
  before the loader's own `map[]` reads author what nobody wrote), and `body action
  save [filename <name>]` — spelling recorded as a **veto point** (B28) — walks the
  subtree (hidden included, stopping at nested systems) writing one declaration per
  body that carries one; 2 of 93 engine-minted bodies correctly skipped. An existing
  file is EDITED not rebuilt; a declaration it already carries is NOT edited (that is
  the ledger's slice). Composed loads KEEP their parsed sections ⇒ 14 diagnostic sites
  now say their verdict once, into the log AND above the datum, written only by an
  explicit save (D33). **Gate `f15_persist.py` 25 legs GREEN**: T6 121 bodies / 0
  divergent fields vs its own no-save control (120, gone); live-tree save == machine
  twin **line for line** (4160) and +32/−0 with a body pushed; T4 byte-identical in
  both regimes; 4169 authored lines preserved; annotations idempotent + negative
  control (5→3); D9 8077 files / 1 written; refusals mute nothing; **SCREEN** 118 of
  8462 footprint px with **1** surviving a 2 px erosion (edge = all a cross-launch
  jitter can move), counterfactual 98 %. `--mutate` RED exactly on modules/routing.
  Battery: b24 120 + `--strip` RED, b25 130/17 + `--mutate` RED, b40 17/17 (18 Params,
  frozen md5 in==out), b4 0 failures, scene E 26/0; twins **18/18** byte-identical;
  frozen md5 pristine throughout. **B25 CLOSED**; B31 open on the session file + ledger.
  **Supervisor-verified 2026-07-31** (11.121 read in full; trees/commits/authors
  checked; B25 close + B31 annotation + §5.50 stub verified at the ledger; diff
  surface confirmed new-path/factory/command-seam only — `protosystem.cpp` untouched,
  old render path unreached; harnesses NOT re-run — committed artifacts + both-ways
  discrimination records accepted as evidence: T6 vs its no-save control, `--mutate`,
  twins 18/18, the D9 8077-file sweep). Deviations ENDORSED: screen leg (verification
  height; the footprint-split + erosion criterion avoids the cross-launch-floor trap
  the entry itself cites), b40_parity vacuous-gate fix (§11.101(g) class, at the
  instrument producing this task's evidence), deduced-module-order source (order is
  semantic, §11.19). §5.50 record-only call ENDORSED (fix shape named — old-path
  bail-out with §2(f); next-batch candidate, the §5.45→F12(iv) pattern). Leg-count
  note: this WIP's "25 legs" vs §11.121(g)'s "26" — the entry wins by this file's
  own authority rule. Veto point → §3: the `body action save [filename <name>]`
  spelling. Foreign same-host app launch observed mid-task (§11.121(j)) → §3
  (shared `~/.spacecrafter` is a standing md5 confound across sessions).
  NEW **§5.50** (record-only): `body action load … coord_func surface_point` kills the
  app — the OLD path carries on with a null orbit (`protosystem.cpp:612-622`).
  Instrument fix: `b40_parity.py` no longer passes vacuously.**

### F8 — B7-hunt-3: stress-modulated teardown hunt  [S, budgeted]
- **Row / recorded:** B7 (§13.B) · §11.95 (158/0, contention hypothesis, `b7_hunt.sh` ready) · §11.97(f) (NEW abort-path datum, composed-body-count axis).
- **Why load-bearing:** open crash-class defect; lowest rank because the negative
  record is strong and the yield is probabilistic — dispatch opportunistically.
- **Task:** same mix under reintroduced stress load ± ASan, adding the composed-body-
  count axis (2 vs 8 rovers, §11.97(f)); budget N ≥ 50 teardowns per variant BEFORE
  concluding; foreground batches only. Hygiene rider: remove/ignore the empty
  `harness/b7_batch_0*` leftover dirs (untracked noise from the §11.95 run).
- **Discriminating check:** positive-controlled detectors (as §11.95); any fire ⇒
  full context capture via `b7_probe.gdb`; no fire ⇒ tightened bound recorded.
- **WIP: DELIVERED 2026-07-31 → §11.122** (code `1e44b639`, harness `6d68edb` + the
  delivery commit). Both variants concluded at N=54 (2-rover, 8-rover): **108
  teardowns / 0 fires** at measured loadavg 15.5–21.8 vs §11.95's 0.22 ⇒
  contention-as-saturation EXCLUDED down to ≤2.8 % pooled; §11.97(f)'s 8-rover
  abort did not reproduce (0/54). ASan was run after (1)(2) because §11.95(b)'s
  "multi-hour rebuild" premise measured **83 s** — and it found a
  **heap-use-after-free on every shutdown on shipped data**
  (`EnvironmentManager.cpp:28`), FIXED with a both-ways check. NEW §5.51 (no
  virtual `~BodyModule`, 373 skipped destructors/shutdown) recorded-not-fixed.
  OPEN unattributed residual: 2 HUNG in one post-fix arm, non-reproducing across
  a second post-fix arm and a pre-fix A/B arm. B7 STAYS OPEN; next hunt = the
  ASan tree over a full mix, not more load.
  **Supervisor-verified 2026-07-31** (11.122 read in full; trees/commits/authors
  checked; the fix diff READ — exactly as claimed, two logic lines with the reason
  at the site citing I5; §5.51 stub with its owed discriminating check verified at
  the ledger; harnesses not re-run — positive-controlled detector records +
  per-teardown load columns + committed artifacts accepted as evidence). Deviations
  (1)–(5) ENDORSED — (3) is the §5.2 re-verify-cached-conclusions rule operating
  exactly as intended (the false "multi-hour" premise was the only thing between
  the row and its defect); (5) a landed fix with an unmeasured regression signal
  would be worse. The §11.122(i) handling ENDORSED: both theories stay live
  (binary vs load cold-start transient), the fix STAYS (no mechanism reaches a
  pre-teardown draw stall from a teardown-only dtor; reverting would reintroduce
  a MEASURED freed-memory write), the mandated pre-warmed N ≥ 50 re-measure is
  the closure path. Next-hunt sequencing per §11.122(m) adopted: **§5.51 fix
  FIRST as its own dispatch (F16 candidate — it masks all teardown measurement),
  then ASan mix N ≥ 50, then TSan; stop buying load.**

### F9 — B12: near-surface star family — DESIGN-FIRST  [L]
- **Row / recorded:** B12 (§13.B, unblocked by Q21) · §11.44 · §2(a) (dark-Sun-disc divergence legitimised as a defect of old, i.e. new must do better).
- **Why load-bearing (and last):** retires a named user-visible divergence, but it is
  regime + module-family design (architecture-adjacent) — the cost of a wrong family
  shape exceeds the divergence's current cost, so it ranks below everything above.
- **Task:** design note FIRST (regime boundary vs G4 thresholds, family shape, I4 home
  for limb-darkening/granulation/chromosphere per Q21), recorded to the ledger with
  veto points; then the minimal slice that retires the dark disc (limb-darkened
  emissive), spots/granulation as content later.
- **Discriminating check:** near-Sun approach scene — disc no longer dark, limb
  darkening measurable radially; far-regime bit-inert (family gated off).
- **WIP:** — **DELIVERED 2026-07-31 (§11.123; design note `claude/b12-design.md`;
  code `3f137ef9`/`be6b2a84`/`a958c05e`).** Both phases: design first, then the
  slice that retires the dark disc. §13.B B12 flipped (open for CONTENT only);
  §11.44's named residual closed; §5.52/§5.53/§5.54 opened (record-don't-fix).
  Battery green: b24 (120 bodies) + its RED leg firing, b25 (130 bodies /
  17 systems, twins 18/18), b40, b4, scene E 26/26, f15_persist, b12_rare 15/15.
  **Supervisor-verified 2026-07-31** (11.123 read in full; design note structure
  checked against all seven mandated items, design-before-code confirmed by
  timestamps 02:42 vs 03:48 — the F6 discipline held; trees/commits/authors
  checked; diff surface new-path only, old `body_sun.frag` untouched, pool fix
  confined to `PipelineRegistry.cpp`; §5.52–§5.54 stubs + B12 flip verified at
  the ledger; harnesses not re-run — both-ways law refutation on the pre-change
  binary + committed prediction-first artifacts accepted as evidence). Deviations
  ENDORSED: (1) the pool per-set-capacity fix is the named defect class one
  dimension over, forced by the task's own zero-validation bar, 7→0 measured;
  (2) the star-gated (not distance-gated) inertness re-derivation FOLLOWS THE
  AUTHORITY over this section's own paraphrase — "far-regime bit-inert (family
  gated off)" above was written under the distance-gate reading the design
  refutes with physics + the row's own 1 AU acceptance measurement (the F3
  A8/A9 drift class: the row wins, the view was stale); (3) the 2.5 R☉ scene
  move is single-variable discipline, §5.53 records the trap. The 2×2 inertness
  argument endorsed (pre|post2 = 0 px proves the binary is not the
  discriminator; the one outlier launch is a same-binary A/A effect, B30
  class). D11's GPU half is derived-not-measured — limitation stated in-entry,
  structural-subset argument endorsed. Veto points V1–V4 + content decisions
  D1–D4 + the chromosphere grammar word → §3.

### F16 — teardown-integrity batch: §5.51 virtual `~BodyModule` + §5.50 push-channel bail-out  [M]
- **Row / recorded:** §5.51 (opened §11.122(h); owed check stated in-row) · §5.50 (opened
  §11.121; mechanism at `protosystem.cpp:612-622` — re-locate at HEAD) · §11.122(m)
  sequencing verbatim: *"Fix §5.51 first in its own dispatch — 373 skipped destructors
  per shutdown is a large enough blind spot that any further teardown result is measured
  through it"* · B7 row (§13.B) next-hunt note · §11.122(e) (`build-asan` tree, 83 s,
  `B7_BIN` — deliberately not `SC_BIN`).
- **Why first:** §5.51 masks the instrument every teardown task reads (373 skipped
  dtors/shutdown, 746 on a `qda` cycle); B7-hunt-4 is blind until it lands. §5.50 is the
  same integrity class with its fix shape already named in-row (the §5.45→F12(iv)
  precedent) and shares the verification apparatus.
- **Task:** (i) §5.51: `virtual ~BodyModule() = default;` (`BodyModule.hpp:149` —
  re-locate at HEAD) + the regression battery the row's own text demands: 13 subclass
  dtors run for the FIRST time ever, releasing Vulkan resources in an order nothing has
  exercised — hunt the fallout, never assume it. QUIET host on purpose: the §11.122(i)
  HUNG question is F17's; a quiet batch compares against §11.95's own quiet 158/0
  without touching that confound. Decision-free dtor-order defects found en route may be
  fixed with both-ways checks (the §11.122(g) precedent); anything larger → record.
  (ii) §5.50: bail out of the OLD path's `addBody` when the created orbit is null —
  warn naming the body, the unknown `coord_func` and the valid values (§2(f)/D12), skip
  the body, app survives; the new path's consumption of the same push map unchanged.
- **Stop boundaries (NOT yours):** the hunt itself (F17: sanitizer mix, §11.122(i)
  closure); §5.52–§5.54 (F18); any teardown-order redesign beyond making the declared
  dtors run (a found defect whose fix needs design → record, own dispatch).
- **Discriminating check:** (i) the §5.51 row's own: same ASan cycle,
  `new-delete-type-mismatch` **373 → 0** (and the `qda` leg 746 → 0), heap-UAF stays 0,
  shutdown rc=0; NO new ASan report class on teardown; validation layer POSITIVELY
  confirmed + 0 VUID through shutdown (resource-release order is validation-visible);
  native quiet teardown batch **N ≥ 50** across the §11.95 mix: 0 fires / 0 hangs
  against the quiet 158/0 baseline. (ii) the §5.50 repro command (in-row, verbatim)
  flips kill → warn+skip+app-lives, log line actionable per §2(f); a composed-file
  `surface_point` body still draws (b24_screen rovers = the standing control); a valid
  push-channel body still lands. Battery green (b24_equivalence, b25_galactic,
  b40_parity, b4_anchors, scene E, f15_persist); frozen md5 in==out.
- **WIP:** — **DELIVERED 2026-07-31 → §11.124** (code `0dd0a938` §5.51 + `ebb41ab2`
  §5.50; harness `732386e`/`39b0ac9`/`fef791b`/entry). §5.51 and §5.50 both
  CLOSED at §5. ASan 373/746/373 → 0/0/0 with no new report class, rc=0,
  re-measured on the delivered binary; validation layer positively loaded, its
  channel located (`vulkan.log`) and shown able to fire, 0 VUID on both teardown
  entries after a completed mid-session reload; native quiet batch **54/54
  CLEAN, 0 fires, 0 hangs**; battery green (b24_equivalence 120, b25_galactic
  130/17, b40_parity 18 twins, b4_anchors 0 fail, scene E 26/0, f15_persist all
  green); frozen md5 in==out. NEW **§5.55** (drawing the rotation axis once kills
  the process at exit — deterministic both ways on BOTH binaries, SIGABRT
  natively / SIGSEGV under gdb: the first §11.15d-class fire that reproduces on
  demand) and **§5.56**, both record-don't-fix. For F17: post-F16 binary is
  `ebb41ab2`, `build-asan` rebuilt at it (re-run `cmake .` there after any new
  source), and the teardown ASan baseline is now SILENT.
  **Supervisor-verified 2026-07-31** (11.124 read in full; trees/commits/authors
  checked; BOTH code diffs READ — the dtor + its WHEN-bound comment, the
  null-orbit guard after the whole determination block with the I2 duplication
  named at the site; §5.50/§5.51 FIXED flips + §5.55/§5.56 stubs + B7-row
  annotation verified at the ledger; harnesses not re-run — both-ways ASan
  table re-measured on the delivered binary + positive-controlled validation
  channel + committed artifacts accepted as evidence). Deviations (1)–(5) all
  ENDORSED: (1) names the §11.122(b) instrument gap instead of rediscovering
  it; (2) is single-variable isolation with §5.55 exercised in its own pair;
  (3) buys a true A/B for one rebuild; (4) honors the stop boundary and §5.56's
  no-safe-instrument argument carries a positive control; (5) is the honest
  form. §5.55's record-only call ENDORSED (fix is teardown-ORDER design — F19
  candidate with the coverage hole it names). Supervisor consequence for F17
  carried into its dispatch: §5.55 = one-launch positive control; line-family
  flags stay OFF in hunted cycles until §5.55 is fixed (a known deterministic
  fire would contaminate every axis-on cycle).

### F17 — B7-hunt-4: the sanitizer mix + the HUNG settle  [M–L]
- **Row / recorded:** B7 (§13.B) · §11.122(m) verbatim: *"run the ASan tree (83 s to
  build, `B7_BIN` ready) over a full N ≥ 50 mix, since it sees the silent half of the
  race that 266 native teardowns could not; then TSan for the thread half"* + contention
  *"stop buying more of it"* · §11.122(i) closure path verbatim: *"re-measure HUNG rate
  on both binaries at N ≥ 50 with the stress load pre-warmed"* · §11.122(c) detector
  discipline (positive-controlled through the SAME classifier) · §0.5 concurrent-
  instance assert + memory-bounded builds.
- **Why after F16:** §11.122(m)'s own sequencing — any teardown result before the §5.51
  fix is measured through 373 skipped destructors. F16 also re-bases what "post-fix"
  means: state the HUNG-arm binary pair explicitly, never inherit §11.122(i)'s pair.
- **Task:** (i) ASan mix, N ≥ 50, full §11.95 9-variant mix, `B7_BIN` = `build-asan`
  binary at post-F16 HEAD, quiet host (the sanitizer sees the silent half regardless of
  load — (m)); any report = a find, attributed at source; decision-free one-line fixes
  with both-ways checks allowed (§11.122(g) precedent), larger → record. (ii) TSan tree
  (own build dir, gitignored like `build-asan`) over the same mix; N ≥ 50 if runtime
  permits, else the achieved N + bound recorded honestly — no silent cap. (iii) settle
  §11.122(i) per its own closure path: stress load PRE-WARMED to steady state before
  cycle 1 (the one measured difference the entry names), N ≥ 50 per arm, arms =
  post-F16 HEAD + the §11.122 pre-fix reference; BOTH live theories (binary vs
  cold-start transient) stated with predictions BEFORE the runs.
- **Stop boundaries (NOT yours):** F16's items (landed); more load-buying (excluded by
  (m)); non-teardown defect classes TSan surfaces in draw paths → record, don't chase;
  cadence instruments (the §11.123(o2) wall-clock check is owed by a cadence-TOUCHING
  task — a 45 s timeout detector is not one; do not convert this hunt into it).
- **Discriminating check:** every detector trusted for silence shown able to FIRE in
  the regime it is trusted in, through the same classifier (§11.122(c) pattern; TSan
  needs its OWN positive control); fires ⇒ context capture (`b7_probe.gdb` armed);
  no fires ⇒ tightened bounds per arm (rule of three); (iii) closes only with magnitude
  AND mechanism predicted from the attributed cause (§11.122(i)'s own rule) — else it
  stays open with the new record appended.
- **WIP:** — **DELIVERED 2026-07-31 → §11.125** (code `550b3f9f` — `.gitignore` only, no
  source change; harness `73048f7`/`befdfba`/`3604b19`/`bec15ce`/delivery). (i) **ASan mix
  N = 54: 54 CLEAN, no report of any class**, full 9-variant mix, `halt_on_error=0`,
  bound ≤ 5.6 %. (ii) **TSan BLOCKED, achieved N = 0**, mechanism named: its own runtime
  SEGVs (`this=0x8` in the allocator local cache) on a `libnvidia-glcore` thread, 3/3;
  suppressions cannot address a crash in the allocator, and the software-ICD substitute
  fails natively (→ §5.60). (iii) **§11.122(i) settled as posed** — binary theory REFUTED
  (HUNG 3/54 post-fix vs 1/54 pre-fix, p = 0.31; the pre-fix binary, 0 in 122 before,
  hung) and replaced by **§5.59** (untimed `waitFrame`, teardown serviced by the same
  loop; confirmed on 4/4 hangs by the app's own watchdog stack). **And the hunt caught a
  fire**: `body action reload` + composed OJM bodies + quit ⇒ **SIGSEGV, 25/26 under
  load, 2/6 quiet**, attributed SINGLE-VARIABLE to §5.51 (delivered source minus
  `BodyModule.hpp` = 6/6 CLEAN; delivered = 6/6 plain + 6/6 gdb FIRE; pre-fix arm 0/54,
  p = 3.7e-07). Two faces captured → **§5.57** (texture ring drained after `app.reset()`
  — §5.55's shape at a second site) and **§5.58** (drawing thread vs reload release).
  NEW §5.57/§5.58/§5.59/§5.60; §11.124(c)'s LeakSanitizer sentence corrected (it prints
  every cycle and measures F16's fix: 385209 B → 17686 B per shutdown, 352 KB per-reload
  leak closed). Deviation: **ARM C (cold-start control) NOT run** — the hour went to the
  fire. **For the supervisor: `master-beta` now reproduces a crash on a shipped user
  action; revert-or-fix is recorded as a product-risk decision, not taken (§11.125(j)).**
  **Supervisor-verified 2026-07-31** (11.125 read in full; trees/commits/authors
  checked; `550b3f9f` confirmed .gitignore-only by its diffstat; §5.57–§5.60 rows +
  §5.55/§11.124(l)(m)/B7-row annotations verified at the ledger; harnesses not
  re-run — pre-committed predictions + interleaved arms + single-variable bisect +
  both-faces gdb captures accepted as evidence). Deviations (1)–(6) all ENDORSED:
  (1) a reproducible crash in the hunted class outranks the last increment on a
  refuted theory, and the cost is stated in-entry, not hidden; (2) is the sharper
  single-variable form; (3) matches the F16 rider exactly; (4) is content-free by
  diffstat + bit-identical restore rebuild; (5)(6) are the honest forms. The
  §11.124(c)/(d) corrections are information preservation operating as designed.
  **SUPERVISOR DECISION on §11.125(j) (recorded as a veto point, §3): option (1)
  FIX-FIRST, adopted as F19 and given this round's third slot; F18 defers to next
  round, section stays.** Revert (2) is excluded structurally: §5.55 fires on BOTH
  binaries, §5.57/§5.58 are long-standing lifetime violations made REACHABLE (not
  created), and reverting would re-mask the instrument + restore a measured 352 KB
  per-reload session leak — a patch protecting its own limitation. Neither (3)
  loses to (1) on the queue's own logic: teardown integrity was this round's head
  BECAUSE it gates every instrument; leaving a session-opened 96 %-reproducer
  crash class across a round boundary when its fix batch is fully specified
  (§11.125(m)) optimizes nothing.

### F19 — teardown-order batch: the manager-lifetime class (§5.55+§5.57), the reload race (§5.58), the untimed wait (§5.59)  [M–L]
- **Row / recorded:** §5.55 (fix shape in-row: the `ShadowService::release()` /
  `Renderer::releaseRegistry()` hook pattern + the owed audit) · §5.57 + §5.58 (opened
  §11.125(g)(h), both faces gdb-attributed at source) · §5.56 (rides: same shape at the
  loaders, dies in the §5.55 regime) · §5.59 (opened §11.125(i): untimed
  `hasCompleted.wait(0)`, teardown serviced by the same loop, 4/108 measured rate +
  llvmpipe repro) · §11.125(m) verbatim: *"§5.55 and §5.57 are the SAME mechanism at
  two sites (a resource handed back to a manager destroyed with the App), so the fix
  should address that class, not the two instances; the audit §5.55 already owes
  ('every file-static that captures a manager reference') should be widened to 'every
  deferred-release container drained after `app.reset()`'"* · §11.124(a)(d) (the WHEN
  bound + per-channel analysis the fix must keep true) · B7 row (the 96 % reproducer =
  the batch's regression instrument).
- **Why now (supervisor decision above):** `master-beta` reproduces a SIGSEGV on a
  shipped user action (~33 % idle, ~96 % under load); the fix batch is fully specified
  by §11.125; every future teardown measurement reads through this class.
- **Task:** (i) the CLASS fix for §5.55+§5.57: no resource may be handed back to a
  manager destroyed with the App — the audit first (*every* deferred-release container
  drained after `app.reset()`; *every* file-static capturing a manager reference),
  then the fix at the class (drain/release hooks ordered before manager death, the
  §5.55 in-row pattern), instances fall out of it; §5.56's loaders handled inside the
  same audit (their regime may CHANGE under your reordering — re-derive, don't inherit
  §11.124(f)'s "harmless today"). (ii) §5.58: the mid-session reload release
  synchronized against frames in flight — the existing deferred-release channels are
  the stated safe mechanism (§11.124(d)); route or fence within that contract; a
  genuine design fork (new ownership model, frame-contract change) → record + stop.
  (iii) §5.59: interruptible/bounded wait ONLY under the as-if bar (I7): observable
  behavior identical on every channel except the previously-hung teardown — no cadence
  change (D11), no new wakeups on the hot path; if that bar cannot be met
  decision-free → record with the fork named. (iv) §5.60 is NOT yours (device-limit
  policy — recorded for Vixy).
- **Stop boundaries (NOT yours):** §5.60; §5.52–§5.54 (F18, next round); any
  frame-pacing semantics beyond §5.59's as-if bar; the old render path (baseline);
  line-family coverage-hole mix additions (the flags stay a one-launch control until
  this batch is VERIFIED, then the hole closes in the next hunt's mix, not yours).
- **Discriminating check:** the 96 % reproducer RED→GREEN (`B7_ROVERS=8` +
  `qda`/`qdaterm` under pre-warmed load: 25-26/26 FIRE on the pre-fix binary → 0/26
  post-fix, N ≥ 26 each way); the §5.55 pcaxis pair flips (`flag planets_axis on` +
  teardown: fire → clean, both entries); ASan legs stay silent (no new report class;
  leak baseline 17686 B/194 unchanged or improved — §11.125(d) is the stated
  baseline); the §11.95-mix quiet batch N ≥ 50: 0 fires / 0 hangs; the F16 validation
  leg re-run (0 VUID through shutdown incl. a completed mid-session reload); §5.59:
  the llvmpipe/load stall path exits within the bound while a normal session's frame
  cadence is bit-inert (in-run A/B); battery green (b24_equivalence, b25_galactic,
  b40_parity, b4_anchors, scene E, f15_persist); frozen md5 in==out; concurrent-
  instance assert per §0.5.
- **WIP:** — **DELIVERED 2026-07-31 → §11.126** (code `83324455`/`071b817a`/`a11fbb65`/`96a94a46`;
  harness `fbb5923`/delivery). **§5.55, §5.57, §5.58 CLOSED at the CLASS**; §5.56
  re-derived and unchanged; **§5.59 stays OPEN with its fix built, measured and
  WITHDRAWN**; NEW **§5.61**; NEW **§13.A A40** (the §5.59 fork, Vixy's).
  (i) The audit ran first and is in-entry (§11.126(a)): of the ten new-path
  family-data file-statics **exactly one captures a manager**, the other nine are
  safe by an explicit guard (`PipelineRegistry.cpp:626-630`); of the deferred-release
  containers, **five were not in §11.125's record**, three of them **never drained at
  all**. The OLD path already carries this fix (`destroySC_context()` ←
  `~SolarSystem`), so §5.55 was a generality dropped in the port. Mechanism =
  `Context::onManagerTeardown` hooks run at the START of `~Context`;
  `s_texture::forceUnload` moved out of `main()` as the terminal drain, its old
  call site deleted (I2).
  (ii) §5.58 = `Context::quiesceFrames()` at `reloadSystem` + `ModularBody::remove`,
  **plus a third site the campaign found**: `~App` stops the drawing worker before
  destroying what it records.
  (iii) §5.59: the cancellation works deterministically (`f19_stall.sh`, 3 arms
  2/2 each: HUNG → rc=139 → rc=0) and **costs too much** — across 3 fixed arms /
  60 loaded cycles **every cycle that entered it died, 3 of 3**, all 57 others
  clean. I7 not met ⇒ withdrawn, `waitFrame` byte-for-byte unchanged, fork → A40.
  **Discriminating checks**: reproducer **27 FIRE/3 HUNG/0 CLEAN (pre-fix, 30) →
  0 FIRE/1 HUNG/29 CLEAN (delivered, 30)**, interleaved, load pre-warmed, ARM R
  rebuilt bit-identical to §11.125's md5; §5.55 pair **4/4 FIRE → 4/4 CLEAN both
  entries**; ASan **0 reports** over 9 cycles incl. the K=8 reload class, leak
  17686 → **17626 B / 194**; quiet mix **54/54 CLEAN**; validation **0 VUID both
  entries with AXIS=on** (F16's exclusion retired), channel positive-controlled;
  battery green; frozen md5 in==out throughout.
  **Supervisor-verified 2026-07-31** (11.126 read in full; trees/commits/authors
  checked; cumulative diff surface read — new-path/tools/app seam only, old
  render path untouched; the surviving `waitAllFrames` read AT SOURCE: a NEW
  quiesce primitive, hot-path `waitFrame` byte-identical, its real-completion
  loop written so a future cancellation cannot silently satisfy it — the A40
  fork's precondition stated in code; §5 flips + A40 + B7 annotation verified at
  the ledger; harnesses not re-run — interleaved arms + bit-identical ARM R +
  both-ways controls + committed artifacts accepted as evidence). Deviations
  (1)–(5) all ENDORSED: (1) is the mandate's own as-if bar OPERATING — building,
  measuring, and refusing a fix that converts a rare hang into a rare crash is
  the correct execution, and the three commits bought a reproducer + a measured
  record; (2) the withdrawal rests on the 3/3 correlation, not the missing
  backtrace, and says so; (3) spends bounded cycles on the scene §11.124(m)
  named as the blind spot, with the weak bound stated; (4) is class-over-
  instance with the idle-device cost argument; (5) is the established scratch
  pattern with bit-identical restores. The §5.59 residual's naming — "the
  cancellation makes a pre-existing teardown-after-stall fault REACHABLE, exactly
  as §5.51 made §5.57 reachable" — is the batch's own class seen recursively;
  endorsed as recorded. A40 correctly Vixy's (both branches user-visible).

### F18 — G4-coherence batch: §5.52 mid-band surface + §5.54 threshold authority + §5.53 level step  [M–L]
*[Header restored 2026-07-31 by the supervisor (session 7 warm-up). Root: the session-6
close commit's acceptance Edit swallowed the blank line + `###` header as trailing
anchor context — SECOND occurrence of the F13 edit-truncation class, both supervisor
WIP-tail edits. Restored verbatim from `e5693f1`; class check added to §0b.4.]*
- **Row / recorded:** §5.52 · §5.53 · §5.54 (all opened §11.123(g); full rows at
  INTENT.md §5 — read them verbatim, they carry mechanisms + owed checks) · G4 verbatim
  (INTENT §2): *"compute only what the observer can distinguish, and draw each regime
  with the representation built for it"* · §11.44 correction (its "bright in both" was
  reading the halo) · §2.0 D3 (mid-band = the common case) / D5 (1k–8k) / D11 · B12 row
  rider (thresholds interact with the content slice — content is NOT this task).
- **Why now:** §5.52 is a user-visible rendering hole in D3's common case (every body
  ~3–16 px across draws NO surface on the new path; the old path draws it); §5.54's dual
  authority (I2) sits under every regime measurement including B12's content slice;
  §5.53 is the measurement trap this cluster already sprang once (F9's 1.34 ratio).
  Ahead of the next B31 slice: that slice carries two carve-outs behind late-August
  decisions (D28/D21) and takes a revisit wave regardless; this batch retires defects now.
- **Task:** (i) §5.52 FIRST — the row's two candidate mechanisms (missing `clearDepth`
  depth-slice clip vs `renderer.bind(family, VARIANT_NO_DEPTH)` returning no layout →
  early return) are DISCRIMINATED BY INSTRUMENT before any fix (the row says the second
  is NOT excluded); then fix at root; old path untouched. (ii) §5.54 — settle the one
  question the row states (px-intent vs fraction-intent), DERIVED from G4's own wording
  + the B12 design's use of it (*smallest added structure ≥ 1 px* is a px criterion),
  recorded as a VETO POINT, never silently decided; ONE authority survives (I2), the
  other deleted; behavior at a 2048-wide render preserved (the literals ARE the px
  values there — predict 0 px scene delta at 2048); the row's own before/after scene
  battery. (iii) §5.53(a) — root-cause the reduced-level washout in the reduction/cache
  path (`s_texture`/`txcache`; a downscale preserves the mean, so the deviation is a
  defect, not a taste); fix if decision-free. §5.53(b) — the two paths' switch-size
  disagreement is MEASURED + RECORDED with a recommendation (a §11.52(b) parity
  question only while both paths exist), not decided.
- **Stop boundaries (NOT yours):** B12 CONTENT (granulation/spots/chromosphere — behind
  Vixy's b12-design §7 decisions, incl. the grammar word); the D5 resolution-range
  policy itself (your veto point informs it); the old render path (baseline by
  construction); anything D21/D28-dependent; the §11.123(o2) cadence check UNLESS this
  task touches a cadence instrument (then the wall-clock-bracketed counter read is
  OWED — §0.5).
- **Discriminating check:** (i) the §5.52 row's own: same disc-centre row, halo
  suppressed, non-zero in BOTH paths — on the Sun AND on a planet (nothing here is
  star-specific); RED on the pre-fix binary. (ii) at 2048: A/B scene battery 0 px (or
  explained per-scene); at a SECOND render width: the gate position measured both sides
  matches the prediction made BEFORE the run; grep-clean: one threshold authority
  tree-wide. (iii)(a) disc-centre RGB across the threshold ≈ the map's known mean on
  BOTH levels, RED pre-fix; D11 unchanged (1 ms/frame denominator). Battery green +
  frozen md5; the b12 instruments (`b12_photosphere.py`/`b12_limb.py`) stay green (the
  star family lives inside these thresholds).
- **WIP:** — **DELIVERED 2026-07-31 → §11.127** (code `94eb3f03`/`2117ccb0`; harness
  `a0671a4`/`9aec4b3`/delivery commit). **§5.52 CLOSED** (candidates discriminated by
  instrument first — candidate 2 REFUTED, candidate 1 CONFIRMED at NDC z = 11574/23588/
  71345/29.9; fixed at the root by splitting the depth MAPPING from the slice ENTRY;
  z = 0.5 predicted and met; disc ratios 0.84/0.88/0.99 vs 0.00/0.00/0.11 pre-fix on the
  Sun AND on planets; b12 s5 0.0000 → 0.9917). **§5.54 CLOSED px-intent** (derived from
  four independent lines; census wider than the row — 0.2 had six spellings; one authority
  + derived gates owned by `setViewportRadius`; bit-identical at 2048; second width
  measured both sides with a counterfactual leg that FAILS as predicted). **§5.53
  root-caused and SPLIT**: (a) it is DATA — the loader prefers authored `-preview` assets
  and the two shipped ones do not match their partners (Jupiter, with no preview, reads
  0.998 ≈ 1.000, which is the row's own premise confirming itself); (b) old 180 px vs new
  409.6 px MEASURED with a recommendation, not decided. NEW **§5.62** (a same-binary
  measurement that shifted between two epochs — recorded, not chased; it does not
  implicate the delivered code). NEW Vixy rows **A41/A42/A43**. Battery green, frozen md5
  in == out. One decision-free rider (a D12 log) was BUILT and WITHDRAWN for failing its
  own bar. `b12-design.md` §3.1 superseded-with-record.
  **Supervisor-verified 2026-08-01** (11.127 read in full; trees/commits/authors
  checked; diff surface confirmed new-path only — all 17 files under
  `src/experimentalModule/`, old path untouched; §5.52/§5.54 CLOSED tails +
  §5.53 OPEN-on-(a)(b) + §5.62 row + A41/A42/A43 rows + B12 content-slice gating
  verified at the ledger; harnesses not re-run — instrument-first discrimination
  records + RED-pre-fix legs at three independent scales (disc ratios, the 1024
  gate counterfactual, the per-file level predictions) + committed artifacts
  accepted as evidence). Deviations (1)–(4) all ENDORSED: (1) keeps §5.53(b)
  answerable as a one-token edit — the F12 one-authority-per-question move; (2)
  is correction-validated-against-the-class (the fraction ordering inverts above
  ~4k — found by replaying the scope, not the trigger); (3) is the instrument-
  chain bar operating — a probe that cannot be shown to fire converts observation
  into fiction, same bar that withdrew §5.59's fix; (4) is the §11.80(a) floor
  discipline plus the sharper exact-arithmetic argument. §5.62's record-don't-
  chase ENDORSED (owed check named in-row; every delivered comparison is inside
  one epoch with its own pair, so the delivery does not rest on the unstable
  quantity). A41/A42/A43 correctly Vixy's; A42's align-to-180 recommendation
  endorsed — parity-instrument trap already sprang twice (F9's 1.34, §5.53's own
  discovery path). Checkpoint-WIP line consolidated into this one by the
  supervisor (protocol: cleared at delivery).

### F20 — B31-impl slice 3: the session file (§3.2) + the §5.32 precondition fix  [L]
- **Row / recorded:** B31 (§13.B — remaining sequence after §11.121: *"the session file
  (§3.2) and the ledger (§2 group D)"*) · `b31-design.md` §3.2/§3.3/§3.5 (the authority:
  session file `~/.spacecrafter/sessions/<name>.ini`, machine-owned + disposable; the
  MANIFEST; trigger `session action save [filename <name>]` / `session action load
  filename <name>`, one registration serving channels 1–5 per §11.55(h)) · §6.1 (the
  save is a D8 USE-SITE: first consumer touching EVERY body — frozen bodies recomputed
  at use +4 iterations per the §11.76 barrier; read the MODEL, never control-surface
  getters (§3.4(e)/B33); **§5.32's fix is a stated PRECONDITION of a trustworthy
  save**) · §5.32 row verbatim + its §11.108(e) measured observable (descend
  compounding ×0.899997 linear vs ×0.9043821 geometric, residual 0.59 m) · D30/D32/D33/
  D36 verbatim (§11.113(i)(k)(l)(o): hybrid delta-where-authored; transients snap to
  settled target, carve-outs = view-offset armed latch, screen fader, trail points;
  EXPLICIT ONLY, load = idempotent PRESET; declarative show state IN, time-bearing
  OUT) · §2 state inventory groups A–C, E–K (group D is the NEXT slice) · §6.2 checks
  T1/T2/T4/T5/T10 · §6.3 (md5 of every untargeted data file unchanged after a save).
- **Why now:** §11.110's own sequencing, fourth step; the ledger slice builds INTO this
  artifact, so it cannot precede it.
- **Task:** (i) **§5.32 FIRST** — the row's own identified fix (recompute the
  reference's spin+reach at the top of `Camera::update`); it closes spin, persistent-
  longitude and the reference half of the B15 residual, and unblocks T10. (ii) The
  session artifact + manifest per §3.2/§3.3, serializing the §2 inventory (groups A–C,
  E–K by their MUST-SAVE/DERIVED/EXCLUDED classes) through the F13 writer — ONE
  serialization authority (I2). The §3.3 in-session unload/reload twin is IN scope only
  as far as it is the SAME code path the design claims; if it needs its own increment,
  record the boundary, don't force it. (iii) The trigger commands per §3.5, spelling
  recorded as a VETO POINT (B28 protocol; F7/F15 precedent). (iv) D8 use-site honored
  (frame-boundary task, I/O off the draw thread, atomic sibling-temp-then-rename).
  (v) Load idempotent (D33), D32 snap-to-settled with its named carve-outs, D36
  boundary enforced.
- **MANDATED CARVE-OUTS (state them in the entry; both behind late-August decisions):**
  `heading` (group B6) is NOT serialized — its meaning across a reference change is
  D28's open question; its exclusion is recorded IN the session file (annotation naming
  D28) so a later slice can add it without archaeology. T3 (the §11.101(f) latch
  prediction) is NOT this slice's DoD — sequenced with D21/§5.27; T1's scene must not
  stake its criterion on the D21-gated combination (grounded child under a scaled
  parent) — compose the scene around it and FLAG the composition as a deviation.
- **Stop boundaries (NOT yours):** group D per-body ledger + T7/T8 (next slice); T3/
  D21; `heading`/D28; §5.41/§5.42 old save surfaces (the session file REPLACES them —
  record-don't-fix stands); autosave/autoload in any form incl. a config key (D33's
  answer); legacy `ssystem.ini` never written (D35/§2.0 D13, md5 in==out asserted);
  §11.109(h2) whitespace-key semantics (SUSPENDED); §11.109(h3) same-name collision
  (record, never decide).
- **Discriminating check:** §5.32: the §11.108(e) observable FLIPS (ten same-frame
  `descend(0.99)` compound geometrically ×0.9043821 post-fix vs linear ×0.9 pre-fix,
  both-ways on the pre-fix binary) + longitude round-trips. T1 (screen, A/A floor
  measured in-scene per §11.80(a), same binary both sides). T2 (dump field-by-field;
  extend with the gap dumps it names — `lockedSkyRot`, the plans — NOT the per-body
  ledger dump). T4 (fixed point: second session file byte-identical). T5(a)(b) (the
  instrument must be able to fail, `--mutate` shape). T10 (frozen-body save equals
  never-frozen within the recompute tolerance — the use-site barrier proof). §6.3 md5
  sweep (every untargeted data file unchanged). Battery green (b24_equivalence,
  b25_galactic, b40_parity, b4_anchors, scene E, f15_persist, b12 instruments, f18
  gates); frozen md5 in==out. §5.62 caution: no check may rest on a cross-epoch
  mid-band disc ratio (in-epoch pairs only).
- **WIP:** —

---

## 2. Blocked — NOT dispatchable (reason stated so the exclusion is challengeable)

- **B1/S4 + B2 + riding rows** (D4 surface streaming, RING asteroid, INSTANCED
  consumer): Vixy-paced architectural line (EntityCore Taskable authority).
- **B5 remainder** (dso3d/tully, ojmMgr, floors): suspended on the §11.96(e) six +
  §11.98(f) three — chiefly the reach-vs-visibility decoupling, now promotion-grade
  GENERAL (§11.97(c) second domain) and Vixy's call before any large content lands.
- **B30 fix**: tracking-convergence semantics suspended (§11.94(d)).
- **B18 residual**: D15 unanswered (the 2026-07-23 batch's only open item).
- **B17 residual**: heading≠0 offset coupling — tilted-dome question (§11.92(d)).
- **B10(a)**: anti-stuck floor VALUE = Vixy's feel-test (§11.79(f)).
- **B14 residuals (ii)(v)(vii)**: source-authority order, meridian texture-
  registration, float32 `re.period` — all Vixy's eye (§11.75(c), §11.86(c), §5.25).
- **B8**: waits for old-path removal by definition.
- **B21 step feel**: Vixy's.
- **Every §13.A row**: Vixy/tester territory by protocol.

## 3. For Vixy — sendable/decidable now (not tasks; parallel to any dispatch)

- ~~**A15 re-ask is SENDABLE**~~ **JOINS THE FINAL TESTER PASS** [vixy 2026-07-30,
  batching principle → §11.116(c)]: tester items accumulate into ONE final pass
  before testing deployment; the final-pass list is ledger-owned (members so far:
  A15, the oort-shadow item below), round-3 file materializes at send time.
- ~~**§11.98(c) missing datum**~~ **RESOLVED 2026-07-30 (§11.116(b))**: the
  originating observation was recovered verbatim from session transcripts — it
  says "oort **SHADOW** showing too early", its configuration reconstructs to
  **free_mode/Sun-ref, fov 340 pinned** (the "(anchored on earth)" text was the
  requested ladder's SPEC, not the watched scene); the **zoom confounder is
  REFUTED** (closed candidate set, zero fov/zoom commands); the anchored-Earth
  refutation never reached that configuration ⇒ genuinely-early stays live there,
  vs expectation-wrong — discriminated in the final tester pass, state-stamped.
- **Decision batches waiting**: §11.96(e)(1–6) + §11.98(f)(i–iii) (oort/§6.9 plan);
  D15 (expanded §11.112 — sub-item (c) mirror-all-four is recommended + mechanical);
  D21 (corrected form §11.101(f)); §11.92(d) heading-coupling; §11.94(d)
  latch-when-settled. D22–D36: ANSWERED + propagated (§11.113). **NEW 2026-07-30:
  D37** (F11/§11.117(k)(1)) — does a hidden body stop being a LIGHT SOURCE?
  Measured: today it does not (a hidden Sun still lights the Moon, 15462 vs 17302
  lit px); illumination is the one contribution D23's general wording reaches and
  the B39 row does not enumerate, and no shipped hidden body is a light source, so
  nothing in the corpus discriminates. Rec (1) keep today's behaviour; reversing
  it later is one branch at `updateSystem`.
- ~~**F1 instrument authorization (§11.99(h))**~~ **SERVED 2026-07-24 (manual
  approval) → root closed §11.100.** Replaced by: **D21** (DECISIONS_PENDING) —
  grounded children vs parent display scaling (`moon_scale=5` swallows the mandate
  scenes; §5.27). Workflow note for shader edits [vixy]: `shaders/compile.sh` +
  `cmake --install` — not hand-copies into the install dir.
- **Session-4 veto points (2026-07-30 — all implemented-and-live, each cheap to
  reverse; silence = endorsed):** (1) B27 hint-suppression gate landed on `primary`,
  not `light_source` as §11.113(f) provisionally placed it — argument at the site
  (origin-driven, a dark primary keeps its hint suppression), byte-inert on shipped
  data, twin-only key (§11.118(c)); (2) `primary` is its own member, NOT a second
  `BodyType` bit (`isMinorBody()` exact-equality trap, §11.118(c)); (3) F13 removal
  records are deliberately UNMARKED comments (a machine-marked removal would delete
  itself on the next write, §11.119(a)).
- **NEW §5.49 (F14, §11.120(j)) — your eye when convenient, no urgency:** the
  OBSERVER's longitude origin is the mesh x̂ column (u=0.75), 90° from the map's
  centre column the meridian fix now targets — derived from source, NOT yet measured
  at the render; on Earth `moveto lon 0` would stand 90° from Greenwich. Never seen
  because no shipped scene puts both conventions in one frame. The discriminating
  test it owes is stated in the row; fixing it is user-visible semantics = yours.
- **Session-5 veto points (2026-07-31 — all implemented-and-live, each cheap to
  reverse; silence = endorsed):** (1) F15's save-trigger spelling **`body action
  save [filename <name>]`** — the system-scope sibling of `body action reload`,
  one `else if` to reverse (§11.121(e), B28 protocol); (2) F9's V1–V4
  (§11.123(l)): Eddington grey-atmosphere limb law centre-normalised (one shader
  line to swap), star-surface type selected by `isStar()` at the loader, shadow
  trait bits not declared on the photosphere (proved inert at source), granule
  scale = model parameter of the procedural field, not a solar datum.
- **B12 content-slice decisions (recorded NOT blocking; needed only before the
  CONTENT slice — `b12-design.md` §7's LOCAL numbering, not DECISIONS_PENDING
  rows):** authored-vs-procedural spots · is a star enterable and what is seen
  inside · does a resolved star's halo alternate with its surface or coexist ·
  the chromosphere module's `module =` grammar word (B28 sign-off owed BEFORE
  that module can be built — the one of the four with a hard ordering).
- **NEW A40 (F19, §11.126(g)(l)) — the session-6 round's one open decision, and it
  is load-bearing for B7's last open class:** when the drawing worker has not
  completed the frame the main loop waits on and the user quits, should the app
  (i) keep waiting (today: no exit, watchdog calls it hung — measured ~1–3/30 under
  load with 8 composed rovers) or (ii) stop waiting, which MEASURABLY reaches a
  teardown fault the waiting hides (3/3 in F19's campaign — the fix was built,
  measured, and WITHDRAWN on exactly that result)? Both branches user-visible.
  `harness/f19_stall.sh` makes either answer testable in three launches. §5.61 (a
  lost wakeup in EntityCore's `WorkQueue::pop`, recorded not claimed) is a candidate
  mechanism for the hang side and sits on the EntityCore authority line — your
  pacing.
- **NEW §5.60 (F17, §11.125(e)) — device-limit policy, D13-flavored, no urgency:**
  the app requests a single 2.68 GB device allocation and dies where the limit is
  enforced (llvmpipe's 2.15 GB cap) — a policy on `maxMemoryAllocationSize` is
  yours; recorded, not designed.
- **Session-6 supervisor decision (recorded as a veto point; cheap to reverse as
  scheduling, not semantics):** F17's find made `master-beta` reproduce a SIGSEGV on
  a shipped user action; the executor's fix-vs-revert-vs-neither suspension was
  DECIDED fix-first by the supervisor (revert excluded structurally — the violations
  pre-exist, §5.55 fires on both binaries, and reverting would re-mask the
  instrument + restore a measured 352 KB/reload leak); F19 took the round's third
  slot, F18 deferred one round. The class fix is landed and verified (§11.126);
  reversal of the scheduling is moot post-delivery, reversal of any F19 mechanism is
  per-commit and each carries its argument at the site.
- **Host note — CORRECTED [vixy 2026-07-31] (§11.121(m) + §11.122(n) carry the
  annotations):** foxy the person was NOT active during session 5 (active only
  before it); F8's "second user active" measured foxy-owned leftover PROCESSES
  (remmina ~31 % CPU), whose load was real and is in the per-teardown record —
  no F8 conclusion moves. The mid-F15 21:49 launch was NOT Vixy (asleep):
  intra-account, most plausibly the F15 executor's own unaccounted early launch
  or a sibling claude session; retroactively unattributable (mtimes overwritten
  by the F8/F9 waves — checked). The 00:00–00:02 twin mtimes ARE attributed
  (F15's script-channel gate leg, commit `ee254dc` 00:03:36). Structural
  closure: §0.5 now carries a concurrent-instance assert before measurement
  launches — the confound is intra-account, md5 re-asserts alone don't cover a
  concurrent launcher. Nothing left for you to decide here.
