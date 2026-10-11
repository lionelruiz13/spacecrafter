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

**Update [Claude Fable 5.1 2026-10-10, supervising session 42 — LovelyFoxDev, Saturday afternoon, the owner present — THE ROUND HIS WORD OPENED: F143 (Opus) with F141 / F142 / F144 (Fable) beside it, F140 after F143's acceptance; his trigger = the §0b line verbatim]:** Warm-up [measured]: both trees clean at open; code `main` @ `38ec7199` UNCHANGED since the 09-27 round, harness `51b6d47`; binary `fe5f0c24` at HEAD; `deploy.sh --check` CURRENT (exit 0); no owner commit; `review.sh status`: 3 commits waiting (his mark at `ee185d4f`). A NEW BOOT (07:33:26, the ninth): the canary's quick arm red on exactly the per-boot members → re-banked at warm-up (harness `1be13c6`, HOST-EVENTS entry), FULL canary 12 / 12 on `fe5f0c24` (`20261010-135138`) — the round's BEFORE. Pass 26 = update-s41 (close = depart). DISPATCHER DEFECTS found at the gate refresh, corrected before any dispatch: (1) the three analysis-only sections carried the code-task template's per-round lines (the whole tree clean, the binary's md5, `find -newer`) while minted to run BESIDE F143, which edits `src/` and builds — premises the task does not stand on, whose failure would have aborted them one by one; replaced by the clean state of each task's own READ set, the binary lines dropped; (2) their untracked-file exclusions named only the entries minted before them (`11.31[01]`, `11.31[012]`), so the sibling agents' entry files would have failed each other's gate — widened to `11.31[0-4]` in all four; (3) F143's block had no environment line for a task that launches the engine — the instance assert and the quick canary arm added (F137's form); its mandate (4) said "the HOST-EVENTS re-bank protocol first" while its boundaries forbid re-banking — the mandate now says STOP and report (the re-bank is the supervisor's act, §11.174(h)). The class of (1)–(2): per-round lines COPIED from the previous section at the mint instead of DERIVED from what the task stands on and from who runs beside it — the mint's filler takes a command list, so the fix is upstream of the tool: the list is written per task. ROUTING at dispatch [derived, veto-open]: the sections' `claude` / `fable` resolves to the `fable-asi-max` agent type — §11.302(j) names the fable-asi projections as the fable agents under his prompt (the first-line gate `prompt-window: present` is theirs too, unexercised until this round), and the three structure tasks are consolidation over many coupled sources (Q-08: effort tracks structural-information load). ROUND OUTCOME (interim 18:2x; the close appends F140's): DISPATCHED 14:00 — F143 (`opus-xhigh`), F141 / F142 / F144 (`fable-asi-max`), the four together; ALL FOUR CUT by the API session limit at ~14:25 (HTTP 429, the 5-hour window exhausted in 25 min by the fan-out; reset 17:40) and RESUMED 17:42 by message in their own contexts after the trees were re-verified (12 dirty files = F143's write list, the binary its rebuild, no engine) — the first measured resume of a cut subagent (Q-30 (e)). FOUR FOR FOUR DELIVERED (every return's first line `prompt-window: present` — the executor-under-his-prompt path exercised) and ACCEPTED: F143 §11.310 → code `9e0ca53c` (the diff read in full, every gate re-run by the supervisor, P6 not reached — the scissor is square by construction, §5.189's reach corrected); F141 §11.311 (the control = his `vzm` + `updateMove`; N11 his), F142 §11.312 (`KeyBinding`, one lookup door; §5.190 / §5.191 minted), F144 §11.313 (`followInfluence`, `aimed`, the `3c87a51d` rule retired; the galaxy boundary a STOP, §5.192 minted). Pass 27 (the four sections) right after the acceptance commit. F140 DISPATCHED 18:12 on F143's acceptance (its premises refreshed; three star-module lines re-pointed; P2's `Camera.cpp` clause pre-empted; the count premise replaced by its own-section check). Three dispatcher defects of one class → §0b.3's rule (a mandate's structural claim is a PREMISES line), Q-89 updated. Section 3 carries his list. **F140 STOPPED AT READING 18:21 (§11.314, nothing mutated), ENDORSED 18:25: the mint ordered his two-step unit with a boundary and a block covering step 1 only; `fovs per frame` is no canary member; P10 consumed the name F143 removed and the law's centre had no holder — a fourth defect of the round, the mint's (§0b.3's rule gains its clause: WRITES from ALL the steps a mandate names). Decisions veto-open (§3 D1–D4, H6-b): step 2's STRUCTURE first = **F145** (Fable max, minted ~18:40, dispatched); the Renderer holds the centre; the gate re-cut to the old pin's bit-identity; the reading = the S7 count + a mid-zoom capture. F140 HELD until F145's block, then refreshed over both blocks. F137 UNBLOCKED by the STOP (no shared-file conflict tonight) and DISPATCHED on F141's block with the defaults (N11 disclosed, revertible). Pass 28: the session-40 / 39 "For Vixy" bodies left (residue extracted); the archive tool gained the `forvixy-s<n>` unit form; section 3 still holds ~50 historical blocks (~2 400 lines) — the census pass is the next FOCUSED candidate.** **CLOSE (2026-10-11 07:4x): the second window (17:40 → 22:40) was exhausted by ~18:50 and cut F137 (group B half-edited), F145 (its first line) AND the supervising session; all three resumed in context at 22:41 (Q-30 (e2)); F145 was cut once more by the Fable-side allowance while writing and resumed from its checkpoint after the owner's reset (07:2x). F137 DELIVERED 00:4x and ACCEPTED 07:35 — every gate re-run by the supervisor, three commits `34202cf2` / `bbe278b2` / `2cb05825` (author Opus 5.5), §5.164 / §5.188 closed, `b21` re-banked, N11 built as predicted and disclosed. F145 DELIVERED 07:28 and ACCEPTED: 26 members, the adapter, 58 files — the delivery-unit datum for the owner (§3 D1'); F140 HELD on his word. Round total: SIX deliveries (F143 code, F141 / F142 / F144 / F145 structures, F137 code in three commits), ONE STOP endorsed (F140), FOUR code commits on `main` (F143's one, F137's three), `review.sh status` 7 commits waiting at his mark. Pass 29 (F137, F145) in the commit right after. Next round: F140 on his unit word; F142's and F144's implementations minted on their blocks + his answers; the §3 census pass; B48's first leg.** **THE DISPATCHABLE SET FOR A FRESH SESSION (his question of 2026-10-11 morning, answered here so the next warm-up reads it; D1' RULED the same morning — two deliveries in one round): (1) F140 refreshed as STEP 1 and F146 minted as STEP 2 (both Opus; the mint inputs are named in F140's WIP line; F146 after F140's acceptance, O6 verifying the pair); (2) F142's IMPLEMENTATION on its block (`INTENT/11.312.md`, 32 P lines; Opus; defaults C1 / Q2 / Q6 built unless he rules; it carries the release-switch dedup after F137 and closes F137's F2 — the press identity); (3) F144's IMPLEMENTATION on its block (`INTENT/11.313.md`, 30 P lines; Opus; H2 / H3 / H8 defaults; `Camera.cpp` and `ssystem_factory.*` are F140's too — sequential with it); (4) the SECTION-3 CENSUS pass (FOCUSED, Opus: ~50 historical decision blocks, each item resolved-with-pointer or carried into the live list, then the body leaves — the archive tool takes `forvixy-s<n>` units); (5) F135's owed KEY-form leg of observation 4 (F141 P33; F137's F1 names the driver form); (6) B48's first leg (the video instrument, §11.302(g)); (7) small: the four command words into `util/scedit/grammar` (F137 F6, the SS channel), README F124's stale "two red as found" (F5), the obliquity / galactic literals' dedup commit after the fov slice (F145 n9(ii)). Sizing: 3 per session is the measured sweet spot, and each max-effort fan-out of 3–4 exhausts a 5-hour window in 25–70 min (Q-30 (e)(e2)) — dispatch in pairs, resume by message on a cut.**

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

