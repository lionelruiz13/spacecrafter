# F57 scope item 1 — the out-of-sample partition, written BEFORE any prediction and BEFORE any scoring

Task F57 (§11.173(d) / Q-55). Written 2026-08-30, code `3ccfc6d8`, harness `47f4bc8`.
This file is the anti-circularity act: the model under test was built partly ON the
corpus it is about to be scored against, so what the model already saw must be named
before anything is counted. Re-scoring training data as confirmation is the failure
mode this partition exists to prevent.

## 0. Method, stated so the partition is challengeable

The model = §11.173(d) + the Q-55 body (`~/shared/QUEUE.md`, read 2026-08-30, never
recalled). Its recorded inputs are enumerated below as S1–S7 from those two sources
ALONE — no inference about what else the model-builder may have known, except S1,
which is named precisely because it cannot be enumerated.

Three labels, and the middle one exists because a two-label partition would be a lie
about a human-observational model:

- **TRAINING** — the recorded model derives a stated claim from this evidence.
  Scorable, but scored as RETRODICTION: a match is consistency, not confirmation.
- **SEEN-UNSCORED** — the model-builder demonstrably had access to the text (he
  relayed it), but no recorded claim is derived from it, and no count was ever taken.
  Weak out-of-sample: it can surprise, but "never seen" cannot be certified.
- **OUT-OF-SAMPLE** — no recorded claim derives from it AND no count of it exists
  anywhere in the sources. The real test.

**The certification limit, stated up front:** the tester corpus was relayed to the
project BY Vixy, who is the model's author. Therefore NOTHING in
`USER_QUESTIONS.md` / `USER_QUESTIONS_ROUND2.md` can be certified unseen. The
strongest honest claim available for any corpus item is *"no claim in the recorded
model was derived from it, and no count of it exists"* — which is what OUT-OF-SAMPLE
means in this file, and it is weaker than an unseen holdout. Every verdict this task
reaches inherits that weakening; it is stated once here and cited, not re-argued.

**A second limit, stated because it bounds the whole task:** the executor read the
three corpus files IN FULL before writing the predictions (unavoidable — Q-55's m1
requires the QUESTION shapes to be classified first, and the question texts sit
inside the same files as the answers). So this is predictions-before-SCORING, not
predictions-before-EXPOSURE, and it is strictly weaker than F55/F56's
predictions-before-a-launch. Two mitigations, both mechanical:
(i) the per-item question-shape classification and the per-item PARTS enumeration are
committed in the predictions artifact, derived from question text only, so the class
boundaries cannot be fitted afterwards to make a prediction come true;
(ii) each prediction carries an information class — **RISKY** (could plausibly fail on
this corpus) vs **CONFIRMATORY** (its outcome is already visible in text the executor
has read) — so a green CONFIRMATORY prediction is never counted as evidence.

## 1. The model's recorded inputs (S1–S7)

| id | input | source |
|---|---|---|
| S1 | Vixy's accumulated observation of the tester, hedged "probably", observational | Q-55 opening; §11.173(d) |
| S2 | §11.168's anticipatory-convergence git history — retrodicted signature (DEPTH side, Vixy himself) | Q-55 "RETRODICTS measured signatures" |
| S3 | the script corpus + **11-of-13 one-morning witness fixes, §11.149(e)** — retrodicted signature (THROUGHPUT side) | Q-55, same clause |
| S4 | the relayed tester self-report on Q1–27 (*"weren't trivial, required him to think a lot"*, *"very sharp"*) — a statement ABOUT the corpus, not its content | Q-55 MODEL REFINEMENT |
| S5 | Q-53's 2020 per-channel logging consultation (ROOM criterion → 2026 cost inversion) — the belief-vintage TYPE SPECIMEN | §11.173(b); Q-55 consequence (2) |
| S6 | Q-53/Q-54's three half-open loops (reservation unvoiced / cost model unshared / refinement unsubmitted) | §11.173(d) |
| S7 | *"the July document's self-contained decision shape"* — a claim about OUR question document's authorship, not about the tester | Q-55 "RETRODICTION already in hand" |

## 2. The partition, per metric

### m1 — answer-shape GIVEN question-shape

| element | label | argument |
|---|---|---|
| the QUESTION shapes of Q1–27 / R1–R13 | **TRAINING (of the interface claim, not of the tester model)** | S7 derives *"decision-shaped, self-contained"* from this document. The shapes are our own authorship; using them as the CONDITIONING variable is legitimate, but no credit accrues to the tester model from them. |
| the aggregate impression *"answers are short/fast"* | **TRAINING** | S1's throughput clause is exactly this impression. Any measurement of mean answer length is retrodiction. |
| Q1–27's effort level | **TRAINING (calibration cell)** | S4. Short answers in Q1–27 are NOT throughput evidence — the cell forbids that reading. |
| the **variance across question classes** (open vs decision-shaped) | **OUT-OF-SAMPLE** | No recorded claim conditions answer length on question class; no such count exists in any source. This is the refinement C2's first real test. |
| the **sub-question coverage rate** (parts answered / parts asked) | **OUT-OF-SAMPLE** | Never counted anywhere. The triage signature predicts DROPPED parts; nothing in the sources says whether parts were dropped. |
| the **ambiguity cases** (answers that forced a re-ask) | **SEEN-UNSCORED** | R2's existence is stated inside the corpus the executor read; the model derives nothing from it. |

