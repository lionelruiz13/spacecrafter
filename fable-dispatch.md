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

## 1. Dispatch order (load-bearing first; each task states why, so the order is challengeable)

### F0 — audit-residuals batch: decision-free fixes from §11.101/§11.102  [S–M]  ← START HERE
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
- **WIP:** —

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
- **WIP:** —

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
- **WIP:** —

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
- **WIP:** —

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
- **WIP:** —

### F7 — B4: CameraAnchors implementation per R3  [M–L]
- **Row / recorded:** B4 (§13.B) · R3 (§11.70: on-orbit PRIMARY; body-attached-keeping-angle NEEDED; fixed-point Universe-only) · Q5 (both channels; no cross-session persistence) · FEATURE_REQUESTS 2026-07-21-02 (save-to-anchor.ini command — folded, not promised).
- **Why load-bearing:** S7 spine gate; the design-deciding half is closed, purely
  Fable territory now.
- **Task:** the three anchor kinds per R3, both §2(c) channels (anchor.ini +
  script/command creation); cross-session persistence explicitly NOT needed.
- **Discriminating check:** per-kind live scenes (on-orbit anchor holds through body
  motion; body-attached keeps angle; fixed-point in Universe mode), reversible;
  battery green.
- **WIP:** —

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
- **WIP:** —

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

- **A15 re-ask is SENDABLE** (§11.82: the fade is complete and live; tester judges
  threshold/band/appearance side-by-side).
- **§11.98(c) missing datum**: the "oort too early" observation refutes on anchored-
  Earth — which configuration produced it? (free mode / other anchor / other content).
- **Decision batches waiting**: §11.96(e)(1–6) + §11.98(f)(i–iii) (oort/§6.9 plan);
  D15 (§11.79(i)); §11.92(d) heading-coupling; §11.94(d) latch-when-settled.
- ~~**F1 instrument authorization (§11.99(h))**~~ **SERVED 2026-07-24 (manual
  approval) → root closed §11.100.** Replaced by: **D21** (DECISIONS_PENDING) —
  grounded children vs parent display scaling (`moon_scale=5` swallows the mandate
  scenes; §5.27). Workflow note for shader edits [vixy]: `shaders/compile.sh` +
  `cmake --install` — not hand-copies into the install dir.
