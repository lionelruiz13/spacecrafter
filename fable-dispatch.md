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

**Update [Fable 2026-08-09, supervising session 12]:** round of 3: **F34 → F35 →
F36** — likely the LAST decision-free round: after it the remaining set is
entirely decision-gated. Warm-up: both trees clean, code `204d402e` / harness
`c8fd86d`, binary confirmed current by no-op rebuild, `free -g` = 52 GiB ⇒ -j12;
NO Vixy commit since session 11's close ⇒ the blocked/decision-gated set stands
unchanged (D15/D21 late-Aug, D37, A40–A43, C4 open; §5.64/§5.65/§5.69/§5.70/
§5.72/§5.75/§5.76 fixes stay Vixy's). **B4-row residual re-check DONE at
source, NO mint:** `moveRelativeXYZ`'s only route up is
`CoreLink::cameraMoveRelativeXYZ`; all 6 of its `ui.cpp` call sites are
commented out and no command spelling exists — nothing reaches it; old-path
dead code, retire-with-old class. No §5 row: reachability is the mint
criterion (§5.79's precedent) and this member is UNreachable. Picks, rows
re-read at the ledger: F34 = **§5.80's owed datum** (S-M; record-only, the fix
is Vixy's by the row's own text), F35 = **§5.81 + §5.79 as ONE task** (both
shipped-reachable degenerate-input guards, identical
verify-then-fix-if-decision-free mandate — merged, S+S), F36 = **§5.77's owed
enumeration** (S; the row's named sizing datum; class fix ONLY if uniform +
purely additive). §5.78's owed data-layout half checked for local
dischargability: NO local `spacecrafter-data` checkout — stays Vixy's. Order
F34→F35→F36; order-independence CHECKED: F34 writes no product code (record +
harness only) and runs FIRST so its measurements baseline on unmodified code;
F35 = `ModularBody.hpp` + `constellation_mgr.cpp`/callers; F36 = startup
logging sites — disjoint; sequential dispatch removes the residual risk.
**Round outcome (session 12 close, 2026-08-09):** F34 → §11.144, F35 →
§11.145, F36 → §11.146 — all three delivered AND supervisor-verified same
session; code moved `204d402e → c12ed803 → 04ae1d3e` (F35 only; F34 and F36
are record-only, code clean at open and close). **§5.80's owed datum PAID**
(F34): the teleport lives on the CONVERTER — (A) is used by exactly the three
triple↔cartesian conversion sites and by nothing else; `descend`/`moveEyeRel`
are exact against the composer (corrects the row's own mover list, measured);
the converter is wrong by a SIGN as well as azimuth handedness (one 180°
rotation, the observer's latitude flips); BOTH shipped readouts blind; same
`moveto` lands 16 700 km apart on free-mode alone; fix = free-flight
semantics, Vixy's. **§5.81 + §5.79 CLOSED** (F35): both empty-case answers
decision-free (limit forced by algebra / siblings' empty answer); §5.79's
"not reproducible here" CORRECTED — the shipped one-liner segfaults the
default field (rc −11), now fixed. **§5.77's owed enumeration PAID** (F36):
the class is 37 startup failure reports on FIVE channels; the expected fix
measured NON-additive (`print_log = true` makes `cLog L_ERROR` a second
console writer — four-cell measurement); decision handed as ONE question
(§11.146(j)). NEW rows: **§5.86** (F34) · **§5.87** (F35) · **§5.88–§5.90**
(F36). Executor quality: F34's both-hypothesis scoring with pre-run committed
predictions + the supplementary-swing discriminator; F35's retargeted branch
probe with positive map; F36's comment-stripping census (a grep = 2.2× wrong)
and the four-cell duplication table are the round's method exemplars.
Remaining dispatchable, next round (all S, owed-datum/verify class — every
FIX they enable is Vixy's): **§5.86's owed data** (consumer dependence +
old-path RA/DE parity target), **§5.88's owed datum** (draw-time cost of an
empty spectral array), **§5.89's owed sweep** (other load-bearing `assert`s
in `src/`). Then the decision-gated set unchanged PLUS the new decisions
minted this round (§11.146(j) console duplication; §5.87/§5.88/§5.89/§5.90
semantics/layout halves). DECISIONS_PENDING open set at close: **D15, D21
(late Aug), D37** + A40–A43 awaiting — unchanged; B31 rides D21/D28/C4.

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

*Derived index (regenerable from `fable-dispatch/archive/`): sections **F0–F36 all
DELIVERED and archived** — F0 §11.103 · F1 §11.104/§11.105 · F2 §11.106 · F3 §11.107
· F4 §11.108 · F5 §11.109 · F6 §11.110 · F7 §11.111 · F8 §11.122 · F9 §11.123 ·
F10 §11.115 · F11 §11.117 · F12 §11.118 · F13 §11.119 · F14 §11.120 · F15 §11.121 ·
F16 §11.124 · F17 §11.125 · F18 §11.127 · F19 §11.126 · F20 §11.128 · F21 §11.129 ·
F22 §11.130 · F23 §11.131 · F24 §11.132 · F25 §11.133 · F26 §11.134 · F27 §11.135 ·
F28 §11.138 · F29 §11.139 · F30 §11.140 · F31 §11.141 · F32 §11.142 ·
F33 §11.143 · F34 §11.144 · F35 §11.145 · F36 §11.146. Live sections:
**F37–F39** (session 13, the D15/D21/D37-unblocked round — below). Queued after
them: §5.86's owed data, S; §5.88's owed draw-cost datum, S; §5.89's owed
assert sweep, S — every FIX they enable is Vixy's; to be minted at their
dispatch per §0b.2.*

### F37 — Propagation pass: the D15 + D21 + D37 answers → §11.⟨next free⟩ (record-only + ONE premise probe) [S–M]

**Why now / why first:** all three answers are in the file (D15 `2b24a1b`, D21+D37
`d521807a`); nothing downstream (F38, F39, the tester pass) may consume them
until they are propagated with their [derived] readings and veto points — the
§11.113 precedent, which is the mandate's shape. Record-only: NO code, NO data.

**Sources (re-read, never recall):** `DECISIONS_PENDING.md` §3 D15 (answers
(a)–(d) + the inform-tester line), §5 D21 `[Vixy reply]`, §13 D37 `[Vixy reply]`;
`INTENT/11.112.md` (D15 expansion); §11.101(f) + `INTENT/5.27.md` (D21 corrected
form); `INTENT/11.117.md` (k)(1) + the B39 §13 row (D37); §11.116(c) (final-pass
batching principle, list is ledger-owned).

**Scope:**
1. §11 entry (file + stub) — full record of the three answers; every [derived]
   reading carries its veto point.
2. Ledger flips: **B18** suspended clause → resolved per D15 (four sub-decisions
   recorded; (c)+(d) implementation = F38, in flight this round); **§5.27**
   behavior half → unblocked (D21; fix = F39); **B39**'s D37 residue → the final
   tester pass; **B31** T3 gate note updated (D21 answered); **B35** mount half
   annotated per D15(a) (VIEW_HORIZON a workaround for old's up-vector flip, the
   new path avoids the up vector ⇒ no longer useful on the new path — trace the
   citation set: B18(a), B35, §5.89 is INFORMED not decided).
3. D15(b) ⇒ **§5.80 fix unblock** [derived, veto point]: *"freeMode swapping in
   both ways must be transparent"* reaches the position channel — the measured
   ~125° entry teleport is not transparent — so the converter-as-composer's-
   inverse fix (§5.80's own datum, F34/§11.144) reads as authorized. RECORD the
   reading; implement nothing.
4. **D37 premise probe (the one measurement/read):** Vixy's reply reasons from
   *"every bodies have the star as parent body"* ⇒ hiding the star hides the
   system via D23 nesting. But `b39_star.py` (§11.117(k)(1)) measured the Moon
   DRAWN AND LIT with the Sun hidden, and B39 hides by parked SUBTREE — so on
   the shipped tree the star appears NOT to be the planets' parent. Establish
   the fact (composed twins under `~/.spacecrafter/modularSystem/`, or one
   launch; `/usr/bin/grep` for ini files). Whichever way it lands, the fact +
   the b39_star measurement + both recs travel WITH the tester question — the
   delegation must not propagate on a premise the field data contradicts; the
   divergence is also Vixy-reportable at close (equalize before action).
5. Final-pass list additions (ledger-owned, §11.116(c)): D37 (question, with
   the premise fact attached); D15(a)–(d) (INFORM, with revise/revert offer —
   Vixy's own closing line); D15(b) heading-stays-stable default (CONFIRM).
6. `vixy-side-ideas.txt` (`2b24a1b`): pointer entries into
   `FEATURE_REQUESTS.md` (parallel-script · script-binding · script-trigger);
   the txt stays the authority for its own text.
7. Field-change awareness: code HEAD `f0c8ef83` (Lionel RUIZ, tester,
   2026-08-26) rewrote `doc/superscript.sts` (+267/−67). Cheap staleness probe
   ONLY (do the §5.96/§5.97 witness lines — incl. §5.97's invisible byte —
   survive in the new file?); record hits as annotations, fix nothing.
8. `DECISIONS_PENDING.md`: propagation arrows under all three + header open-set
   update. If the D-set lands empty, SAY so in the header line (a first).

**DoD:** arrows under D15/D21/D37; §11 entry cited by every touched row; the
premise-probe fact recorded with its instrument; final-pass list carries the
new members; no code tree change (`git -C /home/claude/spacecrafter status`
clean at close).
**WIP:** *(cleared at delivery 2026-08-26 — DELIVERED, §11.149; harness `313012b` CP1 / `5f213fd` CP2 / `458a71f` CP3 / this commit. Code tree untouched and clean throughout, as the section requires.)*
*(ACCEPTED by supervisor [fable 2026-08-26]: entry read in full; commits/authors/trailers checked; all claimed flips verified at the ledger by content. Deviations endorsed: SS-18 same-class extension; FEATURE_REQUESTS header repair; the §11.149(h) authority-inversion repair — its owed stub↔entry-file cross-check sweep is NEXT ROUND's first candidate, queued. The (d5) supersession of §11.117(k)(1) is endorsed on its killer datum (Sun `visible: false` in BOTH frames of the instrument's own artifacts) — noting that the refuted reading had passed the 2026-07-30 supervision, mine.)*

### F38 — D15(c)+(d): sky-lock write-site mirroring + init/config structural parity [M]

**Mandate:** D15 answers (c) *"Continual tracking must be preserved and smooth"*
and (d) *"init/reinit must initialize the state, which now include the freeMode
and config.ini must enable to configure it … default attached=True"* — via F37's
§11 record + `INTENT/11.112.md` + B18/B33 rows. Depends on F37 (propagated
record is the authority it cites).

**Scope:**
1. **(c)** The four old-only lock write sites — current census
   `core.cpp:1083/1372/1410/2313` (verified 2026-08-26; §11.112's
   971/2066/1251/1289 drifted — re-verify at dispatch) — route through the
   both-paths mirror (`Core::setFlagLockSkyPosition`), so enable AND disable
   reach the Camera. Old-path flag transitions bit-identical: the change is the
   mirror, never the old behavior. Policy [derived from (c), veto at the site]:
   Camera `target` tracking participates in the select-while-tracking rule
   identically to old `flag_traking`.
2. **(d)** init/reinit initializes the camera state (sky-lock AND freeMode) —
   `core.cpp:400`'s reset-to-0 becomes the both-paths init from config;
   `config.ini` gains the key(s), default `attached=true`. **Naming check is
   Vixy's own veto offer:** verify "attached" against the OBSERVED semantics
   (freeMode vs `boundToSurface` collision risk; `Camera.hpp` is the source) —
   adopt, or flag with the observed meaning. B28 protocol (new key = product
   surface, spelling recorded as veto point). D12: a configured value that ACTS
   at startup (lock on / freeMode on) is logged. D13: the key enters
   `config.ini` (rewritten at shutdown) — verify the OLD parser tolerates the
   unknown key; record the finding either way.
3. Tester-inform obligations ride F37's final-pass entries, not this task.

**DoD:** both-ways discrimination on the shipped sequence (§11.112(c): `flag
lock_sky_position on` → unzoom-to-init; select-while-tracking → release) — both
paths agree in BOTH directions, asserted against the pre-change binary;
old-path observables bit-identical where the old path is the baseline; battery
green; config/ssystem md5 in==out; B18 row flip; §11 entry + stub; WIP
discipline per §0.6.
**WIP:** *(cleared at delivery 2026-08-26 — DELIVERED, §11.150. Code `f0c8ef83 → b444d381`
(one commit, both items); harness `04680d5` CP1 / `2e0660d` CP2 / this commit. **(c)**: all four
sites (`core.cpp:1083/1372/1410/2313` — census adjudicated as pure line drift from §11.112's
`971/2066/1251/1289`, each context re-read at `606d6b87`) route through
`Core::setFlagLockSkyPosition`; measured DESYNC both ways before (disable old 18.0493° vs new
0.0000°, 131 089 px>32 on a 0 px floor; enable the mirror image), ALL PASS both ways on both rounds
after. The veto-at-the-site is answered BY CONSTRUCTION: the tracking clear had to be mirrored at
the three lock sites that carry one, because the Camera's lock is dormant while `target` is set and
the lock mirror alone measured inert at 1083. Old path bit-identical (32 matched samples, every flag
equal, ≤2.72e-05° = the runs' jd offset). **(d)**: `[navigation] attached = true` +
`flag_lock_sky_position = false`, defaults = today's behaviour, `findEntry`-first read; verified on a
temp-HOME farm (4 legs), field config `03fbee59` pristine throughout. **Name veto ADOPTED**
(`boundToSurface` ambiguity recorded at the define, B28). **D13 measured on both parser routes.**
B18 harness green; A–D battery run PRE and POST on one config — only non-numeric delta is
`skyLocked` False→True in C/D/D′, where old's flag was already 1. NEW **§5.100 §5.101 §5.102
§5.103**, all measured, none fixed. **§5.102 is F39's precondition, not an adjacency.**)*
*(ACCEPTED by supervisor [fable 2026-08-26]: §11.150 read in full; commits/authors/trailers
checked (CP1 correctly carries the pre-change code SHA); code diff scope matches the no-Camera-code
claim; all flips verified. Judgment calls ENDORSED with their arguments: the tracking-clear adjunct
(forced — lock mirror measured inert at 1083; boundary held), §5.100 restraint (the fix is a
new-path behaviour change outside the mandate — surfaced to Vixy at close instead), the D13
probe-key substitution (veto recorded), `attached` adoption (the delegated check, executed as
delegated), b18_run probe-count supersession. The census correction to MY dispatch parenthetical
is accepted as a correction of my inference. §5.102 folded into F39's spec as precondition.)*

### F39 — D21: display scaling = presentation layer; grounded children inherit it; physics unscaled [M–L]

**Mandate:** D21 `[Vixy reply]`: *"Unscaled for physics (orbit, shadow received
and casted), scaled for bounding/rendering, grounded children inherit scaling
(to be visually identical to unscaled, given grounded bodies are
surface-relative, so the referential is the body surface)"* — via F37's §11
record + `INTENT/5.27.md` + §11.101(f) + §2(a)'s two-layer rule. Depends on
F37; runs after F38 (disjoint files, sequential per protocol).

**Scope:**
1. Kill the load-time latch (`SurfacePointOrbitLoader.hpp:88/112` `altStart`
   bake): the MODEL layer places grounded children from the UNSCALED datum,
   read live — the closed latch class (B15/B19/B32) provides the shape;
   §11.113(c) already rules regime selection reads LIVE state, never a latch.
2. PRESENTATION layer: grounded children inherit the parent's display scaling —
   including the 5 s ASmooth ramp, live — so surface content stays visually ON
   the displayed surface (referential = the body surface, per the answer).
3. Physics unscaled: VERIFY orbit + shadow cast/receive read unscaled radii;
   where they read scaled today, that is in-mandate — measure before/after and
   record magnitudes. Script-fetch position reads model truth (D8's script
   channel — `ModularBody.hpp:193`'s own "Just visual scaling" comment becomes
   true). Bounding stays SCALED (the answer's own words).
4. Reach: `moon_scale`, `planet_scale`, sun-scale paths share the latch
   (`ssystem_factory.hpp:556/240/267`) — all in scope; §11.101(f)'s untested
   prediction (startup-loaded vs reload-loaded latch different ramp values ⇒
   `body action reload` MOVES a composed rover) is a pre-fix discriminator to
   confirm, then a post-fix invariant.
5. **Preconditions from F38 (§11.150(m)(n)) and the §11.151 ratifications:**
   (i) **§5.102 FIRST**: the `ASmooth scaling` member is NaN on the shipped
   config for BOTH scaled bodies (§11.16's 0/0 class), reaching
   `getAltitudeReference()` → the observer's distance and the drawn view
   matrix (scene C: altitude readout answers NaN beside old's 100 m). Root it
   before building inheritance on the factor. Note the tension to discriminate:
   §11.101(f) measured FINITE ~7000 km offsets on 2026-07-24 — so the NaN is
   config-dependent (fov-340 farm? init order?) or a later regression; date it.
   Fixing it is in-mandate if decision-free (it is the factor D21 makes
   load-bearing); else record + suspend.
   (ii) **§5.103**: `analyze.py` cannot read a NaN dump — adopt old's `jnum()`
   shape for the new path's half (instrument-grade fix, in-mandate) or carry
   F38's null-substitute workaround, stated either way.
   (iii) **§11.151(b)**: the acceptance criterion is RATIFIED — uniform
   dilation, placement AND extent; the (c3) veto is closed, gate without
   hedging.
   (iv) **§5.100/§5.101**: `zoom auto in` / `zoom auto initial` set up
   DIFFERENT scenes on the two paths — never use them for scene setup.
   (v) Init-order guard from §11.150(n): `loadCamera` must stay BEFORE the new
   config-read block in `Core::init`.

**DoD:** the §11.78(a) mandate scene alive under the SHIPPED config
(`moon_scale=5`, no `flag moon_scaled off` workaround): composed rover ON the
displayed surface; `body action reload` no longer moves it; live scaling toggle
⇒ child rides the ramp while script-fetched position is invariant; shadow
observables recorded (unchanged if already unscaled, corrected+measured if
not); the `b24_screen` scaling-off workaround retired or its retention argued;
battery green; md5 pristine; §5.27 behavior half closed; B31 T3 noted
dispatchable; §11 entry + stub; the §5.102 root NAMED (fixed if decision-free).
**WIP:** *(cleared at delivery 2026-08-26 — **DELIVERED, §11.152**; code `b444d381 → fd98c8e9` (CP1, incl. submodule `224eba7 → 7ce5835`) `→ 8c2cbf61` (CP2) `→ 7ef11aca` (CP3) + the record commit; harness `425ca0d → e890ab7 → 1533547 → 993bdce` + the record commit. DoD, item by item: mandate scene ALIVE under the shipped config with **no `flag moon_scaled off`** — rover `|eclDisplay| = 8687.000 km` == Moon `scaledDatumRadius`, 243 118 drawn px + 40 534 px of cast shadow where PRE drew **0 px and still darkened 24 560**; the ratified UNIFORM-DILATION criterion gated as one scalar, **PRE 0.27716 vs POST 1.3858165 ± 1.5e-7 over thirteen legs**; ramp live (`inheritedScaling` tracks Moon `scaling` sample for sample) while the model `ecl` is 1737.400 km in EVERY leg; shadow radii verified ALREADY unscaled (correction magnitude zero) with the ring exception SUSPENDED as **A44**; reload no longer moves it — and §11.101(f)'s prediction is **REFUTED**, the reload drops the scaling entirely ⇒ NEW **§5.104**; the workaround retired as a workaround, retained as a scene declaration with the argument; battery GREEN (A–D pre/post **0 non-numeric differences**, `b24_equivalence` 120/120, `b24_screen` all gates); md5 pristine every run; **§5.27 CLOSED**, §5.102 ROOTED+FIXED at an indeterminate `ASmooth` phase timer (original defect, not a regression — the dating question answered), §5.103 CLOSED; B31-T3 resolved; D11 cost **≤ 2.3 µs/frame**. Also NEW **§5.105** (old path dumps uninitialized memory as `dist`) and, for F40, a free discriminator: `moveto lon` vs `orbit_lon` differ by **60° free / 90° surface**.)*
*(ACCEPTED by supervisor [fable 2026-08-26]: §11.152 read in full; commits/authors/refreshed
trailers checked incl. the submodule gitlink; §5.27/§5.102/§5.103 closures + A44/§5.104/§5.105
mints verified at the ledger. Judgment calls ENDORSED with their arguments: the EntityCore commit
(precedent verified IN the log — Vixy's own `f28c555` "Fix ASmooth"); observer-altitude-not-dilated
as the uniform-dilation consequence (the gate is a ratio for that reason — VETO POINT relayed to
Vixy at close); `flag moon_scaled off` reclassified scene-declaration (baseline preservation);
`getCachedRootPosition` left model-layer (dominated by its own pre-existing grounded-hop limit,
both-halves-at-once requirement stated). The §5.102 date-discrimination answer — ORIGINAL defect,
variance IS the garbage, all three predicted outcomes present in §11.150's own same-day artifacts —
is the round's method exemplar. F40 dispatch condition met: session healthy.)*

### F40 — §5.80: the freeMode converter becomes the composer's exact inverse (D15(b) ratified → §11.151(a)) [S–M]

**Mandate:** §5.80 row + `INTENT/11.144.md` (F34's paid datum: the teleport
lives on the CONVERTER — a missing negation AND azimuth handedness, composing
to one 180° rotation; used by exactly the three triple↔cartesian sites,
`setFreeMode` both ways + `moveTo`'s free branch, and by nothing else) +
§11.149(b) + **§11.151(a)** (RATIFIED: transparency includes the position
channel). Fix: re-express the converter as the composer's EXACT inverse.
`descend`/`moveEyeRel` are already exact against the composer
(5.5e-12/3.4e-12 AU) and must stay untouched, as must every anchored place.

**Boundaries:** the two §11.144 riders are NOT in scope and may not be settled
on the way (§11.151(a)): what `moveto lat/lon` MEANS in free flight, and what
`get status position` answers there. §5.86 (`observedToBodyLocalPos` ≠
`viewMat` inverse — the RA/DE readout) is a DIFFERENT defect, out of scope.

**DoD:** freeMode toggle both ways = 0 px against the A/A control (pre-fix:
6354 px>8, 124.6797° swing, 11 300 km on Earth); `descend`/`moveEyeRel`
unchanged at their measured exactness class; anchored places bit-identical
(regression leg); reversible pair entered twice, second entry from the first
exit's state; battery green; md5 pristine; §5.80 closed; §11 entry + stub.
**Dispatch condition:** after F39's verification, session health permitting
(§11.151(a)); else heads next round's queue with the stub↔entry-file sweep.
**WIP:** *(cleared at delivery 2026-08-26 — **DELIVERED, §11.153; §5.80 CLOSED**. Code
`7ef11aca → 18b6f13f` (one commit, `Camera.cpp`/`Camera.hpp`); harness `64e07d6 → 4c1d690`
(CP1, the algebra before the code) `→ eafdb33` (CP2) `→ this`. DoD item by item: **the toggle
both ways = 0 px against the A/A control** — 0 px at the F34 place, 0 px with the reference in
frame, 0 px at Mars, 1 px at the shipped place flags-off on a 0 px floor, and the observer
moves 0.0009 km / 0.312 m / 0.0031 km where it moved 13 732.4 / 11 298.6 / 39 945.9 km (pre
numbers reproduced FIRST on a pre binary: `f34_convention.py` 27/27 replay + `f40_inverse
--expect pre` 29/29, both to the digit of §11.144); **`descend`/`moveEyeRel` unchanged**
(2.252e-12 / 2.011e-12 AU against F34's own predictions and floors); **anchored bit-identical**
(8 places, 3 reference bodies incl. two `set home_planet` = `placeAt`, 32 dumped camera fields
each, composed screens max abs pixel difference **0** over 2048×2048); **the reversible pair
twice**, second entry from the first exit's state, every teleport ≤ 2.3 m; **battery green**
(A–D with a same-binary phase control, `b24_equivalence` 120 bodies, `b24_screen`); md5
`03fbee59`/`545a51ef` in==out on every run; **§5.80 CLOSED with both riders explicitly open**
(`get status position` measured byte-identical across the toggle — the fix PRESERVES it, and
§11.153(e) shows leaving `getPlace` alone is what would have changed it). **Corrections to my
own dispatch, each argued in §11.153:** the converter has FOUR sites, not three (`getPlace` is
the fourth, and its header already promised it was the same conversion); and the §11.152(o)
discriminator's post-fix target is **the toggle moving nothing**, not "60 stays 60" — 60.0° is
the FREE reading of `moveto lon 60`, i.e. the defect, and the anchored 90.0° is bit-identical by
DoD. Measured on a real authored rover, two independent derivations agreeing to <0.0001°: PRE
90→60→90 and 60→90→60 (span 30.0000° both directions), POST span **0.0000°** on all three legs,
including one placed where the angle IS 60.0° → **60.0000 → 60.0000 → 60.0000**. **NEW §5.106**
(the shipped-flags 3.3 Mpx is `EnvironmentManager.cpp:81`'s free-mode gate, NOT the observer —
§11.144(i)'s attribution refuted at its node, four-cell measurement on both binaries); **§5.49
sharpened and its own conclusion put in doubt** (recorded on that row, fixed nowhere). **Two
harness scenes re-declared** (`b24_screen`, `b24_select`) because free-flight `moveto lon` now
means the anchored thing; 28 more files combine the two and are unaudited — README carries the
list and the arithmetic.)*
*(ACCEPTED by supervisor [fable 2026-08-26]: §11.153 read in full; commits/authors/trailers
checked (CP1 correctly at the pre-change SHA — algebra licensed before the code moved); §5.80
flip + §5.106 mint + §5.49 sharpening + §11.144(i) refutation-at-node verified. The rider
boundary HELD with arguments and veto points at (e)/(f) — the fix restores the header's own
stated contract, both riders reversible in ≤3 lines. Corrections to MY dispatch spec accepted
as corrections: the FOURTH site (getPlace — leaving it would have broken the readout the riders
protect) and the discriminator target (span-zero is the criterion; "60 stays 60" was the
defect's own number — my derivation error). b24_screen/b24_select re-declarations endorsed
(geometry-preserving, invariance shown both ways). Owed next round: the 28-file free-mode
longitude audit + the b24_select 4-red adjudication (pre-existing at `7ef11aca`).)*

### F41 — §5.104 fix: format-scoped scaling ownership (`display_scale`), per §11.154(b)(c) [S–M]

**Mandate:** §11.154(b) (format-scoped ownership, RATIFIED operable in (c)) +
the §5.104 row. Key name **`display_scale`** [vixy], adopted iff its delegated
deducibility test holds against the full format grammar (*"unambiguous in what
it does and doesn't, the difference with radius can be deduced from name
alone; if it doesn't hold, it's not the right name"*) — adopt or FLAG with the
observed contrast, never silently rename (B28; supervisor vocabulary pre-check
holds, §11.154(c)).

**Scope:**
1. Modular format gains `display_scale` (parser; the loader applies it as the
   body's authored display scaling — the authored DEFAULT under the operator's
   runtime `scaling`, §11.152(c)'s non-folding untouched).
2. Twin emission: `generateComposedTwin` (`ModularSystem.cpp:1657`) translates
   the config values (`moon_scale`/`sun_scale`/`planet_scale`) into per-body
   `display_scale` — forced by the reproduce-legacy contract (§11.113(f)
   precedent; §11.154(c)). Twin-only keys ⇒ no user file migrates (D13/D35 by
   construction).
3. Precedence: when the modular format SERVES the system, the file wins and a
   still-present config value is ignored + logged ONCE naming what overrode it
   (§2(f)); legacy-serving behaviour unchanged.
4. Legacy reload: `reloadCurrentSystem` re-applies the config scaling after
   rebuild (§5.104's legacy branch). Modular reload: by construction (D31) —
   verify, don't assume.
5. NO session-ledger machinery (D30 absorbs per-branch, §11.154(b) — out of
   scope).

**Constraints:** old path unchanged (§11.52(b)); no legacy-file writes (D9/D13);
`scaling` equality gates as TOLERANCE (ASmooth settles 5→1 at 1.00000012,
§11.152(o)); never `zoom auto in`/`zoom auto initial` for scene setup; md5
pristine (temp-HOME farm for config variants, `f38_config.sh` pattern);
`dumpread.py` for dumps; free-mode `moveto lon` semantics changed at F40 —
don't reuse stale free-mode baselines.

**DoD:** legacy reload preserves scaling (pre-fix 5→1 measured both binaries;
post 5→5 within tolerance) with the F39 reload-invariance leg re-run green;
twin emits `display_scale` matching config; the activated-twin route (§11.51(a))
applies FILE scaling with the config value deprecated + logged once; the
deducibility verdict recorded (adopted or flagged); discriminators both ways vs
a pre binary; battery green; md5 pristine; §5.104 CLOSED; §11.155 entry + stub;
WIP discipline (`grep -c '^### F'` = 5 before every commit).
**WIP:** 2026-08-26 — CHECKPOINT 1: code landed + green (binary `fd4612ac`), pre
binary staged `/tmp/sc_f41_pre` (`98d09488`, code `18b6f13f`). Measured both ways
on `f41_ownership.py` legs L1-L5: legacy reload 5→5 post / 5→1 pre (§5.104
reproduced), twin emits `display_scale = 5` for the Moon and nothing for the Sun,
file-wins discriminator L3 (config `moon_scale = 2` → post 5 / pre 2), deprecation
logged on the composed legs only and silent when the config value was not live.
FOUND out-of-scope: `ui.cpp:239` hardcodes `setFlagSunScaled(false)` after
`Core::init`, so `flag_sun_scaled` has never acted (both binaries) — new §5 row
owed. NEXT: command-control legs (a command must still act on a file-owned body),
F39 reload-invariance re-run, A-D battery + b24_equivalence + b24_screen, B28
deducibility verdict, §11.155 + §5.104 flip.

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
