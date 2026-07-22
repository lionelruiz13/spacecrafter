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
| B9 | ~~Az-convention divergence: old `getAltAz` applies 3π−az, new returns Camera-frame raw — probe, then fix at the `ModularObject` surface~~ **DONE 2026-07-22 → §11.60, §11 below** | §11.4, **§11.60** | **Probe REFUTED `3π−az`**: the surface delta is `az_old = π/2 − az_new` (the `3π−az` was old's internal raw→report step; the new raw frame is −π/2 off — the §11.4 line-753 caveat, measured). ONE authority `ModularObject::altAz()` (`az = π/2−raw`) feeds getAltAz + getInfoString + getShortInfoNavString (single `observedPosToAltAz` call site). 234/234 bodies ≤0.00003° pre-fix convention; post-fix parity ≤0.00002°, alt untouched; nav/info strings arcsec-identical to old; also closed the §11.4 label-order swap. **Findings**: ModularObject uninstantiated in production (fix readies the D2 bridge, no user-visible change yet); RA/DE sibling diverges (§5.19, out-of-scope). Locked by `harness/b9_azconv.py` (FAIL pre-fix, PASS post-fix). No regression |
| B11 | ~~Trail recording gate: `flag trails off` STOPS accumulation; re-enable starts FRESH~~ **DONE 2026-07-22 → §11.56, §7 below** | §11.41, §11.48(a), **§11.56** | Display flag now gates recording (`want`, not the fader — the prior port gated on the fade animation). **2×2 matrix measured**: hidden Mars records identically to visible Venus (both +6 pts / +1080 accumulate calls flag-on, both 0 flag-off) — the two gates are orthogonal, §11.54's boundary closed in the affirmative. "Work stopped" proven 2 ways (`accumulateCount` frozen + gdb accumulate breakpoint silent while off). Resolved inside I2/I6 — no second walk, no scheduling change (B1/S4 untouched). Real command spelling = `flag object_trails on|off` (the row's `flag trails` is swallowed silently). 94/94 harness, 0 VUID, config byte-identical |
| B13 | ~~Reference-change view continuity: preserve absolute sky direction across reference switch and free-mode entry/exit; no re-centring~~ **DONE 2026-07-22 → §11.61, §12 below** | §11.19c, §11.48(a), **§11.61** | **`warpToBody` (`set home_planet`) was the ONLY re-centring path** — it held (alt,az,heading) frame-relative and jumped the sky **78.60°** (measured); `switchToBody`/`setFreeMode` already preserved absolute since §11.36. Mechanism = the EXISTING `recoverParams` deduce-identical-view primitive (the `view` quaternion stays retired per §11.19c); inverse formula = `R = viewRotation·placement·calculateSwitchCompensation(dst)`, ONE recoverParams (not switchToBody's freeMode round-trip — that corrupts longitude across a ref change). **Post-fix absDelta ≤6e-6°** (recoverParams Euler floor, = switchToBody's 8e-6°; B30 adds ≤5e-3° fresh-launch), **alt/az delta 78–90° = discriminator** (the two SWAP). Sky-lock composition HANDLED (`lockedSkyRot·comp`, locked switch 6e-6°). Scene E **13→21 OK**, discrimination proven by a recoverParams-disabled mutation (3 ref-switch asserts FLIP to FAIL). **Conflict logged**: B18 §11.58(g)'s "setMount has no caller / runs ALTAZ" is wrong — runtime `mount:equatorial`. No regression (P4 17–51 km, orient 17/48, P-d 0.0000°), config byte-identical |
| B15 | AoI re-derivation on date change: the launch-jd latch is a defect (dates are jumped mid-navigation) | §11.36, §11.48(a) | Recompute cadence (date-jump event vs continuous) is an engineering call bounded by constraint C3. Should close the ~10% seasonal drift and the scene-E `e_in` first-run miss — verify both |
| B16 | ~~Expose `reloadSystem` as a command; keep current state (camera + date), no reset~~ **DONE 2026-07-21 → §11.55, §6 below** | §11.36, §11.45(d), §11.48(a), **§11.55** | Landed as **`body action reload`**; both §2(c) channels exercised live. **Scope grew by one structural fix**: the reload's first live use exposed an I5 violation (EnvironmentManager's cross-frame raw-pointer chain cache dereferences freed bodies) — fixed at the class by destruction notification. **One question suspended**: does "keep current state" cover body-scoped runtime overrides (today the file wins, and the old path desyncs) |
| B17 | Port `view_offset` / `zoom_offset` as a Camera parametrization, config + command channels | §11.19a, §11.45(d), §11.48(a) | In use for tilted dome geometry ⇒ it is a projection-space offset — must NOT be re-derived as a camera rotation |
| B18 | Port `flag_lock_equ_pos` (equatorial-mount sky-lock) | §11.19c, §11.48(a) | The "unexercised legacy feature" premise was wrong — it is exercised, just not by our harness |
| B19 | ~~Hidden-body ticking: current behavior (hidden ⇒ keeps updating) is ratified — lock it with a regression assert~~ **DONE 2026-07-21 → §11.54, §5 below** | §11.15b, §11.36, §11.48(a), **§11.54** | **The row's premise was FALSE**: the new path froze hidden bodies (0.00 km advance over 20 simulated min vs old's 1306.81 km), and the 14 bodies shipped `hidden = true` had never been positioned (`lastJD = 0`; Pluto 5.75e9 km off). So it WAS a behavior change — scope expanded from "assert only" to "implement the ratified semantics + assert", traceable to Q13/A10. Locked by `harness/b19_hidden_tick.py` |
| B20 | ~~Anchored galactic display: at galactic distances while anchored, show the solar-system view from very far, anchor kept — no altitude-driven mode switch~~ **DONE 2026-07-22 → §11.59, §10 below** | §11.36, §11.48(a), **§11.59** | **VERIFY-ONLY, no product code** (A13 "no code change" ratifies the current impl). Anchor kept at 4.9e11 AU (ref=Earth, distance ≫ refAoI ⇒ switch suppressed by the `if(freeMode)` guard); free-flight same-move escalates to Universe (the discriminator). Locked by `harness/b20_anchored_galactic.py`, counterfactual proven. **Finding SUSPENDED**: round-trip anchor does NOT survive — OLD executor re-anchors Earth→SolarSystem on descent (§11.36 "anchored-mode descent"); fix is §6.9/escalation-policy = Vixy's |
| B22 | System-collapse cross-fade at the ~16 px resolved↔dot threshold, "if not too costly" | §11.36, §11.48(b) | The COST BOUND is the decision input: deliver the cross-fade + its measured cost. The threshold constants themselves stay open (A15 — Vixy/tester) — do not tune them here |
| B23 | ~~Restore planet-grid tropics + polar circles, keyed to the corresponding sky-line flags~~ **DONE 2026-07-22 → §11.57, §8 below** | §11.42, §11.48(b), **§11.57** | Tropics ride **LINE_TROPIC** (`flag tropic_lines`), polar circles **LINE_CIRCLE_POLAR** (`flag polar_circle`), at ±axial_tilt / ±(90−axial_tilt). **§11.42's "no axial-tilt scalar" was a cached conclusion, false at source** — `axial_tilt` has loaded into `re.axialTilt` since the port; only a getter was missing. Measured latitudes track obliquity (Earth 23.44/66.56, Jupiter 3.13/86.87, Uranus 97.77/−7.77); screen gating px>32 vs a **0** noise floor, Earth↔Uranus ring reversal on the frame; name-sniff `!="Sun"` → `!isStar()` (I4). Carve-out kept (no independent toggle, no >10 km regime). 25/25 harness, 0 VUID, config+ssystem byte-identical. Finding: the Sun installs no grid at all (out of scope) |
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

