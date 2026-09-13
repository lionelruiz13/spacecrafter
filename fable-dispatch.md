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

**Update [Claude Fable 5.1 2026-09-13, supervising session 30 — LovelyFoxDev, the DSO-DESIGN /
TRAIL-WALKER / REWRITE-CLASS round: F117 · F118 · F119]:** trigger = the §0b verbatim line and nothing
else. Open at **Sunday 18:53 CEST** — the owner's not-reliable window (prompt part 06): this session asks
NOTHING in-line and writes every open choice to §3 in Q-70's shape. Warm-up (every value from the command
beside it, Q-67; `date` 18:53:44 at the first probe): both trees CLEAN at open, code `87d429bd` / harness
`8a7bcd1` (session 29's close commit — no owner write since); definition-drift MATCH (`8e364a3a`); binary
`42d7982c` current (0 `src/` files newer). **THE HOST REBOOTED AGAIN AT 09:40:06** (the third boot in 38 h:
09-11 20:06, 09-12 09:51, 09-13 09:40); the owner re-provisioned the same real logind session on `:2` at
12:56:24 (`loginctl` claude **31/32**, cookie `.R3GVV3`, gnome-shell pid 18192 / Xwayland pid 18364, kernel
start 1789296984; `xdpyinfo` 2448x1332; the inherited `DISPLAY`/`XAUTHORITY` already correct); canary
`--no-scene` **exit 2** at 18:54:34 on `compositor.restarted` ALONE (NOTE `xserver.restarted`, every other
member EQUAL; GPU 255 MiB used / 31855 free, nothing resident), RE-BANKED in one VALUES-block edit with the
argument (the 2026-09-12 values struck not deleted), then **exit 0** at 19:00:29 and the FULL canary **exit
0** 19:00:31–19:02:17 — **12/12 in band, every delta 0.0, the band on a FIFTH boot** (`20260913-190031`);
HOST-EVENTS entry + the §11.174(h) decision flag (`98dad6f`). Config/ssystem md5 pristine
(`03fbee59`/`545a51ef`) in == out; no engine by comm/exe/port (`sc_instances.sh --assert`), port 7805 free;
RAM **52 GiB available of 59**, `-j24`; unpushed **6 code / 48 harness** at open (`origin` unmoved since his
2026-09-12 11:03 push; he pushes at the end of each week); next free §11 **240** after §11.239 (live ∪
archive, `max+1`). Live `### F` **7 → 0** by **archival pass 22** at OPEN (update-s28 + F110/F113/F114/F115/
F116/F111/F112, 462 lines = 121 + 53 + 48 + 46 + 46 + 46 + 53 + 47 + the doubled separator; manifest
`2026-09-13-pass22`, pre-md5 `64824b89` at `98dad6f` reproduced in-process AND from disk; `426a400`;
**nothing carried**; F113 archived as the WITHDRAWN record it is) **→ 3** by the mints below. Instrument
baselines at open (run 18:53, unchanged since the session-29 close): scan **284/357/145** · pair-check
**255/230/25/125** · D 36 · D2 12 · I 93 · I2 37 · M 95. QUEUE CONSUMPTION (the session-29 close's list, in
order): (1) pass 22 — DONE; (2) the owner's week answers — NONE arrived (Sunday; the ledger's tail is
§11.239, his last message §11.233), so the DSO design pass is first on its own standing: **§11.233(c) →
F117** (M — a DESIGN NOTE and no engine line: the census of the old layer BY COMMAND, the as-if criterion
applied per observable channel, dso3d + ojmMgr as `ModularBody` and tully as a Milky Way `BodyModule`, the
nine suspended items dispositioned, the slices named; §2's "B5 remainder" bullet struck at the mint —
disclosure before the first engine slice, which waits for his Saturday reading); (3) B41 — CARRIED: its row
says *"after the consolidation items unless the owner orders otherwise"* and he has not; (4) **the trail
walker's engine leg → F118** (S — a LATENCY problem, not an every-frame one: the dump's frame latency k by
`evalCount`, `residual(k)` predicted from the slice, the edge leg, the mutant only on the predicted-blind
STOP; no delivered engine change); (5) the tool residues → **F119** (M — RE-CUT on a measurement: all five
scedit grammar pins name `54a2b844`, which `git merge-base --is-ancestor` refuses on `master-beta` since the
owner's rewrite, and F110's rehearsal from a plain clone read the anchor gate **AT-PIN-BROKEN 4 / GONE 1**
(`artifacts/f110/rehearsal-table.txt:132-146`) — the intern's `ctest` fails on it THIS WEEK; so the instance
is a CODE commit FIRST (the pins repointed by the map's own convention, the gate's record re-recorded only
after the moved rows are looked at), the class second (step E reaches the code tree's citation carriers;
`purge-path.sh` persists its map; an unparseable `Code:` trailer is reported), proved on the F106 scratch
pair, never on the real trees — every change to the owner's tools a veto point he reads before Saturday);
(6)–(8) carried. Picks: **F117 → F118 → F119** — the design first (his reading is the gate on everything
after it), then the S measurement, then the tool work whose code half has the week's clock. Deliveries: all
to the parent (§11.240+, refreshed at each dispatch). Launch classes: F117 ZERO by default, F118 FUNCTIONAL
(`--no-scene`), F119 ZERO (tools on scratch repos only). Remotes: 6 / 48 unpushed at open; the owner pushes
at the end of each week [vixy 2026-09-12].

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
**Round outcome (session 29 close, 2026-09-12 18:0x — every time in this note is pasted `date` output; the close commit's own
clock is the stamp):** F110 → **§11.234** (the entry document followed by hand from a plain clone: eleven hunks corrected,
`f85_links.py` gains the commit-sha class, `claude/intent_resolve.py` born from the owner's own unresolvable pointers; the
`-O2`/`-Ofast` finding; `c5be42b` ABSENT for a network clone) · **F113 WITHDRAWN before dispatch** (§11.233(b) correction — the
1 GiB was the LOG budget, my misread) · F114 → **§11.235** + §5.100/§5.101 FIXED (99.03° → 8e-06° and 1e-05°; the ramp
predicted in closed form) + **§5.153 minted** at acceptance · F115 → **§11.236** (88 of 1344 bytes are a per-body
requirement; four decisions his) · F116 → **§11.237** + §5.115's SIZE bound FIXED (the rule chosen by 11 987 rotations
against 6; the startup replay) · F112 → **§11.238** (ONE probe home for 44 copy-pasted sites, the residual cross-account;
`gpu.headroom` a gate at 6144 MiB free; the live map taken after my precondition was corrected) · F111 → **§11.239** +
§5.150 FIXED on its iterative half (NESO 0.335 AU caught at rate 7e8; 71 of 75 improved, 0 worse) — **six for six
delivered AND supervisor-verified same session**, every engine binary F91'd by my own hand (`1fe630a4` each time). Code
`fcc277c9 → 87d429bd` (five executor commits), binary `6d63e6c1 → 42d7982c`, harness `16c68276 →` this close. SUPERVISOR
ACTS: the re-bank + HOST-EVENTS (`49de8d2`); §11.232 (`22a6728`); pass 21 (`1d07413`); the mints (`67b6a2b`, `b85dc60`,
`bf36530`+`c9a8764`, `e799aeb`); §11.233 with its correction (`d0a6fca`, `b615595`); six acceptances; §5.153 and B41; §0.5
rewritten; in `~/shared`: Q-70 (the reader's working set — a pointer that resolves only from the writer's side), Q-04 (the
misread, the second instance in five days), Q-67 ×2, Q-56 (the commit-chain shape). OWNER EVENTS IN-SESSION: five messages
(§11.232's three answers, §11.233, its correction) — the round re-cut twice on them; his rewrite + push at 11:03; his model
unloaded at 11:32 on my ask. HOST: rebooted 09:51:57 (the second boot in 14 h); sessions 10/11 and `:2` 2448x1332 at open and
close; a 27B model then a 21 GiB `java`; 25+ canary runs (one red by design, one red by VRAM, green throughout after 11:34);
~90 measuring launches by the executors + 8 by me; the field pair `03fbee59`/`545a51ef` in == out throughout; HOST-EVENTS
carries the boot, the rewrite, the VRAM red and the retired precondition. SUPERVISOR TALLY: **sixteen dispatcher defects**
(5 value, 10 structure, 1 intent), all output-side, none reaching a delivery, each accepted at its acceptance with its
class and its Q-home. EXECUTOR REPORT DEFECTS: none reaching a record (every executor re-measured its own baselines; two
refused mine). EXECUTOR criterion-integrity instances: **≥ 40** (predictions before every build, two of MY expectations
pre-registered FALSE and confirmed; a stated precondition obeyed for 33 minutes rather than re-decided; refutations
root-caused — the frame-rate floor, the local-clone transport, the one-burst model; every instrument shown able to fail by
a mutant; status words named, never quoted; a 901-line regex deletion caught by `git diff --stat` before `git add`).
Archival pass 22 (update-s28 + F110/F113/F114/F115/F116/F112/F111, live `### F` 7 → 0) DEFERRED to the next open.
NEXT-ROUND QUEUE, in order: (1) pass 22 at open; (2) whatever the owner's week answers unlock, the DSO design pass first
among design items (§11.233(c), M–L); (3) B41 after its zoom-half anchor; (4) the trail walker's engine leg (S); (5) the tool
residues now dispatchable: the rewrite's CODE-tree citations (`developer-entry.md`'s harness shas, scedit's `anchor_pin`),
`purge-path.sh`'s map, `list_code_trailers`' skip; (6) ON THE OWNER'S WORD: [Y10] the uniform options · [Y11] V3 the bound's
history shape · [Y12] §5.153 · [Y13] §5.149's shape · [Y14] §5.146's push · [Y5]–[Y7] the intern's gaps (this week) ·
[Y8] the build type · [Y9] the row convention; (7) the (g) tail re-cut; (8) riders. Remotes: unpushed at this close printed
by the close commit's own call; the owner pushes at the end of each week. BASELINES AT CLOSE: the pre-close values are
F111's — 284/357/145 · 255/230/25/125 · D 36 · D2 12 · I 93 · I2 37 · M 95; this close edits `INTENT.md` (one marker at
§5.150) and `HOST-EVENTS.md` besides this file — re-run after the edits and printed by the close commit's own call.


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
     positively mapped both ways (decoy 1 / without 0). **[BLIND TO STAGING BINARIES — measured 2026-09-12, F109 §11.231(j2): all three homes of the probe (`f26_epoch.sh:46`, `f27_reply.py:107`, `f56_canary.sh:470`) test `comm == "spacecrafter"` EXACTLY, and a staging binary's `comm` is its own basename (`sc_f109_iso`, `spacecrafter-pre`…), so the probe read 0 with two staging instances live and holding port 7805; every task that measured a `sc_f*` binary ran it blind. Until the exe-identity probe lands (next round, S): before a launch ALSO assert that no process holds port 7805 (`ss -ltnp`) and that no `/proc/<pid>/exe` resolves under a `sc-f*`/`sc_*` path.]** **[LANDED 2026-09-12, F112 §11.238 — the probe has ONE home, and the bracket before this one is history: before each measurement launch run `bash claude/harness/sc_instances.sh --assert <label>` (exit 2 = an engine is live; exit 4 = the probe could not run, a STOP) or `harness/f116_assert.sh <label>`, which adds the GPU gate `python3 claude/harness/sc_gpu.py --need bank` (exit 3 = not enough room; it reads `BANK_GPU_NEED_MIB` = 6144 MiB of FREE memory from `f56_canary.sh`'s VALUES block, derived from the app's own init log, and names every holder through `nvidia-smi -q -d PIDS` — `--query-compute-apps` is blind to graphics-only holders). In Python `from sc_instances import no_instance`, or nothing: `f96_offset.no_instance()` and five sibling functions already delegate. The criterion is the UNION of three channels, each covering the others' blind spot: `comm` (world-readable — the only cross-ACCOUNT channel — blind to any rename), `/proc/<pid>/exe` (sees copies and renames, never a command line; EACCES across accounts under `ptrace_scope=1`, measured 0 of 578), TCP 7805 (an engine that answers whatever its file is called). THE RESIDUAL is a renamed engine of ANOTHER uid with no server up — say so rather than claim coverage. Never `pkill -f` (§11.231(j)); `sc_instances.py --kill` signals by pid. The 2026-09-12 VRAM red is a GATE on BOTH canary arms now (`gpu.headroom`, FAIL exit 3 below the need); a `used <= N` line is the wrong shape — it refused a host with 27 GB free. The live map: a renamed engine read 0 by the old form and 2 by the probe (the engine is a two-process tree) — §11.238(n).]**
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
L1's word — the attributed halo floor α-scaled).** **Session-29 round (2026-09-12): F110 §11.234 · F113 WITHDRAWN (§11.233(b) correction; ids never reused) · F114 §11.235 · F115 §11.236 · F116 §11.237 · F112 §11.238 · F111 §11.239 — archival pass 22 (update-s28 + the seven) DONE at the session-30 open (`426a400`); live below: the session-30 mints **F117** (the DSO layer on the new path — a design note on the owner's direction, no engine line), **F118** (the trail walker's engine leg — a latency measurement, no delivered engine change), **F119** (the rewrite's code-tree citation class — the scedit pins first, then the tool).** **Session-28 round (2026-09-11/12): F107 §11.229 · F108 §11.230 · F109 §11.231 (a STOP, endorsed — the residual is the instrument's) — three for three DELIVERED and accepted; §5.149–§5.152 minted at acceptances; archival pass 21 (update-s27 + F107–F109) DEFERRED to the next open.** **Session-29 (2026-09-12, Saturday, the owner present): archival pass 21 DONE at open (`1d07413`); the owner's rewrite + push ran mid-warm-up and his replies are §11.232 (a part-time intern without an LLM starts on `master-beta` THIS WEEK — "consolidated first"); live below, dispatch order **F110 → F113 → F114 → F112 → F111** (his second message, §11.233, re-cut the round): **F110** (the intern's first day rehearsed by hand — `doc/developer-entry.md` followed literally from a plain clone on Linux), **F113** (§5.142's fix on his word — the uniform pool to 1 GiB, the D13 reading measured), **F114** (§5.100 + §5.101 on his word — `zoom auto in` tracks and `zoom auto initial` re-aims on the drawn path), **F112** (launch preconditions that see what they guard — the exe-identity instance probe in ONE home for 42 copy-pasted sites, and a GPU-headroom gate derived from the app's init sequence after the 2026-09-12 VRAM red), **F111** (§5.150's fix — the orbit-line sampler gets its own seed, the old path's `batchLastE` shape).** **[12:5x: F113 WITHDRAWN before dispatch (§11.233(b) correction); **F115** (§5.142's ROOT — the uniform consumers censused field by field on both paths, redundancy on three axes, the per-body requirement derived, a proposal with arithmetic; no engine change) and **F116** (§5.115's SIZE bound — 1 GiB total across the five channels, within-session rotation reusing F108's window) minted; order **F110 → F114 → F115 → F116 → F112 → F111**.]** **Session-29 round (2026-09-12): F110 §11.234 · F114 §11.235 · F115 §11.236 · F116 §11.237 · F112 §11.238 · F111 §11.239 — six for six DELIVERED and accepted; F113 WITHDRAWN before dispatch (record kept); §5.153 minted, B41 opened; archival pass 22 (update-s28 + F110/F113/F114/F115/F116/F112/F111) DEFERRED to the next open.**