### m2 — answer-by-action rate across SS-n

| element | label | argument |
|---|---|---|
| the 2026-08-26 `superscript.sts` rewrite, **11 of 13 witnesses fixed** | **TRAINING** | S3 names this exact statistic as a retrodicted signature. Re-reporting 11/13 is retrodiction and is labeled so wherever it appears. |
| SS-1…SS-10 outcome states | **TRAINING** | They ARE the 13 witnesses (§11.149(e)'s table). |
| the **verbal-answer rate** across all 19 SS entries | **OUT-OF-SAMPLE** | The model saw an ACTION count. The absence of words was never counted. |
| the entries **outside** the training specimen (SS-11…SS-15, SS-17, SS-18, SS-19) | **OUT-OF-SAMPLE** | Not among the 13 witnesses; no source states their state. |
| the **needs-a-word vs needs-an-action** split and its outcome correlation | **OUT-OF-SAMPLE** | The discriminating structure of m2. Nothing in the sources partitions the entries this way. |

### m3 — belief-vintage lag

| element | label | argument |
|---|---|---|
| the 2020 ROOM criterion vs the 2026 cost inversion | **TRAINING** | S5, the type specimen; it is the evidence consequence (2) was written from. |
| the 2020 French per-command reference attesting `set … duration` / `moveto … name` / `wait action reset_timer` (SS-2/4/10) | **SEEN-UNSCORED** | Recorded at §11.149(e)/SCRIPT_SURFACE §3 well before the model; the model derives nothing from it. |
| every candidate conflict inside Q1–27 / R1–R13 between an answer and a LATER measurement | **OUT-OF-SAMPLE** | No such comparison exists anywhere in the sources. |
| the instrument-first discipline (§11.149(d): premise right, instrument's reading wrong) | applies as a **method constraint**, not as data | Q-55's own m3 caution. Any conflict is adjudicated against the instrument BEFORE it is scored as belief-lag. |

### m4 — questions asked back

| element | label | argument |
|---|---|---|
| the **existence and count** of the two questions back (both in Q17) | **TRAINING** | Q-55 names them: *"his two questions back = depth markers, the highest-value points"*. The count 2 is an input, not an outcome — it cannot test anything. |
| **other** depth-marker classes (unrequested elaboration, counter-proposal, framing correction, explicit "unclear question") | **OUT-OF-SAMPLE** | Never enumerated in any source; the model names only the questions-back class. |
| **where** the markers land (which question class triggers one) | **OUT-OF-SAMPLE** | No source analyses the trigger. |
| round 2's question-back count | **SEEN-UNSCORED** | Vixy relayed round 2; no count exists. |

### m5 — Vixy-control conditioned on question class

| element | label | argument |
|---|---|---|
| the routing rule itself (structural→Vixy, operational→tester) | **TRAINING (as a confound, not as a result)** | Q-55 states it as the confound to honor. It is the conditioning variable, not an outcome. |
| the **control table** (who determines each item's outcome, by class) | **OUT-OF-SAMPLE** | No such tabulation exists anywhere in the sources. |
| the items where the corpus itself says the design authority already decided (R8/R11's *"your answer no longer decides"*) | **SEEN-UNSCORED** | Visible in the corpus text; no source counts them. |
| whether the control is **computable at all** on this corpus | **OUT-OF-SAMPLE** | The question nobody has asked: if routing is by design, the person-comparison may have no non-empty cell. A "not computable" outcome is a finding about the ROUTING DESIGN, not a failure of the task. |

## 3. What this partition forbids

1. Reporting 11/13 (m2) or "answers are short" (m1) as CONFIRMATION of the model.
   Both are retrodictions by construction and are labeled TRAINING at every mention.
2. Reading a short answer inside Q1–27 as throughput evidence — S4's calibration cell
   (self-reported HIGH effort) forbids exactly that inference.
3. Scoring an answer-vs-measurement conflict as belief-lag before the instrument has
   been checked (§11.149(d) cuts both ways: at D37 the premise was right and the
   ledger's own reading was wrong).
4. Treating any OUT-OF-SAMPLE verdict as if it came from an unseen holdout: §0's
   certification limit weakens every one of them, once, here.
