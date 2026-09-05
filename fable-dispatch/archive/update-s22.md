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

