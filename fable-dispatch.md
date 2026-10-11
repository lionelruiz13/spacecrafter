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

**Update [Claude Fable 5.1 2026-10-11, supervising session 43 — LovelyFoxDev, Sunday morning, the owner present at 08:1x with ONE question (*"Wait, why is the dispatch file that large in the first place ?"*) — THE ROUND HIS WORD OF THE MORNING OPENED (§11.314 (i): the fov slice as two deliveries, verified together); his trigger = the §0b line verbatim]:** Warm-up [measured]: both trees clean at open; code `main` @ `2cb05825` (F137's three commits), harness `93e3303`; binary `57a5a22b` AT HEAD (cmake relinked nothing); `deploy.sh --check` CURRENT; no reboot since the ninth boot (2026-10-10 07:33), the quick canary arm green; `review.sh status` 7 commits waiting at his mark `ee185d4f`. Q-72 struck at the second tool batch (the lost `cd`, instance 4) and again at 10:2x (instance 5) — both recorded in the entry; absolute paths from then on. MINTED at the warm-up (`986c12b`): F140 refreshed as STEP 1 (F139's P1–P25 as amended by F145 (m) + D2's Renderer hunk + H6; 17 files; entry re-allocated §11.316, §11.314 keeps the STOP with forward pointer (j)); F146 as STEP 2 on F145's 28 lines (WRITES derived from BOTH blocks — the 43 step-2 files + the seven step-1 files step 2 also edits, §0b.3's clause applied; ten lines `REFRESH-AT-DISPATCH`); F147 the dispatch file's dead-weight CENSUS on his question (section 3's 66 blocks, 198 848 B = 72 % of the file; the §0 / §0b residue). Mint gate: F140 40 / 40, F147 14 / 14, F146 19 + 10 markers; ONE Q-67 defect of the supervisor caught by the gate (a flag count typed from F145's prose, 2 for 5), corrected from the observed output. DISPATCHED 08:35 in a pair: F140 (`opus-xhigh`, alone in the code tree) + F147 (`opus-xhigh`, harness-only). F147 STOPPED 09:20 on its own criterion — 496 items, 152 CARRIED = 30.6 % (> 20 %) — arguing the census right and the mint's expectation wrong: every CARRIED item is open at its own node, no ruling anywhere, the asks never re-put after 2026-09-19; the STOP LIFTED 09:5x (ladder step 3: the extraction preserves every item, the moves are byte-exact and reversible) and the FOLLOW-UP delivered 09:42 — the live list (152 lines, tags [N+] 94 / [N-] 22 / [T] 17 / [D] 19), the archive plan (76 units; a content-anchored M1 + three passes, simulated and replayed), the to-do. EXECUTED by the supervisor: `fd_archive_pass.py` extended to `forvixy-<slug>` / `index-*` / `protocol-*` (`5c404d8`, the census's D9); M1 `e74f850` (eight one-line forms carrying their reasons — two amended against their bodies —, §1's regenerable sentence, three splits, the 117-line list); passes 30 / 31 / 32 (`81a64ea`, `c28ad19`, `023f753`): the file 307 626 → 118 929 B; D1–D4 propagated (the stale §2 B17 line, the b42 Q3 mark, the §17 note on seven undecided nodes, §11.192's malformed marker); F147 ACCEPTED (`e57b8d0`, pass 33), §11.318. F140 DELIVERED 10:0x and ACCEPTED 10:5x (§11.316): the diff read in full, every gate re-run by the supervisor's hand (canary 12 / 12 twice, f131 5 both, f128 SOL twice, s33, leg Z both pins, census + fovlaw 20 / 20); four P lines refuted (P8 — the S2 form, endorsed; P11 — the grid search's `d <= 0` precondition, a model error of F139's structure; P16's magnitude; H6-P3 as written), one whitespace fix inside the seam hunk, the leg-Z residual rows re-formed to the closed state; COMMITTED as three structural changes on `main`, author Claude Opus 5.5 — `559191a8` (the Renderer's holders) / `c2d24732` (the star field on the Camera's fov and frame) / `a7919278` (H6: the two engines rest 1e-6 deg apart after a key step where they rested 36.3 deg apart); §5.193 (the executor's job starts before the frame's snapshots are built — a race the atmosphere input has had since it was built) and §5.194 (uninitialised zoom members) minted, §5.177 annotated (one hang in 22 launches); pass 34. F146 DISPATCHED 10:50 on F140's acceptance (code `a7919278`, binary `8ac8aedc`; harness `78e7ebc`), its mandate gaining the f131 w-bank re-form (H6 moved the two engines' mid-click separation on BOTH pins — one old-pin FAIL at the band's edge by the supervisor's hand). DATA FOR THE OWNER this session (section 3's session-43 block): the 152 open asks and their HOME; [Y58] / D5; the canary's new-path members +0.002 / +0.003 on this boot in every FULL run since 2026-10-10 13:51 (the old arm exact); `f128 SOL`'s new-pin floor launch-dependent (1 of 5 at 0.000139, the band only); D3 (§11.309(e)'s closure claim fails on seven §17 nodes); D7 (S11 never applied to four §5 forks); H6's residues (D12); the tight cone; the six names. Records: Q-72 instances 4 and 5 (`~/shared/QUEUE/Q-72.md`), Q-110's third instance (the units with no close event), the miss-ledger (the scedit sub-ledger absent from the map). The previous note (session 42) left with this write (pass 35, `update-s42`). **CLOSE (14:3x): F146 cut once by the API window (~11:0x, nothing written), RESUMED 12:22, DELIVERED 14:06 and ACCEPTED 14:3x (§11.317) — three commits `29e93df9` / `35da9c94` / `afae8573` (the third the supervisor's dedup of `rotationOf`, the boundary's defect: `EnvironmentModule.hpp` outside WRITES); O6 reads the pair 1.000 on both pins. THE FOV SLICE IS LANDED: one fov and one frame per new-mode frame for the sky at the Solar level, the old pin bit-identical on every reproducing reading, `projector.*` / `navigator.*` byte-identical. Round total: FIVE deliveries (F147's census and its follow-up, F140, F146), SIX code commits on `main` (F140's three, F146's three), archival passes 30–36, the dispatch file 274 524 B → about 85 KB with one live section (none after this close). `review.sh status`: his mark `ee185d4f`; the module 119 → 124 files, code 13 193 → 13 475, comment ratio 0.07 unchanged; 43 source files outside the header scope changed since the mark. NEXT ROUND'S DISPATCHABLE SET: F142's implementation (B49, on §11.312 — his C1 / Q2 / Q6 words, defaults otherwise); F144's implementation (the reference decider, §11.313 — H2 / H3 / H8; `Camera.cpp` and `ssystem_factory.*` are free now); the one-constant dedup of the obliquity / galactic literals (3 homes — touches `projector.hpp`, his header: his word on the constant's home); §5.193's order fix on his word; the f131 w-rows' form (FOCUSED harness); F135's KEY-form leg; B48's first leg; the small items (the four command words into the scedit grammar, README F124's stale line). Y58 / D5 applies at a close on his word. Sizing: a PAIR per window, the second pair after the first's acceptance (Q-30 (e3)).**

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
6. Abort-tolerance discipline (the reason this file exists):
   - Commit code + harness at **every green checkpoint** (small commits, normal
     trailer discipline: code first, harness carries `Code: <branch> @ <short-sha>`).
   - Update the task's **WIP line** in this file at each checkpoint (one line: date,
     checkpoint reached, next step); clear it at delivery. Commit the WIP update with
     the checkpoint — an uncommitted WIP line protects nothing.
   - Never start a long verification campaign with uncommitted work.
   - Delivery = INTENT §11 entry (file + stub) + §13 row flip + harness commit, as
     usual. An abort before recording ⇒ the successor resumes at the last checkpoint.
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

### F148 — THE CAMERA'S REFERENCE DECIDER, IMPLEMENTED against F144's PREDICTIONS block (30 lines, `INTENT/11.313.md` (h)): in NON-free-move the reference changes only by explicit command, except the influence exception — ONE private method `Camera::followInfluence` run from `update` as the `else` of the free-move walk, ONE member `aimed` (the body the last EXPLICIT change targeted; the implicit climb keeps it), BOTH crossings by `switchToBody` (position and view continuous), the LEAK retired (`changeSystem`'s two writes, `switchToAnchor`'s `if (ret)`), the agent-added system-node unbind rule (`3c87a51d`, four lines) GONE; the galaxy level has NO boundary in the engine (the STOP of (d7): the leave fires for the body → its system step only, P23); the INSTRUMENT of (f) built first and run on the BEFORE binary; f131 observation 1 goes RED by the fix's sign and is RE-FORMED (F137's form); every prediction judged, every deviation reported; RIDER §5.194 (the two `Camera.hpp` hunks the session-43 list gave to "the next task touching that header"), separable [M, **CODE under `src/` within the 4 files below and no other, + harness; the agent may BUILD (alone in the code tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE while it runs (F150 runs beside it in the HARNESS tree only and launches nothing); executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): the block names files, functions, +/- lines, the gates); entry number ALLOCATED: **§11.319**; the supervisor commits in two — the decider, then the rider]

