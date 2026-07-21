# Dispatch view — non-critical, decision-free tasks (2026-07-21, for Claude Opus 4.8)

**Authority note (I2):** this file is a *dispatch view* over `INTENT.md` §13.B — it does
not own any row. On any divergence, §13 wins and the divergence is a staleness bug in
THIS file. Before acting on any row below, **re-read its §13.B row and the recording
entries it cites** (§5.2 class: cached conclusions need re-verification against source,
not recall). Rows carry their `Bxx` id for exact re-matching.

**Selection criteria (traceable):**
- *Information:* `INTENT.md` §13 ledger as of commit 27dbe9e4 + working tree (2026-07-21).
- *Criterion 1 — no pending decision:* every row here is marked UNBLOCKED/REQUESTED in
  §13.B with its deciding answer recorded; rows whose scope still contains a Vixy
  decision or an unanswered tester question are excluded or carved out below.
- *Criterion 2 — off the critical path:* the S4 threading handoff (B1) and everything
  riding it (B2), and the authoring-path core chain (B24→B25, with B27 step 3) are the
  critical/architectural line — excluded.
- *Conclusion:* three lists — dispatchable now, partially dispatchable (explicit
  carve-outs), excluded (with the excluding reason, so the exclusion is challengeable).

**Standing constraints for the executor:** repo rules in `<programmation-principles>`
apply (I1–I6). Verification-surface reachability applies to every row: a green build is
not coverage — each row names its own discriminating check; if a row's check needs a
display, say so instead of substituting a weaker check. Do not improvise answers to
carved-out residuals — stop at the carve-out boundary and record the stop.

---

## 1. Dispatchable now (self-contained, decision-free)

| Row | Task | Spec / recorded at | Notes for execution |
|---|---|---|---|
| B9 | Az-convention divergence: old `getAltAz` applies 3π−az, new returns Camera-frame raw — probe, then fix at the `ModularObject` surface | §11.4 | Probe first (confirm the delta is exactly the convention), then one conversion authority, not per-caller patches |
| B11 | Trail recording gate: `flag trails off` STOPS accumulation; re-enable starts FRESH | §11.41, §11.48(a) | Distinct gate from B19 (hidden body ⇒ still updating ⇒ still recording). Reading the two as one gate produces a wrong implementation — the row says so explicitly |
| B13 | Reference-change view continuity: preserve absolute sky direction across reference switch and free-mode entry/exit; no re-centring | §11.19c, §11.48(a) | Mechanism (revive `view` smoothing quaternion vs derive from alt/az) is the executor's engineering choice and owns the inverse formulas either way. DoD includes adding a view-continuity assertion to scene E (currently silent on exactly this) |
| B15 | AoI re-derivation on date change: the launch-jd latch is a defect (dates are jumped mid-navigation) | §11.36, §11.48(a) | Recompute cadence (date-jump event vs continuous) is an engineering call bounded by constraint C3. Should close the ~10% seasonal drift and the scene-E `e_in` first-run miss — verify both |
| B16 | ~~Expose `reloadSystem` as a command; keep current state (camera + date), no reset~~ **DONE 2026-07-21 → §11.55, §6 below** | §11.36, §11.45(d), §11.48(a), **§11.55** | Landed as **`body action reload`**; both §2(c) channels exercised live. **Scope grew by one structural fix**: the reload's first live use exposed an I5 violation (EnvironmentManager's cross-frame raw-pointer chain cache dereferences freed bodies) — fixed at the class by destruction notification. **One question suspended**: does "keep current state" cover body-scoped runtime overrides (today the file wins, and the old path desyncs) |
| B17 | Port `view_offset` / `zoom_offset` as a Camera parametrization, config + command channels | §11.19a, §11.45(d), §11.48(a) | In use for tilted dome geometry ⇒ it is a projection-space offset — must NOT be re-derived as a camera rotation |
| B18 | Port `flag_lock_equ_pos` (equatorial-mount sky-lock) | §11.19c, §11.48(a) | The "unexercised legacy feature" premise was wrong — it is exercised, just not by our harness |
| B19 | ~~Hidden-body ticking: current behavior (hidden ⇒ keeps updating) is ratified — lock it with a regression assert~~ **DONE 2026-07-21 → §11.54, §5 below** | §11.15b, §11.36, §11.48(a), **§11.54** | **The row's premise was FALSE**: the new path froze hidden bodies (0.00 km advance over 20 simulated min vs old's 1306.81 km), and the 14 bodies shipped `hidden = true` had never been positioned (`lastJD = 0`; Pluto 5.75e9 km off). So it WAS a behavior change — scope expanded from "assert only" to "implement the ratified semantics + assert", traceable to Q13/A10. Locked by `harness/b19_hidden_tick.py` |
| B20 | Anchored galactic display: at galactic distances while anchored, show the solar-system view from very far, anchor kept — no altitude-driven mode switch | §11.36, §11.48(a) | A13 ratified anchored-stays-anchored in the same answer set — no escalation-policy change |
| B22 | System-collapse cross-fade at the ~16 px resolved↔dot threshold, "if not too costly" | §11.36, §11.48(b) | The COST BOUND is the decision input: deliver the cross-fade + its measured cost. The threshold constants themselves stay open (A15 — Vixy/tester) — do not tune them here |
| B23 | Restore planet-grid tropics + polar circles, keyed to the corresponding sky-line flags | §11.42, §11.48(b) | The old coupling is deliberate (they show obliquity directly). Independent-toggle + near-surface-regime halves stay in A4 — out of scope |
| B26 | ~~Run the two-screenshot observable check for the dual-path default flip~~ **DONE 2026-07-21 → §11.53, §4 below** | §11.50(c), §11.53 | **VERIFIED on `DISPLAY=:2`, 6 fresh launches, no product code changed.** The stated criterion was itself defective (≥2.5 s = quarter of the 2 s toggle period ⇒ 50 % test; corrected to odd multiples of 1.0 s, discriminator px>32). New finding spun out: **B30** (new path not bit-stable on a frozen scene) |
| B29 | Runtime COLOR seam port: MEASURE old's reload behavior for runtime per-body colors, then reproduce it | §11.51(f), §11.42, §11.45(d) | Observation task, no design freedom — old's observable IS the spec (parity unconditional here: semantic surface, no physical referent). Per-instance storage + broadcast override stands. Closes the last OLD-ONLY S6 seam class |
| B28 | Loader frame declaration + conversion: data declares its coordinate system, loader converts — one conversion authority | §11.51(d), §11.52(a), §11.49(e) | Fully specified incl. write-back contract (only-when-needed, atomic sibling-temp-then-rename, whole-file clean precondition; text-preserving insertion). Regression criterion = bit-identical for the 7 existing `rot_pole_ra` planets. Actionable diagnostics per §2(f). Larger than the other rows but decision-complete; B14 sequences after it |
| B6 | §11.37 view-roll 134.67° — investigation only | §11.37 | Non-reproducing; one settled observation; artifact preserved. Low priority — attempt reproduction from the artifact, record outcome either way |
| B7 | §11.15d shutdown segfault — probe-log watch | §11.15d, §11.47 | Intermittent, no fire across recent sessions after the structural fixes. Task = check/extend the §11.47 probes when touching shutdown paths; not an active hunt |