---

## 7. Execution log — B11 (Claude Opus 4.8, 2026-07-22)

**Task**: wave §1 task 4 — implement the trail recording gate. Vixy Q14 /
§11.48(a) A1: the display flag gates recording — `flag object_trails off`
STOPS accumulation, and re-enabling starts FRESH (stated reason: the cost of
accumulating a trail nobody sees). Closes the §11.41 display/recording
suspension. The row's core warning: this is a SECOND gate, INDEPENDENT of the
B19/A10 hidden-visible gate — reading the two as one produces a wrong
implementation.

**No behavior change beyond the row.** The gate is exactly what the row
anticipated; the only judgement call was where the fresh-start discard fires
(recorded as design edge (h) in §11.56 — an unreachable same-frame double-
toggle case, documented not guarded). The prior port (§11.41) had gated
accumulation on the DISPLAY FADER interstate, which is neither of the two gates
Vixy named — the fix moves the gate to the flag (`want`), leaving the fader as
the display gate `draw()` reads.

**The interaction the task flagged, resolved and stated.** B19 made hidden
bodies tick translation-only, with NO module update — so if trail recording
lived in a per-body module-update walk, "hidden ⇒ still recording" would not
hold. It does not live there: recording rides `ModularSystem::drawTrails`, a
system-level phase that sweeps every evaluated body in `sortedSystemBodies`
(hidden bodies are still in that list; B19 keeps their position current). So
one authority (I2), no second walk, no scheduling change (B1/S4 never
approached). Verified in the 2×2 matrix: hidden Mars records identically to
visible Venus.

**Full measurements**: INTENT.md §11.56 (a)–(i).
**Instruments committed**: `harness/b11_{run.sh,probe.gdb,trail_gate.py}`.
**Artifacts**: `harness/artifacts/b11/` (gitignored).

