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

**Update [Fable 2026-09-04, supervising session 21 — LovelyFoxDev, the DESKTOP
round: canary / stellar grammar / view offset]:** trigger = the §0b verbatim line,
no in-line transmission. Warm-up: both trees clean at open, code `ba7a32a8` /
harness `d79180b` (one commit landed after session 20's close — **§11.198**, the
round-3 conversation, 2026-09-02); definition-drift assert md5 MATCH (`a5a54d94`);
next free §11 number **199** (live ∪ archive); live `### F` count **4 → 0** by
**archival pass 13** at OPEN (update-s19 + F75–F78, 842 lines, manifest
`2026-09-04-pass13`, pre-md5 `d351a644` reproduced in-process AND from disk,
commit `db0dc57`; one recorded seam tidy) **→ 4** by the mints below. HOST: this is
**LovelyFoxDev, the desktop** — the first desktop round since session 17; booted
18:45 today; claude's REAL logind session (14/15, remote-desktop shape) serves
**`:2`** at 2448x1332 under `/run/user/1003`; F43's `/tmp` substitute is gone and
unrebuilt ⇒ **the §11.174(f) fork has one branch left**; canary `--no-scene`
**exit 2 `compositor.absent`** + note `xserver.restarted` (the refuted mtime
probe) — REPORTED, not mitigated (§11.174(h)); full record HOST-EVENTS 2026-09-04,
commit `17dd50f`. Binary was STALE (Aug 26 — the laptop rounds never reached this
build dir) ⇒ rebuilt at `ba7a32a8` (-j24 per the session hook; affinity is 0-23
here, so the §0.5 "12-core affinity" line is not this host's state today; RAM 52
GiB avail), md5 `c8e12950`, `cmake --build -- -n` empty after. GitHub SSH refused
from this host (publickey) — push impossible here; local contains origin on both
repos (+58/+604). No sync daemon: the tree carries the laptop's commits by the
owner's hand; writer set during the session = this session. Config/ssystem md5
pristine; no spacecrafter process; ASCII gate PASS. Instrument baselines at open:
scan **203/252/125** · pair-check **214/189/25/95** D 35 · D2 11 · I 88 · I2 36 ·
M 81 — every delta over the session-20 close (+1/+1/+1 · +1/+1/0/0 · I+1 · M+1)
attributes to §11.198 (its `entry_only 11.4` pair visible in test I). SUPERVISOR
ACTS AT OPEN (commit `17dd50f`): the F60 routed flips — §5.24 CLOSED as FIXED by
B32 (its harness-exclusion claim refuted at `b24_equivalence.py`); B15's reference
half recorded closed by §5.32; B39's "never caches" clause recorded false since
§5.46's fix ((j)(4), which the session-20 re-queue had omitted without a reason —
taken with the other two). **Dispatcher defect #1, mine:** the re-queue's third
member (B14's §5.28 citer note) was ALREADY enacted at F60 acceptance (`df97965`,
2026-08-30) — a stale re-queue entry. QUEUE CONSUMPTION (session-20 close, in
order): (1) desktop position 1 → **F79** (the canary: probe root-fix + fingerprint
re-bank + the band by measurement; the re-bank TARGET is an OWNER DECISION per
§11.174(f)/(h), asked before dispatch and recorded in the section); (2) the F60
flips — DONE above; (3) scedit item 4 → **F80** (L; minted from the scedit
ledger's four-line stub + C4 + the loader's key sites, measured at mint:
`protosystem.cpp` 103 quoted keys · `ssystem_factory.cpp` 34 · `ModularSystem.cpp`
81); (4) the anchor content-vs-sentence audit — Vixy's word, not minted; (5) items
5/8/19 — Vixy's triage, not minted; (6) F52(k) · F58 record-only carry (nothing
downstream waits); (7) engine on Vixy's word — unchanged. NEW since session 20,
from **§11.198**: (d)'s view-offset DEFECT CANDIDATE ("a next supervising
session's call") → **F81** (S, one functional launch, display-bound — the display
exists here; sites re-read at mint); (e)'s portrait leg ("a cheap candidate") →
**F82** (S, the EXTENSION member). Picks: **F79 → F80 → F81**, F82 if health
permits. Deliveries: F79/F81/F82 to the parent (§11.199+, refreshed at each
dispatch); F80 to the MIRROR ledger (journal `2026-09-04a`; a parent number only
if an engine finding mints one). Launch classes: F79 PHOTOMETRIC by design (the
band re-measure IS the canary's scene arm, run on purpose, six times); F81/F82
FUNCTIONAL (dump/frame geometry, no photometric claim). Remotes: local contains
origin on both; push from this host impossible (HOST-EVENTS).
**Round outcome (session 21 close, 2026-09-05 ~01:00):** F79 → **§11.199** · F80 →
scedit journal `2026-09-04a` + **§11.200** + **§5.124–§5.127** + SS-40…43 (after
ONE §0.7 ABORT on four premises of mine, re-dispatched same day to the same
executor) · F81 → **§11.201** + **§5.128** · F82 → **§11.202** + **§5.129** —
**FOUR for four** delivered AND supervisor-verified same session, every delivery
re-verified by my own runs or recomputation (canary both arms; a fresh scedit
build + ctest 19/19 + the strict corpus run reproducing 4014/0/0; the F81 command
leg re-launched and reproduced on all six cells; F82's 368-blob `verify` and the
`VulkanMgr.cpp` site read). Code `ba7a32a8 → 85cc2785` (five executor commits, all
`util/scedit/`; NO ENGINE BYTE this round — every engine finding record-only);
harness `d79180b →` this close. IN-SESSION OWNER TRANSMISSIONS, all homed: `/tmp` is
session-lifetime; three carried trees; the ssh era keeps `:2` open (SDL2 → X11 →
DISPLAY); the Q-59 collision cause (two concurrent sessions — Fable-5 head → Q-62);
`CLAUDE_CODE_THRIFTY_SONIC=0` verified at the 2.1.260 bundle and ENACTED at the
owner's word in the carried `~/.claude/settings.json` (Q-60 RESOLVED). HEADLINE
FINDINGS: the canary's fingerprint re-banked on the real session with the band
reproduced a third way and the dwell frame byte-identical across two compositors
AND two binaries; the stellar-system file has a contract — 142 keys / 422 read
sites — and 8.2 % of the field data reaches no reader (§5.124), one authored line
(`halo = on`) makes the two paths disagree (§5.126), a use-after-free reachable
from the shipped `anchor.ini` (§5.127); the view offset's two couplings are real
and `set zoom_offset` while tracking is a 29° teleport on the old path only
(§5.128); a portrait window draws the dome 128 px low with a dead band above it,
one shared stage, parity empty (§5.129). SUPERVISOR TALLY, session 21: **sixteen
dispatcher defects** (four ABORT-grade — F80's D16–D19/PROPOSED/`type=`/shipped-copy
set, root: an unmarked answered node §11.78(e), marker now placed; twelve
report-only/output-side — the stale B14 re-queue member, "6734", "24 members",
the harness CONVERT-class gloss, "`cmake -n` empty", the 19:50:18 echo of the
refuted probe, the base-D pointer, method-dependent bounds, "arm `camera`",
"10.5°", the `setLocalVision` aim command, F82's stale `ba7a32a8`), all caught via
report-not-absorb or the §0.7 gate and corrected at their nodes; **four
instrument slips of mine**, all caught before they became findings (the archival
reconstruction assert firing on my own proof — nothing written; the §5.104/§11.104
stub-collision misread; the Q-61 append past the queue's `<EOF/>` marker; the F81
re-run on the shipped `init_view_pos` instead of the run's `initview`). EXECUTOR
criterion-integrity instances this round: **twelve** (F79's refuted P2 · F80's
abort, its corpus-refuted census, the composed-only-≠-new-format recognition, the
`--strict` first run, the `camera` refusal · F81's searched-not-assumed meridian
premise, the mutated model refuted four ways · F82's failed-and-kept S_P1, the
withdrawn resize claim, the predicted-first third launch, its own three marker
corrections). BASELINES AT CLOSE (v2, root-required): scan **209/256/126** ·
pair-check **218/193/25/101** D 35 · D2 11 · I 89 · I2 36 · M 81 — every delta over
the open (204→209 / 253→256 / 125→126 · 214→218 / 189→193 / 25 / 95→101) attributed
per task in its acceptance line; the scan's +1 unmarked is F81's `§11.201 → §11.159`
off-axis pair (method citation, adjudicated, no marker owed). Archival pass 14
(update-s20 + F79–F82) DEFERRED to the next open — recorded here so the deferral is
not silent. NEXT-ROUND QUEUE, in order: (1) archival pass 14 at open; (2) the R28 /
R21 / §5.129 / §5.126 owner answers — none dispatchable, all priced; (3) scedit
items 5 (`app_command_eval.cpp`) · 8 (the emitter, now TWO contracts) · 19's router
half — Vixy's triage; the anchor.ini THIRD contract (unlocks `camera`); the anchor
content-vs-sentence audit — Vixy's word; (4) F52(k) · F58 record-only carry; (5)
the §5.127 members if any is scheduled (own rows first); the Moon's 4.98 px
crescent (a photometric leg, canary green here); the X-server selector's
uid-blindness (§11.199(j), one line); (6) engine, on Vixy's word only: §5.116's
class decision · §5.117's stream fork · §5.119's rider · §5.121 · §5.124–§5.129
fixes · the three untriaged script requests · §5.100 (authorization unanswered).
DECISIONS_PENDING open set: EMPTY (its own header, since 2026-08-26 — the fact
whose grep-shaped misread cost F80's abort). Remotes: GitHub unreachable from this
host (publickey); local contains origin on both repos, +63 code / +638 harness at
close (measured `git rev-list --count`, the close commit included — a first draft
of this line said "+630", reconstructed rather than read: the empirical-sediment
rule, applied to itself); push from the laptop or after the owner's key lands here.

---

**Update [Claude Fable 5.1 2026-09-05, supervising session 22 — LovelyFoxDev, the
REFERENCE round: reconcile / newcomer / entry document]:** trigger = the §0b
verbatim line PLUS an in-line transmission: *"Read back and update if needed
DEPLOYMENT-MAP.md to focus on the work necessary for clean deployment (and stable
reference)"*. The phrase had no prior use in the ledger (grep live ∪ archive ∪
`~/shared`, 0 hits); ONE question asked, answered [vixy 2026-09-05, verbatim]:
*"Stable reference is because another junior developper, major of his promotion,
5th year post-bac, will work on spacecrafter. I would prefer this branch to became
the stable reference for development, otherwise work will continue and require
further feature port. His work will start in a week."* — a SECOND deployment
criterion (DEPLOYMENT-MAP **R0**), a one-week horizon [stated], and the owner's
declared reduced capacity this week [vixy: *"probably a bit overloaded and not at
the best of my capabilities this week"*] ⇒ this session asks nothing further and
closes with a compact decision list. Signing identity: this session signs
**`Claude Fable 5.1`** (the Bash tool's Git section is the identity authority per
`~/shared/QUEUE.md`'s header rule); prior sessions' `Claude Fable 5` signatures
are theirs, untouched (the QUEUE's over-claim lesson). Warm-up: both trees clean at
open, code `85cc2785` / harness `34b6cae` (nothing landed since the session-21
close; +63/+638 unpushed; GitHub still refuses publickey from this host);
definition-drift assert MATCH (`a5a54d94`); binary current (`c8e12950`, `cmake -n`
zero steps, no `src/` file newer); next free §11 number **203** (live ∪ archive);
live `### F` count **4 → 0** by **archival pass 14** at OPEN (update-s20 + F79–F82,
654 lines incl. one pass-13-style seam tidy, manifest `2026-09-05-pass14`, pre-md5
`388deb00` reproduced in-process AND from disk, commit `dadf6b2`) **→ 5** by the
mints below; same boot as session 21 (`uptime -s` 2026-09-04 18:45:08), `:2`
2448x1332 under `.5KBYU3`, canary `--no-scene` **exit 0** (30 members, artifacts
`f56/canary/20260905-095822`, ignored path); config/ssystem md5 pristine
(`03fbee59`/`545a51ef`); no spacecrafter process; ASCII gate PASS (970 CONVERT
files); RAM 50 GiB avail, `-j24`. Instrument baselines at open: scan
**209/256/126** · pair-check **218/193/25/101** D 35 · D2 11 · I 89 · I2 36 · M 81
— to the digit of the session-21 close. QUEUE.md: the owner announced a rework
mid-session then withdrew it ("already done, I mistracked it") — writable; nothing
queued there this session. DEPLOYMENT-MAP read back in full and REWORKED (tier
**R** added — the development-reference criterion R0–R6; a "necessary, and only
that" head; the tester tiers kept under §11.163(h)'s structural correction).
MEASURED AT OPEN for R1 (the branch): local `2023-master` (`194c6074`,
2025-09-20) ⊂ `master-beta`; `origin/2023-master` = `6ec2f43f` (last fetched
2026-08-03 — a fetch is IMPOSSIBLE here) is 15 commits ahead by SHA but **2 by
content** (`git cherry`: 13 `-`, 2 `+` = Kenan-Blasius's `c69687bc`+`6ec2f43f`,
video-as-`s_texture`, 4 files under `src/tools/`, all pure ASCII); a trial merge
in a scratch worktree (aborted, tree clean after) conflicts in 3 files / **4
hunks, every one "keep ours"** (F70 ASCII ×2 · B31 `SC_SESSION` · F62
`div/mul/mod`); submodule pin ours-advanced only (`7ce58350` vs `4e599c35` on
both theirs and the base); version strings EQUAL both sides (2026.07.11 ⇒
§5.112 silent on merge); host FFmpeg 7.1.1 / libavcodec 61.19.101 (the deployed
line's "FFmpeg 8" commit is content-present already). Also measured for R2:
`install_src.sh:22` `[ -n "$BUILD" ] &&BUILD=Release` sets the default only when
already set ⇒ a newcomer's `cmake -DCMAKE_BUILD_TYPE=` is EMPTY (the script's own
comment states the Release intent); INSTALL is the zip/Windows-VCPKG text and never
says `--recurse-submodules`; both install scripts `sudo cmake --install` into
`/usr/local`; the field config is 315 lines / 266 keys / 0 comment lines / 16
sections and `data/default_config.ini` is 3 lines ⇒ the config is app-generated
from `checkConfig`'s schema. **SUPERVISOR DEFECTS AT OPEN, both mine, both
corrected before any mint:** (1) "the same feature built twice" (the
`app_command_eval.cpp` conflict read as F62's aliases vs Calvin's modulo) —
refuted by `git cherry` + the hunk text (it is F70's accents; the modulo commit is
content-present); (2) "15 commits / 26 files" — a SHA count, not content. QUEUE
CONSUMPTION (session-21 close): (1) archival pass 14 — DONE; (2) the
R28/R21/§5.129/§5.126 owner answers — none asked this week (capacity); (3)–(6)
deferred under the new criterion, recorded not dropped; the §5.127 granularity
veto is honoured by F86's own-row-first checkpoint. Picks: **F83 → F84 → F85**,
then F86 and F87 if health permits (both S). Deliveries: all to the parent
(§11.203+, refreshed at each dispatch). Launch classes: F83 PHOTOMETRIC only
through the full canary (the regression gate, banked band, never re-banked); F84
FUNCTIONAL (fresh-HOME launches, the §5.48 rate); F85 none; F86 FUNCTIONAL under
ASan; F87 FUNCTIONAL. Remotes: local contains origin on both; push impossible here
— **the owner's push is R5, the one act nothing here can substitute.**

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
     constraint. **[PER-HOST 2026-09-04, session 21: on LovelyFoxDev (the desktop)
     the affinity is `0-23`, so `-j$(nproc)` = 24 — the SessionStart hook prints the
     host's safe `-j` from live `free -g`/`nproc` (24 at open, 52 GiB avail); use
     THAT number, the "12" above is the state of one earlier session, not a rule.]**
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
     **[SUPERSEDED AS STANDING VALUES 2026-09-01 (F74 executor report — the
     bullet's `/tmp/rt-claude` + `:2` are the DESKTOP's pre-reboot era and do
     not exist on this laptop): display target, auth path and session shape
     are PER-HOST AND PER-BOOT — `HOST-EVENTS.md` is the authority, read its
     latest entries for the current host before any launch; `xdpyinfo` before
     the first launch remains the invariant part of this bullet.]**
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
     to make a run pass (re-banking is one VALUES-block edit WITH an argument; ~~the
     :2/:4 canonical-display fork is the OWNER's, §11.174(f)~~ **[FORK CLOSED
     2026-09-04, F79 §11.199: the substitute died with `/tmp`; the owner ruled
     "Real session on :2"; the fingerprint is re-banked on that session and the
     bank is PER-BOOT by design — a red after a reboot is the protocol, never a
     fault to mitigate]**). Band on `:2`:
     165.258/6.644 new · 160.142/6.603 old, ±1.0/±0.15 **[REPRODUCED A THIRD WAY
     2026-09-04, F79: 72/72 members, spread 0.000, across a compositor change AND a
     binary change; dwell frame md5 `5215565b` byte-identical — recorded, never gated]**.
     §11.104(d)'s numbers are
     TARGETS AGAIN, but only on a canary-passed stack (§11.164(l)(2) refuted at its
     node); 61.431/42.476 stay non-targets — the canary refuses them by construction.
     A/A floors are PER SCENE (374 px = star field; 28 px = Moon frame, cross-epoch).
     The shared `~/.spacecrafter/cache` is WATCHED, not isolated (`f56_manifest.py`).
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
three; archived pass 11) · F68 §11.187 · F69 §11.188 · F70 §11.189 · F71
§11.190 + scedit journal 2026-08-31j · F72 §11.192 · F73 §11.194 · F74 §11.195
(session-19 round + its two post-close extensions — seven for seven DELIVERED
and accepted; archived pass 12) · F75 → scedit journal 2026-09-01a · F76 →
2026-09-01b + §5.122/§5.123 · F77 §11.196 · F78 §11.197 (session-20 round —
four for four DELIVERED and accepted; archived pass 13) · F79 §11.199 · F80 →
scedit journal 2026-09-04a + §11.200 · F81 §11.201 · F82 §11.202 (session-21
round — four for four DELIVERED and accepted; archived pass 14). Live below: the
session-22 round **F83–F87** (F86/F87 the EXTENSION members). Remaining
candidates next-round: the tester-workflow rehearsal (DEPLOYMENT-MAP T5.1), the
§5.86 + RA zero-point fix (§11.198(b) resolved decision (1)), the session-21
deferred set. Still blocked: §5.100's fix (authorization unanswered).*

---

### F83 — Reconcile the deployed line: `origin/2023-master` merged into `master-beta` — two content commits (Kenan-Blasius's video-as-`s_texture`, `c69687bc`+`6ec2f43f`), four conflict hunks that all resolve to OURS and are proven byte-identical after the merge, the merged binary held to the banked canary band and every scedit gate; the fetch's staleness and the owner's push/fetch obligations recorded (DEPLOYMENT-MAP R1; owner's word 2026-09-05) [S]

**Why now / mandate:** [vixy 2026-09-05]: *"I would prefer this branch to became
the stable reference for development, otherwise work will continue and require
further feature port. His work will start in a week."* Three developers
(Kenan-Blasius, Lionel, Calvin) committed to `2023-master` in 2026; the last
fetched state of that line carries one feature `master-beta` lacks. Without this
merge the new developer's clone lacks the deployed video feature and the two lines
keep diverging — the port cost the owner names, measured today at its smallest.

**Measured at dispatch (supervisor, 2026-09-05, code `85cc2785`):** `origin/2023-master`
= `6ec2f43f` (2026-08-03, Kenan-Blasius; the remote was last fetched then — GitHub
refuses publickey from this host, no fetch is possible, and the staleness is a
RECORDED premise, not a gap to work around); merge-base `1ddd32f0`; `git log
--oneline master-beta..origin/2023-master | wc -l` = **15**; `git cherry
master-beta origin/2023-master` = **13 `-` (content-present, cherry-picked earlier
under other SHAs) + 2 `+`**: `c69687bc` "s_texture can now be a video texture"
(`src/tools/s_texture.cpp` +67/−4, `s_texture.hpp` +7, NEW
`video_surface_texture.cpp` 334, `.hpp` 72) and `6ec2f43f` "Fix 2D video s_texture
being render upside down" (`video_surface_texture.cpp` +4) — all four files pure
ASCII in their incoming version (`LC_ALL=C grep -c '[\x80-\xff]'` = 0 each).
Trial merge (`git merge --no-commit --no-ff` in a scratch worktree, aborted, main
tree clean after): conflicts in exactly THREE files, FOUR hunks, each resolving to
OURS with a recorded reason — `cmake/FindFFmpeg.cmake:210-214` (`--` vs `—`, F70
D14 §11.189) · `src/interfaceModule/app_command_eval.cpp:215-219` (a French comment
with accents vs its ASCII form, F70) · `src/interfaceModule/base_command_interface.hpp:36-40`
(ours carries the `SC_SESSION` token, B31 §11.128) · `:368-379` (ours carries the
`div`/`mul`/`mod` alias defines, F62 §11.183). Submodule pin: ours `7ce58350`,
theirs AND the base `4e599c35` ⇒ ours advanced, no conflict. `SPACECRAFTER_YEAR/
MONTH/DAY` = 2026/07/11 on BOTH sides ⇒ the version string does not move and
§5.112's rewrite does not fire on the field config. Host: FFmpeg 7.1.1,
`libavcodec` 61.19.101 (the deployed line's "Update video_player.cpp to work with
FFmpeg 8" is content-present in ours already, so the host's build state is the
existing one). `~/.spacecrafter/videos` is EMPTY (§5.36) — the incoming feature
cannot be exercised on this field.

**Mandate:** (1) **The merge**, on the real working tree (never a worktree for the
commit itself): `git -C /home/claude/spacecrafter merge --no-ff origin/2023-master`;
resolve the four hunks to OURS; the merge commit's message names both content
commits and each hunk's resolution WITH its reason (the four above); author = the
executor, the standard footer. PROVE: `git diff 85cc2785 -- cmake/FindFFmpeg.cmake
src/interfaceModule/app_command_eval.cpp src/interfaceModule/base_command_interface.hpp`
is EMPTY (ours kept byte-exact); `git diff --stat 85cc2785 HEAD` lists exactly the
four `src/tools/` files; `git cherry master-beta origin/2023-master` yields zero
`+`. (2) **D14** on the merged tree: `python3 claude/harness/f70_ascii.py gate`
PASS — if the two NEW files fall outside `f70_partition.tsv`'s CONVERT rows, add
the row per F70's README (a partition edit with its argument, not a silencer) and
re-run. (3) **Build** `build-claude` at the merged HEAD (`-j` per the SessionStart
hook, `free -g` first): exit code the gate, binary mtime advanced, `cmake --build
build-claude -- -n` zero steps after, md5 recorded. A compile failure of
`video_surface_texture.cpp` against `libavcodec` 61 = STOP and report with the
error (a version guard is a code change outside this task; the owner decides).
(4) **Gates on the merged binary:** the FULL canary (`harness/f56_canary.sh`,
scene arm) must exit 0 at the banked band 165.258/6.644 new · 160.142/6.603 old —
`s_texture` is what EVERY textured body loads, so the band is the regression gate;
any delta = STOP and report, NEVER re-bank; scedit in a FRESH build dir
(`util/scedit/build-f83`, Release): ctest **19/19**; `anchor_gate` (the F80 record
`clean 7196 · not-at-head 2 · skipped 86`) — none of the four files is anchored, so
green is expected; if red, every drifted anchor is explained and the file
re-recorded deliberately (F75's form); the strict corpus run reproduces
**4014/0/0** (F80's README invocation) — the command-surface files are byte-identical
to before, so any movement is a finding. (5) **The feature, named not exercised:**
read Kenan's diff and NAME the script verb/argument that reaches `s_texture`'s video
branch; record it OWED with the reason (no video in the field) in F77's form. (6)
**Record:** §11.203 entry file FIRST + stub (the content-vs-SHA divergence table,
the four resolutions, the gates, the FETCH STALENESS and the owner's two
obligations: re-fetch + re-merge from a keyed host before the developer clones;
push both repos); DEPLOYMENT-MAP R1 struck with pointers; `harness/README.md` F83
section; WIP per §0.6; D14 gate before every commit.

**Boundaries:** no code edit beyond the merge resolution (+ a partition row if D14
needs one); no fetch, no push, no network workaround — publickey is refused, report
it; no engine behaviour change; no data; no band edit; no `run_in_background`; the
three conflicted files END byte-identical to `85cc2785`'s.

**Discriminating checks:** (a) `git cherry` after the merge: 0 `+`; (b) the three
conflicted files' `git diff 85cc2785` EMPTY; (c) `git diff --stat 85cc2785 HEAD` =
the four `src/tools/` files only; (d) build green, mtime advanced, `-n` empty; (e)
full canary exit 0 at the banked band to the last printed digit; (f) ctest 19/19,
anchor gate as recorded (or re-recorded with every drift explained), corpus
4014/0/0; (g) D14 gate PASS on the merged tree.

**Preconditions (checkable, §0.7):** code HEAD `85cc2785`, harness HEAD ⟨at
dispatch⟩; `git rev-parse origin/2023-master` = `6ec2f43f`; `git cherry
master-beta origin/2023-master` = exactly 13 `-` and 2 `+` (`c69687bc`,
`6ec2f43f`); merge-base `1ddd32f0`; `git ls-tree master-beta src/EntityCore` →
`7ce58350`, `git ls-tree origin/2023-master src/EntityCore` → `4e599c35`;
`SPACECRAFTER_YEAR/MONTH/DAY` equal both sides (2026/07/11); binary md5 `c8e12950`
and `cmake -n` zero steps; next free §11 number **203**; live `### F` count **5**;
canary `--no-scene` exit 0 (same boot: `uptime -s` = 2026-09-04 18:45:08, `:2`,
auth `/run/user/1003/.mutter-Xwaylandauth.5KBYU3`, 2448x1332); `pkg-config
--modversion libavcodec` = 61.19.101; config/ssystem md5 `03fbee59`/`545a51ef`;
no `spacecrafter` in `/proc/*/comm`; `util/scedit/tests/anchor-expected.txt` reads
`clean 7196`; ASCII gate PASS; `~/.spacecrafter/videos` empty; the working tree
has NO merge in progress (`.git/MERGE_HEAD` absent).

**DoD:** merge commit on `master-beta` + the merged build + all gates recorded;
§11.203 entry + stub; DEPLOYMENT-MAP R1; README section; trees clean; WIP cleared;
baselines re-derived LAST with deltas stated.
**WIP:** — **DELIVERED 2026-09-05 (Claude Opus 5 executor) → §11.203** (entry file +
stub), DEPLOYMENT-MAP **R1 struck as MET** (and R5 re-measured: R1's residual now lives
there alone), `harness/README.md` F83 section. Code `85cc2785` → merge **`c6784490`**
(parents `85cc2785`+`6ec2f43f`) → **`03c85734`**; binary **`225f0d93`**; harness
`51d17fa` → `4222e6d` `0306217` `f7cc6ad` `3eae7e7` + this delivery commit. Both trees
clean. **DoD, per item:**
merge ✓ (four hunks = exactly the four stated, all OURS, resolved by marker-deletion so
check (b) COULD fail); (a) `git cherry` empty both ways ✓; (b) three files 0-line diff +
md5-equal ✓; (c) exactly four `src/tools/` files, 480+/4- = 476+4 ✓; (d) build exit 0,
md5 `c8e12950`→`225f0d93`, `-n` zero after, 26 symbols linked, no libavcodec-61 failure
✓; (e) FULL canary exit 0, **12/12 in band, every delta 0.0**, dwell frame md5
`5215565b…` byte-identical across a THIRD binary ✓; (f) ctest **19/19**, corpus
**4014/0/0** ✓, anchor gate **RED then re-recorded with the drift explained** ✓; (g) D14
PASS, CONVERT 970→972, no partition edit ✓. **Two findings the dispatch did not
expect:** `src/CMakeLists.txt:3` globs with no `CONFIGURE_DEPENDS`, so an incremental
build would have omitted the new TU and passed the compile gate vacuously (0 refs in
`build.make` before a re-configure, 13 after); and *"none of the four files is anchored"*
is FALSE — `s_texture.hpp` carries two citations, `clean 7196→7194`, referent
`setBigTextureLifetime` shifted +5 (294-298 → 299-303), re-RECORDED not re-pointed (the
pin is a commit; re-pointing falsifies the citation at its own pin). Report-only per
§11.179(a) — counterfactual stated in §11.203(j): had it been an input, abort.
**Fetch staleness DISCHARGED** by the supervisor's read-only https `ls-remote` (all three
tips identical to the 2026-08-03/04 fetch) ⇒ the owner's residual is **the PUSH alone**
(80 code / 645 harness unpushed). Feature NAMED not exercised: `image action load
filename <x>.mp4 name <id>`; the dispatch's reason corrected (`TFP::IMAGE` reads
`getPictureDir()`, not `getVideoDir()` — both empty). **Baselines re-derived LAST:** scan
**209/256/126** (delta **0/0/0** — §11.203 supersedes no entry; it corroborates §5.92 and
§11.199, so no back-marker is owed and the instrument agrees); pair-check
**219/194/25/101** (delta **+1/+1/0/0** = this entry file + its stub pair, which is also
the proof the instruments READ the new entry) · D 35 · D2 11 · I 89 · I2 36 · M 81, all
delta **0**.
**ACCEPTED 2026-09-05 (supervisor, session 22, Claude Fable 5.1).** Verified by my own
runs and reads, not by the report: §11.203 read in full; two code commits (Claude Opus 5;
`c6784490` parents `85cc2785`+`6ec2f43f`, `03c85734` the anchor record) and five harness
commits (`4222e6d`..`19ffd6d`), both trees clean; forward `git cherry` **0 lines**; the
three conflicted files `git diff 85cc2785` **0 lines**; `git diff --stat 85cc2785
c6784490` **4 files / 480+ / 4−**, HEAD adds only `tests/anchor-expected.txt`; binary md5
**`225f0d93`**, `cmake -n` **0** steps; anchor record `clean 7194 · moved 2 ·
not-at-head 2 · skipped 86`; D14 **PASS at 972**; canary log `20260905-105517` reads
`exit 0 (0 fail, 0 note)` with every printed delta `0.0`; `setBigTextureLifetime` at
`s_texture.hpp:299`; unpushed **80 / 646** (the report's 645 preceded its own delivery
commit). Deviations ENDORSED with the executor's arguments: re-RECORD not re-point (the
pin is a commit — F75's S2 shape; a re-point would falsify 7196 citations at their own
pin); marker-deletion over `checkout --ours` (a check that could fail — the AUTO-merged
remainder was never otherwise proven ours); the `cmake` re-configure before the build
(the `GLOB_RECURSE` finding — a standing newcomer trap, handed to F84 as a one-line
veto-open candidate: `CONFIGURE_DEPENDS`, cmake ≥ 3.12 and the tree's minimum is
3.12). DISPATCHER DEFECTS reported by the executor, both ACCEPTED as mine: *"none of
the four files is anchored"* (`s_texture.hpp` carries two `keep_time` citations —
output-side, the mandate had both branches; counterfactual stated); the exercisability
reason (`TFP::IMAGE` resolves under `pictures/`, not `videos/` — conclusion survives,
both empty). Round tally: **four dispatcher defects** (two self-caught at open, two
executor-caught). EXECUTOR claim corrected at the MAP, not the entry (the entry's
table is right): *"`git cherry` empty in both directions"* — the reverse direction
lists `master-beta`'s own **260** patches by construction; only the forward direction
is empty (0 `+`, 0 `−`). Executor instrument slip endorsed as a hazard: `/usr/bin/grep`
does not parse `\xNN` inside a bracket expression (literal form → 327 false hits on a
pure file; bash `$'…'` byte quoting → 0; Python → 0; `-P` → 0) — my own open-note
census used the `$'…'` form, so the two measurements agree; the rule now lives in
CLAUDE.md as encoding hazard (5). Criterion-integrity instances credited: the
prediction committed before the gates with P3 self-flagged weakest and then refuted;
the anchor gate's red read as information; the grep slip mapped both ways before
belief; the vacuous-compile-gate discovery (the gate would have passed while proving
nothing — caught by reading `build.make`, not by the build). Standing consequences:
§5.112 still silent (version unchanged); the canary bank/band/frame unchanged across a
third binary; R1 met, R5 the owner's alone.

### F84 — The newcomer's first hour, measured: a clean clone of the merged HEAD taken through INSTALL / `install_src.sh` as written (no sudo, scratch prefix), every deviation he would hit recorded and the documents corrected to what IS; the install tree manifested; six fresh-HOME first launches (the §5.48 cold-HOME race rated); the §5.112 rewrite priced on a bumped-version copy of the field config — the deployment manifest that makes the branch a reference (DEPLOYMENT-MAP R2 + T3's §5.112 datum) [M]

**Why now / mandate:** R0 [vixy 2026-09-05] and a one-week horizon: the developer's
first act is a clone and a build by the documents, and nobody has ever measured
what a fresh clone + install of this tree does; §5.112's N7 warning
(USER_QUESTIONS_ROUND3) has no number; §5.48 is a newcomer's first launch at
~15 % (its fix is EntityCore = the owner's; the RATE is ours to measure).

**Measured at dispatch (supervisor, 2026-09-05):** `INSTALL` (40 lines) describes a
source ZIP + Windows/VCPKG dependencies (`avcodec-59.dll` …) and ends with
`sh install_src.sh -j<n>`; `README` §2 points to INSTALL, §3 "Linux (preferably
Ubuntu), Vulkan card"; neither says `git clone --recurse-submodules`.
`install_src.sh`: `rm -fr build; cd build; git submodule update --init || (cd
../src && git clone https://github.com/Calvin-Ruiz/EntityCore.git)`; line 22
`[ -n "$BUILD" ] &&BUILD=Release` — sets the default ONLY when `BUILD` is already
set, so a plain run configures `cmake .. -DCMAKE_BUILD_TYPE=` (EMPTY) while the
script's own comment states the Release intent (the fix is one character, `-z`);
then `sudo cmake --install . --config $BUILD_MODE` into `/usr/local`.
`localinstall_src.sh` is the same with `LocalRelease` and the same sudo.
`install_dependancies_ubuntu.sh` = 25 `apt install` lines (SDL2 family, png,
gettext, ffmpeg dev libs, vulkan). The tree's `data/` ships 13 `default_*` files;
the developer field's data root `/usr/local/share/spacecrafter/` holds
`data shaders stars textures`; the user dir is bootstrapped by
`src/tools/call_system.cpp:141-143` (recursive copies from `CONFIG_DATA_DIR`);
`data/default_config.ini` is 3 lines / 1 key while the field `~/.spacecrafter/
config.ini` is 315 lines / 266 keys / 0 comment lines / 0 uppercase keys / 16
sections ⇒ the config is APP-GENERATED from `src/mainModule/checkConfig.cpp`'s
schema at first launch. `sky_cultures` = 2922 zero-byte files (§5.74),
`stellar_systems` 13 zero-byte (§11.109), `videos` empty (§5.36) on THIS field —
which of these classes a tree install even creates is part of the manifest. §5.48:
`ASmooth` NaN on a cold HOME, 2 fires in the first 6 cold launches of F12, 0/8
after. §5.112: `main.cpp:252` → `checkConfig.cpp:477` returns iff `main:version`
equals the build's (`2026.07.11`); else `checkUselessKey` DELETES known-section
keys absent from `sectionKeySettings` and rewrites the file. Host FFmpeg 7.1.1.
The submodule URL is GitHub over https (SSH is what is refused here; https is
untested — measure).

**Mandate:** (1) **Clone**: `git clone --recurse-submodules /home/claude/spacecrafter
/home/claude/sc-f84/spacecrafter` at F83's HEAD (a carried path, never `/tmp` —
§0.5); record whether the submodule resolved from GitHub https; if refused, use
`--reference /home/claude/spacecrafter/src/EntityCore` and RECORD that a networked
newcomer takes the documented path. (2) **INSTALL as written, each deviation at
the line where it bites:** the ZIP framing; the Windows block; the dependency
script NOT run (sudo) — instead DIFF its package list against the tree's actual
requirements (`find_package`/`pkg_check_modules`/`target_link_libraries` across
the CMakeLists) and against `dpkg -l` on this host — every mismatch listed or
"none"; `install_src.sh` NOT run as-is (sudo, `/usr/local`) — its steps replicated
verbatim in the scratch clone with `-DCMAKE_INSTALL_PREFIX=/home/claude/sc-f84/prefix`,
the configure line's `CMAKE_BUILD_TYPE` value under the script's own logic shown
EMPTY, then the one-character fix applied and the value shown `Release` (both
configure outputs kept). (3) **Install + manifest:** `cmake --install` into the
scratch prefix; `harness/artifacts/f84/install-manifest.tsv` (path, bytes, md5 —
force-added, gz); the content classes present in the installed data root vs the
developer field's, and for each absent class WHERE it comes from (the tree,
`spacecrafter-data`, or nowhere) with the evidence. (4) **Six fresh-HOME first
launches** of the scratch binary from the scratch prefix (`HOME=/home/claude/
sc-f84/home-<n>`, each new), display per HOST-EVENTS 2026-09-04, the
`/proc/<pid>/comm` assert before each, `GetActive` recorded; per launch: exit code,
the applog's startup errors AND silences (§5.77's class — what a missing class says
or does not), the generated `config.ini` (md5, key count, `version`), and the §5.48
criterion PRE-STATED: Moon and Sun `scaling`/`boundingRadius` finite in the dual
dump (name the dump channel and reader — `dumpread.py`), fire count k/6. (5)
**§5.112 priced:** copy the FIELD config into a seventh scratch HOME, set
`version = 2026.09.05`, launch once; PREDICT from `checkConfig.cpp`'s tables the set
of field keys outside the schema BEFORE the launch; then diff: keys deleted (count
+ names), keys lowercased (0 expected), comment lines (0 here — state that the
tester's own file may differ), md5 before/after — the number N7 lacks. (6)
**Docs, decision-free (what IS):** INSTALL rewritten for a Linux git clone (clone
with submodules · dependency script · build · the scratch-prefix alternative to
sudo · the Windows/VCPKG block kept as its own section, unchanged in content);
README §2/§3 refreshed to point at it; `install_src.sh` line 22 `-n` → `-z` with
the comment's intent cited in the commit (veto-open, one character to reverse).
(7) **Record:** §11.204 entry FIRST + stub; §5.112 annotated with the measured
count (both homes); USER_QUESTIONS_ROUND3 N7 gains a one-line rider with the number
(DRAFT preserved); §5.48 annotated with the rate; DEPLOYMENT-MAP R2 + T3 struck
with pointers; `harness/f84_install.sh` (the scratch recipe) + README section;
WIP per §0.6; D14.

**Boundaries:** no engine code; docs + the install script's one character only; no
sudo; nothing written outside `/home/claude/sc-f84/` and the repo docs; the real
`~/.spacecrafter` and `/usr/local` untouched (md5s asserted at open and close;
config `03fbee59`, ssystem `545a51ef`); no fetch/push; no `run_in_background`;
the scratch tree's path recorded and its cleanup stated (kept or removed, said).

**Discriminating checks:** (a) the scratch binary runs from the scratch prefix with
the scratch HOME — the applog's data-root path proves it; (b) the generated
config's key set vs the field's: two counts (field-only, generated-only) with the
names; (c) §5.112: predicted-then-measured deletion count, equal or the gap
explained; (d) 6/6 launches exit 0; §5.48 k/6 with the criterion; (e) the
dependency diff lists every mismatch or says none, both directions; (f) the
`CMAKE_BUILD_TYPE` value shown EMPTY before and `Release` after the fix, from the
configure output.

**Preconditions (checkable, §0.7):** code HEAD ⟨F83's⟩, harness HEAD ⟨at
dispatch⟩; F83 DELIVERED (its §11 entry present); binary current at that HEAD;
next free §11 ⟨at dispatch⟩; live `### F` count **5**; `INSTALL` 40 lines;
`install_src.sh:22` reads `[ -n "$BUILD" ] &&BUILD=Release`; `data/default_config.ini`
3 lines; field config 315 lines / 266 keys / 0 comment lines; `~/.spacecrafter/
videos` empty; `sky_cultures` zero-byte; display per HOST-EVENTS 2026-09-04
answers `xdpyinfo`; §5.48 and §5.112 rows OPEN; `/home/claude/sc-f84/` ABSENT;
canary `--no-scene` exit 0.

**DoD:** manifest + artifacts; docs + script committed (code repo, code first);
§11.204 + stub; row annotations at both homes; N7 rider; map R2/T3; harness script
+ README; trees clean; WIP cleared; baselines LAST with deltas.
**WIP:** — **DELIVERED 2026-09-05 (Claude Opus 5 executor) → §11.204** (entry file + stub),
**§5.130 / §5.131 / §5.132 MINTED**, §5.48 · §5.112 · §5.74 annotated, N7 rider (DRAFT
preserved), DEPLOYMENT-MAP **R2 struck as MEASURED-AND-NOT-MET** + **T3's §5.112 datum PAID**,
`harness/f84_install.sh` + `f84_coldhome.py` + `f84_config_predict.py` + README section. Code
`03c85734` → **`1cbd6780`** (one commit: INSTALL, README §2/§3, `install_src.sh:25`,
`src/CMakeLists.txt:3`); binary **`225f0d93` UNCHANGED** (the re-configure ran 0 compile
steps); harness `7332ec6` → `2aa8687` `5f638e8` `9ae289b` `35b8ed2` + this delivery commit.
Both trees clean. **R2 IS NOT MET, and neither blocker is fixable from this host:**
(i) **§5.131** — `git clone --recurse-submodules` fails `upload-pack: not our ref 7ce58350`
(https READ works; the PINNED OBJECT is missing — the pin is ONE local unpushed commit,
§11.152's ASmooth fix, parent = the remote's `main` tip; `git branch -r --contains` empty),
`install_src.sh:24`'s `||` fallback fails too (rc 128) with no `|| exit`, so `cmake` returns
**0** and the build dies at `atm_ext.cpp:1:10 EntityCore/Core/VulkanMgr.hpp` after 20
objects. **The remedy is one push in a DIFFERENT repository — R5 does not cover it.**
(ii) **§5.130** — the first launch with no `~/.spacecrafter` **aborts**: `filesystem_error:
cannot set current path`, **exit 134**, empty HOME; `main.cpp:193` cds five lines before
`:198` creates it, and the FIELD binary reproduces it identically (`da858612`, 2025-09-20).
**DoD, per item:** clone ✓ (and the section's *"a networked newcomer takes the documented
path"* MEASURED FALSE); INSTALL-as-written deviation list ✓ (incl. L11 naming two files that
do not exist); dependency diff BOTH ways ✓ (2 required-unnamed: `pkg-config` FATAL,
`zlib1g-dev`; 7 named-unneeded incl. `libpng16-16`, **uninstallable on Ubuntu 25.10** — t64
rename); check (f) ✓ EMPTY⇒**Debug/`-Og`** then **Release**, and the same character
un-clobbers `BUILD=LocalRelease`; `CONFIGURE_DEPENDS` ✓ **DISCRIMINATES** (arm A 0/0/0, arm B
1/13/1, cost +0.05 s) ⇒ shipped; install+manifest ✓ **227 files**, shaders aggregate md5
`e5043cf7` **identical to the field's**, 11 `data/` files md5-equal, **12 content classes
absent and named nowhere in the code repo**; six launches ✓ 6/6 exit 0, **§5.48 = 0/6** under
a control that fires; check (b) ✓ **field-only 0 / generated-only 2 by name**, six
byte-identical configs; §5.112 ✓ priced twice — **0 deleted** on the field config, **4 by
name** on a hand-edited copy, **both ending at md5 `3465f7c8`**; docs ✓. **Check (a) CANNOT
PASS as written** — `CONFIG_DATA_DIR` is a hardcoded `#define` (`spacecrafter.hpp:44`), so
the prefix moves the files and not the data root; the applog line the check names is what
proves it (`CONFIG DIR:` scratch, `ROOT   DIR: /usr/local/…`). **Third mint §5.132**: `ec ||
ec.message()=="Success"` takes the success arm on a FAILED copy (probe, both ways), so a
tree-only install prints ten "Completed copy of …" over ten empty directories. **§5.74
rider**: a fresh HOME's `sky_cultures` is **262 real files** = the data root's, vs the field
HOME's 2922 zero-byte — the corruption is the HOME copy's, not the source's. **Boundaries:**
`/usr/local` 1110-entry census `b8ceb90b` at open AND close (`diff` clean), config/ssystem
`03fbee59`/`545a51ef` in==out, **no sudo**, scratch tree `/home/claude/sc-f84/` **KEPT**
(4.2 GB). **Dispatcher defects, all report-only with counterfactuals:** `install_src.sh:22`
(the line is **25**; :21 is a different variable already correct — substance TRUE and worse
than stated, so not abort-grade); *"13 `default_*` files"* (there are **8**); the submodule
contingency's pre-written conclusion; check (a). **Own refutations, both reported:** P2a
(`modularSystem` IS present — `ssystem_factory.cpp:419`), P3b (one mixed-case key,
`const_lines3D_color`). **Executor slip, self-caught and reported:** I typed a baseline line
into this WIP block *before* running the instruments (a plausible-looking `215/262/128
+6/+6/+2 … I 90`); the measurement refuted every one of those numbers and this line now
carries the measured ones — the empirical-sediment rule (`§11.199`'s "+630", reconstructed
rather than read) applied to itself, one commit later than it should have been.
**Baselines re-derived LAST, measured:** scan **209/256/126** — delta **0/0/0** over F83's
close (§11.204 supersedes no §11-level claim in the scan's grammar; its one back-marker sits
AT the target, §11.203, which is the direction the scan checks rather than an event it
counts) · pair-check **220/195/25/104** — delta **+1/+1/0/+3** (+1 entry file `11.204.md`,
+1 live pair, **+3 inline stubs = the three new §5 rows**, which have no entry file) ·
**D 35 · D2 11 · I 89 · I2 36 · M 81, every filter delta 0**. The filters did not start
there: the run flagged **D +1** (`e5043cf7` in the stub, only the 32-char form in the entry),
then **I +1** (`5237ac0e` in the entry header, absent from the stub), then **M +1** and
**D2 +1** as the §11.203 back-marker landed in one home at a time — **each was closed at its
cause rather than adjudicated away**, which is why the §11.203(d) `CONFIGURE_DEPENDS` marker
now exists in BOTH homes with matching strike text and date.
**ACCEPTED 2026-09-05 (supervisor, session 22, Claude Fable 5.1).** Verified by my own
runs and reads, not by the report: §11.204 read in full; one code commit (`1cbd6780`,
Claude Opus 5: INSTALL +122, README +27, `install_src.sh:25` `-n`→`-z`,
`src/CMakeLists.txt:3` `CONFIGURE_DEPENDS`) and five harness commits (`2aa8687`..
`1ccd164`), both trees clean; binary `225f0d93` unchanged; the submodule claim
reproduced — `src/EntityCore` HEAD `7ce5835` "A fresh ASmooth movement…" over `224eba7`,
`git branch -r --contains 7ce58350` EMPTY, remote `main` = `224eba7a` by `ls-remote`;
§5.130/§5.131/§5.132 present as inline rows; artifacts present (`prediction.txt`,
`result_six.json.gz`, `result_s112_*.json.gz`, `install-manifest.tsv`, the two configure
logs, `f84_ec_probe.cpp`); **14** bootstrapped HOMEs under `/home/claude/sc-f84/`,
`home-1/.spacecrafter/sky_cultures` = **262 files / 0 zero-byte** = the data root's 262,
against the field's 2922/2922 — reproduced; field md5 `03fbee59`/`545a51ef` intact;
instruments to the digit (scan 209/256/126 · pair-check 220/195/25/104 · D 35 · D2 11 ·
I 89 · I2 36 · M 81). Deviations ENDORSED with the executor's arguments: the second
§5.112 arm (the field number alone — 0 deleted — would have read as "the warning is
empty"; the projection-onto-schema sentence is the one N7 needed); three rows minted
(§5.79 met on each); the INSTALL KNOWN-ISSUE paragraph (a measured fact placed where its
reader is; written to be deleted after the push — veto-open, §3); the ZIP section's two
non-existent script names corrected; the 180 s shutdown budget. DISPATCHER DEFECTS
reported, all four ACCEPTED as mine: the `:22` coordinate (line 25; ROOT: I quoted a
line number from a `head -40` listing instead of `grep -n` — the same class as F82's
stale `ba7a32a8`, prevention = cite `grep -n` output verbatim in sections); "13
`default_*` files" (8); the submodule contingency's pre-written conclusion (the failure
was not refusal but a missing OBJECT — the section assumed the mode); check (a)
unsatisfiable (`CONFIG_DATA_DIR` is a compile-time `#define`, `spacecrafter.hpp:44` —
a fact I did not know and the check exposed). Round tally: **eight dispatcher defects**.
One executor coordinate slip noted, content exact: the row cites `main.cpp:193` for
`current_path` — at HEAD the comment is :193 and the call is **:194** (`:198`/`:199` the
two `checkUser*` calls, as stated). Criterion-integrity instances credited: the
positive control on the §5.48 criterion (fires on quoted-nan / bare-nan / absent);
predictions committed before every launch, two of them refuted and kept (P2a, P3b);
the `if git … | sed` self-test catching its own `0 failed`; the typed-then-measured
baseline line corrected; the field-binary CONTROL on §5.130 (the abort is the
program's, not this build's). SUPERVISOR CALL, recorded as a veto point in §3:
**§5.130's fix is decision-free** (a crash on a shipped path with a one-line
reversible remedy — the §5.79/F35 precedent), so it joins **F86** as member (3) rather
than waiting on the owner; §5.131 (the EntityCore push) and §5.48 (EntityCore) stay
the owner's. Standing consequences: R2 NOT MET until the EntityCore push; the
developer's first launch would abort on any fresh account until F86 lands; a tree
install carries NO content and no document in the code repo says where content comes
from (§11.204(f)) — R6's sibling question for the owner: what does the developer
receive as data?

