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
**MID-SESSION EVENTS (recorded as they happened; `date`-measured from 13:40 on — every
earlier "~HH:MM" in this session's prompts was an ESTIMATE, ~1 h fast, dispatcher
defect):** (1) 12:23:23 — the EntityCore submodule commit `7ce58350` was amended
(tree-identical `84f5d94b`) and pushed to `origin/main` FROM THIS CHECKOUT by a hand
with a key this session lacks (writer set: the owner or his other session); the pin was
bumped on `master-beta` as a supervisor act (`32af2efb`, veto point §3) and a clone probe
passed ⇒ §5.131 DISCHARGED. (2) 13:33:40 — `USER_QUESTIONS_ROUND3.md` STAGED with
sixteen round-3 replies by the same external hand; 13:36 — the F85 executor ABORTED at
the §0.7 gate on that staged file (correct on both its grounds; 0 mutations); 13:38:30 —
the owner COMMITTED it (`c5be42b`, "User question replies round 3"). (3) 13:4x — F84's
measured N7 paragraph, dropped by the paste (a copy predating it), RESTORED beside the
reply; the file flipped to ANSWERED; **F88 minted** (the propagation, ledger-only);
F85 re-dispatched with the replies as facts (live `### F` 5 → 6). (4) [vixy, in-session]:
the replies are the MAIN TESTER's (Lionel RUIZ), transmitted by the owner; on his word
the commit's author was amended to the tester's code-history identity at the
quiescent point after F85's delivery: **`c5be42b` → `6ffb017`** (`Lionel RUIZ
<lionel.ruiz@live.fr>`, committer = this session), the four later harness commits
replayed byte-identical — **SHA MAP: `741b4a8→367f2d1` · `c4686fc→4fb22f1` ·
`72aa116→0ee6ab7` · `c0efd3c→190ced4`** (any citation of the old SHAs in this file,
§11.206 or the F85 prompt resolves through this map; nothing was on the remote).
Picks after the events: **F83 ✓ → F84 ✓ → F86 ✓ → F85 ✓ → F88** (the propagation
outranks F87 now: sixteen decisions the map waited on), F87 if health permits.
**Round outcome (session 22 close, 2026-09-05 — the close commit `7852ce5` reads 14:52:16 by its own clock; this line first said "~15:00 `date`-measured" and was NOT measured, the twenty-third dispatcher defect and the one that makes the class visible: a label that CLAIMS measurement without the command beside it is worse than an estimate, because it disarms the reader's check):** F83 → **§11.203**
(the deployed line in; 4 keep-ours hunks proven byte-identical; canary band to the digit
on a third binary; the anchor gate red and right) · F84 → **§11.204** + §5.130–§5.132 (R2
measured NOT MET: the submodule pin on no remote, the first launch aborting; INSTALL
rewritten; `-z` + `CONFIGURE_DEPENDS`; §5.112 priced 0/4; §5.48 0/6; a tree install ships
no content) · F86 → **§11.205** + §5.133/§5.134 (§5.130 fixed 134→0; the anchor UAF under
ASan pre/post; the comet guard both ways; one gate red, held open) · F85 → **§11.206**
after ONE §0.7 ABORT on an environment premise (the owner's in-flight staging — the
gate's design case, 0 mutations) (`doc/developer-entry.md`, INSTALL −26, `f85_links.py`
0 dangling) · F88 → **§11.207** + §5.135 (twenty replies propagated, A43 closed, T1.2/T1.4
closed, the map re-cut) — **five for five delivered AND supervisor-verified same session**
(every delivery re-verified by my own runs: cherry/diff/build/gates, the clone probe, the
bootstrapped HOMEs, the ASan artifacts, the link checker, the instruments); F87 CARRIED
(context budget — seven executor runs). SUPERVISOR ACTS: the pin bump `32af2efb`; the
author amend `c5be42b → 6ffb017` (+ SHA map); the N7 restore; CLAUDE.md hazard (5); §5.130
folded into F86. Code `85cc2785 → a2fd3c5b` (9 executor commits + 1 supervisor: the
merge, the anchor record, the docs + two one-liners, the three startup fixes, the entry
document — NO engine behaviour designed, three crash sites closed); harness `34b6cae →`
this close. OWNER EVENTS IN-SESSION: the EntityCore push (12:23), the replies committed
(13:38), the provenance ruling. SUPERVISOR TALLY: **twenty-two dispatcher defects**
(root: coordinates/counts/times written from memory of a listing — prevention: `grep -n`
and `date` in the same command as the claim; sections re-resolved at every widening)
+ two instrument slips (the "empty in both directions" phrasing corrected at the map;
the 14:20 stamp); EXECUTOR criterion-integrity instances: **≥ 20** (the vacuous compile
gate caught by reading `build.make`; the anchor gate's red read as information; the
§0.7 abort on the staged file; the A/A run first; the predicted-silent ASan; the
checker shown able to fail; the prediction's class refuted and kept; the source table
counted both ways; the twenty-vs-sixteen count). BASELINES AT CLOSE (v2): scan
**211/261/131** · pair-check **223/198/25/107** D 35 · D2 11 · I 89 · I2 36 · M 81 —
every delta over the open attributed per task in its acceptance. Archival pass 15
(update-s21 + F83–F88, live `### F` 6 → 0) DEFERRED to the next open, recorded here.
NEXT-ROUND QUEUE, in order: (1) archival pass 15 at open; (2) the `b4_anchors` P7
discriminator (instrument; twinkle pinned on both binaries, the two windows' lit sets
compared); (3) **F87** §5.111 (S, minted, carried); (4) the decision-free engine list
§11.207(g): §5.86+§5.19 (M, first) · §5.98 (S) · §5.21 (S–M) · §5.66+§5.71 (M) · §5.115
(S–M) · A15's residual (S); (5) the T5.1 rehearsal (M — doubles as the developer's smoke
suite); (6) riders: §5.59's `sender` arm, scedit README `:43`, the `[parallel-script]`
withdrawal question; (7) owner items per §3. Remotes: 86 code / 674 harness unpushed at
close (measured); a key exists on this machine for the session that pushed EntityCore —
the supervisor never pushes.

---

