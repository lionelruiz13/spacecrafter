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
**Round outcome (session 8 close, 2026-08-01):** F22 → §11.130, F23 → §11.131,
F24 → §11.132 — all three delivered AND supervisor-verified same session.
**§5.63 ATTRIBUTED, FIXED, CLOSED — b31-design §6.2 T1 MET**: the residual was the
old path's view DIRECTION, which no session ever carried (107.634° with every camera
field exact), plus a sky lock latching a wall-clock-dated stale pair inside the
restore's frame-less batch (= exclusion 7's restore-to-restore variance); three of
§5.63's own recorded statements corrected en route (a low-spatial-frequency layer is
not a displacement witness; the rotation fit excluded a roll, not a direction; a
sky-locked scene's A/A floor is not 0). **B33 CLOSED** — all four remaining getters
read the path that draws, and TWO had REAL shipped divergence channels (`camera
action descend` splits the altitude authorities by design; four old-only sky-lock
write sites made the toggle dead in one direction — the §11.129 `flag satellites`
class one layer up). **B34's four mechanical members CLOSED** (clear / preload /
trail-restart / position save-load; `ModularBody::preload` ran for the FIRST TIME
EVER and its first exercise found a dropped shipped argument) — the row's one
remaining member is the interactive VIEW/ZOOM ramp pair (feel surface). **B31's
open set is now exactly T3/D21 · heading/D28 · C4's key — ALL Vixy's**; answering
those three closes B31. NEW rows **§5.65–§5.69** (all record-only this round:
§5.65/§5.69 decidable, §5.66 rides §11.92(d), §5.67/§5.68 awareness). Executor
quality: F22 attributed in ONE wave what F21's three could not — the owed
INSTRUMENT was the difference; F23's class survived four audits because no member
had an observable channel; F24 caught its own RED half reading an ABSENT INSTRUMENT
as an absent effect and split the delivery so one script measures both binaries —
**instrument-first, now three-times-proven, goes into future task specs as
structure** (instrument commit, then mechanism commit). Remaining dispatchable,
next round: **B34-ramps to mint** (interactive view/zoom mirror — EXACT-parity
reproduction of `navigation->updateMove` on `Camera::lookRel`; any point where
exact reproduction is impossible becomes a Vixy feel item, not a silent choice),
**§5.62's owed isolation measurement** (S — the pre-§5.52 binary in the current
epoch), **B7-hunt-5 only after A40**, **B12 content** after b12-design §7 +
A41/A42, **§5.64/§5.65 fixes** only after Vixy decides the semantics.
DECISIONS_PENDING open set at close: **D15, D21 (late Aug), D37** + A40–A43
awaiting; B31 closure now rides entirely on D21/D28/C4.

