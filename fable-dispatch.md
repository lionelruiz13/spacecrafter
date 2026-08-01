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

**Archival [2026-08-01, pass 1]:** a unit that no longer contributes to the current
state (a DELIVERED+verified task section; a session update note superseded by a later
one) leaves the live surface as a **pure byte-exact move** to
`fable-dispatch/archive/<id>.md` (IDs: `F<n>`, `update-s<n>`; manifest per pass,
reconstruction-verified against the pre-move commit). References are NEVER rewritten:
resolve any section reference by probing the stated path, then with `archive/`
inserted at the failing component. Lateral search spans live ∪ archive — grep this
file AND `fable-dispatch/archive/` together, never the live surface alone. The
in-file derived index (§1) is regenerable, never authoritative. A wrongly archived
unit moves back at the cost of one probe — when in doubt, a unit stays live.

**Update [Fable 2026-08-01, supervising session 7]:** round of 3: **F18 → F20 → F21**
(F20/F21 minted this session — the two remaining B31 slices). Warm-up found the F18
`###` header DESTROYED by session 6's own acceptance edit — the SECOND F13-class
edit-truncation, both supervisor WIP-tail edits; restored verbatim from `e5693f1`,
and §0b.4 now carries a next-header guard for exactly this.
**Round outcome (session 7 close, 2026-08-01):** F18 → §11.127, F20 → §11.128,
F21 → §11.129 — all three delivered AND supervisor-verified same session. **§5.52,
§5.54, §5.32 CLOSED**; §5.53 root-caused and SPLIT (renderer settled; the colour step
is two mismatched preview ASSETS → A43; switch-size alignment → A42); the G4 gates are
**px-authored with one authority** (the exposed 2-vs-3.07 px discrepancy → A41);
**B31 slices 3+4 LANDED** (session file + §5.32 precondition; read-half authority +
E3/E4/E5 + per-body ledger) — **B31 has NO dispatchable remainder** (open set exactly:
T3/D21 · heading/D28 · C4's key · §5.63). NEW rows: **§5.62** (epoch-unstable mid-band
ratio), **§5.63** (T1 photometric blocker — hunted 3 waves, not attributed, seven
exclusions + a correction of its own text), **§5.64** (timerate pause does not stop
the clock — Vixy semantics); NEW Vixy rows **A41/A42/A43**. **T1 is the round's one
honest NOT-MET** — both B31 slices delivered partial on it, neither absorbed it.
Executor quality: F20 REFUTED this file's added longitude-round-trip check by
derivation (it was the DEFECT's signature, not the fix's) — sharpening the §0b.3
lesson: an ADDED check must derive from the row's mechanism, never its symptom
sentence; F21's T4 fixed-point caught a polarity defect by ONE byte. The §11.121(m)
concurrent-instance assert had its first live fire (two orphaned instances) and its
own instrument hole fixed (`pgrep -f` was self-confirming). Remaining dispatchable,
next round: **F22 to mint** (§5.63 attribution: the owed old-path view-state readback
— the read-half gap one layer down — then T1 closure; the `view_offset` scene hole
rides its closure), **B7-hunt-5 only after A40**, **B12 content** after Vixy's
b12-design §7 set + A41/A42, **§5.64 fix** only after Vixy decides the semantics.
DECISIONS_PENDING open set at close: **D15, D21 (late Aug), D37** + ledger rows
A40–A43 awaiting.