---

### F146 — THE FOV SLICE, STEP 2 OF TWO IN THIS ROUND (dispatched on F140's ACCEPTANCE; the pair verified together by O6; nothing ships between): THE EIGHT FRAME-THREAD SKY LAYERS (+ the GRID, D5) DRAWN THROUGH ONE INTERFACE, IMPLEMENTED against F145's PREDICTIONS block (28 lines, `INTENT/11.315.md`; judged with F139's P1–P25 as amended in (m)): the interface `SkyView` — 26 pure virtuals, the old engine's declared names (S5), in `src/coreModule/skyView.hpp` (the NAME his: `SkyProjection` / `ViewedSky` the alternatives); the OLD half ONE adapter `OldSkyView {prj, nav}` forwarding each member verbatim (`projector.*` / `navigator.*` BYTE-IDENTICAL — F139's P2 kept in its strongest form); the NEW half `CameraSkyView` over F139's `CameraProjection` (+ `projectEye`, the one radial law from eye space), composed ONCE per frame in `EnvironmentManager::update` before the members' loops from PUBLIC Camera primitives (no `Camera.hpp` hunk), `StarFieldEnv::update` copying from it (P27); the Renderer's px label entry `printGravityPx` (F145 (g)'s third hunk); `Core` holding the adapter, `Core::drawnView()` the gated reader beside `drawnFov`; the 56 draw-path declarations + 2 `proj_func` types retyped in 14 headers, ~130 receivers in their sources, the ARITHMETIC verbatim; the three seams of (h) — the pointer VALUE-FED, the nebula's on-screen size ONE formula, `printHorizontal` over the view with its old entry a forwarder; the executor modules above the system pass `*core->oldSkyView` UNGATED (§11.269: no sky layer moves above a system); D5 the grid INCLUDED (0 new members); THE INSTRUMENT O6 built in three harness files (`f131_run.sh 6 both`: the equator LINE's radius over the grid's equator RING's radius at `cap_mid` — 1.00 ± 0.02 after both steps, outside [0.95, 1.05] after step 1 alone, 1.00 on the old pin throughout); every prediction CONFIRMED / REFUTED / NOT REACHED, every deviation reported [L, **CODE under `src/` within the 50 files below and no other, + 3 harness files; the agent may BUILD (alone in the tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE while it runs; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): the block names files, functions, +/- lines, the gates); entry number ALLOCATED: **§11.317**; the supervisor commits in two per F145 (l) — "the sky through one view", then D5's grid]

