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

**Update [Claude Fable 5.1 2026-10-14, supervising session 46 — LovelyFoxDev, Wednesday morning, the owner PRESENT at the trigger (07:2x) with two rulings in the trigger line itself, then unheard]:** Warm-up [measured, 07:24]: both trees clean at open; code `main` @ `9d461bd5`, harness `9d484cd`; binary `81664e4e` AT HEAD (`make -n` compiles nothing); canary `--no-scene` `20261014-073049` exit 0, no engine live; `deploy.sh --check` CURRENT; both ledger instruments exit 0; `review.sh status` his mark `ee185d4f`, HEAD 124 files / 13493 code / ratio 0.07; live `### F` 0 at open; next §11.325 / §5.206. HIS WORDS recorded verbatim as §11.325 (`b9fc735`, 07:41): Q1 YES (a button is a `key`), Q9 (the 2022 odd slot was the ALTERNATE set), and the ruling that the joystick's alternate mode stays and the binding system reaches it through the DEFAULT action (the active one of the two sets) — read as NO plane for the joystick sources in the registry (reading B; B' / C named, one sentence flips it); back-markers at 11.324 (a)4 / (k), §17 K, row B49 in the same commit. MINTED (clock from `date` in the same call, Q-67): F154 (the joystick implementation on 11.324 (j) with P3 / P4 / P9 / P14 / P21 / P29 amended, §5.201 riding — FOCUSED, `opus-xhigh`, the only engine launcher), F155 (the scedit grammar in phase with the engine: F137's four `camera` actions + F149's `bind` family — FOCUSED, `opus-xhigh`, `util/scedit` only, its own build tree, no engine launch, parallel to F154), F156 (§5.204 — the drain re-tests the run's state: an explicit `script action end` ends once — FOCUSED, `opus-xhigh`, AFTER F154's acceptance: it extends the same driver). SUPERVISOR DEFECTS so far: Q-72 instance 9 (four `cd`-led calls of one parallel batch ran in another call's directory and failed loud on relative paths — the ninth instance, after re-reading the prevention; absolute paths from the next call on); two premise lines refused by the filler for multi-line output (re-formed, one dropped as redundant with the ini's md5).

---

