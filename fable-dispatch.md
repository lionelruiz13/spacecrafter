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
**Round outcome (session 10 close, 2026-08-09):** F28 → §11.136, F29 → §11.137,
F30 → §11.138 — all three delivered AND supervisor-verified same session.
**§5.73 CLOSED** (F28): the fix was decision-free for a reason the row did not
carry — the shared buffer never truncated anything, so its size expressed no
policy; `send()` now carries what it sends (I6, all nine call sites), the
overflow discriminated ONE BYTE wide on ASan (1022 fits / 1023 fires / 0
post-fix), every answer byte-identical on four binaries. **§5.46 CLOSED**
(F29): the class was TWO sites, not one (the invisible-reference branch found
by enumerating all five writers of `mat` — the member doc now NAMES its
writers); the 550-px stale-trail miss was PREDICTED from the freeze angle
before the run (549.7 predicted / 550.4 measured); P1 invariant
(`eclRoot == mat.translation`) violated by exactly the up-chain in five scenes
pre-fix, by nobody post-fix; controls bit-identical. **SIX DECISIONS
UN-STARVED** (F30, no code): §5.60 is the video player's own buffer (not pool
sizing — that redirects D13's question), §5.64's disagreement is total,
§5.65/§5.69/§5.70 are mapped negatives with qualifiers, §5.72 is a YES with
the two shipped clients on opposite sides. NEW rows: **§5.74** (search finds
no stars/constellations — owed datum named, dispatch candidate) · **§5.75**
(trail sample loss on date jumps — policy) · **§5.76** (paused decrement runs
time backward — semantics). Supervisor maintenance: §11 stubs 128–138
relocated home from §13.C with a boundary marker (§11.138(h)); §0.5 gains the
XAUTHORITY + `timeout -s KILL` hazards. Executor quality: F28 turned the
row's own framing against itself with its first measurement (the unbounded
producer was 142 B; the bounded-looking one hit the clamp) and found the
ledger's two wrong spellings via a growth control failing; F29's member-doc
writer enumeration is the structural fix for the ten-day blindness, and the
reversible-pair table (Earth 6378 km away while standing on the Moon) is the
defect in one number; F30's every negative carries a positive control on its
own channel, and both regime controls (llvmpipe + native) were run for §5.60.
Remaining dispatchable, next round: **§5.74's owed discrimination** (S — one
launch, catalogue count as positive control), **§5.34** (M — the deferred
`Object::operator=` leak, needs its own raw-holder enumeration; old-path
lifetime consequences), **B4(iv) re-check at the row** (old-path scripted
transitions — verify nothing gates it before minting), **B7-hunt-5 only after
A40**, **B12 content** after b12-design §7 + A41/A42, **§5.64/§5.65/§5.69/
§5.70/§5.72/§5.75/§5.76 fixes** only after Vixy decides the semantics.
DECISIONS_PENDING open set at close: **D15, D21 (late Aug), D37** + A40–A43
awaiting; B31 closure rides D21/D28/C4 (unchanged).

