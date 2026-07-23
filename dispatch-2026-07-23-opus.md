# Dispatch view — non-critical, decision-free tasks (2026-07-23, for Claude Opus 4.8 executors)

**Authority note (I2):** this file is a *dispatch view* over `INTENT.md` §13.B — it does
not own any row. On any divergence, §13 wins and the divergence is a staleness bug in
THIS file. Before acting on any row below, **re-read its §13.B row and the recording
entries it cites** (§5.2 class: cached conclusions need re-verification against source,
not recall). Rows carry their `Bxx` id for exact re-matching.

**Orchestration recommendation (this wave's supervisor-model call, made by Claude
Fable 5 per the 2026-07-23 dispatch mandate):** **Claude Fable 5 orchestrates; Claude
Opus 4.8 (xhigh effort, sequential, DoD-gated) executes** — the same split as the
2026-07-21 wave. Reasons, so the choice is challengeable: (i) the ledger's invariants
(supersession-with-record, provenance grammar, the §11.51(d) no-recall red line, DoD
gating) concentrate in the orchestrator — an error THERE corrupts the single authority
and is the costliest class, while execution errors are caught by each row's
discriminating check; (ii) the 2026-07-21 precedent landed 13+ rows under exactly this
split, with supervision catching premise/criterion defects (B26's coin-flip criterion,
B17's false premise) that execution alone reported but did not arbitrate; (iii) token
economics — executors dominate spend, the orchestrator is a small fraction, so the
stronger model buys the most judgment per token where judgment is load-bearing.
Executors run ONE task at a time against the DoD; the orchestrator reviews evidence
against the row (not the executor's summary), propagates residuals into the ledger,
and owns all carve-out boundaries.

**Selection criteria (traceable):**
- *Information:* `INTENT.md` §13 ledger as of harness commit carrying this file
  (code `master-beta @ 14931a18` — the B5 draw-half landing, §11.80).
- *Criterion 1 — no pending decision:* every row here is UNBLOCKED with its deciding
  answer recorded (most from the 2026-07-23 D4–D20 batch, §11.79); rows whose scope
  still contains a Vixy decision or an unanswered tester question are excluded or
  carved out below.
- *Criterion 2 — off the critical/architectural path:* the S4 threading handoff (B1)
  and everything riding it (B2), and new module-family/regime design (B12) are
  excluded. The §6.9 content-migration remainder is admitted only as ONE tightly
  carved pilot (oort), because §11.80 gave it a live draw surface to verify against.

**Standing constraints for the executor:** repo rules in `<programmation-principles>`
apply (I1–I6). Every row names its own discriminating check — a green build is not
coverage; if a check needs a display, say so instead of substituting a weaker check.
Data values NEVER from recall — cited fetch only (§11.51(d)); loaded data =
`~/.spacecrafter/ssystem.ini` (ISO-8859; use /usr/bin/grep or Read — the Bash-tool
grep wrapper silently excludes untracked ini files). Fresh-launch precondition for
measurements; config/ssystem md5 in==out asserted. §2.0 D11: every cost claim uses the
1 ms/frame denominator. §2.0 D12: any ACTING default you introduce is logged. Do not
improvise answers to carved-out residuals — stop at the boundary and record the stop.
Harness facts for anything galactic: INTENT §11.80 + harness/README.md B5 section
(executor altitude ladder, the B10(c) datum trap, descend-based placement,
phase-toggle floor discipline).

---

## 1. Dispatchable now (self-contained, decision-free)

| Row | Task | Spec / recorded at | Notes for execution |
|---|---|---|---|
| B22 | Live cross-fade completion: band-CROSSING reversible pair (hysteresis at the boundary) + live-ms cost at the band | §11.64, §11.80 | The live surface exists since §11.80 (b5 "uniband" leg renders interior+dot at refDist 1340 AU through the universe executor). Drive px across [T, T+B) both directions (descend/ascend around ~1054–1581 AU at fov 340), assert monotone complementary alphas on-screen + no hysteresis; measure live-ms in-band vs out (denominator 1 ms, D11). Constants stay A15 (tester) — measure, do NOT tune |
| B20 | Anchored-galactic pixel verification (the draw half of the A13-ratified behavior) | §11.59, §11.80 | VERIFY-ONLY. Anchored (not free-mode) at galactic altitude: the solar-system-from-afar view must DRAW (resolved interior / dot per px). Reuse b5 legs with `camera action free_mode state off` + the §11.59 anchor-kept scene. R13 anchor-per-mode memory stays EXCLUDED (escalation-policy, §6.9/B31 inheritor) |
| B14-W0 | W0 (prime-meridian) conversion + write — D4 answered: option (1) | §11.79(a), §11.69(e), §11.67 | Extend the ONE frame authority (`resolveRotationFrame`, B28) to convert W0 like the pole (IAU ICRF-equator-node → the file's ecliptic-node referential), then write the FETCHED IAU W0 values recorded at §11.69(e) — never recall (§11.51(d)). Discriminator: rotation phase of a corrected moon vs its IAU formula evaluation at 2 dates; bit-identical for every body without the new key. BOTH files (shipped committed; loaded `.b14bak` discipline, ISO-8859 preserved) |
| B14-sat6 | Widen B14: the 6 pole-bearing non-cluster Saturn moons (Telesto, Pandora, Janus, Helene, Epimetheus, Prometheus — the 63–121° errors) | §11.75(b), §11.69 | Same 3-source cited-fetch discipline (pck00011 + pck00010 + Archinal text, agreement required); `rot_pole_ra/de` + `rot_frame=absolute_pole`; commutator A/B before/after (only these 6 move); STOP on any body whose pole is absent from a source (record, don't improvise). Hyperion stays unwritten + inline comment (§11.75(b)) |
| B10-cmd | Command follow-through: `radius datum` / `radius ground` runtime commands | §11.79(e) D9key, §11.71 | Command word order matches the data keys' nouns (D9key: data keys STAY `datum_radius`/`ground_radius`). Both §2(c) channels exercised + the 11.54(j) swallow-guard (gdb breakpoint hit-count 1:1 with commands). Numeric assert: the live scalars change and the B10 discriminating cases (enterable/clearance) respond |
| B10-datum0 | System-node `datum=ground=0` class default (any ModularSystem, user-overridable) | §11.75(a), §11.80 | Decided [vixy]. Implementation + THE discriminating case §11.80 hands you: free-mode `moveto altitude X` at a MilkyWay reference must land at X (today 3.2e9+X — measured); the b5 driver's descend workaround becomes optional (keep the harness green both ways). Watch scene E: the ladder's altitude arithmetic must stay green (its rungs are issued from Earth/Universe refs — verify, don't assume) |
| B17 | view_offset port, shape (1): reproduce old exactly | §11.79(c) D6, §11.63 | Bake `xrotation(view_offset · halfFov)`-class fov-coupled rotation into the Camera view stage (old mechanism: navigator.cpp:159 yrotation(look, fixed) + :309 xrotation(eye, fov/2-coupled) — re-verify at source, §11.63). Both channels (config `[navigation] view_offset` + `set zoom_offset`) already measured live. Discriminator: byte-parity-class screen A/B vs old on flat fisheye at 2–3 offsets incl. 0; new-path pixels shift, old baseline untouched |
| B24-D16 | `type=` respell: replaces `declare=`+`module=` in the composed format | §11.79(j), §11.78 | One key = declaration kind + family (`type=BODY` for ModularBody; `type=MESH/ATMOSPHERE/CUSTOM/...` for modules). Parser + B25 emitter + b24 harness edits together (format is parsing-deep only, §11.52(c)). The [derived] module=-retires reading carries a veto point (§11.79(j)) — implement as stated, note the veto point in the DoD evidence. Corpus gate: b24_equivalence green + discriminating post-respell |
| B24-att | Grounded ATTITUDE default: surface-locked; acting default LOGGED | §11.79(l) D18, §2.0 D12, §11.78(f) | Grounded bodies' mesh attitude defaults to the surface frame (static on the ground); explicit rot keys override. The legacy 24 h `rot_periode` default STAYS for backward compat (D9) and now LOGS when it acts (D12 + §2(f): name what fired, why, and how to override). Gate: b24_compose rover attitude static vs an explicit-rot control |
| B25-emit | B27 step-3 capability-key emission + consume co-delivery | §11.79(e) D10key, §11.73(b)(g), §11.78(f) | Emit `sidereal_time=earth_apparent` + `shadow_color` on Earth in the composed twin; consume at the §11.73(b) file:line sites SAME delivery (co-delivery constraint §11.73(g): consume without emit loses Earth sidereal time — gate on it). A3/A4 emit nothing (retire/delete per map). Tier B `type`-retirement is FORMAT-SCOPED (D14): new format only, legacy keeps `type`-driven |
| B32 | Spin-phase freshness: recompute-at-use under the §11.76 barrier | §11.79(n) D20, §5.24 | Mechanism DECIDED (option 2): the tick may genuinely freeze; a use (draw/show, dump, script fetch, selection, save) triggers recompute (+4 iterations for previously-frozen, §11.76). Discriminator: two identical fresh launches agree on `axisRot` for the ~20 pole-bearing moons (today up to 8.7e-2 rad apart); then LIFT the b24_equivalence `axisRot` exclusion (the row's own regression bonus). D11 note: the freeze is the point — no new per-frame cost |
| B30 | Frozen-scene micro-instability: test the stated hypothesis | §11.53(e) | Hypothesis TO TEST, not assume: the per-frame `environment->update(..., delta_time/1000.f, ...)` tone/adaptation chain. Pin inputs (freeze the adaptation input or force a constant) and re-measure the ≤31/255 stepping; attribute or refute. No product behavior change without a measured attribution |
| B7 | Shutdown-segfault active hunt (scoped §11.74(f)) | §11.15d, §11.74 | High-cycle teardown harness under gdb (`b7_probe.gdb` armed) or ASan/TSan build; old render path toggled (uncovered teardown candidate). Exit-code channel is the positive instrument; core files DEAD (ulimit0 + apport). Sample-size hypothesis: budget N ≥ 50 teardowns before concluding |

Suggested order: B22 + B20 (verification-first, smallest, exercise the fresh §11.80
surface while it is new) → B10-cmd → B10-datum0 → B14-W0 → B14-sat6 → B24-D16 →
B24-att → B25-emit → B17 → B32 → B30/B7 opportunistic.

## 2. Partially dispatchable (explicit carve-outs — stop at the boundary)

| Row | Dispatchable part | Carved out (NOT yours) | Recorded |
|---|---|---|---|
| B5 | **Oort content-migration PILOT**: `oort` as a modular body at the SolarSystem floor (the [vixy] mapping rule §6.9), reproducing the old altitude-gated draw (solarSystemModule.cpp:154/196) via G4 floor gating — the pattern-prover for the whole §6.9 content migration, now verifiable against the live draw surface (§11.80) | Module-FAMILY generalization (VOLUMETRIC slot vs cluster traits — prove the pilot with the simplest family that renders, flag the choice); dso3d/tully/starNav migration; ojmMgr dissolution; per-floor efficiency verification; the starNav-Sol/proxy-dot double-representation | §6.9, §11.80 |
| B24 | Screen-layer grounded scene (drawn rover occluding against its parent + its shadow — H4(b)/B3's first observable) | H4 (a) SECONDARY ladder and (c) RGBA8 SELF_COLOR stay B3's; any new shadow-pipeline capability beyond the existing jobs-as-data structure | §11.78, shadow-paths H4, §11.80 |

## 3. Excluded (reason stated so the exclusion is challengeable)

- **B1/S4 + B2** — the threading/handoff architectural line, Vixy-paced (EntityCore
  Taskable authority); §8.5 parameters are plan-time. Riding rows (D4 surface
  streaming, RING asteroid, INSTANCED consumer) wait with it.
- **B31** — Vixy-DEFERRED (2026-07-22) and mandated to get a design pass before
  dispatch (state inventory, save format/trigger, A29-class identity key). NOT
  dispatched blind, per the row.
- **B12** — new near-surface star family = regime + module-family design
  (architecture-adjacent), not a decision-free execution task.
- **B10 (a)** — the anti-stuck floor VALUE is Vixy's feel-test (D12 §11.79(f)).
- **B18 residuals** — D15 is the batch's only unanswered item (§11.79(i)).
- **B20/R13** — anchor-per-mode memory: escalation-policy/§6.9 design, named
  inheritor is the §6.9/B31 implementer.
- **B21 residuals** — step feel (Vixy), keyboard-descent unification (I2 pass, small
  but touches UI routing conventions — batch with the next command-surface sweep).
- **Every §13.A row** — Vixy/tester territory by definition; note A15's re-ask is
  now ACTIONABLE (the fade renders live, §11.80) — that is a question to SEND, not
  a task to execute.