**Update [Claude Fable 5.1 2026-10-13, supervising session 45 — LovelyFoxDev, Tuesday evening, the owner ABSENT (no word in any channel since 2026-10-10); his trigger = the §0b line verbatim]:** Warm-up [measured]: both trees clean at open; code `main` @ `9fc9145e`, harness `5720262`; binary `c444d249` AT HEAD (`make -n` compiles nothing); boot unchanged (the ninth, `2026-10-10 07:33:26`), canary `--no-scene` `20261013-214618` exit 0, no engine live; `deploy.sh --check` CURRENT; both ledger instruments exit 0; `review.sh status` his mark `ee185d4f`, 15 waiting; live `### F` 0; next §11.322 / §5.200. MINTED 22:03 (`321741f`; the clocks of this note are the commits' and `date`'s — F152's F6 caught the supervisor stamping from estimation, Q-67): F151 (§5.191's consolidation through the hook — FOCUSED, the row's "his to order" read as the one-structural-change-per-commit sequencing clause, one revert), F152 (observation 4 in its KEY form, 11.311 P33 — harness, after F151), F153 (the joystick row's structure — Fable, read-only; the finding at the mint: `joypad.ini`'s `command(...)` lists are already an authorable binding layer); gates 42 / 42, 33 + 4 REFRESH, 31 / 31. DISPATCHED 22:04 as a pair: F151 (`opus-xhigh`, the code tree, the only launcher) + F153 (`fable-asi-max`, read-only). F153 DELIVERED 22:33 (`INTENT/11.324.md`): one registry with a tagged key, the door three statements in the controller, his ini's keys as tokens, `-` the ini's action; C1 CORRECTED THE MINT (the joystick's plane is the eye-relative MODE copied each frame at `ui.cpp:921`, not a modifier); four facts beyond the mandate (the owner's own 2022 release branch for command buttons, removed 43 min later; the poll channel 26 field shows read; the dead `UI::handleJoyHat`); twelve questions his (Q1: is a button a `key` of 11.305 — gates the implementation). F151 DELIVERED 22:3x (`INTENT/11.322.md`): the move exactly as predicted (+7 / −7, 0 / −3, net −3, no header); the mint's reach reading REFINED (a loop's FIRST pass only), COMPLETED (an open accepting `struct if`) and REFUTED on its last link (the field's `initial.sts:17` `script action end` masks the HOME path — `configuration action load` shows the false `never closed` errors and `#!` tails written into the dead show); case O of the key leg BEFORE 5 / 2, AFTER 7 / 0; FOUND §5.203 (an absolute image path outside `~/.spacecrafter/<dir>/` ABORTS the engine — the first pre run's abort) and §5.204 (the hook runs twice on an explicit `script action end`, latent); the dispatcher's defect named: a FIELD-data claim in prose, no premise (Q-67; §0b.3 extended to the field, `85c3d12`). BOTH ACCEPTED 22:48 (`7ed6b45`): F151 re-run by the supervisor's hand (the key leg 49 / 0, N `521b5c3a`; codeident, D14, delivery.py), COMMITTED `f571664e` (author Claude Opus 5.5) + `9d461bd5` (the supervisor's: the hook's what-for `//! Clear what the ended script left behind`, comment-only, binary byte-identical `81664e4e`); F153's five structural claims verified against the code and git by the supervisor's hand, §5.200–§5.202 minted from its findings, 11.312 (k) Q14 back-marked (its default not taken); §5.191 CLOSED; B49 annotated; pass 42. F152 REFRESHED 22:50 (`a287e94`; two `ui.cpp` shifts the first refresh missed, the gate caught) and DISPATCHED 22:51 (`e35300e`), DELIVERED 23:4x (`INTENT/11.323.md`): the key form BANKED on both pins — 48 / 0 / 24 on `81664e4e` inside tolerances fixed at 23:05, before the first launch; P33's 0.44028 REFUTED on the old pin's drawn engine (23 km higher) and its 0.44036 / 8.807 REFUTED (0.44031 / 8.806 — F141's arithmetic), both predicted in writing; §5.184's second member GONE in the key form (the old observer's altitude held to the bit), the first STAYS silent; the command rows' verdicts identical; 13 launches, 0 lost; FOUND F1 (`judge 6` writes into its record — README pitfall corrected), F3 (the landing altitude 4.6 km lower on 1 of 17 records → §5.205; the re-bank NOT taken), F5 (the command form moves the old observer's altitude 23 km in the won state → §5.184), F6 (the dispatcher's clock stamps). ACCEPTED 23:5x: the key pair re-run by the supervisor's hand (26 / 0 / 12); the 39 records force-added; §5.184 / H10 / F-5 annotated; 11.311 P33 / (7), 11.299 (5) back-marked; pass 43. SUPERVISOR DEFECTS this session, recorded: Q-72 instance 8 (a lost `cd` in a parallel batch, the LAST call again, after reading the prevention — absolute paths from then on); an empty line variable fed to `sed`; three premise lines refused by the filler for a `>` token; a path re-serialized from memory (`/home/shared/`, failed loud); the prose's handler line numbers swapped in F153's mandate (the block was right); a FIELD-data claim without a premise (F151); CLOCK STAMPS at five sites typed from estimation (F152's F6) — every clock from here from `date` in the same call. ROUND TOTAL: three deliveries (F151 code, F153 structure, F152 harness), TWO code commits on `main` (`f571664e`, `9d461bd5`), passes 42–43, six §5 rows minted (§5.200–§5.205), §5.191 CLOSED; the dispatch file holds no live section at this close. The previous note (session 44) leaves with this write (pass 43, `update-s44`). NEXT ROUND'S DISPATCHABLE SET: the joystick implementation on F153's block (waits on his Q1 / Q2); §5.203 (his word: support the absolute image path or refuse it loud); §5.204 (the drain's double end — FOCUSED, small, a log-count gate); §5.201 (one `.empty()` test — FOCUSED, trivial); §5.205's dig (the travel's arrival frame — FOCUSED harness); the B45 harness dedup (one gdb error filter for f124_legs / f142_keys; the four `_clean` copies; the three older FIFO runners onto `f124_run.sh`; `judge 6`'s write out of the replay path); the scedit grammar words (F137's four, F149's `bind` family — FOCUSED, `util/scedit`); README F124's stale "two red" line (an F124 leg run to confirm); B48's first leg; and, his word: the one-constant dedup (the home), §5.193's order, §5.197 the hold inside `switchToBody`, the UNI banking, the harness ASCII gate (§5.196), §5.200 / §5.202 (intent or defect).

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
   with the pasted output), never prose alone; the filler takes the command. **[2026-10-13, session 45, F151's
   instance: THE SAME FOR EVERY CLAIM ABOUT THE FIELD the task's reach stands on — a script's lines, an ini's keys, a
   texture's presence: F151's P6 / P7 predicted a false error "at the next natural end (`initial.sts`)" in prose, the
   field's `initial.sts:17` is `script action end` and masked it, and the executor had to build a second leg to reach the
   observable; `tail -1 ~/.spacecrafter/scripts/internal/initial.sts => script action end` would have pinned the claim at
   the mint. A text about the field consumed as the field is Q-67's class on data (its instance of 2026-10-13).]** **[2026-10-14, session 46, F155's instance: THE SAME FOR THE STATE OF EVERY INSTRUMENT the task's gate stands on — a test suite's verdict at HEAD, a record's presence, a tool's build: F155's P5 predicted "ctest green BEFORE" for a gate nobody had run in 26 days; `ctest --test-dir util/scedit/build 2>&1 | tail -1` at the mint would have shown `anchor_gate (Failed)` one hop before an executor spent its run finding it. A text about the instrument consumed as the instrument — the third member of the class (code 10-10, field 10-13, instrument 10-14).]** A claim that cannot be a
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
   never across the section boundary. **The engine's citations (2026-10-14, §5.207 / F158 §11.331): at every acceptance whose stat touches `src/`, run `python3 util/scedit/tests/anchor_gate.py` from the code root (1.5 s, no build) — exit 0, or the delivery re-anchors what it moved (`claude/harness/f75_anchors.py --round <id>`: `pins` / `map` / the read table / `sweep` / `verify` / `flips`) or re-records with `--record --why "<argument>"`, in the SAME commit; a sweep also re-records the five scedit gates that print anchors (`ui_selftest`, `lint_rules`, `history_list`, `check_json`, `doc_queries`), digit-stripped identity the criterion. The reason: the grammar is a derived contract of the engine and its gate was bound to no engine event for 26 days (I3 at the instrument).** **Then the section LEAVES (2026-10-10, Q-110): the
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

---

### F156 — §5.204: THE DRAIN RE-TESTS THE RUN'S STATE — `ScriptMgr::update`'s `while (wait_time==0)` loop (`script_mgr.cpp:306`) stops when the command it just ran ended the run (`scriptState` left `PLAY`), so an explicit `script action end` line ends the run ONCE: today W_END → `cancelScript` cleans the queue inside the drain, the drain's next `getFirst` finds it empty and takes the natural-end branch → `terminateScript` → `executeCommand("script action end")` → W_END again — the end-of-run hook `scriptEnded` (the ONE home F151 made, 11.322) runs twice, idempotent today, wrong for the first non-idempotent line it will hold. One condition on one line; the natural end (an empty queue) unchanged; a loop body's `script action end` (the `repeatLoop` branch :316) covered by the same test. Gate: the key leg's next case — the count of `ScriptMgr: script end` per explicit end 2 → 1, the HOME path's `initial.sts:17` end once, A–AD unchanged [S, **CODE: `src/scriptModule/script_mgr.cpp` and no other file + harness; the agent may BUILD (alone in the code tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): one line, one case, a count that can fail); **DISPATCHED AFTER F154's ACCEPTANCE** — it extends the same driver (`f142_keys.py`) and the per-round lines below refresh then; entry number ALLOCATED: **§11.328**]

**Why now / mandate:** §5.204 (INTENT.md, the row, measured 2026-10-13 by F151 §11.322 (8) F2): *"An explicit `script action end` LINE ends the run TWICE, so the end-of-run hook runs twice: W_END runs inside `ScriptMgr::update`'s drain (`script_mgr.cpp:339`), `cancelScript` cleans the queue, and the drain — which does not re-test `scriptState` — finds the empty queue at its next `getFirst` (`script.cpp:183–187`) and takes the natural-end branch → `terminateScript` → `executeCommand("script action end")` → W_END again ... LATENT — every effect of the hook is idempotent today ... A non-idempotent line added to the one home (a "show ended" notification, a counter) would fire twice: the drain re-tests `scriptState` after W_END, or the line waits. OPEN — the next task on the hook."* 11.322 (10): *"keep its effects idempotent or fix the drain first"*. The session-45 close listed it dispatchable (*"§5.204 (the drain's double end — FOCUSED, small, a log-count gate)"*); section 3's item 5.204 offered the owner *"a small fix in ScriptMgr::update's drain, or left until a non-idempotent line needs it"* — he has not spoken; BUILT by I6 (rework at the earliest: the home exists, its first non-idempotent line is a matter of time, and the fix is one condition at the anchor), one revert, flagged in section 3.

**The ORIGIN of the need (S10, the supervisor's):** F151 made `scriptEnded` the ONE home of what a run's end leaves behind, so that a future end-of-run effect is one line there. A home that fires twice per explicit end makes every such line carry its own guard — the duplication the home was built to remove, reborn line by line. The engine that owns the running execution (`ScriptMgr`) is the one node that knows the run ended (it set `scriptState` to NONE itself, inside `cancelScript`); the drain is its own loop — so the test belongs at the loop's head, where "is there still a run to drain" is asked, and nowhere downstream (I9: the anchor; S2: one test, both branches — the queue's command and the loop body's). What it ends up covering: every path through `cancelScript` that a DRAINED command can take (`script action end`, a bound key's script ending the show, a TCP end during a drain) ends once; the natural end (the queue empty at `getFirst`) is untouched; a show that ends itself on its last line reports no false `#!` tail from a second `terminateScript` (today the second W_END's `terminateScript` runs the unclosed-if / loop checks again on an already-cleaned state — read whether that second pass can report anything: P5). NOT this task: `terminateScript`'s own order, `cancelScript`'s resets, the hook's content, §5.199 (a recording captures the engine's own end line).

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F156`), then READ IN FULL: `claude/SHAPE.md`; §5.204's row (INTENT.md, section 5); `INTENT/11.322.md` (6)–(10) (F151's reading of the drain, case O, the double measured); `src/scriptModule/script_mgr.hpp` WHOLE (names.py 36 — the S4 pass: `update`, `cancelScript`, `isPlaying` EXIST; nothing is added) and `script_mgr.cpp` `cancelScript` :109–128, `update` :287–360 (the drain: the `repeatLoop` branch :310–329 with its `executeCommand` :316, the queue branch :330–341 with :339, the natural-end branch :342–347, the deadline / event-queue exits after); `src/interfaceModule/app_command_interface.cpp` `terminateScript` :247–255, W_END :2849–2851, `scriptEnded` (the hook, after F151); `src/scriptModule/script.cpp` `getFirst` :179–190 (what an emptied queue returns); the harness: `claude/harness/f142_keys.py` WHOLE as F154 left it (the fixtures, the TCP play, the markers, case O's `wait_end`), `f142_keys_run.sh`, `claude/harness/README.md` F149's section. (1) The S4 pass written in the entry BEFORE the first Edit. (2) THE CASE FIRST, on the HEAD binary (the BEFORE record): case AE in `f142_keys.py` — a fixture `ae.sts` of two lines (`define f156_ae 1`, `script action end`) played over TCP; the reading: the count of `ScriptMgr: script end` lines in `script.log` between the markers (BEFORE 2), the count of `Execute_command script action end` (BEFORE 2), `End of script` lines; a second reading on the HOME path case O already drives (`initial.sts:17`'s `script action end`): the same counts after its end; run `f142_keys_run.sh s46_f156_pre --phase after`: A–AD as F154 left them, AE's BEFORE reading pasted (RED on the count is the expected BEFORE sign). (3) THE CHANGE: `while (wait_time==0)` gains `&& scriptState == ScriptState::PLAY` (:306) — one condition, no other line; build; `f70_ascii.py gate`. (4) THE GATE: `f142_keys_run.sh s46_f156_post --phase after --baseline s46_f156_pre`: A–AD identical, AE GREEN (1 / 1), O's HOME path ends once; `git diff --stat -- src/` = `script_mgr.cpp`. (5) SHAPE: `delivery.py <F154's accepted code sha> WORKTREE` pasted; net code 0 (one line changed). (6) The entry `claude/INTENT/11.328.md`: P1–P6 judged; what §5.204's row says after (CLOSED, the line); the README paragraph (ONE, case AE). (7) The report (`prompt-window: present` first).

**PREDICTIONS (the supervisor's, from the code read at the mint; judge every line, report every conflict):**
```
PREDICTIONS
P1  | shape      | one line changes: script_mgr.cpp:306 `while (wait_time==0 && scriptState == ScriptState::PLAY) {`; no other hunk in src/; no header; names.py script_mgr.hpp 36 unchanged | check: git diff -- src/ = that hunk | if refuted (a test after each executeCommand): two sites for one effect - S2, revert to the head
P2  | reach      | W_END -> cancelScript sets scriptState NONE INSIDE the drain's iteration (:339 -> commandScript -> cancelScript :117); the loop's next iteration today reaches getFirst (:330) on the cleaned queue, which returns != 1, and the natural-end branch (:342-347) runs terminateScript -> executeCommand("script action end") -> W_END again; after the change the loop exits at its head and update returns through its tail | check: the code, line by line in the entry; then P3 | if refuted (getFirst returns 1 on a cleaned queue, or the state is not NONE there): the row's mechanism is wrong - report, the case still measures
P3  | behaviour  | case AE: BEFORE 2 `ScriptMgr: script end` / 2 `Execute_command script action end` between the markers; AFTER 1 / 1 | check: AE's PASS|FAIL lines both phases | if refuted (still 2 AFTER): a second path re-enters the drain - read who calls update twice or what else runs terminateScript
P4  | behaviour  | the HOME path (case O's initial.sts:17 end): BEFORE 2 ends, AFTER 1; O's other readings unchanged (the cleanup ran once - it was idempotent) | check: O's lines both phases | if refuted: the HOME path ends through another door - name it
P5  | behaviour  | the second terminateScript pass of today reports NOTHING extra (cancelScript cleaned ifSwap / loopOpen through the hook before it ran): no `#!` tail, no MSG_UNCLOSED line appears in AE's BEFORE beyond the duplicate end lines; so the change removes a duplicate, it does not change a diagnostic | check: AE's BEFORE log excerpt | if refuted (a false diagnostic in the BEFORE): a second consequence of the double end - record it at 5.204 as measured
P6  | non-change | A-AD identical pre / post; N identical; no draw path, no mover touched - the FULL canary and f131 are not this task's gates | check: the two drive.logs | if refuted: a case reads the end sequence - name it
```

**Boundaries:** WRITES — CODE (1): `src/scriptModule/script_mgr.cpp`; HARNESS: `claude/harness/f142_keys.py` (case AE), `claude/harness/README.md` (ONE paragraph in F149's section), `claude/INTENT/11.328.md`, `claude/harness/artifacts/f142/**`. NOT yours: every header, `app_command_interface.*`, `script.cpp`, `key_binding.*`, `f142_keys_run.sh`, `INTENT.md`, this file. No install, no stash, no commit, no `run_in_background`; launches EXCLUSIVE; md5 in == out.

**STOP conditions:** a premise fails; P2 refuted by the code (report before the case); AE still reads 2 AFTER; a case of A–AD changes; a change outside WRITES is needed.

**Discriminating checks:** (a) P1–P6 with evidence; (b) AE's BEFORE 2 / AFTER 1 pasted with the log excerpts; (c) the pre / post drive.logs identical on A–AD; (d) `git diff --stat -- src/` = one file, one hunk; (e) the D14 gate; (f) delivery.py's block.

**Preconditions (checkable, §0.7):** the PREMISES block (REFRESHED at dispatch: F154's acceptance moves the HEAD, the binary and the driver's md5); F154 ACCEPTED (`archive/F154.md`).

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch (after F154's acceptance), never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => REFRESH-AT-DISPATCH
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => REFRESH-AT-DISPATCH
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
test ! -e claude/INTENT/11.328.md -a ! -e claude/INTENT/archive/11.328.md && echo free => free
/usr/bin/grep -c '^### F156 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
# the engine slot (F154's while it runs - true only at this task's own dispatch), the boot's fingerprint, the field pair, the TCP channel
bash claude/harness/sc_instances.sh --assert f156-gate >/dev/null 2>&1; echo $? => REFRESH-AT-DISPATCH
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
LC_ALL=C /usr/bin/grep -c '^enable_tcp *= *true\|^tcp_port_in *= *7805' ~/.spacecrafter/config.ini => 2
# the drain and the end paths, pinned (every structural claim of the mandate is a line here)
/usr/bin/grep -n 'while (wait_time==0)' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 306
/usr/bin/grep -n 'commander->executeCommand(comd, wait, origin);' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 339
/usr/bin/grep -n 'commander->executeCommand(loopVector\[indiceInLoop\].text' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 316
/usr/bin/grep -n 'script->getFirst(comd,DataDir,origin)' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 330
/usr/bin/grep -n 'commander->terminateScript();' src/scriptModule/script_mgr.cpp | cut -d: -f1 => 345
/usr/bin/grep -c 'scriptState' src/scriptModule/script_mgr.cpp => 9
sed -n '109,128p' src/scriptModule/script_mgr.cpp | /usr/bin/grep -c 'scriptState = ScriptState::NONE;\|commander->scriptEnded();' => 2
/usr/bin/grep -n 'enum class ScriptState' src/scriptModule/script_mgr.hpp | tr -s '\t' ' ' => 179: enum class ScriptState : char {PLAY, PAUSE, NONE};
/usr/bin/grep -n 'argAction==W_END' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 2849
sed -n '2849,2851p' src/interfaceModule/app_command_interface.cpp | tr -s '\t' ' ' | tr '\n' ';' => if (argAction==W_END) {; scriptInterface->cancelScript();; } else if (argAction==W_PLAY && !filen.empty()) {;
/usr/bin/grep -n 'return executeCommand("script action end");' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 254
/usr/bin/grep -n '^int Script::getFirst(std::string &command, std::string &dataDir, ScriptOrigin &origin)' src/scriptModule/script.cpp | cut -d: -f1 => 179
tail -1 ~/.spacecrafter/scripts/internal/initial.sts => script action end
# S4: the names of the class touched
python3 claude/harness/shape/names.py src/scriptModule/script_mgr.hpp | cut -d: -f1 => script_mgr.hpp 36
# the gate's driver (F154's ACCEPTED version at dispatch - the md5 refreshes then), the README
md5sum claude/harness/f142_keys.py | cut -c1-8 => REFRESH-AT-DISPATCH
/usr/bin/grep -c '^## F' claude/harness/README.md => REFRESH-AT-DISPATCH
```

**WIP:** 2026-10-14 07:5x — MINTED (round supervisor, Claude Fable 5.1, session 46) on §5.204's row and 11.322 (10); waits on F154's acceptance (the same driver); the owner's section-3 item 5.204 unanswered — built by I6's default, one revert. **08:0x: F157 (§5.203, the owner's SUPPORT of this morning, §11.329) goes BEFORE this task on the shared driver — an engine-killing defect reachable in session outranks a latent double end; this section's per-round lines refresh after F157's acceptance.**

---

### F157 — §5.203 SUPPORTED, THE OWNER'S WORD OF THIS MORNING (`INTENT/11.329.md`): the texture cache key is derived for EVERY path the engine accepts — `s_texture::getCacheEntryName` holds the ONE derivation (today the slice arithmetic is written twice, in it and in `getCacheName`, and throws `std::out_of_range` on the loader thread for an absolute name without `.spacecrafter/`, for a `.spacecrafter/` name with no further `/`, and for a relative name with no `/`); the rule, AS CORRECTED after the first run's STOP (`INTENT/11.330.md`; 11.329 (c)1 amended): TODAY's key wherever today's arithmetic returns one, a key where it throws — a name containing `.spacecrafter/` → the part after it; a RELATIVE name (what the engine composes: `textures/<x>`, relative to the curdir, D15) → itself; in both the first directory component abbreviated to its first letter and the four-character extension cut, as today (the field's 27 `t-bodies-*.dat` and every in-tree key unchanged, D9 — the oracle's 1807 files, 0 diffs); an ABSOLUTE name without `.spacecrafter/` → its whole path unabbreviated, the leading `/` KEPT in the entry key and written `_` in the cache file name (`/media/stick/img.png` → entry `/media/stick/img`, file `_media-stick-img.dat`: no in-tree key starts with `/`, so a one-letter root never collides with an in-tree abbreviation, and no cache file starts with `-`); a name with no directory → the file name; the SLICE computed once (today twice), `getCacheName` = today's three-character cut on the same slice with `/` → `-` (G2 of 11.330 (3): byte identity on all 1807, identity first); the two Vulkan debug-name sites (`:427`, `:516`) compute a LABEL, not the key — left alone. Gate: case O's fixture back to its generated ABSOLUTE-path PNG (F151's D4 substituted a field texture to dodge this abort) — BEFORE the engine dies at the image line (SIGABRT, F151's `s45_f151_pre` reading), AFTER case O passes and the cache directory gains no file [S, **CODE: `src/tools/s_texture.cpp` (+ `s_texture.hpp` ONLY if a helper is declared: one line) and no other file + harness; the agent may BUILD (alone in the code tree) and commits nothing, installs nothing, stashes nothing**; engine launches EXCLUSIVE; executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): two functions, one rule, an oracle over the field's paths, a case that can fail); **DISPATCHED AFTER F154's ACCEPTANCE** — it extends the same driver and the per-round lines below refresh then; F156 follows it; entry number ALLOCATED: **§11.330**]

**Why now / mandate:** §5.203 (INTENT.md, the row): *"An absolute image path outside `~/.spacecrafter/<dir>/` ABORTS the engine: `s_texture::getCacheEntryName` (`s_texture.cpp:1265–1275`, called by `TextureLoader::asyncLoad` `:185` on the loader thread) computes `p2 = name.find_first_of('/', p1) - 1` with `p1 = 0` when `.spacecrafter/` is absent — for a name starting with `/` that is 0 − 1 = `npos`, and `substr(npos)` throws `std::out_of_range` nothing catches (`terminate`, SIGABRT); `getCacheName` (`:1248–1263`) has the same arithmetic"* — measured by F151 (§11.322 (2)); RULED this morning by the owner, verbatim in `INTENT/11.329.md` (a): *"absolute image path: support. Every path everywhere in spacecrafter are supported in both absolute and relative path. Relative path have several search location, depending on what is being searched. The default curdir is always the ~/.spacecrafter folder, whatever happens"* — §2.0 D15 since this morning. The design is 11.329 (c)1 [derived, veto-open]; the compat constraint is D9's (the field's cache files keep their names — the listing's md5 is a PREMISES line).

**The ORIGIN of the need (S10, the supervisor's):** a show's images live where the show's author put them — a USB stick, a shared folder, a second disk (R2: content is added OFF session, from anywhere; D15: every path is accepted). Today such a show kills the engine IN session, at the image line, with the audience seated — the worst failure shape R2 (g) names. What the cache key IS: a name unique per source file under one cache directory — so it must be a function of the WHOLE path, and only the in-tree shape may be abbreviated (and only because the field already holds those names). What it ends up covering: every texture name the entry point (`s_texture.cpp:305`) accepts — absolute anywhere, relative from the curdir, `texDir`-relative — keyed without a throw; a later change of the extension rule (the `.png.jpeg` model textures keep a key ending in `.` today) is a SEPARATE decision because it renames field entries — identity first (Q-84), his word second. NOT this task: the two hardcoded font paths (§5.206, his stratum), `FilePath`'s search order, the cache directory's own location, the `.jpeg` extension rule.

**Mandate, in THIS order:** (0) §0 warm-up, the PREMISES gate (`python3 claude/harness/premise_check.py F157`), then READ IN FULL: `claude/SHAPE.md`; §5.203's row and §2.0 D15 (INTENT.md); `INTENT/11.329.md` whole; `INTENT/11.322.md` (2) and D4 (how F151 reached the abort and why it substituted the field texture); the code from the current checkout: `src/tools/s_texture.hpp` WHOLE (names.py 48 — the S4 pass: `getCacheName` ×2 and `getCacheEntryName` EXIST, nothing else names "the key of a texture"; `texDir` :321, `cacheDir` :343), `src/tools/s_texture.cpp` :170–200 (`asyncLoad`'s caller :185), :296–312 (the entry point :305), :420–436 and :510–526 (the two debug-name sites :427, :516), :1060–1072 (the big-texture caller :1068), :1228–1300 (`loadCache`, the two functions :1248–1275, the forwarding overload :1277–1280, the callers :1300), :1355–1405 (the callers :1362, :1401); `src/tools/call_system.hpp` and `src/tools/app_settings.hpp` names (S4: does a "path relative to the config directory" helper EXIST? `ssystem_factory.hpp:756` declares one in the body module — read its name and body; a `tools/` file cannot depend on `bodyModule/`, so if it is the same slice, SAY SO and leave it: two homes across a module boundary is a finding for the owner, not a dependency to add); `src/main.cpp:195–200` and `src/tools/file_path.cpp:100–125` (D15's pins); the harness: `claude/harness/f142_keys.py` WHOLE as F154 left it (`write_o()`, the `n_genrc.png` substitute — ONE occurrence — and the fixture writer that once generated `o.png`), `f142_keys_run.sh`, README F149's section. (1) The S4 pass in the entry BEFORE the first Edit. (2) THE ORACLE FIRST (no engine) — it EXISTS since the first run (`claude/harness/artifacts/f157/cachekey_oracle.py`, md5 `ff9cab77` at the STOP; its record `oracle_run.txt`): replicate the CORRECTED rule (the header's) in its PROPOSED function — the `_` for the leading `/` of an outside path in the cache file name is the one change from its G2 — and re-run it over the field's 1807 files in BOTH forms (REL as the engine composes them — the measured form, 11.330 (1): `textures/<x>` — and ABS in-tree): TODAY's two self-checks must still pass (F151's abort message to the byte; the 27 `t-*.dat` names among today's `getCacheName` outputs), the entry-key and cache-name diffs MUST be 0 / 0 in both forms (P2), and the probes `/media/stick/img.png`, `/mnt/stick/img.png`, `/t/bodies/earth.jpg`, `/home/x/.spacecrafter`, `/home/x/.spacecrafter/foo.png`, `foo.png`, `a/b.png` must key with no throw, the outside ones with their leading `/` kept and NO probe key equal to any in-tree key (P3, P4); the record `oracle_run.txt` rewritten. (3) THE CASE BEFORE, on the HEAD binary: `write_o()` back to the GENERATED absolute-path PNG under `artifacts/f142/run/` (F151's first form; the 4x4 RGBA writer); `f142_keys_run.sh s46_f157_pre --phase after`: the engine ABORTS at O's image line (`gdb.log`: `substr: __pos`, `SIGABRT`) — the cases after O in the driver's order are NOT REACHED in this run, say which; A–N (and whatever precedes O) as before. (4) THE CHANGE: one derivation in `getCacheEntryName`, `getCacheName` composed from it, the two debug sites consuming the shared helper (or left, said); build; `f70_ascii.py gate`. (5) THE GATE: `f142_keys_run.sh s46_f157_post --phase after --baseline s46_f157_pre`: the whole leg runs, O GREEN with the absolute PNG, every other case identical to its last accepted record (F154's acceptance record — its label in the refreshed PREMISES); `ls ~/.spacecrafter/cache/ | grep '^t-' | md5sum` unchanged (no new cache file, none renamed); `git diff --stat -- src/` = `s_texture.cpp` (+ the header if P7 says so). (6) SHAPE: `delivery.py <F154's accepted sha> WORKTREE`; `names.py src/tools/s_texture.hpp` BEFORE / AFTER. (7) The entry `claude/INTENT/11.330.md` (checkpoint in its first lines): P1–P8 judged; the oracle's two tables' diff (empty) and the four new-domain keys pasted; the BEFORE abort lines and the AFTER O lines; what §5.203's row says after (CLOSED, the line); the README paragraph (ONE: case O's fixture restored, the oracle). (8) The report (`prompt-window: present` first).

**PREDICTIONS (the supervisor's, from the code read at the mint; judge every line, report every conflict):**
```
PREDICTIONS
P1  | shape      | the SLICE (find(".spacecrafter/") or the relative name, the first-component abbreviation, the leading '/' of an outside path kept) is computed ONCE in s_texture.cpp and both functions cut it (the entry four characters, the cache name three + "dat", '/' -> '-', a leading '/' -> '_'); the two debug-name sites (:427, :516) are LEFT (a label, not the key: unabbreviated, extension kept): grep -c 'find(".spacecrafter/")' s_texture.cpp 4 -> 3 | check: the grep; the diff | if refuted (two slices remain): the duplication the task removes is still there
P2  | non-change | for every in-tree file (the oracle's 1807, REL and ABS forms) the AFTER key == the BEFORE key, both functions - 0 / 0 diffs in both forms; the 27 cache-directory names among the BEFORE getCacheName outputs; the two self-checks pass | check: oracle_run.txt re-run | if refuted: a field cache entry is renamed - D9 violated, STOP and report which
P3  | shape      | an absolute name without ".spacecrafter/" keys as its whole path, leading '/' KEPT, unabbreviated, the same four-character cut; its cache file name writes the leading '/' as '_' and every other '/' as '-': /media/stick/img.png -> entry /media/stick/img, file _media-stick-img.dat; /t/bodies/earth.jpg -> /t/bodies/earth, _t-bodies-earth.dat (never t-bodies-earth.dat); no probe key equals an in-tree key; no cache file starts with '-' | check: the oracle's probes | if refuted (a collision or a leading dash): fix before acceptance
P4  | shape      | a relative name with no '/' (foo.png) and a ".spacecrafter/" name with no further '/' no longer throw: keys foo and the empty-or-last-component form you state; NOT REACHED by a case (the field's directories are never written) - the oracle covers them | check: the oracle's four probes; a code read | if refuted: a probe throws in the replica - the rule has a hole, name it
P5  | behaviour  | case O with the generated absolute PNG: BEFORE the engine aborts at its image line (gdb.log: 'basic_string::substr: __pos', 'SIGABRT'; the script.log's last line the image load) - F151's s45_f151_pre reading reproduced; AFTER the engine survives, O 7 / 0, the image is drawn (no error line) | check: the two records | if refuted AFTER (an abort or an error line): the key is not the only consumer of the shape - read what else sliced the name
P6  | non-change | every other case identical to F154's acceptance record; the cache directory's t-* listing md5 unchanged after the AFTER run (no new .dat, none renamed); both inis untouched; config / ssystem md5 in == out | check: the drive.logs, the md5 lines | if refuted on the cache listing: a key changed for a field texture - P2's class, STOP
P7  | shape      | no header change unless a helper is declared - then ONE line in s_texture.hpp with a verb-first what-for, names.py 48 -> 49; comment lines added <= 2; net code lines in s_texture.cpp <= 0 (two slices become one, two debug sites shrink) | check: delivery.py's block; names.py | if refuted (net positive): the derivation was added beside the old arithmetic - the old lines must leave
P8  | non-change | the extension rules are UNCHANGED (the entry cuts four characters, the cache name replaces the last three by "dat" - G2): the .png.jpeg model textures keep today's odd keys and odd cache names (x.png.jdat); the last-'.' form and the "entry + .dat" composition are NOT built (identity first, Q-84 - renaming field entries is the owner's decision) | check: the oracle's cache-name diff 0 on all 1807 (the 15 .jpeg and 10 .meta included) | if refuted: a rename - revert to today's two cuts and record the proposal
```

**Boundaries:** WRITES — CODE: `src/tools/s_texture.cpp` (+ `src/tools/s_texture.hpp` one line, only if P7's helper is declared); HARNESS: `claude/harness/f142_keys.py` (`write_o()` and the fixture writer only), `claude/harness/README.md` (ONE paragraph in F149's section), `claude/INTENT/11.330.md`, `claude/harness/artifacts/f157/**` (the oracle and its tables), `claude/harness/artifacts/f142/**`. NOT yours: every other `src/` file (`core.cpp:1478` is §5.206, the owner's), `file_path.*`, `call_system.*`, `app_settings.*`, `ssystem_factory.*`, `f142_keys_run.sh`, `INTENT.md`, this file; `~/.spacecrafter/**` is READ only (the oracle reads paths; the cache directory is never written by you — the engine writes it); no install, no stash, no commit, no `run_in_background`; launches EXCLUSIVE; md5 in == out.

**STOP conditions:** a premise fails; the oracle's BEFORE table does not contain the 27 cache names (the replica is wrong — report, build nothing); P2's diff is not empty under the rule (a field key would change — report the key, propose, build nothing); P5 AFTER still aborts; a change outside WRITES is needed (name the site: a `tools/` → `bodyModule/` dependency is such a site).

**Discriminating checks:** (a) P1–P8 with evidence; (b) the oracle's two tables, their diff (empty) and the four probes pasted; (c) the BEFORE abort lines and the AFTER O lines; (d) the cache listing md5 before / after the AFTER run; (e) `git diff --stat -- src/`; (f) the D14 gate; (g) delivery.py's block, names.py both ends; (h) `grep -c 'find(".spacecrafter/")'` at its predicted count.

**Preconditions (checkable, §0.7):** the PREMISES block (REFRESHED at dispatch: F154's acceptance moves the HEAD, the binary and the driver's md5); F154 ACCEPTED (`archive/F154.md`); the owner's ruling present (`INTENT/11.329.md`).

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch (after F154's acceptance), never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => 6a331fd5
git status --porcelain | /usr/bin/grep -v 'util/scedit/' | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => bb3d8b5f
find src -type f -newer build-claude/src/spacecrafter | wc -l => 0
/usr/bin/grep -c '^CHECKPOINT 2026-10-14 09:01 - STOPPED at step (2)' claude/INTENT/11.330.md => 1
/usr/bin/grep -c '^### F157 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/INTENT/11.329.md -a -e claude/fable-dispatch/archive/F154.md && echo present => present
# the engine slot (F154 accepted - the slot is this task's), the boot's fingerprint, the field pair, the TCP channel; F158 edits util/scedit/ beside you (excluded from the status line above)
bash claude/harness/sc_instances.sh --assert f157-gate >/dev/null 2>&1; echo $? => 0
bash claude/harness/f56_canary.sh --no-scene >/dev/null 2>&1; echo $? => 0
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | cut -c1-8 | tr '\n' ' ' => 03fbee59 545a51ef
LC_ALL=C /usr/bin/grep -c '^enable_tcp *= *true\|^tcp_port_in *= *7805' ~/.spacecrafter/config.ini => 2
# the defect's two functions and their duplicated slice, the callers, the two debug-name siblings, the third overload, the entry point (every structural claim of the mandate is a line here)
/usr/bin/grep -n '^std::string s_texture::getCacheName\|^std::string s_texture::getCacheEntryName' src/tools/s_texture.cpp | cut -d: -f1 | tr '\n' ' ' => 1248 1265 1277
sed -n '1248,1263p' src/tools/s_texture.cpp | tr -s '\t ' ' ' | tr '\n' ';' => std::string s_texture::getCacheName(const std::string &name);{; auto p1 = name.find(".spacecrafter/")+14;; if (p1 == 13); p1 = 0;; const auto p2 = name.find_first_of('/', p1) - 1;; std::string filename = name.substr();; filename = name.substr(p2, name.size() - p2);; filename.front() = name[p1];; for (char &c : filename) {; if (c == '/'); c = '-';; }; memcpy(filename.data() + filename.size() - 3, "dat", 3);; return filename;;};
sed -n '1265,1275p' src/tools/s_texture.cpp | tr -s '\t ' ' ' | tr '\n' ';' => std::string s_texture::getCacheEntryName(const std::string &name);{; auto p1 = name.find(".spacecrafter/")+14;; if (p1 == 13); p1 = 0;; const auto p2 = name.find_first_of('/', p1) - 1;; std::string filename = name.substr();; filename = name.substr(p2, name.size() - p2 - 4);; filename.front() = name[p1];; return filename;;};
/usr/bin/grep -c 'find(".spacecrafter/")' src/tools/s_texture.cpp => 4
/usr/bin/grep -n 'find(".spacecrafter/")' src/tools/s_texture.cpp | cut -d: -f1 | tr '\n' ' ' => 427 516 1250 1267
sed -n '427,428p' src/tools/s_texture.cpp | tr -s '\t ' ' ' | tr '\n' ';' => auto pos = fullName.find(".spacecrafter/");; std::string name = (pos == std::string::npos) ? fullName : fullName.substr(pos+14);;
/usr/bin/grep -rn 'getCacheEntryName\|getCacheName' src --include=*.cpp --include=*.hpp | cut -d: -f1,2 | tr '\n' ' ' => src/tools/s_texture.cpp:185 src/tools/s_texture.cpp:1068 src/tools/s_texture.cpp:1248 src/tools/s_texture.cpp:1265 src/tools/s_texture.cpp:1277 src/tools/s_texture.cpp:1279 src/tools/s_texture.cpp:1300 src/tools/s_texture.cpp:1362 src/tools/s_texture.cpp:1401 src/tools/s_texture.hpp:288 src/tools/s_texture.hpp:311 src/tools/s_texture.hpp:312
sed -n '1278,1281p' src/tools/s_texture.cpp | tr -s '\t ' ' ' | tr '\n' ';' => {; return getCacheName(tex->texName);;};;
/usr/bin/grep -c 'isAbsolute(textureName) || CallSystem::fileExist(textureName)' src/tools/s_texture.cpp => 4
/usr/bin/grep -n 'isAbsolute(textureName) || CallSystem::fileExist(textureName)' src/tools/s_texture.cpp | cut -d: -f1 | tr '\n' ' ' => 305 490 590 986 
/usr/bin/grep -rn '\.spacecrafter' src --include=*.cpp --include=*.hpp | wc -l => 7
# the premise the first run refuted (11.330 (1)): in-tree texture names are RELATIVE - texDir, FilePath's composition, the field's own log
/usr/bin/grep -n 'return REP_TEXTURE' src/tools/app_settings.cpp => 172:	return REP_TEXTURE + "/";
/usr/bin/grep -c 'const std::string REP_TEXTURE = "textures";' src/spacecrafter.hpp => 1
/usr/bin/grep -c 'case TFP::TEXTURE : fullFileName = AppSettings::Instance()->getTextureDir() + fileName;' src/tools/file_path.cpp => 1
/usr/bin/grep 'already in cache' ~/.spacecrafter/log/spacecrafter.log | /usr/bin/grep -c '\.spacecrafter/' => 0
ls ~/.spacecrafter/textures/bodies/ | /usr/bin/grep -c '^earth\.' => 2
# the oracle the first run built (11.330 (2)), at its STOP state - the re-run replicates the corrected rule in it first
md5sum claude/harness/artifacts/f157/cachekey_oracle.py claude/harness/artifacts/f157/oracle_run.txt | cut -c1-8 | tr '\n' ' ' => ff9cab77 7210d316
/usr/bin/grep -c 'REL proposed_G2 | entry diffs 0 | cache-name diffs 0' claude/harness/artifacts/f157/oracle_run.txt => 1
test -e claude/INTENT/11.330.md && echo present => present
# the owner's invariant pinned (11.329 (b)): the curdir, FilePath's absolute branch, the texture directory
/usr/bin/grep -n 'std::filesystem::current_path(appDir);' src/main.cpp | cut -d: -f1 => 198
sed -n '106,111p' src/tools/file_path.cpp | tr -s '\t ' ' ' | tr '\n' ';' => ; // we test if the file has an absolute name, we simply test its existence; if (CallSystem::isAbsolute(fileName)) {; fullFileName = fileName;; isFileExist = CallSystem::fileExist(fullFileName);; return;;
/usr/bin/grep -n 's_texture::setTexDir' src/coreModule/core.cpp | cut -d: -f1 => 96
# the field (D9): the cache names the fix must reproduce, the texture census, the five-letter extensions (model3D only)
ls ~/.spacecrafter/cache/ | /usr/bin/grep -c '^t-bodies-.*\.dat$' => 27
ls ~/.spacecrafter/cache/ | /usr/bin/grep -c '^t-' => 27
ls ~/.spacecrafter/cache/ | /usr/bin/grep '^t-' | md5sum | cut -c1-8 => fb63700e
find ~/.spacecrafter/textures -type f \( -name '*.png' -o -name '*.jpg' \) | wc -l => 677
find ~/.spacecrafter -type f \( -name '*.jpeg' -o -name '*.webp' -o -name '*.tiff' \) | wc -l => 15
ls ~/.spacecrafter/textures/bodies/ | /usr/bin/grep -c '^earth\.\|^moon\.\|^mars\.' => 6
# the measured abort (F151's pre record on the unchanged binary) and the driver (REFRESH at dispatch: F154's accepted version), the fixture's field substitute to undo
/usr/bin/grep -c 'SIGABRT' claude/harness/artifacts/f142/s45_f151_pre/gdb.log => 2
/usr/bin/grep -c 'substr: __pos' claude/harness/artifacts/f142/s45_f151_pre/gdb.log => 1
md5sum claude/harness/f142_keys.py | cut -c1-8 => a02fe710
/usr/bin/grep -c 'n_genrc.png' claude/harness/f142_keys.py => 1
/usr/bin/grep -c '^PASS' claude/harness/artifacts/f142/s46_acc_after/drive.log => 90
/usr/bin/grep -c '^PASS [A-O] ' claude/harness/artifacts/f142/s46_acc_after/drive.log => 49
# S4: the names of the class touched
python3 claude/harness/shape/names.py src/tools/s_texture.hpp | cut -d: -f1 => s_texture.hpp 48
/usr/bin/grep -c '^## F' claude/harness/README.md => 85
```

**WIP:** 2026-10-14 08:1x — MINTED (round supervisor, Claude Fable 5.1, session 46) on §5.203's row and the owner's ruling of this morning (§11.329, D15); waits on F154's acceptance (the same driver, the engine slot); F156 follows it. **DISPATCHED 08:50 after F154's acceptance (`opus-xhigh` under the owner's prompt; the per-round lines refreshed to code `6a331fd5` / binary `bb3d8b5f` / driver `a02fe710`, the gate 41 / 41), alone in the engine's build tree, the only engine launcher; F158 beside it inside `util/scedit/`; harness `6f2e5af`.** **STOPPED 09:01 at step (2), nothing built (`INTENT/11.330.md`): the section's rule — the supervisor's, 11.329 (c)1 — stood on a premise never pinned: in-tree texture names contain `.spacecrafter/`. They do not (`getTextureDir()` = `textures/`; the field's log: 215 cached names, 0 with `.spacecrafter/`): the field's 27 cache files come from today's no-`.spacecrafter/` branch, and the rule renamed every in-tree key (1807 / 1807 on the oracle the executor built — a replica of both functions over the field's files in both name forms, F151's abort to the byte, the 27 names found). The executor proposed the rule that keeps today's key wherever today returns one and checked it (0 / 0); found the one-letter-root collision (`/t/bodies/earth.jpg` = `textures/bodies/earth.jpg`), §5.208 (two field textures per key, 125 cases), the drive-letter form. ACCEPTED AS A STOP RECORD 09:0x by the supervisor — the texDir fact and the log census re-read by its hand; the mint defect is the supervisor's (the fourth text-as-the-thing instance of the session, recorded at Q-89); the section RE-MINTED IN PLACE (F75's precedent): the corrected rule in the header, the oracle step in both forms, P1 (count 3) / P2 (REL) / P3 (the leading `/` kept, `_` in the file name) / P8 (G2) amended, nine PREMISES lines added. RE-DISPATCHED 09:09 (`opus-xhigh`, a fresh instance resuming in 11.330; the gate 49 / 49; harness `1895063`), alone in the engine's build tree, the only engine launcher; F158 beside it.**

---

---

### F159 — THE SCEDIT GRAMMAR IN PHASE WITH THE ENGINE, on the pin F158 set (F155's content, re-minted on its STOP record `INTENT/11.327.md` (4)2): the TEN spellings the engine has and the grammar lacks — F149's `bind key press scope script once` and F137's `move_lat move_lon raise_height lower_height` — enter `sc-grammar.json` AND the two fragments the seed gate couples to it (`args/unit-2.json` for `script`, `args/unit-4.json` for `camera`), in the file's own forms: `bind` a value of `script action` with its `value_docs` and `completable`, the four args `key` / `press` / `release` / `scope`, a `branches` list on `script` (selector `action bind`); the four `camera` actions as values + four `branches` (`requires ["value"]`); the vocabulary 238 → 248 with `release`'s role `value` → `both`; every new anchor at the family pin `main @ 9d461bd5` (the two handler files are unchanged since it) except the key token's reason text, cited `key_binding.cpp:89-91 @ 52568219` (its own pin — the form F158 implemented); the anchor gate re-recorded with its argument; the record gates that print the new words re-recorded, their diffs the words alone; the engine's error texts verbatim, the fourth bind text included [S–M, **CODE TREE: `util/scedit/grammar/{sc-grammar.json, args/unit-2.json, args/unit-4.json}`, `util/scedit/tests/anchor-expected.txt`, and ONLY those of `util/scedit/tests/{ui-selftest,lint,history,check-json,doc}-expected.txt` whose gate prints a new word (each re-recorded by its own gate script, the diff pasted); scedit built in its OWN tree (`util/scedit/build-f159/`) — never the engine, no engine launch; the agent commits nothing, installs nothing, stashes nothing**; runs IN PARALLEL with F157 (disjoint: F157 never enters `util/`, F159 never writes under `src/` or `claude/harness/README.md` — its README paragraph is PROPOSED in the entry); executor = agent type `opus-xhigh` (Claude Opus 5.5, xhigh — FOCUSED under §0b (a): three files + records, the words pinned at their engine lines, gates that can fail, the predictions corrected by F155's measured STOP); entry number ALLOCATED: **§11.332**]

**Why now / mandate:** F155 stopped at its BEFORE this morning (`INTENT/11.327.md`): the anchor gate was red and a one-file edit could not pass the seed gate; its (4)2 wrote the re-mint: *"WRITES = `sc-grammar.json` + `args/unit-2.json` (script) + `args/unit-4.json` (camera), plus `tests/anchor-expected.txt` re-recorded (the new references raise `clean`). Corrected predictions: P1 / P2 + `completable` (and, for `script`, `branches` written to BOTH halves ...); P3 248 + `release`'s role; P4 at the new pin, `lines` refreshed with the family; P6's discriminating channel = the four bind keys on `--check` + `--doc camera action`, `--doc script action|key|press|release|scope`; P7 + the fourth text."* F158 (`INTENT/11.331.md`, accepted 09:2x) made the precondition true: the gate is green at HEAD `52568219` on the pin `main @ 9d461bd5`, `lines` sits at the pin for the whole family, a reference's own `@ <sha>` is honoured. §11.299 F6 (the four camera words absent) and §11.320's `bind` family are the debts; the grammar is scedit's runtime contract (`_meta.merged`), a word the engine knows and the grammar lacks is a false error at the author's desk.

**The ORIGIN of the need (S10, the supervisor's):** the authoring tool exists so that a show author finds the engine's silent edges at the desk (README); a command surface change has two homes — the handler and the grammar — and the grammar's citations are the doc bar's ground (C6). What it ends up covering: every word the engine accepts, documented at its engine line, in BOTH halves the seed gate couples; the gate re-recorded with the argument in the same delivery. NOT this task: the button tokens' entry into `key`'s domain is INCLUDED (F154 is accepted: the reason text at `:89-91` names them — one `value` domain sentence); the owner's renaming of the four camera words (11.311:6, veto-open — the grammar carries the engine's names; a rename lands with the engine's); `anchor_gate.py` (F158's shape — a word that needs a gate change is a STOP); the `.ts` extension, the TUI, the witness corpus.

**Mandate, in THIS order:** (0) §0 warm-up (no engine, no canary), the PREMISES gate (`python3 claude/harness/premise_check.py F159`), then READ IN FULL: `INTENT/11.327.md` whole ((1) the reads, (2b) the fragment coupling, (2c), (3) P1–P8's refutations, (4)2 the plan, (5) F6 the modifier order); `INTENT/11.331.md` (6)–(9) (the pin, `lines`, the per-reference `@ <sha>`, `--record --why`, the five record gates); `util/scedit/README.md` § Build, § Verification (:685–800: the gates, the records, the pin paragraph); `sc-grammar.json`: `_meta` whole, `families.commands.camera` WHOLE (the `branches` form: 16 entries), `families.commands.script` WHOLE, `families.commands.zoom` (`branches` with per-branch value domains), `argument_token_vocabulary` whole (`_source`, `_count_note`, `_expected_count`, `tokens`'s per-token shape `{macro, header_line, role, value_units | key_units}`, `both_roles`), `lint_seeds`; `args/unit-2.json`'s `script` and `args/unit-4.json`'s `camera` (the fragment twins — field-identical to the merged entries: `_merged` says a correction belongs in the fragment FIRST); `util/scedit/src/main.cpp:140–200` (`checkFragments`: the compared fields) and `:325–399` (the `completable` rule); `util/scedit/src/sc_check.cpp:478–527` (the key vocabulary read from the grammar; `unknown-parameter` only where `args_complete`); `util/scedit/tests/doc_queries*` and the four other record gates (which queries / outputs they record — to know which records the new words reach); the engine at HEAD: `src/interfaceModule/app_command_interface.cpp:2884–2923` (W_BIND: `key` defaults to the trigger key else REQUIRED; `press` / `release` at least one, `-` the default; `scope` `global` default, `script`, `once`; the FOUR error texts :2889, :2891, :2893, :2903; the recorded line :2914–2922), `:4425–4441` (the four actions: `value` REQUIRED — *"missing value"* — `evalDouble`, no range check), `src/interfaceModule/base_command_interface.hpp:91, :249–252, :323–330` (the W_ spellings — from here, never typed), `src/uiModule/key_binding.cpp:83–103` (the key token's domain and its reason text :89–91 — the modifiers in MODIFIERS order, `parse` accepts any order and case; the button forms `BUTTON<n>`, `HAT<n>_<UP|DOWN|LEFT|RIGHT>`, `_ALT` refused — F154); `INTENT/11.311.md:19` (N6: `move_lat` / `move_lon` v in [-1, 1], `raise_height` / `lower_height` in [0, 1], a held order until the next value, `value 0` stops); `INTENT/11.305.md` (a), (b), (g) (the lifetimes, `-`); `claude/harness/artifacts/f155/words.sts` and `check-before.txt` (the fixture and its measured BEFORE). (1) The S4 pass in the entry BEFORE the first Edit: the grammar's existing forms against each addition — `branches` (camera's), `value_docs`, `completable` / `completable_excluded`, `both_roles`, the token shape — nothing invented; the record gates' outputs against "where does a new word print". (2) BEFORE: `cmake -S util/scedit -B util/scedit/build-f159 && cmake --build util/scedit/build-f159 -j8 && ctest --test-dir util/scedit/build-f159 --output-on-failure` → 19 / 19 (the tail pasted); `scedit --check` on `words.sts` (12 `unknown-parameter`, the four bind lines) and `--doc camera action`, `--doc script action`, `--doc script key`, `--doc script press`, `--doc script release`, `--doc script scope` (the outputs recorded under `artifacts/f159/`). (3) THE EDIT, fragment first then merged, field-identical: `script` (`unit-2.json` + merged): `bind` in `action.values` + `value_docs` + `completable`; args `key` (value domain: a keyboard token `[CTRL+][KWIN+][SUPER+][SHIFT+][ALT+]<SDL scancode name>` in any order and case, `*`, a joystick `BUTTON<n>` / `HAT<n>_<UP|DOWN|LEFT|RIGHT>` — no `_ALT`; default = the trigger key inside a key-bound script, else REQUIRED → `required: "conditional"`; `source` `app_command_interface.cpp:2885` and `key_binding.cpp:89-91 @ 52568219`), `press` / `release` (a script name or `-`; at least one; the resolution and the three lifetimes in one line each), `scope` (`values [global, script, once]`, default `global`), `branches` [selector `action bind`, requires one of press / release, key conditional, scope optional, effect, source `:2884-2923`]; `camera` (`unit-4.json` + merged): the four in `action.values` + `value_docs` + `completable`, four `branches` entries (`requires ["value"]`, effect from N6, source `:4425-4441`), `value`'s `notes` naming them; the vocabulary: the ten tokens with `macro` / `header_line` (the define's line at the pin) / `role` / units; `release` → role `both` + `key_units`; a `both_roles` line; `_expected_count` 248; `_source` / `_count_note` → `:78-330`; ONE `_meta.amended` dated line; D7: every command and every key carries its own ONE line of documentation, verb first where it is a sentence; the error texts VERBATIM from the engine (the fourth bind text `:2903` included), no modifier ORDER implied (11.327 F6). (4) AFTER: rebuild → the seed gate green (`checkFragments` identical); `python3 util/scedit/tests/anchor_gate.py` → exit 0 after `--record --why "<F159's argument>"` (clean 7180 → + the new references, `header_line:clean` 238 → 248; the record's `# Why:` lines kept and one added); the record gates that print the new words re-recorded by their own scripts, each diff pasted and holding ONLY the new words and their anchors (digit-stripped identity is NOT the criterion here — the words are new — the criterion is: no line changes but those naming a new word); ctest 19 / 19 in `build-f159`; `--check words.sts` 0 findings; the six `--doc` queries answering (`script key` / `scope` exit 0); `python3 claude/harness/f70_ascii.py gate` PASS; `json.load` on the three files. (5) The entry `claude/INTENT/11.332.md` (checkpoint in its first line; P1–P8 judged; the BEFORE / AFTER outputs; the gate's record line; the README paragraph PROPOSED for F155's / this task's place in `harness/README.md` — the supervisor appends it) and ONE dated entry in `claude/util/scedit/INTENT.md` (`[2026-10-14c]`, pointing at §11.332). (6) The report (`prompt-window: present` first).

**PREDICTIONS (the supervisor's, from 11.327's measured refutations and the grammar read at the mint; judge every line, report every conflict):**
```
PREDICTIONS
P1  | shape      | script, BOTH halves field-identical: `bind` in action.values (12) + value_docs + completable (completable == values holds, the seed gate's rule); four new args key / press / release / scope each with doc, value, values (scope), default, required (key "conditional"), source, notes; a `branches` list (new on script) with the `action bind` selector; NO new command (expected_counts.commands 63) | check: the python read of both halves; checkFragments green; the seed gate | if refuted (a `bind` command, or branches in one half): revert - the engine has one command `script`; the fragment is the first home
P2  | shape      | camera, BOTH halves: the four in action.values (13 -> 17) + value_docs + completable; branches 16 -> 20, each `requires ["value"]`, source app_command_interface.cpp:4425-4441 (the pin's lines = HEAD's: the file is unchanged since 9d461bd5); value.notes names the four | check: the python read | if refuted: a second `value` key or a renamed one - the engine reads args[W_VALUE]
P3  | shape      | the vocabulary: 238 -> 248 tokens (the ten of the PREMISES line, none else), _expected_count 248, each new token's header_line = its #define line (249-252, 323-330), roles value / key as the engine reads them; `release` role value -> both with key_units, both_roles 8 -> 9; _source / _count_note name :78-330 | check: the python line -> `248 248 [] both`; the gate's header_line:clean 248 | if refuted on the count: a word already present re-added or one forgotten - the ten are the list
P4  | shape      | anchors: every new source into app_command_interface.cpp / base_command_interface.hpp is written at the family pin's lines (= HEAD's); the key token's reason text cited `key_binding.cpp:89-91 @ 52568219` (its own pin); _meta.anchor_pin UNCHANGED; the gate exits 0 after `--record --why`, clean 7180 -> 7180 + N (N = the new references, stated), header_line:clean 248, lines:clean 127 unchanged (no handler moved) | check: the gate's output and record | if refuted (a reference GONE or AT-PIN-BROKEN): a line typed, not read - fix at the engine line
P5  | behaviour  | ctest 19 / 19 BEFORE (green since F158) and AFTER; the record gates whose output prints a new word (doc_queries at least - say which) re-recorded by their own scripts, each diff holding ONLY lines that name a new word or its anchor | check: the two ctest tails; the record diffs pasted | if refuted AFTER: a count / uniqueness / completable gate - read which; a record diff with an unrelated line - STOP, report
P6  | behaviour  | `--check words.sts`: BEFORE 12 unknown-parameter on the four bind lines (F155's check-before.txt), AFTER 0; `--doc camera action` lists the four, `--doc script action` lists bind, `--doc script key|press|release|scope` exit 0 with their one-line docs (BEFORE key / scope exit 2 unknown-name) | check: the outputs pasted | if refuted AFTER: the tool reads a field the edit did not fill - read sc_check.cpp / the doc code for which, fill it, say so
P7  | shape      | the texts are the engine's strings verbatim: "bind: give 'press <script|->', 'release <script|->' or both" (:2889), "bind: 'key' omitted outside a script started by a key binding - write 'key <key>', such as 'key SHIFT+KP_8'" (:2891), "bind: unknown scope '...' - write global, script or once" (:2893), "bind: no script '...' as given, in the running script's directory or in the scripts directory" (:2903), "command 'camera <action>' : missing value" (:4428); the key token's domain as key_binding.cpp:89-91 states it, the modifiers' ORDER not implied (parse accepts any) | check: grep each in the JSON | if refuted (paraphrased, or an order implied): quote the engine
P8  | non-change | git diff --stat = sc-grammar.json, unit-2.json, unit-4.json, anchor-expected.txt + the re-recorded record files (named), nothing else, nothing under src/; pure ASCII; json.load 0 on the three; the seed counts (commands 63 ... font_targets 10) unchanged; `--doc` on every OTHER command unchanged (0 differing lines) | check: the stat, the gate, the loads, the doc diff | if refuted: outside the slice - revert
```

**Boundaries:** WRITES — CODE TREE: `util/scedit/grammar/sc-grammar.json`, `util/scedit/grammar/args/unit-2.json`, `util/scedit/grammar/args/unit-4.json`, `util/scedit/tests/anchor-expected.txt` (through `--record --why` only), the record files among `util/scedit/tests/{ui-selftest,lint,history,check-json,doc}-expected.txt` whose gate prints a new word (re-recorded by their own scripts, named in the entry); HARNESS: `claude/INTENT/11.332.md`, `claude/util/scedit/INTENT.md` (ONE dated entry), `claude/harness/artifacts/f159/**`. Build directory `util/scedit/build-f159/` (git-ignored; leave it). NOT yours: `util/scedit/tests/anchor_gate.py` (F158's shape — a word that needs a gate change is a STOP), `args/unit-1.json`, `args/unit-3.json`, `ss-grammar.json`, `grammar/witness/**`, `util/scedit/src/**`, `util/scedit/README.md`, anything under `src/` (READ only), `claude/harness/README.md` (F157 writes it this round — propose), `INTENT.md`, this file; no engine build, no engine launch, no install, no stash, no commit, no `run_in_background`. Absolute paths in every command (Q-72). F157 works beside you in `src/tools/`, `claude/harness/f142_keys.py`, `claude/harness/README.md`, `claude/INTENT/11.330.md`: never open a file of its WRITES.

**STOP conditions:** a premise fails; the schema cannot carry a conditional arg or a new value without a form the file does not already use (propose; write nothing); the anchor gate needs a change (F158's shape — propose); a record gate's diff holds a line that names no new word (report before writing it); `checkFragments` red after both halves (report the diff, patch no single half).

**Discriminating checks:** (a) P1–P8 with evidence; (b) the BEFORE / AFTER `--check` and the six `--doc` outputs pasted; (c) the two ctest tails; (d) the anchor gate's record line with its new `# Why:`; (e) the python reads of both halves of both entries and of the vocabulary AFTER; (f) the record files' diffs; (g) `git diff --stat`; (h) the D14 gate; (i) the dated line in the scedit journal and the proposed README text.

**Preconditions (checkable, §0.7):** the PREMISES block (refreshed at dispatch); F158 ACCEPTED (`archive/F158.md`), the gate green at HEAD; F155's STOP record present; `cmake` + a C++17 compiler on the host (F158 built scedit this morning).

```
PREMISES
# per-round variables - refreshed by the dispatcher at dispatch, never at mint
git rev-parse --abbrev-ref HEAD => main
git rev-parse --short=8 HEAD => 52568219
git status --porcelain | wc -l => 0
test ! -e claude/INTENT/11.332.md -a ! -e claude/INTENT/archive/11.332.md && echo free => free
/usr/bin/grep -c '^### F159 ' claude/fable-dispatch.md => 1
git -C /home/claude/spacecrafter.owner-shape rev-parse --short=8 HEAD => 4dfe7bb3
test -e claude/INTENT/11.327.md -a -e claude/INTENT/11.331.md -a -e claude/fable-dispatch/archive/F158.md && echo present => present
# the gate GREEN at HEAD on the new pin (F158 landed: 05c714b3 + 52568219), its record, the three files this task writes, the per-reference pin implemented
python3 util/scedit/tests/anchor_gate.py >/dev/null 2>&1; echo $? => 0
sed -n '/^clean/,$p' util/scedit/tests/anchor-expected.txt | tr -s ' ' | tr '\n' ';' => clean 7180;handler_body_lines:clean 2;header_line:clean 238;lines:clean 127;not-at-head(declared) 18;skipped(_meta) 86;
md5sum util/scedit/grammar/sc-grammar.json util/scedit/grammar/args/unit-2.json util/scedit/grammar/args/unit-4.json util/scedit/tests/anchor-expected.txt | cut -c1-8 | tr '\n' ' ' => fa50a7f9 6d38f140 1a254af8 b50e74d2
python3 -I -c "import json;g=json.load(open('util/scedit/grammar/sc-grammar.json'));print(g['_meta']['anchor_pin'][:22])" => main @ 9d461bd5 -- eve
/usr/bin/grep -c 'OWNPIN' util/scedit/tests/anchor_gate.py => 2
# the engine's words at HEAD: app_command_interface.cpp and the header unchanged since the pin; key_binding.cpp's reason text moved to :89-91 by F154 (a new anchor there carries its own @ <sha>)
/usr/bin/grep -n 'argAction==W_BIND' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 2884
/usr/bin/grep -n 'argAction == W_MOVE_LAT || argAction == W_MOVE_LON' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 4425
/usr/bin/grep -n 'bind: no script' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 2903
git diff --stat 9d461bd5 HEAD -- src/interfaceModule/app_command_interface.cpp src/interfaceModule/base_command_interface.hpp | wc -l => 0
/usr/bin/grep -n 'unknown key' src/uiModule/key_binding.cpp | cut -d: -f1 => 89
sed -n '89,91p' src/uiModule/key_binding.cpp | tr -s '\t' ' ' | tr '\n' ' ' => reason = "unknown key '" + token + "': write [CTRL+][KWIN+][SUPER+][SHIFT+][ALT+] then an SDL scancode name "  "without SDL_SCANCODE_, such as SHIFT+KP_8; a joystick button or hat direction as BUTTON3 or HAT0_UP, "  "no prefix and no _ALT (a binding serves both sets of the joystick's ini); or * for every key";
/usr/bin/grep -n '#define W_BIND\|#define W_KEY\b\|#define W_PRESS\|#define W_RELEASE\|#define W_SCOPE\|#define W_GLOBAL\|#define W_SCRIPT\b\|#define W_ONCE\|#define W_MOVE_LAT\|#define W_MOVE_LON\|#define W_RAISE_HEIGHT\|#define W_LOWER_HEIGHT\|#define W_VALUE\b' src/interfaceModule/base_command_interface.hpp | cut -d: -f1 | tr '\n' ' ' => 91 249 250 251 252 323 324 325 326 327 328 329 330
# the grammar: the ten words absent, the vocabulary against the header's spellings, release's role, the two entries' forms, completable, the counts
/usr/bin/grep -c '"bind"\|"move_lat"\|"move_lon"\|"raise_height"\|"lower_height"' util/scedit/grammar/sc-grammar.json => 0
python3 -I -c "import json,re;g=json.load(open('util/scedit/grammar/sc-grammar.json'));v=g['argument_token_vocabulary']['tokens'];h=open('src/interfaceModule/base_command_interface.hpp').read();sp=set(re.findall(r'^\s*#define\s+W_\w+\s+\"([^\"]*)\"',h,re.M));print(len(v),len(sp),sorted(sp-set(v)),v['release']['role'])" => 238 248 ['bind', 'key', 'lower_height', 'move_lat', 'move_lon', 'once', 'press', 'raise_height', 'scope', 'script'] value
python3 -I -c "import json;g=json.load(open('util/scedit/grammar/sc-grammar.json'));c=g['families']['commands'];print(c['script']['lines'],c['camera']['lines'],'branches' in c['camera'],'branches' in c['script'],sorted(c['script']['args']['action']['completable'])==sorted(c['script']['args']['action']['values']),len(c['camera']['branches']))" => 2842-2944 4230-4508 True False True 16
python3 -I -c "import json;g=json.load(open('util/scedit/grammar/sc-grammar.json'));print(g['_meta']['expected_counts']['commands'],g['argument_token_vocabulary']['_expected_count'],len(g['argument_token_vocabulary']['both_roles']))" => 63 238 8
python3 -I -c "import json;u2=json.load(open('util/scedit/grammar/args/unit-2.json'));u4=json.load(open('util/scedit/grammar/args/unit-4.json'));print('script' in u2['commands'],'camera' in u4['commands'],u2['commands']['script']['args']==json.load(open('util/scedit/grammar/sc-grammar.json'))['families']['commands']['script']['args'])" => True True True
# scedit builds standalone; its gates at HEAD; the records' homes
/usr/bin/grep -c 'add_test' util/scedit/CMakeLists.txt => 19
ls util/scedit/build-f158/scedit >/dev/null 2>&1 && echo built => built
/usr/bin/grep -c '^- \*\*\[2026' claude/util/scedit/INTENT.md => 29
```

**WIP:** 2026-10-14 09:2x — MINTED (round supervisor, Claude Fable 5.1, session 46) on 11.327 (4)2 and F158's acceptance; parallel to F157 by disjoint files.

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

- **Session-45 decision items (2026-10-13, Tuesday evening — THE ROUND RUN IN YOUR ABSENCE: F151 landed as `f571664e` / `9d461bd5`, F153 the joystick row's structure, F152 the key leg banked. Each default below is BUILT unless you say otherwise; one sentence each.)**

  ```
  YOURS — ONE LIST (held set per node; the entry's section in brackets)
  THE SCRIPT'S END, ONE HOME [11.322]:
    ORDER §5.191's row said "his to order" (F142 (e3)): read as the one-structural-change-per-commit clause and BUILT — scriptEnded()
          holds the cleanup, W_END keeps cancelScript() alone, UI::init's copy left (managers 2 -> 1, net -3). One revert.
    HDR   app_command_interface.hpp:65 now reads `//! Clear what the ended script left behind` (was "Tell the key bindings ..."):
          my comment-only commit in your review scope; byte-identical binary.
    5.203 an absolute image path outside ~/.spacecrafter/<dir>/ ABORTS the engine (s_texture::getCacheEntryName underflows):
          support the form (derive a cache name) or refuse it LOUD at the command, naming the line. Nothing built.
    5.204 an explicit `script action end` line ends the run TWICE (the drain does not re-test scriptState): the hook runs twice,
          idempotent today. A small fix in ScriptMgr::update's drain, or left until a non-idempotent line needs it.
    REACH the row's own instance (HOME in a SKIPPED block) is one frame wide; the reachable ones were an open accepting `struct if`
          and a loop's FIRST pass — a HOME there wrote false "never closed" errors and `#!` tails INTO the dead show's file,
          masked on your field by initial.sts:17's `script action end`, visible through `configuration action load`. Closed.
  THE JOYSTICK ROW, STRUCTURED [11.324 (k) — twelve questions; nothing built]:
    Q1    is a button a KEY of your 11.305 design? default YES (`key` is the slot's name; your 2022 release branch 73cb47cd says the
          need was yours). NO = the ini stays the joystick's only layer; the gap list (release for command buttons, scopes,
          in-session binding, `*` reaching buttons) is the cost.
    Q2    the plane's spelling in a token: BUTTON3_ALT (your ini's own key, default) / ALT+BUTTON3 (the keyboard's prefix — a pun:
          the joystick's plane is your EYE-RELATIVE MODE copied each frame at ui.cpp:921, not a key he holds) / a word for the mode.
    Q3    the plane falls back _ALT -> base for joystick sources (the ini's own rule; default yes) or planes independent like keys.
    Q4    `*` spans keys, buttons and hat (default) or keyboard-only. Q6 dead UI::handleJoyHat out (default, -30). Q11 a button token is
          an INDEX (your ini maps zoom_in to BUTTON7 / 9 / 8 per model): index (default) or a role token later. Q12 no removal hook.
    5.200 the _ALT fallback copies a native pair but DROPS a command list: in eye-relative mode BUTTON1 / 2 / 3 do nothing on every
          controller of your joypad.ini — intent or defect (two lines in init)? 5.201 an empty-string index read (one .empty()).
          5.202 a HATn_D = command(...) line is parsed and never fired — a gap, or the hat is for turning?
    NAMES KeyBinding::Source (Device / Input), Base, KEY BUTTON HAT, JoypadController::name / parse — yours.
  OBSERVATION 4, THE KEY FORM [11.323]:
    P33   F141's 0.44036 / 8.807 (old pin, lost race) was an arithmetic slip: 0.44031 / 8.806 by the law at 200053 km; its 0.44028 holds
          on the new pin and reads 0.44029 on the old (the drawn engine sits 23 km higher). Both measured, both marked.
    5.184 the key form carries no readout: the second member GONE from the keys on both pins; the first STAYS (0.000 on temp_point,
          0 log lines — F-5's zero floor measured on the interactive line: your word on the loud failure). The command form
          (moveto delta_lon from a script or TCP) keeps both — H10's surface alone now.
    5.205 the natural scene lands the Camera 199997.7 km above Saturn on 16 of 17 records and 199993.1 on one: the per-frame rows'
          1e-6 would read a false red on such a launch; NOT widened (the cause's row reads red beside it). A dig of the arrival.
    JUDGE `f131_analyse.py judge 6` writes o6_<tag>.json into its record (F146): README's "read-only" corrected; the write out of the
          replay path is obs 6's design — a B45 item.
  THE PROTOCOL: §0b.3's PREMISES rule now reaches FIELD files (85c3d12) on F151's instance (a prose claim about initial.sts, no premise);
    the supervisor's clock stamps at five sites were typed from estimation (F152's F6, Q-67) — corrected from the commits' clocks;
    Q-72 instance 8 (a lost `cd`, the last call of a batch again).
  ```

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