**Update [Claude Fable 5.1 2026-09-05, supervising session 23 — LovelyFoxDev, the
READINESS round: the P7 red / the French labels / the rehearsal]:** trigger = the §0b
verbatim line PLUS an in-line transmission [vixy 2026-09-05, verbatim]: *"The
master-beta will became the reference and get renamed main once ready."* — this
answers the policy half of DEPLOYMENT-MAP **R6** (the branch becomes `main` by RENAME,
the others are not redirected) and names a gate, *"ready"*, which the map's critical
path already defines as **the T5.1 rehearsal + a T5.2 soak** (*"gate the word
'ready'"*, both re-cuts). No question asked of the owner this session (his stated
capacity this week, 2026-09-05); the rename itself is his act and is not prepared
beyond a measured footprint (F92). Warm-up (every value `date`-measured 15:16–15:20,
the command beside the claim — Q-67's rule): both trees CLEAN at open, code
`a2fd3c5b` / harness `3a5ce8f` (one commit past the session-22 close `7852ce5`: the
stamp correction); definition-drift assert MATCH (`a5a54d94`); binary `2815d182`
(F86's) current — `cmake -n` 0 steps, no `src/` file newer; next free §11 number
**208** (live ∪ archive, `max+1`); §5 max 135; live `### F` count **6 → 1** by
**archival pass 15** at OPEN (update-s21 + F83/F84/F85/F88/F86, 974 lines incl. the
pass-13/14-shape seam tidy, manifest `2026-09-05-pass15`, pre-md5 `673303e0`
reproduced in-process AND from disk, commit `6045ea4`; **F87 KEPT LIVE — carried,
not delivered**) **→ 5** by the mints below. Same boot as sessions 21–22 (`uptime -s`
2026-09-04 18:45:08), `:2` 2448x1332 under `.5KBYU3`, canary `--no-scene` **exit 0**
(30 members, artifacts `f56/canary/20260905-151723`); config/ssystem md5 pristine
(`03fbee59`/`545a51ef`); no `spacecrafter` in `/proc/*/comm`; ASCII gate not re-run
at open (no code moved since the last PASS at 972). **HOST DELTA:** RAM **20 GiB
available of 59** (session 22 opened at 50) — writer set enumerated: foxy's `java`
29.6 GB RSS + his desktop (firefox, pulsar, Discord, remmina), i.e. the OWNER's hand
on his own session, not ours; the SessionStart hook prints safe `-j13`; every build
this round runs `free -g` first and drops to `-j6` under 16 GiB (§0.5). Instrument
baselines at open: scan **211/261/131** · pair-check **223/198/25/107** D 35 · D2 11 ·
I 89 · I2 36 · M 81 — to the digit of the session-22 close. Unpushed **86 code / 676
harness** (measured `rev-list --count`; the 674 at close preceded its own close
commit and the stamp correction). **DISPATCHER DEFECTS FOUND AT OPEN, both session
22's (mine) and both corrected before any dispatch:** (1) the close note's *"archival
pass 15 (update-s21 + F83–F88, live `### F` 6 → 0)"* — F87 was never delivered; the
pass moved five sections, not six; (2) F87's *"`body.cpp` wraps 12 labels"* and its
precondition *"= 12"* — measured **15** lines match `_(` at HEAD and `git log
85cc2785..HEAD -- src/bodyModule/body.cpp` is EMPTY, so the 12 was written from
memory at the mint (Q-67's class, a §0.7 abort had it been dispatched as written);
corrected at the section with the command. QUEUE CONSUMPTION (session-22 close, in
order): (1) pass 15 — DONE; (2) the `b4_anchors` P7 discriminator → **F89** (S,
instrument; the red is on the REFERENCE binary and the developer will run the
harness); (3) **F87** — carried, corrected, dispatched this round; (4) §11.207(g)'s
first engine candidate §5.86+§5.19 → **F91** (M); (5) the T5.1 rehearsal → **F90**
(M) — PROMOTED above (4) by today's line: it is the map's own "ready" gate and it
doubles as the developer's smoke suite; (6) the riders (§5.59's `sender` arm, scedit
README `:43`, the `[parallel-script]` question) — not minted, carried; (7) owner
items — §3. NEW from today's line: the rename's footprint → **F92** (S, record-only:
189 harness hits / 118 files + 9 code files measured at open, classified before
anyone renames). **T5.2 (the soak) NOT minted**: a multi-hour campaign needs a design
under the no-`run_in_background` rule (foreground polls, the F19 stall instrument as
hang detector) — next round's candidate, said here so the gate's second half is not
silent. Picks: **F89 → F87 → F90**, then F91 and F92 if health permits (session 22
ran seven executors and closed on context; three is the sweet spot, §0b.2).
**[16:38, at F89's acceptance: F89 found the P7 red is the PRODUCT's — the pre-fix binary
drew its sky 76° from the scene it composed and the gate was green on that; the reference
binary is RIGHT and the instrument is wrong (§11.208(i)). Its two repairs are decision-free
harness work ⇒ **F93** minted (S); picks now **F89 ✓ → F87 → F93 → F90**, then F91/F92.]**
Deliveries: all to the parent (§11.208+, refreshed at each dispatch). Launch classes:
F89 FUNCTIONAL (star-field pixel sets under a frozen clock — no banked-band claim, the
full canary is not required); F87 FUNCTIONAL (farm, French session, sidecar); F90
FUNCTIONAL (the farm, three runs); F91 FUNCTIONAL (RA/DE parity through the sidecar);
F92 none. Remotes: local contains origin on both; push impossible from this session
(publickey) — the owner's push is R5, unchanged.

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
round — four for four DELIVERED and accepted; archived pass 14) · F83 §11.203 ·
F84 §11.204 · F86 §11.205 · F85 §11.206 · F88 §11.207 (session-22 round — five
for five DELIVERED and accepted; archived pass 15). Live below: **F87** (carried
from session 22) + the session-23 round **F89–F92** (F91/F92 the EXTENSION
members). Remaining candidates next-round: the T5.2 soak (the "ready" gate's second
half, needs a foreground-poll design), the §11.207(g) tail (§5.98 · §5.21 ·
§5.66+§5.71 · §5.115 · A15's residual), the riders. Still blocked: §5.100's fix
(authorization unanswered).*

---

### F87 — EXTENSION: §5.111 — the new path's object readouts translated where the old path's are: `ModularObject`'s labels re-wrapped in `_()` with the OLD path's exact msgids (parity: old is the baseline, so the French catalogue answers identically), plus the owed census of every other new-path user-visible string that lost its `_()` in the same port (DEPLOYMENT-MAP T2's decision-free candidate since 2026-08-29, never dispatched; the tester operates in French) [S]

**Why now / mandate:** §5.111 [measured 2026-08-29, §11.158(j2)]: for the same body
at the same frame the old path prints `AD/DE :` / `Distance : … UA` and the path that
DRAWS prints `RA/DE:` / `Distance: … AU` — user-visible on every composed-body
selection in the tester's French session. The row's "product-language decision"
clause dissolves under §11.52(b): old is the baseline, so the msgids are OLD's,
byte-exact — no new string is authored; a label with no old counterpart is
recorded, not invented. The row's owed census is part of the task.

**Measured at dispatch (supervisor, 2026-09-05):** `body.cpp` ~~wraps 12 labels in
`_()`~~ **[CORRECTED at the session-23 open, 2026-09-05 15:2x: `grep -c '_('
src/bodyModule/body.cpp` = 15 LINES (18 `_()` sites, some lines carry two: `:347`,
`:363-364`, `:371`), file untouched since the mint (`git log 85cc2785..HEAD --
src/bodyModule/body.cpp` empty) — the 12 was written from memory, dispatcher
defect]** (`:325,:331,:336,:344,:347` among them; the nav fields `:425-427`
`SA`/`GHA`/`LHA` and `:433` `Az/Alt/coA` are wrapped too); `ModularObject.cpp` wraps 0
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
`src/bodyModule/body.cpp` = **15** (corrected 2026-09-05, session 23; drift re-resolved); `app_locale = fr` in the field
config; display per HOST-EVENTS; next free §11 ⟨at dispatch⟩; live `### F` count
**5**.

**DoD:** wraps (code first) + census + measurement; §11.207 + stub; §5.111 flip;
README; trees clean; WIP cleared; baselines LAST.
**WIP:** 2026-09-05 17:0x — CKPT1 (harness only, no code yet): §0.7 gate PASSED on every
input-side premise (HEADs, F count 6, §11.209 free, §5.111 OPEN, `_(` 0/15, `fr` locale +
md5 `03fbee59`/`545a51ef`, binary `2815d182` + 0 build steps, display `:2` 2448x1332, no
running instance). TWO output-side dispatcher defects, reported not aborted: (1) NO `F44`
section exists in `harness/README.md` (0 hits) — read `f44_parity.py` + `artifacts/f44/`
instead; (2) **there is no `.po` anywhere in either repo or the field** — `_()` is
`Translator::translateUTF8` (`translator.cpp:45`), a map loaded from
`~/.spacecrafter/language/fr.txt` (`"key";"value"`, IDENTITY FALLBACK), which is
D9-frozen field data; the msgid check is therefore EXACT-KEY, not `grep -c`. Instrument
`harness/f87_census.py` (self-test PASS) lands the census: 2087 literals, READOUT 30 (all
`ModularObject.cpp`), `_()`-wrapped **0**; 14 sites have a byte-exact old msgid, of which
**5 msgids reach fr.txt and 5 fall through by identity — old's do too**. NEXT: the 14
wraps in `ModularObject.cpp` (code first), rebuild, then the fr/en farm measurement.
2026-09-05 17:1x — CKPT2: code `1d839b9d` (14 wraps, D14 gate PASS), rebuilt `-j13`,
binary md5 `2815d182` → **`407b3d1d`**, `cmake -n` 0 steps. Census 0 → **14** wrapped, an
exact bijection with old's 14 literal msgid sites (`body.cpp:331-435`); the 16 unwrapped
readout literals are exactly the ones old leaves bare. `f87_labels.py` (self-test PASS)
carries the PRE-FIX baseline measured offline on `artifacts/f44/legA_003.json.navstr.gz`
— 90 bodies, G1 540 red, G2 540 red, OLD-missing **0**, names 68 same/22 differ, U+00A0
**0** — and SIX PREDICTIONS committed before any launch. NEXT: the fr farm launch, then
the en control launch, then `--compare-new` against the pre-fix artifact.
2026-09-05 17:0x — CKPT3+4: canary `--no-scene` exit 0; FOUR farm launches, all exit 0,
md5 in==out on the real `$HOME` each time, concurrent probe 0. **ALL EIGHT PREDICTIONS
CONFIRMED, NONE REFUTED.** fr: G1 540→**0**, G2 540→**0** over 90 bodies, U+00A0 0→**90**,
name channel unchanged at 68/22/22. English control (`app=en,sky=en`): G1 0, G2 0, names
90/0, U+00A0 0, and every NEW `inf`+`nav` string **BYTE-IDENTICAL** to the pre-fix F44
artifact for all 90 bodies. The control's FIRST attempt found a defect instead of passing:
`app_locale=en, sky_locale=fr` printed FRENCH — `Translator::m_translator` is ONE STATIC
map (`translator.hpp:85`) shared by app + sky translators, last loader wins, sky loads
last. 2×2 factorial, byte-exact within columns: (fr,fr)≡(en,fr) md5 `7e14412d`;
(en,en)≡(fr,en) md5 `4e56907c`. `app_locale` is INERT. NEXT: §11.209 + stub, §5.111 flip,
§11.158(j2) back-mark, README, instruments LAST.

### F89 — The `b4_anchors` P7 screen-witness red DISCRIMINATED: the one gate that reds on the reference binary (green ×3 pre-§5.133, red ×2 post) gets its cause named — the twinkle-`rand()` candidate tested the way the two failed experiments could not (b4's own frozen-clock scene, twinkle OFF on both binaries, the two P7 windows' lit-pixel SETS diffed, predictions committed first), the control's margin read from the five F86 runs before any launch, every `rand()` consumer in `src/` enumerated — instrument fix if the instrument, §5.133's row if the product (§11.205(g)(h); session-22 queue position 2; the developer will run this harness) [S, instrument]

**Why now / mandate:** the reference branch carries a harness gate that is RED on its own
binary with the cause unconfirmed (§11.205(g)): *"green ×2 pre-fix + on a `main.cpp`-only
binary, red ×2 on the fixed binary"*. A newcomer who runs `b4_anchors_run.sh` on his clone
meets that red first. Bounded already, by measurement: the Moon's flux-weighted centroid
is **1324.2740 / 1767.6721, mass 76290, in ALL FIVE runs**; every dumped number in
`b4_result.json` is identical; the frames differ in **~19030 px>8** between the two
binary groups against a **730–860 px** within-group floor; no ±2 px translation improves
the match ⇒ the camera did not move, and *"what differs is which faint stars sit above
the threshold"*. Candidate, NOT finding: `hip_star_mgr.cpp:640-643`, `1 -
twinkle_amount*rand()/RAND_MAX` over a stream nothing seeds (`src/` holds 0 `srand(`).
Two experiments failed and are kept with their causes (§11.205(h)): **P8** used the
shipped startup view (3.30 of 4.19 Mpx lit by atmosphere/landscape, no star field);
**P8b** repaired the scene but let the clock run — its A/A control (23502 px>8) came in
LARGER than its A/B, voiding both arms. The instrument note it left: *"comparing
star-field pixels across launches needs the date pinned AND the frame index pinned (or
twinkle off); b4's scene pins the date, which is why its floor is 773 and not 23000."*

**Measured at dispatch (supervisor, 2026-09-05 15:1x, code `a2fd3c5b`):** b4's scene
sends `timerate rate 0` BEFORE the epoch (`b4_anchors.py:185`) and `date jday J0`
(`:186`), advances/returns by `DT_ORBIT`/`DT_SPIN` (`:205-227`); P7 prints at `:422`; the
control's text at `:477` (*"SCREEN WITNESS control: the OTHER date's position is darker
in each shot"*), its in-run rationale at `:441`. F86's five runs are committed:
`harness/artifacts/f86/b4_{pre,pre2,post,post2,mainonly}.json.gz`. Binaries: the
FIXED one is `build-claude/src/spacecrafter` (`2815d182`, code `a4a7c226` = HEAD's
engine state); the PRE-fix state is code **`9e0f1e93`** (the `main.cpp`-only commit —
its binary landed in the pre-fix GROUP, §11.205(g)); F86's scratch tree
`/home/claude/sc-f86/` holds outputs (`out-b4-mainonly`, `b4_farm`, `build-asan`) — whether
a pre-fix RELEASE binary survives there is for the executor to measure (`md5sum` +
`git describe` of its build dir); if none, build `9e0f1e93` in `/home/claude/sc-f89/
build-pre` (carried path, `-j` per §0.5 after `free -g` — 20 GiB avail at this open).
`grep -rc 'srand(' src/ | grep -v ':0'` → nothing (re-measure). Twinkle's command-surface
handle is NOT stated here — find it in `util/scedit/grammar/sc-grammar.json` and
`app_command_interface.cpp`, cite the line.

**Mandate:** (0) **Read before launching:** from the five F86 `b4_*.json.gz`, extract the
TWO scalars P7's control compares in each run (the "darker than" pair) and state the
MARGIN each run passed or failed by — a control that passes by a hair on the pre-fix
binary is a finding by itself, and its numbers bound H1/H2 before any run. (1) **Random
channels:** enumerate every `rand(`/`random(`/`drand48(`/`std::mt19937`/`uniform_*`
consumer in `src/` (EntityCore included, read-only) with its site and whether it reaches
the DRAW of the star field; state whether twinkle is the ONLY per-frame random channel
on the P7 windows. (2) **Predictions, committed to `harness/artifacts/f89/prediction.txt`
BEFORE any launch, each with the number that refutes it:** **H1 (twinkle)**: with twinkle
OFF on BOTH binaries, under b4's frozen clock, the between-binary P7-window difference
collapses to the within-binary floor AND the control is GREEN on both — refuted if the
between-binary difference stays within an order of magnitude of 19030 with twinkle off.
**H2 (the pre-fix binary drew from corrupt memory — §5.133's read)**: the pre-fix
binary's camera/anchor state differs from the fixed one below the JSON's printed
precision — refuted if the full-precision dump (`dumpread.py` channel, every float at
`%.17g`) is byte-identical across the two binaries for `camera.mat` and the anchor's
position; confirmed if any digit differs. **H3 (instrument at the margin)**: the control's
margin on the pre-fix runs is within the within-group floor's equivalent, so a
floor-sized change flips it — decided by (0). (3) **Runs** on a `b3_farm.sh` HOME
(anchors from the shipped `anchor.ini`; `/proc/<pid>/comm` assert before each; GetActive
recorded): b4's scene, twinkle OFF, **two runs per binary** (A/A within each), the P7
windows' lit-pixel SETS (px>8 masks) saved and diffed pairwise — within-binary and
between-binary counts stated; then the same with twinkle ON, one run per binary, so the
twinkle-on floor is re-measured in the same session as the twinkle-off one. (4) **Verdict
and act:** H1 confirmed ⇒ the INSTRUMENT is at fault: `b4_anchors.py` gains one send
(`twinkle off` in the scene setup, beside `timerate rate 0`, with a comment citing this
entry) and the README's b4 section says why; re-run `b4_anchors_run.sh` on the fixed
binary — green is the DoD. H1 refuted ⇒ the difference is the PRODUCT's: attribute it
between §5.133's ownership fix and §5.134's guard (the two commits are `9e0f1e93` →
`a4a7c226`; a build with the comet guard alone is one revert of the ownership hunk in a
scratch tree — do it if health permits, else name it owed), record at §5.133's row
(both homes) what the pre-fix binary was drawing, and leave the gate RED with its cause
named in the README — never re-bank, never demote the control. Either way §11.205(g)'s
*"unconfirmed"* gets its back-marker. (5) **Record:** §11.208 entry FIRST + stub (the
margin table, the random-channel census, the predictions and their fates, the set
diffs, the verdict); §11.205(g)/(h) back-marked in both homes; `harness/README.md`
b4 section (the instrument note upgraded to a rule if H1 holds); WIP per §0.6; D14.

**Boundaries:** no engine code; `b4_anchors.py` edited ONLY on H1-confirmed and only by
the one scene send + comment; no re-bank, no threshold change, no control demotion;
builds and farms under `/home/claude/sc-f89/`; the real `~/.spacecrafter` untouched (md5
asserted); no `run_in_background`; the full canary NOT run (no band claim is made —
say so in the entry rather than substitute a weaker gate).

**Discriminating checks:** (a) the control's two scalars + margin for all five F86 runs,
from the committed artifacts; (b) the random-channel census with sites, and the
"twinkle is/is not the only channel" sentence; (c) predictions committed before the
runs, each with its refuting number, each marked confirmed/refuted after; (d) the P7
window set-diffs: within-binary ×2, between-binary ×4, twinkle off — numbers; the
twinkle-on floor re-measured; (e) the full-precision dump comparison for H2; (f) the
gate's state after the task: green on both binaries (instrument fixed), or red with
the cause named at §5.133 — one of the two, never "unexplained".

**Preconditions (checkable, §0.7):** code HEAD `a2fd3c5b`, harness HEAD ⟨at dispatch⟩;
binary `build-claude/src/spacecrafter` md5 `2815d182`, `cmake -n` 0 steps; the five
`harness/artifacts/f86/b4_*.json.gz` present; `b4_anchors.py:185` reads `send("timerate
rate 0", 1)`, `:186` `date jday`, `:422` the P7 print, `:477` the control string (drift
re-resolved; content drift = abort); `hip_star_mgr.cpp:640-643` the twinkle site (drift
re-resolved); `grep -rl 'srand(' src/` empty; §5.133 FIXED and §11.205 present with
(g)/(h) as quoted; next free §11 **208**; live `### F` count **5**; `/home/claude/sc-f89/`
ABSENT; canary `--no-scene` exit 0 this boot; display per HOST-EVENTS 2026-09-04 (`:2`,
auth under `/run/user/1003`); no `spacecrafter` in `/proc/*/comm`; config/ssystem md5
`03fbee59`/`545a51ef`; `free -g` available ≥ 16 GiB before any build (else `-j6`).

**DoD:** the margin table + census + predictions + set diffs + verdict; the instrument
fix (if H1) with b4 green, or the §5.133 record (if not) with the red explained;
§11.208 + stub; back-markers; README; trees clean; WIP cleared; baselines LAST with
deltas.
**WIP:** DELIVERED 2026-09-05 16:35 → **§11.208** (+ stub). **H1 (twinkle) REFUTED, H2
CONFIRMED with its stated metric refuted, H3 REFUTED, H4/H5 measured:** twinkle off makes the
split deterministic (ref FAIL 217>182 ×2, pre PASS 45<217 ×2; 21950 px between binaries against
a 0–26 px A/A floor), and the cause is §5.133's own defect on the OLD path — the anchor's
observer 182582.8 km from the Moon against the authored 200000.0, the old view 76.46° off after
one tracking command, the star field composed for a different sky (`nbStarsToDraw` 3446 vs 3573).
A binary carrying §5.134's guard ALONE lands in the pre group. Gate left **RED with its cause
named**; `b4_anchors.py` untouched (H1 was its only condition); two stronger repairs recorded,
NOT applied (§11.208(i) — the dispatcher's call). Back-markers at §11.205(c)(g)(h) both homes,
§5.133 and §5.134 rows; `harness/README.md` F7 + F86 sections. No band claim (full canary not
run). Instruments: scan **217/265/131** (flagged-set diff vs the open HEAD **empty**),
pair-check **224/199/25/107**, D 35 · D2 11 · I 89 · I2 36 · M 81 — every filter delta 0.
Left on disk: `/home/claude/sc-f89/` (three binaries in `bin/`, the code worktree `tree/` at
the PRE source state, `build-var/`, eight run dirs).
**ACCEPTED 2026-09-05 ~~16:38 (`date` in the verifying command~~ **[CORRECTED one commit later:
the verifying command's `date` read 16:33:06 and the acceptance commit `d9025dd` reads
16:35:16 — "16:38" was an ESTIMATE wearing the measured label, Q-67 class (c), dispatcher
defect #5 this round, mine; the rule survives only as the `date` output pasted, never a
number typed beside the word]** (supervisor, session 23, Claude
Fable 5.1).** Verified by my own runs and reads, not by the report: §11.208 read in full;
code UNTOUCHED at `a2fd3c5b` (status clean; `git worktree list` shows `/home/claude/sc-f89/tree`
detached at `a2fd3c5b` — registered, does not touch `master-beta`); four harness commits
`846cbeb` → `d264546` → `ef56580` → `9bb8b0d` (Claude Opus 5), tree clean; `b4_anchors.py` and
`b4_anchors_run.sh` `git diff 6594960..HEAD` EMPTY; predictions committed at `846cbeb` BEFORE
the runs at `d264546`; §11.208 markers present at §5.133, §5.134 and three times in
`INTENT/11.205.md`; artifacts R1–R8 + `prediction.txt` + the three analyses present; the three
scratch binaries `531b28aa` / `6f0c8de8` / `9471f2fc` (1.5 GB); instruments to the digit (scan
217/265/131 · pair-check 224/199/25/107 · D 35 · D2 11 · I 89 · I2 36 · M 81). Deviations
ENDORSED with the executor's arguments: the variant runner duplicating eight lines (the
boundary forbade editing the gate; the gate stays `b4_anchors_run.sh`, said in the header);
twinkle switched at the farm's config and VERIFIED from the dump (`twinkleAmountEff` 0.0000);
the pre binary as `a2fd3c5b` with four files reverted rather than a `9e0f1e93` checkout (source
equivalence MEASURED: the diff reduces to the tree-identical gitlink; one build dir, no
dependence on the amended-away submodule commit); the optional third binary built (attribution
became a measurement: §5.134's guard alone lands in the pre group); F86's outputs re-analysed
AND reproduced by fresh runs; the `sets` relaxation to integer window centres (recorded).
DISPATCHER DEFECTS reported, both ACCEPTED as mine: H2's criterion named NEW-path fields
(`camera.mat`, the anchor's position) for an OLD-path (`navModule`) defect — taken literally it
refutes what it tests; output-side by construction (mandate (2) makes the executor write the
predictions), counterfactual stated; and *"every float at `%.17g`"* (only `oldView.*` and
`camera.rootPos` print at 17 digits). Round tally: **four dispatcher defects** (two at open,
two here). Criterion-integrity instances credited: H3 decided from the committed records
before any launch; H1's refuting number stated first and met 845× over; H2's stated metric
refuted and the hypothesis kept where it lives; the A/A floor re-measured with twinkle on AND
off in the same session; the over-claim ("byte-identical in all thirteen") caught by measuring
it. **THE ROUTED CALL, TAKEN:** §11.208(i)'s two repairs are §13.B harness work and
decision-free by the instrument's own criterion — a control that was green on a sky 76° wrong
and reds on the corrected one is a WRONG instrument, and the newcomer's first `b4_anchors_run.sh`
meets that red — so they are minted as **F93** (S), dispatched after F87; the gate stays red
until then, with its cause named in the README. STANDING CONSEQUENCES: §5.133's *"silent on the
shipped build"* is CORRECTED at three homes (not crashing ≠ silent — the shipped build composed
its star field for a different sky than its bodies after one `camera action switch` onto an
orbit anchor); twinkle costs 2165–2229 px of A/A floor and turns P7's verdict into a coin toss
— any pixel comparison switches it off first (`f89_b4_variant.sh <out> off`).

### F90 — T5.1, the tester's day in the app, REHEARSED — and written as the developer's smoke suite: one launch on a farm in French through the shipped command surface (launch · author a body · run a shipped show · search, recorded as deprecated · select and read out · save · reload · one keyboard ramp · quit), every step's observable and pass criterion stated BEFORE the run, three runs (the functional A/A floor — a flake is a finding), every divergence recorded against its §5 row or minted by §5.79's criterion and NONE fixed, the suite runnable by the newcomer in one command and shown able to fail (DEPLOYMENT-MAP T5.1: *"the closing audit before 'ready'"*; the owner's line today makes "ready" the criterion) [M]

**Why now / mandate:** the map's own gate on the word "ready" (T5.1 + T5.2, both
re-cuts of the critical path) has NEVER RUN — every harness in the tree is a proxy built
from OUR model of operation (T5.1's own sentence). Today's line [vixy 2026-09-05]:
*"…renamed main once ready."* ⇒ the rehearsal is the first half of what "ready" means
here, and the SAME artifact is the smoke suite `doc/developer-entry.md` §build/run/test
can hand the newcomer. Search is deprecated (R22) — the step stays IN, run once,
recorded as deprecated: its result is a datum, not a gate.

**Measured at dispatch (supervisor, 2026-09-05 15:1x):** field `config.ini:68`
`app_locale = fr`, `:71` `sky_locale = fr`. Shipped scripts under
`~/.spacecrafter/scripts/{basis,custom,deepsky,fscripts,internal}` (`basis/` holds
`artificial_satellites.sts`, `moon_phases.sts`, … — the executor picks the SHORTEST
show by reading, states the pick and its measured duration). FARM SAFETY, the one
hazard this task must not repeat: `b3_farm.sh` SYMLINKS every `~/.spacecrafter` entry it
does not rebuild (`b3_farm.sh:24`) — `scripts/` included, so anything writing
`scripts/fscripts/startup.sts` writes the OWNER's file (README `:2097-2109`); `f55_farm.sh`
rebuilds `scripts/` and `scripts/fscripts/` as real directories of symlinks (`:31-33`).
`config.ini` is ALSO a symlink under both farms ⇒ any `configuration action save` in the
farm would rewrite the REAL config (§5.42's writer): the F90 farm makes `config.ini` a
COPY, and the real one's md5 (`03fbee59`) is asserted before and after every run.
Verbs at source (cite the line at dispatch-time HEAD): `session action save|load
[filename <name>]` (§11.128(f)), `body action load` / `body action save [filename
<name>]` (§11.121(e)), `body action reload`, `get status position` (§11.135), `select
planet <name>`, `search name <x>`; the quit verb is NOT stated here — find it. The live
key channel is F25's (`f25_ramp.py`, README `:1229`); the drag channel is DEAD on this
host (README `:1277`) — keys only. The path that draws by default is NEW (§11.206(c):
`beta_features.ini [dual_path] render_path` absent ⇒ NEW+pinned). Teardown hazards a
quit can meet: §5.59 / A40 (`f19_stall.sh` is the precedent instrument). The sidecar
reader for info strings is `f44_parity.py` (`.navstr`); the dump reader `dumpread.py`.

**Mandate:** (1) **The suite**, `harness/f90_rehearsal.py` + `f90_rehearsal_run.sh`
(one command: `SC_BIN=… DISPLAY=… ./f90_rehearsal_run.sh <absOutdir>`), ONE launch per
run on an f55-shape farm with `config.ini` a copy, the sequence with each step's
OBSERVABLE, CHANNEL and PASS CRITERION written in the script's header and printed in
the per-step table: **(a) launch** — exit of the startup phase, the applog's startup
errors AND silences (§5.77's class: what a missing class says or does not), GetActive,
the `/proc/<pid>/comm` assert; **(b) author a body** — `body action load` with a minimal
scratch section (a shape the grammar accepts; F86's `section_withperiod.ini` is a
precedent) → the body present in the dual dump; **(c) run a shipped show** — the chosen
`basis/` script via the script verb → script-log start/end, `#!` count, wall time vs the
show's own duration; **(d) search** — `search name <a shipped body>` once → result
recorded, step labelled DEPRECATED (R22), never a gate; **(e) select + read out** —
`select planet Mars` → `get status object`, the info string through the `.navstr`
sidecar — the LANGUAGE of the labels on the drawn path recorded (post-F87 they should be
French; if F87 was not delivered, record the English and cite §5.111), `get status
position`; **(f) save** — `session action save filename f90` (+ `body action save` on the
authored body) → files exist under the FARM; **(g) reload** — `session action load
filename f90` → the restored state equals the saved one on the channels §11.129's T4
names (byte-identity where the ledger claims it), `body action reload` → the authored
body survives; **(h) keyboard** — one zoom-in/zoom-out ramp through F25's key channel →
fov before/after from the dump; **(i) quit** — the verb → exit code, wall time to exit,
`Frame stall` count, any teardown fault (A40/§5.59 class) — recorded, never masked by a
kill. (2) **Three runs**, fresh farm each, tables side by side: a step that passes in
some runs and not others is a FLAKE and a finding (name the axis). (3) **Shown able to
fail**: one injected fault (a misspelled body name in (b)) makes the step fail and the
run exit non-zero — kept as an artifact. (4) **Every divergence/failure** → its existing
§5 row cited at the step (the row's back-marker written at both homes), or a NEW row by
§5.79's criterion (a crash or false-success reachable from a shipped command) — NOTHING
fixed. (5) `doc/developer-entry.md` §build/run/test gains ONE line pointing at the suite
(code repo, code first; veto-open). (6) **Record:** §11.⟨next⟩ entry FIRST + stub (the
step table ×3, the flake count, the divergence list with rows, the language datum, the
timings); DEPLOYMENT-MAP T5.1 struck with the record (RAN; what it found; what "ready"
still waits on = T5.2); `harness/README.md` section; WIP per §0.6; D14.

**Boundaries:** no engine code; the farm only (`config.ini` a COPY in the farm, real md5
asserted in==out, `scripts/` per f55, `/usr/local` untouched); one doc line in the code
repo; FUNCTIONAL — no photometric claim; each launch foreground within-turn (a show
longer than the tool's timeout is not chosen — the pick states this); no
`run_in_background`; three runs' wall budget stated in the entry.

**Discriminating checks:** (a) every step's observable + criterion printed BEFORE the
launch (the header) and the table after; (b) 3/3 tables, flake count with axis; (c) the
injected fault fails the run (artifact); (d) real HOME md5 in==out ×3; (e) the label
language on the drawn path, quoted; (f) the quit's exit code and teardown record ×3.

**Preconditions (checkable, §0.7):** code HEAD ⟨F87's, at dispatch⟩, harness ⟨at
dispatch⟩; F87's state named (delivered or not — either is fine, the label step records
it); binary current at that HEAD; `harness/f55_farm.sh` present and `b3_farm.sh:24` the
symlink line; `config.ini:68` `app_locale = fr`; `~/.spacecrafter/scripts/basis/`
present; the verbs above at source (re-resolved, cited); `f25_ramp.py` present; display
per HOST-EVENTS (`:2`); canary `--no-scene` exit 0; next free §11 ⟨at dispatch⟩; live
`### F` count **5**; `/home/claude/sc-f90/` ABSENT; no `spacecrafter` in `/proc/*/comm`;
config/ssystem md5 `03fbee59`/`545a51ef`.

**DoD:** suite + runner + 3 runs + the fault run; the divergence list with rows; the doc
line; §11 entry + stub; map T5.1; README; trees clean; WIP cleared; baselines LAST.
**WIP:** —

### F91 — EXTENSION: §5.86 + §5.19 — the new path's RA/DE readout computed in the observer's frame, as the tester reads it: `Camera::observedToBodyLocalPos` made the true inverse of `viewMat` (the algebraic inverse `f34_probe_inverse.cpp` already carries, round trip 133.9° → ~1e-11 AU) and ONE conversion authority for `observedPosToRaDe` in old's frame (topocentric, the RA zero point by the equinox definition §11.198(b), never a magic constant), the SA/GHA/LHA nav fields riding it; parity §11.158(f): new == old to ≤ 0.002° for 89/90 bodies, the 90th attributed (§11.207(g) item 1 — both decisions closed by R27 + §11.198(b); T1.4 CLOSED) [M, engine]

**Why now / mandate:** the first decision-free engine item the final pass created
(§11.207(g)(1)); *"he reads coordinates professionally"* (DEPLOYMENT-MAP T1.4) and the
new path's readout for a composed body answers in a scrambled frame (§5.86: 133.9041°
round-trip error; §5.19: the Moon `04h57m15s/+26°41'47"` old vs `03h44m18s/+14°40'05"`
new on the same frame). R27 [stated: tester, via `6ffb017`]: *"The RA/DE must be the
value from our position."* ⇒ observer-centred, i.e. what old computes; decision (1)
resolved at §11.198(b). Old is the baseline (§11.52(b)): the target frame is DERIVED
from old's code, cited — never chosen.

**Measured at dispatch (supervisor, 2026-09-05; row text at HEAD `a2fd3c5b`, sites
last re-verified exact at `d6aec251` by F44 — RE-RESOLVE at HEAD, content drift =
abort):** §5.86: the anchored branch applies `Y(latitude−π/2)` where `viewMat` composes
`X(lat−π/2)`, `Z(−longitude)` where the inverse needs `Z(+lon)`, subtracts `distance`
where the inverse adds it, omits the surface fold — `Camera.hpp:264-274` vs
`Camera.cpp:183-197`; consumers `ModularObject.cpp:19, :47, :115, :152`; the FREE branch
exact when unbound (2.058e-11 AU), off by the surface fold when bound (38.5856° at
`axisRot` 0.7 rad); `observedPosToAltAz` NOT affected (rides `observedToLocalPos`).
§5.19: `ModularObject::getRaDeValue`/`getInfoString`/`getShortInfoNavString` read
`Camera::observedPosToRaDe(body->getObservedPosition())`; old: `Body::getRaDeValue` =
`Utility::rectToSphe(getEarthEquPos)`; feeds `getSelectedRA/DE` and the SA/GHA/LHA nav
fields. §11.158(f): RA zero point a constant −90.0003° epoch-independent, declination
already matched to 0.0036°; with both settled + §5.86's fix, new == old to ≤ 0.002° for
89/90 bodies. Instruments: `harness/f34_probe_inverse.cpp` (the inverse written out),
`f44_parity.py` + `artifacts/f44/legA_003.json.navstr` (the sidecar channel F44
measured on). The idiom to follow: `ModularObject::altAz()` (§5.19's own sentence).

**Mandate:** (1) **Derive the target** before touching code: read `Body::getRaDeValue`
and `getEarthEquPos` at HEAD and STATE the frame old answers in (which equator/equinox,
which origin) with the citation; read §11.198(b) and state where the −90.0003° comes
from as a definition (what rotation is missing, not "a constant"); read §11.158(f) for
the 90th body and its attribution. (2) **The fix, new path only:** `observedToBodyLocalPos`
= the exact inverse of `viewMat` (the probe's algebra), both branches; `observedPosToRaDe`
routed through ONE authority that lands in old's frame; consumers untouched in shape;
every new expression carries its derivation in a comment citing this entry. (3) **Prove
with the project's own primitives:** the round-trip probe re-run pre/post (133.9041° →
the post number, ≤ 1e-9 AU expected); the F44 sidecar leg on the farm — the body set F44
used (state the count), old vs new RA/DE per body, |Δ| stated, **≤ 0.002° for all but
the one §11.158(f) names**, that one attributed as there — a miss on any other body =
STOP and report the residual with its attribution, never widen; the nav fields
(SA/GHA/LHA) equal old vs new on the same frame; an English-locale control leg unchanged
by the fix (readout format is not touched). (4) **Record:** §11.⟨next⟩ entry FIRST +
stub (the derivation, the table, the exception); §5.86 (inline) and §5.19 (row + entry
file) flipped FIXED with pointers; §11.4 and §11.158(f) back-marked; DEPLOYMENT-MAP
(T1.4's work item struck); README section; WIP per §0.6; D14.

**Boundaries:** `src/experimentalModule/` (Camera + ModularObject) only; NO old-path
change; no data; no msgid change (F87's territory); no `run_in_background`; the full
canary NOT run (no photometric claim; `--no-scene` only).

**Discriminating checks:** (a) the frame derivation with old's citations, written before
the fix; (b) the probe round trip pre/post; (c) the per-body table, max |Δ|, the named
exception; (d) the nav fields; (e) the English control unchanged.

**Preconditions (checkable, §0.7):** code HEAD ⟨at dispatch⟩, harness ⟨at dispatch⟩;
§5.86 OPEN and §5.19 OPEN (row + `INTENT/5.19.md`); the six site coordinates above
re-resolved at HEAD (content drift = abort); `harness/f34_probe_inverse.cpp` and
`harness/f44_parity.py` present; `artifacts/f44/legA_003.json.navstr` present; next
free §11 ⟨at dispatch⟩; live `### F` count **5**; display per HOST-EVENTS; canary
`--no-scene` exit 0; binary current at HEAD; `free -g` ≥ 16 GiB before the build.

**DoD:** fix (code first) + probe + table + controls; §11 entry + stub; the two rows
flipped; back-markers; map; README; trees clean; WIP cleared; baselines LAST.
**WIP:** —

### F92 — EXTENSION, record-only: the rename's FOOTPRINT — every `master-beta` in both repositories classified (LIVE POINTER · PIN · HISTORICAL RECORD · CONVENTION) and the owner's rename reduced to one ordered checklist plus one unapplied patch covering exactly the live pointers, so that `master-beta → main` is a single act with a measured blast radius instead of a silent-drift generator (the owner's line 2026-09-05: *"…get renamed main once ready"*; I2 — every document naming the branch is a manual-resync point) [S]

**Why now / mandate:** the rename is decided (today's line) and not yet due ("once
ready"); what it stales is everything that names the branch. Measured at open
(2026-09-05 15:1x, `git grep`): code repo **9 files** — `util/scedit/grammar/sc-grammar.json`
×6, `args/unit-{1,2,3,4}.json` ×2 each, `ss-grammar.json` ×1, `witness/superscript-witness.json`
×1, `tests/derivation-diff.md` ×1 (all of the shape `master-beta @ <sha>` — pins), and
`doc/developer-entry.md` ×1 (the newcomer's ONE sentence naming the branch); harness
repo **118 files / 189 hits** (`dispatch-2026-07-21-opus.md` 20, `fable-dispatch.md` 18,
`DEPLOYMENT-MAP.md` 8, `INTENT.md` 5, `INTENT/11.203.md` 5, `INTENT/11.134.md` 3, scripts
`f29_upchain.py:5`, `f38_mirror.py:11`, `f58_gaptable.py:9,:722`, `f68_provenance.py:26`,
artifacts `f51/f52/f53/f55` json — every script/artifact hit is a `master-beta @ <sha>`
citation by its text), plus every harness commit trailer `Code: master-beta @ <sha>`
(history, never rewritten), `CLAUDE.md`'s branch line, `README.md`'s contract, and the
remote's default `origin/HEAD → origin/2023-master` (no `main` exists locally or among
the fetched remote refs). NOTHING is renamed by this task.

**Mandate:** (1) **The census**, both repos, every hit in exactly ONE class: **LIVE
POINTER** (a reader follows it to the branch today — CLAUDE.md, `claude/README.md`,
`doc/developer-entry.md`, `INSTALL` if it names the branch, `harness/README.md` §Run,
`f85_links.py` if it checks branch names, any script running `git rev-parse master-beta`
or comparing the current branch name — grep for `rev-parse`, `symbolic-ref`, `branch
--show-current` too, and the `githooks/` trailer check) · **PIN** (`master-beta @
<sha>`: resolves by SHA whatever the branch is called — untouched forever; the scedit
`_meta.anchor_pin` class, F75 S2) · **HISTORICAL RECORD** (ledger prose, archived
notes, trailers in history — never rewritten) · **CONVENTION** (the `Code: <branch> @
<sha>` trailer grammar — after the rename new trailers say `main`, old ones stay; the
README sentence that states the grammar is the one line to touch). Counts per class per
repo; the partition sums to the totals. (2) **The checklist for the owner**, ordered, each
line one act: local `git branch -m master-beta main` · `git push -u origin main` · GitHub
default branch → `main` · open PRs' base · every clone's `git remote set-head origin -a`
+ `git branch -u origin/main` · the developer's clone instruction (`developer-entry.md`,
`INSTALL`) · the harness contract line · the `CC-harness` trailer convention going
forward — and what does NOT change (pins, history, `2023-master`'s own existence).
(3) **The patch**, `harness/artifacts/f92/rename-live-pointers.patch` (two hunks sets,
one per repo; code first): exactly the LIVE POINTER edits, `git apply --check` clean in a
scratch worktree of each repo, and — applied THERE only — `git grep master-beta` over the
LIVE-POINTER class = 0 while PIN/HISTORICAL/CONVENTION-history are untouched; the
worktrees discarded, the real trees never modified. (4) **Breakage scan:** which scripts
would fail or lie after the rename (a script that greps the current branch name, a hook
that validates the trailer against the branch) — each named with its line or "none". (5)
**Record:** §11.⟨next⟩ entry FIRST + stub (the census table, the checklist, the
breakage scan); DEPLOYMENT-MAP **R6** annotated — the policy half ANSWERED by the
owner's line (rename, not redirect), the "ready" half = T5.1 (F90) + T5.2 per the
map's own critical path — annotated, not struck (the rename has not happened); §3
item; `harness/README.md` section; WIP per §0.6; D14.

**Boundaries:** NO rename, NO branch created, NO edit of any live pointer in either
real tree (the patch is an artifact), no push, no `git config` change, no history
rewrite; scratch worktrees under `/home/claude/sc-f92/` and removed; no launch.

**Discriminating checks:** (a) the census partition sums to the measured totals, both
repos, every hit classified; (b) `git apply --check` clean in both scratch worktrees and
the post-apply grep over the live class = 0 there; (c) the breakage scan names a line or
says none, with the grep that proves it; (d) both real trees byte-unchanged except the
harness records (`git status` on the code repo EMPTY at close).

**Preconditions (checkable, §0.7):** code HEAD ⟨at dispatch⟩, harness ⟨at dispatch⟩;
`git grep -l master-beta` code = 9 files, harness = 118 files (`-o | wc -l` 189) — drift
= re-measure and report, not abort; `git symbolic-ref refs/remotes/origin/HEAD` =
`refs/remotes/origin/2023-master`; `git branch -a | grep -c ' main$'` = 0; next free §11
⟨at dispatch⟩; live `### F` count **5**; `/home/claude/sc-f92/` ABSENT.

**DoD:** census + checklist + patch artifact + breakage scan; §11 entry + stub; map R6
annotated; README; both trees clean (code untouched); WIP cleared; baselines LAST.
**WIP:** —

### F93 — The `b4_anchors` gate REPAIRED where F89 showed it blind: (1) a NEW assert that the OLD path's distance to the anchor's parent equals the authored radius (`Moon.old.dist` = 200 000.0 km — 17 417 km of discrimination, wrong in b4's own artifacts since F7 and asserted by nothing), (2) P7's screen-witness control re-expressed as a LIT-PIXEL COUNT comparison instead of a max-over-window (846 vs 21 px at t0, 2023 vs 27 at t1 — 30–100× on both binaries where the max form gives −35 / +172), (3) the scene's twinkle OFF so the gate is deterministic (0–2 px A/A instead of ~2200); proven BOTH WAYS on the two binaries F89 left: GREEN on the reference `2815d182`, RED on the pre-fix `sc-scratch-pre` `9471f2fc` at the new assert AND the new control — a gate that now reds on the corrupt build and greens on the correct one, which is the direction it had backwards (§11.208(i)(j); §13.B; the developer's first harness run) [S, instrument]

**Why now / mandate:** §11.208(i): *"the gate was green for three runs on a binary whose
entire star field was 76° wrong"*; on the reference binary it reds because an ordinary star
of luminance 217 sits in the other date's 112×112 window while the Moon's disc peaks at
182 — a MAX-over-window control cannot tell a star from a Moon. Both repairs are recorded
there as strictly stronger, and the supervisor's call at F89's acceptance takes them: a
harness gate whose verdict is inverted with respect to correctness is an instrument defect
(§13.B, decision-free — no engine semantics, no band, no threshold demotion; every change
is a STRENGTHENING shown to fail on the known-bad binary). The developer will run
`b4_anchors_run.sh` on his clone; today it exits 1 on the correct binary.

**Measured at dispatch (supervisor, 2026-09-05 16:3x, from §11.208 — the executor
re-reads the entry, this is its derivation):** the control lives at `b4_anchors.py:476-480`
(max luminance own-window vs other-window, two shots t0/t1); the margin table §11.208(b);
the old path's distance to the anchor's parent is in the dump as `Moon.old.dist` (182 582.8
km pre / 200 000.0 km fixed, seven runs, two sessions, to 0.1 km), the authored radius is
`b4_anchors.ini`'s 200 000 km (P1's constant); the ease-out residue moves dumped NDC by
~1e-7 and can shift a window centre by one pixel between runs (README F7 note); twinkle is
read at `core.cpp:346` from the config key `flag_star_twinkle` (F89 switched it at the
FARM's config; b4's scene sends nothing for it). Binaries: reference `build-claude/src/
spacecrafter` `2815d182`; pre-fix `/home/claude/sc-f89/bin/sc-scratch-pre` `9471f2fc`
(`9e0f1e93` sources, F89 (g)); guard-only `6f0c8de8` (lands in the pre group). A b4 run costs
~2 min 50 s. `f89_p7.py` (`margins`/`sets`/`scalars`/`leafdiff`) and `f89_b4_variant.sh` are
the analysis instruments F89 left; the GATE is `b4_anchors_run.sh` and stays so.

**Mandate:** (1) **The assert**, in `b4_anchors.py` beside P1 (the on-orbit check): the OLD
path's distance to the anchor's parent (`Moon.old.dist` from the dump, the reader
`dumpread.py`) equals the authored semi-major axis within a tolerance DERIVED — from the
ease-out residue and the float precision of the dump field, stated with its derivation, not
picked — asserted at every dump the scene takes; the check's message names the row (§5.133)
and the authored value. (2) **The control**: P7's screen-witness control compares LIT-PIXEL
COUNTS (px>8, the corpus's own metric) of the Moon's own window vs the other date's window
in each shot — own ≥ K × other with K derived from F89's corpus (846/21 and 2023/27 on BOTH
binaries: state K and why, e.g. 4× — margin ≥ 7× below the weakest measured ratio); the
MAX form is REMOVED, not kept beside (a second observable for one claim is I2's duplicate);
the check's text says what it measures. (3) **Twinkle off** in the scene: since b4's scene
sends nothing for it and the config key is what `core.cpp:346` reads, the runner
(`b4_anchors_run.sh`) writes `flag_star_twinkle = false` into the FARM's config copy (the
farm's `config.ini` must be a COPY for that — assert the real one's md5 in==out), with a
comment citing §11.208(j); verify from the dump (`twinkleAmountEff` 0.0000) inside the
script, never assumed. (4) **Prove both ways:** `b4_anchors_run.sh` on the reference binary
→ **exit 0, every check green, ×2** (A/A: the two runs' P7 lit sets within the 0–2 px floor);
on the pre-fix binary → **exit 1** with EXACTLY the new assert and the new control red, every
other check as before (P1–P6 green — they were green on both), ×1; the guard-only binary → the
pre group (×1, optional if health permits). State the counts (own/other px, the distance
values) in the entry's table. (5) **Record:** §11.⟨next⟩ entry FIRST + stub; §11.208(i)
back-marked in both homes (the repairs APPLIED); §5.133's row gains the pointer (the gate
that catches its class); `harness/README.md` F7 section rewritten to the new gate (the "exits
1 on the reference binary" opening REMOVED — it is no longer true); WIP per §0.6; D14 (harness
only — state it).

**Boundaries:** harness only (`b4_anchors.py`, `b4_anchors_run.sh`, README, the entry); NO
engine code; NO re-bank of anything; NO threshold on the existing geometric checks P1–P6
touched; the scene's sends unchanged except nothing (twinkle rides the config, not a send);
the real `~/.spacecrafter` untouched (md5 asserted); all runs under `/home/claude/sc-f93/`;
no `run_in_background`; the full canary NOT run (no band claim); the pre-fix binary is
F89's file — never rebuilt here (if absent, STOP and report).

**Discriminating checks:** (a) the tolerance's derivation, written before the runs; (b)
reference ×2: exit 0, all green, A/A within floor — numbers; (c) pre-fix ×1: exit 1, the two
NEW checks red and ONLY those, with the values (182 582.8 km; the counts); (d) `twinkleAmountEff`
0.0000 read from every run's dump by the script; (e) real config md5 in==out; (f) the README's
F7 opening no longer says the gate reds on the reference.

**Preconditions (checkable, §0.7):** code HEAD `a2fd3c5b` (F87 may have moved it — state
the HEAD at dispatch; the gate is source-independent), harness ⟨at dispatch⟩; §11.208
present with (b)(i)(j) as quoted; `b4_anchors.py:476-480` the max-based control (drift
re-resolved, content abort); `/home/claude/sc-f89/bin/sc-scratch-pre` md5 `9471f2fc` present;
the reference binary current at HEAD (md5 stated at dispatch); `b4_anchors.ini` authored
radius 200 000 km; `core.cpp:346` reads `flag_star_twinkle`; `harness/f89_p7.py` present;
next free §11 ⟨at dispatch⟩; live `### F` count ⟨at dispatch⟩; `/home/claude/sc-f93/`
ABSENT; canary `--no-scene` exit 0; display per HOST-EVENTS; no `spacecrafter` in
`/proc/*/comm`; config/ssystem md5 `03fbee59`/`545a51ef`.

**DoD:** the assert + the count control + twinkle-off in the runner; proven both ways with
the numbers; §11 entry + stub; back-markers; §5.133 pointer; README F7 rewritten; trees
clean; WIP cleared; baselines LAST.
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

- **Session-22 decision items (2026-09-05, the REFERENCE round — kept short on your
  word about this week's capacity; every item is one line to answer or to ignore):**
  - **YOURS, and nothing here substitutes them:** (1) **R5 — the PUSH**: `master-beta`
    (86 commits) and `CC-harness` (674) from a keyed host; the EntityCore push you made
    at 12:23 is in and completed by the pin bump. (2) **R6 — the branch policy**: does
    `master-beta` become the PR target / main, or are Kenan-Blasius, Lionel and Calvin
    redirected to it? One sentence. (3) **R23's procedure**: where does the outside
    catalogue/content installation procedure live, and may it be documented in the
    repository? (§11.204(f): a tree install ships NO content; the entry document names
    this as the one thing it cannot tell the developer.) (4) The developer's platform
    (if Windows, the vcpkg path entered untested) and whether the harness repo is meant
    to be readable by him (the entry document points at it either way).
  - **VETO POINTS taken this session (implemented-and-live, each cheap to reverse;
    silence = endorsed):** (a) the EntityCore pin bump `32af2efb` — the completing half
    of your own push (tree-identical); (b) §5.130's one-line move folded into F86 as
    decision-free (§5.79/F35 precedent) — a fresh account's first launch now works;
    (c) `install_src.sh:25` `-n`→`-z` (the shipped default build was silently Debug/`-Og`
    under a 1 ms/frame budget) and `src/CMakeLists.txt:3` `CONFIGURE_DEPENDS` (+0.05 s
    per build; an incremental build now sees a new file); (d) INSTALL rewritten
    Linux-first (the Windows/VCPKG block kept as its own section), `doc/developer-entry.md`
    (424 lines, name/placement/size veto-open, every sentence sourced and
    machine-checked by `harness/f85_links.py`); (e) the round-3 replies' commit author
    amended to `Lionel RUIZ <lionel.ruiz@live.fr>` on your word, SHA map in the open
    note; (f) CLAUDE.md gained encoding hazard (5) (`/usr/bin/grep` and `\xNN` in
    brackets); (g) the F84/F86 scratch trees under `/home/claude/sc-f84/` (4.2 GB) and
    `/home/claude/sc-f86/` (1.3 GB) KEPT — say the word to remove.
  - **DECISIONS THE REPLIES OPENED (§11.207; the fix stratum is yours per §11.161(c)):**
    **§5.135** — the OLD path rolls the view offset with the heading and the tester
    says it shouldn't; the fix lands on the comparison baseline: correct it, or let it
    retire with B8? · **D37's implementation** (hidden star → dark, ambient by
    `ambient_light`) — schedule? · **N7** — the migration writer should comment out,
    not delete (§5.112's direction; §5.42's writer rides it) — say when · **R25** — the
    tester says "no need" to your own `[parallel-script]` proposal — withdrawn, or kept
    as yours? · **§5.86 + §5.19** are now fully decision-free (R27: observer-centred) —
    the next round's first engine candidate unless you say otherwise.
  - **HELD OPEN, not absorbed:** `b4_anchors` P7's screen-witness CONTROL reds on the
    F86 binary (green ×2 pre-fix and on a `main.cpp`-only binary); bounded — Moon
    centroid bit-identical in all five runs, canary frame byte-identical across a
    fourth binary — cause unconfirmed (twinkle `rand()` unseeded is the candidate, both
    experiments failed); next round's instrument position 1. Also open: §5.48
    (EntityCore, 0/6 today), §5.59's new `sender` arm (unmeasured), the tester's
    unanswered halves (§5.128's snap-back, A42's distance and body list, R19's
    999-dependence, L2).
  - **FACTS, no decision asked:** the deployed line is fully in (`origin/2023-master`
    is 2 commits by content, both Kenan's video-as-`s_texture`; the remote had not moved
    since 2026-08-03); the canary band and dwell frame survived four binaries; §5.112 on
    YOUR field config deletes nothing (0/2 added) and on a hand-authored one projects
    everything off-schema away; a fresh HOME's `sky_cultures` is 262 real files — the
    field's 2922 zero-byte set is the field's copy, not the data root's; every
    "~HH:MM" in this session's prompts before 13:40 was ~1 h fast (estimated, never
    `date`) — twenty-two dispatcher defects this round, all output-side, all corrected
    at their nodes, the root named (numbers quoted from memory of a listing).

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
