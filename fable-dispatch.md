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

**Update [Fable 2026-08-08, supervising session 10]:** round of 3: **F28 → F29 → F30**.
F28 = **§5.73's fix-shape verification** (session 9's named queue head; authority check
at §5: the row owes a measurement nothing has taken and a source verdict — dispatchable
as verify-then-fix-only-if-decision-free). F29 = **§5.46's up-chain flat frame** — NOT
in session 9's queue enumeration; authority check at §5: recorded fix shape (*"assign
`matLocalToBodyPos = flat` in the up-chain loop"*), named discriminating check (A/B an
up-chain ancestor's orbit/trail), no Vixy decision named — B39's record-don't-fix was
that task's scope boundary, not a gate; lateral search live ∪ archive found no deferral
record ⇒ view staleness, session 9's F27 class. F30 = **the decision-feeder sweep**:
the owed pre-decision data of §5.60/§5.64/§5.65/§5.69/§5.70/§5.72 — the session-9
deferral note gates those rows' FIXES (semantics = Vixy's, untouched); each row names
an owed datum that is upstream of the decision and decision-free, and F26 is the
no-code-delivery precedent. Warm-up: both trees clean modulo four session-9 instrument
binaries (`harness/sc_f25_pre`, `sc_f26_{pre,child}`, `sc_f27_pre` — gitignored IN
PLACE this session so §5.62's/F27's recorded paths stay valid; regenerable from the
named commits); binary confirmed current by no-op rebuild; `free -g` = 52 GiB ⇒ -j12;
NO Vixy commit since session 9's close ⇒ D15/D21 (late Aug), D37, A40–A43, C4 all
open — B7-hunt-5, B12-content, §5.64/§5.65/§5.69/§5.70/§5.72 fixes stay blocked;
B35/B37/B38 decision-gated, B36 per-member (pin/unpin rides B1). §5.34 (Object
`operator=` leak — needs its own raw-`ObjectBase*` holder enumeration, old-path
lifetime consequences) noted as a next-round candidate, not taken over the sweep: one
M-sized fix with baseline risk vs six decision un-starvations. Dispatch order =
F28→F29→F30; order-independence CHECKED, not assumed: F28 = `src/tools/io.{cpp,hpp}`,
F29 = the experimentalModule dispatch walk (disjoint files); F30 lands no code, and
its one §5.60 launch reads a startup-time allocation size neither fix touches.

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
**Round outcome (session 9 close, 2026-08-02):** F25 → §11.133, F26 → §11.134,
F27 → §11.135 — all three delivered AND supervisor-verified same session.
**B34 CLOSED** (F25): turn + zoom ramps mirrored with per-step parity through the
live key channel; the 24-term derivation table needed NO Vixy feel item — the one
irreproducible term was float32, not feel, and was closed by removing the
primitive's own round-trip loss (D8); `dragView`'s vertical sign found inverted
since its merge and fixed where the convention is stated. **§5.62's owed item
DISCHARGED** (F26): the epoch shift is on NEITHER binary rebuilt today ⇒ transient
of the epoch-B session, delivered code exonerated, row stays OPEN unattributed;
the row's own `active.lock` explanation refuted; §11.121(m)'s probe REPLACED
(`/proc/<pid>/comm` — the stock pattern under- AND over-reports, §0.5 updated).
**§5.47 CLOSED** (F27): the queue was drained all along — to the `$LOGON`
subscribers; the structure never carried the addressee, fixed at the first hop
(`ClientMessage` slot+id); reply follows the path that draws; heading pins now
1 launch. NEW rows: **§5.70/§5.72** (decidable, yours) · **§5.71/§5.73**
(awareness). S6's operator-seam audit fully landed (B33+B34 both closed).
Executor quality: F25 turned the mandate's feel-item escape hatch into a
measured-exact delivery; F26 answered a two-branch dichotomy with the third
branch the row's own wording missed and REBUILT a standing precondition's
instrument on the way; F27 made the pre-fix binary its own positive control
(leg B) — the no-reply-vs-no-instrument bar is now settled practice. Remaining
dispatchable, next round: **§5.73's fix-shape verification** (S — buffer-sizing
vs truncation policy; dispatchable only if sizing-only is confirmed
decision-free at source), **B7-hunt-5 only after A40**, **B12 content** after
b12-design §7 + A41/A42, **§5.64/§5.65/§5.69/§5.70/§5.72 fixes** only after
Vixy decides the semantics. DECISIONS_PENDING open set at close: **D15, D21
(late Aug), D37** + A40–A43 awaiting; B31 closure rides D21/D28/C4 (unchanged).

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
     batch-boundary check is the precedent instrument. **Instrument superseded
     2026-08-02 (F26, §11.134(b)):** the stock `f18_run.sh`-style pattern is BLIND
     to out-of-tree binaries AND any `pgrep -f <path>` self-matches the wrapper
     (measured: 3 reported with nothing running). Use the `/proc/<pid>/comm` probe
     (`f26_epoch.sh`; Python port in `f27_reply.py`) — covers every account,
     positively mapped both ways (decoy 1 / without 0).
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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F27 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129 ·
F22 §11.130 · F23 §11.131 · F24 §11.132 · F25 §11.133 · F26 §11.134 · F27 §11.135.
Sections F28–F30 minted 2026-08-08 (session 10) below — the prior queue note
stands superseded by the session-10 update note above.*

