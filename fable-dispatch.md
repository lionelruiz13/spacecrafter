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

---

**Update [Fable 2026-08-31, supervising session 18 — TravellingFoxDev, the scedit
round]:** trigger = Vixy's line *"Sequential dispatch: Continue the work on scedit
… test the previous changes landed properly before working on the next
FEATURE_REQUEST.md entries related to scedit and script engine. Use one agent per
feature to implement."* Warm-up: both trees clean, code `4a00cf31` / harness
`f157778`; engine binary at HEAD (`cmake --build -n` empty; last engine commit
`2b8ec034`); definition-drift assert md5 MATCH (`a5a54d94`); next free §11
**185** (live ∪ archive); display `:2` answers with the F28 recipe (this laptop
has a REAL claude login session — §0.5's `/tmp/rt-claude` note is the desktop's);
canary `--no-scene` **2 FAIL by construction** (the desktop's bank: geometry
2448x1332 vs 1920x1080, headless compositor absent) — REPORTED (§3), not
re-banked. PREVIOUS-CHANGES VERIFICATION (the dispatch's first clause): scedit
8/8; F62 11/11; F63 34/34; F61 15/16 → traced to the INSTRUMENT (fps.cpp:150-156,
the watchdog's own SIGUSR1 per stall) and to the HOST (screen lock ⇒ 1 Hz frame
clock, 105 stalls/run on HEAD and on the pre-fix control alike; awake ⇒ 1 stall,
**16/16**; `HOST-EVENTS.md` 2026-08-31); the pre-fix RED control reproduced
(8/16); NEW `f63_scedit_agree.py` 12/12 on engine-written files. Picks (§0b.2,
by mandate position + display availability): **F65** (scedit item 15 a-ii, S) →
**F66** (item 19's scedit-side surface, M) → **F67** (item 6 direct TCP, M —
display-needing, hence in this round while the display exists); scedit tasks
deliver to the MIRROR ledger (stated in F65's header, binding for all three).
NOT dispatchable, listed in §3: the three untriaged engine requests. Remotes:
local CONTAINS origin on both repos (push = ff; fetch refused — auth); the
2026-08-30b non-ff note is superseded. Parent-ledger baselines (pair-check /
scan) untouched by construction: no parent §5/§11 row is written this round
(the mirror ledger is outside both instruments' scope) — re-derive only if a
delivery mints one.
**Round outcome (session 18 close, 2026-08-31):** F65 → scedit journal
`2026-08-31f` · F66 → `2026-08-31g` · F67 → `2026-08-31h` + parent **§11.185** —
three for three delivered AND supervisor-verified same session (every delivery
re-run by me: gates in both build dirs, `--history`/`--doc`/parity/agreement
instruments reproduced to the digit; F67's live legs accepted on their records +
the forced control, not re-launched). Code `4a00cf31 → e2c8477b` (17 executor
commits, no engine source touched), harness `f157778 → 1402dc5` + this close.
scedit: gates **8 → 14** (history_list · check_json · doc_queries · mcp_protocol
· tcp_client · pty_keys), editcore 193 → 266, ui_selftest 13 → 20 frames,
`-Wall -Wextra` real (0 warnings can now fail), the grammar's executes-only
clause; mirror-ledger items **6, 15, 19 (scedit half), 20** closed — D31's
original spec is complete but for the default-greyed ghost DATA (items 11/12).
HEADLINE FINDINGS: a LOCKED screen runs the engine at 1 Hz (105 stalls/run,
both binaries; awake 1 — `HOST-EVENTS.md`); the engine tells a TCP client
NOTHING about a script it plays (§11.185, routed); the Bash `grep` wrapper
skips every ISO-8859 file (rule corrected at its cause, three homes → one file
+ §0.5); scedit's reading agrees with the engine's `#!` verdict on every line
the engine annotated (12/12, then through `--history`); F64's hand-written
family map missed `font`; the MCP "latest" revision (2026-07-28) has no
handshake while the deployed client speaks 2025-11-25 — dual-era, measured.
SUPERVISOR-ERROR TALLY: **three dispatcher glosses** (the unarmed "0 warnings"
bar · the recalled MCP `initialize` · `INTENT/5.47.md` named as a source) — all
executor-caught via §0.7's report-not-absorb clause, all corrected at nodes;
one instrument slip of mine (`$LOG` unset after `wait` — the control run's
end-state line lost, the run itself fully logged). CRITERION-INTEGRITY
INSTANCES: F61's "exactly once" leg (the watchdog's own SIGUSR1 — conflated
sources, refined to attribute or degrade LOUDLY); F65's byte-order stepper
caught by a rendered frame; F66's three-attempt tamper before the criterion
fired; F67's seven wrong checks corrected in place + the (e′) forced refusal.
BASELINES AT CLOSE: pair-check **201/176/25/90** (+5/+5/0/0 = §11.181–185;
no §5 mint); scan **119/159/97** (+1/+1/+2 vs the session-17 close, ALL from
this morning's §11.182 — F65–F67 moved the scan by zero; the two new
candidates attributed in §3(e), partition **82 + 13 named + 2 unclassed** —
the strict-credit v2 package owns the re-partition). NEXT-ROUND QUEUE, in
order: (1) scedit items **11/12** (doc DATA passes: `default_value` literals,
`completable` marker, per-name docs for flags/colours — the ghost's arming AND
the router's next lever, F64 58.5%); (2) scedit item **16** (the 1661 shipped
findings dispositioned → SS-n; the corpus gate's shipped half); (3) scedit
item **4** (stellar-system grammar, L); (4) the session-17 queue unchanged
(F52(k) git checks · strict-credit v2 + re-partition · F60 routed flips ·
§5.116/117 pricing launch · F58 classification); (5) engine, on Vixy's word
only: §5.119 mint · the generic `#!` channel · the LLM triage · the three
untriaged script requests. DECISIONS_PENDING open set at close: unchanged —
the round's Vixy items ride §3 + §11.185(d) + the scedit README's veto points.

---

**Update [Fable 2026-08-31, supervising session 19 — TravellingFoxDev, the
provenance/ASCII round]:** trigger = the §0b verbatim line PLUS four in-line
transmissions, recorded BEFORE anything else moved (→ **§11.186**, commit
`a3a03e1`, the §11.169 precedent): (a) auto lock-screen disabled —
CONFIRMED at the setting (`lock-enabled false` [measured]) with a NAMED
residual (idle-delay still 300, blank still fires, and the 1 Hz attribution
keys on the BLANK — F67's wake mitigation stays in every live-launch prompt
until a blank-only run discriminates; HOST-EVENTS.md twin entry); (b) the
engine must carry command provenance (file/tcp + line) — extends §11.184's
half exactly where it stopped; (c) feedback about TCP-origin commands sent
back on a link DEDICATED to scedit — §11.185(d)(1) answered in part,
masterput's channel FROZEN byte-identical (closed-source client, tolerance
unknowable ⇒ control-leg proof, `$LOGON` reuse excluded); (d) every source
file ASCII, accents removed — new §2.0 constraint **D14**; (e) superscript.sts
missing-doc extraction proposed ("could") — coincides with the session-18
queue's own position 1 (scedit items 11/12). Warm-up: both trees clean at
open, code `e2c8477b` / harness `8c92006`; binary current (`cmake --build -n`
empty; mtime 2026-08-31 08:20) **[CORRECTED at F68 acceptance: that check was
a FALSE GREEN — `-n` is not a `cmake --build` option; with stderr piped away
its help-text failure read as empty output. The real form is
`cmake --build <dir> -- -n`; the executor ran it (zero compile steps), so the
currency conclusion stands by ITS measurement, not mine — §11.158's
configure-false-green class, supervisor tally]**; **definition-drift assert: md5 MATCH**
(`a5a54d94`); next free §11 number **186** verified at open (live ∪ archive),
CONSUMED by the transmission entry ⇒ **187** free at first dispatch; RAM
11 GiB avail ⇒ **-j6** builds (§0.5); display per HOST-EVENTS 2026-08-31
(laptop: REAL claude logind session, F28 XAUTHORITY recipe, `:2` =
1920x1080@143.88); canary bank is the DESKTOP's — `--no-scene` fails 2 by
construction here, REPORTED (§3, session-18 line stands); this round's
launches are FUNCTIONAL (log/wire assertions, no photometry). Picks (§0b.2,
mandate order + dependency): **F68** (TCP provenance, M) → **F69** (the
dedicated feedback link, consumes F68's origin, M) → **F70** (the ASCII
conversion, AFTER both so the sweep converts the settled tree exactly once
and its census stands as the terminal gate, M); **F71** (superscript.sts doc
extraction → scedit items 11/12, M, mirror-ledger delivery) minted as the
EXTENSION member, dispatched only if health permits after the core three.
F68/F69 are the first product-code mandates in five rounds — the "engine on
Vixy's word only" line is DISCHARGED for exactly their scope (§11.186(f)).
D14 binds all four executors' own writes immediately (new code pure ASCII).
Archival pass 11 (update-s16/s17 + F56–F60/F65–F67, all delivered+verified)
is DEFERRED to the round close — recorded here so the deferral is not
silent. Instrument baselines at open, re-derived post-§11.186: pair-check
**202/177/25/90** (+1/+1/0/0 = the transmission entry, no §5 mint);
back-marker scan **119/159/97** — unchanged to the digit (§11.186's markers
use discharge vocabulary — ANSWERED — outside the scan lexicon by the F52
ruling, by design). Parent §5 register untouched at open.
**Round outcome (session 19 close, 2026-08-31):** F68 → §11.187 · F69 →
§11.188 · F70 → §11.189 · F71 → §11.190 + scedit journal `2026-08-31j` —
four for four delivered AND supervisor-verified same session (every proof
re-run by me: wire cmp ×2, scedit ctest ×3, ASCII gate + derivation verify +
27/27 ELF sections; extension 3→4 per §0b.2, F71 being the owner's own
"could"). **THE FIRST PRODUCT-CODE ROUND IN FIVE SESSIONS**: code
`e2c8477b → 96cfc352` (7 executor commits — provenance `423cbe23`, the
dedicated link `be2ddd81`+`630b06fd`, the ASCII sweep `1012c643`+`d64fd437`,
the doc data `50185663`+`94f2af65`+`96cfc352`); harness → this close.
MID-ROUND EVENTS: idle-delay 0 owner-enacted (the 1 Hz hazard's second arm;
lock-vs-blank closed as moot); a scheduled shutdown NOBODY DISARMED rebooted
the host mid-F70 — the discontinuity cost measured ZERO (CP1 discipline +
the rebuilt PRE binary reproducing its pre-reboot md5), and the display
moved `:2 → :0` (HOST-EVENTS). HEADLINE FINDINGS: the engine now tells a
TCP client what it refused, on an opt-in wire proven byte-identical for
everyone else (71/5/0/85 B across THREE tasks and four runs); the HTTP door
shares the input queue and would have carried FALSE provenance (the
near-miss that shaped F68); `App::masterput()` is a file-drop poller, NOT
masterput's TCP channel; the canary's start-epoch probe reads /proc dir
MTIME — its `xserver.restarted` was a ghost and its GATING twin can
false-red the desktop; the `app_command_interface` pair were NEVER ISO-8859
(UTF-8 + one stray 0xA7 each); nested refusals reach nobody (§11.184's
rule, routed); the tree is 100% ASCII with rendered bytes untouched (the
escape fork, §3); §5.119 minted (a ONE-LINE script reaches a past-the-end
deref) · §5.120 minted · §5.92's discharge RETRACTED · SS-26…31 (the
`on`-means-no split; seconds-vs-milliseconds; 18/46 colour names never
driven). SUPERVISOR-ERROR TALLY, all corrected at their nodes: **five
glosses/defects** (the session-18 lock HEADLINE that narrowed the owner's
enactment [owner-caught] · the `cmake --build -n` false green · the hpp/cpp
line gloss · the decode-as-Latin-1 method clause · the "+267 unexamined"
stale premise [all executor-caught via §0.7 report-not-absorb]) + one
propagation (the HOST-EVENTS ghost echo, corrected with the instrument's
refutation). CRITERION-INTEGRITY INSTANCES this round: **5** (F69's vi-post
prediction refuted by its own committed check · F70's 4-red first re-record
attempt + its partition self-audit · F71's `camera value no` rejection +
its colour-state-label rewrite) — prior cumulative 23 (s17) + 4 (s18) ⇒
**32 across sessions**. Round-close events: **archival pass 11** (deferred
at open, honored here: update-s16/s17 + F56–F60 + F65–F67, 10 units
byte-exact, manifest `2026-08-31-pass11`, reconstruction md5 proven);
derived index regenerated. BASELINES AT CLOSE: pair-check **206/181/25/92**
(+4/+4/0/+2 over the round: §11.187–190 entries+stubs, §5.119/§5.120 inline
mints) · scan **124/165/101** (deltas attributed per task; the three
"unmatched" additions are all the documented at-the-corrected-node
inversion; NOTE for strict-credit v2: the scan reads only `INTENT/<id>.md`
as event sources — a §5-sourced back-marker is invisible to it, the
package's SIXTH member). DECISIONS_PENDING open set: unchanged (D37 rides
the final pass). Remotes: local contains origin on both repos, push = ff
(session-18 note stands). **Next-round queue, in order:** (1) the canary
start-epoch fix + two-value re-bank ON THE DESKTOP (gating false-red
hazard; one line + one VALUES edit); (2) scedit item **16** (the 1661
shipped findings dispositioned → SS-n); (3) the grammar anchor repair
(§11.190(e): re-anchor the 324, or the one-`_meta`-field alternative);
(4) scedit item **4** (stellar-system grammar, L); (5) the session-17/18
carried queue (F52(k) git-only checks · strict-credit v2 — SIX members now
· F60 routed flips · §5.116/§5.117 pricing launch · F58 adjacent-loader
classification); (6) engine, on Vixy's word only: §5.119's fix · the
nested-origin rule · the file-origin log tag · `$NOTICE` advertisement ·
the lifecycle event · the three untriaged script requests; (7) §5.100's
fix IF the §3 authorization lands.
**Post-close addendum (2026-08-31/09-01):** three owner rulings → §11.191
(the escape rule RATIFIED [D14 reaches source bytes, never rendered ones];
the file-origin log tag word-given; the canary's objective stated as an
asymmetry — fail-closed stands, false positives removed at the root, no
band-widening licensed); **F72 → §11.192** dispatched, delivered and
accepted same night (one condition at `originTag()`, 16-for-16 lines so no
citation staled; f68 47/47, f69 52/52, f63 34/34; wire and subscriber
recordings byte-identical to F68's and F70's committed baselines; both
provenance instruments gain a DECLARED pre-era, retiring F70's three
known-false reds per §11.191(c)). Session 19 final tally: **FIVE for
five** (F68–F72), six supervisor defects all caught-and-corrected,
criterion-integrity 5, baselines at true close pair-check **208/183/25/92**
· scan **125/166/101**.

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
   - Loaded data `~/.spacecrafter/ssystem.ini` is ISO-8859 and untracked — the Bash
     `grep` wrapper silently excludes it; use `/usr/bin/grep` or Read.
     **[CORRECTED 2026-08-31, F67 executor finding + supervisor measurement: the
     wrapper is ugrep with `-I` — ANY file holding non-UTF-8 bytes is classed binary
     and skipped silently, tracked or not; `src/interfaceModule/app_command_interface.cpp`
     is ~~the one~~ **[CORRECTED 2026-08-31, F69 §11.188(k): ONE OF TWO — the `.hpp`
     of the same name is ISO-8859 as well (18 non-ASCII bytes, already so at
     `423cbe23~1`); census over all 505 tracked regular files under `src/` finds
     exactly those two]** such file among 500 tracked `src/` files,
     `doc/superscript.sts` another
     (11 hits without `-I`, none with). `/usr/bin/grep` or Read on every ISO-8859 file.
     CLAUDE.md (one file: the code-tree path is a link to `claude/CLAUDE.md`) carries
     the corrected rule.]**
     **[SUPERSEDED IN PART 2026-08-31, F70 §11.189(a) — post-D14 state: every
     tracked CONVERT-set file is pure ASCII (code `1012c643`+`d64fd437`); the
     wrapper hazard survives ONLY in `~/.spacecrafter/ssystem.ini`,
     `doc/superscript.sts` and the EXCLUDE rows of `harness/f70_partition.tsv`.
     The "ISO-8859 file(s) in src/" DIAGNOSIS was wrong both times: the
     `app_command_interface` pair were UTF-8 with one stray 0xA7 each —
     whole-file Latin-1 decoding mojibakes such files. SECOND hazard, same
     class: `grep -P '[\x80-\xff]'` under a UTF-8 locale matches code points,
     not bytes — `LC_ALL=C` for byte classes. D14 standing gate:
     `python3 harness/f70_ascii.py gate`. CLAUDE.md carries the clean rule.]**
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
     executor prompt binds this; ~~the §11.156(g) five are the recovery backlog,
     queued~~ **[PAID 2026-08-29, F49 §11.165]**. **Row-flip extension
     [2026-08-30, F60 acceptance, from §11.180's finding that every stale
     reference sits on the NON-working side]:** when a §13 row's state flips
     (delivered/closed/delegated), grep the §5 register for references to that
     row IN THE SAME COMMIT and annotate any the flip stales — the write is the
     one moment the flip's author holds both ends (I3: the state's owner
     informs its dependents).
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
archived pass 10) · F56 §11.176 · F57 §11.177 · F58 §11.178 · F59 §11.179 ·
F60 §11.180 (session-17 round — five for five DELIVERED and accepted;
archived pass 11) · F65 → scedit journal 2026-08-31f · F66 → 2026-08-31g ·
F67 → 2026-08-31h + parent §11.185 (session-18 scedit round — three for
three; archived pass 11). Live below: the session-19 round **F68–F71**.
Remaining candidates next-round: see the session-19 outcome's queue. Still
blocked: §5.100's fix (authorization unanswered).*