**The command, verbatim**: `flag object_trails on|off` (NOT `flag trails` — the
row's shorthand does not exist and is swallowed silently, verified live).

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1 | Both gates measured independently on a 2×2 matrix {flag on,off}×{visible,hidden}, values not adjectives | **met** | `b11_trail_gate.py`, deltas over 6 jumps of 15 sim-days: ON/visible +6 pts +1080 acc; ON/hidden(Mars rel=1) +6 pts +1080 acc; OFF/visible 0/0; OFF/hidden 0/0. Within a flag column hidden==visible (visibility inert on recording); across columns the same body differs (flag is the gate). Independence asserted, not assumed |
| 2 | "Starts FRESH" measured — no pre-off history after off→on | **met** | Per cycle/body: pre-off ≥2 pts → off **points=0, recording=false** → reon **points=1**, first point at **|head−ecl|=0.000000 km**, `head.jd`==body `lastJD` (light-time-corrected; ≠ header jd by the body's light-travel offset) and > pre-off head jd. Regrows to 1+6 |
| 3 | Both entries of the reversible pair, twice (on→off→on→off→on, cycle 2 from cycle 1's produced state) | **met** | Phase 4/5 loop c∈{1,2}, cycle 2 with no re-setup between; all fresh-start asserts green both cycles. Plus the sub-fade re-enable (off, on 0.5 s later): still points=1 — the assert that separates the fixed design from a fader-gated one |
| 4 | The WORK stops when off (not just the drawing) — running-process evidence | **met** | (1) `TrailModule::accumulateCount` (dump) **frozen** across every OFF interval while sim time advanced 2 jumps (dAcc=0). (2) gdb breakpoint on `TrailModule::accumulate` (armed on each flag-ON) fired **6 `PROBE accumulate RAN`, each after a `planetsSetFlagTrails b=1`, none during OFF**. `resetTrail()` empties the buffer (frees the memory too) |
| 5 | Terminal observable on the composed screen, calibrated px>N + noise floor | **met** | Noise floor (same-state pair 1.5 s apart): **max\|Δ\|=0, 0 px at every threshold** (frozen tracked scene bit-stable — measured, not assumed vs B30). Trail present vs absent same date: **25 274 px>32, max\|Δ\|=101** (off frame 2.7M nonblack px — content, not masked fiction). Fresh reon vs off: **0 px** (1-pt buffer draws nothing, `n<2`) — re-enable flashes no grown trail |
| 6 | Command spelling verified from the running process | **met** | `flag object_trails on|off` → `CoreLink::planetsSetFlagTrails`: **11 probe hits / 11 commands, 1:1**. Bogus `flag trails off`: **0 probe hits**, state unchanged (+2 pts over 2 jumps, recording stayed true) — §11.54(j) silent-swallow class |
| 7 | Build green, mtime advanced | **met** | `make -C build-claude -j$(nproc)` exit 0; binary mtime 2026-07-21 23:57 → 2026-07-22 00:31; `accumulate` confirmed out-of-line-called from `update` (`objdump`: `call <…TrailModule10accumulate…>`) |
| 8 | No regression A–D + E, numbers vs recorded classes; config byte-identical | **met** | A–D `predict.py` mat-residual ≤1.09e-07 (deterministic witness); P4 settle-noise class (run1 14.36/13.30/27.24/55.04/25.94, run2 2.86/44.47/41.87/46.01/25.94 km — varies run-to-run, B16's class). Scene E 13/13 exit 0. Orientation **17 restored/48 divergent, P-d 0.0000** (exact §11.54/§11.55 match). 0 VUID, layer positively confirmed. config.ini md5 **03fbee59…** in and out |
| 9 | Trackers | **met** | INTENT §11.56 (entry), §13.B B11 → DONE, §12 row 9 suspension closed; this dispatch row + §7 |
| 10 | Committed on master-beta, correct author/co-author, no push | **met** | see commit hash below |

### Findings recorded, not fixed (out of scope)

- **`flag trails` is not the code's spelling** — the B11 row, §11.41's header,
  and this wave file all wrote `flag trails`; the real flag is
  `flag object_trails` (`FN_OBJECT_TRAILS`, define_key `flag_object_trails`).
  Corrected in the new INTENT §11.56 and §13.B rows; historical mentions left
  as written per the do-not-rewrite-history rule.
- **Design edge (h)**: the rising-edge fresh-start relies on an OFF-frame's
  `update()` clearing `recording`; a zero-frame-gap off→on would not restart.
  Unreachable through any command channel — documented, not guarded.

### Suspended for Vixy

None. Q14 fully specified the gate; every decision traces to it or to the old
port's shape. (The per-name enable was made to start fresh too, by the same
Q14 rule — a per-name enable is a re-enable; stated in the header, not a new
policy.)

### What I did NOT verify

- The gate under `render_path = old` (old path unchanged by construction; the
  new path is the pinned default per §11.53).
- Trail behavior across a `body action reload` (B16 territory; §11.55(i)'s
  override-reset question is open and would reset trail flags too).
- Perceptual A/B against the OLD path's trail recording gate — not required
  here (this is a new-path semantics decision Vixy specified, not a parity
  port), and the old `trail_on` was DEAD (§11.41), so there is no old coupled
  observable to match.

### Reproduction (verbatim)

    # the deliverable run — 2×2 matrix, fresh-start ×2, spelling, screen A/B
    cd /home/claude/spacecrafter
    DISPLAY=:2 ./src/experimentalModule/harness/b11_run.sh \
        b11_trail_gate.py                        # driver exit=0, 94/94
    #  -> artifacts/b11/{b11_result.json, drive.log, gdb.log, *.png}
    #  probe counts: planetsSetFlagTrails 11, setPlanetHidden 2, accumulate RAN 6

    # terminal observable (in artifacts/b11/):
    #   b11_screen_on_a vs b11_screen_off   = 25274 px>32 (trail present/absent)
    #   b11_screen_reon vs b11_screen_off   = 0 px        (fresh reveals nothing)
    #   b11_screen_on_a vs b11_screen_on_b  = 0 px        (noise floor)

    # no-regression (needs init_fov=340; restored to 180 by md5 afterwards)
    DISPLAY=:2 ./build-claude/src/spacecrafter &   # wait :7805, +10 s
    cd src/experimentalModule/harness && python3 ./drive_scenes.py
    for f in gen_a gen_b gen_moon gen_mars gen_mars_2; do \
        python3 ./predict.py /tmp/$f.json; done
    python3 ./orientation_check.py /tmp/orient_b11.json   # 17/48, P-d 0.0000
    #  fresh launch, then:
    python3 ./scene_e_spine.py                             # exit 0, 13/13

    # validation sweep (layer confirmed + 0 VUID)
    DISPLAY=:2 VK_LOADER_DEBUG=layer ./build-claude/src/spacecrafter 2>&1 \
        | grep -E "Insert instance layer|VUID"

### Hygiene

`~/.spacecrafter/config.ini` restored byte-identical [md5
`03fbee59bc3ec506c58f0a3f1e1d73df` before and after the temporary
`init_fov 180 → 340`; the app rewrites config on shutdown, so restore is from
the pre-run backup]. `~/.spacecrafter/ssystem.ini` untouched.
`~/.spacecrafter/beta_features.ini` absent throughout. No temporary `.sts`
installed. `supervised-by.sh` and the root-level `USER_QUESTIONS*.md` /
`FEATURE_REQUESTS.md` left untracked. No harness task list touched.

## 8. Execution log — B23 (Claude Opus 4.8, 2026-07-22)

Full record: **INTENT §11.57**. Planet-grid tropic + polar circles restored on
the body, keyed to the sky-line flags, at the body's own obliquity.

**First paragraph / behavior beyond the row:** additive only. Tropic (±axial_tilt)
and polar (±(90−axial_tilt)) circles were ADDED to the grid; nothing was removed
(the §11.42 ±30/±60 generic parallels stay). A per-frame poll
(`Core::syncPlanetGridSkyState`) was added at both modular-draw sites to push the
`LINE_TROPIC`/`LINE_CIRCLE_POLAR` show+color state into the grid module. The dump
(`body action dual_dump`) gained a `"near"` array (harness instrument, additive
JSON). No A4-carve-out territory was crossed (no independent grid toggle, no >10 km
near-surface regime).

### DoD, item by item
1. **Identify + name the coupling** — **met**. Old `Body::drawPlanetGrid` polls
   `LINE_TROPIC` for tropics and `LINE_CIRCLE_POLAR` for polar circles under the
   `flag_planet_grid` master gate [observed: body.cpp:1255-1258]; commands
   `flag tropic_lines` / `flag polar_circle` [observed: base_command_interface.hpp:409,412].
   Verified the flags exist and are dispatched from the running process (dump's
   `showTropics`/`showPolarCircles` echo them; bogus `flag tropic off` → 0 hits).
2. **Lines render at ±obliquity / ±(90−obliquity), move with obliquity** — **met**.
   Measured from the process dump: Earth tropicLat **23.440**, polarLat **66.560**;
   Jupiter **3.130 / 86.870**; Uranus **97.770 / −7.770** (`tropicLat==axial_tilt`,
   `polarLat==90−axial_tilt`, all distinct). On screen the Earth↔Uranus ring
   positions reverse (Earth tropic 459 px inside polar 662; Uranus tropic 695 px
   outside polar 399).
3. **Keyed to the flags, measured (px>N + noise floor)** — **met**. Noise floor
   (base vs base2, frozen) = **0 px>0** all three planets. ON present / OFF→base-exact,
   each reversible pair twice, px>32: Earth tropic **10 973** / polar **4 196**,
   Jupiter **6 410 / 492**, Uranus **1 449 / 5 431**; OFF reverts at **0 px>0**.
4. **Terminal observable, not pipeline state** — **met**. All px numbers are on the
   composed 2048² FISHEYE frame; the obliquity reversal is asserted on the rendered
   frame (radial-distance means), not on intermediate state.
5. **Command spelling from the process** — **met**. Bogus `flag tropic off` →
   `showTropics` stayed false, 0 px; real `flag tropic_lines on` → true, 10 973 px.
6. **Build green** — **met**. `make -C build-claude -j$(nproc)` exit 0; binary
   mtime advanced 00:31 → 01:28:53 across the two product-code builds (the trailing
   confirmation build was a no-op — no source changed after 01:28:53).
7. **No regression** — **met**. Fresh launches, init_fov=340, FISHEYE, debug_layer.
   A–D: P4 observer parity **24.02 km** (13–55 km settle-noise class); orientation
   **17 restored / 48 divergent, P-d 0.0000°**; position parity baseline. Scene E
   **13/13**, Mars landing 2.270821e-05 AU. 0 VUID, layer positively confirmed.
   `config.ini` md5 **03fbee59…** in and out; `ssystem.ini` md5 **fb87a774…** in and out.
8. **Trackers** — **met**. INTENT §11.57 + §12 row 11 + §13.A A4 + §13.B B23; this section.
9. **Committed on master-beta** — see commit hashes below.

### Deviations / judgement calls (each with its reason)
- **±30/±60 generic parallels kept** (not replaced with old's exact equator+tropic+polar
  set). Additive was the minimal, carve-out-safe choice; the exact parallel set is part
  of the still-open "full planet-grid observable parity" (A4). Recorded, not decided.
- **Tropic/polar colors ARE plumbed** (LINE_TROPIC/LINE_CIRCLE_POLAR, via the same poll),
  matching old; the meridian/parallel color seam (§11.42, command-push) left untouched —
  the SUSPENDED color-authority stays Vixy's.
- **Poll, not push**, for the flag coupling — a true I3 push would couple SkyLineMgr
  (coreModule) to a body module (experimentalModule); rejected with that precondition.
  The poll reproduces old's per-frame read exactly and lives where the coupling already is.

### Findings recorded, not fixed (out of scope)
- **§11.42's "ModularBody exposes no axial-tilt scalar" was FALSE at source** — the
  cached-conclusion / §5.2 class. `axial_tilt` has loaded into `RotationElements::axialTilt`
  since the port [ModularSystem.cpp:716]; only a getter was missing. Corrected in place.
- **The Sun installs NO planet grid** even with `planet_grid=true` (`near`=[null,null]) —
  the system-centre star loads through a path that does not reach the explicit-slot GRID
  declaration. An A4/§11.42-territory grid-installation gap, distinct from B23's scope;
  it makes the `!isStar()` tropic gate unexercisable on shipped data (the guard stands).

### Suspended for Vixy
None new. B23's own residuals (independent grid toggle, >10 km regime, ±30/±60-vs-old
parallels, the Sun-grid gap) all fold into the pre-existing **A4 (c)** suspension.

### Reproduction (verbatim)
    # grid observable + gating (needs planet_grid=true on Earth/Jupiter/Uranus/Moon/Sun
    # in ~/.spacecrafter/ssystem.ini — test-only, restored byte-identical after)
    cd /home/claude/spacecrafter
    DISPLAY=:2 bash src/experimentalModule/harness/b23_run.sh b23_grid.py \
        "$(pwd)/src/experimentalModule/harness/artifacts/b23"   # -> ALL PASS 25/25, exit 0
    # no-regression (init_fov=340 in config.ini, restored to 180 by md5 afterwards)
    cd src/experimentalModule/harness
    DISPLAY=:2 <fresh spacecrafter> & ; python3 ./drive_scenes.py
    python3 ./orientation_check.py /tmp/gen_a.json   # 17/48, P-d 0.0000
    python3 ./predict.py /tmp/gen_a.json             # P4 24.02 km
    DISPLAY=:2 <fresh spacecrafter> & ; python3 ./scene_e_spine.py   # 13/13, exit 0

### Hygiene
`config.ini` restored byte-identical [md5 `03fbee59bc3ec506c58f0a3f1e1d73df`, temporary
`init_fov 180→340`]. `ssystem.ini` restored byte-identical [md5
`fb87a774e728706e9d4e1959c386bb23`, temporary `planet_grid=true` on 5 bodies].
`beta_features.ini` absent throughout. Bulky per-run artifacts (`artifacts/b23/`)
gitignored like b11/b16/b19/b26; the summary `artifacts/b23_measurements.json` is tracked.
`supervised-by.sh` and the root `USER_QUESTIONS*.md` / `FEATURE_REQUESTS.md` left
untracked. No harness task list touched. Known intermittent shutdown segfault (§11.15d)
fired on some run teardowns AFTER the driver exited 0 — did not affect any artifact.

---

## 9. Execution log — B18 (Claude Opus 4.8, 2026-07-22)

**Task:** port `flag_lock_equ_pos` (the equatorial-mount sky-lock) to the new-path
Camera. Full record: **INTENT §11.58**; row **§13.B B18** flipped to DONE; §12 seam row
flipped OLD-ONLY→BOTH. HEAD before: `fb613431`.

**One-paragraph scope statement (report contract):** this change adds new-path behavior
ONLY on the sky-lock path (`Camera::setSkyLock`), which is OFF by default and dormant in
every scene that does not issue `flag lock_sky_position`. It changes no default behavior:
scenes A–D + E reproduce the recorded baseline exactly and every camera dump reports
`skyLocked=false`. It does NOT wire the mount to the new Camera and does NOT add a config
key (neither exists to port — see below).

### DoD, item by item
1. **Old behavior characterized + named** — MET. [observed: navigator.cpp:123-135] lock ON
   holds `equ_vision` (earth-equatorial direction, fixed to the sky) and recomputes
   `local_vision` each frame; default is the mirror. Roll/up is set separately by the MOUNT
   (navigator.cpp:267-288), so shipped `viewing_mode=equator`+lock = whole orientation frozen
   in the equatorial frame. Spec stated in §11.58(a) before implementing.
2. **Observable reproduced + measured** — MET. Δjd=0.05 day (18.0493° sidereal). ON: new
   equ-frame delta **0.0000°**, alt-az **11.8504°**; OFF: equ **18.0493°**, alt-az **0.0000°**
   — the discriminator (both-nonzero would be a no-op) holds. Cross-path: old `helioToEye`
   equ-delta == new `mat` equ-delta to all digits in both states. Units: degrees of
   view-direction change. [measured: b18_analyze.py]
3. **Both channels reach the code** — command MET, config **n/a (does not exist in old)**.
   `flag lock_sky_position` → gdb breakpoint on `Camera::setSkyLock` fired **4× (b=1,0,1,0)**
   for 4 real commands, **0** for bogus `flag lock_sky_positionX` (dump `skyLocked=false`).
   Config: `flag_lock_equ_pos` has no config reader in the old path (reset to 0 every init,
   core.cpp:396) — §2(c)'s config half does not apply; not invented (§11.58(e), corrects the
   §11.48(h) expectation that conflated it with the separate `viewing_mode` mount).
4. **Both reversible entries** — MET. on→off→on→off; second `on` from the first `off`'s state
   re-froze equ (0.0000°), each `off` restored the horizon lock (equ 18.049x, alt-az 0). [b18]
5. **Terminal observable** — MET. Composed screen (2048² FISHEYE): locked pair **0 px>32**
   (max|d|=4), unlocked pair **142 566 px>32**, noise floor (same state twice) **0 px>0**.
6. **Build green** — MET. `make -C build-claude -j$(nproc)` exit 0; binary mtime → 02:35:38.
7. **No regression** — MET. Scenes A–D: P4 **13.25–55.04 km**, orientation **17/48**, P-d
   **0.0000°**, per-body deltas = recorded classes; scene E **13/13**, Mars landing
   **2.270821e-05 AU**. `config.ini` md5 **03fbee59…** in==out on both runs (byte-identical).
8. **Trackers** — MET. INTENT §11.58 + §12 row + §13.B B18 + this section.
9. **Committed on master-beta** — see the commit hash at the end of this section.

### Deviations / judgement calls (each with its reason)
- **`Core::setFlagLockSkyPosition` de-inlined** (header → core.cpp) so the both-paths mirror
  has ONE source (I2) reachable by the command AND the turn/drag unlock sites, without a
  Camera include in core.hpp. The internal `navigation->setFlagLockEquPos` calls that BYPASS
  this method (core.cpp:938,1979 select-while-tracking) are deliberately NOT mirrored — that
  is a tracking-composition decision, suspended.
- **Design choice: hold the WHOLE composed rotation** (recoverParams(lockedSkyRot)) rather
  than direction-only. Exactly reproduces the shipped VIEW_EQUATOR+lock observable and reuses
  the existing deduce-identical-view authority; the VIEW_HORIZON roll difference this implies
  is suspended (below), not a shipped configuration.

### Findings recorded, not fixed (out of scope)
- **The mount (`viewing_mode`) is not wired to the new Camera** — `Camera::setMount` has no
  caller; the shipped `viewing_mode=equator` reaches only the old path. Pre-existing; the
  sky-lock's drift observable is mount-independent, so parity holds regardless (§11.58(g)).
- **§11.48(h)'s "B18 needs config+command" was based on a conflation** of the sky-lock with
  the `viewing_mode` mount — the sky-lock has no config channel in the old path (§11.58(e)).

### Suspended for Vixy
- **VIEW_HORIZON mount + sky-lock roll**: old rolls with the local zenith (direction-only
  hold); this port holds the whole orientation (no roll). Coincident for the shipped
  EQUATORIAL mount; the split needs a decision once the mount is wired.
- **free-mode + sky-lock composition** (hold dormant in freeMode today).
- **select-while-tracking auto-enable** (old auto-sets the flag on select-while-tracking):
  whether the new path should auto-engage sky-lock depends on Camera `target`-tracking
  composition, structurally unlike old `flag_traking`.
- **A startup config default for the sky-lock** — would be a new user-visible key the old
  path never had.

### What I did NOT verify
- **Reference switch WHILE locked**: `switchToBody`/`warpToBody` do not re-capture
  `lockedSkyRot`, so a switch under lock would hold the OLD body's equatorial orientation.
  Not exercised. **B13 (view continuity) inherits `skyLocked`/`lockedSkyRot` as Camera
  orientation state to carry across reference switches** — flagged for that task.
- Sky-lock under a live `moveto` observer motion (only static-observer sidereal advance and
  discrete date jumps were measured; the hold runs after the move block, so it should hold,
  but it was not A/B'd).

### Reproduction (verbatim)
    cd /home/claude/spacecrafter
    make -C build-claude -j$(nproc)                                  # exit 0
    DISPLAY=:2 bash src/experimentalModule/harness/b18_run.sh        # -> probe 4×(1,0,1,0), md5 match
    python3 src/experimentalModule/harness/b18_analyze.py            # -> OFF equ 18.05/altaz 0; ON equ 0/altaz 11.85
    # no-regression (A-D at init_fov 180; E edits init_fov 180→340 then restores by cp)
    cd src/experimentalModule/harness
    DISPLAY=:2 <fresh spacecrafter> & ; python3 ./drive_scenes.py
    python3 ./orientation_check.py /tmp/gen_a.json   # 17/48, P-d 0.0000
    python3 ./predict.py /tmp/gen_a.json             # P4 15.06 km (13.25–55.04 across A-D)
    DISPLAY=:2 <fresh spacecrafter, init_fov=340> & ; python3 ./scene_e_spine.py   # 13/13, exit 0

### Hygiene
`config.ini` restored byte-identical [md5 `03fbee59bc3ec506c58f0a3f1e1d73df`; the B18 run
sends settings as COMMANDS and never edits the file; the scene-E run edited `init_fov 180→340`
and restored by `cp` of a backup, md5 asserted]. `ssystem.ini` untouched. `beta_features.ini`
absent throughout (new path is the pinned default). Per-run artifacts `artifacts/b18/`
gitignored like b11/b16/b19/b23/b26. `supervised-by.sh` and the root `USER_QUESTIONS*.md` /
`FEATURE_REQUESTS.md` left untracked. No harness task list touched.

## 10. Execution log — B20 (Claude Opus 4.8, 2026-07-22)

**Anchored galactic display (ex-A14 / Q8).** VERIFY-ONLY: no product code changed; the
ratified behavior (A13/Q7 "stay anchored WHATEVER your altitude", A14/Q8 "solar-system view
from very far, anchor kept") is ALREADY implemented by the `if (freeMode)` guard on the
escalation call [Camera.cpp:345-350]. Deliverable = a discriminating regression assert +
its counterfactual proof, plus the precise recording of one negative finding.

### DoD, item by item
1. **Current behavior measured at galactic distance while anchored — MET.** Anchored Earth,
   `moveto altitude 7.353e22 m` (= 4.9152e11 AU = scene-E MW-AoI×1.2): ref=**Earth** UNCHANGED,
   freeMode=false, distance=4.9152e11 AU, refAoI=0.0436, refParent=Sun, refCached=true. No
   auto-transition fired (the escalation call is freeMode-gated). Distance ≫ refAoI ⇒ the switch
   WOULD fire in free flight; suppressed by the anchor. [measured: artifacts/b20g/b20_result.json]
2. **The assertion is discriminating — MET, proven not assumed.** Counterfactual mutation at
   Camera.cpp:345 (drop `freeMode` gate, feed anchored `distance`) → rebuild exit 0 → anchored
   outward move ESCALATES (entry1 ref=MilkyWay≠Earth, refParent=Universe, refCached=false) ⇒
   family B+E FAIL (3 asserts); freeMode instrument-chain + free-flight leg stay green. Reverted
   (git diff Camera.cpp empty) → rebuild exit 0 → ALL PASS restored. [measured: /tmp/b20g_cf]
3. **Reversible pair ×2 — traversed; anchor survives OUTWARD both entries, NOT the round trip.**
   near→out1→back1→out2→back2 (second outward from the first round trip's end state). Outward
   invariant (ref_after==ref_before) holds both entries. **BACK legs re-anchor Earth→SolarSystem**
   — the SUSPENDED §11.36 "anchored-mode descent" (see Findings). Round-trip anchor survival =
   **NOT met**, by a distinct pre-existing old-executor defect, not by the escalation policy.
4. **Contrast with free flight — MET.** Same outward move, freeMode ON → ref=Universe (matches
   scene-E mw_out). The only difference between the two runs is `freeMode`; therefore the ANCHOR
   is what suppresses the switch, not the distance.
5. **Anchor semantics — MET.** At galactic distance ref resolves to its body (Earth), refParent=Sun,
   refCached=true, distance=4.9152e11 AU ⇒ solar-system-from-afar, NOT a recentred galactic view
   (the counterfactual's refParent=Universe/refCached=false IS the recentred view, and it fails).
6. **Build green — MET (verify-only).** No product change; final binary = reverted source
   (mtime advanced 2026-07-22 03:18:01 after the revert rebuild, exit 0). Camera.cpp git-clean.
7. **No regression — MET.** A–D P4 13.48–55.06 km (recorded 13–55 class); scene E 13/13, Mars
   landing 2.270821e-05 AU. config.ini restored byte-identically (md5 03fbee59… in==out).
8. **Trackers — MET.** INTENT §11.59 + §13.B B20 row + §11.36 anchored-mode-descent annotation.
9. **Committed on master-beta — see hashes below.**

### Findings recorded, not fixed (SUSPENDED for Vixy)
- **Round-trip anchor does NOT survive: the anchored descent re-anchors Earth→SolarSystem.**
  [measured: gdb backtrace] `EventHandler::handleEvents` → `SolarSystemModule::onEnter`
  [solarSystemModule.cpp:79] → `SSystemFactory::enterSystem` [ssystem_factory.cpp:419] →
  `changeSystem("Sun")` [ssystem_factory.cpp:271] → `Camera::switchToBody(SolarSystem)`. The
  OLD executor's altitude-driven mode transition drives the new camera's reference, UNGATED by
  freeMode — a second auto-transition mechanism the free-flight-only policy never covered. This
  is §11.36's DISTINCT "anchored-mode descent (unchanged from plan)" suspended item, NOT closed
  by A13/A14, and the same "landed on SolarSystem at solar-radius distance" §11.36 line 611
  measured (my back leg: distance 4.6525e-03 AU ≈ 1 R_sun).
  **CONFLICT for Vixy**: Q7 ("stay anchored WHATEVER your altitude") + A14 ("in case we go back
  afterward") read as requiring the descent to keep the Earth anchor. But suppressing the
  old-executor→new-camera coupling is an escalation-policy / §6.9 mode-dissolution decision, and
  A13 explicitly said "no code change". NOT decision-free ⇒ suspended, not improvised.
  **Question**: at anchored galactic altitude, should the DOWNWARD mode crossing be allowed to
  re-anchor the new camera (old-executor coupling), or must the explicit anchor be held across
  the descent too (requires gating the old executor's `changeSystem→switchToBody` against the
  new-path anchored reference — a §6.9-adjacent change)?

### What I did NOT verify
- The DRAWN screen at anchored galactic distance (whether the solar system actually renders as a
  far dot): the new path UPDATES in every executor mode (§11.36 updateExperimental) but `ssystemFactory->draw`
  is not called in inGalaxy/inUniverse modes until the §6.9 executor-mode dissolution — so the
  reference/anchor is measured from the camera dump (mat layer), the pixel view is not. Same
  §11.36 confound named for spine_mw/spine_uni. B20's claim is the reference/anchor, which the
  dump proves; the pixel composition awaits §6.9.
- Anchors other than Earth going out (the outward invariant is body-agnostic and entry 2 exercises
  a SolarSystem anchor, but not, e.g., a moon or a script-created body).

### Reproduction (verbatim)
    cd /home/claude/spacecrafter
    make -C build-claude -j$(nproc)                                        # exit 0
    DISPLAY=:2 bash src/experimentalModule/harness/b20_run.sh              # ALL PASS, md5 match
    # counterfactual (discrimination proof):
    #   edit Camera.cpp:345  if(freeMode){...position.length()...}
    #     -> { float od = freeMode?position.length():distance; if(auto n=reference->findBetterReference(od)) switchToBody(n); }
    make -C build-claude -j$(nproc)                                        # exit 0
    DISPLAY=:2 bash src/experimentalModule/harness/b20_run.sh b20_anchored_galactic.py /tmp/b20g_cf  # 3 FAIL (family B+E)
    git checkout src/experimentalModule/Camera.cpp ; make -C build-claude -j$(nproc)   # revert, exit 0
    DISPLAY=:2 bash src/experimentalModule/harness/b20_run.sh              # ALL PASS restored
    # no-regression (A-D + E; config init_fov 180->340 by cp of a backup, then restored):
    DISPLAY=:2 <fresh spacecrafter, init_fov=340> & ; python3 src/experimentalModule/harness/drive_scenes.py
    for f in gen_a gen_b gen_moon gen_mars gen_mars_2; do python3 .../predict.py /tmp/$f.json | grep P4; done  # 13.48-55.06 km
    DISPLAY=:2 <fresh spacecrafter, init_fov=340> & ; python3 src/experimentalModule/harness/scene_e_spine.py   # 13/13, exit 0

### Hygiene
`config.ini` restored byte-identical [md5 `03fbee59bc3ec506c58f0a3f1e1d73df` in and out; the b20
run sends settings as COMMANDS and never edits the file; the A-D/E runs edited `init_fov 180->340`
and restored by `cp` of a backup, md5 asserted]. No data files touched. `.gitignore` +1 line
(`artifacts/b20g/`). Per-run artifacts `artifacts/b20g/` gitignored like b11/b16/b18/b19/b23/b26.
`supervised-by.sh` and the root `USER_QUESTIONS*.md` / `FEATURE_REQUESTS.md` left untracked. No
harness task list touched. Camera.cpp git-clean (verify-only — the counterfactual was reverted).

## 11. Execution log — B9 (Claude Opus 4.8, 2026-07-22)

**Task**: wave §1 task 8 — fix the azimuth-convention divergence at the
`ModularObject` surface. Probe first, then install ONE conversion authority
(not per-caller patches, I2/I6). Full record: **INTENT §11.60**.

**FIRST PARAGRAPH / behavior beyond the row.** Two things the row did not say:
(1) **The probe REFUTED the recorded formula.** §11.4/§13.B recorded the delta
as `3π−az`; measured, the surface-to-surface delta is **`az_old = π/2 − az_new`**
— a different constant. The `3π−az` is the OLD path's INTERNAL raw→report step
(body.cpp:381, re-confirmed at source); the NEW raw az sits in a frame −π/2 from
it, which is EXACTLY the risk §11.4's own probe note (line 753) flagged. This is
NOT the "dirty delta → suspend" case the task reserved: the delta is a CLEAN
single convention (234/234 non-degenerate bodies, no per-body/per-hop/per-state
residual), just a different constant, determined by measurement (traceable, not
a design choice). (2) The fix also **corrects the §11.4 label-order swap** in
`getShortInfoNavString` (it had printed alt↔az swapped under the "Az/Alt" label)
— unavoidable, because routing the az through the single authority forces
confronting the order, and old is the parity spec.

**Also a structural FINDING**: `ModularObject` is **uninstantiated in production**
today (referenced only in its own files; selection returns an old `Body`-backed
`Object`). So `getSelectedAZ` already shows the old convention and this fix
changes NO user-visible behavior YET — it readies the D2 bridge to be
parity-correct before it is wired. The harness dump instantiates ModularObject
directly (the only live path to the surface today).

**Instruments committed**: `harness/b9_{run.sh,azconv.py}`; the permanent az
observability added to `SSystemFactory::dumpTracePaths` (per-body `altaz_old`/
`altaz_new` from Body::getAltAz and the REAL ModularObject::getAltAz, + a
`<file>.navstr` sidecar with the caller-visible nav/info strings).
**Artifacts**: `harness/artifacts/b9_{prefix,postfix,horizon}/` (gitignored).

**Files touched (product)**: `src/experimentalModule/ModularObject.{hpp,cpp}`
(the `altAz()` authority + 3 exits routed through it), `src/bodyModule/
ssystem_factory.cpp` (dump observability + include).

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1 | Probe [measured]: az delta old-vs-new ≥3–4 bodies AND ≥2 observer states, is exactly the convention with NO per-body/per-hop residual | **met (result differs from recorded)** | `harness/b9_azconv.py`, one fresh launch, FISHEYE, 4 samples = 2 dates (jd 2461233.5/2461321.25) × 3 observers (Paris/Sydney/North) × tracking ON+OFF. **`az_old = π/2 − az_new (mod 2π)` over 234/234 non-degenerate bodies, max 0.00003°, mean 0.000009°** (float32 class). NOT `3π−az` (that lands 90° off). Only outliers = 4× Earth (observer's home body at nadir, az undefined, alt matches exactly). **Conflict with §11.4 reported, resolved via the layer confusion, not silently** |
| 2 | ONE conversion authority at the ModularObject surface, every consumer through it (grep + reasoning) | **met** | `ModularObject::altAz()` [ModularObject.cpp:119-131] = the single site calling `Camera::observedPosToAltAz` (grep-confirmed: 1 call site). Consumed by `getAltAz` (:133), `getInfoString` (:22), `getShortInfoNavString` (:69). `getSelectedAZ/ALT → getSelected().getAltAz → ModularObject::getAltAz` once the bridge is wired. No uncovered path (the old path DUPLICATED `3π−az` at body.cpp:381+:431; this removes that) |
| 3 | Post-fix parity measured, alt unaffected (measured not assumed) | **met** | fixed binary, same bodies/states: **max \|Δaz\| = 0.00002°**, **max \|Δalt\| = 0.00001°** (equator) / **0.00002°** (altaz). Float32 floor. The `b9_azconv.py` lock flips: pre-fix direct \|Δaz\| ≤179.8° (FAIL) / π/2-hyp ~0; post-fix direct ~0 (PASS) / π/2-hyp 179.8° |
| 4 | Both entries of the reversible pair, else state unconditional | **met** | The conversion has NO state branch ⇒ unconditional. Exercised anyway under tracking ON (S1,S2) and OFF (S3,S4), and under BOTH camera mounts — shipped `EQUATORIAL` and `ALTAZ` (config `viewing_mode` temporarily `horizon`, restored byte-identical; dump header `"mount"` = equatorial then altaz) — identical 0.00002° parity. Mount-independent because `observedPosToAltAz` recovers the horizon frame regardless of the fold (alt matches old EXACTLY even under equatorial). Free mode: different regime (body-frame "local", Camera.hpp:148-149), no old-path az counterpart ⇒ parity undefined, stated not tested |
| 5 | Caller-visible check shows old-convention value after the fix | **met** | The REAL `ModularObject::getShortInfoNavString` + `getInfoString` (captured in `b9_postfix/*.navstr`) print `Az/Alt/coA` **arcsecond-identical to old Body**: Moon `+33°37'16"/-07°11'59"/+97°11'59"`, Sun `+00°55'44"/-19°09'22"/+109°09'22"`, Mars `+41°20'57"/-09°02'56"/+99°02'56"`; getInfoString "Alt/Az" Moon `-07°11'59" / +33°37'16"` both paths |
| 6 | Build green, mtime advanced | **met** | `make -C build-claude -j$(nproc)` exit **0** twice (probe 03:45:54, fix 03:56:46); mtime advanced from 03:18 |
| 7 | No regression A–D + E; config restored byte-identical | **met** | **P1 ≤5.77e-17, P2 ≤6.52e-8, P3 angles ≤1.01e-5°, P4 9.22–55.08 km, P5 view-term 0.0000° / residuals ≤1.11e-7; orientation 17 restored / 48 divergent, P-d 0.0000°; scene E 13/13, Mars landing 2.270821e-05 AU**. All recorded classes matched. `config.ini` md5 **03fbee59bc3ec506c58f0a3f1e1d73df** in and out (the horizon-mount test edited `viewing_mode` then restored by `cp` of a backup). My change is provably inert on the pipeline (ModularObject uninstantiated; render/mat path untouched) |
| 8 | Trackers | **met** | INTENT §11.60 (new, (a)–(i)); §13.B B9 → DONE; §11.4 item 4 → RESOLVED; §5 new item 19 (RA/DE sibling, out-of-scope); this dispatch row + this §11 section |
| 9 | Committed on master-beta, correct author/co-author, no push | **met** | see commit hash below |

### Findings recorded, not fixed (out of scope)

1. **RA/DE sibling divergence (§5 item 19).** `ModularObject`'s `getRaDeValue`/
   info strings read `Camera::observedPosToRaDe`, which disagrees with old's
   `Body::getRaDeValue` — Moon old `RA 04h57m15s/+26°41'47"` vs new
   `03h44m18s/+14°40'05"` (~18°). A frame/convention delta (J2000-vs-of-date
   candidate), the RA/DE analog of B9, invisible today for the same reason
   (ModularObject uninstantiated). B9 is azimuth only — not touched; needs its
   own probe + single-authority fix.
2. **ModularObject uninstantiated in production** (finding, not a defect to fix
   here) — the D2 bridge is staged but not wired into selection; documented so
   the D2 worker knows the surface is already parity-correct.

### Suspended for Vixy

**None.** Every decision traces to the task spec or a measurement. The one place
the outcome departs from the written row (the constant is `π/2−az`, not `3π−az`)
is a MEASUREMENT superseding a recorded conclusion, exactly what the traceability
rule prescribes (re-verify cached conclusions against source) — not a decision.
The label-order correction in `getShortInfoNavString` is a parity port (old is
the spec, §11.52(b)), not a user-visible-semantics choice.

### What I did NOT verify

- The surface under `render_path = old` (old path unchanged by construction).
- Free-mode az parity (no old-path counterpart — regime stated, not a gap).
- The RA/DE sibling's cleanness (deferred with the out-of-scope finding).
- Live `getSelectedAZ` through a wired ModularObject (it is uninstantiated;
  verified via the real methods in the dump instead).

### Reproduction (verbatim)

    cd /home/claude/spacecrafter
    # probe (pre-fix): revert the ModularObject fix, keep the dump, build, run
    DISPLAY=:2 bash src/experimentalModule/harness/b9_run.sh b9_azconv.py \
        src/experimentalModule/harness/artifacts/b9_prefix    # RESULT FAIL (raw az)
    # fixed binary:
    DISPLAY=:2 bash src/experimentalModule/harness/b9_run.sh b9_azconv.py \
        src/experimentalModule/harness/artifacts/b9_postfix   # RESULT PASS, md5 match
    #   -> altaz_old/altaz_new per body + <file>.navstr caller-visible strings
    # mount-independence (edit viewing_mode=horizon, restore by cp of a backup):
    #   sed 's/= equator/= horizon/' config.ini ; run to b9_horizon ; cp backup back
    # no-regression (A-D + E), FISHEYE, no config edit needed (mat-layer is fov-free):
    DISPLAY=:2 bash src/experimentalModule/harness/b9_run.sh drive_scenes.py <out>
    for f in gen_a gen_b gen_moon gen_mars gen_mars_2; do \
        python3 src/experimentalModule/harness/predict.py /tmp/$f.json | grep P4; done
    python3 src/experimentalModule/harness/orientation_check.py /tmp/gen_moon.json  # 17/48, P-d 0.0000
    DISPLAY=:2 bash src/experimentalModule/harness/b9_run.sh scene_e_spine.py <out> # 13/13

### Hygiene

`config.ini` restored byte-identical [md5 `03fbee59bc3ec506c58f0a3f1e1d73df` in
and out — every run sends settings as commands; the horizon-mount test edited
`viewing_mode` then restored by `cp` of a backup, md5 re-asserted].
`ssystem.ini` and `beta_features.ini` untouched (beta_features absent).
Per-run artifacts `artifacts/b9_{prefix,postfix,horizon}/` gitignored like the
other B rows. `supervised-by.sh` and the root `USER_QUESTIONS*.md` /
`FEATURE_REQUESTS.md` left untracked. No harness task list touched.

## 12. Execution log — B13 (Claude Opus 4.8, 2026-07-22)

**Task**: wave §1 task 9 — reference-change view continuity: preserve the
absolute sky direction across a reference switch and free-mode entry/exit; no
re-centring. Add a view-continuity assertion to scene E. Full record: **INTENT
§11.61**.

**FIRST PARAGRAPH / behavior beyond the row.** The row framed the mechanism as
an open choice ("revive `view` as a smoothing quaternion vs derive from alt/az").
Neither was needed: **§11.19c had already retired the `view` quaternion and made
`recoverParams` the one transition primitive**, and `switchToBody`/`setFreeMode`
have preserved the absolute direction through it since §11.36 (measured here:
free-mode entry/exit absDelta ≤1e-5°; a switchToBody auto-transition 8e-6°). So
B13 is NOT a subsystem revival — it is closing the ONE remaining re-centring path,
`warpToBody` (`set home_planet`), which held (alt,az,heading) in the new frame
(the REJECTED Q2 option "same framing relative to the new body"). User-visible
change is scoped to `set home_planet` / `syncCameraReference` view orientation:
after a reference switch the eye now keeps looking at the same celestial point
instead of the same local alt/az. Two spillovers beyond the row, both stated:
(1) the sky-lock re-expression (B18 handoff, §11.61(e)) also changes locked-switch
behavior; (2) a **source conflict** with B18 §11.58(g) surfaced (§11.61(f)).

**Instruments committed**: `harness/b13_viewcont.py` (new); the permanent
`absFwd` field added to `Camera::dumpTrace` (root-aligned look direction — the
frame-independent continuity observable). **Artifacts**:
`harness/artifacts/b13/` (gitignored).

**Files touched (product)**: `src/experimentalModule/Camera.cpp`
(`warpToBody` compensation; `switchToBody`/`warpToBody` sky-lock re-expression;
`lastAbsFwd` compute + `absFwd` dump), `src/experimentalModule/Camera.hpp`
(`lastAbsFwd` member). **Harness/docs**: `harness/scene_e_spine.py`
(8 view-continuity asserts, 13→21), `harness/b13_viewcont.py`, `harness/README.md`,
`.gitignore` (+`artifacts/b13/`).

### DoD, item by item

| # | Item | State | Evidence |
|---|---|---|---|
| 1 | Current behavior measured FIRST: does the view re-centre on ref switch today? Report absolute-frame delta (deg) | **met** | `harness/b13_viewcont.py`, fresh launch, FISHEYE, anchored Earth 48.85N/2.35E, look_at az60/alt30. **PRE-FIX ref switch Earth→Mars: absDelta 78.60°, altaz 0.00°** — the sky JUMPED, alt/az held frame-relative = the defect. Free-mode entry/exit absDelta ≤1e-5° = already correct (no defect there) |
| 2 | After the change: absDelta≈0 across (a) ref switch, (b) free entry, (c) free exit; alt/az delta = inter-frame rotation (discriminator) | **met** | POST-FIX settled: **ref switch Earth→Mars absDelta ≤5e-6° / altazDelta 78.4–90.2°; free enter/exit ≤1e-5° / altaz 2.03°**. The absDelta and altazDelta SWAPPED exactly (78.60°↔0 → 0↔78.60°) = the discriminator that the ABSOLUTE (not frame-relative) direction was held. `artifacts/b13/{pre,post,settled}_result.json` |
| 3 | Mechanism owns its inverse formula (state representation + transform) | **met** | Representation = the retired-quaternion-free (alt,az,heading) params, transition via `recoverParams` (ZXZ Euler). Inverse: `R = viewRotation()·placementRotation()·comp`, `comp = reference->calculateSwitchCompensation(dst)` (dst-eq→old-ref-eq); `recoverParams(R)` under `reference=dst` reproduces `eye←root` because `mat_new = mat_old·comp` and `comp == accBodyToBodyPos_old·accBodyPosToBody_dst` [Camera.cpp warpToBody; §11.61(a)]. Done directly (one recoverParams), NOT via the freeMode round-trip (corrupts longitude across a ref change) |
| 4 | Scene E gains a DISCRIMINATING view-continuity assertion (fails pre-change, passes after) | **met** | Scene E **13→21 OK** (8 new asserts). Discrimination PROVEN by temporary mutation (recoverParams(R) disabled in warpToBody, rebuilt exit 0): the 3 ref-switch asserts FLIP to **FAIL** (abs held 90.24°>0.05, discrim 0.00°<10 — exact swap), free-mode + switchToBody asserts stay green (localized). Reverted, rebuilt, 21/21 restored |
| 5 | Sky-lock composition stated (B18 handoff) — handled or suspended | **met (handled, measured)** | Both switches re-express `lockedSkyRot·comp` — DERIVED from Q2 (under an active lock the view IS the held orientation, so preserving absolute REQUIRES re-expressing it). Measured: `flag lock_sky_position on` + `set home_planet Mars` → absDelta **0.000006°** (absFwd identical), alt/az drift 78.6°. Alternative (re-lock to new body) noted as a 1-line change if Vixy prefers; Q2's literal reading is implemented [§11.61(e)] |
| 6 | Both entries of the reversible pair | **met** | Ref switch A→B→A (Earth→Mars→Earth): each leg absDelta ≤5e-6°, second from the first's end. Free enter→exit→enter→exit: each ≤1e-5°. Locked A→B→A: 6e-6° each. Traversed in `b13_viewcont.py` and scene E |
| 7 | Build green, mtime advanced | **met** | `make -C build-claude -j$(nproc)` exit **0** (instrument 04:38:04, fix 04:44:51, mutation 05:01:18, restore 05:03:56); final binary mtime **2026-07-22 05:03:56** |
| 8 | No regression A–D + E vs recorded classes; config restored byte-identically | **met** | **P4 observer parity 17.35–50.89 km** (gen_a 24.34, gen_b 47.36, gen_moon 27.34, gen_mars 50.89, gen_mars_2 17.35 = the 9–55 km class), **0 UNMODELED/FAIL across all 5**, **orientation 17 restored / 48 divergent, P-d 0.0000°**. **Scene E 21/21, Mars landing 2.270821e-05 AU** (new count stated: 13→21). `config.ini` md5 **03fbee59bc3ec506c58f0a3f1e1d73df** in and out |
| 9 | Trackers | **met** | INTENT §11.61 (new, (a)–(g)); §13.B B13 → DONE; this dispatch row + this §12 section; `harness/README.md` |
| 10 | Committed on master-beta, correct author/co-author, no push | **met** | see commit hash below |

### Findings recorded, not fixed (out of scope)

1. **SOURCE CONFLICT — B18 §11.58(g) is wrong about the mount (§11.61(f)).**
   §11.58(g): *"Camera::setMount has no caller [grep clean] ... runs ALTAZ."*
   Refuted: `ssystem_factory.cpp:147` calls `setMount(EQUATORIAL)` for the
   shipped `viewing_mode=equator`, present since commit 96a1b896 (§11.19c);
   every B13 dump reads `mount:equatorial`. B18's DELTA conclusion still holds
   (sky-lock is mount-independent); only its "runs ALTAZ" premise is a stale
   grep. Logged, not fixed (out of row).
2. **App rewrites config.ini on shutdown.** After a run with an edited
   `init_fov`, the process persists the running value on exit — the `cp`-restore
   of the pristine backup must run AFTER the app is fully dead (a `cp` racing the
   shutdown write leaves init_fov=340). Same class as the B18/B20 backup+restore.

### Suspended for Vixy

None new. The sky-lock reference-switch composition (item 5) is HANDLED
(Q2-derived, measured), with the alternative "re-lock to new body" noted as a
trivial reversal if Vixy prefers it — flagged, not blocking. B18's other
sky-lock compositions (free-mode+lock, VIEW_HORIZON+lock, select-while-tracking
auto-enable) remain suspended there and are untouched here.

### Reproduction (verbatim)

    make -C build-claude -j$(nproc)                                 # exit 0
    # defect + fix measurement (FISHEYE, any init_fov):
    DISPLAY=:2 ./build-claude/src/spacecrafter &                    # wait port 7805
    python3 src/experimentalModule/harness/b13_viewcont.py \
        $(pwd)/src/experimentalModule/harness/artifacts/b13/post    # ref-switch 0 / free 0
    # scene E (init_fov=340, edit then restore AFTER app dead):
    cp ~/.spacecrafter/config.ini /tmp/cfg.bak
    sed -i 's/^init_fov  *= 180/init_fov                       = 340/' ~/.spacecrafter/config.ini
    DISPLAY=:2 ./build-claude/src/spacecrafter &                    # fresh launch
    python3 src/experimentalModule/harness/scene_e_spine.py         # 21/21 OK
    # A-D regression:
    DISPLAY=:2 ./build-claude/src/spacecrafter &                    # fresh launch
    python3 src/experimentalModule/harness/drive_scenes.py
    for f in gen_a gen_b gen_moon gen_mars gen_mars_2; do \
        python3 src/experimentalModule/harness/predict.py /tmp/$f.json | grep P4; done
    python3 src/experimentalModule/harness/orientation_check.py /tmp/gen_mars.json  # 17/48, P-d 0.0000
    pkill -f build-claude/src/spacecrafter ; sleep 2 ; cp /tmp/cfg.bak ~/.spacecrafter/config.ini
    # discrimination: disable recoverParams(R) in Camera::warpToBody -> scene E ref-switch asserts FAIL

### Hygiene

`config.ini` restored byte-identical [md5 `03fbee59bc3ec506c58f0a3f1e1d73df` in
and out]. The app persists config on shutdown, so the restore `cp` ran after the
process was confirmed dead (`pgrep` empty). `ssystem.ini` / `beta_features.ini`
untouched (beta_features absent). Per-run artifacts `artifacts/b13/` gitignored.
`supervised-by.sh` and the root `USER_QUESTIONS*.md` / `FEATURE_REQUESTS.md`
left untracked. No harness task list touched.

**Commit** (master-beta, not pushed): "Keep the absolute sky direction across a
reference switch (INTENT 11.61, B13)", author Claude Opus 4.8, Co-Authored-By
Claude Fable 5. Single logical commit (fix + instrument + scene-E asserts +
trackers). This hash-recording line is inside that commit, so the authoritative
hash is the post-amend value in `git log` (pre-amend was `03e2b963`).
`supervised-by.sh` left untracked.