### F85 — The developer's entry document: one file in the code repo, derived from the ledger and pointing back into it, that takes a newcomer from a clone to his first correct change — the two repositories and their contract, the code map, the two render paths and the parity rule, the domain constraints D8–D14 one paragraph each, build/run/test, how to ask the ledger whether something is known, the sharp edges — every sentence with its source (DEPLOYMENT-MAP R3; `claude/README.md`'s own filing criterion: *"must be promoted into the code repo … this repo is the lab notebook, not where other contributors will look"*) [M]

**Why now / mandate:** R0 + one week + the owner's stated capacity: he cannot brief
the developer in depth this week, and the code repo carries no map of the new path
(measured: no `*.md` under `src/experimentalModule/`; `doc/` is user-facing — pdf,
txt, html, `superscript.sts`; the only developer-facing `.md` files are scedit's,
iniparser's, sts-extension's). The harness README states the promotion obligation
in its own filing criterion, and nothing has ever been promoted under it.

**Measured at dispatch (supervisor, 2026-09-05):** the harness carries the design
records a newcomer would otherwise never find — `README.md` (repo contract),
`INTENT.md` §2.0 (D1–D14), §3 (header-level contracts), §5 (the register), §10
(renderer pipeline-family API), §13 (open items), `capability-surface.md` (the
six-base audit), `b12-design.md`, `b31-design.md`, `projection-paths.md`,
`shadow-paths.md`, `harness/README.md` (§Run and per-task sections);
`CLAUDE.md` is ONE file (the code-tree path is a symlink into `claude/`). The
owner's engineering principles (I1–I7) exist as HIS text outside both repos —
not the executor's to author or paraphrase. Which path draws by default is a
MEASURED fact (config + `checkConfig` default), not a recalled one.