---

### F68 — Command provenance: the engine knows a TCP line IS a TCP line (§11.186(b); FEATURE_REQUESTS 2026-08-31 provenance entry) [M]

**Why now / mandate:** Vixy, verbatim (§11.186): *"spacecrafter script engine
must carry the provenance (file/tcp + line)"*. §11.184 built the file half —
`ScriptOrigin` (file, 1-based line, raw text) rides `Script::load` → token
queue → loop replay → `executeCommand`'s origin overload — and stopped, by
recorded decision, exactly at *"the two-argument overloads — TCP/HTTP/UI,
`clear`/`media`/`lift_off` nesting — pass none: log-only"*. The mandate names
TCP as an origin a command must CARRY, not merely lack.

**Sources (re-read, never recall):** `INTENT/11.186.md` (b)(c) (the mandate +
the F69 boundary this task must not pre-empt); `INTENT/11.184.md` IN FULL
(the origin architecture: RAII restore of `currentOrigin`, the annotator's
contract, the `#!` tail rules); `INTENT/11.185.md` (a)(e) (what reaches a TCP
client today; the latch); `src/scriptModule/script_origin.hpp` (the struct +
its header contract); `src/tools/io.hpp` (ClientMessage, `servingClient`
latch, `clientIdTab` never-reused ids, the getInput/setOutput same-thread
contract); `src/appModule/app.cpp` `updateFromSharedData` (the TCP drain
loop); `src/interfaceModule/app_command_interface.{hpp,cpp}` (`currentOrigin`
:220, the overloads :66-74, `executeCommandStatus` :1236). WARNING:
`app_command_interface.cpp` is ISO-8859 until F70 lands — the Bash `grep`
wrapper SKIPS it silently; use `/usr/bin/grep` or Read (§0.5).

**Preconditions (checkable, §0.7 gate):** code HEAD `e2c8477b`, tree clean;
harness HEAD as the dispatch prompt states, tree clean; binary
`build-claude/src/spacecrafter` current at code HEAD (`cmake --build -n`
empty at dispatch); next free §11 number **187**; live `### F` count **12**;
`ScriptOrigin` at `script_origin.hpp` with `valid()` = file+line (as §11.184
left it); the origin overload + `currentOrigin` member present as cited;
display stack per HOST-EVENTS 2026-08-31 laptop entry (F28 XAUTHORITY recipe,
`DISPLAY=:2`, real logind session) — verify `xdpyinfo` answers BEFORE any
launch; RAM < 16 GiB ⇒ `-j6` builds. ANY broken ⇒ abort per §0.7.