Suggested order: B26 (pure verification) → small ratified rows (B19, B16, B11, B23, B18, B20) → B9/B13/B15/B17 (each owns a small design-free mechanism) → B22, B29 → B28 (largest) → B6/B7 opportunistic.

## 2. Partially dispatchable (explicit carve-outs — stop at the boundary)

| Row | Dispatchable part | Carved out (NOT yours) | Recorded |
|---|---|---|---|
| B10 | `datum_radius` + `ground_radius` full scope (i)–(vi) as written, incl. the shared `proximityFactor()` authority and the outward-only anti-stuck floor direction | (a) the FLOOR VALUE — "value is a decision, not a wiring step" (`MIN_MOVEMENT_SPEED` 0.125 reads wrong-scale); propose, don't fix; (b) Q12 round-2 re-ask (what the two-body patch is used for; stop-and-hold vs asymptotic; `radius ground`/`radius datum` re-spelling) — pending tester/Vixy | §5.2, §11.6, §11.48(c) |
| B21 | View-directed free descent ("down" = surface point under the view ray), riding B10's `proximityFactor()` — sequence after B10 | The far/galactic-distance case (A18 residual) — the row itself says it must not be improvised into this work | §11.36, §11.48(a) |
| B27 | Steps (1) site inventory completion (grep is not exhaustive yet — seed list §11.51(c)) and (2) per-site §2(a2) test → declarable capability key | Step (3) generator emission — belongs to B25 (critical/authoring chain); special ORBITS exempt (`*_special` stays) | §11.51(c), §5.5, §11.48(b) |
| B14 | Preparation only: collect IAU/WGCCRE values for the 28-body cluster (Iapetus first) FROM THE REPORT — "never from recall; a confabulated pole is indistinguishable from a measured one" | Landing the corrections — sequenced strictly after B28 (frame declaration must exist so values are declared in the absolute frame) | §11.35, §11.48(a), §11.49(e), §11.51(d) |

## 3. Excluded (reason stated so the exclusion is challengeable)

| Row | Reason |
|---|---|
| All §13.A | Vixy's decisions — blocked by protocol |
| B1 | Critical path: S4 threading handoff, architecture-grade |
| B2 | Rides S4 (B1) |
| B3 | Blocked on counterfactual scenes/models (beyond shipped content) — assessed 2026-07-19 |
| B4 | Design-deciding half unanswered (anchor kinds in actual use — tester round 2) |
| B5 | §6.9 executor-mode dissolution — structural surgery on the module, keep with the main line |
| B8 | Sequenced at old-path removal, whose lifetime was extended (§12 revision, §11.51) |
| B12 | Star near-surface family — feature-family design (regime, shaders); unblocked but neither small nor risk-free; dispatch separately with its own spec if wanted |
| B24 | Design-time residuals require sign-off (`relation=` value domain, INSTANCED hint spelling) + authoring-path core |
| B25 | Sequenced after B24's grammar; authoring-path core (dual-use writer, one serialization authority) |

