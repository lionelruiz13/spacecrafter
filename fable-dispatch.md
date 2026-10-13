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
source, never recall). Compiled at code `master-beta @ ae3a218d`, harness `0abd30c`.

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

**Close = depart [2026-10-10, owner-traced — QUEUE Q-110, §11.303]:** the pass above is no
longer a pass deferred to the next open: a task section leaves this file IN its acceptance
commit (§0b step 4 — `harness/fd_archive_pass.py`, one unit, the same commit as the
acceptance), a session note leaves when the next one is written (§0b step 5), and §1's
derived index names nothing that grows (since pass 31, 2026-10-11: its one sentence is regenerable from the archive listing). "Deferred to the next open" twice is how sixteen delivered
sections and five notes were live on 2026-10-10. The reason, his: the attention needed is on
the OPEN units; a closed one on the live surface is a cost every reader pays — closure without
departure is an addition.

---

**Update [Claude Fable 5.1 2026-10-11, supervising session 44 — LovelyFoxDev, Sunday evening, the owner ABSENT (the Sunday-evening window of his scheduling line); his trigger = the §0b line verbatim]:** Warm-up [measured]: both trees clean at open; code `main` @ `afae8573` (F146's three), harness `134660e`; binary `8a5d8fcf` AT HEAD (`make -n` 0 — the 14:13 mtime predates the 14:26 commit of the same tree); `deploy.sh --check` CURRENT; both ledger instruments exit 0; `review.sh status` his mark `ee185d4f`; live `### F` 0; next §11.319 / §5.196. Two defective checks of the supervisor redone (a `| tail` hiding the build dry-run's exit; a sort keyed on the wrong field). MINTED 17:58 (`90cee65`): F148 (F144's implementation + the §5.194 rider), F150 (B46's open half: one script for a delivery's shape numbers), F149 (F142's implementation, carrying the release-switch dedup after F137); gates 43 / 43, 23 / 23, 54 + 5 REFRESH; the owner's H / Q defaults BUILT (no word since 2026-10-10), each one revert. DISPATCHED 18:00 as a pair: F148 (`opus-xhigh`, the code tree) + F150 (`opus-xhigh`, harness only). F148 STOPPED 18:20 at P11 before any edit — `switchToBody` holds the view (1e-6 rad) and WARPS the place 244 AU at the system boundary (the kept `position` vector read in the new frame; `recoverParams` rotation-only; the owner's baseline has the same form; the free-move walk teleports 0.99 AU at Earth's influence — his D15 (b) measured): the structure's (d4) refuted by measurement; LIFTED 18:5x (ladder step 3): option (b), the hold composed from existing entries inside `followInfluence`; (c) the hold inside `switchToBody` routed to him (§5.197). F150 DELIVERED 18:32 (`delivery.py`; self-test 167 / 9 named) with two instrument findings beyond its task — `names.py` / `writers.py` stripping `/* */` before `//` (then a second mechanism, `'"'` char literals): `ssystem_factory.hpp` read 78 of 162 names since the instruments' birth, `AppCommandInterface` 5 of 91 member functions — and the D14 gate blind to `claude/` (§5.196); its FOLLOW-UP (18:40–18:53) landed one scanner (`decomment.py`), `PASS*`, form.py on `ratio.classify` (SHAPE §4's library row 4.8 / 39 % → 5.2 / 37 %); ACCEPTED 19:0x (`467a116`, pass 37) by the supervisor's hand. D5 / [Y58] APPLIED 19:1x by the declared default (`4feab47` + pass 38 `b728cc8`): §0 step 6 the one-line form, the executor definition's three stale bullets re-pointed, the projection regenerated — the census §7.6 had held it for his word; the discrepancy is in section 3. F148 DELIVERED 20:24 and ACCEPTED 20:5x (`c783650`, pass 39): the decider with the place held (3.3e-5..1.75e-4 AU, 1e-6 rad), OBSERVATION 1 OF THE TESTER CLOSED (f131 obs 1 six RED by the fix's sign, re-formed to 12 PASS), every gate re-run by the supervisor's hand, F150's `delivery.py` block pasted (first live use); COMMITTED `3373ff4f` (the decider) + `2a0fdbef` (the rider), author Claude Opus 5.5; §5.181 / §5.194 CLOSED, §5.197 (the free-move teleport) / §5.198 (the old executor's `onEnter` write) minted. F149 DISPATCHED 20:53 on F148's acceptance (code `2a0fdbef`, binary `35efed69`; gate 59 / 59). SUPERVISOR DEFECTS this session, recorded: Q-72 instances 6 and 7 (a lost `cd` in a parallel batch, twice, each after writing the prevention — no `cd` token in any parallel batch from 19:0x on); a recalled-anchor edit caught by its own assert (SHAPE.md, nothing written); the `:802` attribution and the dump's `system` field in F148's mandate (Q-67's class; miss-ledger); the UNI leg listed with no floor; a commit message saying "dispatched" one step early (`00be4fc`); the session-43 acceptance's re-typed number (11.316:143) caught by F150's oracle. Miss-ledger: two lines (the dump's `system`; the harness ASCII gate) and its misplaced append marker re-seated. The previous note (session 43) leaves with this write (pass 40, `update-s43`). F149 DELIVERED 21:37 and ACCEPTED 21:5x (pass 41): the key-bound scripts built as F142 predicted — 42 / 42 cases, the control byte-identical, net +285 exact; P13's SDL-name rule refuted by measurement (the identifier table); COMMITTED `da70c90c` (author Claude Opus 5.5) + the supervisor's `9fc9145e` (a recorded bind keeps its names as written — D4's resolved absolute paths would not replay on another install); §5.199 minted, §5.191 annotated, B49 implemented; the dispatcher's `internal/M08.sts` defect (Q-67's class) recorded. ROUND TOTAL: three deliveries and one follow-up (F148 after a STOP, F150 + its follow-up, F149), FOUR code commits on `main` (`3373ff4f`, `2a0fdbef`, `da70c90c`, `9fc9145e`), passes 37–41, D5 applied; the dispatch file holds no live section at this close. NEXT ROUND'S DISPATCHABLE SET: §5.191's consolidation through the hook (FOCUSED, B49); the joystick row (Q14, B49 — a structure pass first); the one-constant dedup of the obliquity / galactic literals (his word on the home); §5.193's order fix (his word); §5.197 the hold inside `switchToBody` (his word); the KEY-form leg of observation 4 (F135's P33, harness); B48's first leg; the UNI leg's banking (his word on the binary); the harness ASCII gate (§5.196, his word on the partition); the small items (the four command words into the scedit grammar, README F124's stale line).

---

## 0. Cold-session warm-up protocol (run this first, every dispatch)

1. `CLAUDE.md` auto-loads (the map). Read THIS file; locate your task's section; read
   its **WIP line** — if non-empty, a prior run aborted mid-task: `git -C` log both
   repos since the noted checkpoint and resume from there, do NOT restart from zero.
   After any discontinuity, re-read sources before editing them (a summary/WIP note is
   unidentified knowledge until re-extracted).
2. Re-read the task's §13 row in `INTENT.md` + the `INTENT/<id>.md` entries it names.
   (A task whose mandate lives elsewhere in the ledger — a §11 clause, a QUEUE item —
   has no §13 row; its dispatch section names its mandate rows, which serve the same
   role. Added at F57 acceptance from an executor unstated-premise report.)
3. `git -C /home/claude/spacecrafter status` and `git -C /home/claude/spacecrafter/claude
   status` — both clean expected; note both HEADs.
4. Build: `build-claude/src/spacecrafter` must exist (harness default `SC_BIN`);
   rebuild if the code HEAD moved (`claude/harness/README.md`).
5. Standing constraints (reasons in INTENT.md; violations are delivery-blocking):
   - Old render path = comparison baseline, unchanged by construction (§11.52(b)).
   - Data values NEVER from recall — cited fetch only (§11.51(d) red line).
   - Fresh-launch precondition for measurements; config/ssystem md5 in==out asserted
     (pristine pair as of compile date: `03fbee59` / `545a51ef`).
   - **Encoding:** the Bash `grep` wrapper (ugrep `-I`) skips every file holding non-UTF-8 bytes in silence, so a 0-hit there proves nothing - since F70 only `~/.spacecrafter/ssystem.ini`, `doc/superscript.sts` and the EXCLUDE rows of `harness/f70_partition.tsv` hold such bytes: use `/usr/bin/grep` or Read on them, `LC_ALL=C` for a byte class (`CLAUDE.md`'s encoding hazards (1)-(5) are the authority; §11.188(k), §11.189(a)).
   - Cost claims use the **1 ms/frame** denominator (§2.0 D11). Acting defaults are
     LOGGED (§2.0 D12). Actionable diagnostics per §2(f).
   - **Anchor-first enumeration (owner mechanism, 2026-09-01, §11.193):** a task
     that creates or reroutes an information event (a diagnostic, a notification,
     a recorded fact) NAMES the event's responsibility anchor — its I2 placement;
     in execution space, the detection point — and walks the channels FROM it in
     the task section before implementation. The form is owned at the anchor and
     routed down; sinks never re-render. Reason: enumeration at the anchor is
     complete by topology; at any sink it is recall — the §11.184 log-channel
     miss is the measured instance.
   - A green build is not coverage — every task names its discriminating check; if a
     check needs a display, say so instead of substituting a weaker one.
   - **No `run_in_background` for long campaigns** — foreground within-turn batches
     only (§11.98(h) template fix; three executor stalls root-caused to this).
   - **Memory-bounded builds (2026-07-30 host-OOM incident — killed an executor
     mid-task):** session affinity is now 12 cores, so `-j$(nproc)` self-caps at 12;
     additionally check `free -g` BEFORE each build — available < 16 GiB ⇒ use `-j6`.
     Other sessions share this host's RAM; memory, not cores, is the binding
     constraint. **[PER-HOST 2026-09-04, session 21: on LovelyFoxDev (the desktop)
     the affinity is `0-23`, so `-j$(nproc)` = 24 — the SessionStart hook prints the
     host's safe `-j` from live `free -g`/`nproc` (24 at open, 52 GiB avail); use
     THAT number, the "12" above is the state of one earlier session, not a rule.]**
   - **Concurrent-instance assert, before each measurement launch:** `bash claude/harness/sc_instances.sh --assert <label>` (exit 2 an engine is live, 4 the probe could not run = a STOP) or `harness/f116_assert.sh <label>` (+ the GPU gate, exit 3) - because concurrent sessions share `~/.spacecrafter` and an md5 re-assert cannot see a concurrent launcher; the criterion is the UNION of `comm`, `/proc/<pid>/exe` and TCP 7805, each covering another's blind spot (residual, said and not claimed: a renamed engine of another uid with no server up); on a hit: record it, wait it out, launch fresh; never `pkill -f` (§11.238).
   - **Session environment:** the display target, the auth path and the session's shape are PER-HOST and PER-BOOT - read `HOST-EVENTS.md`'s latest entries for this host and run `xdpyinfo` before the first launch, because an inherited `DISPLAY` / `XAUTHORITY` can name another uid's or a dead display; `timeout -s KILL` bounds nothing in this session type (measured, §11.140(i)): use plain `timeout` or a poll-and-kill watchdog.
   - **§5.109 hazard (2026-08-29, F43):** `moveto … alt` counts from the
     DISPLAY-scaled datum, snapped once — never `flag moon_scaled off; sleep N;
     moveto`; wait the scale settle BY MEASUREMENT (`scaling`==`scalingTarget`
     in the dump) and assert the observer's radius (band checks pass on both
     values).
   - **The display stack is part of the instrument:** we render on our own session (the real `:2`, per HOST-EVENTS) while the owner's RDP relay is view-only and CPU-coupled to what we draw, and the cadence is the config cap (`maximum_fps = 144`, measured [144.0, 146.4] fps, §11.159) - so trust counter RATIOS and in-run A/B only, never absolute fps labels or cross-session cadence, and a stack change (compositor, streamer, X server, screen power) means report + re-baseline.
   - **Environment-fault mitigations are OWNER VETO ITEMS (2026-08-30, §11.174(h),
     owner-stated: *"If the F43 agent reported the failure over mitigating it, I
     would have corrected it"*):** any improvised substitute for missing host
     state (display stack, runtime dir, auth) is reported WITH AN EXPLICIT
     DECISION FLAG — "your correction may differ from my mitigation — say the
     word" — never only as an accomplishment record; the owner's proper fix and
     the mitigation may differ in preconditions the verification surface does
     not reach (*"different path doesn't certify same preconditions"* [vixy]).
   - **ENVIRONMENT CANARY before any measuring launch** (`harness/f56_canary.sh`: full, ~100 s, for photometric tasks; `--no-scene`, seconds, otherwise; §11.176) - because a host change that reaches the instrument reaches every number taken on it: a non-zero exit is a STOP and a report (§11.174(h)), never a mitigation or a widened band; a re-bank is one VALUES-block edit WITH its argument, and the bank is PER-BOOT on the real `:2` session, so a red after a reboot is the protocol (§11.199); the band's values live in that VALUES block, not here; §11.104(d)'s numbers are targets only on a canary-passed stack; A/A floors are per scene (374 px star field, 28 px Moon frame); the shared `~/.spacecrafter/cache` is WATCHED (`f56_manifest.py`).
   - **`/tmp` IS SESSION-LIFETIME, THREE TREES CARRY, SSH KEEPS `:2` (2026-09-04,
     owner-stated, session 21; cross-project record `~/shared/QUEUE.md` Q-61):**
     (1) *"/tmp get cleared at the end of the session — nothing tell when it will
     persist and when it's not traced"* [vixy] — ASSERT every staging binary, farm,
     log or draft under `/tmp` is gone after the working session (the 2026-08-31
     20:57 and 2026-09-04 18:45 wipes were instances, not the rule); anything meant
     to survive lives in a carried tree; a `/tmp`-homed display stack (F43's shape)
     is dead by construction. (2) Exactly THREE trees migrate between devices:
     `~/spacecrafter`, `~/shared`, `~/.claude` [vixy] — `build-claude/`, `/tmp`,
     gsettings and everything else are per-device (why this host's binary was stale
     and its `lock-enabled` differs from the laptop's). (3) Future sessions may run
     over SSH to the desktop [vixy]: *"I would leave :2 open and set DISPLAY to it.
     Spacecrafter use SDL2 which uses X11 which work through DISPLAY"* — the
     canonical display stays the RDP-created real logind session on `:2`, kept
     logged in; `Linger=no` and the mutter auth cookie live only with that session,
     so a logout/reboot = re-provision by the owner + canary re-bank via HOST-EVENTS;
     under ssh `export DISPLAY=:2 XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*)`
     (the F28 recipe) and `xdpyinfo` before the first launch remain the invariant part.
   - **Question routing by stratum (2026-08-29, §11.161(c), owner-stated):**
     old-behavior intent/expectation questions → the main tester (*"he either
     knows or tell what he had always expected, both are a resolution"*);
     Vixy-strata questions (experimentalModule, EntityCore, CoI/big-texture,
     Vulkan layer, rendering paths) → Vixy's recall. The stratigraphy is
     §11.161(b).
   - **Back-marker at the write (§11.161(g), RA-MODEL E18/E55):** an entry that supersedes, refutes or corrects ANOTHER entry's claim places the §11.113(p) back-marker at the superseded node IN THE SAME COMMIT, and a §13 row's state flip greps the §5 register for the references it stales in that commit - because the write is the one moment its author holds both ends (I3: the state's owner informs its dependents), and every stale reference measured sat on the non-working side (§11.180).
6. **Abort-tolerance (the reason this file exists: a cut run is an abort, never a suspend, so it must resume from what is on disk):** an executor commits nothing unless its dispatch prompt says so (§0b (a)); it keeps its checkpoint state in the FIRST LINES of its own entry file (`INTENT/<id>.md`), refreshed at every green checkpoint, and a cut run resumes from there. The supervisor is the one writer of this file and of the commits: it verifies and commits each delivery (code first; the harness commit carries `Code: <branch> @ <short-sha>`), keeps the task's WIP line, and never starts a long verification campaign over uncommitted work. Delivery = the §11 entry (the file first, then its stub) + the §13 / §5 flips + the acceptance commit.
7. **Precondition gate (owner-stated 2026-08-30, §11.175) — the LAST warm-up act; nothing
   mutates before it passes.** **FIRST STEP (2026-09-05, §0b.3's PREMISES block): run
   `python3 claude/harness/premise_check.py <ID>` from anywhere — every line of the
   section's block re-runs against live state and prints observed-vs-stated in checkable
   form; any FAIL is a broken premise (input-side ⇒ abort per the rule below; an
   unrefreshed `REFRESH-AT-DISPATCH` line is a dispatcher defect to report). The prose
   premises below are checked after, the same way.** Enumerate every premise the task section and the dispatch
   prompt state as true, and verify each against live state: both HEADs as stated; the
   next-free §11 number actually free; the live `### F` count as stated; the ledger
   rows/entries the task builds on in their claimed state; environment/instrument
   preconditions (display stack per §0.5, binary at HEAD, fresh-launch conditions);
   dependency deliveries the section names. ANY broken ⇒ **task abort**: report
   observed-vs-stated to the dispatcher, mutate nothing, do NOT adapt or repair the premise —
   that decision belongs to the dispatcher/owner (§11.174(h): report-over-mitigate;
   *"different path doesn't certify same preconditions"* [vixy]). Reason the abort is
   unconditional: a breakage absorbed at initiation propagates silently — the work can look
   green throughout and stays premise-compromised whether or not the breakage resurfaces
   (§11.174 is the full-scale instance); an abort costs one round-trip. A precondition
   discovered broken MID-task has the same semantics from the discovery point: stop,
   checkpoint-commit what is green, report — the gate moves the default leak point to
   initiation, it does not license ignoring later discovery. The gate validates only what
   is ENUMERATED (§0b.3 binds the dispatcher to state preconditions in checkable form);
   a premise you rely on that no source states is itself a report-worthy finding.
   Scope ruling [F59 acceptance 2026-08-30, §11.179(a)]: the abort binds on premises the
   work STANDS ON (inputs); a stated premise the task does not consume — an output-side
   gloss, a label the task itself re-adjudicates — that fails verification is a
   REPORT-worthy dispatcher defect, not an abort trigger. Treating any stated premise as
   non-load-bearing must itself be reported with the argument and the counterfactual
   ("had it been an input, abort") — silence about the reclassification is what the gate
   forbids.

Sizes: **S** ≈ short focused run · **M** ≈ one full session · **L** ≈ full session at
high effort, mandatory checkpoints. Estimates are mine [derived], not measured.

---

## 0b. Supervising-session protocol (Fable) — the user triggers a round with ONE line

**Trigger phrasing (reuse verbatim):** *"Dispatch round: run a supervised dispatch
session per claude/fable-dispatch.md §0b."*

**SHAPE BINDING [owner-ruled 2026-09-19, session 33 — INTENT §2.1 G12, §11.249–§11.255;
operating manual `claude/SHAPE.md`; it OVERRIDES the steps below wherever they differ].**
(a) **Executors are routed by STRUCTURAL-COUPLING LOAD at the mint, never by label** [vixy 2026-10-10, §11.302; the 2026-09-27 structure / implementation split, §11.293, is its common case]: FOCUSED (the input set closed and named in the section, one concern, criteria that can fail) → `opus-xhigh` (Claude Opus 5.5 under the owner's system prompt; step 1 asserts `deploy.sh --check`), bound to report every conflict with the predictions; OPEN-COUPLING (an S4 pass over a family, a shape still to choose, modelling the owner, a PREDICTIONS mint, every acceptance, an edit of the ledger's structure) → `claude` / `fable` - because Opus is strongest on focused work and the conflicts it reports against the predictions are the signal that drives the rework [vixy]. Disjoint file sets may run in parallel in one tree when no agent commits, builds or stashes; an implementer ALONE in the tree may build and still commits nothing; the supervisor verifies, builds, commits. The return opens with `prompt-window: present` (step 4 reads it). The owner's words and the record the rule was tested on (F131-F139): `fable-dispatch/archive/protocol-routing.md`, §11.302.
(b) **Every task section and prompt binds G12** and names
`/home/claude/spacecrafter.owner-shape` as the form to write in; a task states the
MANAGER COUNT of each information it touches, before and after (G12 (S7)), preceded by the need's
ORIGIN and what it will end up covering (G12 (S10), owner-ruled 2026-09-21 - the supervisor writes it
at the mint, from INTENT section 2 and what is known of the space of use), and a fix that needs
the shape to move is a STOP that goes to the owner. (c) **Step 4 accepts on behaviour AND
shape**: net code lines, deleted/added, comment form (`harness/shape/form.py`), manager
count (`harness/shape/writers.py`); a comment-only change is proven by
`harness/shape/codeident.py`, RE-RUN BY THE SUPERVISOR; a delivery that only adds is a
finding to explain. (d) **Step 5 reports `harness/shape/review.sh status`** to the owner:
his review queue is decoupled from the sessions, and only he moves its mark.

1. **Warm-up**: both trees clean + note HEADs; binary exists at code HEAD; re-read this
   file's update notes AND the §13 rows of the candidate tasks (this file is a view —
   §13 + the cited entries win). After any discontinuity, re-verify state before
   relying on it (the 2026-07-30 OOM left an executor's uncommitted diff in the tree;
   "clean expected" is an expectation, not knowledge).
   **Definition-drift assert:** `bash claude/agents/deploy.sh --check` must exit 0 - the executor's deployed projection is GENERATED from its authority + the owner's agent prompt + the bridge and compared byte-exact, because a drifted definition silently changes every executor; on drift run `bash claude/agents/deploy.sh` before any dispatch (§11.302).
   **Ledger instruments (2026-09-06, the miss-ledger item — two
   supervisor sessions reached for them under `harness/` from memory of a note that omitted
   the directory):** `cd /home/claude/spacecrafter/claude && python3 intent_backmarker_scan.py .
   && python3 intent_pair_check.py .` — both live at the harness repo ROOT, not under
   `harness/` (README §F78). The 2026-07-25 supersession-block-in-every-prompt
   era is CLOSED: prompts now carry only per-round variables (HEADs, date, next §11
   number, task pointer, live section count, task boundaries).
2. **Pick the next 3 dispatchable tasks** by file position, honoring deferral/queue
   notes in the update block; a task without a section here is not dispatchable —
   mint the section first, commit, then dispatch. Sizing is measured
   [vixy 2026-07-30]: 6 hard tasks ≈ 80% supervisor context; fewer than 3 repays the
   session's derivation cost too often; 3 is the sweet spot.
3. **Dispatch sequentially** — one `opus-xhigh` executor per task, synchronous, never
   parallel (deliveries are each other's baselines). Every prompt carries: the per-round variables (step 1:
   no supersession block - the definition is generated from its authority), the task-section pointer +
   warm-up order, the mandate verbatim or by exact ledger ref, stop boundaries,
   discriminating checks, checkpoint discipline, and the report format (per-item DoD
   state + evidence pointers + deviations + suspensions + what the next task must know).
   **Preconditions in checkable form (2026-08-30, §11.175):** the prompt's per-round
   variables and the task section together must state every premise the task stands on
   (HEADs, next §11 number, live `### F` count, ledger states built upon, environment/
   instrument requirements, dependency deliveries) such that §0.7's gate can verify each
   against live state — the executor aborts on any broken one, so an UNSTATED
   precondition is a dispatcher defect (ungateable = uncovered; §11.174's canary gap is
   the shape). Refresh the variables at dispatch time, not at round-open.
   **PREMISES block (owner-ruled 2026-09-05, session 23 close, on the Q-67 prevention —
   *"The prevention proposed is nice, you can implement it"*):** every task section
   carries a fenced ```` ``` ```` block headed `PREMISES`, one line per checkable premise
   in the form `<shell command> => <expected stdout>`, where the expected text is the
   PASTED OUTPUT of that command — never a number, a coordinate, a count or a structure
   recalled from a listing or from convention (session 23's seventeen defects: a
   `body.cpp` count, a `.po` that does not exist, a hook role inferred from its
   filename, README pointers quoted before a later insert shifted them). The
   instrument is `harness/premise_check.py <ID>` (`--list` for coverage, `--self-test`
   shown able to fail); it runs at THREE events: the MINT (every line PASSes before the
   section is committed — a premise typed from memory fails one hop before it reaches an
   executor), the DISPATCH (the per-round lines refreshed; a `REFRESH-AT-DISPATCH`
   expectation FAILS by construction so an unrefreshed prompt is visible), and the
   executor's §0.7 gate (first step). What cannot be a command stays in prose, labelled
   `[derived]`/`[stated]`, and is not a premise the gate abort-tests. A live section
   without a block is a coverage gap the rule forbids (`--list` names it). **[2026-10-10, session 42 —
   three instances in ONE round: F143's four readers the mandate's site list missed, F142's "two key
   handlers" refuted by a zero-caller grep, F144's agent rule whose stated mechanism never existed
   (Q-89's class — a text about the code consumed as the code — here the DISPATCHER's own text): every
   STRUCTURAL CLAIM a mandate makes about the code — a caller count, a reader list, an entry point, a
   function's existence or deadness, a site that "does X" — is a PREMISES line (`grep -c` / `grep -n`
   with the pasted output), never prose alone; the filler takes the command. A claim that cannot be a
   command is written `[derived, unchecked]` so the executor's S4 pass verifies it first. The reason:
   the executor is bound to abort on a broken premise, so a claim that is NOT a premise is the one
   breakage that propagates silently into the shape it builds.]** **[Same day, F140's STOP (§11.314): a
   mandate that orders N steps derives its WRITES list and its block from ALL N — F140 ordered the
   owner's two-step delivery with a boundary and a block covering step 1 only (the scope and the
   boundary had different sources), and its executor stopped at reading, rightly. A boundary must never
   forbid what the mandate orders (SHAPE §7b's lesson, third instance this round: F143's `f131_run.sh`
   outdir, F140's `projector.hpp`, F140's step 2). At the mint: for each step the mandate names, point to
   the P lines and the files that carry it; a step with none is OPEN COUPLING and gets its structure
   task first.]**
4. **Verify each delivery BEFORE the next dispatch**: the return message's FIRST line is
   `prompt-window: present` (2026-10-10, §11.302 — absent or elsewhere = the executor ran on
   a projection without the owner's prompt: a dispatcher defect, recorded, the delivery
   still judged); read the §11 entry IN FULL;
   check trees/commits/authors; check every claimed ledger flip (§5, §13,
   DECISIONS_PENDING) at the ledger; judge every deviation and judgment call — endorse
   with the argument, or escalate to a re-analysis agent on doubt. Harnesses are NOT
   re-run when committed artifacts + both-ways discrimination records suffice; a
   claim without such a record IS a reason to re-run or escalate. Record acceptance in
   the task's WIP line; commit the acceptance. **After ANY WIP-tail edit, verify the
   NEXT `###` header still exists before committing** (`grep -c '^### F'` vs expected)
   — two headers have been destroyed by acceptance Edits whose old_string swallowed
   the following header as anchor context (F13, F18); anchor inside the WIP block,
   never across the section boundary. **Then the section LEAVES (2026-10-10, Q-110): the
   acceptance commit carries its archival (`python3 harness/fd_archive_pass.py <pass> <date>
   <pre-commit> F<n>:<start>:<count>`) and §1's index needs no line (since 2026-10-11 its sentence is regenerable from the archive listing) — closure without
   departure is an addition.**
5. **Close**: refresh section 3 (For Vixy) with new veto points/decisions, append the
   round outcome to the session update note, commit, and report to Vixy: deliveries,
   endorsements, anything newly Vixy's, the remaining dispatchable set. **The previous
   session's note archives in the same close (2026-10-10, Q-110): its state is in the new
   note or in §11; two notes live is one too many.**

---

## 1. Dispatch order (load-bearing first; each task states why, so the order is challengeable)

*Derived index (regenerable, never authoritative): every task section not live below has left this file - `ls fable-dispatch/archive/F*.md` lists them, each naming its §11 record in its own header line; F113 was WITHDRAWN before dispatch and keeps its section as the record; F61-F64 (the session-18 scedit round) left no section, their records are `util/scedit/INTENT.md` and `harness/README.md`; the per-round lines that grew here are `fable-dispatch/archive/index-f0-f95.md` and `index-s25-s42.md`.*

### F151 — §5.191's CONSOLIDATION: ONE end-of-execution cleanup at the anchor. `AppCommandInterface::scriptEnded()` — the hook F149 placed at the end of `ScriptMgr::cancelScript`, reached by every end path (the natural end through `terminateScript`, `script action end`, `UI::init`) — becomes the ONE home of what a script's end leaves behind: the three media calls (`textClear`, `audioMusicHalt`, `imageDropAllNoPersistent`) and the if / loop reset (`swapCommand`, `ifSwap->reset()`, `loopOpen`, `loopOpener`); W_END's branch keeps `cancelScript()` alone, `UI::init`'s hand copy of the media list LEAVES; the order is preserved (today W_END calls `cancelScript` first, then its list — after: `cancelScript` → the hook → `endScript` then the list). Manager count of "end-of-execution cleanup": 2 (W_END's list, `UI::init`'s copy — drifted, the if / loop reset missing from the second) → 1. The REACH of the row is re-read on the way: `ScriptMgr::update` drains a skipped `struct if` block inside ONE update (`while (wait_time==0)`, `script_mgr.cpp:306`), so the row's instance is one frame wide; the multi-frame instance is the LOOP state (`loopOpen` / `loopOpener`, set at `:4756–4757`, the body running over frames) — a HOME during a looping show leaves it set and the next natural end reports a FALSE `MSG_UNCLOSED_LOOP` at the dead show's line (`terminateScript` :251–252; the `#!` annotation at that line — the data is the product, D9); the key leg gains that case [S, **CODE under `src/` within TWO files (`app_command_interface.cpp`, `ui.cpp`) and no other — no header changes — + harness; the agent may BUILD (alone in the code tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE while it runs (F153 runs beside it READ-ONLY, no launch, no build); executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): two functions, nine lines that move, one case, criteria that can fail); entry number ALLOCATED: **§11.322**]

**Why now / mandate:** §5.191 (INTENT.md, the row): *"`UI::init` ends a running script by hand ... a copy of W_END's cleanup list MINUS the if / loop reset ... a drifted duplicate (I2) ... Resolution predicted as F142's (e3) follow-up: ONE end-of-execution hook at the anchor (`ScriptMgr::cancelScript`), the two lists folded into it — a second structural change"*, annotated at F149's acceptance: *"the hook is in place ... W_END's list and `UI::init`'s copy can move into it (the consolidation: a second structural change, the next slice on B49)"*; `INTENT/11.320.md` (10): *"`AppCommandInterface::scriptEnded()` now runs at the end of `ScriptMgr::cancelScript` on every end path (natural end, `script action end`, `UI::init`) - W_END's list (`textClear`, `audioMusicHalt`, `imageDropAllNoPersistent`, the if / loop reset) can move into it and `UI::init`'s copy leave; the order is preserved"*; `INTENT/11.312.md` (f): *"end-of-execution cleanup lists | 2, drifted (W_END, `UI::init`) | unchanged in this slice (+ the hook, the future single home)"*. The session-44 close (this file's update note) named it first in the next round's dispatchable set, FOCUSED, B49. The row's 2026-10-10 wording *"his to order"* is read as the SEQUENCING clause of 11.282(e) (one structural change per commit — the hook landed in F149's commit, the fold is the next one), the F149-acceptance annotation being the later wording by the same hand; the owner has written nothing since 2026-10-10: BUILT by that default, one revert, flagged in section 3. One structural change, one commit (SHAPE §6: `writers.py` or the manager list before / after in the message, net lines negative, the gate named).

**The ORIGIN of the need (S10, the supervisor's):** a show ends in four ways the operator can take in session — it runs out, a line says `script action end`, HOME / SUPER+] re-initialise the dome, a key-bound script replaces it — and after every one of them the dome must be the same clean state: no text, no music, no transient image, no half-open `struct` swallowing the next commands (R2 (g): *"recovery to an established clean state between audiences"*, D8's settled-state reading). What a script's end leaves behind is ONE information; the engine that owns the running execution (`ScriptMgr`) is the one node that knows WHEN it ends (I3), and F149 gave it the push edge (`scriptEnded`) — so the cleanup's home is that edge, and nothing else should know the list. What it ends up covering: every present and future end path (a binding's `once` scope, a TCP `script action end`, the next slice's joystick-bound scripts) with no further copy; later, if a show's end must leave MORE behind (a persistent image class, a saved view), the one home grows by one line. NOT this slice: `cancelScript`'s own resets (ScriptMgr's state — already one home), `KeyBinding::endScript`'s content, `Media`'s three calls as a composed `Media::` operation (S4 on `media.hpp`: no such name exists; three lines do not warrant a class — *"unless it's more code than it's worth"*), §5.190, §5.199.

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F151`), then READ IN FULL: `claude/SHAPE.md`; §5.191's row (INTENT.md, section 5); `INTENT/11.312.md` (c)3, (e3), (f); `INTENT/11.320.md` (10) and its Acceptance; `INTENT/11.299.md` (3) (the keyboard's release path, for case O's HOME); the code from the current checkout: `src/interfaceModule/app_command_interface.hpp` WHOLE (names.py 94: the S4 pass — `scriptEnded` EXISTS, extend it; no other name of "end of a run"), `app_command_interface.cpp` — `terminateScript` :247–255, `scriptEnded` :257–260, `executeCommand`'s skip gate :314–316, `commandScript`'s W_END branch :2842–2851, `commandStruct` :4655–4760 (every writer of `swapCommand` / `loopOpen` / `loopOpener`, the `if` and `loop` forms), `reportScriptError` (~:200–220: MSG_UNCLOSED_LOOP :201, the `#!` channel); `src/uiModule/ui.cpp` — `UI::init` :230–264 (the copy :259–262), `handleKeyPressed`'s HOME case :2521–2525 and the SUPER+RIGHTBRACKET case :2438–2446 (the two `app->init()` callers), `handleKeys` / `handleKeysReleased` (the door F149 built, :3351–:3460); `src/appModule/app.cpp` `App::init` :575–640 (`ui->init(conf)` :636: HOME reaches `UI::init` through it); `src/scriptModule/script_mgr.cpp` — `cancelScript` :109–127, `update` :294–350 (the `while (wait_time==0)` drain :306, the loop replay, the natural end → `terminateScript` :345); `src/mediaModule/media.hpp` names (the PREMISES line: no composition of the three calls); the harness: `f142_keys.py` and `f142_keys_run.sh` WHOLE (the leg you extend: the injection :195–210, case N :470–490, the phases, the fixtures under `artifacts/f142/run/`), `harness/README.md` F149's section (:6936–). (1) The S4 pass written in the entry BEFORE the first Edit (the tree clean; the clock of the write against the first mtime): the names of `AppCommandInterface` against "what a run's end leaves behind" → `scriptEnded` exists → EXTEND; `Media` against "clear what a show put up" → three names, no composition → the three calls stay three calls; `UI` → nothing to add. (2) THE CASE FIRST, on the HEAD binary (the BEFORE record): case O in `f142_keys.py` per P6 / P7 below — a fixture show `o.sts` (`struct loop 20` / `wait duration 1` / `struct end`, absolute path under `artifacts/f142/run/`), played over TCP; after two seconds inside the loop, HOME injected where the event loop sends a key (`$ui->key_Modifier = 0`, `handleKeys(SDL_SCANCODE_HOME, 0, 0, PRESSED)` then `RELEASED` — F149's door; the HOME case queues `internal/initial.sts`, which then ends naturally); the readings: `~/.spacecrafter/log/script.log` between markers (an `MSG_UNCLOSED_LOOP` line attributed to `o.sts`, or none), the fixture file's `#!` tail (rewritten each run), and the three media states cleared (the text / image / audio you set up before HOME: a `text action load` line and an image, as the show's own lines); run `f142_keys_run.sh s45_f151_pre --phase after` on the HEAD binary: 42 PASS on A–N and O's BEFORE reading pasted (P7: RED as the defect's sign, judged by you as the expected BEFORE — an `after` run whose O line reads the error). (3) THE CHANGE, one structural change: `scriptEnded()` gains the seven lines in W_END's order after `endScript()`; W_END's branch keeps `scriptInterface->cancelScript();` alone; `UI::init` :260–262 leave. Build (`free -g` first; the host's safe `-j` is the SessionStart hook's), `python3 claude/harness/f70_ascii.py gate`. (4) THE GATE: `f142_keys_run.sh s45_f151_post --phase after --baseline s45_f151_pre`: 42 PASS, N identical to the pre run, O GREEN; `git -C /home/claude/spacecrafter diff --stat` = the two files. (5) SHAPE: `python3 claude/harness/shape/delivery.py 9fc9145e WORKTREE` (run from the code root) pasted into the entry — never re-typed; `writers.py WORKTREE AppCommandInterface src/interfaceModule/app_command_interface.hpp src/interfaceModule/app_command_interface.cpp` BEFORE (`HEAD`) → AFTER for `swapCommand`, `loopOpen`, `loopOpener`, `ifSwap`. (6) The entry `claude/INTENT/11.322.md` (checkpoint state in its FIRST LINES, refreshed at each green step): every P1–P10 judged with evidence; P6's reach reading CONFIRMED or REFUTED from the code with the line; the BEFORE / AFTER readings of O; what §5.191's row must say after this (its reach: the one-frame `if` instance, the multi-frame `loop` instance); the README paragraph for F149's section (TEXT proposed — you append it yourself, ONE paragraph: case O, its fixture, its reading). (7) The report, per the definition's form (its first line `prompt-window: present`).

**PREDICTIONS (the supervisor's, from the code read at the mint; judge every line, report every conflict):**
```
PREDICTIONS
P1  | shape     | AppCommandInterface::scriptEnded() = 8 statements in THIS order: ui->keyBinding().endScript(); media->textClear(); media->audioMusicHalt(); media->imageDropAllNoPersistent(); swapCommand = false; ifSwap->reset(); loopOpen = false; loopOpener = ScriptOrigin(); | check: the function pasted | if refuted (another order): the W_END order was not preserved - the reason
P2  | shape     | W_END's branch = `scriptInterface->cancelScript();` alone; `media->textClear();` in app_command_interface.cpp 3 -> 3 (the W_END line leaves, the hook's enters; :2684 / :2709 untouched); `swapCommand = false;` count unchanged | check: /usr/bin/grep -c on both; the diff hunk | if refuted: a line was added beside, not moved
P3  | shape     | UI::init keeps `scriptInterface->cancelScript();` and loses exactly the three media lines: ui.cpp textClear 1 -> 0, audioMusicHalt 1 -> 0, imageDropAllNoPersistent 1 -> 0; numstat ui.cpp 0 / 3 | check: the greps, the numstat | if refuted: UI::init was reworked beyond the copy (out of scope) or a line kept (the second manager survives)
P4  | shape     | net code lines -3 (app_command_interface.cpp +7 / -7, ui.cpp 0 / -3); no header touched; names.py 94 / 93 unchanged; writers.py AppCommandInterface: for swapCommand / loopOpen / loopOpener / ifSwap the writer set swaps `commandScript` for `scriptEnded` (count per field unchanged); the manager count "end-of-execution cleanup" 2 -> 1; comments added 0 (the name carries the what-for) | check: delivery.py's block, writers.py HEAD vs WORKTREE | if refuted (a header hunk): STOP - the shape moved, report
P5  | behaviour | the key leg: `s45_f151_pre --phase after` on 9fc9145e's binary and `s45_f151_post --phase after --baseline s45_f151_pre` on yours: A-N 42 PASS / 0 FAIL both, N identical (timestamps / counters stripped) | check: the two drive.logs pasted | if refuted: a case sees the cleanup's ORDER (text / audio / image cleared relative to endScript) - name the case and the line it reads
P6  | reach     | ScriptMgr::update :306 `while (wait_time==0)` executes a skipped `struct if` block inside ONE update (every skipped command returns wait 0): the row's instance is one frame wide, no key press reaches it; the multi-frame instance is the LOOP state: `struct loop N` sets loopOpen = true / loopOpener (:4756-4757) and the body runs over frames; HOME -> app->init() -> UI::init (app.cpp:636) -> cancelScript resets ScriptMgr's loop state (loopVector, isInLoop, repeatLoop, nbrLoop) but NOT AppCommandInterface's loopOpen / loopOpener; the next natural end (initial.sts, queued by HOME at ui.cpp:2523) runs terminateScript :251-252 -> reportScriptError(loopOpener, MSG_UNCLOSED_LOOP): a FALSE unclosed-loop error attributed to the dead show's `struct loop` line, its `#!` annotation written there | check: the code first (name every writer of loopOpen; name what UI::init's path resets); then P7 | if refuted (the loop state IS reset on the HOME path): the line; the case has no BEFORE red and the change is shape-only - say so and build P7 as a NON-regression case
P7  | behaviour | case O (P6's instance), BEFORE on 9fc9145e's binary: after HOME inside o.sts's loop and initial.sts's natural end, script.log holds one MSG_UNCLOSED_LOOP line attributed to o.sts's `struct loop` line and the fixture carries the `#!` tail; the text / image set up by o.sts are cleared (UI::init's copy does that already); AFTER: no such line, no tail, the media cleared the same | check: O's PASS|FAIL lines both phases, the BEFORE red pasted as the expected sign | if refuted (no error BEFORE): P6 wrong - the resetting line; (the error AFTER): the hook does not run on the HOME path (UI::init's cancelScript does not reach ScriptMgr::cancelScript, or scriptEnded is not called) - STOP, report the path read from the code
P8  | non-change| script_mgr.*, key_binding.*, every header, handleInputs, handleKeyPressed, handleKeys / handleKeysReleased, terminateScript, every draw path untouched; `git diff --stat -- src/` lists exactly app_command_interface.cpp and ui.cpp | check: the stat | if refuted: outside the slice - revert or STOP
P9  | shape     | f70_ascii.py gate PASS; the two files' comment lines unchanged (0 added); one commit's worth: one structural change, the case riding along (11.282(e)) | check: the gate; the diff | if refuted (a comment added): verb-first, one line, only where the name does not carry it - say which
P10 | gate      | no mover, no draw path, no reference law touched: the FULL canary and f131_run.sh are NOT this task's gates (the --no-scene line of the block is); the key leg is | check: stated in the entry | if refuted (a draw-path file in the stat): P8
```

**Boundaries:** WRITES — CODE (2): `src/interfaceModule/app_command_interface.cpp`, `src/uiModule/ui.cpp`; HARNESS: `claude/harness/f142_keys.py` (case O; the fixture writer), `claude/harness/README.md` (ONE paragraph appended inside F149's section), `claude/INTENT/11.322.md`, `claude/harness/artifacts/f142/**`. NOT yours: every header (`names.py` 94 / 93 are a gate), `script_mgr.*`, `key_binding.*`, `f142_keys_run.sh` (unless the runner must pass a flag — then say so), `f124_run.sh`, `INTENT.md`, this file, `SHAPE.md`; the field's script directory is never written (absolute fixture paths); `~/.spacecrafter/config.ini` / `ssystem.ini` md5 in == out. No install, no stash, no commit, no `run_in_background`; launches EXCLUSIVE (`sc_instances.sh --assert` inside the runner; never `pkill -f`; none alive at close). F153 reads the code tree beside you and writes only its entry: do not edit a tracked file it may be reading without the change being the mandate's (it is: two files).

**STOP conditions:** a premise fails; P7's AFTER still reads the error (the hook is not on the HOME path — report the path, mutate nothing further); a case of A–N changes between the pre and the post run (a reading of the cleanup's order — report the case before judging); a change outside WRITES is needed (name the site); the canary red for a reason other than a boot.

**Discriminating checks:** (a) every P line has a verdict with evidence; (b) O's BEFORE red and AFTER green pasted (the `MSG_UNCLOSED_LOOP` line's text and its attributed line); (c) the pre / post drive.logs 42 / 42 and N identical; (d) the greps of P2 / P3 at their counts; (e) `git diff --stat -- src/` = the two files; (f) `f70_ascii.py gate` PASS; (g) delivery.py's block pasted, net code -3; (h) `writers.py` BEFORE / AFTER for the four fields.

**Preconditions (checkable, §0.7):** the PREMISES block (refreshed at dispatch); F149 ACCEPTED (`archive/F149.md`) and its key leg's last accepted record present (`s44_acc_after3`: 42 PASS); the hook at `script_mgr.cpp:127`.

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch, never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => 9fc9145e
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.32[234].md' | tr '\n' ' ' => 
md5sum build-claude/src/spacecrafter | cut -c1-8 => c444d249
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.322.md -a ! -e claude/INTENT/archive/11.322.md && echo free => free
/usr/bin/grep -c '^### F151 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/fable-dispatch/archive/F149.md -a -e claude/fable-dispatch/archive/F142.md && echo accepted => accepted
# the engine slot, the boot's fingerprint (a red here is a STOP to report, never a re-bank by the executor), the field pair, the TCP channel
bash claude/harness/sc_instances.sh --assert f151-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
test ! -e ~/.spacecrafter/beta_features.ini && echo absent => absent
LC_ALL=C /usr/bin/grep -c '^enable_tcp *= *true\|^tcp_port_in *= *7805' ~/.spacecrafter/config.ini => 2
# the two lists and the hook, pinned at their lines (every structural claim of the mandate is a line here)
/usr/bin/grep -n 'argAction==W_END' src/interfaceModule/app_command_interface.cpp => 2842:		if (argAction==W_END) {
sed -n '2842,2851p' src/interfaceModule/app_command_interface.cpp | tr -s '\t' ' ' | tr '\n' ';' => if (argAction==W_END) {; scriptInterface->cancelScript();; media->textClear();; media->audioMusicHalt();; media->imageDropAllNoPersistent();; swapCommand = false;; ifSwap->reset();; loopOpen = false;; loopOpener = ScriptOrigin();; } else if (argAction==W_PLAY && !filen.empty()) {;
sed -n '257,260p' src/interfaceModule/app_command_interface.cpp | tr -s '\t' ' ' | tr '\n' ';' => void AppCommandInterface::scriptEnded();{; ui->keyBinding().endScript();;};
sed -n '247,255p' src/interfaceModule/app_command_interface.cpp | tr -s '\t' ' ' | tr '\n' ';' => int AppCommandInterface::terminateScript();{; for (const ScriptOrigin &opener : ifSwap->openers()); reportScriptError(opener, MSG_UNCLOSED_IF);; if (loopOpen); reportScriptError(loopOpener, MSG_UNCLOSED_LOOP);; unskippable = true;; return executeCommand("script action end");;};
/usr/bin/grep -n 'swapCommand== true || ifSwap->get()==true' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 315
sed -n '259,263p' src/uiModule/ui.cpp | tr -s '\t' ' ' | tr '\n' ';' => scriptInterface->cancelScript();; media->textClear();; media->audioMusicHalt();; media->imageDropAllNoPersistent();;;
/usr/bin/grep -n 'commander->scriptEnded();' src/scriptModule/script_mgr.cpp => 127:	commander->scriptEnded();
/usr/bin/grep -rn 'scriptInterface->cancelScript();' src | cut -d: -f1,2 | tr '\n' ' ' => src/interfaceModule/app_command_interface.cpp:2843 src/uiModule/ui.cpp:259
/usr/bin/grep -rn 'media->audioMusicHalt();' src --include=*.cpp | cut -d: -f1,2 | tr '\n' ' ' => src/interfaceModule/app_command_interface.cpp:2845 src/uiModule/ui.cpp:261
/usr/bin/grep -rn 'media->imageDropAllNoPersistent();' src --include=*.cpp | cut -d: -f1,2 | tr '\n' ' ' => src/interfaceModule/app_command_interface.cpp:2846 src/uiModule/ui.cpp:262
/usr/bin/grep -c 'media->textClear();' src/interfaceModule/app_command_interface.cpp src/uiModule/ui.cpp | tr '\n' ' ' => src/interfaceModule/app_command_interface.cpp:3 src/uiModule/ui.cpp:1
/usr/bin/grep -n 'bool swapCommand;\|IfSwap. ifSwap;\|bool loopOpen = false;\|ScriptOrigin loopOpener;' src/interfaceModule/app_command_interface.hpp | cut -d: -f1 | tr '\n' ' ' => 192 196 199 200
/usr/bin/grep -rn 'scriptEnded\|endScript' src --include=*.cpp --include=*.hpp | cut -d: -f1,2 | tr '\n' ' ' => src/scriptModule/script_mgr.cpp:127 src/interfaceModule/app_command_interface.hpp:66 src/interfaceModule/app_command_interface.cpp:257 src/interfaceModule/app_command_interface.cpp:259 src/uiModule/key_binding.cpp:138 src/uiModule/key_binding.hpp:43
# the reach: a skipped block drains in ONE update; the loop state crosses frames; HOME reaches UI::init through App::init
/usr/bin/grep -n 'while (wait_time==0)' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 306
/usr/bin/grep -n 'MSG_UNCLOSED_LOOP' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 | tr '\n' ' ' => 201 252
/usr/bin/grep -n 'loopOpen = true;' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 4756
/usr/bin/grep -n 'case SDL_SCANCODE_HOME:' src/uiModule/ui.cpp | cut -d: -f1 => 2521
/usr/bin/grep -c 'app->init();' src/uiModule/ui.cpp => 2
/usr/bin/grep -n 'ui->init(conf);' src/appModule/app.cpp | cut -d: -f1 | tr '\n' ' ' => 636 661 663 688
/usr/bin/grep -n '^void App::init()' src/appModule/app.cpp | cut -d: -f1 => 575
# S4: every name the touched headers declare, and Media's (11.265(g)5) - no composition of the three calls exists
python3 claude/harness/shape/names.py src/interfaceModule/app_command_interface.hpp | cut -d: -f1 => app_command_interface.hpp 94
python3 claude/harness/shape/names.py src/uiModule/ui.hpp | cut -d: -f1 => ui.hpp 93
python3 claude/harness/shape/names.py src/mediaModule/media.hpp => media.hpp 83: Media ~Media initVR360 createSC_context drawVR360 drawViewPort faderUpdate disableFader setKeyColor setProjector audioFunction audioVolume audioSetVolume audioVolumeIncrement audioVolumeDecrement audioMusicLoad audioMusicPlay audioMusicMute audioMusicPause audioMusicResume audioMusicRewind audioMusicHalt audioMusicSync audioMusicDrop audioMusicJump audioUpdate audioSetMusicToPause imageLoad imageDrop imageDropAll imageDropAllNoPersistent imageSet imageSetAlpha imageSetScale imageSetRotation imageSetSphericalBaseAltitude imageSetSphericalTopAltitude imageSetPersistent imageSetLocation imageSetRatio imageUpdate imageDraw imageSetKeyColor textAdd textDel textClear textNameUpdate textDisplay textFadingDuration textSetDefaultColor textDraw setTextColor setTextFont buildTextFont resetTextFont updateTextFont strToVideoType setLoop getDualViewport setDualViewport playerUpdate playerPause playerPlay playerStop playerRestart playerJump playerInvertflow playerIsVideoPlayed isVideoCacheFull playerRecordUpdate playerRecordUpdateDependency setRenderFramerate playerSetSpeed playerIncrementSpeed playerGetSpeed playerGetTimeStatus playerSetShowSubtitles playerGetShowSubtitles playerSubtitlesSetProject playerStopped subtitlesLoadFile subtitlesUnloadFile subtitleGetSubtitleAt
# the gate: the key leg as accepted at F149, its last accepted record (the control's reading)
md5sum claude/harness/f142_keys.py claude/harness/f142_keys_run.sh claude/harness/f124_run.sh | cut -c1-8 | tr '\n' ' ' => beafbd80 4a3e8644 1f851de4
ls claude/harness/artifacts/f142/s44_acc_after3/N_reading.txt claude/harness/artifacts/f142/s44_acc_after3/drive.log | wc -l => 2
/usr/bin/grep -c '^PASS' claude/harness/artifacts/f142/s44_acc_after3/drive.log => 42
/usr/bin/grep -c '^## F' claude/harness/README.md => 85
```

**WIP:** 2026-10-13 21:5x — MINTED (round supervisor, Claude Fable 5.1, session 45) on §5.191's row and 11.320 (10); the owner silent since 2026-10-10 (the row's "his to order" read as the sequencing clause; one revert); the reach correction (one-frame `if` / multi-frame `loop`) is the mint's reading of `script_mgr.cpp:306` and `:4756`, to be confirmed by the executor from the code. **DISPATCHED 2026-10-13 22:0x (`opus-xhigh` under the owner's prompt, projection CURRENT at warm-up; gate 42 / 42 at the mint, re-run by the executor as its first step), alone in the code tree, the only engine launcher; F153 beside it read-only; harness `321741f`.**

---

### F152 — OBSERVATION 4 IN ITS KEY FORM (the leg F135 owes, `INTENT/11.311.md` P33): `f131_run.sh 4 <on|off|both> --key [--refuse]` drives SHIFT+KP_6 then SHIFT+KP_8 where the event loop sends a key (F149's door, `UI::handleKeys` on the SHIFT plane) with twenty `Core::updateMove(10)` inside ONE gdb stop and the release through the same door, on the natural scene and the lost race, both pins; `f131_analyse.py` gains the `key` / `refkey` rows (each engine's turn about Saturn in lon and lat, the old observer's altitude HELD — §5.184's second member, predicted gone in the key form); P33's numbers are the predictions, tolerances written before the first launch, every conflict reported with both values; the COMMAND rows (`nat` / `ref`) stay byte-identical in verdict [S–M, **HARNESS ONLY — nothing under `src/`, no build, no commit, no stash**; engine launches EXCLUSIVE while it runs; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): one leg, three files it extends, numbers it is judged against); entry number ALLOCATED: **§11.323**; dispatched AFTER F151's acceptance and commit — the binary it banks on is the round's]

**Why now / mandate:** `INTENT/11.311.md` P33 (F141's structure, 2026-10-10): *"The KEY form of obs 4 (the leg F135 owes, not built here): call $core->moveLon(1) / updateMove(10) x 20 / moveLon(0), natural state both engines ~200000 km above Saturn: 0.44028 deg per call, 8.806 deg over 20, both pins; lost race on the NEW pin: 0.000 deg on both engines AND the old observer keeps 200049.6 km (5.184's second member GONE; its first STAYS as a silent stuck until temp_point goes, 11.306(c)); lost race on the OLD pin: the old observer 0.44036 deg per call, the Camera turns about its zero-radius point | check: the leg once written, both pins, --refuse and without"*. §5.184 (the tester's observation 4) was measured by F131 in COMMAND form — `moveto delta_lon 0.5 duration 0.1` x 20 (`f131_obs.py:173` / `:177`) — because on 2026-09-26 the SHIFT+keypad keys WERE that command (`ui.cpp` sent `moveto delta_l...`); since F137 (`2cb05825`, 2026-10-11) the keys write a HELD order (`core->moveLon(±1)` at press, `:2652` / `:2710`; `core->moveLon(0)` at release, `:3395`) applied by `Core::updateMove(dt)` through the movement control — `moveto delta_l` has 0 hits in `ui.cpp` — so the bank's obs-4 rows no longer exercise what the tester did (Q-84: identical first, and TRANSITIONS enumerated at the dispatch anchor; a steady-state harness cannot see what has no value at rest). 11.299 (3) built the release path; F149 built the door the leg injects through. Nothing measures the key form; P33 predicts it.

**The ORIGIN of the need (S10, the supervisor's):** in session the operator's alphabet is his gestures — the joystick preferred, the keyboard's five planes, no typing (R2 (g), 11.273); an observation he makes on a GESTURE is banked AS the gesture, on both pins, so that every later change on the gesture's path (B42's request table for the place, the joystick row, B44's control) fails loud exactly where he would see it, and so that the engines' divergence under his hand (§5.184: one turns, the other is thrown) is a number with a gate, not a report. What it ends up covering: the four lateral keys (KP_4 / KP_6 lon, KP_2 / KP_8 lat) on the SHIFT plane in the natural scene and the lost race, both pins, the old observer's altitude as a gated member; later the joystick's lateral axes through the same control (F137's `joystickLat` / `joystickLon` — a sample written per frame, a different door: not this slice), the height keys (leg M of F124 already reads the held form), and the key leg's own cases (F149, bindings). NOT this slice: any change under `src/` (a conflict with P33 is reported, never fixed here), `moveto`'s rows (they stay the command form's gate), the `--refuse` mechanism (exists), the race (the `race_*` rows exist).

**Mandate, in THIS order:** (0) §0 warm-up (the canary line in the block is your launch gate: exit 0 at the mint, this boot — a red is a STOP, never a re-bank of yours), the PREMISES gate (`python3 claude/harness/premise_check.py F152`), then READ IN FULL: `claude/SHAPE.md` §5–§6 (the harness is code: one-line verb-first what-fors, pure ASCII, S4 before building — search `harness/` for the primitive first); `INTENT/11.311.md` P24, P30, P32, P33 and (1); `INTENT/11.297.md` (F135: the runner, the analyser's row grammar, the tolerances-before-runs discipline, reading A of PASS); `INTENT/11.292.md` P13 (obs 4 as first measured); `INTENT/11.299.md` (3) (the release path as built) and its F2; `INTENT/11.320.md` (h) and README F149's "What it does" (the injection); §5.184's row; the harness: `f131_run.sh`, `f131_obs.py` (`send` :50, `dump` :55, `gdb_print` :99, `fly_prefix` :120, `fly_refused` :135, `obs4` :169; the `reuse` of `gdb_inject` :42), `f131_analyse.py` (the row table — how a row is keyed by obs / variant / pin, the `4` rows), `f124_run.sh` (header), `f124_legs.py` :120–160 (the bracket: `break Core::updateMove` :126, the calls inside one stop, `set $core = this`), `f142_keys.py` :195–210 (`set var $ui->key_Modifier`, `call $ui->handleKeys(SDL_SCANCODE_...)`; `$ui` taken at `break UI::handleInputs`), `b21_keypath.py` (`gdb_inject`); the code READ-ONLY: `ui.cpp` `handleKeyPressed`'s four mover lines (:2596 / :2652 / :2710 / :2766 — which key on which plane writes which order), `handleKeys` :3396– and `handleKeysReleased` (:3390 / :3395 the stops), `ui.hpp` :78–85 (MODIFIER: SHIFT = 0x08), `core.cpp` `moveLat` / `moveLon` / `updateMove` (the held slot, `velocityScaling(dt)`, the two engines' relative movers). (1) **THE PREDICTIONS FIXED** in the entry BEFORE the first launch: P33's numbers per row with your tolerances (the race's one-frame margin, the frame quantisation; the lat numbers are NOT in P33 — predict them from the law or declare them banked-on-first-run), the lost race's four readings, and which rows are GATES and which NOTES. (2) **THE DRIVE** (`f131_obs.py --key`): in `obs4`, instead of the twenty commands — ONE gdb stop at the entry of a real `Core::updateMove` (F124's bracket), inside it: `set var $ui->key_Modifier = <SHIFT>` (the value READ from `ui.hpp`, as f142 does), `call $ui->handleKeys(SDL_SCANCODE_KP_6, 0, 0, s_gui::S_GUI_PRESSED)`, twenty `call $core->updateMove(10)`, `call $ui->handleKeys(SDL_SCANCODE_KP_6, 0, 0, s_gui::S_GUI_RELEASED)`, `set var $ui->key_Modifier = 0`, resume; `dump("after_lon")`; the same with KP_8 for lat; `$ui` from `break UI::handleInputs` once (F149's method), `$core` from the stop. Compose from `gdb_inject` / the bracket — build no second injector. (3) **THE RUNNER**: `f131_run.sh 4 <pin> --key` and `--refuse --key` (VAR `key` / `refkey`; usage line updated; `all` unchanged). (4) **THE ANALYSER**: rows `4 key <pin> ...` / `4 refkey <pin> ...` — `turn_lon_deg`, `turn_lat_deg` per engine (the existing readers), `other_turn_*`, `old_altitude_held_km` (the old observer's altitude after the lon drive against before: §5.184's second member — a GATE on the ref new row), the scene rows (`scene_reached`, `race_*`, `camera_reference_final`, `old_home_final`, `refusal_logged`) as the nat / ref rows have them; expected = YOUR predictions of step (1) for the first run — a mismatch is a CONFLICT (both values in the entry); then the rows are BANKED with the measured values and the conflict stated beside them (F135's reading A: the bank detects change, the supervisor judges the numbers at acceptance; never widen a tolerance to make a prediction pass). (5) **THE RUNS**: `f131_run.sh 4 both --key`, `f131_run.sh 4 both --refuse --key` (four launches); then `f131_run.sh 4 both` and `4 both --refuse` (four launches): the COMMAND rows' verdicts IDENTICAL to the last accepted run (the analyser changed; the rows must not) — P32's class. About eight launches, each through `f124_run.sh` (`sc_instances.sh --assert` inside it); a lost capture COUNTED (§5.177). (6) README: ONE paragraph appended INSIDE F135's section (`:6734–`, the instrument's home — no new `## F` section); `FORCE_ADD.txt` extended with the key runs' records that must survive; `python3 claude/harness/f70_ascii.py gate` is blind to `claude/` (§5.196) — census your own files by hand: `LC_ALL=C /usr/bin/grep -c -P '[\x80-\xff]' <file>` = 0 each. (7) **DELIVER**: `claude/INTENT/11.323.md` (checkpoint state in its FIRST LINES; the predictions with their clock; every P33 number CONFIRMED / REFUTED (both values) / NOT REACHED (why); the launch ledger; what §5.184's row and H10 must say after this) and the report (its first line `prompt-window: present`).

**Boundaries:** WRITES ONLY: `claude/harness/f131_obs.py`, `claude/harness/f131_run.sh`, `claude/harness/f131_analyse.py`, `claude/harness/README.md` (ONE paragraph inside F135's section), `claude/harness/artifacts/f131/**` (+ `FORCE_ADD.txt`), `claude/INTENT/11.323.md`. NOT yours: `src/` (`git -C /home/claude/spacecrafter diff --stat` EMPTY), `f124_run.sh`, `f124_legs.py`, `f142_keys.py`, `b21_keypath.py` (a defect in any is REPORTED), `INTENT.md`, this file, `SHAPE.md`, the tester's files (md5 unchanged), `~/.spacecrafter/` beyond what a launch writes (field pair in == out). No build, no commit, no install, no stash, no `run_in_background`. Launch discipline: fresh launch per leg, `sc_instances.sh --assert` before each (inside the runner), never `pkill -f`, signal only pids you started, none alive at close.

**STOP conditions:** a premise fails; the canary or the display fails (§11.174(h): report, never mitigate); the SHIFT plane cannot be taken through `handleKeys` under gdb (report the gdb error, stop the leg); the lost race is not reached (`fly_refused` exists — record, continue with the natural rows, report); a criterion cannot discriminate without a change under `src/` (say which; stop that row); a P33 number refuted — NOT a stop: both values in the entry, the row banked measured.

**Discriminating checks:** (a) the predictions' clock precedes the first launch's `meta.txt`; (b) every `key` / `refkey` row's PASS / FAIL / NOTE line pasted, both pins; (c) every P33 number beside its measured value; (d) the four COMMAND runs' verdict lines identical to the last accepted run's (diff pasted); (e) the drive shown as ONE stop (the gdb.log excerpt: the twenty calls between one `F131BEGIN` / `F131END` pair or the bracket's markers); (f) `git -C /home/claude/spacecrafter diff --stat` empty, tester md5s unchanged, field pair in == out, no process of yours alive; (g) the byte census 0 on your three files and the entry.

**Preconditions (checkable, §0.7):** the PREMISES block (the per-round lines REFRESHED at dispatch after F151's acceptance); F135 ACCEPTED (`archive/F135.md`, 11.297); F137 and F149 LANDED (the key path as built: the premises' grep lines); this boot's canary green at the mint.

```
PREMISES
# per-round variables - REFRESH-AT-DISPATCH (this task runs after F151's acceptance and commit: the binary it banks on is the round's)
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => REFRESH-AT-DISPATCH
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.32[234].md' | tr '\n' ' ' => REFRESH-AT-DISPATCH
md5sum build-claude/src/spacecrafter | cut -c1-8 => REFRESH-AT-DISPATCH
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.323.md -a ! -e claude/INTENT/archive/11.323.md && echo free => free
/usr/bin/grep -c '^### F152 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/fable-dispatch/archive/F151.md && echo accepted => REFRESH-AT-DISPATCH
# the engine slot, the boot's fingerprint (a red here is a STOP to report, never a re-bank by the executor), the field pair, the display
bash claude/harness/sc_instances.sh --assert f152-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
test ! -e ~/.spacecrafter/beta_features.ini && echo absent => absent
DISPLAY=:2 LC_ALL=C xdpyinfo 2>/dev/null | /usr/bin/grep -m1 dimensions | tr -s ' ' => dimensions: 2444x1332 pixels (646x352 millimeters)
LC_ALL=C /usr/bin/grep -c '^enable_tcp *= *true\|^tcp_port_in *= *7805' ~/.spacecrafter/config.ini => 2
# the instruments this leg extends, pinned; the COMMAND rows it must leave unchanged
md5sum claude/harness/f131_run.sh claude/harness/f131_obs.py claude/harness/f131_analyse.py claude/harness/f124_run.sh claude/harness/f124_legs.py claude/harness/b21_keypath.py claude/harness/f142_keys.py | cut -c1-8 | tr '\n' ' ' => 027fc459 229f3e69 7ed9e618 1f851de4 27543164 98c32c6a beafbd80
/usr/bin/grep -c '^4 ' claude/harness/f131_analyse.py => 29
/usr/bin/grep -n 'moveto delta_lon 0.5 duration 0.1\|moveto delta_lat 0.5 duration 0.1' claude/harness/f131_obs.py | cut -d: -f1 | tr '\n' ' ' => 173 177
/usr/bin/grep -n '^def obs4\|^def fly_prefix\|^def fly_refused\|^def gdb_print\|^def dump\|^def send' claude/harness/f131_obs.py | cut -d: -f1 | tr '\n' ' ' => 50 55 99 120 135 169
/usr/bin/grep -n 'reuse(HERE + "/b21_keypath.py"' claude/harness/f131_obs.py | cut -d: -f1 => 42
/usr/bin/grep -n 'VAR=nat\|VAR=ref' claude/harness/f131_run.sh | cut -d: -f1 | tr '\n' ' ' => 16 17
/usr/bin/grep -n 'break Core::updateMove' claude/harness/f124_legs.py | cut -d: -f1 | tr '\n' ' ' => 126
/usr/bin/grep -n 'set var $ui->key_Modifier\|call $ui->handleKeys(SDL_SCANCODE_' claude/harness/f142_keys.py | cut -d: -f1 | tr '\n' ' ' => 203 204
test -f claude/harness/artifacts/f131/FORCE_ADD.txt && echo ok => ok
/usr/bin/grep -c '^## F' claude/harness/README.md => 85
/usr/bin/grep -n '^## F135 ' claude/harness/README.md | cut -d: -f1 => 6734
# the key path as built (F137 + F149): the four movers at press, the two stops in handleKeysReleased, the held slot applied by the frame; the command form the keys no longer disguise as
/usr/bin/grep -n 'core->moveLat(\|core->moveLon(' src/uiModule/ui.cpp | tr -s '\t' ' ' | tr '\n' ';' => 2596: core->moveLat(-1);;2652: core->moveLon(-1);;2710: core->moveLon(1);;2766: core->moveLat(1);;3390: core->moveLat(0);;3395: core->moveLon(0);;
/usr/bin/grep -n 'void moveLat(double);\|void moveLon(double);\|void updateMove(int delta_time);' src/coreModule/core.hpp | tr -s '\t' ' ' | tr '\n' ' ' => 294: void moveLat(double); 295: void moveLon(double); 613: void updateMove(int delta_time);
/usr/bin/grep -c 'moveto delta_l' src/uiModule/ui.cpp => 0
sed -n '78,85p' src/uiModule/ui.hpp | tr -s '\t ' ' ' | tr '\n' ' ' => */ enum MODIFIER {  NONE = 0x00,  CTRL = 0x01,  KWIN = 0x02,  SUPER= 0x04,  SHIFT= 0x08,  ALT = 0x10
/usr/bin/grep -n 'handleKeys(E.key.keysym.scancode' src/uiModule/ui.cpp | cut -d: -f1 | tr '\n' ' ' => 901 905
# the predictions' source (11.311 P33) and the rows this leg answers
python3 claude/intent_resolve.py 11.311 11.297 11.292 5.184 11.299 11.320 11.306 2>&1 | /usr/bin/grep -c 'resolves nowhere' => 0
/usr/bin/grep -c '^P33 ' claude/INTENT/11.311.md => 1
/usr/bin/grep -n '^P33 ' claude/INTENT/11.311.md | cut -d: -f1 => 207
/usr/bin/grep -c '0.44028 deg per call' claude/INTENT/11.311.md => 2
md5sum claude/INTENT/11.311.md claude/INTENT/11.297.md | cut -c1-8 | tr '\n' ' ' => f58a634a dff9d0ee
```

**WIP:** 2026-10-13 21:5x — MINTED (round supervisor, Claude Fable 5.1, session 45) on 11.311 P33 and the session-44 note's dispatchable set; the per-round lines REFRESH-AT-DISPATCH by construction (runs after F151's acceptance: the engine slot and the binary are F151's until then). NOT yet dispatched.

---

### F153 — THE JOYSTICK'S BUTTONS AND HAT AS BINDINGS — THE STRUCTURE PREDICTED FROM THE CODE (the joystick row of B49: `INTENT/11.320.md` (10) Q14, `INTENT/11.312.md` (f) *"joystick button -> action | 1 (`JoypadController`, ini) | unchanged | the later row merges it into this registry"*): what `JoypadController` already IS (per button and per hat direction, on two planes — `mode` false / true, the ini's `_ALT` suffix — a native `UI` action with press AND release, or a COMMAND LIST `command("...", "...")` fired on press only, read from `joypad.ini` at init; `isCommand` / `buttonCommand`), what the operator's binding surface needs it to become (a button or hat direction as the `key` token of `script action bind`, press AND release, the three scopes, `-` = the ini's action — the same registry F149 built for the keyboard, or a second key space in it, or the controller's own chains), WHERE the door is (`handleJoyButtonDown` / `Up` / `handleJoyHat` consulting `resolve` first as `handleKeys` does; `mode` as the plane), the token grammar from his declared names (`BUTTON3`, `HAT0_UP`, `_ALT`: S5), the manager count of every information before and after, the key leg's cases for buttons (the `SDL_JoyButtonEvent` injected at the controller's door), a PREDICTIONS block in 11.312 (j)'s grammar, the questions in premise / default / reason form — NO CODE WRITTEN [M, **READ-ONLY on the code tree** (`git -C /home/claude/spacecrafter status --porcelain` EMPTY at close); writes its entry only; executor = agent type `claude` / model `fable` (OPEN-COUPLING under §0b (a): a family's S4 pass over two classes, a shape to choose, the owner's design to model) — run as `fable-asi-max` (session 42's reading of 11.302(j), veto-open); entry number ALLOCATED: **§11.324**; runs BESIDE F151 (disjoint: F151 edits `app_command_interface.cpp` / `ui.cpp` — read them from `git show HEAD:<path>` if you need their text at the round's HEAD)]

**Why now / mandate:** `INTENT/11.320.md` (10): *"The joystick row (Q14): the registry's key is `(SDL_Scancode, plane)` and `resolve` the one entry; buttons and the hat are not scancodes - the row needs a second key space in the same registry (a tagged key) or its own chains, `parse` / `name` for button tokens, and `JoypadController::handle` calling `resolve` for button press / release; `held` serves buttons unchanged; the `ScriptEvent` key string carries the button's token the same way"*; F149's origin (`archive/F149.md`): *"NOT this slice: the joystick's buttons / hat (the same registry later — (c)4 names what exists)"*; row B49: *"the keyboard line's recovery under the joystick ruling §11.304"*; 11.273 (b) [vixy]: *"Joystick navigation is the preferred way ... The binding layers is authorable, the set of hardcoded binding is very low - everything else is generally a one-line script (.sts)"*. The owner's design (11.305) names the KEY; whether a button is "a key" in his sense, and whether the ini's `command(...)` form — already an authorable binding layer, OFF session, for all shows alike — already covers what he needs, is HIS: this task states the question WITH the code's answer and a default, and predicts the shape so that the implementation (if he wants it) is a FOCUSED task. Nothing is built; the entry is the deliverable.

**The ORIGIN of the need (S10, the supervisor's):** one operator in the dark with the joystick in hand (preferred), a public to adapt to, no typing; today a button's meaning is authored OFF session in `joypad.ini` for EVERY show alike (a command list, a native action, an ALT plane); a show that needs a button to mean something for ITS duration (scope `script`), a one-shot (`once`), a release action on a command button (today press-only), or a binding set by the show's own first line, has no way — the keyboard has had all four since F149. What it ends up covering: every button and hat direction on both planes, press and release, the three scopes, the ini's binding as the `-` default (D9: the shipped `default_joypad.ini` and the field's `joypad.ini` keep their meaning, unread by the registry), the recording of a bound button as the key's is recorded, the key leg's cases; later the Masterput tablet's icons if they ever enter as events (11.273: closed-source, not where he works — named, not covered). NOT the axes (continuous: B44's control, 11.304 — not bindings), not the mouse.

**Mandate, in THIS order:** (0) §0 warm-up (read-only form: no binary, display, instance or canary bullet applies), the PREMISES gate (`python3 claude/harness/premise_check.py F153`), then READ IN FULL: `claude/SHAPE.md`; `INTENT/11.305.md` whole (his design and answers); `INTENT/11.312.md` whole — the METHOD you reproduce: (a) origin, (b) the S4 pass, (c) what else the read found, (e) the shape with arguments, (f) the manager table, (g) the first bindings, (h) the cases, (i) files and functions by name, (j) the PREDICTIONS block, (k) contradictions and questions, (l) deviations; `INTENT/11.320.md` whole (what F149 built, its (9) suspensions, its (10)); `INTENT/11.304.md` (the axes' ruling — what a button is NOT); `INTENT/11.273.md` (b)(c); `INTENT/11.299.md` (3) and §5.188's row (the joystick triggers, B44); `DECISIONS_PENDING.md` §17 K; `SCRIPT_SURFACE.md` SS-44 (the tester's question); the code from the current checkout (`git show HEAD:<path>` for the two files F151 edits beside you): `src/uiModule/joypad_controller.hpp` WHOLE, `joypad_controller.cpp` WHOLE (`init` — the ini's parse; `getButtonActionFromString` :338 — the `command(...)` grammar; `handleJoyButtonDown` :201 / `Up` :211 / `handleJoyHat` :238; `mode`), `key_binding.hpp` / `.cpp` WHOLE (`bind` / `resolve` / `endScript` / `parse` / `name`; the table; `held`), `ui.hpp` WHOLE (the joystick members: `handleJoyHat`, `handleJoyAddStick` / `RemoveStick`, `handleDeal`; the `MODIFIER` planes), `ui.cpp` — `handleInputs` :790–920 (the SDL_JOY* events :816–827 → the controller), `handleKeys` :3396– (the door's form: the first statement), `app_command_interface.cpp` the `W_BIND` branch (the key token's parse; the five errors), `script_origin.hpp` (`key`), `EventScript.hpp`, `script.cpp:119` (the token stamp); the data: `data/default_joypad.ini` (shipped: 100 BUTTON / HAT lines, 20 `command(`), `~/.spacecrafter/joypad.ini` (the field's, 153 lines — `LC_ALL=C /usr/bin/grep`, read-only); the harness: `f142_keys.py` (the cases' form and the injection). (1) S10: the origin above, extended by what the read finds (write it first). (2) S4 over `JoypadController` and `KeyBinding`: every name against the intent "a button or hat direction runs a script on press / release under a scope" — which EXISTS (`buttonCommand`: a command list per button — a binding of a different grammar, press only, ini-authored; `onReleaseAction` for natives; `mode` = a plane; `held`'s identity rule for keys), which is a related space to EXTEND (the registry's key space; `parse` / `name`; the door), which is EMPTY (release for commands; scopes; in-session binding). (3) THE SHAPE with arguments, each alternative priced in lines and in managers: (i) the key space — a tagged key in `KeyBinding` (scancode x plane ∪ button x mode ∪ hat x dir x mode) vs. chains inside the controller vs. a template over the key type; (ii) the door — `handleJoyButtonDown` / `Up` / `handleJoyHat` calling `resolve` FIRST as `handleKeys` does, `mode` as the plane; what `-` returns to (the ini's native action or command list — ONE default per (button, plane), as the key's); (iii) the token grammar — his ini's own names ARE declared names (S5): `BUTTON3`, `HAT0_UP`, the `_ALT` suffix for the second plane (or the key token's `ALT+` prefix — his MODIFIER word `ALT` collides with the controller's "alternate": name the collision, propose, do not decide); (iv) the release for a command button — new capability, K-3's identity (the plane at press) for buttons; (v) the recording — a bound button's `bind` line recorded as the key's (names as written, `9fc9145e`); (vi) `ScriptOrigin::key` carrying the button token unchanged; (vii) whether `joypad.ini`'s `command(...)` lists should ever become `bind` lines (NO by default: D9, the shipped file is the product; say why). (4) The manager table (f) BEFORE / AFTER per information: what button B does at event E on plane P · the running execution's end · the identity a button was pressed as · the trigger token · token <-> (button, plane) · the ini's binding. (5) The cases for the key leg (the controller's door injected: `$ui->...` or the controller pointer from `UI`; an `SDL_JoyButtonEvent` built in gdb; the hat as a value change) — 11.312 (h)'s form, A–N's successors. (6) The files and functions BY NAME with ± lines; the predicted `delivery.py` shape (net lines, names added, writers). (7) The PREDICTIONS block, P1–Pn, 11.312 (j)'s grammar (shape / behaviour / non-change, check, if-refuted). (8) The QUESTIONS for the owner in premise / default / reason form (the collision of (iii); whether a button IS a key of 11.305; whether the ini's `command(...)` is to stay the `-` default; the Masterput). (9) The entry `claude/INTENT/11.324.md` (checkpoint state in its FIRST LINES); the report (its first line `prompt-window: present`): what you found that the mandate did not state, every conflict with 11.320 (10)'s reading, the questions.

**Boundaries:** WRITES ONLY: `claude/INTENT/11.324.md`; scratch under `claude/harness/artifacts/f153/**`. NOT yours: anything under `src/` (READ-ONLY; `git -C /home/claude/spacecrafter status --porcelain` EMPTY at your close — F151 edits two files beside you; a dirty tree showing ONLY `app_command_interface.cpp` / `ui.cpp` is F151's, not a premise failure of yours), `INTENT.md`, this file, `SHAPE.md`, `DECISIONS_PENDING.md`, the harness, the field. No launch, no build, no commit, no stash, no `run_in_background`.

**STOP conditions:** a premise fails (the per-round lines; a dirty `src/` BEYOND F151's two files); the controller's door is not where the mandate says (report the actual path before predicting); 11.305's design turns out to exclude non-keyboard events by its wording (quote it, state the question, predict both readings).

**Discriminating checks:** (a) the S4 pass names every method of both classes against the intent (the lists pasted from `names.py`, then your verdict per name); (b) every alternative of (3) priced (lines, managers, files); (c) the manager table with counts; (d) the PREDICTIONS block in the grammar, each line checkable; (e) the questions in premise / default / reason form; (f) `git -C /home/claude/spacecrafter status --porcelain` shows nothing of yours.

**Preconditions (checkable, §0.7):** the PREMISES block; F149 / F142 / F137 ACCEPTED; the records pinned by md5.

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch, never at mint (READ-ONLY task: no binary, no display, no engine line)
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => 9fc9145e
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.32[234].md' | tr '\n' ' ' => 
test ! -e claude/INTENT/11.324.md -a ! -e claude/INTENT/archive/11.324.md && echo free => free
/usr/bin/grep -c '^### F153 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/fable-dispatch/archive/F149.md -a -e claude/fable-dispatch/archive/F142.md -a -e claude/fable-dispatch/archive/F137.md && echo accepted => accepted
# the controller as it stands: a button is a native action or a COMMAND LIST from the ini (press only for commands), an ALT mode, a hat of four directions; it knows nothing of the registry
python3 claude/harness/shape/names.py src/uiModule/joypad_controller.hpp => joypad_controller.hpp 14: void JoypadController ~JoypadController init handle handleDeal getMode setMode handleJoyButtonUp handleJoyButtonDown handleJoyHat getAxisActionFromString getButtonActionFromString isCommand
python3 claude/harness/shape/names.py src/uiModule/key_binding.hpp => key_binding.hpp 5: bind resolve endScript parse name
/usr/bin/grep -n ' buttonCommand; ' src/uiModule/joypad_controller.hpp | cut -d: -f1 => 160
/usr/bin/grep -n 'joy_button_action\* buttonActions;\|joy_button_action \*buttonActions;\|buttonAltActions;' src/uiModule/joypad_controller.hpp | cut -d: -f1 | tr '\n' ' ' => 158 159
/usr/bin/grep -n 'int mode\|mode;' src/uiModule/joypad_controller.hpp | cut -d: -f1 | tr '\n' ' ' => 106
/usr/bin/grep -n '^void JoypadController::handleJoyButtonDown\|^void JoypadController::handleJoyButtonUp\|^void JoypadController::handleJoyHat\|^joy_button_action JoypadController::getButtonActionFromString\|^void JoypadController::handle(' src/uiModule/joypad_controller.cpp | cut -d: -f1 | tr '\n' ' ' => 158 201 211 238 338
/usr/bin/grep -c 'ui->executeCommand(command);' src/uiModule/joypad_controller.cpp => 1
/usr/bin/grep -c 'keyBinding\|KeyBinding\|resolve(' src/uiModule/joypad_controller.cpp => 0
/usr/bin/grep -n 'SDL_JOYBUTTONDOWN\|SDL_JOYBUTTONUP\|SDL_JOYHATMOTION\|SDL_JOYAXISMOTION\|SDL_JOYDEVICEADDED' src/uiModule/ui.cpp | cut -d: -f1 | tr '\n' ' ' => 816 824 825 826 827
/usr/bin/grep -c 'joypad' src/uiModule/ui.cpp => 11
LC_ALL=C /usr/bin/grep -c '^BUTTON[0-9]* *=\|^HAT[0-9]*_[A-Z_]* *=' data/default_joypad.ini => 100
LC_ALL=C /usr/bin/grep -c 'command(' data/default_joypad.ini => 20
LC_ALL=C /usr/bin/grep -c '^BUTTON[0-9]* *=\|^HAT[0-9]*_[A-Z_]* *=' ~/.spacecrafter/joypad.ini => 153
md5sum ~/.spacecrafter/joypad.ini data/default_joypad.ini | cut -c1-8 | tr '\n' ' ' => d7162669 357c47de
# the registry's door and token table as F149 built them; the origin's key
/usr/bin/grep -n 'resolve(' src/uiModule/ui.cpp | cut -d: -f1 | tr '\n' ' ' => 3408
/usr/bin/grep -c 'SDL_SCANCODE_' src/uiModule/key_binding.cpp => 4
/usr/bin/grep -n 'struct ScriptOrigin {' src/scriptModule/script_origin.hpp | cut -d: -f1 => 36
/usr/bin/grep -c 'key' src/scriptModule/script_origin.hpp => 3
/usr/bin/grep -c 'argAction==W_' src/interfaceModule/app_command_interface.cpp => 26
# the records this structure stands on, pinned
python3 claude/intent_resolve.py 11.305 11.312 11.320 11.304 11.273 11.299 5.188 2>&1 | /usr/bin/grep -c 'resolves nowhere' => 0
md5sum claude/INTENT/11.320.md claude/INTENT/11.312.md claude/INTENT/11.305.md claude/INTENT/11.304.md | cut -c1-8 | tr '\n' ' ' => 024ce871 5369db64 e84fbdf0 551eda8c
/usr/bin/grep -c 'joystick row (Q14)' claude/INTENT/11.320.md => 1
/usr/bin/grep -n 'joystick button -> action' claude/INTENT/11.312.md claude/INTENT/11.320.md | cut -d: -f1,2 | tr '\n' ' ' => claude/INTENT/11.312.md:49 claude/INTENT/11.320.md:187
```

**WIP:** 2026-10-13 21:5x — MINTED (round supervisor, Claude Fable 5.1, session 45) on 11.320 (10) Q14 and the session-44 note's dispatchable set ("a structure pass first"); the finding at the mint: the joystick ALREADY has an authorable binding layer (`joypad.ini`'s `command(...)` lists, press only, all shows alike) — the structure task must price what the registry adds over it, and the question whether a button is a KEY of 11.305 is the owner's. **DISPATCHED 2026-10-13 22:0x (`fable-asi-max`, read-only on the code tree, beside F151; gate 31 / 31 at the mint; harness `321741f`).**

---

## 2. Blocked — NOT dispatchable (reason stated so the exclusion is challengeable)

- **B1/S4 + B2 + riding rows** (D4 surface streaming, RING asteroid, INSTANCED
  consumer): Vixy-paced architectural line (EntityCore Taskable authority).
- ~~**B5 remainder** (dso3d/tully, ojmMgr, floors): suspended on the §11.96(e) six +
  §11.98(f) three — chiefly the reach-vs-visibility decoupling, now promotion-grade
  GENERAL (§11.97(c) second domain) and Vixy's call before any large content lands.~~
  **[UNBLOCKED 2026-09-12, §11.233(c) — the owner gave the DIRECTION (dso3d + ojmMgr as
  `ModularBody`, tully a `BodyModule` of the Milky Way, the as-if rule the
  reach/visibility criterion); the remainder is a DESIGN PASS, minted 2026-09-13 as
  F117 (the note; no engine line). The engine slices stay behind his reading of it.]**
- **B30 fix**: tracking-convergence semantics suspended (§11.94(d)).
- ~~**B18 residual**: D15 unanswered (the 2026-07-23 batch's only open item).~~
  **D15 ANSWERED 2026-08-26 and propagated (§11.149(a)) ⇒ B18's suspension closes;
  (c)+(d) are task F38's, (a) dissolved, (b)'s implementation half is §5.80's
  (recorded, not authorized as a whole — see §11.149(b)'s veto point).**
  **[CLOSED 2026-08-26: the veto was ratified (§11.151(a)) and the fix DELIVERED by
  F40 → §11.153; §5.80 is closed, its two riders still open ON the row.]**
- **B10(a)**: anti-stuck floor VALUE = Vixy's feel-test (§11.79(f)).
- **B14 residuals (ii)(v)(vii)**: source-authority order, meridian texture-
  registration, float32 `re.period` — all Vixy's eye (§11.75(c), §11.86(c), §5.25).
- **B8**: waits for old-path removal by definition.
- **B21 step feel**: Vixy's.
- **Every §13.A row**: Vixy/tester territory by protocol.

## 3. For Vixy — sendable/decidable now (not tasks; parallel to any dispatch)

- **Session-44 decision items (2026-10-11, Sunday evening — THE ROUND RUN IN YOUR ABSENCE: F148 landed as `3373ff4f` / `2a0fdbef`, F150 as the shape script, D5 applied; F149 running at this write. Each default below is BUILT unless you say otherwise; one sentence each.)**

  ```
  YOURS — ONE LIST (held set per node; the entry's section in brackets)
  THE REFERENCE DECIDER, LANDED [11.319]:
    H2    MEASURED: your switchToBody holds the view (1e-6 rad) and WARPS the place — 244 AU at the system boundary, 0.99 AU at
          Earth's influence exit in free move (landing 0.05 AU from the Sun): your D15 (b) "transparent" violated in your baseline's
          form (recoverParams consumes the rotation only; position is never re-expressed). BUILT: the hold composed from existing
          entries inside followInfluence (positionRelativeTo -> switchToBody -> placeAt; place <= 2e-4 AU, view <= 1e-5 rad).
          YOURS (5.197): the hold INSIDE switchToBody — one effect, two consumers; free move stops teleporting; followInfluence
          collapses to one call. One word.
    5.198 the old executor's onEnter relocates the Camera to "Sun" on its implicit descent and at every mode jump — unconditionally
          since L3's if (ret) went (your 11.293 (i1)): B42's request table, or your word that onEnter may not address the Camera (S8).
    H8    nothing built above the first system (your three decisions stand). NAMES followInfluence, aimed.
    UNI   f128's UNI leg was never banked (no floor): bank it, on which binary — or not.
  KEY-BOUND SCRIPTS, LANDED [11.320] (your design of 2026-10-10 built as answered; every default one revert):
    NAMES KeyBinding (KeyBindings / KeyOverride), bind / resolve / endScript / parse / name, UI::keyBinding (your getX form:
          getKeyBinding), AppCommandInterface::scriptEnded, ScriptOrigin::key, ScriptEvent::getKey, the words bind key press
          release scope once global script; the key tokens = the SDL_SCANCODE_ suffixes + your MODIFIER identifiers (SHIFT+KP_8).
    Q-A   a taken RELEASE consumes an armed 2-key (native releases never do): built as the structure wrote it; alternative: only
          a taken press consumes it. Q-B a bad bind over TCP reaches a $DIAG subscriber twice (both error forms, Q5).
    Q-D   a release whose press was never seen goes to the native default. REC the recorded bind: the key explicit, the names
          AS WRITTEN (I overrode the executor's resolved absolute paths: a recorded show must replay on another install).
    F4    a release binding REPLACES the native stop (case B: SHIFT+KP_8 stays held after the release) — your (g) rule, for
          the key sheet. F137's F2 alternative (gate the native stop on the plane recorded at press): recorded, not built.
    5.199 a recording captures the engine's own `script action end` (every script run while recording): replaying stops
          there — pre-existing; the tester's stratum for what a recorded show should contain.
  THE SHAPE SCRIPT [11.321]: every acceptance's numbers come from delivery.py now (pasted, never re-typed). FOUND on the way:
    names.py undercounted ssystem_factory.hpp by half (78 of 162) and writers.py AppCommandInterface at 5 of 91 functions since their
    birth — every S4 name list of those files in a PREMISES block before today was blind (the executors read the headers in full:
    bounded). SHAPE section 4's LIBRARY ROW read 34 code lines as comments: 4.8 / 39 % -> 5.2 / 37 %; the 2018 / 2021 corpora of
    11.279-11.281 not re-measured. 5.196: the D14 gate is blind to claude/ — a root + partition for f70's gate (CONVERT the harness
    code, EXCLUDE the ledger): your word on the form. S6: should the verb-first number exist at all.
  THE PROTOCOL: D5 / [Y58] applied at this close by the declared default (section 3, session 43); the census 7.6 had held it for
    your word — the discrepancy is mine, one revert (4feab47 + b728cc8). Section 17's H8 default ("both engines must accept") and
    your 11.293 (i1) pointed opposite ways on L3: the ruling governed (L3 removed); L2 and the WARNING's form stay yours.
  ```

- **Session-43 decision items (2026-10-11, Sunday — THE ROUND YOUR WORD OF THE MORNING OPENED: F140 (step 1 of the fov slice) landed as `559191a8` / `c2d24732` / `a7919278`; F147's census of this file executed (274 524 → 92 579 B at this write); F146 (step 2) landed as `29e93df9` / `35da9c94` / `afae8573`. Each default below is BUILT unless you say otherwise; one sentence each.)**

  ```
  YOURS — ONE LIST (held set per node; the entry's section in brackets)
  THE DISPATCH FILE AND THE LEDGER'S OWN SHAPE [11.318]:
    HOME  the 152 open asks the census found (117 historical ones in the list at the end of this section, tagged [N+] 94 / [N-] 22 / [T] 17 / [D] 19):
          they stay HERE as the live list (default), or move to their own nodes / DECISIONS_PENDING.md (one home, I2) with this section
          holding only the current round's list. One word.
    Y58   §0 step 6 (executors commit, WIP line) contradicts §0b (a) (agents commit nothing; the checkpoint in the entry's first lines): the
          rewrite is drafted (census §7.6); your ratification applies it (+ one re-point in agents/opus-xhigh.md). Default: applied at the next close.
    D3    §11.309(e)'s "every node is answered or defaulted" fails on seven §17 nodes (b42 Q2 / Q6 / Q9, H7, H8, H10, M3) — noted in
          DECISIONS_PENDING §17; the section does not archive.
    D7    G12 (S11) was never applied to the four §5 forks it bears on (§5.116, §5.113, §5.89, §5.87): a mint question for the next round, or your word per row.
  THE FOV SLICE, STEP 1 LANDED [11.316]:
    NAMES `sizeRolloff`, `rotationOf`, `getEnvironmentStarFieldInput`, `getViewportCenter`, `getMvp2D`, `drawWith` — from the module's vocabulary; yours to rename.
    D12   H6's residues: `zoom fov X duration 0` DURING a timed zoom is now absorbed like a key step (the old engine replaces the plan and snaps);
          `CoreLink::setFov` / `loadBackup` give the Camera a 0.5 s timed zoom where the projector snaps (`Camera.hpp:183`'s default). Default: as built.
    CONE  the new path's star search is a hemisphere below fov 180 (the grid search is exact for d <= 0 only; a cos(cullHalfFov) cap dropped faces):
          ~13 280 stars visited on the worker at any fov below 180 where the old visits its pyramid's zones below 90. Alternative: four tangent
          great-circle planes (~8 lines). Default: the clamp (correct, not minimal).
    5.193 the executor's async job starts before the frame's snapshots are built (the star input; the atmosphere's since it was built): the fix is
          `updateExperimental` before `currentMode->update` (which moves Camera::update before Core::updateMove's feed and timeMgr->update) or a
          double buffer. Your word on the order; nothing built.
    5.194 Camera.hpp: zoomTimer / srcHalfFov / dstHalfFov uninitialised (one `-nan` in a dump); calculateZoomCoefVelocity callerless after H6 —
          two Camera.hpp hunks, yours or the next task touching that header.
    HANG  1 of 22 launches under gdb stopped executing commands after `select planet Mars pointer off` (a futex wait, no signal): 5.177's class,
          unattributed; a stack-printing launcher copy exists (harness/artifacts/f140/f121_run_bt.sh). Your stratum if it recurs.
  THE FOV SLICE, STEP 2 LANDED [11.317]:
    NAMES `SkyView` (alternatives `SkyProjection`, `ViewedSky`), `OldSkyView`, `CameraSkyView`, `Core::drawnView`, `Renderer::printGravityPx`,
          `getEnvironmentSkyView`, `getSkyView`, `CameraProjection::projectEye` / `direction`, `Nebula::onScreenSize` — yours to rename; D2', D5,
          the adapter, the pointer value-fed and the navigator-less forwarder BUILT as F145 proposed (each one revert).
    Q1    the projected depth on the new path is 0 (`projectEye`): its two layer consumers (the constellation art's `< 1` test, the pointer's vertex z)
          keep their behaviour; the alternative is the shader law's normalised depth once a sky clip range has a new-path holder. Default: 0.
    S3    O6's scene is `look_at azimuth 180 altitude 25`, landscape off, pure colours — F145's literal zenith scene reads nothing at fov 60 (the equator
          is out of the field): the executor extended the instrument rather than STOP, endorsed; your word if the scene should be another.
    5.195 `look_at` in the equatorial mount aims the two engines 81 deg apart (the Navigator in local alt-az, the Camera in the mount's param frame):
          B42's look request per mode; until then every cross-pin capture under `look_at` in that mount is void.
    Q3    f131's w-rows (the two engines' separation per wheel window) sample at frame phase: their spread on one law reaches their 5 % band — a
          multi-sample bank or an instant-aware row (FOCUSED harness), or notes. Your word.
    LIT   the obliquity / galactic literals now have 3 homes (`projector.hpp`, `skygrid.cpp`, `CameraSkyView.cpp`), the dome matrix 2, the inverse
          radial law 2 (`atmosphere.cpp`): the one-constant dedup commit is next and touches `projector.hpp`, your header — your word on the home.
  THE INSTRUMENTS ON THIS BOOT:
    CANARY the new-path members read 165.260 / 6.647 against the bank's 165.258 / 6.644 (+0.002 / +0.003) in EVERY FULL run since the ninth boot's
           first (2026-10-10 13:51); the old arm exact. Boot-dependent, new path only, pre-existing. A re-bank with the argument (the boot), or a look
           at what the new path reads that the old does not.
    F128   SOL's new-pin stored-pair floor: 4 of 5 runs today 0.000000 (byte-identical to 2026-09-21), 1 at 0.000139 (max 2 grey, 559 px on the band,
           none on a star); the old pin 0.000000–0.000026 in the same five — both exceed the banked floor 0.000021 on this boot. Not re-banked; an A/A
           series (n >= 5) is the instrument's maintenance — your call.
    F131   the w-bank (the two engines' mid-click separation per wheel window) is the pre-H6 law's; F146 re-forms it on its BEFORE run with the argument.
  ```

- **Session-42 decision items (2026-10-10, Saturday — THE ROUND YOUR WORD OPENED, every executor under your prompt: F143 landed as code `9e0ca53c`; F141 / F142 / F144 structures in §11.311–§11.313, four for four accepted; F140 dispatched 18:12 on F143's acceptance; F137 refreshed on F141's block and waits on F140 (shared files). Each default below is BUILT unless you say otherwise; one sentence each.)**

  ```
  YOURS — ONE LIST (held set per node; the entry's section in brackets)
  B44 / F137 before it builds [11.311 §9]:
    N11  the 0-1 normalisation reaches EVERY axis the config binds (your sentence: "the axis"): the eye-relative TURN axes
         read x2.07 at full deflection on the tester's field (the 0.3 band passed at 37.5 % instead of 66 %), the POINTER
         x2.12 (installed) / 5.55x SLOWER than the shipped default - laws untouched. Alternative: only the four place axes
         (keeps SENSITIVITY alive for the rest = two scales for one device). Default: every axis. A feel change he will notice.
    F-4  the anchored lateral's LAW: (i) angle = velocityScaling(dt) / distance from the centre - your one law, a second
         gated reader (0.069309 deg per call at 200 km, dt 10) - or (ii) a fixed angular rate you name. Default (i).
    N6   the held orders as commands: `camera action move_lat|move_lon|raise_height|lower_height value <v>` (v in [-1,1] /
         [0,1]; 0 = stop; missing value = error). The words `camera` and `value` yours. Default yes.
    N1   the control's HOME: your `vzm` + `Core::updateMove` EXTENDED (no class - the class the day the view orders move).
         Names, yours to rename: `moveLat/moveLon`, `joystickLat/Lon/Height`, `vzm.deltaLat/deltaLon/joy*`,
         `drawnVelocityScaling/drawnDistanceFromCenter`. N10: the obsolete AXISn_SENSITIVITY keys logged once per launch,
         actionable (shipped file kept, D9) - or silent. N7/N8/N9 defaults: updateMove's dt; the free lateral's geometry
         stays in the Camera's inline; two slots per axis (held + sampled), clamped sum.
  B49 / F142's implementation, before it is minted [11.312 §k]:
    C1   K-2 "`*` with a name = `*.sts` literally" vs `play`'s rule: an extension-less name is a VARIABLE
         (app_command_interface.cpp:2834). Default: play's rules verbatim (extension required).
    Q2   the key token: prefixes `CTRL SHIFT KWIN SUPER ALT` + the SDL_SCANCODE suffix (`SHIFT+KP_8`), derived from
         SDL's names + <= 20 exceptions. Alternatives: the sheet's letters, or SDL's quoted names.
    Q6   `UI::handleKeysReleased` is DEAD (0 callers), a twin of handleKeys's release switch: delegate to it (-36,
         default) or leave both. The F137 / F142 sequencing over that switch is mine (the second to land dedups).
    C3   = §5.190: the native CTRL+UP release asymmetry (CTRL released first -> the zoom latch never stopped): your
         one-line fix, or left to the registry (which fixes the BOUND keys by construction).
    Q1   the lookup ABOVE the TUI / video gates (your "looked first"): a global binding on a digit fires while the TUI is
         open. Default above. Q5: errors = reportScriptError (the `#!` into the file) AND the failing status - or the status
         alone. K-4's side effects accepted as legacy (the show's wait cut, speed reset, `end` inside ends the show, a paused
         show un-paused without audio). §5.191: UI::init's hand copy of W_END's cleanup misses the if/loop reset.
  F144 / the reference decider, before its implementation [11.313 §i]:
    H2   the RETURN's accommodation: `switchToBody` both ways (position AND view continuous - your free-move swap law).
         My mint's `relocateToBody` reading was REFUTED by the code (it keeps the numbers, warps ~1 AU, holds no view -
         Q-E). Your "(minus the warp)" read as what the old x1e6 mechanism had. Default switchToBody; one word flips it.
    H3   the agent-added rule of `3c87a51d` ("a system node drops the surface bind") GOES: its premise ("re-arms on the
         next body reference") never existed; it is 5.181's mechanism, and kept, the decider would re-create 5.181.
         Residual until the regime slice: bound to Earth's surface at 586 AU follows its rotation.
    H8   THE GALAXY BOUNDARY - a STOP: no node above the first isolated one is maintained (MilkyWay dist / reach read 0
         while inside a system), so "leaving the galaxy's influence" has nothing to read. By the formula, once maintained:
         2 Mpc (128 radii) where the legacy left at 3.2 kpc; the ROOT's AoI is smaller than the galaxy's (non-monotonic).
         Default: no implicit change above the first system, the root excluded. Three decisions: maintain the frame above
         the isolated node (B39 / D23), the galaxy's influence radius, the root.
    H9   = §5.192: galactic.ini x/y/z are PARSECS by the data, "light years" per the loader's message, AU to the tree: every
         stellar system placed inside the Solar system's reach. The unit is the loader's to own (I9); the file's unit = the
         data author's word (the tester's stratum).
    H5 one saved aimed body (the levels = its chain) . H6 `set home_planet` unconditional on the new path (L3 gone) .
    H7 the session does not save `aimed` yet . H4 free-move unchanged, and it re-aims.
  F137 / B44 LANDED (three commits 34202cf2 / bbe278b2 / 2cb05825; 11.299) - what it built on your defaults, each one revert:
    N11  BUILT: the view and pointer axes read x2.125 at full deflection on the tester's field (F141 wrote x2.07 - a slip;
         the view axes share the pointer's deadzone 5000); the 0.3 band is crossed at raw 13330 (was 22700). The place
         sticks x2.067 before the cubic. F-4 reading (i), N6 `camera action ... value`, N7-N10 as F141 predicted.
    F2   K-3 is HALF-built by the predicted shape: the stop does not test SHIFT (built), but a plain KP_2/4/6/8 release
         - whose press ran an M0x.sts script - ALSO writes the stop and kills a show's held `camera action move_lat value 1`.
         The press-identity memory is B49's registry (F142's implementation, same switch). Accept until then, or a word.
    F7   the commands check no value range: `raise_height value -1` lowers. A loud check or none - yours (S11).
    §5.163 / §5.165 STAY (the descent under the datum, the pole): no clamp added without your word; measured numbers in 11.299.
    F1 (harness): a frame hitting an injected breakpoint carries the gdb stall (~3 s) as its dt - F135's KEY leg must apply
         its orders inside the injection. F5: README F124's "two red as found" is stale (green on this binary). F6: the four
         new command words are absent from util/scedit/grammar (SS channel).
  F145 / step 2 of the fov slice, STRUCTURE LANDED [11.315] - yours before F140 re-dispatches:
    D1'  RULED 2026-10-11 (his words §11.314 (i)): TWO deliveries in ONE round, verified together. THE DATUM it answered: step 2 is 43 files (the 14 layer headers + sources, the three executor
         modules above the system, the interface, the adapter, the Camera's view, the Renderer, Core) - both steps = 58
         files, ~+530 / -80. Your unit (one delivery) stands in the code: step 1 alone also misaligns the pointer from the
         star AT REST. My recommendation: TWO deliveries in ONE round, nothing shipped between - your reason (the lines
         must not lag the stars) is a property of what SHIPS; a 58-file Opus run would not survive today's limits.
    NAME the interface: `SkyView` (default - your word for fov + orientation), `SkyProjection` (F139's), `ViewedSky`;
         `OldSkyView` (the adapter), `CameraSkyView` (the new half; `CameraProjection` stays F139's law object),
         `Core::drawnView`, `Renderer::getViewportCenter / getMvp2D / printGravityPx`, `getEnvironmentSkyView`.
    D2'  the Renderer holds the centre AND its sibling, the 2D MVP `printGravity` derives one line below (one holder,
         read by printGravity and the new arm) AND the label law's px entry - else a third ortho2D formula and a second
         label law. If the Renderer may grow by the centre only: say so.
    D5   the GRID (skygrid) answered by the interface with 0 new members: INCLUDE in F140 (the equator line ON the grid's
         equator at rest, within 1.1 px at fov 60) - default; the illuminates' cull NOT (class B). Veto returns the grid
         to the Navigator's frame.
    The old half: ONE adapter {prj, nav} forwarding verbatim (projector.* / navigator.* byte-identical) - default; the
         alternative (two interfaces on your two classes, `final`) is the old engine's seam fixed into the contract.
    The pointer value-fed: `drawPointer(dt, view, earthEquPos, onScreenSize)`, the two Object-API values from the call
         site (your accepted engine form); alternatives: excluded like the images (rejected: rest misalignment), or a
         (view, prj, nav) signature.
    Findings with owners: the UBO's viewport / centre / MVP2D fed from the PROJECTOR in both modes (core.cpp:622-624) -
         step 1's P5; the obliquity and galactic literals in 2 homes (3 after) - a one-constant dedup commit, yours to
         order; the Object API (35 + 72 declarations) = B42's query-table slice; the horizon labels' fixed-180-degree law
         (projector.cpp:589 TODO) reproduced, not fixed; the Observer's x27 reads in the sky lines kept.
  F140 / the fov slice - STOPPED AT READING 18:21, nothing mutated [11.314]; the contradiction is MINE (session 41's mint:
  your two-step unit with a boundary and a block covering step 1 only). Decisions taken under your rulings, veto-open:
    D1   THE DELIVERY UNIT: your unit stands (both steps, star field first) -> step 2 gets its STRUCTURE first (F145,
         Fable, dispatched tonight): the interface's NAME (F139 wrote `SkyProjection`, "his" - not among the five
         ratified), its member list from the eight layers' actual calls, who implements the OLD frame half (the
         Navigator's inline products - `Projector` holds only to-eye matrices, so "Projector implementing it" cannot
         carry the frame half bit-identically), the 14 + 1 + 2 files. ALTERNATIVE, yours: two deliveries -> F140 runs
         as step 1 + P5 + H6 at once (F139 (f6): step 1 alone leaves the constellation LINES lagging the stars).
    D2   THE SCREEN CENTRE: the Renderer holds it beside `getViewportRadius()` (your 11.308(a): "the rendering-related
         informations"); today `printGravity` derives it inline (Renderer.cpp:199-200) and F139's law read the rect.
    D3   THE OLD-PATH GATE re-cut to its criterion: the old pin BIT-IDENTICAL and `projector.cpp` / `navigator.cpp`
         untouched; a header gains only the interface's `: public` / `override` lines; the layer files are retyped
         with the arithmetic verbatim.
    D4   "FOVS PER FRAME" is no canary member (the mint named one): read as the S7 count from the diff + a mid-zoom
         capture; the canary stays the at-rest gate. F145 names the instrument.
    H6-b the Camera's inertia branch (`Camera.cpp:700-721`, constant `delta = 0.5`): your oracle keeps NO velocity on a
         timed re-aim (`zoomTo`, coef = 0) -> default: the branch goes, a re-aim restarts at zero velocity, the Camera
         keeps its easing curve. A feel change in repeated timed zooms.
  F143 / the viewport [11.310]:
    §5.189's reach was NONE (the scissor is square BY CONSTRUCTION - the dome and a desktop window alike): corrected at
    the row and at 11.308(b); the fix stands on the second-formula finding (Q-85). `toPx` / `toScreenSize` are the
    executor's names (no declared name for a length conversion) - yours to rename.
    (h) `ModularObject::getOnScreenSize` returns a RADIUS in px where the old path's notes treat the value as a
    DIAMETER: the 10 % pointer-suppression rule may see half the old value - unverified (what `angularSize` holds); a
    FOCUSED read next round, or your word.
  THE ROUND ITSELF:
    the executors under YOUR prompt, first exercise: every return opened `prompt-window: present`; the fable structure
    tasks ran as `fable-asi-max` (my reading of 11.302(j) for the sections' `claude`/`fable` label; veto-open).
    the SESSION LIMIT cut all four agents at ~14:25 - four agents at max effort exhausted the window in 25 min (reset
    17:40) - each resumed in its own context, quality held (four for four). Your window lost 3 h: sizing a round
    against the wall-clock window is yours to weigh (Q-30 (e)).
    three dispatcher defects of ONE class in one round (mandates stating greppable facts as prose): bound into §0b.3 -
    every structural claim a mandate makes about the code is a PREMISES line (Q-89's update).
    the canary's dwell frame is LAUNCH-dependent on this stack (two states, each on each binary): the md5 is no
    criterion alone (README). The ninth boot re-banked at warm-up; FULL canary 12/12 before and after F143.
  RESIDUE OF THE EARLIER LISTS, extracted here before their bodies leave (archive: forvixy-s40, forvixy-s39):
    session 40 (everything else ruled 2026-10-10, §11.304-§11.309): F-3 per-axis (default stands) . F-5 moot with Q-C's
    dissolution . F-10 the recorder (N5: no per-frame place record) . M3 untouched . b42 Q2 scope, Q6 the mode holder's
    NAME, Q9 dragView's frame . H7 the fov limits . H8 `oldOk || newOk` . H10 the partial moveto (5.184's first member,
    stuck until temp_point goes) . the VETO POINTS of my 11.293 reading (requests keep both writes; shape C; Q4's
    shared-law default excluded by your "two implementations"; the executor split read as a METHOD).
    session 39 (Q-A..Q-D ruled): Q-E whether a change of reference holds the view - F144 reads it as the switchToBody
    law (H2) . TO THE TESTER through you: T1 the halos / lighting (fov, moment, rings), T2 SHIFT+keypad or joypad for
    Saturn (SS-44), T3 `flag_manual_zoom` in his config.
  ```

- **The open asks of the earlier lists (sessions 4 → 40), carried by the census of 2026-10-11 (F147, §11.318) - every other item of those lists is closed at a ledger node, and its record left with its block (`fable-dispatch/archive/forvixy-*.md`; the verdict per item: `fable-dispatch/census-2026-10-11.md`):**

  ```
  YOURS — ONE LIST (held set per node; the entry's section in brackets)
  tags: [N+] the item's own node states the ask · [N-] the node holds only the open state, the ask's text is here alone
        [T] routed to the tester, through you · [D] a default that queued or running work builds unless you rule
  THE VIEW AND THE CAMERA - where it looks, how it moves, what a view command means:
    §11.94(d) B30's fix: latch the view once tracking settles under frozen time (the proposal), or another convergence rule -> INTENT.md:1411 [C01 #4] [N+]
    §5.49 which longitude `moveto lon 0` names: the mesh x-hat column or the map's centre; the owed `u_sub` render measurement first -> INTENT.md:343 [C03 #1] [N-]
    §5.65 a sky lock turned on in the move's frame latches the PREVIOUS frame: lock what the operator now sees, or keep -> INTENT.md:375 [C15 #1] [N+]
    §5.85 `align_with body` aligns nothing: what the command is FOR (its author's or a show's answer) -> INTENT.md:413 [C28 #3] [N+]
    §5.106 what free flight should SHOW (landscape frozen, atmosphere floods); close-as-accepted or keep the design question -> INTENT.md:454 [C34 #4] [N+]
    §5.109 a commanded `moveto alt` stands above the DRAWN surface (today) or the physical one; D21 is silent on the observer -> INTENT.md:457 [C39 #1] [N+]
    §5.135 the OLD path rolls the offset with the heading (the tester saw it): correct the baseline, or let it retire with B8 -> INTENT.md:516 [C52 #12] [N+]
    `Camera::observedToBodyLocalPos` has no in-tree consumer (kept as §5.86's member): keep or delete -> INTENT/11.213.md:59 [C55 #6] [N-]
    a body with NO direction (the zero vector): print it unlike 'at the pole'; a track on it (§5.148): refuse with a line, or a defined aim -> INTENT.md:530 [C56 #7] [N+]
    §5.153 a view duration under 0.2 s: snap, clamp as today, or run at the commanded time -> INTENT.md:535 [C61 #8] [N+]
    §5.156 `flag track_object on`: old snaps, the Camera eases (56-65 deg transient): snap now, smooth later, or a named divergence -> INTENT.md:538 [C63 #2] [N+]
    §5.155 `look_at azimuth`: the two conventions are 107.66 deg apart at rest; which one `look_at` MEANS -> INTENT.md:537 [C63 #3] [N+]
    §11.244(f) an already-accepted divergence in the identical phase: a predicted residual (as applied), or reverted -> INTENT/11.245.md:56 [C63 #4] [N+]
    §11.246(g) the travel's definition is still two computations kept equal (manager count 2): the shape -> INTENT/11.247.md:14 [C64 #4] [N-]
    §5.163 the anchored descent goes under the datum (-12.5 km after three steps): a clamp, or not -> INTENT.md:545 [C65 #2] [N+]
    §5.165 the Camera's latitude passes the pole (-90.5): a clamp, or not -> INTENT.md:547 [C65 #7] [N+]
  WHAT IS DRAWN - regimes, thresholds, textures, light:
    B12 content slice: the star's spots authored or procedural -> b12-design.md:30 [C05 #1] [N+]
    B12 content slice: is a star enterable, and what is seen inside -> b12-design.md:30 [C05 #2] [N+]
    B12 content slice: a resolved star's halo alternates with its surface, or coexists -> b12-design.md:30 [C05 #3] [N+]
    B12 content slice: the chromosphere module's `module =` word (B28, before it is built) -> b12-design.md:30 [C05 #4] [N+]
    A41 the early-visibility gate: 2 px (the constant's name) or 3.07 px (what ships); `RAYMARCH_MIN_SCREEN_SIZE` rides it -> INTENT.md:1386 [C11 #1] [N+]
    A42 the texture-level swap: since R29 the swap DISTANCE and the body list, the tester's -> INTENT.md:1387 [C11 #2] [T]
    A44 the ring's shadow caster is the one SCALED shadow radius: D21 or the 2026-07-18 extent contract -> INTENT.md:1389 [C34 #2] [N+]
    the 7.73 deg old/new attitude divergence on the Moon (two faces drawn): expected or not (rendering paths) -> INTENT/11.167.md:47 [C42 #5] [N+]
    §5.114 every illuminate draws with green and blue exchanged: the one-line fix (user-visible) -> INTENT.md:468 [C44 #5] [N-]
    §5.126 `halo = on`: nothing on the old path, a halo on the new; which predicate is canonical, and should the data surface lowercase -> INTENT.md:500 [C51 #8] [N+]
    §5.129 a portrait window draws the dome (H-W)/2 px low: intended geometry (a tilted dome) or a slip (EntityCore) -> INTENT.md:505 [C51 #17] [N+]
    camera INSIDE a body: new draws nothing, old the body; and `ModularBody::loaded` still a latch (removing it its own decision) -> INTENT/11.248.md:25 [C64 #5] [N-]
    §5.171 bigTexture 256 px and the `0.2` literal (409.6 px) were one threshold and are two: your re-ruling of the two literals (B45 item 3) -> INTENT.md:553 [C65 #5] [N+]
  THE DSO LAYER (B5, the design note `claude/b5-dso-design.md`):
    V-D1 DSO content engine-instantiated or AUTHORED (527 lines of `mode in_galaxy` to migrate either way) -> INTENT/11.240.md:37 [C62 #1] [N+]
    V-D2 the catalogue-to-AU factors: a measurement to ratify (gates slices 2 and 4) -> INTENT/11.240.md:37 [C62 #2] [N+]
    V-D3 `mode in_sandbox`: no node, no behaviour to preserve - its fate -> INTENT/11.240.md:37 [C62 #3] [N+]
    V-D4 Tully's selection after the port (three readings priced) -> INTENT/11.240.md:37 [C62 #4] [N+]
    V-D5 OjmMgr's full-screen depth clear: keep it (faithful) or measure first (correct) -> INTENT/11.240.md:37 [C62 #5] [N+]
    V-D6 the far edge (hard cut or fade), the peg's appearance and the onset ramp - with the tester -> INTENT/11.98.md:11 [C62 #6] [N+]
    V-D7 Dso3d's labels: one state, two operator words - which word -> INTENT/11.240.md:37 [C62 #7] [N+]
    the slices: slice 1 dispatchable once you have read the note; slice 3 before 2 and 4 -> INTENT/11.240.md:42 [C62 #24] [N-]
  ORBITS, BODIES AND THE TREE:
    §5.75 the trail drops a sample on a date jump: rounding or accumulating changes shipped trails' sample counts (policy) -> INTENT.md:395 [C24 #2] [N+]
    the four-body selector (old-path ulps at a pinned clock): leave it unless you want it attributed (an offer) -> INTENT/11.215.md:36 [C55 #5] [N-]
    §5.140 what `orbit_lon` MEANS (planetographic, or today's +90 deg); unblocks §5.21's two halves -> INTENT.md:522 [C56 #5] [T]
    §5.149 the constructor seeds every solver at JD 0: shape (i)-(iv), or none -> INTENT.md:531 [C59 #1] [N+]
    §5.151 Sedna's shipped orbit: three malformed keys in his data (forward-only) -> INTENT.md:533 [C59 #5] [T]
    `isSystemCentered()` answers by POSITION at load time: was a declared flag the intent -> INTENT/11.229.md:52 [C59 #6] [N-]
    §5.154 the trail walker's stale seed: fix shape (1) re-converge after the walk (candidate), (2), (3), or (4) nothing -> INTENT/11.241.md:50 [C62 #9] [N+]
    the `IterativeEll` fold (modelled, unmeasured): its leg before §5.154's fix, or after -> INTENT/11.241.md:56 [C62 #10] [N-]
    §5.159 a body parked under the tree ROOT never gets a position: the root publishes for it, or no fixed point is parked there -> INTENT.md:541 [C64 #3] [N+]
  TIME:
    §5.64 + §5.76 `timerate action pause` does not stop the clock (and slowing a paused clock runs time backward): the pause semantics -> INTENT.md:373 [C11 #4] [N+]
    §5.69 `keep_time` truncated to 8 bits: honour the documented seconds (texture residency changes) -> INTENT.md:383 [C15 #2] [N+]
  THE SCRIPT AND COMMAND SURFACE:
    §5.70 the turn ramp records `look delta_az ...`, a command no channel replays: what it should record -> INTENT.md:385 [C20 #1] [N-]
    §5.87 `select constellation_star` with an unresolved abbreviation acts on the previous selection: unselect, no-op, or today's -> INTENT.md:417 [C30 #6] [N+]
    §5.110 a composed selection answers exactly like nothing-selected: which of the three contracts -> INTENT.md:459 [C42 #1] [N+]
    §5.113 where the truthiness test belongs: each read, a fail-loud singleton, or both -> INTENT.md:466 [C42 #2] [N+]
    `[parallel-script]` (your request): the tester declined it (R25) - withdrawn, or kept as yours -> INTENT/11.207.md:238 [C44 #7] [N+]
    §5.116 the surface cannot say no: refuse, or named default + log (D9 both ways; §5.117 / §5.118 its family) -> INTENT.md:472 [C47 #2] [N+]
    the `#!` generic channel (~1661 tails into 35 shipped scripts): yes, no, or which subset -> INTENT.md:1081 [C48 #4] [N+]
    `[script-trigger]` (`@requires` the binding): triage and order -> FEATURE_REQUESTS.md:195 [C48 #8] [N+]
    nested refusals reach nobody: a nested call inheriting the outer origin (a change to §11.184's rule) -> INTENT/11.188.md:224 [C49 #8] [N+]
    §5.117 severity versus STREAM: a promotion to L_ERROR moves 12 corpus lines to stderr -> INTENT/11.196.md:246 [C50 #8] [N+]
    the `anchor.ini` grammar, a named and absent third contract (`camera` / `flyto` wait on it) -> INTENT/11.200.md:27 [C51 #10] [N-]
    `type = BODY` on every composed node (§11.89(c)'s transitional call): keep or end -> util/scedit/INTENT.md:679 [C51 #11] [N+]
    D37's implementation (a hidden star darkens, ambient by `ambient_light`): when -> INTENT/11.207.md:37 [C52 #13] [N-]
    §5.137 a reloaded authored body leaves the DRAWN universe (inside §11.55(i)'s 'keep current state') -> INTENT.md:519 [C54 #6] [N+]
  THE TCP AND CONTROL CHANNELS:
    §5.72 the `$LOGON` channel promises logs and carries other clients' answers: its semantics -> INTENT.md:389 [C20 #2] [N+]
    the truncation POLICY: whether and how a too-long answer is MARKED -> INTENT.md:391 [C26 #7] [N+]
    the control channel and a script it plays: carry the script's lifecycle at all -> INTENT.md:1067 [C48 #10] [N+]
    ... its own §5 row, or §5.72 + §5.117 read together -> INTENT.md:1067 [C48 #11] [N+]
    ... WHO MAY MOVE THE DOME: the socket authenticates nobody, MCP `run_command` reaches whatever listens -> INTENT.md:1067 [C48 #12] [N+]
  STARTUP, FAULTS, MEMORY AND LOGS:
    A40 a quit while the drawing worker owes a frame: keep waiting, stop waiting (reaches a teardown fault), or fix the fault then stop (+ §5.61) -> INTENT.md:1385 [C06 #1] [N+]
    §5.60 the 2.68 GB staging buffer where a device limit is enforced: the player's sizing / a fallback path (D13) -> INTENT.md:365 [C07 #1] [N+]
    §5.77 / F36 a startup failure on the console twice when `print_log = true`; and the pre-log `out` accumulator -> INTENT.md:399 [C30 #2] [N+]
    §5.88 a missing star catalogue reported nowhere: which of the three contracts -> INTENT.md:418 [C30 #7] [N+]
    §5.89 an unknown `viewing_mode`: abort, named default + log, or refuse to start -> INTENT.md:420 [C30 #8] [N+]
    N7 the config migration writer comments out, never deletes (§5.112's direction): when -> INTENT.md:464 [C52 #14] [N-]
    §5.144 RSS +68 MB/h under the tester's corpus: one 8-12 h leg (cache or leak), yes / no -> INTENT.md:526 [C56 #3] [N+]
    §5.152 a second instance aborts (exit -6): join / detach before the early return, or the normal shutdown -> INTENT.md:534 [C59 #3] [N+]
    EntityCore's uncapped `EntityCore-logs-<epoch>.txt` pile (read-only submodule, his stratum) -> INTENT/11.230.md:29 [C59 #20] [N-]
    §5.142's ROOT: (O2) cap the shadow casters, (O2') the receive array out of the body block, (O1) carve at first draw, (O5) a named refusal -> INTENT/11.236.md:84 [C61 #6] [N+]
    §5.146 SharedBuffer throws (your shape): your one-line EntityCore edit, or dispatched with your submodule push -> INTENT.md:528 [C61 #10] [N+]
  LOCALE AND TEXT:
    §5.121 one static translation table for two locales: the architecture (one map per translator, one translator, an order) -> INTENT.md:489 [C49 #9] [N+]
    F74's field-consequence launch (`set sky_locale zh_CN` on a pre binary): on record or not -> INTENT/11.195.md:223 [C49 #10] [N+]
    §5.119's rider: should `text` route to `media->updateTextFont`, and with which size -> INTENT.md:485 [C49 #13] [N+]
    §5.111's unmatched set: which translation for the body NAME (UI or sky), the type slot, `" (orbit centre)"` -> INTENT.md:461 [C54 #4] [N+]
    §5.136 `app_locale` inert when it differs from `sky_locale`: one map per translator, one translator, or an explicit order -> INTENT.md:518 [C54 #5] [N+]
  SESSION STATE:
    B31 C4: the non-body selection's catalogue key (D34's unanswered half) -> INTENT.md:1408 [C12 #3] [N+]
  TO THE TESTER, THROUGH YOU:
    the oort-SHADOW onset in free mode / Sun-referenced (L2: not tested yet), state-stamped -> INTENT/11.207.md:56 [C24 #4] [T]
    `comet-particles.sts`'s provenance (no generator in the tree) -> SCRIPT_SURFACE.md:392 [C50 #7] [T]
    §5.128 a show setting `zoom_offset` mid-show: does it expect the view to snap back to `init_view_pos` -> INTENT.md:504 [C51 #16] [T]
    R19's other half: does any of his shows depend on today's 999 -> INTENT/11.207.md:231 [C52 #20] [T]
    the `basis/` shows' `script action pause` with `flag_skip_pause = false`: honour or skip under unattended play -> INTENT/11.218.md:72 [C54 #7] [T]
    `doc/superscript.sts:1529` `Ganymed=503` -> `Ganymede=503` (his file) -> INTENT/11.214.md:5 [C55 #8] [T]
    the §11.214(k)(2) sentence on `$body_selected` (his own installation) -> INTENT/11.214.md:46 [C55 #9] [T]
    the new path's nav string on two lines with `" / "` (a readout-format expectation) -> INTENT/11.213.md:60 [C55 #10] [T]
    `fscripts/panorama5.sts:102` a `struct if end` that closes nothing -> INTENT/11.218.md:62 [C56 #9] [T]
    `06old.sts` lands 156 of 170 bodies (13 duplicate names + TDRS 3 with no `coord_func`) -> INTENT.md:1134 [C57 #9] [T]
  THE INTERN, THE REPOSITORY AND THE DEPLOYMENT MAP:
    R23 where the outside content-installation procedure lives, who owns it, may the code repo name it -> INTENT/11.234.md:27 [C52 #3] [N+]
    R6's other half: do pull requests still target `2023-master` -> INTENT/11.232.md:22 [C53 #2] [N+]
    `doc/developer-entry.md` §9 'Engineering principles': the text, or permission to reconstruct it from the ledger's uses -> INTENT/11.234.md:27 [C61 #2] [N+]
    the remote form for the intern's second clone: a key / access, or the HTTPS form -> INTENT/11.234.md:27 [C61 #3] [N+]
    the measurement build (`RelWithDebInfo -O2`) is not the shipped one (`Release -Ofast -O3`): re-point and re-bank, or state it -> INTENT/11.234.md:17 [C61 #4] [N+]
    map Q2: does the DEVELOPER's criterion inherit the TESTER's both-paths exclusion (48 rows carried both ways) -> INTENT/11.243.md:9 [C63 #8] [N+]
    map Q1 (what 'deployment' names), Q3 ('once ready'), Q4 (`R1`'s three meanings, a rename) -> INTENT/11.243.md:8 [C63 #9] [N+]
    map [V1] 732 lines: should the tiers leave the file -> INTENT/11.243.md:53 [C63 #10] [N+]
  THE HOST, THE HARNESS AND THE LEDGER'S OWN SHAPE:
    `f23_b33_control`'s S1 / S2 skylock legs red: the B33 intent (your stratum) before anyone re-baselines -> INTENT/11.171.md:34 [C44 #6] [N+]
    LLM assistance: which mode first, a local-only default, the checker a hard gate in agent mode -> FEATURE_REQUESTS.md:409 [C48 #5] [N+]
    an anchor can resolve while its sentence is false: a content-vs-anchor audit, on your word -> fable-dispatch/archive/F75.md:1 [C50 #3] [N-]
    the one marker call `§11.76 -> §11.75`: a marker at §11.75, or §2.0's span covers it -> INTENT/11.197.md:255 [C50 #10] [N+]
    the malformed marker span at `INTENT/11.192.md:29` (2 `[`, 1 `]`): yours or the next editor's -> INTENT/11.192.md:29 [C50 #11] [N-]
    strict-credit v2's six priced candidates (C1-C6): enact any -> INTENT/11.197.md:301 [C50 #12] [N+]
    `lock-enabled` true on the desktop: one gsettings line -> HOST-EVENTS.md:225 [C51 #2] [N+]
    the 38 `sc-*` scratch trees and logs under /home/claude (sc-f84 4.1 G ... sc-f123): remove or keep -> harness/artifacts/f147/scratch-trees-2026-10-11.txt:1 [C52 #11] [N-]
    every inline §5 row carries its own `§5.N` on its line (a convention) -> INTENT/11.234.md:25 [C61 #5] [N+]
    the canary's coverage: members for release, kernel, toolchain, interpreter, soname + a driver gate (queued): veto to keep it small -> INTENT/11.244.md:13 [C63 #11] [N-]
    Q-83 / Q-84: is 'information maximisation' your name for the generator; is the oracle-spent reason yours -> ~/shared/QUEUE/Q-83.md:77 [C63 #13] [N+]
    the executors' `Co-Authored-By: Claude Fable 5.1` line: one of the 'redundant self co-authors' your tool drops, or kept -> INTENT/11.232.md:10 [C63 #26] [N-]
    `CLAUDE.md` 'not group-writable is a defect' also flags git's own 0444 objects: a rewording -> INTENT/11.264.md:12 [C65 #9] [N+]
    section 0 step 6 (executors commit) against section 0b (a) (no agent commits; the checkpoint in the entry): rule (text proposed in census §7.4) -> INTENT/11.264.md:9 [C65 #10] [N+]
    the three older FIFO runners onto `f124_run.sh`; the premise filler (`harness/artifacts/s34/mint.py`) a tool or evidence -> INTENT/11.264.md:20 [C65 #11] [N+]
  ```
