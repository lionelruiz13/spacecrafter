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
| B16 | Expose `reloadSystem` as a command; keep current state (camera + date), no reset | §11.36, §11.45(d), §11.48(a) | Both channels per §2(c) where applicable |
| B17 | Port `view_offset` / `zoom_offset` as a Camera parametrization, config + command channels | §11.19a, §11.45(d), §11.48(a) | In use for tilted dome geometry ⇒ it is a projection-space offset — must NOT be re-derived as a camera rotation |
| B18 | Port `flag_lock_equ_pos` (equatorial-mount sky-lock) | §11.19c, §11.48(a) | The "unexercised legacy feature" premise was wrong — it is exercised, just not by our harness |
| B19 | Hidden-body ticking: current behavior (hidden ⇒ keeps updating) is ratified — lock it with a regression assert | §11.15b, §11.36, §11.48(a) | Small; exists so a later perf optimisation cannot silently freeze hidden bodies |
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
