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

**Update [Fable 2026-08-26, supervising session 13]:** trigger = Vixy: *"I have
replied to D15, D21 and D37 … run a dispatch session with opus xhigh."* Between
sessions: §11.147 (scedit-fork folder merge, ids §5.91–98/§11.138–146 renumbered)
+ §11.148 (the 67-artifact divergence root-traced to per-evaluation satellite
drift) landed Vixy-directed; next free §11 number is **149**. Warm-up: both trees
clean; code `f0c8ef83` / harness `2b24a1b`; code HEAD is **Lionel RUIZ's**
(tester) doc-only commit — `doc/superscript.sts` +267/−67 + `debug.sh`, NO `src/`
change ⇒ the 2026-08-23 binary is current (F38 confirms by no-op rebuild).
Answers verified at the file: D15 (a)–(d) + inform-tester line (`2b24a1b`,
2026-08-26), D21 + D37 `[Vixy reply]` lines (`d521807a`, 2026-08-22). **Round of
3, the answers' own unblocked set** (the decision-free S-candidates §5.86-data /
§5.88-datum / §5.89-sweep stay queued next round): **F37 → F38 → F39** below.
Order: record first (F37 baselines nothing, but its premise probe reads the
unmodified tree), then the two code tasks — disjoint files (F38 `core.cpp`/
`Camera` vs F39 `SurfacePointOrbitLoader`/`ModularBody`/factory), sequential per
protocol. Census correction folded into F38's spec: §11.112(c)'s four bypass
sites drifted to `core.cpp:1083/1372/1410/2313` (verified 2026-08-26; the old
971/2066/1251/1289 no longer exist — the zoom-out family now routes through the
mirror; these four still poke `setFlagLockEquPos` directly).
**Round outcome (session 13 close, 2026-08-26):** F37 → §11.149 · F38 → §11.150
· F39 → §11.152 · F40 → §11.153, plus the supervisor entry §11.151 (Vixy's two
mid-round ratifications) — all four delivered AND supervisor-verified same
session; round extended 3→4 per §11.151(a)'s recorded plan. Code `f0c8ef83 →
b444d381 → fd98c8e9 → 8c2cbf61 → 7ef11aca → 18b6f13f` (+ submodule `EntityCore
224eba7 → 7ce5835`, the ASmooth phase-timer fix, precedent = Vixy's own
`f28c555`). **DECISIONS_PENDING's open set is EMPTY — first time since the file
was compiled**; D37 rides the final tester pass WITH its premise fact (the
b39_star "Moon stays lit" reading was an instrument artifact — hiding the star
hides the system, exactly as Vixy's premise said). CLOSED: §5.27, §5.80,
§5.102 (rooted: ORIGINAL uninitialized ASmooth phase timer — the malloc-garbage
variance explains every divergent historical reading, incl. the Moon silently
stuck at ×1 on some launches), §5.103, §5.31-era B18 sky-lock half, B31's T3.
NEW: §5.99–§5.106 (8 rows), A44 (ring shadow caster: D21 vs the 2026-07-18
extent contract), 6 final-pass members (D37 question + D15(a)–(d) INFORM ×4 +
D15(b) heading CONFIRM). Instrument-artifact refutations this round: 4
(b39_star `lit()`, §11.101(f)'s reload prediction, §11.144(i)'s 3.3 Mpx
attribution, §11.58's "0.0000°" scoped to its degenerate configuration) — the
round's meta-lesson: criterion-that-cannot-discriminate hunting pays.
**Next-round queue, in order:** (1) the stub↔entry-file cross-check sweep
(§11.149(h) — the authority-inversion class); (2) b24_select 4-red adjudication
+ the 28-file free-mode-longitude audit (S, §11.153(o)); (3) §5.86's owed data ·
§5.88's owed draw-cost datum · §5.89's owed assert sweep (S each); (4) §5.100's
one-line fix IF Vixy's answer authorizes it. Supervisor-context note: 4 hard
tasks + verification ≈ well under budget this session (large-window regime —
datum for the sizing rule, not a supersession of it).
**Post-close extension (same day):** Vixy's rulings kept landing in-conversation
⇒ **§11.154** (residual absorbed with structure; §5.104 answered by
format-scoped ownership) and **F41 → §11.155** (`display_scale` ADOPTED against
the measured grammar; both reload branches fixed and discriminated both ways;
twin carries the config value across the ownership transfer; §5.104 CLOSED).
Round total: **five tasks, seven §11 entries (149–155)**. NEW from F41: §5.107
(extent-cache load-time latch — first measured member of §5.104's wider-seam
probe) · §5.108 (`flag_sun_scaled` never acted, either path — `ui.cpp:239`
hardcode + uninitialized `SunScale`) · D30's DELTA branch located unimplemented
for this key (B31's). Instrument corrections: F26's `/proc` comm probe counts
2/instance (liveness test, not instance count); §11.152(p)(5)'s reload warning
superseded from `d6aec251`. Supervisor-error tally this session, all caught by
the defense layers: a phantom outcome block (self-caught pre-commit), a wrong
census parenthetical (F38), a wrong discriminator target (F40), a mis-homed
stub (F41) — three of four caught by executor verification, the layer working
as designed.

**Update [Fable 2026-08-29, supervising session 14]:** round of 3: **F42 → F43 →
F44** — the session-13 queue in order; queue item 4 (§5.100's fix) stays blocked,
its authorization question unanswered at this open. Warm-up: both trees clean at
open, code `d6aec251` / harness `631d0c8`, binary confirmed current by no-op
rebuild, `free -g` = 30 GiB ⇒ -j12; NO Vixy commit since session 13's close ⇒
the decision-gated set stands unchanged (D-set still EMPTY; A40–A44,
§11.146(j), the two §11.144 riders, §5.106's close-vs-annotate, and the
§5.87/§5.88/§5.89/§5.90 semantics halves all wait). Next free §11 number
verified **156**. Picks, rows re-read at the ledger: F42 = **§11.149(h)'s owed
sweep** (stub↔entry-file cross-check, the authority-inversion class; harness
repo only), F43 = **§11.153(o)'s owed audit** (b24_select 4-red adjudication +
the 28 unaudited free-mode-longitude files; README carries the list and the two
corrected precedents), F44 = **§5.86's owed data** (consumer dependence +
old-path RA/DE parity target; record-only). Order: F42 first (every later task
reads the ledger it repairs), then F43 (instrument), then F44 (measurement) —
surfaces disjoint (INTENT tree / harness .py / record + one probe), and NO
mandate touches product code: the code tree should be clean at every point of
this round, a first. Extension to F45 (§5.88's draw-cost datum) + F46 (§5.89's
assert sweep) only if session health permits after F44's verification, minted
then per §0b.2. Round-open events: **archival pass 7** (update-s12 + F37–F41
moved byte-exact, manifest `2026-08-29-pass7`, commit `4f9685f`); **host
incident, resolved**: Vixy's 2026-08-26 morning harness commits from the foxy
account left two foxy-owned fan-out dirs in the harness `.git/objects` (`29/`,
`e4/`) — this round's first commit FAILED on exactly that; the local repair was
classifier-denied and routed to Vixy per the gate protocol, resolved by
`sudo chown claude -R /home/claude/spacecrafter` [vixy 2026-08-29: *"the only
clean correction is on my side so reporting it to me is the only valid move
here"*]. The code repo carried 4 foxy-owned object FILES in claude-owned dirs —
no blockage there, checked. Recurrence condition recorded: any foreign-account
commit that creates a NEW fan-out dir re-arms the class; the report-to-Vixy
route is the standing answer.

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
   - **Session-environment hazards (2026-08-09, F28/F30, §11.138/§11.140(i)):**
     (a) the inherited `XAUTHORITY` belongs to another uid — every display refuses;
     ~~`export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*)`~~
     **[SUPERSEDED 2026-08-29, F43 §11.157(f): the host rebooted 2026-08-27 and
     claude has NO login session — the stack is a REBUILT headless GNOME/Xwayland
     under `XDG_RUNTIME_DIR=/tmp/rt-claude`, verified against the recorded
     values (Meta-0 2448x1332@59.96, GPU-real, Swapchain/Rect per §11.106);
     `export XAUTHORITY=/tmp/rt-claude/.mutter-Xwaylandauth.*` until a real
     login session exists; if the auth file is gone (another reboot), STOP and
     report rather than improvise a new stack silently]** with
     `DISPLAY=:2` (forced, not defaulted — the inherited `DISPLAY=:0` makes
     `${DISPLAY:-:2}` keep the wrong one), verify `xdpyinfo` BEFORE the first
     launch (full note `harness/README.md`). The stack is rebuilt-not-inherited:
     a candidate variable for any A/A floor vs pre-2026-08-27 baselines.
     (b) `timeout -s KILL` bounds NOTHING
     in this session type (measured: rc=124 only after the child's full run;
     mechanism unattributed, signal-mask hypothesis refuted) — use plain `timeout`
     (measured working) or an explicit poll-and-kill watchdog.
   - **§5.109 hazard (2026-08-29, F43):** `moveto … alt` counts from the
     DISPLAY-scaled datum, snapped once — never `flag moon_scaled off; sleep N;
     moveto`; wait the scale settle BY MEASUREMENT (`scaling`==`scalingTarget`
     in the dump) and assert the observer's radius (band checks pass on both
     values).
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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F36 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129 ·
F22 §11.130 · F23 §11.131 · F24 §11.132 · F25 §11.133 · F26 §11.134 · F27 §11.135 ·
F28 §11.138 · F29 §11.139 · F30 §11.140 · F31 §11.141 · F32 §11.142 ·
F33 §11.143 · F34 §11.144 · F35 §11.145 · F36 §11.146 · F37 §11.149 ·
F38 §11.150 · F39 §11.152 · F40 §11.153 · F41 §11.155 (F37–F41 archived,
pass 7). Live sections: **F42–F44** (session 14, the session-13 queue — below).
Queued after them: §5.88's owed draw-cost datum, S; §5.89's owed assert sweep,
S (both = this round's extension candidates, minted on session health); §5.100's
one-line fix once its §3 authorization question is answered — every FIX these
enable is Vixy's.*

### F42 — The stub↔entry-file cross-check sweep: §11.149(h)'s owed sweep of the authority-inversion class [S–M]

**Why now / why first:** §11.149(h) [observed 2026-08-26]: while flipping §5.27,
F37 found the expanded entry `INTENT/5.27.md` — the AUTHORITY by the ledger
header's own rule — still carrying the ORIGINAL refuted root while the derived
in-file stub carried the correction: for a month the derived view was correct
and the authority was refuted, in exactly the direction that makes the header
rule dangerous. That one pair was repaired; *"the class is not repaired …
Recorded as an owed sweep, not run here."* This task runs the sweep. First in
the round because every later task reads the ledger this repairs. Harness repo
only — NO product code, NO data.

**Sources (re-read, never recall):** `INTENT.md` header (authority rule +
maintenance invariant + tag grammar); `INTENT/11.149.md` (h) (the class
definition and the §5.27 repair's shape — the precedent for any repair here);
`INTENT/5.27.md` post-repair (what a repaired pair looks like).

**Scope:**
1. Enumerate every stub↔entry pair (`INTENT/` holds 172 entry files; each has an
   in-file stub in §5 or §11). The pair count is stated in the entry — a sweep's
   denominator is part of its result.
2. Per pair, cross-check STATUS-BEARING content, not prose detail (the stub is a
   derived summary; body differences are by design): OPEN/CLOSED/SUSPENDED
   state, supersession/correction blocks (`[CORRECTION`, `SUPERSEDED`,
   `REFUTED`, strike-through, dated annotations postdating the pair's split or
   creation), and citations that route authority ("fix = task Fn", "rides Dn").
   The check: does either side carry a status change the other lacks?
3. Classify each divergent pair: **(i) DANGEROUS** — the stub carries a
   correction the entry file lacks (authority refuted; the §5.27 shape) ⇒
   REPAIR in place: transcribe the correction into the entry file, strike the
   superseded text WITH its supersession pointer (maintenance invariant), never
   delete. **(ii) INVERSE** — the entry file carries status the stub misstates ⇒
   refresh the stub (it is derived; that is the cheap direction). **(iii)
   CONFLICT** — the two sides assert incompatible substance and neither is a
   marked correction ⇒ record with BOTH texts cited, repair nothing, flag for
   the supervisor.
4. The method must be stated precisely enough that every "consistent" verdict is
   a positive map, not absence-of-noticing (what was compared; what was
   deliberately not compared).
5. Class closure: nothing structural today prevents a correction landing on one
   home only. PROPOSE (do not enact) a one-line addition to `INTENT.md`'s
   maintenance header binding corrections to entry-file-first order; the
   supervisor ratifies or routes to Vixy at acceptance.

**Boundaries:** no product code, no data, no harness scripts, no §5/§13 row
STATE changes — a repair transcribes an existing recorded status to its other
home, it never advances one. Where a divergence suggests a row should close or
reopen, that is a class-(iii) finding, not an action.

**DoD:** pair count stated; every pair checked with the method recorded;
divergent pairs enumerated with class + per-pair disposition; class-(i) pairs
repaired with supersession markers; class-(iii) pairs recorded with both texts;
the header-rule proposal stated with its argument; §11 entry at the next free
number + stub — the entry checked against its own stub before delivery (the
sweep includes its own delivery); code tree untouched
(`git -C /home/claude/spacecrafter status` clean at close); WIP per §0.6.
**WIP:** *DELIVERED 2026-08-29 → **§11.156** (entry file + stub). **Denominator: 171 entry
files = 146 LIVE PAIRS + 25 archived-in-place** (retired index line verbatim inside the file
⇒ one home left, nothing that can diverge — verified on all 25); 80 further §5/§11 stubs are
inline, not pairs. Method stated as a positive map (compared: S1 state + strike state · S2
supersession/correction markers · S3 authority routing · S4 dates; NOT compared, by design:
N1 measured detail · N2 discovery provenance · N3 supporting citations · N4 wording/case ·
N5 statements about other rows), four tests in `claude/intent_pair_check.py` + a whole-corpus
line-level pass; every flag adjudicated by reading both texts, coverage limits stated.
**DISCRIMINATING CHECK BOTH WAYS on §5.27**: pre-repair tree (`458a71f` = `d43abec^`) FLAGGED
class-(i) on the right content (D + D2 both fire); current tree PASSES. **13 divergent pairs
of 146: 4 class-(i) + 9 class-(ii) + 0 class-(iii)** — class (iii) zero as a result, not an
absence: every divergence had a MARKED correction on exactly one side. Class-(i) repaired in
the ENTRY (§5.26 — the §5.27 shape, with §11.108(j) recording an annotation that went to the
stub alone; §11.96; §11.112; §11.155); class-(ii) stubs refreshed (§5.2, §11.86, §11.89,
§11.97, §11.112, §11.124, §11.136, §11.137, §11.144) — two of them were relaying a REFUTED
claim into the index (§11.86's texture-registered offsets, §11.144's 3.3 Mpx). Header-rule
one-liner PROPOSED not enacted (entry-first ordering; the argument is that it makes the
dangerous direction structurally unreachable). OUT OF SCOPE, RECORDED: 5 entry-to-entry
supersessions with no back-marker at the superseded node (§11.113(p)) — §11.122(i) is stale
in BOTH homes, which is why a pair sweep cannot reach it. Code tree `d6aec251` clean
throughout. Commits `89b83cc` (CP1) · `30548d9` (CP2) · this one.*
*(ACCEPTED by supervisor [fable 2026-08-29]: §11.156 read in full; commits/authors/diff
scope checked — the trailer-empty and 171-vs-172 flags both resolved AGAINST my probes (a
non-trailer line before the block; my warm-up `ls` counted the `archive` subdir). §5.26
repair and §11.144/§11.86 stub refreshes verified at the ledger by content; code tree clean.
Judgment calls ENDORSED with their arguments: the (i-refuting)/(i-incomplete) split
(differentiated repairs, no redefinition); repair form keyed to the home (§11.99 +
§11.113(p) precedents); `intent_pair_check.py` at `claude/` top level (harness/ is the
app-verification surface, correctly out of bounds); the refreshed-stub signature stated as
the next run's baseline. The (f) proposal RATIFIED and enacted in the header structure
paragraph [veto point for Vixy, one line to reverse — the archival-rule precedent:
maintained-artifact conventions are delegated]. The (g) five back-markers = next-round
queue candidate (mechanical §11.113(p) transcriptions; boundary: markers only, no state
advance — §11.122's row state question rides it as a finding, not an action).)*

### F43 — b24_select's four reds adjudicated + the 28-file free-mode-longitude audit (§11.153(o), (j)(3)) [S–M]

**Mandate:** §11.153(o)(3)(4) + (j)(3), owed at F40's acceptance: the b24_select
4-red adjudication (pre-existing at `7ef11aca` — verified same failures, same
numbers, pre binary + pre-convention harness) and the 28 unaudited harness
files that combine `camera action free_mode state on` with `moveto … lon`. Two
halves, one task: both are the same convention change's harness fallout.

**Sources (re-read, never recall):** `INTENT/11.153.md` (j)(3) + (o);
`harness/README.md`'s F40 section (the 30-file arithmetic + the two corrected
precedents, each with its at-the-site arithmetic); `harness/b24_select.py` (the
four failing gates: ndc 0.6886/0.6918 against a 0.6852 limb, alpha off its
prediction by 2.342°, the R5 cluster click); §11.106 + §11.118(e) (what
b24_select's legs assert and why they were built); the §5.49 row — CONSULT,
never act on it (its own render measurement is owed elsewhere and its
conclusion is recorded as in-doubt, §11.153(k)).

**Scope:**
1. **(A) Adjudicate the four reds** on the current binary: per failure,
   attribute — INSTRUMENT (scene geometry / threshold / prediction arithmetic
   stale against the harness's own history) vs PRODUCT defect (mint the §5 row,
   record don't fix) vs UNRESOLVED (name the owed datum). Instrument repairs
   are in-mandate; any repair must reproduce committed-baseline geometry where
   geometry was the gate's point, arithmetic at the site (the (j)(2)(3)
   precedent).
2. **(B) Audit the 28 files**: per file, the discriminating read — does any
   gate couple the free-mode observer's longitude to authored content or to a
   committed baseline number? Classify: UNAFFECTED (no coupling — state why) /
   AFFECTED-corrected (geometry-preserving re-declaration with its arithmetic,
   invariance shown) / AFFECTED-stale-baseline (re-baseline with the argument;
   the old numbers stay citable in git). Resolve README's "the rest are
   unaudited" note to the audit's outcome.
3. Launches per §0.5 (temp-HOME farm + md5, `/proc/<pid>/comm` assert,
   `DISPLAY=:2` + XAUTHORITY export, plain `timeout`).

**Boundaries:** NO product code — this task lives entirely in `claude/harness/`.
Product defects found = §5 rows + report, never fixes. §5.49 must not be
"fixed" through a harness edit. The A–D battery only if an edited file is a
battery member; if run, same-binary phase control FIRST (§11.153(i)).

**DoD:** four reds each attributed with evidence (or the named owed datum);
b24_select green OR red-with-minted-rows, stated which; 28/28 files classified
with per-file reasons; corrected files reproduce their intended geometry
(numbers cited both ways); README updated; §11 entry at the next free number +
stub; md5 pristine on every launch; code tree clean at close; WIP per §0.6.
**WIP:** *DELIVERED 2026-08-29 → §11.157 (+ NEW §5.109, product, record-only).
Both halves done; b24_select GREEN reproducing §11.106's numbers; 30/30
classified; README's "unaudited" note resolved. NOT green and stated as such:
`b3_ladder.py` is 1-failure red after correction, with an unattributed
+1.4–1.6 % drift vs §11.104(d) and a named discriminating check. NEWLY VIXY'S:
§5.109's layer half. Trail below.*
*2026-08-29 CP1 — half (A) DONE. The display stack had to be
RE-ESTABLISHED (host rebooted 2026-08-27; claude had no login session, no `:2`,
`/tmp` wiped incl. §11.153(o)(1)'s two binaries) — headless GNOME restarted at
the recorded `2448x1332 @ 59.96`, app GPU-real on the RTX 5090, Swapchain
1024x1024 / Rect 2048x2048 as §11.106 recorded. All four b24_select reds
attributed INSTRUMENT with the product cause measured and minted as **§5.109**
(`moveto … alt` counts from the display-SCALED datum, snapped once, so the
harness's moveto landed ~3 s into a 5 s scale ramp: observer 11 750.80 km
instead of 9737.40). Repaired (measured settle + observer-radius gate +
midpoint read in the clicked state): **b24_select GREEN**, reproducing
§11.106's committed numbers (alpha 12.670, ss 0.0432/0.0106, swap 0.0425/0.0108,
selDist 5.905047e-05, cluster 0.0179, parity 90). Discrimination both ways:
`artifacts/f43/sel_pre|sel_control|sel_post`.
2026-08-29 CP2 — half (B) DONE, 30/30 classified (README's 30 = 28 + the two
corrected precedents; arithmetic reproduced). AFFECTED-corrected: `b3_ladder.py`
(same `nadir_lon` class as b24_select, but the SITE carries its illumination and
DEM value so the b24_screen precedent applies — `obs_lon_cam() = (270-L)%360`,
nadir bit-identical 140.300; 19 failures -> 1), `b3_cost.py` (routed through that
one authority), `b3_earth_ab.py` (270 -> 0). AFFECTED-superseded, documented not
re-baselined: `f34_convention.py`. The rest UNAFFECTED with the read named; the
one gate reading could not settle (f23/f24's lit guard) MEASURED green
(`f43_litguard.py`, 113 357 px>8 vs 20 000/1 000, control = predicted disc area).
b3_ladder residual NOT absorbed: +1.4-1.6 % on four metric caps vs §11.104 and
b250's site luma 20.97 < 30 — unattributed, discriminating check named.
NEXT: README + §11.157 + stub.*
*(ACCEPTED by supervisor [fable 2026-08-29]: §11.157 read in full; commits/authors/scope
checked (harness-only, code tree clean at `d6aec251` throughout); §5.109 mint + stub +
README resolution verified at the ledger. One record defect found and repaired at
acceptance: the entry cited CP2's PRE-AMEND sha `5a284a5` (the amend carried the 25→24
count fix); annotated in the entry file, on-branch chain `988218d → 575d7fc → f2210a4`.
Judgment calls ENDORSED with their arguments: the display-stack rebuild verified against
recorded values BEFORE measuring (§11.122(o) discipline — and §0.5's XAUTHORITY recipe is
now stale, supervisor updates it at close); b3_ladder's opposite repair (the site carries
the instrument's illumination + DEM window — b24_screen precedent, invariance bit-identical
×3); f34_convention documented-not-re-baselined (I2 — f40_inverse already scores both);
the litguard control replacement (the session's next criterion-that-cannot-discriminate
instance, replaced by a predicted disc area); b3_ladder left RED, not absorbed — its
named check (corrected harness vs a `922701c9` build) is QUEUED next round. §5.109's
layer half joins §3 for Vixy; its timing half is a defect either way, standing hazard
recorded for every grounded-scene harness.)*

### F44 — §5.86's owed data: who consumes the scrambled RA/DE, and what the old path answers (record-only) [S]

**Mandate:** the §5.86 row's own owed clause: *"whether any consumer depends on
today's answer, and what the old path's RA/DE says for the same body — a parity
target this row does not assume."* The FIX stays with §11.4's closer — the row
routes it; it is NOT this task's even where it looks decision-free (the
algebraic inverse already sits written out in `f34_probe_inverse.cpp`).

**Sources (re-read, never recall):** the §5.86 row; `INTENT/11.144.md` (j) (the
133.9° round-trip measurement + probe); `Camera.hpp:258` caveat +
`Camera.hpp:264-274` / `Camera.cpp:183-197`; `ModularObject.cpp:19/:47/:115/:152`
(the cited consumers — re-verify the lines at dispatch, they may have drifted);
§11.153(o)(7) (`-std=c++20` for any probe rebuild).

**Scope:**
1. **Consumer census (positive map):** from `observedToBodyLocalPos` +
   `observedPosToRaDe` UP to every observable channel (script/TCP `get`
   answers, on-screen UI text, dumps, selection logic, anything else) — each
   terminal consumer named REACHABLE or UNREACHABLE on the shipped surface,
   with its route; census method stated (grep basis + call-graph walk,
   comment-stripped — the F36 lesson: a grep alone measured 2.2× wrong).
2. **Old-path parity target:** measure old's RA/DE for ≥1 body on a stamped
   configuration, both paths in ONE launch (the dual dump or `get status` —
   whichever already carries both; if neither does, SAY so and name what an
   instrument would need rather than building beyond S-size). Record beside the
   new path's answer for the same body + scene.
3. Record on the §5.86 row (annotation; the row stays OPEN) + the §11 entry.

**Boundaries:** record-only for product code — no fix. Measurement per §0.5.

**DoD:** consumer set enumerated positively with reachability verdicts; the
old-path RA/DE datum recorded with its configuration stamp and stated on the
row as the parity target; §5.86 annotated, still OPEN; §11 entry at the next
free number + stub; no product-code change; md5 pristine; WIP per §0.6.
**WIP:** *(cleared at delivery 2026-08-29 — **DELIVERED, §11.158**; RECORD-ONLY,
code `d6aec251` untouched and clean at open and close; harness `441497a →
b8d51ec` (CP1, census instrument) `→ 04e5780` (CP2, parity measurement) + the
record commit. DoD item by item — **consumer set enumerated positively**:
comment-stripped census over 501 files (`harness/f44_census.py`, self-test
suppresses 5 comment hits / keeps 4 code hits) plus a read call-graph walk;
the row's four consumer sites are the COMPLETE set, and the bounding fact is
that a `ModularObject` is built at only four places (product
`ssystem_factory.cpp:914`/`:937`, instrument `:1202`/`:1239`) ⇒ **on the
shipped surface these readouts answer only for a body the old tree does not
carry**. **6 REACHABLE** (TCP `get status object` · the TUI nav line and every
quantity derived from it · the five sky-display overlays, fed unconditionally
every frame · the old path's view aiming · `set home_planet selected`, which
caches the scrambled place under the body's name · `isSameLogicalObject`);
**5 UNREACHABLE, each with the read that grounds it** (`Core::getDeRa` DEAD —
one code hit, its own definition · the six `#selected_*` cut off by
`solarsystem_selected.cpp:48`'s `OBJECT_BODY` filter · `#selected_star_*` by
`setSelectedObject`'s single caller in `case OBJECT_STAR` · the old pointer
twice over · `cleverFind`/`searchByNameI18n`, old-tree-only). **Old-path RA/DE
datum recorded with its stamp**: existing channel FOUND not built (the dual
dump's `.navstr`, `ssystem_factory.cpp:1179-1218`); 90 both-tree bodies, one
fresh launch, shipped place, `jd 2461233.5`, md5 `03fbee59`/`545a51ef` in ==
out, exit 0. Old vs new **median 60.3636°**, decomposed into four attributed
terms — §5.86 itself (shipped arithmetic reproduced offline to **0.00187°**),
§11.4's zero point (**−90.000283°**, spread 0.005378°, DE already matching to
0.003613°, **epoch-independent**: 0.000462° while the spin moved 130.0552°),
an **origin convention** (old is observer-centred, `navigator.hpp:140-147`;
0.996287° measured vs 0.993226° predicted on the Moon), and **Eris** (trees
1.198373° apart). **PARITY TARGET: ≤ 0.002014° for 89 of 90 bodies.**
Free-branch discriminator exact both ways (|ΔDE| **0.000000°**, ΔRA
**70.092466°** spread **0.000000°** = `−axisRot`). **§5.86 annotated, still
OPEN**; §11.158 entry file written FIRST then the stub; forward marker added
at §11.144(j)(1) (the §11.156 entry-to-entry class). **No product-code change;
md5 pristine.** NEW **§5.110** + **§5.111**, record-only. Two of the task's own
predictions REFUTED and kept with their numbers; a third guard (C1) refused a
non-discriminating epoch pair. **Newly Vixy's/§11.4's**: the calibration is now
two DECISIONS with numbers — which RA zero point, and which ORIGIN.)*
*(ROUND EXTENDED after this acceptance per the session-14 update note's recorded
plan: session health excellent, F45 + F46 minted below.)*
*(ACCEPTED by supervisor [fable 2026-08-29]: §11.158 read in full; commits/authors/scope
checked (record-only held: `src/` untouched, code clean at `d6aec251` throughout); §5.86
annotation + §5.110/§5.111 mints + stub + the §11.144(j)(1) forward marker verified at the
ledger. One repair at acceptance: the entry's cited artifacts (`claude/artifacts/f44*/`)
were UNTRACKED — `.gitignore:1` ignores `artifacts` and F43/F40 force-added their cited
discrimination records; F44's are force-added here (712K, gz/json) so the citations resolve
by commit, not by luck. Judgment calls ENDORSED with their arguments: the two refuted
predictions KEPT with numbers (the angular-separation-blind-to-rotation lesson + the
sidereal-alias epoch guard — the round's method exemplars, same class as F43's litguard);
the three-gates-not-looser-bound repair; §5.110 minted by reading with its live check named
owed; the census's comment-stripped, line-preserving instrument. The (b) reachability gate
— shipped blast radius = composed-body selection only, while the dump exercises the
readout for all 90 — is the entry's most consequential fact and correctly bounds (f)'s
target. §11.4's two numbered decisions join §3 for Vixy.)*

### F45 — §5.88's owed datum: what an empty spectral array costs at draw time (record-only) [S]

**Mandate:** the §5.88 row's own owed clause: *"what an empty spectral array
costs at draw time, which this task did not measure."* The repair fork (make
`initFromFile` report / make it throw so the existing `catch` fires / caller
checks — three contracts, one changing control flow) is Vixy's AFTER this
datum; nothing here fixes.

