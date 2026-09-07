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

**Update [Claude Fable 5.1 2026-09-06, supervising session 25 — LovelyFoxDev, the
OFFSET-FRAME / LOCATION-ORBIT / SECOND-SOAK round: F96 · F97 · F98]:** trigger = the §0b
verbatim line and nothing else (no in-line transmission). Open at **Sunday 10:20 CEST**,
outside the owner's reliable window ⇒ this session asks NOTHING and closes with a compact
decision list. Warm-up (every value `date`-measured 10:20:38–10:21:53, the command beside
the claim — Q-67): both trees CLEAN at open, code `a2a880ef` / harness `fa7104e` (the
session-24 close commit); definition-drift assert MATCH (`8e364a3a`); binary `404b9e89`
current — `cmake -n` 0 steps, no `src/` file newer; same boot as sessions 21–24 (`uptime
-s` 2026-09-04 18:45:08); `:2` 2448x1332 under `.5KBYU3`; canary `--no-scene` **exit 0**
(30 members, artifacts `f56/canary/20260906-102151`); config/ssystem md5 pristine
(`03fbee59`/`545a51ef`); no `spacecrafter` in `/proc/*/comm`; RAM **51 GiB available of
59**, `-j24`; next free §11 **216** (live ∪ archive, `max+1`); unpushed **90 code / 736
harness** (`rev-list --count`; 735 + the close commit). Live `### F` **3 → 0** by
**archival pass 17** at OPEN (update-s23 + F91/F94/F95, 649 lines + the doubled-seam
tidy, manifest `2026-09-06-pass17`, pre-md5 `7fcb0abd` reproduced in-process AND from
disk, the archive files written before the live surface — Q-56's ordering; commit
`5d94d3f`; **nothing carried**) **→ 3** by the mints below. Instrument baselines at open
(run 10:28:5x): scan **226/279/135** · pair-check **231/206/25/110** · D 35 · D2 11 · I 89
· I2 36 · M 81 — to the digit of the session-24 close. QUEUE CONSUMPTION (session-24
close, in order): (1) pass 17 — DONE; (2) **the second soak over `fscripts/` → F98**
(M–L); (3) **§5.138's leg + the §5.86 pole-guard rider → F96** (M — widened from the
queue's S: the leg's fix is F91's own shape one member over, decision-free by old-parity
and by the row's family, so the leg and the fix travel together, the fix CONDITIONAL on
the leg confirming the model); (4) the §11.207(g) tail's next item **§5.21 → F97** (M);
NOT minted from that tail, each with its reason: §5.66+§5.71 (each still owes a
meaning/feel answer the round-3 replies did not give — §5.66's landing, §5.71's duration
law); §5.115 (R20's "8 launches" needs a per-launch key the per-day file layout lacks —
a layout the field's tooling and scedit read, so the owner sees the shape before it
ships); A15's residual (a photometric tuning pass without the eye that judges it); (5)
the 8–12 h RSS leg — owner's word only; (6) instrument residues, (7) riders, (8) owner
items — carried. **TWO DISPATCHER-SIDE FINDINGS AT THE MINT, both measured before any
number entered a section:** (a) **`06old.sts` authors 170 satellites, not 3000**
(`LC_ALL=C /usr/bin/grep -c 'body action load'` = 170; 359 lines; no `struct loop`) —
the number is the F90 executor's (§11.211(h)), written without its command (Q-67's
class on the executor side) and propagated to §5.137 ("thousands of … lines"),
§11.215(m), `DEPLOYMENT-MAP.md:687` and this file's session-24 §3 items; ~~the corpus's
largest authoring show is `14.sts` at 529 lines, the total 719 lines in 9 shows~~ **[CORRECTED at F98's acceptance, §11.218(i): my census pattern assumed the command's word order — `06.sts` writes `body name "X" … action load` 1013 times; measured both ways, 1719 authored bodies in 8 shows (06.sts 1013 · 14.sts 528 · 06old.sts 170 …); the 170 stands, the rest were LINE counts of one order — dispatcher defect 4, the sibling class inside a measured number]**. F98
corrects the four ledger homes with markers at both homes; this file's two are
corrected at the close. (b) **a concatenation fingerprint is locale-dependent** — `cat
$(ls *.sts)` and `cat *.sts` over the same 137 files hashed `68c9b4ba` vs `e2123d2b`
under `fr_FR.UTF-8`, because `ls` and the shell glob collate differently (the writer
set enumerated FIRST: newest `fscripts/` mtime 2026-07-11, no process); F98's premise
carries a sorted per-file digest (`3995e501`) and the driver keeps per-file md5s.
**All three mints PASS `premise_check.py` at the mint event (F96 21/21, F97 22/22, F98
26/26, run 10:41:30 — and this sentence first carried three figures typed BEFORE the
run, 24/21/25: caught by pasting the instrument's own output, tallied as dispatcher
defect 1 of this round, Q-67's class at its smallest).** Picks: **F96 → F97 → F98** (the soak LAST so it soaks
the binary the round leaves; three executors, §0b.2's sweet spot). Deliveries: all to
the parent (§11.216+, refreshed at each dispatch). Launch classes: F96 PHOTOMETRIC for
ONE full canary post-fix (the atmosphere's light direction rides the changed
authority) + FUNCTIONAL readout legs; F97 FUNCTIONAL (farm, dual dump, `--no-scene`);
F98 FUNCTIONAL (the soak — hours on `:2`, no photometric claim). Remotes: local contains
origin on both; push impossible here — the owner's push is R5, unchanged.
**Round outcome (session 25 close, 2026-09-06 — every time in this note is pasted `date` output;
the close commit's own clock is the stamp):** F96 → **§11.216** + §5.138 FIXED + the §5.86 pole
rider closed + **§5.139** minted at acceptance (ONE inverse of the render rotation; alt/az, the
atmosphere's sun direction, the descent and the tracking aim are old's with the offset armed —
27.000007° → 0.000017°; the config key a LATENT offset; the zero-vector guard question) · F97 →
**§11.217 IN PART** + §5.21 PARTLY FIXED (the latitude landed; the STOP fired at the derivation:
`surface_point`'s `orbit_lon` ninety degrees east → **§5.140**; the loader's null parent →
**§5.141**; a third defect named, 86.306371° on Mars) · F98 → **§11.218** + **§5.142 / §5.143 /
§5.144** minted at acceptance (4.508 h, 8 complete cycles, 541 samples, no FAIL flag; two of the
tester's shows abort the app; one runtime system empties the old half; LEAK 68.3 MB/h; §5.115 at
193 MB/h; the four "3000" homes corrected both ways) — **three for three delivered AND
supervisor-verified same session**, every delivery re-verified by my own runs (the smoke suite on
`eb3f5e50` and on `46849f69`; the F96 leg reports and guard answers read; the F97 scores read
from the committed JSON; the F98 abort arm C, its negative arm and the old-path bisect
reproduced, `f98_repro14.py`, 18:30–18:41). Code `a2a880ef → 24100461 → 22499f04` (two executor
commits: `Camera.{hpp,cpp}`; `orbit.cpp` one token + two loader files — nothing else); binary
`404b9e89 → eb3f5e50 → 46849f69`; harness `fa7104e →` this close. SUPERVISOR ACTS: archival pass
17 (`5d94d3f`); three mints under the PREMISES rule (`b111c87`; 21 / 22 / 26 PASS at the mint);
**six §5 mints at acceptances** (§5.139 – §5.144, all record-only, the F32/§5.79 precedent);
F97's and F98's per-round premises refreshed by section-bounded replace with content asserts
(`7d6395a`, `68efa44`); the §0b.1 ledger-instruments line (the miss-ledger item, two
recurrences); `f45_run.sh:102/186` annotated (Q-69); in `~/shared`: Q-67 ×3 (a dispatcher
instance, a sibling-class instance, the census-order instance), Q-68 (the tool's 120 s default
timeout), **Q-69 NEW** (locale-dependent concatenation fingerprints, sweep done), one
miss-ledger line. OWNER EVENTS IN-SESSION: none — the trigger line only; no question asked
(Sunday). HOST: same boot throughout (`uptime -s` 2026-09-04 18:45:08); RAM 23 GiB available of 59 at the
close; `:2` unlocked on all 541 soak samples; no HOST-EVENTS entry owed. SUPERVISOR TALLY: **six
dispatcher defects**, all output-side, all corrected at their nodes — Q-67's class ×3 (the
premise counts typed before the run, caught by pasting the instrument; the map line `:687`
for a sentence at `:707`; §11.213(i2)'s measure-zero premise carried from my F91-acceptance
disposal) and its SIBLING class ×3 (structure asserted without the read: "one used" at
`protosystem.cpp:600`; the census pattern modelling one of the command's two word orders — the
class hiding INSIDE a measured number, a PREMISES line pinning the pattern's blind spot with
the count; the byte-identical-`plan` requirement inconsistent with the design's own change);
plus instrument slips of my own hand, none in a record: one ugrep pattern past its complexity
limit, the F97 result's structure guessed twice before it was listed. EXECUTOR REPORT DEFECTS:
none this round. EXECUTOR criterion-integrity instances: **≥ 18** (F96: the leg re-taken with
the addendum committed before the re-run; the freshness partition independent of the model;
both mutations refuted; the offset-0 identity claimed only where a same-binary control licensed
it; the measure-zero premise refuted on a shipped command. F97: the frame derived before one
byte of code; the STOP at its own clause with both readings; only the separable half landed;
three refuted predictions kept as refuted; the probe's Mars corrected by the live leg. F98: the
criteria, cap and model table before any process; the shakedown; the DEATH control finding the
instrument's own label defect; CAPPED / SHOW-TIMEOUT / INTERRUPTED discriminated; R1 refuted by
its own instrument; the baseline re-measured from a `git archive`; the uncredited lists diffed
with `comm`; the census taken both ways). BASELINES AT CLOSE (measured 18:47:01): scan
**234/289/135** · pair-check **234/209/25/116** · D 35 · D2 11 · I 89 · **I2 37 · M 83** — over the
open (226/279/135 · 231/206/25/110 · I2 36 · M 81): +8 raw event lines and +10 pairs (§11.216's
four markers, §11.218's four spans; uncredited 135 → 133 → 135: F97 paid two §5.21 arrears,
§11.218's two citation-shaped candidates disposed at its (q)); +3 entry files / +3 live pairs
(the three entries); +6 inline stubs (§5.139 – §5.144); I2 +1 and M +2 are §11.218's entry-only
markers at §11.211/§11.215, licensed by the section's own condition (neither stub carries the
number, measured). Archival pass 18 (update-s24 + F96/F97/F98, live `### F` 3 → 0) DEFERRED to
the next open. NEXT-ROUND QUEUE, in order: (1) archival pass 18 at open; (2) **§5.141's fix** (S,
decision-free: the sibling's guard mirrored + the both-ways proof in §5.50's shape); (3)
**§5.139's mechanism leg** (S–M, `ModularBody`'s update walk to the pruner; the fix decision-free
once confirmed); (4) **§5.143's mechanism leg** (S: `14.sts` line-bisected to the `parent none`
load) + `bodies_old > 0` asserts in every dual-dump instrument; (5) **§5.142's reading** (S: the
uniform pool's bound and the per-body cost at `BufferMgr`, and `06.sts` alone for the first-error
body count — the policy stays the owner's); (6) ON THE OWNER'S WORD: the `orbit_lon` ruling →
§5.21's two halves + §5.140's fix (M); the 8–12 h cache-vs-leak leg (§5.144); the pool policy
(§5.142); (7) the (g) tail: §5.115's retention design shown first, §5.66+§5.71 (feel/meaning
owed), A15's residual; (8) instrument residues: `supervised-by.sh` B1 (owner-authorized),
`f85_links.py` guard, `f89_p7.py margins`, the b4 `/proc` probe, the `dumpread.py` duplicate,
the pinned-clock one-parameter experiment (dump later than 1.5 s after the jump, §11.218(l));
(9) riders: `observedToBodyLocalPos` with no consumer (owner), the `[parallel-script]`
question, scedit README `:43`, the tester's `panorama5.sts:102`; (10) owner items per §3.
Remotes: **92 code / 760 harness** unpushed before this close's commit (measured 18:47:01); push
from a keyed host — the supervisor never pushes.

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
19 (update-s25 + F99–F102) DEFERRED to the next open.**