### F28 — §5.73: the shared send buffer vs a ≥1023-byte answer — verify the fix shape, fix only if decision-free  [S]

- **Row / recorded:** §5.73 VERBATIM (INTENT.md §5; recorded by F27, §11.135(j);
  derived from source, NOT reproduced): `setOutput` clamps the answer to
  `MAX_BUFFER` = **1024** `[io.cpp:63, 344-348]`; the send path copies
  `data + '\n'` with `strcpy` — **L + 2** bytes for an answer of length L — into
  `buffer = new char[bufferSize]` `[io.cpp:175]`, where `bufferSize` =
  `tcp_buffer_in_size` = **1024** in the loaded config. So **L ≥ 1023 writes past
  the end**, up to 2 bytes at the clamp. Pre-existing on both binaries; untouched
  by F27's routing change (the `strcpy` ran even when nobody received the string).
  Unbounded producers: `get status object` (`getSelectedObjectInfo`) and `search`
  (`getListMatchingObjects`). Owed by the row: *"measure the two commands' actual
  output length on a shipped scene, which nothing has done."*
- **Why now:** session 9's named queue head; the only §5 fix row whose gate is a
  source verdict rather than a Vixy decision — IF sizing-only is decision-free.
- **Task:** (i) Take the owed measurement first: actual reply lengths of
  `get status object` (a selected shipped body) and `search` (a prefix with many
  matches) on a shipped scene, on the wire. (ii) At source, decide whether a
  sizing-only fix is decision-free. The buffer is SHARED with the receive path
  (`tcp_buffer_in_size` semantics must not change); the honest shapes are
  size-to-message on the SEND copy (the send path allocates/ensures L+2) or a
  separate send buffer sized to the queue's front. Anything that changes
  truncation behaviour, protocol output for currently-working answers, or the
  receive path's config meaning is NOT sizing-only ⇒ record the fork at the row
  and STOP (that is a complete delivery — the row's conditional says so).
  (iii) If decision-free: land it, with the overflow discriminated BOTH WAYS on
  the ASan tree (`build-asan`) — a ≥1023-byte answer must self-name as
  `heap-buffer-overflow` pre-fix and arrive intact, byte-complete, post-fix; the
  native tree must show the same answer delivered unchanged. (iv) F27's
  `f27_reply.py` scenario stays green on the same connection (the routing fix and
  this fix share the function).
- **Stop boundaries (NOT yours):** truncation POLICY (protocol-visible — Vixy's,
  the row says so); §5.72's `$LOGON` semantics (deliberately preserved by F27);
  receive-path behaviour; protocol redesign; other `get` handlers.
- **Discriminating checks:** ASan both-ways leg (overflow self-names pre-fix, 0
  reports post-fix, same drive); reply byte-completeness vs the producer's string
  length; `f27_reply.py` green; battery green (TCP suites); frozen md5 in==out;
  §0.5 concurrent-instance assert (`/proc/<pid>/comm` probe).
- **WIP:** — **DELIVERED 2026-08-08 → §11.136** (code `5b86be0f`, harness
  `1317493`/`2ac284b`/the entry commit). §5.73 **FIXED** with its owed
  measurement in-row; the fix was decision-free because the shared buffer never
  truncated anything. NEW **§5.74** (`search` returns no star and no
  constellation on the shipped corpus). Truncation POLICY remains Vixy's and
  now has a closer case: `get status planets_position` = 854 of 1024 B.

### F29 — §5.46: the up-chain walk publishes a flat frame it never writes  [S–M]

- **Row / recorded:** §5.46 VERBATIM (INTENT.md §5; found by B39, §11.117(k)(2)):
  `dispatchUpdate`'s UP-CHAIN loop (`while (body->isNotIsolated)`) assigns
  `body->mat = parentTilted` and calls `preUpdate`/`update`, and **nothing writes
  `matLocalToBodyPos`** — whose own contract says *"Set on EVERY position update
  (visible or not)"* — while every DOWNWARD path writes it explicitly and says
  why. Consumers: `ModularSystem::drawOrbits`/`drawTrails`/`drawTails` build each
  body's parent frame from `getMatLocalToBodyPos()`, so an up-chain ancestor's OWN
  orbit/trail/tail is placed in a stale frame — reachable on today's corpus
  (observer on the Moon ⇒ Earth is up-chain and carries a TRAIL module). This is
  the class §11.39 believed closed (*"correct for EVERY body"* — true of the
  descent, false of the climb). Fix shape IN-ROW: *"assign
  `matLocalToBodyPos = flat` in the up-chain loop where `mat = parentTilted` is
  assigned, and A/B the orbit/trail of an up-chain ancestor to see what moves."*
- **Why now:** recorded fix shape + named discriminating check + no decision
  named; B39's record-don't-fix was its scope boundary ("changes rendered output
  for a body B39 does not otherwise touch") — rendering the CONTRACTED frame is
  this task's whole mandate, not a side effect.
- **Task:** (i) Re-verify the row at source (it is 9 days old; §5.2 class).
  (ii) Instrument BEFORE fixing: the A/B scene (observer on/near the Moon, Earth's
  trail and orbit enabled) with a measured observable of where the up-chain
  ancestor's orbit/trail draws; commit the PREDICTION first — derive from the
  stale-frame mechanism where the line sits pre-fix (the frame of the last
  descent through Earth) and where the correct frame puts it. (iii) Land the
  one-line fix. (iv) A/B both ways + the as-if control: a scene with NO up-chain
  ancestor (observer on Earth, same content) must be bit-identical pre/post —
  every downward-path body already had the write, so the fix must change nothing
  for them.
- **Stop boundaries (NOT yours):** any old-path change (§11.52(b) baseline);
  orbit/trail rendering quality beyond frame correctness; B39/§5.44 hidden-body
  semantics; §5.27/D21 scaling questions if they intersect the scene (route
  around: `flag moon_scaled off` per the standing harness rule).
- **Discriminating checks:** up-chain ancestor's orbit/trail moves to the
  predicted position post-fix (prediction committed pre-run, both ways on the
  pre/post pair); no-up-chain control scene bit-identical pre/post; battery
  green; frozen md5 in==out; concurrent-instance assert.
- **WIP:**

### F30 — the decision-feeder sweep: six rows' owed pre-decision data, no fixes  [S; read-only + one instrumented launch]

- **Rows / owed items (each verbatim at its row; the fixes are ALL decision-gated
  and NOT in scope):**
  - **§5.60** — *"WHICH allocation this is and whether it is one buffer or one
    pool — the answer decides whether the fix is a policy on pool sizing or a
    fallback path"* (feeds Vixy's D13 device-limit call; recorded §11.125(e)).
  - **§5.64** — *"which consumers read `getTimeSpeed()` and therefore already
    behave as if paused"* (the readout/clock disagreement is the defect shape;
    feeds the pause-semantics call; §11.128(j)).
  - **§5.65** — *"whether any shipped script actually issues the pair
    [`moveto` + `flag lock_sky_position on`] in one block"* (§11.130(c)).
  - **§5.69** — *"whether any shipped show relies on today's truncated
    `keep_time` value"* (the intended-maximum half is Vixy's, not scannable;
    §11.132(g)).
  - **§5.70** — *"whether any archived recording already carries the broken
    [`look delta_az`] line, since fixing the emitter does not fix a recorded
    file"* (§11.133(j)).
  - **§5.72** — *"whether any shipped client subscribes with `$LOGON` and parses
    command answers off that stream"* (§11.135(c)).
