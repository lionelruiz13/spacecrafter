# F57 scope item 3 — the five metrics measured, per-prediction outcomes

All numbers reproduce from `python3 harness/f57_score.py` (per-part judgments in
`f57_coverage.tsv`, markers in `f57_markers.tsv`) plus the two git re-measurements
recorded in §3 and §4 below. Predictions: `f57_predictions.json`, md5
**`4d5b2486f35490c1a080f5bf2ded3ead`**, committed `09b5e80` BEFORE any of this existed.
Partition labels: `f57_partition.md`, committed `cf2cd5e` before the predictions.

---

## 1. m1 — answer shape GIVEN question shape

**Corpus**: 40 items (27 round-1, 13 round-2), 72 pre-committed parts (64 D, 8 E).

| measure | value | partition |
|---|---|---|
| round-1 answer length | median **15.0** words, mean 17.2, range 3–43 | TRAINING |
| all items | median 16.0, mean 18.1, range 1–66 | TRAINING |
| E-bearing vs pure-D median | 16.0 vs 16.5 → **ratio 0.970** | OUT-OF-SAMPLE |
| D-part coverage | **0.789** (47 full, 7 partial, 10 dropped of 64) | OUT-OF-SAMPLE |
| E-part coverage | **0.562** (3 full, 3 partial, 2 dropped of 8) | OUT-OF-SAMPLE |
| D−E gap | **22.7 pp** | OUT-OF-SAMPLE |
| round 1 vs round 2, all parts | **0.704** (38.0/54) vs **0.944** (17.0/18) | OUT-OF-SAMPLE |
| coverage by parts-per-item (round 1) | 1 part **0.938** · 2 parts **0.788** · 3–4 parts **0.500** | OUT-OF-SAMPLE |
| coverage by question length (round-1 terciles) | 45–59 w **0.821** · 61–80 w **0.765** · 87–301 w **0.587** | OUT-OF-SAMPLE |

- **P1.1 PASS** (median 15.0 ≤ 20, mean 17.2 ≤ 30) — TRAINING/CONFIRMATORY, earns the model
  nothing (sc4), and sc1 forbids reading it as effort evidence.
- **P1.2 FAIL.** Predicted ≥ 1.5× longer answers where an open part was asked; measured
  **0.970×**. The `executor_hunch` field committed with the prediction called this failure in
  advance. ⇒ **C2 is not supported on the length channel**: where depth is invited, the answer
  does not grow.
- **P1.3 PASS on its stated failure condition, with one clause of the claim missed and recorded.**
  Gap 22.7 pp ≥ 20 ✓, E ≤ 0.65 ✓ (0.562), but the claim also said D ≥ 0.85 and D measured
  **0.789** ✗. The miss is the more interesting half: coverage of *decision-shaped* parts is not
  near-complete either — 13 of 48 round-1 D-parts were dropped or partial.
- **P1.4 PASS.** Both re-ask causes present in the same corpus: R2 re-asks because an ANSWER was
  ambiguous ("(a) is better" against case-labels colliding with option-labels); R1 re-asks
  because the QUESTION was defective (engine vocabulary in a document that declares
  implementation knowledge not assumed). Both are §11.48(d)'s own diagnosis, reproduced here
  from the corpus.

**The result nobody predicted — the budget is per ITEM.** Coverage falls monotonically with
parts-per-item (0.938 → 0.788 → 0.500) and with question length (0.821 → 0.765 → 0.587), and
round 2 — which re-asked the dropped parts ONE AT A TIME — recovered them at **0.944**. Two
specimens: Q6's far-descent part, dropped, re-asked in isolation at R6, answered in two words
("Last selected."); Q12's what-for and stop-vs-slow parts, both dropped, both delivered at R4.
A dropped part is not a refusal and not a missing opinion. It is a per-item budget, and the
budget is spent on the first parts.

---

## 2. m2 — answer-by-action rate across SS-n

Per-entry classification (made at measurement time — NOT pre-committed; the P2.3 partition
RULE was, the per-entry assignment was not; recorded as the weaker form it is):