**Sources (re-read, never recall):** the §5.88 row (the mechanism:
`hip_star_mgr.cpp:489-493` try/catch that never fires;
`string_array.cpp:33-45` silent `size == 0`; shipped `stars.ini` names
`stars_hip_sp_0v0_4.cat`, root carries `0v0_0`); the §5.90 row (the split
roots: the LIST from `~/.spacecrafter/stars.ini`, the FILES from
`/usr/local/share/spacecrafter/stars/`, and `~/.spacecrafter/stars/` unread —
CONSULT: the control below rides this mechanism and whatever it measures about
it is recorded on §5.90, not chased); §11.146(g) (F36's measurement of the
0-report); `f44_census.py` (reusable comment-stripped census instrument).

**Scope:**
1. **Source read first:** what consumes the spectral array on the draw path
   (per-star per-frame lookup? colour table at load? index arithmetic on an
   empty vector?) — the cost SHAPE, stated with file:line, so the measurement
   knows what to look for (visuals, per-frame time, both).
2. **Measure empty-vs-populated:** shipped state (array empty) vs a CONTROL
   with the array populated. Control route to try first: temp-HOME farm whose
   `stars.ini` names the catalogue version that EXISTS in the loader's file
   root (`0v0_0`) — never edit the real `~/.spacecrafter` (md5 in==out).
   If the §5.90 split-root mechanism defeats every no-code control, SAY SO
   with the read that grounds it and name what a control needs — do not force
   one. Observables: star-channel pixels/colours (screenshot compare, star
   flags on), any per-frame cost against the **1 ms/frame** denominator (D11;
   counter ratios only, no absolute fps), console/log.
