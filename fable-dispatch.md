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

**Update [Claude Fable 5.1 2026-09-07, supervising session 27 — LovelyFoxDev, the SATURDAY-TOOL /
TWO-STEP-SOLVER / DUMP-CHANNEL round: F103 · F104 · F105]:** trigger = the §0b verbatim line and
nothing else. Open at **Monday 08:57 CEST** — a weekday morning; the owner replied to the session-26
close at 08:4x (§11.223, recorded by the previous session before it closed at `805c157`), so this
session asks NOTHING in-line and writes its decisions to §3 in Q-70's shape. Warm-up (every value
`date`-measured 08:57:10–08:58:40, the command beside the claim — Q-67): both trees CLEAN at open,
code `474c595d` / harness `805c157` (the four post-close commits of session 26: the close, its
M-delta correction, §11.223, the Y2 site correction); definition-drift assert MATCH (`8e364a3a`);
binary `b5f08778` current (mtime 2026-09-06 23:21, 0 `src/` files newer); same boot as sessions
21–26 (`uptime -s` 2026-09-04 18:45:08); `:2` 2448x1332 under `.5KBYU3`, `loginctl` sessions 14/15
alive (the owner's own seat0 sessions listed beside them — since boot, not new); canary `--no-scene`
**exit 0** (30 members, `f56/canary/20260907-085838`); config/ssystem md5 pristine
(`03fbee59`/`545a51ef`); no `spacecrafter` in `/proc/*/comm`; no lock file; RAM **52 GiB available
of 59**, `-j24`; next free §11 **224** (live ∪ archive, `max+1` — my first one-liner printed the
max, 223, and was corrected by the premise-block command before it reached a section); unpushed
**94 code / 796 harness** at open. Live `### F` **4 → 0** by **archival pass 19** at OPEN (update-s25 +
F99/F100/F101/F102, 867 lines + two seam tidies = 871 removed, 2986 → 2115; manifest
`2026-09-07-pass19`, pre-md5 `9dbab0d0` reproduced in-process AND from disk, archive files written
before the live surface — Q-56; commit `06eec39`; **nothing carried**) **→ 3** by the mints below.
Instrument baselines at open (run 08:58:34): scan **254/322/144** · pair-check **239/214/25/118** ·
D 35 · **D2 12** · I 89 · I2 37 · **M 89** — over the session-26 close (252/319/143 · 238/213/25/118 ·
D2 11 · M 88) every delta is §11.223's two commits: +1 entry file/+1 live pair, +2 raw lines/+3
pairs/+1 uncredited, D2 +1 = §11.223's stub strike (the Y2 correction), M +1 = `11.223 [entry_only
11.215, 11.220, 5.145, 5.84]` (entry-wins, licensed) — recorded, not chased. QUEUE CONSUMPTION
(the session-26 close's re-ordered list, in order): (1) pass 19 — DONE; (2) **`supervised-by.sh`'s
silent fallback made LOUD → F103** (S–M — widened by the read: B1's fix is the one line §11.212(g)
priced, but the tool the owner operates Saturday also DISCARDS the maps it builds — `build_map` writes
into a `mktemp -d` the EXIT trap deletes — and §11.223(d)'s requirement that the old→new map be
committed is the same tool's change; his phrase "commit tracking" is read as that map — a veto
point; exercised on a throwaway nested clone pair whose scratch origins are set behind by the live
counts, the live pair `--dry-run` only); (3) **§5.145's fix → F104** (S–M — the corrected ruling as
written: two solver steps per call, the loop untouched; the SCOPE read as the iterative CLASS — all
four advancing branches of `eccentricAnomaly` + both comet steps — with Eris's branch alone built as
the MUTATION so the two forms are measured side by side, a veto point; the three-class partition of
the 120 records predicted from the field census: 60 iterative (the six `comet_orbit` bodies are ALL
parked), 60 not); (4) **§5.146** — on his word, NOT minted (§11.223(c): the contract stated, "edit
it" not yet said); (5) **the dump channel's two items → F105** (S–M — widened by the read: §11.220(j4)
is not a residue beside (j3), it is 8 of the 30 new-only records — the seven hidden children of the
never-walked `Universe` and `orbit_autour_point` — that the barrier call would refresh into the
IDENTITY frame; so the call lands WITH the barrier's missing precondition (a never-published frame
is not served, one D12 line per body) and the mutation measures (j4)'s magnitude once; the partition
(20, 2, 8) predicted by name); (6) owner-word items (§5.142's policy, `orbit_lon`, §5.144's leg),
(7) the (g) tail, (8) instrument residues, (9) riders — carried, not minted. Picks: **F103 → F104 →
F105** (the Saturday tool first — no engine change, independent; then the engine change whose
baselines F105 inherits; three executors, §0b.2's sweet spot). Deliveries: all to the parent
(§11.224+, refreshed at each dispatch). Launch classes: F103 NONE (no launch, no canary — said in
its section); F104 and F105 FUNCTIONAL (`--no-scene`). Remotes: local contains origin on both;
push impossible here — the owner's push is R5, Saturday/Sunday per §11.223(d).
**Round outcome (session 27 close, 2026-09-07 — every time in this note is pasted `date` output; the
close commit's own clock is the stamp):** F103 → **§11.224** + §11.212(g) B1 FIXED (no fallback: a STOP
at the preview, `--branch-alias`, the maps persisted under `claude/sha-maps/`) + TWO pre-existing
blocking defects of the tool found by its first end-to-end run and fixed with mutants (D1: `build_map`
killed every run after the code rewrite and before rollback; D2: the repair map re-derived after the
rewrite aborted every run on a pair with a dangling trailer — this pair has 2) + **§5.147 minted at
acceptance** (the 13-commit collapse under `commit-tree`, the owner's decision before Saturday) · F104 →
**§11.225** + §5.145 FIXED (two solver steps per call, both paths; Eris 1.1987° → 1.06e-05°; exactly one
record of 120 moved; the F91 table `1fe630a4`) · F105 → **§11.226** + §11.221(n1)/§11.220(j3) DISCHARGED
(the header's `oldSystem`/`inSystem`; a dump a use for 120/120) + the barrier's precondition (a
never-published frame is REFUSED, one D12 line per body — the 8 anchors under `Universe` keep their
honest zero; `useNow()` returns `bool`) + **§5.148 minted at acceptance** (tracking a zero-vector body
writes 66 NaNs into the old path's view state, on the baseline binary too) — **three for three
delivered AND supervisor-verified same session**, every delivery re-run by my own hand (F103: the
live pair's `--dry-run` with HEADs asserted, D1's isolated case, D3's twins off the live repo, 883 map
pairs re-checked, P1 → P2 → P3 reproduced on the scratch clone; F104: `seqcheck` 120/120, F100's freeze
leg with Eris 1.066e-05° from my own dump, F91 `1fe630a4`, the smoke suite; F105: the dump stage's
(20, 2, 8) by name with the 8 D12 lines, the `p23` header `galactic`/false, the operator arm, F91, the
smoke suite's 10-line reload window). Code `474c595d → ead2d478 → a30b2c75 → 318c0c8b → 48cc3727`
(four executor commits: the solver + comments; the header field; the barrier call + precondition;
the refusal line); binary `b5f08778 → 0c61f1b5 → e411b838`; harness `805c157 →` this close. SUPERVISOR
ACTS: archival pass 19 (`06eec39`); three mints under the PREMISES rule (`fe09601`; 39/31/27 PASS at the
mint after three instrument catches — two grammar collisions and one count the mint commit itself
would have falsified, rewritten as a floor); three acceptances (`2d21b13`, `65e2256`, this close) each
after my own runs; **§5.147 and §5.148 minted** at acceptances with markers at their attributing nodes
(§11.224(h), §11.226(f), §11.216(i) — entry and stub each); the per-round premises refreshed at each
dispatch by content-located replace (F105's `:2121 → :2127` after F104's comment edit); `README.md`'s
map sentence corrected (`purge-path.sh` writes no map); `DEPLOYMENT-MAP` R5 annotated; in `~/shared`:
Q-67 ×3 (the mint-time set; F103's six; F104's assembly sub-class). OWNER EVENTS IN-SESSION: none —
the trigger line only; nothing asked (Monday). HOST: same boot throughout (`uptime -s` 2026-09-04
18:45:08); `loginctl` sessions 14/15 and `:2` 2448x1332 at open and at close (12:31); RAM 52 GiB
available at both; 30 measuring launches by the executors + 11 by me, canary green and `/proc` clear
before every one, no lock file at any check, the field pair `03fbee59`/`545a51ef` in==out throughout
— **no HOST-EVENTS entry owed**. SUPERVISOR TALLY: **eleven dispatcher defects** (3 value-class, 8
structure-class), all output-side, none reaching a delivery — F103's six (the whole-history trailer
count where the tool consults the range's 138 pairs; "0 dangling" where the live pair has carried 2
since July, the tool's own `--dry-run` unrun at the mint; the code preview "to the digit" under a
fixture that empties the code range; "map lines = rewritten commits" with the repair append in my
read; `README.md:129` called the repo contract; "validated at parse time"), F104's three (the 60/60
double count against my own census line; Sedna in Eris's branch against my own paragraph; the F100
result-file name), F105's two (the mass-ratio band for a barycentre declared `a = b = 1` — half the
distance, 39× off; class I's membership 29 → 27 left unstated); plus the three the instrument caught
at the mint. EXECUTOR REPORT DEFECTS: none reaching a record (F105's marker string and first driver
self-caught; F105's one malformed `Code:` trailer, `5832b27`, left unrewritten by design). EXECUTOR
criterion-integrity instances: **≥ 24** (F103: the predictions before any run with three section
claims pre-registered as refuted; the fixture pinned at the dispatch HEAD; P6 as two scripts at one
instant; both mutants; the maps fingerprint-verified; the "commit tracking" reading flagged. F104: the
structural seqcheck shown able to fail both ways; the partition from the field file before the fix;
the pre binary reproducing the defect first; the `altaz_old` instrument corrected when its own
mutation leg beat its floor; the mutation kept as a null with its cause; the S5 refutation controlled.
F105: eight values predicted numerically before the build; the reload window pre-registered as its own
risk and caught by "exactly 8"; the message corrected, not the logic; a manufactured arrear removed
rather than credited; the zero-vector consequence routed, not minted).
Archival pass 20 (update-s26 + F103/F104/F105, live `### F` 3 → 0) DEFERRED to the next open.
NEXT-ROUND QUEUE, in order: (1) archival pass 20 at open; (2) ~~**BEFORE SATURDAY, the owner's:** §5.147's
decision (accept-and-map / refuse-and-resolve) — the rewrite is his act, the tool is ready (`§3 [Y8]`)~~
**[ANSWERED 17:5x, §11.227: neither — the collapse is the tool's defect; F106 minted and dispatched
in this session, post-close; the queue's (2) is then whatever F106 leaves owed]**;
(3) **§11.225(j2)/(j3)'s leg** (S, decision-free: does the orbit-line sampler's walk through
`iterativeLastE` leave the seed where Eris's +32 rad staleness says it was? `f104_solver.py`'s sweep +
one launch with `flag orbits` on/off, predictions first); (4) **§5.146** on his word (EntityCore); (5)
ON THE OWNER'S WORD: §5.148 + §11.216(i)'s zero-vector answer · §5.142's policy · the `orbit_lon` ruling
→ §5.21's two halves + §5.140 · §5.144's 8–12 h leg; (6) the (g) tail: §5.115's retention design,
§5.66+§5.71, A15's residual; (7) instrument residues: `purge-path.sh`'s own sha map (the README says
it is owed); `list_code_trailers`'s silent skip of an unparseable `Code:` line (a warning line — S);
the `dumpread` self-test's two synthetic cases (§11.226(c)); `f96_offset.parse_dump` as the fourth
reader bypassing `dumpread` (§11.226(i1)); the scan's EVENT lexicon without `FIXED` (by ruling —
name, do not change); a PREMISES line per cited pointer (Q-67's candidate); recursive importer
censuses; `f99_sweep.py`'s old-chain blind spot; `f85_links.py` guard; `f89_p7.py margins`; the b4
`/proc` probe; the `dumpread` duplicates (`b24_equivalence`/`f89_p7`); (8) riders: the guard's
"outside the walk" vs "not walked yet" (needs the system's identity inside `ModularBody` — the owner's
nested line), §5.84's own trigger, `observedToBodyLocalPos` with no consumer, the `[parallel-script]`
question, scedit README `:43`, the tester's `panorama5.sts:102`, `TDRS 3` and the 13 duplicate names in
`06old.sts`; (9) owner items per §3. Remotes: **98 code / 821 harness** unpushed before this close's
commit (measured 12:31:33); push from a keyed host — the supervisor never pushes. BASELINES AT CLOSE
(measured 12:34:12, after this close's mints and markers): scan **258/328/142** · pair-check
**242/217/25/120** · D 35 · D2 12 · I 89 · I2 37 · M 89 — over F105's delivery (258/328/142 ·
242/217/25/119): +1 inline stub = §5.148; the scan unchanged (a MINTED marker and a "MEASURED" marker
are outside its event lexicon, the shape §11.220(k) and §11.226(m) record); no filter moved; the
uncredited set diffed EMPTY against the delivery tree with the instrument's own lines.

---

**Update [Claude Fable 5.1 2026-09-06, supervising session 26 — LovelyFoxDev over SSH from the
laptop, the NULL-PARENT / PARKED-READOUT / SYSTEM-SWITCH / UNIFORM-POOL round: F99 · F100 · F101 ·
F102]:** trigger = the §0b verbatim line PLUS one environment note [vixy 2026-09-06, verbatim]:
*"First attempt over ssh from the laptop (TravellingFoxDev) - I have no physical access to the
desktop (LovelyFoxDev) - precising in case it changes something."* — no in-line transmission
beyond it. Open at **Sunday 21:24 CEST**, outside the owner's reliable window ⇒ this session
asks NOTHING and closes with a ~~compact~~ decision list **[CORRECTED IN-SESSION 2026-09-06 on the owner's word → Q-70: the proxy is the reader's working set — held set per position · reach to the furthest local element · non-lexical anchors — never density; "compact" was a decision-depth bound mis-mapped onto text length since session 23; the §3 block of this close is the first in that shape]**. What the ssh change DID change, measured:
nothing that reaches a launch — the owner's RDP-created real logind session survived his move
(`loginctl` lists claude's sessions 14/15 unchanged), `:2` answers `xdpyinfo` at 2448x1332
under `.5KBYU3` exactly as HOST-EVENTS 2026-09-04 banks it, and the canary is green (Q-61's
(3): the display lives with that session, not with his seat). Warm-up (every value
`date`-measured 21:24:37–21:26:29, the command beside the claim — Q-67): both trees CLEAN at
open, code `22499f04` / harness `d9b911e` (the session-25 close commit); definition-drift assert
MATCH (`8e364a3a`); binary `46849f69` current — dry build 0 steps, no `src/` file newer; same
boot as sessions 21–25 (`uptime -s` 2026-09-04 18:45:08); canary `--no-scene` **exit 0** (30
members, artifacts `f56/canary/20260906-212529`); config/ssystem md5 pristine
(`03fbee59`/`545a51ef`); no `spacecrafter` in `/proc/*/comm`; RAM **52 GiB available of 59**,
`-j24`; next free §11 **219** (live ∪ archive, `max+1`); unpushed **92 code / 761 harness**
(`rev-list --count`). Live `### F` **3 → 0** by **archival pass 18** at OPEN (update-s24 +
F96/F97/F98, 733 lines = 113 + 212 + 207 + 197 + two seam tidies — the doubled `---` after the
moved note and the session-25 mint's own `---` before F96, restoring 5d94d3f's exact seam;
manifest `2026-09-06-pass18`, pre-md5 `96ea375c` reproduced in-process AND from disk, the
archive files written before the live surface — Q-56's ordering; commit `fcfdd04`; **nothing
carried**) **→ 4** by the mints below (`cb6009f`). Instrument baselines at open (run 21:25:28–21:26:27):
scan **234/289/135** · pair-check **234/209/25/116** · D 35 · D2 11 · I 89 · I2 37 · M 83 — to
the digit of the session-25 close. QUEUE CONSUMPTION (session-25 close, in order): (1) pass 18
— DONE; (2) **§5.141's fix → F99** (S — WIDENED by the read: the row's `parent <unknown>` arm
is refused by `ModularSystem::loadBody:1067-1071` before any loader runs, so the arm that
reaches the dereference is `parent none`, and on it the OLD path's own `location_orbit`
branch dereferences the same null parent FIRST, `protosystem.cpp:599`, called before the new
path — two sites, the old-path guard under the §5.50/§11.124(h) precedent, a veto point in
§3); (3) **§5.139's mechanism leg → F100** (S–M — the row's candidate, the walk's visibility
gate, does not survive the read: every body the walk reaches gets its `mat` translation
refreshed visible or not; the frozen 48 are the PARKED subtrees — `hidden = true` in the
field's `ssystem.ini` on every dwarf planet and asteroid of F96's frozen list, their moons
under them — plus the isolation residue; their eye-frame position has ONE writer, the D8
barrier `useNow()`, whose memo `evaluatedJD == currentJD` (`ModularBody.cpp:459`) keys on
the date alone while `mat` also follows the camera; F96's dumps were at `timeSpeed 0`, and
the dump (`ssystem_factory.cpp:1192`) and the SELECTION (`ModularSystem.cpp:270`) both
already call the barrier — so the operator's `get status object` on a hidden dwarf at a
pinned clock is PREDICTED stale too; leg first, fix inside the barrier's contract, STOP if
refuted); (4) **§5.143's leg → F101** (S — the candidate chain read site by site: the old half
enumerates `currentSystem` (`:1181`), `14.sts:22` leaves the system, `:25` loads `Solsys parent
none`, `:27 set home_planet Solsys` re-enters through `enterSystem → changeSystem →
createSystem` (`:764-768`, `:294-307`, `:741`) into a fresh one-star `ProtoSystem`; the assert
lands at `dumpread.load_dump` — the channel's single reader, 21 importers — raising by default,
the census/soak readers opting out by name); (5) **§5.142's reading → F102** (S — the row's
own *"`06.sts` alone for the number of bodies at which the first error appears"* is WRONG:
alone it is clean by F98's measurement, the launch is arm C; the executor's full arm-C applog
still exists under the kept `sc-f98/` tree, 10 226 lines measured at the mint, so the reading
may need no launch; the pool is ONE 1 MiB block never grown, `app.cpp:274` → `BufferMgr.cpp:8`);
(6) owner-word items, (7) the (g) tail, (8) instrument residues, (9) riders — carried, not
minted. **All four mints PASS `premise_check.py` at the mint event (F99 24/24, F100 24/24, F101
25/25, F102 22/22; 21:48:31–21:48:34) — after NINE first-run FAILs the instrument caught before
any executor could: six grep patterns carrying `->` (refused as a mutating token — an
instrument-grammar collision, rewritten with `..`), the keyed-loader count typed **7** where
`modules.cpp` registers **8** (Q-67's class — dispatcher defect 1 of this round, corrected in the
title and body), and CR line endings in the tester's `14.sts` (`tr -d '\r'`).** Picks: **F99 →
F100 → F101 → F102** (the two code tasks first — deliveries are each other's baselines — the
instrument task, then the reading; four S-class executors ≈ §0b.2's three). Deliveries: all to
the parent (§11.219+, refreshed at each dispatch). Launch classes: all FUNCTIONAL
(`--no-scene`); F102's one launch, if needed, ABORTS by design (§5.142's reproduction).
Remotes: local contains origin on both; push impossible here — the owner's push is R5,
unchanged.
**Round outcome (session 26 close, 2026-09-07 — every time in this note is pasted `date` output; the
close commit's own clock is the stamp):** F99 → **§11.219** + §5.141 FIXED at BOTH sites (the row's
`parent <unknown>` arm refuted, `parent none` measured; the OLD path's `protosystem.cpp:599` died
first and masked the loader's twin) · F100 → **§11.220** + §5.139 FIXED (the barrier's memo keyed on
(date, parent frame); 48 → 29 frozen, the 29 the never-walked `dist` 0 class; `get status object` is
OLD-first — the row's reach corrected to `flag track_object on`) + **§5.145** minted at acceptance
(the +4 re-convergence short for Eris — every "Eris exception" since F44 was the barrier; YOUR
constant) · F101 → **§11.221** + §5.143's mechanism MEASURED (the altitude, `leaveSystem()`, not the
authored system; 8 shows; `S02.sts` with zero loads empties the half) + the `dumpread` guard · F102
→ **§11.222** + §5.142 READ AND PRICED with ZERO launches (the pool runs out at `06.sts`'s 675th body;
the three prices) + **§5.146** minted at acceptance (the refused `SubBuffer` released into the free
list, Vixy's stratum) — **four for four delivered AND supervisor-verified same session**, every
delivery re-verified by my own runs (F99: arm B on both binaries + the smoke suite; F100: the leg at
both clocks + the smoke suite; F101: the self-test, its mutant, legs `p23` and `ctl_S02`; F102:
`vulkaninfo` and the applog count to the body). Code `22499f04 → 1af7fa48 → 474c595d` (two executor
commits: F99's two guard sites, F100's `ModularBody.{hpp,cpp}` — nothing else); binary `46849f69 →
8e2c6ef3 → b5f08778`; harness `d9b911e → a67e6d0 →` this close. SUPERVISOR ACTS: archival pass 18
(`fcfdd04`); four mints under the PREMISES rule (`cb6009f`, 95/95 PASS at the mint after nine
first-run FAILs the instrument caught); the open note (`50cf583`); four acceptances (`776a59c`,
`253cf1b`, `e4a3b0c`, this close) each after my own runs; **§5.145 and §5.146 minted** at
acceptances with markers at every attributing node (§11.158(f4), §11.213(g), §11.220(j1),
§11.222(g6)(i), both homes each); two instrument flags of my own mint aligned at this close (a `D8`
token at §11.213's stub, §11.220's stub citation set); the per-round premises refreshed at each
dispatch by content-located replace; in `~/shared`: **Q-70 NEW** (the owner's proxy correction, the
trace of "compact"), Q-67 ×3 (the mint-time catch; the executor's third-home miss; the sibling
class dominating). OWNER EVENTS IN-SESSION: ONE — the message on "compact" (Q-70), recorded and
applied at this close; nothing asked (Sunday). HOST: same boot throughout (`uptime -s` 2026-09-04
18:45:08); RAM 52 GiB available at open and at close; the RDP session and `:2` survived the ssh move
(HOST-EVENTS entry at this close); no lock file at any of the executors' checks. SUPERVISOR TALLY:
**ten dispatcher defects** — nine all output-side, all caught by an executor's report-not-absorb or
an instrument, none reaching a delivery, plus ONE at the close (the M-delta attribution above, typed
from expectation, corrected one commit later) — value class ×5 (the keyed-loader count typed 7 for 8, caught
at the mint; the `INTENT/11.153.md` pointer for §11.152(p)(2); a "red" control written empty where
my own row says 1/277; the row sentence "corrected" at the mint against its own record) and
structure class ×5 (the "§5.50 shape" label against its definition; `get status object` assumed
new-path at two homes; the 22nd importer credited to the wrong file; the flat census pattern that
missed `artifacts/f98/f98_repro14.py`; §5.143's headline asserting a candidate its body marked
derived). EXECUTOR REPORT DEFECTS: F99's "eleven launches" (self-caught, its third home aligned by
me); F98's §11.218(g) "zero errors after 06+06old" (found by F102 two rounds late from F98's own
log). EXECUTOR criterion-integrity instances: **≥ 20** (F99: the arm predicted before the launch
and the site attributed by frame; the pre binary reproducing the landed `c125adf0` FIRST; the sweep
red on the pre tree. F100: the partition key built from fields that do not know they are frozen;
one binary, two clocks; both mutations built, one as a binary; the cost read where fps was
saturated; three predictions kept refuted. F101: the `p23` leg the section lacked; every count
diffed with the instrument's own grammar; the red control that could not be red reported, not
forced; a flush-lag lower bound named. F102: N predicted before the log was opened, one body off;
the alignment pinned to the device the engine picks; the 7.7/body artefact explained; the
corpus parser's own defect caught by its output). BASELINES AT CLOSE: (measured 01:32:37 after this close's mint and alignments) scan **252/319/143** · pair-check **238/213/25/118** · **D 35** · D2 11 · I 89 · I2 37 · **M 88** — over F102's close (252/319/143 · 238/213/25/117 · D 36 · M 87): +1 inline stub = §5.146; D −1 = the `D8` token dropped from my §11.213 stub marker (the §5.145 mint's own flag, closed at its cause); M +1 = §11.222's pair, which the instrument lists as `stub_only 11.113, 11.218, 5.142, 5.50, 5.60` once my (i) marker cited §5.116 in the entry with no §5.116 in the stub's marker — the entry-wins shape, licensed; and §11.220's flag (`entry_only 11.158, 11.213, 5.145`) did NOT clear when its stub marker was realigned to name those nodes — the instrument's reading of that stub marker is unexplained and is left named, not forced **[this clause first attributed M +1 to a `§5.146` count difference typed from expectation, in the close commit `86ce0e5`; corrected one commit later to the instrument's own line — dispatcher defect 10 of the round, the close's own, Q-67's class]**; the scan unchanged: a MINTED marker is not in its event lexicon.
Archival pass 19 (update-s25 + F99/F100/F101/F102, live `### F` 4 → 0) DEFERRED to the next open.
NEXT-ROUND QUEUE, in order **[RE-ORDERED 2026-09-07 08:4x on the owner's replies, §11.223]**: (1) archival
pass 19 at open; (2) **`supervised-by.sh`'s silent fallback made LOUD** (S, decision-free — the tool he
operates Saturday; + the SHA-map requirement stated at §11.223(d)); (3) **§5.145's fix** (S, decision-free: ~~the
iterative orbit types resume with 1 + 8, I4~~ the Newton path of those types takes two steps per call, 2 + 8, the loop untouched — §11.223(b) as corrected); (4) **§5.146** on his word (EntityCore: the contract
line + `SharedBuffer`'s bind/release guards, §11.223(c)); (5) **the dump channel's two items** (S, decision-free: a
system-identity field in the header, §11.221(n1); the barrier for the 30 new-only records, §11.220(j3)); ~~(3) the
dump channel~~ (6) ON THE OWNER'S WORD: §5.142's
policy · ~~§5.145's constant~~ (resolved, §11.223(b)) · the `orbit_lon` ruling → §5.21's two halves + §5.140 · §5.144's 8–12 h
leg; (7) the (g) tail: §5.115's retention design, §5.66+§5.71, A15's residual; (8) instrument
residues: a PREMISES line per cited pointer (Q-67's candidate), recursive importer censuses,
`f99_sweep.py`'s old-chain blind spot, `supervised-by.sh` B1, `f85_links.py` guard, `f89_p7.py
margins`, the b4 `/proc` probe, the `dumpread` duplicates (`b24_equivalence`/`f89_p7`); (9) riders:
`observedToBodyLocalPos` with no consumer, the `[parallel-script]` question, scedit README `:43`, the
tester's `panorama5.sts:102`, `TDRS 3` and the 13 duplicate names in `06old.sts`; (10) owner items
per §3. Remotes: **94 code / 792 harness** unpushed before this close's commit (measured 01:29:13);
push from a keyed host — the supervisor never pushes.

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

1. **Warm-up**: both trees clean + note HEADs; binary exists at code HEAD; re-read this
   file's update notes AND the §13 rows of the candidate tasks (this file is a view —
   §13 + the cited entries win). After any discontinuity, re-verify state before
   relying on it (the 2026-07-30 OOM left an executor's uncommitted diff in the tree;
   "clean expected" is an expectation, not knowledge). **Definition-drift assert
   (2026-08-29, §11.161(g)):** `md5sum claude/agents/opus-xhigh.md
   .claude/agents/opus-xhigh.md` must MATCH — the tracked file is the authority, the
   `.claude` one its deployed projection; on mismatch, regenerate the projection from
   the authority before any dispatch. **Ledger instruments (2026-09-06, the miss-ledger item — two
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
   without a block is a coverage gap the rule forbids (`--list` names it).
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
for five DELIVERED and accepted; archived pass 15) · F89 §11.208 · F87 §11.209 ·
F93 §11.210 · F90 §11.211 · F92 §11.212 (session-23 round — five for five
DELIVERED and accepted; archived pass 16) · F91 §11.213 · F94 §11.214 · F95
§11.215 (session-24 round — three for three DELIVERED and accepted; archived
pass 17). Live below: the session-25 mints **F96** (§5.138 + the §5.86 pole
guard), **F97** (§5.21) and **F98** (the `fscripts/` soak). Remaining candidates
next-round: the §11.207(g) tail (§5.66+§5.71 · §5.115 · A15's residual), the
instrument residues, the riders. Still blocked: §5.100's fix (authorization
unanswered).* **Session-25 round (2026-09-06): F96 §11.216 · F97 §11.217 (in part — the
STOP endorsed) · F98 §11.218 — three for three DELIVERED and accepted; archival pass 18
(update-s24 + F96/F97/F98) DEFERRED to the next open.**
**Session-26 (2026-09-06): archival pass 18 DONE at open (`fcfdd04`); live below: the
session-26 mints **F99** (§5.141's fix, both sites), **F100** (§5.139's leg + the barrier
fix), **F101** (§5.143's leg + the old-half assert at the one reader), **F102** (§5.142's
reading, priced).** **Session-26 round (2026-09-07): F99 §11.219 · F100 §11.220 · F101 §11.221 · F102
§11.222 — four for four DELIVERED and accepted; §5.145/§5.146 minted at acceptances; archival pass
19 (update-s25 + F99–F102) DEFERRED to the next open.** **Session-27 (2026-09-07): archival pass 19
DONE at open (`06eec39`); live below: the session-27 mints **F103** (`supervised-by.sh` B1 made
loud + the SHA maps persisted — the tool the owner operates Saturday), **F104** (§5.145's fix on
the corrected ruling: two solver steps per call), **F105** (the dump channel's two items: the
header's system identity + the barrier for the 30 new-only records, with the never-published-frame
precondition).** **Session-27 round (2026-09-07): F103 §11.224 · F104 §11.225 · F105 §11.226 — three for
three DELIVERED and accepted; §5.147/§5.148 minted at acceptances; archival pass 20 (update-s26 +
F103–F105) DEFERRED to the next open.**

---

### F103 — `supervised-by.sh`'s SILENT fallback made LOUD (B1, §11.212(g)) and the rewrite's old→new SHA maps PERSISTED beside the ledger: `reachable_in_code` (`:233-240`) and `build_repair_map` (`:781`) take the branch name out of a historical `Code:` trailer and, when `refs/heads/<name>` no longer resolves, judge reachability against `HEAD` with no message — measured non-verdict-preserving (§11.212(g): `is-ancestor … master-beta` rc 0, `… 2023-master` rc 1; the same flip reproduced on today's HEAD at the mint); after the owner's rename every one of the 788 trailers naming `master-beta` takes that fallback, and he operates this tool Saturday/Sunday (§11.223(d)). The fix: an unresolvable trailer branch is a STOP at the preview (step A, before any prompt, `--dry-run` included) naming the branch, its trailer count and the fix — a `--branch-alias=<old>=<new>` channel in the `--author-fix` shape — never a substitute target; `:781` becomes an assertion. The maps `build_map` writes into the `mktemp -d` the EXIT trap deletes (`:674`, `:802`, `:844`) are copied under `claude/sha-maps/` and committed by the closing commit, so the ledger's pins and trailers resolve through them by convention (the archival rule's resolution-by-insertion, never a search-and-replace over the record) — the owner's "commit tracking" [derived reading, veto point §3]. Exercised on a THROWAWAY nested clone pair under `/home/claude/sc-f103/` whose scratch origins sit behind exactly as the live pair's do; the live pair sees `--dry-run` only [S–M, harness tooling; no engine code; no launch; veto points §3 on the two forms]

**Why now / mandate:** §11.223(d) [vixy 2026-09-07 verbatim: *"Y4 - will be operated on saturday
or sunday, along with supervised_by.sh patching of commit history with commit tracking"*] on top of
§11.212(g) B1 (F92's breakage scan: the fallback *"is not verdict-preserving"*, *"the fix is one
line: warn (or refuse) when the branch named in a trailer is gone"*) and the session-25/26 queues
(*"`supervised-by.sh` B1 (owner-authorized)"*; *"S, decision-free — the tool he operates Saturday;
+ the SHA-map requirement stated at §11.223(d)"*). Why the maps [derived at §11.223(d), the
reason restated here so it is challengeable]: a history rewrite re-hashes every commit it touches;
step E (`:848-905`) repoints TRACKED `*.md` tokens in the working tree — which reaches neither the
git history of those files nor any foreign surface (`~/shared/QUEUE.md` bodies and HOST-EVENTS
copies, the owner's notes, scratch trees, this file's archive) — the untracked spaces
`~/shared/QUEUE.md`'s header names as the reason IDs are never renumbered ("a kind of
use-after-free"). A committed map is the resolver for those spaces, the archive drawer's
convention one axis over: a cited sha that resolves to no reachable commit is looked up as a
PREFIX in the maps (the `map_lookup` rule the script already has, `:263-274`). What the owner's
"commit tracking" means is READ as this and said so in §3 — his correction may differ.

**Measured at dispatch (supervisor, 2026-09-07 09:11, code `474c595d`, harness `06eec39`; the
PREMISES block re-runs what is a command):** the script is 976 lines, md5 `4a55d463`; the two
sites: `:233-240` (`target=HEAD`, the `refs/heads/${br}` test, `merge-base --is-ancestor`) and
`:781` (the same test inside `build_repair_map`); `reachable_in_code`'s only caller is
`dangling_trailers` (`:250-256`), itself called at `:634` (the preview count, BEFORE the y/N),
`:643`, `:795` (`build_repair_map`'s input), `:835` and `:839` (the post-rewrite assertion) —
the task re-reads and confirms the caller set; `--author-fix` is the existing hand-repair channel
(5 occurrences: parsing `:401-409`, `resolve_author` `:205-217`); `--branch-alias` appears 0
times; `claude/sha-maps` does not exist; the harness history carries **788** `Code:` trailers,
every one naming `master-beta`; upstreams `origin/master-beta` (94 behind) / `origin/CC-harness`
(797 behind at the mint — refreshed at dispatch); `refs/heads/master-beta` = `474c595d` EXISTS
today, so the fallback is LATENT — and the flip is reproducible on today's tip: `rev-list --count
refs/heads/master-beta..474c595d` = **0** (reachable), `refs/heads/2023-master..474c595d` = **490**
(not — `194c6074` is the deployed line's tip); `git branch --contains 474c595d` names `master-beta`
alone; the `merge-base --is-ancestor` form the script uses is what these count (the instrument
refuses `git merge…` as a mutating token, so the premise carries the read-only twin);
`git config user.name` = `Claude` at every scope — the WILDCARD the script resolves from the
`Co-Authored-By: Claude Fable 5.1` trailer (present on the last commit of both repos); `.gitignore:85`
= `claude` is TRACKED, so a clone of the code repo ignores the nested harness clone as the live
pair does; `/usr/bin/script` exists (the pty driver — `:887 if [ -t 0 ]` makes the repoint prompt
proposal-only on a pipe); git 2.51.0, `filter-branch` present behind its deprecation banner. The
tool's homes: `README.md:129` (the repo contract), `harness/README.md:4220` (F92's note),
`DEPLOYMENT-MAP.md:269`, §11.212(g).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f103/prediction.txt`, committed before any run):
P1 PRE-fix on the clone pair with `refs/heads/master-beta` renamed to `main` and HEAD on
`2023-master`: `--dry-run` proceeds SILENTLY and reports N dangling trailers — predict N from
`git rev-list 2023-master..master-beta` intersected with the trailer set, state the number; P2
POST-fix, the same clone state: `--dry-run` STOPs at step A with one §2(f) block — what (788
trailers name `master-beta`, which no local ref resolves), consequence (their reachability cannot
be judged; the old code judged them against HEAD, which flips the verdict — §11.212(g)), fix
(`--branch-alias=master-beta=main`, or `git branch master-beta main`) — exit 1, no prompt reached;
P3 POST-fix with `--branch-alias=master-beta=main`: 0 dangling and the preview counts identical to
the un-renamed clone's, to the digit; P4 the FULL run on the un-renamed clone pair (pty-driven
`y`/`y`, `--supervisor='Test Supervisor <test@example.invalid>'`): both repos rewritten, the three
`VERIFIED` lines, `claude/sha-maps/<stamp>/code.tsv` + `harness.tsv` (+ `repair.tsv` when
non-empty) committed by the closing commit, each map's line count = that repo's rewritten-commit
count, every pair fingerprint-equal (`%T|%at|%ae|%s`) old vs new, the `.md` repoint as before;
P5 a second run on the rewritten clone: *"Nothing to do in either repo."*; P6 the LIVE pair:
`--dry-run` before and after the change prints the same preview (N code / N harness selected, 0
dangling) and both live trees stay clean with HEADs unmoved — asserted before and after each of
the two dry-runs. (2) **THE FIX** in `supervised-by.sh`: (a) `reachable_in_code` returns three
outcomes and never substitutes: reachable (0), unreachable (1), branch-unresolvable (2: `br`
non-empty, no `refs/heads/${br}`, no alias); an alias map `--branch-alias=<old>=<new>`
(repeatable; parsed beside `--author-fix` at `:394-417`; `<new>` must resolve as a local branch at
parse time, else `error:` + exit 1) consulted before the ref test; (b) `dangling_trailers` (or a
sibling `unresolvable_trailers`) separates the two failure classes so step A can STOP — printed
BEFORE the confirmation, BEFORE *"Nothing to do"*, and under `--dry-run` too (exit 1: a preview
that would lie must not exit 0); the block §2(f)-shaped, one line per unresolvable branch with its
trailer count, then the two fixes; (c) `:781` — the same alias lookup, and an unresolvable branch
there is `STOP … rollback_all; exit 1` (unreachable by construction after (b): an assertion, not a
fallback); (d) THE MAPS: after `:844`, copy `${WORK}/code.map`, `harness.map` and `repair.map`
(each when non-empty) to `${HARNESS_REPO}/sha-maps/<UTC yyyymmddThhmmssZ>-<code-tip8>-<harness-tip8>/`
as `code.tsv` / `harness.tsv` / `repair.tsv` (the TSV the script already writes — `<old-full-sha>
TAB <new-full-sha>`, one line per commit whose sha changed — nothing re-rendered, I9), plus
`sha-maps/README.md` written ONCE (the contract: a line's meaning, the files never edited, the
prefix-resolution rule, newest map first); `git add` them; the closing commit (step E) becomes
UNCONDITIONAL whenever a map was written — `EXPECTED` (`:912`) = the touched `.md` set ∪ the new
map files, and every NOT-COMMITTED branch (`:916-930`) and `--no-commit` name the map files, so
nothing is left uncommitted in silence; (e) the header comment (`:123-133`) gains the map
paragraph — the file documents its own rule (I1). NOTHING else moves: the selection rule, the
msg-filter, the identity grammar, the clean-tree check and the rollback are untouched — asserted
by the diff. (3) **THE PROOF** on the clone pair: `/home/claude/sc-f103/origin/{code,harness}.git`
as bare clones of the LIVE repos (reads only), their tips then `update-ref`'d to the live
`origin/master-beta` / `origin/CC-harness` shas so the scratch upstreams are behind by exactly the
live counts (a premise); the pair cloned NESTED (`sc-f103/pair/spacecrafter` +
`…/spacecrafter/claude`, upstreams set, `git status` clean in both); P1–P5 there; explicit
timeouts on every call (Q-68: `filter-branch` over ~900 commits is minutes). The live pair: P6
only. (4) **RECORD:** §11.⟨next⟩ FIRST + stub; §11.212(g) B1 back-marker at BOTH homes (FIXED,
the form); `harness/README.md:4220` annotated + a `supervised-by.sh` section stating the alias
channel and the map convention; `README.md:129` gains the `sha-maps/` line (the repo contract);
`DEPLOYMENT-MAP.md:269` annotated; the §3 `[Y4]` node is the supervisor's at acceptance; WIP per
§0.6; D14.

