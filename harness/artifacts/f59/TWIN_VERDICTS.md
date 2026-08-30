# F59 scope 2 — the §5-side twin: verdict table

Universe and method: `METHOD.md` (committed first, commit `3f6c5fc`). Every pair below was
adjudicated **by reading both ends**; the enumerator's mechanical `mech` column was an
input, never the verdict. Tree: harness `f345b33`, code `d6aec251`, 2026-08-30.

**RESULT: 44 primary pairs — MARKED 15 · FALSE-POSITIVE 15 · OFF-AXIS 8 · MINT-ROUTE 6 ·
AMBIGUOUS 0 · UNMARKED-REAL 0.** All 13 of §11.165(c)'s third-bucket pairs are present and
are marked `[F49-13]` below. **No §5-node annotation is owed by this audit.**

A zero cannot be trusted on its own, so what could have made it non-zero is stated at the
foot (§ "why this zero can fail").

| # | pair | verdict | evidence / argument |
|---|---|---|---|
| 1 | §11.149 → §5.2 | FALSE-POSITIVE | `INTENT.md:885`. *"§5.2 class"* is a CLASS LABEL (the authority-inversion class is named after that row); the `REFUTED` in the window belongs to §11.101(f)'s claim at §5.27. Nothing about §5.2 is asserted. |
| 2 | §11.156 → §5.2 | **MARKED** | `INTENT.md:265` carries `[STUB REFRESHED 2026-08-29 — F42/§11.156 … Transcribed from INTENT/5.2.md; nothing advanced.]`. The event is a derived-view repair, not a claim correction, and the pointer is there anyway. **Instrument scores `no-marker`** — `REFRESHED` is outside `MARKRE`, and the span sits >160 chars from any uppercase keyword (finding M1). |
| 3 | §11.6 → §5.2 | FALSE-POSITIVE | `INTENT/11.6.md:3`. **Direction inverted**: the line IS the back-marker at §11.6 (*"CORRECTED 2026-07-20 — DO NOT APPLY AS STAGED (§5.2)"*) — §5.2 is the superseder, §11.6 the superseded node, marker correctly placed. §11.165(b)'s family on the §5 axis. `[F49-13]` |
| 4 | §13 row → §5.2 | OFF-AXIS | `INTENT.md:995`, a §13 row. Source is not a §11 entry. The substance is carried at the row itself: *"The residual observation is REFUTED + RESOLVED (2026-07-20)"*. |
| 5 | §11.108 → §5.26 | **MARKED** | `INTENT.md:284` *"SWEPT 2026-07-25 (F4, §11.108(c)) — THE ROW'S OWN OBSERVABLE IS REFUTED AT SOURCE-OF-TRUTH"*; `INTENT/5.26.md:4` `[OBSERVABLE SUPERSEDED 2026-07-25 by F4 §11.108(c) …]` and `:8`. **Both homes.** |
| 6 | §11.165 → §5.26 | FALSE-POSITIVE | `INTENT/11.165.md:43` — §11.165(c)'s own residual partition quoting the `§11.108 → §11.55` co-occurrence. Catalogue (N5 by §11.156(b)). |
| 7 | §11.101 → §5.27 | **MARKED** | `INTENT.md:286` `[MECHANISM CORRECTED 2026-07-24, §11.101(f) …]`; `INTENT/5.27.md:5` `[SUPERSEDED 2026-07-24 by §11.101(f) …]`, `:8`. **Both homes.** |
| 8 | §11.149 → §5.27 | **MARKED** | `INTENT/5.27.md:5` (transcription credited to §11.149(h)) and `:9` `BEHAVIOUR HALF UNBLOCKED … propagated §11.149(c)`; mirrored at `INTENT.md:286`. |
| 9 | §11.156 → §5.27 | FALSE-POSITIVE | `INTENT/11.156.md:43`. §5.27 is F42's **discriminating-check specimen** on a pre-repair tree (*"the method FLAGS §5.27 class-(i), and flags it on the right content"*), not a corrected claim. The repair it verifies is §11.149(h)'s, which pair 8 covers. |
| 10 | §13 row → §5.28 | OFF-AXIS | `INTENT.md:1019`. Source is a §13 row; the `REFUTED` belongs to §11.86(c)'s sub-claims and `→ §5.28` is a route. **See finding M4: §5.28 is a REUSED ID.** |
| 11 | §13 row → §5.30 | OFF-AXIS | `INTENT.md:1012`. `SUPERSEDED` belongs to b24_screen's occlusion asserts; §5.30 sits inside a struck clause. Also **NO-LIVE-NODE**: the row's only home is `INTENT/archive/5.30.md`. |
| 12 | §11.101 → §5.31 | MINT-ROUTE | `INTENT/11.101.md:44`. §11.101(j): *"defects §5.28 … §5.31 … minted"*; the nearby `REFUTED` belongs to §11.53's hypothesis. Row archived; `INTENT/archive/5.31.md:5` carries §11.101(e) in full regardless. `[F49-13]` |
| 13 | §11.165 → §5.32 | FALSE-POSITIVE | `INTENT.md:912`. §11.165's stub quoting §11.128(a2)'s correction **of §11.108(e)**, not of §5.32. §5.32's own row is marked twice anyway (`[ANNOTATED 2026-07-25, F4 §11.108(e) …]`, `[CLOSED 2026-08-01, F20 §11.128(a)(a2) …]`) — carried into scope 3 as a multi-claim case. |
| 14 | §13 row → §5.46 | OFF-AXIS | `INTENT.md:1034` (B39's row). The `REFUTED` belongs to D37's measurement reading. |
| 15 | §11.122 → §5.51 | MINT-ROUTE | `INTENT/11.122.md:3` *"NEW §5.51"*; the `SUPERSEDED` in the window is §11.122's own residual STATE marker. |
| 16 | §11.125 → §5.51 | FALSE-POSITIVE | `INTENT/11.125.md:3`. The `REFUTED` belongs to §11.122(i)'s HUNG residual; §5.51 is cited as the crash's **attributed cause** — an escalation, not a correction of the row's claim. §5.51 is `FIXED 2026-07-31 (§11.124(a)–(g))` and does not cite §11.125: **an enrichment gap, explicitly outside §11.113(p)'s class** (supersede/refute/correct), recorded not marked. `[F49-13]` |
| 17 | §11.127 → §5.52 | **MARKED** | `INTENT.md:329` *"CLOSED 2026-07-31 by F18, §11.127(a)"* with the row's candidate 2 `REFUTED` and candidate 1 `CONFIRMED`, both quoted at the node. |
| 18 | §11.127 → §5.53 | FALSE-POSITIVE | `INTENT.md:844`. The `WITHDRAWN` belongs to a diagnostic F18 built and withdrew, not to §5.53. The row carries a dated §11.127(c) pointer anyway (*"(a) ROOT-CAUSED 2026-07-31 by F18, §11.127(c)"*) — invisible to **both** marker forms (finding M1). |
| 19 | §11.163 → §5.56 | **MARKED** | `INTENT.md:337` `[GROUND REFUTED — AND MEASURED — 2026-08-29, F47 §11.163(d) … The row's PREMISE holds 23/23 … The row's INFERENCE does not …]`. |
| 20 | §11.122 → §5.59 | FALSE-POSITIVE | `INTENT.md:833` / `INTENT/11.122.md:3`. The line IS §11.122's own `[STATE 2026-08-29 …]` marker citing §5.59 as **what survives** — direction inverted, and a route. |
| 21 | §11.126 → §5.59 | **MARKED** | `INTENT.md:343` *"The fix was BUILT, MEASURED and WITHDRAWN by F19, 2026-07-31, §11.126(g)"*. |
| 22 | §11.165 → §5.59 | FALSE-POSITIVE | `INTENT/11.165.md:50`. §11.165(d)'s proposed disposition prose; the `SUPERSEDED` is §11.122(i)'s **proposed** state word, not anything of §5.59's. |
| 23 | §13 row → §5.59 | OFF-AXIS | `INTENT.md:1015` (B7's row). Describes §5.59; corrects nothing of it. |
| 24 | §11.134 → §5.62 | **MARKED** | `INTENT.md:349`, inside `[EXTENDED 2026-08-02 by F26 … (§11.134; …)`: *"ONE OF THIS ROW'S OWN STATEMENTS IS REFUTED: `active.lock` does NOT explain the 45-byte `app.log` …"*. Textbook §11.113(p). |
| 25 | §11.129 → §5.63 | **MARKED** | `INTENT.md:351` `[EXTENDED 2026-08-01 by F21's bounded attribution attempt (§11.129(b) …) … (1) the row's own characterisation is CORRECTED …]`. |
| 26 | §5.89 → §5.79 | OFF-AXIS | `INTENT.md:400`. Source is a §5 row's stub; the `CORRECTED` belongs to §5.89's **own** reach claim and §5.79 is cited as the mint criterion. |
| 27 | §11.144 → §5.80 | **MARKED** | `INTENT.md:384` *"EXTENSION 2026-08-09 (F34, §11.144; code UNCHANGED at `204d402e`): THE OWED DATUM IS ANSWERED, and the row's own text is corrected on one point."* Instrument misses it: unbracketed **and** the keyword is lowercase (finding M1). `[F49-13]` |
| 28 | §11.153 → §5.80 | **MARKED** | `INTENT.md:384` *"FIXED AND CLOSED 2026-08-26 (F40, §11.153 …) … the site count in this row is corrected: there are FOUR, not three"*. Same invisibility (finding M1). `[F49-13]` |
| 29 | §11.157 → §5.80 | FALSE-POSITIVE | `INTENT/11.157.md:33`. The `AFFECTED, SUPERSEDED` verdict is about the **harness** `f34_convention.py` whose C3 gates assert the convention F40 removed — not about the row. `[F49-13]` |
| 30 | §11.160 → §5.89 | **MARKED** | `INTENT.md:400` *"OWED SWEEP DISCHARGED 2026-08-29 (F46 → §11.160)"* + *"THE ROW'S OWN REACH CLAIM IS CORRECTED AND WIDENED"*. Both present at the node; >160 chars apart and unbracketed, so both instrument forms miss (finding M1). |
| 31 | §11.101 → §5.104 | MINT-ROUTE | `INTENT/11.101.md:23` — the citation sits **inside F49's own back-marker** at §11.101(f): *"NEW §5.104"*. `[F49-13]` |
| 32 | §11.152 → §5.104 | MINT-ROUTE | `INTENT/11.152.md:35` *"NEW §5.104"*; §5.104's stub cites §11.152(i) as its measurement provenance. `[F49-13]` |
| 33 | §11.155 → §5.104 | **MARKED** | `INTENT.md:430` *"FIXED AND CLOSED 2026-08-26 (F41, §11.155; code `18b6f13f → 742cdc82 → d6aec251`)"* with the row's own measurement both ways. The flagged `SUPERSEDED` belongs to §11.152(p)(5)'s warning. Instrument misses the marker (finding M1). `[F49-13]` |
| 34 | §11.156 → §5.104 | FALSE-POSITIVE | `INTENT/11.156.md:87` — §11.156(g)'s table quoting §11.152's assertion. Catalogue. |
| 35 | §11.165 → §5.104 | FALSE-POSITIVE | `INTENT/11.165.md:10` — §11.165(a)'s table + its summary, quoting the same. Catalogue. |
| 36 | §5.27 → §5.104 | OFF-AXIS | `INTENT/5.27.md:31`. Source is a §5 entry file; the relation is *"NEW §5.104"*, a mint. `[F49-13]` |
| 37 | §11.144 → §5.106 | MINT-ROUTE | `INTENT/11.144.md:33` — the line IS §11.144's own `[ATTRIBUTION REFUTED 2026-08-26, F40 §11.153(l) → new §5.106 …]` marker. Direction inverted: §5.106 supersedes §11.144(i), and §5.106's stub says so (*"This also REFUTES §11.144(i)'s reading"*). `[F49-13]` |
| 38 | §11.156 → §5.106 | FALSE-POSITIVE | `INTENT/11.156.md:59` — class-(ii) description quoting §11.144's marker. Catalogue. |
| 39 | §11.155 → §5.107 | MINT-ROUTE | `INTENT.md:896` *"NEW §5.107"*; §5.107's stub cites §11.155(g2)(k) as provenance. |
| 40 | §11.157 → §5.107 | FALSE-POSITIVE | `INTENT/11.157.md:41`. The `CORRECTED` belongs to *"the CORRECTED harness"*; §5.107 is named as a drift **candidate**. `[F49-13]` |
| 41 | §11.164 → §5.107 | FALSE-POSITIVE | `INTENT/11.164.md:18` — **and the superseder says so itself**: *"§5.107's own row is untouched — what is refuted is its role in the ladder's residual, not the row."* The cleanest specimen in the table: an entry that pre-empted this exact question. `[F49-13]` |
| 42 | §11.166 → §5.110 | **MARKED** | `INTENT.md:439` `[OWED DATUM PAID 2026-08-29, §11.166 (F50) … (6) ONE OF THIS ROW'S OWN CITATIONS IS CORRECTED …]`. |
| 43 | §11.170 → §5.113 | **MARKED** | `INTENT.md:445` `[BACK-MARKER — that clause is SUPERSEDED 2026-08-30 by §11.170(i) (F53): it CANNOT …]`, with the superseded clause struck-not-deleted. |
| 44 | §5.110 → §5.113 | OFF-AXIS | `INTENT.md:439`. Source is a §5 row's stub; *"minted separately as §5.113"* is a mint, and the `CORRECTED` belongs to §5.110's own citations. |

## Secondary probe 1 — widened lexicon (2 extra pairs)

| pair | verdict | argument |
|---|---|---|
| §11.130 → §5.63 | **MARKED** | `INTENT/11.130.md:60` heads *"(h) THREE THINGS §5.63 SAID THAT THIS RETRACTS OR CORRECTS"*, and §5.63's row carries *"THREE of this row's own statements are corrected in §11.130(h)"* (`INTENT.md:351`, inside the `[CLOSED 2026-08-01 by F22 (§11.130) …]` block). **Invisible to the frozen lexicon because the source says `RETRACTS OR CORRECTS` — present tense, outside the five** (finding M2). |
| §5.115 → §5.117 | OFF-AXIS | `INTENT.md:449`; source is a §5 row, relation is a route (`→ §5.117`), keyword matched is the widened `AMENDED`. |

## Secondary probe 2 — the shape probe (an independent detector, run because a lexicon negative is weak evidence)

Rule, stated: over `INTENT/<id>.md`, a line carrying BOTH a *row-referring* phrase
(`ROW'S OWN` / `this row's` / `the row's` / `row itself`, case-insensitive) AND a
correction word (the five, plus lowercase forms, plus `is wrong` / `is false` /
`does not hold` / `overstat` / `narrow`) AND a `§5.N` citation. **22 pairs, 15 of them
new** to the primary table. Adjudicated: **0 arrears**, all mint/route/confirmation —
§11.108→§5.32 (corrects §11.108(e), marked at §11.108) · §11.110→§5.42/§5.41/§5.32/§5.39
(mints and routes; the *"is also corrected"* is about §11.101(i)(3)) · §11.110→§5.27
(sequencing note) · §11.117→§5.99 (mint inside §11.117's own marker) ·
§11.125→§5.57/§5.58/§5.60 (*"NEW §5.57, §5.58, §5.59, §5.60"*) · §11.126→§5.56
(re-derives and **confirms** §11.124(f)'s ground; the correction came later at §11.163(d),
pair 19) · §11.128→§5.32 (*"The row's fix is the row's own"* — confirmation) ·
§11.156→§5.18 (route of the floor value) · §11.48→§5.2 (class label).

## Why this zero can fail — the discriminating structure

1. **The detector is demonstrated able to fail in both directions** before use
   (`f59_enum_check.log`: P1 finds an injected pair, P2/P3 refuse the lowercase and
   out-of-window variants, P4 shows the frozen home lookup answering with the wrong row).
2. **The sibling class is non-empty.** F49 ran the same instrument on the §11↔§11 axis and
   found **four** real arrears. The method-class produces non-zero answers.
3. **The adjudication step discriminates.** 15 of 44 pairs resolve MARKED and **5 of those
   15 are invisible to the mechanical probe** — so the reading is not rubber-stamping the
   machine; it overturns it in both directions (marked-where-machine-says-no ×5;
   not-an-event-where-machine-says-yes ×15).
4. **Two independent widenings could have produced arrears and did not** — a 17-word
   lexicon (+2 pairs, one of them a MARKED specimen the frozen lexicon cannot see) and a
   semantic shape probe keyed on *"the row's own"* (+15 pairs, all confirmations or mints).
5. **The one-home-only failure mode was checked separately.** Of the MARKED pairs, four
   have a target with two homes (§5.2, §5.26, §5.27 ×2); in every case the marker is in the
   home(s) it belongs in, and §5.26/§5.27 carry it in **both** — i.e. the §11.156
   authority-inversion class has **zero live instances on this axis**, F42/§11.149(h) having
   repaired the two that existed.

**What this audit still cannot see** (unchanged from `METHOD.md` §5): a correction asserted
across two lines with neither line carrying both the citation and a keyword; and any
correction phrased without a row-referring phrase *and* without a lexicon word.

## Findings routed (nothing enacted here)

- **M1 — the marker half has its own lexicon blind spot, and it is wider than the event
  half's.** `has_backmarker` accepts form A (citation inside a bracketed span with a
  case-insensitive marker word) **or** form B (citation within 160 chars of an *uppercase*
  keyword). A marker that is **neither bracketed nor uppercase-keyworded** is invisible.
  Six measured specimens on this axis, all genuine, dated, correctly-placed markers:
  `STUB REFRESHED` (§5.2), `ROOT-CAUSED` (§5.53), `OWED SWEEP DISCHARGED` (§5.89),
  `FIXED AND CLOSED` (§5.104, §5.80), `EXTENSION … corrected` (§5.80). This is the **half-2
  twin of F57's half-1 finding** — same disease, other half. → strict-credit v2.
- **M2 — the event lexicon misses the present tense.** `§11.130 → §5.63`'s source heading is
  *"THREE THINGS §5.63 SAID THAT THIS RETRACTS OR CORRECTS"*; the five words are past
  participles only. One measured specimen. → strict-credit v2.
- **M3 — the stub collision, measured on this axis.** 23 §5 numbers have no live register
  row, so `stub("5.N")` answers with §11.N's line for each of them (`stub("5.31")`
  demonstrated). Second order: `stub("5.28")` returns the LIVE §5.28 — which is not the
  §5.28 that pre-2026-08-04 citations mean (M4). → strict-credit v2's stub-collision fix.
- **M4 — §5.28 IS A REUSED ID, and this is not an instrument property.** `INTENT/archive/5.28.md`
  is *"B14 `rot_pole_w0` conversion targets an axis 90° from the texture convention"*
  (archived 2026-07-31, the referent of decision **A33** and of §11.101(b)/§11.114);
  `INTENT/5.28.md` + register row 28 are *"`Translator::getAvailableLanguagesCodes` —
  `pop_back()` on the empty join ⇒ UB"*, minted 2026-08-04 (commit `250cf1e`). **Two
  defects, one id, both with live citations** — an I2 violation in the ledger's own id
  space, and every §5.28 citation now needs its date to disambiguate. NOT a §5 mint (§5.79
  is about shipped-surface defects); **ROUTED** — renumbering or aliasing is a ledger
  decision, not an executor's.
- **M5 — a third axis nobody audits.** 8 of the 44 pairs have a **§13 row or a §5 row** as
  source. §11.165(c) put the §5-source case in its own bucket and stopped there; the
  §13-source case has never been named. Adjacent to F60's §5↔§13 sweep — flagged for it.
- **M6 — §5.51's enrichment gap.** §11.125 attributes a 25-of-26-cycle teardown SIGSEGV to
  §5.51's missing virtual destructor; §5.51's row records the §11.124 fix and never cites
  §11.125. Not an arrears (§11.113(p) covers supersede/refute/correct, not escalation) —
  recorded so the distinction is visible rather than assumed.
