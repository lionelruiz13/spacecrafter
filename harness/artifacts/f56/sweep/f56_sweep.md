# F56 — the dim-era correction sweep: claim-level verdict table

**Scope**: every entry §11.156 – §11.168 plus §11.172, walked at its entry file.
**Occasion**: §11.174 — sessions 14 and 15 (2026-08-29, tasks F42–F52) were dispatched
without a Wayland display [vixy]; the rendered scene came out ~2.7× dark; no instrument
of the day noticed. §11.174(d)'s first-pass partition is a triage list; this is the
partition done per claim. §11.174(j) lifted the gate: *"the correction sweep proceeds
regardless — the dim-era numbers are wrong whatever latched them"*.

## Verdicts used

| verdict | meaning |
|---|---|
| **SUSPECT-STACK** | a photometric ABSOLUTE (a luminance, a mean, a high-threshold pixel count, a max) read from pixels rendered on 2026-08-29 in a faulty-dispatch session. The measurement happened; what it measures is the frame that stack produced, and it is not comparable with anything measured on another day. |
| **RELATIONAL-SURVIVES** | a ratio, an in-run A/B, an event-locking, a rank separation, an md5 identity, a same-run difference — with the argument stated PER CLAIM. Never a class waiver. |
| **CLEAN** | the claim's channel is not rendered pixels at all (source read, git, dump/JSON, TCP reply, app-log text, INTENT text), or the measurement is dated outside the dim window. |
| **RE-MEASURED CLEAN** | §11.174(d) or this sweep listed it suspect, and F56 re-took it on the healthy stack and got the same number (or the same bytes). A refinement of CLEAN, kept separate because it is *paid*, not *argued*. |

Two families are named where they occur rather than folded into a verdict:
**FAULT-CHARACTERISATION** — a dim-era pixel number that is wrong as a claim about the
product and *right* as a description of the fault (it is one of the only quantitative
records of the dim state that exists); and **DIRECTIONALLY SAFE** — a dim-era absolute
whose verdict survives *a fortiori* because the dim state pushes the statistic in the
direction that makes the verdict harder to reach.

## Summary

| entry | task | date | launches | verdict |
|---|---|---|---|---|
| §11.156 | F42 | 2026-08-29 | **none** | CLEAN (ledger text + git only) |
| §11.157 | F43 | 2026-08-29 | several | **MIXED** — geometry/dump CLEAN, 2 pixel clusters SUSPECT-STACK |
| §11.158 | F44 | 2026-08-29 | 1 | CLEAN (no pixel is read; every number is an angle, a dump field or a string) |
| §11.159 | F45 | 2026-08-29 | 9 | **RE-MEASURED CLEAN** on all three members §11.174(d) suspected |
| §11.160 | F46 | 2026-08-29 | **none** | CLEAN (source census + `nm`/`strings`) |
| §11.161 | — (supervisor) | 2026-08-29/30 | none | CLEAN (owner testimony + git) |
| §11.162 | — (supervisor) | 2026-08-29 | none | CLEAN (ledger text) |
| §11.163 | F47 | 2026-08-29 | 1 | CLEAN (gdb breakpoint counts; §5.53's disc-centre RGB explicitly NOT re-taken) |
| §11.164 | F48 | 2026-08-29 | 2 | **MIXED** — the binary A/B RELATIONAL-SURVIVES, every absolute SUSPECT-STACK |
| §11.165 | F49 | 2026-08-29 | **none** | CLEAN (ledger scan) |
| §11.166 | F50 | 2026-08-29 | 1 | CLEAN (script-log text, TCP replies, dumps) |
| §11.167 | F51 | 2026-08-29/30 | 1 | **MIXED** — the heaviest photometric entry in the corpus |
| §11.168 | F52 | 2026-08-30 | none | CLEAN (git-only, and dated after the healing) |
| §11.172 | F55 | 2026-08-30 | 5 | CLEAN (session 16, correctly dispatched; this is the entry that found the healing) |

**Claim-level count**: 14 entries walked · **17 claim clusters SUSPECT-STACK** (all in
§11.157, §11.164, §11.167) · **9 RELATIONAL-SURVIVES** · **6 RE-MEASURED CLEAN** ·
the remaining clusters CLEAN by channel.

---

## §11.156 (F42) — CLEAN

The entry states its own scope: *"HARNESS REPO ONLY — no product code, no data, no
harness script, no app launch"*. With no launch there is no frame. Every number it
issues is a file/stub count from `intent_pair_check.py` or a git state.

Twelve pixel-shaped values appear in it and **all twelve are quotations of measurements
made on other dates** — §5.26's cross counts (11583, 11082, 130, 129, 171, 100, 1022.3,
158.79/158.93 px, all 2026-07-25 / F4), §11.144's teleport counts (3 300 642 → 3 300 632
px>8, 2026-08-26 / F40), §11.98(c)'s oort onset (~100–112 AU vs 66.8 AU, 2026-07-24).
None is F42's and none is dated inside the dim window. **No marker owed.**

