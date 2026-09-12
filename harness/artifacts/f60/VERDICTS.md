# F60 — the §5↔§13 verdict table

Machine table: `f60_verdicts.tsv` (one row per reference, both directions, with the
ground). Coverage is verified by SET IDENTITY against the enumerated universe, not by
count: forward 73 / 73 and backward 61 / 61, `enumerated − verdicted = ∅` and
`verdicted − enumerated = ∅` both ways (`f60_coverage.log`).

## Result

| direction | pairs | COHERENT | NON-STATE | NOT-A-ROW | **STALE** | AMBIGUOUS |
|---|---|---|---|---|---|---|
| §5 → §13 (forward, in scope) | **73** | 32 | 21 | 15 | **3** | 2 |
| §13 → §5 (backward) | **61** | 37 | 19 | 2 | **2** | 1 |

Excluded and sized, never adjudicated: 17 references from `ARCHIVE-ENTRY` sources
(10 distinct pairs) — retired history, see METHOD.md §2.

## Why so few references are stale: the row id is OVERLOADED

The single finding that organises this table. A §13 row id names two different things:

- a **QUEUE** — work the row will do. A reference to the queue goes stale the moment
  the row closes, delivers or delegates. (36 references: 18 forward `QUEUE` readings +
  the backward state claims.)
- an **ARTIFACT** — the thing the row BUILT, which outlives it: a code seam
  (`B16`'s `body action reload`), a protocol (`B28`'s new-product-surface spelling
  sign-off, used by the owner himself on 2026-08-26 at §11.154(b), five weeks after
  the row closed), a defect class (`B15/B19/B32`, "the closed load-time-latch class"),
  a precedent (`B4`'s retire-with-old row). **18 forward references are of this kind
  and NONE of them can go stale by a state move.**

Plus 21 forward references that are pure **PROVENANCE** ("found by the B24 equivalence
A-vs-A control") — a dated event a later state cannot falsify.

So the axis's real denominator is the QUEUE readings, not the 73: **3 of ~19 queue
references are stale**, and every one of them names a row that DELIVERED.

## The three STALE forward references (annotated at their nodes, same commit)

1. **§5.24 → B32** — `INTENT/5.24.md`. The entry's header reads **"OPEN → ledger
   B32"**; B32 is **ROW DONE** (recompute-at-use LANDED 2026-07-24, code `2912d303`,
   §11.93; archived 2026-07-31, §13.C: *"spin freshness — row done"*). The file is
   ALSO internally inconsistent: the retired index line appended at its own foot
   already reads *"~~OPEN → ledger B32~~ **FIXED by B32 (2026-07-24, …§11.93)**"* — the
   DERIVED view is fresher than the AUTHORITY, and `intent_pair_check.py` cannot see it
   because this id has no live stub to pair against (it counts as "archived-in-place").
   §11.163(c)'s shape, exactly, with the evidence sitting inside the same file.
2. **§5.2 → B10** — `INTENT/5.2.md`. *"B10 must therefore NOT remove the base-altitude
   workaround"* — a constraint addressed to a row that has since DELIVERED (**DONE
   2026-07-22 → §11.71**, D12-ratified §11.79(f)); the residual it protects now lives at
   §5.18 (floor VALUE suspended for Vixy). Whether the workaround survived that landing
   is a source question this record-only sweep did not take.
3. **§5.44 → B39** — `INTENT.md` §5 register (inline stub, no entry file). *"The row
   exists so B39 does not 'port' the old behaviour…"* — B39 is **DONE 2026-07-30
   (§11.117)** and its own §13 row states the outcome: *"§5.44 impossible here"*. The
   guard's purpose is discharged; the OLD-path defect record is untouched by that
   (§11.52(b): old is the frozen baseline, tracked-not-reproduced).

## The two STALE backward references (ROUTED — §13 rows are not edited by this task)