| class | entries | discharged |
|---|---|---|
| **ACTIONABLE** (a file edit answers it) | SS-1, SS-3, SS-6, SS-7, SS-16 | **5 / 5** |
| **MIXED** (an edit + a question the edit cannot answer) | SS-2, SS-4, SS-8, SS-10, SS-18 | action half **4.5 / 5** (SS-18 partial: (c)(d)(e) done, (a)(b) standing); word half **1 / 5** (only SS-8's, and by implication) |
| **WORD-ONLY** (only a sentence can answer) | SS-5, SS-9, SS-11, SS-12, SS-13, SS-14, SS-15, SS-17, SS-19 | **0 / 9** |
| verbal replies anywhere in the file | — | **0 / 19** |

- **P2.1 PASS** — zero written replies across all 19 entries.
- **P2.2 PASS, re-measured independently rather than re-read.** Both blobs fetched from the code
  repo (`70dee810:doc/superscript.sts` 44 067 B / 1 408 lines → `f0c8ef83:doc/superscript.sts`
  53 915 B / 1 607 lines) and each witness spelling counted by regex: **11 of 13 gone, 2 survive**
  — exactly the figure §11.149(e) and Q-55 carry. **The counting unit matters and is recorded so
  the number is unambiguous**: one witness per distinct SPELLING, with SS-4 (`movetocity` +
  `moveto … name`) and SS-9 (`image … spacecraft` + `landscape … spacecraft`) each carrying two.
  Counted by §11.149(e)'s TABLE ROWS instead, the same evidence reads 10 of 12 script-side rows
  (13 rows including the code-side recorder row, which could not change). Same evidence, two
  units, no contradiction. Survivors: `landscape … spacecraft on` (1 occurrence, byte-identical)
  and the `Ganymed=503`/`Saturn=600` table (2 occurrences).
  Sub-check: the 0xA0 census **4 → 3 by LINE** reproduces exactly (pre L94/L523/L1086/L1098 →
  post L619/L1245/L1257, all three inside `#` comments); by BYTE it is 5 → 4, one pre-rewrite
  line carrying two. Unit difference, not a discrepancy.
- **P2.3 PASS** — actionable 1.0 ≥ 0.75, word-only 0.0. The contrast is what makes the action an
  ANSWER rather than silence: where an edit could discharge the entry, every one was discharged;
  where only a sentence could, none was.
- **P2.4 PASS** — `f0c8ef83` subject *"Update superscript.sts"* = **2 words**, body **empty**,
  author Lionel RUIZ, 2026-08-26 08:59:50 +0200. 199 lines added and 11 witnesses fixed, with no
  sentence anywhere saying so.

---

## 3. m3 — belief-vintage lag

| measure | value |
|---|---|
| answers refuted by a later ledger measurement, after the instrument check (sc3) | **0** |
| apparent conflicts resolving as INSTRUMENT-wrong | **3** |
| history-class answer rate | **0.083** (0.5 of 6: SS-2, SS-4, SS-5, SS-9, SS-10 all 0; Q16b 0.5) |
| everything-else answer rate | **≥ 0.75** (Q/R corpus 0.764; actionable SS entries 1.0) |

- **P3.1 PASS (0, inside 0–2) — with the boundary that makes it honest.** The zero is NOT
  "he is always right". It is that **almost no answer in this corpus is reachable by a project
  measurement**: his answers are about his shows, his scripts and his audience, and the project
  cannot observe any of the three. m3 as posed is therefore **NAMED-PARTIALLY-SCORED**: the
  answer-vs-measurement channel is nearly empty by construction, and the metric's own instrument
  boundary is the finding.
- **P3.2 PASS** — three, and all three are worth naming because they are three different
  instruments: (i) Q17's ring-shadow referent — the QUESTION used engine vocabulary
  (§11.48(d)); (ii) Q1's "(a) is better" — the QUESTION's case-labels collided with its
  option-labels, so the answer's ambiguity is the question's defect; (iii) SS-6's
  `set stall_radius_unit = 5.0` — the ENGINE accepted the line, dropped the value and reported
  nothing, so nothing could ever have updated his belief. §11.149(d)'s precedent cuts both ways
  and it cut this way all three times.
- **P3.3 PASS** — history 0.083 against ≥ 0.75, a gap of ≥ 66 pp. And the mechanism is visible in
  the same corpus: history is the one thing an ACTION cannot answer, which is why every
  history-class entry is still open after a rewrite that closed everything an edit could reach.
  §11.149(e)'s *"a deletion is not an answer"* is this measurement's qualitative twin.
- **What C3 actually needs, derived from (iii):** belief-lag is only detectable where a feedback
  channel exists. Where the engine accepts and stays silent, a belief cannot age visibly, and no
  amount of re-elicitation discipline will surface it. That is the §11.169 diagnostic-schema
  territory arriving from the human-interface side.

---

## 4. m4 — questions asked back, and the other depth markers

| measure | value | partition |
|---|---|---|
| questions asked back | **2**, both inside Q17's answer, 0 in round 2, 0 in SCRIPT_SURFACE | TRAINING |
| depth markers, pre-committed (broad) definition | **21 instances over 17 of 39 answered items** | OUT-OF-SAMPLE |
| depth markers, the project's own narrow class (§11.48(c), "answers that exceed their question" ⇒ a new tracker item) | **4** | SEEN-UNSCORED |
| unrequested script-surface proposals | **6** (Q5, Q12, Q22, R3, R8, R9) | OUT-OF-SAMPLE |
| unrequested proposals about rendering internals | **0** | OUT-OF-SAMPLE |

- **P4.1 PASS** — TRAINING; the count was an INPUT to the model and cannot test it.
- **P4.2 FAIL, by 2.6×.** Predicted 3–8 instances, measured **21**. Depth markers are not rare —
  they are pervasive and SHORT. **Definition sensitivity, recorded rather than resolved in the
  convenient direction**: under the project's own narrower class (a volunteered proposition that
  became a tracker item — §11.48(c)'s four: Q22→A30, Q5→B4, Q12→B10, Q6→B10(iv-b)) the count is
  4, inside the predicted band. Scored against the definition that was pre-committed, which is
  the broad one: FAIL.
- **P4.3 PASS**, with one boundary call made at scoring: Q21's volunteered non-uniform red glow
  is an APPEARANCE proposal (what the audience sees — his domain), not a rendering-internals
  proposal, so the second clause holds at zero. Six script-surface proposals against zero
  internals is the direction the prediction claimed.
- **P4.4 PASS at n=2, low power, stated in advance.** Both questions-back attach to the one item
  whose terms the document left undefined.
- **The refinement this forces on C2.** The depth budget IS spent — 21 volunteered propositions,
  4 of which the project promoted to tracker items and one of which (Q22) opened a new row (A30)
  — but it is spent on **volunteering what he holds**, not on **covering what he was asked**.
  "Maximize value-per-thought" measured from his side: he answers from his own model of what
  matters, not from the question's structure.

---

## 5. m5 — Vixy-control conditioned on question class

**The control table, by the authority boundary §11.48 states in its own words** (*"an operator
answer is decisive on what the product must do … and is not decisive on mechanism, which stays
derived"*):

| cell | populated? | why |
|---|---|---|
| OBSERVABLE ("what must the product do") × tester | **yes** — 13 refs resolved outright, round 1 | his class by rule |
| SHIPPED-SHOW FACT ("what do your scripts do") × tester | **yes** — decisive by §2(b) immutability | his class by evidence |
| SHIPPED-SHOW FACT × Vixy | **EMPTY — by EVIDENCE** | he cannot answer what the tester's shows do |
| MECHANISM × tester | **EMPTY — by AUTHORITY** | the boundary rule; demonstrated once (below) |
| MECHANISM × Vixy | **yes** — D15, D21, D23, D6, §11.49(a), §11.51 | his class by rule |

- **P5.4 PASS, and sharper than predicted.** Not one empty cell but **two**, empty for two
  different reasons. So a Vixy-vs-tester person-comparison has **no non-confounded cell in either
  direction**: what would read as "Vixy controls the decisions" is the projection of a two-axis
  routing — authority for mechanism, evidence for field facts — onto one axis. **m5's naive form
  is NAMED-NOT-SCORED with its boundary**, which is Q-55's own instruction honored, and the
  non-computability is a property of the routing DESIGN, not of either person.
- **The one measured demonstration of the boundary**: Q13/A10. The tester answered *"It should
  be where it is now"*; §11.48(a) propagated it as *hidden bodies keep updating*; five days later
  Vixy's D23 reversed the MECHANISM (hidden bodies do not tick) while the OBSERVABLE he asked for
  survived, moved into the §11.76 use-site barrier (§11.113(b), marker §11.114). One answer,
  split across the boundary, each half landing with its owner.
- **P5.1 FAIL** — predicted ≤ 3 paired refs, measured **4**: A10 (D23), A9 (§11.49(a), Vixy
  verbatim), A3 (§11.51 round-4), A24→B17 (D6). Each is a recorded design-authority decision on a
  ref the tester also answered.
- **P5.2 PASS** — R8 (*"your answer no longer decides what gets built now"*) and R11 (*"an engine
  rule settled that both channels get built either way"*): 2 of the 11 live round-2 items say so
  in the question text itself.
- **P5.3 PASS, and the reconstruction is the interesting part.** 13 of 27 round-1 refs resolved
  outright; the 11 narrowed ones were narrowed by (i) a DROPPED PART — A3's reload survival, A4's
  toggle, A18's far descent, A27's data question — (ii) an INSTRUMENT fault — A9's label
  collision, A28's vocabulary — or (iii) a CLASS MISMATCH — A15's threshold values (unjudgeable
  unseen, re-ask deferred by design), A17's new disambiguation question, A29's enumeration, A2's
  reading boundary, A7's unreachable multi-star case. **Not one narrowing traces to a wrong or
  evasive answer inside his own class.** Within-class correctness in this corpus is 1.0; the cost
  falls entirely on COVERAGE.

---

## 6. Scoreboard

| id | metric | class | outcome |
|---|---|---|---|
| P1.1 | m1 | CONFIRMATORY / TRAINING | PASS (earns nothing) |
| P1.2 | m1 | RISKY | **FAIL** — 0.970 vs ≥1.5 |
| P1.3 | m1 | RISKY | PASS on the stated failure condition; the D ≥ 0.85 clause of the claim missed (0.789), recorded |
| P1.4 | m1 | CONFIRMATORY | PASS |
| P2.1 | m2 | RISKY | PASS |
| P2.2 | m2 | CONFIRMATORY / TRAINING | PASS, independently re-measured (11/13, unit recorded) |
| P2.3 | m2 | RISKY | PASS |
| P2.4 | m2 | RISKY | PASS |
| P3.1 | m3 | RISKY | PASS (0) — with the instrument boundary that makes the zero meaningful |
| P3.2 | m3 | RISKY | PASS (3, three different instruments) |
| P3.3 | m3 | RISKY | PASS (0.083 vs ≥0.75) |
| P4.1 | m4 | CONFIRMATORY / TRAINING | PASS (earns nothing) |
| P4.2 | m4 | RISKY | **FAIL** — 21 vs 3–8 (4 under the project's narrower class; definition sensitivity recorded) |
| P4.3 | m4 | RISKY | PASS (6 vs 0), one boundary call recorded |
| P4.4 | m4 | RISKY | PASS at n=2, low power |
| P5.1 | m5 | RISKY | **FAIL** — 4 vs ≤3 |
| P5.2 | m5 | CONFIRMATORY | PASS |
| P5.3 | m5 | RISKY | PASS |
| P5.4 | m5 | RISKY | PASS, sharper than claimed (two empty cells, two reasons) |

**15 PASS · 3 FAIL · 1 mixed-and-recorded** (P1.3 counted as the mixed one). Of the 15 passes,
**4 are CONFIRMATORY or TRAINING and earn the model nothing** by the partition's own rule. The
model's real score is therefore over the **14 RISKY** predictions: **10 clean passes · 1 mixed ·
3 failures**. Two of the three failures (P1.2, P4.2) are the same finding arriving twice — the
depth budget exists but is not spent where the model implied — and the third (P5.1) is a count
that the corpus simply exceeds. All three are traced above; none is absorbed.