3. Record on the §5.88 row (+§5.90 if the control discriminates its
   mechanism); §11 entry at the next free number + stub, entry file first
   (§11.156(f)).

**Boundaries:** NO product code, NO data-root writes (`/usr/local/share` is
the field, D9), no real-HOME edits. The contract choice stays Vixy's.

**DoD:** cost shape stated from source; empty-vs-populated measured (or the
no-control verdict grounded); numbers on the D11 denominator where cost is
claimed; §5.88 annotated (stays OPEN, its fix fork untouched); md5 pristine;
code tree clean; §11 entry + stub; WIP per §0.6.
**WIP:** *(empty)*

### F46 — §5.89's owed sweep: every load-bearing `assert` in `src/` (record-only) [S]

**Mandate:** the §5.89 row's own owed clause: *"whether any other `assert` in
`src/` is load-bearing in the same way, which F36 did not sweep."* The shipped
build type is `RelWithDebInfo` (`-DNDEBUG` — asserts compile to nothing,
measured §11.146(h)); §5.89's instance is `core.cpp:487-492` (unknown
`viewing_mode` reports to stderr, `assert(0)` dead, execution falls through).
Repair choices (abort / named-default-and-log per D12 / refuse-to-start) are
Vixy's per the row; nothing here fixes.

**Sources (re-read, never recall):** the §5.89 row incl. its D15(a) INFORMED
annotation (EQUATORIAL is the only defensible named default on the drawn path,
but VIEW_HORIZON stays config vocabulary — D9/D13); §11.146(h);
`f44_census.py` (the comment-stripped, line-preserving census instrument —
reuse it; a raw grep measured 2.2× wrong, F36).