**Why now / mandate:** the owner's unit (§11.307(b)(2): both steps, star field first; §11.314 (i): two deliveries in one round, verified together) — step 1 alone leaves the constellation lines, sky lines, hints and pointer lagging the stars they join by up to 26.8 deg of fov for 2–4 frames per click (F139 (f6)). F145 structured this step from the eight layers' ACTUAL calls (26 members, no more, no less — its (b)(c)(d)), chose the adapter with the bit-identity argument per member ((e)), composed the new half from public Camera primitives ((f)), extended D2 ((g)), placed the three seams ((h)), re-cut the gate ((i)), named the instrument ((k)) and predicted the diff ((l)); ACCEPTED 2026-10-11. You IMPLEMENT its block and report EVERY conflict. The names are HIS to give; built as F145 proposed unless the WIP line below carries his word: `SkyView`, `OldSkyView`, `CameraSkyView`, `Core::drawnView`, `Renderer::getMvp2D` / `printGravityPx`, `SSystemFactory::getEnvironmentSkyView`, `EnvironmentManager::getSkyView` — and the defaults D2' (the MVP + the px entry), D5 (the grid), the adapter over two interfaces, the pointer value-fed (B-2), `printHorizontal`'s navigator-less forwarder — each one revert.

**The ORIGIN of the need:** F145's section, by reference (one dome, ONE fov and ONE frame per frame for every CPU-projected sky layer; a later layer takes the interface and nothing else; the old engine stays what it is through the same interface's old implementation). NOT this slice: the images (class B), the labels' fixed-fov dome law (reproduced, the TODO named), the illuminates' cull, `starLines`, the Object API (B42's query table).

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F146`, refreshed at dispatch on step 1's accepted state), then READ IN FULL: `claude/SHAPE.md`; `INTENT/11.315.md` whole (the census (b), the split (c), the interface (d), the adapter (e), the new half (f), the Renderer (g), the layers and the seams (h), the gate (i), the manager table (j), the gates and O6 (k), the diff (l), the amendments (m), his items (n), the WRITES (p), the block); `INTENT/11.316.md` whole (step 1 AS BUILT: the law object's and snapshot's final names, the Renderer accessors' names, where the composition lives today — what you move into `CameraSkyView::update`); `INTENT/11.301.md` table (c) rows 2–10, (e)(a), (f6), the block; `INTENT/11.314.md` (b)1–3; `INTENT/11.269.md` (b)(d) (why nothing moves above a system); the code from the current checkout: the 14 layer headers WHOLE + `skygrid.hpp`, `skygrid_mgr.hpp`; their sources at every receiver site ((b)'s file:line); `src/coreModule/projector.hpp` and `src/navModule/navigator.hpp` whole (the 26 signatures the interface COPIES — Vec3d in / out, `int x, int y`, `Vec4f Color` by value, `const Vec3d &` back from `getPrecEquVision`); `Renderer.{hpp,cpp}`, `core.{hpp,cpp}` (`drawnFov`, `uboCamUpdate`, `:102` the Navigator's construction; F137's `updateMove` and readers are not yours), `EnvironmentManager.{hpp,cpp}`, `ssystem_factory.hpp`, `CameraProjection.hpp`, `StarFieldEnv.cpp`, `Camera.hpp` (read-only: `observedToLocalPos`, `observedToBodyEquPos`, `oldLocalToLocal`, `getCurrentSystem`, `getReferenceBody`), `ModularBody.hpp:270–290, 955–970` (`computeAxisRotation`, `getMat`), `anchor_point_body.cpp:28–80` (the spin's sign — P6's loud check), `navigator.cpp:255–270` (`mat_dome_fixed`), `projector.cpp:565–600` (`projectCustomFixedFov`), `atmosphere.cpp:237–252` (the module's inverse), the executor modules at their layer calls; the harness: `f131_run.sh`, `f131_obs.py` (`obs5` at `:192`), `f131_analyse.py`, `f128_leg.sh` / `f128_compare.py`, `trio_census.py`. (1) The S4 pass per touched class from the names in the block — the interface's members are the Projector's / Navigator's declared names (S5), consumed; a name that exists for the same EFFECT is consumed, never doubled. (2) BUILD THE INSTRUMENT FIRST (F145 (k)): `obs6()` beside `obs5()` in `f131_obs.py`, rows `line_over_grid_cap_mid` (gate `near 1.00 +- 0.02`) and `line_over_grid_cap_rest60` (note) in `f131_analyse.py`, `f131_run.sh`'s usage `<1-6|all>`; RUN IT on the step-1 binary as the BEFORE: new pin predicted OUTSIDE [0.95, 1.05] at `cap_mid`, old pin 1.00 ± 0.02 — paste both. (3) BUILD F145's block in its order, one group at a time, a build + `f70_ascii.py gate` + a checkpoint line in your entry after each group: (A) `skyView.hpp` (the class-top block ≤ 4 lines, ~8 what-fors, 26 pure virtuals), `oldSkyView.hpp` (26 one-statement forwarders, no arithmetic), `CameraProjection::projectEye` with `project` composing it (P7), `CameraSkyView.{hpp,cpp}` (P5 / P6: the matrices of (f), `update` wired in `EnvironmentManager::update` between `system = camera.getCurrentSystem()` and the first `m->update(...)` — P28; `StarFieldEnv::update` copies `law`, `j2000ToLocal` and the cone's axis from `getSkyView()` — P27), the Renderer's `printGravityPx` split (P8's third hunk), `Core::oldSkyView` built right after `navigation` + `Core::drawnView()` beside `drawnFov` (P13), `SSystemFactory::getEnvironmentSkyView()`, `EnvironmentManager::getSkyView()`; (B) the 14 headers + sources RETYPED (P9 / P10: receivers and parameter types only, every expression verbatim; `&Projector::x` → `&SkyView::x` at the 24 `proj_func` sites; the includes / forward declarations as (h); the three seams: the pointer `drawPointer(int, const SkyView &, const Vec3d &earthEquPos, float onScreenSize)` with its two values computed at the four executor call sites (P11), the nebula's private `onScreenSize(const SkyView &)` with the API override a one-line forwarder through `OldSkyView(prj, nav)` (P12), `s_font::printHorizontal` over the view with the `const Projector *` entry a forwarder through `OldSkyView(prj, nullptr)` (P12)); the Solar level's eight call sites → `core->drawnView()`, the three modules above the system → `*core->oldSkyView` (P14); (C) D5: `skygrid*` retyped, `solarSystemModule.cpp:196` → `drawnView()` (P15). Keep a per-group file list and `git diff -- <files>` for (A)+(B) and (C) so the supervisor commits in two. (4) GATES, both pins, by your own hand, the step-1 binary as the BEFORE: the FULL canary (both arms unchanged at rest, the old arm delta 0.0 — a per-boot red is a STOP); `f131_run.sh 5 both` and `6 both` (P17 / P19 / P20); `f128_leg.sh SOL` (`old_vs_stored_old` rel 0.0, max delta 0); `f124_run.sh` leg Z, `f125_fovlaw.py`, `s33_refchange.py` ALL PASS (unchanged by this step); P21's bounded cross-pin check of the sky-line layer (an f128-style ON-minus-OFF difference on the sky lines, both pins at rest — an EXTENSION of f128 for a reading it cannot take, allowed); P24's pointer-on-star rest capture; `trio_census.py -v` before / after (P22's numbers). (5) Shape numbers: `git diff --numstat` per file against (l); `names.py` on the four new headers (P3: `SkyView` + 26 + the destructor) and on every retyped header; the residual greps of P9 (sum 39) and P10 (nebula.hpp 1, nebula.cpp 2, the rest 0); `writers.py` on `Projector` unchanged; `git diff --stat -- src/coreModule/projector.* src/navModule/navigator.*` EMPTY (P2); the manager table (j) re-derived — fovs per new-mode frame at the Solar level 2 → 1, the Navigator's frame set 12 → 3. (6) The entry `claude/INTENT/11.317.md`: checkpoint state in its FIRST LINES, refreshed after each group; every P1–P28 and every amended F139 line judged with evidence; every deviation with its argument; the names flagged for him; the O6 readings before / after both pins; what the next slice (B47's conversions above the system; the media) must know. (7) The report, per the definition's form.

**REFRESHED AT DISPATCH (2026-10-11 ~11:0x, session 43, on F140's acceptance — code `a7919278`, binary `8ac8aedc`; step 1's record `INTENT/11.316.md`, read it WHOLE, its (7) first):** (i) the law object's FINAL form: `CameraProjection {Mat4d localToEye; float halfFov; int mode; float radius, cx, cy; bool project(const Vec3d &local, Vec3d &win) const}` — `localToEye` is `Mat4d` (not F139's `Mat4f`), ONE operation, NO `projectCheck` (collapsed into `project`); P7's `projectEye` and `direction` go beneath it. (ii) the snapshot `StarFieldInput {fovDeg, altitudeM, ConvexS cone{1}, Mat4d j2000ToLocal, CameraProjection law}` lives in `hip_star_mgr.hpp`, held as public `EnvironmentManager::starInput`, read through `SSystemFactory::getEnvironmentStarFieldInput()`; the policies `OldStarFrame` / `CameraStarFrame` live in `zone_array.hpp`; `HipStarMgr::sizeRolloff(fov)` is the one roll-off law. (iii) the Renderer holds `getViewportCenter()` (`const Vec2f &`) and `getMvp2D()` (`const Mat4f &`) beside `getViewportRadius()`; `printGravityPx` is NOT built — yours (P8's third hunk). (iv) THE FRAME: `StarFieldEnv::update` composes in the CAMERA's own local axes — `eyeToLocal` from three `observedToLocalPos` calls, `j2000ToLocal = eyeToLocal * rotationOf(system.getMat()) * mat_j2000_to_vsop87`, `law.localToEye = eyeToLocal^T` — with NO `oldLocalToLocal` permutation (the stars match the old pin within 0.055 px so). F145 (f) puts `P` into the view's local members so the layers' OLD-local vectors (the cardinals' north `(-1, 0, 0.12)`) land right; when P27 makes `StarFieldEnv` copy `law` and `j2000ToLocal` from the view, the PAIR must stay consistent (both carry P, or neither); a mismatch shows as the stars turned by the permutation — loud, P20's class. (v) the cone's precondition binds any search you build: the grid search is exact for half-spaces with `d <= 0` only (§11.316 (1b)1). (vi) THE ORDER DEFECT, §5.193: the executor's async job starts before `EnvironmentManager::update`, so the star SNAPSHOT is read a frame late or torn; your frame-thread layers read the view in `draw`, after `update` — unaffected; do NOT mitigate it. (vii) `trio_census.py -v` after step 1 (F145's P22 counted from `9e0ca53c`): Projector 227 calls, `getFov` 42, `projectLocal` 14; Navigator `earthEquToLocal` 20, `j2000ToEarthEqu` 9 — P22's AFTER numbers shift by −1 each against F145's. (viii) f131's w-bank (`f131_analyse.py` rows `5 nat old/new max_abs_delta_w1..w9_deg`) is the PRE-H6 law's: H6 moved the two engines' mid-click separation on BOTH pins (old w1 26.10 → 26.81 / 26.88, w4 10.05 → 9.79 / 9.50 — one FAIL at the band's edge by the supervisor's hand, §11.316's acceptance): in your BEFORE run paste the as-written rows, then RE-FORM those expectations with your BEFORE run's numbers and the argument (H6) — F137's form, never a widened tolerance; the rest rows and the ring's `fov_table` are untouched by H6 and by you. (ix) `Camera.hpp` stays NON-change: §5.194 (the uninitialised zoom members) and D11 (`calculateZoomCoefVelocity` callerless) are recorded there, not yours. (x) the canary's new-path members read 165.260 / 6.647 (+0.002 / +0.003 against the bank) in every FULL run since the ninth boot — pre-existing; P18 is judged as UNCHANGED against 165.26 / 6.647, not the bank's digits. (xi) `f128 SOL`'s new-pin floor is launch-dependent on this boot (1 of 5 runs today at rel 0.000139, max 2 grey on the band): a red there is read with a second run, never re-banked. (xii) the two blank lines a commit of yours would leave: none — you commit nothing; the supervisor commits in two per F145 (l) (the sky through one view; D5's grid), so keep (C)'s diff separable.

**Boundaries:** WRITES — F145 (p)'s step-2 list (43): `src/coreModule/skyView.hpp`, `oldSkyView.hpp` (new); `src/experimentalModule/CameraSkyView.{hpp,cpp}` (new); `src/experimentalModule/EnvironmentManager.cpp`; `src/experimentalModule/Renderer.{hpp,cpp}` (the px entry only); `src/coreModule/core.hpp`; `src/coreModule/{nebula,nebula_mgr,constellation,constellation_mgr,skyline,skyline_mgr,skyDisplay,skydisplay_mgr,meteor,meteor_mgr,cardinals}.{hpp,cpp}`; `src/tools/{object_base,object,s_font}.{hpp,cpp}`; `src/executorModule/{stellarSystemModule,inGalaxyModule,inUniverseModule}.cpp`; `src/coreModule/{skygrid,skygrid_mgr}.{hpp,cpp}` — PLUS the seven step-1 files (l) names for step 2 (the mandate's second step derives its WRITES from BOTH blocks, §0b.3): `src/experimentalModule/CameraProjection.hpp` (`projectEye`), `src/experimentalModule/environmentModules/StarFieldEnv.cpp` (P27), `src/experimentalModule/EnvironmentManager.hpp` (`skyView`, `getSkyView`), `src/coreModule/core.cpp` (the adapter, `drawnView`), `src/bodyModule/ssystem_factory.{hpp,cpp}` (`getEnvironmentSkyView`), `src/executorModule/solarSystemModule.cpp` (the call sites) = 50 code files; HARNESS: `claude/harness/f131_obs.py`, `f131_analyse.py`, `f131_run.sh` (O6 only; a gate EXTENDED for a reading it cannot take, never narrowed), `claude/INTENT/11.317.md`, `claude/harness/artifacts/f146/**`. NON-CHANGE, a gate (F145 (i)): `projector.*`, `navigator.*` BYTE-IDENTICAL (P2); `Camera.hpp`, `ModularBody.*`, `ubo_cam.*`, `shaders/`, `src/mediaModule/*`, `src/bodyModule/*` but the factory, `src/uiModule/*`, `src/interfaceModule/*`, `starLines.*`, `illuminate*`, `oort.*`, `inSandBoxModule.cpp`; the 35 Object-API declarations and their overriders in `body.hpp` / `ModularObject.hpp` / `hip_star_wrapper.hpp` (P9's STOP). No install, no stash, no commit.

**STOP conditions:** a premise fails; P3's member count refuted at reading (a 27th member a layer calls on its draw path — name the site); a retyping needs an Object-API overrider outside WRITES; the old pin moves on any gate (P17 — the diff names the hunk; never re-bank); O6 cannot be built on this host (the grid or the equator line does not draw in the capture scene — paste the capture); the canary red on the BEFORE binary for a reason other than a boot.

**Discriminating checks:** (a) every P line has a verdict with evidence; (b) `git diff --stat -- src/coreModule/projector.* src/navModule/navigator.*` EMPTY; (c) the old pin bit-identical on the four gates; (d) O6 before (new pin outside [0.95, 1.05]) / after (1.00 ± 0.02), old pin 1.00 ± 0.02 both times; (e) P9's and P10's residual greps; (f) `f70_ascii.py gate` PASS; (g) the numstat against (l), ±30 %; (h) the manager table (j) re-derived on the diff.

**Preconditions (checkable, §0.7):** the PREMISES block (refreshed at dispatch); F140 (step 1) ACCEPTED and COMMITTED by the supervisor, the binary rebuilt at that commit (its WIP line; the BEFORE of this step); F145 ACCEPTED (`fable-dispatch/archive/F145.md`).

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch (after F140's acceptance), never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => a7919278
git status --porcelain | wc -l => 0
git -C claude status --porcelain | /usr/bin/grep '^??' | /usr/bin/grep -v 'INTENT/11.317.md' | tr '\n' ' ' => 
md5sum build-claude/src/spacecrafter | cut -c1-8 => 8ac8aedc
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.317.md -a ! -e claude/INTENT/archive/11.317.md && echo free => free
/usr/bin/grep -c '^### F146 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
# the engine slot, the boot's fingerprint (ninth boot; a red here is a STOP to report, never a re-bank by the executor), the field pair
bash claude/harness/sc_instances.sh --assert f146-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
test ! -e ~/.spacecrafter/beta_features.ini && echo absent => absent
# the structure this step implements, pinned (F145's 28 lines; F139's block as amended in (m)); step 1's record, pinned
md5sum claude/INTENT/11.315.md | cut -c1-8 => a820b68a
sed -n '/^PREDICTIONS F145 -> F140/,/^P28 /p' claude/INTENT/11.315.md | md5sum | cut -c1-8 => fc46a7e7
sed -n '/^PREDICTIONS F145 -> F140/,/^P28 /p' claude/INTENT/11.315.md | /usr/bin/grep -c '^P[0-9]' => 28
md5sum claude/INTENT/11.316.md | cut -c1-8 => 69e245b1
# step 1 LANDED (the three commits; the law object, the member, the snapshot's holder, the policies' home, the Renderer's two accessors; no projectCheck / projectEye / printGravityPx yet; the Camera's local frame without the permutation)
git log --format=%h -3 | tr '\n' ' ' => a7919278 c2d24732 559191a8
test -e src/experimentalModule/CameraProjection.hpp -a -e src/experimentalModule/environmentModules/StarFieldEnv.cpp && echo present => present
python3 claude/harness/shape/names.py src/experimentalModule/CameraProjection.hpp => CameraProjection.hpp 1: project
/usr/bin/grep -c 'Mat4d localToEye' src/experimentalModule/CameraProjection.hpp => 1
/usr/bin/grep -c 'getViewportCenter\|getMvp2D' src/experimentalModule/Renderer.hpp => 2
/usr/bin/grep -c 'printGravityPx' src/experimentalModule/Renderer.hpp => 0
/usr/bin/grep -c 'observedToLocalPos' src/experimentalModule/environmentModules/StarFieldEnv.cpp => 3
/usr/bin/grep -c 'oldLocalToLocal' src/experimentalModule/environmentModules/StarFieldEnv.cpp => 0
/usr/bin/grep -n 'StarFieldInput starInput' src/experimentalModule/EnvironmentManager.hpp => 52:    StarFieldInput starInput; // Written by StarFieldEnv::update
/usr/bin/grep -c 'struct OldStarFrame\|struct CameraStarFrame' src/starModule/zone_array.hpp => 2
# the 14 headers: 92 declarations taking the old engine's objects (56 draw-path, 35 Object API, 1 comment - 11.315 (c)); untouched by step 1
for h in src/coreModule/nebula.hpp src/coreModule/nebula_mgr.hpp src/coreModule/constellation.hpp src/coreModule/constellation_mgr.hpp src/coreModule/skyline.hpp src/coreModule/skyline_mgr.hpp src/coreModule/skyDisplay.hpp src/coreModule/skydisplay_mgr.hpp src/coreModule/meteor.hpp src/coreModule/meteor_mgr.hpp src/coreModule/cardinals.hpp src/tools/object_base.hpp src/tools/object.hpp src/tools/s_font.hpp; do /usr/bin/grep -c 'Projector\s*\*\|Navigator\s*\*' $h; done | paste -sd+ | bc => 92
test ! -e src/coreModule/skyView.hpp -a ! -e src/coreModule/oldSkyView.hpp -a ! -e src/experimentalModule/CameraSkyView.hpp && echo absent => absent
# the frame half is the Navigator's inline products; the two flags O6 needs exist; obs6 does not
/usr/bin/grep -c 'Vec3d earthEquToLocal\|Vec3d j2000ToEarthEqu\|Vec3d helioToEarthPosEqu' src/navModule/navigator.hpp => 4
/usr/bin/grep -c 'equatorial_grid\|equator_line' src/interfaceModule/base_command_interface.hpp => 5
/usr/bin/grep -c 'def obs5' claude/harness/f131_obs.py => 1
/usr/bin/grep -c 'def obs6' claude/harness/f131_obs.py => 0
# the call sites above the system that no source named before F145 (h)
/usr/bin/grep -c 'core->projection, core->navigation' src/executorModule/stellarSystemModule.cpp src/executorModule/inGalaxyModule.cpp src/executorModule/inUniverseModule.cpp | tr '\n' ' ' => src/executorModule/stellarSystemModule.cpp:14 src/executorModule/inGalaxyModule.cpp:6 src/executorModule/inUniverseModule.cpp:4
# f131's w-bank is the PRE-H6 law's (nine rows per pin; the re-form is this task's, F137's form)
/usr/bin/grep -c '^5   nat old  max_abs_delta_w' claude/harness/f131_analyse.py => 9
/usr/bin/grep -n '^5   nat old  max_abs_delta_w4_deg' claude/harness/f131_analyse.py => 149:5   nat old  max_abs_delta_w4_deg                 gate  rel         10.0533      5%
# S4: the two old headers whose declared names the interface copies (S5); core.hpp by count (F137's 144)
python3 claude/harness/shape/names.py src/coreModule/projector.hpp => projector.hpp 78: stringToProjectionType projectionTypeToString Projector ~Projector getViewportCenter getViewportFloatCenter getViewportRadius setViewportDisk getViewportPosX getViewportPosY getViewportWidth getViewportHeight getViewport applyViewport setFov getFov setMaxFov getMaxFov getAimFov changeFov updateAutoZoom zoomTo setClippingPlanes getClippingPlanes checkInViewport checkInMask setModelViewMatrices projectEarthEqu projectEarthEcliptic projectJ2000Galactic projectEarthEquFixed projectEarthEquCheck projectEarthEquLineCheck unprojectEarthEqu unprojectJ2000Normalized projectJ2000 projectJ2000Check projectJ2000LineCheck projectHelioCheck projectHelio projectHelioLineCheck unprojectHelio projectLocal projectLocalCheck unprojectLocal unprojectNormalizedLocal projectDome projectDomeFixed projectCustomFixedFov fisheyeProjectCustom allsphereProjectCustom ekisolidProjectCustom asphericProjectCustom projectCustom projectCustomCheck projectCustomLineCheck sVertex3 sVertex3v getCursorPosEqu printGravity180 unprojectViewport getMatProjectionOrtho2D getMatProjection getClippingFov getMatLocalToEye getMatJ2000ToEye getMatEarthEquToEye dumpTrace fisheyeUnproject allsphereUnproject ekisolidUnproject asphericUnproject fisheyeUnprojectNormalized allsphereUnprojectNormalized ekisolidUnprojectNormalized asphericUnprojectNormalized unproject unprojectNormalized
python3 claude/harness/shape/names.py src/navModule/navigator.hpp => navigator.hpp 51: Navigator ~Navigator updateTransformMatrices updateVisionVector updateViewMat moveTo setFlagTraking getFlagTraking setFlagLockEquPos getFlagLockEquPos getEquVision getPrecEquVision getLocalVision setLocalVision restoreVision getObserverHelioPos localToEarthEqu earthEquToLocal earthEquToLocalFixed earthEquToJ2000 j2000ToEarthEqu helioToLocal helioToEarthEqu helioToEarthPosEqu earthPosEquToHelio getHelioToEyeMat getEarthEquToEyeMat getEarthEquToEyeMatFixed getLocalToEyeMat getLocalToHelioMat getJ2000ToEyeMat getDomeFixedMat geTdomeMat updateMove setViewingMode getViewingMode setHeading setDefaultHeading getHeading setViewOffset getViewOffset setViewOffsetTransition getViewOffsetTransition getFlagAutoMove getMoveCoef getFlagChangeHeading changeHeading update lookAt alignUpVectorTo dumpTrace
python3 claude/harness/shape/names.py src/coreModule/core.hpp | wc -w => 146
```

**WIP:** 2026-10-11 ~09:00 — MINTED (round supervisor, Claude Fable 5.1, session 43) on the owner's delivery-unit word (§11.314 (i)) over F145's accepted block; NOT dispatched: waits on F140's (step 1's) acceptance and commit, its per-round and step-1-dependent lines (`REFRESH-AT-DISPATCH`) filled then. Defaults built unless he says a word: the names (`SkyView` and the six others), D2', D5, the adapter, the pointer value-fed, `printHorizontal`'s forwarder. **REFRESHED 2026-10-11 ~11:0x on F140's acceptance (code `a7919278`, binary `8ac8aedc`): the per-round and step-1-dependent lines filled by the tool, the REFRESHED-AT-DISPATCH paragraph with step 1 as built (§11.316 (7)), the f131 w-bank re-form added to its mandate (H6 moved the mid-click rows). DISPATCHED ~11:0x (`opus-xhigh` under the owner's prompt), alone in the code tree, the only engine launcher.**

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