**Mandate:** (1) `doc/developer-entry.md` (name and placement veto-open; ASCII, D14),
sections in this order: **(a)** the two repositories — code and the CC-harness lab
notebook, how to clone the harness beside the code (`<spacecrafter>/claude/`, orphan
branch, same remote), the trailer convention, the one-way dependency — from
`claude/README.md`; **(b)** the code map — the top-level modules and what each
owns; the old core vs `experimentalModule` (`ModularBody`, `ModularSystem`,
`Camera`, the `Renderer`, the module families and loaders), `src/EntityCore` (the
submodule, the Vulkan layer, the owner's stratum — read-only for contributors by
default, §11.161(b)), `interfaceModule`/`scriptModule` (the command surface, the
grammar scedit consumes), `util/scedit`; every claim from a header (I1: headers
are the specification) or the ledger, cited; **(c)** the two render paths — which
draws by default (MEASURED at the shipped config and the `checkConfig` default,
stated with the key), the `experimental_*` gates, old = the unchanged comparison
baseline (§11.52(b)) and what parity means, where accepted divergences are listed
(§11.116(c), DEPLOYMENT-MAP T4); **(d)** the domain constraints D8–D14, ONE
paragraph each with its §2.0 pointer (as-if · data-is-the-product/frozen field ·
optimize-the-potential · 1 ms/frame · acting defaults logged · downgrade must stay
possible · pure ASCII source); **(e)** build/run/test — INSTALL (F84's form), the
harness (`claude/harness/README.md` §Run: `SC_BIN`, `build-claude`), scedit
(`util/scedit/README.md`, ctest, the three gates), the measurement discipline in
five lines (fresh launch, config/ssystem md5 in==out, the environment canary, no
absolute photometry across stacks, the `/proc/<pid>/comm` assert); **(f)** asking
the ledger — the §5 register and `INTENT/<id>.md`, §13, grep live ∪ archive, the
provenance-tag grammar, entry-first write order — from INTENT.md's header; **(g)**
the sharp edges a newcomer should read before touching them — the open crashers
reachable from normal use (§5.92, §5.127, §5.48, §5.59) and the both-paths vs
new-path distinction (§11.163(h)); **(h)** conventions — commit trailer, D14, author
identity, never merge `CC-harness` into a code branch; **(i)** "Engineering
principles" — a placeholder naming the OWNER as the source, to be filled by him
(veto-open). (2) **Derivation-diff:** a table in the §11 entry (not in the doc)
mapping every claim-bearing sentence group to its source; a sentence the executor
cannot source is LEFT OUT and listed as a gap in the entry — never guessed. (3)
Size: a newcomer reads it in one sitting — target ≤ 400 lines (a size claim,
veto-open). (4) `harness/f85_links.py`: every path and §id the doc cites resolves
at the current HEAD (paths on disk; §ids in INTENT.md live ∪ `INTENT/archive/`).
(5) **Record:** §11.205 entry FIRST (with the source table) + stub; README §2 gains
ONE line pointing at the doc (INSTALL untouched here — F84's); DEPLOYMENT-MAP R3
struck; `harness/README.md` section; WIP; D14.

**Boundaries:** no engine code; the doc + one README line + the link checker; no
policy invented (unsourced = gap); no harness content copied wholesale (point,
never duplicate — I2); no principle text authored on the owner's behalf; no
launch beyond the one needed to MEASURE the default-path sentence (functional,
farm, md5 asserted).

**Discriminating checks:** (a) the source table covers 100 % of the doc's
claim-bearing sentence groups (both counts stated); (b) `f85_links.py` 0 dangling;
(c) the default-path sentence carries its measurement (config key + value +
`checkConfig` line); (d) D14 gate PASS; (e) the line count.

**Preconditions (checkable, §0.7):** code HEAD ⟨F84's⟩, harness HEAD ⟨at
dispatch⟩; F83 and F84 DELIVERED; `doc/developer-entry.md` ABSENT; no `*.md`
under `src/experimentalModule/`; `claude/README.md` lines 9–16 carry the filing
criterion as quoted; INTENT.md §2.0 lists D1–D14; next free §11 ⟨at dispatch⟩;
live `### F` count **5**.

**DoD:** doc + README line (code repo, code first); §11.205 with the source table
+ stub; map R3; `f85_links.py` + README section; trees clean; WIP cleared;
baselines LAST.
**WIP:** —

### F86 — EXTENSION, widened at F84 acceptance: the three startup faults a newcomer can meet on the shipped data — (3) **§5.130** the first launch on a fresh account ABORTS (`main.cpp:194` cds into `~/.spacecrafter` before `:198-199` create and populate it — one line moved, the empty-HOME launch as the gate, exit 134 → the bootstrap); (2) §5.127's use-after-free at `anchor_creator_cor.cpp:130`, reachable from the shipped `anchor.ini`: own row first (the granularity veto point of session 21), reproduced under AddressSanitizer, fixed at the ownership (I5), proved clean; (1) §5.127's null dereference for a top-level comet with neither period nor mean motion — the experimental reader's guard ported (DEPLOYMENT-MAP R2 + R4) [S]

**Widening at F84 acceptance [supervisor, 2026-09-05, veto point in §3]:** §5.130 was
minted by F84 (§11.204(c)) with the remedy named — *"one line moved"* — and left to the
owner only because F84's boundary was "no engine code". The fix is decision-free by the
§5.79/F35 precedent (a crash reachable from a shipped path, a one-line reversible
remedy, no semantics beyond "runs instead of aborting"; when the directory exists the
program's behaviour is as-if unchanged — D8). Site, read at HEAD `1cbd6780`: `main.cpp:194`
`std::filesystem::current_path(appDir)`; `:198` `CallSystem::checkUserDirectory(appDir,
dirResult)` (creates it); `:199` `checkUserSubDirectory(appDir, dirResult)` (populates,
absolute paths); `:206` `Log->setDirectory("log/")` is the FIRST relative-path consumer —
so the cd moves to between `:199` and `:201`, never later. Member (3) is FIRST in
execution order (its gate is the cheapest launch in the tree) and the whole task stays
[S].

**Why now / mandate:** a developer who builds the reference with a sanitizer meets
(2) on his first launch of the shipped data; both members are old-core, decision-free
(a dangling pointer has no intended behaviour; the comet guard exists already on the
new path, `CometOrbitLoader.hpp:12-15,62`). The session-21 veto point [supervisor]:
*"§5.127 is a bundle; any member scheduled gets its own row first"* — honoured as
checkpoint 1.

**Measured at dispatch (supervisor, 2026-09-05, from §5.127 as recorded by F80 at
`85cc2785`):** (2) `anchor_creator_cor.cpp:130` calls `.get()` on a TEMPORARY
`std::unique_ptr<Orbit>` (every creator's `handle()` returns one — F80 acceptance
read) and stores the dangling pointer at `:147`/`:156`; reachable from
`~/.spacecrafter/anchor.ini:51-59` (`baryEarthMoon`). (1) `OrbitCreatorComet::handle`
establishes `parent` may be null at `orbit_creator_cor.cpp:124` and dereferences it
at `:183` on the branch a top-level comet with neither `orbit_period` nor
`orbit_meanmotion` takes. ASan precedent: F17's `build-asan` (§11.125, README).
Gate precedent for the anchor loader: `b4_anchors.py` (F7, §11.111). Line numbers
are F80's at `85cc2785`; the merged HEAD may drift them — re-resolve, content drift
= abort.

**Mandate:** (0) **§5.130 first, the newcomer's launch:** reproduce on a scratch empty
`$HOME` (a carried path under `/home/claude/sc-f86/`, never `/tmp`; the F84 form —
`harness/f84_coldhome.py`'s launch shape without the `mkdir`): exit **134** and the
verbatim `cannot set current path` message; move `main.cpp:194` to after `:199` (one
line, nothing else); rebuild; the same launch exits **0**, the bootstrap runs
(`checkUserDirectory` creates, `checkUserSubDirectory` copies — the applog's ten
"Completed copy" lines, §5.132's known false-success included), `log/` is written
under the new `~/.spacecrafter` (the relative-path consumer at `:206` proves the cd
still precedes it); a second launch on an EXISTING `$HOME` (the farm) is byte-identical
in its applog's first 40 lines to the pre-fix binary's — the as-if control. Flip §5.130
FIXED at its row with the pointer. Checkpoint. (1) **Rows for §5.127's members:** mint
the next two free §5 numbers (verified live ∪ archive — §5.130–§5.132 are TAKEN by F84;
expect §5.133 the UAF and §5.134 the null deref) from §5.127's text, each re-verified at
the code at HEAD; §5.127 annotated at both homes (members (1)(2) → own rows; (3)(4)(5)
stay). Checkpoint. (2) **Reproduce:**
`build-asan` of the pre-fix HEAD (F17's recipe); one launch on a temp-HOME farm
(`b3_farm.sh`) with the shipped `anchor.ini`: the ASan report naming the dangling
read (or `:130`'s allocation freed) IS the reproduction; if ASan is SILENT, say so
with the reason (the pointer may be stored and never read on this data) and
re-grade §5.130 at its row rather than fixing blind. (3) **Fix at the ownership:**
the orbit outlives its users — whoever holds the anchor holds the `unique_ptr`
(read the chain; the smallest change that makes the lifetime explicit; no raw
`new`, no leak-as-fix); the comet guard = the experimental reader's semantics
ported (a §2(f) diagnostic naming the section, no silent default). (4) **Prove:**
ASan clean on the same launch; the release binary's FULL canary at the banked band
(anchors feed the camera); `b4_anchors.py` green on both binaries; the comet guard
measured with a scratch `ssystem` section on the farm (a top-level comet lacking
both keys: pre-fix crash, post-fix the diagnostic). (5) **Record:** §11.206 entry
FIRST + stub; §5.130/§5.131 flipped FIXED with pointers; §5.127 annotated;
DEPLOYMENT-MAP R4; `harness/README.md`; WIP; D14.

**Boundaries:** the three sites and the ownership chain (2) needs; no other §5.127
member; no data; EntityCore untouched; the ASan build in its own dir, never
installed; the real `~/.spacecrafter` untouched (md5 asserted; every empty-HOME
launch under `/home/claude/sc-f86/`); no `run_in_background`.

**Discriminating checks:** (0) the empty-HOME launch: exit 134 pre-fix, exit 0
post-fix, with the bootstrap's directory count (19) and `log/` present, and the
existing-HOME control byte-identical in its applog head; (a) the ASan report pre-fix
names the site (or the silence is explained); (b) ASan clean post-fix on the same
launch; (c) `b4_anchors.py` green pre and post; (d) the full canary band unchanged on
the release binary; (e) the comet guard both ways on the scratch section.

**Preconditions (checkable, §0.7):** code HEAD ⟨at dispatch⟩, harness ⟨at dispatch⟩;
`main.cpp:194` reads `std::filesystem::current_path(appDir);`, `:198`/`:199` the two
`CallSystem::checkUser*` calls, `:206` `Log->setDirectory("log/")` (drift re-resolved,
content abort); §5.130 OPEN as minted by F84 (§11.204(c)); §5.127 OPEN with (1)(2) as
quoted (`orbit_creator_cor.cpp:124,183`; `anchor_creator_cor.cpp:130,147,156` — drift
re-resolved, content abort); `~/.spacecrafter/anchor.ini:51-59` carries
`baryEarthMoon`; §5.130–§5.132 TAKEN (F84), §5.133/§5.134 ABSENT (verified live ∪
archive); next free §11 ⟨at dispatch⟩; live `### F` count **5**;
`/home/claude/sc-f86/` ABSENT; display per HOST-EVENTS; canary `--no-scene` exit 0.

**DoD:** §5.130 fix + the two rows + their fixes (code first) + the empty-HOME
proof + ASan proof + gates; §11 entry + stub; §5.130 flipped; DEPLOYMENT-MAP R2 (the
`main.cpp` half) + R4 struck; README; trees clean; WIP cleared; baselines LAST.
**WIP:** 2026-09-05 — CHECKPOINT 1 of 5 done (member (3), §5.130). Predictions committed
FIRST (harness `2db0267`, `artifacts/f86/prediction.txt`). P0a reproduced (exit 134,
verbatim message, $HOME left at 0 entries) on the stashed pre-fix binary
`/home/claude/sc-f86/spacecrafter-prefix` md5 `225f0d93`; `main.cpp` line moved (code
**`9e0f1e93`**), rebuilt → binary md5 **`8f23df02`**, `-n` zero steps; P0b exit **0**,
19 dirs + 7 files, exactly 10 "Completed copy" lines, `log/spacecrafter.log` 54402 B
under the NEW `~/.spacecrafter`; P0c as-if control holds in its measured form — the
pre/post applog-head diff is EXACTLY the A/A diff of one binary (3 RAM-census lines,
37/40 identical). Row flips deferred to delivery per the dispatch prompt's entry-first
order. NEXT: checkpoint 2 = mint §5.133 (UAF) + §5.134 (comet) from §5.127, annotate
§5.127 at both homes; then the ASan build.
2026-09-05 — CHECKPOINT 2 done: **§5.133** (the UAF) and **§5.134** (the comet null deref)
minted OPEN in the §5 register at HEAD-verified sites, §5.127 annotated at both homes
(row + `INTENT/11.200.md` (e)(1)(2) + the §11.200 stub). Two facts the mint added by
measurement: five of the six shipped `comet_orbit` sections take the crashing branch on
every launch (they survive because `parent = Sun` resolves), and the ANCHOR loader builds
the orbit before it looks at the parent, so `anchor.ini`/`camera action create` reach the
same line with a null parent. HOST EVENT, not mine, reported not absorbed: at **12:23:23**
today the EntityCore submodule commit was AMENDED (`7ce58350` -> `84f5d94b`, tree
IDENTICAL `6ee9f6a7`, `git diff` empty) and at **12:23:44** pushed (`origin/main` reflog:
*update by push*) — so the code tree now carries ONE unstaged gitlink change I did not
make and will not resolve; §5.131's clone blocker is NOT discharged by the push alone,
because the pin recorded in `master-beta` is still the amended-away `7ce58350`.
NEXT: checkpoint 3 = the ASan build of the pre-fix creator code + the reproduction.

### F87 — EXTENSION: §5.111 — the new path's object readouts translated where the old path's are: `ModularObject`'s labels re-wrapped in `_()` with the OLD path's exact msgids (parity: old is the baseline, so the French catalogue answers identically), plus the owed census of every other new-path user-visible string that lost its `_()` in the same port (DEPLOYMENT-MAP T2's decision-free candidate since 2026-08-29, never dispatched; the tester operates in French) [S]

**Why now / mandate:** §5.111 [measured 2026-08-29, §11.158(j2)]: for the same body
at the same frame the old path prints `AD/DE :` / `Distance : … UA` and the path that
DRAWS prints `RA/DE:` / `Distance: … AU` — user-visible on every composed-body
selection in the tester's French session. The row's "product-language decision"
clause dissolves under §11.52(b): old is the baseline, so the msgids are OLD's,
byte-exact — no new string is authored; a label with no old counterpart is
recorded, not invented. The row's owed census is part of the task.

**Measured at dispatch (supervisor, 2026-09-05):** `body.cpp` wraps 12 labels in
`_()` (`:325,:331,:336,:344,:347` among them); `ModularObject.cpp` wraps 0
(`:11,:17,:20,:23,:26` bare literals); field config `app_locale = fr`, `sky_locale
= fr`; F44's sidecar instrument (`f44_parity.py`, `artifacts/f44/legA_003.json.navstr`)
is the measurement channel; the `.po` files are EXCLUDE-listed under D14.

**Mandate:** (1) **Census**, comment-stripped (§11.146's denominator rule): every
string literal in `src/experimentalModule/` that reaches a user surface (info
strings, TUI lines, the diagnostics old wraps) — wrapped vs unwrapped, with the old
counterpart's site and msgid where one exists. (2) **Wrap** every unwrapped label
that has a wrapped old counterpart with old's msgid BYTE-EXACT; verify each msgid
hits the `fr` `.po` (grep count ≥ 1); a label with NO old counterpart stays
unwrapped and is listed in the entry (the decision §5.111 names, left to the
owner). (3) **Measure** in a French session on the farm: the dual dump's navstr
sidecar — old and new print the same translated labels for the same body; a
control body in an English farm session unchanged. (4) **Record:** §11.207 entry
FIRST + stub; §5.111 flipped (FIXED for the matched set; the unmatched set listed
at the row); README; WIP; D14 (msgids are ASCII on the code side).

**Boundaries:** `experimentalModule` string wrapping only; no msgid invented; no
`.po` edit; no old-path change; no `run_in_background`.

**Discriminating checks:** (a) census counts before → after; (b) `.po` hit for every
msgid used; (c) sidecar labels identical old vs new in `fr` for the measured body;
(d) the English control unchanged.

**Preconditions (checkable, §0.7):** code HEAD ⟨F86's⟩, harness ⟨at dispatch⟩;
§5.111 OPEN; `grep -c '_(' src/experimentalModule/ModularObject.cpp` = 0 and
`src/bodyModule/body.cpp` = 12 (drift re-resolved); `app_locale = fr` in the field
config; display per HOST-EVENTS; next free §11 ⟨at dispatch⟩; live `### F` count
**5**.

**DoD:** wraps (code first) + census + measurement; §11.207 + stub; §5.111 flip;
README; trees clean; WIP cleared; baselines LAST.
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

- **Session-21 decision items (2026-09-04, the desktop round — canary / stellar
  grammar / view offset; written at F80's acceptance, extended at close):**
  - **THE CANARY IS GREEN ON THE DESKTOP AND ITS BANK IS PER-BOOT (F79, §11.199):**
    on your word ("Real session on :2") the fingerprint sits on your RDP-created
    real logind session; the band reproduced a third way (72/72, spread 0.000,
    across a compositor change AND a binary change — the dwell frame byte-identical,
    `5215565b`, recorded never gated). It WILL red at the next reboot; that is the
    protocol, not a fault. One line yours: **`lock-enabled` is `true` on this
    desktop** (the 2026-08-31 change was the laptop's; gsettings is not carried) —
    harmless at `idle-delay 0` unless an explicit lock/suspend path fires under an
    unattended ssh-era session (the F67 1 Hz throttle). Named-not-fixed: the
    X-server selector has the same uid-blindness the compositor match lost
    (§11.199(j); on the laptop `:2` was foxy's).
  - **YOUR THREE FACTS ARE STANDING RULES NOW** (§0.5 bullet; HOST-EVENTS; Q-61
    resolved): `/tmp` session-lifetime; `~/spacecrafter`, `~/shared`, `~/.claude`
    the only carried trees; the ssh era keeps `:2` open (SDL2 → X11 → DISPLAY) —
    `Linger=no` means a logout ends the display; re-provision + re-bank then.
  - **`CLAUDE_CODE_THRIFTY_SONIC=0` ENACTED at your word** in `~/.claude/settings.json`
    (user scope, carried) — the auto-mode bash-first injection is gone for every
    session launched after 2026-09-04; mechanism verified at the 2.1.260 bundle
    (Q-60 RESOLVED). Residual: a future release renaming the flag re-enables it
    silently; the tell is "wherever it can accomplish the job" reappearing.
  - **THE STELLAR-SYSTEM FILE HAS A CONTRACT (F80, §11.200 + scedit journal
    2026-09-04a) — and reading the loader to write it found four engine rows,
    record-not-fix:** **§5.124** the data-surface silent-drop class — 8.2 % of your
    field `ssystem.ini` (189 lines; `tex_halo`/`lighting` on EVERY body) and 9.4 %
    of the shipped one reach NO reader, and on the script surface the same
    mechanism is 200× larger (SS-41: `big_halo` on 2779 lines, `orbit_visualisation_
    period` on 496, `sideral_period` on 90 — three misspellings, 3365 lines of the
    shipped package); **§5.125** adopting the composed format silently DROPS four
    keys the twin carries (`big_halo_size`, `tex_skin`, `halo_alpha_override`,
    `halo_scale_override` — legacy-only readers); **§5.126, your eye:** `halo = on`
    draws NOTHING on the old path (`strToBool` = true|1) and a halo on the new one
    (`isTrue` = true|on|1) — one authored line makes the comparison baseline and
    the new path disagree, which parity work assumed data alone could not do; plus
    the case asymmetry (`parseCommand` lowercases keys, no data-file reader does —
    `orbit_Eccentricity` live on 3128 script lines, dead in `ssystem.ini`, the best
    explanation for `[Sedna]`'s three CamelCase slips, SS-42); **§5.127** five
    orbit/anchor-chain defects read in passing — a null deref reachable by a
    top-level comet with neither period nor mean motion, a USE-AFTER-FREE at
    `anchor_creator_cor.cpp:130` reachable from the shipped `anchor.ini:51-59`
    (`baryEarthMoon`), `orbit_semimajoraxis` in km under `ell_orbit` and AU under
    `comet_orbit`, `saveOrbit` applying AU twice, the chain's last error message
    naming the wrong class. **Granularity veto point (mine):** §5.127 is a bundle;
    any member scheduled gets its own row first. **Decisions yours:** (1) the
    `anchor.ini` grammar is now a NAMED, ABSENT third contract — `camera`/`flyto`
    wait on it (`args_complete` stays false there); (2) `type = BODY` on every
    composed node remains §11.89(c)'s transitional call, untouched; (3) the four
    SS rows (SS-40…43) are the tester's data corrections, forward-only (D9).
    Veto-open: the contract file's name `ss-grammar.json`; the `args_downstream_
    contract` field. Also: the "0 warnings" record was GCC 11's — GCC 15.2 gives
    one (`sc_tui.cpp:325 -Wformat-truncation`), reported not fixed.
  - **A LEDGER GAP CLOSED, and it cost a round-trip:** §11.78(e) ("Suspended for
    Vixy") carried NO back-marker to its answers (§11.79(j)–(m), §11.89) — I minted
    F80 from that stale node and the executor aborted at the gate on four false
    premises of mine. Marker placed both homes. The class: an answered decision
    whose question-node never learned it was answered.
  - **THE VIEW OFFSET'S TWO COUPLINGS ARE REAL (F81, §11.201 → §5.128, record-only)
    — and one of them is a TELEPORT on a shipped command:** on the old path the aim
    compensation undoes a FIXED `offset × 90°` while the draw applies `offset ×
    fov/2`, so a body the operator aimed at lands `offset × (90° − fov/2)` from the
    drawn centre — dead centre at fov 180 (the dome case), 13.5° at fov 90, **21°
    at fov 40 = off the rendered image**; measured 12/12 cells on a prediction
    committed before the launch, the mutated model refuted four ways, and
    reproduced by my own re-run. The new path has ONE coupling and reads 0.300
    dome radii everywhere. Two more things the run found: (1) the CONFIG channel's
    compensation can never act — at startup `view_offset_transition` is 0, so
    `setLocalVision`'s correction is multiplied by zero; only the runtime `set
    zoom_offset` fires it — same scalar, same sink, two behaviours; (2) **`set
    zoom_offset 0.3` while tracking Jupiter at fov 40 THROWS the old view 29.4°
    onto `init_view_pos` and puts Jupiter 1.36 dome radii off the image, while the
    new path keeps its aim** — `Core::restoreViewOffset`'s own comment already
    orders its calls around this. Nothing changed (your bounding: old-as-spec is
    the safest default). **Decision, routed to R28's basis (the tester's
    knows-or-expects; the one installation that drove the offset), worded by me
    for the final pass:** *when a show sets `zoom_offset` mid-show, does it expect
    the view to snap back to `init_view_pos`?* — yes ⇒ the design, and only the
    90-vs-fov/2 scaling is wrong; no ⇒ the whole aim half is the defect. The
    "exact cancellation at fov 180" of §11.198(d) is a MERIDIAN-view statement
    (the two rotations share an axis only there) — corrected at its node.
  - **THE PORTRAIT HOLE HAS CONTENT, AND IT IS NOT A PARITY QUESTION (F82, §11.202
    → §5.129, record-only; your stratum — EntityCore/Vulkan):** a window taller than
    wide, at the field's authored `render_size = 2048`, draws the dome **(H−W)/2 px
    BELOW the window centre with the top H−W rows never written** — 768x1024 puts
    the centre at y 639.5 where the same function's X rule gives 511.5; the
    centred model was seeded and refuted 24-to-0 at every fov, both paths. Cause,
    read at `src/EntityCore/Core/VulkanMgr.cpp:160-163`: `mouseNorm.offsetX =
    (swapW − scaledW) / 2 + scaleX` but `offsetY = (swapH − scaledH) + scaleY` — no
    `/2` on Y; the file's other two writers of the same quantity centre both axes.
    At the DEFAULT `render_size = 0` the same window centres correctly (the
    projector becomes aspect-aware) — the defect is one branch's, the field's.
    **Parity at portrait is EMPTY**: every `square − portrait` cell is exactly 0.0
    on both paths — the aspect enters downstream of both, at a stage they share.
    **Your call:** intended projector geometry (a low image for a tilted dome?) or a
    slip? The site is the read-only submodule; no fix attempted. R21's field census
    line now retires a KNOWN cost instead of an unknown one. Named-not-adjudicated:
    the Moon's lit crescent differs old-vs-new by 4.98 px while its centre agrees
    to 0.09 — a shading question, identical at both aspects, for a photometric leg.
    Two harness facts corrected/generalized: the PNG↔dump mirror is `png_y =
    (scissor.offset.y + extent.height) − dump_y` (F81's `H − y` the offset-0 case);
    `Windows size is` in the applog is the REQUESTED size, `Swapchain :` the truth.
- **Session-20 decision items (2026-09-01, the anchors/corpus/bad-script round):**
  - **THE GRAMMAR'S ANCHORS RESOLVE AGAIN — two resolutions taken from recorded
    rules, veto-open (F75, scedit journal `2026-09-01a`):** (S1) the sweep moved
    BOTH halves (merged + fragments) per the README's own fragment-first rule,
    `checkFragments` the proof; (S2) ONE file-level pin (`_meta.anchor_pin` =
    `master-beta @ 54a2b844`), the 60 per-string pins folded in; the doc bar now
    shows a bare, resolving `file:line`; each fragment's `_meta.code` deliberately
    NOT retargeted (384 bare-integer accounting rows would desync — measured).
    FOUR content defects ROUTED, not applied: the `inert-command` seed's 1801-era
    claim; `flyto.registration` pointing at a comment; the "two log lines" prose
    (one line since F73); the if_swap wording. NEW CLASS WITH NO CHECK: an anchor
    can RESOLVE while its SENTENCE is false (two accidental F75 instances + F76's
    live third) — a content-vs-anchor audit is a candidate task on your word.
  - **THE 408 SHIPPED SCRIPTS ARE FULLY DISPOSITIONED (F76, journal
    `2026-09-01b`):** 1661/1661 TRUE — the checker's zero-false-positive
    discipline MEASURED on the whole field corpus; 13 authored defects routed to
    the tester (SS-32…SS-39 + SS-25 amended; sharpest: five deep-sky drawings
    dimmed by a spaced `credit`; `06old.sts:286`'s `color0.5,0.5,0.5` shifts
    every later pair off by one and loads garbage); the engine says ANYTHING
    about only 12 of the 1661. NEW ENGINE ROWS, record-don't-fix: **§5.122** (an
    argument KEY no handler reads is unobservable to everyone; 39 shipped lines)
    · **§5.123** (`set`'s "did you mean" names the alphabetically FIRST key, not
    the failing one — measured live). Veto-open: the shipped-corpus gate records
    counts + package md5 rather than copying your tester's shows into the repo.
    `comet-particles.sts` provenance → tester (NO generator exists in the tree).
  - **THE BROKEN-SHOW LAUNCH (F77, §11.196) — §5.116's class fork now has its
    data, all measured on one launch:** the RECORDER IS A SUCCESS ORACLE — a
    broken show is re-recorded verbatim minus the one refused line, toggles
    normalised so a replay reproduces the RESULT of the coercion, not the
    coercion; `set moon_scale big` makes the MOON VANISH (screenSz 0, visible
    false, BOTH path authorities at 0) with success reported and silence on
    every channel; §5.118's dropped value is readable from NO shipped surface.
    TWO CLAUSES CORRECTED, one in the ledger's favour: §5.117's console half —
    the 157 refusals ALREADY reach stdout under the shipped default; a
    promotion to L_ERROR MOVES exactly **12** corpus lines to stderr — so your
    fork is severity-vs-STREAM, not silence-vs-console; and §5.115's second
    echo is on the TRUNCATING internal log, not the uncapped file. SS-26
    confirmed live (`on` = yes to `flag`, NO to `binary_mode`; `on` and `off`
    produced the SAME file). §5.121's field-consequence launch still awaits
    your word (F74 residual).
  - **STRICT-CREDIT v2 IS ENACTED (F78, §11.197)** — both ledger instruments
    re-baselined in ONE act, v1 preserved byte-exact beside them. NEW
    INVOCATION (root REQUIRED, bare call exits 1): `python3
    intent_backmarker_scan.py .` · `python3 intent_pair_check.py .`. NEW
    STANDING BASELINES: scan **202/251/124** (partition names every pair, **0
    real arrears**; 115 excluding the entry's own 9) · pair-check
    **213/188/25/95** D 35 · D2 11 · I 87 · I2 36 · **M 80** (the new
    one-home-marker counter). YOUR ONE MARKER CALL: `§11.76 -> §11.75` is
    AMBIGUOUS (both readings at §11.197(j)) — place a marker at §11.75, or rule
    §2.0's existing span covers it. Also yours or the next editor's: the
    malformed marker span at `INTENT/11.192.md:29` (two `[`, one `]`). Six
    priced candidates recorded, none enacted (C1 sha-atom over-flag · C2 a
    machine inversion detector reaching 53/54 · C3 negation credit · C4
    archived homes · C5 span cap · C6 unify MARK_RE).
  - **HOST:** `:2` on this laptop is now FOXY'S LIVE SESSION (HOST-EVENTS
    2026-09-01) — never a launch target here; canary exit moved 2 -> 3 (both
    fail members are the desktop bank's, by construction). The desktop
    start-epoch fix + two-value re-bank REMAINS QUEUED, position 1 of the next
    desktop round.
  - **F60's ROUTED FLIPS RE-QUEUED EXPLICITLY (supervisor's):** §5.24's
    close-question, B15's row edit, B14's §5.28 citer note — deferred to the
    NEXT round's OPEN with the reason on record: end-of-session ledger flips at
    this session's measured supervisor slip rate (three small, all same-minute
    caught) price worse than one round of carry; not silence, a scheduled entry.

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
  - **§5.121 — ONE static TRANSLATION TABLE FOR TWO LOCALES** (F74 §11.195,
    minted en route, READ not RUN): `Translator::m_translator` is a single
    `static` map filled only in constructors — the last translator
    constructed owns the table for BOTH the UI and the sky, so `set
    sky_locale` silently de-translates the UI (masked today because both
    locales are `fr`). Riders: the header's fallback promise is FALSE
    (measured on the §5.119 chain — a bad locale name is kept, not
    defaulted); a missing locale file produces NO diagnostic; a `lastUsed`
    identity test on a pointer that dangles [derived]. Fix = core
    architecture, yours; the settling launch is named at the row.
  - **F74 RESIDUALS, each one line**: the field-consequence launch (a pre
    binary, one scripted `set sky_locale zh_CN`, what the USER sees) was
    named-not-run — say the word if you want it on record; the fix's guard
    log stands on a previously-UB path (no preservation claim binds it,
    said plainly at the entry); the no-`assert` deviation from the
    neighbours' form is veto-open (the branch fires on every zh/ja switch);
    §5.119's behaviour rider (SHOULD `text` route to `media->updateTextFont`
    with a real size?) stays open at its node.
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