**Update [Fable 2026-08-09, supervising session 11]:** round of 3: **F31 → F32 →
F33**. Warm-up: both trees clean, code `bd3f7117` / harness `52ad1fe`, binary
confirmed current by no-op rebuild, `free -g` = 52 GiB ⇒ -j12; NO Vixy commit
since session 10's close ⇒ the whole blocked/decision-gated set stands unchanged
(D15/D21 late-Aug, D37, A40–A43, C4 open; §5.64/§5.65/§5.69/§5.70/§5.72/§5.75/
§5.76 fixes stay Vixy's). Picks per the session-10 queue, rows re-read at the
ledger: F31 = **§5.74's owed discrimination** (S; row names the datum and the
positive control), F32 = **§5.34** (M; session 10's named deferral, taken now —
the enumeration-first condition is in the row), F33 = **B4(iv)**, minted after
the owed at-the-row re-check: gate-check AT SOURCE found the travel half
(`moveTo`×3, `transitionToPoint`) ungated pure position-frame work;
`transitionToBody`'s heading tail is D28 territory handled by B4's own
kind-(2) precedent (inherit-at-clause, mirror old's measured behavior, invent
nothing); `alignCameraToBody` is an explicit up-vector command (not a switch
default, so not D28's question) portable iff an exact camera equivalent
derives per F25's table method — so the task is dispatchable with those three
boundaries IN the section, not gated. Order F31→F32→F33; order-independence
CHECKED: F31 is read-mostly (conditional fix confined to the search
aggregation), F32 = `src/tools/object.{cpp,hpp}`, F33 = camera/command-surface
files — disjoint; F32's lifetime fix and F33's camera work share no state, and
sequential dispatch removes the residual risk. §5.75/§5.76 NOT taken: policy/
semantics rows, Vixy's by their own text.

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
   - **Session-environment hazards (2026-08-09, F28/F30, §11.136/§11.138(i)):**
     (a) the inherited `XAUTHORITY` belongs to another uid — every display refuses;
     `export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*)` with
     `DISPLAY=:2` (forced, not defaulted — the inherited `DISPLAY=:0` makes
     `${DISPLAY:-:2}` keep the wrong one), verify `xdpyinfo` BEFORE the first
     launch (full note `harness/README.md`). (b) `timeout -s KILL` bounds NOTHING
     in this session type (measured: rc=124 only after the child's full run;
     mechanism unattributed, signal-mask hypothesis refuted) — use plain `timeout`
     (measured working) or an explicit poll-and-kill watchdog.
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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F30 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129 ·
F22 §11.130 · F23 §11.131 · F24 §11.132 · F25 §11.133 · F26 §11.134 · F27 §11.135 ·
F28 §11.136 · F29 §11.137 · F30 §11.138. Live sections: **F31–F33** (minted
2026-08-09, session 11), below.*

### F31 — §5.74's owed discrimination: why `search` returns no star and no constellation  [S; one launch]

- **Row (the owed item, verbatim):** §5.74 — *"Owed before it is judged: which of
  the two it is, on one launch, with the catalogue's own count as the positive
  control"* — the two candidates being: the star/constellation name catalogues
  are **not loaded** in a default launch, or the prefix match **never fires**.
  Found by F28 (§11.136(h)): 26-letter sweep at `maxobject 320`, zero `(S)` and
  zero `(C)` over 233 sampled entries; `Core::listMatchingObjectsI18n` aggregates
  four catalogues `[observed: coreModule/core.cpp:2367-2399]`.
- **Why now:** S-sized, named next-round candidate by session 10; the row cannot
  be JUDGED (code fix vs config vs product question) until the cause is
  discriminated.
- **Task:** one fresh launch; discriminate at each candidate cause's own surface:
  (a) **loaded?** — the star/constellation NAME catalogues' own counts read from
  the live process or its logs, with the answering catalogues (planets, nebulae)
  as the positive control on the same channel; (b) **fires?** — drive `search`
  with a prefix taken FROM the loaded catalogue's own content (never recall).
  Record the datum as a dated EXTENSION on §5.74. **CONDITIONAL FIX (F28
  precedent — measure first):** fix ONLY if the cause is a code defect whose fix
  is decision-free (no user-visible-semantics choice, no config/default change);
  a catalogue-content or default-config cause is Vixy's — record, don't fix.
- **Stop boundaries (NOT yours):** no config/data-file changes (D9 frozen field;
  md5 in==out); no EntityCore edits; if the fix requires choosing what `search`
  SHOULD match (i18n vs english naming, scope), that is semantics — record.
- **Discriminating checks:** the catalogue's own count is the control (zero from
  `search` against a NONZERO loaded catalogue discriminates match-vs-load; zero
  against zero discriminates load); if a fix lands: both ways on the same drive
  (pre-fix zero / post-fix hits, prediction stated first); fresh-launch
  precondition + `/proc`-comm concurrent-instance probe; md5 pristine.
