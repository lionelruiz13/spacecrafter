# F59 scope 1 — the enumeration method, committed BEFORE any verdict

Task F59 (§11.165(h)(3)), 2026-08-30. This document is the audit's **boundary**: what is
inside the universe, what is outside, and by which mechanical rule. It is committed in
its own commit, ahead of every verdict, so the boundary cannot be adjusted to fit the
answers (the F58 form: *universe committed before classification*).

## 0. The question being enumerated

§11.165(h)(3)(i), verbatim: *"the §5-side twin (does every §5 row carry the marker for
the §11 entry that corrected it — 13 candidate pairs sit in (c)'s third bucket)"*.

So the axis is **§11-node → §5-row correcting relations**, and the twin's test is
§11.113(p) applied at the **§5 row**: does the corrected row carry a marker naming the
entry that corrected it.

## 1. Half 1 — the candidate detector

Deliberately the FROZEN instrument's own rule (`intent_backmarker_scan.py:30-33, 86-99`),
so the twin's numbers are comparable with its §11↔§11 sibling:

- lexicon `REFUTED|SUPERSEDED|CORRECTED|RETRACTED|WITHDRAWN`, **uppercase, exact**;
- a `§N.M` citation within **160 characters** of a keyword, **on one line**;
- `target != source`;
- **restriction:** target must be `§5.N` (this audit's axis).

**One stated widening — the SOURCE set.** The instrument reads `INTENT/<id>.md` as
sources and opens `INTENT.md` only to resolve a *target's* stub; an assertion living in
an inline stub line, or in an archived entry file, is invisible to it as a source. This
enumeration scans three classes and labels every row with the one it came from, so the
instrument's own subset stays exactly recoverable:

| class | text | in the frozen scan's source set? |
|---|---|---|
| `A-entry` | `INTENT/<id>.md` | yes |
| `B-stub-5` / `B-stub-11` / `B-stub-other` | `INTENT.md` lines, attributed by register from the `## N.` headers | **no** |
| `C-archive` | `INTENT/archive/<id>.md` | **no** |

**Measured on this tree:** 62 candidate lines → **44 distinct (src, tgt) pairs** with a
§5 target (`A-entry` 35 · `B-stub-11` 18 · `B-stub-5` 2 · `B-stub-other` 7 · `C-archive`
**0**).

## 2. Half 2 — home resolution is re-implemented, and why

The instrument's `stub(idnum)` resolves a §-id by the first `INTENT.md` line starting
with the number after the dot (`:40-45`). That is the **stub-collision defect** recorded
at §11.165's fourth-property note, and it lands squarely on this audit's axis: **23
numbers below the maximum are absent from the live §5 register** (measured: 3, 4, 6,
10–17, 22–24, 29–31, 33, 37–40, 45 — closed rows are moved to `INTENT/archive/5.N.md`),
so for those targets the lookup returns a §11 stub. Demonstrated, not asserted:
`stub("5.31")` returns `31. **Row 10: AXIS module live …**`, which is §11.31's stub
(`f59_enum_check.py` P4).

Homes are therefore resolved explicitly:

```
homes(§5.N) = { the live register stub line, located INSIDE the §5 numbered list }
            ∪ { INTENT/5.N.md }
            ∪ { INTENT/archive/5.N.md }
```

A target with **no** home is reported as such — it cannot carry a marker, and that is a
verdict of its own, not a violation.

The `mech` column applies the instrument's two marker forms (**A** = citation inside a
bracketed marker span; **B** = citation within 160 chars of an uppercase keyword) to
those correctly-resolved homes. **It is an input to adjudication, never a verdict**:
every pair in the delivered table is read at both ends.

## 3. Verdict classes (fixed here, before any pair is read)

| verdict | meaning |
|---|---|
| **MARKED** | the §5 row carries a marker naming the correcting §11 entry (§11.113(p) satisfied) |
| **UNMARKED-REAL** | a real correction of a claim the §5 row makes, and the row does not carry it ⇒ arrears: annotate at the node per §11.161(g), same commit |
| **MINT-ROUTE** | the relation is a mint/route (*"NEW §5.104"*), not a correction of the row's own claim — out of class by §11.165(c)'s own ruling |
| **FALSE-POSITIVE** | the keyword belongs to a neighbouring claim, or the line IS the back-marker read backwards — named, never deleted (the F56/F57 exception discipline) |
| **NO-LIVE-NODE** | the §5 row has no live home to carry a marker (archived/closed) — recorded, and what the archived home says is stated |
| **AMBIGUOUS** | cannot be honestly adjudicated: listed with both readings, never resolved by guess |

**[CLASS ADDED DURING THE WALK, with its argument — 2026-08-30, scope 2.** A seventh
verdict, **OFF-AXIS**: the source is a §13 row or a §5 row, not a §11 entry, so the
twin's question (*"does the §5 row carry the marker for the §11 entry that corrected
it"*) has no §11 entry to ask about. 8 of the 44 pairs are this shape. Filing them under
FALSE-POSITIVE would have been a lie about why they are not arrears, and §11.165(c) had
already given the §5-source case its own bucket — this only names the §13-source case
too. The class is additive: it removes nothing from the six above and changes no
verdict.]**

## 4. The floor assertion

§11.165(c)'s third bucket names 13 pairs. All 13 are present in this enumeration and are
checked individually: §11.101→§5.31 · §11.125→§5.51 · §11.144→§5.106 · §11.144→§5.80 ·
§11.152→§5.104 · §11.153→§5.80 · §11.155→§5.104 · §11.157→§5.107 · §11.157→§5.80 ·
§11.164→§5.107 · §11.6→§5.2 · §5.27→§5.104 · §11.101→§5.104. **They are the floor, not
the list.**

## 5. What this enumeration cannot see — stated so the bound is honest

1. A correcting relation asserted **across two lines**, or in prose **without one of the
   five uppercase words**, is invisible BY DESIGN (the session-16 discharge-vocabulary
   ruling; the incompleteness-lexicon member of the queued strict-credit v2 package).
   **Sized, not waved at:** a secondary probe with a widened lexicon (`--widen`, 17 words,
   `f59_pairs_widened.tsv`) adds exactly **2** pairs over the whole corpus —
   `§11.130 → §5.63` and `§5.115 → §5.117`. Reported separately; **not merged** into the
   primary table.
2. The detector over-generates: a keyword may belong to a neighbouring claim. That is what
   the FALSE-POSITIVE verdict is for.
3. Half 2 answers **per pair**, not per claim — which is exactly what scope 3 (the
   multi-claim case) exists to probe, under its own committed sampling rule.

## 6. Instrument status

`intent_backmarker_scan.py` and `intent_pair_check.py` are **FROZEN** this round. Nothing
here edits them; `f59_enum.py` is a task-local enumerator. Every observation made about
the instruments is ROUTED to the queued strict-credit v2 package.

## 7. Both directions, before trusting the enumerator

`f59_enum_check.py` (log: `f59_enum_check.log`), predictions in the assertions:
P1 an injected in-window uppercase line **must** produce its pair (+1, produced);
P2 the same sentence in lowercase **must not** (+0); P3 the same sentence with the
citation pushed past 160 chars **must not** (+0); P4 `homes("5.31")` must return the
archived entry and not a §11 stub, while the frozen `stub("5.31")` returns §11.31's.
**All four pass.**