---
*Handoff protocol: the executor updates §13.B rows on completion (per the ledger's own rule) and this file's row state; findings that change any row's premises go to INTENT.md first, this file second.*

---

## 4. Execution log — B26 (Claude Opus 4.8, 2026-07-21)

**Task**: wave §1 task 1 — run the two-screenshot observable check for the
dual-path default flip (§11.50(c)). Pure verification; no product code
changed, and none turned out to be needed.

**Full measurements**: INTENT.md §11.53 (a)–(g).
**Artifacts**: `harness/artifacts/b26/` (168 PNG + 6 gdb logs + 6 driver logs —
gitignored, on disk) and `harness/artifacts/b26_measurements.json` (committed).
**Instruments committed**: `harness/b26_run_case.sh`, `b26_default_flip.py`,
`b26_analyze.py`, `b26_probe.gdb`, README section.

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1a | Screenshot channel named + implementation + written + content is the rendered frame | **met** | `body action screenshot` → `App::takeScreenshot` → `SaveScreenInterface::takeScreenShot` [app_command_interface.cpp:3576-3581; app.cpp:1011-1014; save_screen_interface.cpp:146-172]. 28 files/run, distinct mtimes at the commanded 0.25 s cadence (22:42:46.718651888 … 22:42:48.966626079). Live content: mean RGB [0.744,0.734,0.717], max 255, 602 890/4 194 304 non-zero px; tracks commanded state (pin-old vs pin-new = 47 955 px) |
| 1b | Noise floor MEASURED | **met** | same-state pair 3 s apart: max\|Δ\| = 0 (c1,c2,c3,c5) / 4 (c4) / 12 (c6); **px>32 = 0 in all six**. Same-path worst over a 6 s burst: max\|Δ\| ≤ 31, px>8 ≤ 5, px>32 = 0 |
| 1c | Counterfactual sensitivity | **met** | pinned old vs pinned new, same camera: max\|Δ\| = **71/255**, px>0 = 47 843…47 964, px>8 = 1 153…1 156, **px>32 = 133…136**, mean\|Δ\| = 1.93e-2. Side by side with 1b: px>32 **0 vs 133…136**, no overlap |
| 1d | Confounders controlled and named | **met** | time frozen AND proven (two `dual_dump` headers ~12 s apart, identical jd 2461233.500013901, every run); ASmooth settled 15 s + a 120 s-settle control run; auto-play `scripts/fscripts/startup.sts` (sets `timerate rate 1`) overridden after it; focus/compositing excluded by the channel (app-side readback, not an X grab); landscape+atmosphere off (they mask) |
| 2 | Default run, file absent | **met** | `ls ~/.spacecrafter/beta_features.ini` → ENOENT; lookup site `getConfigDir()+"beta_features.ini"` [app_settings.cpp:125-128]; no "Loaded experimental settings" log line. 24/24 shots NEW; **0/14 pairs at 2.5 s and 0/20 at 1.0 s differ** (px>32 = 0) |
| 3 | Alternate run | **met** | file verbatim `[dual_path]\nrender_path                    = alternate\n` at `/home/claude/.spacecrafter/beta_features.ini`. **20/20 pairs at 1.0 s differ** (px>32 = 135); nominal 2.5 s pair (stab00,stab10) differs: max\|Δ\| = 71, px>8 = 1 154, px>32 = 135. Phase string `ONNNNOOOONNNNOOOONNNNOOO` |
| 4 | File present with the DEFAULT value | **met** | `render_path = new` → identical to case 2 at every lag; log proves the file WAS read. Third and fourth directions added beyond the DoD: `render_path = old` (24/24 shots bit-identical to the pinned-old reference) and `render_path = bogus_typo` (refusal log line fires, default kept) — together these exclude "read but ignored" from all sides |
| 5 | Which path runs, established NOT by the screenshot pair | **met** | gdb probe on `App::takeScreenshot` reading `core._M_ptr->ssystemFactory->{drawModularSystem,pathPinned}` **from process memory at every capture**; app launched UNDER gdb (ptrace_scope = 1). Default/`new`: `draw=1 pinned=1` ×27 of 28 (28th = the deliberate pinned-old reference). `old`: `draw=0 pinned=1`. `alternate`: `pinned=0`, `draw` flipping in runs of four 0.25 s shots. **Agrees 24/24 with the pixel classification** |
| 6 | Trackers | **met** | INTENT.md §11.53 (new entry, 2026-07-21), §13.B B26 row → CLOSED, new §13.B row **B30**, §11.50(c) verification-state paragraph closed in place with its two corrections; this file's row + this section |
| 7 | Committed on master-beta, tree buildable | **met** | see commit list below; no product source touched ⇒ the tree is the verified-green d343f6c4 build plus docs/harness |

### Deviations from the task spec (each with its reason)

1. **The DoD's "two screenshots ≥2.5 s apart" was replaced by a 24-shot /
   0.25 s burst with a lag sweep, and the nominal 2.5 s pair is reported
   inside it.** Reason: the 2.5 s criterion is *unsound* — the toggle is a
   1000 ms square wave (period 2.0 s), so 2.5 s is a quarter-period offset
   and differs only 50 % of the time. Measured: 1.0 s → 20/20 differ,
   2.0 s → 0/16, 2.5 s → **7/14**, 3.0 s → 12/12. My run's nominal pair
   happened to land on the differing half; on the other half a correct build
   would have been recorded as broken. Raising the sampling density is not
   lowering the bar — the reported verdict is strictly stronger than the one
   asked for. §11.53(d).
2. **Two extra cases run beyond the four required** (`render_path = old`,
   `render_path = bogus_typo`) and **one control run** (120 s settle).
   Reason: "new" is indistinguishable from ignoring the file, so DoD item 4
   alone cannot exclude a read-but-ignored value; `old` closes that from the
   other side, `bogus_typo` exercises §11.50(c)'s "refused, not absorbed"
   clause observably, and the settle control converted the residual's
   "still converging?" assumption into a measurement (refuted).
3. **Four harness documentation sites corrected in place** (README.md,
   `ab_orientation.py`, `dual-dump.sts` ×2). Documentation only, no logic.
   Reason: they assert the 1 s auto-toggle as the *default*, which the very
   change under verification made false; `ab_orientation.py` silently
   degrades to a one-phase clustering without the opt-in file. Recorded at
   §11.53(f).

### Findings recorded, not fixed

- **B30 (new ledger row)** — the new path is **not bit-stable on a frozen
  scene**: discrete state steps persisting seconds, max\|Δ\| ≤ 31/255 on
  ≤ 3 898 px of 4 194 304 (0.09 %), px>32 = 0, whole-frame low-brightness
  both-sign shift (total luminance 0.003 %). The **old path measures
  max\|Δ\| = 0** under the identical protocol ⇒ mechanism is inside the new
  path. Settling refuted (120 s settle no quieter than 15 s). Below the
  §11.52(b) perceptual bar, so not a parity blocker — but it invalidates
  bit-level A/B assertions on the new path, which is a harness-wide
  constraint. §11.53(e).
- **`InitParser::getStr` logs a WARNING on every successful read**
  [init_parser.cpp:104-108] — a warning with no problem behind it, the
  inverse of the §2(f) actionable-diagnostics directive. §11.53(f).

### Suspended for Vixy

**None.** Every decision taken was traceable to the task spec, §11.50(c), or
a measurement; the one judgement call (replacing an unsound sampling interval
with a denser one that strictly contains it) is recorded as deviation 1 with
its measured justification rather than absorbed.

### Reproduction (verbatim)

    cd /home/claude/spacecrafter/src/experimentalModule/harness
    rm -f ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c1_default                       # exit 0
    printf '[dual_path]\nrender_path                    = alternate\n' > ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c2_alternate                     # exit 0
    printf '[dual_path]\nrender_path                    = new\n'       > ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c3_new                           # exit 0
    printf '[dual_path]\nrender_path                    = old\n'       > ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c4_old                           # exit 0
    printf '[dual_path]\nrender_path                    = bogus_typo\n'> ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c5_bogus                         # exit 0
    rm -f ~/.spacecrafter/beta_features.ini
    SETTLE_EXTRA=120 ./b26_run_case.sh c6_longsettle   # exit 0
    for t in c1_default c2_alternate c3_new c4_old c5_bogus c6_longsettle; do ./b26_analyze.py $t; done

### Hygiene

`~/.spacecrafter/beta_features.ini` removed at the end (absence == every
default; a leftover file silently re-specifies the next run) [`ls` → ENOENT].
`config.ini` md5 `03fbee59bc3ec506c58f0a3f1e1d73df`, `ssystem.ini` md5
`fb87a774e728706e9d4e1959c386bb23` — unchanged from the §11.47 values. Six
clean `shutdown action now`, `[Inferior 1 … exited normally]` in all six gdb
logs: **no §11.15d fire** (data point for B7, binary d343f6c4, mtime
2026-07-21 15:44).

## 5. Execution log — B19 (Claude Opus 4.8, 2026-07-21)

**Task**: wave §1 task 2 — lock the ratified hidden-body ticking behavior with
a regression assertion.

**Headline: the row's premise did not hold.** The row said *"current behavior
ratified ⇒ this is not a behavior change"*. Measured on the wave baseline
binary: the new path **froze** hidden bodies completely, and 14 bodies that
ship `hidden = true` had **never been positioned at all**. The task spec
anticipated exactly this (*"If you find the current behavior does NOT match the
ratified statement, that is a finding"*), so the finding is reported and the
scope expansion is named rather than performed silently.

**Full measurements**: INTENT.md §11.54 (a)–(j).
**Instrument committed**: `harness/b19_hidden_tick.py` + README section.
**Artifacts**: `harness/artifacts/b19/` (9 dumps + `b19_result.json`).

### Scope expansion, stated (this is the one judgement call in the task)

Delivering only an assertion would have delivered a permanently-red test and
left the ratified semantics unimplemented — the row's own stated purpose
(*"so a later perf optimisation cannot silently freeze hidden bodies"*) is
unreachable if they are already frozen. I implemented the semantics. It is
**traceable, not improvised**: Vixy answered Q13 verbatim *"It should be where
it is now"* (§11.48(a) A10), and the mechanism used is the one already in the
tree for the same class — the `selectiveUpdate` else-branch's own comment
states the requirement (*"positions of non-drawn bodies stay queryable and
sortable"*) and a hidden body is a non-drawn body (I6: fix the class).

**Where I stopped, per the scope bound**: I did not touch update
*scheduling* (B1/S4). `updateHiddenBodies` adds the missing list to the walks
that already exist, using the existing translation-only refresh; no cadence,
ordering or budget was changed. The per-frame cost this restores is recorded
in §11.54(h) as the thing the ratified answer bought.

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1 | Assertion exists inside a runnable harness scene; entry point + command line stated | **met** | `src/experimentalModule/harness/b19_hidden_tick.py`. `DISPLAY=:2 /home/claude/spacecrafter/build-claude/src/spacecrafter &` (wait for port 7805, +10 s), then `cd /home/claude/spacecrafter/src/experimentalModule/harness && python3 ./b19_hidden_tick.py [outdir]`. Exit 0/1; machine-readable `artifacts/b19/b19_result.json`; README section "Hidden-body ticking (B19…)" |
| 2 | Discriminating — PROVEN to fail when the guarded behavior is removed, then reverted | **met, twice** | (i) baseline binary (pre-fix, tree `047f2d7e`): **18 red** — hidden legs `moved_new = 0.00 km` vs `moved_old = 1306.81 km` (Moon) / `2576.26 km` (Phobos), `ΔlastJD = 0.000000000 d`. (ii) temporary one-line mutation on the FIXED code (`return;` at the head of `updateHiddenBodies`) → rebuild (exit 0, binary mtime 23:35) → **16 red, exit 1**, shown legs green. Revert → rebuild (exit 0, mtime 23:37) → **59/59 green, exit 0**; `git diff` on product source = exactly the 54 added lines of this change, nothing else. Logs: `b19_mut.log`, `b19_final.log` |
| 3 | Observable = the body's actual state advance, not a freeze-survivable proxy | **met** | `ecl` = `ModularBody::eclipticPos` [dumped ModularBody.cpp:569-570], written ONLY by `transformParentToBodyPos`/`transformBodyToParent` immediately after the orbit evaluation [ModularBody.hpp:404-405, 539-540]. A freeze stops calling exactly those ⇒ `ecl` keeps its hide-time value; it is the position a re-shown body is drawn at. `lastJD` dumped as corroboration only. Membership family A (`relation` 4→1→4) makes a vacuous pass impossible |
| 4 | Both entries of the reversible pair, second hide from the state the first show produced | **met** | one uninterrupted run: hide→show→hide→show, no restart. Entry 1 hidden: Moon 1308.12 km / err 0.004 km, Phobos 2578.82 km / 0.000 km. Entry 2 hidden: Moon 1307.92 km / 0.011 km, Phobos 2593.16 km / 0.000 km. Shown legs between and after them also asserted. Extra: hiding the **camera reference itself** (`body name Earth hidden true` while standing on Earth) — advances correctly (err 1.943 km), child Moon keeps ticking, reference preserved, no crash |
| 5 | Time control stated and proven (positive evidence, B26 class) | **met** | `timerate rate 0` sent BEFORE the epoch (first run put it after and leaked **+1.390e-05 d = 1.20 s** from the auto-played `startup.sts`'s `timerate rate 1` — measured, then corrected). All 9 dump header jd == commanded at **0.0e+00 d**; the only jd movement is the four commanded 20-minute jumps; per-body `ΔlastJD` = 0.0138889 d ± 2.6e-07 (residual = light-travel retardation changing, a correct term) |
| 6 | Build green; binary mtime advanced | **met** | `make -C /home/claude/spacecrafter/build-claude -j$(nproc)` exit **0** three times (fix 23:22, mutation 23:35, revert 23:37); mtime advanced each time from 15:44 |
| 7 | Trackers | **met** | INTENT.md **§11.54** (new, 2026-07-21, (a)–(j)); §13.B **B19 → DONE**; §11.15b(b) suspension struck + closed, and its three other pointers (§11.15 residual (a), §11.36 suspended list, §11 "Suspended for Vixy" history) updated in place; §9 seam table hide/show row **corrected** (wrong command spelling); this file's row + this section |
| 8 | Committed on master-beta, correct author/co-author, never pushed | **met** | see commit list below |

### No-regression, at the heights this could plausibly move

- Scenes A–D (`drive_scenes.py` + `predict.py`, fresh launch, `init_fov = 340`):
  P2 mat-residuals ≤ **1.7e-07**; P3 old-vs-new angles ≤ **2.4e-05 deg**,
  relative distances ≤ **2.2e-07**; P4 observer parity **12.55 / 21.29 /
  21.10 / 55.07 / 25.94 km**; P5 ≤ **1.74e-07** with the same named causes.
  The §11.16/§11.17 recorded class, unchanged.
- Scene E (`scene_e_spine.py`, fresh launch): **13/13 OK**, both second
  entries included, Mars landing 2.270821e-05 AU.
- `orientation_check.py /tmp/gen_a.json`: **17 parity-restored / 48
  named-divergent**, P-d **0.0000 deg** — the §11.35 spectrum, unchanged.
- Zero VUID on every launch (`debug_layer = true`, config.ini:307). Clean
  `shutdown action now` exits, **no §11.15d fire** (data point for ledger B7,
  on the post-fix binary, mtime 2026-07-21 23:37).
- Side effect that is an improvement, not a regression: the 14 ship-hidden
  bodies became live. New-vs-old error, before → after: Pluto
  **5 751 229 598.7 → 94.7 km**, Eris **2 362 129 737.1 → 219.0 km**, Vesta
  **595 634 826.8 → 19.3 km**, Ceres **264 080 024.3 → 1.9 km**, Charon
  **22 616.2 → 0.000 km**.

### Findings recorded, not fixed (out of scope)

1. **`body … action hide` / `action show` do not exist.** The `body` action
   dispatch is `load|remove|clear|drop|initial|preload|dual_dump|screenshot`
   [app_command_interface.cpp:3554-3577]; the working spelling is
   `body name <X> hidden true|false` [same file, 3592-3600]. Verified live:
   `body Sun action hide` leaves `relation = 4`; `body name Sun hidden true`
   gives `relation = 1`. Consequence: **§11.44's rare-path evidence "hide/show
   Sun (`body Sun action hide/show`) ×2" traversed nothing** — the "app
   survived" claim is true and empty. §9's row corrected; §11.44 left as
   written with a pointer.
2. **`dumpTracePaths`'s comment is false for Pluto** — it claims *"lastJD
   stays fresh through recursiveTranslationUpdate"* [ssystem_factory.cpp:
   569-572], but Pluto is *hidden*, not merely invisible, and read
   `lastJD = 0` until this change. Every `dumpHops` spin term for
   Pluto/Charon in the §11.34/§11.35 record was evaluated at JD 0 (tilt and
   commutator terms are pole-constant and unaffected — which is why 115.60°
   re-measures identically today). Flagged for whoever reopens that spectrum.
3. `pkill -f 'build-claude/src/spacecrafter'` and `pgrep -f` on the same
   pattern **self-match the issuing shell** — a "STILL ALIVE" reading that is
   the instrument seeing itself. Liveness was confirmed by port 7805 absence
   instead. (Same class as the agent-brief warning; recorded because it fired
   here.)

### Suspended for Vixy

None. The one semantic question this row could have raised — freeze vs tick —
was already answered verbatim (Q13/A10), which is why implementing was not an
improvisation. The adjacent question that is **NOT** answered here and must not
be read into §11.54: whether a hidden body's *modules* tick (trail recording
in particular). `recursiveTranslationUpdate` runs no module update, so today a
hidden body's trail does not accumulate. That is **B11's** row.

### Reproduction (verbatim)

    # regression assertion (the deliverable)
    DISPLAY=:2 /home/claude/spacecrafter/build-claude/src/spacecrafter &
    #  wait for `ss -ltn | grep :7805`, then +10 s for async texture loads
    cd /home/claude/spacecrafter/src/experimentalModule/harness
    python3 ./b19_hidden_tick.py            # exit 0; artifacts/b19/b19_result.json

    # discrimination (temporary, revert afterwards)
    #  insert `return;` as the first statement of ModularBody.hpp
    #  updateHiddenBodies(), then:
    make -C /home/claude/spacecrafter/build-claude -j$(nproc)   # exit 0
    #  relaunch fresh, rerun the script      -> exit 1, 16 red, shown legs green
    #  revert, rebuild, relaunch, rerun      -> exit 0, 59 green

    # no-regression (needs init_fov = 340 in ~/.spacecrafter/config.ini,
    # restored to 180 afterwards - verified byte-identical by diff)
    python3 ./drive_scenes.py
    for f in /tmp/gen_a.json /tmp/gen_b.json /tmp/gen_moon.json \
             /tmp/gen_mars.json /tmp/gen_mars_2.json; do python3 ./predict.py $f; done
    python3 ./orientation_check.py /tmp/gen_a.json
    #  fresh launch, then:
    python3 ./scene_e_spine.py              # exit 0, 13/13

### Hygiene

`~/.spacecrafter/config.ini` restored byte-identical after the temporary
`init_fov 180 → 340` change [`diff` → empty; `init_fov = 180` re-read].
`~/.spacecrafter/beta_features.ini` absent throughout (shipped state).
`~/.spacecrafter/ssystem.ini` untouched. No harness task list touched.

---

## 6. Execution log — B16 (Claude Opus 4.8, 2026-07-21)

**Task**: wave §1 task 3 — expose `reloadSystem` on the command surface,
keeping the current state (camera + date), closing the §2(c) gap opened at
§11.36.

**Scope grew beyond what the row anticipated, stated first because burying it
would be a report defect.** Two things landed that the row did not name:

1. **An I5 defect fixed at the class.** `EnvironmentManager` caches the
   previous frame's reference chain as raw `ModularBody*` and dereferences
   every entry that leaves the chain to fire `leave()`
   [observed: EnvironmentManager.cpp:31-37]. A reload destroys every body on
   that chain between two frames ⇒ use-after-free on the next update. The fix
   is not a guard at the reload site: `ModularBody::~ModularBody` now **pushes**
   the destruction to the aggregation (I3), which is I5's own remedy clause
   ("legal if destruction-notified"). Same latent bug reachable today by
   `body action drop` on a chain ancestor — that is why it was fixed at the
   class rather than around the instance (I6).
2. **`ModularSystem::reloadSystem()` gained a guard and a bool return.** As
   written it did `clearChildren(); loadSystem(systemFilename)` unconditionally.
   On a system with no source file — the universe/milkyway spine nodes — that
   **destroys the hierarchy** and reloads nothing. It now refuses, with an
   actionable diagnostic (§2(f)).

**Full measurements**: INTENT.md §11.55 (a)–(k).
**Instruments committed**: `harness/b16_{run.sh,probe.gdb,reload.py,overrides.py,channels.py,reload_check.sts}` + README section.
**Artifacts**: `harness/artifacts/b16{,ovr,ch}/` (gitignored) + `artifacts/b16_measurements.json` (committed).

**The command, verbatim**: `body action reload`

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1 | Command exists and positively reaches its handler (running-process evidence) | **met** | App launched **under** gdb (`b16_probe.gdb`), breakpoint on `SSystemFactory::reloadCurrentSystem`. Hits vs invocations: run 1 **3/3**, run 2 **1/1**, run 3 **2/2**. Corroborated by the handler's own line `System 'SolarSystem' reloaded (observer state kept)` at matching app-clock stamps (030420 / 038822 in the channels run). The 1:1 count is the discrimination against the §11.54(j) silently-swallowed-spelling class |
| 2 | The reload actually reloaded; data file restored byte-identically | **met** | File the running app loads: **`~/.spacecrafter/ssystem.ini`** — relative path resolved against the process cwd, which `main.cpp:194` sets to `~/.spacecrafter/`; the Solar system is created with the literal `"ssystem.ini"` [ssystem_factory.cpp:119] opened by a plain `ifstream` [ModularSystem.cpp:837]. Mutation `[moon] radius = 1737.4 → 3474.8`; md5 `fb87a774e728706e9d4e1959c386bb23` → `894a0ccdc5a78728d6c3e635808aeb1c` → **`fb87a774e728706e9d4e1959c386bb23`** (asserted in-driver, re-checked from the shell after the run). Live-tree observable: Moon `boundingRadius` **1.18460794e-05 → 2.36921551e-05 AU (ratio 2.0000000) → 1.18460775e-05**. Screen: baseline vs mutated **px>8 = 160**, baseline vs restored **px>8 = 0**, no-reload control **px>8 = 0** |
| 3 | State preservation measured, not asserted | **met** | **Measured directly** (`Camera::dumpTrace` + dump header, 5 dumps): `jd` **2461233.500013901**, `reference` **Earth**, `tracked` **Moon**, `longitude` 0.0410152376, `latitude` 0.852593362, `distance` 4.26359038e-05 AU, `heading` 0, `halfFov` 1.57079637, `boundToSurface` true, `freeMode` false, `mount` equatorial — **all bit-identical across three reloads**. `tracked` was added to `dumpTrace` in this task so the tracked-body re-seat is measured, not inferred. **Residual with predicted magnitude**: `alt` ≤ 1.19e-07 rad, `az` ≤ 1.55e-06 rad (0.32″ ≈ 0.001 px) vs a no-reload control spread of 8.9e-08 / 1.20e-07 rad — cause: the tracking loop re-plans every frame on a 5 s eased profile [Camera.cpp:264-266] and has no exact fixed point; predicted decay to control scale on the next sample, **confirmed** (third reload `az` Δ = 1.79e-07). **Inferred, not measured**: nothing |
| 4 | Both entries of the reversible pair; reload while the reference is a destroyed-and-recreated body | **met** | Three reloads in one uninterrupted session (2nd starts from the state the 1st produced with mutated data; 3rd from the 2nd's restored state), plus 1 and 2 reloads in two further sessions. The reference was **Earth** — destroyed and recreated — in every one, and the tracked body **Moon** likewise. It does not break **because** the references are re-seated by name: `ModularBodyPtr::redirect` alone leaves them **valid** (they land on the surviving SolarSystem node), which is not **preserved**. Also traversed: reload while the reference IS the system node (`ref=SolarSystem` after a free-flight descent) — works |
| 5 | §2(c) channels | **met** | §2(c) asks for scriptable AND scriptless; one command registration serves both because the script executor and every live channel share `AppCommandInterface::executeCommand` [script_mgr.cpp:311/330; app.cpp:737 mkfifo, :746 TCP; ui.hpp:203; joypad_controller.cpp:224]. **Both were run**: live TCP, and `script action play filename b16_reload_check.sts` (single line `body action reload`) — probe **2 hits / 2 invocations**, state kept in both dumps. **The config channel does not apply**, and no key was invented: a reload has no startup value to configure, and §11.50(c)'s rule is that a flag lands in a file only when the choice is actually relevant to someone |
| 6 | Build green; binary mtime advanced | **met** | `make -C /home/claude/spacecrafter/build-claude -j$(nproc)` exit **0** twice (23:55:10, 23:57:28); mtime advanced from 15:44 both times |
| 7 | No regression (scenes A–D, scene E), `config.ini` restored byte-identically | **met** | See below |
| 8 | Trackers | **met** | INTENT.md **§11.55** (new, (a)–(k)); §13.B **B16 → DONE**; §2(c) live-instance struck; §11.36 rare-path line + suspended list; §11.45(d) capability table row `reloadSystem` **NO → yes**; §11.45(d) "Reading" item (v); §12 S6 summary; §9 Body add/remove row; harness README; this file's row + this section |
| 9 | Committed on master-beta, correct author/co-author, never pushed | **met** | see commit list below |

### No-regression, at the heights this could plausibly move

Fresh launches, FISHEYE, `init_fov = 340`, `debug_layer = true`.

- Scenes A–D (`drive_scenes.py` + `predict.py`): P1 exact ≤ **8.48e-17**;
  P2 mat-residuals ≤ **1.09e-07**; P3 old-vs-new angles ≤ **1.32e-05 deg**,
  relative distances ≤ **2.20e-07**; P4 observer parity **16.32 / 13.44 /
  27.14 / 55.06 / 14.17 km**; P5 ≤ **2.06e-07**, same named causes. The
  §11.16/§11.45 class, unchanged (B19 measured 12.55/21.29/21.10/55.07/25.94 km
  on the same scenes — the spread is the tracking-phase sampling, not drift).
- `orientation_check.py /tmp/gen_a.json`: **17 parity-restored / 48
  named-divergent**, P-d **0.0000 deg** — identical to §11.54's figures.
- Scene E (`scene_e_spine.py`, fresh launch): **13/13 OK**, both second
  entries, Mars landing 2.270821e-05 AU.
- **Zero VUID** on every launch [`grep -c VUID ~/.spacecrafter/log/vulkan.log` = 0].
  Clean `shutdown action now` exits ×2, **no §11.15d fire** (data point for
  ledger B7, binary mtime 2026-07-21 23:57:28).

### Findings recorded, not fixed (out of scope)

1. **The reload is new-path only.** `ModularSystem::reloadSystem` has no
   old-path counterpart — `ProtoSystem::load` loads, nothing dismantles the
   old system. Under the flipped default (new path pinned, §11.53) the visible
   result is right for every unconfigured user; on `render_path = old` the
   command changes nothing visible. Retires with the old path; an old-path
   teardown that is scheduled for deletion was not worth writing.
2. **The two paths desync on body-scoped state after a reload.** Measured:
   `body name Mars hidden true` → reload → new-path `relation` back to 4
   (visible) while the old path still holds Mars hidden. Same for the scale
   seams. This is the observable face of the suspended question below.
3. **`ModularSystem::loadSystem` opens an unqualified relative path.** It
   works only because `main.cpp:194` chdirs into `~/.spacecrafter/`; it does
   not go through `FilePath` like every other data file, so a future change to
   the working directory silently empties the new-path system (the failure
   mode is a log line, `Unable to open file ssystem.ini`, and a black sky).
   Not touched — it is load-path structure, not this row.

### Suspended for Vixy

**Does "keep current state" extend to body-scoped runtime overrides?**
The decision (Q27/A20) names *camera + date*, and both are kept exactly. What
the rebuild-from-file necessarily drops is everything a runtime command had
changed **on the bodies**. Measured, not assumed
[`harness/b16_overrides.py`, artifacts `b16ovr/`]:

- `flag moon_scaled on` with a live Moon `scaling` of **5.0000** →
  `boundingRadius` **5.92303877e-05 → 1.18460775e-05 AU** (back to ×1);
- `body name Mars hidden true` → `relation` **1 (hidden) → 4 (orbiting)** on
  the new path, **while the old path still holds Mars hidden**.

The same mechanism is the only sub-ulp figure in the main run (launch-built
Moon `boundingRadius` 1.18460794e-05 vs reload-built 1.18460775e-05 = 1.6e-07
relative ≈ 1.3 float32 ulp — the `ASmooth`-eased `scaling` asymptote vs an
exact 1).

**Options seen** (none chosen): (1) **file wins for bodies** — today's
behaviour, simplest, and arguably what the word *reload* means; (2) **re-apply
recorded overrides after the rebuild** — needs a per-body override ledger that
does not exist, and it is the *same* ledger B29's colour-authority row needs;
(3) **re-apply only the global scale seams** — cheap, covers the common case,
and arbitrary about where it stops.

### Reproduction (verbatim)

    # the deliverable run (mutation + restore + reversible pair)
    cd /home/claude/spacecrafter
    DISPLAY=:2 ./src/experimentalModule/harness/b16_run.sh b16_reload.py \
        "$PWD/src/experimentalModule/harness/artifacts/b16"     # driver exit=0

    # what the reload does NOT keep (the suspended question)
    DISPLAY=:2 ./src/experimentalModule/harness/b16_run.sh b16_overrides.py \
        "$PWD/src/experimentalModule/harness/artifacts/b16ovr"  # driver exit=0

    # 2(c) both channels, one launch
    cp src/experimentalModule/harness/b16_reload_check.sts ~/.spacecrafter/scripts/
    DISPLAY=:2 ./src/experimentalModule/harness/b16_run.sh b16_channels.py \
        "$PWD/src/experimentalModule/harness/artifacts/b16ch"   # driver exit=0
    rm ~/.spacecrafter/scripts/b16_reload_check.sts

    # no-regression (needs init_fov = 340 in ~/.spacecrafter/config.ini,
    # restored to 180 afterwards - verified by md5)
    DISPLAY=:2 ./build-claude/src/spacecrafter &   # wait for :7805, +10 s
    cd src/experimentalModule/harness && python3 ./drive_scenes.py
    for f in /tmp/gen_a.json /tmp/gen_b.json /tmp/gen_moon.json \
             /tmp/gen_mars.json /tmp/gen_mars_2.json; do python3 ./predict.py $f; done
    python3 ./orientation_check.py /tmp/gen_a.json
    #  fresh launch, then:
    python3 ./scene_e_spine.py               # exit 0, 13/13

    # the negative case (guard + actionable diagnostic), on a live instance:
    #  camera action free_mode state on
    #  moveto altitude 73530345406464000000000 duration 0   -> ref=Universe
    #  body action reload   -> L_ERROR "Cannot reload the system 'Universe' ..."

### Hygiene

`~/.spacecrafter/config.ini` restored byte-identical [md5
`03fbee59bc3ec506c58f0a3f1e1d73df` before and after the temporary
`init_fov 180 → 340`]. `~/.spacecrafter/ssystem.ini` restored byte-identical
[md5 `fb87a774e728706e9d4e1959c386bb23`], and the driver asserts it in-run.
`~/.spacecrafter/beta_features.ini` absent throughout (shipped state).
The channel-2 `.sts` was removed from `~/.spacecrafter/scripts/` and lives in
the harness directory instead. `supervised-by.sh` left untracked. No harness
task list touched.