**Why now / mandate:** Q-B closed 2026-10-10 (the owner's law verbatim `INTENT/11.309.md` (a)(f), read (d)); F144 structured it from the code and was ACCEPTED 2026-10-10 (`fable-dispatch/archive/F144.md`): the decider's home by the RA with three rejected homes, one saved `aimed` with the levels derived from its chain, both crossings by `switchToBody` (the mint's `relocateToBody` reading REFUTED by the code — Q-E), the leak list gaining the `3c87a51d` unbind rule (the condition under which observation 1 closes at all — (d5)'s counterfactual), the STOP met above the first system (P23), the as-if table from the F131 dump. Observation 1 of the tester (§5.181, F131 row 1 — `initial_night.sts` ends in twilight with no star on the new pin) is the OLDEST open slice of the tester page (2026-09-26). The owner's defaults — the structure's H1–H10 — stand BUILT unless his word, each one revert: put to him in section 3 (the session-42 block: H2 / H3 / H8) on 2026-10-10 and again in the session-43 note; no word since (he was present 08:1x with one question). You IMPLEMENT the block and report EVERY conflict: H2 `switchToBody` both ways (P13); H3 the four unbind lines go (P3 / P15); H5 one saved body; H6 `set home_planet` unconditional on the new path (P7 / P25); H7 the session does not save `aimed`; H8 no implicit change above the first system, the root excluded (P23). The NAMES are his to give (H1): `followInfluence`, `aimed` — built as F144 proposed, flagged.

**The ORIGIN of the need:** F144 (b), by reference (one operator takes a public from a body's surface up through its system and back by script lines written off session; nothing in session may jump; the Camera's reference is the one thing the whole frame agrees on, 11.265(l)). What it ends up covering: every system level of the tree, every body the operator can aim at, both directions. NOT this slice: the regime laws (the surface / orbit bind — B47's slice; `stall_radius_unit`'s seam stated in (d9)), the view movers, free-move's own rule beyond stating it (H4), the galaxy boundary (H8 — nothing to read in the engine).

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F148`), then READ IN FULL: `claude/SHAPE.md`; `INTENT/11.313.md` whole ((b) the origin, (c) the S4 table, (d1)–(d9) the shape with its arguments and the as-if table, (e) the manager table, (f) the gates and THE INSTRUMENT, (g) the predicted diff and the decider's text, (h) the block, (i) his items, (j) the deviations and what was relied on); `INTENT/11.309.md` (a)(d)(f); `INTENT/11.292.md` row 1 (observation 1; P3(c) the handlers of each script line); `INTENT/11.317.md` (9) (what the sky slice left: the view composes from `camera.getCurrentSystem()` each frame; §5.193 is not yours); `INTENT/11.316.md` (7) and the §5.194 record (`INTENT.md` row §5.194); the code from the current checkout: `src/experimentalModule/Camera.hpp` WHOLE; `Camera.cpp` — the ctor (:22), `switchToBody` (:295), `warpToBody` (:318), `relocateToBody` (:374), `update` (:381–456: the walk at :447–454), `rebindReference`, `dumpTrace` (:911; `"reference"` at :802), `saveSession` / `restoreSession`, `setBoundToSurface`, `setFreeMode`, `setHalfFov` and the zoom members' readers (:85–88, :225–235 — the rider); `ModularBody.hpp:800–950` (`findBetterReference` :812, `isInAreaOfInfluence` :882, `isSystem` :940, `getParent`, `useNow`, `getAreaOfInfluence`) and `ModularBody.cpp`'s `updateReach` (~:401–416), `dispatchUpdate` (~:278–300), `calculateSwitchCompensation`; `ModularBodyPtr.hpp`; `src/bodyModule/ssystem_factory.cpp:240–285` (`changeSystem` :257, the two writes :261 / :268, `enterSystem` / `leaveSystem` ~:613–628) and `.hpp:640–740` (`cameraSwitchToAnchor` :655–658 — L2, NOT yours —, `switchToAnchor` x2 :727–738, `syncCameraReference`); `CameraAnchors.{hpp,cpp}` read-only (the explicit entries' callers: `switchTo`, `transitionToBody`, `setFollowRotation`); `SessionFile.cpp:430–450`; `core.cpp` `setHomePlanet`; `solarSystemModule.cpp:50–80`, `inGalaxyModule.cpp:45–65` (the OLD oracle, unchanged by construction); the harness: `f124_run.sh` (its header: `./f124_run.sh <label> <driver.py> [args]`, `F124_OUT_ROOT`), `b3_ladder.py` (the FIFO ladder form), `f131_obs.py` (`dump()` :55 — `body action dual_dump filename <p>`; the scene set-up), `f131_run.sh`, `f131_analyse.py` rows 31–50 (obs 1's bank), `f128_leg.sh`, `f121_run.sh` + `s33_refchange.py`, `f56_canary.sh`, `sc_instances.sh`; the dump `harness/artifacts/f131/keep/O1_new_late.json.gz` (gzip JSON — the body set the as-if table was transcribed from; read by Python). (1) The S4 pass from the names in the block (Camera 79, CameraAnchors 29, `ssystem_factory.hpp` 80 words): a decider for non-free-move (none — the premise), the saved aimed body (none), the three entries (consume), `isInAreaOfInfluence` / `isSystem` / `getParent` / `system` (consume) — each said; S9 on the unbind rule: `git log -S'setBoundToSurface(false);' --oneline -- src/experimentalModule/Camera.cpp` names `3c87a51d` and its comment's premise ("it re-arms on the next body reference") — verify at reading that no writer re-arms it (5.181's list: `loadCamera`, the declared anchors, the session restore write `true` explicitly; none on a body reference). (2) BUILD THE INSTRUMENT FIRST ((f)'s last bullet): `claude/harness/f148_cross.py` (the driver, in `b3_ladder.py` / `f131_obs.py`'s form: new pin, landed on Earth at the field's place, `timerate rate 0`; a dump after each of `moveto altitude 8.0E+13 duration 0` (534.8 AU — P12), `9.0E+13` (601.6 — P11), `8.5E+13` (568.3 — P13), then `set home_planet Mars` at 601.6 AU with two dumps two frames apart and a return at `8.5E+13` (P14), then `9.0E+13 duration 3` from landed (P26: dumps during and after; the ring's `posDelta`), then `1.5E+17` and `1.5E+19` (P23), then P28's two bisections (8.5E+13 .. 9.0E+13 halved six times; `body name Pluto hidden false` and the second), then P29's two 300-frame parks at `8.9E+13` / `8.7E+13`; reads `camera.reference`, `camera.aimed` (P24 — on the BEFORE binary the field is ABSENT: read "absent"), `boundToSurface`, `rootPos`, `absFwd`, `freeMode`, `system` from each dump; computes the system node's AoI from the dump's visible bodies by the (d8) transcription, NEVER pasted — on the run's date it is Neptune's distance x 19.4 ≈ 586 AU; prints one `PASS|FAIL|NOTE P<n> <criterion> observed=<v> expected=<v> tol=<t>` line per criterion, exit 1 on any FAIL) + `claude/harness/f148_run.sh` over `f124_run.sh` (`F124_OUT_ROOT=claude/harness/artifacts/f148`, the instance assert inside). RUN IT on the BEFORE binary `8a5d8fcf` — predicted: the reference stays Earth at every rung (no decider exists; the leak fires only through the old executor's level change at 1e16 m = 66 846 AU, far above the ladder), `set home_planet Mars` relocates to Mars and nothing returns, `aimed` absent — paste. Then the BEFORE gates: `bash claude/harness/f131_run.sh 1 both` (the twenty bank rows PASS per pin as banked; paste), `bash claude/harness/f128_leg.sh SOL --phase run`, `bash claude/harness/f121_run.sh <label> claude/harness/s33_refchange.py` (ALL PASS), the FULL canary (`f56_canary.sh`, 12 / 12, old delta 0.0). (3) BUILD F144's block in (g)'s order — ONE STRUCTURAL CHANGE: `Camera.hpp` (the member `ModularBodyPtr aimed;` beside `reference` :258; the private `void followInfluence();` with ONE verb-first what-for; `rebindReference` gaining `aimed = dst;`), `Camera.cpp` (the ctor's init list `aimed(reference)`; `switchToBody` and `warpToBody` each gaining `aimed = dst;` and LOSING their `if (dst->isSystem()) setBoundToSurface(false);` pair at :314–315 and :324–325 — P3; `update`'s walk at :449–454 becoming `if (freeMode) { ... } else { followInfluence(); }` in place — P5; `followInfluence` as (g) writes it (≤ 18 lines; it calls only `switchToBody`, `isSystem`, `isInAreaOfInfluence`, `getParent` and the member `system`; it re-asserts `aimed` across its own switch — P4); `dumpTrace` gaining `"aimed"` beside `"reference"` (:802) — P24), `ssystem_factory.cpp` (`changeSystem` losing its two `camera->switchToBody(...)` lines :261 / :268; `currentSystem` and `selectSystem()` stay — P6), `ssystem_factory.hpp` (`switchToAnchor(const std::string&)` :727–732 calling `syncCameraReference(anchorName)` unconditionally — the `if (ret)` :729 goes — P7; `cameraSwitchToAnchor`'s `if (oldOk && !newOk)` :657 UNTOUCHED — L2, slice 4's). THEN THE RIDER §5.194, its own hunks, separable (`git diff -- src/experimentalModule/Camera.hpp` must read as two groups; the supervisor commits them apart): `zoomTimer`, `srcHalfFov`, `dstHalfFov` (`Camera.hpp:290–292`) get default member initialisers to the rest state a finished plan leaves them in — read the ctor and `setHalfFov`'s plan start / end to NAME the values (if the ctor already writes them on a path you can show, say so and change nothing); `calculateZoomCoefVelocity` (`Camera.hpp:234`; 1 hit in `src/` = its definition) DELETED (S2: callerless since H6). Build (`free -g` first: available ≥ 16 GiB ⇒ the SessionStart hook's safe `-j`, else `-j6`), `python3 claude/harness/f70_ascii.py gate`, a checkpoint line in your entry. (4) GATES AFTER, both pins, by your own hand: the instrument (P11–P14, P23, P25, P26, P28, P29 — a PASS / FAIL line each; P25 = `set home_planet SolarSystem` from Earth: the node becomes the reference, `aimed == reference`, no WARNING from `syncCameraReference` in the engine log); `f131_run.sh 1 both` — P15 / P16: exit 1 with exactly SIX RED gate rows (new pin: `star_points_late`, `stars_in_drawn_buffer_late`, `observers_apart_late_km`, `observers_view_apart_late_deg`; old pin: the two ring rows) and `camera_bound_late` `true` on both — THE FIX'S SIGN; paste every line; then RE-FORM the obs-1 bank (`f131_analyse.py` rows 31–50) to the AFTER state WITH THE ARGUMENT (F137's form — the fix is the argument; never a widened tolerance): rows 32 / 33 to the observed residuals (predicted < 1 deg / < 20 km — bank the value you observe with a tolerance from the two pins' spread, stated), rows 34 / 36 to the old pin's class (565 ± 28 / 3320 ± 166 — the new pin now draws the stars), row 49 `true`, the notes 42–47 to the observed values; re-run `f131_run.sh 1 both` → exit 0, paste; `f131_run.sh 2 both`, `3 both`, `4 both`, `2 both --refuse`, `3 both --refuse`, `4 both --refuse`, `5 both`, `6 both` — every row as the bank (P17; the known `2ref new mw_band_end_over_pre` FAIL; the w-rows' launch-to-launch sampling spread (README F131, observation 5's note) read, never re-banked); `f128_leg.sh SOL --phase run`, `GAL`, `UNI`, `TILT` (P18 — every line as today; the legs climb in free mode); `f121_run.sh <label> s33_refchange.py` ALL PASS (P19); the FULL canary (P20: new 165.26 / 6.647 on this boot, old delta 0.0); P21 / P22 read from the O1 ring (`python3 claude/harness/f131_analyse.py ring ...`); P27 NOT REACHED (after the supervisor's commit); P8 is the SUPERVISOR's (`writers.py` takes a rev and you commit nothing — report the writer sets from your reading of the diff). (5) Shape: `git diff --numstat` per file against (g) (+27 / −9, ± 4 per file; the rider's hunks counted apart); `names.py Camera.hpp` = 80 before the rider's deletion, 79 after (+ `followInfluence`, − `calculateZoomCoefVelocity`); `/usr/bin/grep -c 'dst->isSystem()' src/experimentalModule/Camera.cpp` = 0; `camera->switchToBody` in the factory's `.cpp` = 0; `if (ret)` in `switchToAnchor` = 0; the one new comment ONE line, verb first; no ledger grammar in code (`INTENT\|11\.3\|F148\|\[observed` = 0 in the four files); the manager table (e) re-derived on the diff. (6) The entry `claude/INTENT/11.319.md` (checkpoint state in its FIRST LINES, refreshed after each step): every P1–P30 judged with evidence; every deviation with its argument; the BEFORE / AFTER readings on both pins; the galaxy STOP recorded as P23's verdict (H8 — the owner's; nothing built above the first system); the as-if at the crossings (the LEVEL sequence and the continuity — never the metre); the names flagged; the rider's hunks named with the values chosen and why; what the regime slice (the bind law — B47's slice; `stall_radius_unit`'s seam, (d9)) and B42 must know. (7) The report, per the definition's form (its first line `prompt-window: present`).

**REFRESHED AT MINT (2026-10-11 17:5x, session 44, on `afae8573` — what moved since the structure's `38ec7199`):** (i) F143 (`9e0ca53c`), F137 (`34202cf2` / `bbe278b2` / `2cb05825`), F140 (`559191a8` / `c2d24732` / `a7919278`) and F146 (`29e93df9` / `35da9c94` / `afae8573`) LANDED: `Camera.cpp`'s anchors are as the block reads them (`switchToBody` :295, `warpToBody` :318, `relocateToBody` :374, `update` :381, the walk :447–454 UNCHANGED; the two unbind pairs at :314–315 / :324–325 UNCHANGED) but `dumpTrace` is at :911 and `"reference"` at :802 (the structure read 938–978: H6 removed the Camera's inertia branch); the factory's `changeSystem` is :257 with the two writes :261 / :268 (were :260 / :267), the `if (ret)` :729 (was :722–724), `oldOk && !newOk` :657; `ssystem_factory.hpp` gained `getEnvironmentSkyView` (80 words). (ii) `Camera::update` runs in BOTH modes from `SSystemFactory::updateExperimental` (P16's premise; `executor.cpp:61` at the structure's read) — verify at reading, before the old-pin runs. (iii) §5.193 (the executor's async job starts before the frame's snapshots) is NOT yours and is not to be mitigated; your decider runs inside `Camera::update`, so a leave or a return changes `reference` before `EnvironmentManager::update` reads `getCurrentSystem()` — say in the entry whether a body → system switch changes `system` (by the walk's definition the first isolated node above a body of the system IS the system node — unchanged; the dump's `system` field shows it). (iv) The canary's new-path members read 165.260 / 6.647 on this boot (+0.002 / +0.003 against the bank, pre-existing) — P20 is judged against those digits; `f128 SOL`'s new-pin floor is launch-dependent on this boot (1 of 5 at rel 0.000139) — a red there is read with a second run, never re-banked. (v) `~/.spacecrafter/scripts/internal/initial_night.sts` md5 `78887972` = the tester's copy (the block). (vi) The ONE structural change and the rider are SEPARABLE diffs — keep them so; you commit nothing, the supervisor commits in two. (vii) The names of F148's harness files are this task's (`f148_cross.py`, `f148_run.sh`); the obs-1 bank re-form edits `f131_analyse.py` rows 31–50 ONLY. (viii) F150 runs beside you in the HARNESS tree (`harness/shape/delivery.py`, `writers.py`'s `WORKTREE` form, `INTENT/11.321.md`) and reads the code tree read-only; it writes NO README section — you write yours (`## F148 - ...`, appended, the file's form); one writer per file holds.

**Boundaries:** WRITES — CODE: `src/experimentalModule/Camera.hpp`, `src/experimentalModule/Camera.cpp`, `src/bodyModule/ssystem_factory.cpp`, `src/bodyModule/ssystem_factory.hpp` (the 4 files — P1); HARNESS: `claude/harness/f148_cross.py`, `claude/harness/f148_run.sh` (new), `claude/harness/f131_analyse.py` (rows 31–50, the obs-1 bank ONLY), `claude/harness/README.md` (ONE section appended), `claude/INTENT/11.319.md`, `claude/harness/artifacts/f148/**`, the runners' own records under `artifacts/f131/runs/`, `artifacts/f128/`, `artifacts/f124/`. NON-CHANGE, a gate (P9): `ModularBody.*`, `ModularSystem.*`, `CameraAnchors.*`, `EnvironmentManager.*`, `src/executorModule/*`, `src/interfaceModule/*`, `src/coreModule/*`, `src/navModule/*`, `SessionFile.*`, every shader; `cameraSwitchToAnchor`'s L2 conditional. No install, no stash, no commit, no `run_in_background`; launches EXCLUSIVE (`sc_instances.sh --assert` before each; never `pkill -f`; none alive at close).

**STOP conditions:** a premise fails; the BEFORE instrument shows a reference change at any rung (a path to the Camera the structure did not read — name it); P11's switch moves `rootPos` by more than 1e-6 AU or turns `absFwd` by more than 1e-5 (a body → system `switchToBody` does not hold position and view: (d4)'s reading is wrong — STOP, H2 to the owner); P3's deletion moves a gate OTHER than obs 1 (the bind kept `true` on a system node reaches something — name it); the canary red for a reason other than a boot; the instance probe non-zero (record, wait it out, launch fresh).

**Discriminating checks:** (a) every P line has a verdict with evidence; (b) the instrument's BEFORE / AFTER lines; (c) f131 obs 1: six RED by the fix's sign, then re-formed to exit 0, both pins; (d) the four non-change greps (`dst->isSystem()` 0, `camera->switchToBody` 0 in the factory, `if (ret)` 0 in `switchToAnchor`, `names.py Camera.hpp` 79 after the rider); (e) `git diff --stat -- src/` lists exactly the four files; (f) `f70_ascii.py gate` PASS; (g) the manager table (e) re-derived; (h) the rider's hunks separable.

**Preconditions (checkable, §0.7):** the PREMISES block; F144 ACCEPTED (`fable-dispatch/archive/F144.md`); F143 / F137 / F140 / F146 LANDED (the HEAD line); F150 beside you writes nothing under `src/` and launches nothing.

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch, never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => afae8573
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.319.md\|INTENT/11.321.md\|harness/shape/delivery.py' | tr '\n' ' ' => 
md5sum build-claude/src/spacecrafter | cut -c1-8 => 8a5d8fcf
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.319.md -a ! -e claude/INTENT/archive/11.319.md && echo free => free
/usr/bin/grep -c '^### F148 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
# the engine slot, the boot's fingerprint (ninth boot; a red here is a STOP to report, never a re-bank by the executor), the field pair
bash claude/harness/sc_instances.sh --assert f148-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
test ! -e ~/.spacecrafter/beta_features.ini && echo absent => absent
# the structure this task implements, pinned (F144's 30 lines, INTENT/11.313.md); its acceptance record
md5sum claude/INTENT/11.313.md | cut -c1-8 => 2308df94
sed -n '/^PREDICTIONS F144 -> its implementation/,/^P30 /p' claude/INTENT/11.313.md | md5sum | cut -c1-8 => 7d9e7acf
sed -n '/^PREDICTIONS F144 -> its implementation/,/^P30 /p' claude/INTENT/11.313.md | /usr/bin/grep -c '^P[0-9]' => 30
test -e claude/fable-dispatch/archive/F144.md -a -e claude/fable-dispatch/archive/F143.md && echo accepted => accepted
# the Camera at HEAD: the three entries (his names), the free-move walk, the two bind-rule pairs that GO (P3), no aimed / followInfluence yet
/usr/bin/grep -n 'void switchToBody(ModularBody \*dst);\|void warpToBody(ModularBody \*dst);\|void relocateToBody(ModularBody \*dst);' src/experimentalModule/Camera.hpp | tr '\n' ' ' => 37:    void switchToBody(ModularBody *dst); 39:    void warpToBody(ModularBody *dst); 41:    void relocateToBody(ModularBody *dst);
/usr/bin/grep -n '^void Camera::switchToBody\|^void Camera::warpToBody\|^void Camera::relocateToBody\|^void Camera::update\|^void Camera::dumpTrace\|^Camera::Camera' src/experimentalModule/Camera.cpp | tr '\n' ' ' => 22:Camera::Camera(ModularBody *reference, float longitude, float latitude, float altitude) : 295:void Camera::switchToBody(ModularBody *dst) 318:void Camera::warpToBody(ModularBody *dst) 374:void Camera::relocateToBody(ModularBody *dst) 381:void Camera::update(double jd, float deltaTime) 911:void Camera::dumpTrace(std::ostream &out) const
/usr/bin/grep -n 'if (auto newRef = reference->findBetterReference(position.length()))' src/experimentalModule/Camera.cpp => 452:        if (auto newRef = reference->findBetterReference(position.length()))
/usr/bin/grep -n 'system = ModularBody::dispatchUpdate(reference, jd, mat);' src/experimentalModule/Camera.cpp => 447:    system = ModularBody::dispatchUpdate(reference, jd, mat);
/usr/bin/grep -n 'dst->isSystem()' src/experimentalModule/Camera.cpp | tr '\n' ' ' => 314:    if (dst->isSystem()) 324:    if (dst->isSystem()) // same no-surface rule as switchToBody
/usr/bin/grep -c 'aimed\|followInfluence' src/experimentalModule/Camera.hpp src/experimentalModule/Camera.cpp | tr '\n' ' ' => src/experimentalModule/Camera.hpp:0 src/experimentalModule/Camera.cpp:0
/usr/bin/grep -n 'ModularBodyPtr reference;\|ModularBodyPtr target;' src/experimentalModule/Camera.hpp | tr '\n' ' ' => 258:    ModularBodyPtr reference; 259:    ModularBodyPtr target;
/usr/bin/grep -n 'out.appendEntry("reference"' src/experimentalModule/Camera.cpp => 802:    out.appendEntry("reference", reference ? reference->getEnglishName() : "");
# the tree's primitives the decider consumes (S5), unchanged since the structure
/usr/bin/grep -n 'inline bool isInAreaOfInfluence() const {\|inline bool isSystem() const {\|inline ModularBody \*findBetterReference(const float observerDistance) {' src/experimentalModule/ModularBody.hpp | tr '\n' ' ' => 812:    inline ModularBody *findBetterReference(const float observerDistance) { 882:    inline bool isInAreaOfInfluence() const { 940:    inline bool isSystem() const {
# the leak (5.181) and its conditional, at HEAD (lines moved since the structure: 260/267 -> 261/268, 722 -> 729)
/usr/bin/grep -n 'camera->switchToBody' src/bodyModule/ssystem_factory.cpp | tr '\n' ' ' => 261:        camera->switchToBody(ModularBody::findBodyOnce("SolarSystem")); 268:        camera->switchToBody(ModularBody::findBodyOnce(mode + "System"));
/usr/bin/grep -n '^void SSystemFactory::changeSystem' src/bodyModule/ssystem_factory.cpp => 257:void SSystemFactory::changeSystem(const std::string &mode)
/usr/bin/grep -n 'if (ret)' src/bodyModule/ssystem_factory.hpp => 729:        if (ret)
/usr/bin/grep -n 'bool switchToAnchor(const std::string& anchorName) {' src/bodyModule/ssystem_factory.hpp => 727:    bool switchToAnchor(const std::string& anchorName) {
/usr/bin/grep -n 'if (oldOk && !newOk)' src/bodyModule/ssystem_factory.hpp => 657:		if (oldOk && !newOk)
# the rider 5.194 (Camera.hpp hunks the session-43 list gave to the next task touching that header): three uninitialised members, one callerless inline
/usr/bin/grep -n '^    float zoomTimer;\|^    float srcHalfFov;\|^    float dstHalfFov;' src/experimentalModule/Camera.hpp | tr '\n' ' ' => 290:    float zoomTimer; 291:    float srcHalfFov; 292:    float dstHalfFov;
/usr/bin/grep -rn 'calculateZoomCoefVelocity' src/ | wc -l => 1
# the writer sets at HEAD (writers.py, the S7 count the acceptance compares against)
python3 claude/harness/shape/writers.py HEAD Camera src/experimentalModule/Camera.hpp src/experimentalModule/Camera.cpp | /usr/bin/grep '  reference  \|  boundToSurface  ' | tr -s ' ' | tr '\n' ';' => 3 reference rebindReference switchToBody warpToBody; 1 boundToSurface setBoundToSurface;
# the gates that judge this row: f131 obs 1's twenty bank rows (the four + two that must go RED, P15 / P16), the dump the as-if table was read from, the legs
/usr/bin/grep -c '^1   nat' claude/harness/f131_analyse.py => 20
/usr/bin/grep -n '^1   nat new  star_points_late\|^1   nat new  stars_in_drawn_buffer_late\|^1   nat both observers_apart_late_km\|^1   nat both observers_view_apart_late_deg\|^1   nat both camera_bound_late' claude/harness/f131_analyse.py | tr -s ' ' | tr '\n' ';' => 32:1 nat both observers_view_apart_late_deg gate earth_view 12.0602 0.02;33:1 nat both observers_apart_late_km gate earth_chord 1332.33 20;34:1 nat new star_points_late gate near 0 10;36:1 nat new stars_in_drawn_buffer_late gate near 0 10;49:1 nat both camera_bound_late note eq false -;
test -f claude/harness/artifacts/f131/keep/O1_new_late.json.gz && echo present => present
ls claude/harness/f131_run.sh claude/harness/f131_obs.py claude/harness/f128_leg.sh claude/harness/f121_run.sh claude/harness/s33_refchange.py claude/harness/f124_run.sh claude/harness/b3_ladder.py claude/harness/f56_canary.sh | wc -l => 8
test ! -e claude/harness/f148_cross.py -a ! -e claude/harness/f148_run.sh && echo absent => absent
md5sum ~/.spacecrafter/scripts/internal/initial_night.sts | cut -c1-8 => 78887972
# S4: every name the touched headers declare (names.py, 11.265(g)5)
python3 claude/harness/shape/names.py src/experimentalModule/Camera.hpp => Camera.hpp 79: Camera distanceToReference velocityScaling dumpTrace saveSession restoreSession switchToBody warpToBody relocateToBody update draw setBoundToSurface setFreeMode lookTo lookRel oldLocalToLocal setMount toggleMount setSkyLock getSkyLock getViewParams getAbsFwd getForwardLocal getPlanTimers getZoomSrcHalfFov getZoomDstHalfFov moveHeading setHeading getHeading moveRel moveEyeRel moveRelLon moveRelLat moveRelAlt multAlt moveTo fold viewRotation placementRotation setViewOffset getViewOffset armViewOffset restoreViewOffsetLatch observedToLocalPos observedToBodyEquPos observedToBodyLocalPos observedPosToRaDe observedPosToAltAz trackBody getReferenceBody getTrackedBody rebindReference getCurrentSystem isFreeMode setHalfFov getReferenceRelativePosition positionRelativeTo getRootPosition placeAt getPlace setLongitude setLatitude setAltitude getLatitude getLongitude localToBodyEqu posePart posePartToPose calculateZoomCoef calculateZoomCoefVelocity recoverParams advanceView renderViewRotation viewMat viewOffsetEyeRotation advanceViewOffset effectiveViewOffset paramForward start
python3 claude/harness/shape/names.py src/bodyModule/ssystem_factory.hpp | wc -w => 80
python3 claude/harness/shape/names.py src/experimentalModule/CameraAnchors.hpp => CameraAnchors.hpp 29: CameraAnchors ~CameraAnchors load add remove switchTo setFollowRotation placeCurrentAt travelToPoint travelToBody transitionToPoint transitionToBody update isMoving getTravelStart getTravelDirection getTravelDistance getTravelStartTime getTravelEndTime getCurrent dumpState parseKind buildBody ensureBody createAnchorBody find currentPlace installTravel getTravelSeq
```

**WIP:** 2026-10-11 17:5x — MINTED (round supervisor, Claude Fable 5.1, session 44) on F144's accepted block, the owner's H-defaults standing (no word since 2026-10-10; each one revert), the §5.194 rider from the session-43 list's "the next task touching that header"; gate run at the mint. **DISPATCHED ~18:00 (`opus-xhigh`, gate 43 / 43 at 18:00:10), alone in the code tree. STOPPED 18:20 at P11's STOP before any edit, replicated 18:28, reported 18:32 (`INTENT/11.319.md`, 105 lines): `switchToBody` holds the VIEW (1e-6 rad) and WARPS THE PLACE — 244.58 / 244.39 AU at the leave, 231.00 / 230.83 AU at the return (the kept `position` vector read in the new reference's frame; `recoverParams` consumes the rotation only; the owner's baseline has the same form); the free-move walk teleports 0.9926 AU at Earth's influence exit (the control scene) — D15 (b) measured violated; the BEFORE ladder: no reference change at any rung, `set home_planet SolarSystem` refused silently, `aimed` absent. LIFTED 18:5x by the supervisor (ladder step 3): option (b) — the hold composed from existing entries inside `followInfluence` (`positionRelativeTo` → the switch → `placeAt(p, true)`: measured 7e-5 AU / 1e-6 rad), `switchToBody` untouched; P4 / P11 / P13 / P26 / P29 re-predicted (the float floor: <= 2e-4 AU at 600 AU, <= 1e-5 rad); option (c) — the hold inside `switchToBody`, one effect for its two consumers — routed to the owner as H2 with the teleport as a §5 candidate. Dispatcher defects acknowledged: `:802` is `saveSession`'s "reference" (dumpTrace's at :914-915); the dump has no `system` field (miss-ledger); the onEnter relocate (`SolarSystemModule::onEnter → switchToAnchor("Sun")`) a B42-class second writer, recorded, outside WRITES. RESUMED by message 18:5x in its own context.**

### F150 — THE SHAPE OF A DELIVERY ASSEMBLED BY ONE SCRIPT (row B46's OPEN half, §11.256: *"the per-delivery numbers are reported by hand, no script assembles them yet"*): `harness/shape/delivery.py <base> [<head>|WORKTREE] [--class Class:hpp:cpp]...` prints the SHAPE block an acceptance pastes — per file the numstat split code / comment / blank by `ratio.py`'s rule (trailing comments count as code), the totals and the net; per touched header `names.py` BEFORE → AFTER with the names added / removed; per touched class (header AND source in the diff) `writers.py`'s (field, writers) pairs BEFORE → AFTER with the pairs that changed; the comment form of the ADDED comment lines (one line, verb first — `form.py`'s measures: blocks of 1 / 2 / 3+ lines, median length); the ledger-grammar grep over the ADDED lines (`INTENT\|11\.[0-9]\|F1[0-9][0-9]\|\[observed\|\[measured\|\[vixy`); `f70_ascii.py gate`'s verdict; the module ratio at both ends (`ratio.py`); the header-scope file count (`review.sh`'s scope); and its own ORACLE TEST against the two recorded deliveries — every number reproduced or the difference named [S, **HARNESS ONLY — nothing under `src/`, no build, no launch, no commit, no stash, NO README / SHAPE.md edit (the supervisor's line at acceptance; F148 writes the README beside you)**; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): the input set is the six instruments, one concern, criteria that can fail); entry number ALLOCATED: **§11.321**]

**Why now / mandate:** SHAPE.md §6 *"ACCEPTING A DELIVERY: behaviour evidence AND shape — net code lines, deleted / added, comment form, manager count"* and §0b (c) bind every acceptance to these numbers; today they are TWO hand-derivations of one information — the executor's shape section (`INTENT/11.317.md` (7): *"code +794 / -435 (net +359), comments +21, blanks +45"*) and the supervisor's acceptance line (the same numbers re-typed) — a second manager (S7) and the channel of Q-67 (a number typed from memory of a listing). One command, one home, pasted by both. The two recorded deliveries are the oracles: F146's two executor commits `a7919278..35da9c94` (11.317 (7): 50 files, +860 / −435; `names.py` Renderer 53 → 54, EnvironmentManager 13 → 14, CameraProjection 1 → 3, core 144 → 145, ssystem_factory 77 → 78, nebula 36 → 37; new `skyView.hpp` 27, `oldSkyView.hpp` 27, `CameraSkyView.hpp` 29; `writers.py Projector` identical) and F140's three `2cb05825..a7919278` (11.316: read its shape section for the numbers; 17 files, +265 / −80 by numstat).

**The ORIGIN of the need (S10):** the owner's review time is the scarcest resource and its cost is the DISTANCE between the informations he needs (SHAPE §1); what he needs at a delivery is its shape in numbers he can trust — the output of a command, pasted. What it ends up covering: every acceptance (the supervisor), every executor's shape section (the same command — one home), the session close's `review.sh status`, and later a RATCHET (a count that may only go down, 11.265(g) — not this slice). NOT this slice: the ratchet, a hook, any judgment (the script reports, the supervisor judges).

**Mandate, in THIS order:** (0) §0 warm-up (harness form: no build, binary, display, instance or canary bullet applies), the PREMISES gate (`python3 claude/harness/premise_check.py F150`), then READ IN FULL: `claude/SHAPE.md` §4–§6; the six instruments WHOLE — `harness/shape/names.py`, `writers.py`, `ratio.py`, `form.py`, `codeident.py`, `review.sh` — and `harness/f70_ascii.py`'s `gate` entry; `INTENT/11.317.md` (7) and its Acceptance's SHAPE line; `INTENT/11.316.md`'s shape section; `INTENT/11.265.md` (g) (the ratchet idea, for what the block must make easy later); `harness/README.md`'s form (one `## F<n>` section per instrument — you write none, you propose its text in the entry). (1) The S4 pass: what each instrument ALREADY computes — compose, never re-implement: `ratio.py`'s classifier is THE rule for code / comment / blank; if it is not importable (a script body), the licensed change is to factor it into a function inside `ratio.py` with `ratio.py`'s own output UNCHANGED (prove it: its output at `a7919278` and `35da9c94` identical before / after, pasted); `names.py` reads a path (working tree) or `--rev`; `writers.py` takes a rev only — extend it with `WORKTREE` as the rev (the files read from disk), its old behaviour unchanged (output at `HEAD` identical before / after, pasted); `form.py`'s measures over a file list with a `read` callback — reuse its `measure`. (2) BUILD `harness/shape/delivery.py`: `<base>` a rev; `<head>` a rev or `WORKTREE` (the working tree, untracked files under `src/` included); `--class Class:hpp:cpp` repeatable, default = every class declared in a touched header whose `.cpp` of the same stem is also in the diff; output ONE fenced `SHAPE` block in a FIXED order documented in the script's header: (1) per file `+code -code +comment -comment +blank -blank`, the totals, the net code; (2) per touched header `names BEFORE -> AFTER [+added] [-removed]`; (3) per class `field: writers BEFORE -> AFTER` for the pairs that CHANGED, and the count of unchanged pairs; (4) the added comment lines' form (count, blocks 1 / 2 / 3+, median length, verb-first share); (5) the ledger-grammar hits in the ADDED lines (file:line, or 0); (6) `f70_ascii.py gate` PASS / FAIL (run from the code root); (7) `ratio.py` at both ends (for a WORKTREE head: the working tree's module); (8) the header-scope count (headers under `src/` in the diff) and the count outside it. No prose, no judgment, exit 0 (the numbers are the deliverable; a FAIL of the gate is a line, not an exit). One-line verb-first what-fors; pure ASCII (`f70_ascii.py gate` on your own file). (3) THE ORACLE TEST `delivery.py --self-test`: runs the script on `a7919278..35da9c94` and asserts the recorded numbers above; then on `2cb05825..a7919278` against 11.316's; a number that does not reproduce → the self-test FAILS and the report names the RULE difference (the executor's own classifier against `ratio.py`'s trailing-comment rule, a blank-line convention, a renamed file counted twice) — NEVER fit the script to the record, never re-type the record: the self-test's expected values are quoted from the two entries with their line numbers. (4) RUN IT ON THE LIVE ROUND: `python3 claude/harness/shape/delivery.py afae8573 WORKTREE` while F148 edits beside you — read-only on the code tree; paste the block (the numbers are F148's business; yours is that the script runs on a dirty tree with untracked files, and that `WORKTREE` reads what `git status` shows). (5) The entry `claude/INTENT/11.321.md` (checkpoint state in its FIRST LINES): the self-test's lines; the two oracle runs pasted; what each line of the block means and which instrument produced it; the one change to `writers.py` (and to `ratio.py` if any) with its before / after proof; the README section's TEXT and the SHAPE.md §6 line PROPOSED (not written) — the supervisor lands them at acceptance. (6) The report, per the definition's form (its first line `prompt-window: present`).

**Boundaries:** WRITES — `claude/harness/shape/delivery.py` (new), `claude/harness/shape/writers.py` (the `WORKTREE` rev only), `claude/harness/shape/ratio.py` (the classifier factored — ONLY if needed, output unchanged), `claude/INTENT/11.321.md`, `claude/harness/artifacts/f150/**`. NOT yours: `harness/README.md`, `SHAPE.md`, `src/` (`git -C /home/claude/spacecrafter diff --stat` EMPTY of your doing — F148 edits there, read it only), every other instrument, `fable-dispatch.md`, `INTENT.md`. No launch, no build, no commit, no stash.

**STOP conditions:** a premise fails; an oracle number cannot be reproduced AND the rule difference cannot be named (then the record or the instrument is wrong — report both values, stop that assertion, continue); `names.py` or `writers.py` disagree with the record for a reason INSIDE the instrument (a bug) — report, do not fix beyond `WORKTREE`.

**Discriminating checks:** (a) the self-test's lines, every expected value quoted with its entry line; (b) `ratio.py` and `writers.py` outputs identical before / after your change, at two revs; (c) the WORKTREE run on F148's dirty tree; (d) `f70_ascii.py gate` PASS on your files; (e) the block's fixed order documented in the script's header and followed by the output.

**Preconditions (checkable, §0.7):** the PREMISES block; the two oracle entries pinned (their md5 lines); F148 beside you writes under `src/` and in `harness/f148_*`, `f131_analyse.py`, the README — none of them yours.

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch, never at mint
git -C claude rev-parse --abbrev-ref HEAD => CC-harness
git rev-parse --short=8 HEAD => afae8573
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.321.md\|INTENT/11.319.md\|harness/f148_' | tr '\n' ' ' => 
test ! -e claude/INTENT/11.321.md -a ! -e claude/INTENT/archive/11.321.md && echo free => free
/usr/bin/grep -c '^### F150 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
# the instruments the script composes, pinned; the one it adds is absent
md5sum claude/harness/shape/names.py claude/harness/shape/writers.py claude/harness/shape/ratio.py claude/harness/shape/form.py claude/harness/shape/codeident.py claude/harness/shape/review.sh claude/harness/f70_ascii.py | cut -c1-8 | tr '\n' ' ' => 94166600 8db0ee55 5bf03e0b 1825d3e9 2e46a622 0c53baf9 3b9b092d
test ! -e claude/harness/shape/delivery.py && echo absent => absent
/usr/bin/grep -c 'delivery.py' claude/harness/README.md claude/SHAPE.md | tr '\n' ' ' => claude/harness/README.md:0 claude/SHAPE.md:0
/usr/bin/grep -c '^## F' claude/harness/README.md => 82
# the two deliveries whose recorded shape numbers are the ORACLES (11.317 (7) for F146's two commits; 11.316 for F140's three), pinned
md5sum claude/INTENT/11.317.md claude/INTENT/11.316.md | cut -c1-8 | tr '\n' ' ' => d9d7e057 69e245b1
git diff --numstat a7919278 35da9c94 | awk '{a+=$1; d+=$2; n++} END {print "files", n, "+", a, "-", d}' => files 50 + 860 - 435
git diff --numstat 2cb05825 a7919278 | awk '{a+=$1; d+=$2; n++} END {print "files", n, "+", a, "-", d}' => files 17 + 265 - 80
git diff --numstat ee185d4f 2cb05825 | awk '{a+=$1; d+=$2; n++} END {print "files", n, "+", a, "-", d}' => files 24 + 265 - 194
python3 claude/harness/shape/ratio.py a7919278 | tail -1 => a7919278 files 122 code 13257 comment 879 ratio 0.07
python3 claude/harness/shape/ratio.py 35da9c94 | tail -1 => 35da9c94 files 124 code 13479 comment 888 ratio 0.07
python3 claude/harness/shape/names.py --rev a7919278 src/experimentalModule/Renderer.hpp | cut -d: -f1 => Renderer.hpp 53
python3 claude/harness/shape/names.py --rev 35da9c94 src/experimentalModule/Renderer.hpp | cut -d: -f1 => Renderer.hpp 54
python3 claude/harness/shape/names.py --rev 35da9c94 src/coreModule/skyView.hpp | cut -d: -f1 => skyView.hpp 27
git diff --numstat a7919278 35da9c94 -- src/coreModule/skyline.cpp => 117	117	src/coreModule/skyline.cpp
/usr/bin/grep -c 'code +794 / -435' claude/INTENT/11.317.md => 1
# the review mark (the owner's), read-only to this task
git rev-parse --short=8 refs/review/headers => ee185d4f
bash claude/harness/shape/review.sh status 2>/dev/null | tail -2 | tr -s ' ' | tr '\n' ';' => ee185d4f files 119 code 13193 comment 881 ratio 0.07; HEAD files 124 code 13475 comment 889 ratio 0.07;
```

**WIP:** 2026-10-11 17:5x — MINTED (round supervisor, Claude Fable 5.1, session 44) on B46's open half; the oracles are the two recorded deliveries. **DISPATCHED ~18:00 (`opus-xhigh`, gate 23 / 23 at 18:00:09), harness tree only. DELIVERED 18:32 (`INTENT/11.321.md`, 638 lines): `harness/shape/delivery.py` composing the six instruments (`writers.py` gains `WORKTREE`, `ratio.py`'s classifier factored importable — 14 / 14 outputs `cmp`-identical before / after); `--self-test` 167 PASS / 9 FAIL, every FAIL a NAMED record-side rule difference the test computes and reproduces (11.317:155 counted "new" headers against the review mark; 11.316:117 / :143 predate the supervisor's whitespace re-indent — `-w` on that file reproduces; "+7 names" counted names added to pre-existing types); the dirty-tree run NOT REACHED (F148 had not edited by 18:32). FINDINGS: (1) `names.py` / `writers.py` strip `/* */` before `//` — `ssystem_factory.hpp:432`'s `ra;//*180.0/M_PI;` opens a phantom block to :494, names.py 78 where ~162 exist, `writers.py SSystemFactory` 0 fields (reach: that header + `main.cpp:80`); (2) the D14 gate is blind to `claude/` (3 non-ASCII bytes in `delivery.py` under a PASS — fixed by hand; miss-ledger); (3) `form.py` counts a `*`-leading code line as a comment (F146: 28 against 21); (4) the session-43 acceptance re-typed a number its author had just changed (11.316:143) — the second-manager mechanism this task closes. ACCEPTED IN PRINCIPLE 18:5x; FOLLOW-UP dispatched by message (WRITES extended to `names.py` / `writers.py` / `codeident.py` / `form.py`): `PASS*` + exit 0 for a named record-side rule, the single-pass decomment at ONE home for the three strippers with the whole-`src/` proof, form.py's block state, finding 2 proposed not built.**

### F149 — KEY-BOUND SCRIPTS, IMPLEMENTED against F142's PREDICTIONS block (32 lines, `INTENT/11.312.md` (j); row B49, the owner's design §11.305 (a)(b)(f)(g) — his words the ruling): ONE registry `KeyBinding` (`src/uiModule/key_binding.{hpp,cpp}`, a by-value member of `UI`) consulted at the TOP of `UI::handleKeys` — the ONE door for press and release (the event loop :901 / :905) — above the TUI and video gates; the inline release switch replaced by a delegation to `handleKeysReleased` (the dead twin becomes the live one, and CARRIES F137's four stop cases: the dedup Q9 / P10 gave to the second to land); `script action bind [key <key>|*] [press <name|->] [release <name|->] [scope <global|script|once>]` as the 26th `W_` action of `commandScript`, its errors LOUD in both forms (e5); the scopes' lifecycle with ONE hook at the anchor `ScriptMgr::cancelScript` → `AppCommandInterface::scriptEnded` → `UI`; the trigger key carried PER TOKEN (`ScriptOrigin::key`, one defaulted parameter through the four doors) so "key omitted = this key" resolves and a recorded `bind` replays; K-3's press record = the repeat filter; the KEY LEG (14 cases A–N, `harness/f142_keys.py` on the b21 pattern over TCP 7805) built first; every prediction judged, every deviation reported [L, **CODE under `src/` within the 17 files below and no other, + harness; the agent may BUILD (alone in the code tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE while it runs; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): the block names files, functions, +/- lines, the cases); entry number ALLOCATED: **§11.320**; dispatched AFTER F148's acceptance and commit (the same tree, both build)]

**Why now / mandate:** the owner designed the surface and answered every question of it on 2026-10-10 (`INTENT/11.305.md` (a)(b)(c)(d)(f)(g)); F142 structured it from the code and was ACCEPTED 2026-10-10 18:1x (`fable-dispatch/archive/F142.md`): the registry's home by the RA (the key identity including the ² plane exists only in UI; the end signal arrives over existing pointers), ONE lookup door (the mandate's "two key handlers" refuted by a zero-caller grep), K-4's mechanism read from the engine (`Script::load` PREPENDS — `script.cpp:129–130`), `bind` carrying `filename` alone (the grammar's trailing slot empty today — his K-4 sentence an extension point), four contradictions surfaced first, 16 questions in premise / default / reason form; two §5 rows minted from it (§5.190 the native CTRL+UP release asymmetry, §5.191 `UI::init`'s drifted copy of W_END's cleanup — neither yours). F137 (B44, `2cb05825`) landed FIRST on the release switch, so this implementation CARRIES THE DEDUP (Q9's rule: *"whichever implementation lands second carries the dedup"*). The owner's words on C1 / Q2 / Q6 were asked 2026-10-10 and 2026-10-11; none came — the structure's defaults stand BUILT, each one revert: **Q3 (C1)** `play`'s rules (r1)+(r2) verbatim — an extension-less name is a VARIABLE, `press foo` = the variable `foo`, `press *` the variable `*`; **Q2** the key token `[CTRL|SHIFT|KWIN|SUPER|ALT+]*<SDL_SCANCODE suffix>` (his enum identifiers, `ui.hpp:78–85`), parsed case-insensitively, the parser DERIVED from `SDL_GetScancodeName` with an exception table ≤ 20 rows (P13; if the rule does not hold, the licensed deviation is the plain `{"KP_8", SDL_SCANCODE_KP_8}` table, reported with the SDL-name fact that forced it — P12); **Q6** the delegation (−36, the stops land in `handleKeysReleased`); **Q1** above the TUI / video gates; **Q4** bind-time resolution (loud at the authoring line; the stored value a full path); **Q5** both error forms (`reportScriptError` + `debug_message` / `executeCommandStatus`); **Q7** NOT in this slice (§5.190 his); **Q8** the four stops UNCONDITIONAL on the base key's release (the arrows' pattern) — F137's F2 finding (a plain KP release whose press ran `M0x.sts` also writes the stop and kills a show's held `camera action move_lat value 1`) STAYS with this default; the one-line alternative (gate the native stop on the plane recorded at press, the registry's `held`) is recorded in your entry for the owner, not built; **Q10** `key *` with `script` / `once` pushes on the wildcard chain only; **Q11** scope absent or `global`; **Q12** a `bind` with the key omitted is RECORDED with the explicit key (P29, the `commandFlag` precedent); **Q13** `setScriptPath` untouched; **Q14** the joystick row: nothing now (`joypad_controller.*` NON-change, P14); **Q15** K-4's side effects accepted (the show's wait cut, its speed reset, `end` inside ends the show, a paused show un-paused without audio); **Q16** globals survive a HOME reinit, script-scoped ones die with the execution. The NAMES are his to give, built as F142 proposed and flagged: `KeyBinding` (`KeyBindings` / `KeyOverride` the alternatives), `bind` / `resolve` / `endScript` / `parse` / `name`, `UI::keyBinding`, `AppCommandInterface::scriptEnded`, `ScriptOrigin::key`, `ScriptEvent::getKey`, the words `bind key press release scope once global script`.

**The ORIGIN of the need:** F142 (a), by reference (one operator, fewer than a hundred people, no typing in session, a keyboard allocated on five modifier planes; keys that do what THIS show needs, authored off session in one line, with lifetimes matching a show and a sure way back; a wrong binding fails LOUD off session, at the line). What it ends up covering: every key the handlers distinguish (base scancode x plane) plus any scancode, press and release, any script the engine runs, the show's keys gone by themselves at the show's end, hold-to-X without a blocking wait, the four keyboard movers as bindings. NOT this slice: the joystick's buttons / hat (the same registry later — (c)4 names what exists), the mouse, §5.190, §5.191's consolidation (the hook's follow-up, a second structural change).

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F149`, refreshed at dispatch on F148's accepted state), then READ IN FULL: `claude/SHAPE.md`; `INTENT/11.312.md` whole ((a) the origin, (b) the S4 pass, (c) what else the read found, (d) K-4's mechanism, (e1)–(e8) the shape with its arguments, (f) the manager table, (g) the first bindings and the sequencing conflict, (h) the key leg's 14 cases, (i) the file list and the functions by name, (j) the block, (k) the contradictions and the questions, (l) the deviations); `INTENT/11.305.md` (a)(b)(c)(d)(f)(g) (his design and answers — the ruling); `INTENT/11.311.md` (1) and (4)(d) (F141's order surface); `INTENT/11.299.md` (its (3) the keyboard's release path as BUILT, its F2 finding); `INTENT/11.273.md` (b)3–4 (the keyboard's planes); the code from the current checkout: `src/uiModule/ui.hpp` WHOLE; `ui.cpp` — `handleInputs` (:790–920: the CTRL+ESC rescue, the two `handleKeys` calls :901 / :905, the `key_Modifier` capture), `handleKeysOnVideo` (:941), `handleKeyPressed` (:1068 to its end — the switch tables; the four mover lines :2596 / :2652 / :2710 / :2766; the GRAVE / SUPER mechanics ~:1120–1140), `handleKeysReleased` (:3351–3394) and `handleKeys` (:3396–3458: the gates, the INLINE release switch :3412–3456 with F137's KP cases :3447 / :3452), `UI::init` (~:250–270: the cancelScript copy :259); `src/interfaceModule/app_command_interface.cpp` — `commandScript` (:2830 to its end: the 25 `argAction==W_` tests, W_END :2837, `play` :2846–2853), `parseCommand` (~:125–189: the trailing-key drop), `executeCommandStatus` / `debug_message` (~:1216–1236), `reportScriptError` (~:208–220), `commandFlag`'s recording rewrite (~:1247–1251), `OriginScope` (~:271–276), `terminateScript` (~:247–255); `app_command_interface.hpp` whole; `base_command_interface.hpp` the `W_` block (:240–330); `src/scriptModule/script_mgr.{hpp,cpp}` whole (`playScript` :63–76, `addScriptFirst` :81–107, `cancelScript` :109–127, `update` ~:294–347), `script.{hpp,cpp}` whole (`load` :88, `loadInternal` :99–137 — the prepend :129–130, the token stamp :119), `script_origin.hpp` whole, `script_interface.{hpp,cpp}` (`playScript` :70), `script_annotator.hpp`; `src/eventModule/EventScript.hpp` whole, `EventScriptHandler.{hpp,cpp}` whole (`handle` :31, `loopDone` :38 — the reverse replay); `src/coreModule/core.cpp:1750–1790` (the latches, the guarded stops), `core.hpp:290–300` (`moveLat` / `moveLon`); `src/appModule/app.cpp` at the event-handler wiring; `src/tools/file_path.cpp:130–145` (r2), `app_command_eval.cpp:75–96` (r1); the harness: `b21_keypath_run.sh` and `b21_keypath.py` whole (the pattern), `f124_run.sh` (header), `sc_instances.sh`; the field: `~/.spacecrafter/config.ini` (`enable_tcp`, `tcp_port_in` — `/usr/bin/grep`, ISO bytes), `~/.spacecrafter/log/script.log`'s form from one launch, `~/.spacecrafter/scripts/internal/M08.sts` (what KP_8 runs today). (1) The S4 pass from the names in the block (ui.hpp 92, app_command_interface 93, script_mgr 36, script 16, script_interface 35, script_origin 5, EventScript 4, EventScriptHandler 4): the registry (none — build), the run path (door (B) — use), the running execution (`scriptState` — extend by the origin's key), the press identity (none — build inside the registry), the lifetime signal (`cancelScript` — use, one hook), the error forms (two — use both) — each said; a name that exists for the same EFFECT is consumed, never doubled. (2) BUILD THE INSTRUMENT FIRST ((h)): `claude/harness/f142_keys.py` (the driver: the engine under gdb on a FIFO through `f124_run.sh`; `break UI::handleInputs` once to take `this`; per case the driver sets `$ui->key_Modifier` and calls `$ui->handleKeys(<scancode>, 0, 0, s_gui::S_GUI_PRESSED|RELEASED)`, resumes, reads `~/.spacecrafter/log/script.log`'s tail; the `bind` lines sent over TCP 7805 with ABSOLUTE fixture paths — `claude/harness/artifacts/f142/run/<x>.sts`, each one line `define f142_<x> 1`; the show of cases G / H three `wait duration 5` lines timestamped; one `PASS|FAIL|NOTE <case> <criterion> observed= expected=` line per case) + `claude/harness/f142_keys_run.sh`; the config pair md5 in == out as b21 asserts. RUN case N (the control) and the natives on the BEFORE binary — the `bind` action does not exist: cases A–M read the unknown-action error (paste the control's log: it is P30's baseline). (3) BUILD F142's block in (i)'s order, groups, a build + `f70_ascii.py gate` + a checkpoint line after each: (A) `src/uiModule/key_binding.{hpp,cpp}` (the class — `bind` / `resolve` / `endScript` / `parse` / `name`; the chains per (key, event); the `*` sentinel `SDL_SCANCODE_UNKNOWN`; `held[s] = {plane, taken}`; one-line verb-first what-fors as (i) writes them; the module's comment row — SHAPE §4), `ui.hpp` (+4: include, member, accessor), `ui.cpp` — the lookup as the FIRST statement of `handleKeys` (`if (keyBinding.resolve(key, key_Modifier, state)) { if (ANY_MOD(SUPER)) RESET_MOD(SUPER); return 1; }` — P2), the inline release switch :3412–3456 replaced by `return handleKeysReleased(key, mod, unicode, state);` and `handleKeysReleased`'s switch made IDENTICAL to today's inline one — F137's four KP cases included (`KP_2` / `KP_8` → `core->moveLat(0)`, `KP_4` / `KP_6` → `core->moveLon(0)`, unconditional) — P3 / P10: the stops appear ONCE, net about −30 in `ui.cpp`; (B) `base_command_interface.hpp` (+6: `W_BIND W_KEY W_PRESS W_SCOPE W_ONCE W_SCRIPT`; `W_RELEASE` :323 / `W_GLOBAL` :324 reused — P5), `app_command_interface.cpp` (the `argAction==W_BIND` branch, the 26th test — P4; the five errors of (e5) in BOTH forms; the recording rewrite of P29), `app_command_interface.hpp` (+1 `scriptEnded`), `script_mgr.cpp` (`cancelScript` gaining `commander->scriptEnded();` — ONE hunk, P7; `playScript`'s signature — the second hunk, P8); (C) the trigger key per token (P6): `ScriptOrigin` gaining `std::string key` (`script_origin.hpp`), the defaulted parameter threaded `ScriptEvent(file, key)` → `EventScriptHandler` → `ScriptInterface::playScript(file, key)` → `ScriptMgr::playScript(file, key)` → `Script::load(file, path, key)` → stamped at the one token-creation site `script.cpp:119`; `commandScript` reading `currentOrigin.key`; `addScriptFirst`'s tokens stay origin-less; `ScriptEvent::getKey`. (4) GATES by your own hand: the 14 cases A–N (P17–P30; P30 = case N's log IDENTICAL to the BEFORE control's for the same calls); `bash claude/harness/f131_run.sh 4 both` and the FULL canary UNCHANGED (P31 — no draw path, no mover law touched); P32 by position (`handleInputs` untouched — P11). (5) Shape: `git diff --numstat` per file against (i) (net `src/` +285 ± 60; `key_binding.cpp` ≤ 230 — P12); the greps of P2 (`keyBinding` in `ui.cpp` 2 or 3), P3 (`handleKeysReleased(` = 2), P4 (`argAction==W_` = 26; `"bind"` once in `base_command_interface.hpp`), P5 (+6 / −0 there), P7 (`endScript\|scriptEnded` = 4 lines in `src/`), P8 (`script_mgr.cpp` 2 hunks), P11 (`repeat` 0; no hunk in `handleInputs`), P13 (`SDL_SCANCODE_` ≤ 25 in `key_binding.cpp`), P14 (`joypad_controller` absent from the stat), P15 (the stat ⊆ (i)'s list), P16 (ledger grammar 0; `form.py` on the new header within his module's row); `names.py` on every touched header BEFORE → AFTER; the manager table (f) re-derived on the diff. (6) The entry `claude/INTENT/11.320.md` (checkpoint state in its FIRST LINES, refreshed after each group): every P1–P32 judged with evidence; every deviation with its argument; the 14 cases' readings BEFORE / AFTER; the names flagged; F137's F2 alternative recorded for the owner (one line, not built); what the joystick row (Q14) and §5.191's consolidation must know. (7) The report, per the definition's form (its first line `prompt-window: present`).

**REFRESHED AT MINT (2026-10-11 17:5x, session 44, on `afae8573` — what moved since the structure's `38ec7199`):** (i) F137 (`2cb05825`) LANDED the four movers as F141's orders — `core->moveLat(-1)` :2596, `core->moveLon(-1)` :2652, `core->moveLon(1)` :2710, `core->moveLat(1)` :2766 — and the two stop groups in the INLINE release switch of `handleKeys` (:3447 `core->moveLat(0)`, :3452 `core->moveLon(0)`); `moveto delta_l` 0 in `ui.cpp`; so P9 is CONFIRMED AT READING (you change no `handleKeyPressed` line), its "8 after both land" reads 6 lines (4 starts + 2 grouped stops), and P10's stops MOVE with the switch into `handleKeysReleased` (they are not added). (ii) `handleKeys` is at :3396 (was :3404), `handleKeysReleased` :3351 (was :3359), the event loop's two calls :901 / :905 (were :908 / :912), `W_RELEASE` / `W_GLOBAL` at :323 / :324 (were :319 / :320), `case SDL_SCANCODE_` 118 (was 114: F137's four). (iii) The prepend reads `p2Last->pNext=pFirst; pFirst=p2First;` at `script.cpp:129–130` (no spaces — a grep with spaces finds nothing). (iv) `Core::moveLat` / `moveLon` exist (`core.hpp:294–295`) and the commands `camera action move_lat|move_lon|raise_height|lower_height value <v>` (`W_MOVE_LAT` :249; `app_command_interface.cpp:4380`) — N6 BUILT: `-` on SHIFT+KP_8 and the script line write the same held slot by construction. (v) §5.190 and §5.191 are in the ledger (2 rows) — neither yours. (vi) F148 (the reference decider) landed before you in `Camera.*` / `ssystem_factory.*` — disjoint from your 17 files; its f131 obs-1 re-form is in the bank you run `4 both` against. (vii) The per-round lines below are `REFRESH-AT-DISPATCH` by construction until F148's acceptance.

**Boundaries:** WRITES — CODE (17, P15): `src/uiModule/key_binding.hpp`, `key_binding.cpp` (new), `ui.hpp`, `ui.cpp`; `src/interfaceModule/app_command_interface.hpp`, `app_command_interface.cpp`, `base_command_interface.hpp`; `src/scriptModule/script_origin.hpp`, `script.hpp`, `script.cpp`, `script_mgr.hpp`, `script_mgr.cpp`, `script_interface.hpp`, `script_interface.cpp`; `src/eventModule/EventScript.hpp`, `EventScriptHandler.hpp`, `EventScriptHandler.cpp`; HARNESS: `claude/harness/f142_keys.py`, `claude/harness/f142_keys_run.sh` (new), `claude/harness/README.md` (ONE section appended), `claude/INTENT/11.320.md`, `claude/harness/artifacts/f142/**`. NON-CHANGE, a gate: `joypad_controller.*` (P14), `handleInputs` (P11), `handleKeyPressed`'s lines (P9 — the four movers are F137's), `core.*`, `src/experimentalModule/*`, `src/coreModule/*` but nothing, `app.cpp`, every draw path; the field's script directory is never written (absolute fixture paths); `~/.spacecrafter/config.ini` / `ssystem.ini` md5 in == out. No install, no stash, no commit, no `run_in_background`; launches EXCLUSIVE (`sc_instances.sh --assert`; never `pkill -f`; none alive at close).

**STOP conditions:** a premise fails; case N (the control) differs from the BEFORE log for the same calls (P30 — the lookup is not transparent: STOP, report the first differing line); a case needs a change outside WRITES (name the site); the SDL-name rule fails AND the plain table exceeds 300 lines (P12 — report with the fact, continue with the table); the TCP channel refuses the `bind` lines (the field config's `enable_tcp` is a premise — report, do not edit the config); the canary red for a reason other than a boot.

**Discriminating checks:** (a) every P line has a verdict with evidence; (b) the 14 cases' lines BEFORE (the control, the unknown-action errors) / AFTER; (c) the stops appear ONCE in `ui.cpp` and `handleKeysReleased(` has exactly 2 hits (definition + call); (d) `argAction==W_` 26, `W_BIND` 1 in the words header; (e) `endScript\|scriptEnded` exactly 4 lines in `src/`, none at W_END nor in `UI::init`; (f) `repeat` 0, `handleInputs` untouched; (g) `git diff --stat -- src/` ⊆ the 17 files; (h) `f70_ascii.py gate` PASS; (i) the numstat against (i) ± 60 net; (j) the manager table (f) re-derived.

**Preconditions (checkable, §0.7):** the PREMISES block (the per-round lines refreshed at dispatch); F142 ACCEPTED (`archive/F142.md`); F137 LANDED (`2cb05825` in the log); F148 ACCEPTED and COMMITTED (its WIP line; the binary rebuilt at that commit — the BEFORE of this task).

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch (after F148's acceptance), never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => REFRESH-AT-DISPATCH
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.320.md' | tr '\n' ' ' => REFRESH-AT-DISPATCH
md5sum build-claude/src/spacecrafter | cut -c1-8 => REFRESH-AT-DISPATCH
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.320.md -a ! -e claude/INTENT/archive/11.320.md && echo free => free
/usr/bin/grep -c '^### F149 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/fable-dispatch/archive/F148.md && echo accepted => REFRESH-AT-DISPATCH
# the engine slot, the boot's fingerprint (a red here is a STOP to report, never a re-bank by the executor), the field pair
bash claude/harness/sc_instances.sh --assert f149-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => REFRESH-AT-DISPATCH
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
test ! -e ~/.spacecrafter/beta_features.ini && echo absent => absent
# the structure this task implements, pinned (F142's 32 lines, INTENT/11.312.md); its acceptance record; the two 5 rows minted from it
md5sum claude/INTENT/11.312.md | cut -c1-8 => aa3ce179
sed -n '/^ *PREDICTIONS *$/,/^ *P32 /p' claude/INTENT/11.312.md | md5sum | cut -c1-8 => fef2d6fa
sed -n '/^ *PREDICTIONS *$/,/^ *P32 /p' claude/INTENT/11.312.md | /usr/bin/grep -c '^ *P[0-9]' => 32
test -e claude/fable-dispatch/archive/F142.md -a -e claude/fable-dispatch/archive/F137.md && echo accepted => accepted
/usr/bin/grep -c '§5.190\|§5.191' claude/INTENT.md => 2
# ONE door: both key events enter handleKeys (the event loop), the dead twin has no caller (its definition is the one hit), the release switch is INLINE
/usr/bin/grep -n 'handleKeys(E.key.keysym.scancode' src/uiModule/ui.cpp | tr '\n' ' ' => 901:				handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_PRESSED); 905:				handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_RELEASED);
/usr/bin/grep -n '^int UI::handleKeys(SDL_Scancode key, Uint16 mod\|^int UI::handleKeysReleased(SDL_Scancode key, Uint16 mod\|^int UI::handleKeyPressed(SDL_Scancode key, Uint16 mod\|^int UI::handleKeysOnVideo(SDL_Scancode key' src/uiModule/ui.cpp | tr '\n' ' ' => 941:int UI::handleKeysOnVideo(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state) 1068:int UI::handleKeyPressed(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state) 3351:int UI::handleKeysReleased(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state) 3396:int UI::handleKeys(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
/usr/bin/grep -c 'handleKeysReleased(' src/uiModule/ui.cpp => 1
/usr/bin/grep -n 'if (state==s_gui::S_GUI_RELEASED) {' src/uiModule/ui.cpp => 3412:	if (state==s_gui::S_GUI_RELEASED) {
/usr/bin/grep -c 'case SDL_SCANCODE_' src/uiModule/ui.cpp => 118
/usr/bin/grep -c 'repeat' src/uiModule/ui.cpp => 0
# F137 LANDED (2cb05825): the four movers are F141's orders at press, their two stop groups in the INLINE switch - the dedup (P3 / P10 / Q9) is THIS task's; no script-disguised mover left
/usr/bin/grep -n 'core->moveLat(\|core->moveLon(' src/uiModule/ui.cpp | tr -s '\t' ' ' | tr '\n' ';' => 2596: core->moveLat(-1);;2652: core->moveLon(-1);;2710: core->moveLon(1);;2766: core->moveLat(1);;3447: core->moveLat(0);;3452: core->moveLon(0);;
/usr/bin/grep -c 'moveto delta_l' src/uiModule/ui.cpp => 0
/usr/bin/grep -n 'void moveLat(double);\|void moveLon(double);' src/coreModule/core.hpp | tr '\n' ' ' => 294:	void moveLat(double); 295:	void moveLon(double);
/usr/bin/grep -n '#define W_MOVE_LAT' src/interfaceModule/base_command_interface.hpp => 249:#define W_MOVE_LAT                  "move_lat"
# the command the design extends: 25 actions today, bind the 26th; the two words reused, the six new ones absent
/usr/bin/grep -n '^int AppCommandInterface::commandScript' src/interfaceModule/app_command_interface.cpp => 2830:int AppCommandInterface::commandScript(uint64_t &wait)
/usr/bin/grep -c 'argAction==W_' src/interfaceModule/app_command_interface.cpp => 25
/usr/bin/grep -n 'argAction==W_END' src/interfaceModule/app_command_interface.cpp => 2837:		if (argAction==W_END) {
/usr/bin/grep -n '#define W_RELEASE\|#define W_GLOBAL' src/interfaceModule/base_command_interface.hpp | tr -s '\t' ' ' | tr '\n' ' ' => 323:#define W_RELEASE "release" 324:#define W_GLOBAL "global"
/usr/bin/grep -c 'W_BIND\|W_SCOPE\|W_ONCE\|W_PRESS' src/interfaceModule/base_command_interface.hpp => 0
# the engine: the prepend (K-4's mechanism), the one end anchor, the origin per token, the three doors' signatures (one defaulted parameter each, P6)
/usr/bin/grep -n 'p2Last->pNext=pFirst;\|pFirst=p2First;' src/scriptModule/script.cpp | tr -s '\t' ' ' | tr '\n' ' ' => 129: p2Last->pNext=pFirst; 130: pFirst=p2First;
/usr/bin/grep -n '^int Script::load(const std::string &script_file, const std::string &script_path )' src/scriptModule/script.cpp => 88:int Script::load(const std::string &script_file, const std::string &script_path )
/usr/bin/grep -n 'token=new Token(line, script_path, ScriptOrigin::fromFile(script_file, lineNo, line));' src/scriptModule/script.cpp | cut -d: -f1 => 119
/usr/bin/grep -n '^void ScriptMgr::cancelScript()' src/scriptModule/script_mgr.cpp => 109:void ScriptMgr::cancelScript()
/usr/bin/grep -n 'bool playScript(const std::string &fullFileName);' src/scriptModule/script_mgr.hpp => 54:	bool playScript(const std::string &fullFileName);
/usr/bin/grep -n 'bool playScript(const std::string& _script);' src/scriptModule/script_interface.hpp => 70:	bool playScript(const std::string& _script);
/usr/bin/grep -n 'ScriptEvent(const std::string& _fileName) : Event(E_SCRIPT) {' src/eventModule/EventScript.hpp => 36:    ScriptEvent(const std::string& _fileName) : Event(E_SCRIPT) {
/usr/bin/grep -n '^void EventScriptHandler::loopDone()' src/eventModule/EventScriptHandler.cpp => 38:void EventScriptHandler::loopDone()
/usr/bin/grep -n '^struct ScriptOrigin {' src/scriptModule/script_origin.hpp => 36:struct ScriptOrigin {
/usr/bin/grep -c 'key' src/scriptModule/script_origin.hpp => 0
/usr/bin/grep -n 'scriptInterface->cancelScript();' src/uiModule/ui.cpp => 259:	scriptInterface->cancelScript();
# the UI's identity vocabulary (the plane byte, the sticky SUPER), no name table in the tree, the registry absent
/usr/bin/grep -n 'uint8_t key_Modifier = 0x00;\|SUPER= 0x04,' src/uiModule/ui.hpp | tr -s '\t' ' ' | tr '\n' ' ' => 82: SUPER= 0x04, 409: uint8_t key_Modifier = 0x00;
/usr/bin/grep -rl 'SDL_GetScancodeName\|SDL_GetScancodeFromName' src/ | wc -l => 0
ls src/uiModule/key_binding.hpp src/uiModule/key_binding.cpp 2>&1 | /usr/bin/grep -c 'No such\|Aucun' => 2
# the instrument's channel: TCP 7805 on in the field config; the b21 pattern present; the leg absent
LC_ALL=C /usr/bin/grep -c '^enable_tcp *= *true\|^tcp_port_in *= *7805' ~/.spacecrafter/config.ini => 2
ls claude/harness/b21_keypath_run.sh claude/harness/b21_keypath.py | wc -l => 2
test ! -e claude/harness/f142_keys.py -a ! -e claude/harness/f142_keys_run.sh -a ! -d claude/harness/artifacts/f142/run && echo absent => absent
# S4: every name the touched headers declare (names.py, 11.265(g)5)
python3 claude/harness/shape/names.py src/uiModule/ui.hpp => ui.hpp 92: UI ~UI registerFont init initInterfaces draw updateTimeouts saveCurrentConfig drawGravityUi drawScriptStatusIcon handleClic handleMove handleKeys initTui localizeTui drawTui handleKeysTui tuiUpdateWidgets tuiUpdateIndependentWidgets setFlagShowTuiMenu setDateTimePosition setDateDisplayNumber handleJoyHat handleInputs moveMouseAlt moveMouseAz moveLat moveLon raiseHeight lowerHeight zoomIn stopZoomIn rightClick leftClick zoomOut stopZoomOut stopLowerHeight stopRaiseHeight speedDecrease stopSpeedDecrease speedIncrease stopSpeedIncrease pauseScriptOrTimeRate centerMouse turnUp stopTurnUp turnDown stopTurnDown turnRight stopTurnRight turnLeft stopTurnLeft turnHorizontal turnVertical executeCommand handleDeal handleKeysOnVideo handlKkeysOnTui handleKeyPressed handleKeysReleased flag toggle asBool tuiCb1 tuiCbSettimezone tuiCbSetTimeDisplayFormat tuiCbAdminLoadDefault tuiCbAdminSaveDefault tuiCbAdminSetLocale tuiCbAdminShutdown tuiCbViewportRelated tuiCbTuiGeneralChangeLandscape tuiCbTuiGeneralChangeSkyCulture tuiCbTuiGeneralChangeSkyLocale tuiCbDayKey tuiCbScriptsDeepsky tuiCbScriptsNavigation tuiCbScriptsPlanets tuiCbScriptsShows tuiCbScriptsCustom tuiCbScriptsBasis tuiCbEffectsMilkywayIntensity tuiCbEffectsNebulaeLabelMagnitude tuiCbSetlocation tuiCbStars tuiCbEffects tuiCbSkyTime tuiCbChangeColor tuiCbLocationChangePlanet handleDeltaSpeed handleJoyAddStick handleJoyRemoveStick
python3 claude/harness/shape/names.py src/interfaceModule/app_command_interface.hpp | cut -d: -f1 => app_command_interface.hpp 93
python3 claude/harness/shape/names.py src/scriptModule/script_mgr.hpp => script_mgr.hpp 36: ScriptMgr ~ScriptMgr playScript addScriptFirst playStartupScript cancelScript pauseScript fasterSpeed slowerSpeed defaultSpeed resumeScript recordScript recordCommand annotate cancelRecordScript isPlaying isPaused isRecording isFaster update getScriptList getScriptPath setPath setLoop initIterator setNbrLoop resetScriptLoop getMuliplierRate waitOnVideoTermination setWaitOnVideoTermination setIsVideoPlayed setFlagSkipPause getFlagSkipPause acquireGlobalLock releaseGlobalLock getRecordDate
python3 claude/harness/shape/names.py src/scriptModule/script.hpp => script.hpp 16: Token ~Token printToken getToken getTokenPath getOrigin Script ~Script printScript load clean getFirst addFirstInQueue loadInternal addFirst addSecond
python3 claude/harness/shape/names.py src/scriptModule/script_interface.hpp => script_interface.hpp 35: ScriptInterface ~ScriptInterface getSelectedScript getSelectedScriptDirectory getScriptPath slowerSpeed fasterSpeed defaultSpeed resumeScript cancelScript pauseScript cancelRecordScript initScriptIterator setScriptNbrLoop setScriptLoop resetScriptLoop setScriptPath addScriptFirst playScript recordScript recordCommand annotate setSelectedScript setSelectedScriptDirectory getScriptList isScriptPlaying isScriptRecording isScriptPaused isSkipPauseDisabled setSkipPauseDisabled waitOnVideoTermination setWaitOnVideoTermination setIsVideoPlayed acquireGlobalLock releaseGlobalLock
python3 claude/harness/shape/names.py src/scriptModule/script_origin.hpp => script_origin.hpp 5: fromFile fromTcp valid lineText where
python3 claude/harness/shape/names.py src/eventModule/EventScript.hpp => EventScript.hpp 4: ScriptEvent ~ScriptEvent getFileName toString
python3 claude/harness/shape/names.py src/eventModule/EventScriptHandler.hpp => EventScriptHandler.hpp 4: EventScriptHandler ~EventScriptHandler handle loopDone
```

**WIP:** 2026-10-11 17:5x — MINTED (round supervisor, Claude Fable 5.1, session 44) on F142's accepted block, the owner's C1 / Q2 / Q6 defaults standing (no word since 2026-10-10; each one revert), F137 landed first so this one carries the dedup; NOT dispatched: waits on F148's acceptance and commit (the same tree, both build), its per-round lines (`REFRESH-AT-DISPATCH`) filled then.

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