**Boundaries:** `claude/supervised-by.sh`, `claude/sha-maps/README.md` (new), the three README/
map homes, the ledger; NO engine code; NO edit of any tracked `.md` beyond the record files; the
LIVE pair is NEVER rewritten and never invoked without `--dry-run`; every rewrite under
`/home/claude/sc-f103/`; no `run_in_background`; nothing under `/tmp` carries (the script's own
`mktemp` is its own — the persisted maps are the proof they left it). No launch, no display, no
canary — a task that needs none says so.

**Discriminating checks:** (a) P1 vs P2 on the SAME clone state — silent pre-fix / STOP post-fix
(both ways); (b) P3 — the alias restores the un-renamed preview to the digit; (c) P4 — the maps
committed, fingerprint-verified pair by pair, counts equal to the rewritten sets; (d) P5
idempotence; (e) P6 — the live pair unmoved (HEADs, status) and its preview identical pre/post;
(f) the script diff confined to the four sites + the arg parser + the header; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that are not
commands: the harness HEAD as the prompt states it; the live pair clean at every check; the
executor never types `y` at a prompt of the LIVE pair (there is none to reach under `--dry-run`).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 474c595d
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => b5f08778
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 224
grep -c '^### F' claude/fable-dispatch.md => 3
git rev-list --count origin/master-beta..HEAD => 94
test $(git -C claude rev-list --count origin/CC-harness..HEAD) -ge 797 && echo ok => ok
test $(git -C claude log --format='%B' | grep -cE '^Code:[[:space:]]+[^[:space:]]+[[:space:]]+@[[:space:]]+[0-9a-f]{7,40}[[:space:]]*$') -ge 788 && echo ok => ok
# the tool, re-resolved at HEAD (content drift = abort)
md5sum claude/supervised-by.sh | cut -c1-8 => 4a55d463
wc -l < claude/supervised-by.sh => 976
sed -n '233,240p' claude/supervised-by.sh | grep -c 'target=HEAD\|refs/heads/\${br}\|merge-base --is-ancestor' => 4
sed -n '781p' claude/supervised-by.sh | grep -c 'refs/heads/\${br}' => 1
sed -n '250,256p' claude/supervised-by.sh | grep -c 'dangling_trailers()\|reachable_in_code' => 2
sed -n '634p' claude/supervised-by.sh | grep -c 'BASELINE_DANGLING=\$(dangling_trailers' => 1
sed -n '674p' claude/supervised-by.sh | grep -c 'WORK=\$(mktemp -d); trap' => 1
sed -n '802p;844p' claude/supervised-by.sh | grep -c 'build_map' => 2
sed -n '887p' claude/supervised-by.sh | grep -c 'if \[ -t 0 \]' => 1
sed -n '912,913p' claude/supervised-by.sh | grep -c 'EXPECTED=\|ACTUAL=' => 2
grep -c -- '--author-fix' claude/supervised-by.sh => 5
grep -c -- '--branch-alias' claude/supervised-by.sh => 0
test -e claude/sha-maps ; echo $? => 1
# the repo pair as the tool will see it
git -C claude log --format='%B' | grep -E '^Code:[[:space:]]+[^[:space:]]+[[:space:]]+@' | awk '{print $2}' | sort -u => master-beta
git rev-parse --abbrev-ref '@{upstream}' => origin/master-beta
git -C claude rev-parse --abbrev-ref '@{upstream}' => origin/CC-harness
git rev-parse --verify -q refs/heads/master-beta | cut -c1-8 => 474c595d
git rev-parse --verify -q refs/heads/2023-master | cut -c1-8 => 194c6074
git rev-list --count refs/heads/master-beta..474c595d => 0
git rev-list --count refs/heads/2023-master..474c595d => 490
git branch --contains 474c595d --format='%(refname:short)' | tr '\n' ' ' => master-beta
git config user.name => Claude
git log -1 --format='%B' | grep -c 'Co-Authored-By: Claude Fable 5.1' => 1
git -C claude log -1 --format='%B' | grep -c 'Co-Authored-By: Claude Fable 5.1' => 1
grep -c '^claude$' .gitignore => 1
which script | head -1 => /usr/bin/script
git --version => git version 2.51.0
# the record homes
grep -n 'supervised-by.sh:236-238' claude/harness/README.md | cut -d: -f1 => 4220
sed -n '51p' claude/INTENT/11.212.md | grep -c 'B1 .*supervised-by.sh:236-238' => 1
grep -c 'supervised-by' claude/DEPLOYMENT-MAP.md => 1
test -e /home/claude/sc-f103 ; echo $? => 1
```

**DoD:** predictions before any run; the fix at the four sites + the alias channel + the map
persistence; P1–P6 measured; §11 entry + stub; §11.212(g) marker; the README/map homes; trees
clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-07 -> **§11.224** (`INTENT/11.224.md` + stub). B1 FIXED: no fallback at all (`resolve_branch_target` as the one home, a third `unjudgeable` outcome, a §2(f) STOP at step A' that exits 1 under `--dry-run`, `--branch-alias=<old>=<new>`, `:781` an assertion) -- reproduced at RANGE scale first (138 dangling reported against a true 2, silently), discriminated both ways on one clone state, and provably inert on the live pair (pre- and post-fix scripts at the SAME instant: byte-identical, 915 lines, md5 `9852fa64`). Maps PERSISTED to `sha-maps/<stamp>/{code,harness,repair}.tsv` + a written-once contract README, closing commit unconditional when a map was written (P4 rc 0: 83/798/2 lines, 883/883 pairs fingerprint-equal, 286 paths = 282 `.md` + 4 maps; P5 "Nothing to do in either repo."; two runs byte-identical). P1-P6 all measured. **TWO PRE-EXISTING BLOCKING DEFECTS found and fixed** under the forced-expansion rule, each with a faithful mutant (§11.224(f)(g)): `build_map`'s `set -e` death after the code rewrite and before `rollback_all`, and `build_repair_map` re-deriving the dangling set after the rewrite -- together they made the tool UNRUNNABLE on this pair. **ONE ITEM IS THE OWNER'S AND IS OWED A §3 NODE + a §5 mint at acceptance:** `git commit-tree` drops `gpgsig`, so the rewrite silently removes `cebebf44`'s signature and collapses a 13-commit duplicate chain, `master-beta` 3829 -> 3816, content assertion still passing (§11.224(h), both readings). Markers at §11.212(g) BOTH homes; `harness/README.md` (F92 note + a new `supervised-by.sh` section), `README.md` (`sha-maps/` in the repo contract), `DEPLOYMENT-MAP.md` R6. Harness `fe09601` -> `9775af8` -> `06dc311` -> `f3cec53` -> `55b6fc8` -> `deef236` -> this record; code `474c595d` byte-untouched. Scratch pair `/home/claude/sc-f103/` (959 MiB) left reset, nothing in it uncommitted that is cited.
**ACCEPTED 2026-09-07 — the verifying commands' `date` read 10:10:04–10:12:16 (supervisor, session 27,
Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.224 read in full; the
`supervised-by.sh` diff (`fe09601..366a8077`, +526/−65, 976 → 1380 lines) READ end to end —
`resolve_branch_target` the one home with three outcomes, `unresolvable_trailers`, step A' placed before
every consumer of a verdict and firing under `--dry-run` at exit 1, the alias parsed beside
`--author-fix` and validated after pair discovery, `:781` an assertion with rollback, D1's `if/else` +
`return 0` + WARNING + `READ BEFORE PUBLISHING`, D2's `BASELINE_DANGLING_LIST` captured at step A, step
D' copying the three maps verbatim + the once-written README + `git add`, the closing commit
unconditional with `EXPECTED` widened and every NOT-COMMITTED branch naming the maps; code UNTOUCHED at
`474c595d`; seven harness commits `9775af8 → 366a807` (Claude Opus 5; the predictions at `9775af8`
before any run), both trees clean; §11.212(g) `[B1 FIXED … → §11.224]` at both homes; `README.md`
§sha-maps, `harness/README.md:4230` + the F92 note, `DEPLOYMENT-MAP.md` R6; instruments to the digit of
the report (scan 254/322/144 · pair-check 240/215/25/118 · D 35 · D2 12 · I 89 · I2 37 · M 89). **AND by
my own hand:** the delivered script `--dry-run` on the LIVE pair (10:10:51) — rc 0, HEADs and both
statuses unmoved before and after, 94/805 in range, 138 distinct trailers, **2** already dangling, no
STOP; D1's isolated `set -e` case rc 1; D3's twins read off the live repo (`cebebf44` gpgsig 1 /
`b8dddd6c` 0, same tree `e77a4151`, author, date, subject, parent `1ddd32f0`; `master-beta` 3829; ONE
signed commit in the range; the 13 old shas cited in `INTENT/11.203.md` alone); all **883** map pairs
re-checked fingerprint-equal in the clone's object store, 0 unresolvable; P1 → P2 → P3 REPRODUCED on
the scratch clone in state R (10:12:09–16): pre-fix rc 0 / **138** dangling / 0 warning words, post-fix
STOP rc 1 with no prompt, alias rc 0 / **2** dangling, P1-vs-P3 = 138 diff lines, the clone restored to
state U (`474c595d`, clean, upstream `origin/master-beta`), the live pair untouched. Deviations ENDORSED
with the executor's arguments: the two forced-scope expansions D1/D2 (blocking for P4 AND for the
owner's Saturday run; each discriminated by a mutant with committed logs; the untouched functions
block-md5-verified — check (f) PARTIAL by that scope, endorsed); the alias validated after pair
discovery (the branch lives in the CODE repo, discovered after the arg loop — the mandate's "at parse
time" was mine); the `--help` awk (arg parser, in scope; a magic line number truncates the help the
moment the header grows — which this change did); `code.tsv` verbatim at 83 (I9); P6 in the
two-scripts-one-instant form (stronger than asked); the fixture pinned at `fe09601`; a WARNING rather
than a STOP for unpaired commits (D3 is the owner's; the tool says so and prints Undo). ONE delivery
defect corrected here: `README.md`'s new section said `purge-path.sh` writes maps too — it does not (0
mentions; it shares the rewrite mechanism, not the map step): sentence corrected with a marker,
`purge-path.sh`'s own map owed as an instrument residue. SUSPENSIONS ENDORSED and ROUTED: **D3 → §5.147
MINTED at this acceptance** (record-only, the §5.131 precedent — a repository-history property that
makes the owner's planned operation lossy; markers at §11.224(h) both homes; the decision his, §3); the
scan's half-1 EVENT lexicon lacking `FIXED` (instrument residue, queue item 8). DISPATCHER-SIDE
FINDINGS, all ACCEPTED as mine, all output-side (the gate passed 39/39; none was an input the task
stood on): (1) "788 trailers take the fallback" — a whole-history LINE count where the tool consults
138 distinct in-range pairs (a measured number modelling the wrong set — Q-67's sibling); (2) "0
dangling" at P3/P6 — written from expectation where the live pair has carried 2 since 2026-07-22, and
the tool's own `--dry-run` (read-only, seconds) would have shown it at the mint — Q-67's class: the
measuring command existed and was not run; (3) "preview identical to the digit" for the CODE preview
in state R — structure asserted without the read (HEAD on `2023-master` makes the code range empty by
the fixture); (4) "line count = rewritten-commit count" — the `:815` repair append was in my read and
not in the relation; (5) "`README.md:129` is the repo contract" — a pointer inside "History before
this repo"; (6) "validated at parse time" — the pair is discovered after the loop. Round tally so far:
**six dispatcher defects** (2 value-class, 4 structure-class). STANDING CONSEQUENCES: **the tool is
runnable on this pair and was not before this round** (D1 killed every run after the code rewrite, D2
rolled back every run on a pair with a pre-existing dangling trailer — this pair has two); before
Saturday's rewrite the owner decides D3 (§5.147); the rewrite is deterministic (two runs,
byte-identical maps, tip `d40f4eb1`); `claude/sha-maps/` is created by the first real run; a renamed
branch now STOPs the tool until `--branch-alias` names the new one; the census the tool consults is the
RANGE's distinct `<branch> <sha>` pairs (138 today), never the history's trailer lines;
`/home/claude/sc-f103/` (959 MiB) holds the pair and my runs under `logs/supervisor/`.

### F104 — §5.145's fix on the owner's CORRECTED ruling (§11.223(b), [vixy verbatim]: *"keeping the same iteration count but doubling each iteraton per cycle on the newton path (or the one Eris involves)"*): the ITERATIVE solvers take TWO steps per call where they took one — `EllipticalOrbit::eccentricAnomaly`'s four advancing statements (`orbit.cpp:524`, `:531`, `:543`, `:559`; the `e == 0` and `e == 1` returns untouched) and `IterativeEll::operator()` / `IterativeHyp::operator()` (`iterative_orbits.hpp:97`, `:41`) — ONE constant in ONE home; the barrier's `1 + RESUME_EXTRA_ITERATIONS` loop untouched (`ModularBody.cpp:512-514`; `ModularBody.hpp:338` stays 4), so a parked body's use gives 2 + 8 = 10 steps where F100 measured 9 → 1.1e-05° (§11.220(j1)); `orbit.cpp` is SHARED, so the OLD path converges twice as fast per frame after a date jump — an as-if change on the comparison baseline, strictly more exact (veto point §3) — and §5.84's residual changes shape (annotated at the fix); the three-class partition PREDICTED from the field census before any build: 60 iterative bodies (54 `ell_orbit` + 6 `comet_orbit`, the six comets ALL parked), of which the PARKED ones move toward old by their own 5-step residual (Eris by 1.1987°) and the WALKED ones by ulps, while the 60 non-iterative records (`*_special`, `*_custom`, `e == 0`) stay byte-identical; the MUTATION (Eris's branch alone) discriminates the class from the instance; the cost under D11 measured on the solver itself [S–M, engine, BOTH paths — the veto point above]

**Why now / mandate:** §5.145 [measured 2026-09-06, §11.220(j1) (F100); minted at F100's
acceptance; **RESOLVED 2026-09-07 → §11.223(b)** on two owner messages, the second correcting
the first reading: *"Y2 - double the iterations per cycle for those types of orbits only,
otherwise it might became slightly noticeable at high simulation speed"* then *"Wait, why 1+8 ?
shouldn't it be 2+8 then, by keeping the same iteration count but doubling each iteraton per cycle
on the newton path (or the one Eris involves) ?"*]; session-26 close queue position 3 (*"S,
decision-free: the Newton path of those types takes two steps per call, 2 + 8, the loop
untouched"*). **The scope of "those types" is READ here and said so, because the two forms
differ in what they fix [derived]:** the mechanism is *one step per call from a stale seed*
(§5.145's own row, `ModularBody.hpp:1033`), which every ITERATING branch of `eccentricAnomaly`
shares — the `e < 0.2` fixed-point iteration (`:524`, LINEAR convergence, factor e), the
`0.2 ≤ e < 0.9` Newton step (`:531`, Eris's), the two Laguerre-Conway steps (`:543` elliptic,
`:559` hyperbolic) — and both comet solvers' Newton step; so the CLASS form doubles all six, and
the INSTANCE form (Eris's branch alone) is the row's symptom. The owner's parenthesis *"(or the one
Eris involves)"* locates the change, it does not restrict it to one band [derived reading; the
narrower form is a veto point in §3 with this argument, and the task builds it as the MUTATION so
the two are measured side by side]. Cost bound [derived, the task measures]: one extra solver step
per iterative body per evaluation — 60 bodies × two paths per frame ≈ 120 steps of one
`sin`/`cos` pair and a division — the order of microseconds against D11's 1 ms; the barrier's
loop, which the first reading would have doubled at 5 full subtree refreshes per parked body per
frame at a running clock, is what his first sentence priced.

**Measured at dispatch (supervisor, 2026-09-07 09:11, code `474c595d`):** the sites above,
each a premise line; the field file: **54 `ell_orbit`** (12 with e ≥ 0.2 — neried 0.747, neso
0.630, setebos 0.561, sycorax 0.510, **Eris 0.437 (hidden)**, laomedeia 0.385, prospero 0.331,
pasiphae 0.293, halimede 0.262, carme 0.260, psamathe 0.223, ananke 0.217 — the other 42 below
0.2, 8 of them e = 0) and **6 `comet_orbit`**, all `EllCometOrbit` (e < 1) and ALL hidden:
Pallas 0.231, Juno 0.257, Vesta 0.089, Haumea 0.191, Makemake 0.15, Sedna 0.859; F100's parked
set `P \ I` (19, `artifacts/f100/partition_f96_leg_pre.json`) by solver: **11 iterative**
(Arrokoth 0.037, Ceres 0.080, Charon 0.013, Eris 0.437, Nix 0.016 — `ell_orbit`; Haumea, Juno,
Makemake, Pallas, Sedna, Vesta — `comet_orbit`), **5 with e = 0** (`return M`, no iteration:
Goldilocks_Zone, Kerberos, Namaka, Solar_System, Styx), Pluto `pluto_special`, and Hiiaka /
Hydra_ — both-tree bodies whose `ssystem.ini` sections the exact-key census did not match (spelled
otherwise in the file): the task resolves them from the dump's names against the file's sections.
The batch path `EllipticalOrbit::fastPositionAtTimevInVSOP87Coordinates` (`orbit.cpp:447-458`:
10 pre-iterations from a zero `batchLastE`, then one step per sample) goes through the same
function and doubles with it — its consumers (trails, orbit lines) and cost are the task's to
name. Comments that become false: `ModularBody.hpp:1033-1034` (*"exactly ONE Newton step"*),
`ModularBody.cpp:505-507`, `CameraAnchors.cpp:484`, `TrailModule.cpp:219` — `grep -rn 'ONE
Newton\|one Newton' src` = **5** lines, each updated. F100's record for Eris
(`parked_table_prepost.txt`): `Eris | 1.19872 / 170.716 / 5,5 | 1.19872 / 1.09669e-05 / 5,9`;
the F91 table at `c125adf0` (fr) with Q2's two exceptions (§11.220(i2): Eris 1.199089259, Puck
0.000277777); instruments `f100_freeze.py` (`--stages freeze --clock pinned`, the parked table),
`f100_partition.py`, `f100_identity.py`, `f91_run.sh --expect post` + `f91_parity.py`,
`f90_rehearsal_run.sh`, `dumpread.py`; the supervisor's F100 acceptance runs at
`/home/claude/sc-f100/supervisor/post` (Eris 1.1987249° reproduced there); `artifacts/f102/
f102_sizes.cpp` as the shape of a standalone harness compiled against the tree.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f104/prediction.txt`, before any build): P1 the
THREE-CLASS partition of the 120 records from the field census + F100's partition JSON,
independent of any dump: NON-ITERATIVE (byte-identical `mat` translation and altaz at the
un-moved launch state on both clocks), WALKED-ITERATIVE (differences of ulps — state the bound and
its reason: one extra step on a converged seed), PARKED-ITERATIVE (each named body moves toward
old by its own residual — Eris from 1.1987° to ≤ 3e-05° (the band of F100's 9- and 15-step
values), the others from their F100 launch-state values, read per body from
`parked_table_prepost.txt`'s pre column); P2 the F91 table: exactly ONE row changes — Eris's
RA/DE string becomes old's and the Q2 exception count goes 2 → 1; Puck's stays (say from the
record what Puck's cause is, or say unknown); P3 the smoke suite rc 0 with §11.211's step states;
P4 D11: ns per solver step from a standalone timing harness against the tree's `orbit.cpp` ⇒
the per-frame bound for 60 bodies × 2 paths against 1 ms; P5 the MUTATION (Eris's `0.2 ≤ e < 0.9`
branch doubled ALONE) also fixes Eris and leaves every OTHER parked iterative body at its 5-step
residual — name which bodies the class form moves that the branch form does not (Ceres, Vesta,
Arrokoth, Charon, Nix in the `e < 0.2` branch; Sedna beside Eris; Haumea, Juno, Makemake, Pallas
in `IterativeEll`); P6 the pre binary reproduces F100's Eris 1.1987249° FIRST (the instrument
sees the defect before it is shown fixed). (2) **THE FIX:** `constexpr int
ITERATIVE_STEPS_PER_CALL = 2` in `iterative_orbits.hpp` beside `WARP_PRECISION` (the header both
solvers include — one home, I2), applied as a loop of that count around each of the four
advancing statements of `eccentricAnomaly` (the `if (lastE == 0)` seeding stays OUTSIDE the loop)
and around the step of each `operator()`; the constant's comment cites §11.223(b) and §5.145 and
states the reason (a stale seed converges in steps, and a use buys 1 + 4 calls); the five
comments updated; NOTHING else — `RESUME_EXTRA_ITERATIONS`, `useNow`, data untouched. (3)
**PROVE:** build (memory-bounded, `-j` per the hook's number), binary md5 recorded, the pre
binary preserved as `bin/spacecrafter-pre`; P6 then P1 through F100's freeze stage at the PINNED
clock (the parked table, both binaries) and through `f100_identity.py` at the un-moved launch
state, both clocks — scored BODY BY BODY against the pre-registered class of each; any body
outside its class ⇒ STOP and report; P2 `f91_run.sh --expect post` on the post binary, the table
diffed line by line against the `c125adf0` record; P3; P4; P5 the mutation binary built, run
through the same parked table, reverted, the post binary bit-reproduced after the revert (F100's
shape); canary `--no-scene`; D14; the frozen pair in==out. (4) **RECORD:** §11.⟨next⟩ FIRST +
stub; §5.145 FIXED (the code sha, the post value per parked body); §5.84 annotated (the shared
solver now takes two steps per call — the old path's post-jump residual halves in step count;
its own trigger's magnitude still unmeasured, say so); §11.220(j1)+(i2) and §11.223(b)
back-markers both homes; §11.76(b) annotated (the constant unchanged, the solver doubled: *"4
extra iterations"* now buy 8 steps); §11.215(g) annotated (the four bodies' run-to-run ulps are
PREDICTED to change and NOT re-measured — a 3 h soak); the map (`grep -c '5\.145'`); README; WIP
per §0.6; D14.

**Boundaries:** `src/bodyModule/iterative_orbits.hpp` (the constant + two loops) +
`src/bodyModule/orbit.cpp` (four loops) + the four comment sites in `experimentalModule/` —
comments only there; NO change to `ModularBody.{hpp,cpp}` logic; no data; no msgid; the farm only
(real HOME md5 in==out); FUNCTIONAL (`--no-scene`); no `run_in_background`; runs under
`/home/claude/sc-f104/`. **STOP conditions:** Eris after one use > 3e-05° post-fix (the doubling
insufficient — report, do not raise further: the count is the owner's); any NON-ITERATIVE body
moving (the change reached beyond its class); the F91 table moving on any row but Eris's.

**Discriminating checks:** (a) P1 body by body, the partition key from the field file, not from
the measurement; (b) P2 exactly one row; (c) P5 the mutation separates class from instance on the
same table; (d) P4 the number against 1 ms; (e) the smoke suite + canary; (f) P6 the defect seen
first on the pre binary; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that are not
commands: display per HOST-EVENTS (`:2`, the RDP-created real session — `xdpyinfo` 2448x1332 at
08:57); canary `--no-scene` exit 0 before the first launch (08:58 at the open); `free -g` ≥ 16 GiB
before the build (52 GiB at open ⇒ `-j24`); the harness HEAD as the prompt states it; §5.145's row
reads RESOLVED and not FIXED.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 474c595d
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => b5f08778
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 225
grep -c '^### F' claude/fable-dispatch.md => 3
# ledger states the work stands on
grep -m1 '^145\. ' claude/INTENT.md | grep -c 'RESOLVED 2026-09-07' => 1
grep -m1 '^145\. ' claude/INTENT.md | grep -c 'FIXED' => 0
grep -m1 '^84\. ' claude/INTENT.md | grep -c 'OPEN (old-path only' => 1
# sites, re-resolved at HEAD (content drift = abort)
sed -n '515p' src/bodyModule/orbit.cpp | grep -c 'double EllipticalOrbit::eccentricAnomaly(double M, double &lastE) const' => 1
sed -n '517,562p' src/bodyModule/orbit.cpp | grep -c 'lastE = M + eccentricity \* sin(lastE);\|lastE += ' => 4
sed -n '519p;525p;532p;544p;548p' src/bodyModule/orbit.cpp | tr -d '\t' | tr '\n' ' ' => if (eccentricity < 0.2) { } else if (eccentricity < 0.9) { } else if (eccentricity < 1.0) { } else if (eccentricity == 1.0) { } else {
sed -n '566,573p' src/bodyModule/orbit.cpp | grep -c 'eccentricAnomaly(meanAnomaly, iterativeLastE)' => 1
sed -n '448,458p' src/bodyModule/orbit.cpp | grep -c 'batchLastE' => 3
sed -n '39,45p;93,101p' src/bodyModule/iterative_orbits.hpp | grep -c 'H -= ' => 2
sed -n '4p' src/bodyModule/iterative_orbits.hpp | grep -c 'WARP_PRECISION = 1e-8' => 1
sed -n '316p;346p' src/bodyModule/orbit.cpp | grep -c 'return orbit(JD - t0, d1, d2);' => 2
sed -n '338p' src/experimentalModule/ModularBody.hpp | grep -c 'RESUME_EXTRA_ITERATIONS = 4' => 1
sed -n '512,514p' src/experimentalModule/ModularBody.cpp | grep -c 'RESUME_EXTRA_ITERATIONS\|recursiveTranslationUpdate(currentJD, frame)' => 2
sed -n '1033,1034p' src/experimentalModule/ModularBody.hpp | grep -c 'exactly ONE Newton' => 1
sed -n '484p' src/experimentalModule/CameraAnchors.cpp | grep -c 'ONE Newton step' => 1
sed -n '219p' src/experimentalModule/bodyModules/TrailModule.cpp | grep -c 'ONE Newton step' => 1
grep -rn 'ONE Newton\|one Newton' src --include=*.cpp --include=*.hpp | wc -l => 5
# the field census (LC_ALL=C /usr/bin/grep: the wrapper skips the Latin-1 file silently)
LC_ALL=C /usr/bin/grep -ic '^[[:space:]]*coord_func[[:space:]]*=[[:space:]]*ell_orbit' ~/.spacecrafter/ssystem.ini => 54
LC_ALL=C /usr/bin/grep -ic '^[[:space:]]*coord_func[[:space:]]*=[[:space:]]*comet_orbit' ~/.spacecrafter/ssystem.ini => 6
# F100's record and instruments
grep -m1 'Eris' claude/harness/artifacts/f100/parked_table_prepost.txt | tr -s ' ' | cut -c1-100 => Eris | 1.19872 / 170.716 / 5,5 | 1.19872 / 1.09669e-05 / 5,9
test -f claude/harness/artifacts/f100/partition_f96_leg_pre.json && python3 -c "import json;j=json.load(open('claude/harness/artifacts/f100/partition_f96_leg_pre.json'));print(len(j['P']),len(j['I']),len(j['both']),j['n'])" => 29 29 10 120
grep -rl c125adf0 claude/harness/artifacts/f100/ | wc -l => 1
test -d /home/claude/sc-f100/supervisor/post ; echo $? => 0
test -f claude/harness/f100_freeze.py && test -f claude/harness/f100_partition.py && test -f claude/harness/f100_identity.py && test -f claude/harness/f91_run.sh && test -f claude/harness/f91_parity.py && test -f claude/harness/f90_rehearsal_run.sh && test -f claude/harness/dumpread.py && echo ok => ok
test -f claude/harness/artifacts/f102/f102_sizes.cpp && echo ok => ok
test -e /home/claude/sc-f104 ; echo $? => 1
```