- **DELIVERED 2026-08-09** (Opus 5 executor, §11.139; harness only — no product
  code changed, code tree clean at `bd3f7117` at open AND close). **The
  discrimination is paid: NOT LOADED, and the load never ran** — one launch under
  gdb, planets **90** / nebulae **407** on the same channel against constellations
  **0** / star index **0**; `loadLinesAndArt` and `loadCommonNames` entered **0
  times** while `setSkyCultureDir`'s reject branch fired once (all 2922 files under
  `~/.spacecrafter/sky_cultures` are 0 bytes). **The match is sound**: loading a
  culture from outside the frozen field on the same launch took the catalogues to
  **3**/**3183** and the same 36 commands from **S 0 · C 0** to **S 1085 · C 3**,
  the live-index prefix going 0 → 104 `(S)`, controls unmoved on all 27 unclamped
  prefixes. **No fix taken** — the cause is catalogue content, which the task's
  boundary reserves to Vixy; §5.74 stays OPEN with the question stated for her
  (does the delivered data carry these files?). NEW **§5.77** (the rejection never
  reaches the app's own log, which says `Check sky_cultures subdirectory ok`) and
  **§5.78** (`loadSciNames` has no caller — sci index 0 before AND after a working
  load). 20/20 checks PASS, app exit 0, frozen md5 in == out.
  **ACCEPTED 2026-08-09 (supervisor):** §11.139 read in full; §5.74 extension +
  §5.77/§5.78 verified at the ledger; commits `ce7a97b`/`5473386` authored
  correctly, both trees clean; the 2922×0-byte field claim re-measured
  independently (2922 files, 0 non-empty). All six deviations ENDORSED: the
  36-prefix widening (digit-keyed star index makes letters-only incomparable;
  F28's 26 still reproduced), the fixture leg (closes the disjunction on both
  sides instead of by elimination — shipped command, frozen field untouched by
  construction), the labelled synthetic constellation tokens (§11.51(d)
  respected: real data verbatim+md5, probe tokens claim nothing), the shared
  `f27_reply.Session` extension (re-measured 0 FAIL on the path F31 did not
  use), the two new §5 rows, the recorded abort (no ledger claim from it).

### F32 — §5.34: `Object::operator=` leaks the previous rep — enumeration-first fix  [M]

- **Row (verbatim at §5.34):** both overloads (`tools/object.cpp:105-126`)
  overwrite `rep` after retaining the new one — every reassignment leaks one
  reference. Old path: each STAR selection change leaks its refcounted
  `StarWrapperBase` (self-deletes at zero, never reaches zero). New path: each
  composed-body selection leaks one `ModularObject` **plus a permanent
  `ModularBodyPtr::ref` entry scanned on every body destruction**. NOT fixed
  with the selection wave: *"releasing correctly changes object lifetime on the
  old path … so it needs its own enumeration of who holds raw `ObjectBase*` and
  its own verification"* (§11.106(h)).
- **Why now:** session 10's named deferral ("one M-sized fix with baseline
  risk"), taken this round; the permanent `ref` growth degrades every body
  destruction and the leak class sits on BOTH paths' selection surface.
- **Task:** (1) **ENUMERATION FIRST** — every holder of a raw `ObjectBase*` (and
  every raw wrapper pointer that escapes an `Object`), classified: does ANY rely
  on the wrapper outliving its last retaining `Object`? Deliver the enumeration
  in the §11 entry (the member-doc-writer-enumeration precedent, F29). (2) Fix
  both overloads — retain-new / release-old / assign, self-assignment safe; if
  the enumeration shows the same hole in the copy/move ctors, fix at the class,
  scope stays `Object`'s own special members. (3) Verify per below.
- **Stop boundary (converts to decision, not fix):** if the enumeration finds a
  holder that DEPENDS on the leak — uses the wrapper after its last retaining
  `Object` is gone — STOP the fix, record holder + dependency + consequence,
  report. That is a lifetime-design decision.
- **Discriminating checks (both ways, one protocol):** pre-fix the leak is
  MEASURED (LSan/ASan or a refcount trace: N selection changes ⇒ N unreleased
  wrappers; `ModularBodyPtr::ref` size monotone under composed churn); post-fix
  ZERO on the same drive; use-after-free hunt post-fix under ASan with selection
  CHURN on both paths (old star churn including deselect/reselect; new
  composed-body churn; reversible pair select⇄deselect driven twice); render
  parity untouched (old path is the baseline: relevant b-batteries green); md5
  pristine.
- **WIP:** —

### F33 — B4(iv): the C3 scripted camera transitions act on the path that draws  [M-L; mandatory checkpoints]

- **Row:** B4 REMAINS (iv) — *"the old path's scripted transitions (moveTo /
  transitionTo* / alignCameraToBody, the C3 time-driven half of row 19) stay
  old-path-only — unasked by R3, unmeasured here."* Command surface: `camera
  action move_to target point|body / transition_to target point|body /
  align_with body` `[observed: app_command_interface.cpp:4208-4320]` — shipped
  script commands whose effect today reaches only the old `AnchorManager`.
- **Pre-mint gate-check (supervisor, 2026-08-09, at source):** `moveTo`×3 =
  timed travel of a non-body anchor's heliocentric position
  (`anchor_manager.cpp:405-486`; refuses on-body, refuses while moving) and
  `transitionToPoint` (`:528`) = instantaneous switch capturing the observer's
  current frames — pure position-frame work, **ungated**. `transitionToBody`
  (`:549`) lands lon/lat by bisection then ends in a HEADING tail
  (`setHeading(-axisAngle)`, `changeHeading(0, 5000)`) — roll-at-reference-
  switch = **D28 territory; B4's kind-(2) precedent applies**: record the D28
  dependency at the clause, mirror today's MEASURED old behavior, invent no
  roll semantics. `alignCameraToBody` (`:793`) = `navigator->alignUpVectorTo`
  — an explicit up-vector command, NOT a switch default (so not D28's
  question); port iff an exact camera equivalent DERIVES (F25's term-table
  method); else record the derivation gap.
- **Task:** make the five command forms drive the new camera path (dual — old
  `AnchorManager` untouched, baseline by construction). The new path already
  carries `Camera::moveTo` (`Camera.cpp:601`) and the F7 anchor registry.
  Parity target = old's measured behavior through the LIVE command channel
  (B34's bar): travel = per-step position vs the predicted path; switches =
  continuity invariants (position/view continuity within old's own measured
  bounds); refusal cases answer the same (on-body, already-moving, negative
  time, unknown name); heading tail mirrored ONLY if the new path expresses
  heading today — otherwise deliver the rest and record the member.
- **Stop boundaries (NOT yours):** old path untouched; no new key spellings
  (B28); no roll semantics invented (D28 inherited-at-clause); EntityCore
  authority line untouched; §5.71/`panView` and the `look_at` family stay
  suspended (§11.92(d)) — do not fold them in.
- **Discriminating checks:** per-member A/B old-vs-new through the live command
  channel with the prediction COMMITTED before the run (F29 precedent);
  reversible pairs driven twice (switch A→B→A→B, each entry from the previous
  exit's state); `b4_anchors` 76/76 stays green; `b32`/`b24_equivalence`/`b39`
  green (reference-switch adjacency — §5.32/F29 class); fresh-launch
  precondition + concurrent-instance probe; md5 pristine.
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

- **Session-10 decision data (2026-08-09, F30/§11.138 + F28/§11.136 — every
  waiting decision below now has the datum its row said it owed; nothing new is
  ASKED, the existing questions just got their facts):**
  - **§5.60 (D13 device-limit):** the 2.68 GB allocation is ONE BUFFER — the
    video player's staging buffer, sized `(32 MiB+80)×80` from HOST RAM tiers,
    allocated unconditionally at startup whether or not a video ever plays.
    NOT pool sizing: `maxMemoryAllocationSize` is queried nowhere in `src/`,
    and `BufferMgr`'s failure branch leaves a silently unusable manager. So
    your policy call is about the PLAYER's sizing (and/or creating a fallback
    path that today does not exist at either end). Also sharpened: on llvmpipe
    the allocation SUCCEEDED — it is not what caused the recorded SEGV.
  - **§5.64 + NEW §5.76 (pause semantics):** the readout/clock disagreement is
    TOTAL — 6 of 6 gated consumers already behave as paused; exactly ONE line
    (`TimeMgr::update`) is on the wrong side. And the same family measured
    worse: `timerate action decrement` from a held pause sets rate −1.0 (time
    runs BACKWARD at real time; shipped key `J`), and each ladder command
    RECORDS the wrong rate. Making the pause hold the clock changes no gated
    consumer's behaviour — the decision is cleaner than the row suggested.
  - **§5.65 (lock-after-move seam):** 0 in-block pairs in 434 shipped files;
    the only two `lock on` scripts wait a full second first, as if the author
    knew. The shipped corpus does not constrain your choice. (Scope: scripts —
    a TCP client can still issue the pair in one frame.)
  - **§5.69 (keep_time):** no shipped show sets it; but one internal script +
    the documented example ride the command's DEFAULT, which is the worst
    value there is (documented 10 s → 160 frames = 1.11 s at shipped fps).
    Any change to the default's meaning reaches exactly those.
  - **§5.70 (ramp recording):** no recording exists on THIS field (weaker than
    "none exists" — D9 freezes fields individually; an operator's own
    recordings are what the scan cannot see). Correction that constrains the
    respell: `delta_alt` is ALREADY a registered word meaning an observer
    altitude delta in metres on `moveto` — the replacement spelling must not
    collide with it.
  - **§5.72 ($LOGON channel):** YES — and the two shipped clients fall on
    OPPOSITE sides: `recever_client.c` (subscribes, issues nothing) goes
    SILENT if the broadcast copy of addressed answers is removed;
    `send_recev_client.c` keeps working (it now gets the addressed copy).
  - **Truncation policy (from F28/§11.136, riding §5.73's closure):** the
    1024-byte clamp is untouched and now has a concrete case — `get status
    planets_position` is 854 of 1024 B on shipped data, ~5 `body action
    load`s from silently truncating mid-token with no marker. The overflow
    is fixed; whether/how a too-long answer should be MARKED is yours.
- **Session-10 veto points (all implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) F28 removed the shared-buffer send path at ALL
  NINE `send` call sites, not just the overflowing two — every constant answer
  was wire-measured byte-identical pre/post, and the `SMALL_BUFFER` comment's
  never-enforced claim was retired at the site; (2) F29's fix covers TWO sites
  (§5.46 named one — the second is the invisible-reference branch), both pure
  contract-restorations, reversible per-site; (3) the eleven §11 stubs
  (128–138) that had accreted inside §13.C were relocated back to §11 as a
  pure line move, with a boundary marker so the class cannot recur
  (§11.138(h)).
- **New awareness rows, no action needed now: §5.74** (`search` returns no star
  and no constellation on the shipped corpus — cause not yet discriminated,
  owed datum named in-row, next-round dispatch candidate) · **§5.75**
  (`TrailModule::accumulate` drops samples on date jumps — every date-stepping
  show carries a trail that lags its body; the fix is one expression but
  changes shipped-trail sample counts = policy).
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