- **Why now:** all six fixes wait on Vixy; every row names the datum owed BEFORE
  the decision can be judged; the data is decision-free and mostly read-only.
  Batching un-starves six decisions in one S run. F26 is the precedent that a
  measurement with no code is a full delivery.
- **Task:** per item, acquire exactly the owed datum, with its channel positively
  mapped: **§5.60** — identify the allocation at its callsite (one launch; the
  validation layer names size 2684360960 — a gdb break on `vkAllocateMemory`
  filtered on that size, or the layer's own callstack config; state buffer-vs-pool
  and which subsystem asked). **§5.64** — whole-`src` enumeration of
  `getTimeSpeed()` consumers, per-consumer verdict: behaves-as-paused vs
  reads-the-raw-clock. **§5.65/§5.69/§5.70** — corpus scans: shipped scripts +
  `~/.spacecrafter` data (ISO-8859 — `/usr/bin/grep` or Read, per the standing
  rule), stating the corpus enumerated (paths, file counts) so absence is a
  mapped negative, not a failed lookup. **§5.72** — tree + shipped-tools scan for
  `$LOGON` producers/consumers. Delivery: one §11 entry for the sweep + a dated
  EXTENSION on each of the six rows carrying its datum with provenance tags.
- **Stop boundaries (NOT yours):** NO fixes anywhere (all six gated); no
  EntityCore edits; no asset creation; §5.62 (cross-epoch class, its own row
  forbids the chase); if §5.60's identification exceeds one debug session, record
  the partial + the exact remaining step.
