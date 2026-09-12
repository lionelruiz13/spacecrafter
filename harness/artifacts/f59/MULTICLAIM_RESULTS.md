# F59 scope 3 — the multi-claim walk: results

Rule and corpus: `MULTICLAIM_BOUNDARY.md` (committed first, `8601e09`). **23 pairs walked
exhaustively, no sampling.** Claims were enumerated **from the superseder, independently of
the marker** — every superseder file grepped for its target's id and every mentioning line
read — precisely so the count could exceed what the marker records.

**RESULT: 49 corrected claims enumerated · 49 COVERED · 0 PARTIAL · 0 UNCOVERED.**

## A — the twin's MARKED pairs (16)

| pair | claims the superseder corrects | verdict |
|---|---|---|
| §5.2 ← §11.156 | **0** — a derived-view refresh, *"nothing advanced"* | COMPLETE (vacuous) |
| §5.26 ← §11.108(c) | 3 — the recorded observable does not reproduce (129 px vs 11583) · the 11583 is **re-attributed** to §11.92(d)'s coupling (11 082 px, 158.79 predicted vs 158.93 measured) · the row is NOT closed, it stays open on the reference-switch half | COMPLETE (all three at `INTENT/5.26.md:8`, mirrored at the stub) |
| §5.27 ← §11.101(f) | 2 — the mechanism is mis-stated (a load-time latch, not absence-of-scaling) · D21's option set is therefore incomplete | COMPLETE |
| §5.27 ← §11.149 | 2 — the correction had landed on the stub alone and never on the authority (the §5.2 class) · D21 answered ⇒ the behaviour half unblocks | COMPLETE |
| §5.52 ← §11.127(a) | 3 — candidate 2 REFUTED · candidate 1 CONFIRMED and the row closed · the row's *"the disc is bright in both"* reading was the additive halo, not the disc | COMPLETE |
| §5.56 ← §11.163(d) | 4 — the INFERENCE refuted (`BasicMeshLoader` has a user-declared dtor) · one citation corrected (`modules.cpp:72-97` → `:45-68`) · the PREMISE re-verified 23/23 · **the fix is SMALLER than the row implies** | COMPLETE |
| §5.59 ← §11.126(g) | 2 — the fix was BUILT, MEASURED and WITHDRAWN (it converts the hang into a crash, I7 as-if unmet) · **the fork is SUSPENDED FOR VIXY** and is why the row is still open | COMPLETE |
| §5.62 ← §11.134 | 2 — the `active.lock` statement REFUTED · the owed isolation measured, the shift on neither binary | COMPLETE |
| §5.63 ← §11.129(b) | 1 — the row's own characterisation corrected (the residual is not photometry-without-displacement) | COMPLETE |
| §5.63 ← §11.130(h) | 3 — the milky-way inference retracted · a rotation fit excluded a ROLL, not a change of direction · a sky-LOCKED scene's A/A floor is not 0 (26–51 px) | COMPLETE — the row names all three |
| §5.80 ← §11.144(b) | 2 — the *"used by `moveTo`'s free branch, `descend`, `moveEyeRel` and `setFreeMode(false)` alike"* text corrected · the row's minted numbers refined (124.8° / 11 300 km → 124.6797° / 11 298.6 km) | COMPLETE |
| §5.80 ← §11.153(b) | 1 — the site count corrected, FOUR not three (`getPlace()` is the fourth) | COMPLETE |
| §5.89 ← §11.160 | 2 — the reach claim CORRECTED AND WIDENED (not startup-only: `Core::init` runs twice and is mid-session reachable) · the row's citation has DRIFTED `core.cpp:487-492` → `:526-535`, assert at `:533` | COMPLETE |
| §5.104 ← §11.155 | 2 — FIXED AND CLOSED with the row's own measurement both ways · the owed wider-seam probe STAYS OPEN on the row | COMPLETE |
| §5.110 ← §11.166 | 1 — one of the row's own citations corrected (`ssystem_factory.hpp:592/597` → `:580`/`:586`) | COMPLETE |
| §5.113 ← §11.170(i) | 1 — the trailing *"also undetermined"* clause SUPERSEDED (census now 13 guarded / 7 reachable / 0 undetermined) | COMPLETE — struck-not-deleted at the row |

## A′ — §5.32's two markers: the shape (ii) describes, caught and repaired at the node

