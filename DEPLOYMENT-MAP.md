# DEPLOYMENT-MAP — the space between HERE and "the main tester operates the new path, fully transparently"

**Status**: DERIVED VIEW over the ledger (compiled 2026-08-29 by Claude Fable 5 at harness
`4cebcd0`, code `d6aec251`, on Vixy's request — §11.162 records the compilation). On any
divergence the ledger (§5 rows, §13 rows, INTENT/<id>.md entries) wins and the divergence is
a staleness bug HERE. Statuses were re-extracted mechanically from the ledger at compile time
+ this session's row reads; ~~rows marked ⚠ were NOT re-read in full at compile time — triage
them before relying.~~ **[⚠ SET DISCHARGED 2026-08-29 by F47 (§11.163), Vixy-ordered: all
twelve §5 rows + B38 read at source against `d6aec251`, each ⚠ struck in place below with its
verdict. Headline: NO mechanism had disappeared — the rows were right, only their citations
had drifted. Three deltas (§5.5 fixed en route and never updated · §5.56's stated ground
refuted and measured · §5.21's reach understated), one new row (§5.112), and one structural
correction to how this map should read the set — see the critical path.]** Maintenance:
correct in place with dated strikes; regenerate wholesale when the drift exceeds reading
comfort.

**[REWORKED 2026-09-05 — A SECOND CRITERION, on the owner's word (supervising session 22,
Claude Fable 5.1). Trigger: *"Read back and update if needed DEPLOYMENT-MAP.md to focus on the
work necessary for clean deployment (and stable reference)"*; the phrase asked once and
answered [vixy 2026-09-05, verbatim]: *"Stable reference is because another junior developper,
major of his promotion, 5th year post-bac, will work on spacecrafter. I would prefer this
branch to became the stable reference for development, otherwise work will continue and
require further feature port. His work will start in a week."*]** The map now carries TWO
criteria with different clocks: **R0** (the developer, ONE WEEK — tier R below) and **T0** (the
tester, decision-paced — the tiers that follow). **What is NECESSARY, and only that, as of
2026-09-05** — the rest of this file is the derivation and stays challengeable:
- **For R0:** ~~R1 reconcile the deployed line (**F83**)~~ **[DONE 2026-09-05, §11.203 —
  merged at `c6784490`; R1's residual collapsed into R5]** · ~~R2 the newcomer's first hour + the
  deployment manifest (**F84**)~~ **[MEASURED 2026-09-05, §11.204 — NOT MET, and what remains is
  the OWNER's twice over: ~~**§5.131** the EntityCore push (a DIFFERENT repository from R5's, so
  R5 does not cover it) and **§5.130** one line moved in `main.cpp`~~ **[BOTH GONE by 14:30:
  §5.131 DISCHARGED — the owner's 12:23 push (§11.205(i)) + the pin bump `32af2efb` (supervisor
  act) + a clone probe rc 0; §5.130 FIXED by F86 (§11.205(b), code `9e0f1e93`). What R2 still
  owes: F85 deletes INSTALL's two now-obsolete paragraphs and re-verifies the clone line; the
  CONTENT question (a tree install ships none, no document says where it comes from —
  §11.204(f)) is the owner's, R6's sibling]**. F84 shipped the docs and
  the two one-line build fixes; the manifest says a tree install carries no content at all]** ·
  ~~R3 the entry document (**F85**)~~ **[MET, §11.206]** · R4 startup memory-unsafety
  (**F86** for the two old-core members; §5.48 is EntityCore = Vixy's) · **R5 the PUSH (Vixy —
  nothing here reaches the developer without it)** · R6 the branch policy (Vixy, one sentence).
- **For T0**, after §11.163(h)'s test (*does the NEW path behave differently here? else backlog*):
  T1.1 the zoom pair (Vixy, one line — §5.100's authorization asked 2026-08-26, unanswered) ·
  §5.111 (**F87**) · §5.86 + the RA zero point (§11.198(b) resolved decision (1); dispatchable
  next round with the origin held old-as-spec until R27) · §5.53(b) (Vixy, one token) · the
  final-pass SEND (Vixy; R21/R27/R28/R29 travel inside it) · ~~the T5.1 rehearsal (next round)~~ **[RAN 2026-09-05, F90 §11.211; T5.1 struck at T5]** ·
  ~~§5.112's warning (N7 drafted; F84 prices its number)~~ **[PAID 2026-09-05, F84
  §11.204(j): 0 deleted / 2 added on the field config, 4 deleted on a hand-edited copy, both
  ending at one md5; N7 carries both numbers. See T3.]**
- **Not required for either**, so the exclusion is visible: every both-paths defect (T1.6,
  T1.10, T1.11 — backlog by §11.163(h)); ~~B1/B2/B3 (until R21's census says otherwise)~~
  **[R21 ANSWERED 2026-09-05 — see the re-cut head below: the census excludes nothing, so
  this escape clause no longer holds by absence; B1/B2/B3 stay excluded on their OWN
  argument (Vixy-paced architecture), which is a weaker exclusion than the one it
  replaces]**; B8.

**[RE-CUT 2026-09-05 — the round-3 replies landed (F88 → §11.207); twenty answers, and
they move this head on both criteria. Previous head kept above, struck where superseded.]**
**What is NECESSARY, and only that, as of 2026-09-05 evening:**
- **For R0 (the developer, one week):** **R5 the PUSH** (Vixy; `master-beta` + `CC-harness`
  from a keyed host — nothing here substitutes it) **[ANNOTATED 2026-09-07, F103's acceptance:
  R5 is scheduled Saturday/Sunday 2026-09-12/13 *"along with supervised_by.sh patching of commit
  history with commit tracking"* [vixy, §11.223(d)]; the tool was made runnable this round
  (§11.224 — B1's fallback gone, two pre-existing blocking defects fixed, the old→new maps
  persisted under `claude/sha-maps/`), and ONE decision precedes the rewrite and is irreversible
  after the force-push: **§5.147** — `git commit-tree` drops `gpgsig`, so the rewrite collapses a
  13-commit duplicate chain on `master-beta` (3829 → 3816) and drops a contributor's signature;
  accept-and-map or refuse-and-resolve, his word.]** **[AMENDED 2026-09-07 17:5x, §11.227: his word was NEITHER — the tool must not touch unselected commits at all, so the collapse is a tool defect and F106 fixes it before Saturday (a `--commit-filter` that keeps an unchanged commit's object and signature); no decision remains before the rewrite.]** **[DONE 2026-09-07, F106 → §11.228: the fix is in the tool and measured. R5's rewrite leg now has a computed expectation instead of a hazard — of the 98 commits in `origin/master-beta..HEAD`, **83 change sha** (the 82 Claude-authored ones plus `f0c8ef83`, which inherits a rewritten parent) and **15 keep their object untouched**, the whole `1ddd32f0..c6784490^2` side chain including `cebebf44` **with its GitHub signature**; `master-beta` stays 3833 and nothing collapses. The live pair's `--dry-run` is byte-identical before and after the fix (944 lines, md5 `3f2e421e`), and the live pair was never rewritten. R5 remains ONE act, the owner's: the push, from a keyed host.]** · ~~**R6 the branch policy** (Vixy, one
  sentence)~~ **[ANNOTATED 2026-09-05: the policy half is ANSWERED — *"The master-beta will
  became the reference and get renamed main once ready"* [vixy, verbatim] = RENAME, not
  redirect. What is left of R6 for R0 is the owner's ACT (the rename, footprint measured by
  F92 → §11.212: 4 live pointers out of 226 occurrences, patch + eleven-act checklist ready)
  and ONE unanswered sentence, the PR target — see the row below]** · and **one new owner question, from R23**: where the outside catalogue
  installation procedure lives, and whether it may be documented in the repository. That
  third item is R6's sibling and §11.206's third residual; it is what stands between a
  clone and a sky with more than 26 561 stars in it. R1–R4 are met or owner-only.
  **Nothing else is necessary for R0.**
- **For T0 (the tester, decision-paced):** the shape changed more than the length. **Two
  T1 gates CLOSE** — T1.2 (the dome-geometry call, R28) and T1.4 (§11.4's pair, R27) — and
  what they were blocking becomes WORK, not decisions. So T0's necessary set is now:
  **T1.1** the zoom pair (Vixy, one line, §5.100's authorization still unanswered since
  2026-08-26 — with T1.2 and T1.4 gone this is **the last operator-basics decision
  standing**) · **T1.3** the reach/visibility batch (DSO content) · **T1.5** §5.109's
  layer half · **§5.53(b)** (T1.9, one token) · ~~the **T5.1 rehearsal**~~ **[RAN 2026-09-05, F90 §11.211 — "ready" now waits on T5.2 alone]** · and the
  **decision-free queue R27/R28/R18/R19/R20/L1 just created** (§11.207(g): ~~§5.86+§5.19~~
  **[DELIVERED 2026-09-06, F91 §11.213]**, ~~§5.98~~ **[DELIVERED 2026-09-06, F94 §11.214]**,
  §5.21, §5.66+§5.71, §5.115, A15's residual). The final-pass SEND is no longer on
  this list: it fired, and came back — **nineteen of twenty answered, L2 alone open**.
- **Newly NOT necessary, so the shrinkage is visible**: A43's data regeneration
  (**cancelled** — R29 says it is design, and doing it would destroy the effect) · the
  §5.74/§5.78 search family (**de-prioritized** by R22 — still open, no longer on any
  path) · §5.90's fix as a *loader* question (R23 makes the default correct; what is left
  is the undocumented procedure, moved to R0 above).
- **Newly necessary, and small**: **§5.135** (the old path's heading-coupled offset, minted
  by R28) — but its fix is NOT decision-free, because it lands on the comparison baseline;
  it sits behind an owner call, not behind work.

## R — Reference for development (the second criterion, 2026-09-05)

**R0 — The criterion** [vixy 2026-09-05, quoted above]: a junior developer clones THIS branch in
one week and develops on it; no work continues on the deployed line that would later need
porting. Derived requirements, each with its state MEASURED on 2026-09-05:

1. ~~**R1 — `master-beta` contains the deployed line.** Local `2023-master` (`194c6074`,
   2025-09-20) ⊂ `master-beta`. `origin/2023-master` = `6ec2f43f` (last fetched 2026-08-03 —
   no fetch is possible from this host, publickey refused) is 15 commits ahead by SHA and
   **2 by content** (`git cherry`: Kenan-Blasius's video-as-`s_texture`, `c69687bc`+`6ec2f43f`,
   four files under `src/tools/`, all ASCII); a trial merge conflicts in 4 hunks, every one
   "keep ours" (F70 ASCII ×2, B31 `SC_SESSION`, F62 `div/mul/mod`); version strings equal
   (2026.07.11 — §5.112 silent); submodule pin ours-advanced only. → **F83**. Residual, Vixy's:
   the fetch is a month stale — re-fetch + re-merge from a keyed host before the developer
   clones.~~ **[MET 2026-09-05 by F83 → §11.203; code merge `c6784490`, HEAD `03c85734`,
   binary `225f0d93`.** The four hunks resolved to ours exactly as measured, and proven so by
   a check that could have failed (markers deleted, not `git checkout --ours`, so the three
   files' `git diff 85cc2785` being empty is a result about the AUTO-merged remainder too);
   `git cherry` after the merge is ~~empty in both directions~~ **[supervisor correction at
   acceptance: empty in the FORWARD direction — 0 `+`, 0 `−`; the reverse direction lists
   `master-beta`'s own 260 patches by construction, §11.203(b)'s table has it right]**; the merge's stat is 4 files /
   480 insertions = the two commits' 476 + 4 to the line. Gates on the merged binary: full
   canary **12/12 in band, every delta 0.0**, dwell frame md5 unchanged across a third
   binary; D14 PASS (CONVERT 970 → 972, no partition edit); ctest 19/19; corpus 4014/0/0.
   The anchor gate reded and was right — two citations of `s_texture.hpp:294-298` moved +5
   to 299-303, explained and re-recorded (not re-pointed: the pin is a commit).
   **THE RESIDUAL IS SMALLER THAN THIS ROW SAID:** a read-only https `ls-remote`
   [measured: supervisor, 2026-09-05 11:05] returns all three remote tips byte-identical to
   the 2026-08-03/04 fetch, so nothing was pushed since and this merge covers the ENTIRE
   deployed-line delta. The re-fetch/re-merge half is DISCHARGED; what is left of R1 is
   **R5, the push, and nothing else** (80 code / 645 harness commits unpushed; https READ
   works from this host, SSH and push do not).**]**
2. ~~**R2 — A clean clone builds, installs and runs by the documents.** INSTALL is the
   source-ZIP + Windows/VCPKG text and never says `--recurse-submodules`; `install_src.sh:22`
   leaves `CMAKE_BUILD_TYPE` EMPTY by a one-character inversion of its own stated intent; both
   install scripts `sudo` into `/usr/local`; the config is app-generated from `checkConfig`'s
   schema (`data/default_config.ini` is 3 lines; the field's is 315 lines / 266 keys / 0
   comments); §5.48 fires on a cold HOME at ~15 % (EntityCore). → **F84** (measure, manifest,
   docs, §5.112's number, §5.48's rate); §5.48's FIX is Vixy's.~~
   **[MEASURED END TO END 2026-09-05 by F84 → §11.204, and R2 is NOT MET. Two blockers, each
   one owner act away, neither visible before the measurement:**
   **(i) §5.131 — `git clone --recurse-submodules` FAILS.** `upload-pack: not our ref
   7ce58350`. https READ works; the PINNED OBJECT is missing. The pin is one LOCAL UNPUSHED
   commit (§11.152's ASmooth fix) whose parent IS the remote's `main` tip; `git branch -r
   --contains` is empty. `install_src.sh:24`'s `||` fallback fails too (rc 128) and the line
   has no `|| exit`, so `cmake` returns 0 and the build dies at `atm_ext.cpp:1:10
   EntityCore/Core/VulkanMgr.hpp`. **The remedy is one `git push` in the EntityCore
   repository — a DIFFERENT repository from R5's, so R5 does not cover it.**
   **(ii) §5.130 — the first launch on a machine with no `~/.spacecrafter` ABORTS**
   (`filesystem_error: cannot set current path`, exit 134, empty HOME): `main.cpp:193` cds
   into the directory five lines before `:198` creates it. The FIELD binary reproduces it
   identically, so it is shipped behaviour (`da858612`, 2025-09-20). One line moved.
   **What F84 DID close:** `install_src.sh` line **25** (not 22 — 21 is a different variable
   whose test is already correct) `-n` → `-z`, measured both ways (`CMAKE_BUILD_TYPE=` EMPTY ⇒
   `CMakeLists.txt:95-99` FORCES **Debug**/`-Og`, and the documented `BUILD=LocalRelease`
   override was being clobbered); `src/CMakeLists.txt:3` gains `CONFIGURE_DEPENDS` (arm A 0
   steps/0 refs/0 objects, arm B 1/13/1, cost +0.05 s per build); `INSTALL` rewritten
   Linux-first and `README` §2/§3 refreshed, including the two blockers, the sudo, the
   compile-time data root, and what the repository does not contain. Code `03c85734` →
   **`1cbd6780`**.
   **The manifest, which is R2's other half:** a tree install is **227 files** — binary, 214
   shaders whose aggregate md5 `e5043cf7` is IDENTICAL to the field's, 11 `data/` files all
   md5-equal — **and no content**: `stars/`, `textures/`, `icon.bmp` and nine class
   directories are absent; the tree has none of them, no repository document names them, only
   the ledger names `spacecrafter-data`. **And §5.132: a failed class copy prints "Completed
   copy of X" anyway** (`ec || ec.message()=="Success"` takes the success arm on failure,
   proved by probe), so a tree-only install creates ten empty directories silently.
   **`CONFIG_DATA_DIR` is a hardcoded `#define`** (`spacecrafter.hpp:44`), so
   `-DCMAKE_INSTALL_PREFIX` cannot relocate a deployment. §5.48 rated **0/6** with a control
   that fires (row still OPEN; its fix is Vixy's, and the binary measured has the ASmooth fix
   a newcomer's clone cannot reach). §5.112 priced — see T3.
   ~~**R2 therefore reduces to: the EntityCore push (Vixy) + the `main.cpp` line (Vixy).**~~
   **[2026-09-05, F86 → §11.205: the `main.cpp` HALF IS PAID.** Code `9e0f1e93`: the line moved
   below the two `checkUser*` calls, and the launch F84 could not make — a `$HOME` with nothing
   in it — now exits **0**, builds **19 directories and 7 files**, prints ten "Completed copy"
   lines and writes `log/spacecrafter.log` inside the new directory; the as-if control on an
   existing `$HOME` differs in exactly the three RAM-census lines an A/A pair of one binary
   differs in. **§5.130 CLOSED.** The EntityCore half moved too, but not to done: at **12:23**
   that day an external writer amended and PUSHED the submodule commit (`7ce58350` →
   `84f5d94b`, tree identical), so the clone now fails on a **stale pin** rather than on a
   missing push — the remaining act is `git add src/EntityCore` + commit in THIS repository
   (§5.131, §11.205(i)). **R2 therefore reduces to: bump the submodule pin (Vixy), and
   nothing else.** `INSTALL` section 6's `mkdir -p ~/.spacecrafter` paragraph is now obsolete
   and is named for F85 to delete, not edited by F86.]**]**
   **[R2's DOCUMENTATION HALF IS CLOSED 2026-09-05 by F85 → §11.206(d), code `d260d89c`:
   BOTH obsolete paragraphs are deleted — §6's `mkdir` AND §3's KNOWN ISSUE (§5.131's) — and
   neither was deleted on the strength of its row's flip: the clone probe was re-run (rc 0,
   submodule `84f5d94b`, header present) and an empty-`$HOME` launch was re-run (exit 0,
   19 directories + 7 files, real HOME md5 in==out). `INSTALL` §5, the "what the repository
   does NOT contain" section, is measured and kept. **Nothing of R2 remains on the
   documentation side; its pin-bump residual was discharged at code `32af2efb`.**]**
   **[R2's CONTENT QUESTION IS ANSWERED, AND THE ANSWER OPENS AN OWNER ITEM — 2026-09-05,
   round-3 **R23** (F88 → §11.207): *"By default, only limited catalogs are loaded. Correct
   catalogs are loaded in an outside installation procedure."* `[stated: tester (Lionel
   RUIZ), via owner commit 6ffb017]`. F84's manifest found that a tree install ships no
   content and that **no repository document names where content comes from** (§11.204(f));
   R23 says where: an **installation procedure outside both repositories**. So the gap is
   not that the loader is wrong or that the install is broken — it is that the procedure
   is **undocumented**, which for a newcomer cloning this branch is the difference between
   26 561 stars and the sky. **Its location, and whether it may be written down here, is
   the OWNER's — R6's sibling, and the third residual F85 left at §11.206.** Promoted into
   the R0 head above: it is the only R0 item this round added.]**
3. ~~**R3 — An entry document exists in the code repo.** None does (`doc/` is user-facing; no
   `*.md` under `src/experimentalModule/`); `claude/README.md`'s own filing criterion states the
   promotion obligation and nothing was ever promoted under it. → **F85**.~~
   **[MET 2026-09-05 by F85 → §11.206; code `d260d89c` + `a2fd3c5b`. `doc/developer-entry.md` — 424 lines,
   pure ASCII, nine sections, **78 claim-bearing sentence groups / 78 sourced** (source table
   §11.206(b)) — is the FIRST thing ever promoted under that filing criterion. It points and
   never duplicates: the harness README, INTENT.md's header, `INSTALL` and
   `util/scedit/README.md` are cited, not copied. Which path draws by default is MEASURED, not
   recalled, and the answer is not in `config.ini` at all (§11.206(c)). `README` section 2
   gains one line pointing at it; `claude/harness/f85_links.py` holds its citations to
   **129 paths / 25 continuations / 15 ledger ids, 0 dangling**, shown able to fail on one
   injected fault per class — so the document cannot rot silently, which is the property that
   makes it a REFERENCE rather than a snapshot. Three residuals, all recorded not guessed:
   the **I1–I7 principles are the owner's text and exist in neither repo** (§9 is a
   placeholder naming him — the one thing here he must still write), no human authorship
   convention is stated anywhere, and R23's outside installation procedure is undocumented.
   Name and placement stay veto-open; 424 lines against a 400 target is reported, not
   absorbed (§11.206(i)).]**
4. ~~**R4 — No memory-unsafety reachable from the shipped data at startup.** §5.127(2) the UAF
   from `anchor.ini`, §5.127(1) the comet null deref (old core) → **F86**; §5.48 (EntityCore)
   → Vixy.~~ **[MET FOR THE OLD-CORE HALF 2026-09-05 by F86 → §11.205; code `a4a7c226`.**
   **§5.133** (the UAF, minted from §5.127(2) before being touched): REPRODUCED under ASan at
   the predicted read — `orbit.hpp:36` ← `AnchorPointOrbit::update:44` ← `AnchorManager::update:306`,
   after one shipped command `camera action switch name orbit_autour_lune` — and CLEAN on the
   same launch after the fix, which is an ownership statement (`AnchorPointOrbit` holds the
   `unique_ptr`), not a guard. Two things the row did not know: a plain startup is SILENT
   because only the current anchor is updated, and the RELEASE binary takes that command
   without crashing, so the corruption is silent on the shipped build. **§5.134** (the comet
   null deref): the experimental reader's guard ported with a §2(f) diagnostic; pre-fix
   SIGSEGV, post-fix exit 0, control loads pre-fix — and **five of the six shipped
   `comet_orbit` sections run that branch on every launch**, surviving only because
   `parent = Sun` resolves. Gates: full canary 12/12 in band, every delta 0.0, dwell frame md5
   unchanged across a fourth binary. **RESIDUAL, and it is not ours**: §5.48 (EntityCore, the
   ASmooth NaN) is still Vixy's, so R4 is met for everything this repository owns. One gate
   red is recorded at §11.205(g) — `b4_anchors`' screen-witness CONTROL — bounded to the star
   field's brightness channel by the Moon's centroid being bit-identical in all five runs.]**
5. **R5 — The branch is on the remote.** ~~+63 code / +638 harness commits unpushed~~
   **[RE-MEASURED 2026-09-05 after F83: **+80 code** (the 63, plus the 15 SHAs the merge
   makes ancestors, plus F83's 2) **/ +645 harness**]**; GitHub
   refuses publickey from this host. → **Vixy** (push from the laptop, or land the key here).
   **[2026-09-05, §11.203(i): R1's residual now lives HERE and nowhere else. A read-only
   https `ls-remote` [measured: supervisor, 11:05] returns `2023-master` `6ec2f43f`,
   `master-beta` `76ee38c7`, `CC-harness` `eb9af25c` — all three byte-identical to the
   2026-08-03/04 fetch, so nothing has been pushed by anyone since and there is nothing
   left to re-fetch or re-merge. https READ works from this host; SSH and push do not.
   This is the one act nothing here can substitute.]** **[2026-09-05, F84 §11.204(b) →
   §5.131: it is TWO pushes in TWO repositories — this one (both branches) AND
   `Calvin-Ruiz/EntityCore` (`7ce58350`, §11.152's ASmooth fix, onto `main`); without the
   second no `git clone --recurse-submodules` of this branch builds. **[2026-09-05 12:23,
   measured mid-run by F86, §11.205(i): the SECOND push HAPPENED — the commit was amended
   (`7ce58350` → `84f5d94b`, tree identical) and pushed by an external writer. It does not
   fix the clone by itself: the pin in `master-beta` still names the amended-away SHA, so the
   act that remains is a pin bump in THIS repository, not a push in that one.]** Supervisor's reading
   at F84 acceptance: the `main.cpp` half of R2's residual is NOT the owner's — §5.130's
   one-line move is decision-free (§5.79/F35 precedent) and goes to F86.]** **[2026-09-05
   14:30: the EntityCore push HAPPENED (from this checkout, 12:23 — a key this session does
   not hold; the owner's hand or his other session) and the pin bump followed (`32af2efb`);
   the clone probe passes. R5 is back to ONE act: push `master-beta` (~~81~~ **86** code commits at the session-22 close, measured) and
   `CC-harness` from a keyed host — or from here, since a key evidently exists on this
   machine for some session; the supervisor never pushes.]**
6. **R6 — Branch policy.** Three developers committed to `2023-master` in 2026 (Kenan-Blasius,
   Lionel, Calvin). For `master-beta` to be the reference, either it becomes the PR target /
   main, or the others are redirected to it. → **Vixy**, one sentence; nothing here can decide
   it. Also his: the developer's platform (if Windows, the vcpkg/`install_src.bat` path enters
   through R1 untested here) and whether the harness repo is meant to be readable by him
   (F85 points at it either way).
   **[ANNOTATED 2026-09-05 — THE POLICY HALF IS ANSWERED, AND THE ROW IS NOT STRUCK BECAUSE
   THE ACT HAS NOT HAPPENED.** [vixy 2026-09-05, verbatim, in-line at the round's trigger]:
   *"The master-beta will became the reference and get renamed main once ready."* So of the
   two readings this row offered, it is the **RENAME**: the branch becomes `main`, and
   Kenan-Blasius, Lionel and Calvin are **not** redirected. The rename is the OWNER's act and
   nothing here prepares it beyond a measured footprint — **F92 → §11.212**: all 226
   occurrences of `master-beta` in both repositories partitioned (**4 LIVE POINTERS · 133
   PINS · 89 HISTORICAL RECORDS · 0 CONVENTION**, stamped at code `0b46a63f` / harness
   `58f10f6`, instrument `harness/f92_census.py`), an unapplied patch covering exactly the
   live class (`harness/artifacts/f92/rename-live-pointers.patch`, `git apply --check` rc 0
   in a scratch worktree of each repo, post-apply LIVE 0), an ordered eleven-act checklist
   with a check per act, and a breakage scan whose one finding is `supervised-by.sh`'s silent
   branch→HEAD fallback. **[THAT FINDING IS CLOSED 2026-09-07, F103 → §11.224:
   the fallback is gone — an unresolvable trailer branch is now a §2(f) STOP at the preview,
   exit 1 even under `--dry-run`, with `--branch-alias=<old>=<new>` as the repair channel; so
   act 1 (`git branch -m`) and the tool are now order-INDEPENDENT instead of silently
   order-dependent, and the checklist needs no reordering. Measured cost of the old behaviour
   at range scale: 138 dangling trailers reported against a true 2, in silence. TWO FURTHER
   pre-existing defects, found by running the tool end to end for the first time, had made it
   UNRUNNABLE on this pair in either order and are fixed (§11.224(f)(g)). ONE ITEM IS NOW THE
   OWNER'S, BEFORE the rewrite and irreversible after the push: `git commit-tree` drops
   `gpgsig`, so the rewrite removes `cebebf44`'s signature and collapses a 13-commit duplicate
   chain — `master-beta` 3829 → 3816 commits, while the script's content assertion keeps
   passing because the tip tree is unchanged (§11.224(h), two readings written out).]** **The gate the sentence names, *"once ready"*, is the map's own:
   T5.1 ran 2026-09-05 (F90, §11.211), so it is T5.2 alone.** **RESIDUAL, still Vixy's and
   NOT answered by the line:** the PR target. `doc/developer-entry.md` says pull requests
   target `2023-master`; the rename makes the branch's NAME false there and says nothing
   about that half, so the patch changes the name and leaves the target — one sentence, his.
   The two riders above (the developer's platform, whether the harness repo is his to read)
   are untouched by the line as well.]**

Not required for R0 (challengeable): tester transparency (T0's tiers), the final pass, any T1
decision — a developer can work on a branch whose tester-facing divergences are still open,
provided they are RECORDED (they are: §5, T4).

**T0 — The criterion** [derived from the ask + D15(b)'s transparency language]: the tester
runs his existing workflows — the scripts and data he authors, TUI/keyboard operation,
search/select/readouts, saves/config, multi-hour shows — on a deployment where the NEW path
draws, and observes nothing he must adapt to; wherever behavior deliberately differs, he was
INFORMED first (the final-pass revise/revert offer, §11.116(c)). "Restricted" = his install,
his data, forward-corrected delivery (D9).

## T1 — Decision gates (Vixy's; nothing below them can close these)

Ordered by operational weight for the tester, not by age:

1. **§5.100 + §5.101 — `zoom auto in` / `zoom auto initial`** (asked 2026-08-26,
   unanswered): the two paths end **99° apart** after a shipped unzoom; `zoom auto in`
   starts tracking on the OLD path only. These are bread-and-butter operator commands —
   the single largest transparency hole with a one-line-class fix already scoped.
2. **§11.92(d) — the heading≠0 × offset≠0 coupling (B17 residual, the TILTED-DOME
   question)**: old rolls the view offset with the heading, the new path keeps it
   dome-fixed. Planetarium-geometry-central (tilted domes are the deployment reality);
   blocks the §5.66 `look_at` family and §5.71's `panView` port. NOTE: per §11.161(c) the
   *expectation* half ("what should a tilted-dome operator see") may be tester-routable. **[2026-09-02, §11.198(d): expectation half ROUTED — round-3 R28, NARROWED to the heading×offset combination (heading = one of the owner's two named under-exercised configurations); plain-offset behavior stays old-parity by the owner's silent-population bounding; aim/draw coupling defect candidate recorded, not minted.]** **[2026-09-04, F81 §11.201: the candidate is MEASURED and **§5.128 is MINTED** (record-only) — old's aim site compensates a FIXED 90° while its draw applies fov/2, so a mid-show `set zoom_offset` lands the aimed body 0.0000°/13.5000°/21.0000° off the drawn centre at fov 180/90/40 and throws the old view 29.4474° onto `init_view_pos` while the new path keeps its aim. R28 gains the question it could not yet ask: does a show that sets the offset mid-show EXPECT the view to return to `init_view_pos`? — the answer decides whether only the scaling is wrong or the whole aim half is.]** **[GATE CLOSED 2026-09-05 by round-3 R28 (F88 → §11.207) — [stated: tester (Lionel RUIZ), via owner commit 6ffb017]: *"The offset is screen dependant. The change of heading shouldn't shift the offset position. I saw that it does now but it shouldn't."* **The decision this tier existed to extract is made, and it is made in favour of the NEW path**: the offset is screen/dome-fixed, so the delivered shape was right and old's heading coupling is a defect — minted **§5.135** (old-path only; mechanism re-read at `navigator.cpp:324` then `:329`). **§5.66 and §5.71 UNBLOCK and move to work.** Two things stay: (i) §5.128's snap-back question — R28 does not mention it, so the fork it names is still undecided; (ii) §5.135's FIX is not decision-free, because correcting the old path means touching the comparison baseline (§11.52(b)) — an owner call, not a gate on the tester's transparency. **This row leaves the decision tier.**]**
3. **§11.96(e)(1–6) + §11.98(f)(i–iii) — the reach/visibility decoupling batch**: gates
   the B5 remainder = dso3d/tully/ojmMgr floors — i.e. the DEEP-SKY content classes on
   the new path. A planetarium show without its DSO layer is not transparent.
4. **§11.4's two numbered decisions** (RA zero point −90.0003° epoch-stable; origin
   observer- vs body-centred — origin sub-question tester-routable) → unlocks the §5.86
   fix (+ §5.19 folds in). **[2026-09-02, §11.198(b): decision (1) RESOLVED by delegation — equinox definition, catalog-rooted, residual attributed; decision (2) → round-3 R27.]** Until then the new path's RA/DE readouts for composed bodies
   answer in a scrambled frame — he reads coordinates professionally. **[2026-09-05: with (1)
   resolved, the §5.86 fix is DISPATCHABLE with the origin held old-as-spec (observer-centred)
   until R27 — the parity target §11.158(f) states (≤ 0.002° for 89/90 bodies); next round's
   first engine candidate, deferred behind tier R this round.]** **[GATE CLOSED
   2026-09-05 by round-3 R27 (F88 → §11.207) — [stated: tester (Lionel RUIZ), via owner
   commit 6ffb017]: *"The RA/DE must be the value from our position."* ⇒ decision (2)'s
   origin is **OBSERVER-CENTRED (topocentric)**, which is what old already computes — so
   the old-as-spec hold becomes a tester-STATED requirement rather than a default, and it
   is the strongest form this answer could take: he reads coordinates professionally, as
   this row's own sentence says. With (1) resolved by definition (§11.198(b)), **BOTH
   numbered decisions are answered and this row LEAVES the decision tier.** §5.86 + §5.19
   are fully decision-free and dispatchable at the stated parity target. **T1.4 CLOSED.**]**
   **[WORK ITEM STRUCK 2026-09-06 — DELIVERED by F91 §11.213, code `5a1e5749`. The fix this
   entry unlocked is landed and measured: **88 of 90 both-tree bodies print the same RA/DE
   string on both paths, byte for byte**, the two exceptions named (Eris, the trees' own
   1.198deg position gap; Puck, one arcsecond of declination = float32), and scored the way
   §11.158(f) scored it the max away from Eris is 0.002014deg on Deimos -- that entry's
   published target and its own body. **Decision (1) turned out not to be a decision at all**:
   the "-90.0003deg epoch-stable RA zero point" this item names is the `+M_PI_2` inside
   `ModularBody::getAxisRotation()`, i.e. the constant half of the surface fold §5.86 already
   said the readout omits -- there was never a zero point to choose, and §11.198(b)(1)'s
   "frame-construction defect, not a choice" is confirmed at the digit. Decision (2) did real
   work: R27's topocentric answer is why `ModularObject::getEarthEquPos` moved to the
   observer-centred authority, which also repairs `set home_planet selected` (it feeds that
   value straight back through `earthPosEquToHelio`). Two further defects in the same member
   were measured and fixed on the way -- the local hour angle was built from the observer's
   LATITUDE, and a bare `fmod` printed all three hour angles negative at any pre-J2000 date.
   §5.86 and §5.19 both FIXED. Left NAMED, not chased: `observedToLocalPos` still divides by
   the offset-free rotation while its input carries the B17 pitch (the alt/az half; a
   photometric surface, out of F91's canary). **[PAID 2026-09-06, F96 §11.216, code `24100461`: `observedToLocalPos` is
   `renderViewRotation()ᵀ` now, so the alt/az readout, the atmosphere's sun direction,
   `moveEyeRel` and the tracking feedback read ONE expression. Measured with
   `set zoom_offset 0.3` armed: old-vs-new alt/az **27.000006873°** before, **0.000017139°**
   after, the Sun with them; byte-identical at every shipped default, and the FULL canary
   this item was waiting for is **exit 0, 12/12 in band, dwell frame md5 `5215565b`
   unmoved**. §5.138 FIXED; the §5.86 pole rider closed in the same commit.]**]**
5. **§5.109's layer half** — what `moveto alt` means above a display-scaled body (drawn
   vs physical surface). He authors scenes on scaled bodies.
6. **The script-semantics batch** (both-paths defects his authoring will hit; each is a
   one-liner-class fix behind a semantic call): §5.64/§5.76 (pause doesn't hold the
   clock / time runs backward), §5.65 (lock-after-move latch), §5.69 (`keep_time` 8-bit),
   §5.70 (unreplayable ramp recording — respell constrained by `delta_alt`), §5.72
   ($LOGON), §5.75 (trail drops on date jumps), §5.82 (`transition_to point name`
   dropped), §5.85 (`align_with` doesn't align), §5.87 (`constellation_star` acts on
   previous selection), §5.91/§5.93/§5.94/§5.95/§5.96 (script-surface family incl. the
   recorder diverging from the author's text), §5.53 (colour-map level jump), the ≥1023 B
   truncation-marker policy (§11.138).
7. **Forks whose owed data is now PAID, awaiting the call**: §5.88 (catalogue-load
   reporting — only caller-check names file+key), §5.89 (the one-site dead guard, must
   cover the mid-session route), §5.90 (stars.ini pairing + is `~/.spacecrafter/stars/`
   a search path — tester-routable per §11.161(c1)), A40 (quit vs incomplete frame),
   A41/A42/A43 (early-visibility gate px / texture-level switch / preview-asset data —
   A43 is a DATA regeneration, i.e. the paid product) **[A43 GATED on R29, 2026-09-02 — §11.198(c): regeneration may destroy the tester's authored two-skin design]** **[~~A43~~ **CLOSED AS DESIGN 2026-09-05** by round-3 R29 (F88 → §11.207) — [stated: tester (Lionel RUIZ), via owner commit 6ffb017]: *"It's a design."* ⇒ the Sun/Moon preview mismatch is the tester's AUTHORED two-appearance mechanism, so **the regeneration is CANCELLED, not deferred** — the paid `spacecrafter-data` product loses this item, and doing it would destroy an effect he uses. A43 leaves §13.A (17 → 16). **A42 does NOT close with it**: R29's own text said the answer would decide at what distance the swap should engage, and that number was not given — A42 is re-shaped from an align-or-leave engineering call into a DESIGN PARAMETER the tester still owes. **§5.90 also moves here** by R23/R24 — see T3; its loader half stops being the question and the undocumented outside procedure becomes it.]**, A44 (ring shadow contract),
   §5.106 (free-flight environment: close-as-accepted vs design question), the two
   §11.144 riders (free-flight `moveto` meaning + `get status position` — defaults live,
   confirm or redirect), §5.108 (`flag_sun_scaled` dead — reviving it is a behavior
   change).
8. **B31 completeness residue**: C4's non-body catalogue key (D34's unanswered half) +
   D30's DELTA branch for `display_scale` (located, unimplemented). Session save/restore
   is otherwise functionally complete (T1 met, T3/T4/T10 green).
9. **§5.53(b) — the big-texture level gate: old swaps at 180 px, the new path at 409.6 px**
   [PROMOTED here 2026-08-29 by F47 §11.163(g); this map's compile did not carry it].
   **The triage set's only NEW-PATH-SPECIFIC *visible* divergence**: on the two shipped
   bodies that have preview assets — the **Sun and the Moon**, which is to say the two he
   shows most — the disc visibly changes colour, and it does so at 2.28× the distance it
   used to. The row's own recommendation is on record (align to 180 while both paths
   exist) and the gate is one token: `BODY_BIG_TEXTURE_BOUNDING_SIZE = 409.6f`, kept
   deliberately separate from `BODY_CLOSE_RANGE_BOUNDING_SIZE` precisely so this question
   stays answerable. Vixy's because engaging the big texture earlier means more VRAM
   resident sooner (D5/D6 residency, D10 headroom). Its sibling **§5.53(a)** is a DATA
   decision — two shipped previews are photometrically inconsistent with their
   full-resolution partners — riding `spacecrafter-data` forward propagation (D9).
   **[(a) RE-GRADED, (b) SHARPENED, 2026-09-05 by round-3 R29 (F88 → §11.207) — *"It's a
   design."* `[stated: tester (Lionel RUIZ), via owner commit 6ffb017]`. **(a) is no
   longer a data decision**: the two previews differ from their partners because he
   AUTHORED them that way, so the D9 forward-propagation item is withdrawn. **(b) is
   unchanged in substance and stronger in argument**: the gate is still the
   new-path-specific visible divergence on the two bodies he shows most, and R29 makes
   aligning it MORE consequential, not less — the swap distance now carries a designed
   effect, so moving the gate moves the effect. Still one token, still Vixy's (D5/D6
   residency, D10 headroom), and now also waiting on A42's unnamed number.]**
10. **§5.35 / ~~§5.98~~ / §5.41** [TRIAGED here 2026-08-29 by F47, all BOTH-PATHS so none gates
   T0, each behind a named call]: `day_key_mode` — what should the control DO (today it
   neither sticks nor acts) · `$body_selected` answers 999 for **Saturn and Ganymede**,
   two typo'd spellings, gated on SS-17's owed answer from the script-surface owner
   (**tester-routable**, §11.161(c) — he authors `struct if body_selected equal 600`) ·
   `camera action save`, gated on B31's re-expression decision.
   **[§5.98's GATE IS OPEN 2026-09-05 — round-3 R19 (F88 → §11.207): *"Satun must be
   corrected to Saturn and Ganymed to Ganymede."* `[stated: tester (Lionel RUIZ), via
   owner commit 6ffb017]` ⇒ the routing worked and the fix is AUTHORISED, two spellings,
   decision-free; SS-17 answered in `SCRIPT_SURFACE.md`. **He answered with the
   instruction, not with the asked question** — whether any show depends on today's 999
   is still unknown, and is recorded as unknown rather than assumed absent. `day_key_mode`
   (§5.35) and `camera action save` (§5.41) are untouched by round 3.]**
   **[§5.98 IS STRUCK FROM THIS ITEM 2026-09-06 — FIXED by F94 §11.214, code `a2a880ef`,
   binary `404b9e89`: `"Ganymed"` → `"Ganymede"` (`core.cpp:2251`) and `"Satun"` →
   `"Saturn"` (`:2255`). `$body_selected` answers **503** and **600**, measured pre/post on
   the shipped command surface with the predictions committed before the first launch and a
   control body (Titan = 604) firing on both binaries. **And the unknown this item names —
   *whether any show depends on today's 999* — is MEASURED for this field rather than left
   unknown**: 1 of 408 shipped scripts reads the variable, tests it against 0, and takes the
   same arm on both binaries. T1.10 now carries `day_key_mode` (§5.35) and `camera action
   save` (§5.41) only; both are still behind their named calls. One item goes OUT to the
   tester with the fix: `doc/superscript.sts:1529` `Ganymed=503` → `Ganymede=503`, his file,
   routed at SS-17 and not edited here.]**
11. **§5.113 + §5.110's fix routing** [ADDED 2026-08-30, F50 §11.166; both-paths at the
   mechanism (the uninitialized `Object` singleton predates the split) ⇒ NOT T0-gating]:
   ONE missing-guard class, three shipped reaches with nothing selected — `set
   home_planet selected` teleports the observer 1 AU and caches the fiction under the
   empty name · `flag object_coordinates on` draws a live-looking readout for nobody ·
   `illuminate hp <absent>` feeds INDETERMINATE memory into the grid. The class question:
   where does the truthiness test belong — each read, the singleton (fail loudly), or
   both (I6). §5.110's own fork rides the same sitting: three contracts, and the live
   check proved the composed-selection answer INDISTINGUISHABLE from nothing-selected
   (so diagnostic-only repair cannot restore discriminability). Rider, listed on §5.110:
   `$body_selected` is additionally STALE (answers the released body) after `deselect` —
   the doc's own contract sentence is broken; same repair sitting.

## T2 — Work, dispatchable now or upon its T1 gate

- **§5.111** — wrap the new path's info strings in `_()` (parity restoration; the tester
  operates in FRENCH; decision-free candidate, next round). **[MINTED 2026-09-05 as F87, the
  session-22 extension member: old's msgids byte-exact, unmatched labels listed not invented.]**
- ~~**§5.110** — the owed live check (one script), then the type-filter fix routing.~~
  **[PAID 2026-08-29, F50 §11.166: live check RUN — six of six predictions matched, and
  the composed-selection answer is CHARACTER-IDENTICAL to nothing-selected (1 AU, mag −10,
  vernal point: plausible, not error-shaped) while the app distinguishes the two in the
  same frame. What remains is the FIX ROUTING → T1 (three contracts, one changes what a
  shipped variable means). NEW sibling **§5.113** minted en route → T1 below.]**
- **§5.49's owed render measurement** (`f14_meridian.py` `u_sub`) — settles whether
  `moveto lon 0` stands over Greenwich, the map centre, or 90° off; the row's conclusion
  is recorded in-doubt (§11.153(k)). DATA-AUTHOR-CENTRAL: he places content by lon/lat.
  Measurement is S and unblocked; any fix is Vixy's.
- **Composed-body i18n selection asymmetry** (§11.158(d5)): `select object <translated>`
  cannot reach composed bodies while `select planet <english>` can — measure the
  consequence for a French-locale operator, then route.
- ~~**§5.41/§5.42** ⚠ — … Rows not re-read this session — triage first.~~
  **[TRIAGED 2026-08-29, F47 §11.163: both mechanisms present at `d6aec251`. **§5.41 → T1,
  B31-gated** (B31 §3.4(d) wants the surface re-expressed on the session serializer, not the
  prefix patched; `~/.spacecrafter/anchors` still absent ⇒ still always fails). **§5.42 → T1 ×2,
  unchanged — plus a SCOPE CORRECTION that spawned NEW §5.112**: its "unknown key preserved"
  was measured on the COMMAND path at a matching version; a version MISMATCH runs
  `checkUselessKey()` and DELETES unknown keys through the same writer, with no user action.
  See T3. Both rows are BOTH-PATHS ⇒ neither gates T0.]**
- ~~**§5.20/§5.21** ⚠ — `linearOrbit` lerp weights swapped; `LocationOrbit` degraded spin.
  Authored-orbit-visible if real; rows not re-read this session — triage first.~~
  **[TRIAGED 2026-08-29, F47 §11.163: **§5.20 → NOT blocking** — not "dead code" merely by
  grep, it has NO CONSTRUCTION SITE ON EITHER PATH (absent from `modules.cpp:45-68`, no
  `protosystem.cpp` funcname), so it cannot be authored-orbit-visible. **§5.21 → T2, and its
  reach was UNDERSTATED**: the new path REGISTERS `LocationOrbitLoader` (`modules.cpp:49`),
  which builds the identical defective `LocationOrbit` ⇒ a BOTH-PATHS authoring trap, not
  new-path-superseded. Fix NOT decision-free (D9: authored `orbit_lat` may compensate);
  QUESTION ROUTED TO THE TESTER, joins the final-pass batch.]**
  **[ANSWERED 2026-09-05 — round-3 R18 (F88 → §11.207): *"No."* `[stated: tester (Lionel
  RUIZ), via owner commit 6ffb017]` ⇒ no `location_orbit` body was ever authored, so the
  D9 objection that made this fix non-decision-free is EXCLUDED and **§5.21 is
  decision-free**. Its both-paths reach is unchanged, so one fix serves both.]**
  **[PARTLY DELIVERED 2026-09-06, F97 §11.217 (code `24100461` → `22499f04`): the
  LATITUDE half is fixed on both paths at the one constructor both loaders call
  (`lat(_lat*M_PI/180)`; measured 58.31008° → 45.00000° on both halves of a live dump),
  and the double-spin trap now carries an §2(f) signal (0 lines pre / 1 post). **§5.21
  STAYS ON T2**, and the "decision-free" above is REFUTED at its own target: R18 did
  remove D9, but `surface_point` + grounded — the structural home the triage points at —
  lands **exactly 90° east** of the planetographic longitude old's `AnchorPointBody`, the
  camera's placement and `moveto lon` all mean (the `+M_PI_2` of `getAxisRotation()`;
  measured both ways in one run and offline over 64 states), so "exact against old" and
  "lands where `surface_point` + grounded lands" cannot both hold. A THIRD defect is named
  with it: the class never applies the parent's `getRotEquatorialToVsop87()`, **86.306371°**
  off old's own authority on Mars. Both readings at §11.217(d); the remainder is S once
  the question is answered and undispatchable until it is. NOTE for T-planning: the 90° is
  `surface_point`'s, a RATIFIED key (D16–D19), so this touches the authoring grammar.]**
- **§5.107** — extent-cache latch (5th member of the closed latch class; the fix shape
  exists).
- **§5.71 + §5.66** — the `panView` port + `look_at` halves (after T1.2).
  **[UNBLOCKED 2026-09-05 — T1.2 closed by round-3 R28 (F88 → §11.207): the offset is
  screen/dome-fixed, so both port under the NEW path's convention and neither waits on a
  decision any more. One caveat carried, not absorbed: §5.66's OWN owed item — which
  landing an operator typing `look_at azimuth X altitude Y` means, the drawn bodies' or
  the drawn sky's — is a DIFFERENT question and R28 does not answer it.]**
- **B35/B36/B37 residues** — config-only / declared-but-driverless / UI-only capability
  audits: they BOUND what "the new path" can express; completing them completes the
  transparency claim's denominator.
- ~~Already-queued hygiene (not tester-facing): b3_ladder's `922701c9` check · the five
  §11.156(g) back-markers · the §11.161(f) stratigraphy validation.~~
  **[ALL THREE DISCHARGED 2026-08-29/30, session 15: F48 §11.164 (the drift is dated
  ENVIRONMENT — neither product nor harness; a driver bump changed the rendered
  photometry, see T3's hardware note) · F49 §11.165 (arrears paid, the class's extent
  corrected 5→4) · F52 §11.168 (the strata ORDER measurably from git under window +
  author controls; the spike clause refuted — premise rework lands add-alongside).]**

## T3 — Verify at HIS field (restricted-deployment-specific; mostly final-pass cargo)

- **Field-content family on HIS install** (tester-routable, ratified §11.161(c1)): does
  his deployment carry sky-culture content (§5.74 — search currently returns NOTHING on
  our 2922×0-byte field), stellar_systems, the full star catalogues (§5.90 — our install
  silently runs 26 561 stars instead of millions; his stars.ini↔catalogue pairing)?
  **[ANSWERED IN THREE PARTS 2026-09-05 (F88 → §11.207), `[stated: tester (Lionel RUIZ),
  via owner commit 6ffb017]` — and the answers MOVE the question rather than close it.
  **R22** *"Search is deprecated."* ⇒ the sky-culture/search half is **DE-PRIORITIZED**:
  §5.74 and §5.78 stay OPEN with no fix owed, and leave every deployment path. He answered
  a larger question than the one asked — whether search works on HIS install is still
  unknown, and now uninteresting. **R23** *"By default, only limited catalogs are loaded.
  Correct catalogs are loaded in an outside installation procedure."* ⇒ **our 26 561 stars
  are the PRODUCT's default, not a fault of our install**, so §5.90's headline reading is
  withdrawn — while its MECHANISM (list from the user dir, files from the data root,
  mismatch silent) is untouched and stays OPEN, and **§5.88 gains weight**: if a limited
  catalogue is normal, *"a missing catalogue is reported NOWHERE"* is what every site
  experiences, with nothing to tell an operator which of the two he is running. **What
  becomes the live question is the PROCEDURE itself**: it exists, it is outside both
  repositories, and no document here describes it — R2's content question in its exact
  form, R6's sibling, §11.206's third residual, and now an R0 item. **R24** *"We should but
  for now it is in another directory."* ⇒ `~/.spacecrafter/stars/` SHOULD be honoured and
  is not used today: a stated want with no current dependent, so no D9 risk either way.]**
- **His content census** [NEW question for the final pass]: which content classes do his
  real shows actually load? This BOUNDS T1.3's urgency and whether B1/B2 (D4 streaming,
  RING asteroid, INSTANCED) block him at all — today they are assumed architectural-only.
  **[ANSWERED 2026-09-05, AND IT BOUNDS NOTHING AWAY — round-3 R21 (F88 → §11.207):
  *"All have been tested, but sometimes long ago, so maybe some features could have
  altered the way it shall work."* `[stated: tester (Lionel RUIZ), via owner commit
  6ffb017]`. **ONE sentence for the whole nine-line census** — nine lines were sent, one
  answer came back, and it is propagated as one answer and never as nine ticks. **Every
  class is IN**: sky cultures, deep-sky 3D, full catalogues, other star systems, videos,
  large body populations, runtime images/audio, joystick/hardware, AND the portrait
  window. So T1.3's urgency is NOT reduced, B1/B2's "unless the census says otherwise"
  exclusion no longer holds by absence (they stay excluded on the weaker, Vixy-paced
  argument), and §5.129's portrait cost (§11.202: the dome drawn 128 px low with a dead
  band above it) is a cost the field may be paying rather than a hypothetical. **The
  staleness caveat is part of the answer and must travel with it**: *tested*, possibly
  *long ago*, explicitly not *works today* — this reply bounds the SET of classes, and
  says nothing about the STATE of any of them. No class may be called healthy on it.]**
- **His hardware**: §5.60 (the unconditional 2.68 GB video staging allocation vs his
  GPU's limit), his real dome/projector stack vs our headless `:2` (every cadence and
  pixel baseline here is stack-local — §11.159(k7)), his `maximum_fps`.
  **[SHARPENED 2026-08-30, F48/F51 §11.164/§11.167: "stack-local" now includes the
  DRIVER — a host driver bump (`580.568.0 → 580.636.192`, no code change) made the
  shipped Moon render to a different image: mean ×0.371, a fifth of the disc newly
  below L=32, locally ×0.140, from bit-identical model state; not a lost upload
  (refuted on four channels), most plausibly the driver's shading. Consequence for HIS
  field: rendered APPEARANCE is a property of (binary, driver) jointly — appearance
  claims and luminance-based expectations do not transfer across driver versions, his
  included. Our corpus is mostly immune by construction (px>8 gates; the census is
  §11.167(i)) but any absolute-luminance expectation he holds is not.]**
- **His config migration**: the new keys (`attached`, `flag_lock_sky_position`,
  twin-emitted `display_scale`) measured D13-tolerant on both parser routes (§11.150) —
  re-verify on his actual config version. **[SHARPENED 2026-08-29, F47 §11.163(f) → NEW
  §5.112: there is nothing to "re-verify first". The next forward-corrected delivery bumps
  `SPACECRAFTER_VERSION`, and on that version mismatch the FIRST LAUNCH rewrites his
  `config.ini` by itself — no user action, no `configuration action save` — destroying every
  comment, lowercasing every key, and DELETING every key of a known section that is not in
  the schema table (`checkConfig.cpp:477`/`:518-520`/`:523-524`/`:604-606`). §5.42's measured
  "unknown key preserved" was the same-version command path only. Dormant here solely because
  build and file both read `2026.07.11` — which is why every launch's config md5 stays
  pristine. **This is a data-loss hazard on the delivery mechanism itself; he should be told
  before the delivery, not after.**]** **[2026-09-05: told — N7 in USER_QUESTIONS_ROUND3
  (DRAFT); the NUMBER (keys deleted from the field config on a version bump) is F84's to
  measure, predicted from `checkConfig.cpp`'s tables first. Measured today: the field config
  has 0 comment lines and 0 uppercase keys, so on THIS field only the deletion arm can bite.]**
  **[T3's §5.112 DATUM IS PAID 2026-09-05 by F84 → §11.204(j); the row carries the full
  record, N7 the sentence. Two launches, both predicted by name and count first, from a
  schema extracted mechanically (`harness/f84_config_predict.py`: 16 sections, 268
  `section:key` pairs). **On the installed field config verbatim, version bumped:
  DELETED 0, ADDED 2** (`navigation:attached`, `navigation:flag_lock_sky_position` — the two
  D15(d)/§11.150 keys `checkConfig.cpp:412-418` names in its own comment), 266 → 268 keys,
  16 → 16 sections, no removal line in the log. So the deletion arm CANNOT bite on THIS
  field: the file is app-generated and holds nothing off-schema. **On the same file plus the
  hand-authored content the warning is about** (two comments, an off-schema key, a mixed-case
  off-schema key, an unknown section with two keys): **DELETED 4 by name**, 17 → 16 sections,
  2 → 0 comment lines, all five removals logged as `(Warn.)` — **and both runs end at the
  SAME md5 `3465f7c8`**. The migration is a PROJECTION onto the schema, not a partial loss.
  **Consequence for the send**: the danger to the TESTER is proportional to how much he has
  hand-edited his own `config.ini`, and this map cannot know that — his file is not this one.
  N7 now carries both numbers so he can judge it himself. The T3 line above stays open only
  for "his actual config version"; the mechanism is measured.]**
  **[AND HE ANSWERED WITH A FIX DIRECTION 2026-09-05 — round-3 **N7** (F88 → §11.207):
  *"Put a # in front of the deprecated lines would be better"* `[stated: tester (Lionel
  RUIZ), via owner commit 6ffb017]`. Shown the two numbers, he does not acknowledge the
  hazard — he **exercises the revise/revert offer** and states how the mechanism should
  behave: **comment out, never delete**. That is the migration writer's spec (§5.42's
  writer rides the same change), and this clause's own measurement is its discriminator —
  under a comment-out the two runs would no longer converge on md5 `3465f7c8`. Recorded
  as the tester's EXPECTATION: he is a code contributor, but the fix's stratum ruling and
  its schedule stay the owner's (§11.161(c)). Nothing here authorises the edit.]**
- **The B14 data package** (poles/W0/periods corrections) riding the next
  `spacecrafter-data` delivery, forward-only (D9); his baselines shift accordingly.
- **His script corpus**: the shipped 434 are our proxy; HIS files are the real test —
  needs his cooperation (superscript.sts is already his own rewrite).

## T4 — Deliberate divergences he gets INFORMED about, not fixes (final-pass cargo, §11.116(c))

D15(a)–(d) INFORM ×4 (with the revise/revert offer) · D15(b) heading-stability CONFIRM ·
D37 as a QUESTION with its premise fact · A15 fade thresholds · the oort-SHADOW onset
item (state-stamped) · `transition_to body` keeps whole orientation (D28/A38 — the two
paths' images deliberately differ at that member) · the free-flight defaults (riders, if
Vixy confirms) · §5.106's environment semantics (if closed-as-accepted) · the field-content
questions (T3) · the content census (T3) · possibly §11.4's origin sub-question.
**[ADDED 2026-08-29, F47 §11.163(j)(4) — three members of the triage set belong in this
batch and the compile did not carry them]:** **§5.83** as an INFORM — `camera action move_to
… duration 0` NaNs on the old path and the new path guards it, a deliberate divergence (no
shipped script uses it; 3 files use the command, all with non-zero durations) · **§5.21** as
a QUESTION to the tester — *has anyone authored `location_orbit`, and was `orbit_lat` written
in degrees or tuned by eye?* (the fix is blocked on the answer, D9) · ~~**§5.98** as a QUESTION
to the tester — *do your shows test `body_selected` against 999 for Saturn/Ganymede?* (SS-17;
the fix is two spellings, free once given)~~ **[OFF T4's CARGO 2026-09-06: asked, answered
with an instruction (R19), and DELIVERED — F94 §11.214, code `a2a880ef`. The question itself
was answered by measurement for this field (1 of 408 scripts, same arm on both binaries) and
what goes to him instead is a one-character doc fix, routed at SS-17]**.

**[THE BATCH CAME BACK 2026-09-05 — F88 → §11.207; `[stated: tester (Lionel RUIZ), via owner
commit 6ffb017]`. This tier is DISCHARGED EXCEPT ONE MEMBER.]** Member by member:
**D15(a)–(d) INFORM ×4** — N1, N3, N4 drew no reply, which under this batch's own contract
(*silence = accepted*) means they stand as built; **D15(b) CONFIRM** — round-3 **R26**:
*"Stable heading when switching or at least change of camera orientation smoothly."* ⇒
confirmed, plus a fallback clause the question did not offer, which reinforces the
minimum-acceleration requirement rather than weakening the default · **D37** — round-3
**R14 + R15**: *"If we hide the star, it won't light the scene."* / *"It depends on the
ambient_light value."* ⇒ answered as option (2), ambient-valued; the DECISION closes, the
implementation is the owner's to schedule · **A15's fade thresholds** — round-3 **L1**:
*"It would be better to have no residual threshold"* ⇒ one of four axes judged (remove the
residual step); the threshold and band stay open · **the oort-SHADOW onset item** —
round-3 **L2**: *"I didn't test it yet"* ⇒ **THE ONE MEMBER STILL OPEN**, state-stamped,
carried to whatever pass comes next · **D28/A38** — **N5**: *"Just switch to the planet,
keeping the old values (that we will change by script anyway)."* ⇒ no revert; option (a)
stands, and the accumulating-tilt price is accepted because his shows set the framing by
script · **§5.83 INFORM** — silent, therefore accepted · **§5.21** — **R18** *"No."* ⇒
answered, D9 risk excluded · ~~**§5.98** — **R19** *"Satun must be corrected to Saturn and
Ganymed to Ganymede."* ⇒ answered, fix authorised~~ **[CARGO DISCHARGED 2026-09-06, F94
§11.214: authorised AND landed, code `a2a880ef`; nothing of §5.98 rides T4 any more. The
one thing that does go to him is a doc token, `superscript.sts:1529` `Ganymed` →
`Ganymede`, routed at SS-17]** · **the field-content questions and the
content census** — R22/R23/R24 and R21, see T3 · **§11.4's origin sub-question** — **R27**
*"The RA/DE must be the value from our position."* ⇒ observer-centred, T1.4 closed.
**Not carried in this batch and now on record**: the free-flight defaults and §5.106 were
never sent (they wait on Vixy), and **N7** — the config-migration warning — came back not
as an acknowledgement but as a **change request**: *"Put a # in front of the deprecated
lines would be better"*, which EXERCISES the revise/revert offer and gives §5.112 its fix
direction (see T3).

## T5 — UNMAPPED (the map's own edges; completeness > certitude)

1. ~~**No end-to-end tester-workflow rehearsal has ever run.** The A-D battery, b24_*, and
   scene harnesses are proxies built from OUR model of operation. The closing audit
   before "ready" is a his-day-in-the-app suite: author a body, run a show, search,
   select, save, reload, quit - one sitting, new path, French locale. CANDIDATE TASK
   (M), buildable now, sharpest after T1.1/T1.4. [2026-09-05: deferred behind tier R this
   round - next round's first slot; it doubles as the developer's smoke suite.]~~
   **[RAN 2026-09-05, F90 -> INTENT §11.211. It is `harness/f90_rehearsal.py` +
   `f90_rehearsal_run.sh`: one command, one launch on a private farm in the field's own
   French locale, nine operator steps, every step's observable and pass criterion printed
   BEFORE the launch, exit non-zero on any failure - and it is the line
   `doc/developer-entry.md` §5 now hands the newcomer (code `0b46a63f`). WHAT IT FOUND, in
   three runs that are step-for-step identical (zero flakes, 91.2 s each) plus one injected
   operator typo that reds two steps and exits 1: THREE divergences, every one pre-existing
   and none fixed. §5.77/§11.146's startup silences, reproduced exactly. §11.117(k)(3)'s two
   anchor bodies dropped by `body action reload` - and its open consequence question
   answered, both names still resolving and both owned bodies rebuilt on the next anchor use,
   so neither is a §5 candidate. And ONE NEW ROW, **§5.137**: a body an operator authors with
   `body action load` leaves the DRAWN path on `body action reload`, stays on the old one,
   and `search` / `get status object` keep answering for it in full - a false success by
   §5.79's criterion, record-only because the governing question is §11.55(i)'s, suspended
   for Vixy. It also confirmed F87 from the tester's seat: the drawn readout is French, all
   four labels `fr.txt` msgstrs, U+00A0 present. WHAT "READY" STILL WAITS ON: **T5.2 alone**,
   and this campaign says why that needs a different instrument rather than more of the same -
   five launches of ~91 s cannot see the stability class, and this suite's own quit measured
   0.6 s five times out of five.]**
   **[THAT SENTENCE IS SPENT 2026-09-06, F95 → §11.215: T5.2 has now run, so "READY STILL
   WAITS ON: T5.2 alone" is a past state, and what "ready" rests on is item 2's strike
   below plus the two limits named there (B30 is outside the soak; the `fscripts/` corpus
   is a second soak, owed). Two riders for this suite itself, both measured by F95: it is
   step-for-step IDENTICAL over **ten** consecutive runs on `404b9e89`, a binary F90 never
   saw (F90's three runs were on `407b3d1d`, since moved by F91 and F94), which extends its
   zero-flake record across two binary changes; and its `show_own_duration` is blind to
   `struct loop`, so any future caller that plays a looping show gets a budget that is a
   slice rather than a duration (`harness/README.md`, F95 section).]**
2. ~~**Multi-hour soak under show load** (the stability class: §5.61 lost wakeup, §5.59/A40
   teardown, B7's intermittent §11.15d segfault, B30 frozen-scene micro-instability,
   §5.62's unattributed epoch shift) — never run. A planetarium session is hours.~~
   **[RAN 2026-09-06, F95 → INTENT §11.215. It is `harness/f95_soak.py`: a DETACHED
   `setsid` driver owns one launch for H hours while every supervising call stays a
   foreground read, which is the only shape §0.5's no-`run_in_background` rule leaves —
   and it makes the campaign survive an executor abort, `status` being the resume point.
   TWO LEGS on binary `404b9e89`, **2.735 h + 3.004 h = 5 h 44 min**, the eight shipped
   `basis`/`custom`/`deepsky` shows round-robin with every authored pause RESUMED, 128
   complete cycles, 688 samples: **F1 DEATH, F2 HANG, F3 QUIT and F4 FROZEN FILE all
   NONE, in both legs.** Both detectors were shown able to fail first, on live controls
   (`kill -9` → F1 within one sample; `kill -STOP` → F2 at a measured 54.3 s against
   F19's 45 s bound, then recovery). **Four of the five class members did not fire**:
   the probe round trip never passed **208 ms** in 688 samples, `This frame stall is
   very long` measured **0**, the quits were **0.65 s and 0.61 s, exit 0, no teardown
   fault** — and with ten `f90_rehearsal_run.sh` launch/quit cycles that is **thirteen
   clean quits**, so §11.15d did not fire either. RSS is bounded, not leaking: it rises
   ~220 MB over ~14 cycles and then holds through 26 consecutive exactly-zero steps,
   with a residual 0.28–0.38 MB/h tail that three hours cannot separate from a very slow
   leak (named, not hidden). §5.62 did not recur at its own scale; what the pinned-clock
   instrument found instead is §5.84's seeded solver caught happening — four bodies of
   120, OLD path only, two values each, ≤ 9.437e-16 AU, with the NEW path bit-identical
   120/120, which is also the first positive test of §11.76(b)'s D8 use-site barrier.
   **THE FIFTH MEMBER, B30, IS OUTSIDE THIS INSTRUMENT AND STAYS UNMAPPED**: the soak
   reads no pixel. Two further limits, stated so "ready" is not over-read: the playlist
   is image-overlay shows, so it does NOT exercise body authoring — a second soak over
   the tester's own `fscripts/` corpus (`06old.sts` authors ~~3000~~ **170** satellites, §5.137) is
   named and OWED — and thirteen clean quits is a rate, not a proof against an
   intermittent class. **[BOTH HALVES SUPERSEDED 2026-09-06, F98 → §11.218: the number is
   **170** (measured; the corpus authors **1719** bodies in 8 shows once both word orders of
   the command are counted), and the second soak is **NO LONGER OWED — it RAN**: 4.508 h, 8
   complete cycles of 135 shows, 541 samples, **no F1/F2/F3/F4**, quit exit 0 in 0.76 s, 139
   real-HOME md5s in == out. It also found what the first soak could not: **two of the
   tester's own shows (`06.sts` then `14.sts`) abort the application** — uniform buffer pool
   exhausted → `Device lost while waiting frame completion` → `terminate`, SIGABRT,
   reproduced three ways **[SHARPENED 2026-09-07, F102 → §11.222 / §5.142: it is ONE show that
   empties the pool, not two. `06.sts` costs **1344 measured bytes per authored body** across the
   two paths against a **1 MiB** pool created once and never grown, so the first refusal is its
   **675th** body and `06.sts` alone accounts for 1355 of the 1557 errors; `14.sts` contributes
   the last 200 and the death, because its stars are `mode in_galaxy` OJM models (128 B, neither
   body path) whose refused allocation is reported as success and whose indeterminate offset is
   handed to the GPU as a dynamic offset. Read-only, no policy chosen: the three prices —
   grow to 2 MiB, refuse at the load authority, or shrink the 768-byte shadow-caster array — are
   at §11.222(h) and the choice is the owner's.]** — and **`14.sts` alone empties the OLD path's dump half** (246 → 1 →
   0 bodies) for the rest of the session while `select`+`get status object` still answers a
   full readout for bodies `search` reports NOF. The LEAK rule says **LEAK** on this corpus
   (68.3 MB/h, 42.4 MB/cycle over 8 cycles) where it said NO LEAK on the eight shipped shows,
   and §5.115's script log costs **193 MB/h** here against 1.88 MB/h there — 828 MB in one
   session, which is the field's "gigabyte-large script logs" made arithmetic. What is STILL
   unmapped: B30 (no pixel is read) and §5.137's own trigger `body action reload`, which
   appears in ZERO shows of this corpus.]** **"Ready" is the owner's word from here.**]**
3. ~~**B38's residual state** ⚠ (dead tokens + reachable-but-defective handlers) — the
   command-surface sweep's defect row; verify what remains open at the row.~~
   **[RESOLVED 2026-08-29, F47 §11.163(i) — NOT unmapped. State read: all EIGHT survivors
   present at `d6aec251`, the one closed member stayed closed, line drift only (plus `m_flags`
   and the obsolete list relocating to `app_command_init.cpp:111`/`:14-23`). It is an
   ENUMERATED eight-member batch, every member both-paths and every member behind a
   semantics-or-message decision ⇒ **moves to T1.6**, the script-semantics batch, and being
   both-paths it does not gate T0.]**
4. **Joystick/hardware UI path** (B37 territory) — no hardware here; untestable until his
   field.
5. ~~**Older ⚠ rows never re-read this session**: §5.5, §5.35, §5.36, §5.53, §5.56, §5.83,
   §5.84, §5.98 — status OPEN by marker; triage into T1/T2/T4 or close.~~
   **[TRIAGED 2026-08-29, F47 §11.163 — all eight read at source; NO mechanism had
   disappeared. Dispositions: **§5.5 → CLOSE-CANDIDATE** (fixed en route by
   §11.91/§11.107/§11.118; the row described the pre-fix tree — a §5-row-stale-against-its-§13-row
   inversion F42's sweep structurally cannot reach; survivors are old-path or `type`-keyed,
   and one of them IS §5.98's site) · **§5.35 → T1** (decision: what should `day_key_mode`
   DO) · **§5.36 → T2 blocked on an ASSET not a decision** (`~/.spacecrafter/videos` still
   empty ⇒ rides T3) · **§5.53 → T1 ×2, (b) PROMOTED** (see T1.9) · **§5.56 → OPEN,
   ground refuted and re-stated, not blocking** (measured: the one destructor that exists
   runs 0 times while its owner's runs 1; consequence nil because nothing reads what it
   clears) · **§5.83 → T4 INFORM** (the new path guards a case old NaNs on) · **§5.84 →
   NOT blocking, instrument caveat only** (old-path-only; any parity read right after a
   `move_to body` inherits a future Newton seed) · **§5.98 → T1 routed to the tester**
   (SS-17's owed answer; joins the final-pass batch).]**

## Explicitly NOT blocking (so the exclusion is challengeable)

- **B8 old-path removal** — transparent operation KEEPS old present (it is the baseline).
- **B1/B2/B3 architectural lines** — unless T3's content census says his shows need those
  classes; today they are Vixy-paced by design.
- **Perfect parity on interactive free-flight residuals** — ≤2.3 m/toggle accepted with
  structure (§11.154(a)); the usage-path model covers the tester's interactive use.

## The critical path, compressed

Vixy answers T1.1 + T1.2 + T1.3 (operator basics · dome geometry · DSO content) and the
T1.4 pair → two or three dispatch rounds burn T2 → the final pass fires carrying T3+T4
(one batch, state-stamped, now including the content census and the field-content
questions) → ~~the T5.1 rehearsal~~ **[RAN 2026-09-05, F90 §11.211]** + a T5.2 soak gate the word "ready". The decision batches
are the long pole; every measured datum they were waiting on is, as of session 14, PAID.

**[STRUCTURAL CORRECTION 2026-08-29, F47 §11.163(h) — this changes how the map reads its own
defect rows, and it shortens the path.]** Ten of the triage set's thirteen members are
**BOTH-PATHS** defects — they live in the command interface, the TUI, the config layer or the
old core, and behave identically whichever path draws. **Against T0's own criterion —
*"observes nothing he must ADAPT to"* — a both-paths defect is not a transparency hole at
all.** It is behaviour he already has, and has had for years. It may deserve fixing, and
§5.112 deserves a warning, but it does not gate the sentence *"he operates the new path
without noticing."* Only **three** of the thirteen bear on transparency: §5.53(b) (now T1.9),
§5.83 (T4 INFORM) and §5.21 (both-paths, but the new path's registration makes it a new-path
authoring trap too). **The test to apply to every remaining row before it is allowed onto
this critical path: does the NEW path behave differently from the old here? If not, it is
backlog, not a deployment gate.** The one genuinely new item F47 adds to the path is §5.112,
and it sits on the DELIVERY mechanism rather than on the render path.

**[2026-09-05 — THE SHORT POLE IS NOW TIER R, one week.]** R1–R4 are dispatched this round
(F83–F86); R5 and R6 are the owner's and cannot be substituted from here. T0's path is
unchanged in shape but no longer the binding clock: the developer can start on a branch whose
tester-facing divergences are open and recorded. The two clocks meet at F84's deployment
manifest — the same artifact answers "what does a clone install" for him and "what will the
next delivery carry" for the tester.

**[2026-09-05 EVENING — THE DECISION BATCHES STOPPED BEING THE LONG POLE (F88 → §11.207).]**
The critical path above was written when *"the decision batches are the long pole"* was
true. Twenty round-3 replies changed that: **T1.2 and T1.4 CLOSED**, and what they gated
became work. The path now reads: **Vixy answers T1.1** (§5.100's authorization, unanswered
since 2026-08-26 — now the LAST operator-basics decision standing) **+ T1.3** (the DSO
content batch) **+ T1.5** → the decision-free queue R18/R19/R20/R27/R28/L1 created burns in
two or three dispatch rounds (**§5.86 + §5.19** first: RA/DE, both decisions closed) → the
~~**T5.1 rehearsal**~~ **[RAN 2026-09-05, F90 §11.211]** and a **T5.2 soak** gate the word "ready". **The final pass is behind
us, not ahead**: it fired, and nineteen of twenty members came back — L2 alone is
outstanding, and it needs him at the machine, not a decision. **Two things the replies
made SMALLER**: A43's data regeneration is cancelled outright, and the §5.74/§5.78 search
family leaves every path. **One thing they made bigger**: R21's census retires nothing —
every content class is in, portrait included, so the "unless the census says otherwise"
exclusion that carried B1/B2/B3 no longer holds by absence. **And one they left where it
was**: §5.128's snap-back fork, which R28 does not touch. §11.163(h)'s test still governs
every row before it is allowed onto this path — *does the NEW path behave differently
here?* — and §5.135, the one row this round added, passes it in the unusual direction: the
divergence is real and it is the OLD path that is wrong.