---

### F117 — THE DSO LAYER ON THE NEW PATH, DESIGNED ON THE OWNER'S DIRECTION (§11.233(c), verbatim: *"the reach/visibility coupling if I'm not wrong, the as-if rule operate on the visible effects, or those which may became visible. Ideally, dso3d and ojmMgr must be bodies (modularBody), for tully it's harder because it is massively intanciated so it can't be handled this way. Maybe as a BodyModule of the milkyway ?"*) — a DESIGN NOTE, `claude/b5-dso-design.md` in the `b12-design.md`/`b31-design.md` form, and NO engine line: (i) the CENSUS of what the old layer IS — `Dso3d` (`inGalaxyModule/dso3d.hpp:40`, drawn by the IN_GALAXY executor alone, `inGalaxyModule.cpp:141`, fed by the `dso3d` command family, `commandDso3D` at `app_command_interface.cpp:1676`), `Tully` (`coreModule/tully.hpp:54`, drawn by IN_UNIVERSE at `inUniverseModule.cpp:115/:119`, and a SELECTION reach — `searchAround` at `core.cpp:1309`, `TullyWrapper : ObjectBase`), `OjmMgr` (`ojmModule/ojm_mgr.hpp:43`, seven reader files, the `in_galaxy`/`in_universe` OJM models of `14.sts`, §11.222(d)), `Oort` (`coreModule/oort.hpp:77`, drawn at `solarSystemModule.cpp:203` unless the gated-off pilot `OortModule` (`OortModule.hpp:36`; `ssystem_factory.cpp:545`, `core.cpp:467`) was instantiated) — every reader file, every command token, every config key (`oort_elements = 10000`, `flag_oort = true`, `oort_color`), every field script naming them (of the tester's `~/.spacecrafter/scripts`: dso3d 2 · tully 2 · oort 1 · `in_galaxy` 9 · `in_universe` 6; `ojm` 96, of which the mesh-module `body action load … .ojm` uses must be partitioned OUT); (ii) the AS-IF criterion (D8) applied PER OBSERVABLE CHANNEL — each visible effect of the old layer (the drawn output per executor mode, selection/search, labels and fonts, fades, script replies, the command family) classified visible / may-become-visible / never-visible, so that §11.96(c)'s reach-vs-visibility coupling (boundingRadius = culling extent AND navigational reach → reference hijack; the 2×-peg onset) is decided by the criterion, not by a peg; (iii) the DESIGN: `Dso3d` and `OjmMgr` content as `ModularBody` — which node of the nested tree they hang from (MilkyWay → SolarSystem → …, §11.217(b)), the loader, the modules, and what becomes of the executor-mode gating that draws them today under §11.223(e)(2) (*"a system which can be seen from outside as it is shown from inGalaxy mode"* — these bodies ARE the seen-from-outside content); `Tully` as a `BodyModule` of the Milky Way node, massively instanced, the INSTANCED batch service being A6/§11.51(e)'s named renderer role; each of §11.96(e)(1)–(6) and §11.98(f)(i)–(iii) ANSWERED by the criterion or LEFT to the owner with both readings and their costs; (iv) the SLICES — decision-free, sized, each with the check that can fail it and its parity instrument (`b5_oort.py`'s shape: a parity ratio plus the discrimination that the flag-off tree stays byte-unchanged), the first one dispatchable next round [M, read-only on the engine; harness only (the note, the entry, README); ZERO launches unless a live census is the cheapest enumeration of an observable (then FUNCTIONAL, `--no-scene`, said so in the report); veto points §3 — the design is DISCLOSED before any engine change, by the construction of this task]

**Why now / mandate:** B5's row (`INTENT.md:1190`, the DIRECTION marker of 2026-09-12); §11.233(c) — *"the design is not done … a DESIGN PASS (M–L), dispatchable, first among the design items"*; DEPLOYMENT-MAP T1.3 (`:82`) in T0's necessary set — the critical path's DSO item; the session-29 queue item (2); §2's "B5 remainder" bullet UNBLOCKED by the direction (struck at this mint). A design pass and NOT a build, on purpose: the direction is the owner's and the design changes the tree's shape — disclosure before action on a model shift; the first engine slice is the NEXT round's, after he has read the note (his window is Saturday).

