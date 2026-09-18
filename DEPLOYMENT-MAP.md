# DEPLOYMENT-MAP — what remains for deployment

**This file is a DERIVED VIEW over the ledger and owns no row.** Every state line below was read
at `INTENT.md` (§5 rows, §13 rows) or at `INTENT/<id>.md` on the compile date, never copied from
this map's own previous annotation. On any divergence the ledger wins and the divergence is a
staleness bug HERE. Maintenance rules, the archive convention and the compile stamp are at the
end. Resolve any ledger id with `python3 claude/intent_resolve.py <id>`.

---

## 0. The criteria — the owner's own sentences, and what they leave open

Two criteria, two readers, two clocks. Nothing in this file is a criterion except his words.

**R0 — the developer** `[vixy 2026-09-05, verbatim]`: *"Stable reference is because another
junior developper, major of his promotion, 5th year post-bac, will work on spacecrafter. I would
prefer this branch to became the stable reference for development, otherwise work will continue
and require further feature port. His work will start in a week."*
Restated by him for the named person `[vixy 2026-09-12, §11.232(a)2, verbatim]`: *"Unknown yet -
he didn't started yet and the previous reference was the old core, so it must be transparent with
the new one. The new version shall be equally capable as the old one, so that work can start
transparently without inheriting the legacy engine, but it must be workable. If there is
unfinished or bugged aspects, it could disrupt work there, potentially. There is a lot of changes
already, the whole new path, so it must be consolidated first."*
With the reader named `[vixy 2026-09-12, §11.232(a)1, verbatim]`: *"The goal was to make it
finished or stable for further work by someone working on part-time internship on it (and also,
pushed by now). … The main tester want to test every features made by the one doing his part-time
internship on spacecrafter, and it will start this week. (also, one note, I misunderstood
something, the one which will work on it in the context of his part-time internship doesn't use
LLM, not sure it change much though)."*
Three riders, answered the same day `[vixy 2026-09-12, §11.232(a)2]`: *"master-beta as is; no
rename now"* · *"Linux"* · *"Yes: developer-entry.md is his entry, harness readable"*.

**T0 — the main tester** `[vixy 2026-08-29, §11.162, verbatim — the sentence that opened this
map]`: *"Can you map the space to resolve before restricted deployment can be operated by the
main tester (new path usable, fully transparently)?"*

**The gate on the branch's name** `[vixy 2026-09-05, verbatim]`: *"The master-beta will became the
reference and get renamed main once ready."*

**This regeneration's own trigger** `[vixy 2026-09-18, verbatim]`: *"Update or rewrite the
DEPLOYMENT-MAP.md to keep it focused on what remains now for deployment."*

### The four questions his sentences leave open — asked here, not answered

**Q1 — which criterion does *"deployment"* name today?** The 2026-09-18 sentence says *"what
remains now for deployment"* and does not say for whom. **Reading A: R0** — the intern develops
on `master-beta` transparently. Then the remaining set is §1's first three nodes plus whatever of
the consolidation set would disrupt him, and most of it is already met. **Reading B: T0** — the
restricted deployment the main tester operates. Then everything in this file is in scope: T1's
gates, T2's work, T3's field verification, T4's cargo, T5's edges. **Reading C: both, R0 first**
(which is how the last three rounds were dispatched, on *"consolidated first"*). This file is
written for C and marks which items belong to which, so A or B can be read off it. One word.