- **Discriminating checks:** every negative carries a positive control on the
  same channel (each corpus grep proven on a string known present; the gdb/layer
  break proven by hitting the 2.68 GB allocation); §5.60's launch under the
  fresh-launch precondition + concurrent-instance assert; no code diff at close
  (`git -C` both repos: harness-only changes).
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
- **Session-9 veto points (2026-08-02 — all implemented-and-live, each cheap to
  reverse; silence = endorsed):** (1) F27's **additive reply routing** — `get`
  answers now reach the connection that issued them AND the `$LOGON` subscribers
  keep receiving every answer exactly as before (minus the duplicate when the
  subscriber is the issuer); one condition in `ServerSocket::deliver` reverses it
  (§11.135(c)); whether the log channel SHOULD keep carrying other clients'
  answers is **§5.72**, deliberately preserved; (2) F25's **`lookRel` convention +
  `Core::dragView` vertical-sign fix** — the new path's mouse-drag vertical now
  matches old (it was INVERTED since the merge that added it, measured both ways
  with the agreeing azimuth as control, §11.133(g)); reversal = one negation at
  the call site; (3) F25's **pole-snap assignment** (§11.133(d)) — the float32
  params→direction→params round trip near the pole ate old's clamp epsilon and
  flipped the azimuth by π per frame (a 180° image roll); the snap now ASSIGNS
  (D8 as-if, strictly more exact), keeping old's own 1e-6 epsilon; the only
  alternative was a 345×-larger epsilon = a user-visible stopping altitude.
- **NEW decidable rows from session 9 (recorded, none blocking):** **§5.70** (the
  interactive turn ramp RECORDS `look delta_az …` — an unregistered command name
  AND unregistered keys, so a recorded show cannot replay an operator's pan; what
  the ramp should record instead is command-surface = B28-adjacent, yours) ·
  **§5.72** (the `$LOGON` channel is greeted as the LOG feed and carries other
  clients' command answers — semantics yours; today's additive routing preserves
  every shipped delivery).
- **Session-9 awareness, no action needed:** **§5.71** (`Core::panView` — the turn
  ramp's command twin — still old-only; one-line fix shape recorded in-row, rides
  your §5.66/§11.92(d) `look_at` family + its duration branch is perceptual-parity
  class) · **§5.73** (a TCP answer of ≥1023 bytes overflows the server's shared
  send buffer — pre-existing on both binaries, derived from source, not
  reproduced; NOT fixed because a truncation policy is protocol-visible and the
  buffer is shared with the receive path; a buffer-sizing-only fix may be
  decision-free — next-round candidate to verify at source) · **§5.62 owed item
  DISCHARGED** (F26, §11.134): the shift is on NEITHER binary in the current
  epoch ⇒ transient of the epoch-B session, delivered code EXONERATED, row stays
  OPEN unattributed; its own `active.lock` explanation REFUTED en route — the two
  shifted runs' instrument state is simply unknown; in-epoch-only stands, now
  with measured per-body A/A floors (Sun 0 / Jupiter 0.10 % / Mars 0.57 %).
- **B34 CLOSED (session 9, F25/§11.133) — the mandate's escape hatch was never
  needed:** the interactive view AND zoom ramps now act on the path that draws,
  with per-step parity measured through the live key channel (worst per-step
  divergence 1.965e-08 rad over a 360-step hold; zoom agreement 3.4e-06°); the
  24-term derivation table found an exact camera equivalent for every term — **no
  Vixy feel item is owed**. S6's operator-action seam audit (§11.108) is now fully
  landed: B33 + B34 both closed; the class residues live in B35 (mount write-half)
  and §5.71.
- **`get status position` is now a working read-only channel** (F27, §11.135) for
  observer place + heading — the reply follows the path that draws (B33's bar,
  measured against `dual_dump.control` per field); heading pins cost one launch
  instead of two.
