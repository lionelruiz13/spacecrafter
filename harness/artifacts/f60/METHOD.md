# F60 — the §5↔§13 row↔row staleness axis: the method, committed before any verdict

Task F60 (`fable-dispatch.md`), mandate = the F47 acceptance's queue-member sentence
(`fable-dispatch/archive/F47.md:99-100`) and §11.163(c)/(k)(1), which named the class
and left it unswept:

> "the §5 row is stale against the §13 row that owns its work, while the stub↔entry
> pair is internally consistent — which is precisely why F42's pair sweep could not
> reach it (both homes carry the same stale text). **The class F42 closed was
> stub-vs-entry; this one is row-vs-row, and it is not swept by anything today.**"
> — `INTENT/11.163.md` (c)

## 1. What the unit is

The unit is a **reference**: a live §5 text names a §13 row id, and the question is
whether the §13 row's **current state** still supports what the §5 text **assumes**
about it. The instrument set does not reach it, measured:

| instrument | its axis | why it cannot see this one |
|---|---|---|
| `intent_pair_check.py` | stub ↔ entry file, same id | §11.163(c)'s case has both homes carrying the SAME stale text ⇒ internally consistent |
| `intent_backmarker_scan.py` (FROZEN) | §N.M → §N.M supersession events | resolves `§`-citations only; a bare `B32` / `A33` token is not a citation to it |

## 2. Source universe (`srcclass`), every exclusion stated

| class | what | count | in scope? |
|---|---|---|---|
| `LIVE-STUB` | the 95 numbered rows of the §5 register, `INTENT.md` §5 | 95 rows | YES |
| `LIVE-ENTRY` | `INTENT/5.N.md` where N has a live register row | 10 files | YES |
| `INPLACE-ENTRY` | `INTENT/5.N.md` where N has NO live register row (4, 16, 17, 22, 23, 24 — "archived in place" 2026-07-31: the derived index line retired, the ENTRY stayed live under `INTENT/`) | 6 files | YES — the file is live text and §5.24 is still marked **OPEN** inside it |
| `ARCHIVE-ENTRY` | `INTENT/archive/5.N.md` | 18 files | **NO** — enumerated and reported so the exclusion is sized, never adjudicated and never annotated: an archived entry is retired history and the archival convention's own rule is that references are never rewritten (F59's ruling: the 23 retired §5 numbers live only at archive) |

## 3. Target universe

Every §13 row id ever allocated, resolved to its CURRENT home — `LIVE-A` (18 rows of
§13.A) · `LIVE-B` (25 rows of §13.B) · `ARCHIVED` (a body file at
`INTENT/archive/<id>.md`, 15 B-rows) · `CLOSED-PTR` (named only in §13.C's closed
lists, pointer-only) · `UNKNOWN` (a token matching no allocated id — reported, never
dropped).

## 4. Detector

Forward: `(?<![A-Za-z0-9_])([AB]\d{1,2})(?![0-9])` over the source text.
Backward: `§5\.(\d+)` over the live §13 row lines (the "a §13 row claims ownership of
a §5 fix" direction the task section names).

**The detector over-generates, and the over-generation is measured, not hypothetical.**
§11.73's B27 hardcode-key design labels its keys **A1…A9 / Tier-A / Tier-B**, and
§5.5's entry file carries nine of them. `A8/A9 type-keyed → B27` in §5.5 is a pair of
KEYS, not a pair of §13.A rows. Therefore **no verdict in the delivered table is
produced by the script**: the script fixes the universe and the boundary; every
enumerated reference is adjudicated by READING both texts. `--selftest` proves the
collision is produced (P5) precisely so it cannot be silently absorbed.

## 5. Both-ways mapping of the enumerator, before any verdict

`python3 f60_enum.py --selftest` (log: `f60_selftest.log`) — all five pass:

| probe | expectation | measured |
|---|---|---|
| P1 positive detect | `B28`, `B16` found in a synthetic routing sentence | found |
| P2 in-word / hex refused | `BMT_RGBA8_SELF_SHADOW`, `0xA0`, `SCK_A1X` → no hits | none |
| P3 unallocated id | `B99` resolves UNKNOWN, not dropped | UNKNOWN |
| P4 resolver, both directions | `B27`→LIVE-B · `B32`→ARCHIVED · `A33`→CLOSED-PTR | as expected |
| P5 known collision produced | §5.5's Tier-A key hits appear (so they must be read) | 12 hits |

## 6. Verdict classes (assigned by reading, one per reference)

- **COHERENT** — the §13 row's current state supports what the §5 text assumes.
- **STALE** — the referenced row moved (closed / archived / delivered / delegated /
  answered) in a way that contradicts the §5 text's assumption ⇒ annotate the §5 node
  per §11.161(g) (UPPERCASE keyword, citation inside the marker span, original
  preserved, same commit).
- **NON-STATE** — the reference is historical/attributive ("found by the B24
  composition inventory", "the rotation twin of the B19 position-freeze"): it asserts
  a past event, not a current state, so a state move cannot falsify it. Recorded as
  its own class rather than folded into COHERENT, because calling a provenance
  sentence "coherent with the row's current state" would misstate what was checked
  (the F59 OFF-AXIS precedent).
- **NOT-A-ROW** — the token is not a §13 row reference (the measured namespace
  collision).
- **AMBIGUOUS** — the reference admits more than one honest reading; both listed,
  never resolved by guess (§5.2's class).

State DECISIONS implied by any verdict (a row that should close or move) are
**ROUTED to the supervisor, never enacted** — flips are the supervisor's, and §13
rows are not edited by this task at all (task boundary).

## 7. Accepted incompleteness (stated so the bound is honest)

- a reference made in prose without the id token ("the ledger row that owns the
  composition grammar") is invisible to the detector;
- §12 rows (`S4`/`S6`/`S7`/`S8`) and §6 decisions (`D9`/`D15`/`D21`/`D23`) cited from
  §5 texts are an ADJACENT axis, sized by `--adjacent` and reported, not verdicted:
  the mandate's axis is §5 ↔ §13;
- the target-state resolver reads the §13 row's own current text; a §13 row whose own
  text is stale would propagate that staleness one level up — named here, not solved.

## 8. Universe as enumerated (before adjudication)

- FORWARD: **162** references, **83** distinct (source, target) pairs;
  in scope (excluding `ARCHIVE-ENTRY` sources): **145** references, **73** pairs.
- BACKWARD: **111** references, **61** distinct pairs, from live §13 rows to §5 ids.
- ADJACENT (sized, not verdicted): **57** distinct (§5 source, `S`/`D` id) pairs.
