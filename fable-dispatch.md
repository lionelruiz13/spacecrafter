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

**Update [Fable 2026-08-30, supervising session 16]:** trigger = the §0b
verbatim line, PLUS an in-conversation Vixy transmission recorded BEFORE
anything else moved (→ **§11.169**, commit `32066fa`: the log-content
schema — each proactive/implicit decision logs its CAUSE + CONTENT + a
self-contained OVERRIDE action; each user-facing error logs WHAT +
CONSEQUENCES + PREVENTION; valid values enumerated or their SHAPE stated;
D12 + §2(f) annotated in place with the element-by-element delta). Round
of 3: **F53 → F54 → F55** — the session-15 queue in order (items 1–3);
items 4–6 (F49's two named-not-run audits · the §5.5-class staleness
sweep · F52(k)'s two git-only checks) plus the NEW §11.169 schema audit
are extension candidates if health permits, minted then per §0b.2; item 7
(§5.100's fix) stays blocked — NO Vixy commit since session 15's close
(harness HEAD at open = `ea30749`, my own close commit; every commit
since session 13's authorization question is Claude-authored), so the §3
authorization stands unanswered and the decision-gated set is unchanged
(D-set verified EMPTY at the file, 2026-08-26 note stands). Warm-up: both
trees clean at open, code `d6aec251` / harness `ea30749`; binary current
(code HEAD unmoved since the 2026-08-26 rebuild, binary mtime
2026-08-26); `free -g` = 47 GiB ⇒ -j12 OK; **definition-drift assert:
md5 MATCH** (`f9446618`). Next free §11 number verified **170** (§11.169
minted at this open). Picks, rows re-read at the ledger: F53 = **§5.113's
owed one-launch measurement** (the three shipped reaches of the unguarded
singleton, extra legs on f50_selvars' driver), F54 = **§11.167(i)'s seven
never-recorded gate values** (five files; recording fix, decision-free —
no threshold moves, no re-baselining), F55 = **§11.167(j)(2)'s first-60 s
sampler** (the launch window no instrument has ever observed). Order =
recorded queue order; surfaces mostly disjoint (measurement legs + row
annotation / five harness files' recording paths / one new sampler
script) — F54 touches `f14_meridian.py` whose gate F55's window
discussion cites, so F54 runs before F55 per queue order and F55 reads
F54's delivery state. NO mandate touches product code — the code tree
should be clean at every point, the third all-record/instrument round in
a row. Round-open events: **§11.169 recorded** (above); **archival
pass 9** (update-s14 + F48–F52 moved byte-exact, manifest
`2026-08-30-pass9`, commit `febc8fd`). Instrument baselines at open,
re-derived from the committed instruments on this tree: pair-check
**185/160/25/85** (+1/+1/0/0 from §11.169, four test counters unchanged);
back-marker scan **90/125/83** — §11.169 added NO event lines and the
unmarked count is the F52-acceptance invariant to the digit.
**Round outcome (session 16 close, 2026-08-30):** F53 → §11.170 · F54 →
§11.171 · F55 → §11.172 — three executor tasks, all delivered AND
supervisor-verified same day; round closed at 3 per the recorded plan (the
CONVERSATION was this round's extension: §11.169 at open + §11.173 at close +
§5.114/§5.115 minted + Q-50–Q-55 discharged — the largest owner-testimony
harvest of any session). **CODE TREE CLEAN THROUGHOUT** — `d6aec251` at open
and close, the third all-record/instrument round in a row. HEADLINE
FINDINGS: **the dim Moon does not reproduce** (F51's driver returns July's
165.258 from a bit-identical dump, same driver/binary/boot — the driver
attribution REFUTED as stated, §11.164/§11.167 present-tense corrected at
four nodes; the farm's never-isolated texture cache is the surviving lead);
the first-60 s window observed for the first time (43.4 s of every launch —
the transient is A42/A43's preview→big swap, now with its time domain);
§5.113's three reaches measured well-formed (the fail-loudly argument
sharpened); the seven gate values recorded (one gate's margin proved
UNSTABLE, not shifted — 0.199 swing on a 0.25 bar); f14's scene does not
reproduce run-to-run (two-run rule for NCC margins). NEW rows: **§5.114**
(illuminate g/b swap, minted at acceptance) · **§5.115** (script-log
unbounded growth, fork collapsed by the testimony chain). Criterion-
integrity instances this round: **5** (F53's G16 self-catch · F54's
self-refuting red + my baseline correction · F55's prospective repeat +
P7 refuting its own premise) — the class counts 18 across sessions.
Supervisor-error tally: the F53 prompt's missing mint license (repaired,
license restored) · the F54 executor's stale post-state baselines
(corrected by measurement; baselines-as-last-act adopted) · two wrong
side-ideas assumptions (untracked/unrouted — channel healthier than
modeled) · the "binary consultation" over-collapse (owner-corrected) ·
the §5.115 "unrecoverable" clause (owner-refuted, amended) · one bare-`cd`
cwd trap (loud-caught). Scan-owner rulings: discharge vocabulary stays
outside the supersession lexicon BY DESIGN; proximity-credit named as the
third family property, instrument frozen this round, strict-credit v2
queued as a deliberate re-baselining act. Stable baselines at close:
pair-check **188/163/25/87** · scan **106/136/83** with TWO named
exceptions (`§11.171→§11.165` benign-in · `§11.164→§11.167`
proximity-out; 83 real, twice attributed). **[Post-close state at
`c193a62`: pair-check 190/165/25/87 · scan 106/137/84 — the 84th is
`§11.172→§11.174`, a THIRD co-occurrence exception (the §11.174
annotation's citation inside the window of the paragraph's pre-existing
CORRECTED; direction inverse, the annotation IS the marker). 83 real,
three named exceptions — the strict-credit v2 case strengthens.]** **Next-round queue, in
order:** (1) ~~texture-cache instrumentation + farm isolation (md5 manifest
in every preflight; the dim-Moon discriminator; S)~~ **[RESHAPED post-close
per §11.174: the ENVIRONMENT CANARY + cache manifest + the affected-claims
correction sweep across §11.156–§11.168/§11.172 — the sweep GATED on the
owner's §11.174(c) answers; the canary (display fingerprint + photometric
band) is dispatchable regardless; M]**; (2) the tester-model
corpus validation (Q-55's five conditioned metrics; predictions-first;
S); (3) the §11.169 schema audit of existing diagnostic sites (instances
banked: the camera-line, the illuminate clamp, the bulk-script D12 shape;
S); (4) F49's two named-not-run audits (§5-side twin; multi-claim; S);
(5) the §5.5-class row↔row staleness sweep (F47 acceptance; S);
(6) F52(k)'s two git-only checks (S); (7) the scan strict-credit v2 +
re-baseline (deliberate instrument change; S); (8) §5.100's fix IF the §3
authorization lands. DECISIONS_PENDING open set at close: **still EMPTY**
— the round's Vixy items ride §3 + their rows.

**Update [Fable 2026-08-30, supervising session 17]:** trigger = the §0b
verbatim line. Round of 3: **F56 → F57 → F58** — the session-16 queue in
order (items 1–3), item 1 taken in its post-close RESHAPED form with its
sweep gate LIFTED by §11.174(j) (*"the correction sweep proceeds
regardless"* — the (c) arms were answered by measurement at (f)–(j) and
the live chase closed at six exonerations). Items 4–7 (F49's two
named-not-run audits · the §5.5-class row↔row staleness sweep · F52(k)'s
two git-only checks · the scan strict-credit v2 re-baseline) are
extension candidates if health permits, minted then per §0b.2; item 8
(§5.100's fix) stays blocked — NO Vixy commit since the session-13
authorization question (every commit through `3703e54` is
Claude-authored), D-set verified still EMPTY at the file. Warm-up: both
trees clean at open, code `d6aec251` / harness `3703e54`; binary current
(code HEAD unmoved since 2026-08-26, binary mtime 2026-08-26); `free -g`
= 23 GiB ⇒ -j12 OK, though NO build is expected — the fourth
all-record/instrument round in a row if it holds; **definition-drift
assert: md5 MATCH** (`a5a54d94`). Next free §11 number verified **176**
(§11.175 last, minted post-close). Picks, rows re-read at the ledger:
F56 = **the environment canary + cache manifest + dim-era correction
sweep** (§11.174(e)(d)(j) + §11.172(i), the §11.159(k7) healthy-stack
cadence echo riding; the §11.174(f) canonical-display fork stays
OWNER'S — the canary banks on :2 with the fork flagged), F57 = **the
tester-model corpus validation** (Q-55's five conditioned metrics,
predictions-first, out-of-sample partition binding), F58 = **the
§11.169 schema audit** (the gap table as the fix's work order). Order =
recorded queue order; surfaces disjoint (harness instruments + INTENT
annotations / read-only corpus analysis / read-only code census); NO
mandate touches product code. Round-open events: **archival pass 10**
(update-s15 + F53–F55 moved byte-exact, manifest `2026-08-30-pass10`,
commit `28ca553`). Instrument baselines at open, re-derived from the
committed instruments on this tree: pair-check **191/166/25/87**
(+1/+1/0/0 from §11.175 vs the recorded post-close 190/165/25/87);
back-marker scan **106/137/84** — unchanged to the digit, the 84th
still the named benign co-occurrence (83 real, three named exceptions).
NOTE for F56's delivery: the sweep MIGRATES the scan's numbers by
design (every SUSPECT-STACK marker adds event lines/pairs) — the
delivery owes the delta attribution, and the strict-credit v2
re-baseline (queue item 7) stays a SEPARATE deliberate act.

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
     `:2` virtual monitor 59.96, panel 164.5) — ~~sharpest hypothesis: the dwell's
     "20 s" denominator was nominal and the true cadence is EXACTLY the config cap
     (§11.123(o2), H1; discriminating check owed by the next cadence-touching task:
     wall-clock-bracketed counter reads). Until settled:~~ **[DISCHARGED 2026-08-29,
     F45 §11.159(k7), accepted by this item's owner: 4392 frames over a [30.0,
     30.5] s bracket = [144.0, 146.4] fps, 4× reproduced ⇒ H1 CONFIRMED on the
     CURRENT (post-reboot, F43-rebuilt) stack — cadence = the config cap. The
     historical "161.3" label is permanently unattributable: its stack was
     destroyed by the 2026-08-27 reboot (§11.121(m)'s retroactive shape).]**
     Standing: trust counter RATIOS and
     in-run A/B only; never absolute fps labels, never cross-session cadence; a
     stack change (compositor, streamer, headless X, screen power state) ⇒ report +
     re-baseline.
   - **Environment-fault mitigations are OWNER VETO ITEMS (2026-08-30, §11.174(h),
     owner-stated: *"If the F43 agent reported the failure over mitigating it, I
     would have corrected it"*):** any improvised substitute for missing host
     state (display stack, runtime dir, auth) is reported WITH AN EXPLICIT
     DECISION FLAG — "your correction may differ from my mitigation — say the
     word" — never only as an accomplishment record; the owner's proper fix and
     the mitigation may differ in preconditions the verification surface does
     not reach (*"different path doesn't certify same preconditions"* [vixy]).
   - **ENVIRONMENT CANARY (2026-08-30, §11.176; enumerated §0.7 precondition per
     §11.175(e)):** before any measuring launch, run `harness/f56_canary.sh` (full,
     ~100 s, for photometric tasks; `--no-scene`, seconds, otherwise). Non-zero exit
     = STOP and report per §11.174(h) — never mitigate silently, never widen the band
     to make a run pass (re-banking is one VALUES-block edit WITH an argument; the
     :2/:4 canonical-display fork is the OWNER's, §11.174(f)). Band on `:2`:
     165.258/6.644 new · 160.142/6.603 old, ±1.0/±0.15. §11.104(d)'s numbers are
     TARGETS AGAIN, but only on a canary-passed stack (§11.164(l)(2) refuted at its
     node); 61.431/42.476 stay non-targets — the canary refuses them by construction.
     A/A floors are PER SCENE (374 px = star field; 28 px = Moon frame, cross-epoch).
     The shared `~/.spacecrafter/cache` is WATCHED, not isolated (`f56_manifest.py`).
   - **Question routing by stratum (2026-08-29, §11.161(c), owner-stated):**
     old-behavior intent/expectation questions → the main tester (*"he either
     knows or tell what he had always expected, both are a resolution"*);
     Vixy-strata questions (experimentalModule, EntityCore, CoI/big-texture,
     Vulkan layer, rendering paths) → Vixy's recall. The stratigraphy is
     §11.161(b).
   - **Back-marker at the write (2026-08-29, §11.161(g), RA-MODEL E18/E55):** any
     entry that supersedes/refutes/corrects ANOTHER entry's claim carries the
     §11.113(p) back-marker at the superseded node IN THE SAME COMMIT — every
     executor prompt binds this; the §11.156(g) five are the recovery backlog,
     queued.
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
   mutates before it passes.** Enumerate every premise the task section and the dispatch
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
   "clean expected" is an expectation, not knowledge). **Definition-drift assert
   (2026-08-29, §11.161(g)):** `md5sum claude/agents/opus-xhigh.md
   .claude/agents/opus-xhigh.md` must MATCH — the tracked file is the authority, the
   `.claude` one its deployed projection; on mismatch, regenerate the projection from
   the authority before any dispatch. The 2026-07-25 supersession-block-in-every-prompt
   era is CLOSED: prompts now carry only per-round variables (HEADs, date, next §11
   number, task pointer, live section count, task boundaries).
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
   **Preconditions in checkable form (2026-08-30, §11.175):** the prompt's per-round
   variables and the task section together must state every premise the task stands on
   (HEADs, next §11 number, live `### F` count, ledger states built upon, environment/
   instrument requirements, dependency deliveries) such that §0.7's gate can verify each
   against live state — the executor aborts on any broken one, so an UNSTATED
   precondition is a dispatcher defect (ungateable = uncovered; §11.174's canary gap is
   the shape). Refresh the variables at dispatch time, not at round-open.
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

~~**Supersession-block TEMPLATE** (the executor's standing definition is stale — every
prompt carries this, values refreshed): intent authority =
`/home/claude/spacecrafter/claude/INTENT.md`, expanded entries `claude/INTENT/<id>.md`
(NO `src/experimentalModule/INTENT.md`, NO §12, NO `dispatch-2026-07-19.md`); delivery
record = §11 entry at the next free number ⟨N⟩ + §13/§5 flips + this file's WIP line at
every checkpoint; today's date ⟨date⟩; `claude/` is its own repo — code committed
first, harness carries `Code: master-beta @ <sha>`; memory-bounded builds per §0.5;
no `run_in_background`; both HEADs stated ⟨code, harness⟩.~~
**[SUPERSEDED 2026-08-29, session 15 warm-up — the era this template served is CLOSED
per §0b.1 + §11.161(g): the standing definition is RA-reprojected (tracked authority
`claude/agents/opus-xhigh.md`, md5-guarded projection), so prompts carry per-round
variables only (HEADs, date, next §11 number, task pointer, live `### F` count, task
boundaries). Struck-not-deleted: the variable list above remains the historical record
of what a prompt had to carry while the definition was stale.]**

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
pass 7) · F42 §11.156 · F43 §11.157 · F44 §11.158 · F45 §11.159 ·
F46 §11.160 · F47 §11.163 (session 14 + its post-close extension — all six
DELIVERED and accepted; archived pass 8) · F48 §11.164 · F49 §11.165 ·
F50 §11.166 · F51 §11.167 · F52 §11.168 (session-15 round — five for five
DELIVERED and accepted; archived pass 9) · F53 §11.170 · F54 §11.171 ·
F55 §11.172 (session-16 round — three for three DELIVERED and accepted;
archived pass 10). Session-17 round (2026-08-30): the session-16 queue
minted in order as **F56** (environment canary + cache manifest + the
dim-era correction sweep) · **F57** (the tester-model corpus validation)
· **F58** (the §11.169 schema audit) below. Extension candidates: F49's
two named-not-run audits (§5-side twin; multi-claim) · the §5.5-class
row↔row staleness sweep (F47 acceptance) · F52(k)'s two git-only checks ·
the scan strict-credit v2 + re-baseline (deliberate instrument change).
Still blocked: §5.100's fix (authorization unanswered).*

### F56 — The environment canary + the cache manifest + the dim-era correction sweep (§11.174(e)(d)(j), §11.172(i)) [M]

**Why now / mandate:** §11.174's disposition at (j): the live chase ended with six
exonerations and a latched-state model — *"the ENVIRONMENT CANARY inherits the
watch with instrumentation the dim era never had"*, and *"the correction sweep
proceeds regardless — the dim-era numbers are wrong whatever latched them"* (the
(d) gate on the owner's (c) answers is LIFTED by that clause; the fix-timestamp
and mechanism arms were answered by measurement at (f)–(j)). Queue position 1
(recorded at session-16 close, reshaped post-close per §11.174). Three folded
parts + one echo, one task because they share the launches and the entry: the
canary is §11.157(f)'s missing verification-surface member; the cache manifest
is §11.172(i)'s instrument gap; the sweep is §11.174(d)'s partition done
properly.

**Sources (re-read, never recall):** `INTENT/11.174.md` IN FULL ((e)(g)(j) carry
the accumulated canary spec: display fingerprint + loginctl + nvidia-smi VRAM
snapshot + RDP-connection snapshot (the (j) `ss` pattern) + dispatch-method
fingerprint + the photometric reference band **165.258/6.644 new · 160.142
old**, `f51_run.sh`'s scene; (d) carries the sweep partition; (f) carries the
canonical-display fork — OWNER'S, unanswered); `INTENT/11.172.md` (i)(k) (the
cache gap: `b3_farm.sh` symlinks `cache/`, a `.dat` measured rewritten mid-run;
the 15-s hostile window; 61.431/42.476 are NON-TARGETS); `INTENT/11.159.md`
(k7) (the cadence H1 discharge whose measurement is now faulty-stack — the
healthy echo this task pays); `INTENT/11.157.md` (f) (the substitute stack and
what its verification surface missed); §0.5 in full.

**Preconditions (checkable, §0.7 gate):** code HEAD `d6aec251`, tree clean;
harness HEAD as the dispatch prompt states, tree clean; binary
`build-claude/src/spacecrafter` present, mtime 2026-08-26 (no rebuild owed);
next free §11 number **176**; live `### F` count **3**; display stack per §0.5
post-reboot (`DISPLAY=:2` forced + `XAUTHORITY=/tmp/rt-claude/.mutter-Xwaylandauth.*`,
`xdpyinfo` answers 2448x1332, gnome-shell pid **147372** alive);
`~/.spacecrafter/cache/` exists as a real shared directory (not farm-isolated);
§11.174(d)'s partition UNSWEPT (no SUSPECT-STACK markers yet at
§11.157/§11.159/§11.164/§11.167); healthy reference values dated 2026-08-30;
instrument baselines at dispatch as the prompt states. ANY broken ⇒ abort per
§0.7.

**Scope:**
1. **The ENVIRONMENT CANARY** (new `harness/f56_canary.sh` + helper as needed):
   one preflight instrument that (a) fingerprints the display stack (DISPLAY
   target + `xdpyinfo` geometry; compositor pid + birth time; `loginctl`
   session table for claude; `XDG_RUNTIME_DIR`; `/tmp/.X11-unix` map), (b)
   snapshots GPU state (`nvidia-smi`: used MiB, util, process list), (c)
   snapshots RDP connection state (`ss` ESTAB 127.0.0.1↔3389, one-shot — the
   (j) pattern), (d) records the dispatch-method fingerprint (claude logind
   sessions present y/n), and (e) runs the `f51_run.sh` reference scene and
   asserts BOTH paths inside a band around 165.258/6.644 · 160.142 — band
   width DERIVED from the measured spread (the (j) bracket: six runs to the
   last digit) and stated with its argument, never guessed. Loud non-zero exit
   on any member out of band, printing observed-vs-banked per member; the
   canary is itself a diagnostic surface — §11.169's elements apply (cause,
   content, override). **Banking decision, made here and flagged for the
   owner:** the band banks on **:2** (harness default; every healthy value was
   measured there) — the §11.174(f) canonical-display fork (:2 substitute vs
   :4 real session) is the OWNER's; re-banking on :4 is one values-block swap,
   and the entry says so.
2. **Instrument chain, both directions:** the canary demonstrated ABLE TO
   FAIL — (i) fingerprint arm against a deliberately wrong expectation (or a
   dead DISPLAY), loud exit shown; (ii) photometric arm: the committed
   dim-era value class (61.431) asserted against the band and REFUSED. Both
   demonstrations committed as artifacts.
3. **The CACHE MANIFEST:** a recursive md5+mtime manifest of
   `~/.spacecrafter/cache/` taken pre/post inside the canary run (and usable
   standalone in §0.5 preflights); mutations REPORTED with per-file deltas,
   never prevented (isolation would change what runs measure — cold-cache is
   a measurement-condition change, OUT of scope). Demonstration: one launch
   bracketed by manifests, catching the known in-run texture-cache write
   class (§11.172(i): `t-bodies-moon_normal.dat` rewritten mid-run) — record
   as it comes.
4. **The CORRECTION SWEEP** (§11.174(d) done properly): walk §11.156–§11.168
   + §11.172 entry files. Per entry, a claim-level verdict table (committed
   artifact): each measured claim classified **SUSPECT-STACK** (photometric
   absolute from a 2026-08-29 dim-era session) / **RELATIONAL-SURVIVES**
   (ratio, in-run A/B, event-locking — argument stated PER CLAIM, never as a
   class waiver) / **CLEAN** (text/INTENT/git channel). Every SUSPECT-STACK
   claim gets a back-marker-class annotation AT ITS NODE citing §11.174
   (§11.161(g): UPPERCASE keyword, citation inside the span, same commit;
   originals preserved). The (d) members are the FLOOR, not the list:
   §11.157's stack-verification values + b24_select adjudication, §11.159's
   A/A floor (374 px) + the cadence H1 discharge measurement, §11.164's
   ladder/centre-luma numbers, §11.167's series + dark-class analysis —
   *"nothing grandfathered without the sweep"*. §11.174(d) itself annotated
   SWEPT with the table's pointer.
5. **The healthy-stack cadence echo** (pays §11.174(d)'s owed member): repeat
   §11.159(k7)'s wall-clock-bracketed counter read on the current stack, one
   launch — H1 predicts the config cap (144 ⇒ [144.0, 146.4] fps on a
   [30.0, 30.5] s bracket). Record as-is: confirm ⇒ the discharge's logic
   stands re-measured on a healthy stack; miss ⇒ reopen AT THE NODE, never
   absorbed. The A/A screenshot floor (374 px, same faulty-stack list) MAY
   ride an existing launch as a same-scene capture pair if cheap; else
   named-not-done with the boundary stated.
6. **Record:** §11 entry at **176** + stub, entry-file-first (§11.156(f));
   artifacts under `harness/artifacts/f56/` force-added; back-markers same
   commit; README section for the canary. The canary becomes an enumerated
   §0.7 precondition per §11.175(e) — the §0.5 standing-line edit itself is
   the SUPERVISOR's at acceptance, not this task's.

**Boundaries:** NO product code, NO data writes, NO re-baselining of any
existing harness gate (the canary band is a NEW instrument's initial banking);
real `~/.spacecrafter` READ + manifested only; temp-HOME farm for scene runs;
§0.5 discipline throughout (fresh launch, md5 in==out, comm-probe 0, plain
`timeout`, no `run_in_background`). Mint license: a NEW distinct-mechanism
defect found en route = mint per §5.79's criterion, else list it on the row it
belongs to (the F50 form).

**DoD:** canary committed + both fail-directions demonstrated; band banked
with the spread argument + the :2/:4 fork flagged; cache manifest demonstrated
on a real launch; sweep verdict table complete over §11.156–§11.168/§11.172
with markers at every SUSPECT-STACK node and §11.174(d) annotated; cadence
echo recorded either way; §11 entry + stub; both instrument baselines
re-derived as the LAST act before the delivery commit with deltas explained
(the sweep migrates the scan — attribute the delta); code tree clean at close;
WIP per §0.6.
**WIP:** DELIVERED 2026-08-30 → **§11.176** (`INTENT/11.176.md` + stub). All six scope
items done, DoD met. Canary `f56_canary.sh` committed with `f56_band.py`,
`f56_manifest.py`, `f56_starfield.py`, README section, artifacts `harness/artifacts/f56/`;
band 165.258/6.644 · 160.142/6.603 ±1.0/±0.15 DERIVED (spread 0.000 over nine launches),
banked on :2 with the §11.174(f) fork left OWNER'S; both fail directions demonstrated
(geometry→2, dead display→3, dim-era json→1 with 152/152 refused, no-members→4, green
control→0). Cache manifest: P3 REFUTED, zero mutations — the §11.172(i) write is EPISODIC
(instrument mapped both ways). Cadence echo PAID twice (4392 lines / [30.0, 30.500] s).
Star-field null: today's frames BYTE-IDENTICAL to four dim-era ones ⇒ the dim state never
touched the non-textured channel; the 374 px floor reproduces and is per-scene. Sweep
table over 14 entries: 17 suspect clusters in exactly §11.157/§11.164/§11.167, "likely
clean" true by CHANNEL (F44/F47/F50 launched on the faulty stack and read no pixel).
Follow-up run: **b3_ladder GREEN** — §11.104(d)'s July numbers reproduce, §11.164(l)(2)
refuted, the driver bump measured photometrically inert. 19 markers + 6 stub mirrors,
§11.174(d) SWEPT, (e) fix (2) delivered (fix (1) still queued). Code tree clean at
`d6aec251` throughout. Baselines at delivery: pair-check **192/167/25/87** (+1/+1/0/0, entry-first signature) · scan **113/148/87** (+7/+11/+3, all this delivery's own markers; real arrears unchanged at 83, six named exceptions) — attributed in §11.176(l).

*(ACCEPTED by supervisor [fable 2026-08-30]: §11.176 read in full; commits/authors/scope
checked (chain `88a81b4 → b556654 → 0a76ad6 → e752760 → 5fc4200 → ef45862 → fa5d789 →
82d2ada → d3f8cab`, harness-only, all carrying `Code: master-beta @ d6aec251`, code
clean throughout); prediction md5s `e46040b1`/`30edf511` confirmed by my own hash;
`f45_run.sh`/`b3_ladder_run.sh` verified UNMODIFIED at git; marker placement verified
at the diff (`fa5d789`: six entry files + three INTENT.md stub-region hunks — §5/§13
untouched as claimed); both instruments re-run by me, recorded post-states reproduced
to the digit (**192/167/25/87** · **113/148/87**), and the F54 lesson HELD — baselines
re-derived after the last follow-up commit, fixed-point confirmed. Judgment calls
ENDORSED with their arguments: the ladder run beyond the mandate's letter (the sweep's
own table named it, one launch, no constant moved, P9 committed first with three
distinguishable outcomes — the round's headline: b3_ladder's ENTIRE residual was the
dispatch fault, four tasks' cost re-attributed, §11.164(l)(2) refuted at its node);
the RE-MEASURED CLEAN fourth verdict class (paid vs argued is a real distinction
§11.174(d) itself owed for §11.159); `--samples 2` (a preflight, not an experiment;
spread 0.000 measured over nine launches); frame pruning (I2 — byte-identical to the
committed ladder frame); instrument reuse unmodified (I2, one authority per claim);
the two in-run instrument fixes (both recorded, both re-verified — "an instrument must
not report a stack fault it did not measure" is the chain discipline verbatim); the
named-not-done four (each with its boundary); the no-mint ×3, each argued at §5.79
(the dim state stays a mechanism-less symptom — §5.2 class — with its NEW reach bound
banked at (e): the star channel never went dim, so every global-output-transform
candidate is eliminated). SCAN-OWNER RULINGS at this acceptance: (1) the
stub-collision property (§11.165's new record, 79 ambiguous ids) — the instrument
stays FROZEN; its two-line fix JOINS the queued strict-credit v2 as ONE deliberate
re-baselining package (the family now counts FIVE named properties); (2) the two
source-edits ENDORSED with the boundary made explicit: legitimate ONLY on the
delivery's OWN authored text and ONLY accuracy-increasing — historical evidence stays
untouchable, the F49 ruling unmoved; (3) next runner's baseline **192/167/25/87** ·
**113/148/87**, 83 real, SIX named exceptions. Executor tally, minor: the WIP's "19
markers" is the stale pre-`d3f8cab` figure (the entry file wins — the derived-label
rule applying to the WIP itself). SUPERVISOR ENACTMENT in this commit: the canary
becomes an enumerated §0.5 standing precondition (§11.175(e)/§11.176(k)(8) — one
bullet, cheap to reverse). Newly Vixy's, routed at close: the :2/:4 canonical-display
fork now has a live instrument waiting on it; §11.174(e)'s fix (1), the host-events
channel, planned as a close-time supervisor act. F57 next, §11 number 177.)*

### F57 — The tester-model corpus validation: Q-55's five conditioned metrics, predictions first (§11.173(d)) [S]

**Why now:** §11.173(d) records the tester cognitive model (triage-default
throughput, value-per-thought interface rule, belief-vintage) as
VINTAGE-TAGGED AND FALSIFIABLE, with the measurement corpus identified and the
task sketch minuted in Q-55 — and the final tester pass INHERITS the model's
consequences (question shapes, conflict rules), so testing it before that pass
is load-bearing. Queue position 2 (recorded at session-16 close).
Predictions-first, the F52 form.

**Sources (re-read, never recall):** `INTENT/11.173.md` (d) IN FULL; the Q-55
body at `~/shared/QUEUE.md` (grep `Q-55`; the five metrics m1–m5 AND both
calibration cells are THERE — read, never re-derive); `USER_QUESTIONS.md` +
`USER_QUESTIONS_ROUND2.md` + `SCRIPT_SURFACE.md` (the corpus);
`INTENT/11.149.md` (d)(e) (the premise-right/instrument-wrong precedent
binding m3; the answer-by-action specimen).

**Preconditions (checkable, §0.7 gate):** code HEAD `d6aec251` clean; harness
HEAD as the prompt states, clean; next free §11 number as the prompt states;
live `### F` count **3**; the three corpus files present, USER_QUESTIONS.md
carrying the 27 verbatim answers (2026-07-21) and ROUND2 the tester's
questions back; Q-55 present in QUEUE.md marked CLOSED with body kept. NO
launches, NO display needed. ANY broken ⇒ abort per §0.7.

**Scope:**
1. **Out-of-sample discipline FIRST:** the model was built partly ON this
   corpus (§11.173(d) names two RETRODICTED signatures). Before measuring,
   partition each metric: what the model already saw (retrodiction — scored
   but LABELED as such) vs what it did not (the real test: m1's variance +
   ambiguity cases, m4's depth markers, m5's conditioned control). Re-scoring
   training data as confirmation is the circularity failure — the entry
   states the partition explicitly.
2. **Predictions committed FIRST** (git, md5 cited in the entry): per metric,
   the model's predicted direction/range conditioned exactly as Q-55 states
   (m1 answer-shape GIVEN question-shape classified first — Q1–27 were
   deliberately decision-shaped, fast answers there are correct behavior; m2
   answer-by-action rate across SS-n; m3 belief-vintage lag WITH the
   instrument-checked-first caution, §11.149(d) cutting both ways; m4
   questions-asked-back as depth markers; m5 Vixy-control conditioned on
   question class — the routing confound honored: structural→Vixy,
   operational→tester BY DESIGN).
3. **Measure all five** on the named corpus; per-metric match/mismatch,
   mismatches TRACED, never absorbed. Calibration cells applied: Q1–27 carry
   a self-reported HIGH-effort cell (shallow-looking answers there are NOT
   throughput evidence); praise scores (quality − prior), not absolutes.
4. **Verdict on the model:** which clauses survive, refine, refute — landing
   as an ANNOTATION on §11.173(d) (belief-vintage stamped); consequences for
   the final-pass question shapes, IF any, ROUTED via §3, not enacted.
5. **PURPOSE BOUNDARY, in the entry verbatim:** interface/routing design
   only — never the person.

**Boundaries:** read-only everywhere (NO product code, NO data, NO launches);
`~/shared/QUEUE.md` is cross-project — read, never edited by this task;
final-pass files not rewritten.

**DoD:** partition stated; predictions pre-committed with md5; five metrics
measured with per-prediction outcomes; §11.173(d) annotated; §11 entry +
stub, entry-file-first; baselines re-derived as the last act with deltas
explained; code tree clean; WIP per §0.6.
**WIP:** —

### F58 — The §11.169 schema audit of existing diagnostic sites: the gap list is the fix's work order (§11.169(c)) [S]

**Why now:** §11.169(c) names it a queue candidate, not run: audit EXISTING
shipped diagnostic/acting-default surfaces against the two schemas (error:
WHAT/CONSEQUENCES/PREVENTION; acting default: CAUSE/CONTENT/OVERRIDE;
self-containment on the action element). Instances already banked: the
camera-line (§11.170(f): `syncCameraReference`'s lone diagnostic fails all
three error elements — the schema's first use as an evaluation standard), the
illuminate clamp (`loadIlluminate`'s `angular_size<1 → defaultSize`, a SILENT
acting default noted at F53 acceptance), the bulk-script D12 shape (§5.115's
known-deliberate, cost-forced silence). Queue position 3 (recorded at
session-16 close).

**Sources (re-read, never recall):** `INTENT/11.169.md` IN FULL ((b) the
schemas + the scope floor; (c) the reach list — §5.110/§5.113/§5.88/B28's
loader validation as first clients); §2.0 D12 + §2(f) at INTENT.md as
annotated; `INTENT/11.170.md` (f); the §5.115 row (the pre-classification);
product code READ-ONLY at `d6aec251`.

**Preconditions (checkable, §0.7 gate):** code HEAD `d6aec251` clean; harness
HEAD as the prompt states, clean; next free §11 number as the prompt states;
live `### F` count **3**; §11.169 (a)–(e) present as described. NO launches,
NO display needed. ANY broken ⇒ abort per §0.7.

**Scope:**
1. **Universe stated and bounded FIRST** (the census is only as good as its
   boundary): the mandatory scope floor is user-facing surfaces — script
   errors and data errors (§11.169(b)); operationally, the emission sites
   reachable from (i) script/TCP command handling (`app_command_interface`/
   `app_command_eval` refusals + diagnostics), (ii) data/config loading paths
   (ssystem/config/star/nebula/sky-culture loaders), (iii) the ledger-banked
   instances. Internal debug channels OUT (the scope floor), stated. The
   universe = a committed site list WITH the enumeration method (grep
   patterns + paths walked) so the boundary is checkable.
2. **Per site:** which schema applies (error / acting default / both, per
   (b)'s double-face) and which elements are PRESENT vs MISSING, cited
   `file:line`, emitted text quoted. Self-containment tested on the action
   element (valid values enumerated, or their shape?). Known-deliberate
   sites (§5.115's bulk-script class) carried as their own category, never
   as naive gaps.
3. **Output = the GAP TABLE as a committed artifact** + summary stats (sites
   by schema, element coverage). The table IS the fix's work order — no
   fixes, no wording changes, record-only.
4. **Record:** §11 entry + stub, entry-file-first; §11.169(c)'s
   queue-candidate clause annotated DISCHARGED (back-marker form). Mint
   license per §5.79 for distinct-mechanism defects found en route — with
   the expectation stated: this audit LISTS element gaps as the work order;
   a missing element is NOT itself a §5 mint (a row needs its own mechanism
   per §5.79's criterion).

**Boundaries:** NO product code changes, NO data, NO launches; harness repo
only for the artifact + entry.

**DoD:** universe bounded + method committed; gap table complete over the
stated universe with per-site citations; banked instances included and
reconciled; §11.169(c) annotated; §11 entry + stub; baselines re-derived as
the last act with deltas explained; code tree clean; WIP per §0.6.
**WIP:** —

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

- **Session-16 decision items (2026-08-30, F53–F55 / §11.170–172 + the conversation → §11.169/§11.173, §5.114/§5.115):**
  - **[POST-CLOSE, 2026-08-30 → §11.174: ATTRIBUTION ARRIVED FROM YOU — sessions
    14–15 were dispatched without a Wayland display; the dim days are the
    faulty-dispatch days; the cache demotes to instrument gap. The item below
    stands for its measurements; its "cannot be A/B'd" clause is SUPERSEDED —
    the dispatch-wrong/dispatch-right A/B is now possible and is YOURS. Your
    internal record's specifics are asked (§11.174(c)): what was missing, fix
    timestamp, session-13 status, deliberate reproduction. Original kept.]**
  - **THE DIM MOON IS NOT THE DRIVER'S — the attribution below is REFUTED as stated**
    (F55 §11.172(e)): F51's unmodified driver, run today from a dump bit-identical in
    32/33 fields, returns **JULY's 165.258 to the last digit** — same binary, same
    driver `580.636.192`, same gnome-shell pid, same boot. The old path healed with it
    (42.5 → 160.1 — BOTH paths were dim on 2026-08-29, which also CORRECTED one of
    F51's four channels). The 2026-08-29 state is real (frames committed) but is now
    an **unattributed one-day state that cannot be A/B'd, minted or fixed until it
    reproduces**. The strongest surviving lead is §11.172(i): **the temp-HOME farm
    never isolated `~/.spacecrafter/cache`** — one shared mutable texture cache under
    every photometric measurement this corpus ever took (measured: a `.dat` rewritten
    inside an F55 run). Cache instrumentation is next-round position 1 so any
    recurrence is catchable. Your driver A/B is OFF the critical path.
  - **A42's trade-off now has both sides priced** (F55 §11.172(c)(d)): the authored
    `moon-preview.jpg` is a DIFFERENT, brighter picture than `moon.jpg`, and it is on
    screen **1.4–3.4 s at every launch that opens on the Moon** — a visible −11.5 %
    photometric step (hf ×1.58) locked to the `big … ready` events on two channels
    with the events *moving between runs and the steps moving with them*. "Align or
    leave" now reads: leave = this transient at every Moon opening.
  - **§5.115 — the script log, fork COLLAPSED by your own testimony chain**
    (§11.173(b)): every recovered preference (the tester's value ranking AND his 2020
    room criterion, your debug-value AND remove-or-generalize) points at **uniform
    bounded retention across all channels**; the 2020 consultation excludes only
    removal, it never saw "bounded". Remaining: ONE tester scalar (window depth,
    decision-shaped question ready) + your density direction beneath it.
  - **§5.114** — every illuminate draws with green/blue exchanged at the loader
    (`(r, b, g)` into a `(r, g, b)` constructor; explicit-colour reach verified).
    One-line fix pending the compensating-swap check; record-don't-fix held.
  - **f23_b33_control's S1/S2 skylock legs are red at `d6aec251`** (F54 §11.171(f)):
    pre-fix binary parts the authorities, current agrees — likelier the seam
    CONVERGED and a defect-demonstration leg outlived its defect; needs the B33
    intent (your stratum) to be read before anyone re-baselines.
  - **[parallel-script] routes to the final tester pass as a RESUBMISSION** —
    your two recovered objections are answered by the file's current text
    (FEATURE_REQUESTS provenance update; one gap: resume/speedup composition
    structural, not yet written as clauses).
- **Session-16 equalization items (facts, no decision asked):** the tester model +
  its same-day refinement (triage-default, value-per-thought, belief-vintage —
  §11.173(d), consequences already operating in this file's question shapes) · the
  first 15 s of every launch are photometrically hostile and now MEASURED (splash
  10.5 s → convergence → preview until 12.6–14.1 s; the generic harness opening
  floor is 6.5 s clear on a 1.4 s-spread interval — §11.172(k)(4)) · the log FILES
  carry `SDL_GetTicks` millisecond stamps (every applog claim in the corpus is
  retro-timeable — §11.172(h)) · 61.431/42.476 join §11.104(d)'s numbers as
  non-targets · the §11.169 log-content schema is now an operating evaluation
  standard (first client §11.170(f): `set home_planet selected`'s lone diagnostic
  fails all three elements).

- **Session-15 decision items (2026-08-29/30, F48–F52 / §11.164–168):**
  - **NEW §5.113 + §5.110's fix routing, one sitting** (F50 §11.166): ONE missing
    truthiness-guard class, three shipped reaches with nothing selected — `set
    home_planet selected` teleports the observer 1 AU and CACHES the fiction
    under the empty name · `flag object_coordinates on` draws a live-looking
    readout for nobody · `illuminate hp <absent>` feeds INDETERMINATE memory
    into the grid. The class question: where does the truthiness test belong —
    each read, the singleton (fail loudly), or both (I6). And §5.110's own
    three-contract fork gained the deciding fact: the composed-selection answer
    is CHARACTER-IDENTICAL to nothing-selected (1 AU, mag −10, vernal point —
    plausible, not error-shaped) while the app distinguishes the two in the
    same frame ⇒ a diagnostic-only repair cannot restore discriminability.
    Rider on the row: `$body_selected` answers the RELEASED body after
    `deselect` (doc's own contract sentence broken) and 999 for any composed
    body.
  - **The dim Moon is YOUR host's driver, as far as measurement can reach**
    **[REFUTED as stated 2026-08-30, F55 §11.172(e) — see the session-16 block
    above: same driver, July's value back to the last digit, BOTH paths healed;
    the shared texture cache is the surviving lead and the driver A/B is off the
    critical path. Original kept below.]**
    (F48 §11.164 + F51 §11.167): the shipped Moon renders mean ×0.371 (locally
    ×0.140), a fifth of the disc below L=32 where July was above 100, from
    BIT-IDENTICAL model state — dated (2026-08-23, 2026-08-26], coinciding
    with the NVIDIA bump `580.568.0 → 580.636.192` and NOT the reboot. The
    texture-upload candidate is REFUTED on four channels (72 frames/one md5
    with a liveness control; registered fine structure; the file's own tint
    reproduced; the source's transient branch). **The driver A/B is the only
    remaining attribution route and it is host state — yours.** Meanwhile the
    harness corpus is mostly immune (px>8 gates; census: 34 CLEAR · 9 FLAGGED
    · 4 FLAGGED-WEAK, §11.167(i)) and §11.104(d)-era absolute photometry is
    permanently non-reproducible.
  - **The 7.73° old/new attitude divergence** (F51 §11.167(f)): same instant,
    same eye distance to nine digits, same spin phase to 1e-5° — and the two
    paths draw different FACES of the Moon (view-matrix attitudes subtend
    7.7346°, most of a visible hemisphere at 10.28° angular radius). Routed to
    you per the §11.161(c) strata rule (rendering paths); deliberately NOT
    minted (`experimental_path` is a dev gate, the shipped default pins the
    new path — no shipped surface shows both).
- **Session-15 veto points (implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) **§11.122's residual closed as posed** at F49
  acceptance (§11.165(d) enacted: the mechanism is §5.59's, the magnitude
  §11.125's — survives as §5.59 (open) + the ARM-C sub-question; one line each
  home to reverse); (2) **`intent_backmarker_scan.py` committed** (the F49
  audit instrument, byte-exact) with the writer-side convention ruling
  (supersessions use an UPPERCASE keyword, citation INSIDE the marker span)
  and, after a second silent-wrong-tree incident, its **root argument made
  REQUIRED** (loud-fail; measurement logic untouched, verified to the digit);
  (3) **§11.157's litguard claim SCOPED at the node** (a proxy scene — verdict
  unthreatened, quantity scene-inclusive); (4) the §11.156(g) five corrected
  to FOUR (the fifth was the scan reading its own quarry backwards — the
  2026-07-25 marker was already yours-compliant).
- **Session-15 equalization items (facts, no decision asked):**
  - **EntityCore was born inside a game** — `LaserBombon` and the library grew
    together six months, separated 2021-12-20 (*"as I learned it"*, with the
    learning vehicle named); **experimentalModule has TWO births** — a 2023-08
    alpha, 720 days of dormancy, the live line from 2025-08-13 (your "~1–2 yr"
    dates the live line to the month).
  - **The stratigraphy is now measured, not testimony** (F52 §11.168): the
    strata ORDER by deletion ratio under a window control and two author
    controls — your own hands, same 11 months: inherited `src/` 0.4712 vs
    module 0.1294 (3.6×). The SPIKE clause is refuted 0-of-4: in EntityCore,
    premise rework arrives as NEW STRUCTURE BESIDE THE OLD (the Taskable
    introduction is a pure addition), not as deletion events.
- **Session-15 awareness, no action needed:** §11.158's two denominators
  corrected in place (501→584 files; 1719→1727 driver-census rows — the two
  extras are llvmpipe) · seven harness gates print values that are recorded
  NOWHERE (cheap decision-free fix, queued) · the first ~60 s of any launch
  remain photometrically unobserved (sampler named, queued) · b3_ladder stays
  deliberately red with its failure now UNDERSTOOD (environment, not
  instrument or product).
  - **§5.109's LAYER half** — `moveto … alt` counts altitude from the
    DISPLAY-scaled datum (measured 7 legs, f43_ramp.py; R7 separates three
    conventions). Should a commanded altitude stand above the DRAWN surface (as
    it does — D21's grounded-children rationale would suggest it) or the
    physical one? D21 is silent on the observer (the §11.149(c6) shape). The
    TIMING half (snapped once, never re-converged) is a defect either way.
  - **§11.4's calibration is now TWO NUMBERED DECISIONS** (F44, §11.158(f)):
    (1) the RA zero point — a constant **−90.0003°**, epoch-independent, with
    declination already matched to 0.0036°; (2) the ORIGIN — old is
    observer-centred (its own doc says so), a view-matrix inverse is
    body-centred; user-visible ~1° on the Moon, invisible elsewhere. With both
    settled + §5.86's fix, new == old to ≤0.002° for 89/90 bodies. RIDER per
    §11.161(c): the origin sub-question ("what does a working user expect?") is
    TESTER-routable — your call whether it joins the final pass.
  - **§5.89's fork is a ONE-SITE decision** (F46: 150 asserts, exactly one
    load-bearing — a project-wide assert policy would re-solve 149
    non-problems), WITH a reach correction: the guard runs twice per startup
    AND from `configuration action load` mid-session ⇒ a refuse-to-start
    repair misses the live route. D15(a)'s boundary intact (EQUATORIAL the only
    defensible named default; VIEW_HORIZON stays config vocabulary).
  - **§5.88's fork datum** (F45 §11.159(i)): the run-time line names the
    symptom, never the file — of the three contracts only *caller checks*
    holds both the file name and the config key. Cost when reachable:
    +1.05% of D11 + 79.3 MB/hour on two flushed streams; zero pixels ever
    (the array never reaches the draw path). Default launch: no cost, no
    report.
  - **§5.90 sharpened, questions unchanged**: the mismatch is a NAME mismatch,
    not an incompatible table (836 distinct spInt values over all 3215 named
    stars, none reaching the present table's 4122) — the data root serves the
    file, confirmed behaviourally. **Routing RATIFIED [vixy 2026-08-29,
    §11.161(c1)]**: the tester is the data's principal author ⇒ the
    FIELD-CONTENT question family (§5.74's three members + §5.90) joins the
    final tester pass, filtered per question by "what the tester knows better
    how to answer"; some historical Vixy answers were themselves
    tester-sourced, so past answers are not evidence against this routing.
- **Session-14 veto points (implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) **§11.156(f) entry-first write order** ratified +
  enacted in the INTENT.md header (one line to reverse; generator RA-MODEL
  E55/E48 per §11.161); (2) **F45's cadence discharge** — H1 confirmed on the
  current stack, the historical "161.3" reclassified permanently
  unattributable; (3) **the fix-shape composition** (§11.161(e)): as-if RA
  restructuring sanctioned for new-path/seam code, old path keeps §11.52(b)
  precedence during parity — one line reverses the composition; (4) the two
  RA protocol lines (§0.5 routing-by-stratum; back-marker-at-the-write).
- ~~**Session-14 offers**: a REPROJECTION DRAFT of `.claude/agents/opus-xhigh.md`
  (your file; RA-MODEL names the per-dispatch supersession block as manual
  resync at the most-traversed crossing) — on request.~~ **AUTHORIZED + EXECUTED
  [vixy 2026-08-29 → §11.161(g)]: the definition is RA-reprojected; tracked
  authority `claude/agents/opus-xhigh.md`, deployed `.claude` projection
  md5-guarded at §0b.1 warm-up; predecessor archived byte-exact; dispatch
  prompts drop the supersession block from the next round. Your one-line veto
  reverses it (the archived predecessor restores by copy).**
- **Session-14 awareness, no action needed**: §5.110 (six `#selected_*`
  answer uninitialized-singleton constants — 1.0 AU, mag −10 — for a composed
  selection; attributed by reading, live check owed) · §5.111 (the new path's
  info strings carry 0 `_()` vs old's 12 — measured in a French session) ·
  Eris: the two trees place it 1.198° apart (§11.3 class, not chased) ·
  b3_ladder is deliberately RED (+1.4–1.6% unattributed drift, discriminating
  check queued) · screenshot A/A floor on this stack: 374 px / 3-of-255 ·
  `flag planets off` does not remove the reference body from the frame
  (measured, unattributed, §11.157(g)).
- **STILL OPEN from session 13**: the §5.100/§5.101 authorization question
  (asked in-conversation 2026-08-26, unanswered) · A44 (ring shadow caster:
  D21 vs the 2026-07-18 extent contract).
- **THE DEPLOYMENT MAP (2026-08-29, on your request → §11.162):**
  `claude/DEPLOYMENT-MAP.md` — the whole space to "tester operates the new
  path transparently", tiered: your decision gates ordered by his operational
  weight (the zoom pair FIRST, tilted-dome §11.92(d), the DSO batch, §11.4's
  pair…), the dispatchable work, the at-his-field checks, the INFORM cargo,
  and the honest UNMAPPED edge (no tester-workflow rehearsal nor soak has
  ever run). One new final-pass question: his CONTENT CENSUS. The map's own
  claim: the decisions are the long pole — every datum they waited on is now
  PAID.

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