**Scope:**
1. **Census:** every `assert(`/`assert (` in `src/` (EntityCore included — it
   is in the built binary; read-only by protocol), comment-stripped, count
   stated with the method.
2. **Classify each:** (i) LOAD-BEARING — the author meant it fatal (no other
   handling; falling through changes behaviour) AND the failing condition is
   REACHABLE from user input, config, script/TCP command, or shipped data;
   (ii) INVARIANT-CHECK — a debug diagnostic whose removal under NDEBUG leaves
   correct handling in place; (iii) UNREACHABLE — the condition cannot occur
   from the shipped surface (ground the negative). For every class-(i): what
   executes after the dead assert, reachable from what, observable how.
   Reachability grounded the F44 way (route named, or the read that grounds
   the negative).
3. **Record:** on the §5.89 row (the sweep it owed); class-(i) members beyond
   §5.89's own become §5 rows ONLY if their mechanism is distinct (§5.79's
   mint criterion: reachable from a shipped surface); otherwise they list on
   §5.89 as the class's members. §11 entry + stub, entry file first.

**Boundaries:** NO product code; no fixes, no "obvious" guard repairs; the
per-member repair fork is Vixy's with the class in one place. Static analysis
task — no launches expected; if one is needed to ground a reachability claim,
§0.5 discipline applies.