**The reading the mint stands on [derived; each fact a premise line]:** four classes, three executor-mode draw sites plus the oort's conditional one, all owned by `Core` (`core.hpp:711-713`); `Tully` is ALSO an `ObjectBase` through `TullyWrapper` (`coreModule/TullyWrapper.hpp:30`, `tools/object.hpp`), i.e. selectable — a channel a draw-only census would miss; `OjmMgr` is read by the inGalaxy / inSandBox / inUniverse executors; the `dso` command (`app_command_interface.cpp:1638-1668`, `dsoHideAll`/`dsoSelectType`) is the 2-D nebula layer and NOT this — the `dso3d` family is `load` (`dsoNavInsert` / `dsoNavSetupVolumetric`) and `restart`; the pilot's findings travel: one sample-law authority, frozen seed, no shadow passes, the hijack mechanism, the 2×-peg onset (§11.96, §11.98, audited §11.102(e)); the `MilkyWayEnv` environment module (`environmentModules/MilkyWayEnv.hpp:25`) and the `MilkyWay` class (`coreModule/milkyway.hpp:64`) are the Milky Way's two homes today — the note says which one the Tully module hangs from and why; the new tree is nested already (§11.217(b)); §11.223(e)(2) is an ARCHITECTURAL line of the owner's, *"nothing dispatched from it"* — the note relates to it, it does not enact it. Precedents for the form: `b12-design.md` (§0 "what this note decides, and what it deliberately does not"; provenance grammar; a measurable per decided boundary), `b31-design.md`.

**Measured at dispatch (supervisor, 2026-09-13 19:1x–19:4x, code `87d429bd`, harness `426a400`):** the sites at the lines quoted; `OjmMgr`'s readers 7 files; the field keys and the script counts as quoted; `claude/b5-dso-design.md` absent; `/home/claude/sc-f117` absent; canary green on both arms on the re-banked boot (`20260913-190031`, 12/12); the owner ABSENT (Sunday) — nothing asked, every open choice goes to §3.