**DoD:** predictions before the build; the fix (code first); P1–P6 measured with the mutation;
§11 entry + stub; §5.145 flipped; §5.84, §11.76(b), §11.215(g) annotated; back-markers; map;
README; trees clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-07 -> **§11.225** (+ stub); §5.145 **FIXED** (code `ead2d478`, binary `b5f08778` -> **`0c61f1b5`**, the post value listed per parked body). `ITERATIVE_STEPS_PER_CALL = 2` in `iterative_orbits.hpp`, looped around the four advancing branches of `eccentricAnomaly` and each `operator()` step (seeding outside, Laguerre-Conway temporaries inside); `useNow()`'s 1 + 4 and `RESUME_EXTRA_ITERATIONS` untouched; `src/experimentalModule/` COMMENT-ONLY (0 non-comment lines); `ONE Newton` 5 -> 0. Predictions committed one commit BEFORE the code (`5bb9b03`). **P0** post call k == pre call 2k, 120/120 bit-identical, red controls 35/120 + 73/120. **P1** partition 66/40/14 predicted from the field file, held body for body; Eris **1.1987248926268401 -> 1.05951e-05** and the ONLY record of 120 whose `mat` moved (2.029 AU); 120/120 at the running clock. **P2** one diff line vs `c125adf0`, Eris's, Q2 88 -> 89 of 90, new md5 `1fe630a4`. **P3** rc 0 (S5's distance refuted and controlled to the wall clock). **P4** 2.358 us/frame = **0.236 % of D11**. **P5** mutation `6b8085cb` fixes Eris too and is 120/120 indistinguishable on the engine -- the pre-registered null; discrimination on the sliced solver; reverted with `0c61f1b5` bit-reproduced. **P6** the pre binary showed the defect first. No STOP fired. Eight launches, canary green, `/proc` 0, md5 in==out on all eight; D14 PASS. Annotated: §11.220(j1)(i2), §11.223(b), §5.84, §11.76(b), §11.215(g), both homes each; `DEPLOYMENT-MAP.md` names §5.145 0 times. Three dispatcher-side findings, all output-side, all reported before the measurement (§11.225(k)). New instruments: `f104_census.py`, `f104_solver.{py,cpp}`, `f104_seqcheck.py`, `f104_aa.py`; README section added.
**ACCEPTED 2026-09-07 — the verifying commands' `date` read 11:10:38 and 11:11:55–11:15:07 (supervisor,
session 27, Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.225 read in
full; ONE code commit `ead2d478` (Claude Opus 5; `iterative_orbits.hpp` + `orbit.cpp` + four comment
sites, 91+/37−, the diff READ: `ITERATIVE_STEPS_PER_CALL = 2` beside `WARP_PRECISION` with its reason,
the loop around each of the four advancing branches with the seeding outside and the Laguerre-Conway
temporaries inside, the loop around each `operator()` step, and in `experimentalModule/` comment lines
only) and six harness commits `5bb9b03 → 43e9b37` (Claude Opus 5; the predictions at `5bb9b03` before
the code), both trees clean; binary `0c61f1b5`, dry build 0 steps; §5.145 reads *FIXED 2026-09-07,
§11.225 (task F104), code `ead2d478`, binary `0c61f1b5`* with the post value per parked body; §5.84
annotated; §11.220(j1)(i2), §11.223(b), §11.76(b), §11.215(g) name §11.225 in the entry AND the stub;
`DEPLOYMENT-MAP` 0 hits, said not assumed; README §F104; instruments as the entry's (m) states them
(257/327/142 · 241/216/25/119 · D 35 · D2 12 · I 89 · I2 37 · M 89). **AND by my own hand on
`0c61f1b5`:** canary `--no-scene` exit 0; `f104_seqcheck.py solver-pre solver-post` — **120 of 120
bit-identical across 12 arms** (post[k] == pre[2k]); F100's freeze leg at the pinned clock
(`sc-f104/supervisor/post`, 11:11:59) — frozen **29 = I** exactly, 0 FAIL, and from its launch dump
Eris after ONE use (`evalCount` 5) **1.06636e-05°** old-vs-new alt/az (was 1.1987249° on `b5f08778`
at my F100 acceptance), Ceres 1.13e-05, Haumea 9.6e-06, Sedna 1.09e-05, Mars walked; `f91_run.sh
--expect post --locale fr` (11:12:48) — rc 0, 0 FAIL / 0 NOTE, table md5 **`1fe630a4`**, Q2 **89 of 90**
with Puck the only exception; the smoke suite (11:13:28–11:15:06) rc 0, driver exit 0, four frozen
files MATCH; `/proc` clear, no lock, the field pair `03fbee59`/`545a51ef` in==out. Deviations ENDORSED
with the executor's arguments: the `altaz_old` instrument corrected when the mutation leg beat its
one-pair floor (`--pairwise` over 29 unreachable records: NOT separable — the conclusion "not
evidence either way" is the honest one and matches §11.215(g)'s reason F100 skipped the channel); the
scope as the CLASS with the narrow form as the mutation — indistinguishable on the engine, discriminated
on the sliced solver, so the veto point is a COST question (2.36 µs worst case vs the branch form's
smaller share), not a correctness one; the P3 S5-distance refutation controlled by a same-hour pre
run rather than explained; (j1) the dead `warp()` methods and (j2) the orbit-line sampler moving the
position seed recorded and NOT minted (bounded by the barrier, unmeasured — a leg for the next round,
queued at the close with (j3)'s unattributed +32 rad staleness as its question). DISPATCHER-SIDE
FINDINGS, all ACCEPTED as mine, all output-side (mandate (1) had the executor rebuild the partition
from the field file, which is what re-adjudicated them; the gate passed 31/31): (1) the headline's
"60 iterative / 60 non-iterative" double-counts the 8 `e = 0` `ell_orbit` bodies my own census line
named — a value computed from a listing without its own subtraction (Q-67's class); (2) the P5 list
puts Sedna in Eris's branch where my dispatch paragraph says `comet_orbit` 0.859 — a wrong-set
placement against my own read (the SIBLING class); (3) the F100 result-file name implied by the
section (`<tag>_<clock>_result.json`) is `f100_result.json` — a pointer typed from expectation.
Round tally so far: **nine dispatcher defects** (3 value-class, 6 structure-class). STANDING
CONSEQUENCES: **every iterative solver takes two steps per call on BOTH paths** (`ead2d478`); a parked
body's one use now buys ten steps and Eris reads 1.06e-05° where it read 1.1987°; the F91 table's
current md5 is **`1fe630a4`** (`c125adf0` retired to the pre-F104 record, one row apart); the old
path's post-jump convergence is twice as fast per frame (the as-if veto point, §3); `altaz_old` is not
an inertness channel — 90 of 120 records move between two launches of one binary at one pinned
instant (§11.225(e)); the solver can be measured without a launch (`f104_solver.py`, `f104_seqcheck.py`);
the count is still the owner's; `/home/claude/sc-f104/` holds the farms, the pre/post/mutant binaries
and my runs under `supervisor/`.

### F105 — the dump channel's two owed items, both at their anchor `SSystemFactory::dumpTracePaths`: (i) the header names the system the OLD column was taken in (§11.221(n1): `"oldSystem"` = `SolarSystem` / `galactic` / the `systems` key, plus `"inSystem"`; `dumpread`'s `EmptyOldHalf` message names it; every parity artifact becomes self-describing) and (ii) the new-only loop calls the barrier (§11.220(j3): `nb.useNow()` before `nb.dumpTrace(out)` at `:1227-1233`, so a dump is a use for 120 of 120 records, not 90) — with the consequence PREDICTED before the launch, record by record over the 30 new-only names: **20 unchanged** (the 18 `dist 0` systems and `Universe` return at `useNow`'s first line — not `renderHidden`; `SolarSystem` is walked), **2 refreshed into a REAL eye-frame position** (`baryEarthMoon` under the walked Earth, `orbit_autour_lune` under the walked Moon — parents that publish `parkedChildFrame`), and **8 that a use refreshes into the IDENTITY frame** — the seven hidden children of the never-walked `Universe` and `orbit_autour_point` under its hidden centre — because `parkedChildFrame` defaults to identity (`ModularBody.hpp:2127` — `:2121` at the mint, +6 by F104's comment edit above it, refreshed at dispatch) and `publishParkedFrame` runs only for walked nodes (`:1003-1007`): §11.220(j4)'s hazard, which the SELECTION channel already exercises for those 8 today (`ModularSystem.cpp:269-270`; new-only names answer the new path); so the barrier gains its missing PRECONDITION — a use in a frame that was NEVER published is not served (a `parkedFramePublished` flag set at `:1006`, `useNow` returning `false` without refreshing and propagating through hidden parents; ONE D12 line per body naming what / consequence / fix) — and the 8 keep their honest `dist 0` in the dump while the F100 partition instrument's class I keeps its key; the mutation (no guard) measured once for the 8 identity-frame values, (j4)'s magnitude on record; STOP before any fix if the leg's partition is not (20, 2, 8) [S–M, engine + instrument, new path only — veto point §3 on the guard's form]

**Why now / mandate:** the session-26 close queue position 5 (*"the dump channel's two items
(S, decision-free: a system-identity field in the header, §11.221(n1); the barrier for the 30
new-only records, §11.220(j3))"*), §3 `[H1]`/`[H2]` of the session-26 block. §11.221(n1)
[executor, F101]: the dual dump *"presents the two paths side by side without ever saying which
system the old column was taken in … no system identity at all. A one-field header addition
would make every parity artifact self-describing; it is a dump-channel change"* (the reason the
field is owed: F101 read the system off the process's stdout transitions — a witness outside the
artifact). §11.220(j3) [executor, F100]: *"the second loop, which emits the 30 new-only records
`[:1223-1245]`, does not call it … Not fixed here: adding the call would change what the
instrument reports for 30 records and belongs to whoever holds the dump channel."* §11.220(j4)
[executor, F100, the hazard the call meets]: *"`parkedChildFrame` can be permanently unpublished
… a hidden body whose parent is outside the current system's walk (`big_dipper` under `Universe`)
would therefore be refreshed into the IDENTITY frame if anything ever used it … Recorded because
a future task that makes the dump use them, or that selects one, meets it."* **The mint READ
(j4) against the 30 names and it is not a residue, it is 8 of the 30 [derived, the leg
confirms]:** the guard is therefore part of (ii), not a separate design — a barrier that computes
in a frame nobody published is not the D8 contract (*"as soon as the position is used … it
should be computed"* [vixy, §11.76(b)] presupposes a frame to compute in), and the alternative —
the dump printing identity-frame numbers as if they were eye-frame positions for 8 records — is an
instrument that lies (the F100 class-I key `dist == 0` would then misclassify them). What the
guard changes on the OPERATOR channel [derived, measured by the leg]: `select` + `flag
track_object on` on one of the 8 aims today at an identity-frame point; with the guard it aims at
the zero vector — the class the 18 `dist 0` systems are already in (§11.216(i)'s zero-vector guard
question, session-25 §3, still the owner's) — one wrong replaced by a known one, not a new class.
The real fix of (j4) — publishing frames for parked children of unwalked ancestors — is a design
the owner paces (nested modular bodies, §11.223(e)); said in §3.

**Measured at dispatch (supervisor, 2026-09-07 09:11, code `474c595d`):** the sites, each a
premise line; the 30 new-only records of F96's launch dump (`artifacts/f96/leg_pre/
cmd_p0_launch.json.gz`) by (`relation`, `dist == 0`): **(1, true) × 10 · (4, true) × 1 · (5,
true) × 18 · (5, false) × 1** — the ten `relation 1` (HIDDEN) records: `baryEarthMoon` (parent
Earth), `orbit_autour_lune` (Moon), `big_dipper`, `center_sun`, `galaxy_center`, `pleiades`,
`sun_in_gal`, `un_point`, `orbit_autour_point (orbit centre)` (Universe), `orbit_autour_point`
(parent = that centre) — owned anchor bodies, `anchor.ini`'s (§11.200; the dump's own comment at
`:1136-1139`); `Universe` relation 4; the 19 systems relation 5, `SolarSystem` the only one with
`dist ≠ 0`. F100's partition JSON: |P| 29 · |I| 29 · |P ∩ I| 10 — the ten above are exactly
`P ∩ I`. `useNow()` as landed by F100 (`ModularBody.cpp:455-515`): `:457` the first-line return
(`!renderHidden || !parent`), `:473` `parent->useNow()` unconditional, `:474-475` the parent
frame (`matLocalToBodyPos` for a hidden parent, `parkedChildFrame` otherwise), `:501` the memo
key; `publishParkedFrame` (`ModularBody.hpp:1003-1007`) writes `parkedChildFrame` only when
`hiddenBodies` is non-empty, from the walk's two call sites (`:944`, `:971`); the member defaults
to `Mat4f::identity()` (`:2127` at dispatch; `:2121` at the mint). The header writer `:1108-1172` (`"type":"header"` at `:1112`,
the `extraHeader` lambda at `:1170` for Core-owned keys — the executor MODE is Core's, not the
factory's: the field lands where its owner is, I2/I9, and the mode is NOT added here unless Core
already holds an accessor, say which); the factory's identity: `currentSystem == ssystem.get()`
(`SolarSystem`), `== galacticSystem.get()` (`galactic`, `leaveSystem` `:772-778`), else the
`systems` map key (`:1160`), `inSystem` (`:1179`). The reader: `dumpread.py` returns the header
dict unchanged (additive key invisible to every `header[...]` consumer — 28 key reads over 12
readers, none enumerating keys); `EmptyOldHalf` at `:84`; `f100_partition.py:78-80` keys class I
on `dist == 0`. Readers: 24 importers flat / 27 recursive (F101's census). Instruments: F101's
`f101_bisect.py` + the prefix shows in `artifacts/f101/shows/` (`p23` = the galactic state,
`ctlmars` = the solar one), F100's `f100_freeze.py` (`--stages operator` for the track arm),
`dumpread.py`, `f90_rehearsal_run.sh`, `b3_farm.sh`.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f105/prediction.txt`, before any build): P1 the
(20, 2, 8) partition of the 30 by NAME from the tree (parent chain + `relation` + which parents
the walk visits), the 8 with their predicted identity-frame values (each body's declared local
position in `anchor.ini` / its orbit — the number the mutation must print); P2 the 2 with bands
(`baryEarthMoon`'s eye distance within Earth's own record ± the Moon's distance × the mass ratio;
`orbit_autour_lune` within the Moon's ± its declared offset); P3 the header field on three
states: the launch scene (`SolarSystem`, `inSystem` true), after F101's `p23` prefix (`galactic`,
false — and `EmptyOldHalf`'s message carrying the name), after `p25desc` (`SolarSystem` again);
P4 the operator arm on ONE of the 8 (`select planet pleiades` + `flag track_object on`, pinned
clock): pre-guard the camera aims at the identity-frame point (predict the alt/az from P1's value
and the launch camera), post-guard at the zero-vector class — the SAME aim as tracking a `dist 0`
system (`51PegSystem`) measured in the same launch as the control; P5 the applog carries exactly
8 D12 lines at the first dump (one per body, none repeated across a second dump in the same
launch); P6 the 90 both-tree records and the F91 table byte-identical pre/post (the change touches
no walked body). (2) **THE LEG, pre-change binary** (the round's current one — the per-round
md5): the launch-scene dump: the 30 records' `dist` and `mat` as the baseline (30 × dist 0 but
`SolarSystem`; the header without the field). (3) **THE FIX, in this order and each built:** (a)
the header field + the `EmptyOldHalf` message (`ssystem_factory.cpp` + `dumpread.py`), P3
measured; (b) the MUTATION first — `nb.useNow()` in the new-only loop with NO guard: P1's 8
identity-frame values measured (the (j4) magnitude on record), then the binary reverted; (c) the
guard — `bool parkedFramePublished` set in `publishParkedFrame`, `useNow()` returning `bool`
(true = served; a walked body is served by construction; a hidden body whose non-hidden parent
never published, or whose hidden parent was not served, returns false BEFORE any refresh and
logs ONCE per body: what (a use of `<name>` in a frame `<parent>` never published), consequence
(the readout stays at its unevaluated value), fix (declare the body under a body the current
system walks) — the once-per-body flag beside it), the dump's call kept; P1 (20, 2, 8 with the 8
at `dist 0`), P2, P4, P5, P6 measured post. **If the leg's partition is not (20, 2, 8) as
predicted** → STOP after (a): record the measured partition and what it implicates, deliver (a)
only, report — the guard's precondition is then not what the mint read. (4) **RECORD:**
§11.⟨next⟩ FIRST + stub; §11.221(n1) and §11.220(j3) back-markers both homes (DISCHARGED);
§11.220(j4) back-marker (its 8 members named, the magnitude measured, the guard — and the
selection-channel reach measured: a §5 candidate for the supervisor at acceptance, not minted by
the task); §5.139/§11.220 annotated at the barrier's contract (the third half of the key: a frame
must exist); `f100_partition.py`'s docstring (class I's key unchanged, and why); `harness/README.md`
(the header field in the dump contract line beside F101's); the map (`grep -c '11\.221\|11\.220'`
— say what it names); WIP per §0.6; D14.

**Boundaries:** `src/bodyModule/ssystem_factory.cpp` (the header + the one call),
`src/experimentalModule/ModularBody.{hpp,cpp}` (the flag, the `bool` return, the once-log),
`claude/harness/dumpread.py` (the message), `f100_partition.py` (docstring only); NO old-path
change; no data; no msgid; the farm only (real HOME md5 in==out; F101's prefix shows are
COPIES); FUNCTIONAL (`--no-scene`); no `run_in_background`; runs under `/home/claude/sc-f105/`.

**Discriminating checks:** (a) P1 by name, the partition key from the tree, not the dump; (b)
the mutation's 8 values equal to the pre-registered identity-frame numbers (the (j4) mechanism
measured, not inferred); (c) P4 the operator arm both ways, with the `dist 0` control in the same
launch; (d) P3 on three system states, the message naming the name; (e) P5 exactly 8 lines,
once; (f) P6 byte-identity of the 90 + the F91 table; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that are not
commands: display per HOST-EVENTS (`:2`); canary `--no-scene` exit 0 before the first launch;
`free -g` ≥ 16 GiB before each build; the harness HEAD as the prompt states it; F104's delivery
in the tree as the prompt states it (this task's baseline binary is F104's).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => ead2d478
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 0c61f1b5
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 226
grep -c '^### F' claude/fable-dispatch.md => 3
# sites, re-resolved at HEAD (content drift = abort)
sed -n '1181p' src/bodyModule/ssystem_factory.cpp | grep -c 'currentSystem..begin()' => 1
sed -n '1192p' src/bodyModule/ssystem_factory.cpp | grep -c 'nb..useNow()' => 1
sed -n '1223,1245p' src/bodyModule/ssystem_factory.cpp | grep -c 'useNow' => 0
sed -n '1227p;1232p' src/bodyModule/ssystem_factory.cpp | grep -c 'ModularBody::forEach\|nb.dumpTrace(out)' => 2
sed -n '1112p' src/bodyModule/ssystem_factory.cpp | grep -c 'type.*header.*jd' => 1
sed -n '1170,1172p' src/bodyModule/ssystem_factory.cpp | grep -c 'extraHeader(out)\|out << "}' => 2
sed -n '1154p;1160p;1179p' src/bodyModule/ssystem_factory.hpp | grep -c 'galacticSystem;\|systems;\|bool inSystem = true;' => 3
sed -n '294,297p;772,778p' src/bodyModule/ssystem_factory.cpp | grep -c 'currentSystem = ssystem.get()\|currentSystem = galacticSystem.get()' => 2
sed -n '1003,1007p' src/experimentalModule/ModularBody.hpp | grep -c 'publishParkedFrame\|hiddenBodies.empty()\|parkedChildFrame = flat' => 3
sed -n '2127p' src/experimentalModule/ModularBody.hpp | grep -c 'Mat4f parkedChildFrame = Mat4f::identity();' => 1
sed -n '457p;473p;475p;501p' src/experimentalModule/ModularBody.cpp | grep -c '!renderHidden || !parent\|parent..useNow()\|parent..parkedChildFrame\|sameFrame(frame, evaluatedFrame)' => 4
sed -n '269,270p' src/experimentalModule/ModularSystem.cpp | grep -c 'getSelected()\|sel..useNow()' => 2
sed -n '838,840p' src/coreModule/core.cpp | grep -c 'ssystemDualDump\|dumpTracePaths(file' => 2
grep -c 'oldSystem' claude/harness/dumpread.py src/bodyModule/ssystem_factory.cpp | tr '\n' ' ' => claude/harness/dumpread.py:0 src/bodyModule/ssystem_factory.cpp:0
grep -n 'class EmptyOldHalf' claude/harness/dumpread.py | cut -d: -f1 => 84
sed -n '78,80p' claude/harness/f100_partition.py | grep -c 'dist") == 0' => 1
grep -l -E 'import dumpread|from dumpread' claude/harness/*.py | wc -l => 24
grep -rl -E 'import dumpread|from dumpread' claude/harness/ --include=*.py | wc -l => 27
# the 30 new-only records of F96's launch dump, by (relation, dist == 0)
python3 -c "import gzip,sys,collections;sys.path.insert(0,'claude/harness');import dumpread;rs=[dumpread.loads(l.strip().rstrip(',')) for l in gzip.open('claude/harness/artifacts/f96/leg_pre/cmd_p0_launch.json.gz','rt') if l.strip().startswith('{')];nn=[r for r in rs if r.get('type')=='body' and r.get('old') is None];print(len(nn),sorted(collections.Counter((r['new'].get('relation'),r['new'].get('dist')==0) for r in nn).items()))" => 30 [((1, True), 10), ((4, True), 1), ((5, False), 1), ((5, True), 18)]
test -f claude/harness/artifacts/f100/partition_f96_leg_pre.json && python3 -c "import json;j=json.load(open('claude/harness/artifacts/f100/partition_f96_leg_pre.json'));print(len(j['P']),len(j['I']),len(j['both']),j['n'])" => 29 29 10 120
# instruments
test -f claude/harness/f101_bisect.py && test -d claude/harness/artifacts/f101/shows && test -f claude/harness/f100_freeze.py && test -f claude/harness/dumpread.py && test -f claude/harness/f90_rehearsal_run.sh && test -f claude/harness/b3_farm.sh && echo ok => ok
test -e /home/claude/sc-f105 ; echo $? => 1
```

**DoD:** predictions before the build; the pre leg; (a) the field; (b) the mutation measured
and reverted; (c) the guard; P1–P6; §11 entry + stub; the three back-markers; the README line;
trees clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-07 → **§11.226** (`INTENT/11.226.md` + stub). Code `ead2d478` → `a30b2c75` (the header field) → `318c0c8b` (the barrier call + its precondition) → `48cc3727` (the refusal line corrected by the smoke suite's own finding); binary `0c61f1b5` → `f26d5ad7` → `ad7a4e47` (MUTATION, reverted, `f26d5ad7` bit-reproduced) → `3f19cb6b` → **`e411b838`**. Gate 27/27. Predictions committed before the first build (`80ee70a`): the (20, 2, 8) partition BY NAME and the 8 identity-frame values NUMERICALLY — **seven exact, the eighth inside its ±4e-06 band**. P1 (20, 2, 8) with the 8 at `dist` 0 · P2 `|bary−Earth|` 0.001205379178 vs predicted 0.001205379219 (the section's mass-ratio gloss refuted) · P3 `SolarSystem`/`galactic`/`SolarSystem` on three states with `EmptyOldHalf` naming it · P4 both ways with the `dist 0` control, and the hazard measured LIVE on the baseline binary through `select planet pleiades` · P5 exactly 8 D12 lines, no repeat · P6 camera + 72/72 + 27/27 + 90/90 + F91 `1fe630a4` 0 FAIL. Smoke rc 0 step for step as §11.220(i3) and it found the pre-registered reload window (10 lines). §11.221(n1) and §11.220(j3) DISCHARGED at both homes + two further homes each; §11.220(j4) answered with its 8 named; §5.139 and §11.220(f1) annotated at the barrier's contract; `f100_partition.py` docstring; README ×4. **A §5 candidate for the supervisor, not minted: tracking a zero-vector body writes 66 non-finite values into the OLD path's view matrices — measured on the PRE binary too.**
**ACCEPTED 2026-09-07 — the verifying commands' `date` read 12:23:04 and 12:24:11–12:29:48 (supervisor,
session 27, Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.226 read in
full; THREE code commits `a30b2c75 → 318c0c8b → 48cc3727` (Claude Opus 5; `ssystem_factory.cpp` — the
`oldSystem`/`inSystem` fields resolved from the factory's own three members and the ONE `nb.useNow()`
call in the new-only loop; `ModularBody.{hpp,cpp}` — `parkedFramePublished` written on the line after
the frame it describes, `useNow()` returning `bool` with the precondition BEFORE the memo test and the
refusal propagating through a hidden parent, the once-per-body D12 line saying what is observed and
naming both ways to be there; 139+/5−, the diff READ, nothing else) and eight harness commits `80ee70a
→ 7003ffb` (Claude Opus 5; the predictions at `80ee70a` before any build), both trees clean; binary
`e411b838`, dry build 0 steps; §11.221(n1) and §11.220(j3) DISCHARGED at both homes (+ §5.143's row and
§11.218(l)); §11.220(j4) marked with its 8; §5.139 and §11.220(f1) annotated; `f100_partition.py`'s
docstring; README ×4; `DEPLOYMENT-MAP` 0 hits, said not assumed; instruments to the digit of the
entry's (m) successor value (258/328/142 · 242/217/25/119 · D 35 · D2 12 · I 89 · I2 37 · M 89). **AND by
my own hand on `e411b838`** (`sc-f105/supervisor/`): the F105 driver's dump stage (12:24:14) — header
`SolarSystem`/true, the 30 new-only records **20 / 2 / 8 by name**, the 2 at `evalCount` 5 with
|bary − Earth| **0.00120537918** vs ½|Moon − Earth| 0.00120537922 and |oal − Moon| 0.0012205, the 8 at
`dist` 0 / `evalCount` 0, exactly **8** D12 lines each naming its parent (the propagation case its
orbit centre); F101's `p23` leg (12:24:47) — post-dump header **`galactic` / false** at 0/120, pre
`SolarSystem` / true at 90/120, 139/139 frozen files in==out; the operator stage (12:25:49) —
`pleiades` tracked to the SAME aim as the `51PegSystem` control, Mars before/after to the digit, the 8
lines in both channels; `f91_run.sh --expect post --locale fr` (12:27:27) — rc 0, table **`1fe630a4`**
byte-identical to F104's, Q2 89/90; the smoke suite (12:28:08–12:29:47) rc 0 with the **10** lines of
the reload window; canary exit 0, `/proc` clear, no lock, the field pair `03fbee59`/`545a51ef` in==out.
Deviations ENDORSED with the executor's arguments: the `dumpread` self-test left at 17 (real-data
evidence on both branches committed; the alternative named and costed); the precondition BEFORE the
memo test with the reload window pre-registered as the task's own risk and caught by its own "exactly
8" check, the MESSAGE corrected rather than the logic (the line had asserted a cause it cannot know);
everything re-measured on the delivered binary; the new `f105_dump.py` beside an unmodified
`f101_bisect.py`; one manufactured arrear removed rather than credited (§11.218(q)). SUSPENSIONS ENDORSED
and ROUTED: **the zero-vector second consequence → §5.148 MINTED at this acceptance** (record-only,
§5.79's criterion: a shipped command, 66 non-finite values measured in the old path's view state on the
baseline binary too; markers at §11.226(f) and §11.216(i); the decision the owner's with §11.216(i)'s
question); the guard's "outside the walk" vs "not walked yet" left undistinguished (one flag, an honest
message — the distinction needs the current system's identity inside `ModularBody`, the owner's
nested-modular-bodies line); the malformed `Code:` trailer on `5832b27` (`… @ a30b2c75 (the mutation is
not in it)`) NOT rewritten — right: four later commits are cited; F103's maps resolve exactly that pin
after the rewrite, and `list_code_trailers`'s silent skip of an unparseable `Code:` line is an
instrument residue (queued). DISPATCHER-SIDE FINDINGS, all ACCEPTED as mine, all output-side (the
gate passed 27/27): (1) the P2 band "the Moon's distance × the mass ratio" — `anchor.ini:58-59` has
`a = b = 1` and `BarycenterOrbit2` uses `b/(a+b)` with no mass anywhere: the offset is HALF the
Earth–Moon distance, 39× the band I wrote (a physical quantity asserted without the read — the
SIBLING class; had the task gated on it, a correct value would have read as a failure); (2) "the F100
partition instrument's class I keeps its key" — right about the key, silent that |I| goes 29 → 27 and
|P ∩ I| 10 → 8 on any post-F105 dump (a consequence left unstated); (3) "the applog carries exactly 8
D12 lines" — an ambiguity between the process's stdout and the log file, resolved by measuring both
(8/8); (4) not mine but marked: §11.220(j4)'s *"nothing uses those bodies (evalCount 0)"* is refuted
by the selection channel. Round tally: **eleven dispatcher defects** (3 value-class, 8 structure-class).
STANDING CONSEQUENCES: **a dual dump is a use for 120 of 120 records** (`48cc3727`); `useNow()` returns
whether the use was served (callers today ignore it); the dump header carries `oldSystem` and
`inSystem` and `EmptyOldHalf` names them (a pre-F105 dump says it predates the field); the 8 anchor
bodies under `Universe` are REFUSED a frame with one D12 line each, and `select` + track on one of them
now lands in the zero-vector class the 18 `dist 0` systems are already in — whose second consequence
is §5.148; `f100_partition.py`'s class I is 27 on a post-F105 launch dump; `/home/claude/sc-f105/`
holds the legs and my runs under `supervisor/`.

### F106 — the rewrite touches ONLY the selected commits (§11.227(a), [vixy verbatim]: *"the rewrite should only rewrite the commit message (and author) on commit Claude is the author, from the last pushed change to the HEAD … the commit themselves shouldn't be affected, so their signature shouldn't move"*): a `--commit-filter` mode in `supervised-by.sh` that emits the ORIGINAL commit id when nothing about a commit changes — not selected (its author after the env-filter is not a Claude identity), message byte-identical to the original, mapped parents identical to the original parents — and `git commit-tree "$@"` otherwise, as today; so an unselected commit keeps its object AND its `gpgsig`, and §5.147's collapse (`cebebf44` rebuilt without its signature onto its twin `b8dddd6c`, 13 commits lost, §11.224(h)) is PREVENTED rather than decided: the 15-commit side chain keeps its shas, the merge keeps both parents, the branch count is unchanged, `code.tsv` lists only what changed; the one case a signature cannot survive — a signed commit whose mapped PARENT changed (the signature signs the parent sha) — is named in a §2(f) block and the closing summary, never dropped silently, and is shown on a throwaway GPG key; exercised on the F103 clone pair, the live pair `--dry-run` only [S, harness tooling; no engine code; no launch]

**Why now / mandate:** §11.227 (the owner's reply, 17:5x, five hours after the close — the tool
he operates Saturday must embody it, and no round is certain before then); §5.147 as resolved
(a tool defect, reading (iii)); §11.224(h) (the measurement: `git commit-tree` drops `gpgsig`,
3829 → 3816 on the clone pair, reproduced twice). **The reading the mint stands on [derived;
each fact a premise line]:** `filter-branch --env-filter --msg-filter` (`supervised-by.sh:915-917`)
rebuilds EVERY commit in the range; its default commit filter is `git commit-tree "$@"`, and
`--commit-filter` replaces it with a command that receives the (filtered) message on stdin,
`<tree> -p <mapped-parent>…` in `$@`, `$GIT_COMMIT` = the original id, and the author/committer
env AFTER the env-filter — and whose stdout is the new commit id, which MAY be an existing one.
So "leave it alone" is one line: `echo "$GIT_COMMIT"` when nothing changed. A signature is over
the whole object (tree, parents, author, committer, message): it survives exactly when the
object is byte-untouched, hence when the mapped parents equal the originals — a signed commit
downstream of a rewritten Claude commit cannot keep it, by construction, and must be REPORTED.
In the live range that case is empty: the one signed commit (`cebebf44`, `%G?` ≠ N once in 98)
hangs off `1ddd32f0`, 0 commits ahead of `origin/master-beta`; the 15 of `1ddd32f0..c6784490^2`
are all non-Claude; `c6784490` is Claude Opus 5's and is rebuilt with its two mapped parents; the
harness range has 0 signed commits. On the F103 clone pair (state U, 94/798, the same signed
commit present with its header) the pre-fix collapse is reproducible (§11.224(f)(h)) and the
post-fix invariants are checkable by sha.

**Measured at dispatch (supervisor, 2026-09-07 17:56–17:58, code `48cc3727`, harness at the mint
commit):** the script md5 `6816c210`, 1380 lines; `--commit-filter` absent (0); the `filter-branch
-f` call at `:915` with `--env-filter`/`--msg-filter` at `:916-917`; the two filter modes at `:415`
(`--emit-env`) and `:426` (`--msg-filter`); `rewrite()` `:660`, `build_map()` `:907` with `UNPAIRED+=`
at `:976`, `scan_range()` `:953`; `git filter-branch -h` names `commit-filter`; the live history as
above (3833 on `master-beta`; 82 Claude / 16 non-Claude in the range); F103's `code.tsv` lacks
`cebebf44` (0) — the collapse's signature; the clone pair reset at `474c595d` / `fe096019`, clean,
94/798 behind its scratch origins, `cebebf44` present there with `gpgsig` 1; `/usr/bin/gpg` present
(the negative case's key).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f106/prediction.txt`, committed before any run): P1
PRE-fix (the delivered script, md5 `6816c210`) on the reset pair: the collapse — `rev-list --count`
3829 → 3816, `cebebf44` absent from the new history and from the map, 13 UNPAIRED (§11.224's
numbers); P2 POST-fix, same reset pair: `cebebf44` keeps its sha with `gpgsig` 1 (`cat-file -p |
grep -c '^gpgsig'`), all 15 of `1ddd32f0..c6784490^2` keep their shas, the rewritten `c6784490` has
TWO parents (the mapped first, the unchanged second), `rev-list --count` unchanged at 3829,
UNPAIRED 0, `code.tsv` = exactly the commits whose sha changed = the selected ones ∪ every
unselected commit with a rewritten ancestor inside the range — PREDICT that number from the graph
(`git rev-list --topo-order` + the selection rule) before the run and state which unselected
commits are rebuilt and why; the harness side unchanged in behaviour (0 signed; predict its map
count from F103's 798); both content assertions and both `VERIFIED` lines; P3 idempotence; P4 the
LIVE pair `--dry-run` before and after (identical unless you add a preview line for
signed-to-be-rebuilt commits — say which, and it prints 0 today); P5 the NEGATIVE case on the
clone: a throwaway key (`GNUPGHOME` under the scratch, `gpg --batch --quick-gen-key`), a commit
authored by a non-Claude identity and SIGNED with it, placed ON TOP of a selected Claude commit
in the clone's range; the tool must rebuild it (its parent changed), its signature is gone, and
BOTH the §2(f) block at the rewrite and the closing summary name it — with the reporting
disabled as a mutant shown silent; P6 the equality mutant: the message comparison disabled ⇒
every unselected commit rebuilt ⇒ the collapse returns (P1's numbers) — the fix shown able to
fail. (2) **THE FIX** in `claude/supervised-by.sh`: a `--commit-filter` mode beside `--emit-env`
and `--msg-filter` (the same self-invocation pattern, `$SELF` absolute); it reads the message from
stdin exactly as the msg-filter does (`cat; printf X` — the sentinel), compares BYTE-EXACT against
the original message (`git cat-file commit "$GIT_COMMIT"` after the header — say which extraction
and prove it on one unselected and one selected commit by the emitted id), compares the mapped
parents in `$@` against `git rev-parse "$GIT_COMMIT"^@` in order, compares the post-env-filter
author (`$GIT_AUTHOR_NAME`/`EMAIL`) against the original's; ALL equal ⇒ `printf '%s' "$GIT_COMMIT"`;
else `git commit-tree "$@"` with the message on stdin — and if the original object carried
`gpgsig`, append `<sha>	<subject>` to `${WORK}/signatures-dropped` (a path exported to the filter,
`CODE_MAP`'s shape), which `rewrite()` turns into a §2(f) block (what: the commit, whose parent
changed; consequence: its signature cannot survive a parent's rewrite; the check command) and
the closing summary repeats by name; the `rewrite()` call gains `--commit-filter`; the header
comment states the rule with §11.227(a) verbatim; `build_map`'s UNPAIRED block is left as the
assertion it now is; the heredoc `sha-maps/README.md` line *"A commit that rebuilt byte-identical
kept its sha and appears in no file"* becomes *"an unselected commit whose parents did not change
keeps its object — sha and signature — and appears in no file"*. NOTHING else moves — assert by
diff. (3) **THE PROOF** on `/home/claude/sc-f103/pair` (reset to state U before AND after every run
— `reset --hard` to `474c595d` / `fe096019`, the scratch origins untouched; `gc.auto 0` stays): P1,
P2, P3, P5, P6; explicit timeouts (Q-68); P4 on the live pair with HEADs + `git status` asserted
around each dry-run. (4) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.147 FIXED (the tool; the P2
numbers); §11.224(f)(h) and §11.227(c) back-markers both homes; `harness/README.md`
§supervised-by.sh and `README.md` §sha-maps (the "kept its sha" sentence); `DEPLOYMENT-MAP.md` R5;
WIP per §0.6; D14.

**Boundaries:** `claude/supervised-by.sh` + the record files; NO engine code; the LIVE pair never
rewritten and never invoked without `--dry-run`; every rewrite and the throwaway key under
`/home/claude/sc-f103/` (the key never enters `~/.gnupg`); no `run_in_background`; nothing under
`/tmp` carries; no launch, no display, no canary (say so).

**Discriminating checks:** (a) P1 vs P2 on the same reset pair — the collapse present pre-fix,
absent post-fix, by sha; (b) `cebebf44`'s `gpgsig` header present after the rewrite (byte-identical
object); (c) P5 both ways — the unavoidable case reported by name, its mutant silent; (d) P6 the
fix shown able to fail; (e) P3 idempotence; (f) P4 the live pair unmoved; (g) the diff confined to
the new mode, the `rewrite()` argument, the header, the heredoc line; (h) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that are not
commands: the harness HEAD as the prompt states it; the live pair clean at every check; the
executor never types `y` at a prompt of the LIVE pair.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 48cc3727
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => e411b838
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 228
grep -c '^### F' claude/fable-dispatch.md => 4
# the tool, re-resolved at HEAD (content drift = abort)
md5sum claude/supervised-by.sh | cut -c1-8 => 6816c210
wc -l < claude/supervised-by.sh => 1380
grep -c -- '--commit-filter' claude/supervised-by.sh => 0
grep -n 'filter-branch -f' claude/supervised-by.sh | cut -d: -f1 => 915
sed -n '915,917p' claude/supervised-by.sh | grep -c 'filter-branch -f\|--env-filter\|--msg-filter' => 3
grep -n 'if \[ "${1:-}" = "--msg-filter" \]\|if \[ "${1:-}" = "--emit-env" \]' claude/supervised-by.sh | cut -d: -f1 | tr '\n' ' ' => 415 426
grep -n '^rewrite() {\|^build_map() {\|^scan_range() {\|UNPAIRED+=' claude/supervised-by.sh | cut -d: -f1 | tr '\n' ' ' => 660 907 953 976
git filter-branch -h 2>&1 | grep -c 'commit-filter' => 1
# the history: the one signed commit and its chain, on the LIVE repo
git cat-file -p cebebf44 | grep -c '^gpgsig' => 1
git cat-file -p b8dddd6c | grep -c '^gpgsig' => 0
git log -1 --format='%P' cebebf44 | cut -c1-8 => 1ddd32f0
git rev-list --count refs/remotes/origin/master-beta..1ddd32f0 => 0
git log --format='%G?' origin/master-beta..HEAD | grep -cv '^N$' => 1
git -C claude log --format='%G?' origin/CC-harness..HEAD | grep -cv '^N$' => 0
git log --format='%an' origin/master-beta..HEAD | grep -vc '^Claude' => 16
git log --format='%an' origin/master-beta..HEAD | grep -c '^Claude' => 82
git rev-list --count master-beta => 3833
git rev-list --count 1ddd32f0..c6784490^2 => 15
git log --format='%an' 1ddd32f0..c6784490^2 | sort -u | tr '\n' ' ' => Calvin Ruiz Kenan-Blasius Lionel RUIZ lionelruiz13
git log -1 --format='%an' c6784490 => Claude Opus 5
grep -c '^cebebf44' claude/harness/artifacts/f103/p4_maps/code.tsv => 0
# the F103 clone pair, the fixture (its range is the F103-time state: 94 / 798)
test -d /home/claude/sc-f103/pair/spacecrafter/claude && echo ok => ok
git -C /home/claude/sc-f103/pair/spacecrafter rev-parse --short=8 HEAD => 474c595d
git -C /home/claude/sc-f103/pair/spacecrafter/claude rev-parse --short=8 HEAD => fe096019
git -C /home/claude/sc-f103/pair/spacecrafter status --porcelain | wc -l => 0
git -C /home/claude/sc-f103/pair/spacecrafter rev-list --count '@{upstream}..HEAD' => 94
git -C /home/claude/sc-f103/pair/spacecrafter/claude rev-list --count '@{upstream}..HEAD' => 798
git -C /home/claude/sc-f103/pair/spacecrafter log --format='%G?' '@{upstream}..HEAD' | grep -cv '^N$' => 1
git -C /home/claude/sc-f103/pair/spacecrafter cat-file -p cebebf44 | grep -c '^gpgsig' => 1
which gpg | head -1 => /usr/bin/gpg
test -e /home/claude/sc-f106 ; echo $? => 1
```

**DoD:** predictions before any run; the fix; P1–P6; §11 entry + stub; §5.147 FIXED; the
back-markers; the README homes + the heredoc line; R5; trees clean; WIP cleared; baselines LAST.
**WIP:** 2026-09-07 checkpoint 2 — predictions pre-registered (`acee76b`, before any run); **P1
GREEN**: the pre-fix collapse reproduced to the digit on the reset pair (3829→3816, 94→81, 13
UNPAIRED by name, `cebebf44` gone, maps 83/798/2 with §11.224(i)'s three md5s, tip `d40f4eb1`,
545/282/286) — one prediction of mine and one clause of §11.224(h) REFUTED: the rewritten merge
keeps TWO parents (the top 2 of the side chain have no twin). Pair reset to state U. Next: the fix.

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

- **Session-27 decision items (2026-09-07, the SATURDAY-TOOL / TWO-STEP-SOLVER / DUMP-CHANNEL round —
  F103 · F104 · F105, three for three). Q-70's shape: one decision per node, its held set at the node,
  anchors by ID and symbol, correlated items together. Nothing asked in-session (Monday). The first
  node is the one Saturday's operation needs.**

  ```
  YOURS — nothing here substitutes them
  [Y8] §5.147 · the rewrite COLLAPSES 13 commits ─ decide BEFORE Saturday's supervised-by.sh run;
       irreversible after the force-push
       fact    master-beta carries the same 13 commits TWICE: cebebf44 (GPG-signed by GitHub, Lionel's
               "Update install_dependancies_ubuntu.sh", 2026-04-30) beside its unsigned twin b8dddd6c —
               same tree, author, date, subject, same parent 1ddd32f0 — rejoining at c6784490;
               `git commit-tree` drops gpgsig, so the rewrite rebuilds the signed chain INTO the
               unsigned one: 3829 → 3816 commits, range 94 → 81, reproduced twice byte-identically,
               while the tool's content assertion PASSES (the tip tree is unchanged); the 13 old
               shas are cited in INTENT/11.203.md only, in 0 trailers, and are ABSENT from the map
       (i)     ACCEPT AND MAP — widen build_map's twin search to the whole new history so the map
               carries cebebf44 → b8dddd6c and the twelve, step E repoints them; the signature
               stays lost; cheap, each has exactly one fingerprint twin
       (ii)    REFUSE AND RESOLVE FIRST — the tool STOPs on any unpaired commit; the duplicated
               chain is resolved deliberately before any rewrite (the tool's own philosophy)
       today   the tool WARNS (a §2(f) block at the map, READ BEFORE PUBLISHING at the end, Undo
               printed) and proceeds
       ask     (i) or (ii) — one word
       →       §11.224(h) · §5.147 · DEPLOYMENT-MAP R5
       ANSWERED [vixy 2026-09-07, ~17:50] → §11.227: "the rewrite should only rewrite the commit
       message (and author) on commit Claude is the author, from the last pushed change to the
       HEAD … the commit themselves shouldn't be affected, so their signature shouldn't move" —
       NEITHER (i) nor (ii): the collapse is the TOOL's defect (filter-branch rebuilds every
       commit in the range; commit-tree drops gpgsig); a commit-filter that leaves unselected
       commits untouched keeps cebebf44's signature and the 13 shas (its parent 1ddd32f0 is
       already pushed) — F106, dispatched the same session; the one unavoidable case (a signed
       commit DOWNSTREAM of a rewritten one) is empty here and will be reported by name

  [Y4] the tool you operate Saturday ─ what changed, in the order you will meet it
       state   RUNNABLE NOW, and was not before this round: build_map's `[ ] && printf` returned 1
               on its last iteration and `set -e` killed every run AFTER the code rewrite and
               BEFORE rollback (D1: 73 of 138 trailers left dangling, in silence); build_repair_map
               re-derived "already dangling" AFTER the rewrite and aborted every run on a pair with
               a pre-existing dangling trailer (D2) — this pair has 2 (5f68d374, 841175d3, twins
               unique). Both fixed, each with a mutant on record
       B1      a renamed branch now STOPs the preview (exit 1, `--dry-run` too) with a §2(f) block;
               `--branch-alias=master-beta=<new-name>` is the one flag that proceeds; the rename
               and the tool are now order-INDEPENDENT (F92's checklist needs no reordering)
       maps    claude/sha-maps/<UTC>-<code8>-<harness8>/{code,harness,repair}.tsv + a written-once
               README, committed by the closing commit (unconditional once a map exists) — your
               "commit tracking" was READ as this map; your correction may differ
       today   `--dry-run` on the live pair: 94 code / 805 harness selected, 138 distinct trailers,
               2 already dangling, no STOP; deterministic (two runs, byte-identical maps)
       skip    ONE pin the parser SKIPS silently: harness 5832b27's trailer reads
               `Code: master-beta @ a30b2c75 (the mutation is not in it)` — outside the grammar; the
               map resolves it after the run; the parser's silence is a residue (queue 7)
       →       §11.224 · README.md §sha-maps · harness/README.md §supervised-by.sh · §11.212(g)

  [Y2] §5.145 FIXED on your ruling as corrected ─ two veto points, each cheap to reverse
       scope   read as the iterative CLASS (all four advancing branches of eccentricAnomaly + both
               comet steps); Eris's branch alone was built as the MUTATION: indistinguishable on
               the engine (120/120), so the two forms differ in COST only — 2.36 µs/frame worst
               case = 0.24 % of D11 for the class form, the narrow form's share smaller
       old     orbit.cpp is shared: the old path converges twice as fast per frame after a date
               jump — strictly more exact on the comparison baseline (as-if)
       count   still yours: two steps sufficed for Eris by four orders (1.1987° → 1.06e-05°); the
               alternative shape — iterate to a criterion — already exists in the dead warp()
               methods (§11.225(j1))
       →       §11.225 · §5.145 · §11.223(b) · §11.76(b)

  [Y9] §5.148 · tracking a ZERO-VECTOR body poisons the old path's view state ─ 66 NaNs, on the
       baseline binary too
       fact    `select` + `flag track_object on` on any of the 19 dist-0 records (18 star systems +
               Universe; now also the 8 anchors under Universe, refused a frame by F105) writes nan
               into helioToEye / matLocalToEye / matJ2000ToEye / matEarthEquToEye / headingVector /
               moveAim; the navigator step that first produces it is not read
       ask     with §11.216(i)'s question, one answer: what a body with no position answers to a
               track — refuse with a §2(f) line at the tracking seam, or a defined aim
       →       §11.226(f) · §5.148 · §11.216(i)

  [Y3] §5.146 ─ carried: minted on your word "edit it" (SharedBuffer's bind/release guards + the
       contract line at BufferMgr::releaseBuffer, EntityCore)              → §11.223(c) · §5.146
  [Y1] §5.142 · the pool policy ─ carried (grow / refuse / degrade)          → §11.222(h)
  [Y5] §5.144 · the 8–12 h leg ─ carried  ·  [Y6] §5.140 · orbit_lon ─ carried
  [Y7] scratch trees ─ sc-f103 (959 M: the clone pair + the P4 maps' object store), sc-f104 (898 M:
       the pre/post/mutant binaries), sc-f105 (955 M) join the list; sc-f98 (1.8 G) still holds the
       only complete arm-C applog

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] F103  `--branch-alias` in the `--author-fix` shape; the STOP at step A'; the maps' location
             and naming; the closing commit unconditional once a map exists; D1/D2 fixed under the
             forced-expansion rule (both blocking for your run)
  [V2] F104  the class scope ([Y2]); ITERATIVE_STEPS_PER_CALL = 2 in iterative_orbits.hpp, one home
  [V3] F105  useNow() returns bool; a use in a NEVER-published frame is REFUSED with one D12 line
             per body (the 8 keep their honest zero); the header's two fields; a dump a use for
             120/120 (the 2 anchors under Earth/Moon now move with the frame)
  [V4] two §5 mints at acceptances (§5.147 [Y8] under §5.131's precedent — a repository-state
             property in the defect register; §5.148 [Y9])
  [V5] archival pass 19; this block

  HELD OPEN, not absorbed
  [H1] §11.225(j2)/(j3): the new path's orbit-line sampler walks the position solver's own seed
       (180 dates through iterativeLastE), and Eris's pre-fix staleness (+32 rad) is unattributed
       — a leg, next round (S)
  [H2] §11.226(h): after `body action reload` the two anchors under Earth/Moon are refused for
       ONE frame (their creator uses them before the walk publishes) — measured self-healing; the
       line names both ways to be there
  [H3] the guard cannot tell "outside the walk" from "not walked yet" — needs the system's
       identity inside ModularBody: your nested-modular-bodies line (§11.223(e))
  [H4] instrument residues: purge-path.sh rewrites the same way and writes NO map (README corrected
       at acceptance); list_code_trailers skips an unparseable `Code:` line silently; the scan's
       EVENT lexicon carries no FIXED (by ruling), so B1's FIXED marker is invisible to it
  [H5] §5.84's own trigger still unmeasured; §11.215(g)'s four bodies' ulps predicted to change,
       not re-measured (a 3 h soak)

  FACTS, no decision asked
  [F1] three for three delivered AND supervisor-verified same session, every delivery re-run by my
       own hand; code 474c595d → 48cc3727 (four executor commits); binary b5f08778 → e411b838
  [F2] exactly ONE readout moved across the round at the un-moved launch state: Eris's parked
       position (1.1987° → 1.06e-05°); the F91 table's md5 is now 1fe630a4 (c125adf0 retired)
  [F3] eleven dispatcher defects this round (3 value-class, 8 structure-class), all output-side,
       all caught before a delivery; three more caught AT THE MINT by the instrument
  [F4] the host: same boot, `:2` alive with sessions 14/15, RAM 52 GiB at open and close; 41
       launches, no HOST-EVENTS entry owed
  ```

- **Session-26 decision items (2026-09-07, the NULL-PARENT / PARKED-READOUT / SYSTEM-SWITCH /
  UNIFORM-POOL round — F99 · F100 · F101 · F102, four for four). THE FIRST BLOCK IN Q-70's SHAPE, on
  your word of 2026-09-06: one decision per node, its held set stated at the node, anchors by ID
  and symbol, correlated items placed together; length is a free variable. Nothing asked in-session.**

  ```
  YOURS — nothing here substitutes them
  [Y1] §5.142 · the uniform pool ─ a D13 POLICY in EntityCore
       fact    1 MiB, created once, never grows; a 06.sts body costs 1344 B on both paths
               (320 old-eager + 1024 new-at-load, alignment 64); the pool runs out at the
               675th body of 06.sts ALONE (reproduced in two launches); 14.sts adds nothing —
               its 527 "stars" are 128-byte in-galaxy OJM models that walk into the empty pool
       GROW    2 MiB holds the whole corpus (1.72 MiB) with 14 % headroom; each MiB = 780
               bodies today / 1024 after B8; on a smaller device the grown pool fails through
               §5.60's own unguarded return ("Failed to create buffer bloc") — silently
       REFUSE  anchor = the load authority (OjmMgr::load already prints both outcomes);
               then the body is ABSENT (§5.50's shape: 339 of 1013 satellites silently gone)
               or PRESENT without its disc (the new path supports it, the old does not)
       DEGRADE meshFrag is 768 of its 784 B of shadowingBodies[8]; a cap of 1 gives 2.1x the
               bodies per byte; the runtime-sized allocation exists, the shader's array does not
       ask     one word: grow / refuse / degrade — and under REFUSE: absent, or disc-less
       →       §11.222(h) · §5.142 · §11.161(b)

  [Y2] §5.145 · RESUME_EXTRA_ITERATIONS = 4 ─ YOUR constant (§11.76(b))
       fact    same date, same binary, only the evaluation count varies:
               5 → Eris 1.198725°  ·  9 → 1.1e-05°  ·  15 → 2.2e-05°
               every "Eris = the trees' own 1.198° gap" since F44 was this (markers placed)
       ask     raise it (9 suffices today; a slower converger moves it again), or a
               convergence criterion at the use (cost priced by F100's evalCount instrument)
       →       §11.220(j1) · §5.145
       ANSWERED [vixy 2026-09-07] → §11.223(b): "double the iterations per cycle for those types of
       orbits only, otherwise it might became slightly noticeable at high simulation speed" — the
       ~~iterative types resume with 1+8, the rest keep 1+4~~ CORRECTED on your second message
       (08:5x): the NEWTON STEPS PER CALL double in the solver of those types — 2+8 at a use, the
       barrier's loop unchanged, every per-frame evaluation twice as fast; §5.84 annotated at the
       fix; orbit.cpp is shared — old converges faster after a date jump (veto point)

  [Y3] §5.146 · the allocator releases a REFUSED SubBuffer into its free list ─ read, not run
       fact    acquireBuffer leaves offset/size indeterminate on refusal; SharedBuffer binds
               through it and its destructor hands the garbage range back (SubBuffer.hpp:6-10,
               BufferMgr.cpp:43-44, SharedBuffer.hpp:9,15-18); the arm-C log cannot separate
               a phantom grant from the landscape's legitimate release
       ask     "read it first" = one discriminating launch (S, next round), or yours
       →       §11.222(g)(6) · §5.146
       ANSWERED [vixy 2026-09-07] → §11.223(c): free's contract, no assertion — "the application must
       ensure only valid allocations are freed"; the fix site is SharedBuffer (the caller) + the
       contract line at BufferMgr::releaseBuffer; EntityCore — minted on your one word "edit it"

  [Y4] R5 · the PUSH ─ master-beta 94 / CC-harness 792 unpushed at 01:29 (before this close)
       ANSWERED [vixy 2026-09-07] → §11.223(d): Saturday/Sunday, "along with supervised_by.sh patching
       of commit history with commit tracking" — B1's silent fallback is fixed FIRST (next round, S);
       ONE LINE THE REWRITE NEEDS: emit and commit the old→new SHA map — 133 pins / 695 trailers
       resolve through it, never by search-and-replace over the record (§11.212)
  [Y5] §5.144 · the 8–12 h cache-vs-leak leg ─ yes / no  (carried from s25, unchanged)
  [Y6] §5.140 · what `orbit_lon` MEANS ─ one word; unblocks §5.21's two halves  (carried)
  [Y7] scratch trees ─ say the word: sc-f99 · sc-f100 · sc-f101 · sc-f102 join s25's list;
       sc-f98 (1.8 G) holds the ONLY complete arm-C applog — keep until [Y3]'s launch

  ROUTED TO THE MAIN TESTER (§11.161(c)), not asked
  [T1] eight of his shows fly above 1e16 (14 · S02 · S07 · S09 · S10 · S12 · S12old · W15):
       is leaving the solar system for good MEANT? a home planet set out there decides
       whether the descent comes home or lands in a fresh empty system   → §11.221(n3)
       ANSWERED BY YOU, not the tester [vixy 2026-09-07] → §11.223(e): 1e16 is the designed inGalaxy
       entry; the mode is to be REMOVED by nested modular bodies (a system = a body seen from
       outside as inGalaxy shows it) — question withdrawn; §5.143 retires with that design
  [T2] 06old.sts lands 156 of 170: 13 duplicate names + TDRS 3 with no coord_func
       (both paths refuse it and say why)                                  → §11.221(g)
  [T3] 06.sts alone aborts the reference binary at its 675th body — one show, not two → [Y1]

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] F99  the OLD path guarded inside protosystem.cpp's location_orbit branch (a crash
            guard, the §5.50/§11.124(h) precedent; the only path it alters ended in SIGSEGV)
            + the new loader's twin; `parent none` now refused in words on both paths
  [V2] F100 useNow() keyed on (date, parent frame); the +4 bound to a date change; the
            selection's per-frame use costs one 16-float compare (0.0000 refreshes held)
  [V3] F101 dumpread.load_dump raises on an empty old half by default; 4 readers opt out
  [V4] two §5 mints at acceptances (§5.145 [Y2], §5.146 [Y3]); three rows corrected at
            their headlines by the executors' measurements (§5.141, §5.139, §5.143)
  [V5] archival pass 18; this block's shape (Q-70)

  HELD OPEN, not absorbed
  [H1] the dump header names no system — one field, dump channel (S)         → §11.221(n1)
  [H2] the dump calls the barrier for 90 of its 120 records (S)               → §11.220(j3)
  [H3] §5.143 stays OPEN (retires with B8); `search` NOF outside the system (R22)
  [H4] §11.218(g)'s "zero errors after 06+06old" — an F98 executor claim its own log
       refutes; corrected by F102 with markers, the arm-C figures reproduced to the digit

  FACTS, no decision asked
  [F1] your ssh move changed nothing that reaches a launch — measured at open; Q-61(3) holds
  [F2] `select planet` + `get status object` answers the OLD path for every both-tree name:
       a new-path readout needs a new-only name or the dump's sidecar          → §11.220(e1)
  [F3] ten dispatcher defects this round (5 value-class, 5 structure-class), all output-side,
       nine caught before a delivery and one at the close (corrected one commit later); the PREMISES instrument caught one AT THE MINT; two
       executor report defects (F99 self-caught; F98's found by F102 from F98's own log)
  ```

- **Session-25 decision items (2026-09-06, the OFFSET-FRAME / LOCATION-ORBIT / SECOND-SOAK round —
  F96 · F97 (in part) · F98; written Sunday evening on your stated capacity: nothing asked
  in-session, every item one line to answer or to ignore; the first one is the round's headline):**
  - **YOURS, and nothing here substitutes them:** (1) **"READY" HAS A NEW FACT AGAINST IT — §5.142:
    two of your tester's own shipped shows, `fscripts/06.sts` (1013 authored bodies) then
    `fscripts/14.sts` (528), played in the order the directory sorts them, exhaust the UNIFORM
    buffer pool, lose the Vulkan device and ABORT the reference binary** (`Can't allocate buffer in
    'uniform BufferMgr' !` ×1557, each one followed by `Succesfull loading ojm`, then a stall parked
    in `App::draw`, device lost, SIGABRT at +76 s — reproduced three ways by the executor and by my
    own hand; `14.sts` alone is clean, `06old.sts`+`14.sts` (698) is clean, 1156 bodies stand with
    zero errors — it is the ACCUMULATED count). Your stratum (EntityCore `BufferMgr`), a D13-class
    policy: grow the pool, refuse the load with a §2(f) line, or degrade — say which, or say
    "read it first" and the reading is one S task. (2) **R5 — the PUSH**: `master-beta` (the count is
    in the close note) and `CC-harness` from a keyed host — unchanged, still the one act that makes
    any of this reach the developer. (3) **§5.144 — the RSS grows 68 MB/h under his corpus and did
    not saturate in 4.5 h** (LEAK by F95's rule, the steps falling 68 → 32 MB — the shape of a cache
    that has not filled; 897 MB of media in the corpus): ONE 8–12 h leg with the
    `texture: already in cache` count beside the RSS separates a cache from a leak. Yes/no. This
    subsumes session 24's item (3) on F95's 0.3 MB/h tail (180× smaller). (4) **§5.115 in
    arithmetic: 193 MB/h, 828 MB in one 4.5 h session of his own shows** — one working day on the
    shipped default crosses a gigabyte, in one dated file; R20's "8 launches" is ~6.6 GB on this
    load. The retention half is decision-free; the per-launch key vs the per-day file layout is the
    design you should see before it ships (§11.207(g)(5), not minted this round for that reason).
    (5) **§5.140 — `orbit_lon` MEANS TWO THINGS**: `surface_point` + grounded lands a rover ninety
    degrees EAST of the planetographic longitude the camera, `moveto lon` and old's
    `AnchorPointBody` mean (measured both ways, live on Mars: the cells swap); it is the mesh
    convention's `+π/2` (`getAxisRotation()`'s own *"TODO Fix ojml ?"*), the same term §11.152(o)
    could not explain and F91 removed from RA/DE. `orbit_lon` is a RATIFIED key (D16–D19), so its
    meaning is your ruling — Reading 1 (planetographic, one meaning per key across paths and
    providers) costs NO content (census: 0 uses of `surface_point` in ssystem / scripts /
    modularSystem / doc / data) and unblocks §5.21's remaining two halves (the frozen spin and the
    missing `getRotEquatorialToVsop87()`, 86.3° on Mars); Reading 2 keeps today's landing and makes
    one key mean two longitudes. One word. (6) **§5.139 — a body out of the view cone keeps a frozen
    eye-frame position** after a camera move, so its alt/az and RA/DE answer in a camera state that
    no longer exists (48 of 120 records, 109.9°–170.7° from old after one `select` + track; the
    code's own comment says non-drawn bodies stay queryable): the update walk's pruner is unread —
    an S–M leg in `ModularBody`, decision-free once the mechanism is confirmed; say if you know the
    pruner. (7) **The pole guard at the ZERO vector** (§11.216(i)(j2)): 29 of 120 shipped records
    (systems, anchors) have no direction at all and print `AD/DE : 00h00m00s / +90°00'00"` now where
    they printed `06h00m00s / +00°00'00"` — both undefined; should "no direction" print differently
    from "at the pole"? One line either way. (8) **Scratch trees, say the word to remove**
    (`du -sh` at 18:35): `sc-f84` 4.2G · `sc-f86` 1.3G · `sc-f89` 1.5G · `sc-f98` 1.8G · `sc-f97`
    340M · `sc-f96` 222M · `sc-f94` 189M · `sc-f95` 125M · `sc-f93` 64M · `sc-f90` 27M · `sc-f91`
    6.4M · `sc-pass16` — every cited number resolves by commit without them.
  - **ROUTED TO THE MAIN TESTER, not asked (§11.161(c)):** his `fscripts/panorama5.sts:102` carries
    a `struct if end` that closes nothing (the engine's own annotator wrote the diagnostic onto
    the farm's copy; his file is byte-identical) · the seven shows that chain another script
    (`07`, `07g`, `07isc`, `S11a`, `S11b`, `S14`, `W14`) are the only SHOW-TIMEOUTs the soak
    records — a duration model cannot see a chained script; not a fault · `06.sts` + `14.sts` in
    directory order is a reproducible abort on this build (§5.142) — until the pool question is
    answered, playing them in one session is the one thing his corpus does that the reference
    binary cannot survive.
  - **VETO POINTS taken this session (implemented-and-live, each cheap to reverse; silence =
    endorsed):** (a) **F96's fix** (code `24100461`, `Camera.{hpp,cpp}` only): `observedToLocalPos`
    is the inverse of the RENDER rotation, so the alt/az readout, the atmosphere's sun direction,
    the view-directed descent and the tracking feedback read ONE expression — 27.000007° off old
    with `set zoom_offset 0.3` armed before, 0.000017° after; byte-identical at every shipped
    default (the F91 tables, the dwell frame `5215565b`); the tracking site no longer undoes the
    offset by hand; the pole branch of `observedPosToRaDe` answers RA 0 / DE ±90° as old does.
    (b) **F97's latitude** (code `22499f04`, `orbit.cpp` ONE token: `lat(_lat)` →
    `lat(_lat*M_PI/180)` in a class no shipped or loaded scene reaches on either path — the
    comparison baseline moves for no scene; plus a `L_WARNING` when a body spells both
    `location_orbit` and the grounded relation, at the loader). (c) **Three §5 mints at F96's and
    F97's acceptances** (§5.139 frozen positions · §5.140 the ninety degrees · §5.141 the loader's
    null parent, §5.50's class, fix decision-free next round) and **three at F98's** (§5.142 the
    abort · §5.143 the old half emptied by a runtime `parent none` system, a standing hazard for
    parity instruments — assert `bodies_old > 0` before reading the old half · §5.144 the leak
    verdict). (d) **The soak driver is ONE instrument for both corpora** (`f95_soak.py
    --playlist-dir/--root/--cap/--skip-show`), its duration model has one home (`sts_duration.py`,
    imported by the smoke suite too, proven equal on 145 files) and sees `struct loop`. (e) The
    §0b.1 warm-up bullet now names the two ledger instruments' directory (the miss-ledger item,
    two recurrences). (f) `f45_run.sh:102/186` annotated: its `md5sum ./* | md5sum` is a
    glob-ordered digest — consistent in its own in==out use, bank-unsafe across locales (Q-69).
    (g) archival pass 17.
  - **HELD OPEN, not absorbed:** §5.21's two remaining halves (blocked on (5), not on effort);
    §5.142's mechanism (your stratum); §5.143's mechanism (one launch, `14.sts` line-bisected to the
    `parent none` load); §5.144's discriminating leg ((3)); `body action reload` — §5.137's own
    trigger — is in NO soaked show yet; the pinned-clock drift on the NEW half when shows move the
    clock (235/286 bodies, ≤ 1.49e-07 AU, annotated at §5.62/§5.84 — one-parameter experiment:
    dump later than 1.5 s after the jump); the §11.207(g) tail (§5.66+§5.71, §5.115, A15's
    residual) with the reasons they were not minted in the open note.
  - **FACTS, no decision asked:** the F97 STOP clause fired exactly where it was written to —
    the executor derived the frame before one byte of code, found the fork, delivered the
    separable half with its own both-ways proof and recorded both readings; `re.period` is a
    `float`, so on a linear-`re` parent §5.21's frozen spin is an I2 defect and not a numeric one
    (−7e-6°), while on Earth it is +8.856° · the config key `[navigation] view_offset` is a LATENT
    offset on the new path (stored at startup, armed by the first commanded view move) · the
    tester's corpus cycle is 31.9 min of wall for 135 shows at a 60 s cap, 104–106 authored
    pauses resumed per cycle, probe round trip ≤ 214 ms over 541 samples, thirteen shows capped
    exactly as the model predicted · **six dispatcher defects this round, all mine, all
    output-side, all caught via report-not-absorb or the instrument** — three of Q-67's class
    (premise counts typed before the run; a map line number; a stale reachability premise carried
    from an acceptance-time disposal) and three of its SIBLING class (structure asserted without
    the read: "one used"; a census pattern that modelled one of two word orders; a `plan`
    byte-identity requirement inconsistent with the design's own change) — the last is the class
    hiding INSIDE a measured number, recorded at Q-67.

- **Session-24 decision items (2026-09-06, the ENGINE-AND-SOAK round — F91 · F94 · F95, three
  for three; written Sunday morning on your stated capacity: nothing asked in-session, every
  item one line to answer or to ignore):**
  - **YOURS, and nothing here substitutes them:** (1) **R5 — the PUSH**: `master-beta` (90
    commits) and `CC-harness` (the count is in the close note) from a keyed host — unchanged,
    still the one act that makes any of this reach the developer. (2) **"READY" IS NOW YOUR
    WORD**: both gates the map put on it have run — T5.1 (F90, four identical runs) and T5.2
    (F95: 5 h 44 min of show load in two launches, 128 cycles, thirteen clean quits, no member
    of the stability class fired) — and what the soak CANNOT see is stated at §11.215(m): no
    pixel (B30), no body authoring (the `fscripts/` corpus, `06old.sts`'s ~~3000~~ **170** satellites [corrected 2026-09-06 at F98's acceptance, §11.218(i): `06.sts` is the 1013-body show; 1719 bodies in 8 shows] —
    a second soak, owed), an intermittent class at thirteen quits. The rename is §11.212(h) when
    you say so; **the PR-target sentence is still yours** (session 23's item 2). (3) **The RSS
    tail** — 0.28–0.38 MB/h, monotone, reproduced in both legs, 3–4 pages per cycle, ~3 MB over
    an eight-hour day: is it worth ONE 8–12 h leg to separate a settling tail from a slow leak?
    Yes/no. (4) **The four-body selector** (Ananke/Neried/Setebos/Sycorax answer two doubles ≤
    136 ulps apart on the OLD path at a pinned clock — §5.84's mechanism measured): the
    recommendation is to LEAVE it (2–4 orders below §11.87(c)'s accepted float floor; the new
    path is bit-stable 120/120) — say the word only if you want the selector attributed.
    (5) **`Camera::observedToBodyLocalPos` now has no in-tree consumer** (F91 kept it as
    §5.86's member and the probes' contract): keep or delete — an interface decision, yours.
    (6) **Scratch trees, say the word to remove** (measured at the close): `sc-f84` 4.2 GB ·
    `sc-f86` 1.3 GB · `sc-f89` 1.5 GB (+ a registered code worktree: `git worktree remove
    --force`) · `sc-f90` · `sc-f93` · `sc-f91` · `sc-f94` (the preserved pre-fix binary, 190 MB)
    · `sc-f95` — every cited number resolves by commit without them.
  - **ROUTED TO THE MAIN TESTER, not asked (§11.161(c)):** the doc token `doc/superscript.sts:1529`
    `Ganymed=503` → `Ganymede=503` (his file; after F94 it is the one spelling on that line
    matching nothing) · the sentence at §11.214(k)(2) on `$body_selected` (1 of 408 scripts on
    this field reads it, tests 0, same arm both binaries — open only for his own installation)
    · the new path's nav string is TWO lines with `" / "` where old prints one with `"/"`
    (§11.213(i4): a readout-format expectation, not a frame question) · the pause item
    (session 23 (7)) enriched: honouring the authored pauses unattended costs ~39 resumes per
    159 s cycle, `diaporama.sts` alone authors 100 in a `struct loop`.
  - **VETO POINTS taken this session (implemented-and-live, each cheap to reverse; silence =
    endorsed):** (a) **F91's fix** (code `5a1e5749`, `src/experimentalModule/` only): the RA/DE
    readout is `viewMat`'s exact inverse landing in old's frame — including the FULL surface
    fold, whose constant `+π/2` was §11.4's "−90.0003° zero point" (so §11.4 closes with no
    constant anywhere); plus two port slips found and fixed in the same member (the LOCAL hour
    angle was built from the observer's LATITUDE; a bare `fmod` printed negative hour angles
    pre-J2000); plus `ModularObject::getEarthEquPos` re-pointed to the observer-centred
    authority (I1 — `Body::getEarthEquPos`'s contract; `set home_planet selected` feeds it
    back through the inverse map). Result: 88 of 90 bodies print old's RA/DE string byte for
    byte (Eris = the two trees' own 1.198° position gap; Puck = one arcsecond of float32).
    (b) **F94's two literals** (code `a2a880ef`, `core.cpp:2251/:2255`): `$body_selected`
    answers 600/503 on your tester's instruction. (c) **§5.138 minted at F91's acceptance**
    (record-only): the new path's alt/az readout and the atmosphere's sun direction ride the
    offset-FREE rotation while their inputs carry the B17 offset — pitched by `offset ×
    halfFov` whenever `set zoom_offset` is armed; one leg owed, decision-free, next round.
    (d) **The F95 instrument** (`f95_soak.py`, one detached driver + foreground reads — the
    design Q-68 measured possible): a fourth harness family the developer can run. (e) the
    §11.215 marker's second home at the §11.76 stub (the executor wrote one home). (f)
    archival pass 16.
  - **HELD OPEN, not absorbed:** the second soak over `fscripts/` (owed, next round's
    candidate); §5.138's leg + the §5.86 pole-guard rider (one S task); `f44_parity.py`'s
    `reconstruct` is now WRONG for a post-F91 binary by construction (§11.213(k)(3) — reuse
    `f91_parity.py`); every harness value recorded before `5a1e5749` for the new path's
    RA/DE/SA/GHA/LHA/LPA is a record of the pre-fix engine (a sweep for such matchers was NOT
    run — §11.158(l)(2)'s warning stays live); F90's `show_own_duration` is `struct
    loop`-blind (`diaporama.sts` SHOW-TIMEOUTs every cycle by that model, not by the app);
    the `Frame stall detected` rate (~80/h unlocked; 1–2 per 92 s launch) is recorded, never
    gated, and unattributed.
  - **FACTS, no decision asked:** §11.4's calibration item was never a calibration item ·
    the OLD path's Kepler solver seeds from a `mutable` member and steps once per call
    (`orbit.cpp:515-573`) — §5.84, now measured; the NEW path's `useNow()` barrier (§11.76(b),
    your `+4` constant) is what makes it bit-stable · the script log costs 1.88 MB/h under
    show load, ~45 MB across R20's eight launches, all in ONE per-day file (§5.115) · thirteen
    `shutdown action now` on `404b9e89`, all exit 0 within 0.7 s · **three dispatcher defects
    this round, all mine, all output-side, all caught via report-not-absorb** (a ledger number
    rounded in a prompt; a section whose "comment allowed" contradicted its own zero-hit
    check; a two-run stall sample stated as a rate) — and one EXECUTOR report defect caught by
    my own instrument run (a baseline claimed "all 0" where the pair-check read +1/+1: a
    one-home marker), Q-67's class from the other side.

- **Session-23 decision items (2026-09-05, the READINESS round — your line *"renamed main
  once ready"* was the trigger; kept short on your word about this week's capacity; the
  F92 executor's own rename item follows this block and is its expansion):**
  - **YOURS, and nothing here substitutes them:** (1) **R5 — the PUSH**: `master-beta` (88
    commits) and `CC-harness` (708 before this close's commit) from a keyed host — unchanged,
    still the one act that makes any of this reach the developer. (2) **R6's remaining half,
    one sentence: after the rename, do pull requests still target `2023-master`?** Your line
    answers the policy (rename, not redirect) and not this; `doc/developer-entry.md` §1 says
    they do; the patch changes the branch's NAME and leaves that half alone. **The rename
    itself is §11.212(h): eleven ordered acts, each with a check that can fail, and a patch
    covering exactly the four live pointers** (`harness/artifacts/f92/rename-live-pointers.patch`,
    regenerated at this close after the definition edit below; step 0 re-measures before
    anything). What NOT to touch: 133 pins, 695 trailers, 89 records — a search-and-replace
    would destroy the record it claims to maintain. (3) **R23's procedure** — where the outside
    catalogue installation lives and whether it may be documented (carried from session 22).
    (4) **§5.111's unmatched set** (§11.209(g)): the body NAME on the drawn path stays English
    — one token at two sites (`_(body->getEnglishName())` at `ModularObject.cpp:11,:33`), but
    WHICH translation (UI or sky — `getNameI18n()` exists beside it) is the question both
    files' comments flag; `ModularBody` in the type slot (a different string from old's
    `_(getTypePlanet(...))`, not a missing wrap); the generated `" (orbit centre)"`. Until
    answered, a French readout is French except its first line. (5) **§5.136** — `app_locale`
    is INERT whenever it differs from `sky_locale` (one static map, last-loaded wins, the app
    logs the locale it then ignores; three shipped surfaces; D12): core architecture, §5.121's
    family — one map per translator, one translator, or an explicit order. (6) **§5.137** — a
    body an operator authors then `body action reload`s leaves the DRAWN universe while
    `search`/`get status object` keep answering for it (your tester's `06old.sts` authors ~~3000~~ **170** [corrected 2026-09-06 at F98's acceptance, §11.218(i); `06.sts` is the 1013-body show]
    satellites this way); the repair is inside §11.55(i)'s suspended question ("keep current
    state" across a reload). (7) **Routed to the main tester, not asked:** the shipped `basis/`
    shows all carry `script action pause` and `flag_skip_pause = false` — under UNATTENDED
    play, honour or skip? (old-behaviour intent, §11.161(c)).
  - **VETO POINTS taken this session (implemented-and-live, each cheap to reverse; silence =
    endorsed):** (a) **F93's gate repairs** — `b4_anchors`' max-over-window control REMOVED
    for a lit-pixel count (K = 4, 10× under the weakest measured ratio), a NEW assert that
    the old path's distance to its anchor equals the authored radius (1.0 km, derived), the
    scene's twinkle OFF via the farm copy with P0 asserting it; `b24_select`'s English-label
    matcher replaced by a shape matcher. The reference is GREEN on both gates and the
    corrupt build REDS (nine P1b) — the direction the gate had backwards for six weeks.
    (b) **F87's fourteen wraps** (`ModularObject.cpp`, old's msgids byte-exact, code
    `1d839b9d`) — the ONLY engine change this round. (c) **One paragraph in
    `doc/developer-entry.md` §5**: the newcomer's 90-second smoke suite
    (`f90_rehearsal_run.sh`). (d) **The executor definition's footer line** (this close,
    `agents/opus-xhigh.md:89`): it named `Claude Fable 5` as the standing co-author, so
    three executor commits this round carry `Fable 5` and three `Fable 5.1` while the AUTHOR
    is `Claude Opus 5` throughout; now it says: the co-author is the SUPERVISING session's
    identity as the dispatch prompt names it (the projection regenerated, md5 asserted). No
    SHA amended. (e) **Scratch trees KEPT**: `/home/claude/sc-f89/` 1.5 GB (three binaries +
    a detached code worktree at `a2fd3c5b` in the pre-fix source state — `git worktree
    remove --force` disposes of it), `sc-f93/` 53 MB, `sc-f90/` ~30 MB, plus session 22's
    `sc-f84/` 4.2 GB and `sc-f86/` 1.3 GB — say the word to remove. (f) **F91 carried**
    (§5.86+§5.19, the RA/DE fix — decision-free since R27; the reason is in the picks line).
  - **HELD OPEN, not absorbed:** **T5.2 — the multi-hour soak is now the ONLY member of the
    map's "ready" gate** (T5.1 ran: three identical runs + mine); it needs a design under the
    no-background rule (foreground polls, `f19_stall.sh` as the hang detector) — next round's
    candidate, sized M–L, display-bound, hours. Instrument residues, each one line and owed:
    `supervised-by.sh`'s SILENT fallback on a missing branch ref (B1 — after the rename, 695
    trailers take it; a history-rewriting tool, yours to authorize); `f85_links.py`'s
    `IsADirectoryError` masquerading as "dangling found" (exit 1 both ways); `f89_p7.py
    margins` reads pre-F93 artifacts only; `b4_anchors_run.sh` still has no
    concurrent-instance probe; `b4_anchors.py` duplicates `dumpread.py`'s grammar.
  - **FACTS, no decision asked:** **the P7 red was the PRODUCT's** — the pre-fix (shipped-
    class) binary composed its star field for a sky 76.46° from its bodies after one
    `camera action switch` onto an orbit anchor (§5.133's use-after-free read: the old
    observer 237 022 km off, `Moon.old.dist` 182 582.8 vs authored 200 000.0), the gate was
    green on that and red on the corrected binary; "silent on the shipped build" is
    corrected at three homes — not crashing ≠ silent; a use-after-free read is only as
    visible as the allocator makes it (four anchor legs stay green on the corrupt build) ·
    twinkle costs ~2200 px of A/A floor and made the gate a coin toss · the translation
    channel is a frozen field `.txt` with identity fallback, NOT gettext — no `.po` exists;
    i18n work copies msgids or changes `spacecrafter-data` · the quit verb is `shutdown
    action now` · `ScriptAnnotator::flush` rewrites the played `.sts` IN PLACE — any farm
    that plays a shipped show must copy it · the rename's blast radius is four lines and
    cannot be re-audited by grep afterwards (`main` occurs 676/2040 times already) ·
    **seventeen dispatcher defects this round, all mine, all output-side, all caught via
    report-not-absorb**: fourteen of Q-67's class (numbers, counts, times, pointers from
    memory of a listing — incl. one estimate WEARING the measured label, corrected one
    commit later) and three of a SIBLING class now named at Q-67: structure asserted from
    convention rather than extracted (the `.po`, the "githooks trailer check", the farm's
    "symlinked `config.ini`") — prevention: a grep/read of the named file in the same
    command as the claim, for structure as for numbers.

- **THE RENAME, WHEN YOU WANT IT: ELEVEN ORDERED ACTS AND FOUR LINES TO EDIT
  (session 23, F92 → §11.212; nothing here renames anything).** Your line —
  *"The master-beta will became the reference and get renamed main once ready"* —
  answers **R6**'s policy half (rename, not redirect; the map's row is annotated,
  not struck). The footprint is measured: **226 occurrences of `master-beta` in the
  two repositories = 4 LIVE POINTERS · 133 PINS · 89 HISTORICAL RECORDS · 0
  CONVENTION.** The live four are `doc/developer-entry.md` §1, `CLAUDE.md:8` and
  `agents/opus-xhigh.md:33`+`:88`; they are in
  `harness/artifacts/f92/rename-live-pointers.patch`, which `git apply --check`s rc 0
  in a scratch worktree of each repo and leaves LIVE 0 when applied there. **The
  checklist is §11.212(h)**, each act with a check that can fail. **ONE SENTENCE IS
  YOURS AND THE LINE DOES NOT ANSWER IT: after the rename, do pull requests still
  target `2023-master`?** (the entry document says they do; the patch changes the
  branch's name and leaves that half alone). Two riders, both cheap and both listed
  not done: `supervised-by.sh:236-238` falls back to `HEAD` in silence when a
  trailer's branch is gone (measured: the fallback flips a verdict), and
  `f75_anchors.py:442`/`f80_ssgrammar.py:46` spell the branch out in the pins they
  WRITE. And one correction to a thing the dispatch believed: **`githooks/pre-commit`
  is not a trailer check** — it refuses derivations by content and knows nothing of
  branches, so the rename cannot break it.

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