| pair | claims | verdict |
|---|---|---|
| §5.32 ← §11.108(e) | **0 corrections** — it ADDS a consumer the row did not list (`Camera::descend`, ten descents in one frame compounding linearly). An enrichment, carried at the row anyway. | COMPLETE (vacuous) |
| §5.32 ← §11.128(a)(a2) | 2 — **the first marker's own *"Same fix as the row's"* clause is CORRECTED** (a once-per-frame fix cannot change what ten uses inside ONE frame do) · the row's `setBoundToSurface` longitude conversion has NO runtime channel, so that pair is not traversable from any §2(c) channel | COMPLETE |

**This is the corpus's positive control and it was not constructed for the purpose.**
§11.108(e)'s *"same fix"* claim was **relayed inside §5.32's own first marker**, left standing
by it, and corrected by the **second** marker at the same node three sessions later — the exact
multi-claim shape §11.165(h)(3)(ii) names, occurring in the live ledger and repaired where
§11.113(p) says it must be. A walk that could not see this would be worthless; this one sees
it, in the corpus, without being told.

## B — the five §11↔§11 markers F49 itself placed (5)

| pair | claims | verdict |
|---|---|---|
| §11.122(i) ← §11.125(i) | 3 — Theory A refuted by its own prediction (ARM P 3/54 vs ARM R 1/54, p = 0.31; the pre-fix binary, 0 HUNG in 122 here, hung) · replaced by §5.59's mechanism + rate (4/108 = 3.7 %) · **explicitly NOT re-attributed**: why those two, ARM C unrun | COMPLETE |
| §11.101(f) ← §11.152 | 3 — the timing prediction refuted at BOTH ends (startup latches 1; `reloadCurrentSystem` re-applies none ⇒ §5.104) · the LATCH mechanism confirmed and the latch now dead · (iii)'s D8 leak closed, with the one channel that could not be made invariant named and left open | COMPLETE (a three-bullet marker) |
| §11.108(e) ← §11.128(a2) | 3 — *"same fix as the row's"* corrected (imprecise, not wrong about the class) · the entry's measurement untouched and reproduced to the digit four sessions later · the FAR-branch residual named, same class, on §5.32's residual | COMPLETE |
| §11.98(c) ← §11.116(b) | 5 — (ii) zoom REFUTED · (i) still live and **never tested in the observation's own configuration** · (iii) still live · the observation is configuration-silent · the **SHADOW** qualifier the paraphrase dropped, restored | COMPLETE — the largest marker in the corpus, and the one that restores a dropped word |
| §11.156(g) ← §11.165(b) | 2 — row 4's DIRECTION is inverted · the class's extent is FOUR, not five (with the *lower bound* caveat explicitly untouched) | COMPLETE |

## The criterion-integrity instance of this walk — kept, because it is the walk's own lesson

Two candidate gaps were raised and both were **false, and both false for the same reason my
probes were case-blind in exactly the way I was auditing the instrument for**:

- `§5.56` — probe `smaller` reported ABSENT; the row says **`SMALLER`**. Present, covered.
- `§5.59` — probes `[Ss]uspended` / `[Vv]ixy` reported ABSENT; the row says **`THE FORK,
  SUSPENDED FOR VIXY (§11.126(l))`**. Present, covered.

Both were caught by **reading the row** rather than trusting the probe — which is the rule
`METHOD.md` §2 states (*"the `mech` column is an input to adjudication, never a verdict"*),
applied to my own instruments and not only to the frozen one. Had either stood, this
document would have reported two arrears that do not exist. Recorded rather than tidied:
the audit's finding M1 is that a case-sensitive matcher misses real markers, and the audit
nearly published two instances of its own finding as fact.

## Sensitivity of the count, stated

"49 claims" is a judgment at one grain: a claim is distinct when the target could have been
right about one and wrong about the other (`MULTICLAIM_BOUNDARY.md` §1). A finer grain would
count more claims and could not lower the coverage, since every enumerated claim is covered;
a coarser one would count fewer. The **verdict** (0 uncovered) is grain-independent as long
as the enumeration comes from the superseder rather than from the marker, which is the
property this walk was built around.

## The remainder, unchanged

Every other marked pair in the §11↔§11 corpus is not walked — roughly 40–55 pairs, none
§5-targeted (`MULTICLAIM_BOUNDARY.md`). That is the natural corpus for a later pass.