**Q2 — does R0 inherit T0's both-paths exclusion?** §11.163(h) established the test this map has
used since 2026-08-29: *does the NEW path behave differently here? If not, it is backlog, not a
deployment gate.* That test was derived from T0's own words — the tester *"observes nothing he
must ADAPT to"*, and a defect he has had for years is not something to adapt to. **R0's criterion
is different in kind**: *"it must be workable. If there is unfinished or bugged aspects, it could
disrupt work there"*. A both-paths defect disrupts a developer exactly as much as a new-path one.
Nobody has ruled this. **What each reading adds**: under *inherits*, the remaining set is what
§1–§3 list. Under *does not inherit*, it gains ~45 open both-paths rows — the whole script-
semantics batch (T1.6), the command-surface batch (B38's eight), the silent-coercion class
(§5.116 · §5.117 · §5.118 · §5.122 · §5.123), the locale layer (T1.14) and the config writer
(§5.42). Every such row is marked **[both-paths — Q2]** below and is carried, never dropped:
`harness/artifacts/f120/sweep.tsv` holds one row per member with its argument.

**Q3 — *"once ready"*: is anything still gating the word?** Both gates this map itself put on it
have run — the end-to-end operator rehearsal (§11.211) and the multi-hour soak, twice (§11.215,
§11.218). So *"ready"* has no instrument left in front of it here; it is your word, or you name a
new gate. Note what the soaks cannot see: T5 below.

**Q4 — the tier-R ids collide with two other live namespaces, and this regeneration widened the
collision.** `R1`–`R6` are this map's derived R0 requirements; `R1`…`R13` are the round-2 REPLY
ids in `USER_QUESTIONS_ROUND2.md`; `R1`/`R2` are the ledger's own root requirements in `INTENT.md`
§2 (potential expansion, the small-interactive-planetarium use case). Three meanings for `R1`.
This pass added **R7–R11** by the max+1 rule, which extends the overlap with round 2 from
`R1`–`R6` to `R1`–`R11`. Inside this file the ambiguity is removed by convention — a reply id is
always written qualified (`round-3 R21`, `round-2 R7`) and a bare `R<n>` is always this tier — but
a grep from outside cannot tell. **A rename is yours and no executor will take it**; the cheapest
form is a prefix on this tier (`DR<n>`), which costs eleven citations in `INTENT.md`,
`DECISIONS_PENDING.md` and `INTENT/<id>.md`.

---

## 1. WHAT REMAINS — YOURS

Ordered by how much work your word unlocks. Each node: the question · what it unlocks · the
anchor. Nothing here can be substituted from inside the project.

**Y-1 · Read `claude/b5-dso-design.md`, then seven nodes.** It is 1023 lines and its §0 says what
it decides and what it does not. Your reading is the single largest unlock in this file:
**five slices of engine work**, the first decision-free, become dispatchable the moment you have
read it; slice 3 (the OJM models — the field's real DSO layer, 560 command lines) goes before 2
and 4 by field risk; 2 and 4 wait on one of the seven nodes. The seven are
`[Y17]`–`[Y23]` of `fable-dispatch.md` §3 (authored vs engine-instantiated content · the
catalogue→AU factor · `mode in_sandbox` · Tully's selectability · OjmMgr's full-screen depth
clear · the three appearance questions · two operator words for one label state).
→ T1.3 · §11.240 · B5

**Y-2 · §5.142 — your tester's own shows abort the binary; the root is measured and four
decisions are yours.** `fscripts/06.sts` costs 1344 measured bytes per authored body against a
1 MiB pool, so its 675th body is refused, the device is lost and the process aborts. You ruled the
pool stays 1 MiB, and asked the root instead: 884 of the 1180 `sizeof` bytes are redundant in one
of three measured senses; the per-body requirement is 88 B of payload, 128 B carved, 8192 bodies
per MiB. Four options with their arithmetic and their walls are at §11.236(i)(k); only the
REFUSE option is scale-free. **Unlocks**: the one thing in this file that makes the reference
binary die on shipped content he authored.
→ T1.12 · §5.142 · §11.236(k)

**Y-3 · The intern's three gaps — the only items with a clock, and the clock is this week.**
(a) **R7**, where the outside content-installation procedure lives and whether it may be named in
the code repo: until then his clone runs a program with no sky and nothing tells him that is
expected. (b) **R8**, the text of the engineering principles I1–I7, or permission to reconstruct
them from the ledger's uses for your correction: he meets `I<n>` citations in week 1 with no text
to check them against. (c) **R9**, a deploy key or the HTTPS form for the second clone: as written
he cannot run it at all. One sentence each closes (a) and (c).
→ R7 · R8 · R9 · §11.232(c)4 · §11.234(l)

**Y-4 · T1.5 — what `moveto … alt A` means above a display-scaled body.** Drawn surface or
physical surface. He authors scenes on scaled bodies, and every harness in this project carries a
workaround for it. **Unlocks** §5.109's fix and retires a standing hazard from the measurement
protocol.
→ T1.5 · §5.109

**Y-5 · T1.9 + A42 — the big-texture level gate, one token, and the number is the tester's.**
Old swaps the colour map at 180 px, the new path at 409.6 px, so on the Sun and the Moon — the two
bodies he shows most — the disc visibly changes colour at 2.28× the distance it used to. Round-3
R29 (*"It's a design."*) made this sharper, not softer: the swap distance now carries an authored
effect. Aligning needs a distance you and he choose together.
→ T1.9 · §5.53 · A42

**Y-6 · T1.13 — the view-offset family, and one of its two halves touches the comparison
baseline.** (a) §5.135: the OLD path rolls the view offset with the heading and the tester has
seen it and says it should not; correcting it means touching the frozen baseline (§11.52(b)), so
it is correct-it or let-it-retire-with-B8. (b) §5.128: old's aim compensation undoes a fixed
`offset × 90°` while its draw applies `offset × fov/2`; does a show that sets the offset mid-show
EXPECT the view to snap back to `init_view_pos`? Round-3 R28 does not touch it.
→ T1.13 · §5.135 · §5.128 · B17

**Y-7 · T1.15 — five fix-shape calls on the new path, each one word, each priced.** §5.153 a
commanded view duration under 0.2 s is floored (38.35° apart from old at `duration 0.1`): snap,
clamp as today, or honour it. §5.154 the trail walker's stale seed (4.33 arcsec on one frame after
an unhide): re-converge after the walk (5 solver calls against 1790, 0.3 %), a third seed member,
the batch pair, or nothing. §5.148 what a body with no position answers to a track (66 NaNs into
the old path's view state today). §5.149 the constructor's JD-0 seed, four shapes at §11.229(h1).
§5.152 a second instance aborts instead of refusing cleanly, and the refusal line never reaches
the console. Plus the pole-vs-no-direction readout question at §11.216(j2).
→ T1.15 · §5.153 · §5.154 · §5.148 · §5.149 · §5.152

**Y-8 · T1.16 + `[Y26]` — the EntityCore pair, scheduled together or not at all.** §5.146:
`~SharedBuffer` hands a REFUSED `SubBuffer` back to the pool with an indeterminate offset that
reaches `vkCmdBindDescriptorSets`; you gave the shape (*"it should throw whenever it can't
allocate"*). The edit is one line in your submodule, and a pin naming an unpushed EntityCore
commit breaks a `--recurse-submodules` clone — the intern's, this week. Riding with it:
`7ce58350` is an ORPHAN gitlink reachable from no ref of EntityCore, which is why every code
commit older than `f4ceb208` names a submodule commit a fresh recursive clone cannot check out.
Push it, or accept it.
→ T1.16 · §5.146 · §5.131 · §11.242(f)

**Y-9 · R10 — every cost number this project has published was measured on the wrong build.**
`build-claude` is RelWithDebInfo and compiles `-O2 -g -DNDEBUG`; `install_src.sh`'s documented
path configures Release and compiles `-ggdb3 -Ofast -Wall -O3`. §2.0 D11 prices everything in
1 ms/frame. Re-point `build-claude` at Release and re-bank, or keep `-O2` and state it as the
measurement platform. **Not touched here**: re-configuring invalidates the canary's photometric
band and every baseline standing on it.
→ R10 · §11.234(g)

**Y-10 · R6's two halves.** (a) The rename, when you want it: eleven ordered acts each with a
check that can fail, and a patch covering exactly the four live pointers (§11.212(h)); 133 pins,
89 historical records and 0 convention uses must NOT be touched. (b) **One sentence never
stated**: after the rename, do pull requests still target `2023-master`? `doc/developer-entry.md`
says they do, and the intern reads it as it stands.
→ R6 · §11.212(h)

**Y-11 · T1.17 — §5.115's density half.** The size bound is live and is your number (1 GiB total
across channels, rotating within a session). What is untouched is what fills it: the per-command
echo, the ~100k-command bulk path, the dedup-with-counter. Under the tester's own corpus the
script log costs 193 MB/h. The design should be seen before it ships.
→ T1.17 · §5.115 · §11.237(m)

**Y-12 · T1.18 — one authored value, two meanings, and the two paths disagree.** §5.126: `halo =
on` draws NOTHING on the old path and a halo on the new one, because the legacy loader accepts
`true|1` and the composed one accepts `true|on|1` — a DATA-ONLY route to a divergence that parity
work assumes cannot happen; plus `orbit_Eccentricity`, live in a script and dead in
`ssystem.ini`, the best explanation for `[Sedna]`'s lost elements. §5.125: adopting the composed
format silently drops `big_halo_size`, `tex_skin`, `halo_alpha_override`, `halo_scale_override`.
Which predicate is canonical, and should the data surface lowercase? Neither is decision-free.
→ T1.18 · §5.126 · §5.125

**Y-13 · T1.7's four suspended rows with a live consequence.** **A40** teardown vs an incomplete
frame (§5.59: the app does not begin to exit at all; three launches decide it, and an operator
quits every night). **A41** the G4 early-visibility gate, 2 px or 3.07 px, one token that decides
when a body starts drawing. **A15** the fade threshold and band (L1 judged one of four axes; the
rest is what the audience sees). **A44** the RING shadow caster's scaled radius, unexercised on
shipped content today and therefore cheap to leave.
→ T1.7 · A40 · A41 · A15 · A44

**Y-14 · The schedule calls, one line each.** §5.112's fix, on the tester's own direction
(*"Put a # in front of the deprecated lines would be better"*) — comment out, never delete; the
writer is shared with §5.42. D37's implementation (a hidden star goes dark, ambient-valued) —
decided, unscheduled. Round-3 R25: the tester says *"no need"* to your `[parallel-script]`
proposal — withdrawn, or kept as yours. §5.106 and the free-flight defaults were NEVER SENT in
the final pass and still wait on you.
→ §5.112 · §5.42 · D37 · §5.106

**Y-15 · The instrument and tool residues that are in the CODE tree, so they are the intern's
too.** `util/scedit/tests/anchor_gate.py` carries twelve duplicated top-level definitions (the
second copy wins) and misdiagnoses a missing submodule commit as `(out-of-range)`; one row of
`ss-grammar.json` carries a `[NOT AT HEAD: …]` marker. His `ctest` is 19/19 from a fresh
configure at `564ec489`, but a transport clone reads 4 AT-PIN-BROKEN, all four being `[Y26]`'s
orphan. Also yours by `[Y28]`/`[Y29]`: the rewrite tools' file-set rules and what counts as
evidence under `harness/artifacts/**`.
→ R11 · §11.242(e)(g)(m)

---

## 2. WHAT REMAINS — THE TESTER'S

Routed to him per §11.161(c) (old-behaviour intent and field-data content are his). Relay is
yours. None of these is a task; each is an answer.

**X-1 · §5.140 — what `orbit_lon` MEANS.** `surface_point` + grounded lands a rover ninety degrees
EAST of the planetographic longitude the camera, `moveto lon` and old's `AnchorPointBody` all
mean. It is a RATIFIED authoring key. **Unlocks** §5.21's two remaining halves, which are blocked
on it and not on effort. `[vixy 2026-09-12]` routed it to him.
→ §5.140 · §5.21 · §11.233(e)

**X-2 · A42's number — at what distance should the two paths swap texture level?** R29 said his
answer would decide it and the number was not given. It GATES T1.9.
→ A42 · T1.9

**X-3 · round-3 L2 — the oort-shadow onset.** *"I didn't test it yet."* The ONE member of the
twenty-item final pass still open; it needs him at the machine, not a decision.
→ T4 · §11.207

**X-4 · The script surface: 36 of 43 `SS-n` entries are open and every one is his.** The sharpest:
SS-12 (what should a recording CONTAIN — it decides an entire defect class, §5.96) · SS-9 (the one
surviving `spacecraft on` line) · SS-2/SS-4/SS-5/SS-8/SS-10 (lines his 2026-08-26 rewrite removed,
where the question *did this ever work* is the valuable part and a 2020 reference attests three of
the spellings) · SS-40/SS-41/SS-42 (two dead keys on EVERY body over 189 lines of his
`ssystem.ini`, three misspelled keys over 3365 lines of the shipped package, and `[Sedna]`'s three
lost orbital elements) · SS-32/SS-33/SS-34/SS-35/SS-36/SS-37/SS-38 (spellings the engine ignores
across the shipped shows). Bulk answers are fine and the file says so.
→ `SCRIPT_SURFACE.md` · §5.124 · §5.151 · §5.97

**X-5 · Unattended play: honour or skip the authored pauses?** Every shipped `basis/` show carries
`script action pause` with `flag_skip_pause = false`; honouring them unattended costs ~39 resumes
per 159 s cycle, and `diaporama.sts` alone authors 100 in a `struct loop`. Old-behaviour intent.
→ `fable-dispatch.md` §3 session-23 (7)

**X-6 · One doc token in his own file.** `doc/superscript.sts:1529` reads `Ganymed=503`; after
F94 that is the one spelling on the line matching nothing. Routed at SS-17, never edited here.
→ SS-17 · §11.214

**X-7 · His field, which this project cannot read.** Which content classes his real shows load
was answered once — round-3 R21, *"All have been tested, but sometimes long ago, so maybe some
features could have altered the way it shall work."* — and that sentence bounds the SET, not the
STATE. What is still his: his `config.ini` version and how much of it he hand-edited (§5.112's
danger is proportional to exactly that), his GPU against §5.60's 2.68 GB staging allocation, his
dome/projector stack, his `maximum_fps`, and any file of his corpus newer than the copy soaked.
→ T3 · §5.112 · §5.60

**X-8 · Data he owns, forward-only (D9).** `06old.sts` lands 156 of 170 bodies: 13 duplicate names
and TDRS 3 with no `coord_func`, both paths refusing and saying why. `[Sedna]`'s orbit in the
shipped `ssystem.ini` is not the authored one.
→ §5.151 · §11.221(g)

---

## 3. DISPATCHABLE NOW — decision-free, sized, in dispatch order

**Every launch-class item on this list is currently behind one environment question**: the canary
is RED on `display.geometry` on this boot (`:2` came up 2444x1332 against a bank of 2448x1332) and
was deliberately not re-banked — `HOST-EVENTS.md`, 2026-09-18. Zero-launch items are unaffected.

**D-1 · The `IterativeEll` fold leg** (S, zero engine change, one launch class). Modelled only:
the mean anomaly folds on every call while the seed's H does not — one cell per orbit where two
Newton steps cannot recover; Sedna 731.9 AU, Haumea 7.13, Juno 1.18. The SHARED solver, BOTH
render paths, no hide needed. F111's shape on the comet family. → §11.241(i)

**D-2 · §5.49's owed render measurement** (S). `f14_meridian.py`'s `u_sub` settles whether
`moveto lon 0` stands over Greenwich, the map centre, or 90° off. The row's conclusion is recorded
in-doubt. Data-author-central; any FIX is yours. → §5.49

**D-3 · §5.107, the extent-cache latch** (S). Fifth member of the closed latch class; the fix
shape exists. Owed with it: the count of bodies whose `isVisible` never went true in a shipped
show. → §5.107

**D-4 · §5.71, the `panView` port** (S). `look_at delta_azimuth/delta_altitude` still moves the
old navigator alone; R28's dome-fixed answer lifted the block. **Its sibling §5.66 is NOT
dispatchable with it** — see T2. → §5.71

**D-5 · §5.88, the catalogue-load diagnostic** (S). R23 makes a limited catalogue NORMAL, so
nothing tells an operator which of the two skies he is running. The cheapest of the three fixes
and the only one that does not need R7 answered first. → §5.88

**D-6 · `install_src.sh:24`'s missing `|| exit`** (S, code). The recursive-clone fallback fails
and `cmake` still returns 0, so the build dies later at a missing EntityCore header with no line
naming the cause. §5.131's two riders, OPEN and unowned. → §5.131

**D-7 · The composed-body i18n selection asymmetry** (S, measure then route). `select object
<translated>` cannot reach composed bodies while `select planet <english>` can; measure the
consequence for a French-locale operator. → §11.158(d5)

**D-8 · B35/B36/B37, the capability audits** (M). They BOUND what *"the new path"* can express,
which is the denominator of the transparency claim. → B35 · B36 · B37

**D-9 · DSO slice 1** (S) — the reach/visibility split alone, with nothing using it; a mutant that
must move ~533 AU and not one pixel. **Gated only on Y-1**, your reading. → §11.240 · §5 of
`b5-dso-design.md`

**D-10 · B41, the preload signal** (S–M) — decision-free once the zoom-half anchor is chosen
(`zoomToBothPaths` does not know the body); your words opened it. Its row says: after the
consolidation items unless you order otherwise. → B41 · §11.235(l)

**D-11 · The re-cut of §11.207(g)'s decision-free queue** (S, reading). Three of its labels were
refuted at their own rows (§5.66, §5.71's feel half, and item 6); the list must be re-cut before
anything is dispatched from it again. → `fable-dispatch.md` §3 session-28 `[H4]`

---

## 4. The critical path, compressed

For **R0** the path is three sentences long and it is entirely yours: **R7 · R8 · R9** (content
procedure, principles, remote form) and nothing else stands between the intern and a working
reference; R1–R5 are met or closed, R6's rename waits on your *"ready"*, and everything else he
meets is consolidation whose urgency depends on **Q2**. For **T0** the shape is unchanged and the
length is not: the decision batches stopped being the long pole when the twenty round-3 replies
landed, the final pass has FIRED and returned with nineteen of twenty answered, and both *"ready"*
gates this map set — the operator rehearsal and the multi-hour soak — have RUN. What is left is
**your reading of the DSO design** (which alone converts five slices of work from blocked to
dispatchable), **§5.142** (the only item here that kills the reference binary on his own content),
the **five one-word fix-shape calls** of T1.15, and **T1.5 · T1.9 · T1.13** — after which T2's
dispatchable list burns in two or three rounds and the word *"ready"* has nothing in front of it
but you. The test §11.163(h) put on every row before it may join this path still governs — *does
the NEW path behave differently here?* — **and Q2 asks whether that test is the right one for a
developer at all**; if the answer is no, the path gains the both-paths backlog and roughly
doubles.

---

## R — Reference for development (the R0 tier)

Closed, gap kept, never reused: **R1** (the deployed line is inside `master-beta`; merge
`ba9df31f`, §11.203) · **R5** (both branches pushed 2026-09-12 11:03–11:04, 0 unpushed measured
again at this compile; §11.232(b) — the standing cadence is *"the end of the week"*, and this
compile is a Friday). Their rows are in `harness/artifacts/f120/disposition.tsv`; their text is in
the archive.

**R2** — a clean clone builds, installs and runs by the documents.
State: **MET** for the clone, the build and the launch (§11.204 measured it NOT met; §11.205 and
§11.206 and the pin bump `f4ceb208` closed both blockers; the clone probe returns rc 0 and an
empty-`$HOME` launch exits 0). Ledger: §5.130 **FIXED**, §5.131 **CLOSED for the clone** with
*"the two `install_src.sh:24` riders stay OPEN and unowned"*.
Live remainder: the fallback with no `|| exit` (D-6). What it blocks: nothing today; it re-arms
the moment a pin names an unpushed submodule commit, which is exactly what Y-8's EntityCore edit
would do.

**R3** — an entry document exists in the code repo.
State: **MET** — `doc/developer-entry.md`, the first thing ever promoted under `claude/README.md`'s
filing criterion (§11.206), followed end to end by hand and corrected (§11.234), its citations
machine-checked (136 paths / 29 continuations / 21 ledger ids / 3 commit shas, 0 dangling).
Live remainder on this row: no human authorship convention is stated anywhere. Its two other
recorded residuals now have ids of their own: R7 and R8.

**R4** — no memory-unsafety reachable from the shipped data at startup.
State: **MET for everything this repository owns** (§11.205: §5.133 the anchor UAF and §5.134 the
comet null deref both fixed and gated).
Live remainder: **§5.48** — **OPEN** at its row, the `ASmooth` NaN on a cold `$HOME`, ~15 % of
first launches, rated 0/6 with a control that fires. EntityCore, yours.

**R6** — branch policy.
State: policy **ANSWERED** (*"will became the reference and get renamed main once ready"* =
rename, not redirect); act **DEFERRED** (*"master-beta as is; no rename now"*).
Two live halves: the rename itself (Y-10a) and the PR-target sentence (Y-10b).

**R7** — where the outside content-installation procedure lives. *(new this pass; carried since
2026-09-05 inside R2's row and the head, with no id)*
State: **UNCHANGED / unanswered** at §11.234(l). Round-3 R23 `[stated: tester, via owner commit
1e6ca60]`: *"By default, only limited catalogs are loaded. Correct catalogs are loaded in an
outside installation procedure."* A tree install is 227 files and **no content** (§11.204(f)); no
document in either repository names the procedure, and `doc/developer-entry.md` §5 says so in its
own words. Blocks: the intern's first run — a program with no sky and no way to learn that this is
expected. One sentence in `INSTALL` §5 closes it. Yours.

**R8** — the engineering principles I1–I7. *(new this pass; R3's residual (i))*
State: **UNCHANGED** at §11.234(l). `doc/developer-entry.md` §9 is a placeholder addressed to you
by name; the ledger cites `I<n>` by number (I2 alone 33 times) and neither repository states what
they say. Blocks: he meets the citations in week 1 with nothing to check them against. The text,
or permission to draft it from the ledger's uses for your correction. Yours.

**R9** — the remote form for the second clone. *(new this pass; §11.232(c)4(iii))*
State: **UNCHANGED** at §11.234(l). The document's `git clone -b CC-harness <same-remote-url>
claude` resolves to the SSH form, which needs a key on his account; the rehearsal substituted a
local path and said so at the step. Not cosmetic: a clone over the git transport carries no
unreachable objects, which is why one stale sha read *"bad object"* for him and *"exists"* here.
A deploy key, collaborator access, or the HTTPS form written into the document. Yours.

**R10** — the build the documents ship is not the build every measurement uses. *(new this pass;
`[Y8]` of session 29, never carried by this map)*
State: **MEASURED**, §11.234(g). `build-claude` is RelWithDebInfo → `-O2 -g -DNDEBUG`;
`install_src.sh` configures Release → `-ggdb3 -Ofast -Wall -O3`; `CMakeLists.txt` branches on
Debug/Release/LocalRelease only, so RelWithDebInfo takes none of them. Every number published
under §2.0 D11's 1 ms/frame denominator was taken on the first while a user runs the second.
Yours: re-point and re-bank, or state `-O2` as the measurement platform. Y-9.

**R11** — what the intern's own clone reports. *(new this pass)*
State: **MEASURED at the transport clone**, §11.242 / `fable-dispatch.md` §3 `[F4]`. `ctest` is
19/19 from a fresh configure at `564ec489`; the scedit anchor gate reads **4 AT-PIN-BROKEN** on a
transport clone, all four being the EntityCore orphan of Y-8; `util/scedit/tests/anchor_gate.py`
has twelve duplicated top-level definitions and misdiagnoses a missing submodule commit as
`(out-of-range)`; one `ss-grammar.json` row carries a `[NOT AT HEAD: …]` marker. All of it is in
the CODE tree, so it is his surface, not the harness's. Y-15.

Also landing on his first hours, homed at their own rows rather than here: **§5.152** (a second
instance ABORTS instead of refusing, and the refusal never reaches the console) · **§5.132** (a
failed data-class copy prints *"Completed copy of X"* and leaves the directory empty) · **§5.119**'s
rider (hardcoded `/home/planetarium/…` font paths).

Not required for R0, and the exclusion is challengeable: the T0 tiers below — a developer can work
on a branch whose tester-facing divergences are open **provided they are RECORDED**, which is what
§5 and T4 are for. **Q2 is the live challenge to exactly this sentence.**

---

## T1 — Decision gates (nothing below them can close these)

The decider is named per item. Ordered by operational weight, not by age.

Closed, gap kept, never reused — T1.1 (the zoom pair, answered §11.233(d) and built F114 →
§11.235; the two paths' look-direction gap went 99.0336°/99.0318° → 8.0e-06°/1.0e-05°); T1.2 (the
tilted-dome coupling, decided by round-3 R28 in favour of the NEW path's dome-fixed offset, its
two residues now T1.13); T1.4 (§11.4's pair, origin answered by round-3 R27, decision (1) turned
out not to be a decision, work landed at §11.213).

**T1.3** — the DSO layer on the new path. Decider: **owner**, then dispatchable.
State: **direction given** (§11.233(c)) and **designed** (F117 → §11.240, `claude/b5-dso-design.md`,
1023 lines, 272 citations resolved). The as-if criterion applied per observable channel does not
merely permit the reach/visibility split, it REQUIRES it. 5 of the 9 suspended items are answered,
1 in part, 3 stay his. What remains as a GATE: your reading, plus `[Y17]`–`[Y23]`. What is no
longer gated: five slices, the first decision-free (D-9).

**T1.5** — §5.109's layer half. Decider: **owner**.
State: **OPEN** at §5.109. What `moveto … alt A` names above a display-scaled body: the drawn
surface or the physical one. Y-4.

**T1.6** — the script-semantics batch. Decider: **owner**, with the members whose semantics are
the tester's routed to him. **[both-paths — Q2]**
State: every member **OPEN** at its row. Members, refreshed and widened at this compile:
§5.64 + §5.76 (pause does not hold the clock; a slowed paused clock runs backward) · §5.65
(lock-after-move latch) · §5.69 (`keep_time` truncated to 8 bits, the default included) · §5.70
(an unreplayable ramp recording) · §5.72 (`$LOGON` carries other clients' answers) · §5.75 (a
trail drops a sample on a date-stepping show) · §5.82 (`transition_to point name` dropped) ·
§5.85 (`align_with` does not align) · §5.87 (`constellation_star` acts on the previous selection)
· §5.91 · §5.93 · §5.94 · §5.95 · §5.96 (the script-surface family, including the recorder
diverging from the author's text) · §5.73's truncation-marker policy at ≥1023 B · §5.42 (the
config writer destroys comments) · **and the members this map did not carry**: §5.114 (every
illuminate has GREEN and BLUE exchanged at the loader — visible, one line, one discriminating
launch owed) · §5.116 (no invalid state: every unrecognised `flag` value becomes OFF and every
non-numeric value becomes 0, reported as success) · §5.117 (157 user-facing refusals written at
`L_DEBUG`) · §5.118 (`set stall_radius_unit` filters instead of refusing) · §5.120
(`lunar_eclipse_umbra`/`_penumbra`: four registered names that build nothing) · §5.122 (an
argument key the handler never reads is unobservable) · §5.123 (*"did you mean"* names the wrong
argument) · §5.124 (the same silent-drop class on the DATA surface, 189 lines of the field
`ssystem.ini`) · §5.89 (the unknown-`viewing_mode` dead guard) · **B38's eight members** (dead
tokens and reachable-but-defective handlers, resolved into this batch by §11.163(i)).

**T1.7** — the forks whose owed data is PAID, awaiting the call. Decider: **owner**.
State: **OPEN**. §5.88 (catalogue-load reporting — also D-5) · §5.89 (the one-site dead guard) ·
§5.90's MECHANISM (list from the user dir, files from the data root, mismatch silent) · **A40**
(quit vs an incomplete frame, §5.59's fork) · **A41** (the early-visibility gate's value) ·
**A42** (the texture-level swap distance — the number is the tester's, X-2) · **A44** (the ring
shadow caster's scaled radius) · **A15** (the fade threshold and band) · §5.106 (free-flight
environment semantics — never sent) · the two §11.144 riders (free-flight `moveto` meaning,
`get status position`) · §5.108 (`flag_sun_scaled` is dead; reviving it is a behaviour change).
**A43 LEFT this list 2026-09-05** — round-3 R29 *"It's a design."*, so the regeneration is
cancelled, not deferred.

**T1.8** — B31's completeness residue, and the wider seam it names. Decider: **owner**, then
dispatchable.
State: **OPEN** at B31 (the session file and the ledger). C4's non-body catalogue key (D34's
unanswered half) + D30's DELTA branch for `display_scale`, located and unimplemented. Added at
this compile: **§5.104's class question** — a *"state-preserving"* `body action reload` that does
not preserve state; the ruling covered scaling only and the other seams `Core::init` sets and
`reloadSystem` rebuilds are unenumerated.

**T1.9** — the big-texture level gate. Decider: **owner** + the tester's number.
State: §5.53 **(a) WITHDRAWN as a defect** (R29: the two previews differ because he authored them
that way), **(b) OPEN and sharper**. Old swaps at 180 px, the new path at
`BODY_BIG_TEXTURE_BOUNDING_SIZE = 409.6f`. The set's only new-path-specific VISIBLE divergence, on
the Sun and the Moon. Y-5.

**T1.10** — `day_key_mode` and `camera action save`. Decider: **owner**. **[both-paths — Q2]**
State: §5.35 **OPEN** (an inert capability end to end — what should the control DO?); §5.41
**OPEN** and B31-gated (B31 §3.4(d) wants the surface re-expressed on the session serializer, not
the prefix patched; `~/.spacecrafter/anchors` still does not exist, so the command always fails).
§5.98 left this item 2026-09-06, FIXED.

**T1.11** — the missing-guard class and §5.110's fix routing. Decider: **owner**.
State: §5.113 **OPEN** and explicitly unaffected by F91/F96; §5.110 **OPEN**, its mechanism the
TYPE FILTER upstream. Three shipped reaches with nothing selected — `set home_planet selected`
teleports the observer 1 AU and caches the fiction under the empty name · `flag
object_coordinates on` draws a live-looking readout for nobody · `illuminate hp <absent>` feeds
indeterminate memory into the grid. Where does the truthiness test belong: each read, the
singleton, or both (I6)?

**T1.12** — the uniform pool's consumer side. *(new this pass)* Decider: **owner**.
State: §5.142 **OPEN**, with the root measured and four options priced (§11.236(i)(k)); the pool
stays 1 MiB by your word. Y-2.

**T1.13** — the view-offset family. *(new this pass; T1.2's two residues)* Decider: **owner**.
State: §5.135 **OPEN** (both readings recorded, neither taken); §5.128 **OPEN**, record-only.
Y-6.

**T1.14** — the locale layer. *(new this pass)* Decider: **owner**. **[both-paths — Q2]** for its
second and third members.
State: §5.111's matched set **FIXED** (F87, 14 msgids) and its **UNMATCHED set OPEN** — the type
slot, the body NAME, the generated `" (orbit centre)"`; until it is answered a French readout is
French except its first line, and WHICH translation (UI or sky) is the question both files'
comments flag. §5.121 **OPEN** (one static table serves both locales; unmeasured). §5.136
**OPEN** (`app_locale` does nothing whenever it differs from `sky_locale`, and the app logs the
locale it discards). The tester operates in French, so this is his daily surface.

**T1.15** — the new path's fix-shape calls. *(new this pass)* Decider: **owner**.
State: §5.153 **OPEN** · §5.154 **OPEN** · §5.148 **OPEN** · §5.149 **OPEN** · §5.152 **OPEN**,
plus the readout question at §11.216(j2). Each is one word and each has its shapes priced. Y-7.

**T1.16** — `SharedBuffer` throws on a refused allocation. *(new this pass)* Decider: **owner**
(EntityCore).
State: §5.146 **OPEN**, the shape GIVEN `[vixy 2026-09-12]`. Scheduling is coupled to your
submodule push, not to effort. Y-8.

**T1.17** — the script log's density half. *(new this pass)* Decider: **owner**.
State: §5.115 — retention **FIXED** (F108) and the size bound **FIXED** (F116, 1 GiB total, your
number); the **density half OPEN**. Y-11.

**T1.18** — one authored value, two meanings. *(new this pass)* Decider: **owner**, with the
lowercasing half arguably the tester's.
State: §5.126 **OPEN** (every clause a semantics decision, so none is decision-free); §5.125
**OPEN**. Y-12.

---

## T2 — Work, dispatchable now or upon its T1 gate

Keyed by ledger id, as before. The dispatch order for the decision-free members is §3.

**§5.49** — the owed render measurement. **OPEN**; S and unblocked (D-2); any fix is the owner's.
**§5.107** — the extent-cache latch. **OPEN**; the fix shape exists (D-3).
**§5.71** — the `panView` port. **OPEN**; dispatchable (D-4); the ramp-duration feel answer is
still owed and is not part of the port.
**§5.66** — `look_at`'s two halves. **OPEN**, and **this map's predecessor was wrong about it**:
it said §5.66 and §5.71 both *"port under the NEW path's convention and neither waits on a
decision any more"*. R28 lifted the FRAME block; §5.66's own owed item — which landing an
operator typing `look_at azimuth X altitude Y` means, the drawn bodies' or the drawn sky's — is a
different question and is open. Corrected here, at §11.207(g)'s node, and at session-28 `[H4]`.
**§5.21** — `LocationOrbit`. **PARTLY FIXED** (the unconverted latitude, both paths, measured
58.31008° → 45.00000°); (a) the frozen spin and the third defect (86.306371° off old's own
authority on Mars) are **OPEN** and **no longer decision-free** — blocked on X-1, not on effort.
**Composed-body i18n selection asymmetry** (§11.158(d5)) — measure, then route (D-7).
**B35 / B36 / B37** — the capability audits (D-8). They bound the denominator of the transparency
claim.
**B5 / the DSO slices** — five slices in `b5-dso-design.md` §5: slice 1 (S, decision-free, D-9),
slice 3 (M, the OJM models — the field's real DSO layer), then slices 2 and 4 (M each, gated on
`[Y18]`), then slice 5 (S, retiring the old draw sites one class at a time).
**B41** — the implicit preload signal (D-10).
**B33 / B34** — the QUERY half of the control surface reads the OLD path (`getHeading`,
`getViewOffset`, the observatory getters, `getMountMode`), and the interactive view ramp is
MEASURED to move the old navigator alone while the new camera stays bit-identical. Both are
transparency holes of the readout-versus-drawn kind; neither has been scheduled.
**§5.131's riders** — the `|| exit` (D-6).
**§5.88** — the catalogue-load diagnostic (D-5).
**§5.144** — the 8–12 h discriminating leg (a cache that has not filled, or a leak: +68.3 MB/h
over 4.5 h of his corpus without saturating). Needs one yes/no from the owner, then S.
**§5.99** — the harness's own instrument defect (`b39_star.py` counts the whole frame). Not a
product surface; listed so the exclusion is visible.

Discharged since the compile and no longer T2 work: §5.111's matched set (§11.209) · §5.110's
live check (§11.166) · §5.41/§5.42 (triaged to T1) · §5.20 (no construction site on either path)
· the three hygiene items (§11.164, §11.165, §11.168) · §5.86 + §5.19 (§11.213) · §5.138
(§11.216) · §5.139 (§11.220) · §5.141 (§11.219) · §5.150's iterative half (§11.239).

---

## T3 — Verify at HIS field (restricted-deployment-specific)

**The field-content family.** Answered in three parts and narrowed rather than closed: round-3 R22
*"Search is deprecated."* takes §5.74 and §5.78 off every path (**OPEN**, no fix owed); R23 makes
26 561 stars the PRODUCT's default rather than a fault of our install, so §5.90's headline reading
is withdrawn while its **MECHANISM stays OPEN** and §5.88 gains weight; R24 *"We should but for
now it is in another directory."* makes `~/.spacecrafter/stars/` a stated want with no current
dependent. The live question is the PROCEDURE — R7.

**His hardware.** §5.60 **OPEN** (a single unconditional 2.68 GB device allocation) against his
GPU's limit; his dome/projector stack against our headless `:2`; his `maximum_fps`. Standing fact
that travels with every appearance claim: rendered appearance is a property of (binary, driver)
JOINTLY — a driver bump with no code change made the shipped Moon render at mean ×0.371
(§11.164/§11.167), and cadence is stack-local (§11.159(k7)).

**His config migration.** §5.112 **OPEN**. The mechanism is measured on both arms: on the field
config verbatim a version bump DELETES 0 and ADDS 2; on the same file plus hand-authored content
it DELETES 4 by name, and both runs end at one md5 — a projection onto the schema, not a partial
loss. So the danger to him is proportional to how much he has hand-edited his own `config.ini`,
which this map cannot know. N7 carries both numbers; his answer is the fix direction (Y-14).

**The B14 data package** — poles/W0/periods corrections riding the next `spacecrafter-data`
delivery, forward-only (D9); his baselines shift accordingly. B14 **OPEN** on W0/drift/loaded-6.

**His script corpus.** Narrowed by measurement: F98 soaked the tester's own `fscripts/` corpus for
4.508 h (135 shows, 8 cycles, 541 samples) and it produced §5.142, §5.143 and §5.144. What remains
is HIS install and any file newer than the copy soaked.

**The portrait window.** §5.129 **OPEN**, record-only: a window taller than wide draws the dome
`(H-W)/2` px below centre with a dead band above. R21 puts the portrait window IN scope, so this
is a cost the field may be paying rather than a hypothetical.

---

## T4 — Deliberate divergences he gets INFORMED about, not fixes

State: **the batch FIRED on 2026-09-05 and came back — nineteen of twenty answered** (§11.207).
D15(a)–(d) stand as built by the batch's own silence-equals-accepted contract; D15(b) confirmed by
round-3 R26; D37 answered by R14+R15; D28/A38 by N5; §11.4's origin by R27; §5.83 accepted in
silence; §5.21 by R18; §5.98 authorised by R19 and then delivered.

**Still open, one member: round-3 L2**, the oort-shadow onset — *"I didn't test it yet."* X-3.

**Never sent, and they wait on the owner, not on him**: the free-flight defaults, and §5.106's
environment semantics if it is to be closed as accepted.

**Candidate cargo for a second pass, if one is ever scheduled** (none is): whichever of T1.15's
five calls is answered with *"leave it"*, and the as-if decisions the DSO port makes visible.

---

## T5 — UNMAPPED (this map's own edges; completeness over certitude)

**Re-derived at this compile, not carried.** Four of the predecessor's five edges closed — the
rehearsal ran (§11.211), the soak ran twice (§11.215, §11.218), B38 turned out to be enumerated
rather than unmapped (§11.163(i)) and the older unread rows were triaged (§11.163). What follows
is what the regeneration could not map plus what the sweep showed has no home. Ids continue from
the predecessor's maximum; T5.1, T5.2, T5.3 and T5.5 are gaps.

**T5.4** — the joystick/hardware UI path. B37 territory; no hardware here, untestable until his
field. Unchanged.

**T5.6** — B30, the new-path frozen-scene micro-instability. The one member of the stability class
no instrument in this project reads: the soak reads no pixel, and the fix is suspended for the
owner on tracking-convergence semantics (§11.94(d)). It is the single named gap inside the
evidence that retired T5.2.

**T5.7** — what the intern actually meets. The entry path was rehearsed by hand (§11.234) by an
LLM reading its own project. A part-time developer without an LLM has never been observed
following it, and his findings reach this project only through git history and the main tester
(§11.232(c)1) — there is no channel by which a confusion of his becomes a row here.

**T5.8** — whether the both-paths backlog disrupts a DEVELOPER. Q2's unresolved half. The
§11.163(h) test has never been applied under R0's criterion, and no measurement exists of what a
newcomer hits first: a both-paths defect, or a new-path one.

**T5.9** — the STATE of his content classes. Round-3 R21 bounds the SET (every class is in scope,
portrait included) and explicitly not the state — *"tested, but sometimes long ago"*. No class may
be called healthy on it, and nothing here can measure his install.

**T5.10** — `body action reload` is exercised by NO soaked show. §5.137's own trigger appears in
zero shows of the shipped playlist and zero of the tester's corpus, so the one measured false
success on the new path (an authored body leaves the drawn universe while every readout still
answers for it) is outside both soaks by construction.

**T5.11** — the DSO layer's field content under either reading of `[Y17]`. The field carries 527
lines of `body … mode in_galaxy` in `14.sts` alone, to migrate under either answer; how much more
of his corpus authors DSO content is unmeasured, and the design note says the count is a
measurement nobody has taken.

---

## Explicitly NOT blocking (so every exclusion is challengeable)

**B8, old-path removal** — transparent operation KEEPS the old path present: it is the comparison
baseline.
**B1 / B2 / B3, the architectural lines** — Vixy-paced by design. **The exclusion is weaker than
it was**: round-3 R21 retired the *"unless the content census says otherwise"* escape this map
used to lean on, so what carries them now is the pacing argument alone.
**Perfect parity on interactive free-flight residuals** — ≤2.3 m per toggle accepted with
structure (§11.154(a)); the usage-path model covers his interactive use.
**§5.20** — `linearOrbit`'s swapped lerp weights: no construction site on EITHER path, so it is
unreachable from both.
**§5.44 · §5.67 · §5.83 · §5.84 · §5.105 · §5.143** — old-path records and instrument caveats.
The old path is unchanged by construction (§11.52(b)); §5.84 is additionally a standing caveat for
any parity read taken right after a `move_to body`.
**§5.99 and the harness instrument residues** — verification tooling, not a product surface. The
exception is anything under `util/scedit/`, which is in the CODE tree and therefore R11.
**§5.150's special-orbit half** — measured as a null: 1790 past-date calls on Mars and Pluto left
their own next position bit-identical.
**Every open both-paths defect, UNDER T0's criterion ONLY** — §11.163(h). **This exclusion is
exactly what Q2 asks about**, and every member it covers is carried at T1.6, T1.10, T1.14 and in
`harness/artifacts/f120/sweep.tsv` rather than dropped.

---

## Maintenance, the archive convention, and the compile stamp

**Maintenance.** This file is a derived view; the ledger wins on every divergence and a divergence
here is a staleness bug here. **A state change** = the item's state line is REPLACED, with its
ledger pointer beside it — no dated annotation chain, no struck text; the history belongs to the
ledger and to the archive. **A closure** = the item leaves at the next archival pass and its id
becomes a gap. **An id is never renumbered and never reused**; a new item takes max+1 over
live ∪ archive **within its own tier**. **A reply id is always written qualified by its round**
(`round-3 R21`); a bare `R<n>` in this file is always the R tier (Q4).

**The archive convention.** The drawer is `claude/DEPLOYMENT-MAP/`; closed items move there as
`DEPLOYMENT-MAP/archive/<id>.md` by pure move, with a row in
`DEPLOYMENT-MAP/archive/MANIFEST-<date>.md`. References are NEVER rewritten: resolve any
`DEPLOYMENT-MAP.md:<line>` or `DEPLOYMENT-MAP/<id>.md` reference by probing the stated path first,
then with `archive/` inserted at the failing component. **Lateral search spans live ∪ archive** —
grep this file AND `DEPLOYMENT-MAP/archive/` together, never the live surface alone.

**Do not cite this file by line number.** Cite an item by its id, or a tier by its heading. The
one line-range citation that existed (`doc/developer-entry.md`, into the T4 tier) was already
false before this regeneration and now names the section instead.

**Compile stamp.** Regenerated wholesale **2026-09-18** by Claude Opus 5 (task F120, recorded at
INTENT §11.243) under the predecessor's own maintenance clause, at code `master-beta @ 52efbfa1`
and harness `CC-harness @ 4cfa64d`. The predecessor — 855 lines, md5 `3c7350ce…`, compiled
2026-08-29 and last amended 2026-09-13 — is byte-exact at
`claude/DEPLOYMENT-MAP/archive/2026-09-18-predecessor.md`. The item-by-item correspondence between
the two files is `claude/harness/artifacts/f120/disposition.tsv` (71 rows, one per predecessor
item) and it is proved mechanically by `claude/harness/f120_ids.py`; the ledger-side sweep that
produced everything marked *new this pass* is `claude/harness/artifacts/f120/sweep.tsv` (341 rows
over seven sets — 131 §5 · 16 §13.A · 26 §13.B · DECISIONS_PENDING (measured empty) · 12
FEATURE_REQUESTS · 43 SS · 112 §3 nodes — each set named by its resolving command). **ZERO ledger
states were changed by this regeneration** — it reads rows, it does not flip them.