---

### F99 — §5.141's fix: `LocationOrbitLoader` guards the parent it dereferences — the sibling's shape (`findBodyOnce` + a §2(f) line at the anchor naming what, consequence, fix; `return nullptr`, never throw) with "no orbit, no body" served by the consumer's EXISTING refusal (`ModularSystem.cpp:1245-1247`); the row's reach arm CORRECTED by the read and then MEASURED: `loadBody` refuses an UNKNOWN parent name before any loader runs (`:1067-1071`), so the arm that reaches the dereference is `parent none` (`loadBody` substitutes the system for `none`, `:1064-1067`; the loader looks up the literal `"none"`) — and on that arm the OLD path's own `location_orbit` branch dereferences the same null parent FIRST (`protosystem.cpp:598-599`, called before the new path at `ssystem_factory.cpp:829`/`:836`), so the guard lands at BOTH sites in the §5.50 shape, the both-ways proof pre SIGSEGV / post rc 0 + the lines, the same-class sweep over the nine orbit loaders recorded [S, engine, both paths — the old-path half a crash guard under the §5.50/§11.124(h) precedent, veto point §3]

**Why now / mandate:** §5.141 [observed 2026-09-06, §11.217(h1) (F97); minted at F97's
acceptance, record-only; session-25 close queue position 2, "S, decision-free"]:
`LocationOrbitLoader::load` looks the parent up (`orbitModules/LocationOrbitLoader.hpp:20`,
`ModularBody::findBody` — nullptr on a miss, `ModularBody.hpp:1558-1567`) and dereferences
it at `:47-49`; the sibling `SurfacePointOrbitLoader` guards the same miss
(`SurfacePointOrbitLoader.hpp:144-150`: `findBodyOnce` + a `L_WARNING` naming the fix);
§11.124(h) guarded the same class at `protosystem.cpp` for the old path (§5.50: "no orbit,
no body"). **Two things the mint READ that the row does not say — both premises of the
both-ways proof, both to be MEASURED, never inherited:** (a) the row's headline arm,
`parent <unknown>`, does not reach the loader through `body action load`:
`ModularSystem::loadBody` resolves the parent NAME first and refuses a miss with its own
line before `loadOrbit` runs (`ModularSystem.cpp:1067-1071`, then `:1200`), and the old
path refuses the same push the same way (`protosystem.cpp:531-536`); the arm that DOES
reach the dereference is **`parent none`** — `loadBody` substitutes the system itself for
`none` (`:1064-1067`) and proceeds, while the loader looks up the literal `"none"` and gets
nullptr. (b) On that arm the OLD path's `location_orbit` branch (`protosystem.cpp:598-608`)
dereferences a null `parent` as well — `parent` stays a null `shared_ptr` when
`str_parent == "none"` (`:531`) — and old's `addBody` runs BEFORE the new path's `loadBody`
for one push (`ssystem_factory.cpp:829`, then `:836`), so the pre-fix crash is PREDICTED to
be old's `:599`, the new loader's dereference masked behind it until old is guarded. Two
sites, one class, one function each; the old-path change is the §5.50 precedent exactly (a
crash guard at the same function; no shipped or loaded scene reaches the class — census
`location_orbit` 0/0/0, §11.217(a)) and is said as a veto point in §3.

**Measured at dispatch (supervisor, 2026-09-06 21:25–21:41, code `22499f04`; the PREMISES
block re-runs what is a command):** the nine registered orbit loaders `modules.cpp:45-53`
(eight keyed + the default `SpecialOrbitLoader`). Parent handling as READ (the task
re-reads and states each): `ElipticOrbitLoader.hpp:3,8` and `CometOrbitLoader.hpp:3,9` —
`findBody` then `if(!parent)` falling back to the `parent_rot_*` keys (guarded; a
different behaviour, recorded not judged); `BaryOrbitLoader.hpp:11-16` — `findBodyOnce`
×2, a log + `return nullptr` on a miss (guarded); `SurfacePointOrbitLoader.hpp:144-150` —
guarded, proceeds (its orbit tolerates a null parent, `:114-115`);
`LocationOrbitLoader.hpp:20,47-49` — UNGUARDED; `Still`/`Earth`/`Lunar`/`Special`: whether
they touch `parent` at all is the task's to read. The consumer chain:
`ModuleLoaderMgr::loadOrbit` (`ModuleLoaderMgr.cpp:101-109`) wraps the keyed loader in a
`catch (...)` that falls through to the DEFAULT loader — a loader that THROWS is silently
replaced by a `SpecialOrbit` (D12's opposite), which is why the guard must RETURN nullptr;
`ModularSystem::loadBody:1245-1247` refuses a null orbit with *"Invalid orbit '…' for body
'…', skip loading this body."* (`L_ERROR`) — the consequence line already exists at the
consumer; `CameraAnchors::createAnchorBody` `:149-150` is the other `loadOrbit` caller and
checks `!orbit` itself — whether its `orbitParams["parent"]` is validated before the call is
for the task to read (`anchor.ini`'s grammar, §11.200). Instruments: `f90_rehearsal_run.sh`
(the smoke suite), `f91_parity.py` + `f91_run.sh` (the 90-body table — the
no-shipped-body-moves control, `c125adf0` fr at `46849f69`, §11.217(f)), `dumpread.py`,
`b3_farm.sh`.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f99/prediction.txt`, committed before any
launch): P1 arm A (`… coord_func location_orbit parent Nonexistent …`) → NO crash pre-fix,
both paths' refusal lines (old `:534`, new `:1070`), the body on neither dump half; P2 arm B
(`… parent none …`) → SIGSEGV pre-fix, attributed to OLD's `:599` as the first dereference in
call order — the applog's last lines and (under gdb, §0's sediment) the faulting frame say
WHICH site, not the reading; P3 post-fix arm B → alive, rc 0 at `shutdown action now`, ONE
§2(f) line per path naming what (a `location_orbit` needs a parent body; `none` is not one),
consequence (the body is not created on that path), fix (declare `parent = <body>`), the
body on neither half, a `still_orbit` control body from the same script on BOTH halves; P4
the F91 table byte-identical pre/post (fr) and the smoke suite rc 0 (census 0/0/0 ⇒ no
shipped body can move); P5 the `CameraAnchors` arm: reachable or pre-validated, from the
read. (2) **THE LEG, pre-fix** (`/home/claude/sc-f99/bin/spacecrafter-pre` preserved at
`46849f69`; farm, French locale): arms A and B on fresh launches — the rc, the applog tail,
the dump, the frame. (3) **THE FIX:** `LocationOrbitLoader.hpp` — `findBodyOnce` (the
sibling's call), null ⇒ the `L_ERROR` line at the anchor (§11.193: the loader is the one site
holding the coord_func AND the parent name) and `return nullptr` (never throw — see the
`catch (...)` above); `protosystem.cpp`'s `location_orbit` branch — `if (!parent)` ⇒ the same
line's old-path twin and `return`, BEFORE `:599` (the §5.50 shape in the same function);
comments cite the entry; nothing else. (4) **PROVE:** P1–P5 measured post-fix; the
nine-loader sweep as a table in the entry; D14. (5) **RECORD:** §11.⟨next⟩ FIRST + stub;
§5.141 FIXED with the arm CORRECTED at the row (the `<unknown>` arm refuted by `:1067-1071`,
the `none` arm measured, the old-path site added — a marker, this task's); §5.50 annotated (a
second site of its class guarded in the same function); §11.217(h1) back-marker both homes;
DEPLOYMENT-MAP (`grep -n '5\.141'` = 0 at the mint — say so if still 0); README section; WIP
per §0.6; D14.

**Boundaries:** `src/experimentalModule/orbitModules/LocationOrbitLoader.hpp` +
`src/bodyModule/protosystem.cpp` (the `location_orbit` branch ONLY — an OLD-PATH crash
guard under the §5.50/§11.124(h) precedent, veto point §3); nothing else; no data; no msgid;
the farm only (real HOME md5 in==out); FUNCTIONAL (`--no-scene`); no `run_in_background`;
runs under `/home/claude/sc-f99/`. If arm B does NOT crash pre-fix on either path (P2
refuted) → STOP before the fix: record the measured behaviour, re-derive the reach, report —
the guard is still the sibling's shape and may land, but the row's reach claim is then the
thing to correct, and that is the dispatcher's call.

**Discriminating checks:** (a) P2 red pre-fix (the signal / rc) and green post-fix (rc 0)
on the same script; (b) the pre-fix crash attributed to a SITE from the log/frame, not from
the reading; (c) P1 unchanged pre/post (two refusal lines, no crash); (d) the control body on
both halves post-fix while the `none`-parent body is on neither; (e) the F91 table
byte-identical + the smoke suite rc 0; (f) the nine-loader sweep table; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that
are not commands: display per HOST-EVENTS (`:2`, the owner's RDP-created real session,
alive under ssh — `xdpyinfo` 2448x1332 at 21:24); canary `--no-scene` exit 0 before the
first launch; `free -g` ≥ 16 GiB before the build (52 GiB at open ⇒ `-j24`); the harness
HEAD as the prompt states it; §5.141's row reads OPEN, record-only.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 22499f04
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 46849f69
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 219
grep -c '^### F' claude/fable-dispatch.md => 4
# ledger states the work stands on
grep -m1 '^141\. ' claude/INTENT.md | grep -c 'OPEN, record-only' => 1
grep -m1 '^141\. ' claude/INTENT.md | grep -c 'FIXED' => 0
grep -m1 '^50\. ' claude/INTENT.md | grep -c 'FIXED 2026-07-31' => 1
# sites, re-resolved at HEAD (content drift = abort)
sed -n '20p' src/experimentalModule/orbitModules/LocationOrbitLoader.hpp | grep -c 'ModularBody::findBody(params\["parent"\])' => 1
sed -n '47,49p' src/experimentalModule/orbitModules/LocationOrbitLoader.hpp | grep -c 'parent..get' => 3
sed -n '144,145p' src/experimentalModule/orbitModules/SurfacePointOrbitLoader.hpp | grep -c 'findBodyOnce\|if (!parent)' => 2
sed -n '1064,1071p' src/experimentalModule/ModularSystem.cpp | grep -c 'parentName != "none"\|Can.t find parent\|return;' => 3
sed -n '1245,1247p' src/experimentalModule/ModularSystem.cpp | grep -c 'if (!createInfo.orbit)\|skip loading this body' => 2
sed -n '531,536p' src/bodyModule/protosystem.cpp | grep -c 'str_parent != "none"\|can.t find parent' => 2
sed -n '598,599p' src/bodyModule/protosystem.cpp | grep -c 'location_orbit\|parent..getSiderealDay' => 2
sed -n '829p;836p' src/bodyModule/ssystem_factory.cpp | grep -c 'currentSystem..addBody(param)\|loadBody(param, nullptr, true)' => 2
sed -n '101,109p' src/experimentalModule/ModuleLoaderMgr.cpp | grep -c 'catch (...)\|defaultOrbitLoader..load' => 2
sed -n '149,150p' src/experimentalModule/CameraAnchors.cpp | grep -c 'loadOrbit(orbitParams)\|if (!orbit)' => 2
sed -n '11,13p' src/experimentalModule/orbitModules/BaryOrbitLoader.hpp | grep -c 'findBodyOnce\|== nullptr' => 3
grep -c 'registerModule("' src/experimentalModule/modules.cpp => 8
# the census (LC_ALL=C /usr/bin/grep: the wrapper skips non-UTF-8 files silently)
/usr/bin/grep -rl 'location_orbit' ~/.spacecrafter/ssystem.ini ~/.spacecrafter/scripts/ 2>/dev/null | wc -l => 0
LC_ALL=C /usr/bin/grep -c 'location_orbit' doc/superscript.sts => 0
# instruments
test -f claude/harness/f90_rehearsal_run.sh && test -f claude/harness/f91_parity.py && test -f claude/harness/f91_run.sh && test -f claude/harness/dumpread.py && test -f claude/harness/b3_farm.sh && echo ok => ok
test -e /home/claude/sc-f99 ; echo $? => 1
```

**DoD:** predictions before the launch; the pre-fix leg (both arms, the site attribution);
the fix at both sites (code first); the post-fix proofs; the sweep; §11 entry + stub; §5.141
flipped with the arm correction; §5.50 annotated; back-markers; map; README; trees clean;
WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-06 → **§11.219** (entry + stub). **§5.141 FIXED at BOTH sites**,
its headline arm and its site count corrected at the row: `parent <unknown>` is refused
before any loader runs (`ModularSystem.cpp:1067-1071`), the reaching arm is **`parent none`**
— silent on both paths — and the FIRST dereference is the OLD path's `protosystem.cpp:599`
(`ssystem_factory.cpp:829` before `:836`), which was masking the row's own site. Code
`22499f04` → **`1af7fa48`** (the two named files only), binary `46849f69` → **`8e2c6ef3`**;
harness `ad067a5` → `adbc496` → `ae6bcb5` → the record. Gate 24/24; predictions committed
before the first launch; pre-fix **14/14** (rc −11, gdb frame) / post-fix **15/15** (rc 0, one
§2(f) line per path, `"Loading body ProbeLocB"` **0 → 1** — the second site reachable only
once the first stopped dying); sweep **1 of 9 → 0 of 9** by instrument, both trees; F91 table
byte-identical at `c125adf0` (reproduced on the pre binary first), smoke suite rc 0, D14 PASS,
frozen pair in==out on all EIGHT launches ["eleven" here was the THIRD home of the number the executor corrected at the entry and README — aligned at acceptance]. §5.50 annotated (second member, same function,
above its own guard); §11.217(h1) back-marked at both homes; README §F99; `DEPLOYMENT-MAP`
`grep -c '5\.141'` = 0, unchanged. The STOP clause did not fire.
**ACCEPTED 2026-09-06 — the verifying commands' `date` read 22:23:50–22:28:05 (supervisor, session
26, Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.219 read in full;
ONE code commit `1af7fa48` (Claude Opus 5; `protosystem.cpp` + `LocationOrbitLoader.hpp`, 48+/1−,
the diff READ: one `if (!parent)` guard per site, an `L_ERROR` at each anchor naming what /
consequence / fix, `return` and `return nullptr`, `findBodyOnce` — nothing else) and six harness
commits `ad067a5 → 9c662bb` (Claude Opus 5; the predictions at `ad067a5` before the first launch at
`adbc496`), both trees clean; binary `8e2c6ef3`, dry build 0 steps; §5.141 reads *FIXED 2026-09-06
(F99, §11.219, code `1af7fa48`), at BOTH sites*; §5.50 annotated a second time; §11.217(h1) names
§11.219 in the entry file AND the stub; README §F99 (`:4745`); `DEPLOYMENT-MAP` 0 hits, said not
assumed; instruments to the digit of the entry's (l) close (scan 236/293/136 · pair-check
235/210/25/116 · D 35 · D2 11 · I 89 · I2 37 · M 84 — +1 entry file/+1 live pair the entry, +2 raw
lines the two new markers, the one uncredited pair disposed as citation-shaped at (l), M +1 the
entry-only citations licensed by the header's entry-wins rule). **AND arm B run by my own hand on
BOTH binaries with the delivered runner (`/home/claude/sc-f99/supervisor/{pre,post}`, 22:24:31 and
22:25:26): pre `46849f69` — 14/14, `"rc": -11` recorded, the log ending at `Loading new Stellar
System object... ProbeLocB` and nothing after; post `8e2c6ef3` — 15/15, rc 0, ProbeLocB on neither
half with the `parent none` + `still_orbit` control on BOTH; AND the smoke suite on `8e2c6ef3`
(22:26:26–22:28:05): rc 0, the nine step states of §11.211 (S1/S7 DIVERGENCE with citations, S4
DEPRECATED, six PASS), S5's shape OK, exit 0 in 0.7 s, 2 frame stalls, frozen 4/4 in==out, `/proc`
clear after every run.** Deviations ENDORSED with the executor's arguments: `findBodyOnce` (the
sibling's call, inert by the 0/0/0 census, the byte-identical F91 table the measurement); the
old-path guard INSIDE the branch rather than after the block (the fault precedes the block's end —
an after-the-block guard is inert there); its own §2(f) wording rather than §5.50's (the
coord_func is valid, the declaration incomplete); the lock-file guard recorded per leg (a reused
pid would read as a result); the stub aligned to its entry (test I 90 → 89, the omission shown
load-bearing). DISPATCHER-SIDE FINDING reported, ACCEPTED as mine with the counterfactual: the
section's headline called the old-path half "a crash guard in the §5.50 shape" while §11.124(h)
defines that shape by PLACEMENT (after the whole block) — a label asserted without re-reading its
definition (Q-67's SIBLING class); the mandate's own words placed it correctly, and had a reader
followed the headline the guard would have been inert and P2 post-fix would have stayed red — the
check structure covered it. Round tally: **two dispatcher defects** (the keyed-loader count at the
mint; the "§5.50 shape" label). EXECUTOR Q-67 instance, self-caught before delivery: "eleven
launches" typed from the campaign's shape, eight counted from the pid lines — corrected at the
entry and README, its THIRD home (this WIP line) aligned by me above. STANDING CONSEQUENCES:
**`parent none` on a `location_orbit` is refused in words on both paths** (pre-fix it was a silent
SIGSEGV on the OLD path, which masked the new path's identical dereference — `"Loading body
ProbeLocB"` 0 → 1); `parent <unknown>` never reached either loader and still does not;
`f99_sweep.py` audits the nine orbit loaders' parent handling on any tree in a second (UNGUARDED
1/9 → 0/9; the old chain read by hand, no third site); the topology datum for every push-channel
defect: `SSystemFactory::addBody` feeds OLD first, so the first path to die hides the second;
`/home/claude/sc-f99/` holds the farms, the pre-fix binary (`bin/spacecrafter-pre`, `46849f69`)
and my runs.

### F100 — §5.139's mechanism leg, then the fix INSIDE the barrier's own contract: the frozen readouts are the PARKED bodies' (`hidden = true` in the field's `ssystem.ini` — every dwarf planet and asteroid of the frozen set — and their subtrees) at a PINNED clock: `useNow()`'s memo `evaluatedJD == currentJD` (`ModularBody.cpp:459`) is the right key for the orbit position and the WRONG key for the camera-dependent `mat`, so after the first use at a given date no camera move ever reaches a parked body's eye-frame position again; the dump ALREADY calls `useNow()` per body (`ssystem_factory.cpp:1192`) and the SELECTION every frame (`ModularSystem.cpp:270`), which is why the instrument and the operator's `get status object` are predicted to freeze TOGETHER — the leg discriminates pinned vs running clock and parked vs walked bodies with predictions before the launch; the fix re-keys the memo on (date, parent frame) with the +4 iterations bound to a date change only, under D11's denominator; STOP if the leg refutes the memo [S–M, engine, new path]

**Why now / mandate:** §5.139 [measured 2026-09-06, §11.216(j1) (F96); minted at F96's
acceptance, record-only; session-25 close queue position 3, "the fix decision-free once
the mechanism is confirmed"]: after `select planet Jupiter` + `flag track_object on` at
offset 0, **48 of 120 records keep a byte-identical new-path `mat` translation** and sit
109.893°–170.695° from old's alt/az; the row's candidate was the walk's visibility gate
(`preUpdate`/`update`, the same gate §5.107 records), *"which pruner excludes them is the
unread part"*. **The mint READ the walk and the candidate does not survive it; a sharper
one does, and it is stated here as [derived] for the leg to confirm or refute:** every
body the walk REACHES gets its `mat` translation refreshed whether visible or not
(`selectiveUpdate`'s else branch `ModularBody.hpp:915-945`, `recursiveTranslationUpdate`
`:951-972`, `dispatchUpdate`'s else branch and its climb `ModularBody.cpp:382-407`,
`:408-449`). What the walk does NOT reach is (i) PARKED subtrees — `hiddenBodies`, for
which only `publishParkedFrame` runs (`ModularBody.hpp:1003-1007`; B39/D23: *"hidden bodies
shouldn't tick"*) — and (ii) nodes above the isolation stop (`isNotIsolated = false` at
`ModularSystem.cpp:196`; the climb `ModularBody.cpp:408`). F96's 48: the systems and
anchors (ii, `dist` 0 — §11.216(i)'s zero-vector class) and **exactly the field's hidden
bodies with their moons** (i): `eris ceres haumea arrokoth pluto makemake sedna vesta
pallas juno` carry `hidden = true` in `~/.spacecrafter/ssystem.ini`, `charon/nix/kerberos/
styx/hydra` sit under the hidden Pluto, `hiiaka/namaka` under the hidden Haumea (the census
line is a premise). The parked bodies' eye-frame position has ONE writer, the D8 barrier
`useNow()` (`ModularBody.cpp:455-482`: `recursiveTranslationUpdate(currentJD, frame)` 1+4
times from the parent's published frame) — and the barrier returns early when
`evaluatedJD == currentJD` (`:459`), a stamp written by the translation refresh itself
(`ModularBody.hpp:683`, `:857`) and compared to the frame date `dispatchUpdate` publishes
(`ModularBody.cpp:375`). That key is complete for `eclipticPos` (a function of the date)
and INCOMPLETE for `mat` (a function of the date AND the parent's flat frame, which follows
the camera): at a pinned clock the first use at that date refreshes `mat`, every later use
returns at `:459`, and the camera moves on without it. F96's dumps were taken at
`"timeSpeed":0` (the header; a premise line) — and the dump DOES call `nb->useNow()` for
every body (`ssystem_factory.cpp:1192`, *"A DUMP IS A USE"*), so the instrument was not
missing the barrier, the barrier was memoized. The SELECTION gets the same call every
frame (`ModularSystem.cpp:261-270`, *"Being the SELECTION is a use"*) on the same key ⇒ the
operator channel — pause or pin the clock, select a hidden dwarf (`S10.sts` selects a
hidden body, the code's own comment), move the camera, `get status object` — is predicted
to answer the OLD camera state too. The fix under this reading stays inside the barrier's
own contract (§11.76(b) [vixy]: *"As soon as the position is used … it should be
computed"*): the memo must also recognise a changed parent frame; the +4 re-convergence
iterations belong to a DATE change only (the seed is date-driven, `ModularBody.hpp:1015-1023`);
a frame-only change is one translation refresh. Decision-free because the contract is
recorded and the fix restores it; D11 prices it (the selection + the star are the per-frame
users, `:261-270`).

**Measured at dispatch (supervisor, 2026-09-06 21:25–21:41, code `22499f04`):** the sites
above, each a premise line; `getObservedPosition()` = `mat.getTranslation()`
(`ModularBody.hpp:1493-1495`) and its readout consumers `ModularObject.cpp:59,87,176,194,213`
(RA/DE, equatorial, alt/az — the strings `get status object` prints); the F96 leg artifacts
`harness/artifacts/f96/leg_pre/` (`f96_report.txt` line 3 names the 48; `cmd_p0_launch` →
`cmd_a0_off0` is the move at offset 0), the instrument `harness/f96_offset.py`
(`freshness()` at `:457`, `body_pos()` reads `mat[12..14]`), `f91_parity.py`/`f91_run.sh`
(the byte-identity control), `f90_rehearsal_run.sh`. The clock: F91/F96's drivers pin it
(`timerate rate 0` — the header's `timeSpeed 0`); a RUNNING clock is `timerate rate 1`
(state the command used).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f100/prediction.txt`, before any launch):
P1 at the pinned clock the frozen set of F96's move = parked subtrees ∪ the isolation
residue, body for body (the field census + the dump's `relation` field as the partition
key, INDEPENDENT of the freshness measurement); P2 the same move at a RUNNING clock leaves
0 parked bodies frozen (each frame's new date defeats the memo) while the isolation
residue stays at `dist` 0; P3 at the pinned clock `select planet Ceres` (hidden Dwarf) +
`get status object` before/after the move answers the SAME alt/az string after the move
(stale), while old's `altaz_old` for Ceres in the same dump moved — and at the running
clock it follows; P4 the walked control (`select planet Mars`) fresh at both clocks; P5
post-fix: 0 parked bodies frozen at the pinned clock, every parked body's alt/az within
F91's floor (≤ 3e-5°) of `altaz_old` and its RA/DE string old's, the WALKED set
byte-identical to pre-fix at every stage, the P0 launch-state table byte-identical; P6 the
MUTATION: re-keying on the date only (= no fix) leaves the 48; dropping the memo entirely
also passes P5 but pays 1+4 refreshes per use per frame — the chosen form's per-frame cost
for the selection + the star, measured or bounded against 1 ms/frame (D11). (2) **THE LEG,
pre-fix binary** (farm, French locale): (a) F96's move at the pinned clock, the freshness
partition joined to the parked/isolated partition — P1 to the body; (b) the same at the
running clock — P2; (c) P3 and P4 through the OPERATOR channel (`select` + `get status
object`, the string), at both clocks; every dump pair also read by `freshness()`. (3) **THE
FIX** (`ModularBody.{hpp,cpp}` only — `useNow()` and the stamp it keys on): the early return
compares the parent frame the last refresh used as well as the date (a stamp beside
`evaluatedJD`, or a by-value compare against `parent->parkedChildFrame` /
`matLocalToBodyPos` — say which and why); a date change keeps the 1+4 iterations; a
frame-only change runs the translation refresh once; the comment cites the entry and
§11.76(b). **If the leg refutes the candidate** (parked bodies frozen at the RUNNING clock
too, or walked bodies frozen, or the partition not matching P1) → STOP before the fix:
record the measured partition and what it implicates, deliver nothing in code, report. (4)
**PROVE post-fix:** P5 and P6; the F91 table byte-identical pre/post at the un-moved launch
state (fr), the smoke suite rc 0, canary `--no-scene`; D14. (5) **RECORD:** §11.⟨next⟩
FIRST + stub; §5.139 FIXED (or STOP-annotated with the measured partition); §5.107
annotated (its second member's mechanism is the barrier's memo, not the walk's gate);
§11.216(j1)(j3) back-markers both homes; §11.117 (B39, the barrier's origin) annotated at
the clause the memo contract changes; the map (grep `5.139`); README section; WIP per §0.6;
D14.

**Boundaries:** `src/experimentalModule/ModularBody.hpp` + `ModularBody.cpp` only (the
memo and its stamp); NO old-path change; no data; the farm only (real HOME md5 in==out);
FUNCTIONAL (`--no-scene`); no `run_in_background`; runs under `/home/claude/sc-f100/`.

**Discriminating checks:** (a) P1 body for body, the partition key independent of the
measurement; (b) P2 — the running clock un-freezes the parked set on the SAME binary; (c)
P3 through the operator's string, both clocks; (d) P5 pre/post with the walked set
byte-identical; (e) P6 both mutations refuted; (f) the F91 table + smoke suite; (g) the
D11 number; (h) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that
are not commands: display per HOST-EVENTS (`:2`); canary `--no-scene` exit 0 before the
first launch; `free -g` ≥ 16 GiB before the build; the harness HEAD as the prompt states
it; §5.139's row reads OPEN, record-only.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 1af7fa48
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 8e2c6ef3
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 220
grep -c '^### F' claude/fable-dispatch.md => 4
# ledger states the work stands on
grep -m1 '^139\. ' claude/INTENT.md | grep -c 'OPEN, record-only' => 1
grep -m1 '^139\. ' claude/INTENT.md | grep -c 'FIXED' => 0
# sites, re-resolved at HEAD (content drift = abort)
sed -n '457p;459p' src/experimentalModule/ModularBody.cpp | grep -c '!renderHidden || !parent\|evaluatedJD == currentJD' => 2
sed -n '479,480p' src/experimentalModule/ModularBody.cpp | grep -c 'RESUME_EXTRA_ITERATIONS\|recursiveTranslationUpdate(currentJD, frame)' => 2
sed -n '375p' src/experimentalModule/ModularBody.cpp | grep -c 'currentJD = jd' => 1
grep -n 'evaluatedJD = jd' src/experimentalModule/ModularBody.hpp | cut -d: -f1 | tr '\n' ' ' => 683 857
sed -n '1493,1494p' src/experimentalModule/ModularBody.hpp | grep -c 'getObservedPosition() const\|return mat.getTranslation()' => 2
sed -n '1003,1006p' src/experimentalModule/ModularBody.hpp | grep -c 'publishParkedFrame\|hiddenBodies.empty()\|parkedChildFrame = flat' => 3
sed -n '1192p' src/bodyModule/ssystem_factory.cpp | grep -c 'nb..useNow()' => 1
sed -n '269,270p' src/experimentalModule/ModularSystem.cpp | grep -c 'ModularBody::getSelected()\|sel..useNow()' => 2
sed -n '196p' src/experimentalModule/ModularSystem.cpp | grep -c 'isNotIsolated = false' => 1
sed -n '338p' src/experimentalModule/ModularBody.hpp | grep -c 'RESUME_EXTRA_ITERATIONS = 4' => 1
# the field census (the parked set) and F96's record
python3 -c "import re;t=open('/home/claude/.spacecrafter/ssystem.ini',encoding='latin-1').read();s=re.split(r'^\[([^\]]+)\]\s*$',t,flags=re.M);d={s[i].strip().lower():s[i+1] for i in range(1,len(s)-1,2)};f=lambda b:(lambda m:m.group(1) if m else '-')(re.search(r'^\s*hidden\s*=\s*(\S+)',d[b],re.M|re.I));print(' '.join(b+':'+f(b) for b in ['eris','ceres','haumea','arrokoth','pluto','makemake','sedna','vesta','pallas','juno','charon','mars']))" => eris:true ceres:true haumea:true arrokoth:true pluto:true makemake:true sedna:true vesta:true pallas:true juno:true charon:false mars:false
zcat claude/harness/artifacts/f96/leg_pre/cmd_a0_off0.json.gz | head -1 | grep -o '"timeSpeed":[^,]*,"timePaused":[^,]*' => "timeSpeed":0,"timePaused":false
grep -o '48 frozen / 72 re-evaluated of 120' claude/harness/artifacts/f96/leg_pre/f96_report.txt | head -1 => 48 frozen / 72 re-evaluated of 120
grep -c 'frozen: 51PegSystem' claude/harness/artifacts/f96/leg_pre/f96_report.txt => 1
grep -n 'def freshness' claude/harness/f96_offset.py | cut -d: -f1 => 457
# instruments
test -f claude/harness/f96_offset.py && test -f claude/harness/f91_parity.py && test -f claude/harness/f91_run.sh && test -f claude/harness/f90_rehearsal_run.sh && test -f claude/harness/dumpread.py && echo ok => ok
test -e /home/claude/sc-f100 ; echo $? => 1
```

**DoD:** predictions before the launch; the leg at both clocks through the dump AND the
operator channel; the fix (code first) or the STOP; the post-fix proofs incl. the mutation
and the D11 number; §11 entry + stub; §5.139 flipped or annotated; §5.107 annotated;
back-markers; map; README; trees clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-06 (F100) — **§11.220** + **§5.139 FIXED** (code `1af7fa48` -> `474c595d`, `ModularBody.{hpp,cpp}` only, binary `8e2c6ef3` -> `b5f08778` bit-reproduced; harness `776a59c` -> `e01814a` -> `9ba27ec` -> `e27ff87` -> `f49c501` -> the record). The leg REFUTED the row's candidate and confirmed the memo: one binary, two clocks — 48 frozen = `P u I` at `rate 0` with the 19 parked bodies' `evalCount` IDENTICAL over 1875 frames and old-vs-new to 170.715618 deg, 29 = `I` at `rate 1`. Post-fix 29 = `I` at both clocks, 1.2044e-05 deg, the pre hypothesis failing on exactly the 19; launch state byte-identical 120/120 + 90/90 + camera; F91 `c125adf0`; smoke rc 0; D14 PASS; both mutations refuted (no-memo 5.018/frame held vs 0.0000). Row reach CORRECTED at the row: `get status object` is old-first, `flag track_object on` is the reachable channel and it ran the aim to alt = -pi/2. Back-markers §11.216(j1)(j3)(d2)(l3) both homes, §11.117(c)+index, §11.76(b) both homes, §5.107, §11.218; map names §5.139 0 times. **Owner item raised, not touched: `RESUME_EXTRA_ITERATIONS = 4` is short by ~4 on Eris (1.198725 deg of readout error at 5 evaluations, 1.09669e-05 at 9) — the constant is [vixy]'s.**
**ACCEPTED 2026-09-06 — the verifying commands' `date` read 23:38:53 and 23:40:09–23:46:59 (supervisor,
session 26, Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.220 read in
full; ONE code commit `474c595d` (Claude Opus 5; `ModularBody.{hpp,cpp}`, 68+/7−, the diff READ: the
memo keyed on (date, parent frame) with `evaluatedFrame` beside `evaluatedJD`, `sameFrame` an exact
element compare, the date half read BEFORE the ancestor climb, the +4 bound to a date change only —
nothing else) and seven harness commits `e01814a → 82a1f7f` (Claude Opus 5; the predictions at
`e01814a` before the first launch), both trees clean; binary `b5f08778`, dry build 0 steps; §5.139
reads *FIXED 2026-09-06, F100 §11.220* with its reach sentence corrected (`track_object` named);
§5.107 names §11.220; §11.216 carries four markers in the entry and one in the stub; §11.76 both homes;
§11.117 entry (the stub carries no barrier-contract text — entry-only licensed); README §F100
(`:4811`); `DEPLOYMENT-MAP` 0 hits, said not assumed; instruments to the digit of the entry's stated
successor value (scan 243/303/139 · pair-check 236/211/25/116 · D 35 · D2 11 · I 89 · I2 37 · M 86).
**AND the delivered leg run by my own hand on `b5f08778` at BOTH clocks (`sc-f100/supervisor/post`,
23:40:09–23:44:37): pinned — frozen 29 = I exactly, the pre-fix hypothesis P∪I FAILING on exactly the
19 named bodies, `evalCount` still for 0 of 19 over 1875 frames, parked old-vs-new 1.93e-05° (Ceres)
after the move and Eris 1.1987249° at the launch state before it (the (j1) under-convergence,
reproduced); running — 29 = I, parked 1.66e-05°; AND the smoke suite on `b5f08778` (23:45:20–23:46:59):
rc 0, the nine step states of §11.211 (S1/S7 DIVERGENCE with citations, S4 DEPRECATED, six PASS), S5's
shape OK, exit 0 in 0.7 s, 1 frame stall, frozen 4/4 in==out, `/proc` clear, no lock file.**
Deviations ENDORSED with the executor's arguments: three refuted predictions kept as refuted (P3a,
P3c's observable, P5b); the `track` stage added — the section asked for the operator channel and the
read showed `get status object` is old-first, so without the track arm the row's reach would have
been left unmeasured instead of corrected; the post-fix pinned leg re-run once the instrument could
name the binary it looks at; `parent->useNow()` unconditional (0.0000 refreshes/frame with the camera
held, one 16-float compare, annotated at §11.117(c)). SUSPENSION ENDORSED and MINTED: the +4 constant
short by about four for Eris — **§5.145** (record-only; the decision the owner's, §11.76(b)) with
markers at §11.158(f4), §11.213(g), §11.220(j1) both homes — the "Eris = the trees' own 1.198° gap"
attribution carried by three entries since F44 is superseded by measurement. DISPATCHER-SIDE FINDINGS
reported, ACCEPTED as mine with the counterfactual: (1) the section's P3 asserted `get status object`
reads the NEW path's block — it is old-first (`core.cpp:1091-1097`), a structure asserted without the
read (Q-67's SIBLING class); (2) §5.139's reach sentence (mine, at F96's acceptance) inherited the same
assumption — corrected at the row; had either been an INPUT the executor stood on, an abort — they
were the predictions the leg tests, and the leg refuted them as designed. Round tally: **three
dispatcher defects** (the keyed-loader count; the "§5.50 shape" label; the old-first readout assumed
at two homes). STANDING CONSEQUENCES: **the D8 barrier now keys on (date, parent frame)** — a parked
body's readout follows the camera at a pinned clock (48 → 29 frozen, the 29 being the never-walked
`dist` 0 class, §11.216(i)); every un-moved-view observable and the F91 table (`c125adf0`) are
byte-identical across `474c595d`; a harness that recorded a PARKED body's readout after a camera
move at `timerate rate 0` now reads differently, because the number is right; `select planet` +
`get status object` answers the OLD path for every both-tree name — a new-path readout must use a
new-only name or the dump's `.navstr` sidecar; the dump calls the barrier for 90 of 120 records
(§11.220(j3), the dump channel's own item); `/home/claude/sc-f100/` holds the farms, the post and
no-memo binaries and my runs.

### F101 — §5.143's mechanism leg AND the old-half assert at the channel's ONE reader: the dump's old half enumerates `currentSystem` (`ssystem_factory.cpp:1181`) and the tester's `14.sts` moves that pointer — `moveto alt 1.1E+16` (`:22`) leaves the system, the `parent none` load (`:25`) lands in whatever system is current, `set home_planet Solsys` (`:27`) re-enters through `enterSystem` → `changeSystem(querySelectedAnchorName())` → `createSystem()` for a name no system carries (`:764-768`, `:294-307`, `:741`) — a fresh `ProtoSystem` holding one star, after which every `parent Earth` load is refused by that system's own name search (`protosystem.cpp:531-535`); the leg bisects farm COPIES of the 14.sts prefix (25 / 27 / 31 lines) with `f98_repro14.py --dump-after-each`, predictions first, the switch read back from the applog; then `dumpread.load_dump` raises on an empty old half by DEFAULT and the census/soak readers opt out by name (I9: one assert at the anchor, its 21 importers enumerated from it, F98's own dumps as the both-ways control) [S, instrument + reading; no engine change]

**Why now / mandate:** §5.143 [measured 2026-09-06, §11.218(h) (F98); minted at F98's
acceptance, record-only; session-25 close queue position 4]: after `14.sts` the old half
goes **246 → 1**, the corpus's `body action clear` then leaves **0**, and no later `body
action load` reaches it again while the new half keeps every body; the row's candidate,
*"the old half follows the ACTIVE system"*, is `[derived, NOT confirmed]`; the row's
standing consequence — *"instruments assert `bodies_old > 0` before reading the old
half"* — is owed as an instrument change. **The mint READ the chain the candidate needs
and it is coherent site by site; the leg confirms it or refutes it, nothing here is
measured:** the old half of `dual_dump` is `for (auto it = currentSystem->begin(); …)`
(`ssystem_factory.cpp:1181`); `currentSystem` moves in `changeSystem(mode)` (`:294-307`:
`systems.at(mode)` or, for a name no system carries, `createSystem(mode)` — a NEW
`ProtoSystem` seeded with one star named from `mode`, `:741`, `:379-392`) and in
`leaveSystem()` (`:772-778`, → `galacticSystem`); `enterSystem()` (`:764-770`) calls
`changeSystem(querySelectedAnchorName())` when `!inSystem`; old's `addBody` resolves a
parent by `searchByEnglishName` INSIDE the current system and refuses a miss
(`protosystem.cpp:531-535`). `14.sts` (`~/.spacecrafter/scripts/fscripts/14.sts`,
Latin-1 — `LC_ALL=C` tools only, never the wrapper): `:22 moveto alt 1.1E+16 duration 0`,
`:25 body action load name Solsys type Sphere … parent none hidden true … coord_func
still_orbit …`, `:27 set home_planet Solsys`, `:29 select planet Solsys pointer off`,
`:31 flag track_object on`, then the 527 bodies. Candidate chain: `:22` leaves the system
(which caller of `leaveSystem` — the task reads it), `:25` adds `Solsys` to the then-current
system on BOTH paths, `:27`/`:29` re-enter with the anchor name → `changeSystem` →
`createSystem(<name>)` → `currentSystem` = a system that holds one body ⇒ old half 1
(F98's `Solsys on both`); the corpus's later `body action clear` removes what that system
holds ⇒ 0; every later `parent Earth` push is refused by `:531-535` in a system that has no
Earth ⇒ F98's cycles 2–8 `0 / 286`. The mutation the leg must be able to see: the LOAD
itself (`:25`, a `parent none` body) empties the half — then prefix-25 alone already
reads 1, and prefix-27 adds nothing.

**Measured at dispatch (supervisor, 2026-09-06 21:25–21:41):** `dumpread.py` is the dump
channel's single reader (§11.153; **21** importers — `grep -l -E 'import dumpread|from
dumpread' harness/*.py`); `load_dump` (`dumpread.py:69-91`) returns `(header, pairs,
missing_new, missing_old)` and a record with `"old": null` lands in `missing_old` — an
empty old half is therefore `pairs == []` with `missing_old` full, and today NOTHING says
so; `f95_soak.py:1323` derives `bodies_old` from that tuple (a reader that must keep
answering 0 — it MEASURES the emptiness); F98's own dumps are the real-data control:
`harness/artifacts/f98/repro/bisect_oldpath.result.json` (246/276 → 1/277) and the driver
`harness/artifacts/f98/f98_repro14.py` (`--shows`, `--gap`, `--dump-after-each`, `--bin`;
it builds an f55-shaped farm with the 137 `.sts` as COPIES — §11.211(c): the annotator
rewrites played files in place). The tester's file: md5 in the PREMISES, asserted in==out
at the end; the truncated prefixes are FARM copies under new names, never his file.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f101/prediction.txt`): the chain above as
P1–P5 with the old/new counts predicted per prefix — prefix-25 (load only): old 246+1 / new
276+1; prefix-27 (+`set home_planet Solsys`): old = the new system's body count (predict
it from `createSystem`'s seeding: the star's name is `mode` minus its last six characters,
`:381` — read what `querySelectedAnchorName()` returns for `Solsys` and what body that
yields; if the name comes out EMPTY the star is refused at `protosystem.cpp:523` and the
count is 0 — say which you predict and why); prefix-31 (+select+track): F98's measured
1/277 to reconcile; the mutation (prefix-25 already 1) named as the refuter; the applog
lines that witness a switch (`Loading new Stellar System object`, `changeSystem`/anchor
lines — name them from the source before the run). (2) **THE LEG** (reference binary
`46849f69`, farm, `--no-scene` canary): three fresh launches, one per prefix copy
(`f101_14_p25.sts`, `_p27`, `_p31`, each a `sed -n '1,Np'` of the farm's copy), each
`--dump-after-each`; plus the negative control — prefix-27 with line 27 replaced by a
comment — and the shipped-scene control `set home_planet Mars` alone (no switch
predicted); the old/new counts, the names in each half, and the header's system identity
if the dump carries one (read `dumpOldViewState`, `core.cpp:838-850`) or the applog's
witness lines. (3) **THE ASSERT** at the anchor: `dumpread.load_dump(path, *,
require_old=True)` raises a NAMED exception carrying the path and both counts when the old
half is empty for every body record; signature widened keyword-only (every existing
`header, pairs, mn, mo = load_dump(p)` compiles unchanged — I1); the 21 importers
enumerated in the entry and classed — comparison/parity readers keep the default,
census/soak readers (`f95_soak.py:1323` and any other that REPORTS the count) pass
`require_old=False` with a one-line reason at the call; a self-test on a synthetic
empty-old dump shows the raise AND the opt-out; the real-data control: F98's post-`14.sts`
dump (from `bisect_oldpath` — regenerate on the farm if the dump itself was not kept)
raises under the default, the pre-`14.sts` dump passes. (4) **RECORD:** §11.⟨next⟩ FIRST +
stub; §5.143's mechanism written at the row from the measurement (the switch site, the
exact counts per prefix, the refuter's fate) — the row stays OPEN (retires with B8) with
"the assert landed" and the pointer; §11.218(h)(l) back-markers both homes; §5.137
annotated if the reading moves its class statement; `harness/README.md` (the `dumpread`
contract line); WIP per §0.6; D14.

**Boundaries:** NO engine code (the leg reads; `git -C /home/claude/spacecrafter status`
clean throughout); `harness/dumpread.py` + the opting callers + the leg's driver only; the
farm only (the 137 + 2 real-HOME files md5 in==out); FUNCTIONAL (`--no-scene`); no
`run_in_background`; runs under `/home/claude/sc-f101/`; the tester's corpus never edited.

**Discriminating checks:** (a) the per-prefix counts against P1–P5, the refuter
discriminated (prefix-25 vs prefix-27); (b) the switch WITNESSED in the applog, not
inferred from the count; (c) the negative control unmoved; (d) the assert red on the
synthetic case AND on F98's real post-14 dump, green on the pre-14 dump and under the
opt-out; (e) the 21 importers each named with its class; (f) 139 md5s in==out; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that
are not commands: display per HOST-EVENTS (`:2`); canary `--no-scene` exit 0 before the
first launch; the harness HEAD as the prompt states it; §5.143's row reads OPEN,
record-only; no `spacecrafter` in `/proc/*/comm` before each launch.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 474c595d
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => b5f08778
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 221
grep -c '^### F' claude/fable-dispatch.md => 4
# ledger states the work stands on
grep -m1 '^143\. ' claude/INTENT.md | grep -c 'OPEN, record-only' => 1
grep -m1 '^143\. ' claude/INTENT.md | grep -c 'derived, NOT confirmed' => 1
# sites, re-resolved at HEAD (content drift = abort)
sed -n '1181p' src/bodyModule/ssystem_factory.cpp | grep -c 'currentSystem..begin()' => 1
sed -n '764,768p' src/bodyModule/ssystem_factory.cpp | grep -c 'enterSystem\|changeSystem(querySelectedAnchorName())\|inSystem = true' => 3
sed -n '294,307p' src/bodyModule/ssystem_factory.cpp | grep -c 'systems.at(mode)\|createSystem(mode)' => 2
sed -n '741p' src/bodyModule/ssystem_factory.cpp | grep -c 'SSystemFactory::createSystem' => 1
sed -n '381,382p' src/bodyModule/ssystem_factory.cpp | grep -c 'name.substr(0, name.size()-6)\|bodyParams\["parent"\] = "none"' => 2
sed -n '531,535p' src/bodyModule/protosystem.cpp | grep -c 'str_parent != "none"\|searchByEnglishName(str_parent)\|return;' => 3
sed -n '838p' src/coreModule/core.cpp | grep -c 'void Core::ssystemDualDump' => 1
# the tester's file (Latin-1: LC_ALL=C, /usr/bin tools; the wrapper skips it silently)
md5sum ~/.spacecrafter/scripts/fscripts/14.sts | cut -c1-8 => 31503adb
LC_ALL=C sed -n '22p;25p;27p' ~/.spacecrafter/scripts/fscripts/14.sts | tr -d '\r' | cut -c1-40 | tr '\n' '|' => moveto alt 1.1E+16 duration 0|body action load name Solsys type Sphere|set home_planet Solsys|
LC_ALL=C /usr/bin/grep -c 'parent none' ~/.spacecrafter/scripts/fscripts/14.sts => 2
wc -l < ~/.spacecrafter/scripts/fscripts/14.sts => 577
# the reader and its importers
sed -n '69p' claude/harness/dumpread.py | grep -c 'def load_dump(path):' => 1
sed -n '87,88p' claude/harness/dumpread.py | grep -c 'rec.get("old") is None\|missing_old.append' => 2
grep -l -E 'import dumpread|from dumpread' claude/harness/*.py | wc -l => 22
sed -n '1323p' claude/harness/f95_soak.py | grep -c '"bodies_old": len(pairs) + len(missing_new)' => 1
# F98's control data and driver
test -f claude/harness/artifacts/f98/f98_repro14.py && test -f claude/harness/artifacts/f98/repro/bisect_oldpath.result.json && test -f claude/harness/f55_farm.sh && echo ok => ok
grep -n 'add_argument("--dump-after-each"' claude/harness/artifacts/f98/f98_repro14.py | cut -d: -f1 => 62
test -e /home/claude/sc-f101 ; echo $? => 1
```

**DoD:** predictions before the launch; the three-prefix leg + two controls; the assert at
the anchor with the 21 importers classed and the both-ways control; §11 entry + stub; the
row's mechanism written; back-markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-07 → **§11.221** (entry + stub), §5.143's mechanism WRITTEN at the row (stays OPEN, retires with B8; its candidate and the section's chain both refuted, the "assert `bodies_old > 0`" consequence DISCHARGED), §11.218(h)+(l) back-markers at BOTH homes, §5.137's class-sibling claim corrected, `harness/README.md` (contract line + F101 section). **Eleven launches, no engine change** (code `474c595d` / binary `b5f08778` untouched, tree clean at all six commits); 139/139 md5 in==out on every leg, `3995e501` over the 137, `/proc` clear and no lock at 22 checks. THE FINDING: **the ALTITUDE empties the old half at `14.sts:22`, three lines before anything is loaded** — `leaveSystem()` makes the never-populated `galacticSystem` current and the dump enumerates `currentSystem`; p23 **0/120**, p25 **1/121** (the script's own Sphere, `ecl` == its `orbit_x/y/z`), p27 = p31 = p27nc **1/121**. `set home_planet Solsys` moves no count — it re-points the galactic anchor off `Sun`, which decides the descent: p25desc RECOVERS **90/121**, p31desc does not (**0/122**), and `06old.sts` after it is refused **170/170** (old 0/278) against the control's **246/277**. Reach widened: **8 of 137 shows** cross 1e16 and `S02.sts` empties the half with **zero `body action load`**. The assert LANDED — `dumpread.load_dump(path, *, require_old=True)` raises the named `EmptyOldHalf` (path + both counts + a §2(f) fix line) when a dump has body records and `bodies_old == 0`; `python3 dumpread.py selftest` **17 PASS / 0 FAIL**, and the committed mutation (`bodies_old < 2`) turns it **13 PASS / 4 FAIL**, so the threshold is a measured choice. Real-data controls both ways from F98's OWN dumps: pre-14 246/276 GREEN, **post-14 1/277 GREEN** (the section's named RED control is green BY the criterion — reported), soak c002 0/286 RED, plus F101's p23 0/120 RED and p25 1/121 GREEN. the importers censused MECHANICALLY and classed (**24** by the section's flat pattern, **27** recursively); 4 opt out (`f95_soak.py`, `artifacts/f98/f98_repro14.py`, and my two) — the section's `harness/*.py` census misses `f98_repro14.py`, which the guard would have broken. Section boundaries held throughout; nothing under `/tmp`.
**ACCEPTED 2026-09-07 — the verifying commands' `date` read 00:41:36 and 00:41:52–00:43:28
(supervisor, session 26, Claude Fable 5.1).** Verified by my own runs and reads, not by the report:
§11.221 read in full; code UNTOUCHED at `474c595d`, binary `b5f08778` before and after, clean; eight
harness commits `675c49f → f1037fa` (Claude Opus 5; the predictions at `675c49f` before any process,
two addenda each before the launches they predict), tree clean; the `dumpread.py` diff READ
(`load_dump(path, *, require_old=True)`, `EmptyOldHalf` carrying the path and both counts, the §2(f)
message, the opt-outs at `f95_soak.py` and `artifacts/f98/f98_repro14.py` with their reasons);
§5.143 reads *MECHANISM MEASURED 2026-09-07, §11.221* and names `leaveSystem`/`galacticSystem`, stays
OPEN (B8); §5.137 annotated (shape sibling, not class); §11.218(h)(l) name §11.221 in the entry AND
the stub; README §F101 (`:4927`) + the `dumpread` contract line (`:1811-1815`); instruments to the
digit of the entry's (p) (scan 249/314/142 · pair-check 237/212/25/117 · D 36 · D2 11 · I 89 · I2 37
· M 87 — every filter delta 0, +4 raw/+6 pairs named, the one uncredited `11.221 → 11.113` disposed).
**AND by my own hand: `python3 dumpread.py selftest` 17 PASS / 0 FAIL and the mutant (`bodies_old <
2`) 13 PASS / 4 FAIL exit 1 (00:41:52); leg `p23` on `b5f08778` (00:42:08): 90/120 → 0/120 — the
altitude alone; `ctlmars --then fscripts/S02.sts` (00:42:36): 90/120 → 90/120 → 0/121 — a show with
zero `body action load` empties the half; exit 0 / quit 0.7 s both, 14.sts `31503adb` and the pair
`03fbee59`/`545a51ef` pristine, `/proc` clear, no lock.** Deviations ENDORSED with the executor's
arguments: the `p23` leg (the section's three prefixes could not separate the load from the
altitude — a green that cannot discriminate), the descent pair (the row's "for the rest of the
session" needs it), `--then` and `ctl_S02` (each with predictions committed first; C7 refutes the
row's headline on a second shipped show); P0 refuted and kept (170 → 156: 13 duplicate names + `TDRS
3` with no `coord_func`, refused identically by both paths — not a §5 candidate); the threshold
`bodies_old == 0` (the set a comparison is DEFINED on, not a magic number); the opt-out written into
F98's delivered `f98_repro14.py` (an opting caller, not a recorded criterion); `f40_env.py` keeping
the default; §5.137 annotated narrowly. SUSPENSIONS ENDORSED: the dump header carries no system
identity (a one-field dump-channel addition — next-round S candidate, queued below); `search` NOF
outside the system (R22, retired); the 1e16-then-home-planet intent question ROUTED to the main
tester (§3). FIVE DISPATCHER-SIDE FINDINGS reported, ALL ACCEPTED as mine with the counterfactual:
(1) the warm-up pointer `INTENT/11.153.md` for the single-reader ruling — it is §11.152(p)(2); a
pointer typed from memory (Q-67's class; had §11.153 contradicted the ruling, an abort); (2) the 22nd
`dumpread` importer credited to F100's `f100_freeze.py` in the prompt AND in my acceptance commit —
it is F99's `f99_locguard.py`; structure asserted without the read (the SIBLING class); (3) the
mandated RED control "F98's post-14.sts dump raises" — that dump is 1/277, a number my own row
carries; a value written against the record I held (Q-67's class); (4) the importer census
`harness/*.py` never sees `harness/artifacts/f98/f98_repro14.py` — the first caller the guard would
break; a pattern's model of the surface inside a measured count (the SIBLING class, F98's census
shape a second time); (5) §5.143's headline and the section's chain assert an authored SYSTEM as
the cause where the body said `[derived, NOT confirmed]` — the leg was built to test it and refuted
it as designed, but the headline asserted more than its body. Round tally: **eight dispatcher
defects** (the loader count; the "§5.50 shape" label; the old-first readout at two homes; and these
five). STANDING CONSEQUENCES: **`dumpread.load_dump` refuses an empty old half by default** — a
reader that measures the emptiness opts out by name; `b24_equivalence.load_dump` and
`f89_p7.load_dump` are same-named different functions outside the guard; a dual dump is comparable
only INSIDE the loaded system, and the stdout transition lines are the cheapest witness of which
system the old column describes; **eight of the tester's 137 shows fly above 1e16 and empty the old
half** (`14 S02 S07 S09 S10 S12 S12old W15`), and a home-planet set while out there decides whether
the descent comes home or lands in a fresh empty system; `06old.sts` lands 156 bodies, not 170;
`/home/claude/sc-f101/` holds the eleven farms and my two runs.

### F102 — §5.142's owed reading, priced for the owner's D13 policy: the uniform pool is ONE 1 MiB block created once (`app.cpp:274` → `BufferMgr.cpp:8`) and never grown — `acquireBuffer` only carves its free list and answers `VK_NULL_HANDLE` + the log line when it cannot (`:37-67`, `:55`); the per-body uniform cost on BOTH paths read at the acquire sites and rounded up to `minUniformBufferOffsetAlignment` (`VulkanMgr.cpp:82`); the body count at the first refusal PREDICTED from that arithmetic and committed BEFORE the log is read, then MEASURED from a full applog of arm C (`06.sts` then `14.sts` — the row's "`06.sts` alone" is corrected: alone it is clean, §11.218(g)); the chain from the null `SubBuffer` to the device loss read at the consumer; the three policies priced (grow / refuse with a §2(f) line at the anchor / degrade) — nothing fixed, EntityCore read-only [S, reading; one launch at most]

**Why now / mandate:** §5.142 [measured 2026-09-06, §11.218(g) (F98), reproduced by the
supervisor; minted at F98's acceptance, record-only, VIXY'S STRATUM; session-25 close
queue position 5 and §3's headline item]: *"Owed before pricing: the pool's size and the
per-body cost read at `BufferMgr` (one reading), and `06.sts` alone for the number of
bodies at which the first error appears."* The second half of that sentence is WRONG as
written (a dispatcher slip at the mint): `06.sts` alone is CLEAN — 1013 bodies, 0 buffer
errors, and 1156 with `06old.sts` after it; the errors begin inside `14.sts` (§11.218(g)).
The launch that carries the number is arm C (`06.sts`, a 60 s gap, `14.sts`), and F98 kept
only the applog's TAIL of it (`repro_06_gap60_14.applog_tail.txt.gz`). The row is
corrected at the row by this task, marker included (output-side). **What the mint READ
(submodule `src/EntityCore`, read-only):** `context.uniformMgr` is constructed with
`bufferBlocSize = 1*1024*1024`, `uniformBuffer = true`, HOST_VISIBLE|HOST_COHERENT with
DEVICE_LOCAL preferred (`app.cpp:274`); the constructor calls `master.createBuffer` ONCE
for exactly that size (`BufferMgr.cpp:8`) and seeds the free list with one `SubBuffer`
spanning it (`:16-20`); `acquireBuffer(size)` rounds `size` up to
`uniformOffsetAlignment` (`:39-41`), searches the free list, and when nothing fits leaves
`buffer.buffer == VK_NULL_HANDLE` and logs *"Can't allocate buffer in '<name>' !"*
(`:52-55`) — NO second block, no growth path (`createBuffer` appears once in the file);
the alignment is the device's `minUniformBufferOffsetAlignment` (`VulkanMgr.cpp:82`). The
callers of `uniformMgr` are the per-body modules of BOTH paths (`grep -rn uniformMgr` over
`experimentalModule/` + `bodyModule/` — the count is a premise).

**Mandate:** (1) **READ + PRE-REGISTER** (`artifacts/f102/prediction.txt`, before the log
is opened): the alignment on this device (read from the engine's own log if it prints the
limit, else `vulkaninfo`, else a two-line probe — state the source and the value); for the
body kinds `06.sts` and `14.sts` author (`filename Star_*|Planet|…`, `type` per line —
census both word orders, F98's lesson), the persistent `acquireBuffer` calls per body per
path (which module, which struct, which size, rounded) — and the per-frame
`fastAcquireBuffer` users named as NOT counting; the shipped launch scene's baseline draw
on the pool (120 bodies + globals) if derivable; the predicted body index N at the first
refusal = (pool − baseline) / per-body-both-paths, with the mutation (one path's share
only) beside it; the predicted refusals per body after the first (how many acquisitions
each later body attempts — F98's 1557 over 202 bodies ≈ 7.7/body is the number to
explain). (2) **MEASURE:** the kept tail does NOT carry the first refusal (116 lines, 33 refusals,
the first at its line 2 — measured at the mint), but the executor's FULL arm-C applog may
still exist in the kept scratch tree, `/home/claude/sc-f98/repro_06_gap60_14/repro.applog`
(a premise line says whether it did at the mint; `/tmp`-class survival is never assumed —
verify, then copy the lines you cite into `artifacts/f102/`); if it is there and complete,
no launch; else ONE launch on a farm
(arm C shape via `f98_repro14.py --shows fscripts/06.sts,fscripts/14.sts --gap 60` or its
equivalent, FULL applog kept; the app WILL abort — `/proc/*/comm` clear asserted after,
139 md5s in==out): the bodies loaded before the first refusal (both word orders), the
refusals per body after it, N against the prediction and the mutation. (3) **THE CHAIN:**
where the null `SubBuffer` is stored without a check (the consumer of `acquireBuffer`'s
return — `SharedBuffer`/the module), the first bind or write that reaches it, and how that
becomes `App::draw`'s wait (`app.cpp:831`) and the device loss — with citations; whether
the *"Succesfull loading ojm"* that follows each refusal is the same body reporting success
on a refused allocation (§5.116's class — name the site that says success). (4) **PRICE for
the owner, no policy chosen:** (a) GROW — bytes per body × the corpus's maximum (1719
bodies, both paths today; the new path's share alone after B8) ⇒ the pool size that
holds it and its memory class; (b) REFUSE — the anchor (the acquiring module knows the
body and the pool; §11.193) and what the body then IS (absent from the draw? drawn
without the module?), the §2(f) line drafted; (c) DEGRADE — what a body could drop to
fit, if anything the modules already support. (5) **RECORD:** §11.⟨next⟩ FIRST + stub;
§5.142 annotated with the reading, the numbers and the "alone" correction (marker);
§5.60 annotated (the pool-sizing member now has its numbers); §11.218(g) back-marker both
homes; the map (`grep -n '5\.142'` = 0 at the mint — T5's readiness sentence names the
abort; annotate where it does); README section; WIP per §0.6; D14.

**Boundaries:** NO code change (EntityCore read-only; no engine edit; both trees'
`git status` clean of code throughout); the farm only; at most ONE launch, FUNCTIONAL
(`--no-scene`); the abort in that launch is EXPECTED — the concurrent-instance probe
after it, the real HOME md5 in==out; no `run_in_background`; runs under
`/home/claude/sc-f102/`.

**Discriminating checks:** (a) N predicted before the log is read, then measured, the
one-path mutation refuted; (b) the alignment's source stated; (c) the refusals-per-body
number explained by the per-body acquisition count; (d) the chain cited site by site; (e)
the three prices with their arithmetic; (f) 139 md5s in==out and `/proc` clear after the
abort; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose premises that
are not commands: display per HOST-EVENTS (`:2`); canary `--no-scene` exit 0 before the
launch; the harness HEAD as the prompt states it; §5.142's row reads OPEN, record-only.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 474c595d
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => b5f08778
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 222
grep -c '^### F' claude/fable-dispatch.md => 4
# ledger states the work stands on
grep -m1 '^142\. ' claude/INTENT.md | grep -c 'OPEN, record-only' => 1
grep -m1 '^142\. ' claude/INTENT.md | grep -c 'alone for the number of bodies' => 1
# sites, re-resolved at HEAD (content drift = abort); the submodule pin is a premise too
git -C src/EntityCore rev-parse --short=8 HEAD => 84f5d94b
sed -n '274p' src/appModule/app.cpp | grep -c '1\*1024\*1024, "uniform BufferMgr", true' => 1
sed -n '8p' src/EntityCore/Core/BufferMgr.cpp | grep -c 'master.createBuffer(bufferBlocSize' => 1
grep -c 'createBuffer' src/EntityCore/Core/BufferMgr.cpp => 1
sed -n '39,41p' src/EntityCore/Core/BufferMgr.cpp | grep -c 'uniformOffsetAlignment' => 1
sed -n '55p' src/EntityCore/Core/BufferMgr.cpp | grep -c "Can't allocate buffer in" => 1
sed -n '82p' src/EntityCore/Core/VulkanMgr.cpp | grep -c 'minUniformBufferOffsetAlignment' => 1
grep -rn 'uniformMgr' src/experimentalModule/ src/bodyModule/ --include=*.cpp --include=*.hpp | wc -l => 71
sed -n '831p' src/appModule/app.cpp | wc -l => 1
# F98's kept evidence
ls claude/harness/artifacts/f98/repro/ | wc -l => 7
zcat claude/harness/artifacts/f98/repro/repro_06_gap60_14.applog_tail.txt.gz | grep -c "Can't allocate buffer in 'uniform BufferMgr'" => 33
grep -o '"body_action_load_executed": *[0-9]*' claude/harness/artifacts/f98/repro/repro_06_gap60_14.result.json | head -1 => "body_action_load_executed": 202
test -f claude/harness/artifacts/f98/f98_repro14.py && test -f claude/harness/f55_farm.sh && echo ok => ok
# output-side (report, never abort): the executor's full arm-C applog in the kept scratch tree, if it survived
test -f /home/claude/sc-f98/repro_06_gap60_14/repro.applog && echo present || echo absent => present
test -e /home/claude/sc-f102 ; echo $? => 1
```

**DoD:** the reading with its citations; N predicted then measured; the chain; the three
prices; §11 entry + stub; §5.142 and §5.60 annotated; back-markers; map; README; trees
clean; WIP cleared; baselines LAST.
**WIP:** DELIVERED 2026-09-07 → **§11.222** (`INTENT/11.222.md` + stub), harness `5a2c608` → `29f511a`
→ this record; **code UNTOUCHED** (`474c595d` throughout, `src/EntityCore` read-only at `84f5d94b`,
binary `b5f08778`, `git status` clean at every commit), **ZERO launches** — the kept arm-C applog was
complete, so the one allowed launch was not spent and the canary was correctly not run (reported).
Gate 22/22. **N predicted 674 (bracket 662–692) and committed at `5a2c608` before the log was
opened; MEASURED 675** — `ZHONGXING-20A`, the 675th body of **`06.sts`**, in TWO different launches
(arm C and F98's own shakedown); the one-path mutations (new-only 904, old-only no refusal at all)
both miss. Pool 1 MiB, one block, no growth; alignment **64** (`vulkaninfo` on the device the
engine's own log names). Per body **1344 B** = old 192+128 in `Moon`'s ctor + new 192+832 in
`BasicMesh`'s; `14.sts` is not a body push at all (`mode in_galaxy` → `OjmMgr::load`, 128 B, neither
path). 1557 = **1355 in `06.sts`** + 2 (the landscape at the system switch) + 200 of 201 in-galaxy;
launch-scene baseline measured by inversion at **142 529–142 592 B**. "7.7/body" refuted as a
division artifact. Chain cited site by site to the device loss, incl. `ojm_mgr.cpp:86` saying success
200 times over a refused allocation and `:196` handing the indeterminate offset to
`vkCmdBindDescriptorSets`. Three prices priced, **no policy chosen** (2 MiB / refuse at the load
anchor / 768-byte caster array). §5.142 annotated with the reading AND the "alone" correction
REVERSED (its original sentence was right); §5.60 and §5.116 annotated; §11.218(g) back-marked at
BOTH homes; map, README, artifacts `harness/artifacts/f102/`. ~~2026-09-07 01:02 — CHECKPOINT 1 (the READING + the PRE-REGISTRATION, committed BEFORE the
applog is opened). Gate 22/22 PASS; the kept arm-C applog VERIFIED complete (10226 lines, the
`terminate called without an active exception` death line at its end) ⇒ NO LAUNCH. Alignment **64**
(source: `vulkaninfo` GPU0 RTX 5090 `minUniformBufferOffsetAlignment = 0x40`, the device pinned by
the engine's own `Device :` line in an independent applog — the engine prints no limit). Pool 1 MiB,
one block, no growth. Per-body carved, MEASURED (`f102_sizes.cpp`): 06.sts body = 192+128 (old
`Moon::selectShader` ctor) + 192+832 (new `BasicMesh` ctor) = **1344 B**; 14.sts body = **128 B**
(`ojm_mgr.cpp:77`, `mode in_galaxy` — neither body path). Baseline (90 ssystem bodies) 123072 B +
globals. PREDICTION committed: **N = 674, the first refusal INSIDE 06.sts** (bracket 662–692), the
refused request the 832 B `meshFrag`; mutations: new-only 904, old-only never. NEXT: open the
applog, measure N and the refusal structure, then the chain, the three prices, the record.
2026-09-07 01:08 — CHECKPOINT 2 (the MEASUREMENT, `f102_applog.py` over the surviving arm-C applog;
no launch). **N = 675**, body `ZHONGXING-20A`, **inside `06.sts`** (applog line 6621) — predicted 674,
bracket 662–692, so the prediction lands one body short and INSIDE the bracket; the one-path
mutations (new-only 904, old-only never) both MISS. The refused request is the OLD path's 192 B
`globalVertProj`, not the 832 B `meshFrag` I named — that sub-prediction is REFUTED and recorded as
such. Structure to the digit: 1557 refusals = 1355 in `06.sts` (674 bodies clean, then one at 3, then
338 at 4) + 2 at the landscape the system switch builds + 200 of `14.sts`'s 201 in-galaxy loads; the
model at baseline B in (142528, 142592] reproduces that histogram EXACTLY, so the shipped launch
scene's own draw on the pool is MEASURED by inversion at **142.5–142.6 KB**. `06.sts` alone is NOT
clean — the row's mint-time correction is itself wrong, and both readings are now recorded. 201
`Succesfull loading ojm` lines for 200 refused allocations (§5.116's class at `ojm_mgr.cpp:86`).
NEXT: the chain write-up, the three prices, the §11.222 record.~~ (checkpoint trail struck, kept.)
**ACCEPTED 2026-09-07 — the verifying commands' `date` read 01:28:11–01:29:13 (supervisor, session
26, Claude Fable 5.1).** Verified by my own runs and reads, not by the report: §11.222 read in full;
code UNTOUCHED at `474c595d`, `src/EntityCore` at `84f5d94b`, binary `b5f08778`, clean; four harness
commits `5a2c608 → a67e6d0` (Claude Opus 5; the prediction at `5a2c608` before the log was opened),
tree clean; §5.142 reads *READ AND PRICED 2026-09-07 (F102, §11.222)* with the marker in the RIGHT
direction (the row's own sentence restored); §5.60, §5.116 and §11.218(g) name §11.222 in the entry
AND the stub; README §F102 (`:4990`); the map's one hit named; instruments to the digit of the
entry's close (scan 252/319/143 · pair-check 238/213/25/117 · D 36 · D2 11 · I 89 · I2 37 · M 87).
**AND by my own hand (01:28): `vulkaninfo` — RTX 5090 `minUniformBufferOffsetAlignment = 0x40`,
llvmpipe `0x10`; the surviving arm-C applog (10 226 lines, ending on `terminate called without an
active exception`): first refusal at line 6621, **675** old-path `Loading new Stellar System object`
lines from the `06.sts` play echo (line 2073) to it, the 675th being `ZHONGXING-20A`; 1557 refusals
and 201 in-galaxy successes in total; the delivered parser re-run: 1357 solar + 200 in-galaxy,
677 old / 680 new.** Deviations ENDORSED with the executor's arguments: no canary (no measuring
launch was made — the preflight has no launch to precede); F98's shakedown applog read beyond the
section's letter (it is the log §11.218(g) was written from); §11.218(g) corrected in part with the
arm-C figures explicitly preserved; the `meshFrag`-first sub-prediction kept as refuted; the
corpus parser's own defect found by its output before any number entered a record. SUSPENSIONS
ENDORSED: the policy (§3, yours); "what the body IS" under REFUSE (yours); the two named
candidates — **§5.146 MINTED** at this acceptance (the allocator releasing a refused `SubBuffer`,
read not run, Vixy's stratum; markers at §11.222(g)(6)/(i) both homes), `ojm_mgr.cpp:86` annotated at
§5.116 by the entry. DISPATCHER-SIDE FINDING reported, ACCEPTED as mine — and it is the round's
sharpest: **the mint "corrected" a correct row sentence on the strength of §11.218(g)'s claim
("zero buffer errors after `06.sts`+`06old.sts`, the errors begin inside `14.sts`"), which that
entry's own shakedown log refutes** — a correction validated against one cached conclusion instead
of the record it was written from (FM-2; "corrections are error candidates"); the mandate then
instructed the executor to write the reversal into the ledger, and the executor wrote the marker in
the measured direction instead and flagged it. Round tally: **nine dispatcher defects** (4 of the
value class, 5 of the structure class). EXECUTOR REPORT DEFECT found two rounds late: F98's
§11.218(g) sentence above — its own artifact contradicts it; corrected by F102 with markers, the
arm-C figures reproduced to the digit. STANDING CONSEQUENCES: **`06.sts` is a ONE-show hazard on
the reference binary — the pool runs out at its 675th body, whatever follows**; `14.sts`'s stars are
128-byte in-galaxy models on neither body path; a `06.sts` body costs 1344 B of a 1 MiB pool that
never grows (320 old-eager + 1024 new-at-load); the three prices are at §11.222(h) and §3; the
allocator's post-refusal bookkeeping is §5.146; `/home/claude/sc-f98/` must survive until §5.146's
launch (it holds the only complete arm-C log).

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