**Scope:**
1. **`ScriptOrigin` gains a channel kind** (design yours, contract-first in
   the header): a command's origin distinguishes at minimum FILE (path+line,
   exactly today's semantics) and TCP (with the connection identity — the
   never-reused `clientIdTab` id, which is what makes a later reply follow
   the connection, I5/§5.47's own argument). `valid()`'s meaning (file+line,
   the `#!` writer's gate) MUST NOT widen — the annotator writes files only;
   re-read `script_annotator.hpp`'s contract before touching the struct and
   state in the header what each consumer may assume. UI/nested/synthesised
   lines stay origin-less (log-only) — the mandate names file/tcp; map and
   REPORT the other producers (HTTP `?command=`, mkfifo, joypad) with their
   observed entry points, do not wire them without a word.
2. **The TCP drain threads the origin**: `App::updateFromSharedData`'s loop
   passes an origin naming TCP + the serving connection's id. The socket
   layer is the only place that knows it (io.hpp's own doctrine) — expose it
   from `ServerSocket` alongside `getInput()` without changing any wire
   behaviour (D8: a client must see byte-identical traffic; this task sends
   NOTHING new on any socket — that is F69's, and only behind its opt-in).
3. **Diagnostics carry it**: at the funnel (`executeCommandStatus`, and the
   sibling refusal sites that bypass it — map them: :306/:385/:400 class,
   `"Unable to execute script"` :2889), a TCP-origin command's log line names
   its origin (e.g. `tcp#<id>: <what>`) exactly as a file-origin line names
   `<file>:<line>` (§11.184's shape). §2(f)/§11.169 content untouched or
   improved, never regressed. No new bytes on any socket (F69's boundary).
4. **Gate** (new `harness/f68_provenance.py`, temp-HOME farm, fresh launch,
   md5 in==out, comm-probe 0, plain `timeout`, §0.5 discipline + the wake
   mitigation recorded per run): legs proving (i) a file-origin fault logs
   `file:line` (control, §11.184's behaviour unchanged); (ii) the SAME fault
   sent as a TCP line logs the TCP origin with the id; (iii) a nested call
   (`clear`/`media` class) still restores the outer origin (the RAII leg);
   (iv) a `$LOGON` subscriber + a plain client observe ZERO new bytes across
   the whole battery vs pre-change (record the wire both sides — this is
   F69's baseline control, committed as an artifact it will cite); (v) the
   `#!` writer still writes ONLY for file-origin faults (a TCP fault must
   not touch any file). Both-ways discrimination on (ii): the pre-change
   binary shows the origin-less line.
5. **Record:** §11 entry at **187** + stub (entry-first, §11.156(f));
   code committed FIRST (engine), harness commits carry `Code: master-beta @
   <sha>`; README section; WIP line per §0.6; new code PURE ASCII (D14).

**Boundaries:** NO wire-visible change on any socket (masterput frozen —
that whole surface is F69's, behind opt-in); NO annotator file-writes for
non-file origins; NO data writes; old render path untouched; UI/HTTP/mkfifo
origins mapped-not-wired; mint license per §5.79 for distinct-mechanism
defects found en route, else list on the entry.

**DoD:** kind carried + threaded + logged; gate green with both-ways records;
wire-silence leg (iv) committed as F69's citable baseline; build green at
`-j6`; §11.187 + stub + README; both instrument baselines re-derived as the
LAST act with deltas attributed; trees clean; WIP cleared.
**WIP:** DELIVERED 2026-08-31 — §11.187 (entry file + stub); engine
`423cbe23`; gate `harness/f68_provenance.py` **43/43** over three launches
(pre `fc651978` twice as A/A + both-ways control, post `444db012`) and
`f63_annotations.py` **34/34** on the delivered binary as the file-half
control; wires committed as F69's frozen baseline (S 71 B / P 5 B / HTTP
85 B, byte-identical pre == pre2 == post). Back-markers at §11.184 and
§11.186(b), annotations at §5.117 and §11.185(a), FEATURE_REQUESTS half (1)
flipped; harness README F68. Canary exit 2 as stated (+1 new NOTE:
`xserver.restarted`). Baselines re-derived LAST: pair-check
**203/178/25/90** (round-open tree `00829ce` measured at **202/177/25/90**:
+1/+1/0/0 = this entry and its stub); scan **120/161/98** (round-open
**119/159/97**: +1 raw = the §11.184 back-marker line, +2 pairs = its two
citations, +1 unmatched = §11.184→§11.186, the detector's documented
inversion — the line IS the back-marker and §11.186(b) is the mandate it
cites).
**ACCEPTED 2026-08-31 (Fable, session 19):** entry read in full; commit
chain/authors verified (code one commit, exactly the named surface, added
lines 0 non-ASCII — D14 holds); wire artifacts re-compared by me (S/P/Q/HTTP
identical ×3); both back-marker homes verified; §5.117 annotation +
FEATURE_REQUESTS half-flip + stub placement verified; baselines reproduced
to the digit by my own runs. All five judgment calls ENDORSED with their
arguments: (1) HTTP `bool http` → no-origin (a false `tcp#<id>` would have
been worse than none — the mandate's own point); (2) the file-half
asymmetry of `originTag()` — conservative, reversible, both readings at the
definition; the sharp question ROUTED to §3; (3) flag-name emitter left to
§11.178's work order; (4) byte-level ISO-8859 patches with count asserts;
(5) the comm-truncation rename (instrument integrity). TWO dispatcher
defects it reported, both mine, tallied: the hpp/cpp line-attribution gloss
in the F68 section; the warm-up `cmake --build -n` false green (my §0b.1
check piped stderr away and read silence as currency — the executor's
`-- -n` form is the real instrument; conclusion survived by its
measurement, not mine). Canary NOTE (`xserver.restarted`) echoed to
HOST-EVENTS at acceptance.

### F69 — The dedicated feedback link: TCP-origin feedback comes back on an opt-in channel; masterput's wire is frozen (§11.186(c); §11.185(d)(1) ruling; FEATURE_REQUESTS 2026-08-31) [M]

**Why now / mandate:** Vixy, verbatim (§11.186): *"feedback about tcp sent
back (note: an existing tcp path exists, used by masterput (which is
closed-source), do not modify this channel) - and sent it back through the
tcp link dedicated for scedit."* This is §11.185(d)(1) answered in specific
form. Consumes F68's origin — dispatched only after F68's acceptance.

**Sources (re-read, never recall):** `INTENT/11.186.md` (c) (the two
structural consequences: FROZEN wire for the unsubscribed, opt-in dedicated
link, `$LOGON` reuse EXCLUDED); `INTENT/11.185.md` IN FULL ((a) the two
existing reply classes; (e) the addressed-reply/broadcast semantics F27
left); `INTENT.md` §5.47/§5.72/§5.117 stubs (the channel's three open rows —
this task composes with, never silently resolves, any of them);
`src/tools/io.{hpp,cpp}` (verb dispatch `computeNormalString` :640-,
`clientBroadcastTab`, `deliver`, the same-thread contract);
F68's delivery entry + its leg-(iv) wire baseline artifact;
`util/scedit/src/sc_tcpclient.*` + the feed pane (scedit's consumer half);
scedit mirror ledger §2 constraints + §4 verification bar. ISO-8859 grep
warning as in F68 until F70 lands.

**Preconditions (checkable, §0.7 gate):** F68 DELIVERED and accepted (its
§11.187 entry exists; its WIP line reads DELIVERED); code HEAD = F68's
delivery sha, tree clean; harness HEAD as the prompt states; next free §11
number **188**; live `### F` count **12**; `computeNormalString` still
dispatches exactly `$NOTICE`/`$LOGON`/`$LOGOFF` (no other verb landed);
display + RAM + canary stance as F68's. ANY broken ⇒ abort.

**Scope:**
1. **The opt-in verb** (engine, io.cpp): a connection declares itself a
   feedback subscriber by an explicit new `$`-verb (name yours — state the
   candidates and the choice's reason; advertise it in the `$NOTICE` reply,
   which is itself a wire change VISIBLE to any client that asks — argue its
   safety or withhold it, your call, stated either way); a per-slot table in
   the ServerSocket idiom (`clientBroadcastTab`'s shape), cleared on
   disconnect/slot-reuse; a send path addressed to feedback subscribers
   ONLY. The same-thread contract holds (application thread sends; the
   server thread never composes).
2. **The routing** (engine, command interface): at the diagnostic funnel
   F68 mapped, a refusal/diagnostic for a **TCP-origin** command is ALSO
   delivered to feedback subscribers, carrying its provenance (`tcp#<id>`)
   and the §11.169 three-part content. Scope = feedback about TCP-origin
   commands, exactly the mandate; script lifecycle events and file-origin
   `#!` mirroring are NOT ruled — leave unwired, state the boundary in the
   entry (the (d)(1) poll-replacement line stays a routed question).
   Success-acks: not feedback in the ruled sense — unwired unless you find
   a §2(f) argument, stated either way. The log keeps everything it has
   today (the wire ADDS a copy for subscribers; it never diverts).
3. **The frozen-wire proof** (the mandate's hard boundary): extend F68's
   leg-(iv) battery — a plain client and a `$LOGON`-only subscriber each
   record their full wire across a scenario set (refused command, played
   script with faults, `get status`, second client's answer) on the PRE-F69
   binary and on the delivered one: **byte-identical for both**, diffed and
   committed. The feedback subscriber's OWN wire is the positive
   discrimination (receives every routed diagnostic). Both directions
   demonstrated, artifacts committed.
4. **scedit consumes it** (the "dedicated for scedit" half):
   `sc_tcpclient` gains the subscription; the feed pane shows feedback
   lines (existing 500-line bounded feed; visual distinction yours,
   README-stated); the `--tcp` path documents it. scedit's tcp gate
   extends to assert the subscription + at least one received feedback
   line against a stand-in server; the LIVE leg (real engine, one launch,
   a refused TCP command seen coming back) rides the same launch as leg 3
   where possible. Mirror-ledger journal entry for the scedit half.
5. **Record:** parent §11 entry at **188** + stub (the engine half is
   parent territory; the scedit half journals in the mirror, cross-cited);
   code-first commit discipline; README (harness + scedit) sections; WIP
   per §0.6; new code PURE ASCII (D14). Annotate §5.72/§5.117 at their
   nodes (the wire now carries a copy for subscribers — their severity/
   place claims gain a conditioned clause), back-markers same commit;
   §11.185(d)(1) flipped to ANSWERED+IMPLEMENTED at its node.

**Boundaries:** the unsubscribed wire is BYTE-IDENTICAL — any observed
delta is a delivery-blocking defect, never a judgment call; `$LOGON`
semantics untouched (§5.72's conflation stays as recorded); no new port
unless the verb route fails a stated constraint (then STOP and report —
config surface is B28-protocol territory); no auth work (the (d)(3) dome
question stays Vixy's); no lifecycle events; mint license per §5.79.

**DoD:** verb + routing + frozen-wire proof (both directions, committed
diffs) + scedit consumption with gates; parent §11.188 + mirror journal +
row annotations; builds green both trees (`-j6`); baselines re-derived as
last act, deltas attributed; trees clean; WIP cleared.
**WIP:** DELIVERED 2026-08-31 — §11.188 (entry file + stub); engine `be2ddd81`,
scedit `630b06fd`. Gate `harness/f69_feedback.py` **49/49** over three fresh
launches (pre `444db012` twice as A/A + both-ways control, post `2ea5d54b`);
scedit `ctest` **14/14** in `build-lovely` and a fresh `build-f69`, 0 warnings,
three consecutive green suites. FROZEN WIRE REPRODUCED: S 71 B / P 5 B / Q 0 B /
HTTP 85 B, pre == pre2 == post AND byte-identical to F68's committed
`artifacts/f68/wire.pre.*`; subscriber D's own wire 939 B (post) vs 0 B (pre) is
the positive discrimination, both directions committed. Verb `$DIAGON`/`$DIAGOFF`
(candidates + rejections argued at (b)); `$NOTICE` NOT extended — veto point
stated at (c). Gates widened: tcp_client 23/65→30/112, mcp_protocol 77→85,
ui_selftest 20→21 frames (feed mask now E/D/L off the pixels). Records:
§11.185(d)(1) flipped ANSWERED+IMPLEMENTED with the lifecycle half still routed,
§11.186(c) DELIVERED, §11.187(i) back-marked (the canary NOTE is the instrument),
§11.187(j) closed; §5.72 + §5.117 annotated at their nodes; scedit mirror journal
`2026-08-31i` + its §4 gate inventory. Canary exit 2 as stated; GetActive false
at all six reads. TWO findings outside the mandate, both reported not fixed: the
canary's start-epoch proxy is `/proc/<pid>` mtime (probe committed; the same
proxy backs a GATING check) and `app_command_interface.hpp` is ISO-8859 too, so
tracked `src/` holds TWO such files — rule corrected at CLAUDE.md and §0.5.
Baselines re-derived LAST: pair-check **204/179/25/90** (F68's post-state
203/178/25/90: +1/+1/0/0 = this entry and its stub); scan **121/162/99** (F68 post-state **120/161/98**: +1 raw = the §11.187(i) CORRECTED line, +1 pair = its citation, +1 unmatched = §11.187→§11.188 — the SAME documented inversion F68 recorded for §11.184→§11.186, the line at the corrected node IS the back-marker; the §11.185/§11.186 flips used discharge vocabulary and moved the scan by zero, by the F52 ruling).
**ACCEPTED 2026-08-31 (Fable, session 19):** entry read in full; commit
chain/authors/trees verified; all four unsubscribed wires re-compared BY ME
against F68's committed baseline (S/P/Q/HTTP equal; D 939 B); scedit ctest
**14/14 re-run by me** in the executor's build dir. Judgment calls ENDORSED
with arguments: `$DIAG` naming (I1/I2 — carries what it is, avoids the
"feedback" word collision); `$NOTICE` NOT advertised (the reply IS bytes on
masterput's wire — the conservative side of the same residual-information
argument; veto-open, §3); idempotent `$DIAGON` (§2(f), `$LOG` untouched);
`sc_mcp` visited beyond named scope (I3 — the alternative was a silently
false `note` contract; reported not absorbed); the mcp_gate wait-not-sample
fix (a real race made visible, argued); the `§`-in-ledger encoding line
(D14's own boundary clause). The ONE wire delta (the `$DIAGON` string itself
now answered at the socket layer) accepted as the structural minimum of the
opt-in the ruling mandated — bounded to the exact novel string, disclosed in
§3. Criterion-integrity instance on record: the vi-post prediction refuted
by its own committed check, kept untidied. THE CANARY FINDING accepted as
delivered evidence: §11.187(i)'s NOTE and MY HOST-EVENTS echo described a
NON-EVENT (the proxy is `/proc/<pid>` dir mtime) — my echo corrected at
HOST-EVENTS this commit, supervisor tally (echoed an instrument's claim
before its probe was verified); canary fix + re-bank queued as a DESKTOP
act (§3 — the same proxy backs a GATING member: false-red abort hazard for
any desktop photometric task until fixed). Nested-refusal gap routed to §3
as a §11.184-rule decision.

### F70 — Every source file pure ASCII: census, conversion, exclusion table (D14, §11.186(d)) [M]

**Why now / mandate:** D14, verbatim: *"Every source file must be in ASCII,
accents are to be removed for this purpose."* One-shot conversion of the
existing tree + the standing census instrument that keeps it true. Runs
AFTER F68/F69 so the sweep converts the settled tree exactly once and the
terminal census gates the round's own new code too.

**Sources (re-read, never recall):** `INTENT.md` §2.0 **D14** (the boundary
clause is veto-open — your exclusion table is its challengeable form);
`INTENT/11.186.md` (d) (the 199-file open census, the named exclusion
candidate); CLAUDE.md's ISO-8859 rule (what narrows after this lands);
scedit `grammar/witness/` READMEs/notes (what the witness files PIN);
`po/`-or-equivalent translation catalogs IF any tracked (find them first).

**Preconditions (checkable, §0.7 gate):** F69 delivered and accepted; code
HEAD = F69's delivery sha, tree clean; harness HEAD as the prompt states;
next free §11 number **189**; live `### F` count **12**; the round-open
census reproduces to the file (199 tracked files non-ASCII at `e2c8477b`;
re-derive at YOUR HEAD and attribute any delta to F68/F69's diffs — their
prompts bind ASCII-only new code, so the expected delta is 0). ANY broken ⇒
abort.

**Scope:**
1. **Census instrument** (`harness/f70_ascii.py`, committed BEFORE any
   conversion): enumerate ALL tracked files in the code repo; per file:
   encoding class (ASCII / UTF-8 / ISO-8859 / binary), non-ASCII line count,
   and context classification (comment / string literal / other) for source
   files. Output committed as the pre-state artifact.
2. **The boundary, stated then applied**: partition every non-ASCII file
   into CONVERT (source: C/C++/headers/CMake/GLSL/scripts/scedit sources +
   tests + grammar DATA whose prose scedit displays) vs EXCLUDE (files
   whose non-ASCII bytes are their FUNCTION: `grammar/witness/*` pinning
   `doc/superscript.sts` bytes; translation payloads; `doc/`; `data/`;
   binary assets). The table is the deliverable Vixy vetoes against — every
   EXCLUDE row carries its one-line reason. In doubt ⇒ EXCLUDE + flag,
   never silent conversion.
3. **String-literal hazard pass, BEFORE converting them**: for every
   non-ASCII **string literal** in CONVERT files, trace the consumer:
   gettext key (does a catalog msgid match byte-for-byte? then conversion
   BREAKS the lookup — convert both or exclude+flag), protocol/data-matching
   string (compare target), user-visible text (conversion intended by the
   mandate). Committed as a per-literal disposition list — this is the step
   that keeps the sweep from being a semantic change in disguise. The known
   wire string (*"Vous receverez maintenant les logs"*, io.cpp) is ASCII
   already — masterput's frozen wire is NOT at risk from accent removal,
   verify rather than assume (any literal that REACHES a socket is in the
   trace set).
4. **Conversion**: accents transliterated to ASCII (é→e class, both UTF-8
   and ISO-8859 sources; `app_command_interface.cpp` decoded from ISO-8859
   first — verify with `iconv`, never byte-strip); non-accent non-ASCII
   (arrows, °, œ→oe, typographic quotes) transliterated to nearest-ASCII
   with the per-character map committed; NO other byte changes (whitespace,
   line endings, content untouched — the diff must be reviewable as pure
   transliteration). Mechanical verification: a script re-derives the diff
   from the map and asserts equality, committed with the sweep.
5. **Terminal gates**: census reruns → ZERO non-ASCII in the CONVERT set;
   engine full build green (`-j6`); scedit build + its OWN full gate suite
   green (its tests may pin accented bytes in expected outputs — reconcile
   at the root, report any witness-class collision instead of editing a
   witness); one engine smoke launch (functional, §0.5 + wake mitigation)
   proving boot + one script play unaffected.
6. **Record:** §11 entry at **189** + stub; the CLAUDE.md/§0.5 ISO-8859
   rule narrowing is the SUPERVISOR's at acceptance (flag it, don't edit
   CLAUDE.md yourself); README; WIP per §0.6; commits sized per checkpoint
   discipline (census / hazard pass / conversion / gates — an abort must
   lose at most one).

**Boundaries:** NO semantic change — a literal whose conversion could
change behaviour lands in the disposition list, not in the diff, unless
traced safe; witness files NEVER edited; `doc/superscript.sts` NEVER
edited; translation catalogs edited ONLY as the traced pair of a converted
key, listed; the code repo only (harness/ledger files are exempt from D14's
letter — "source file" — flag any you think should follow); mint license
per §5.79.

**DoD:** pre-census + exclusion table + literal dispositions + pure-
transliteration proof + zero-count terminal census + both builds green +
scedit gates green + smoke launch; §11.189 + stub + README; baselines
re-derived as last act; trees clean; WIP cleared.
**WIP:** DELIVERED 2026-08-31 — §11.189 (entry file + stub); code
`1012c643`. **282 files converted, 8269 non-ASCII bytes → 0**; census 561
non-ASCII files → 278, every survivor an EXCLUDE member; `mixed` class 2→0;
CONVERT-set characters **0**. Diff 2731/2731 with added==removed in EVERY
file. Gates: `f70_ascii.py gate` PASS · `f70_translit.py verify --from
630b06fd` PASS · **`f70_binary_equal.py`: all 27 allocated ELF sections
identical, 7 938 005 B** (decoy 1 bit ⇒ exit 1) · engine build `-j6` green,
0 new warnings · scedit `ctest` **14/14**, 0 warnings · **frozen wire
71/5/0/85 B**, A/A == post == F68's committed wires, and the 939 B SUBSCRIBED
wire byte-identical too · engine stdout pre/post identical across 2359 lines
but an ASLR handle and 1 MiB of GPU free memory · smoke launch = the
battery's post phase (boot, Vulkan, script play, `#!` on line 3, md5 in==out,
comm probe). Two rules: prose transliterated (`f70_map.tsv`, 82 noted rows),
C-family string literals re-spelled as `\xNN` escapes of the SAME bytes —
which is why the dome's 22 degree-sign sites, the $DIAG wire and every pinned
fixture are untouched. 619 literals dispositioned with a traced consumer each;
`_()` proven NOT gettext and all 71 catalogues proven to carry ZERO non-ASCII
keys, so no catalogue was edited. 5 records re-recorded deliberately (9 lines,
all grammar prose, diff committed); witness/, doc/, data/, third_party/
byte-untouched. Self-audit of the delivered table found two GLSL sources under a `data/` row
that called them binary assets — fixed at the root, code `d64fd437` (+2 files,
+1 map row: U+200B, on the converter's first REFUSAL).
Back-markers at §11.186(d), §11.188(k), §11.188's
ui-selftest note and §2.0 D14's boundary clause, all this commit.
**OWED TO THE SUPERVISOR:** the CLAUDE.md/§0.5 ISO-8859 rule NARROWS (src/ now
holds no non-UTF-8-decodable file) and must also be CORRECTED — the two
`app_command_interface.*` files were never ISO-8859 files (UTF-8 + one stray
0xA7 each, code `2b8ec034`) — plus a second grep hazard belongs beside it
(`-P '[\x80-\xff]'` under a UTF-8 locale matches code points, not bytes).
**ENVIRONMENT, reported not mitigated:** `:2` is gone after the 20:57 reboot;
ran on `:0` 1920x1080 (own `xdpyinfo`); canary exit **3** not 2, both failures
from the hard-coded `BANK_DISPLAY=":2"`; nothing re-banked (§11.174(f) is the
owner's). GetActive false at all 6 reads. 3 of 49 wire checks fail BY
CONSTRUCTION (they assert the PRE binary lacks `$DIAGON`; F70's PRE is F69's).
Baselines LAST: pair-check **205/180/25/90** (F69 post-state 204/179/25/90:
+1/+1/0/0 = this entry and its stub); scan **123/164/101** (F69 post-state
121/162/99: +2/+2/+2 = the two back-markers placed at §11.186(d) and
§11.188(k), each read source-and-target-backwards by the detector — the SAME
documented inversion F68 recorded for §11.184→§11.186 and F69 for
§11.187→§11.188: the line AT the corrected node IS the back-marker, and
§11.189 is the entry it cites).
**ACCEPTED 2026-08-31 (Fable, session 19):** entry read in full; trees/
commits verified; ALL THREE proofs re-run BY ME on the delivered tree —
census gate PASS (0 non-ASCII over 957 CONVERT-matched files), `verify
--from 630b06fd` PASS (the tree IS the derivation), `f70_binary_equal.py`
PASS (27/27 allocated sections, 7 938 005 B) — plus baselines reproduced to
the digit. Judgment calls ENDORSED: the ESCAPE RULE as the D8/D9-conservative
reading of D14 (the mandate's stated purpose — ASCII files — fully met;
rendered output, wire bytes and pinned fixtures untouched; the alternative's
cost named and routed to §3 as the round's sharpest veto point); the map's
three deliberate deviations with their arguments (`·`→`*` reads as member
access otherwise); the five record re-recordings (diff read, committed,
grammar-prose-only — the 4-red first attempt is the criterion that forced
the right mechanism); the in-tree PRE rebuild (Release/NDEBUG + 0 source
paths in .rodata measured, md5 reproduced across the reboot); the
partition self-audit that reclassified two GLSL files out of its own
EXCLUDE row (fixed at the root, refusal mechanism exercised on U+200B).
THE ENCODING-DIAGNOSIS CORRECTION ACCEPTED: the app_command_interface pair
were never ISO-8859 files (UTF-8 + one stray 0xA7 each, both from
`2b8ec034`) — my F70 section's "decode as ISO-8859 first" method clause
would have mojibaked them; executor-caught, dispatcher tally (with F69's
"non-UTF-8-decodable" measurement standing — it was the CLASSIFICATION
that was wrong). The "199" gloss handled correctly under §11.179(a) with
the counterfactual stated — third dispatcher gloss this round, tallied.
CLAUDE.md/§0.5 rule corrected AND narrowed at acceptance (this commit);
reboot audit accepted (nothing resumed on trust; the discontinuity
measurably cost zero — the rebuilt PRE reproduced its pre-reboot md5).
Canary exit 3 folded into the §3 canary item (BANK_DISPLAY=":2" now names
a display that does not exist on this host).

### F71 — EXTENSION: the missing documentation, extracted — scedit items 11/12 from the current superscript.sts, code-dug where silent (§11.186(e); session-18 queue position 1) [M]

**Why now / mandate:** Vixy (§11.186(e)): *"One of the task could be to
extract the missing documentation from the current version of
superscript.sts (and dig the code where needed)"* — read as the proposal it
is; it coincides with the session-18 queue's position 1: scedit items
**11/12** (doc DATA passes: `default_value` literals, `completable` marker,
per-name docs for flags/colours — the default-greyed ghost's arming AND the
doc router's next lever, F64 58.5%). Dispatched ONLY after the core three
deliver and health permits (§0b.2).

**Sources (re-read, never recall):** scedit mirror ledger
(`claude/util/scedit/INTENT.md`) items 11, 12, and 9's residue (the 2026-08-04
doc-mining journal + its 6 queued code-consistent answers + S-NP-1's
suspension — do not re-answer what it already banked); `doc/superscript.sts`
CURRENT bytes (ISO-8859 — Read or `/usr/bin/grep`; the tester's rewrite,
+267 lines unexamined since §11.149(e)); `grammar/sc-grammar.json` schema +
the 324 arg specs; `app_command_interface.cpp` per-handler truth (post-F70:
plain ASCII by then); `SCRIPT_SURFACE.md` (SS-n grammar, C2 ownership
split); F64/F66's doc-surface shape (what `--doc` serves — the data this
task feeds).

**Preconditions (checkable, §0.7 gate):** F68–F70 delivered and accepted;
code HEAD = F70's delivery sha, tree clean; harness HEAD as the prompt
states; next free §11 number as the prompt states; live `### F` count
**12**; `doc/superscript.sts` md5 as the prompt states (the extraction's
input pinned); items 11/12 still OPEN at the mirror ledger. ANY broken ⇒
abort.

**Scope:**
1. **`default_value` backfill (item 11)**: the 35 bare-token candidates
   from the shell-slice report first, then the rest of the 324 — each
   default as DATA, each source-anchored (superscript.sts line, or code
   file:line when the doc is silent — the `[superscript-attested]` /
   code-cited tag discipline of item 9), NEVER regexed from prose (C2).
   A doc-vs-code disagreement is a FINDING (SS-n row), not a silent pick.
2. **`completable` marker (item 12)**: schema marker on values[], the
   known false positive (`xRRGGBB`) resolved, validator check added at
   the grammar gate.
3. **Per-name docs for flags/colours**: the family F64 measured as the
   router's lever — per-name entries (incl. the `font` family F64's map
   missed), source-anchored as above.
4. **The missing-doc report**: commands/args/flags reachable in code but
   absent or under-documented in the CURRENT superscript.sts — extracted
   as a structured PROPOSAL artifact for the tester (his file, §11.149(e);
   NEVER edit `doc/superscript.sts`), plus SS-n rows where a doc claim and
   code diverge. The +267 unexamined lines get their first systematic
   read; bank what they answer.
5. **Gates + record**: grammar validator + scedit's doc gates green;
   `--doc`/`--search` still serve (F66's gates re-run); one measured
   router datum IF cheap (the f64 harness re-run on the enriched data —
   record as it comes, no target); mirror-ledger journal entry + items
   11/12 state flips; parent stub only if a parent-ledger fact surfaces
   (else the mirror entry suffices, cross-cited from the WIP line); WIP
   per §0.6; all new data PURE ASCII (D14 — transliterate sourced prose
   at extraction time, noting each).

**Boundaries:** `doc/superscript.sts` read-only; engine code read-only;
findings route SS-n per C2; S-NP-1 stays suspended; no router/LLM work
(the triage is still Vixy's, §3); mint license per §5.79.

**DoD:** items 11/12 flipped with data landed + validator; per-name pass
incl. `font`; missing-doc proposal artifact + SS-n rows; gates green;
mirror journal; WIP cleared; baselines re-derived only if a parent row
moved.
**WIP:** — **DELIVERED 2026-08-31** → parent **§11.190** (`INTENT/11.190.md` +
stub) + **§5.119**/**§5.120** minted + back-markers at **§5.92** and **§5.116**
(same commit, §11.161(g)); scedit mirror ledger journal `2026-08-31j` with items
**11 and 12 struck**; tester's channel **SS-26…SS-31**. Code `50185663`,
`94f2af65`, `96cfc352`. All five scope items done. **60** `default_value`
literals of 324 (34 of the slice's 35 survive the code; `camera value` rejected
with its argument) · **120** `completable` arrays, **five** false positives
where the ledger recorded one · **153** family names documented (97 flags / 46
colours / 10 font targets) and the **`font` family check ARMED** · missing-doc
proposal artifact + content read of the 266/67 delta. Gates on a fresh
`build-f71final`: seed gate **43 ok / 0 FAIL** with 3 NEW checks each
falsification-tested · `ctest` **14/14** · 0 warnings · D14 ASCII gate **PASS** ·
C3 corpus gate green with a newly armed rule · F66 parity **340/340**, hit rate
**80/340 unchanged**. Two things the round close must carry: (1) a **dispatcher
premise did not hold** — the "+267 unexamined lines" had a checker pass on
2026-08-30 (SCRIPT_SURFACE §4, SS-20…25); reported per §11.179(a), not an abort,
measured delta **266/67**; (2) harness HEAD moved to `2a4d0b0` mid-task, one
commit by the concurrent Fable session touching only
`PENDING_LEDGER-2026-08-31.md` — Allowance 1 satisfied, though that file is not
one of the two the prompt named.
**ACCEPTED 2026-08-31 (Fable, session 19):** entry read in full; trees/
commits verified (THREE code commits — the entry header names two; the third,
`96cfc352`, is the README/contract tail of the same delivery: verified
grammar+README only, noted as a header omission, not a defect); ctest 14/14 +
ASCII gate PASS re-run by me; item 11/12 flips, journal 2026-08-31j,
§5.119/§5.120 stubs and SS-26…31 all verified at their homes; baselines
reproduced to the digit. ENDORSED: the two-clause default criterion with the
`camera value no` rejection as C2's demonstration; the strong-property
completable gate (silence cannot satisfy it); the per-name split of
`checkNamesFamilyV2Content` (I2 — 97 copies of one fact refused); the
self-pinning anchors with the 324 stale ones recorded-not-repaired
(re-anchoring queued below); the 8 unverified divergences kept OUT of SS
rows with the distinction stated — the SS channel's integrity is worth more
than its row count. Both §11.179(a) handlings correct (the "+267" premise —
FIFTH dispatcher gloss this round, mine, tallied: carried from §11.149(e)
without checking SCRIPT_SURFACE §4's 2026-08-30 pass). §5.119's NUMBER
consumed by this mint — §11.185(d)(2)'s forward-looking "§5.119" annotated
at its node THIS commit (F60's stale-forward-reference class, caught at
the write). Scan limitation (only INTENT/<id>.md as event sources — a
§5-sourced back-marker is invisible) joins the strict-credit v2 package.

### F72 — POST-CLOSE: the file-origin log tag — `originTag()` names `<file>:<line>` too (§11.191(b); reverses §11.187(d)'s asymmetry on the owner's word) [S]

**Why now / mandate:** owner ruling §11.191(b) (*"good idea"* on the §3
question): a FILE-origin refusal at the diagnostic funnel and its bypassing
sibling names `<file>:<line>` in the LOG, exactly as a TCP one names
`tcp#<id>`. F68 predicted the reversal as one condition at `originTag()`
(`channel != TCP` → `channel != NONE`, §11.187(d)).

**Sources (re-read, never recall):** `INTENT/11.191.md` (b) (the ruling and
its explicit non-reach: wire, `#!` channel, the ~1661 generic-channel
decision all UNMOVED); `INTENT/11.187.md` (d)(f) (the asymmetry, the gate
legs that measured it); `INTENT/11.188.md` (d) (the three routing sites —
`sendFeedback` stays TCP-only); `harness/f68_provenance.py` (the legs that
flip); `harness/f69_feedback.py` (the frozen-wire re-check).

**Preconditions (checkable, §0.7 gate):** code HEAD `96cfc352`, tree clean;
harness HEAD as the prompt states, tree clean (concurrent-session allowances
as F69–F71 stated); next free §11 number **192**; live `### F` count **5**;
binary current at code HEAD; §11.187(d) carries the REVERSED marker;
display `:0` per HOST-EVENTS post-reboot entries. ANY broken ⇒ abort.

**Scope:** (1) the one-condition change at `originTag()` with the header
comment updated to name §11.191(b) as the reversal's authority; (2) gate:
`f68_provenance.py` legs that asserted the file-origin funnel refusal
UNTAGGED now assert the tag (both-ways: the pre binary — current HEAD's —
stays untagged; state the flipped legs in the entry); `f63_annotations.py`
re-run green (the `#!` channel must be untouched); `f69_feedback.py` re-run
— the frozen wire must reproduce 71/5/0/85 B and the `$DIAG` records must be
UNCHANGED (file-origin refusals still route to no socket); (3) record: §11
entry at **192** + stub, entry-first; back-marker at §11.187(d) flipping
REVERSED → DELIVERED; §5.117 annotation updated (its emitter now names BOTH
origins); README note; WIP per §0.6; new code pure ASCII (D14).

**Boundaries:** LOG channel only — no wire change (delivery-blocking bar as
F69/F70); no annotator change; no new tag content beyond the origin prefix;
mint license per §5.79.

**DoD:** condition + flipped gate legs green both ways; f63 green; frozen
wire reproduced; §11.192 + markers; builds green; trees clean; WIP cleared;
baselines re-derived LAST with deltas attributed.
**WIP:** — DELIVERED 2026-09-01, §11.192. Code `master-beta @ 1014e5a5` (16
lines changed for 16 — no line number moved). Gates: `f68_provenance.py`
**47/47** (43/43 before), `f63_annotations.py` **34/34**, `f69_feedback.py`
**52/52** (49/49 before), three fresh launches each; wire 71/5/0/85 B ==
F68's committed recordings, subscriber 939 B == F70's and byte-identical
pre-vs-post in one run. Both provenance instruments now take a DECLARED
pre-era (`SC_PRE_TAGS` / `SC_PRE_ERA`), which retires the three known-false
reds F70 took. One stated deviation: the condition is `where().empty()`, not
`channel == NONE` (§11.192(b)). Artifacts `harness/artifacts/f72/`.
**ACCEPTED 2026-09-01 (Fable, session 19 post-close):** entry read in full;
trees/commits verified; wires re-compared BY ME (four unsubscribed == F68's
baseline, D's 939 B == F70's recording); ASCII gate + baselines reproduced
to the digit. ENDORSED: `where().empty()` over the predicted literal (the
namer asks the naming predicate — I2 at the predicate level; degenerate-
origin-proof; the literal form one line away, veto-open at §11.192(b)); the
zero-net-line-delta discipline (F68's +25 lesson applied); the DECLARED
pre-era as §11.191(c)'s first enactment (declared-not-inferred is the
right polarity: a wrong declaration reds, never greens); the nine
neighbour instruments checked not assumed; the one-line-marker-span rule
recorded for every future back-marker author. SUPERVISOR TALLY +1, mine:
the §11.191(b) REVERSED marker reached §11.187's entry but NOT its derived
stub in my `eef776d` commit — executor-caught, fixed at both ends,
reported not absorbed (the pair-check's own divergence class, caught by
hand before the instrument). Round addendum: **F72 → §11.192**, session 19
closes at FIVE for five.

### F73 — POST-CLOSE 2: the log line becomes the intent-modified line — one renderer, shared with the annotator (§11.193(a)-(c); supersedes F72's message rendering in part) [S/M]

**Why now / mandate:** owner statement §11.193 (verbatim there): the log's
error line is the **intent-modified line** — the raw line as read (author
comments kept, so the line is unambiguous) + the ` #! <message>` tail —
origin-prefixed, self-sufficient, matching what the user finds in the file;
on a READ-ONLY file the log still shows it. Reasons owner-stated:
non-duplication of the error reporting itself + emergent resolution (log
and file are one rendering, they cannot disagree).

**Sources (re-read, never recall):** `INTENT/11.193.md` IN FULL ((c) is the
spec with its veto points; (b) Root 1 is the decoupling that makes the shape
derivable); `INTENT/11.192.md` (what F72 measured — every "did not move" leg
stays a bar); `INTENT/11.184.md` (the annotator's composition contract: tail
position by the parser's quote toggle, after the author's comment,
idempotent replacement); `src/scriptModule/script_annotator.{hpp,cpp}`;
`harness/f68_provenance.py` + `f69_feedback.py` (the legs that assert log
shapes — they flip WITH this change, both-ways).

**Preconditions (checkable, §0.7 gate):** code HEAD `1014e5a5`, tree clean;
harness HEAD as the prompt states (concurrent-session allowances as
F69–F72); next free §11 number **194**; live `### F` count **7**; binary
current at code HEAD; §11.192 carries the SUPERSEDED-IN-PART marker.
ANY broken ⇒ abort.

**Scope:**
1. **One renderer**: extract/share the annotator's line+tail composition
   (never duplicate it — I2 is this task's own subject): input = raw line +
   origin + message; output = the intent-modified line. An existing `#!`
   tail on the line is REPLACED in the rendering (the writer's idempotency
   rule, same code).
2. **The LOG consumes it** at the funnel and the bypassing sibling: ONE
   line per error — `<origin-prefix> <intent-modified line>` — replacing
   the legacy two-line emission (the collapse is part of the shape; the
   owner's example is one line). Prefix wording yours with veto (`Error
   executing <file>:5:` is the owner's example spelling).
3. **File-origin AND TCP-origin log lines** unified through the same
   renderer (TCP = [derived] in the spec, veto flagged in the entry);
   nested refusals stay origin-less and keep their current lines (§11.184's
   rule, untouched).
4. **What does NOT move** (each re-measured, F72's bars): the WIRE —
   `$DIAG|origin|message|subject` unchanged, frozen wire 71/5/0/85 B + D's
   939 B reproduced against the committed baselines; the FILE writer —
   ruled class only, `f63_annotations.py` green; message CONTENT — today's
   `debug_message` text rides in the tail (the §11.169 three-part upgrade
   stays F58's work order); §5.117's severity claim.
5. **Gates**: f68/f69 legs updated WITH the change (declared pre-era
   discipline per §11.192(f)); the sharpest leg re-derived for the new
   shape: post line == renderer(raw line, origin, message) recomputed
   independently by the gate from the raw script bytes — the gate must not
   trust the engine's own composition; a read-only-file leg (log shows the
   intent-modified line, file untouched); f63 34/34; build green `-j6`;
   ASCII gate PASS.
6. **Record**: §11 entry at **194** + stub entry-first; back-markers at
   §11.192 (DELIVERED flip of its SUPERSEDED-IN-PART) and §11.193; §5.117
   annotation updated; README; WIP per §0.6; new code pure ASCII (D14);
   one-line marker spans (the §11.192(h) scanner rule).

**Boundaries:** LOG rendering only — no wire change (delivery-blocking), no
file-write behaviour change, no message-content rewrite, no nested-origin
change; mint license per §5.79.

**DoD:** renderer shared not duplicated; one-line log shape measured
both-ways incl. read-only leg; all F72 bars re-held; §11.194 + markers;
trees clean; WIP cleared; baselines re-derived LAST with deltas attributed.
**WIP:** — DELIVERED 2026-09-01, §11.194. Code `master-beta @ 9a3b7a55`
(+36 net lines; the shift map and the two annotated citations are §11.194(i)).
Gates: `f68_provenance.py` **58/58** (47/47 before), `f69_feedback.py`
**53/53** (52/52), `f63_annotations.py` **34/34**, three fresh launches each;
`f22_b10_offset.py` and `f4_scriptspeed.sh` re-run green on rewritten legs.
Wire reproduced: 71/5/0/85 B == F68's recordings, D's 939 B == F70's,
pre == pre2 == post in-run. The renderer needed no extraction — the
composition has been a pure static on the annotator since §11.184; the gates'
expected line is recomposed independently by the new `harness/f73_line.py`.
The addendum's re-execution leg landed as a gate leg AND as a code change
(`reportScriptError`'s bracket now excludes the machine tail). FOUR neighbour
instruments were WRONG, not stale: two would have gone red, two vacuously
green. THREE veto-open items: the `Error executing ` prefix for both origins,
the TCP unification, and `reportScriptError` keeping a SECOND form (§11.193(f)
anchor-first says a sink should not). Precondition reported not absorbed:
§11.192's SUPERSEDED marker had reached its entry and not its stub.
Artifacts `harness/artifacts/f73/`.
**ACCEPTED 2026-09-01 (Fable, session 19 post-close 2):** entry read in full;
wires re-compared BY ME (four == F68's recordings, D == F70's); baselines
reproduced to the digit; trees clean. ENDORSED: `Error executing` for BOTH
origins (the collapse deletes the only error-words; asymmetry would need a
stated reason); TCP unified (the rendering's property, not the channel's);
the addendum-forced `withoutAnnotation` bracket fix (the owner's
formation-time property demanded it — in-scope by the addendum's own
mandate, log-only, wire-certified); the recomposition instrument OUTSIDE
the gates (one copy, its own subject); the FOUR-WRONG-NEIGHBOURS census
with its class separation — the BLIND pair (vacuously-green negatives) is
the round's sharpest instrument finding, §11.191(c)'s class caught
pre-emptively. THE (a) FINDING BANKED: the records coupled what the code
had already separated (`withAnnotation` public static since §11.184) — a
record that says less than its own code, named as a failure mode.
`reportScriptError`'s second form routed to §3 (the one unsatisfied
anchor-first site, owner's call). TALLY item 9, mine: the §11.192
SUPERSEDED marker at entry-not-stub (my a0f936b) — third instance of the
class ⇒ the marker-both-homes step is now named a CLASS defect; a
pair-check extension candidate joins the strict-credit v2 package
(deliberate instrument act, queued). F74 dispatches on this state.

### F74 — POST-CLOSE 2: §5.119's fix — the past-the-end dereference removed, under the owner's two conditions (§11.193(e)) [S]

**Why now / mandate:** owner authorization §11.193(e), verbatim: *"you can
correct it, but the one correcting it must verify it's structurally
reachable, not just seemingly so - and not modify the behavior beyond
removing the UB access."*

**Sources (re-read, never recall):** `INTENT/5.119.md` (the row: the claim,
the neighbours' guard shape, the one-line reach `set sky_locale zh_CN`);
`INTENT/11.193.md` (e); `src/appModule/fontFactory.{hpp,cpp}` at HEAD.

**Preconditions (checkable, §0.7 gate):** F73 delivered and accepted; code
HEAD = F73's delivery sha, tree clean; next free §11 number **195**; live
`### F` count **7**; §5.119 carries the FIX-AUTHORIZED marker. ANY broken ⇒
abort.

**Scope:** (1) STRUCTURAL reachability first: the full call chain from
`set sky_locale zh_CN` to the dereference, every guard on the path read and
cited; then a MEASURED demonstration — a sanitizer build (`-fsanitize=
address` on the affected objects or a local asan build; the desktop
build-asan binaries do not load here) or equivalent instrumentation showing
the UB access fire on that one script line; if reachability FAILS the
structural test, STOP — the finding replaces the fix (report, no code
change, §5.119 annotated with the analysis). (2) The fix: the neighbours'
guard shape, removing the UB access and NOTHING else — every defined-path
behaviour byte-identical (as-if bar); state the defined-behaviour argument
in the entry. (3) Gate: the reachability demonstration both ways (pre
binary fires under sanitizer, fixed binary does not; the same script's
DEFINED effects identical on both); build green; ASCII gate. (4) Record:
§11 entry at **195** + stub; §5.119 flipped FIXED with the reachability
record; README; WIP; D14.

**Boundaries:** one function's guard — no other behaviour change, no
refactor, no severity/log change beyond what the guard's own path needs
(D12 if the guard ACTS: log the skip, §2(f) shape); mint license per §5.79.

**DoD:** reachability established structurally AND demonstrated, or the
finding replacing the fix; the guard landed with the as-if argument; gates
both ways; §11.195 + §5.119 flip; trees clean; WIP cleared; baselines
LAST.
**WIP:** —

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

- **Session-19 decision items (2026-08-31, the provenance/ASCII round):**
  - **THE CANARY'S START-EPOCH PROBE IS WRONG, AND ONE OF ITS TWO MEMBERS
    GATES** (F69 §11.188(j)): `f56_canary.sh:270` reads `/proc/<pid>`
    directory MTIME as a start time — pid 14079 "restarted" 19 h into its
    own life by that probe (three-way measurement committed). The
    `xserver.restarted` NOTE F68 saw was a non-event (HOST-EVENTS
    corrected); the SAME proxy backs `compositor.restarted`, a GATING
    fail-2 — **on the desktop this can falsely abort any photometric task**.
    Fix = one line (btime+starttime) PLUS a re-bank of two banked epochs ON
    THE DESKTOP (one VALUES edit with argument, §0.5). Queued as next
    desktop-round position 1; not fixable from this laptop. **[OBJECTIVE
    RULED 2026-08-31 → §11.191(c): *"falsely abort is not as bad as wrongly
    continue, but that's to optimize against (false positive)"* — fail-closed
    stands; the fix removes the false positive at its root (the wrong proxy);
    no band-widening or gate-demotion is licensed.]** **[EXTENDED at
    F70 acceptance: the 20:57 reboot moved this laptop's display to `:0` —
    `BANK_DISPLAY=":2"` now names a display that does not exist here, canary
    exit 3 (was 2). Same disposition: per-host banks are your §11.174(f)
    fork; nothing re-banked.]**
  - **[RATIFIED 2026-08-31 → §11.191(a): *"we mustn't change the
    user-visible part"* — the escape rule stands owner-endorsed; the
    reversal path is dead; the map-row sub-vetos stand by silence.]**
  - **D14'S ONE DESIGN FORK — the ESCAPE RULE (F70 §11.189(c)(h), veto-open,
    the round's sharpest):** the tree is 100% ASCII (282 files, 8269 bytes,
    gate standing), but C-family STRING LITERALS were re-spelled as `\xNN`
    escapes of the SAME bytes rather than transliterated — so the dome still
    draws `°` at 22 sites, wire records and pinned fixtures are byte-identical
    (27/27 ELF sections equal), and D9's shipped surfaces never moved. If
    *"accents are to be removed"* was meant to reach the RENDERED text too,
    say the word: the cost is `45deg` in the dome's labels and five scedit
    records re-recorded. Also veto-open there: `§`→`S`, `°`→`deg`, `·`→`*`
    map rows; the vendored trees excluded; one visible loss (`Jérôme`→
    `Jerome` in a CMake banner — no escape exists in CMake).
  - **`$DIAGON`/`$DIAGOFF` — the dedicated link's protocol, three calls
    veto-open** (F69 §11.188(b)(c)(g)): (1) the VERB NAME (`$SCEDITON`
    rejected on I1, `$FEEDBACK` on a word collision — the channel is named
    by what it carries); (2) **`$NOTICE` does NOT advertise it** — that
    reply is itself bytes on masterput's wire; one line reverses if you
    want discovery over the freeze; (3) the one wire delta on unsubscribed
    connections: the literal string `$DIAGON` is now answered at the socket
    layer instead of silently refused by the app — the structural minimum
    of the opt-in your ruling mandated, bounded to that exact novel string.
  - **NESTED REFUSALS REACH NOBODY — the feature's sharpest gap, and it is
    §11.184's rule** (F69 §11.188(h), measured): a command run inside
    another (`media` → `audio`, `clear`'s thirty) passes no origin, so its
    refusal routes to no wire and no `#!` — the subscriber is told nothing
    while the log carries it twice. Making a nested call inherit the outer
    origin is a change to §11.184's recorded rule: yours to say.
  - **F71'S ENGINE HARVEST — two new §5 rows and a retraction, all READ not
    RUN** (§11.190(d)): **§5.119** `FontFactory::updateAllFont` dereferences a
    past-the-end iterator, reached by ONE script line (`set sky_locale zh_CN`)
    — record-don't-fix held; the fix looks one-line and is yours to word;
    **§5.120** the lunar-eclipse colour/flag quartet is routed-and-inert,
    reporting success (§5.116's family); §5.92's "inventory discharged"
    RETRACTED at its node (forwarding crosses files — `dso3d action load
    index abc` still reaches an uncaught `std::stof` terminate). TESTER
    CHANNEL: **SS-26…SS-31** minted — sharpest: `on` means yes on every
    `flag` and NO on `media pause` (two coercion functions, field data
    measured clean today); `transition action skip duration` is documented
    seconds, is milliseconds; only 18 of 46 colour names are ever driven as
    colours in his own file. The 324 grammar `source` anchors are pinned at
    `b12c8cdd` and no longer resolve at HEAD (+141 lines) — re-anchoring
    queued, new rows self-pin.
  - **[ANSWERED YES 2026-08-31 → §11.191(b): "good idea" — word-given as
    post-close task F72 (the one condition + flipped gate legs; log channel
    only, wire and `#!` decision unmoved).]** **[SHAPE CORRECTED 2026-09-01
    → §11.193: the ratified shape is the INTENT-MODIFIED LINE in the log
    (raw line + comments + `#!` tail, one line, origin-prefixed), not a
    prefix on the legacy messages — delta traced to three roots (the
    rendering/write coupling in our records; my question's narrow frame,
    seventh tally; two I2 derivations locating the authority at different
    nodes). Re-dispatched as F73.]**
  - **SHOULD A FILE-ORIGIN REFUSAL NAME `<file>:<line>` IN THE LOG?** (F68
    §11.187(d)): the funnel now prefixes TCP-origin refusals with `tcp#<id>`;
    the file half is DELIBERATELY absent — the executor read "every failing
    command names its line in the log" as the LOG half of §11.184's disclosed
    ~1661-tails decision (still yours) and declined to prejudge it. The other
    reading is real (a log line naming the line costs nothing; the log is not
    the script). Reversal = one condition at `originTag()`
    (`channel != TCP` → `channel != NONE`). Both readings at the definition.

- **Session-18 decision items (2026-08-31, the scedit round on TravellingFoxDev):**
  - **[ANSWERED IN FULL 2026-08-31 → §11.186(a) + HOST-EVENTS `149d518`:
    lock-enabled false AND idle-delay 0, both measured. The owner enacted the
    second arm mid-round on reading the residual flag, stating the narrowed
    first enactment traced to THIS item's own headline (lock named, blank
    operative) — supervisor report-shaping defect, tallied. Hazard removed
    both arms; lock-vs-blank attribution closed as moot; executors keep a
    zero-cost GetActive check-and-record at launch.]**
  - **YOUR DISPLAY DURING DISPATCH: a locked screen runs the engine at 1 Hz.**
    Measured: screensaver active → `Frame stall detected` every 1000 ms for
    whole runs (105/run, HEAD and the pre-fix control alike); awake → 1. Any
    fixed-sleep instrument or per-frame claim taken on a locked session is
    wrong by up to a second per frame (F63's 34/34 survived it only because it
    waits on the script log). Options: (a) the claude session never blanks/locks
    while dispatch runs (`idle-delay 0`, lock off) — a host setting, YOURS;
    (b) each launch wakes the display itself (`org.gnome.ScreenSaver.SetActive
    false` + `SimulateUserActivity`, what the control did once) — an instrument
    mitigation reported here per §11.174(h): say the word. Side fact: that D-Bus
    call DISMISSED THE LOCK from inside the session (`LockedHint` yes→no) — a
    process in claude's session can unlock claude's screen.
  - **THE CANARY BANK IS THE DESKTOP'S** — on this laptop it fails
    `display.geometry` (1920x1080) and `compositor.absent` by construction.
    Per-host banks (a host key in the VALUES block) = one edit + one scene run
    per host; the :2/:4 fork stays open on the desktop. Not done.
  - **THE `#!` GENERIC CHANNEL** (FEATURE_REQUESTS 2026-08-30): still yours —
    yes / no / which subset (unknown command and unknown flag dominate; ~1661
    tails into 35 shipped scripts on first run).
  - **LLM ASSISTANCE TRIAGE** (FEATURE_REQUESTS 2026-08-31): F66 builds the
    tool surface (no model call); the standalone router (gemma4 on CPU, 58.5%
    two-level) waits for: which mode first · local-only default · checker as a
    hard gate in agent mode.
  - **THREE UNTRIAGED ENGINE REQUESTS NOT DISPATCHED** — `[parallel-script]`
    routes to the tester as a RESUBMISSION (§11.173's interface; the
    resume/speedup clauses still unwritten); `[script-binding]` needs the
    key-map authority decision (one surface or two, B37); `[script-trigger]`
    `@requires` binding. Implementable only after your triage — say which, in
    what order, and they get sections.
  - **Remotes:** local contains origin on BOTH repos (a push would
    fast-forward); `git fetch` is refused for this user (auth) — the
    2026-08-30b "not fast-forward" note is superseded by measurement.
  - **THE CONTROL CHANNEL SAYS NOTHING ABOUT A SCRIPT IT PLAYS** (§11.185, F67,
    measured live: after `$LOGON`'s confirmation an entire play that wrote five
    `#!` diagnostics put ZERO bytes on the wire; `setOutput` has two callers —
    `get status` and `search name`). Three questions routed, none decided:
    (1) should the control channel carry script lifecycle at all (one line at
    `terminateScript` would replace scedit's ≤1 Hz file watch with an event);
    (2) own §5 row (`§5.119`, one edit) or §5.72 + §5.117 read together (both
    carry back-markers now); (3) **who may move the dome** — the socket
    authenticates nobody at either end and MCP `run_command` reaches whatever
    listens (README § For machines says so; binding it to a machine that can
    reach a real dome is a decision, not a default).
  - **scedit UX calls taken by the executors, README-stated, veto open:** the
    reading of "history" (the current buffer's set — the engine's `#!` channel
    IS the log); the pane keys (F5/F3/F4 + Ctrl-E/N/P, default-closed, count on
    the status line); the live keys (F6–F9, F11/F12, Ctrl-U; a second Ctrl-S
    takes the destructive branch, the quit-warning precedent); the 500-line
    feed; the ≤1 Hz / ≤5 min post-play file watch (the guarantee lives in
    `save()`, which compares the file before EVERY save and refuses); the MCP
    field names, tool descriptions, version string `0.5.0`, dual-era protocol
    (2025-11-25 handshake + 2026-07-28 stateless — the deployed Claude Code
    speaks the former, measured), the ported ranking formula; `-Wall -Wextra`
    on scedit's own targets only.
  - **THE DISPLAY WAKE RAN UNDER AN EXECUTOR, twice** (F67's two live launches:
    screen locked → `SetActive false` + `SimulateUserActivity`, recorded in the
    result JSON, 1–2 stalls/run) — as the F67 section instructed, reported per
    §11.174(h). Your standing answer (unblanked session during dispatch, or
    per-launch wake as policy) decides whether the next display-needing task
    carries the same clause.
  - **Facts, no decision asked:** (a) the Bash `grep` wrapper skips EVERY
    ISO-8859 file silently (ugrep `-I`; `app_command_interface.cpp` is the one
    such file in `src/`) — CLAUDE.md/§0.5 corrected, and CLAUDE.md turned out to
    be ONE file (the code-tree path is a symlink into `claude/`); (b) two
    instruments overwrote their own evidence this round (`f64_doc_router.py`'s
    one-level rows, untracked; `f67_tcp_live.py`'s first-run host record) —
    Q-56's class, both now timestamp their artifacts or are banked as hazards;
    (c) an executor's binding smoke ran `claude -p` on YOUR credentials
    (stopped by "Credit balance is too low") — recorded as a boundary: no
    owner-credential invocation without your word; (d) F64's Python family map
    missed `font` (10 targets never shown to the model) — the copy-vs-authority
    defect the F66 surface removes; (e) the back-marker scan's two new unmarked
    candidates are both §11.182's: its forward marker `→ §11.183` read as an
    event (both nodes name each other — a marker-shape mismatch), and
    `§5.10` = scedit's `tests/derivation-diff.md` §5.10 (a namespace collision,
    F60's NOT-A-ROW class) — candidates for the named-exception partition, left
    to the strict-credit v2 package that owns it.
- **Session-17 decision items (2026-08-30, F56–F60 / §11.176–§11.180):**
  - **THE DIM ERA IS FULLY ATTRIBUTED AND FENCED, and b3_ladder WAS NEVER RED**
    (F56 §11.176(g)): on the healthy stack the unmodified ladder returns
    §11.104(d)'s JULY numbers to the digit — the ±1.5 % drift, the failing gate,
    the shadow witness, all of it was the two wrongly-dispatched sessions; the
    dim/healthy ratios re-derive §11.167(e)'s transform from the other side.
    **§11.164(l)(2) is refuted at its node**: July's absolutes are TARGETS again —
    behind the canary. The driver bump is photometrically inert from both sides.
    NEW reach bound (§11.176(e)): the dim state never touched the star channel
    (byte-identical frames across the epoch) ⇒ confined to textured-body shading,
    every global-output-transform candidate eliminated. **YOUR ONE OPEN FORK: the
    canary banks its band on :2 (the F43 substitute) — is :2 or :4 (your real
    session) the canonical render host? (§11.174(f))** Re-bank = one VALUES edit +
    one run. The correction sweep marked all 17 suspect clusters (exactly
    §11.157/§11.164/§11.167); "likely clean" proved clean BY CHANNEL.
  - **THE SCRIPT SURFACE CANNOT SAY NO — three rows, one family** (F58 §11.178 +
    F58-acceptance mint): **§5.116** `flag atmosphere yes` turns the atmosphere
    OFF and reports success (every unrecognized flag value → OFF, every
    non-numeric set value → 0; `Utility::isBoolean` was written and never wired —
    the tree's own evidence of oversight); **§5.118** `set stall_radius_unit`
    silently drops any value ≤ 1.0 behind a void wrapper (composes with §5.116
    into exactly §5.97(c)'s measured shipped line — the SS-6 witness open since
    2026-08-04); **§5.117** all 157 script-error refusals are written at L_DEBUG
    into the one log that never rotates (§5.115's). Parent decision = §5.116's
    class fork: refuse vs named-default-and-log, with D9 cutting both ways
    (shipped shows may depend on a bad value meaning OFF). Owed before pricing:
    one launch with a deliberately bad script.
  - **THE §11.169 SCHEMA IS ALREADY YOUR PRACTICE — the gap is legacy inventory**
    (F58 §11.178(d)): of 309 schema-applicable log records, errors carry WHAT
    (0.49 + 0.43 partial) but CONSEQUENCES 0.07 / PREVENTION 0.06; acting
    defaults carry CAUSE 0.77 but OVERRIDE 0.09. **14 of the 15 complete records
    were written by this project after §2(f) was recorded** (the 15th is your own
    `protosystem.cpp:530`, 2022). Work order committed
    (`harness/artifacts/f58/f58_gap_table.tsv`): the cheapest line is ONE
    sentence at one chokepoint (a refused command stops nothing and tells
    nobody — the return is discarded at both transports); the illuminate clamp
    and §5.115's bulk class are ONE code path, so §5.115's compression arm is a
    dependency of that class's fix, not an alternative to it.
  - **THE TESTER MODEL, MEASURED — its mechanism moved** (F57 §11.177): the
    throughput signature is neither brevity nor error (within-class correctness
    1.0) but **COVERAGE PER ITEM** — 0.938 → 0.788 → 0.500 as a question
    accumulates parts, and an isolated re-ask refills it (0.944). His depth is
    spent VOLUNTEERING (21 unrequested propositions, 4 became tracker rows), not
    covering. **"Correctness necessary, secondary" is UNSUPPORTED — stop citing
    it as measured.** Seven final-pass question-shape rules derived and recorded
    (§11.177(i)): one decision per item · never bundle history (0.083 answer
    rate) · leave a volunteer slot · name the observable · no mechanism
    questions · a re-elicitation trigger needs a feedback channel. TWO OWED
    RE-ASKS join the final pass: Q11's *"more than one system loaded at once?"*
    (dropped, was on NO row — conditions A30's scope) and A7's multi-star halo.
  - **§5.28 NAMES TWO DEFECTS** (F59 §11.179(i)(M4)): the 2026-08-04 Translator
    mint reused B14's retired number. Resolved IN PLACE (disambiguation notes at
    four homes, allocation root closed in the archival note: max over live ∪
    archive); **your one-line veto swaps to renumbering** if you prefer the
    other cost curve.
- **Session-17 veto points (implemented-and-live, each cheap to reverse;
  silence = endorsed):** (1) the ENVIRONMENT CANARY as a §0.5 standing
  precondition (one bullet); (2) **`HOST-EVENTS.md` created** — §11.174(e)'s fix
  (1), the append-only dispatch-environment channel, seeded from the ledger
  (your entries welcome; one file to delete); (3) the §0.7 SCOPE RULING (abort
  binds on inputs; a failed output-side gloss = report + counterfactual, not
  abort — §11.179(a)'s case); (4) the §0.5 ROW-FLIP extension (a §13 state flip
  greps the §5 register in the same commit — I3 at the ledger layer, from
  §11.180's finding that every stale reference sat on the non-working side);
  (5) the archival DEPENDS-ON ruling (a citation alone does not block archival);
  (6) §5.118 supervisor-minted (the F32 precedent — false success from a
  shipped command belongs in the registry).
- **Session-17 equalization items (facts, no decision asked):** the register's
  cross-reference axes are now AUDITED — §5-side back-markers 0 arrears in 44
  pairs, multi-claim 0 uncovered in 49, row↔row 5 stale in 134 (all annotated or
  routed) · the scan's residual partition is **82 + named exceptions** ("83
  real" was a carried-forward label, corrected at four homes — the executor
  caught the supervisor's gloss, the gate's report-not-absorb clause working in
  the new direction) · the first-60 s + dim-era instruments now compose: any
  future photometric task opens with `f56_canary.sh` and a cache manifest ·
  strict-credit v2 has FIVE measured members and stays ONE deliberate act ·
  supervisor-error tally this round: two dispatcher glosses, both
  executor-caught, both corrected at nodes.

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