**Update [Fable 2026-08-02, supervising session 9]:** round of 3: **F25 → F26 → F27**.
F25 = **B34-ramps** (session 8's queued item — the row's last member; minted at
dispatch per §0b.2). F26 = **§5.62's owed isolation measurement** (queued, S). F27 =
**§5.47's drain path** — NOT in session 8's queue enumeration; authority check at §5:
the row carries a recorded fix shape (*"follow `outputQueue` through
`ServerSocket::run`'s send loop"*), names no Vixy decision, and is
instrument-load-bearing (the ONLY side-effect-free read of observer position + heading;
its silence forces every heading pin through a write-after-read channel at two launches
per pin, §11.118(f)). Lateral search live ∪ archive found no deferral record ⇒ the
omission is view staleness, same class as session 8's F23/F24. Warm-up: both trees
clean (code `d88f5be2`, harness `87ae3f1`), binary confirmed current by no-op rebuild;
`free -g` = 13 GiB available ⇒ **-j6 this session**; NO Vixy commit since session 8's
close ⇒ A40–A43/D15/D21/D28-residue/D37/C4 all open — B7-hunt-5, B12-content,
§5.64/§5.65 stay blocked; B35/B37/B38 decision-gated, B36 per-member (pin/unpin rides
B1), B39/B27 done. Dispatch order = queue order; F25→F26 order-independence CHECKED,
not assumed (F26 lands no code; its frozen scene never exercises F25's key-path diff;
F26 builds both its binaries fresh at named commits regardless of HEAD).

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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F24 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129 ·
F22 §11.130 · F23 §11.131 · F24 §11.132.
Live sections this round (session 9): **F25 · F26 · F27** below.*

### F25 — B34's last member: the interactive view/zoom ramps act on the path that draws  [L]

- **Row / recorded:** B34 row (INTENT.md §13.B) — the view ramp is MEASURED, not read:
  arrow key → `Core::turnLeft/Right/Up/Down` → `vzm` → `Core::updateMove` →
  `navigation->updateMove` (`core.cpp:1817-1826` at the row's compile), with NO
  `Camera::` mirror (`Camera::lookRel`'s only caller is `Core::dragView`,
  `core.cpp:1754`) ⇒ under the new render path **the arrow keys turn nothing**. Live
  probe on record: `harness/xkey.c` + `f4_keyprobe.py` (XTEST, Left held 2500 ms) —
  positive control OK (old moves 73 777 px>32), new camera az/alt **bit-identical**,
  same body in the same dump: old moves 872.79 px, new |Δ| 0.000e+00 NDC. Sibling by
  source, **unprobed**: the continuous ZOOM ramp (`core.cpp:1799-1801` →
  `Projector::changeFov` only). Precedents: §11.108(d) = the altitude ramp (B21),
  whose coefficient transferred verbatim — the shape to follow where it holds; F4 §11.108
  did NOT fix this member because the turn needs `navigation->updateMove`'s
  sign/cadence/fov-scaling reproduced on `Camera::lookRel` — an interactive-FEEL
  surface. Instruments that now exist and F4 lacked: `body action dual_dump`'s
  `control` object (§11.131), `Camera::getPlace()` = exact inverse of `moveTo`
  (§11.131), the `oldView` readback (§11.130). All file:line pointers are at the
  row's compile — **re-locate every route at HEAD first** (F24's opening move).
- **Mandate [session-8 queue, verbatim]:** *"EXACT-parity reproduction of
  `navigation->updateMove` on `Camera::lookRel`; any point where exact reproduction is
  impossible becomes a Vixy feel item, not a silent choice."*
- **Why now:** B34's ONE remaining member — the last recorded operator action where
  the drawn universe ignores the operator; every mechanical sibling landed (§11.132).
- **Task:** (i) **Instrument first, own commit** (three-times-proven, session-8
  structure): whatever per-step readout the parity measurement needs on BOTH paths
  (additive only; dual_dump precedent), wired to nothing; the pre-fix binary is a
  build of that commit so ONE script measures both. (ii) **Derivation-diff
  `navigation->updateMove`**: enumerate EVERY term of the turn ramp — sign
  conventions, cadence law (per-frame vs time-based, accel/decel of discretionary
  movement), fov scaling of the step, clamps/poles, interaction with tracking/lock —
  as a committed table; then map each term onto `Camera::lookRel`/the camera frame.
  A term with no exact camera equivalent = **named Vixy feel item** (record, stop on
  that term — never approximate silently). (iii) Wire the turn mirror at the seam
  the route already has (the `Core::updateMove`/vzm layer, where the altitude ramp
  mirrored) — old path unchanged by construction. (iv) ZOOM ramp: **probe before
  mirroring** — if the new path consumes the same `Projector` fov, the ramp may
  already reach it; measure, then mirror only what is measured missing, same
  term-table discipline. (v) Verify through the LIVE key channel: extend
  `f4_keyprobe` — delivered binary moves BOTH paths under XTEST hold with per-step
  deltas EQUAL at the row's own bar (px>32 + az/alt readouts); pre-fix binary
  reproduces F4's asymmetry; key-RELEASE deceleration parity; diagonal (two keys);
  cadence at two fovs (the fov-scaling term is where a feel divergence hides);
  zoom ramp both-fov readouts converge identically.
- **Stop boundaries (NOT yours):** old-path behavior (§11.52(b) baseline); any
  inexact-reproduction point (→ Vixy feel item, recorded); B35 mount write-half;
  §5.66 look_at family (rides §11.92(d)); new command spellings (B28 — the ramps are
  key-driven; want a command? record the want); D21/D28-residue/C4; §5.62's
  consequence binds you — in-epoch pairs only, never cross-epoch.
- **Discriminating checks:** both-ways XTEST ladder (delivered: both paths move,
  deltas equal; pre-fix: new bit-identical while old moves); release-deceleration
  both-ways; battery + f-suites green; frozen md5 in==out; §11.121(m)
  concurrent-instance assert; §11.123(o2) cadence caution — counter ratios and
  in-run A/B only, never absolute fps labels.
- **WIP:** 2026-08-02 — **DELIVERED, B34 CLOSED.** Code `dc95d0de` (instrument, nothing wired)
  + `d9de42ac` (turn + zoom mirrors); harness `f823af9`/`d1f1cad`/`578575d`/`892ea0d`/`6c4ab44`/
  `c9b1989`/`a91e7fb`/`501acf7`. Record: **§11.133** (`INTENT/11.133.md`, 12 sections incl.
  the 24-term derivation table), §13.B **B34 CLOSED**, NEW **§5.70** (the recorded turn action
  is unreplayable) + **§5.71** (`Core::panView` old-only, the row's one residue, rides
  §5.66/§11.92(d)), `capability-surface.md` §3.2 rows flipped, `harness/README.md` F25 section.
  **No Vixy feel item owed** — the one inexact term is float32, predicted exactly (pole pins
  9.0037e-08 rad apart by construction, measured 8.997e-08). `f25_ramp` 94 PASS delivered /
  69 PASS pre-fix, 0 FAIL either way; `f25_drag` ALL PASS both ways; battery exit 0 + scene E
  0 FAIL; b24×2, f15, f20, f22, f23, f24 green; frozen md5 in == out on every launch.
  **Supervisor-verified 2026-08-02** (§11.133 read in full; commits/authors/trees checked;
  diff surface verified at the diff level — `navModule/navigator.cpp` absent from the diff,
  core.cpp/hpp additions only, the single deletion is Camera.cpp's snap round-trip removal;
  B34 flip + §5.70/§5.71 rows + §11.133 stub verified at the ledger; harnesses not re-run —
  per-step both-ways ladders with the pre-fix bit-identical column + committed artifacts
  accepted). Deviations ALL ENDORSED: (1) `lookRel` carries old's convention — I2, the
  convention stated once where it is written, both call sites handled; the `dragView`
  vertical-sign fix it forced is a measured pre-existing defect (both-ways record with the
  agreeing azimuth as control) and moves parity the mandate's direction; (2) gdb-driven drag
  leg — the XTEST motion channel positively mapped dead (root 0×0), §11.108(d) precedent,
  layer above source-verified and stated; (3) `f4_keyprobe` annotated as a pre-fix witness
  rather than edited — F4's record preserved, asserts-closed-defect stated; (4) the pole-snap
  ASSIGNMENT — D8 as-if strictly-more-exact, residual predicted-then-measured (9.0037e-08 /
  8.997e-08 rad), the unjustifiable-epsilon feel item avoided by removing the primitive's own
  loss (zero-patch class); (5) B34 CLOSED with `panView` routed to §5.71 — B33→B34/B35
  routing precedent, the member is F25's own find, visibility + one-line fix shape preserved.
  **No-feel-item claim endorsed at the row's own bar**: the 24-term table carries an exact
  equivalent or a measured discrimination per term.

### F26 — §5.62's owed isolation: the pre-§5.52 binary in the current epoch  [S]

- **Row / recorded:** §5.62 VERBATIM (INTENT.md §5; recorded by F18, §11.127(e)):
  the identical source measured the identical frozen scene (mid-band disc, halo
  suppressed, disc-integrated new/old ratio) at 23:05 → Sun 0.8356 / Mars 0.8770 /
  Jupiter 0.9949, and at 00:02 + 00:05 → Sun 0.8356 / Mars 0.9198 / **Jupiter
  1.2909** (brighter than old — neither noise nor a wash); build diff COMMENTS only;
  the two late runs agree to the digit; OLD path bit-stable across epochs (Jupiter
  old_disc 3908 both sides); every documented precondition clean at the second
  epoch; `active.lock` = rotation marker only (retraction recorded). Owed IN-ROW:
  *"the same scene on the pre-§5.52 binary in the current epoch, which isolates
  whether the shift lives in the delivered code at all or in the app's accumulated
  state."* Consequence in force for everyone: in-epoch pairs only.
- **Why now:** S; owed before §5.62 can be judged; every future mid-band measurement
  carries the in-epoch-only constraint until this discriminates.
- **Task:** (i) Reconstruct the scene + measurement exactly per §11.127(e)/F18's
  harness — if the harness has drifted and the scene cannot be reproduced, STOP and
  say so; never substitute a weaker scene. (ii) Identify at the git log (never
  recall) the §5.52 fix commit and its PARENT; build BOTH in separate build dirs —
  the one-variable pair is (pre-§5.52 parent) vs (§5.52-carrying child `2117ccb0`
  or the F18 delivery commit as logged), NOT vs current HEAD (F25 may have landed;
  keep the pair tight). (iii) In ONE epoch — same session, minutes apart, fresh
  launches, §11.121(m) assert before each — measure both binaries, with an in-epoch
  A/A repeat per binary (the row's precedent: agreement to the digit). (iv)
  Discriminate and EXTEND the row: pre-§5.52 shows the shifted values too ⇒ the
  shift lives in accumulated app/host state, delivered code exonerated; pre-§5.52
  shows the first-epoch values while the child shows the shifted ones in the same
  epoch ⇒ the code is implicated and §5.62 escalates from record-don't-chase (say
  so; do not chase further). Either way the row gains the measurement + its
  conclusion at the row's own bar. (v) NO fixes, NO chasing beyond the owed
  measurement — anything new = record as a row.
- **Stop boundaries (NOT yours):** any fix; texture-cache/config mutation; old
  path; cross-epoch comparisons (the row's own consequence).
- **Discriminating checks:** old_disc bit-stability replicated in every launch (the
  row's own control); per-binary in-epoch A/A agreement to the digit; frozen md5
  in==out; fresh launches; no `t-*.dat` written during the session (the row checked
  this — re-assert it); concurrent-instance assert.
- **WIP:** 2026-08-02 — **DELIVERED (§11.134). NO code** (code untouched at `d9de42ac`);
  harness `42904a7` (instrument + reconstruction) + the delivery commit. Pair at the git
  log: fix `94eb3f03`, parent `96a94a46`, child `2117ccb0`; rebuilt in worktrees, md5
  `df00c3e3` / `23ac7fdb` (staged as `harness/sc_f26_{pre,child}`). ONE epoch
  16:00:31–16:12:02, four interleaved fresh launches, every precondition recorded per
  launch. **Result: the shift is on NEITHER binary** — the child reads the FIRST-epoch
  numbers (Jupiter 0.9949 / 0.9939, new_disc 3888 / 3884 vs epoch B's 5045; Sun 19666
  bit-identical), the pre binary reads its own recorded signature bit-identically twice
  (0 / 0 / 444; old_disc 23536 / 7683 / 3908), A/A floor 0 / 39 / 4 counts. ⇒ delivered
  code exonerated, **no escalation**, transient of the epoch-B session, in-epoch-only
  consequence stands; §5.62 stays OPEN (owed item discharged, not attributed). The task's
  dichotomy is answered by a third branch — the pre-§5.52 binary cannot carry the shifted
  observable at all, so the discriminating leg is the child in a new epoch (recorded).
  §5.62's own `active.lock` statement REFUTED en route.
  **Supervisor-verified 2026-08-02** (§11.134 read in full; code tree confirmed untouched
  and clean at `d9de42ac`; harness commits/authors checked; §5.62 row extension verified at
  the ledger — discrimination + refutation + STAYS-OPEN with the named next step; the
  one-variable pair argued at three levels — §5.54 gate bit-identical at 2048, submodule
  pointer identical across all four commits, Renderer byte-identical child→HEAD; the
  instrument validated against the row's own five artifact sets to the digit BEFORE being
  trusted). Deviations ALL ENDORSED: (1) -j8 at 38 GiB measured — inside the rule with OOM
  headroom; (2) worktrees — strictly better abort tolerance, main tree never moved; (3) the
  §11.121(m) probe REPLACED — the stock pattern both under-reports (blind to out-of-tree
  binaries) and over-reports (pgrep -f self-match, measured 3 with nothing running), the
  `/proc/<pid>/comm` replacement positively mapped both ways (decoy 1 / without 0) — the
  instrument-chain bar applied to a standing precondition, PROPAGATE to §0.5 at close;
  (4) run.log unlinked by the driver's own rm — every lost datum enumerated as held
  elsewhere, wrapper fixed forward, re-run correctly refused to preserve the interleaving;
  (5) the third-branch answer — the row's own owed-statement was one binary short, my F26
  spec inherited it, the executor corrected rather than glossed (endorsed as
  supersession-with-record). Verdict ENDORSED at the row's own bar: transient of the
  epoch-B session; exoneration rests on a 290×-floor discrimination.

### F27 — §5.47: the reply that never arrives — `get status position`'s drain path  [S–M]

- **Row / recorded:** §5.47 VERBATIM (INTENT.md §5; recorded by F12, §11.118(i)):
  `AppCommandInterface::commandGet` resolves the argument and calls
  `tcp->setOutput(coreLink->tcpGetPosition())`; `ServerSocket::setOutput` pushes
  onto `outputQueue` under its lock and returns; **nothing arrived on the driving
  socket in a 6 s poll, twice**, in a session where `timerate`, `date`, `flag`,
  `select`, `body action dual_dump`, `body action screenshot` all worked on that
  SAME connection; no "No tcp : i can't send" in the app log. Consequence: this is
  the ONLY side-effect-free read of the observer position AND the heading readout —
  its silence forces every heading instrument through `heading delta_azimuth`,
  which WRITES both authorities after reading (two `experimental_path` pins needed
  two launches, §11.118(f)). Fix shape IN-ROW: *"follow `outputQueue` through
  `ServerSocket::run`'s send loop and find out whether it is drained at all,
  drained to a different socket, or drained only when a second client is
  connected."*
- **Why now:** instrument-load-bearing for every future control-surface task; no
  Vixy decision named; the drain path is a TCP-server question isolated from the
  body path.
- **Task:** (i) Positively map the drain at source: `ServerSocket::run`'s send
  loop, the `outputQueue` lifecycle, WHICH socket the drain writes to and under
  what condition (the row's three hypotheses). (ii) If not decidable at source,
  instrument first (own commit): a D12-class log at the drain decision (socket id,
  queue depth) — additive. (iii) Fix at the root: a get's reply lands on the
  connection that issued it. If the routing is a DESIGN (replies to a different
  channel on purpose) ⇒ that is semantics: record + STOP, Vixy's. (iv) Verify
  live, both ways: F12's 6-s-poll scenario reproduced — pre-fix binary silent,
  delivered binary replies on the driving socket; the six working commands
  unchanged on the same connection; a second-client leg if the mechanism
  implicates one. (v) Reply CONTENT verified against dual_dump's `control` object
  (B33's bar reaches this channel: the readout must agree with the path that
  draws). (vi) Note in the entry that heading pins can now use the read-only
  channel; ONE demonstration, no harness rewrites (future tasks pick it up).
- **Stop boundaries (NOT yours):** protocol redesign (framing, new commands); other
  `get` handlers beyond what the root requires; B37/B38 surfaces; reply-routing
  semantics if that is what the root is (→ record + suspend).
- **Discriminating checks:** both-ways driving-socket scenario (pre-fix: 6 s
  silence ×2; delivered: reply within the poll); working-commands control set
  green on the same connection; battery green (TCP-touching suites); frozen md5
  in==out; concurrent-instance assert.
- **WIP:** 2026-08-02 — **CP1: the drain is MAPPED, at source AND on the wire; no code touched yet.**
  The queue IS drained, on every pass of `ServerSocket::run` (`checkDataToSend`, io.cpp:388 +
  630-639, the loop's own wait is a 1 ms `SDLNet_CheckSockets`), and it is drained to
  `broadcast` (io.cpp:641-655), which sends to every client whose `clientBroadcastTab` entry is
  true — a table set true in exactly ONE place, the `$LOGON` command (io.cpp:614). So the reply
  goes to the FEEDBACK SUBSCRIBERS, never to the issuer as such, and with no subscriber it is
  popped and lost. The addressee is dropped at the FIRST hop: `computeNormalString(client, …)`
  knows the connection and pushes the bare string (io.cpp:625). Hypothesis 3 (second client)
  REFUTED — the client count is irrelevant, the subscription is the condition. Measured pre-fix
  (`harness/sc_f27_pre`, md5 `42f83cd3`, = code `d9de42ac`), `harness/f27_reply.py`: A 6 s
  silence ×2 on the driving socket with all six §5.47 commands green on that same connection
  (select 1 216 563 px>8, 2 166 878 lit px, 0 refusals); B the SAME socket after `$LOGON`
  receives the reply in **0.006 s**, content == the dump's `control` (alt/heading authorities
  diverge, reply follows NEW); C listener 1 copy / issuer 0, twice; D script-`get` → subscriber
  only; E 0/3 read-only heading samples. Next: root fix (carry the issuing connection from input
  to output).
  **CP2 (2026-08-02): root fix landed + verified both ways.** Code `d13681eb` (`src/tools/io.{cpp,hpp}`
  only): a request and an answer are a `ClientMessage` carrying the SLOT and the connection ID that
  slot held, `getInput` latches the request being served and clears the latch when the batch is
  drained, `setOutput` stamps the answer, `deliver` sends it to that connection AND (unchanged) to
  the `$LOGON` subscribers, minus the double copy. Delivered binary md5 `8a93ca97`. `f27_reply.py`
  A–F **0 FAIL both ways**: A reply in **0.002 / 0.006 s** on the driving socket (pre: 6 s silence
  ×2), content == `control.reported` per field with the heading authorities **6.16° apart** and the
  reply following NEW; B one copy, never two; C issuer 1 + listener 1, before AND after the issuer's
  connection is replaced (pre: 0 + 1 twice); D/F1 a script's answer still goes to the subscribers and
  never to the last speaker, and the app now LOGS the discard when nobody can receive it (pre:
  silence); F2 an HTTP-issued answer (issuer already hung up) falls back to the subscribers and the
  next tenant of the slot inherits nothing; E **3/3** heading samples in ONE launch across two pin
  toggles (pre 0/3). Regressions green: battery exit 0 + scene E 0 FAIL, b16 channels (2/2 probe
  entries), b10_cmd ALL PASS, frozen md5 in == out everywhere. Next: ledger (§11.135, §5.47 flip,
  the two new rows) + the doc supersessions.
  **DELIVERED 2026-08-02 — §5.47 CLOSED.** Code `d13681eb` (`src/tools/io.{cpp,hpp}` only; no
  instrument commit was needed — the drain was decidable at source and the pre-fix binary is HEAD's
  own build, so ONE script measured both). Harness `dba0ccd`/`6ec172a`/delivery commit. Record:
  **§11.135** (`INTENT/11.135.md`, 12 sections), **§5.47 FIXED** with the three hypotheses answered
  in-row, NEW **§5.72** (the `$LOGON` channel conflates the log feed with other clients' answers —
  preserved deliberately, semantics = Vixy's) + **§5.73** (`strcpy` of a `MAX_BUFFER+1` answer into
  a `tcp_buffer_in_size` buffer: 1023 bytes overflows, pre-existing on both binaries),
  `capability-surface.md` §3.2 statement superseded, `harness/README.md` f27 section + the f12
  section annotated, `f23_b33_control.py` docstring annotated. **ONE VETO POINT for §3** (implemented
  and live, one condition in `ServerSocket::deliver` reverses it): the addressed copy is **additive**
  — the `$LOGON` subscribers keep receiving every answer exactly as before, so no shipped client
  loses a delivery; whether they SHOULD is §5.72. Two in-file residues fixed with the root and stated
  as deviations: the unlocked `inputQueue` push (the routing reads what was pushed) and the never-
  released `clientBroadcastTab` (a third such array was being added). §11.118(f)'s recorded cost is
  discharged: heading pins are now readable 3/3 in ONE launch.

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
- **Session-8 veto points (2026-08-01 — all implemented-and-live, each cheap to
  reverse; silence = endorsed):** (1) F22's **`sky_vision` key in `[observer]`**
  (B28 class) — the old path's view direction, which no dual seam carries (B19's
  exclusion clause measurably false for this member, §11.130(k)); one key + one
  setter to reverse, older builds ignore it (D9/D13); (2) F22 asserts the old
  path's **offset latch through a restore-only setter rather than an aim** (an aim
  would move the view; the alternative couples what D32 separates); (3) F24's
  **replace-inherits-NAME's-provenance** (`body action load … replace true` on a
  file-declared body stays UNclearable — the requirement old's shape encodes: a
  clear never removes declared data, §11.132(b)); (4) F24 adds **no second
  clear-guard** for a camera referenced on a pushed body (old's guard covers old's
  home planet; the I5 destruction contract redirects the camera — a second guard
  would be a NEW user-visible rule, stated not invented).
- **NEW decidable rows from session 8 (recorded, none blocking):** **§5.65**
  (`moveto` + `flag lock_sky_position on` in ONE script block latches the PREVIOUS
  frame's sky — fixing the seam changes what a shipped command does mid-show;
  semantics = yours) · **§5.69** (`body action preload keep_time` truncated to
  8 bits — 3 s×144 fps = 432 → 176 frames, the command's own 10 s default → 160;
  honoring the documented seconds changes shipped-show texture residency, a
  D5/D6-adjacent call; the fix itself is one field width).
- **Session-8 awareness, no action needed:** **§5.66** (`look_at`'s camera half and
  old-navigator half land 94.4° apart — B9 az-convention family, rides your
  §11.92(d)) · **§5.67** (after any `look_at` the old path runs with norm-2 vision
  vectors; `constellation.cpp`'s art-fade dot test is twice as permissive —
  pre-existing old-path behaviour) · **§5.68** (the dual place setters are dual but
  NOT equivalent: old clamps lat ±90°, maps 0→1e-6, floors alt at 0.1 m; the camera
  clamps nothing — a write-half asymmetry every `moveto` already has).
- **B31 STATUS after session 8 — T1 MET, the session feature is functionally
  complete:** a saved session now restores the camera, the old path's sky direction,
  the view offset on both paths, flags/values/colours and the per-body ledger, with
  T4 byte-identity and T10 at 0. The row's ENTIRE open set: **T3 (rides your D21,
  late Aug) · `heading` (rides your D28, late Aug) · C4's non-body catalogue key
  (D34's unanswered half)**. Answering D21+D28+C4 closes B31.
- **B33 CLOSED (session 8) — two of its four last members were lying on SHIPPED
  commands** (altitude readout during `camera action descend`; the sky-lock toggle
  dead in one direction after select-while-tracking). Residues named in-row and
  routed: the four old-only sky-lock write sites ride your **D15/§11.58(iii)**;
  the mount write-half rides **B35**'s spelling (its requirement is stated at the
  setter).