## §11.157 (F43) — MIXED

| node | claim | channel | verdict |
|---|---|---|---|
| (a) | the four reds attributed to instrument; ndc radii 0.6886/0.6918/0.6852, 2.342°, observer off by 2013.40 km; the geometry inverted from dumped distances | dump + geometry | **CLEAN** — every number is a dumped distance or an angle derived from one; no pixel enters the attribution |
| (a) 4th red | the stale click midpoint at ndc 0.697013 selects `''`; the control run reproduces the three reds and P3 passes | in-run control | **RELATIONAL-SURVIVES** — a selection outcome (a string) under a deliberately toggled variable |
| (b) | §5.109's seven ramp legs: 8687.00 / 9737.40 / 16 687.00 / 11 961.28 / 6949.60 km, the ramp samples | dump | **CLEAN** — `scaling`, `scalingTarget`, `scaledDatumRadius` are dump fields |
| (c) | b24_select green, reproducing §11.106's committed numbers (α 12.670°, screenSize 0.0432/0.0106, selDist 5.905047e-05 AU, 90 parity names) | dump + TCP text | **CLEAN** |
| (d) | the 30-file audit, its denominator and per-file classification | source/text | **CLEAN** |
| **(d) litguard** | **113 357 px>8 of 4 194 304 against guards of 20 000 / 1 000; the predicted disc-area band 101 646–107 836 px** | **PIXELS, absolute** | **SUSPECT-STACK** — measured 2026-08-29. *DIRECTIONALLY SAFE*: a dim frame yields FEWER px>8, so the true healthy count is ≥ 113 357 and the guards' 5.7×/113× margins can only grow. The band overshoot (+5.1 %) is likewise a lower bound. The verdict stands; the QUANTITY does not travel. |
| (d) `flag planets off` | 113 357 px before and after, identical **to the pixel** | in-run A/A | **RELATIONAL-SURVIVES** — an identity between two frames of one run; both would be dim together |
| **(e) ladder** | **centre luma 3.1 → 60.7 · 1 909 800 lit px · caps 29.2/30.8/59.1/127.0/316.0 px · b20 shadow 2313 · per-leg luma 103.21/90.27/87.01/71.14/52.47/20.97 · `site_luma` 20.97 < 30 (the one red) · the ±1.4–1.6 % / −6.4 % / −17 % drifts against §11.104(d)** | **PIXELS, absolute + cross-epoch** | **SUSPECT-STACK.** The comparison is 2026-07-25 (healthy) against 2026-08-29 (dim): it measures the dispatch fault, not five sessions of drift. The b250 red (`site_luma` 20.97 vs ≥ 30) is the sharpest consequence — a RED GATE that may be entirely the faulty stack. The July side (181.39 centre luma) is CLEAN. |
| (e) invariance | `nadir_lon` bit-identical under both conventions; the prediction file byte-identical | arithmetic | **CLEAN** |
| (f) | the rebuilt stack verified: `Meta-0 2448x1332 59.96`, GPU-real, Swapchain (1024,1024), `mouseNorm` (511.5,…) | X/app text | **CLEAN — and this is the node §11.174(e) indicts.** Every member it checked passed. The member it did not have was photometry, which is exactly the one that had failed. |
| (g) | the two drifted scenes; b3_ladder's entry point | source/text | **CLEAN** |
| (h)(3) | *"do not treat the corrected file as a green baseline"* + §11.164's discharge | successor rule | **SUSPECT premise** — the discharge rests on (e)'s dim numbers |
| (h)(4) | *"a stack change since 2026-08-26 is a candidate variable for any A/A floor"* | successor rule | **RE-MEASURED CLEAN** — F56 measured the A/A floor across the boundary: unchanged to the digit (374 px / 3-of-255) |