1. **B15 → §5.32** — B15's `[UNDER-SCOPED 2026-07-24, §11.101(d2)]` block ends
   *"Reference half closes with §5.32's one-site fix; the invisible-CHILDREN half does
   not"*. §5.32 **CLOSED 2026-08-01** (F20, §11.128(a)(a2), code `b4105a83`) — the
   reference's spin and reach ARE refreshed at the top of `Camera::update`. B15 carries
   no later block. What is owed is the delta, not the closure: B15's *invisible-children*
   half is what remains.
2. **B39 → §5.46** — *"`publishParkedFrame` keeping the parent frame the up-chain never
   caches (→ §5.46)"*. §5.46 **FIXED 2026-08-09** (F29, §11.139, code `b78d6549`): the
   up-chain now assigns `matLocalToBodyPos` at both sites, so the relative clause is
   false at HEAD.

## The three AMBIGUOUS references (readings listed, NOT resolved)

- **§5.46 → B39** — *"collapsing it is B39-barrier work"*: (a) a routing to B39, which
  is DONE and cannot take it; (b) work that touches the barrier B39 built, which
  persists. The row itself is FIXED, so nothing depends on the choice today.
- **§5.89 → B1** — *"the RING/asteroid line (B1/S4)"*: (a) the architectural line named
  by its head row (`fable-dispatch.md` §2 groups *"B1/S4 + B2 + riding rows"*);
  (b) an imprecise cite for **B2**, which is the row that actually carries the RING
  asteroid item. Both rows are live and blocked together, so no state move is implied.
- **B14 → §5.28** (backward) — a pre-2026-08-04 citation, so by §11.179(i)(M4)'s dating
  rule it means the **archived** §5.28 (`rot_pole_w0` 90° axis, A33's referent), not the
  live Translator row. **F59's acceptance placed disambiguation notes at three homes —
  all three on the §5 side. This §13-side citer is not covered.**

## The seven namespace collisions the detector cannot resolve (all measured here)

Any future instrument on this axis inherits every one of them:

| # | collision | specimen |
|---|---|---|
| 1 | §11.73's B27 **hardcode-key labels** A1…A9 / Tier-A / Tier-B | §5.5's entry, 9 hits |
| 2 | `b31-design.md` §2's **state-inventory rows B1–B75** | §5.63's *"§2 row B19"*, and *"the B10 line of f20_session.py's scene A"* |
| 3 | **harness leg ids** A1–A5 | §5.50's *"b24_screen stays green A1–A5"* |
| 4 | **artifact probe keys** | §5.87's `[…f35_result_post.json B3]` |
| 5 | **program output** — star spectral types | §5.88's `'Type spectral : B9p'` |
| 6 | `b31-design.md`'s **own §5.N sections** vs INTENT §5.N | B31's row: *"§5.3's annotation mechanism"* = b31-design §5.3 *Inline annotation (O7/O8)*, NOT INTENT §5.3 (`notableBody`) |
| 7 | §11.163's own verdict table numbers its members 1–13 | (detector-adjacent, no live hit) |

Collision 6 was raised by me as an ID-SLIP finding (*"§5.39 meant, §5.3 written"*) and
**refuted by reading** `b31-design.md`'s section list — the third over-call this sweep
caught in itself (see §11.180(f)).

## Adjacent, measured, ROUTED — archived §5 entries read by LIVE §13 rows

The pass-1 archival criterion is *"a closed entry/row that no open row, pending decision
or scheduled work **reads**"* (`INTENT.md:17`), and each archived body repeats it
per-unit (*"closed, read by no open row/decision"*). Measured today, **7 archived §5
entries are read by live §13 rows**, 6 of them by rows that are OPEN:

| archived §5 | read by | row state |
|---|---|---|
| §5.29, §5.30, §5.33 | **B3** | OPEN |
| §5.37, §5.39 | **B31** | OPEN |
| §5.40 | **A30** | OPEN |
| §5.31 | **B39** | DONE (live row) |

Two honest readings: the criterion means *depends on* (these are historical
causes/preconditions, so the assertions stand) or it means *cites* (the assertions are
false for these seven). **Not resolved here** — the archival convention's reading is
the supervisor's.