**Mandate:** (1) **CENSUS** (`b5-dso-design.md` §1): per class — what it draws, in which executor mode, from which data (catalog files, `14.sts`'s models), through which commands (every token, from `app_command_interface.cpp`/`coreLink`), which config keys, which UI hooks, which selection/search paths; the reader set BY COMMAND (the grep counts are premises — a file the census does not explain is a gap); the field's use (which of the tester's scripts name which commands — the `ojm` 96 partitioned into mesh-module uses vs `OjmMgr` uses); the pilot's state (what `createExperimentalOort` builds, what `solarSystemModule.cpp:196-203` gates). (2) **THE AS-IF TABLE** (§2): one row per observable channel found in (1) — visible / may-become-visible / never-visible, with the mechanism that makes it so; the reach/visibility coupling decided per row (which reach effects are visible → must be preserved; which are not → may be decoupled); the peg (§11.96(e)(5), §11.98(f)(i)) and the ramp (§11.98(f)(iii)) placed in the table, not argued outside it. (3) **THE DESIGN** (§3–§4): the node each content class hangs from and why (the as-if rows that force it); the loader (`ModuleLoaderMgr`, the `OortLoader` precedent, `moduleLoader/OortLoader.hpp:13`); the modules per body (draw, culling, selection — `TullyWrapper`'s role re-expressed); the INSTANCED path for Tully (A6/§11.51(e): generalize the live TAIL batch, never duplicate); the executor-mode gating's fate for these classes, related to §11.223(e)(2) without enacting it; each §11.96(e)(1)–(6) and §11.98(f)(i)–(iii) item with a disposition — ANSWERED (by which row) or OWNER's (both readings, the cost of each); what the design deliberately does NOT decide (§0, b12's form). (4) **THE SLICES** (§5): ordered, decision-free, sized S/M, each with its discriminating check stated as a check that can fail, its parity instrument, and the byte-unchanged assertion for the flag-off tree; the first slice written so a section can be minted from it. (5) **RECORD:** §11.⟨next⟩ FIRST + stub; B5's row annotated (design delivered, slices named, what stays the owner's); DEPLOYMENT-MAP T1.3 annotated; §11.96(e) and §11.98(f) marked at both homes (entry + stub) with the disposition table's pointer; `harness/README.md` F117 section only if an instrument was written; WIP per §0.6; D14.

**Boundaries:** NO engine change — `git -C /home/claude/spacecrafter status --porcelain` EMPTY at every harness commit; the note, the entry and the ledger annotations only; ZERO launches by default — a census launch, if any, is FUNCTIONAL on `:2` after `f116_assert.sh <label>`, `--no-scene` canary before it, config/ssystem md5 in == out, and is SAID in the report; no data, no EntityCore; nothing under `/tmp` carries; scratch, if any, under `/home/claude/sc-f117/`.

**Discriminating checks:** (a) every `file:line` the note cites resolves at HEAD to the text it claims (the executor's own resolution table — a cited line not holding its text is a defect); (b) the reader-set census is by command and complete (every file of the premise counts explained); (c) the as-if table covers every command token, config key and selection path the census found (a token without a row = incomplete); (d) every §11.96(e)/§11.98(f) item has a disposition; (e) each slice's check can fail, and says how; (f) zero engine diff; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-13; canary `--no-scene` exit 0 before any launch (if any); the owner's direction §11.233(c) unchanged (no later owner message in the ledger — the executor's warm-up reads §11's tail).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 87d429bd
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 42d7982c
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 240
grep -c '^### F' claude/fable-dispatch.md => 3
# the old layer's four classes, their owner and their draw sites, re-resolved at HEAD (content drift = abort)
grep -n '^class Dso3d' src/inGalaxyModule/dso3d.hpp | cut -d: -f1 => 40
grep -n '^class Tully' src/coreModule/tully.hpp | cut -d: -f1 => 54
grep -n '^class OjmMgr' src/ojmModule/ojm_mgr.hpp | cut -d: -f1 => 43
grep -n '^class Oort ' src/coreModule/oort.hpp | cut -d: -f1 => 77
grep -n '^class OortModule' src/experimentalModule/bodyModules/OortModule.hpp | cut -d: -f1 => 36
grep -n '^class MilkyWay:' src/coreModule/milkyway.hpp | cut -d: -f1 => 64
grep -n 'std::unique_ptr.Oort. oort;\|std::unique_ptr.Dso3d. dso3d;\|std::unique_ptr.Tully. tully;' src/coreModule/core.hpp | cut -d: -f1 | tr '\n' ' ' => 711 712 713
grep -n 'core-.dso3d-.draw' src/executorModule/inGalaxyModule.cpp | cut -d: -f1 => 141
grep -n 'core-.tully-.draw' src/executorModule/inUniverseModule.cpp | cut -d: -f1 | tr '\n' ' ' => 115 119
grep -n 'core-.oort-.draw' src/executorModule/solarSystemModule.cpp | cut -d: -f1 => 203
grep -n 'tully-.searchAround' src/coreModule/core.cpp | cut -d: -f1 => 1309
grep -rl 'OjmMgr\|ojmMan' src --include='*.cpp' --include='*.hpp' | wc -l => 7
grep -n '^void SSystemFactory::createExperimentalOort' src/bodyModule/ssystem_factory.cpp | cut -d: -f1 => 545
grep -n 'createExperimentalOort' src/coreModule/core.cpp | cut -d: -f1 => 467
grep -n '^int AppCommandInterface::commandDso3D' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 1676
# the field, the ledger, the precedents
grep -n 'oort_elements\|flag_oort\|oort_color' ~/.spacecrafter/config.ini | tr -d ' ' | tr '\n' ' ' => 53:oort_elements=10000 174:oort_color=0.0,0.5,1.0 226:flag_oort=true
for t in dso3d tully ojm oort in_galaxy in_universe; do printf '%s=' $t; /usr/bin/grep -rl "$t" ~/.spacecrafter/scripts | wc -l; done | tr '\n' ' ' => dso3d=2 tully=2 ojm=96 oort=1 in_galaxy=9 in_universe=6
grep -n '^| B5 |' claude/INTENT.md | cut -d: -f1 => 1190
grep -c 'DIRECTION 2026-09-12, §11.233(c)' claude/INTENT.md => 1
grep -n 'T1.3\*\* the reach/visibility batch' claude/DEPLOYMENT-MAP.md | cut -d: -f1 => 82
grep -n '^    - \*\*(e) Suspended for Vixy (6, enumerated)' claude/INTENT/11.96.md | cut -d: -f1 => 8
grep -n '^    - \*\*(f) Suspended for Vixy (new/measured)' claude/INTENT/11.98.md | cut -d: -f1 => 10
grep -n '^    - \*\*(e) T1 → ANSWERED BY THE OWNER' claude/INTENT/11.223.md | cut -d: -f1 => 13
grep -n '^    - \*\*(c) T1.3 — DIRECTION GIVEN' claude/INTENT/11.233.md | cut -d: -f1 => 9
test -f claude/b12-design.md -a -f claude/b31-design.md -a -f claude/harness/b5_oort.py && echo ok => ok
test -e claude/b5-dso-design.md ; echo $? => 1
test -e /home/claude/sc-f117 ; echo $? => 1
```

**DoD:** the note with its five sections; the census by command; the as-if table complete over the census; every suspended item dispositioned; the slices with checks that can fail; §11 entry + stub; B5 / T1.3 / §11.96(e) / §11.98(f) annotated; zero engine diff; trees clean; WIP cleared; baselines LAST.
**WIP:** — DELIVERED 2026-09-13 → §11.240 (entry + stub). `claude/b5-dso-design.md` (1023 lines, pure ASCII; 272 citations, 0 unresolved by `harness/f117_cites.py`); census by command (`harness/f117_census.py`) corrected FOUR of this section's own premises; the as-if criterion REQUIRES the reach/visibility split (`BMT_NO_REACH` + `reachRadius`, inert by construction); 5 of the 9 suspended items answered, 1 in part, 3 his; five slices, the first decision-free. Annotated: B5 row, B2/A6 row, DEPLOYMENT-MAP T1.3, §11.96(e) + §11.98(f) + §11.233(c) + §11.51(e) at both homes. ZERO launches, zero engine diff. Baselines: open 284/357/145 · 255/230/25/125 · D36 D2-12 I93 I2-37 M95 → close 284/357/145 · 256/231/25/125 · D36 D2-12 I93 I2-37 **M96** (+1 = this pair, named at §11.240(l)). **ACCEPTED 2026-09-13 — verifying commands' `date` 20:0x (supervisor, session 30, Claude Fable 5.1).** Verified by my own reads and runs: §11.240 read in full; six harness commits `385fe60 → 2e0f0eb`, every one authored Claude Opus 5 with the `Code: master-beta @ 87d429bd` trailer and the supervising footer; code tree UNTOUCHED (`87d429bd`, porcelain empty, binary `42d7982c`, zero launches — the section's default, so no canary was owed); the stub at `INTENT.md:1115`; the B5 row (`:1192`) and DEPLOYMENT-MAP T1.3 (`:82`) carry the delivery; §11.96, §11.98, §11.233 and §11.51 each carry the §11.240 marker; `### F` 3; instruments by my own run **284/357/145 · 256/231/25/125 · D36 D2-12 I93 I2-37 M96** to the digit of (l); D14 PASS; the note 1023 lines, 0 non-ASCII bytes. The four premise corrections RE-VERIFIED AT THE SOURCE by my own commands: `ojmMan` 0 hits and `ojmMgr` at `core.hpp:727` (the intended set is 8, `coreLink.cpp` holding 3 uses); `commandDso2D` (`:1695`) → `Core::loadDso2d` → `dso3d->loadCommand` (`core.cpp:2635-2638`) while `dso3d` reaches `core->dsoNav` (`coreLink.cpp:841`); `core.cpp:479-486` loads `Milkyway/Milkyway.ojm` in the `else` arm; `\bINSTANCED\b` 0 in `src/`; the scripts tree 2772 files, 2308 media by extension. The note's §0 (decides / does not decide), §3.2 (the split — both channels visible, one quantity cannot serve both, the shape inert by construction, four alternatives with their defects), §4 (5 / 1 / 3) and slice 1 (predictions + a mutant that must move ~533 AU and no pixel) read: the argument holds and the check can fail. DEVIATIONS ENDORSED with the executor's arguments: §3 not written (my prompt bounded it — the (m) items are lifted at close); the note spelled `S11.x` (pure ASCII, the gate cannot classify a harness file, the entry uses `§`); no launch and no canary (the census was cheaper by command); the A6/§11.51(e) annotation beyond the mandate's list — the row claimed a mechanism the tree does not hold, and the maintenance invariant wants that said at both homes (A6 itself NOT discharged, correctly). DISPATCHER-SIDE FINDINGS, ACCEPTED as mine, all output-side, all STRUCTURE class: (1) `ojm = 96` — a `grep -rl` over a media tree read as a corpus count (the class s29 named "a grep read as a census", third instance); (2) the script word `dso3d` bound to the class `Dso3d` by NAME — my own reading line even quoted the `dsoNav*` calls it routes to and did not see they do not reach `Dso3d`; (3) `oort = 1` / `tully = 2` a path and an image directory (class of (1)); (4) `ojmMan` — a member name typed from memory that no listing ever showed (Q-67's structure sub-class), whose count REPRODUCED while its set was wrong — the sharpest lesson of the four: a premise about a SET must list the members, not count them (the instrument verifies reproduction, never meaning). Round tally: **four** (0 value, 4 structure). MINT DECISIONS: F117-a..f NOT minted here (§5.79's criterion is met by F117-a and F117-b on the face of it; both are the old path's and B5's slices 2–3 own their sites — held at §3 for the owner's ordering); the seven owner items and four veto points go to §3 at close verbatim from (m). STANDING: slice 1 is dispatchable and decision-free; `OjmMgr` is the field's DSO layer (560 lines); the `[:line]` shorthand is unsafe across a file change (`f117_cites.py` runs on any note); `f117_census.py` is the shape of a field census.

---

### F118 — THE TRAIL WALKER'S ENGINE LEG: §11.239(h)'s SLICE NUMBER (Elara 8.92e-05 AU = 3.3 arcsec, ONE frame after ONE unhide) MEASURED IN THE ENGINE, OR BOUNDED THERE WITH THE NUMBERS — `TrailModule::resumeAfterHidden` (`TrailModule.cpp:169`) re-evaluates `missed = (date − lastJD)/deltaTrail` samples (`:190`; `deltaTrail = 1` sim day, `TrailModule.hpp:193`; `missed > maxTrail = 1460` resets instead, `:193-199`) each `1 + RESUME_EXTRA_ITERATIONS = 5` times at its OWN date (`:229`, `ModularBody.hpp:338`) through the POSITION solver (`:233`, `iterativeLastE`), so the seed ends converged at `lastJD + missed·deltaTrail` — up to one `deltaTrail` behind the frame — and the body's NEXT evaluation (two Newton steps per call, `iterative_orbits.hpp:31`) meets a stale seed: the third walker F111 named, priced on the slice (`artifacts/f111/trail_walker.txt`: Elara 8.9167e-05 AU 3-D / 6.67e-06 radial, Leda 8.01e-05, Sinope 5.26e-05, all on the `ell<0.2` fixed-point branch whose error after two steps is LINEAR in the staleness; Newton-branch bodies 1e-12..1e-16) and disposed a RIDER because its fix is a design choice with no old-path shape to restore; it fires on an EDGE (the unhide), so §11.239(b)'s every-frame regime does not exist for it and the engine leg is a LATENCY problem: how many frames after the unhide does a dump arrive (`evalCount` deltas, §11.239(l)1), and what is left of the transient by then? [S, harness only; NO delivered engine change; FUNCTIONAL launches on `:2`, `--no-scene` canary, `f116_assert.sh` before every launch; ONE scratch-tree mutation under `/home/claude/sc-f118/` allowed only if the dump's latency puts the transient below the floor (F107's shape, §11.229); veto points §3]

**Why now / mandate:** §11.239(h) — *"an engine leg needs a dump on the frame after an unhide, and the walker fires on an EDGE … That leg is owed"*; §11.239(l)7; §5.150's marker names the trail walker a rider with its engine leg owed; §3 [H3]; the session-29 queue item (4). Decision-free: a measurement. The §5 mint (does the engine number meet §5.79's *"with consequence"*?) is the supervisor's at acceptance; the fix shape is NOT this task's (§11.239(h): every available change is a design choice).

**The reading the mint stands on [derived; each fact a premise line]:** the hide channel is `body name <X> hidden true|false|toggle` (`app_command_interface.cpp:4218`, `coreLink->setPlanetHidden`); `renderHidden` has ONE writer (the owning system's walk, `ModularBody.hpp:2076`) and `useNow` is a no-op for a walked body (`ModularBody.cpp:456-457`), so a dump reads the walk's own position (§11.239(l)2) — the same fact that made F111's catch readable; the trail flag is `body_trace` (`commandBodyTrace`, `app_command_interface.cpp:1748`; `TrailModule::show` at `TrailModule.cpp:16`, a per-name override `nameOverride`); at the walk, `date` is the body's own sim time *"already brought to the current frame by the D8 barrier before this call"* (`TrailModule.cpp:187-189`) — so the transient sits on the frame AFTER the unhide frame, not on the unhide frame itself; `missed ≥ 1` needs `date − lastJD ≥ 1` sim day across the hidden span (a `date` jump or a rate), and `missed ≤ 1460` or the trail resets and nothing is walked; at a PINNED clock the frame after the unhide re-evaluates at the same date, two steps from the stale seed, and every later frame two more — the residual decays per frame on the fixed-point branch (the rate of that decay is what the slice model gives, `f107_model.py`; `f111_predict.py chain` models the frame chain); the dump's latency k in frames is read from `evalCount` (`ModularBody.hpp:709/:871`), F111's `frames_between`; F111 measured 167-168 frames between two dumps with the flag off at rate 1e8 — the channel's own latency is the first number this task must own. The controls F111 established: the within-launch control (same launch, same jump, no hide → the D8 barrier's own post-jump residual, F111 (c)'s floor) and the two-date control that must fail.

**Measured at dispatch (supervisor, 2026-09-13 19:1x–19:4x, code `87d429bd`, harness `426a400`):** the sites at the lines quoted; the slice file with Elara's row; F111's instruments present; `/home/claude/sc-f118` absent; the binary `42d7982c` (F111's delivered one — the sampler fix is IN it, so the sampler cannot confound this leg at a pinned clock: §11.239(f) 0 of 40 with the flag on); canary green both arms on the re-banked boot (`20260913-190031`).

**Mandate:** (1) **PRE-REGISTER** (`artifacts/f118/prediction.txt`, before any launch): for the walked `ell<0.2` bodies (Elara, Leda, Sinope, Himalia, Lysithea at least) the residual on frame +1, +2, … +k after the walk at a pinned clock, from the slice (the seed's staleness `deltaTrail·n` less the fraction the `missed` floor leaves — state it per body — then two fixed-point steps per frame); the dump latency k you EXPECT from F111's `frames_between` (a range, with its source); therefore the observable predicted for the dump, `residual(k)`, per body, against the F111 threshold (5.6e-07 AU, 100× the model floor) — and the STOP rule: if `residual(k)` is below the threshold for every body at the low end of the k range, (2) is predicted BLIND and (3) is entered after (2) confirms k; the within-launch control's expected value (the D8 barrier's post-jump residual at the chosen jump); the two-date control. (2) **THE EDGE LEG** on `42d7982c`: `body_trace` on for the named bodies (+ one Newton-branch control body, Neried), F100's pinned clock; hide by name; jump the date by N sim days with `1 ≤ missed ≤ 1460` (state N and the `missed` it yields); unhide; dump AS FAST AS THE CHANNEL ALLOWS and read k from `evalCount`; the within-launch control (same launch, same jump, no hide); the two-date control. PERTURBED per record at the F111 threshold, `f111_rate.py`'s reader reused or extended (say which). If the transient is caught: name, magnitude, k, vs `residual(k)` predicted. If not, and k is what (1) said makes it invisible: STOP (2) with the numbers, go to (3). (3) **THE MUTATION**, only on that STOP: one worktree at `87d429bd` under `/home/claude/sc-f118/`, ONE change that makes the frame-after-the-walk readable without changing what the walk does — the cheapest is a log line at the end of `resumeAfterHidden` and in the body's next evaluation printing `|pos − converged|` for the walked body (the executor may choose the F111 shape instead — a walk re-fired every frame at a running clock — and says why); RED on the mutant at the predicted magnitude; the mutant's control (the same build, the walk's seed re-converged by `1 + RESUME_EXTRA_ITERATIONS` calls at `date` after the loop) at the floor — which is also the measured PRICE of one of the candidate fix shapes, stated as such and not taken. (4) **THE NUMBER's HEIGHT** (Q-71): the engine number vs the slice's, in AU, in arcsec at the body's distance from the observer of the scene, and as the OBSERVABLE it lands on — the body's drawn position on frame +1 (the trail POINT written by the walk is the walk's own converged sample; say which observable carries the transient and for how many frames at 144 fps). (5) **RECORD:** §11.⟨next⟩ FIRST + stub; §11.239(h) and (l)7 marked at both homes; §5.150's rider marker annotated with the engine number and k (the row's state unchanged — the mint decision is the supervisor's); the candidate fix shapes NAMED with their arguments and, for the re-converge shape, its measured price from (3) — NOT taken; `harness/README.md` F118 section; WIP per §0.6; D14.

**Boundaries:** NO delivered engine change — `git -C /home/claude/spacecrafter status --porcelain` EMPTY at every harness commit; the scratch worktree is the only place a line changes and it is never installed; FUNCTIONAL launches on `:2`; `bash claude/harness/f116_assert.sh <label>` before every launch; `--no-scene` canary before the first; config/ssystem md5 in == out per launch; explicit timeouts (Q-68); no `run_in_background`; nothing under `/tmp` carries; every scratch binary under `/home/claude/sc-f118/`.

**Discriminating checks:** (a) the predictions committed before any launch, k as a range with its source; (b) k measured by `evalCount`, never assumed; (c) the transient caught (name, magnitude, k, vs `residual(k)`) OR the below-floor STOP with the numbers, then the mutant RED at the predicted magnitude and its control at the floor; (d) the two-date control fails; (e) the within-launch control (jump without hide) reads the floor; (f) no delivered diff; (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; `:2` per HOST-EVENTS 2026-09-13; canary `--no-scene` exit 0 before the first launch; F111 DELIVERED (the binary carries the sampler fix — its md5 is a premise).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 87d429bd
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 42d7982c
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 241
grep -c '^### F' claude/fable-dispatch.md => 3
# the walker, its constants, the hide and trace commands, re-resolved at HEAD (content drift = abort)
grep -n '^void TrailModule::resumeAfterHidden' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 169
grep -n 'const int missed = static_cast.int.((date - lastJD) / deltaTrail);' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 190
grep -n 'for (int i = 0; i <= RESUME_EXTRA_ITERATIONS; ++i) {' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 229
grep -n 'orbit-.positionAtTimevInVSOP87Coordinates(date, sampleJD, tmp);' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 233
grep -n '^bool TrailModule::show = false;' src/experimentalModule/bodyModules/TrailModule.cpp | cut -d: -f1 => 16
grep -n 'double deltaTrail = 1;' src/experimentalModule/bodyModules/TrailModule.hpp | cut -d: -f1 => 193
grep -n 'int maxTrail = 1460;' src/experimentalModule/bodyModules/TrailModule.hpp | cut -d: -f1 => 192
grep -n 'constexpr int RESUME_EXTRA_ITERATIONS = 4;' src/experimentalModule/ModularBody.hpp | cut -d: -f1 => 338
grep -n 'constexpr int ITERATIVE_STEPS_PER_CALL = 2;' src/bodyModule/iterative_orbits.hpp | cut -d: -f1 => 31
grep -n 'if (!renderHidden || !parent)' src/experimentalModule/ModularBody.cpp | cut -d: -f1 => 457
grep -n '++evalCount;' src/experimentalModule/ModularBody.hpp | cut -d: -f1 | tr '\n' ' ' => 709 871
grep -n 'coreLink-.setPlanetHidden(args\[W_NAME\], true);' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 4218
grep -n '^int AppCommandInterface::commandBodyTrace' src/interfaceModule/app_command_interface.cpp | cut -d: -f1 => 1748
# the slice, the ledger, the instruments
grep -c '^Elara.*8.9167e-05' claude/harness/artifacts/f111/trail_walker.txt => 1
grep -c '(h) THE TRAIL WALKER' claude/INTENT/11.239.md => 1
grep -c 'trail walker' claude/INTENT.md => 1
test -f claude/harness/f111_rate.py -a -f claude/harness/f111_predict.py -a -f claude/harness/f107_model.py -a -f claude/harness/f100_run.sh -a -f claude/harness/f116_assert.sh && echo ok => ok
test -e /home/claude/sc-f118 ; echo $? => 1
```

**DoD:** predictions before any launch; the edge leg with k measured; the catch or the STOP-then-mutant with both controls; the number's height; §11 entry + stub; the §11.239 / §5.150 markers; README; no delivered diff; trees clean; WIP cleared; baselines LAST.
**WIP:** — DELIVERED 2026-09-13 → §11.241 (entry + stub). NO ENGINE CHANGE (code `87d429bd`, porcelain empty at every commit; the scratch mutant leg (3) NEVER ENTERED — the STOP did not fire, as pre-registered). **THE NUMBER: CERES 6.8877e-05 AU = 4.33 arcsec on EXACTLY ONE FRAME** (k=1 by `evalCount`), A/A floor EXACTLY 0 (bit-identical `ecl`), predicted 6.8954e-05 at the arm's own measured staleness — ratio 0.999; 4.71e-07 at k=2, below the dump's print at k=3; Arrokoth 4.25e-06, everything else 0. **§11.239(h)'s CORPUS REFUTED IN THE ENGINE**: no satellite carries a TrailModule (`ModularBody.cpp:940-946`, `!isSatellite()`), so not one of its 40 slice rows — Elara included — can run the walker; the reachable corpus is 18 bodies (9 SpecialOrbit with NO iterative seed, proved by making Mars and Pluto reconstruct 358 samples each and move by not one bit; 6 comet_orbit on a Newton branch; 3 `ell_orbit`), **10 of them SHIPPED HIDDEN** and recording nothing until an operator shows them. Channel: TCP blind at k 195–714 (0 on every body); a `.sts` script with `wait duration 0.001` separators answers at k = 1,2,3,5 EXACTLY. Controls: the mandate's "jump without hide = a floor" PREDICTED FALSE before the first launch and measured **246x LARGER**; two-date control 9 of 9; the walk's own trail POINTS converged to their float32 storage (9.11e-08 AU at Ceres's headJD) — the transient is in the body's position, not in anything drawn. Four fix shapes named and priced (the re-converge shape = 5 calls against the 1790 the walk already makes, 0.3 %, and its RESULT is the measured trails-off arm), **NONE TAKEN** — the §5 mint is the supervisor's. **ACCEPTED 2026-09-13 — verifying commands' `date` 20:5x (supervisor, session 30, Claude Fable 5.1).** Verified by my own reads and runs: §11.241 read in full; six harness commits `8f38b7c → 17cebe7`, every one authored Claude Opus 5 with the `Code: master-beta @ 87d429bd` trailer (6 of 6) and the supervising footer; code tree UNTOUCHED (`87d429bd`, porcelain empty, binary `42d7982c`, the scratch leg never entered); the stub at `INTENT.md:1117`; §11.239 carries two §11.241 markers and its stub a third, §5.150's row the rider annotation; `### F` 3; instruments by my own run **289/359/145 · 257/232/25/125 · D36 D2-12 I93 I2-37 M96** to the digit of (m); D14 PASS; canary `20260913-204233` on disk. The load-bearing claims RE-VERIFIED AT THE SOURCE by my own commands: `ModularBody.cpp:940-946` gives `TRAIL` only to `orbit_visualization_period > 0 && !isSatellite() && !artificial`; `commandBodyTrace` (`:1748`) reaches `coreLink->bodyPenDown()` (`:1757/:1798`) — the old pen; `flag object_trails` is the seam (`TrailModule.hpp:43/:154`, `setGlobalShow`); `artifacts/f118/edge2_report.txt` carries k per arm by `evalCount` (script arms 1/2/3/5, TCP 195–714) and the reconstruction table. DEVIATIONS ENDORSED with the executor's arguments: the `body_trace` label re-adjudicated (the object and intent were right, the token wrong, decidable in one lookup — MINE); the corpus re-cut mid-task with the corrected predictions committed before scoring (output-side; the abort counterfactual stated); the `.sts` one-frame channel in place of the mandate's "dump as fast as the channel allows" (a better instrument, and it retires a claim this ledger carried as general); the canary bracketing two launches (the third delivery in a row to report this bracket — the rule is re-cut at §3, not silently); the F111 threshold not transferred (the arm-to-arm statistic with a measured zero floor is the stronger criterion). DISPATCHER-SIDE FINDINGS, ACCEPTED as mine, all output-side, all STRUCTURE class: (1) `body_trace` bound to the trail flag by NAME (the second name-match this round, after `dso3d`); (2) the whole named body set unreachable — the slice priced over the walked-iterating set, the walker's set is the non-satellite one, and I carried the slice's rows without reading `deduceBodyModuleList` (a reach asserted from a slice); (3) the within-launch control called "a floor" — a mechanism carried from F111's context where `useNow`'s early return, which F111 itself recorded at (l)2, makes it 246× the signal; (4) the dump channel taken as THE channel — an enumeration at a sink (I9): the `.sts` channel was one grep away. Round tally: **eight** (0 value, 8 structure). MINT DECISION: **§5.154 MINTED at this acceptance** — the walker leaves the unhidden body's seed up to one `deltaTrail` stale, one frame of 4.33 arcsec on Ceres (21.8 body radii; 1 px at fov ≤ 2.94°), a departure from D23's as-if-never-hidden intent, reachable from shipped commands on the ten bodies the field ships hidden; fix shape 1 (re-converge after the walk, 0.3 % of the walk's own cost, its result the measured trails-off arm) the named candidate, the shape a veto point; the `IterativeEll` fold (i) NOT minted (modelled only) — queued as a measurement leg of its own (S, a running clock on the comet family, F111's shape) and named at §3. STANDING: a `.sts` script with `wait duration 0.001` separators is a one-frame command channel; a module census is a dump field (`"trail":[]`); ten of eighteen trail bodies ship hidden; `maxTrail` 60 on the Asteroid class. Marked: §11.239(h), §11.239(l)7, §11.239's stub, §5.150's rider (row state untouched). README F118. RECORDED NOT CHASED: `IterativeEll` folds the mean anomaly while its seed does not — modelled to 731.9 AU once per orbit, shared solver, BOTH paths, no hide needed, and unreachable from any date this task could set (the search is in the record). FOUR dispatcher findings, all output-side, each with its counterfactual — the loudest: `body_trace` is the old BodyTrace PEN, not the trail flag (`flag object_trails` is the seam), registered BEFORE the first launch. 2 measuring launches, `f116_assert` clear on both, md5 `03fbee59`/`545a51ef` in==out on both, canary exit 0 at 20:22:50 and 20:42:33 (BRACKETING, not per-launch — deviation reported). D14 PASS before every commit. Baselines: open **284/357/145 · 256/231/25/125 · D36 D2-12 I93 I2-37 M96** → close **289/359/145 · 257/232/25/125 · D36 D2-12 I93 I2-37 M96**, the uncredited set IDENTICAL LINE FOR LINE against the pre-delivery tree; two flags raised by the writes and both closed at their cause, named at §11.241(m).

---

### F119 — A COMMIT CITATION INSIDE THE CODE TREE IS OUTSIDE THE REWRITE TOOL'S REACH, AND THE INTERN'S `ctest` FAILS ON IT THIS WEEK: `supervised-by.sh`'s step E repoints tracked `*.md` in the HARNESS repo only (`supervised-by.sh:184/:190`), so the owner's 2026-09-12 rewrite (83 code commits re-sha'd; the map `claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/code.tsv`, 86 lines) moved `54a2b844 → b45d3b58` and `ba7a32a8 → 8d41fbe3` and left the SIX scedit grammar pins — `_meta.anchor_pin` in `util/scedit/grammar/args/unit-{1..4}.json:6` and `sc-grammar.json:7` (five `54a2b844`) and in `ss-grammar.json:7` (`ba7a32a8`) — naming objects no branch reaches (`git merge-base --is-ancestor` exit 1 on both; a NETWORK clone does not even carry them, §11.234(e)); `util/scedit/tests/anchor_gate.py` (ctest target `anchor_gate`) is built to fail exactly there, and F110's rehearsal from a plain clone read **AT-PIN-BROKEN 4 / GONE 1, moved 321** (`artifacts/f110/rehearsal-table.txt:132-146`, RIDERED at §11.234) — `doc/developer-entry.md` cites harness shas the same way (`:282/:437/:476/:498`: reachable TODAY, dangling after the NEXT Saturday run) — plus the two residues §3 has carried since session 27: `purge-path.sh` rewrites the same way and persists NO map (0 `sha-maps` mentions in 35 KB), and `list_code_trailers` (`supervised-by.sh:388`, the `sed -nE … /p` at `:390`) DROPS a `Code:` line its regex does not match, silently. THREE fixes in TWO ACTS, in this order: **ACT 1 — the instance, ONE CODE commit:** the six pins repointed BY THE MAP (the tool's own documented rule, `supervised-by.sh:206-210`: *"resolved by CONVENTION — look the token up as a PREFIX in the maps, newest first"*), `anchor_gate.py` run, its `moved` rows LOOKED AT (which delivery moved each handler — this week's own, F110–F116 — and whether the anchor still names the same construct at its new line), and `tests/anchor-expected.txt` re-recorded with `--record` ONLY then, the look in the commit message — the gate's own contract (`util/scedit/README.md:789`: *"a record, not a silencer … edited deliberately, never regenerated blind"*); **ACT 2 — the class, harness commits:** (a) step E's file set gains the CODE tree's tracked citation carriers, found by the same prefix lookup over the merged maps (the set MEASURED at HEAD — expected the entry document and the six grammar files — never assumed), repointed in the same run as ONE code commit whose EXPECTED set is proved the way the harness closing commit's is (`:216-218`), under the same preview and y/N; (b) `purge-path.sh` writes its map into the SAME drawer in the SAME format (`<old-full-sha><TAB><new-full-sha>`, `map_lookup`'s contract at `:427-431`) — ONE convention, the executor states how the two scripts share the writer; (c) an unparseable `Code:` trailer is REPORTED at step A in §11.212's shape (a §2(f) block: the line, the commit, what the parser expected) and never dropped — each proved on the F106 SCRATCH PAIR (`harness/f106_pair.sh`, STATE U per `artifacts/f103/setup.txt`): a planted code-tree citation left dangling by the tool at `c4b02bf8` and repointed by the fixed one; a purge on the pair that writes a map; a planted malformed trailer that step A reports [M, code (`util/scedit/grammar/*.json`, `util/scedit/tests/anchor-expected.txt`; NOTHING under `src/`) + harness (`supervised-by.sh`, `purge-path.sh`, `harness/README.md`); ZERO engine launches; the tools run ONLY on scratch pairs (the F106 pair, `/home/claude/sc-f119/`), NEVER on the real trees; veto points §3 — the OWNER runs these tools every Saturday, and every change to them is a veto point he reads first]

**Why now / mandate:** §3 [H4] (session 29: *"the rewrite tool's class gap … with your Saturday run done, the (7) tool items are dispatchable"*) and session 27's [H4] (the map, the skip); §11.234's rider on the pins; the session-29 queue item (5); the intern's clock — he clones `master-beta` THIS WEEK (§11.232) and `doc/developer-entry.md` sends him to `ctest`. Decision-free on ACT 1: the map is the tool's own resolver and the re-record is the gate's own procedure; ACT 2 extends the tool's STATED intent one repository over (its header: step E exists so that traceability *"does not silently rot"*, `:175-182`) — the SHAPES (the code-tree file-set rule, how the two scripts share the map writer, the report's wording) are veto points, not decisions taken for him.

**The reading the mint stands on [derived; each fact a premise line]:** the maps are append-only evidence, never edited (`:209-211`); a repoint is a NEW commit on top of the rewritten branch, never a rewrite of the record; `build_repair_map` (`:1210`) is the precedent for mapping an ALREADY-dangling token to its twin by content — the shape ACT 2(b)'s consumers need when a past `purge-path.sh` run left no map (the 2026-09-07 purge, harness `9116a6a`, is such a run); `f106_pair.sh reset` restores the throwaway pair to STATE U and asserts it — the executor's proving ground; the 321 `moved` rows are NOT this task's to silence: each is a handler that shifted between the pin and HEAD, and the gate exists so that somebody looks (`anchor_gate.py:25-34`); `_meta` blocks are dated measurement records and stay out of the gate's scope by design (`:30-33`) — the pin is the ONE `_meta` field that names a commit. The intern reads `developer-entry.md` and runs what it says (§11.234): ACT 1 is what makes his `ctest` green; nothing in ACT 2 is visible to him.

**Measured at dispatch (supervisor, 2026-09-13 19:1x–19:4x, code `87d429bd`, harness `426a400`):** the pins and their counts as quoted; both old shas exist as local objects (`git cat-file -t` = commit) and both are refused by `merge-base --is-ancestor`; both map lines present; the tools at `c4b02bf8` / `86be032b` (their last commits `17fc2b5` / `9116a6a`); the F106 pair's setup record present; `/home/claude/sc-f119` absent; the owner ABSENT (Sunday) — every shape choice goes to §3.

**Mandate:** (1) **ACT 1 — the pins.** Resolve each of the six pins through the map by the documented convention (state the lookup: token → line → new sha, and that the new sha is an ancestor of `master-beta`); run `anchor_gate.py` BEFORE any edit and paste its tally (the AT-PIN-BROKEN / GONE / moved counts at HEAD on THIS tree, against the rehearsal's from the clone — say why they differ if they do); repoint the six pins (the `_meta.anchor_pin` string keeps its sentence, only the sha moves); run the gate again — AT-PIN-BROKEN and GONE must read 0 and `moved` must be the SAME 321 (a moved count that changes with the pin repoint is a finding); LOOK at the moved rows: group them by file and by the delivery that moved them (this week's F110–F116 code commits, `git log -L` or `blame` on the moved handlers), confirm each anchor resolves to the same construct, and re-record with `--record`; ONE code commit, message = the look (per-delivery counts, any anchor that did NOT resolve to the same construct and what was done about it — a re-anchor is a grammar edit and is SAID); `ctest` from `util/scedit`'s build green, the count of gates run/skipped stated. (2) **ACT 2(a) — the class.** In `supervised-by.sh`: the code-tree citation census — the tracked text files of the CODE repo carrying a token that prefixes an entry of either map (measured at HEAD by the tool's own `map_lookup`; the expected set is a premise); step E's preview lists them beside the harness `*.md`; on y, the repoint lands as ONE code commit on the rewritten branch (its EXPECTED set proved as `:216-218` proves the harness one); `--dry-run` shows the plan and prompts nothing (`:247`). (3) **ACT 2(b) — the map.** `purge-path.sh` persists `<old><TAB><new>` for every commit it re-shas, into `sha-maps/<UTC>-<code-tip8>-<harness-tip8>/` with the same three files (an absent repo's file empty, never missing), committed by its closing act — ONE writer shared by both scripts (a sourced file, or the function copied with its origin named — the executor chooses and states why; I2 rules against two writers drifting apart). (4) **ACT 2(c) — the trailer.** `list_code_trailers` keeps its `sort -u` output contract; a `Code:` line that the regex refuses is counted and REPORTED at step A (§11.212's block shape: the harness commit, the line verbatim, the expected form) and the run STOPs there — never silently fewer trailers. (5) **THE PROOF, on the F106 pair only:** `f106_pair.sh reset`; plant a code-tree file with a citation of a commit the pair's rewrite will move; run the tool at `c4b02bf8` → the citation dangles (RED, by `merge-base`); reset; run the fixed tool → repointed, the code commit's expected set proved; a purge on the pair with the fixed `purge-path.sh` → a map directory with the three files, the count equal to the commits re-sha'd; plant one malformed `Code:` trailer → step A reports it and stops; the pair reset to STATE U at the end, asserted. (6) **RECORD:** §11.⟨next⟩ FIRST + stub; §11.234's rider marked at both homes (the pins FIXED); `harness/README.md`: the `supervised-by.sh` section (`:4237`) gains the code-tree class and the trailer report, `purge-path.sh`'s section the map; §3's [H4] (sessions 27 and 29) closed at acceptance by the supervisor; WIP per §0.6; D14 (the grammar files are JSON with ASCII pins — the gate must stay green).

**Boundaries:** code: `util/scedit/grammar/*.json` (the six pin strings only) and `util/scedit/tests/anchor-expected.txt` — NOTHING under `src/`, `doc/`, or elsewhere in the code tree (a re-anchor of a grammar fragment is a finding SAID before it is made); harness: `supervised-by.sh`, `purge-path.sh`, `harness/README.md`, new instruments under `harness/f119_*`; the two tools are NEVER run on `/home/claude/spacecrafter` or `/home/claude/spacecrafter/claude` — scratch pairs only, and `f106_pair.sh assert` before and after; no engine launch; no EntityCore; nothing under `/tmp` carries; the maps under `sha-maps/` are never edited (append-only evidence).

**Discriminating checks:** (a) the gate's tally before and after the pin repoint, pasted, AT-PIN-BROKEN/GONE 0 → and `moved` unchanged by the repoint; (b) `ctest` green with its gate count; (c) the tool at `c4b02bf8` leaves the planted code-tree citation dangling and the fixed tool repoints it (RED → GREEN on the same pair, `merge-base` the judge); (d) the purge's map: line count = commits re-sha'd, format byte-compatible with `map_lookup`; (e) the malformed trailer reported with the line verbatim, the run stopped; (f) the real trees untouched by any tool run (`f106_pair.sh assert` and both real trees' `status --porcelain` at every commit); (g) D14.

**Preconditions (checkable, §0.7):** the PREMISES block is the gate; prose: harness HEAD as the prompt states; no owner run of either tool since `426a400` (the maps directory holds exactly one run — a premise); the F106 pair restorable (`f106_pair.sh assert` or `reset` succeeds — the executor's first act, reported if it fails).

```
PREMISES
# per-round variables — refreshed by the dispatcher at dispatch, never at mint
git rev-parse --short=8 HEAD => 87d429bd
git status --porcelain | wc -l => 0
md5sum build-claude/src/spacecrafter | cut -c1-8 => 42d7982c
python3 -c "import os,re;print(max(int(m.group(1)) for d in ['claude/INTENT','claude/INTENT/archive'] for f in os.listdir(d) for m in [re.match(r'11\.(\d+)\.md',f)] if m)+1)" => 242
grep -c '^### F' claude/fable-dispatch.md => 3
# the six pins, their reachability and the map, re-resolved at HEAD (content drift = abort)
for f in util/scedit/grammar/args/unit-1.json util/scedit/grammar/args/unit-2.json util/scedit/grammar/args/unit-3.json util/scedit/grammar/args/unit-4.json util/scedit/grammar/sc-grammar.json util/scedit/grammar/ss-grammar.json; do printf '%s ' "$(grep -c '54a2b844' $f)"; done => 1 1 1 1 5 0
grep -c 'ba7a32a8' util/scedit/grammar/ss-grammar.json => 1
git branch --contains 54a2b844 --list master-beta | wc -l => 0
git branch --contains ba7a32a8 --list master-beta | wc -l => 0
awk '/^54a2b844/{print substr($2,1,8)}' claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/code.tsv => b45d3b58
awk '/^ba7a32a8/{print substr($2,1,8)}' claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/code.tsv => 8d41fbe3
git branch --contains b45d3b58 --list master-beta | wc -l => 1
git branch --contains 8d41fbe3 --list master-beta | wc -l => 1
git ls-files util/scedit/grammar | wc -l => 10
git ls-files doc/developer-entry.md util/scedit/tests/anchor-expected.txt | wc -l => 2
grep -c 'anchor_pin' util/scedit/tests/anchor_gate.py => 4
grep -n 'AT-PIN-BROKEN' claude/harness/artifacts/f110/rehearsal-table.txt | cut -d: -f1 | head -1 => 135
grep -c '54a2b844' claude/harness/artifacts/f110/rehearsal-table.txt => 3
grep -n 'edited deliberately\|never regenerated\|anchor_gate' util/scedit/README.md | cut -d: -f1 | tr '\n' ' ' => 706 789 803
grep -n '16c6827\|1e6ca60' doc/developer-entry.md | cut -d: -f1 | tr '\n' ' ' => 282 437 476 498
# the tools, the map drawer, the scratch pair
grep -n '^list_code_trailers() {' claude/supervised-by.sh | cut -d: -f1 => 388
grep -n "sed -nE 's/^Code:" claude/supervised-by.sh | cut -d: -f1 => 390
grep -n '^# ONLY tracked \*.md, and never applies without your y/N.' claude/supervised-by.sh | cut -d: -f1 => 184
grep -n '^# Step E reaches tracked \*.md in the harness repo' claude/supervised-by.sh | cut -d: -f1 => 190
grep -n '^build_repair_map() {' claude/supervised-by.sh | cut -d: -f1 => 1210
grep -c 'sha-maps' claude/purge-path.sh => 0
md5sum claude/supervised-by.sh claude/purge-path.sh | cut -c1-8 | tr '\n' ' ' => c4b02bf8 86be032b
git -C claude ls-files 'sha-maps/*' | wc -l => 4
wc -l claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/code.tsv claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/harness.tsv claude/sha-maps/20260912T090341Z-71b6fe51-aaef2d12/repair.tsv | awk '{print $1}' | tr '\n' ' ' => 86 858 2 946
grep -n 'harness/artifacts/f103/setup.txt' claude/harness/f106_pair.sh | cut -d: -f1 | head -1 => 9
test -f claude/harness/f106_pair.sh -a -f claude/harness/artifacts/f103/setup.txt -a -f util/scedit/tests/anchor_gate.py -a -f util/scedit/tests/anchor-expected.txt && echo ok => ok
test -e /home/claude/sc-f119 ; echo $? => 1
```

**DoD:** ACT 1 landed as one code commit with the look in its message and `ctest` green; ACT 2(a)(b)(c) landed with the pair proofs (c)(d)(e); the real trees untouched by any tool run; §11 entry + stub; §11.234 marked; README; trees clean; WIP cleared; baselines LAST.
**WIP:** 2026-09-13 21:1x — **ACT 1 DONE, code `87d429bd` -> `d2fe86c7`** (six pins repointed by the map, `54a2b844 -> b45d3b58` code.tsv:32 and `ba7a32a8 -> 8d41fbe3` code.tsv:27; trees IDENTICAL both ways so the repoint is content-preserving; the 359 moved rows looked at and 359/359 attributed to 10 commits by `harness/f119_moved.py`; the one GONE row declared `[NOT AT HEAD]` — THE ONE EDIT BEYOND THE PINS, said in the commit message, reversible in one line; `--record` only after the look; ctest 19/19 from a fresh `build-f119`, `anchor_gate` #14 Passed, 19 run / 0 skipped). MEASURED ON A TRANSPORT CLONE (the intern's real view): 7174 broken references (AT-PIN-BROKEN 5888 + UNRESOLVED 1286) -> **4**; the residual four are EntityCore anchors pinned at gitlink `7ce58350`, reachable from NO ref of the submodule (twin = main's tip `84f5d94b`, same tree) — NOT fixable in this tree, an owner item. NEXT: ACT 2(a) step E's code-tree census, then (b) the map writer, then (c) the trailer STOP; proofs on the F106 pair only (asserted STATE U at open). 2026-09-13 21:5x — **ACT 2 WRITTEN, NOT YET PROVED ON THE PAIR** (harness `0f0c729` -> this commit): `sha_map_lib.sh` born (ONE writer + the prefix resolver, sourced by both tools); step E now scans the CODE tree too (all tracked text files, each hit printed with its line) and lands its own code commit FIRST; step A''' previews what a run would strand (visible under `--dry-run`); step A'' STOPs on a `Code:` line the grammar refuses; `purge-path.sh` persists its map in the same drawer by the same writer. MEASURED, and it re-cut the shape: widening the HARNESS side would rewrite `sha-maps/*/*.tsv` and dated `artifacts/**` (1715 hits) — `*.md` there is protecting evidence, so it STAYS; the code tree's carriers are 9 files over SIX extensions (0 false positives in 102 772 tokens). NEXT: the pair proofs (c)(d)(e), then README + §11.242. 2026-09-13 22:0x — **ALL FIVE PAIR PROOFS GREEN**, pair back at STATE U and asserted, real trees untouched by every tool run. (c) RED->GREEN on one pair: the pre-F119 tool leaves 54a2b844/ba7a32a8/e3afca8f dangling in 9 code files; the fixed tool repoints all three (0 old tokens left) in a code commit of 14 files over SEVEN extensions, made FIRST, the harness trailer naming it. (d) the purge's map: 13 lines == 13 commits re-shaed, three files, 40/40 TSV, committed by its closing act. (e) the trailer STOP fired on the pair's OWN malformed line, nothing touched. **THREE PRE-EXISTING DEFECTS IN `purge-path.sh` FORCED INTO SCOPE** (each one blocks the mandate's own (3)/(d); the UNMODIFIED tool could not complete a single run): a SIGPIPE in the range derivation (5/5 exit 141, zero output), the identity-mapped tokens marking unchanged files TOUCHED (closing commit refused), and G10 asserting the absence of tokens it must not repoint (67 false reds). Plus `sha-maps/**` excluded from its repoint -- it would have rewritten the maps themselves, which their own README forbids. NEXT: README + §11.242 + §11.234 markers.

---

## 2. Blocked — NOT dispatchable (reason stated so the exclusion is challengeable)

- **B1/S4 + B2 + riding rows** (D4 surface streaming, RING asteroid, INSTANCED
  consumer): Vixy-paced architectural line (EntityCore Taskable authority).
- ~~**B5 remainder** (dso3d/tully, ojmMgr, floors): suspended on the §11.96(e) six +
  §11.98(f) three — chiefly the reach-vs-visibility decoupling, now promotion-grade
  GENERAL (§11.97(c) second domain) and Vixy's call before any large content lands.~~
  **[UNBLOCKED 2026-09-12, §11.233(c) — the owner gave the DIRECTION (dso3d + ojmMgr as
  `ModularBody`, tully a `BodyModule` of the Milky Way, the as-if rule the
  reach/visibility criterion); the remainder is a DESIGN PASS, minted 2026-09-13 as
  F117 (the note; no engine line). The engine slices stay behind his reading of it.]**
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

- **Session-29 decision items (2026-09-12, Saturday — the INTERN'S-FIRST-DAY / ZOOM-TRACKING / UNIFORM-ROOT / LOG-BOUND / LAUNCH-PRECONDITIONS / SAMPLER-SEED round: F110 · F114 · F115 · F116 · F112 · F111, six for six delivered and supervisor-verified same session; F113 WITHDRAWN before dispatch on your correction). You were present: your messages are §11.232 (three answers) and §11.233 (with its correction block), and the round was re-cut twice on them. Q-70's shape: one decision per node, its held set at the node, anchors by ID and symbol. The F110 executor opened this block with [Y5]–[Y9]; [Y10]–[Y16] and the three lists after them are the supervisor's at close.**

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


  ```
  YOURS — from the later deliveries (supervisor, at close)
  [Y10] §5.142 · the ROOT you asked for has numbers, and four decisions are yours (§11.236(k))
        fact    1344 B per 06.sts body = 320 old-eager + 1024 new-at-load; only 88 B is a per-body
                requirement (128 carved → 8192 bodies/MiB); 768 B is ONE shadow-receive array
                embedded in FOUR blocks (a ringed ray-capable body carries it three times); 448 B
                are matrices derivable from their neighbour field; a body nobody draws holds 1344
                carved bytes containing 4 written ones — 06.sts's 1013 all do
        ask     (O2) cap the shadow-caster array — VISIBLE (the weakest occluder dropped), not
                as-if; (O2') the receive array out of the per-body block — an arena in your
                EntityCore; (O1) carve at first draw — moves the wall from load to camera;
                (O5) a NAMED refusal after any of them. The options table with its arithmetic is
                §11.236(i); the pool stays 1 MiB, by your word
        facts   shaders/src/receivedShadowsDecl.glsl:20 re-declares MAX_SHADOW_CASTERS = 8 with
                nothing tying it to bodyShaderInterface.hpp:47 (an I2 double — live the day O2
                edits one of them); the field's max_shadow_cast = 8 against the code default 4
                (context.hpp:194) leaves half of every receive array unfillable on a default config
        →       §11.236(g)(h)(i)(k) · §5.142 · §11.233(b) correction

  [Y11] §5.115 · the 1 GiB bound is LIVE ─ five veto points, each cheap to reverse (§11.237(m))
        V1      the RULE: the channel holding the MOST bytes rotates — neither "the channel being
                written" (11 987 rotations for 11 987 lines with the excess intact) nor "all five";
                measured, one line to revert
        V2      an in-session rotation spends one of the eight launch slots
        V3      WHAT THE BOUND COSTS IN HISTORY — built as your words, "never hold more than
                1 GiB": at 193 MB/h the first crossing comes after ~5.5 h and the gigabyte is gone
                in ~40 s. "Keep the LAST 1 GiB" is a different shape (a 128 MiB live-file bound
                per channel), named and NOT built — say the word if that is what you meant
        V4      no config key (F108's argument)  ·  V5 the number
        also    a directory left over budget by the OLD build rotates during startup before the
                console exists — the lines are now replayed to it (the second commit, unasked,
                measured 0/6 → 6/6)
        →       §11.237(d)(e)(i)(m) · §5.115 (size bound FIXED, density half OPEN)

  [Y12] §5.153 · a commanded view duration UNDER 0.2 s is not honoured by the drawn path ─ minted
        at F114's acceptance: Camera::lookTo floors every plan at 0.2 s (Camera.cpp:932); at
        `duration 0.1` old lands at 0.105 s and the two paths are 38.35° apart at that instant.
        Snap, clamp as today, or run the plan at the commanded time — perceptual, yours
        →       §11.235(j)1 · §5.153

  [Y13] §5.149 · the constructor's JD-0 seed ─ carried: the pointer you could not resolve this
        morning is the inline row INTENT.md:515 (python3 claude/intent_resolve.py 5.149); the
        shapes (i)–(iv) are at §11.229(h1); today the two-step fix pays for it
  [Y14] §5.146 · SharedBuffer THROWS on a refused allocation ─ your shape is recorded; the edit is
        EntityCore, and a pin naming an unpushed submodule commit breaks the intern's
        --recurse-submodules clone THIS WEEK (§11.204(b)) — your one line, or dispatched in the
        same act as your EntityCore push                                    → §11.233(f)
  [Y15] §5.150 · two halves NOT minted at F111's acceptance ─ the special-orbit half (Europa
        3.5e-08 AU through the static ephemeris cache, walked identically by the OLD path) stays
        a caveat at §5.84; the trail walker (3.3 arcsec one frame after an unhide, new-path-only,
        no old shape to restore) is a rider with its engine leg owed. Say the word if either
        deserves its own row                                                → §11.239(f)(h)
  [Y16] scratch trees ─ sc-f110 2.6 G · sc-f116 557 M · sc-f112 532 M · sc-f114 185 M ·
        sc-f111 276M · sc-f115 176 K join the list (21 G across all sc-* before sc-f111); the
        two registered worktrees sc-f89/tree and sc-f107/tree still stand

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] the canary re-bank on the 09:51:57 boot (the block's own prescription); the full band
       reproduced on a FOURTH boot
  [V2] archival pass 21 at open; F113 kept as the WITHDRAWN record of my misread (ids never reused)
  [V3] F110: eleven doc hunks (+98/−24, each traced to a rehearsal step); f85_links.py's commit-sha
       class; claude/intent_resolve.py as the ledger's resolver (the doc names it)
  [V4] F114: the two sites; Camera::oldLocalToLocal as the ONE home of the init-view conversion,
       ssystem_factory.cpp rerouted through it (outside the section's file list — I2)
  [V5] F116: the rule (V1 of [Y11]) and the startup replay
  [V6] F112: ONE implementation with two entry points; sc_gpu.py as a third home with the NUMBER
       banked in the canary; gpu.headroom a GATE on both arms at 6144 MiB FREE; b22_live_run.sh's
       kill routed; F90_SKIP_CANARY=1 now leaves a trace; the doc's §5 item 2 corrected
  [V7] F111: the const_cast (a named compromise); the pair's return value ignored; §5.150 flipped
       on its iterative half only
  [V8] §5.153 minted; B41 (the preload signal) opened in §13.B from your words; the §3 nodes of
       sessions 26–28 answered by §11.232/§11.233; §0.5's instance bullet rewritten from §11.238(l)
  [V9] this block

  HELD OPEN, not absorbed
  [H1] the DSO design pass — dso3d + ojmMgr as ModularBody, tully a Milky Way BodyModule, the
       as-if rule the reach/visibility criterion (§11.233(c)) — dispatchable, M–L, first among the
       design items
  [H2] B41 the preload signal — the zoom half has no anchor (zoomToBothPaths does not know the
       body); decision-free once one is chosen (§11.235(l))
  [H3] the trail walker's engine leg (it fires on an edge; S) · §5.150's special half as a caveat
  [H4] the rewrite tool's class gap: citations in the CODE tree (developer-entry.md's harness shas,
       scedit's anchor_pin 54a2b844 in four grammar fragments) are not repointed; with your Saturday
       run done, the (7) tool items are dispatchable (purge-path.sh's map, list_code_trailers' skip)
  [H5] [Y8] the build type — every D11 number on -O2 while the documented build ships -Ofast
  [H6] the intern's three gaps [Y5]–[Y7] — the only items with a clock (this week)
  [H7] §5.115's density half; the EntityCore log pile (395 files today); the drawn orbit line has
       no observable (a readout of orbitPoint[] first, §11.239(l)4)

  FACTS, no decision asked
  [F1] six for six delivered AND supervisor-verified same session, every delivery re-run by my own
       hand (F91 on every engine binary — d607cfdc, 95087b68, 42d7982c — table 1fe630a4 each time);
       code fcc277c9 → 87d429bd (five executor commits: the doc, the zoom pair, the log bound ×2,
       the sampler); binary 6d63e6c1 → 42d7982c
  [F2] the host: rebooted 09:51:57; your rewrite + push at 11:03 (0 unpushed then); a resident 27B
       model until 11:32, then your java (21 GiB RSS, up to 2.9 GiB VRAM) — its 15:48–16:21 VRAM
       crossing cost F112 33 minutes and one resume, and the precondition that refused it was MINE
       (used ≤ 4000: the wrong shape; the derived gate reads FREE ≥ 6144); 25+ canary runs, green
       after the 11:34 re-bank throughout
  [F3] SIXTEEN dispatcher defects (5 value, 10 structure, 1 intent), all output-side, none reaching a
       delivery: the intent one is the §5.142 misread (Q-04's second instance in five days; the
       prevention: every clause of an incoming ruling located in the inferred structure before the
       first propagating edit); two values quoted after my own writes had moved them (Q-67's new
       sub-class); one commit whose message described edits its tree lacked (Q-56's third); the rest
       structure asserted without the read (a grep read as a census, twice; a shared object that was
       two; a launch precondition typed instead of derived)
  [F4] for the intern: his plain clone shows none of claude/; c5be42b is ABSENT for him (a network
       clone carries no unreachable objects); the resolver exists; the canary's per-host trap is
       now said in the document; PRs target 2023-master as the doc says, on your "as is"
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