## §11.158 (F44) — CLEAN

One live launch on the F43 stack, named at (k) (`XAUTHORITY` under `/tmp/rt-claude/`,
`DISPLAY=:2`, `xdpyinfo` verified). **Nothing in the entry reads a pixel.** The parity
headline (median 60.3636° old-vs-new), the ≤ 0.00187° offline reproduction, the
−90.000283° zero point, the alt/az control (max |Δalt| 9.97e-06°), the free-branch
0.000000° spread, the Eris 1.198373°/1.198612° pair: all are angles from the `.navstr`
sidecar and the JSON dump, or offline recomputations of them. A frame was rendered and
never read. **No marker owed** — and the entry is a useful positive: a session-14 task
whose whole result is untouched, because of its CHANNEL, not its luck.

## §11.159 (F45) — RE-MEASURED CLEAN (all three members)

§11.174(d) listed *"§11.159's A/A floor (374 px) and its cadence H1 discharge"* as
suspect. Both are now paid, and a third member falls out of the same measurement.

| node | claim | F56 measurement | verdict |
|---|---|---|---|
| (d) P1 | the eight star-field frames fall into exactly **two md5 classes**, and the classes cut across the leg variable | today's two frames land in **class A** — md5 `a09147d7`, BYTE-IDENTICAL to `empty1`, `empty3`, `empty4` of 2026-08-29; class B (`9baf6ed4`) holds `empty2`, `full1`, `full4` | **RE-MEASURED CLEAN** — the null is reproduced across the dim boundary, to the byte |
| (k)(4) | the **A/A floor: 374 px of 4 194 304 at max channel delta 3/255** | every cross-class pair, in either epoch and across it, differs by **exactly 374 px at max delta 3**; today's two runs are identical | **RE-MEASURED CLEAN** — the floor is not stack-dependent |
| (k)(7) | the cadence: **4392 lines over a [30.0, 30.5] s bracket = [144.0, 146.4] fps**, H1 = the config cap | two launches today: **4392 lines / [30.0, 30.500] s**, same target star hp 677 | **RE-MEASURED CLEAN** — F45's number to the digit, twice, on a healthy stack |
| (f) P6 | per-frame cost Δ = +0.0105 ms, rank-separated 4 v 4, with a negative control of opposite sign | — | **RELATIONAL-SURVIVES** — an interleaved in-run A/B with its own null; and the cadence identity above shows the frame clock did not move between the epochs either |
| (g) | 153.0 B/frame across two flushed streams, 22.0 kB/s at 143.999 fps | — | **CLEAN** — byte counts of log files |
| (e)(j) | the spectral readout, the spInt census (836 distinct values, 4056 max), the root split | TCP text + files | **CLEAN** |

**The by-product matters more than the discharge.** The star-field frame is a
photometric channel (10 832 lit px, lit mean 36.58) that is byte-identical across the
dim boundary. It is coloured from `HipStarMgr::color_table` (§11.159(a)), not from the
`s_texture` entry the Moon's two paths share. Had the Moon's measured transform
(×0.3717) applied here, **4825 of 10 832 lit pixels would have dropped below L = 8** and
the lit mean would have gone 36.58 → 13.60. Zero pixels moved. **The dim state was
confined to textured-body rendering; every global-output-transform candidate (gamma,
colour space, swapchain format, compositor-side conversion) is eliminated.**

## §11.160 (F46) — CLEAN

Header: *"no launches"*. The census (150 assert sites, 46/16/88 ownership split), the
build-flag reads, the `nm -DC` / `strings` probes with their controlled fixture — source
and binary inspection. The three `px`-matching lines are the C++ pointer variable `px` in
`intrusive_ptr.hpp`; the `screenshot` hit is `save_screen.cpp` named as a code path. **No
marker owed.**

## §11.161 / §11.162 (supervisor entries) — CLEAN

