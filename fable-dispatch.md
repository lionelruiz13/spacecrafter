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
source, never recall). Compiled at code `master-beta @ ae3a218d`, harness `0abd30c`.

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

**Update [Claude Fable 5.1 2026-09-12, supervising session 29 — LovelyFoxDev, the INTERN'S-FIRST-DAY /
SAMPLER-SEED / LAUNCH-PRECONDITIONS round: F110 · F111 · F112]:** trigger = the §0b verbatim line PLUS one
owner sentence: *"Just before starting the dispatch, reminds me where I should focus in priority in order to
unlock the most work"* — answered in-line before the first mint (the list is in this session's transcript; its
ledger form is the §3 block at close). Open at **Saturday 10:55 CEST** — the owner's deep window, and he was
PRESENT: three questions asked in-line, three answered (§11.232). Warm-up (every value from the command beside
it, Q-67; `date` 10:55:56 at the first probe): both trees CLEAN at open, code `71b6fe51` / harness `aaef2d12`;
definition-drift MATCH (`8e364a3a`); binary `6d63e6c1` current. **THE HOST REBOOTED AGAIN AT 09:51:57** (the
second boot in 14 h; `loginctl` 5/6 → **10/11**, cookie `.8TBXV3`, `:2` 2448x1332 — HOST-EVENTS 2026-09-12);
canary `--no-scene` exit 2 by design (the three per-boot members), RE-BANKED with the argument. **THE OWNER'S
HISTORY REWRITE RAN MID-WARM-UP (11:03) AND BOTH BRANCHES ARE PUSHED**: code `71b6fe51 → fcc277c9`, harness
`aaef2d12 → 16c68276`, 0 unpushed on each, 83 + 846 commits rewritten, 605 citations repointed (the PREMISES
blocks of the live sections included), maps under `sha-maps/`; every open-time value re-measured at 11:13:30
before any write. **THEN A RED THE BANK CANNOT PREDICT**: the FULL canary failed `photometric.empty` — the app
died at *"Failed to allocate chunk of 256 MiB"* with 1591 MiB of VRAM available, because `qwen3.8:UD-Q6_K`
(27.3B) sat resident in the owner's ollama under `KEEP_ALIVE=-1` holding 29.5 GB; NOT mitigated (uid 997,
§11.174(h)); asked; released on his word; FULL canary **exit 0** at 11:34:15, 12/12 in band, every delta 0.0 —
the band on a FOURTH boot. HIS REPLIES (§11.232, verbatim): a part-time intern WITHOUT an LLM starts on
`master-beta` THIS WEEK on Linux, the main tester will test his features, pushes at the end of each week, no
rename, `doc/developer-entry.md` is his entry and the harness readable, *"the new version shall be equally
capable as the old one … it must be consolidated first"*. Config/ssystem md5 pristine (`03fbee59`/`545a51ef`) in
== out; no instance, port 7805 free; RAM **52 GiB available of 59**, `-j24`; next free §11 **233** after §11.232
(live ∪ archive, `max+1`). Live `### F` **3 → 0** by **archival pass 21** at OPEN (update-s27 + F107/F108/F109,
335 lines = 162 + 60 + 63 + 48 + the doubled separator; manifest `2026-09-12-pass21`, pre-md5 `4ee97677`
reproduced in-process AND from disk by an independent script; `1d07413`; **nothing carried**) **→ 3** by the
mints below. Instrument baselines at open (run 10:58, unchanged by the rewrite): scan **271/339/144** ·
pair-check **247/222/25/124** · D 35 · D2 12 · I 89 · I2 37 · M 91 — to the digit of the session-28 close.
QUEUE CONSUMPTION (the session-28 close's list, RE-ORDERED on the owner's *"consolidated first"* + *"this
week"*): (0) NEW, ahead of everything — **the intern's entry path → F110** (S–M — the doc followed by hand from
a plain clone; measured at the mint: a plain clone shows him none of `claude/`; line 276 hardcodes `DISPLAY=:2`
and line 306 sends him to a canary banked on THIS host, so `f90_rehearsal_run.sh` STOPS on his machine by
construction; `c5be42b`, cited three times, has been unreachable since its 2026-09-05 amend and today's rewrite
moved its successor again — `f85_links.py` checks no sha; the owner's three gaps (R23, §9, the remote form)
routed, not written); (1) pass 21 — DONE; (2) **the exe-identity probe → F112** (M, not S — the census at the
mint found **42** copy-pasted `comm` sites, not three; widened to ONE home + a GPU-headroom GATE after today's
red, derived from the app's own init log); (3) **§5.150's fix → F111** (S–M — the old path's `batchLastE` pair
called from `sampleOrbit`; the one-frame transient caught in the engine first at an every-frame-resample rate,
or the reachable-rate STOP); (4)–(9) carried; the (7) tool residues (`purge-path.sh`'s map, `list_code_trailers`'
skip) are now dispatchable — the Saturday run has happened. **THEN HIS SECOND MESSAGE (§11.233, ~12:0x, while
F110–F112 were being minted) DECIDED FOUR ITEMS OF THE PRIORITY LIST AT ONCE**: §5.142 → 1 GiB → **F113** (S —
one constant at `app.cpp:274` + the D13 reading measured as the veto point on his number); §5.100/§5.101
AUTHORISED → **F114** (S–M — `trackBody`/`lookTo` exist, two sites; the preload signal he named recorded as a
follow-on); T1.3 answered in DIRECTION (the B5 remainder is a DESIGN PASS, queued, not this round); §5.140 → the
tester; §5.146's shape (throw) coupled to his EntityCore push; [Y4] closed; and four of my §5 pointers were
unresolvable for him (inline rows carry no `§5.` — the fix rides F110). Picks, RE-CUT on *"consolidated first"*
and his answers: **F110 → F113 → F114 → F112 → F111** — the doc first (the intern's clock), then the abort his
corpus reproduces, then the last operator-basics hole, then the probe every later launch stands on, then the
sampler's seed; the last two carried to the next round if capacity runs out (§0b.2: five is past the sweet
spot, said here on purpose). Deliveries: all to the parent (§11.234+, refreshed at each dispatch). Launch **[RE-CUT 12:3x–12:5x: F113 WITHDRAWN before dispatch — the 1 GiB was for the LOGS (§11.233(b) correction, my misread, Q-04); the owner then asked the ROOT of the 1344 B → **F115** (S–M, read-only census + arithmetic) and gave the log bound's semantics → **F116** (S). Order: F110 (DELIVERED 12:39, ACCEPTED 12:5x) → F114 → F115 → F116 → F112 → F111; six live sections, the tail carried if capacity runs out.]**
classes: all FUNCTIONAL (`--no-scene`; F113 adds one FULL canary after); F110 builds from a clone (no install);
F112 launches one decoy. Remotes: 0 unpushed at open on both; the owner pushes at the end of each week [vixy
2026-09-12].

---

**Update [Claude Fable 5.1 2026-09-11, supervising session 28 — LovelyFoxDev, the SEED-STALENESS /
LOG-RETENTION / RESIDUAL-STEP round: F107 · F108 · F109]:** trigger = the §0b verbatim line and nothing
else. Open at **Friday 20:15 CEST** — a weekday evening, the eve of the owner's Saturday `supervised-by.sh`
run (§11.223(d)); this session asks NOTHING in-line, writes its decisions to §3 in Q-70's shape, and touches
NEITHER `supervised-by.sh` NOR `purge-path.sh`: the tool he runs tomorrow is FROZEN at the F106 acceptance
state, and the two instrument residues that name it (`purge-path.sh`'s own map, `list_code_trailers`' silent
skip) stay queued BY DECISION, not by effort — a change the evening before his run would make the §3 [Y4]
model he already read stale. Warm-up (every value from the command beside it, Q-67; `date` 20:15:08 at the
first probe): both trees CLEAN at open, code `7ceea976` / harness `8192ad4` (session 27's post-close commit
— no owner write since); definition-drift assert MATCH (`8e364a3a`); binary `e411b838` current (0 `src/`
files newer). **THE HOST REBOOTED AT 20:06:16, nine minutes before the trigger** (`uptime -s`; the
2026-09-04 18:45:08 boot of sessions 21–27 ended, `/tmp` with it): `loginctl` claude sessions **5/6** (the
bank named 14/15), Xwayland `:2` pid 12735 under the new cookie `.AF5BV3`, `xdpyinfo` 2448x1332, the
inherited `DISPLAY`/`XAUTHORITY` already correct — the owner re-provisioned the RULED display class at
20:13:30, two minutes before the line; canary `--no-scene` **exit 2** at 20:19:47 (FAIL `compositor.restarted`,
NOTE `xserver.restarted`, every other member EQUAL to the bank — the VALUES block's per-boot paragraph
firing as written), RE-BANKED in one VALUES-block edit with its argument (the three per-boot members; the
2026-09-04 values struck not deleted; the band untouched), then **exit 0** at 20:26:14 and the FULL canary
**exit 0** at 20:28:03 (165.258/6.644 · 160.142/6.603, every delta 0.0 — the band on a THIRD boot);
HOST-EVENTS entry + the §11.174(h) decision flag (`1d28fc4`). Config/ssystem md5 pristine
(`03fbee59`/`545a51ef`) in == out; no `spacecrafter` in `/proc/*/comm`; no lock file; RAM **53 GiB available
of 59**, `-j24`; next free §11 **229** (live ∪ archive, `max+1`); unpushed **98 code / 831 harness** at
open, origin refs UNMOVED since session 27 ⇒ the Saturday rewrite has NOT run yet. Live `### F` **4 → 0**
by **archival pass 20** at OPEN (update-s26 + F103/F104/F105/F106, 948 lines = 130 + 215 + 198 + 204 + 199
+ the doubled separator after the moved note; manifest `2026-09-11-pass20`, pre-md5 `635c369f` at `8192ad4`
reproduced in-process AND from disk, archive files written before the live surface — Q-56; the in-process
instrument every pass used is now a FILE, `harness/fd_archive_pass.py`, same manifest schema; commit
`3c38b05`; **nothing carried**) **→ 3** by the mints below. Instrument baselines at open (run 20:15): scan
**263/332/144** · pair-check **244/219/25/120** · D 35 · D2 12 · I 89 · I2 37 · M 89 — to the digit of the
session-27 post-close. QUEUE CONSUMPTION (the session-27 close's list, in order): (1) pass 20 — DONE; (2)
what F106 left owed — nothing for the owner (§11.228); (3) **§11.225(j2)/(j3)'s leg → F107** (S — WIDENED
by the read: (j3)'s "+32 or −8 rad" are coincidences of a non-monotonic residual map; the seed's FIRST
writer is the CONSTRUCTOR, `ModularBody.cpp:121/:126`, which evaluates every body at `parent->lastJD` = **0**
before the first frame, so Eris's first use meets a seed **75.765 rad** behind (2π · 2461233.5 / 204109.40 d);
the sampler (j2) cannot reach a parked body — `flag_planets_orbits = false` in the field and parked subtrees
are not walked; predictions first, one mutation on a scratch worktree at `4cd00139`, NO delivered engine
change; two §5 candidates reported, minted at acceptance if at all); (4) §5.146 on his word — NOT said,
not minted; (5) owner-word items — carried; (6) the (g) tail: **§5.115's retention → F108** (S–M — the fork
collapsed at §11.173(b) and the scalar paid by R20, so the mechanism is the executor's under FIXED
invariants: numbered rotation at open, every channel, the current file keeps its name; a COMPILED constant,
NOT a config key — `checkConfigIni` would write a new key into the field's `config.ini` and break the
`03fbee59` precondition every later task asserts; the key is a veto point); **§5.66 + §5.71 — NOT minted:
the (g) list's "decision-free" is REFUTED at both rows** (§5.66 owes which landing `look_at azimuth X
altitude Y` means; §5.71 owes the duration-branch feel answer — each row's own R28 annotation says so);
**A15's residual step → F109** (S — the (g) list's "tuning pass over B22's constants" is imprecise: §11.82(b)
names a MECHANISM lever, the α-scaling of the halo disc floors; L1 authorises the direction; the attribution
among the candidate floors is measured before the change; photometric, on a canary green on both arms
tonight); (7) instrument residues — carried (the two tool items by decision, above); (8) riders, (9) owner
items per §3 — carried. Picks: **F107 → F108 → F109** (no engine change first; then the engine change whose
launches are cheapest; then the photometric one). Deliveries: all to the parent (§11.229+, refreshed at each
dispatch). Launch classes: F107 FUNCTIONAL (`--no-scene`), F108 FUNCTIONAL (`--no-scene`), F109
PHOTOMETRIC (full canary — green at 20:28:03). Remotes: local contains origin on both; push impossible here —
the owner's push is R5, Saturday.
**Round outcome (session 28 close, 2026-09-12 01:1x — every time in this note is pasted `date` output; the
close commit's own clock is the stamp):** F107 → **§11.229** + §11.225(j3) ATTRIBUTED (the constructor seeds every
body at JD 0, 75.765 rad before Eris's first use; the replay on the tree's own sliced text reproduces the landed
pre AND post dumps float32 for float32; one line on the pre tree buys what two steps per call bought) + (j2)
ANSWERED (a channel on exactly one record, Europa 3.5e-08 AU; the sixteen larger movers are the OLD path's plot)
+ **§5.149/§5.150/§5.151 minted at acceptance** · F108 → **§11.230** + §5.115's RETENTION half FIXED (every
channel keeps eight launches by numbered rotation, the current file keeping its name; a compiled constant; six
D12 lines per launch; `logread.py` the ONE reader of any cLog channel; the section's own mutant a no-op —
`rename` replaces its destination — and the cap broken a real way) + **§5.152 minted at acceptance** (a refused
second instance aborts with SIGABRT and is silent on the console, both binaries) · F109 → **§11.231**, a STOP
endorsed: the ~10 % residual L1 was asked to judge is the INSTRUMENT's (an offline px axis 1.3–1.7 % low, the
dot's own brightening, 8-bit clipping); the interior emits 0.12 % of the swing at the lowest in-band px, the
collapse is continuous, §11.82(b)'s attribution REFUTED, no engine line moved — **three for three delivered AND
supervisor-verified same session**, every delivery re-run by my own hand (F107: the pre slice rebuilt from the
scratch worktree, `--sweep` md5 `f6c8f70f`, the replay call for call; F108: the full canary on `6d63e6c1` green
to the digit, a real-home launch with the inode chain and the deleted file's size by my own `stat`; F109: the
analyzer of record, the decomposition, the interior fit and the edge bracket on the committed sweeps). Code
`7ceea976 → fcc277c9` (ONE executor commit: F108's three files); binary `e411b838 → 6d63e6c1`; harness
`8192ad4 →` this close. SUPERVISOR ACTS: the canary re-bank + HOST-EVENTS (`1d28fc4`); archival pass 20 +
`fd_archive_pass.py` (`3c38b05`); three mints under the PREMISES rule (`4a86720`; 32/34/20 after three
instrument catches); three acceptances (`294c1bd`+`3b9ecc9`, `ef7032e`, this close) each after my own runs;
**§5.149–§5.152 minted** at acceptances with markers at their attributing nodes (entry and stub each); the
per-round premises refreshed at each dispatch by content-located replace (F108's next-free; F109's HEAD, binary
and next-free); §0.5's concurrent-instance bullet corrected (blind to staging binaries); in `~/shared`: Q-67 ×4
(the mint-time set with the two-range-`sed` sub-class; F107's three; F108's five; F109's two with the
prose-escapes-refresh sub-class), **Q-71 NEW** (an instrument artifact transmitted as a product fact through the
tester channel and returned as a mandate — four hops, none re-verifying the number's height). OWNER EVENTS
IN-SESSION: none — the trigger line only; nothing asked (Friday night). HOST: rebooted 20:06:16 (the entry at
open); same boot through the close; sessions 5/6 and `:2` 2448x1332 at open and close; RAM 53–54 GiB; **17
canary runs** (one red by design, then green on every run), ~50 measuring launches by the executors + 4 by me,
no lock file at any check, the field pair `03fbee59`/`545a51ef` in == out throughout; **HOST-EVENTS entry
written at open (the reboot), none owed at close**. SUPERVISOR TALLY: **thirteen dispatcher defects** (5
value-class, 8 structure-class), all output-side, none reaching a delivery — the mint's three (a line number
mislabelled by a two-range `sed`; a pattern matching a comment; a REFRESH line where a measurement was one
command away), F107's three (14 vs 12 by the census keys; `planets_orbits` reaching no iterating body; the
token form), F108's five (`min(k,8)` on a non-empty directory; a log stamp that does not exist; a pattern census
that missed six readers; a mutant that cannot fail; the console flush order), F109's two (a per-round md5 baked
into prose; a README section asserted). EXECUTOR REPORT DEFECTS: none reaching a record (§11.230(n) carried a
duplicated D14 sentence from a later edit — struck at acceptance). EXECUTOR criterion-integrity instances:
**≥ 30** (F107: predictions by name before any run with a self-refutation kept; a mutation refuted and
root-caused, then narrowed; a third mutation to attribute rather than argue; the control bit-reproduced three
times; the within-launch comparison chosen for its measured floor. F108: the dispatch's counts corrected in
writing before the build; the field's five files archived before the campaign deleted them; the identification
method replaced by three keys; the mutant predicted a no-op before it was built; the reader given a selftest
shown able to fail. F109: the baseline first and its shape reproduced; the floor census from the dump before a
line was compiled; one env-gated build with the A/A free; the decisive arm's refutation split by a
pre-registered ladder; the lever priced, not pulled; two instrument defects fixed at the root, a third
recorded). Archival pass 21 (update-s27 + F107/F108/F109, live `### F` 3 → 0) DEFERRED to the next open.
NEXT-ROUND QUEUE, in order: (1) archival pass 21 at open; (2) **the exe-identity concurrent-instance probe**
(S, instrument, three homes — the standing assert is blind to staging binaries, §11.231(j2)); (3) **§5.150's
fix** (S, new path: a separate plot seed in `OrbitModule`, the old path's own shape; the slice as its proof);
(4) ON THE OWNER'S WORD: §5.149's shape · §5.152 · F108's four veto points · the fade profile (§11.231) ·
§5.142's policy · the `orbit_lon` ruling → §5.21's two halves + §5.140 · §5.144's 8–12 h leg; (5) the tester:
the F109 question (does he SEE a step?) + Sedna (§5.151) + the §11.207(i) list; (6) the (g) tail RE-CUT (§5.66
and §5.71 owe answers; item 6 closed by measurement; §5.115's density half); (7) instrument residues:
`purge-path.sh`'s own sha map and `list_code_trailers`' silent skip (AFTER the owner's Saturday run);
`dumpread.load_dump` vs F100's `pinned_*.json.gz` (AttributeError at `dumpread.py:157`, my own probe at the
mint); `f96_offset.parse_dump` bypassing `dumpread` (§11.226(i1)); the D14 gate's partition not covering
`claude/harness`; the `dumpread` self-test's two synthetic cases; a PREMISES line per cited pointer; the scan's
EVENT lexicon (by ruling); `f99_sweep.py`; `f85_links.py`; `f89_p7.py margins`; the b4 `/proc` probe; (8)
riders: the guard's "outside the walk" vs "not walked yet" (§11.223(e)); §5.84's own trigger;
`observedToBodyLocalPos`; the `[parallel-script]` question; scedit README `:43`; `panorama5.sts:102`; `TDRS 3`
and the 13 duplicate names; (9) owner items per §3. Remotes: **99 code / 857 harness** unpushed before this
close's commit (measured 01:11:13); push from a keyed host — the supervisor never pushes. BASELINES AT CLOSE:
this close edits `fable-dispatch.md` only (outside both instruments' read set), so the values are F109's —
scan **271/339/144** · pair-check **247/222/25/124** · D 35 · D2 12 · I 89 · I2 37 · M 91 — re-run after the
edit and printed by the close commit's own call.

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
     `dd815ab3~1`); census over all 505 tracked regular files under `src/` finds
     exactly those two]** such file among 500 tracked `src/` files,
     `doc/superscript.sts` another
     (11 hits without `-I`, none with). `/usr/bin/grep` or Read on every ISO-8859 file.
     CLAUDE.md (one file: the code-tree path is a link to `claude/CLAUDE.md`) carries
     the corrected rule.]**
     **[SUPERSEDED IN PART 2026-08-31, F70 §11.189(a) — post-D14 state: every
     tracked CONVERT-set file is pure ASCII (code `cb521cf1`+`71ef30ac`); the
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
     positively mapped both ways (decoy 1 / without 0). **[BLIND TO STAGING BINARIES — measured 2026-09-12, F109 §11.231(j2): all three homes of the probe (`f26_epoch.sh:46`, `f27_reply.py:107`, `f56_canary.sh:470`) test `comm == "spacecrafter"` EXACTLY, and a staging binary's `comm` is its own basename (`sc_f109_iso`, `spacecrafter-pre`…), so the probe read 0 with two staging instances live and holding port 7805; every task that measured a `sc_f*` binary ran it blind. Until the exe-identity probe lands (next round, S): before a launch ALSO assert that no process holds port 7805 (`ss -ltnp`) and that no `/proc/<pid>/exe` resolves under a `sc-f*`/`sc_*` path.]**
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
**Session-26 (2026-09-06): archival pass 18 DONE at open (`e1d6c8c`); live below: the
session-26 mints **F99** (§5.141's fix, both sites), **F100** (§5.139's leg + the barrier
fix), **F101** (§5.143's leg + the old-half assert at the one reader), **F102** (§5.142's
reading, priced).** **Session-26 round (2026-09-07): F99 §11.219 · F100 §11.220 · F101 §11.221 · F102
§11.222 — four for four DELIVERED and accepted; §5.145/§5.146 minted at acceptances; archival pass
19 (update-s25 + F99–F102) DEFERRED to the next open.** **Session-27 (2026-09-07): archival pass 19
DONE at open (`bf6bfbe`); live below: the session-27 mints **F103** (`supervised-by.sh` B1 made
loud + the SHA maps persisted — the tool the owner operates Saturday), **F104** (§5.145's fix on
the corrected ruling: two solver steps per call), **F105** (the dump channel's two items: the
header's system identity + the barrier for the 30 new-only records, with the never-published-frame
precondition).** **Session-27 round (2026-09-07): F103 §11.224 · F104 §11.225 · F105 §11.226 — three for
three DELIVERED and accepted; §5.147/§5.148 minted at acceptances; archival pass 20 (update-s26 +
F103–F105) DEFERRED to the next open.** **Post-close, same day: the owner's [Y8] reply (§11.227) →
F106 §11.228 minted, delivered and accepted (§5.147 FIXED — the rewrite leaves unselected commits
untouched); archival pass 20 = update-s26 + F103–F106.** **Session-28 (2026-09-11): archival pass 20 DONE
at open (`3c38b05`); live below: the session-28 mints **F107** (§11.225(j2)/(j3)'s seed-staleness leg — the
constructor seeds every body at JD 0; no delivered engine change), **F108** (§5.115's fix: uniform bounded
retention, eight launches, every channel, a compiled constant), **F109** (A15's residual step removed on
L1's word — the attributed halo floor α-scaled).** **Session-28 round (2026-09-11/12): F107 §11.229 · F108 §11.230 · F109 §11.231 (a STOP, endorsed — the residual is the instrument's) — three for three DELIVERED and accepted; §5.149–§5.152 minted at acceptances; archival pass 21 (update-s27 + F107–F109) DEFERRED to the next open.** **Session-29 (2026-09-12, Saturday, the owner present): archival pass 21 DONE at open (`1d07413`); the owner's rewrite + push ran mid-warm-up and his replies are §11.232 (a part-time intern without an LLM starts on `master-beta` THIS WEEK — "consolidated first"); live below, dispatch order **F110 → F113 → F114 → F112 → F111** (his second message, §11.233, re-cut the round): **F110** (the intern's first day rehearsed by hand — `doc/developer-entry.md` followed literally from a plain clone on Linux), **F113** (§5.142's fix on his word — the uniform pool to 1 GiB, the D13 reading measured), **F114** (§5.100 + §5.101 on his word — `zoom auto in` tracks and `zoom auto initial` re-aims on the drawn path), **F112** (launch preconditions that see what they guard — the exe-identity instance probe in ONE home for 42 copy-pasted sites, and a GPU-headroom gate derived from the app's init sequence after the 2026-09-12 VRAM red), **F111** (§5.150's fix — the orbit-line sampler gets its own seed, the old path's `batchLastE` shape).** **[12:5x: F113 WITHDRAWN before dispatch (§11.233(b) correction); **F115** (§5.142's ROOT — the uniform consumers censused field by field on both paths, redundancy on three axes, the per-body requirement derived, a proposal with arithmetic; no engine change) and **F116** (§5.115's SIZE bound — 1 GiB total across the five channels, within-session rotation reusing F108's window) minted; order **F110 → F114 → F115 → F116 → F112 → F111**.]**

---

### F110 — THE INTERN'S FIRST DAY, REHEARSED BY HAND: `doc/developer-entry.md` followed LITERALLY from a plain clone of `master-beta` on Linux by a reader with no LLM — every command run as written, every cited path AND every cited commit sha resolved (paths by existence, shas by REACHABILITY in the repository they belong to), the document corrected where the rehearsal measures it misleading him (the smoke-suite/canary paragraph that STOPS on any host but this one; the `c5be42b` citations unreachable since 2026-09-05; the authorship sentence observed before the rewrite), `f85_links.py` taught the class it could not see (a commit citation), and the gaps only the owner can fill (R23's content procedure, §9's placeholder, the SSH remote form) ROUTED to §3 with the doc's own sentences — nothing invented [S–M, code-tree doc + harness (`f85_links.py`, README, `f110_*`); ONE full build from a clone under `/home/claude/sc-f110/` (no install, no `sudo`); the smoke suite on `:2`; FUNCTIONAL, `--no-scene` canary; veto points §3]

**Why now / mandate:** §11.232(a)2 — *"Yes: developer-entry.md is his entry, harness readable"*, *"Linux"*, *"it must be consolidated first"*, and the intern *"will start this week"*; §11.232(c)1 (the channel shift: a human without an LLM writes nothing into this ledger — every answer he needs must be in his reading path) and (c)4 (the four gaps named from the doc's own declared gaps); §11.204 (the doc's genesis, tier R, *"newcomer's 90-second smoke suite"*); the session-22 §3 item (4), ANSWERED today. Decision-free: every claim in the doc is checkable, a correction restores a measured truth, and anything that needs the owner's word is routed, not written.

**The reading the mint stands on [derived; each fact a premise line]:** a plain clone of `master-beta` carries none of `claude/`, `CLAUDE.md`, `.claude/` (`git ls-files` 0 hits); the doc prescribes the second clone at line 30 (`git clone -b CC-harness <same-remote-url> claude`) and the remote is SSH (`git@github.com:lionelruiz13/spacecrafter.git`) — a key or the HTTPS form is the owner's to hand over; line 241 sends him to `sh install_src.sh -j<n>`, which runs `sudo cmake --install` into `/usr/local` (1 hit in the script) — the owner's install lives there (4 entries under `/usr/local/share/spacecrafter`) and the executor NEVER runs that half; line 276 hardcodes `DISPLAY=:2` (this host's display, HOST-EVENTS 2026-09-12) into the smoke-suite command, and `f90_rehearsal_run.sh` runs the canary FIRST and stops on red unless `F90_SKIP_CANARY=1` (its header, line 6) — the canary's bank is PER HOST (`BANK_HOST_BOOT` at `f56_canary.sh:132`, the process epochs beside it), red by construction on his machine, while line 306 tells him *"never mitigated silently, never widened away"*: a reader who cannot bank it meets a STOP with an instruction not to work around it; `c5be42b` is cited three times (lines 261, 362, 398) and is NOT reachable from `CC-harness` (`merge-base --is-ancestor` exit 1) — it was AMENDED into `6ffb017` on 2026-09-05 (session-22 §3, "SHA map in the open note") and that commit was rewritten today into `1e6ca60e` (`sha-maps/…/harness.tsv`); neither event reached the code tree's doc, and `f85_links.py` (129 paths, 25 continuations, 16 ledger ids, 0 dangling) checks no commit sha — a class it cannot see; §9 is a placeholder (line 423) he meets in week 1; the content-installation procedure is undocumented (line 263) and INSTALL §5 says the tree ships NO content — his clone runs EMPTY, which is EXPECTED and must be said to him as such; the PR-target sentence (line 47) stands on the owner's *"master-beta as is"*; the authorship paragraph (lines 415–419) was *"observed 2026-09-05"* — before today's rewrite recorded `Supervised-By` trailers and dropped self co-authors, so it is re-observed, not assumed.

**Measured at dispatch (supervisor, 2026-09-12 11:4x–12:0x, code `fcc277c9`, harness `1d07413`):** the lines above by the PREMISES block; `f85_links.py` green; INSTALL's six sections at lines 11/26/37/47/74/90; `src/CMakeLists.txt:3` `CONFIGURE_DEPENDS`; `/home/claude/sc-f110` absent; canary `--no-scene` exit 0 at 11:26:26 and FULL exit 0 at 11:34:15 on the re-banked boot (`20260912-113229`); VRAM 980 MiB used after the owner unloaded his model — the app needs ≥ ~2.2 GB at init (HOST-EVENTS 2026-09-12).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f110/prediction.txt`, committed before the first clone): for every numbered step the doc's §1, §5 and §8 put a first-day reader through — the two clones, INSTALL §3–4 (build half), `f70_ascii.py gate`, `f85_links.py`, the smoke suite, scedit's `cmake`/`ctest` (nineteen gates, SKIPs expected where inputs are absent — say which), `githooks/install.sh`, the harness README "Run" section's first command — a predicted class: PASSES-as-written / MISLEADS (what he sees, what is true) / BLOCKS (and whether the block is DESIGNED, e.g. no content); plus the reachability of every commit sha the doc cites (`grep -oE` the 7–8-hex tokens), predicted per sha and per repository. (2) **THE REHEARSAL**, under `/home/claude/sc-f110/`, as HE would do it except where his machine differs (each difference stated at the step): `git clone -b master-beta --recurse-submodules /home/claude/spacecrafter code` (the local path stands in for the remote — the SSH form is his key; say so), then the doc's §1 second clone from `/home/claude/spacecrafter/claude`; INSTALL §3–4 through the BUILD only (configure Release, compile with the hook's `-j` after `free -g`; NO `sudo cmake --install`, NO write under `/usr/local`, NO write to `~/.spacecrafter`); every §5 command run AS WRITTEN from the clone with `SC_BIN` = the clone's binary — the smoke suite on `:2` once with the canary (green here) and once as he would have to on his host (`F90_SKIP_CANARY=1`, recording what the skip loses); the scedit block at lines 286–288; the hook install; the README "Run" command; each with its exit code and the first line of what he sees. (3) **MEASURE** what `f85_links.py` cannot: every cited sha's reachability in the clone it belongs to (`git merge-base --is-ancestor`), and the TRUTH (not existence) of the ten `[path:line]` claims a first-day reader acts on — §1 clone, §5 build/smoke, §7's sharp-edge line numbers, §8's hook — by reading the cited lines. (4) **THE CORRECTIONS** — `doc/developer-entry.md` in the code tree, minimal, each with its measurement in the commit message, veto-open: the smoke-suite/canary paragraph for a reader on ANOTHER host (the bank is per host; `F90_SKIP_CANARY=1` and what it loses; `DISPLAY` is his own; the canary is the dispatch desktop's instrument — said plainly); `c5be42b` → the reachable sha, plus ONE sentence that commit citations in this file are not repointed by the harness rewrite tool (`supervised-by.sh` repoints inside the harness repo only — F103/F106) so a reader who finds a dangling sha looks it up in `claude/sha-maps/`; the authorship paragraph re-observed at `fcc277c9`/`16c68276`; anything else the rehearsal measures false. NOT written: R23's procedure, §9's text, the remote form — these are the owner's and go to §3 quoting the doc's own lines. (5) **`f85_links.py` gains the class**: every 7–8-hex commit token in the doc checked by `git merge-base --is-ancestor <sha> <branch>` in the repository the sentence names (harness by default; the code repo when the sentence says so), shown able to fail on the PRE-correction doc (exit 1, `c5be42b` named) and green after; usage line updated. (6) **RECORD:** §11.⟨next⟩ FIRST + stub; §11.204 marked at both homes (the first human rehearsal of the doc — what held, what misled); §11.232(c)4 marked with the measured outcome per gap; the session-22 §3 item (4) and the doc's genesis row in `DEPLOYMENT-MAP.md` R1/R3 if they name the smoke suite or the doc (`grep -c` first); `harness/README.md` F110 section; WIP per §0.6; D14. (7) **THE RESOLVER — §11.233(h), the owner hit this defect TODAY with four of my own pointers:** an inline §5 row is written `142. **…**` with no `§5.` on its line and no `INTENT/5.142.md`, so a reader who searches `5.142` finds only cross-references and one who opens `INTENT/` finds nothing — the same reader the entry document's `Sec.N.M` sentences address. `claude/intent_resolve.py <id>` accepting `Sec.5.142`, `§5.142`, `5.142`, `11.233`: prints the resolution — the entry file path when `INTENT/<id>.md` or `INTENT/archive/<id>.md` exists (the resolution-by-insertion rule), else `INTENT.md:<line>` of the inline row (`^N\. \*\*` within its section's range: §5 rows between `## 5.` and `## 6.`, §11 stubs between `## 11.` and the maintenance marker; §13 rows by `| ID |`), exit 1 when it resolves nowhere; `--self-test` shown able to fail; measured on the six ids of §11.233(h) (at `1d07413`: 5.142 → 508, 5.100 → 426, 5.101 → 428, 5.149 → 515, 5.146 → 512, 5.140 → 506 — re-measured at HEAD, the stub insertion moved nothing above line 520); ONE sentence in the doc's opening paragraph, after the `f85_links.py` sentence (line 13), telling a human how a `Sec.` id resolves — by the resolver, and by hand (`grep -n '^142\. ' claude/INTENT.md`); `f85_links.py`'s own `Sec.` resolution may import it (I2 — one resolver) — say whether it does and why. Whether every inline row should carry its `§5.N` on its own line is a convention change and is NOT this task's — routed to §3.

**Boundaries:** code tree: `doc/developer-entry.md` only (`INSTALL` only if a sentence is measured false — say so, one line); harness: `f85_links.py`, `harness/README.md`, `f110_*` drivers and `artifacts/f110/` (the per-step table with its prediction column, the exit codes, the smoke-suite outputs); NO engine code, NO `sudo`, NO write under `/usr/local` or `~/.spacecrafter` (the smoke suite's farm is private by design — its frozen-four assert stands); the build under `/home/claude/sc-f110/`; FUNCTIONAL launches on `:2` (`--no-scene` canary before the first; the instance assert by `/proc/<pid>/exe` + `ss -ltnp` 7805 — the `comm` probe is blind, §0.5, F112 lands after this task); config/ssystem md5 in == out asserted; explicit timeouts on every call that could exceed 120 s (Q-68); no `run_in_background`; nothing under `/tmp` carries; the intern's identity is not in the tree — the doc addresses "you".

**Discriminating checks:** (a) the local-remote clone + build succeeds; the clone binary's md5 vs `6d63e6c1` stated (a build-path difference is expected — say what differs); (b) the smoke suite exit 0 from the CLONE's harness with `SC_BIN` = the clone's binary, both with the canary and with `F90_SKIP_CANARY=1`; (c) the per-step table's prediction column committed BEFORE the rehearsal, the outcome column after, every mismatch a finding; (d) `c5be42b` unreachable (exit 1) in the harness clone before the correction, the corrected sha reachable (exit 0) after; `f85_links.py`'s new check FAILS on the pre-correction doc and PASSES after — shown able to fail; (e) `git diff` of the doc confined to the measured corrections, each traceable to a rehearsal step; (f) D14 gate green; (g) every command in the doc's §5 has a recorded exit code from the clone.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; canary `--no-scene` exit 0 before the smoke suite; `free -g` available ≥ 16 GiB before the build; VRAM free ≥ 4 GB (`nvidia-smi`) before any launch — the 2026-09-12 red.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the document, re-resolved at HEAD (content drift = abort)
wc -l < doc/developer-entry.md => 431
python3 claude/harness/f85_links.py | tail -1 => DANGLING : 0
git ls-files | grep -cE '^(CLAUDE\.md|\.claude/|claude/)' => 0
grep -n 'git clone -b CC-harness .same-remote-url. claude' doc/developer-entry.md | cut -d: -f1 => 30
grep -n 'DISPLAY=:2 claude/harness/f90_rehearsal_run.sh .absOutdir.' doc/developer-entry.md | cut -d: -f1 => 276
grep -n 'Run the environment canary first' doc/developer-entry.md | cut -d: -f1 => 306
grep -n 'Placeholder -- to be written by Calvin Ruiz' doc/developer-entry.md | cut -d: -f1 => 423
grep -n 'This repository does not document that procedure' doc/developer-entry.md | cut -d: -f1 => 263
grep -n 'pull requests target .2023-master.' doc/developer-entry.md | cut -d: -f1 => 47
grep -n 'sh install_src.sh' doc/developer-entry.md | cut -d: -f1 => 241
grep -c 'c5be42b' doc/developer-entry.md => 3
git -C claude merge-base --is-ancestor c5be42b CC-harness; echo $? => 1
# the instruments and the field around him
grep -n 'F90_SKIP_CANARY=1' claude/harness/f90_rehearsal_run.sh | head -1 | cut -d: -f1 => 6
grep -n '^BANK_HOST_BOOT=' claude/harness/f56_canary.sh | cut -d: -f1 => 132
grep -n '^[0-9]\. ' INSTALL | tr '\n' ' ' => 11:1. What the build needs 26:2. Install the dependencies 37:3. Get the source, WITH the submodule 47:4. Build and install 74:5. The data the repository does NOT contain 90:6. First run
git remote get-url origin => git@github.com:lionelruiz13/spacecrafter.git
grep -c 'sudo cmake --install' install_src.sh => 1
grep -n 'CONFIGURE_DEPENDS' src/CMakeLists.txt | cut -d: -f1 => 3
ls /usr/local/share/spacecrafter 2>/dev/null | wc -l => 4
test -e /home/claude/sc-f110 ; echo $? => 1
```

**DoD:** predictions before the first clone; the rehearsal table (prediction / outcome per step, exit codes); the sha-reachability measurement; the doc corrections with their measurements; `f85_links.py`'s new check shown able to fail; the resolver with its self-test and the six measured ids; §11 entry + stub; the §11.204 / §11.232(c)4 markers; the §3 routing of the owner's three gaps; README; trees clean; WIP cleared; baselines LAST.
**WIP:** — DELIVERED 2026-09-12 → **§11.234** (+ stub). Code `fcc277c9 → d67833cd` (ONE file, `doc/developer-entry.md`, 11 hunks, no engine line; INSTALL untouched — no sentence measured false). Harness `b85dc60 → 6d1aad0` (predictions, pre-clone) `→ 0aed1dc` (the rehearsal) `→ a54f976` (`f85_links.py`'s sha class + `intent_resolve.py`) `→` this delivery. Rehearsal 9/13 rows matched, every mismatch worse for him than predicted; smoke suite exit 0 on BOTH arms from the clone; `f85_links.py` exit 1 on a fresh clone and `ctest` 18/19 are the two that stop him; `c5be42b` → `1e6ca60`; the §11.233(h) six → 508/426/428/515/512/506. Routed to §3 as **[Y5]–[Y9]**: R23, §9, the remote form, the `-O2`-vs-`-Ofast` build finding, the inline-row convention. Gate PASSED (25/25). Predictions pre-registered (`artifacts/f110/prediction.txt`). REHEARSAL RUN, steps S1–S6b + S9–S12: clone rc 0 (EntityCore `84f5d94b` from the REAL remote — §11.204(b) discharged over the network); build half rc 0, clone binary `2dd6a35a`; D14 gate PASS; **`f85_links.py` exits 1 on a fresh clone** (`build-claude/src/spacecrafter` — the documented build makes `build/`); hook install rc 0 (sets `core.hooksPath` in BOTH repos); README "Run" command rc **127** (`xvfb-run` absent); smoke suite rc 0 BOTH arms (armed 12:12:52, `F90_SKIP_CANARY=1` 12:14:44), frozen four in==out, canary `--no-scene` exit 0 12:12:41; `c5be42b` ABSENT as an object in a network-shaped clone, `1e6ca60` reachable; 10/10 cited lines TRUE but `orbit.cpp:600/:606` (doc :376) drifted — the site is `:611`/`:617`. **FINDING: the delivered binary is `RelWithDebInfo` (`-O2 -g`) while the documented build is `Release` (`-Ofast -O3`).** NEXT: S7/S8 scedit cmake+ctest, then the corrections, `f85_links.py`'s sha class, the resolver. **ACCEPTED 2026-09-12 — verifying commands' `date` 12:4x–12:5x (supervisor, session 29, Claude Fable 5.1).** Verified by my own runs and reads: §11.234 read in full; code `d67833cd` (Claude Opus 5, ONE file, the supervising footer), four harness commits `6d1aad0 → 0aed1dc → a54f976 → 758b4fd` interleaved with my two pathspec commits (`d0a6fca`, `b615595` — no file overlap, no clobber), the delivery's trailer `Code: master-beta @ d67833cd`; binary `6d63e6c1` untouched, 0 `src/` files newer; the stub above the marker; markers at §11.204 and §11.232 (entry and stub each) and DEPLOYMENT-MAP R3; §3 [Y5]–[Y9] opening the session-29 block; README F110; `### F` 5. **By my own hand:** `intent_resolve.py` on the six ids → 508 / 426 / 428 / 515 / 512 / 506, `--self-test` PASS, `--self-test --break-scoping` exit 1, an unknown id exit 1; `f85_links.py` at HEAD DANGLING 0 (136 paths / 29 continuations / 19 ids / 3 shas) and on `fcc277c9`'s document DANGLING 1 — `c5be42b` *not an ancestor* — exit 1; D14 gate PASS; instruments **272/340/144 · 250/225/25/124 · D 35 · D2 12 · I 91 · I2 37 · M 93** to the digit. The clone / build / smoke / ctest legs accepted on their committed records (the rehearsal table with its prediction column at `6d1aad0`, 9 of 13 matched and every mismatch worse for him; both smoke arms exit 0 with the frozen four in == out; ctest 18/19 with `anchor_gate`'s four AT-PIN-BROKEN at the rewrite map's line 32). DEVIATIONS ENDORSED with the executor's arguments: the eleven-hunk correction (+98/−24 — each hunk traced to a step, every added sentence carrying its measurement; the document is for a human); `f85_links.py` importing the resolver (I2 — the two copies had already diverged); the `--no-local` re-measurement (a local-path clone copies unreachable objects — a limit recorded for every future rehearsal); the comm-probe sentence NOT written (F112 owns it); `INSTALL` untouched. DISPATCHER-SIDE FINDINGS, ACCEPTED as mine, output-side: (1) the stated pair-check baseline `247/222/25/124 · I 89 · M 91` was the 10:58 measurement quoted AFTER my two mint commits had moved it to `249/224/25/124 · I 91 · M 93` (value class, Q-67's own shape); (2) my verification's footer check grepped `git show` output for `^Co-Authored-By` — the body is indented, the check read 0 on a commit that carries the footer (structure class: a check that could only fail); (3) at this acceptance's write, a script whose guard aborted mid-way was followed by a commit (`bf36530`) whose message described edits the tree did not contain — corrected by the next commit, named there (structure class: a commit chained to a step that could fail without stopping it). Round tally so far: **four** (1 value, 2 structure, 1 intent — the §11.233(b) misread, Q-04's second instance). NEW FINDINGS ROUTED: **[Y8]** `build-claude` is `RelWithDebInfo` (`-O2 -g`) while `install_src.sh` ships `Release` (`-Ofast -O3`) — every D11 cost number this ledger holds was taken on the former; the owner's call, and re-configuring `build-claude` invalidates the canary's band; the scedit `anchor_pin 54a2b844` residue — the rewrite's repoint does not reach the CODE tree's citations (the tool's class gap, queue item 7); `F90_SKIP_CANARY=1` leaves no trace → F112; the document's §5 item 2 (`/proc/<pid>/comm`) → F112 corrects it once the probe lands. STANDING: `/home/claude/sc-f110/` kept (~2 GB); `python3 claude/intent_resolve.py <id>` is the ledger's resolver; `c5be42b` is ABSENT (not merely unreachable) in a network clone.

---

### F113 — §5.142's FIX ON THE OWNER'S WORD (§11.233(b): *"a pool of 1 Gio is fairly generous and won't be critical either, but it should apply on single-session as well"*): THE UNIFORM POOL GROWS FROM 1 MiB TO 1 GiB, FIXED — `app.cpp:274` `1*1024*1024` → `1024*1024*1024` — because the tester's own `06.sts` exhausts 1 MiB at its 675th body and the reference binary then loses the device and ABORTS (§5.142; F98 reproduced it three ways, §11.222 priced the pool), and because the "number of sessions" proxy fails on a year-long run [vixy]; D12: the creation site logs the size, the per-body cost and the reason (one line, the F108 pattern — *"change this constant and rebuild"*); the D13 consequence MEASURED, not argued — `BufferMgr`'s constructor RETURNS on `createBuffer` failure (`BufferMgr.cpp:9`, §5.60's shape), so on a device whose `HOST_VISIBLE|DEVICE_LOCAL` heap is smaller than the pool (no resizable BAR) the outcome is whatever `VulkanMgr::createBuffer` (`VulkanMgr.cpp:706`) does with the PREFERRED flag — read it, state fallback-or-failure with the code lines, and the memory type chosen on THIS device from the app's own log; the proof: F98's arm C (`06.sts` then `14.sts`, 1541 authored bodies) exits 0 with ZERO `Can't allocate buffer` lines on the post binary and aborts on the pre binary, both by the reproduction instrument `artifacts/f98/f98_repro14.py` [S, engine (`app.cpp`: one constant + one log line); FUNCTIONAL launches on `:2`, `--no-scene` canary; the FULL canary after (the pool's memory type must not move on this device — the band to the digit); veto point §3: the NUMBER, if the D13 reading changes it]

**[WITHDRAWN 2026-09-12 12:3x, BEFORE ANY DISPATCH — §11.233(b)'s correction block: the owner's *"pool of 1 Gio"* was the LOG budget, not this pool; his next message — *"app.cpp:274 is the uniform BufferMgr which should stay very small … 1 MiB is sufficient. Resizable BAR is one more reason to keep it small"* — and, asked for a consumer-side shape, *"First - why does the problem exists in the first place ? … why 1344 B allocated … why so many are required ?"*. So this section's mandate (GROW to 1 GiB) is VOID at its root; the section stays as the record of the misread (ids are never reused), its PREMISES block gates nothing, the root question is **F115** and the log budget **F116**. Nothing below this line is dispatched.]**

**Why now / mandate:** §5.142 (OPEN — a hard abort from shipped content, above §5.79's bar in every direction); §11.222(h)(1) (GROW's arithmetic: 2 MiB holds today's corpus with 14 % headroom; the owner chose 1 GiB against the unbounded run, not the corpus); §11.233(b) (the decision, verbatim); §11.232(c)2 (consolidation first — with the intern starting this week, the one thing the tester's corpus does that the reference binary cannot survive). Decision-free now: the number is his; the task changes one constant and measures what the number costs where it costs.

**The reading the mint stands on [derived; each fact a premise line]:** `app.cpp:274` is the ONE creation of the uniform pool (`createBuffer` count 1, §11.222(h)(1)) — `BufferMgr(vkmgr, UNIFORM_BUFFER_BIT, HOST_VISIBLE|HOST_COHERENT, preferred DEVICE_LOCAL, 1*1024*1024, "uniform BufferMgr", uniformBuffer=true)` per the constructor at `BufferMgr.hpp:23`; the constructor calls `master.createBuffer(...)` and on `false` logs *"Failed to create buffer bloc"* and RETURNS with no buffer, no free space and no mapping (`BufferMgr.cpp:8-11`) — the same unguarded shape §5.60 records; when it succeeds the whole block is `mapMemory`'d at creation (`:20`), so a 1 GiB pool is a 1 GiB host-visible MAPPING (virtual; resident only where touched — the RSS and the app's own `GPU memory … used` lines measure the real cost); alignment 64, so a body costs 1344 B on the two paths together ⇒ 1 GiB ≈ 799 000 bodies; the field carries `fscripts/06.sts` and `fscripts/14.sts`; F98's reproduction instrument and its seven result dirs are landed (`artifacts/f98/repro/`); the negative arm is `panorama1.sts` (0/0/exit 0); §5.146 (a refused `SubBuffer` released into the free list) is a DIFFERENT row — a bigger pool makes refusal rarer, it does not fix the allocator (EntityCore, the owner's shape given, §11.233(f)). D13's question is not the size, it is the FAILURE MODE the size reaches on a smaller device (§11.222(h)(1)'s own sentence).

**Measured at dispatch (supervisor, 2026-09-12 12:1x, code `fcc277c9`, harness `1d07413`):** the sites at the lines quoted; EntityCore pinned at `84f5d94b`, read-only; `/home/claude/sc-f113` absent; VRAM 980 MiB used after the owner's unload (32607 MiB total); canary green both arms on the re-banked boot.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f113/prediction.txt`, before any build): pre binary, arm C → 202 loads executed / 1557 buffer errors / 1 device lost / exit −6 (§5.142's numbers, same instrument); post binary → all 1541 loads, 0 `Can't allocate buffer` lines, exit 0 on `shutdown action now`; the applog's `GPU memory … used` delta at the pool's creation ≈ +1024 MiB and the memory-type index it lands on (from `vulkaninfo`'s table, §11.222's artifact); the RSS delta predicted from the mapping's touch pattern (state the model: mapped-not-touched vs touched); the D13 reading's PREDICTION from `VulkanMgr::createBuffer`'s code — fallback to a non-device-local `HOST_VISIBLE` type, or creation failure — BEFORE any run; F91 `1fe630a4` unchanged; the canary band unchanged (uniform memory is not photometry; the type unchanged on this device). (2) **THE FIX:** the constant, plus ONE D12 line at creation naming the size, the per-body cost (1344 B) and the reason (§5.142, the owner's 1 GiB, *change this constant and rebuild*); `BufferMgr`'s silent return is NOT touched (EntityCore; §5.60 stays OPEN and is marked with the widened reach). NOTHING else moves — assert by diff. (3) **THE PROOF:** arm C on the pre binary (the delivered `6d63e6c1`, kept under `/home/claude/sc-f113/`) and on the post binary — the F98 shape: the played shows COPIED to a farm (the annotator rewrites a played `.sts` in place; the frozen four in == out asserted), `f98_repro14.py` as the driver; the negative arm; the smoke suite; `f91_run.sh --expect post --locale fr` at `1fe630a4`; the FULL canary after (band to the digit, memory type asserted from the applog); the D11 cost: 0 per frame, the creation-time ms measured once. (4) **THE D13 READING:** `VulkanMgr::createBuffer` at `:706` — what happens when the preferred `DEVICE_LOCAL` type's heap cannot hold 1 GiB: a fallback to a non-device-local host type (then uniform reads cross PCIe — a D11 note, not a failure) or a creation failure (then the app runs with NO uniform pool and every body fails to load — §5.142's abort at body ONE): stated with the code lines; if it is FAILURE, the §3 veto point says so with the number that would fit a 256 MiB BAR heap (e.g. 128 MiB ≈ 100 000 bodies) — the OWNER's call, not the executor's; if it is fallback, the veto point says what the fallback costs. (5) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.142 → FIXED (the pool) with the D13 reading in the marker; §11.222(h) marked at both homes (GROW chosen, the number, the reading); §11.218(g)/(m) marked; §5.60 annotated (the same unguarded return, reach widened by the grown pool); §5.146 cross-referenced (unchanged); the §3 veto point; `harness/README.md` F113 section; WIP per §0.6; D14.

**Boundaries:** `src/appModule/app.cpp` (`:274` + the log line); NO EntityCore change (read-only — §5.146 and §5.60 stay open); no data; FUNCTIONAL launches on `:2` (`--no-scene` canary before the first; the instance assert by `/proc/<pid>/exe` + `ss -ltnp` 7805; config/ssystem md5 in == out; the shows played from a farm copy); the FULL canary once, after; explicit timeouts on every call that could exceed 120 s (Q-68); no `run_in_background`; nothing under `/tmp` carries; scratch under `/home/claude/sc-f113/`.

**Discriminating checks:** (a) pre ABORTS (202/1557/−6) and post SURVIVES (1541/0/0) on the same shows through the same instrument — the fix shown able to fail; (b) the applog's creation-time memory delta ≈ 1 GiB and the memory type's heap stated; (c) the negative arm unchanged; (d) the FULL canary band to the digit, F91 byte-identical, the smoke suite green; (e) the D13 reading with code lines, predicted before any run; (f) the diff = one constant + one log line; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; canary `--no-scene` exit 0 before the first launch; VRAM free ≥ 4 GB (`nvidia-smi`) before any launch; F110 DELIVERED (its smoke suite ran on the pre binary).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the pool's one creation and the allocator's silent return, re-resolved at HEAD (content drift = abort)
grep -n 'context.uniformMgr = std::make_unique.BufferMgr.(vkmgr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1\*1024\*1024, "uniform BufferMgr", true);' src/appModule/app.cpp | cut -d: -f1 => 274
grep -c '1\*1024\*1024, "uniform BufferMgr"' src/appModule/app.cpp => 1
grep -n 'Failed to create buffer bloc' src/EntityCore/Core/BufferMgr.cpp | cut -d: -f1 => 9
grep -n 'bool uniformBuffer = false' src/EntityCore/Core/BufferMgr.hpp | cut -d: -f1 => 23
grep -n 'bool VulkanMgr::createBuffer' src/EntityCore/Core/VulkanMgr.cpp | cut -d: -f1 => 706
git -C src/EntityCore rev-parse --short=8 HEAD => 84f5d94b
# the reproduction instrument, the field's two shows, the ledger
test -f claude/harness/artifacts/f98/f98_repro14.py && echo ok => ok
ls claude/harness/artifacts/f98/repro | wc -l => 7
ls ~/.spacecrafter/scripts/fscripts/06.sts ~/.spacecrafter/scripts/fscripts/14.sts | wc -l => 2
grep -n '^142\. \*\*' claude/INTENT.md | head -1 | cut -d: -f1 => 508
grep -c 'GROW' claude/INTENT/11.222.md => 2
nvidia-smi --query-gpu=memory.total --format=csv,noheader => 32607 MiB
test -e /home/claude/sc-f113 ; echo $? => 1
```

**DoD:** predictions before any build; the constant + the D12 line; arm C pre/post through the landed instrument; the D13 reading with its code lines and the §3 veto point; F91, the smoke suite, the full canary; §11 entry + stub; §5.142 FIXED; the §11.222 / §11.218 / §5.60 markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** WITHDRAWN 2026-09-12 12:3x before dispatch — see the header note; archived as the record of a dispatcher misread at the next pass.

---

### F114 — §5.100 + §5.101's FIX ON THE OWNER'S WORD (§11.233(d): *"yes, same as for any kind of tracking in spacecrafter"*): `zoom auto in` STARTS THE NEW PATH'S TRACKING beside old's — `Camera::instance->trackBody(…)` (the API at `Camera.hpp:386`) at the site of `navigation->setFlagTraking(true)` (`core.cpp:1393`), next to the B17 `armViewOffset(true)` mirror already there (`:1402`) — and `zoom auto initial` RE-AIMS THE DRAWN VIEW where old calls `navigation->moveTo(InitViewPos, move_duration, true, -1)` (`:1439`, `:1485`): `Camera::instance->lookTo(<init direction in the camera's frame>, move_duration)` (the API at `Camera.hpp:83`, eased over `duration`, so the snapped-vs-eased question §5.101 raised has its answer in the existing ramp — the END state is the parity target, the ramp the perceptual class, the §11.92/B34 precedent), the frame conversion being the one `SSystemFactory::loadCamera` (`ssystem_factory.cpp:154`) performs for `init_view_pos` at startup, REUSED not re-derived; the census of the remaining old-only `setFlagTraking` writes (§5.100 owes it: `core.cpp:399` the init reset) done with the fix; the PRELOAD signal the owner named (§11.233(d)) recorded as a follow-on row at acceptance, not built here [S–M, engine (`core.cpp` two sites; `Camera.{hpp,cpp}` only if the init direction needs a callable conversion), FUNCTIONAL launches on `:2`, `--no-scene` canary; F38's `f38_gaps.py` re-run pre/post as the proof; veto points §3]

**Why now / mandate:** §5.100 and §5.101 (OPEN since 2026-08-26 — *"Recommendation carried, decision not taken"*, the authorization asked at session 13 and unanswered until today); DEPLOYMENT-MAP T1.1 — *"bread-and-butter operator commands — the single largest transparency hole with a one-line-class fix already scoped"*, the LAST operator-basics decision on the map; §11.233(d) (the word, verbatim); D15(c) (*"continual tracking must be preserved"*); §11.232(c)2 (consolidation first). Decision-free now: the owner's word is the semantics (tracking like any tracking); the code is the existing Camera API at two sites.

**The reading the mint stands on [derived; each fact a premise line]:** `Core::autoZoomIn` (`core.cpp:1386`) writes `navigation->setFlagTraking(true)` at `:1393` and moves old's view with old's OWN `move_duration`; the dual setter `Core::setFlagTracking(true)` cannot be used there because it takes `getAutoMoveDuration()` — changing old's duration is forbidden by construction (§11.52(b); §5.100's own sentence); the B17 mirror `Camera::instance->armViewOffset(true)` at `:1402` proves the seam is already a dual site. `Core::autoZoomOut` (`:1429`) re-aims old with `navigation->moveTo(InitViewPos, move_duration, true, -1)` at BOTH branches (`:1439` manual, `:1485` full) and already mirrors the DISABLE halves (`setFlagTracking(false)`, `setFlagLockSkyPosition(false)`, `armViewOffset(false)`) — F38's work; only the AIM half is old-only. The Camera API: `trackBody(ModularBody *)` `:386` / `trackBody(nullptr_t)` `:383`; `lookTo(const Vec3f &direction, float duration, bool isMaxDuration)` `:83` and `lookTo(alt, az, duration, …)` `:84` — a ramp exists. F38's instrument `f38_gaps.py` measured the two gaps on `select planet Mars` + `zoom auto in duration 0` (old `flagTraking = 1`, `camera.tracked = ''`, old Mars at (1024, 1024), the new path aimed 0.2056 rect units off at a 0.0023° field) and `zoom auto initial duration 0` from a tracked Mars (old moves 99.0318°, new 0.0214° = sidereal drift) — its 20 landed artifacts are the pre-fix record; re-run on the post binary it is the proof both ways. `InitViewPos` is Core's member in old's frame; §5.101 records the conversion `(x, y, z)_old → (y, −x, z)_camera` at `loadCamera` — the executor re-reads `ssystem_factory.cpp:154+` for the exact form and makes it ONE home if it is not already callable (I2).

**Measured at dispatch (supervisor, 2026-09-12 12:1x, code `fcc277c9`, harness `1d07413`):** the sites at the lines quoted; `core.cpp` holds 4 `trackBody|lookTo` mentions today (the executor classifies them — none at the two sites); `f38_gaps.py` present, 20 artifacts under `artifacts/f38/gaps/`; `/home/claude/sc-f114` absent; canary green both arms.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f114/prediction.txt`, before any build): on the post binary, `f38_gaps.py`'s two legs — after `zoom auto in duration 0`: `camera.tracked = 'Mars'`, Mars at the new path's screen centre within the A/A floor while old still reads (1024, 1024); after `zoom auto initial duration 0` from a tracked Mars: the NEW look direction moves by old's angle (the 99.03° class in that scene) and the two paths' Mars screen positions agree within the float floor; `camera.tracked` cleared by the unzoom-to-init (the DISABLE half is already dual — assert it); the fov halves unchanged (§11.40's mirror); with `duration 1`, the END state after the ramp agrees within the same floor and the per-frame divergence along the ramp is MEASURED and stated, not gated (the B34 shape); the census of old-only `setFlagTraking` writes (grep) with each site classified (`:399` the init reset, coherent by `loadCamera`'s clear). (2) **THE FIX:** the two sites; the init direction's conversion read at `loadCamera` and REUSED — a helper only if the conversion is not callable today (one home, I2); comments cite §5.100/§5.101, §11.233(d), D15(c). NOTHING on the old side moves (`navigation->…` lines untouched by construction) — assert by diff. (3) **THE PROOF:** `f38_gaps.py` on the pre binary (the gaps reproduced to the digit) and on the post binary (closed); the ramp leg at `duration 1`; F38's four sky-lock sites unchanged (`f38_mirror.py` green); `f91_run.sh --expect post --locale fr` at `1fe630a4`; the smoke suite (its keyboard-ramp step included); the per-step parity instrument (`f25_ramp.py`) reused if it reaches this ramp — say if not. (4) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.100 and §5.101 → FIXED; §11.150(k) marked at both homes; DEPLOYMENT-MAP T1.1 struck-with-record; the PRELOAD signal → `FEATURE_REQUESTS.md` (the owner's words, §11.233(d)) and a §13.B candidate line, both at acceptance by the supervisor from the entry; `harness/README.md` F114 section; WIP per §0.6; D14.

**Boundaries:** `src/coreModule/core.cpp` (the two sites), `src/experimentalModule/Camera.{hpp,cpp}` only for a conversion helper; NO old-path change; no data, no EntityCore; FUNCTIONAL launches on `:2` (`--no-scene` canary before the first; the instance assert by `/proc/<pid>/exe` + `ss -ltnp` 7805; config/ssystem md5 in == out); explicit timeouts (Q-68); no `run_in_background`; nothing under `/tmp` carries; scratch under `/home/claude/sc-f114/`.

**Discriminating checks:** (a) `f38_gaps.py` pre (gaps) / post (closed) — the fix shown able to fail; (b) `camera.tracked` set by `zoom auto in` and cleared by `zoom auto initial`; (c) end-state parity within the float floor after the eased ramp, the ramp's divergence stated; (d) F38's four sites green, F91 byte-identical, the smoke suite green; (e) the diff confined to the two sites (+ the helper if any); (f) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; canary `--no-scene` exit 0 before the first launch; VRAM free ≥ 4 GB before any launch; F113 DELIVERED (its binary is this task's pre — the pool change is inert for this scene, say so with the applog).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the two sites, the mirrors beside them and the Camera API, re-resolved at HEAD (content drift = abort)
grep -n '^void Core::autoZoomIn' src/coreModule/core.cpp | cut -d: -f1 => 1386
grep -n '^void Core::autoZoomOut' src/coreModule/core.cpp | cut -d: -f1 => 1429
grep -n 'navigation-.setFlagTraking(true);' src/coreModule/core.cpp | cut -d: -f1 | tr '\n' ' ' => 1393
grep -n 'navigation-.moveTo(InitViewPos, move_duration, true, -1);' src/coreModule/core.cpp | cut -d: -f1 | tr '\n' ' ' => 1439 1485
grep -n 'Camera::instance-.armViewOffset' src/coreModule/core.cpp | cut -d: -f1 | tr '\n' ' ' => 1402 1454 1492 2497 2639
grep -n 'void trackBody\|void lookTo\|void lookAt\|void track(' src/experimentalModule/Camera.hpp | cut -d: -f1 | tr '\n' ' ' => 83 84 383 386
grep -c 'trackBody\|lookTo' src/coreModule/core.cpp => 4
grep -n 'void SSystemFactory::loadCamera' src/bodyModule/ssystem_factory.cpp | cut -d: -f1 => 154
# F38's instrument and record; the ledger rows
test -f claude/harness/f38_gaps.py && echo ok => ok
ls claude/harness/artifacts/f38/gaps | wc -l => 20
grep -n '^100\. \*\*' claude/INTENT.md | head -1 | cut -d: -f1 => 426
grep -n '^101\. \*\*' claude/INTENT.md | head -1 | cut -d: -f1 => 428
test -e /home/claude/sc-f114 ; echo $? => 1
```

**DoD:** predictions before any build; the two sites (+ the one-home conversion); `f38_gaps.py` pre/post; the ramp leg; F38's sites, F91, the smoke suite; §11 entry + stub; §5.100 + §5.101 FIXED; the §11.150 marker; T1.1 struck-with-record; the preload signal recorded; README; trees clean; WIP cleared; baselines LAST.
**WIP:** — DELIVERED 2026-09-12 → **§11.235** (+ stub). Code `d67833cd → d0e0c51e` (ONE commit, three files: `core.cpp` the two sites, `Camera.hpp` the conversion helper `oldLocalToLocal`, `ssystem_factory.cpp` rerouted to it — one home, I2); binary `6d63e6c1 → d607cfdc`. Harness `c9a8764 → 2a2c432` (gate + predictions before any build) `→ a95c193` (the pre leg) `→ f041fbc` (post + ramp) `→ 47450bc` (regressions + the farm leg) `→` this delivery. **§5.100 + §5.101 FIXED** at their inline rows with closing blocks; §11.150(k) marked at BOTH homes with its own correction; DEPLOYMENT-MAP **T1.1 struck-with-record at four homes** (+ B18's row and the §11.162 line annotated by the row-flip rule); `harness/README.md` F114 section. RESULTS: `f38_gaps.py` UNCHANGED pre/post — `camera.tracked` `'' → 'Mars'`, **99.0336° → 8.00911e-06°**; `zoom auto initial` **99.0318° → 1.0102e-05°**; the drawn path's Mars unfreezes (`visible false → true`) and rides centre across the 0.05 d advance; old's Mars still pinned at (1024.0000, 1024.0000), the old side byte-identical on five of six scenes. The RAMP predicted in closed form BEFORE the build and measured at `duration 1` AND `10`: max divergence **57.667°** vs **57.596°** predicted, end state **1.0e-05°**, old's only residual its own one-frame start quantization; `f25_ramp.py` does NOT reach this ramp (said, with the guard that proves it). The conversion tested where the field config cannot test it (farm at `init_view_pos = 1,0,0`, three candidates 90° apart): **7.19e-06°**, the camera's (alt,az) back to `loadCamera`'s exact startup pair. The OTHER `autoZoomOut` branch reached by its own leg (20 fov doublings under `manual_zoom`). `f38_mirror.py` ALL PASS 6/6 (S4 passes THROUGH the changed branch), F91 table `1fe630a4` byte-identical, smoke rc 0 FAIL none, D14 PASS at every commit, field pair in == out around all seven launches. REPORTED, not absorbed: the section's *"F113 DELIVERED"* precondition is FALSE (withdrawn) — reclassified non-input per §11.179(a) with the counterfactual; §5.100/§11.150(k)'s *"coherent by accident"* for `core.cpp:399` is CORRECTED to by-construction; one prediction REFUTED and corrected at its first re-statement (the frozen `screen` is `ModularBody::update` not running for a culled body — the `lastJD` lag is Mars's light time, identical on both binaries). TWO RESIDUALS recorded not fixed, §5 candidates for the supervisor: the camera's **0.2 s view-plan floor** (a commanded `duration 0.1` dumps `viewT = 0.2000`; the paths are **38.35°** apart when old lands) and **tracking is body-only on the drawn path** (the star leg is inconclusive on this host, said so). The PRELOAD signal's anticipation points are enumerated in §11.235(l) with the anchor-first reading: the anchor is `Camera::trackBody(ModularBody *)` itself, not its callers. **ACCEPTED 2026-09-12 — verifying commands' `date` 13:51–13:53 (supervisor, session 29, Claude Fable 5.1).** Verified by my own runs and reads: §11.235 read in full; code `d0e0c51e` (Claude Opus 5, ONE commit, three files +61/−4, the supervising footer) — the diff READ line by line: zero changed lines holding `navigation->`, `ssystem_factory.cpp`'s one line is `camera->lookTo(Camera::oldLocalToLocal(v), 0)` (new-path code, the same vector), both sites inside the existing `if (Camera::instance)` guards; seven harness commits `2a2c432 → c0ae9bf` (the predictions at 12:59, before the first compile), the delivery's trailer `Code: master-beta @ d0e0c51e`; binary `6d63e6c1 → d607cfdc`, 0 `src/` files newer, the pre kept at `/home/claude/sc-f114/spacecrafter-pre-6d63e6c1` (md5 re-measured `6d63e6c1`); the stub at `INTENT.md:1104`; §5.100/§5.101 FIXED at their inline rows; §11.150(k) marked (entry + stub); DEPLOYMENT-MAP T1.1 struck-with-record; README F114; `### F` 7. **By my own hand:** `f91_run.sh <outdir> --expect post --locale fr` on `d607cfdc` at 13:52 — **0 FAIL 0 NOTE, table `1fe630a4`**, the pair `03fbee59`/`545a51ef` in == out, canary `--no-scene` exit 0 before it (my first invocation passed `--expect` as the outdir and read the script's own usage refusal as a red — an instrument mis-use of mine, corrected by reading its usage line); instruments **273/342/144 · 251/226/25/124 · D 35 · D2 12 · I 92 · I2 37 · M 95** to the digit of (o). The gaps / ramp / farm / mirror / smoke legs accepted on their committed both-ways records (the pre run reproducing 2026-08-26's 99.0336 / 99.0318 to four decimals; the ramp's two laws committed before the build and measured 57.667 against 57.596; the farm leg at a non-zenith init direction able to fail three distinguishable ways). DEVIATIONS ENDORSED with the executor's arguments: `ssystem_factory.cpp` rerouted through the helper (the mandate's own "one home", I2; new-path code, the STOP condition does not fire); the three extra legs (read-only on the delivered binary); the artifact policy; the digit-exact / band split. THE REFUTED PREDICTION KEPT (`lastJD` lags by Mars's light time in BOTH binaries; what freezes is `screenPos` inside the culled body's skipped `update`) — the commit message stands, the entry corrects it. DISPATCHER-SIDE FINDINGS, ACCEPTED as mine, output-side: (1) the prose precondition *"F113 DELIVERED (its binary is this task's pre)"* was STALE — F113 was withdrawn at 12:3x and the withdrawal's same-class sweep missed the section that cited it (structure class; the executor's non-input reclassification is correct, the binary being pinned by the PREMISES md5 line); (2) the scan baseline stated as *"measured at `c9a8764` 12:48"* was measured at `758b4fd` before my two commits added event lines — a MEASURED label on a value my own later writes had moved, the Q-67 sub-class recorded in this very session and not executed at the next dispatch (value class); (3) `core.cpp:399` *"coherent by accident"* → by construction, corrected by the executor at three homes (structure class, from the mint's reading of §5.100). Round tally: **seven** (2 value, 4 structure, 1 intent). MINTED AT ACCEPTANCE: **§5.153** (the drawn path's 0.2 s view-plan floor, from (j)1 — new-path-only, reachable from `duration` in (0, 0.2) s, 38.35° apart at old's landing; the owner's perceptual call); (j)2 (body-only tracking on the drawn path) recorded as a CANDIDATE at §5.100's row, its measurement blocked on this host's catalogue state; the PRELOAD signal → `FEATURE_REQUESTS.md` 2026-09-12 + **B41** in §13.B, carrying (l)'s anchor-first reading (`Camera::trackBody` is the tracked half's anchor; the zoom half has none). STANDING: the delivered binary is **`d607cfdc`** (F116's pre; F115 is read-only); `harness/f114_run.sh` carries the exe/port/VRAM launch asserts for any launch before F112 lands.

---

### F115 — §5.142's ROOT, ON THE OWNER'S QUESTION (§11.233(b) correction, verbatim: *"First - why does the problem exists in the first place ? Two aspects, why 1344 B allocated (which is not small, let make sure there is no redundancy), why so many are required ? Increasing it blindly to 16 Mio can work, but it would break again with 16 times as many bodies."*): THE UNIFORM CONSUMERS CENSUSED FIELD BY FIELD ON BOTH PATHS — every persistent `SharedBuffer<T>` a BODY holds on `context.uniformMgr`, per body class and per module (old path: `body_moon/bigbody/smallbody/artificial/sun`, `ring`, `axis`, `atm_ext`; new path: `BasicMesh.hpp:47-48`, `LayeredMesh.hpp:122-126`, `meshShadowFill.hpp:45/:78`, `OjmModule.hpp:100-103`; F102's 28-row table `artifacts/f102/sizes.tsv` with sizeof and the carved size at alignment 64 — `meshFrag` 784 → 832, `globalVertProj` 164 → 192, `ojmShadowBlock` 928 → 960 …) — REDUNDANCY measured on three axes: cross-PATH (one authored body carries the OLD path's eager 320 B AND the NEW path's 1024 B while both paths live, §11.222(h) — a cost B8 retires, or one to retire now?), cross-MODULE (one quantity carved in two blocks of the same body — a ModelView matrix, a clipping/fov triple), cross-BODY (content identical for every body yet carved per body — `shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER=8]` is 768 of `meshFrag`'s 784 B, `bodyShaderInterface.hpp:47/:71`; light info; per-system values); the per-body REQUIREMENT derived from what actually varies per body per frame (a model matrix, a few floats) against what is per-draw-constant, per-system or per-launch; WHY every AUTHORED body needs a block at all (carved at load on the new path; two of the old path's three classes carve at first draw); and a PROPOSAL with arithmetic — bytes per body under each option × the corpus (1719 authored bodies in 8 shows, §11.218(i)) and the body count 1 MiB then holds — for the owner to choose from; NO engine change, the pool STAYS 1 MiB, no option enacted [S–M, read-only + one compile-time size instrument (F102's `f102_sizes.cpp` extended to the full set); ZERO launches unless a live dump is the cheapest census of which bodies carve what — then FUNCTIONAL, `--no-scene`, one launch, said so; veto points §3]

**Why now / mandate:** the owner's question verbatim (§11.233(b) correction, 2026-09-12 12:4x) — asked instead of a shape, so a shape chosen without it is exactly what he refused (*"Increasing it blindly to 16 Mio can work, but it would break again with 16 times as many bodies"*); §5.142 (OPEN — the abort at the 675th body of `06.sts`); §11.222(h) (GROW / REFUSE / DEGRADE priced; the 1344 B measured; DEGRADE's own analysis names `shadowingBodies[8]` as 98 % of one block); §11.232(c)2 (consolidation first — the reference binary must survive the tester's corpus before the intern builds on it). Decision-free as a reading: it produces numbers and options; the choice stays his.

**The reading the mint stands on [derived; each fact a premise line]:** 74 files outside EntityCore acquire a `SharedBuffer<T>` (the census pattern); the per-BODY ones are the old path's body classes and `ring`/`axis`/`atm_ext` (15 files under `src/bodyModule/`), and the new path's mesh modules (`BasicMesh.hpp:47-48`, `LayeredMesh.hpp:122-126` + `.cpp:53/:64/:65`, `meshShadowFill.hpp:45/:78`) and `OjmModule.hpp:100-103` (in-galaxy models: `ojmVert/ojmGeom/ojmLight/ojmShadowBlock`) — everything else on the pool is per-system or per-feature (fog, landscape, sky grids, tully, oort, the sun's halo scalars, the shadow service) and counts once, not per body; F102 measured 1344 B per `06.sts` body on the two paths together (320 old-eager + 1024 new-at-load) and the alignment (64) that turns 784 B into 832 carved; `bodyShaderInterface.hpp` declares `shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER]` (5 mentions) with the constant shared by the fragment shaders — so a per-body cap is a shader array-length change, not a C++ edit alone; the old path carves at first draw for `BigBody`/`SmallBody` and at construction for `Moon` (§11.222(h)(3), `body_moon.cpp:78`); F102's `f102_sizes.cpp` links nothing and prints sizeof + carved size for its 28 structs — the instrument to extend.

**Measured at dispatch (supervisor, 2026-09-12 12:5x, code `d67833cd`, harness after F110's acceptance):** the sites at the lines quoted; §5.142 at `INTENT.md:508` (the resolver agrees); `/home/claude/sc-f115` absent; no launch class unless (2)'s dump is chosen.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f115/prediction.txt`): the census's expected shape (files, row count per path, the 1344 B reproduced as a sum BEFORE the census is extended), the predicted redundancy per axis from the reading — which quantities are duplicated cross-path, cross-module, cross-body — and the predicted per-body minimum in bytes. (2) **THE CENSUS:** every `SharedBuffer<T>` acquired on `context.uniformMgr` by a per-body object on either path — `file:line`, `T`, `sizeof(T)`, carved size (the F102 instrument extended, numbers pasted, the 28 landed rows reproduced byte for byte), LIFETIME (carved at load / at first draw / per frame; released when), and for every FIELD of `T` its variability class: per body per frame · per body constant · per system · per launch · identical for all bodies; if a live dump is the cheapest way to see which bodies carve what on the field's 120 records and on `06.sts`, ONE functional launch. (3) **REDUNDANCY, three axes, each claim with its two `file:line` homes:** cross-path (which fields the old and new blocks of the same body both carry; the bytes B8 retires; whether anything is carved for a body the drawn path never draws), cross-module (the same quantity in two of one body's blocks), cross-body (fields whose content is identical across bodies at a frame — `shadowingBodies[8]` first — and what carving them once would need: a per-system block, a push constant, an indexed buffer). (4) **THE REQUIREMENT:** the minimum a body must own per frame, in bytes, with its fields; the body count 1 MiB holds at that minimum; what the owner's *"why so many are required"* answers to — the count of bodies that hold a block at load vs the count drawn in a frame (measured on `06.sts` if the dump is run: authored 1013, drawn N). (5) **THE PROPOSAL:** options with arithmetic — bytes per body, bodies per MiB, the wall on `06.sts` + `14.sts` (1541), what changes in shaders, what changes in EntityCore (his stratum, named not touched), what each costs per frame under D11 — e.g. carve at first draw (the old path's own shape), cap the receiver array with the shader following it, move all-bodies-identical fields to a per-system block, retire the cross-path double with B8; NO recommendation ranked without the numbers, and the numbers before the ranking. (6) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.142 annotated with the root's numbers (no state change — OPEN, the choice his); §11.222(h) marked at both homes; §11.233(b) marked; `harness/README.md` F115 section; WIP per §0.6; D14.

**Boundaries:** READ-ONLY on the engine (no code change, no build of the app); the size instrument is a standalone `g++` compile under `artifacts/f115/` in F102's style; at most ONE functional launch (`--no-scene` canary before it; the exe+port instance assert; the frozen pair in == out; the played show from a farm copy) and only if said why; no EntityCore change; no option enacted; explicit timeouts; no `run_in_background`; nothing under `/tmp` carries; scratch under `/home/claude/sc-f115/`.

**Discriminating checks:** (a) the extended instrument reproduces F102's 28 rows byte for byte and the 1344 B sum before any new row is trusted; (b) every redundancy claim carries two `file:line` homes and a byte count; (c) the per-body minimum stated with its fields, and the body count 1 MiB holds at it; (d) the proposal's arithmetic reproduces today's wall (675th body of `06.sts`) and states each option's wall; (e) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; if a launch is chosen: `:2` per HOST-EVENTS 2026-09-12, canary `--no-scene` exit 0, VRAM free ≥ 4 GB.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the consumers, the shader array and F102's instrument, re-resolved at HEAD (content drift = abort)
grep -rl 'SharedBuffer<' src --include=*.cpp --include=*.hpp | grep -vc '^src/EntityCore' => 74
grep -n 'SharedBuffer<' src/experimentalModule/bodyModules/OjmModule.hpp | cut -d: -f1 | tr '\n' ' ' => 100 101 102 103
grep -rn 'SharedBuffer<' src/experimentalModule/meshModules/*.hpp src/experimentalModule/meshModules/*.cpp 2>/dev/null | cut -d: -f1,2 | tr '\n' ' ' => src/experimentalModule/meshModules/BasicMesh.hpp:47 src/experimentalModule/meshModules/BasicMesh.hpp:48 src/experimentalModule/meshModules/LayeredMesh.hpp:122 src/experimentalModule/meshModules/LayeredMesh.hpp:123 src/experimentalModule/meshModules/LayeredMesh.hpp:124 src/experimentalModule/meshModules/LayeredMesh.hpp:125 src/experimentalModule/meshModules/LayeredMesh.hpp:126 src/experimentalModule/meshModules/meshShadowFill.hpp:45 src/experimentalModule/meshModules/meshShadowFill.hpp:78 src/experimentalModule/meshModules/LayeredMesh.cpp:53 src/experimentalModule/meshModules/LayeredMesh.cpp:64 src/experimentalModule/meshModules/LayeredMesh.cpp:65
grep -rln 'SharedBuffer<' src/bodyModule/*.cpp src/bodyModule/*.hpp | wc -l => 15
grep -n 'MAX_SHADOW_CASTERS_PER_RECEIVER' src/experimentalModule/meshModules/bodyShaderInterface.hpp | head -2 | cut -d: -f1 | tr '\n' ' ' => 47 71
grep -c 'shadowingBodies' src/experimentalModule/meshModules/bodyShaderInterface.hpp => 5
test -f claude/harness/artifacts/f102/sizes.tsv -a -f claude/harness/artifacts/f102/f102_sizes.cpp -a -f claude/harness/artifacts/f102/pool_baseline.json && echo ok => ok
wc -l < claude/harness/artifacts/f102/sizes.tsv => 28
grep -c '1344' claude/INTENT/11.222.md => 5
grep -c 'DEGRADE' claude/INTENT/11.222.md => 1
grep -n '^142\. \*\*' claude/INTENT.md | head -1 | cut -d: -f1 => 508
test -e /home/claude/sc-f115 ; echo $? => 1
```

**DoD:** predictions first; the census with lifetimes and field classes, the 28 rows reproduced; the three redundancy axes with homes and bytes; the per-body minimum; the proposal with arithmetic and no enacted option; §11 entry + stub; §5.142 / §11.222 / §11.233 markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** — DELIVERED 2026-09-12 → **§11.236** (+ stub). Code `d0e0c51e` **UNTOUCHED** (read-only task: no engine change, no app build, binary `d607cfdc` at open and close, EntityCore read-only). Harness `1890e47 → cc28d95` (gate + predictions, before any struct was read) `→ 98ba1e6` (the instrument + corpus census) `→ 3019188` (the three axes + the requirement) `→` this delivery. **ZERO LAUNCHES**, criterion pre-registered. Gate PASSED 17/17. **THE ANSWER TO THE OWNER'S TWO HALVES:** *why 1344 B* — **884 of the 1180 `sizeof` bytes a `06.sts` body holds are redundant** in a measured sense: cross-PATH **320 B** (192 of it the literally identical `globalVertProj`, same seven quantities from two files; plus `atm_ext.hpp:20-30` and `AtmExtModule.hpp:76-86` proved FIELD-IDENTICAL by instrument), cross-MODULE **448 B** over 8 fields that are a pure function of a neighbour field plus **one 768 B receive array embedded in FOUR blocks** (1536 B on a ray-capable body, 2304 B on a ringed one, one fill source), cross-BODY **808 of the new path's 948 B (85 %)**; *why so many are required* — **they are not**: acquire in the constructor, fill in `draw`, so a never-drawn body holds **1344 carved bytes containing 4 written ones**, and on `06.sts` that is all 1013. **REQUIREMENT 88 B payload → 128 B carved → 8192 bodies/MiB** (corpus 0.209 MiB). **OPTIONS PRICED, NONE RANKED:** B8 alone walls at body **885**, a cap of 4 (the code default) at **944**, cap 1 → 640 B no wall (2 `#define`s, but VISIBLE — `ModularSystem.cpp:492` truncates, so D8 does not cover it), array-out-of-block → EntityCore/HIS, +per-system block → the minimum, lazy carve → F102's objection unsoftened, REFUSE → the only scale-free one. Growing the pool excluded by his own word. Check (a) FULL: the extended instrument's first 28 `--tsv` lines **diff EMPTY** against `artifacts/f102/sizes.tsv`, reached through the REAL `bodyShader.hpp` (it compiles standalone) so F102's copies are independently checked; `f115_mirrors.py` 4/4 + self-test. §11.218(i)'s 1719 and §11.222(h)'s 1183/531/5 and 1.581 MiB reproduced by an independent parser, whose wall model lands on **body 675**, F102's measured N. MARKERS: §5.142 annotated, **state unchanged (OPEN)**; §11.222 at BOTH homes ((d) incomplete — `PhotosphereModule.cpp:58`, `AtmExtModule.cpp:73`; (h)(1) GROW withdrawn, (h)(3) widened); §11.233(b) at both homes; README F115. REPORTED NOT ABSORBED: `meshShadowFill.hpp:45/:78` are the fill TEMPLATES, not acquire sites (the section's file list says otherwise); the new path's per-body set is **six** module classes, not four. FOUND NOT FIXED: `shaders/src/receivedShadowsDecl.glsl:20` is a second independent `#define` of the receiver cap; `06old.sts:286` has a field-data typo the engine mis-pairs (D9, frozen). MY OWN ERRORS, kept visible: two corpus-parser errors (caught against F102's numbers) and one field-table error (`bodyRingVert` given a `PlanetRadius` it lacks) caught by my own gate. BASELINES LAST at this delivery vs my measured open (`1890e47`): scan **273/342/144 → 277/347/144** (+4 event lines, +5 pairs — my four marker spans; **the unmarked SET is byte-identical to the baseline**, verified by diffing the two lists after moving one marker off the `(h)` header line, where it had created a false `§11.222 → §11.161` candidate); pair-check **D 36 · D2 12 · I 92 · I2 37 · M 95 → D 36 · D2 12 · I 93 · I2 37 · M 95** — the single +1 on I is §11.236's own row, residual atom `sec: [11.236]`, the entry's self-citation inside its `(j)` markers, which a stub does not repeat. D14: all 12 `artifacts/f115/` files **0 non-ASCII**; `f70_ascii.py gate` PASS. **ACCEPTED 2026-09-12 — verifying commands' `date` 14:3x (supervisor, session 29, Claude Fable 5.1).** Verified by my own runs and reads: §11.236 read in full; code `d0e0c51e` UNTOUCHED (tree clean, binary `d607cfdc` at open and close, EntityCore read-only); four harness commits `cc28d95 → 98ba1e6 → 3019188 → 31ede6e` (Claude Opus 5; the predictions first at 14:01, before any struct body was read; the delivery's trailer `Code: master-beta @ d0e0c51e`, the supervising footer); the stub at `INTENT.md:1109`; §5.142 annotated with the root's numbers, state OPEN; §11.222 and §11.233 marked at both homes; README F115; `### F` 7; ZERO launches, the criterion pre-registered. **By my own hand:** `f115_sizes.cpp` recompiled from its own header line (rc 0), 74 rows, `--tsv` byte-identical to the landed `f115_sizes.tsv`; F102's 28 rows all present and identical; `BOTH_06sts_body` **1344**; `f115_redundancy.py` reproducing the requirement — **76 B per frame + 12 B constant = 88 B payload, 128 B carved, 8192 bodies per MiB (7078 after the launch scene's 142 529 B)**; instruments **277/347/144 · 252/227/25/125 · D 36 · D2 12 · I 93 · I2 37 · M 95** to the digit of (a)/(j) (the +1 inline stub is §5.153, mine at F114's acceptance). DEVIATIONS ENDORSED: the table's 74 rows against a predicted 34 ± 6 (the refutation kept); the corpus parser written from `parseCommand`'s own loop rather than F102's (an independent reproduction of 1719 and the 1183/531/5 split); the instrument's step-0 gate catching its author's own `bodyRingVert` slip; one marker moved off the `(h)` header line to keep a pre-existing routing citation from becoming a false candidate. DISPATCHER-SIDE FINDINGS, ACCEPTED as mine, output-side: the section named `meshShadowFill.hpp:45/:78` as acquire sites — they are the two fill TEMPLATES — and named four new-path files where the per-body set has six module classes (`PhotosphereModule.cpp:58`, `AtmExtModule.cpp:73` besides) — a grep read as a census, structure class; non-input, the executor censused from source. Round tally: **eight** (2 value, 5 structure, 1 intent). NO §5 MINT: the two found-not-fixed items are routed — `shaders/src/receivedShadowsDecl.glsl:20` re-declares `MAX_SHADOW_CASTERS = 8` independently of `bodyShaderInterface.hpp:47` (an I2 double with no consequence while both read 8 — a §5 candidate the moment either moves, i.e. option O2's first edit); the field's `max_shadow_cast = 8` against the code default 4 (`context.hpp:194`) leaves half of every receive array unfillable on a default config (a D12-class fact for the owner); `06old.sts:286`'s `color0.5,0.5,0.5` is the tester's, already routed (F76). THE OWNER'S FOUR DECISIONS (§11.236(k)) → §3 [Y10]: O2's cap drops the least-significant occluder (VISIBLE — not as-if), O2' needs an arena in his EntityCore stratum, O1 moves the wall from load to camera, O5 names the refusal after any of them; growing the pool is off the list by his word. STANDING: `/home/claude/sc-f115/` 128 KB kept; the parse-like-the-engine hazard (strict `>> key >> value`, no resync) in README F115 for any script over the field shows.
**WIP-PREV2:** 2026-09-12 14:2x — CHECKPOINT 2. THE CENSUS INSTRUMENT LANDED. `f115_sizes.cpp` extends F102's and its **first 28 `--tsv` lines diff EMPTY against `artifacts/f102/sizes.tsv`** (check (a) FULL) — and it does so while including the REAL `bodyModule/bodyShader.hpp` instead of F102's copied declarations (measured: that header pulls only `<list> <string> <memory>`, `vecmath.hpp`, `bodyShaderInterface.hpp` — it compiles standalone), so the reproduction is now an independent check ON F102's mirrors. 74 rows: the 26 landed + old-path blocks F102 never priced (`globalTescGeom` 12→64, `ringFrag` 8→64, `ShadowFrag_mc8` 240→256, `OjmShadowFrag_mc8` 272→320) + the receive array priced per entry and per cap + **25 per-body CARVED totals by class and path**. Headlines: `BOTH_06sts_body` = **1344** ✓; `NEW_ojm` **1216**/artificial body; `NEW_layered_tes_ray` **2048**; cap-1 → **640** both paths, array-out → **576**. `f115_mirrors.py` (I2 guard on the 3 uncompilable declarations) green + shown able to fail. `f115_corpus.py` reproduces §11.222(h)'s split INDEPENDENTLY — **1183 / 531 / 5, 1719 total, 1.581 MiB** — using the ENGINE's own strict `>> key >> value` pairing (`app_command_interface.cpp:163`), and its wall model lands on **body 675 of 06.sts**, F102's measured N. TWO PARSER FINDINGS en route, both recorded: `mode in_universe` routes to `OjmMgr::load` too (`:4119`, three modes not one) and `06old.sts:286` has a field-data typo (`color0.5,0.5,0.5`, no space) that the ENGINE mis-pairs exactly as F102 did. NO LAUNCH. NEXT: the field-by-field variability table and the three redundancy axes.
**WIP-PREV:** 2026-09-12 14:0x — CHECKPOINT 1. §0.7 gate PASSED: `premise_check.py F115` **17 PASS / 0 FAIL** (13:57); harness HEAD `1890e47` clean, code `d0e0c51e` clean, binary `d607cfdc`; definition md5 `8e364a3a` MATCH; live `### F` 7. Ledger baselines measured AT MY OPEN HEAD (not quoted): scan **273/342/144**, pair **D 36 · D2 12 · I 92 · I2 37 · M 95**. Check (a) HALF DONE: F102's `f102_sizes.cpp` recompiled UNCHANGED at `d0e0c51e` → 28 rows **byte-identical** to `artifacts/f102/sizes.tsv` (md5 `58e63b9e`), and 192+128+192+832 = **1344 B** reproduced as a sum before any extension. Predictions PRE-REGISTERED (`artifacts/f115/prediction.txt`, 13227 B, 0 non-ASCII) naming what was read before them. NEXT: the census (mandate (2)) — the new path's four module files + `bodyShaderInterface.hpp`, then the old path's 15 `src/bodyModule/` files, field by field. No launch spent (the criterion is written in the prediction, §5).

---

### F116 — §5.115's SIZE BOUND ON THE OWNER'S WORD (§11.233(b) correction, verbatim: *"1 GiB TOTAL across channels; rotate within a session at the bound"*): F108's numbered window bounds the number of LAUNCHES a channel keeps, not the bytes ONE launch writes — a session that runs for a year (his instance: the pre-Vulkan version, over a year non-stop) grows `script.log` at 193 MB/h under his shows (§11.218(m)) with nothing to stop it, which is why *"the proxy the tester said (number of sessions) may fail"*; the bound: `LOG_RETENTION_BYTES = 1 GiB` beside `LOG_RETENTION_LAUNCHES` (`log.hpp:88`), the TOTAL over the five channels' live and archived files; when the total crosses it the channel being written rotates as `openLog` would — `rotate()` at `log.cpp:93` reused: the live file becomes `.1`, the oldest archive is deleted, the window stays ≤ 8 files — and a fresh live file opens; the check sits in `write()` (`log.cpp:247`) on a per-channel byte counter kept by `write()` itself plus the archives' sizes read at open and at each rotation (never `file_size` per line — D11); ONE D12 line at each in-session rotation naming the budget, the total, the channel and the file deleted; `write_log=false` untouched; the 392 `EntityCore-logs-*.txt` are the submodule's pile, outside the five channels — named, never touched [S, engine (`src/tools/log.{hpp,cpp}`) + the readers if a mid-session rotation changes what they read (`logread.py`, `f108_rotate.py`); FUNCTIONAL launches on `:2`; the proof under a MUTATED budget so the rotation fires in seconds; veto points §3: the rotating-channel rule and the slot an in-session rotation consumes]

**Why now / mandate:** the owner's answer verbatim (§11.233(b) correction, 12:4x); §5.115 (retention half FIXED by F108 in launches; the byte dimension left open by construction — §11.230's window counts launches); §11.173(b) (uniform bounded retention across all channels — the size bound is the same principle on the other axis); §11.218(m) (193 MB/h measured). Decision-free on F108's mechanism: the budget is his number, the rule states its one open choice as a veto point.

**The reading the mint stands on [derived; each fact a premise line]:** `cLog::rotate(LogfilePath)` (`log.cpp:93`) deletes `<base>.7.log`, shifts `.6 → .7 … .1 → .2`, renames the live file to `.1`, and pushes its report line into `openReport` (flushed once all channels are open — F108's shape at `main.cpp:2xx`); `openLog` (`:185`) calls it then opens the live file truncated; `write()` (`:247`) takes `writeMutex`, formats, and streams to `logFile.at(fichier)` — the one place every byte of every channel passes, so a counter there is exact and free; the five channels open at `main.cpp:221-225`; `LOG_RETENTION_LAUNCHES` appears 9 times in `log.cpp` (the rotation and its two report lines); the field's `log/` holds 40 channel files today (8 × 5, F108's steady state) and 392 EntityCore files; `f108_rotate.py` proves the launch-window semantics and `logread.py` is the ONE reader of any channel (§11.230) — both must stay green, and `logread.py` must read across an in-session rotation (the live file's history continues in `.1`). The ONE open choice: at the bound, rotate the channel being written (keeps the other channels' history; the writing channel is the one that grew) or rotate ALL five (uniform window semantics) — the executor states both consequences and picks one, veto-open. An in-session rotation consumes one slot of the eight: after it the channel keeps fewer LAUNCHES — the owner's *"1 GiB total"* accepts this by construction, said in the entry as a veto point.

**Measured at dispatch (supervisor, 2026-09-12 12:5x, code `d67833cd`):** the sites at the lines quoted; the field's log directory as above; `/home/claude/sc-f116` absent; canary green both arms on this boot.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f116/prediction.txt`): the rule (which channel rotates; the total's denominator = live + archives of the five channels, measured at open and maintained by the counters); the predicted file set and D12 lines after a MUTATED-budget launch (budget a few MiB: the rotation count, the files deleted, the window ≤ 8 throughout); the per-line cost (one add, one compare); the interaction with F108's open-time rotation (the slot consumed); `logread.py`'s reading across the rotation. (2) **THE FIX:** the constant with its reason (§5.115, the owner's 1 GiB, *"change this constant and rebuild"* — the F108 pattern, NO config key); the per-channel counters + the total; the in-session rotation reusing `rotate()`; the report line through the same channel F108 used (the internal channel + the console under `print_log`); NOTHING else moves — assert by diff. (3) **THE PROOF:** a scratch build with the budget MUTATED (env-gated or a scratch tree under `/home/claude/sc-f116/`, the delivered constant never launched to its bound — a 1 GiB log is not a test) playing a chatty show (a `struct loop` show or F98's driver) until the rotation fires at least twice; the file set and D12 lines as predicted; the DELIVERED build on the same show never rotates in-session (the bound not reached) — both ways; `f108_rotate.py` green on the launch window; `logread.py` across the rotation; the smoke suite; `f91_run.sh --expect post --locale fr` at `1fe630a4`; the counter's cost measured once. (4) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.115 → the SIZE bound FIXED in the marker (the density half (2) still OPEN); §11.230 marked at both homes; §11.233(b) marked; `harness/README.md` F116 section; WIP per §0.6; D14.

**Boundaries:** `src/tools/log.{hpp,cpp}` (+ `main.cpp` only if the report's flush point needs it — say why); the readers; NO config key, NO `checkConfig.cpp`; NO data; NO EntityCore (the pile is named, not touched); FUNCTIONAL launches on `:2` (`--no-scene` canary before the first; the exe+port instance assert; config/ssystem md5 in == out; the played show from a farm copy — the annotator rewrites played `.sts`); explicit timeouts (Q-68); no `run_in_background`; nothing under `/tmp` carries.

**Discriminating checks:** (a) under the mutated budget the rotation fires at the predicted byte count and the file set matches; the unmutated build never rotates in-session on the same show — shown both ways; (b) the D12 line names budget, total, channel, deleted file; (c) `logread.py` reads across the rotation; (d) `f108_rotate.py`, the smoke suite, F91 green; (e) the diff confined to `log.{hpp,cpp}` (+ readers); (f) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; canary `--no-scene` exit 0 before the first launch; VRAM free ≥ 4 GB; F114 DELIVERED (its binary is this task's pre).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# F108's mechanism, re-resolved at HEAD (content drift = abort)
grep -n '^constexpr int LOG_RETENTION_LAUNCHES = 8;' src/tools/log.hpp | cut -d: -f1 => 88
grep -n '^void cLog::rotate' src/tools/log.cpp | cut -d: -f1 => 93
grep -n '^void cLog::openLog' src/tools/log.cpp | cut -d: -f1 => 185
grep -n '^void cLog::write(const std::string& texte' src/tools/log.cpp | cut -d: -f1 => 247
grep -c 'LOG_RETENTION_LAUNCHES' src/tools/log.cpp => 9
grep -c 'Log retention' src/tools/log.cpp => 2
grep -n 'openLog(LOG_FILE' src/main.cpp | cut -d: -f1 | tr '\n' ' ' => 221 222 223 224 225
# the field's log directory, the readers, the ledger
ls ~/.spacecrafter/log/ | grep -cE '^(spacecrafter|script|tcp|shader|vulkan)(\.[0-9]+)?\.log$' => 40
ls ~/.spacecrafter/log/ | grep -c '^EntityCore-logs-' => 395
test -f claude/harness/f108_rotate.py -a -f claude/harness/logread.py && echo ok => ok
grep -c 'FIXED' claude/INTENT/11.230.md => 3
grep -n '^115\. \*\*' claude/INTENT.md | head -1 | cut -d: -f1 => 454
test -e /home/claude/sc-f116 ; echo $? => 1
```

**DoD:** predictions first; the constant + counters + in-session rotation + the D12 line; the mutated-budget proof both ways; the readers green; §11 entry + stub; §5.115's size bound FIXED in the marker; the §11.230 / §11.233 markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** 2026-09-12 14:5x — gate PASS (18/18 + prose; canary exit 0 14:38:19); baselines at open scan 277/347/144, pair 252/227/25/125 (D36 D2 12 I93 I2 37 M95); predictions committed; THE FIX IS IN at code `8cca5ddf` (binary `fada7b10`), `f108_rotate.py` 2 launches 0 FAIL on a farm, delivered arm measured (17.3 MiB written, ZERO in-session rotations at 1 GiB), `prediction_mutant.txt` committed with the exact numbers. NEXT: build M1 (budget 4 MiB + instrumentation, `log.cpp` only) and M2 (rule (a)) and run them.

---

### F111 — §5.150's FIX: THE ORBIT-LINE SAMPLER GETS ITS OWN SEED — `OrbitModule::sampleOrbit` (`OrbitModule.cpp:94-114`) samples 180 dates through `positionAtTimevInVSOP87Coordinates` (`:110`), i.e. through the POSITION solver's `iterativeLastE` (`orbit.hpp:120`), leaving the seed ~π of mean anomaly off for the next frame's use (§11.229(h2): 0.0246 AU / 834 arcsec on Pasiphae at HEAD for one frame, once per `period/180` of simulation time; Europa 3.475e-08 AU measured PERSISTENTLY by mutation 3); the OLD path's plot (`orbit_plot.cpp:142/:174`) uses the batch pair `prepairFastPositionAtTimevInVSOP87Coordinates` + `fastPositionAtTimevInVSOP87Coordinates` with its OWN `batchLastE` (`orbit.hpp:122`; `orbit.cpp:442` reset, `:447` ten warm-up steps then one step per point) — *"a plot is not a use"*, the generality the port dropped (§11.52(b)'s rewrite monitor); the fix is the sampler calling the SAME pair (the base-class defaults at `orbit.hpp:40/:45` fall back to the position call for orbit types without a batch seed — exactly the old path's behaviour on those types); the trail walker (`TrailModule.cpp:233`, RESUME_EXTRA_ITERATIONS+1 calls per missed sample) is the third walker — measured, named, changed only if its own measurement says so [S–M, engine (`OrbitModule.cpp`, `sampleOrbit` only) + harness; FUNCTIONAL launches on `:2`, `--no-scene` canary; new path only, the old path untouched by construction; veto points §3]

**Why now / mandate:** §5.150 (OPEN, *"the fix shape named not taken"*); §11.229(h2) (site, reach, consequence measured and predicted, the old path's shape named); §11.232(c)2 — consolidation first: a defect the NEW path has and the old does not (§11.163(h)'s test passes in the disqualifying direction); the session-28 queue item (3). Decision-free: the old path's own shape, no constant, no policy.

**The reading the mint stands on [derived; each fact a premise line]:** `sampleOrbit` is called from `update` when `!sampled || |date − lastSampleJD| ≥ period/ORBIT_POINTS` (`:132`); the sampler's 180 calls at `calc_date = date + (d − 90)·increment` leave `iterativeLastE` at E(date + 89·increment) ≈ +π of mean anomaly; the next frame's `useNow()` runs `ITERATIVE_STEPS_PER_CALL = 2` Newton steps (`iterative_orbits.hpp:31`) from there — the transient the slice priced (§11.229(h2), P4). The two seeds are `mutable` members of ONE `EllipticalOrbit` object that BOTH paths share (the old plot runs while the new path draws — F107 measured 16 old-plot movers with the flag on): each `prepairFast…` resets `batchLastE` and the 180 `fast…` calls complete inside one call of one thread [derived — the executor asserts no yield inside `sampleOrbit`/`computeOrbit`], so the two plots sharing `batchLastE` is sequential reuse, never interleaving; `orbit_plot.cpp:142/:174` are the two old call sites. `wantShown` (`:91`) gives the flag its reach: `flag satellites_orbits on` reaches the 40 walked iterating records (all satellites), `flag planets_orbits on` reaches none (F107). The transient lives in ONE frame: catching it in the engine needs a dump on the frame after a resample — at a time rate where every frame advances ≥ `period/180` for the sampled body, EVERY frame resamples and every dump carries it; the rate per body follows from `getSiderealPeriod()` (`ModularBody.hpp:1204`) and the frame cadence (144 fps, §11.159(k7)); whether `timerate` reaches it is the executor's first computation. F107's instruments: `f107_orbitflag.py` (the flag leg at F100's pinned clock with the within-launch control), `f107_model.py` (the slice model; `model_sampler.txt` names Pasiphae twice), `f107_replay.py`, `f100_run.sh`/`f100_identity.py`.

**Measured at dispatch (supervisor, 2026-09-12 11:4x–12:0x, code `fcc277c9`, harness `1d07413`):** the sites at the lines quoted; `OrbitModule.cpp` holds 0 `FastPositionAtTimevInVSOP87Coordinates` calls; the field: `flag_planets_orbits = false` (`flag_satellites_orbits` absent from `config.ini`, the one hit is the planets key); F107's artifacts present (`artifacts/f107/`); `/home/claude/sc-f111` absent; canary green both arms on the re-banked boot; F112's probe lands BEFORE this task — use it (until then: `/proc/<pid>/exe` + `ss -ltnp` 7805).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f111/prediction.txt`, before any build): the per-body one-frame transient at HEAD for the walked iterating records from the slice (`f107_model.py`), the every-frame-resample rate per body and whether `timerate` reaches it (state the maximum reachable and the model's prediction AT that rate), the expected dump delta pre-fix (≥ the transient for the bodies the rate reaches) and post-fix (0 to the float floor — F107's 0 of 40 at rate 1 is the floor's measurement), Europa's persistent 3.475e-08 AU → 0 post-fix (mutation 3's attribution), the sampled LINE unchanged pre/post (ten warm-up steps from 0 converge for every e in the field — predict max |Δ orbitPoint| per body from the slice), the trail walker's own perturbation from the slice (its last sample's E vs the frame's — its magnitude decides rider vs this task), the cost (ten warm-up + 180 single steps per resample vs today's 180 two-step calls; per resample, not per frame; against D11 at rate 1 and at show-load rates). (2) **THE PRE-FIX CATCH** on `6d63e6c1`: `flag satellites_orbits on` at the pinned clock, the rate from (1), dumps at F100's cadence, the within-launch control (same launch, flag off — F107's shape) and the two-date control that shows the comparison able to fail; the transient CAUGHT by name and magnitude (Pasiphae the worst) — if the engine cannot reach the rate, STOP at that step and report the reachable maximum with the model's prediction there; only then the fallback mutation on a scratch tree (sample at +π every frame) that makes the channel frame-persistent on the pre binary and absent on the post. (3) **THE FIX:** `sampleOrbit` — `orbit->prepairFastPositionAtTimevInVSOP87Coordinates(date, increment)` once, then `fastPositionAtTimevInVSOP87Coordinates(date, calc_date, orbitPoint[d])` per point; the osculating branch unchanged; the pair's return value read at the old plot (`:142-153`) before it is used or ignored — say which and why; the comment cites §5.150, §11.229(h2) and the old site: *a plot is not a use*. NOTHING else moves — assert by diff. (4) **THE PROOF:** the same leg on the post binary — 0 of 40 move (the float floor), Europa 0; the sampled line pre/post at the pinned clock with the flag on (a screenshot pair, byte-identical or within the A/A floor stated from two same-binary runs); `f91_run.sh --expect post --locale fr` at `1fe630a4` (the readout is not the plot); the smoke suite; the D11 cost measured once (a resample's µs and resamples per second at rate 1 and at the show-load rate). (5) **RECORD:** §11.⟨next⟩ FIRST + stub; §5.150 → FIXED with the marker; §11.229(h2) marked at both homes; §11.225(j2) marked (the channel closed); §5.84's instrument caveat (the 16 old-plot movers) re-stated unchanged; the trail walker's measurement recorded at its own line; `harness/README.md` F111 section; WIP per §0.6; D14.

**Boundaries:** `src/experimentalModule/bodyModules/OrbitModule.cpp` (`sampleOrbit` only); NO change to `orbit.{hpp,cpp}`, `orbit_plot.cpp`, `TrailModule.cpp` (measured, named; a rider if its own measurement says so — reported, not fixed here), no data, no EntityCore; FUNCTIONAL launches on `:2` (`--no-scene` canary before the first; the instance assert; config/ssystem md5 in == out per launch); explicit timeouts (Q-68); no `run_in_background`; nothing under `/tmp` carries; every scratch binary under `/home/claude/sc-f111/`.

**Discriminating checks:** (a) the transient caught pre-fix by name and magnitude as predicted (or the reachable-rate STOP with its number); (b) gone post-fix — 0 of 40, Europa 0; (c) the line unchanged within the stated floor; (d) F91 byte-identical, the smoke suite green; (e) the diff confined to `sampleOrbit`; (f) the control able to fail (two pinned dates ⇒ not identical); (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; canary `--no-scene` exit 0 before the first launch; VRAM free ≥ 4 GB before any launch; F112 DELIVERED (its probe is this task's instance assert) — if the prompt says otherwise, the `/proc/<pid>/exe` + port form stands.

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the sampler, the two seeds and the old plot, re-resolved at HEAD (content drift = abort)
grep -n '^void OrbitModule::sampleOrbit' src/experimentalModule/bodyModules/OrbitModule.cpp | cut -d: -f1 => 94
grep -n 'orbit-.positionAtTimevInVSOP87Coordinates(date, calc_date, orbitPoint\[d\]);' src/experimentalModule/bodyModules/OrbitModule.cpp | cut -d: -f1 => 110
grep -c 'FastPositionAtTimevInVSOP87Coordinates' src/experimentalModule/bodyModules/OrbitModule.cpp => 0
grep -n 'if (!sampled || std::abs(date - lastSampleJD) .= period / ORBIT_POINTS)' src/experimentalModule/bodyModules/OrbitModule.cpp | cut -d: -f1 => 132
grep -n 'return body-.isSatellite() ? showSatellites : showPlanets;' src/experimentalModule/bodyModules/OrbitModule.cpp | cut -d: -f1 => 91
grep -n 'batchLastE = 0;' src/bodyModule/orbit.cpp | cut -d: -f1 => 442
grep -n '^void EllipticalOrbit::fastPositionAtTimevInVSOP87Coordinates' src/bodyModule/orbit.cpp | cut -d: -f1 => 447
grep -c 'eccentricAnomaly(meanAnomaly, batchLastE)' src/bodyModule/orbit.cpp => 2
grep -n 'mutable double iterativeLastE = 0;\|mutable double batchLastE = 0;' src/bodyModule/orbit.hpp | cut -d: -f1 | tr '\n' ' ' => 120 122
grep -n 'virtual std::pair.double, double. prepairFastPositionAtTimevInVSOP87Coordinates(double JD0, double deltaJD) {' src/bodyModule/orbit.hpp | cut -d: -f1 => 40
grep -n 'virtual void fastPositionAtTimevInVSOP87Coordinates(double JD0, double JD, double \*v) const {' src/bodyModule/orbit.hpp | cut -d: -f1 => 45
grep -n 'FastPositionAtTimevInVSOP87Coordinates' src/bodyModule/orbit_plot.cpp | cut -d: -f1 | tr '\n' ' ' => 142 174
grep -n 'orbit-.positionAtTimevInVSOP87Coordinates(date, sampleJD, tmp);' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 233
grep -n 'constexpr int ITERATIVE_STEPS_PER_CALL = 2;' src/bodyModule/iterative_orbits.hpp | cut -d: -f1 => 31
# F107's instruments and records; the field; the ledger row
test -f claude/harness/f107_orbitflag.py -a -f claude/harness/f107_model.py -a -f claude/harness/f107_replay.py -a -f claude/harness/f100_run.sh -a -f claude/harness/f100_identity.py && echo ok => ok
grep -c 'Pasiphae' claude/harness/artifacts/f107/model_sampler.txt => 2
grep -c '§5.150' claude/INTENT.md => 1
grep -c 'batchLastE' claude/INTENT/11.229.md => 1
grep -n 'flag_satellites_orbits\|flag_planets_orbits' ~/.spacecrafter/config.ini | cut -d= -f2 | tr -d ' ' | tr '\n' ' ' => false
test -e /home/claude/sc-f111 ; echo $? => 1
```

**DoD:** predictions before any build; the pre-fix catch (or the reachable-rate STOP); the fix in `sampleOrbit` alone; the proof legs (a)–(g); §11 entry + stub; §5.150 FIXED; the §11.229 / §11.225 markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** —

---

### F112 — LAUNCH PRECONDITIONS THAT SEE WHAT THEY GUARD: (i) the concurrent-instance probe identifies an engine by what it IS — `/proc/<pid>/exe` (a copied or renamed binary keeps nothing else) and the port it holds — in ONE home that every LIVE caller routes through, the **42** copy-pasted `comm == "spacecrafter"` sites (22 shell, 18 Python by census — not the three §11.231(j2) named) partitioned live/frozen by rule; (ii) the canary's `--no-scene` arm GATES on GPU headroom DERIVED from the app's own init sequence (512 + 160 + 1280 MiB dedicated, then a 256 MiB chunk, before any window — `available : 1591 MiB` killed it on 2026-09-12) instead of noting at a guessed 8192 MiB, and names the holder [M, harness only; launches: one decoy (a copy of `6d63e6c1` renamed) for the probe's positive map + the canary twice; `--no-scene`; veto points §3]

**Why now / mandate:** §11.231(j2) and the session-28 §3 [H1] (*"the exe-identity probe is next round's first item"*, measured blind with two `sc_f109_iso` live and the probe at 0); HOST-EVENTS 2026-09-12 and §11.232(d) — the `--no-scene` arm passed with a NOTE while no launch could start, and only the photometric arm caught it by failing to launch; F111 and every later measuring task stand on this probe; the session-28 queue item (2). Decision-free: an instrument that answers the question it is asked; the anchor rule (§11.193): the app STATES its need in its own log at init — the gate is a proxy for that statement and is derived from it, banked with its argument like the band.

**The reading the mint stands on [derived; each fact a premise line]:** `comm` is the executable's basename truncated to 15 bytes — a staging binary named `sc_f109_iso` or `spacecrafter-pre` never equals `spacecrafter`; `/proc/<pid>/exe` is the real file (F109's `b22_live_run.sh` fix matches on it — *"which no shell can ever satisfy"*), so identity by exe covers copies and renames; the engine holds TCP 7805 when its server is up — a second, independent channel (`ss -ltnp`) with its own residual (`--no-scene`/farm launches without the server); the census by the grep pattern is 42 files, the three named sites at `f26_epoch.sh:46`, `f27_reply.py:108`, `f56_canary.sh:476` (F109 wrote `:107` — the pasted value wins), the smoke suite's own at `f90_rehearsal_run.sh:52-54`; the canary's VRAM member is a NOTE at `:416` against `BANK_VRAM_NOTE_MIB=8192` (`:171`), *"NOT a gate"* by its own text; the failed launch's applog shows four `Dedicated allocation of` lines and two `Failed to allocate chunk of 256 MiB` with `available : 1591 MiB` at init; the GPU total is 32607 MiB. The identity CRITERION must be a positive map (decoy 1 / without 0 on each channel, F26's precedent) with its residual stated (a renamed copy outside the naming convention, in an unlisted directory, with no server up) — never a criterion that cannot fail.

**Measured at dispatch (supervisor, 2026-09-12 11:4x–12:0x, code `fcc277c9`, harness `1d07413`):** the census and sites by the PREMISES block; one `sc_f109*` staging binary under `/home/claude/sc-f109/`; port 7805 free; `/home/claude/sc-f112` absent; VRAM 980 MiB used after the owner's unload; canary green both arms.

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f112/prediction.txt`): the identity criterion (exe basename ∈ {`spacecrafter*`, `sc_*`, `sc-*`} OR exe path under `/home/claude/sc-*/` or `*/build*/src/` OR the pid holds TCP 7805 — or a better one, argued) and its positive map per channel with the decoy; the residual the criterion cannot see, stated; the live/frozen partition RULE (live = reached by the smoke suite, the canary, `f91_run.sh`, `f100_run.sh`, the F107 drivers, `f95_soak.py`, and any driver a live README section names as runnable; frozen = a one-shot campaign driver whose artifacts are landed — annotated in place, bytes otherwise untouched); the headroom threshold: the number and its derivation from the applog's init sequence on `6d63e6c1` (dedicated allocations + the first chunk + the reference scene's textures, read from a GREEN run's applog too), and the canary members that change. (2) **THE PROBE, ONE HOME:** `harness/sc_instances.sh` and `harness/sc_instances.py` (one criterion, two languages, each with a `--self-test` shown able to fail), printing pid · uid · exe · port per hit; the decoy = a COPY of the delivered binary as `/home/claude/sc-f112/sc_f112_decoy` run on `:2` for ≥ 10 s then killed (the app quits on its own if it must — say how the process was held) — the old `comm` form reads 0 with it live (the blind spot reproduced), the new probe reads 1 by exe and, if the server came up, 1 by port; 0 after; every LIVE caller routed through the home (the F108 precedent — `logread.py` as the one reader), the frozen ones annotated with one comment line pointing at the home; `f56_canary.sh`'s `concurrency.spacecrafter_pre` member and `f90_rehearsal_run.sh`'s assert among the live; §0.5's bullet rewritten at acceptance by the supervisor from the entry. (3) **THE GATE:** `f56_canary.sh` gains `gpu.headroom` — `nvidia-smi` free MiB vs `BANK_GPU_NEED_MIB` in the VALUES block with its derivation comment — FAIL below it on BOTH arms, the message naming the holder(s) from `nvidia-smi --query-compute-apps` (pid, name, MiB) so the report says WHO; shown able to fail without the owner's model: the bank mutated above the free amount ⇒ exit non-zero with the holder line; the real value ⇒ PASS; the NOTE member kept as a note. (4) **RECORD:** §11.⟨next⟩ FIRST + stub; §11.231(j2) marked at both homes (landed); HOST-EVENTS 2026-09-12's routing line marked; §11.232(d) marked; `harness/README.md` F112 section (the probe's contract, the criterion, the residual, the gate's derivation); WIP per §0.6; D14.

**Boundaries:** harness only — NO engine code, NO data; the decoy is a copy under `/home/claude/sc-f112/`, launched at most twice, killed by pid (never `pkill -f` — §11.231(j)); the frozen drivers' bytes untouched except the one annotation line each (or none — say which); NO change to the photometric band or any banked epoch; canary runs `--no-scene` plus the mutated-bank run; explicit timeouts (Q-68); no `run_in_background`; nothing under `/tmp` carries.

**Discriminating checks:** (a) with the decoy live: old `comm` form 0, new probe 1 by exe (and by port if the server is up), both implementations; 0 after — the positive map both ways; (b) the headroom gate FAILS under the mutated bank naming the holder, PASSES at the derived value; the derived value stated with its applog lines; (c) the census partition committed with its rule, every live caller's diff = one call to the home; (d) the smoke suite and F91 green after the routing; the canary `--no-scene` green with the new member; (e) `--self-test` of both homes shown able to fail; (f) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-12; VRAM free ≥ 4 GB before the decoy launch; F110 DELIVERED (its rehearsal ran the smoke suite with the old assert — this task changes that assert after).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => d0e0c51e
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => d607cfdc
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 237
grep -c '^### F' claude/fable-dispatch.md => 7
# the probe's homes and the canary's VRAM member, re-resolved at HEAD (content drift = abort)
grep -lE '= "spacecrafter"|== "spacecrafter"|-x .spacecrafter. /proc' claude/harness/*.sh claude/harness/*.py | wc -l => 43
grep -n "grep -l -x 'spacecrafter' /proc" claude/harness/f26_epoch.sh | head -1 | cut -d: -f1 => 46
grep -n 'read().strip() == "spacecrafter"' claude/harness/f27_reply.py | cut -d: -f1 => 108
grep -n '= "spacecrafter" \] && n=' claude/harness/f56_canary.sh | cut -d: -f1 => 476
grep -n '^BANK_VRAM_NOTE_MIB=' claude/harness/f56_canary.sh | cut -d: -f1 => 171
grep -n 'note "gpu.vram_pressure"' claude/harness/f56_canary.sh | cut -d: -f1 => 416
# the 2026-09-12 failed launch's own account of what the app needs
grep -c 'Dedicated allocation of' claude/harness/artifacts/f56/canary/20260912-112628/scene/dwell.applog => 4
grep -c 'Failed to allocate chunk of 256 MiB in GPU memory' claude/harness/artifacts/f56/canary/20260912-112628/scene/dwell.applog => 2
grep -o 'available : [0-9]* MiB' claude/harness/artifacts/f56/canary/20260912-112628/scene/dwell.applog | head -1 => available : 1591 MiB
nvidia-smi --query-gpu=memory.total --format=csv,noheader => 32607 MiB
# a staging binary to reproduce the blind spot with; the port; the scratch dir
find /home/claude/sc-f109 -maxdepth 3 -type f -name 'sc_f109*' | wc -l => 1
ss -ltnp | grep -c 7805 => 0
test -e /home/claude/sc-f112 ; echo $? => 1
```

**DoD:** predictions (criterion, residual, partition rule, threshold derivation) before any code; the one-home probe in both languages with self-tests; the decoy map both ways; every live caller routed; the headroom gate shown able to fail and passing at the derived value; §11 entry + stub; the §11.231(j2) / HOST-EVENTS / §11.232(d) markers; README; trees clean; WIP cleared; baselines LAST.
**WIP:** —

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

- **F110 — THE INTERN'S ENTRY PATH, FOLLOWED BY HAND. Three gaps only you can fill, quoted from
  the document's own lines, and one finding that is a decision rather than a defect.** The
  rehearsal ran the whole path from a plain clone (§11.234): the clone, the build, the D14 gate,
  the smoke suite on both arms, scedit's `cmake`/`ctest`, the hook, the harness README's first
  command. Everything measurable was measured and corrected in `doc/developer-entry.md`; these
  four were NOT written, because they are yours.

  ```
  YOURS — nothing here substitutes them
  [Y5] R23 · the content-installation procedure ─ his clone runs EMPTY and nothing tells him why
       doc says  "by default only limited catalogues are loaded, and the correct ones are loaded
                 by 'an outside installation procedure' [owner, R23]" and, next sentence,
                 "This repository does not document that procedure, and where it lives and who
                 owns it is the one thing still to ask the owner."  INSTALL sec.5 agrees: the
                 tree ships NO content.
       measured  a tree install is the binary, the shaders and eleven metadata files
                 (§11.204(f)); a first launch then prints ten "Completed copy of ..." lines over
                 ten EMPTY directories (§11.204(i), §5.132) — so the log cannot tell him either
       ask       where the procedure lives, who owns it, and whether it may be named in the code
                 repo. One sentence in INSTALL sec.5 closes it. Until then his first run is a
                 program with no sky and no way to find out that this is expected.
       →         §11.232(c)4(i) · §11.234(l) · R23 · §5.74

  [Y6] §9 · "Engineering principles" is a placeholder addressed to you, and he meets I1-I7 in week 1
       doc says  "**Placeholder -- to be written by Calvin Ruiz, the project owner.** ... The
                 ledger argues from a set of engineering invariants it cites by number, I1 to I7
                 ... **Neither repository states what they say.** The text is the owner's and
                 lives outside both, so it is not restated here: a paraphrase of a principle you
                 cannot check against its author is worse than an empty section. Until he fills
                 this in, read an I<n> citation as a pointer to him."
       ask       the text, or permission to reconstruct it from the ledger's uses FOR YOUR
                 CORRECTION (I2 alone appears 33 times, so the uses are dense enough to draft
                 from — but a drafted principle is exactly the thing the section refuses to do
                 unchecked). Either answer closes it; silence leaves him citing a pointer.
       →         §11.232(c)4(ii) · §11.234(l)

  [Y7] the remote form ─ he cannot run the document's second clone at all
       doc says  "git clone -b CC-harness <same-remote-url> claude", the remote being
                 git@github.com:lionelruiz13/spacecrafter.git [observed: git remote -v]
       measured  the rehearsal substituted a LOCAL path and says so at the step; it is not a
                 substitute he has. An SSH form needs a key on that account.
       ask       a deploy key / collaborator access, or the HTTPS form written into the document.
                 NB the choice is not cosmetic: the rehearsal measured that a clone over the git
                 transport does NOT carry unreachable objects, which is why the stale sha in this
                 document read "bad object" for him and "exists" for me (§11.234(e)).
       →         §11.232(c)4(iii) · §11.234(l)

  [Y8] THE BUILD EVERY MEASUREMENT USES IS NOT THE BUILD THE DOCUMENT SHIPS — your call, not a defect
       measured  build-claude (the binary every number in this ledger is taken on, 6d63e6c1) is
                 configured RelWithDebInfo and compiles `-O2 -g -DNDEBUG`; install_src.sh's
                 documented path configures Release and compiles `-ggdb3 -Ofast -Wall -O3`.
                 CMakeLists.txt branches on Debug (:101), Release (:118) and LocalRelease (:134,
                 :142) ONLY, so RelWithDebInfo takes none of them, CMAKE_CXX_FLAGS stays empty
                 and CMake's own RELWITHDEBINFO default applies. Both read from each build's own
                 flags.make; sizes 191 022 536 B against 218 127 672 B.
       why it    §2.0 D11 prices everything in 1 ms/frame. Every cost number this project has
       matters   published was measured on -O2 while a user runs -Ofast -O3.
       ask       re-point build-claude at Release and re-bank, or keep -O2 and state it as the
                 measurement platform. NOT touched here: re-configuring build-claude would
                 invalidate the canary's photometric band and every baseline standing on it.
       →         §11.234(g)

  ALSO YOURS, one line each, no answer needed today
  [Y9] whether every inline §5 row should carry its own "§5.N" on its line — the convention
       change §11.233(h) raised. The RESOLVER now exists either way
       (claude/intent_resolve.py, and the entry document tells him to use it), so this is a
       preference about the file's shape, not a blocker. → §11.233(h) · §11.234(k)
  ```

- **Session-28 decision items (2026-09-11/12, the SEED-STALENESS / LOG-RETENTION / RESIDUAL-STEP round —
  F107 · F108 · F109, three for three; F109 a STOP endorsed). Q-70's shape: one decision per node, its held
  set at the node, anchors by ID and symbol, correlated items together. Nothing asked in-session (Friday
  night). Your Saturday run meets `supervised-by.sh` EXACTLY as the session-27 [Y4] below describes it —
  nothing in this round touched it.**

  ```
  YOURS — nothing here substitutes them
  [Y1] §5.149 · every iterative solver is SEEDED AT JD 0 by the constructor ─ the CAUSE of §5.145's
       [2026-09-12: the pointer was UNRESOLVABLE for you (§11.233(h)) — §5.149 is the inline row
       `149. **…**` at INTENT.md:515, no entry file; the shape question (i)–(iv) still stands]
       1.198725°; the two-step fix PAYS for it (all ten steps a use buys spent walking back 75.765 rad)
       fact    ModularBody.cpp:121 lastJD = parent->lastJD (0 before the first frame), :126 the
               evaluation; the replay on the sliced text reproduces F104's pre AND post dumps float32
               for float32; a one-line seed reset on the pre tree reads 1.06e-05° at ONE step per call
       shapes  (i) skip the ctor evaluation — measured UNAVAILABLE: isSystemCentered() answers by
               POSITION and both orbit loaders read it at LOAD time (49 records move)
               (ii) reset the seed after the ctor's call — fixes Eris, breaks Sedna by 55° (the comet
               family's first step from (H,c,s) = (0,1,0) is 44 rad; the ctor's call is what burns it)
               (iii) evaluate the ctor at a date the body will be used at (the loader has no date)
               (iv) iterate to a criterion — already written and dead in warp() (§11.225(j1))
       ask     which shape, or none: today the post-fix arm has 10 steps and this seed needs 8; a
               slower converger in an authored system re-exposes it
       →       §11.229(b)(c)(d)(h1) · §5.149 · §5.145 · §11.76(b)

  [Y2] F108 · the log window ─ four veto points, implemented-and-live, each cheap to reverse
       V1      a COMPILED constant (LOG_RETENTION_LAUNCHES = 8, src/tools/log.hpp), NOT a config key: a
               key through checkConfigIni writes into the field's config.ini (the 03fbee59 precondition
               every task asserts) and D13 would have an older build read a key it does not know; so
               D12's self-contained action is "change that line and rebuild", which the log line says.
               If you want it operator-settable, say so — the key lands in checkConfigIni and the
               precondition is re-banked once
       V2      the depth is UNIFORM: four channels that used to truncate now keep 8 launches — the field
               dir went 5 files / 194 KB → 40 files / 1.46 MB, bounded there forever (vulkan ~100 KB +
               spacecrafter ~82 KB per launch are almost all of it); per-channel depth is yours to rule
       V3      six D12 lines per launch, 2203 B, every line self-contained
       V4      a REFUSED second instance consumes one slot of eight (the opens sit 30 lines above the
               lock check); moving them below it is a main.cpp ordering change — yours
       legacy  the 34 dated script-*.log (5.8 MB on this host) are NEVER touched by the app: one line
               names them; delete by hand
       →       §11.230(c)(d)(m) · §5.115 (retention FIXED, density OPEN) · §11.173(b)

  [Y3] §5.152 · a SECOND instance ABORTS ─ exit −6 (SIGABRT): a joinable LinuxExecutor thread is
       destroyed at main.cpp:250's `return 0`; and the refusal line never reaches the console
       (setDebug runs 19 lines lower). Both binaries, pre-existing. Fix shape: join/detach before
       the early return, or route it through the normal shutdown (:399 already waits for it)
       →       §11.230(l) · §5.152 · main.cpp:248-251

  [Y4] §11.231 · the "~10 % residual step" L1 judged IS NOT ON THE SCREEN ─ two items
       ANSWERED [vixy 2026-09-12] → §11.233(g): "he was answering our 10%, if it's not visible the tester
       won't care - because he care about what the public will see" — CLOSED, no profile change, the
       tester question withdrawn
       fact    the number was b22_live_analyze.py's own (an offline px axis 1.3–1.7 % low, the
               dot's own brightening, 8-bit clipping); with the dot suppressed the interior emits
               0.12 % of the swing at the lowest in-band px; the collapse is continuous at T
       tester  ONE question to transmit: does he SEE a brightness step at the collapse, or was he
               answering the 10 % we quoted him? (his L1 answer addressed our number)
       you     whether to change the fade's PROFILE at all (linear in α, measured 91–95 %; perceived
               brightness is not) — user-visible appearance; §11.82(b)'s named lever is measured to
               make it WORSE (energy ∝ α³: a dip near T and a late rush)
       →       §11.231(f)(g)(h)(l) · §11.82(b) REFUTED · A15 · §11.207(g) item 6 refuted

  [Y5] §5.151 · Sedna's shipped orbit is not the authored one ─ data, the tester's: three malformed
       keys in one [Sedna] section (orbit_Period with a capital P → Gauss mean motion; 76,0616 with
       a comma → 76.0; orbit_LongOfPericenter without '='); forward-only (D9)
       →       §11.229(i1) · §5.151

  [Y6] isSystemCentered() answers by POSITION and both orbit loaders read it at LOAD time
       (ModularBody.hpp:1788-1800, ElipticOrbitLoader.hpp:12, CometOrbitLoader.hpp:14) — was a
       declared flag the intent? a body's JD-0 position decides which frame its children's orbits
       are built in                                                        → §11.229(d)(l2)

  [Y7] the re-bank ─ the host rebooted 20:06:16; you re-provisioned the same real session on :2 at
       20:13:30; the three per-boot members re-banked with the argument; green on 17 runs since.
       Your correction may differ (another display to bank on) — one edit
       →       HOST-EVENTS 2026-09-11 · f56_canary.sh VALUES block

  [Y8] scratch trees ─ sc-f107 (1.9 G: a git worktree at 4cd00139 + the control and three mutant
       binaries), sc-f108 (937 M), sc-f109 (365 M) join the list; `git worktree list` now carries
       sc-f107/tree beside sc-f89/tree

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] the canary re-bank (the block's own prescription for a per-boot red)
  [V2] archival pass 20, and the pass instrument written down (harness/fd_archive_pass.py)
  [V3] F108's shape under the fixed invariants: numbered rotation, the constant, the six lines,
       logread.py as the ONE reader of any cLog channel (ten drivers routed through it)
  [V4] four §5 rows at acceptances: §5.149 [Y1]; §5.150 (the orbit-line sampler perturbs the
       position seed — new path; the old path's batchLastE shape named as the fix, S); §5.151 [Y5];
       §5.152 [Y3]
  [V5] F109's STOP endorsed: no engine line moved; §11.82(b)'s attribution marked REFUTED
  [V6] two b22 instrument fixes at the root (goto convergence under a body reference; the kill by
       /proc/<pid>/exe + the config restore as an EXIT trap — the runner had killed its own caller)
  [V7] this block; supervised-by.sh and purge-path.sh UNTOUCHED by decision

  HELD OPEN, not absorbed
  [H1] the concurrent-instance probe is BLIND to staging binaries (comm == "spacecrafter" exactly,
       in f26_epoch.sh / f27_reply.py / f56_canary.sh) — measured with two live; the exe-identity
       probe is next round's first item (S); §0.5 corrected meanwhile
  [H2] §5.150's fix (a separate plot seed in OrbitModule) — S; the predicted one-frame transient
       (834 arcsec, Pasiphae, high time rates) still uncaught in the engine
  [H3] with orbit lines ON the OLD path's own moons move by up to 1.87e-07 AU through the static
       ephemeris cache — an instrument caveat at §5.84 for any parity run with the lines on
  [H4] §11.207(g)'s list carries three "decision-free" labels refuted at their rows this session
       (§5.66, §5.71, item 6) — the list is re-cut before anything is dispatched from it again
  [H5] §5.115's density half; EntityCore's own uncapped EntityCore-logs-<epoch>.txt pile (387
       files from unclean exits, read-only submodule) — yours
  [H6] dumpread.load_dump cannot read F100's pinned_*.json.gz (AttributeError, dumpread.py:157; my
       own probe at the mint) — the fourth-reader family, instrument residue
  [H7] the M flags 11.229 / 11.196 (the entry-wins shape, named)

  FACTS, no decision asked
  [F1] three for three delivered AND supervisor-verified same session, every delivery re-run by my
       own hand; code 7ceea976 → fcc277c9 (ONE executor commit, F108's three files); binary
       e411b838 → 6d63e6c1
  [F2] the host: rebooted 20:06:16, same boot since; sessions 5/6 and :2 at open and close; RAM
       53–54 GiB; 17 canary runs (one red by design, then green throughout); ~54 launches; no lock
       file at any check; the field pair in == out throughout
  [F3] thirteen dispatcher defects (5 value, 8 structure), all output-side, all caught before a
       delivery — two new sub-classes named in Q-67 (a two-range sed; a per-round value in prose)
  [F4] unpushed at close: 99 code / 857 harness (measured 01:11:13); the push is yours, Saturday
       → PUSHED 2026-09-12 11:03–11:04 by you, 0 / 0 at 11:13:30 (§11.232(b))
  ```


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
               same tree, author, date, subject, same parent 1ddd32f0 — rejoining at ba9df31f;
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
       DELIVERED 18:4x → §11.228 (F106), accepted by my own run on the scratch pair: 3829 stays 3829,
       cebebf44 keeps its sha and its gpgsig, 15 of 15 side-chain shas kept, the merge keeps both
       parents, 0 signature warnings; on the live pair 83 of 98 commits will change sha (82 yours as
       Claude + 30ea1f8d on top of one of them), 15 keep their object — NOTHING is left for you to
       decide before the run; §5.147 FIXED

  [Y4] the tool you operate Saturday ─ what changed, in the order you will meet it
       state   RUNNABLE NOW, and was not before this round: build_map's `[ ] && printf` returned 1
               on its last iteration and `set -e` killed every run AFTER the code rewrite and
               BEFORE rollback (D1: 73 of 138 trailers left dangling, in silence); build_repair_map
               re-derived "already dangling" AFTER the rewrite and aborted every run on a pair with
               a pre-existing dangling trailer (D2) — this pair has 2 (e5f0889b, 5ad587af, twins
               unique). Both fixed, each with a mutant on record
       B1      a renamed branch now STOPs the preview (exit 1, `--dry-run` too) with a §2(f) block;
               `--branch-alias=master-beta=<new-name>` is the one flag that proceeds; the rename
               and the tool are now order-INDEPENDENT (F92's checklist needs no reordering)
       maps    claude/sha-maps/<UTC>-<code8>-<harness8>/{code,harness,repair}.tsv + a written-once
               README, committed by the closing commit (unconditional once a map exists) — your
               "commit tracking" was READ as this map; your correction may differ
       today   `--dry-run` on the live pair: 94 code / 805 harness selected, 138 distinct trailers,
               2 already dangling, no STOP; deterministic (two runs, byte-identical maps)
       skip    ONE pin the parser SKIPS silently: harness d1f8fea's trailer reads
               `Code: master-beta @ 3ffea018 (the mutation is not in it)` — outside the grammar; the
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

  [Y3] §5.146 ─ ANSWERED [vixy 2026-09-12] → §11.233(f): "it should throw whenever it can't allocate" —
       the shape; EntityCore, coupled to your submodule push before the intern's clone (§11.204(b));
       was: carried: minted on your word "edit it" (SharedBuffer's bind/release guards + the
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
       own hand; code 4cd00139 → 7ceea976 (four executor commits); binary b5f08778 → e411b838
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
       ANSWERED [vixy 2026-09-12] → §11.233(b): GROW to 1 GiB, fixed, single-session too ("a pool of
       1 Gio is fairly generous and won't be critical either") — F113, session 29; the D13 reading on a
       smaller BAR heap measured there as the veto point on the number
       CORRECTED [vixy 2026-09-12 12:3x] → §11.233(b): "1 GiB was for the logs … app.cpp:274 … should stay
       very small … 1 MiB is sufficient" — my misread; the pool STAYS 1 MiB; asked for a shape he asked the
       ROOT ("why 1344 B … why so many") → F115; the 1 GiB is §5.115's size bound → F116
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
       ANSWERED [vixy 2026-09-12] → §11.233(e): "that's for the main user/tester, no ?" — ROUTED to the
       tester (§11.207(i)); §5.21's halves stay behind his answer
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
    endorsed):** (a) **F96's fix** (code `58655d7a`, `Camera.{hpp,cpp}` only): `observedToLocalPos`
    is the inverse of the RENDER rotation, so the alt/az readout, the atmosphere's sun direction,
    the view-directed descent and the tracking feedback read ONE expression — 27.000007° off old
    with `set zoom_offset 0.3` armed before, 0.000017° after; byte-identical at every shipped
    default (the F91 tables, the dwell frame `5215565b`); the tracking site no longer undoes the
    offset by hand; the pole branch of `observedPosToRaDe` answers RA 0 / DE ±90° as old does.
    (b) **F97's latitude** (code `61da4edc`, `orbit.cpp` ONE token: `lat(_lat)` →
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
    endorsed):** (a) **F91's fix** (code `012105d6`, `src/experimentalModule/` only): the RA/DE
    readout is `viewMat`'s exact inverse landing in old's frame — including the FULL surface
    fold, whose constant `+π/2` was §11.4's "−90.0003° zero point" (so §11.4 closes with no
    constant anywhere); plus two port slips found and fixed in the same member (the LOCAL hour
    angle was built from the observer's LATITUDE; a bare `fmod` printed negative hour angles
    pre-J2000); plus `ModularObject::getEarthEquPos` re-pointed to the observer-centred
    authority (I1 — `Body::getEarthEquPos`'s contract; `set home_planet selected` feeds it
    back through the inverse map). Result: 88 of 90 bodies print old's RA/DE string byte for
    byte (Eris = the two trees' own 1.198° position gap; Puck = one arcsecond of float32).
    (b) **F94's two literals** (code `7509c809`, `core.cpp:2251/:2255`): `$body_selected`
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
    `f91_parity.py`); every harness value recorded before `012105d6` for the new path's
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
    one sentence: after the rename, do pull requests still target `2023-master`?** **[2026-09-12,
    §11.232(a)2: *"master-beta as is; no rename now"* — the rename waits on your "ready"; the
    intern starting this week reads the doc's `2023-master` sentence as it stands; the PR-target
    half is still unstated.]** Your line
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
    `d467740d`) — the ONLY engine change this round. (c) **One paragraph in
    `doc/developer-entry.md` §5**: the newcomer's 90-second smoke suite
    (`f90_rehearsal_run.sh`). (d) **The executor definition's footer line** (this close,
    `agents/opus-xhigh.md:89`): it named `Claude Fable 5` as the standing co-author, so
    three executor commits this round carry `Fable 5` and three `Fable 5.1` while the AUTHOR
    is `Claude Opus 5` throughout; now it says: the co-author is the SUPERVISING session's
    identity as the dispatch prompt names it (the projection regenerated, md5 asserted). No
    SHA amended. (e) **Scratch trees KEPT**: `/home/claude/sc-f89/` 1.5 GB (three binaries +
    a detached code worktree at `4cb2e298` in the pre-fix source state — `git worktree
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
    to be readable by him (the entry document points at it either way). **[ANSWERED
    2026-09-12, §11.232(a)2: *"Linux"*; *"developer-entry.md is his entry, harness readable"* —
    the developer is a part-time intern without an LLM, starting this week.]**
  - **VETO POINTS taken this session (implemented-and-live, each cheap to reverse;
    silence = endorsed):** (a) the EntityCore pin bump `f4ceb208` — the completing half
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
    `master-beta @ b45d3b58`), the 60 per-string pins folded in; the doc bar now
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
  - **[ANSWERED IN FULL 2026-08-31 → §11.186(a) + HOST-EVENTS `c4ad633`:
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
  - **f23_b33_control's S1/S2 skylock legs are red at `3ccfc6d8`** (F54 §11.171(f)):
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
  `ff3d1d94:doc/superscript.sts`).

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
  - **The tester rewrote `doc/superscript.sts` this morning** (`30ea1f8d`, +267/−67)
    and **eleven of the thirteen witness lines the ledger tracked are already
    fixed or removed** — including the invisible 0xA0 byte and both respell cases
    (`date_display_*`, `zrot/yrot`). That answers **SS-16** (fix in place vs keep
    as a historical document) **by action**: the historical witness now lives at
    `ff3d1d94:doc/superscript.sts`. Two survive: the `landscape … spacecraft on`
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
  (F15's script-channel gate leg, commit `26a39a0` 00:03:36). Structural
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