**DoD:** assert census count + method; every member classified with grounds;
class-(i) set enumerated with fall-through consequence + reach; §5.89
annotated (stays OPEN); new rows only per the mint criterion, stated; code
tree clean; §11 entry + stub; WIP per §0.6.
**WIP:** *(empty)*

---

## 2. Blocked — NOT dispatchable (reason stated so the exclusion is challengeable)

- **B1/S4 + B2 + riding rows** (D4 surface streaming, RING asteroid, INSTANCED
  consumer): Vixy-paced architectural line (EntityCore Taskable authority).
- **B5 remainder** (dso3d/tully, ojmMgr, floors): suspended on the §11.96(e) six +
  §11.98(f) three — chiefly the reach-vs-visibility decoupling, now promotion-grade
  GENERAL (§11.97(c) second domain) and Vixy's call before any large content lands.
- **B30 fix**: tracking-convergence semantics suspended (§11.94(d)).
- ~~**B18 residual**: D15 unanswered (the 2026-07-23 batch's only open item).~~
  **D15 ANSWERED 2026-08-26 and propagated (§11.149(a)) ⇒ B18's suspension closes;
  (c)+(d) are task F38's, (a) dissolved, (b)'s implementation half is §5.80's
  (recorded, not authorized as a whole — see §11.149(b)'s veto point).**
  **[CLOSED 2026-08-26: the veto was ratified (§11.151(a)) and the fix DELIVERED by
  F40 → §11.153; §5.80 is closed, its two riders still open ON the row.]**
- **B17 residual**: heading≠0 offset coupling — tilted-dome question (§11.92(d)).
- **B10(a)**: anti-stuck floor VALUE = Vixy's feel-test (§11.79(f)).
- **B14 residuals (ii)(v)(vii)**: source-authority order, meridian texture-
  registration, float32 `re.period` — all Vixy's eye (§11.75(c), §11.86(c), §5.25).
- **B8**: waits for old-path removal by definition.
- **B21 step feel**: Vixy's.
- **Every §13.A row**: Vixy/tester territory by protocol.

## 3. For Vixy — sendable/decidable now (not tasks; parallel to any dispatch)

- **Session-13 decision items (2026-08-26, F37–F40 / §11.149–§11.153):**
  - **§5.100/§5.101 authorization question (ASKED in-conversation, unanswered at
    close):** does D15(c)'s *"continual tracking must be preserved"* authorize
    the new-path tracking start at `zoom auto in` (one line beside the existing
    `armViewOffset`) and the `autoZoomOut` re-aim mirror (needs a Camera
    `lookTo` + a parity question on old's eased ramp)? Yes ⇒ §5.100 dispatches
    decision-free next round.
  - **A44** — `RingModule`'s shadow caster is the ONE scaled shadow radius, by
    your 2026-07-18 extent contract; D21 points the other way. Unexercised on
    shipped content (no scaled body has rings). Which contract wins?
  - ~~**§5.104** — what should reload re-apply?~~ **ANSWERED [vixy 2026-08-26 →
    §11.154(b)]: ownership FORMAT-SCOPED** — legacy: config.ini owns scaling;
    modular: the new FILE owns it, deprecating config.ini whenever the new
    format serves the solar system. Implementation rides B16's seam; one gate:
    **B28 sign-off on the new-format scaling key** before any emitter writes it.
    **[OPERATED same day → §11.154(c): key = `display_scale` (delegated
    deducibility test rides F41); derivation ratified verbatim; F41 dispatched.]**
  - **§5.106** — entering free flight changes what the ENVIRONMENT draws
    (landscape stops updating, atmosphere floods the dome: 3.3 Mpx — this, not
    the teleport, was §11.144(i)'s pixel count). Old path's own `isOnBody()`
    semantics; what free flight should SHOW is yours — same family as D15(b)
    transparency. **[DE-URGENTED per §11.154(a)'s usage-path model
    (interactive-only path); veto point open: close-as-accepted vs keep the
    design question — the atmosphere-flood sub-question stays owed either way.]**
  - **The two §11.144 riders, now with veto points at their sites (§11.153(e)(f),
    each ≤3 lines to reverse):** free-flight `moveto lat/lon` now names the
    anchored place (restoring `Camera.hpp:195-197`'s own contract) — say the
    word if free flight should give it a DIFFERENT meaning; `get status
    position` across the toggle is byte-identical — say the word if the
    free-flight readout should answer something else.
  - **§5.49 reversal warning** — F40 MEASURED the observer half (pose azimuth =
    `lon − π/2`); composing with the confirmed texture half gives `u = lon/360
    − 0.5`, i.e. `moveto lon 0` over the map's CENTRE — the OPPOSITE of §5.49's
    headline. Both halves stay [derived]; the render measurement the row owes
    (`f14_meridian.py` `u_sub`) settles it before anyone "fixes" §5.49.
  - **§5.105's owed check** — old path dumps uninitialized memory as `dist` for
    never-sorted bodies; owed before acting: is `Body::getDistance()` read
    outside the dump for such a body (instrument-grade vs live defect)?
- **Session-13 veto points (implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) F39's uniform dilation dilates the child's offset
  AND extent but NOT the observer's altitude (`moveto alt` = real metres above
  the displayed surface — why the gate is a ratio); (2) the ASmooth fix is
  COMMITTED in the EntityCore submodule (precedent: your `f28c555` "Fix
  ASmooth"); (3) `flag moon_scaled off` in the seven older harnesses is
  re-read as a SCENE DECLARATION, not removed (their committed baselines
  measure real geometry); (4) F38's `[navigation] attached = true` +
  `flag_lock_sky_position = false` spellings (B28; `boundToSurface` naming
  constraint recorded at the define); (5) F40's `getPlace` rides the corrected
  converter (keeps the readout byte-identical — the alternative silently broke
  it).
- **Session-13 awareness, no action needed:** §5.99 (b39_star's whole-frame
  `lit()` criterion — instrument, fix needs a re-run) · the A–D battery is not
  phase-locked (same-binary control first, §11.153(i)) · `dumpread.py` is now
  the dump channel's single reader · the tester's `superscript.sts` rewrite
  fixed 11 of 13 script-side witnesses (§11.149(e); the +267 new lines are
  unexamined; scedit's corpus gate not re-run) · Lionel's rewrite answered
  SS-16 by action (fixed in place; pre-rewrite bytes citable at
  `70dee810:doc/superscript.sts`).

- **Session-13, F37/§11.149 — TWO THINGS TO EQUALIZE ON, both about facts rather
  than decisions (nothing here asks you for a choice):**
  - **Your D37 premise was right and OUR measurement's reading was wrong.** You
    reasoned *"every bodies have the star as parent body, in which case hiding the
    star hide every bodies in the system"*; the ledger had a measurement that
    looked like the opposite (a hidden Sun still lighting the Moon). The probe
    settles it your way: the shipped `ssystem.ini` has **90 body sections and
    exactly one `parent = none` — `[sun]`**, the runtime tree is
    `Moon → Earth → Sun → SolarSystem → MilkyWay → Universe`, **91 of 120 bodies
    sit under the Sun** and the other 28 are system nodes and hidden anchors with
    `boundingRadius = 0`. On the same run's own screenshots, hiding the Sun makes
    the Moon **disappear** (disc interior 4.738 → 0.000). The instrument counted
    the whole frame, which the star field dominates, so it could not see the Moon
    leave (→ new defect **§5.99**; §11.117(k)(1) corrected in place, counts kept).
    **Consequence you may care about**: D37, which is now delegated to the tester,
    has **no observable on shipped content** — both answers give the same frame —
    so the question needs an authored scene or the multi-star case you named. It
    goes to the tester with that fact attached rather than as posed.
  - **The tester rewrote `doc/superscript.sts` this morning** (`f0c8ef83`, +267/−67)
    and **eleven of the thirteen witness lines the ledger tracked are already
    fixed or removed** — including the invisible 0xA0 byte and both respell cases
    (`date_display_*`, `zrot/yrot`). That answers **SS-16** (fix in place vs keep
    as a historical document) **by action**: the historical witness now lives at
    `70dee810:doc/superscript.sts`. Two survive: the `landscape … spacecraft on`
    line and the `$body_selected` number table. **Owed and not done**: the 267
    ADDED lines are unexamined and scedit is not runnable from this working copy,
    so this was a "did the old problems go away" check, not a fresh pass.

- **Session-12 decision data (2026-08-09, F34/§11.144 + F35/§11.145 +
  F36/§11.146):**
  - **§5.80's owed datum is PAID — the teleport lives on the CONVERTER.**
    (A) `spheToRect(−lon,lat)·d` is used by exactly the three sites that
    convert between the anchored triple and the free cartesian member
    (`setFreeMode` both ways, `moveTo`'s free branch) and by NOTHING else:
    `descend`/`moveEyeRel` never see the triple and are exact against the
    composer (5.5e-12/3.4e-12 AU vs 6.7e-06 for the flipped sign) — so the
    repair's blast radius is smaller than §5.80 first estimated, its nature
    (free-flight semantics) unchanged. The converter is wrong TWO ways — a
    missing negation AND azimuth handedness — composing to one 180° rotation
    (the observer's latitude flips sign across the toggle). NEITHER shipped
    readout can see it: `selDist` by construction, `get status position`
    because `getPlace()` inverts what the converter just wrote (entry triple
    returned identically across a 13 732 km teleport). One command, two
    places: the same `moveto` lands 16 700 km / 170.6° apart depending only
    on free-mode. **Your decision when ready:** what `moveto lat/lon` MEANS
    in free flight (re-expressing the converter as the composer's inverse
    leaves `descend`/`moveEyeRel` and every anchored place untouched — the
    datum's statement, not a proposal).
  - **F36's decision (§11.146(j)) — the startup-silence fix turns on ONE
    question:** the silent class is 37 sites on FIVE channels (not "stderr"),
    and NO uniform additive routing exists — measured, not argued: the
    installed config has `print_log = true`, which makes `cLog L_ERROR`
    write the console too, so the §5.77 row's own expected one-liner prints
    the message TWICE (four-cell measurement, §11.146(f)). **Should a
    startup failure appear on the console twice when `print_log = true`?**
    Yes ⇒ the additive call lands at 20 sites (13 need new wording first).
    No ⇒ the fix is not additive: remove raw writes (changes the console
    for `print_log = false` installs) or give `cLog` a log-only entry point
    (B28-adjacent). Sub-question deciding 2 more sites: extend the pre-log
    `out`-accumulator idiom to the failure paths?
  - **§5.90 — the round's biggest operational find: the app runs on 26 561
    stars** instead of the level-2/3 catalogues' millions, silently. The
    catalogue LIST is read from `~/.spacecrafter/stars.ini` but the FILES
    from `/usr/local/share/spacecrafter/stars/`; the two disagree on
    versions on this install — and `~/.spacecrafter/stars/`, where the
    loader does NOT look, carries exactly the requested versions. The log
    says `Loading catalog X` with no outcome and summarizes
    `max_geodesic_level: 1`. **Your questions (same class as §5.74's):**
    does the delivered `spacecrafter-data` ship a `stars.ini` matching the
    catalogues it installs, and is `~/.spacecrafter/stars/` meant to be a
    search path? NOTE the mechanism is CODE (split roots, I2), not field
    content — the fourth member of the field question but the first whose
    data exists locally in the unread root.
  - **Supervisor hypothesis [derived, NOT measured]:** §5.90 may explain
    §11.142(h)'s sparse HIP index (3 of 14 swept ids resolve) — the missing
    level-2/3 catalogues carry the bulk of HIP stars. Discriminating check
    for whoever gets it: matching list/files pair at the loader's path,
    re-sweep the 14 ids. If confirmed, the field question's third member
    reclassifies from field content to §5.90's code mechanism.
  - **New rows recorded, fixes routed, none blocking:** **§5.86**
    (`Camera::observedToBodyLocalPos` is not `viewMat`'s inverse — the new
    path's RA/DE readout computes in a scrambled frame, 133.9° round-trip
    error; fix belongs with §11.4's closer; the algebraic inverse is written
    out in `f34_probe_inverse.cpp`) · **§5.87** (`select constellation_star`
    with an unresolved abbreviation acts on the PREVIOUS selection — unselect
    vs no-op vs today's is yours, §2(f) attached) · **§5.88** (a missing star
    catalogue reports NOWHERE — not even the console; three contract shapes,
    yours after the owed draw-cost datum) · **§5.89** (unknown
    `viewing_mode` falls through a DEAD `assert` in the shipped build type —
    abort vs named-default-and-log (D12) vs refuse-to-start, yours).
- **Session-12 veto points (implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) F35's §5.81 guard answers the LIMIT — a body at
  `distance == 0` reads `screen (0,0)`, the value the algebra forces for
  every finite projection factor; NO NaN sentinel, because "belongs to the
  drawn surface" already has one authority (sweep membership + hidden-list
  unregistration) and a sentinel would be a silent second one (I2); one
  `if` to reverse (§11.145(a)). (2) F35's §5.79 guard — an empty
  constellation selection answers an empty `Object`, the sibling holders'
  own answer, tolerated end-to-end by the one caller; this also FIXES a
  measured SIGSEGV on the shipped `select constellation_star <abbrev>` at
  the default field state (rc −11 → alive, EOL/EOL); one `if` to reverse
  (§11.145(c)(d)).
- **Session-11 decision data (2026-08-09, F31/§11.141 + F32/§11.142 + F33/§11.143):**
  - **§5.74 (search finds no stars/constellations) — the answer is the FIELD:**
    the load never ran; every one of the **2922 files under
    `~/.spacecrafter/sky_cultures` is 0 bytes** (`western-spacecrafter/info.ini`
    included), so the configured culture is rejected ABOVE the loaders — and the
    rejection reaches only stderr while `spacecrafter.log` says `Check
    sky_cultures subdirectory ok` (§5.77). The match itself measured SOUND
    (fixture load on the same launch: 0 → 1085 `(S)` + 3 `(C)`). **Your
    question:** does the delivered `spacecrafter-data` carry sky-culture
    content, or ship it empty? The field-content class now has THREE members
    answered by that one question: `sky_cultures` (2922×0 B), `stellar_systems`
    (13×0 B, §11.109), and the sparse HIP star index (3 of 14 swept ids
    resolve, §11.142(h)).
  - **D28's decision surface grew a concrete member (F33):** old's
    `transition_to body` ends at heading 0 (measured: ramp from 49.139° over
    5 s, start = minus the body's SCREEN axis angle); the new path holds the
    whole orientation (A38). Which ships at a reference switch is exactly
    D28's existing question — nothing new asked, it got a shipped-command
    instance.
  - **§5.85:** `align_with body` measured to NOT align (second call moves
    heading another 25.2°; start-dependent by 29.6°), and its author's inline
    note says why. What the command is FOR only its author or a show that
    wants it can say; 0 shipped scripts use it.
  - **§5.82:** `transition_to point name <X>` DROPS its documented name
    (hard-coded `temp_point`; 19 shipped lines pass names that never had an
    effect). Honouring it changes what shipped lines DO = product decision.
  - **§5.80 (the round's biggest find):** entering free flight TELEPORTS the
    observer ~125° around its reference at constant distance (11 300 km on
    Earth, 99 450 km at Mars, 6354 px on screen vs a 0-px A/A control);
    `selDist` is blind to it by construction. The owed datum (which
    parametrization the free-flight movers were BUILT against) is decision-free
    and next round's dispatch candidate; the FIX that follows is a free-flight
    semantics change = yours.
  - **§5.78 (F31):** `loadSciNames` has no caller (sci-name star search is
    structurally dead) and `updateI18n` drops 1140 loaded names (several names
    per HIP, last wins — one-name-per-star intended?). Rows carry what's owed.
- **Session-11 veto points (all implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) F32 replaced `operator=`'s self-assignment guard
  with retain-first ORDER (also covers two Objects sharing one rep; net zero
  for literal self-assignment); the holder enumeration is now `object.hpp`'s
  header doc. (2) F33's travel = a re-declared MOTION LAW (`TravelOrbit` on
  the anchor body) — one position authority, pure function of the date; old's
  logistic curve transcribed QUIRKS INCLUDED (the 9.11e-04 start pop, the
  1.5e-08 never-arrives) because old is the baseline. (3) F33's
  `transition_to body` carries NO heading tail on the new path until D28
  answers — the two paths' images deliberately differ at that member. (4)
  F33's `placeAt` writes `foldLat` BEFORE `recoverParams` (A38 restoration
  under the shipped equatorial mount, 1.57e-02 → 7.04e-07 rad; the null
  control was run — order reversed is bit-identical broken). (5) The new
  path's unknown-name refusal carries a diagnostic old lacks (behaviour
  identical, §2(f) filled on the port side). (6) **§5.79 supervisor-minted**
  from F32's in-entry record — a crash reachable from a shipped command
  (`select constellation_star` on an empty selection) belongs in the registry;
  local untestability is not a mint criterion.
- **Session-10 decision data (2026-08-09, F30/§11.140 + F28/§11.138 — every
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
  - **Truncation policy (from F28/§11.138, riding §5.73's closure):** the
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
  (§11.140(h)).
- **New awareness rows, no action needed now: §5.74** (`search` returns no star
  and no constellation on the shipped corpus — cause not yet discriminated,
  owed datum named in-row, next-round dispatch candidate) · **§5.75**
  (`TrailModule::accumulate` drops samples on date jumps — every date-stepping
  show carries a trail that lags its body; the fix is one expression but
  changes shipped-trail sample counts = policy).
- ~~**A15 re-ask is SENDABLE**~~ **JOINS THE FINAL TESTER PASS** [vixy 2026-07-30,
  batching principle → §11.116(c)]: tester items accumulate into ONE final pass
  before testing deployment; the final-pass list is ledger-owned (~~members so far:
  A15, the oort-shadow item below~~ **members after F37/§11.149(g): A15 · the
  oort-SHADOW item below · D37 as a QUESTION, carrying the premise fact that on
  shipped content its two options give the SAME frame · D15(a)(b)(c)(d) as four
  INFORM items with Vixy's own revise/revert offer · D15(b)'s heading-stability
  default as a CONFIRM item**), round-3 file materializes at send time. The
  §11.116(c) state-stamp rider applies to every member; (g1)'s scene is AUTHORED,
  so the authored file is part of its stamp.
- ~~**§11.98(c) missing datum**~~ **RESOLVED 2026-07-30 (§11.116(b))**: the
  originating observation was recovered verbatim from session transcripts — it
  says "oort **SHADOW** showing too early", its configuration reconstructs to
  **free_mode/Sun-ref, fov 340 pinned** (the "(anchored on earth)" text was the
  requested ladder's SPEC, not the watched scene); the **zoom confounder is
  REFUTED** (closed candidate set, zero fov/zoom commands); the anchored-Earth
  refutation never reached that configuration ⇒ genuinely-early stays live there,
  vs expectation-wrong — discriminated in the final tester pass, state-stamped.
- **Decision batches waiting**: §11.96(e)(1–6) + §11.98(f)(i–iii) (oort/§6.9 plan);
  ~~D15 (expanded §11.112 — sub-item (c) mirror-all-four is recommended + mechanical);
  D21 (corrected form §11.101(f))~~ **D15 + D21 ANSWERED 2026-08-26/08-22 and
  propagated (§11.149) ⇒ DECISIONS_PENDING's open set is EMPTY, a first**;
  §11.92(d) heading-coupling; §11.94(d)
  latch-when-settled. D22–D36: ANSWERED + propagated (§11.113). ~~**NEW 2026-07-30:
  D37** (F11/§11.117(k)(1)) — does a hidden body stop being a LIGHT SOURCE?
  Measured: today it does not (a hidden Sun still lights the Moon, 15462 vs 17302
  lit px); illumination is the one contribution D23's general wording reaches and
  the B39 row does not enumerate, and no shipped hidden body is a light source, so
  nothing in the corpus discriminates. Rec (1) keep today's behaviour; reversing
  it later is one branch at `updateSystem`.~~ **D37 — CORRECTED AND MOVED
  2026-08-26 (§11.149(d)); struck rather than deleted because the struck text is
  what this channel would have relayed.** Vixy's answer **DELEGATES** the decision
  to the main tester/user (*"it changes the behavior of spacecrafter under
  identical use"*) ⇒ D37 leaves this list for the final tester pass. And the
  measurement quoted above is **REFUTED on its own committed artifacts**: with the
  Sun hidden the Moon is **GONE, not lit** (disc-interior mean 4.738 → 0.000; 95 %
  of the 1217 px>32 within 150 px of the Moon's centre; the Sun reads
  `visible: false` in BOTH frames, so nothing of it can have "left"). Cause →
  **§5.99**: `b39_star.py`'s `lit()` counts the whole 2048² frame, which the star
  field dominates (15 462 px background vs ~1 850 px Moon). Vixy's own premise is
  what holds: the shipped tree has **exactly one root** (`[sun] parent = none`),
  91 of 120 runtime bodies sit under it, and the 28 that do not are all
  `boundingRadius = 0` + `visible: false` — so **hiding the star hides the system**
  and D37's two options are indistinguishable on shipped content. Reversal is still
  one branch at `updateSystem`; the question is reachable only in an AUTHORED scene
  (a body `parent = none` beside the star) or a multi-star delivery.
- ~~**F1 instrument authorization (§11.99(h))**~~ **SERVED 2026-07-24 (manual
  approval) → root closed §11.100.** ~~Replaced by: **D21** (DECISIONS_PENDING) —
  grounded children vs parent display scaling (`moon_scale=5` swallows the mandate
  scenes; §5.27).~~ **D21 ANSWERED 2026-08-22, propagated §11.149(c): unscaled for
  physics, scaled for bounding/rendering, grounded children inherit — fix = task
  F39; §5.27's behaviour half and B31's T3 both unblock.** Workflow note for shader
  edits [vixy]: `shaders/compile.sh` + `cmake --install` — not hand-copies into the
  install dir.
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
