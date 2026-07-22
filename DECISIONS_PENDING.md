# Pending decisions — 2026-07-21 Opus dispatch wave residuals

**Status:** compiled 2026-07-22 by Claude Fable 5 (supervising the wave), at Vixy's request.
**Update 2026-07-22 (Fable):** D1–D3 answered by Vixy inline below; **propagated → INTENT.md §11.75**. D1 RESOLVED (option 2 generalized to any ModularSystem; B10 (c) closed, implementation scheduled). D2 RESOLVED by criterion (→ §2.0 D8; B14 widened +6 Saturn moons, Hyperion = inline-comment). D3 **PARTIAL — the answer sentence is incomplete** ("…which hold more recent datas,"); completion asked, incl. the new B14 propagation hazard (§11.75(c)).
**Authority note (I2):** this is a *view* over `INTENT.md` (same folder) §11.53–§11.75 and §13 — it owns no decision. Each item carries its `§ref`; on any divergence the INTENT entry wins and the divergence here is a staleness bug. Nothing here blocks a landed row; every listed row is committed and green. A `[Fable rec]` is my derived recommendation with its argument, never a decision made.

**How to answer:** inline, one line per item is enough; the `Dxx` key is the stable match-back. "No strong opinion" is useful — it says the choice is safe on engineering grounds (I'll pick the `[Fable rec]` and record it). A structural counter-argument overrides any rec.

---

## 1. Load-bearing (changes user-visible behavior or shipped data)

**D1 — MilkyWay center-relative altitude intent** `[B10, §11.71 suspended (c)]`
The deleted `altitudeRelativeToRadius` flag's ONE authored non-default was three factory nodes (Universe/MilkyWay/per-system) setting it `false` = "measure altitude from the galactic centre." The ctor was inert, so the runtime never honored it; B10 migrated to `datum=ground=radius` = **bit-identical to today's runtime**. Question: keep runtime-parity, or honor the authored intent (`datum=ground=0` for MilkyWay = a galaxy-scale free-mode navigation change — altitude measured from centre, not the 3.2e9 AU shell)?
Options: (1) keep runtime-parity (today's behavior, safest); (2) honor authored intent (MilkyWay altitude from centre — only MilkyWay bites; user-visible in galactic free-flight).
`[Fable rec]` (1) unless you recall wanting galaxy-from-centre navigation — the intent was authored but never experienced, so "restore" it is really "introduce" it; introducing user-visible behavior no one has seen wants your eye, not a default.
(2) - Honor the authored intent for the default, with datum=ground=0, same for any ModularSystem, because those represent a system and as such, it is intended to go inside them. As a default, it can be trivially overrided by user-provided value if needed.
→ **propagated §11.75(a); B10 suspended (c) RESOLVED** — class default off the node's system nature; implementation scheduled with its discriminating harness case.

**D2 — B14 scope: widen beyond the 15.5-cluster?** `[B14, §11.68/§11.69 suspended]`
B14-land corrected 14 of the 28-body `rot_obliquity=15.5` cluster. Adjacent garbage that is OUT of the cluster's scope:
- **(a)** 7 non-cluster garbage-tilt Saturn moons (Telesto, Pandora, Janus, Helene, Epimetheus, Prometheus, +1) — these carry the *large* 63–121° commutator angles (the cluster does not) and DO have IAU poles. Widen B14 to fix them?
- **(b)** 6 loaded-only Jupiter moons (Adrastea, Ananke, Leda, Lysithea, Metis, Sinope) — in `~/.spacecrafter/ssystem.ini` but NOT in the shipped `data/default_ssystem.ini`. Metis/Adrastea have poles; the other 4 are irregular (no defined pole). Should the shipped default gain these 6, and should the 2 with poles be corrected?
- **(c)** Hyperion — chaotic tumbler, **no defined pole**; cannot take a static pole. Leave its garbage tilt, or set a nominal/mean value with a comment, or exclude explicitly?
`[Fable rec]` (a) yes — they're the same garbage class and the more visually wrong ones; (c) leave + comment (a static pole is a lie for a chaotic tumbler); (b) is really a shipped-vs-loaded reconciliation question — see D3.
Everything must be physically exact, or visually identical to the physically identical. Which is why multi-frame iterative position calculation is used (visually identical for human vision) and why hidden bodies (or bodies neither shown nor calculated because their halo can't be distinguished from the main body anyway) aren't updated. It save processing time without compromising visual fidelity, enabling to easily handle tens of thousands of bodies at 160Hz on this hardware (with the position and visibility update being single-threaded).
→ **propagated §11.75(b) + extracted as domain constraint §2.0 D8**. Derived applications (veto if wrong): B14 WIDENED +6 pole-bearing Saturn moons; Hyperion stays unwritten + inline data comment (§11.51(d) outranks a nominal); (b) folds into D3. One surface tension flagged at D8: "hidden aren't updated" vs B19/§11.54's tester-mandated position continuity — reconciled as exempt-from-full-work, not position-frozen.

**D3 — Shipped-vs-loaded data-file divergence** `[B14-prep, §11.68; B28, §11.67]`
The app loads `~/.spacecrafter/ssystem.ini` (34 cluster bodies, untracked) but ships `data/default_ssystem.ini` (28, tracked). They diverge by 6 Jupiter moons. This is a standing hazard beyond B14: fixes to the tracked file don't reach a user whose `~/.spacecrafter/` copy diverged, and vice-versa. Question: is the loaded file supposed to be a hand-maintained superset, or should install refresh it from the shipped default? (Decides whether data corrections need to touch both, and whether B31's write-back should reconcile them.)
`[Fable rec]` this one I can't rec without knowing your install/authoring workflow — it's a genuine you-only fact.
There is another repository, spacecrafter-data, which hold more recent datas, 
→ **PARTIAL — the sentence above is incomplete; propagated as-is §11.75(c), completion asked**: (i) authority order among the THREE sources (spacecrafter-data / shipped `data/default_ssystem.ini` / loaded `~/.spacecrafter/ssystem.ini`); (ii) does install/refresh flow spacecrafter-data → shipped → home; (iii) must B14's landed poles (and the D2 widening) land in spacecrafter-data too — **if it is upstream authority, a future refresh silently reverts the 14 cited IAU poles**; (iv) do the 6 loaded-only Jupiter moons dissolve into a sync from spacecrafter-data?

**D4 — B14 W0 (prime-meridian) frame conversion** `[B14-land, §11.69 suspended]`
B14-land wrote the poles (loader converts them) but NOT `rot_rotation_offset` (W0): IAU W0 references the meridian from the ICRF-equator node, the file's W0 references the ecliptic node (confirmed: Mars file 136.005 vs IAU 176.630). Writing raw IAU W0 = correct number, wrong frame ("looks right in the file"). Fetched W0 values are recorded (§11.69(e)), not written. Question: (1) extend the loader to convert W0 like it converts the pole (one authority), then write IAU W0; or (2) leave the meridian as-is (the moons' rotation phase stays approximate; pole/orientation is corrected, which is what an audience sees)?
`[Fable rec]` (1) — the same frame-declaration machinery B28 built for the pole should own W0; a half-corrected body (right pole, wrong meridian) is the §11.49(e) hazard on the meridian axis. But it's a small loader extension, schedulable, not urgent.

**D5 — B14 derived-vs-verbatim pole provenance** `[B14-land, §11.69]`
Iapetus's pole is a raw verbatim IAU constant. The 13 inner-regular moons' poles are **derived-from-fetched** — J2000 evaluations of the 3-source-agreed IAU *formula* (time-dependent pole), not a raw table constant. Reproducible arithmetic on fetched coefficients (hand-verified on 2), not confabulation — but a different provenance class. Ratify that derived-from-fetched-formula counts as "from the report," or require raw-constant only (would drop the 13 time-dependent ones)?
`[Fable rec]` ratify derived-from-fetched — the formula IS the report's content for time-dependent poles; the J2000 evaluation is arithmetic, fully traceable. Requiring raw constants would discard correct data for a provenance formality.

**D6 — B17 view_offset port option** `[B17, §11.63 suspended]`
The row's "projection-space offset" premise was false — old `view_offset` is a fov-coupled *view rotation* (re-aims the sky). R11 confirmed scripts change it during shows (so both channels are needed, already built), but did NOT resolve the port shape. Options: (1) reproduce old exactly (bake the fov-coupled rotation into the Camera — byte-parity on flat fisheye, but a rotation, contradicting the original "projection-space" goal); (2) true projection-space (new NDC-offset uniform in the shared GPU shaders — matches the goal, diverges from old at the dome edge, new hook); (3) retire on the new path, keep `render_path=old` as the tilted-dome fallback. Deciding fact: does the tilted-dome operator want the sky **re-aimed** (1) or the image **slid without re-aim** (2)?
`[Fable rec]` (1) — parity with what your shows use today is the safer default, and the "projection-space" goal was my derivation from a wrong premise, not your requirement. But this is genuinely a your-shows question.

**D7 — B22 cross-fade cost ratification** `[B22, §11.64 suspended]`
"If not too costly" was the decision input; measured cost = **~22.6 ns/band-frame = 0.00014% of a 16.7 ms frame, 0 added GPU draw calls**. Ship the cross-fade? (The threshold/band constants stay A15 — a tester judges them once the fade is live behind §6.9.)
`[Fable rec]` ship — the cost is three orders of magnitude below "too costly"; the only reason it isn't already ratified is that "not too costly" was explicitly reserved as your call.

---

## 2. Naming / spelling (I2 consistency — cheap, but land before the keys are authored)

**D8 — B28 `rot_frame` key** `[B28, §11.67 suspended]` — spelling `rot_frame`, values `absolute_pole | parent_relative`. Sign off, or re-spell? `[Fable rec]` keep — matches the `rot_*` prefix; the two values name the two existing populations.

**D9 — B10 data-key vs command word order** `[B10, §11.71 suspended (b)]` — data keys are `datum_radius`/`ground_radius`; the command was requested `radius datum`/`radius ground`. Re-spell the data keys to match the command word order, or keep the `<noun>_radius` form? `[Fable rec]` keep data keys as `datum_radius`/`ground_radius` (reads as a property); make the command match the data, not vice-versa.

**D10 — B27 capability-key spellings** `[B27, §11.73 suspended]` — `sidereal_time={generic|earth_apparent}`, `surface_model`, `trail_length`. Sign off the spellings (B25 emits them). `[Fable rec]` accept as-is; they're descriptive and §2(a2)-conformant.

**D11 — B21 descend command spelling** `[B21, §11.72 suspended]` — `camera action descend coef <c>`. Keep? `[Fable rec]` keep (consistent with `camera action <verb>`).

---

## 3. Value / mechanism sign-offs

**D12 — B10 anti-stuck floor VALUE** `[B10, §11.71 suspended (a)]` — the mechanism (`proximityFactor(escaping)` floored outward) is wired; the value `ANTISTUCK_ESCAPE_FLOOR = 1e-6`·radius (~6.4 m on Earth) is a **placeholder**, deliberately NOT `MIN_MOVEMENT_SPEED`'s wrong-unit 0.125. Question: is ~6.4 m-scale right, and should it be one shared constant with `MIN_MOVEMENT_SPEED` or two? `[Fable rec]` two constants (different units/roles — escape epsilon vs traversal speed); the value wants a quick feel-test at a surface, which is yours.

**D13 — B28 frame-aware accumulation generalization** `[B28, §11.67 suspended]` — the ancestor-tilt loop guarded by `absoluteTiltFrame` is required for `absolute_pole` to be correct on non-system-centered parents (moons); inert on all current data, and B14-prep's harness proved it correct on Iapetus. Sign off, or veto for a different consumption mechanism? `[Fable rec]` sign off — it's proven correct and inert until used; the alternative (loader-side decompose) is geometrically impossible (a 2-DOF tilt can't hold a general 3-DOF parent⁻¹·absolute rotation).

**D14 — B27 `type`-as-identity retirement** `[B27, §11.73 suspended, Tier B]` — whether to retire `type`-string identity tests (e.g. `type=="Moon"`) into explicit capability keys (`light_source`, `shadow_exempt`). Meets R10 today via `type`; the question is whether to go further. `[Fable rec]` defer with B24/B25 — it's an authoring-path design choice, not a wave residual.

**D15 — B18 sky-lock compositions (4)** `[B18, §11.58 suspended]` — each is a Camera-orientation composition old made implicitly: (a) VIEW_HORIZON mount + lock roll (direction-only hold vs whole-orientation — coincident for the shipped equatorial mount); (b) free-mode + lock; (c) select-while-tracking auto-enable; (d) a startup config default old never had. All dormant today. `[Fable rec]` (a) decide when a non-equatorial mount is wired; (b)(c) decide with the tracking/free-mode composition work; (d) no default (old had none). i.e. none is urgent — but you may want (c) (auto-enable on select-while-tracking) if your shows lock while tracking.

---

## 4. Work items to schedule (not decisions — flagging so they're not lost)

- **B31** exhaustive state save + persistent-body write-back `[§11.66(d)]` — **DEFERRED by you (2026-07-22)**; stays mandated, needs a design pass before dispatch (state inventory, save format/trigger, cross-session persistent-body identity key — A29 name-key hazard applies).
- **B20/R13 anchor-per-mode memory** `[§11.66(e), §11.70(k)]` — you answered the intent (hold anchor per mode, restore on return); implementation is a §6.9/escalation-policy change (the old-executor→new-camera `changeSystem→switchToBody` coupling, ungated by freeMode). Schedule with §6.9/B5.
- **B7 §11.15d active hunt** `[§11.74(f)]` — 1 true fire this wave (B23 teardown race); ledger open. Scoped-not-started: high-cycle teardown harness (`b7_probe.gdb` ready) under gdb or ASan/TSan, old render path toggled (uncovered teardown candidate).
- **B10 multAlt unwired** `[§11.71]` — `Camera::multAlt` has 0 callers (keyboard altitude not dual-routed); B21 routed `descend()` for its need, but the keyboard path is a separate I2 unification for later.
- **§6.9 executor dissolution (B5)** — gates live-pixel verification of B20 (anchored galactic draw) and B22 (cross-fade); both are mechanism-complete, pixel-unverified until it lands.
