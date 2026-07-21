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
| B26 | Run the two-screenshot observable check for the dual-path default flip (landed, build green, observable unverified) | §11.50(c) | Requires a display: identical ≥2.5 s apart under default, differing under `alternate`. Pure verification — no code expected |
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