**Update [Fable 2026-08-01, supervising session 8]:** round of 3: **F22 → F23 → F24**
(all three minted this session). F22 is session 7's queued item (§5.63 attribution →
T1 closure, the B10 scene hole riding). F23/F24 are NOT in session 7's "remaining
dispatchable" enumeration — that enumeration is a queue VIEW, and §13 (the authority)
carries two rows with recorded fix shapes and no blocking decision: **B33's
non-heading getter residue** (D28 gated only the heading member; F12 landed the
template) and **B34's mechanical seam mirrors** (`body action clear` / `body action
preload` / trail fresh-restart — *"same fix shape as every landed seam mirror"*).
Lateral search live ∪ archive found no deferral record on either ⇒ the omission is
view staleness, not an unrecorded deferral. B34's interactive view/zoom ramps stay
OUT (own future task — the one B34 member with a feel-reproduction surface). Warm-up:
both trees clean, binary confirmed current at code `4dc29023` by no-op rebuild;
A40–A43/D15/D21/D37 all still open (no Vixy commit since session 7's close) ⇒
B7-hunt-5, B12-content, §5.64 stay blocked accordingly.

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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F21 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129.
Live sections: **F22–F24** (minted session 8, below). The current queue lives in the
latest session update note above.*

### F22 — §5.63 attribution: the old-path view-state readback, then T1 closure  [M–L]

- **Row / recorded:** the §5.63 row VERBATIM (INTENT.md §5 — it carries F21's full
  bounded-attribution record: seven exclusions incl. the corrected characterisation —
  saved/restored lit sets positionally DISJOINT, 0/400 overlap, median NN 50.2 px ≈
  1.1° at fov 45; the whole residual is old-path sky — stars 2418 / milky way 744 /
  nebulae 2493 px, all three off ⇒ 0 px>8; transience excluded; dome-centre rotation
  excluded — the milky way sits in the SAME place both sides while the star/DSO field
  does not; magnitude UNSTABLE across restores — 3185/5655/7834/7830 — with the
  divergent term on the RESTORED side) · §11.129(b) + harnesses `f21_s563{,b,c}.py`,
  artifacts `harness/artifacts/f21s563*` · §11.128(h) (T1 origin; in-scene A/A floor
  0) · `b31-design.md` §6.2 T1 · B31 row (this is T1's blocker, one of the row's last
  four open items) · the OWED probe, stated in-row: *"a readback of the old path's own
  view state (`Navigator::getLocalVision` / `getPrecEquVision` /
  `mat_local_to_earth_equ`) on both sides — which does not exist today"* · the
  sharpened hypothesis: what `hip_stars`/`nebulas` consume BEYOND the navigator
  (equatorial-frame transform / geodesic grid / refraction pre-pass) diverges on a
  restore while the milky way — same navigator — agrees · the scene hole, §5.63
  exclusion (6): `f20_session.py` scene A's `set view_offset 0.25` is NOT a
  registered `set` name (app: *"Did you mean zoom_offset ?"*) ⇒ §2 row B10 was never
  exercised; the registered §2(c) channel is `set zoom_offset <v>` (B17/§11.63; dual
  since §11.92; D32's view-offset arming-latch carve-out applies to its restore).
- **Why now:** B31's open set is exactly four items; this is the only one that is
  neither Vixy's (D21/D28) nor a design item (C4). Session 7 queued it first.
- **Task:** (i) Build the READBACK — the old path's view-state fields
  (localVision / precEquVision / the local→equ matrix; extend to the frame transforms
  the hypothesis names if those three do not discriminate) exposed on the existing
  dump channel, additive-only (instrument precedent: F20's T2 gap dumps; the old
  RENDER path stays unchanged by construction — a const readback is not a render
  change). (ii) Attribute: run the readback on both sides of the SAME save/restore
  ladder `f21_s563` runs; fields that differ where the screen differs AND agree in the
  stars-off control localize the consumer; the milky-way-agrees contrast bounds the
  candidate set; restore-to-restore VARIANCE (exclusion 7) is itself a signature —
  a term that varies on one side only smells of ordering/latch state, so diff the
  restore's drive ORDER against the rebuilt-by-commands order. (iii) Fix at the
  root — expected in the RESTORE/session path (drive the missing seam, or fix the
  ordering/latch); if the root lands in OLD-path code proper, STOP: record + suspend
  (§11.52(b) — the baseline is unchanged by construction). (iv) T1: green in
  `f20_session.py`, or the residual accepted ONLY at the row's own bar (magnitude AND
  mechanism predicted from the attributed cause). (v) The scene hole: re-point scene
  A's B10 line to the registered spelling; verify the line now ACTS (did-you-mean
  line gone + the offset observable moves) and that the restore holds it through the
  D32 carve-out; extend T2/T5 coverage for it — an added check derives from the
  row's MECHANISM, never its symptom sentence (§0b.3 lesson).
- **Budget (state it, spend it, stop):** F21's 3 waves are SPENT and recorded. You get
  the NEW instrument plus at most 3 further mechanism waves. Not attributed at
  budget ⇒ stop, EXTEND §5.63 with the new exclusions (F21's protocol), and still
  deliver (i) and (v) — they are unconditional deliverables; T1 then stays honestly
  NOT-MET.
- **Stop boundaries (NOT yours):** §5.64 (Vixy semantics); T3/D21; `heading`/D28; C4;
  §5.62 (record-don't-chase stands — and its consequence binds YOU: in-epoch pairs
  only, never cross-epoch); old-path render behavior; A41/A42/A43.
- **Discriminating checks:** the readback DISCRIMINATES (differs where the screen
  differs, agrees under the stars-off control — if it does neither, say so and widen
  the field set, don't infer); on attribution a ONE-VARIABLE flip both ways
  (re-drive/fix the attributed term ⇒ the lit-set disjointness collapses to the A/A
  floor; the pre-fix binary keeps the divergence); T4 byte-identity + the full
  f20_session suite green; the f21_s563 ladders re-run on the delivered binary;
  scene-A fix both ways (pre-fix binary: did-you-mean logged + offset inert;
  delivered: line absent + offset drives + restore holds); battery green; frozen md5
  in==out; §11.121(m) concurrent-instance assert before every measurement launch.
- **WIP:** ~~2026-08-01 · in flight~~ **DELIVERED 2026-08-01 → §11.130.** §5.63 **ATTRIBUTED, FIXED and CLOSED**; **b31-design §6.2 T1 MET** (f20_session ALL GREEN, T1 39 px>8 / in-scene floor 51, T4 byte-identical 10978 B). The readback the row was owed exists (`oldView` on the dual dump, four owners, all `const`); the residual was the OLD path's view direction, never carried (107.634° apart with every camera field identical) plus a sky lock latching a stale pair inside the restore's frame-less batch (launch date = system clock ⇒ exclusion 7's variance). Fix: `[observer] sky_vision` + `Core::restoreSkyVision`; 3294 → **38 px>8 / floor 44**, direction → **0.0°**, stars 392-vs-689 → **392-vs-392**, and **0.00e+00° on an AIMED direction against a 101.999° control**. §2 row **B10** exercised for the first time and had the same defect (offset on the camera only) — both paths now, ordered. NEW **§5.65 / §5.66 / §5.67** (all record-don't-fix). B31's open set is now exactly **T3/D21 · heading/D28 · C4's key** — all three Vixy's. Code `170c3ad6` `4d31e9d0` `42ec495a` `ff2c798b`; harness `b48add9` `33546e7` `dcc29fe` `752500f` + the entry. Two veto points in §11.130(k): the new `sky_vision` key, and asserting the old path's offset latch through a setter rather than an aim.
  **Supervisor-verified 2026-08-01** (§11.130 read in full; commits/authors/trees checked; diff surface verified at the diff level — **zero deletions in navModule/starModule/projector**, the old path gains only `const` dump methods + two restore-only setters with one caller each, nothing a draw call reaches; §5.63 closure tail + §5.65/§5.66/§5.67 rows + B31 three-item annotation + b31-design B19/T1 annotations all verified at the ledger; harnesses not re-run — both-ways ladder tables with per-run in-scene floors + committed artifacts accepted). Deviations ALL ENDORSED: (1) stars-off control non-discrimination said-not-inferred — the control removes the CONSEQUENCE; the A/A-floor control that replaced it is measured and is the right one; (2) scene-A both-ways in one launch — registry untouched, strictly stronger; the code half carries its found-failing→fixed-passing record; (3) the two additive old-path setters — §11.52(b)'s reason is baseline BEHAVIOR, which the diff shows untouched; (4) 9-digit `sky_vision` store — value at the consumer's precision, §11.128(c) precedent. The (h) corrections ENDORSED as supersession-with-record — the *low-spatial-frequency-layer-is-not-a-displacement-witness* lesson is promotion-grade general; the T1-floor re-measure carries its own mechanism (lock capture at whatever frame the command lands, alt/az/lockedSkyRot ~1e-3 across launches) and the content-signature-gone evidence, so T1-MET is endorsed at the row's own bar. §5.65/§5.66/§5.67 routing endorsed (→ section 3).

### F23 — B33 residue: the four remaining control-surface getters read the path that draws  [M]

- **Row / recorded:** B33 (§11.108(f); class residue after F12: *"`getViewOffset`, the
  observatory lat/lon/alt getters, `Core::getMountMode`, `getFlagLockSkyPosition` —
  none unblocked by D28"* — read as: none NEEDED D28; the heading member was the only
  D28-gated one) · the landed template §11.118(f) (the getter asks
  `Core::getExperimentalPath()` — reports the path that DRAWS; in ALTERNATE mode it
  follows the toggle; units/normalisation preserved; private second readers folded
  through the getter — `tcpGetPosition` precedent, I2) · the second instance,
  verified at source in-row: `moveto multiply_alt`/`delta_alt` build their ABSOLUTE
  target from `coreLink->observatoryGetAltitude()` and then move BOTH paths
  (`app_command_interface.cpp:2983`/`:3025` as-of-§11.108 — re-locate at HEAD) ·
  Camera's own getters exist with ZERO external readers (§11.108(f)).
- **Why now:** every divergence source that appears (they keep appearing — heading
  did) reports fiction through these getters until the class closes; B31's save had
  to route AROUND them (*"read the MODEL, never control-surface getters"*) — closing
  the class removes the trap for every FUTURE consumer that doesn't know that rule.
- **Task:** apply the F12 template to each of the four members (the observatory group
  is three getters, one mechanism); census private/second readers of each underlying
  readout and fold them through the getter (I2); then VERIFY — not re-derive — that
  the multiply_alt/delta_alt targets derive from the drawn path.
- **Vacuity honesty (state it per getter in the entry):** where dual setters hold the
  two paths in agreement today (viewOffset — dual since §11.92; skyLock — §11.58;
  mount — config-only, B35), the fix is LATENT: the discriminating check must FORCE
  divergence (gdb injection, F4's probe precedent, or the `render_path` pin) — derive
  the check from the mechanism (two authorities exist; the drawn one must be
  reported), never from a natural repro that may not exist today. Name per getter
  which divergence channel is real vs injected. Altitude may have a REAL channel
  (§11.108(d)'s B21-gap history); if one exists at HEAD, use it.
- **Stop boundaries (NOT yours):** `configuration action save` persisting
  `navigation->getHeading()` (B31 ledger-adjacent; §5.41/§5.42 record-don't-fix);
  §11.102(b2) RA/DE-AltAz readout skew (rides §11.92(d), Vixy); NO new command
  surface (B35/B37 spellings are Vixy's; the mount stays unwired — the getter fix
  must not wire it, D15 adjacency); old render path unchanged.
- **Discriminating checks:** per getter BOTH WAYS (diverged: delivered getter == the
  drawn path's value while the pre-fix binary reports old's; agreed: value unchanged
  — the regression half); the multiply_alt discriminator (diverged altitude ⇒
  post-fix target derives from the drawn path, measured on both binaries);
  TUI/readout consumers unchanged where the paths agree; battery green; frozen md5
  in==out.
- **WIP:** 2026-08-01 · CP1 code `93d83ebd` (INSTRUMENT: `body action dual_dump` gains
  a `control` object — {reported, old, new} per member — because no member of the class
  had an observable channel; `Camera::getPlace()` = the inverse of `moveTo`'s target in
  both modes; pre-fix binary `artifacts/f23/sc_f23_pre` md5 `ebe92184`) · CP2 code
  `9770649f` (the four getters + the `tcpGetPosition` place fold) · CP3 harness
  `f23_b33_control.py` **ALL GREEN both binaries** (`artifacts/f23/run2`): anchored
  200 km→49999.855 m and free 40 000 km→9 999 999.363 m ladders, disc 104 120→831 450
  lit px, pin traversed twice, `moveto multiply_alt 1` moves the drawn observer **0 px>8
  post-fix vs 738 266 px>8 pre-fix** (teleport 10 000→40 000 km), `multiply_alt 2` target
  19 999 998.7 m vs pre-fix 80 000 003.6 m, sky-lock toggle locks (camera drift 15.0411°
  /sidereal hour) vs pre-fix 0.0000°. · CP4 harness `f23_b33_inject.py` +
  `_run.sh` **BOTH WAYS GREEN** (`artifacts/f23/inject_post3` / `inject_pre`): the two
  LATENT members forced by gdb on the DRAWN path — offset old 0.3 / drawn 0.15 and mount
  old equatorial / drawn altaz; the delivered binary reports the drawn value, the pre-fix
  one the old, and the pin flips both back and forth on two entries. ·
  **DELIVERED 2026-08-01 → §11.131.** Code `93d83ebd` (instrument) + `9770649f` (the four
  getters + the `tcpGetPosition` place fold); harness `f6cf64b`/`cbd4c3c`/`5676057`/
  `4b38f94` + the entry. **B33 CLOSED.** Two of the four members were NOT latent, against
  the row's implication: altitude (`camera action descend` is new-path-only by design) and
  the sky lock (four shipped sites write the old flag alone) — the row's own second
  instance fires, `moveto multiply_alt 1` moving the drawn observer 0 px>8 delivered vs
  738 266 px>8 pre-fix, and the lock toggle now locks (15.0411°/sidereal hour vs 0.0000°).
  View offset + mount LATENT, divergence injected by gdb, both ways. Regression half
  measured at the composed screen: 1965 px>8 pre vs post against floors 2063/2052 (below
  launch variance), with the old observer moving 0.104276 m ONTO the camera's float grid.
  NEW **§5.68** (dual place setters not equivalent — old clamps, the camera does not);
  **B34** gains `position save`/`position load` (read AND restore reach the old observer
  only — F24's class, deliberately not folded); **B35** gains the mount write-half
  precondition. Full battery + b24/b25/b40/b4 + f15/f20/f21×2/f22/f18 green; frozen md5
  in == out.
  **Supervisor-verified 2026-08-01** (§11.131 read in full; commits/authors/trees
  checked; B33-CLOSED + §5.68 + B34/B35 annotations + capability-surface §3.2 verified
  at the ledger; harnesses not re-run — per-member both-ways tables, two-entry pin
  traversals, and committed instrument records accepted). Deviations ALL ENDORSED:
  (1) instrument-first commit — the class survived four audits precisely because no
  observable channel existed; building the channel before measuring IS the lesson;
  (2) `CoreBackup` refusal — folding the read alone would store the drawn place and
  restore it into the path that is not drawing: read and write halves move together
  or the pair goes incoherent; routed to B34, which THIS round's F24 absorbs;
  (3) `dragView` not folded (old-path unprojection unchanged, §11.52(b));
  (4) mount write-half unmirrored — zero callers ⇒ a mirror could not be shown to do
  anything; *a green build is not coverage* applied against the fix itself;
  (5) the non-zero regression half — 1965 px>8 below in-run floors 2063/2052,
  attributed to the float-grid place quantum with the improvement argument (both
  authorities now on the drawn grid; §11.130(f)'s precision rule mirrored). Sky-lock
  auto-enable divergence routing endorsed (§11.58(iii)/D15, Vixy's). §5.68 → section 3.

### F24 — B34 mechanical seam mirrors: `clear`, `preload`, trail fresh-restart, `position save/load`  [M]

- **Row / recorded:** B34 (§11.108(f)(k)); members IN scope, each with its recorded
  source route (as-of-§11.108 — re-locate at HEAD): **(a) `body action clear`**
  (`core.cpp:865` → `ProtoSystem::removeSupplementalBodies`) — script-added NEW-path
  bodies survive the clear; **(b) `body action preload`** (`core.cpp:827-834`) —
  `ModularBody::preload` has never been called (B36: it drags
  `BasicMesh`/`LayeredMesh`/`BodyModule::preload`); **(c) trail fresh-restart** —
  `CoreLink::startPlanetsTrails` (`coreLink.cpp:1144`), live callers `core.cpp:370`
  (config) + `core.cpp:1833` (`setHomePlanet`), both old-only ⇒
  `TrailModule::startTrail` never runs. Fix shape recorded in-row: *"mirror at the
  `SSystemFactory` seam, verify on the live app through the command"*
  (§11.45/§11.46/§11.65 precedent). **(d) `position save`/`position load`** [ADDED at
  dispatch, session 8 — F23/§11.131(f)(j) routed it here]: `CoreBackup::saveBackup`
  reads `core->observatory->get{Latitude,Longitude,Altitude}` DIRECTLY
  (`backup_mgr.cpp:66-68`) and `loadBackup` restores via `core->observatory->moveTo`
  (`:57`) — old alone; the fov half was mirrored at §11.45 T7. The coherent fix is
  read half (the F23 getters) + write half (the dual seam) in ONE change; F23's
  refusal to fold the read alone is the argument. Discriminating shape from the
  ledger: `position save` → move → `position load` → the CAMERA is back.
- **Why now:** decision-free parity (the standing §11.52(b) mandate), and (b) is the
  first live exercise of a declared capability chain — find-at-first-exercise defects
  are the point, not a risk (B5 precedent: 3 defects at the first live exercise of
  drawNested/drawStarProxy).
- **Task:** (a) clear-mirror — new-path script-pushed bodies removed; respect B39's
  nesting/hidden machinery (a hidden pushed body must also clear — verify against
  old's rule, don't assume) and F15's `declaredParams` (a cleared body's declaration
  must NOT be written by a later `body action save`); (b) preload-mirror — wire the
  seam so the command reaches `ModularBody::preload`; MEASURE that it does something
  (load-state/timing observable), and fix what first exercise exposes (in-class,
  record each); (c) trail-restart-mirror — both events reach
  `TrailModule::startTrail`; `b11_trail_gate.py` extended, re-pointed never loosened.
- **Stop boundaries (NOT yours):** the interactive VIEW ramp + zoom ramp (B34's big
  member — own future task; it carries the one feel-reproduction surface); B36's
  `pin()`/`unpin()` (S4's client); B37/B35 command-surface additions (Vixy
  spellings); old path unchanged; B39's pre-existing reload loss of 2 hidden bodies
  (record if touched, don't absorb).
- **Discriminating checks:** per mirror BOTH WAYS, on the live app THROUGH THE
  COMMAND: (a) pushed new-path body gone after clear (name lookup + screen) vs
  SURVIVES on the pre-fix binary; old-path clear behavior unchanged (its removal set
  identical); (b) the preload observable flips (pre-fix: no call — post: measured
  effect); (c) trail-point reset on `setHomePlanet` with recording on, vs no reset
  pre-fix; suites: b11_trail_gate, the b24 suite, f15_persist + f20/f21 session
  suites (the clear interacts with persistence), battery green; frozen md5 in==out.
- **WIP:**

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
- **Session-7 veto points (2026-08-01 — all implemented-and-live, each cheap to
  reverse; silence = endorsed):** (1) F20's save-trigger spelling **`session action
  save|load [filename <name>]`** — the sibling of `body action save`, one `else if`
  each way (§11.128(f), B28 protocol); (2) F21's **`flag satellites` polarity fix** —
  the toggle was a no-op in one direction because the command read the stored HIDE
  bit as the SHOW flag; it now toggles both ways (behavior-visible on one shipped
  verb direction; caught by T4 failing by one line, §11.129(c)); (3) F18's
  **px-authority conversion** is behavior-preserving at 2048 by exact arithmetic
  (D8 as-if) — the decision it EXPOSES is A41, not the conversion itself; (4) F21's
  restore-annotation may REWRITE the loaded session file (machine-owned, idempotent,
  §4.2's own instruction — §11.129(h)).
- **NEW decidable rows from session 7 (all recorded in §13.A / §5, none blocking a
  current dispatch):** **A41** (G4 early-visibility gate: the named constant always
  said 2 px, the shipped gate is 3.07 px — which is right is a product question;
  `RAYMARCH_MIN_SCREEN_SIZE` rides it) · **A42** (texture-level switch: old 180 px vs
  new 409.6 px — rec ALIGN to 180 while both paths exist, cost = big texture resident
  earlier, a D5/D6/D10 call) · **A43** (the Sun + Moon `-preview` assets are DIFFERENT
  PICTURES from their full-res partners — the colour step users see at the gate is the
  DATA; rec regenerate both via `spacecrafter-data`, D9 forward-only) · **§5.64**
  (`timerate action pause` does not stop the simulation clock — making it hold
  changes what a shipped script's pause does mid-show; semantics = yours).
- **B31 STATUS after session 7 — no dispatchable remainder:** slices 3+4 landed
  (§11.128 the session file + §5.32 precondition; §11.129 the read-half authority +
  bulk rows + per-body ledger). The row's ENTIRE open set is now: **T3 (rides your
  D21, late Aug) · `heading` (rides your D28, late Aug) · C4's non-body catalogue key
  (D34's unanswered half) · §5.63** (the T1 photometric blocker — 3-wave hunt spent,
  seven exclusions recorded, next probe named). Answering D21+D28+C4 and closing
  §5.63 closes B31.
- **Awareness, no action needed: §5.62** (a same-binary mid-band disc ratio shifted
  between two epochs of one session, reproducibly — cross-epoch ratios are not
  trusted; in-epoch pairs only) · **§5.63's correction** (the residual is DISJOINT
  lit sets ~1.1° apart, not a photometric wash — the divergent term is on the
  RESTORED side and varies restore-to-restore).
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