§11.161 is owner testimony plus a `[derived]` mapping; its (f) DISCHARGED block is F52's
git-only measurement dated 2026-08-30. §11.162 compiles a derived view over ledger rows.
Neither runs anything. §11.162(d)'s *"every measured datum they were waiting on is, as of
session 14, PAID"* is a completeness claim whose warrant runs through data this sweep
partitions — **noted, not marked**: it makes no photometric claim of its own.

## §11.163 (F47) — CLEAN

One launch under gdb. The discriminating result is four breakpoint hit counts (1/1/1/**0**
over three resolved locations) with a positive control — invariant under any photometric
transform. (a) N1 states that §5.53's disc-centre RGB is deliberately **NOT re-measured**,
so the one pixel datum in the neighbourhood is inherited, not taken. (g)'s dense "px"
numbers (180, 409.6, 0.2, 2048, 2.28×) are **source constants** and their arithmetic.
**No marker owed.**

## §11.164 (F48) — MIXED

| node | claim | channel | verdict |
|---|---|---|---|
| **(a)** | **the `922701c9` arm reads 29.191/30.830/59.100/126.969/316.052, site_luma 20.97, shadow 2314, centre luma 60.7091; the current binary 29.186/30.835/…, centre luma 60.7155** | PIXELS, absolute | **SUSPECT-STACK** as values. **But the CONCLUSION — signature P refuted, the drift is not the product's C++ — is RELATIONAL-SURVIVES**: both arms ran on the same stack, the same instance, the same day, and the comparison is arm-to-arm. Two dim numbers still compare validly with each other. |
| (a) split-by-date | four 2026-07-25 runs at centre luma 181.386–181.389 vs three 2026-08-29 runs at 60.71 | cross-epoch | **SUSPECT-STACK as an "environment epoch"** — the split by DATE is real and is now ATTRIBUTED: 2026-08-29 is the faulty-dispatch day. The entry's *"dated environment"* reading was right in form and wrong in cause (it offered the driver bump; §11.172(e) refuted that, §11.174 named the dispatch). |
| (b) | the reproduction control: **0 differing leg fields**, six distances and seven scalars bit-identical to F43's committed run | in-run/same-day identity | **RELATIONAL-SURVIVES** |
| **(c)** | **mean luminance 165.26 → 61.43, hf 6.644 → 2.464; lit support unmoved (3 300 873 vs 3 284 272 px>8, XOR 16 601); ratio flat radially 0.334–0.378, structured angularly 0.180–0.540** | PIXELS, cross-epoch | **SUSPECT-STACK / FAULT-CHARACTERISATION.** As a claim about the product it is void (§11.172(e) already corrected the present tense). As a description of the dim state it is one of the two best records that exist — and the *"the model did not move; only the pixels did"* finding is exactly right, now with a cause. |
| (c) heightmap | b20 stays invisible (0 changed px), b30 30.83 in a 28.9–30.1 band | same-run differences | **RELATIONAL-SURVIVES** — difference counts within one run; the ray-march reads real relief in both epochs |
| (d) | the driver bracket (1721 applogs, 1639/80, boundary (2026-08-23 08:44, 2026-08-26 11:06]); three `creating uninitialized texture` events | LOG-TEXT | **CLEAN** — and now known to be a **measured coincidence** (§11.174(d) said so; §11.172(e) refuted the driver by re-measurement on the same driver) |
| (g) | `boundingRadius` 1772.148246090878 km on both binaries | dump | **CLEAN** |
| (i) | the settle traces: 5872.42 vs 1771.60 km on first poll | dump | **CLEAN** |
| (j) | build provenance, binary md5s, shader byte-identity (214 files, 0 differing) | git/files | **CLEAN** |
| (k) | the stack caveat: same instance, pid 147372, `Meta-0 2448x1332 59.96` | X text | **CLEAN as stated — and this is the node that shows the fault was invisible**: every stack member the task could check matched, and the frames were dim anyway. |
| (l)(2) | *"§11.104(d)'s absolute numbers are no longer reproducible on this host"* | successor rule | **CORRECTED** — they were not reproducible **on 2026-08-29**. §11.172(k)(3) already extended the non-target rule to this entry's own numbers; the sweep adds the reason. |

## §11.165 (F49) — CLEAN

*"no launch"*. Ledger-scan counts (71/74/82 events, 92/95/116 pairs, 68/64/80 unmarked,
the 20/24/13/3/4 partition) and quoted, non-photometric relayed measurements (3/54 vs
1/54 HUNG counts, 6949.6 km, 179.999429°). **No marker owed.**

## §11.166 (F50) — CLEAN

One launch on the F43 stack, named at (n). The terminal observable is the app's own
SCRIPT LOG: seven `print` values per leg over five legs, plus `get status object` and the
dual dump. The character-for-character identity between leg C and both `deselect` legs
(the entry's central finding) is a **text** identity. `selected_magnitude` −10 / 1.49643
is an astronomical magnitude from `getMag`, not a pixel. The (j)-closing back-marker's
*"a real white illuminate"* and the `ra :00h00m` overlay are imported from §11.170 (F53,
2026-08-30, healthy session). **No marker owed.**

## §11.167 (F51) — MIXED, and the densest node in the sweep

| node | claim | channel | verdict |
|---|---|---|---|
| **(a)** | **`disc_mean` 61.431 / `hf_mean` 2.464 at every one of 72 samples over 355 s, range 0.0; all 72 PNGs one md5 `0c7de389`, identical to F48's committed frame** | PIXELS, absolute | **SUSPECT-STACK** (already CORRECTED by §11.172(e) for its present tense; the sweep adds the attribution). The FLATNESS is **RELATIONAL-SURVIVES** — 72 identical frames is a within-run identity. |
| (b) | `evalCount` 6063 → 57 621 = [143.9, 144.9] eval/s; the old-path frame differs, the toggled-back frame is byte-identical | dump counter + md5 identity | **CLEAN / RE-MEASURED CLEAN** — the evaluation cadence is a dump counter, and F56's independent line-per-frame count today gives the same [144.0, 146.4] fps band |
| (c) | 391 texture events, 390 before the first TCP command, 0 inside the window; `Can't upload` 0 in 829 committed logs | LOG-TEXT | **CLEAN** |
| (d)(1) | fine structure REGISTERED: r_hf **0.6189** at zero shift, collapsing to ≤ 0.004 at 16–128 px | cross-epoch correlation | **RELATIONAL-SURVIVES, reading narrowed** — a correlation between a July frame and a dim frame is immune to a level change; it establishes that the dim frame carried the same surface features in the same places. That is now a statement about the FAULT (the content was there; the shading was not), and it is corroborated by today's healthy runs drawing from the same cache. |
| (d)(2) | the disc keeps the source texture's TINT: R/B 1.0278 July, 1.0241 today-new, 1.0205 today-old, file 1.0301 | within-frame ratio | **RELATIONAL-SURVIVES** — a colour ratio inside each frame |
| (d)(3) | the old path draws a detailed Moon: 2 748 124 px>8, hf 1.744 | PIXELS, absolute | **SUSPECT-STACK — already CORRECTED by §11.172(f)** (the old path was dim too: 42.476 → 160.142 today). Marker exists; the sweep adds the cause. |
| (d)(4) | the no-cmd branch is transient, read at source | SOURCE | **CLEAN** |
| **(e)** | **the dark class: 184 437 px < 16 and 530 895 < 32 today vs 0 and 3 in July; 522 070 px (20.5 % of the disc) below 32 where July was above 100; percentiles 110/129/…/201 vs 10/12/…/123; r² 0.408 (not a tone map); per-leg ratios ×0.537 … ×0.140, monotone in offset** | PIXELS, absolute + cross-epoch | **SUSPECT-STACK / FAULT-CHARACTERISATION.** This is the sharpest quantitative portrait of the dim state anywhere in the ledger — a non-pointwise, position-dependent darkening of a textured body. As a claim about the shipped product it is void; as evidence about the fault it is the most valuable thing in the entry, and it composes with F56's star-field null: the transform touched textured-body shading and nothing else. |
| (f) | P2/P3 did not land; the 7.73° attitude divergence between paths (`mat` first rows, `axisRot` agreeing to 1e-5°) | dump geometry | **CLEAN** — the attitude finding is dump-based and untouched by photometry; it remains routed to Vixy |
| (g) | no §5 row minted, the reach unattributed | reasoning | **CLEAN, premise corrected** — the surviving candidate it names (the driver) is refuted (§11.172(e)); the reach is now attributed to the dispatch fault, and the restraint is vindicated |
| (h) | the driver census re-derived: 1721 common, 0 disagreeing, denominator 1727 = 1639/86/2; per-directory epoch labels (263 dirs, 252 pre, 8 post) | LOG-TEXT | **CLEAN as a census.** The **"epoch" framing is SUPERSEDED**: the labels partition runs by driver version, and the driver is refuted as the carrier. The partition is still a true statement about driver versions; it is no longer a partition into photometric epochs. |
| **(i)** | **the photometric-baseline census: 283 → 59 → 46 → 45 members; verdicts 34 CLEAR / 9 FLAGGED / 4 FLAGGED-WEAK; the EXPOSURE TABLE (T=8 → 0.995 … T=128 → 0.024; disc mean ×0.371; site_luma ×0.537…×0.140)** | mixed | **SPLIT.** The denominator, the criterion and the per-file classification are **CLEAN** (source/text). The **exposure table is SUSPECT-STACK / FAULT-CHARACTERISATION** — it is the dim transform's own profile, measured on the dim frame pair, not an "epoch" property. Consequence for readers: the census's *ranking* of exposure is a ranking against THAT transform. Its structural criterion (an absolute constant threshold on a textured body's pixels) is what makes a gate fragile, and that stands. The margins F54 later recorded (5.16× / 6.07× / 6.35× · 15.87× · 116.9×/59.8× · …) were measured 2026-08-30 and are **CLEAN**. |
| (i) 5. | *"the ladder's `site_luma >= 30` is ALREADY RED at 20.97"* | PIXELS | **SUSPECT-STACK** — see §11.157(e); the red may be the stack |
| (j)(3) | *"use the exposure table, not a single 2.7×"* | successor rule | **CORRECTED** — the table describes the dim state, which is not a standing property of this host |
| (j)(4) | *"do not treat §11.104(d), §11.105's 177/175.9, b3_earth_ab's four views, f18_level's disc means, f21_s563's lit counts as cross-epoch targets"* | successor rule | **STANDS, with its reason replaced.** Cross-day absolute photometry is untrustworthy — but because a one-day fault can move it invisibly, not because a driver bump moved it permanently. F56's canary is the instrument that makes the rule enforceable instead of advisory. |

## §11.168 (F52) — CLEAN

Git-only, no launch, no build, dated 2026-08-30. Every number is a commit statistic.
(m)'s instrument post-conditions are counter identities. **No marker owed.**

## §11.172 (F55) — CLEAN

Session 16, correctly dispatched (§11.174(a)); five launches on 2026-08-30. This is the
entry that measured the healing (165.258 / 6.644 from a bit-identical dumped state) and
the three-stage preview→big transient with its event locking. Its (i) named the shared
texture cache as *"the one lead this task can hand over"*.

**F56 answers (i) with an instrument and a measurement**: `f56_manifest.py` brackets a
launch with a recursive md5+mtime manifest of `~/.spacecrafter/cache`, and the canary's
own reference run produced **zero mutations** — not a rewrite, not an mtime. So the
write §11.172(i) measured (`t-bodies-moon_normal.dat`, mtime 2026-08-30 10:09:21, inside
F55's run 1) is **EPISODIC, not per-run**; the manifest reads that very mtime, still the
last one, which is what makes the zero a measurement rather than a blind spot (the
instrument is separately mapped both ways on a control: 4 of 4 mutation classes detected,
0 on the null arm). The cache remains a real cross-run state variable — now watched.

---

## What the sweep changes about §11.174(d)'s partition

1. **Two of its four named suspects are cleared by measurement, not by argument**:
   §11.159's A/A floor (374 px) and its cadence H1 discharge both reproduce exactly on
   the healthy stack. The partition was right to suspect them (they were unpaid); it is
   the payment that clears them.
2. **"LIKELY CLEAN" is confirmed per claim, and the reason is CHANNEL, not luck.**
   F44, F47 and F50 all LAUNCHED THE APP on the faulty stack. They are clean because
   they read dumps, gdb breakpoints and log text — a rendered frame existed in all three
   and none of them looked at it.
3. **The suspect set is exactly three entries**: §11.157 (2 clusters), §11.164 (3),
   §11.167 (5). No photometric absolute from sessions 14–15 lives anywhere else in
   §11.156–§11.168.
4. **One suspect claim is a RED GATE**: b3_ladder's `site_luma >= 30` failing at 20.97
   (§11.157(e), §11.164(a), §11.167(i)). A failing gate attributed to the environment,
   where the environment is now known to have been faulty, is the sweep's single
   highest-value follow-up: one launch of the corrected ladder on the healthy stack
   decides whether that red exists at all. **RUN — see below.**
5. **The dim state's REACH is newly bounded** (F56's star-field null): it did not touch
   the star channel by one bit. Whatever it was, it acted on textured-body shading.

---

## The follow-up, RUN: b3_ladder on the healthy stack (P9, committed at `5fc4200` before the launch)

`b3_ladder_run.sh terrain --site moon`, the file UNCHANGED, no gate constant touched,
one launch, comm probe 0, md5 in == out. **LADDER GREEN — 0 FAILURES.**

| quantity | 2026-07-25 (July, committed) | 2026-08-29 (dim) | **2026-08-30 (today)** |
|---|---|---|---|
| centre luma | 181.3887 | 60.7155 | **181.3897** |
| frame lit px>32 | — | 1 909 800 | **3 207 131** |
| lift20 cap radius | 28.812 | 29.186 | **28.840** |
| b30 cap radius | 30.346 | 30.835 | **30.330** |
| b45 cap radius | 58.265 | 59.100 | **58.265** |
| b90 cap radius | 125.314 | 126.969 | **125.311** |
| b250 cap radius | 337.505 | 316.050 | **337.502** |
| b20 shadow witness | 2790 | 2313 | **2794** |
| **b250 `site_luma`** (the RED gate, bar ≥ 30) | 149.93 | **20.97 — FAIL** | **149.93 — PASS** |
| per-leg site_luma | 192.13 / 194.36 / 202.43 / 186.16 / 171.96 / 149.93 | 103.21 / 90.27 / 87.01 / 71.14 / 52.47 / 20.97 | **192.15 / 194.36 / 202.43 / 186.16 / 171.95 / 149.93** |
| disc metric on `terrain_base_zoom.png` | 165.258 / 6.644 / 9.514 / n 2 544 661 | 61.431 / 2.464 / 4.230 / n 2 535 950 | **165.258 / 6.644 / 9.514 / n 2 544 661** |

P9a, P9b and P9c all land. **The ladder's entire residual was the faulty dispatch**:
the red gate, the ±1.4–1.6 % cap drift, the −6.4 % on b250 and the −17 % shadow witness
all vanish, and the six per-leg luma values return to July's to two decimals. The dim /
today ratios re-derive §11.167(e)'s profile to the digit (0.5371 · 0.4644 · 0.4298 ·
0.3821 · 0.3051 · 0.1399 against its ×0.537 · ×0.464 · ×0.430 · ×0.382 · ×0.305 ·
×0.140), which is the same transform measured from the other side.

**Three consequences beyond the ladder.**

1. **§11.164(l)(2)'s standing rule is REFUTED as stated.** *"§11.104(d)'s absolute
   numbers are no longer reproducible on this host and must not be used as a target"* —
   they are reproduced today to two decimals on six legs and to the printed digit on the
   disc metric. The rule was written from inside the fault. What survives is the
   weaker, still-correct form: cross-day absolute photometry is only trustworthy with a
   stack check, which is what the canary now is.
2. **The driver bump is measured photometrically inert.** July ran on `580.568.0`,
   today runs on `580.636.192` (§11.164(d)'s bracket), across a reboot and a rebuilt
   display stack — and the numbers are the same. §11.172(e) refuted the driver by
   re-running the dim scene; this refutes it from the July side too.
3. **A cross-epoch A/A floor for a TEXTURED body now exists.** July's frame vs today's:
   464 244 px differ at all, but only **28 px by more than 3/255**, max delta **15**,
   and all four disc metrics agree to the printed digit. (The 374 px / 3-of-255 floor is
   the STAR-FIELD scene's; floors are per scene, and this is the Moon scene's, measured
   across five weeks rather than within a day.) Today's ladder frame is also
   **byte-identical** (md5 `5215565b`) to the canary's own reference-scene frame taken
   by a different driver script — the two instruments agree to the byte.
