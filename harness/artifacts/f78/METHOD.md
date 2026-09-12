# F78 — strict-credit v2: the METHOD, committed BEFORE any instrument edit

Task F78 (`### F78` in `claude/fable-dispatch.md`), 2026-09-01, dispatched by Claude Fable 5.
This document is the **boundary and the prediction sheet** for the one deliberate
re-baselining act over the two ledger instruments. It is committed in its own commit,
ahead of every instrument edit, so that no predicted effect can be adjusted to fit a
measurement (the F59/F60 form: *method before verdict*; `harness/artifacts/f59/METHOD.md`
is the precedent this follows).

Nothing below has been measured on a v2 instrument at the time of this commit. Corpus
counts quoted are those **already recorded in the ledger sources** (cited each time); no
count of a v2 output exists yet, and none was computed.

---

## 0. The state this stands on (the §0.7 gate, all verified before this file was written)

| premise | stated | observed |
|---|---|---|
| code HEAD / tree | `8d41fbe3`, clean | `8d41fbe3`, clean — **no code is touched by this task** |
| harness HEAD / tree | `011bbeb`, clean | `011bbeb`, clean |
| live `### F` count | 4 | 4 |
| next free §11 | 197 | free over live ∪ archive (`grep -rn '11\.197'` → 0 hits) |
| pair-check v1 | 212/187/25/95, D/D2/I/I2 35/11/87/34 | reproduced to the digit |
| scan v1 | 129/170/102 | reproduced to the digit |
| D14 gate (code tree) | PASS | PASS (961 CONVERT files, 0 non-ASCII) |
| F75/F76/F77 | delivered + accepted | acceptance blocks present in their WIP tails |

The dispatch prompt's **superseded** baselines (212/187/25/95 · 35/11/87/34) are the ones
that hold; the F78 section's own paragraph states the pre-F76/F77 values (211/186/25/93 ·
34/11/87/34) and is stale by exactly the deltas the prompt attributes.

## 0b. Two stated premises that did not survive verification — REPORTED, not absorbed

Both are **output-side labels** (§11.179(a)'s scope ruling), not inputs this work stands
on; the content each names was in hand from another source named in the same section, so
the gate PASSES. Counterfactual stated: had either been an input, the gate says abort.

1. **"the REQUIRED-root ruling (§11.165 veto (2))"** — §11.165 contains no veto and no
   root ruling (`/usr/bin/grep -n veto INTENT/11.165.md` → 0 hits). The ruling is the
   **F52 acceptance / §11.168(m)**, which the instrument's own header comment cites
   correctly (`intent_backmarker_scan.py:21-25`) and which the section's Sources line also
   reaches. Content recovered, mis-citation reported.
2. **§11.168(m)'s own closing clause is FALSE at the source**: *"`intent_pair_check.py` is
   cwd-relative and has no such trap."* It is not cwd-relative — `root = args[0] if args
   else os.path.dirname(os.path.abspath(__file__))` `[observed: intent_pair_check.py:143]`,
   i.e. it defaults to the **script's own directory**, so a run launched from an extracted
   pre-tree without an explicit root silently measures the LIVE tree: **exactly** the trap
   §11.168(m) measured on the scan, present in the other instrument, unchanged since the
   instrument's first commit (`6f231cf`, verified at `git show`). This is not a new member;
   it is the re-verification the mandate asks for, and it decides §7 below.

---

## 1. The invariant that orders every change (stated first, because it decides several)

The scan has two halves and they fail in **opposite directions**:

- **Half 1 (event detection, `KEYRE`)** too narrow ⇒ a real supersession is never asked
  about (a **silent** miss). Too wide ⇒ candidate pairs that are not events (**noisy**,
  visible, costs reading).
- **Half 2 (marker credit, `MARKRE`/form B)** too narrow ⇒ a compliant marker is filed as
  an arrear (**noisy**, visible). Too wide ⇒ a real arrear is credited and disappears
  (**silent**, and it is the failure the instrument exists to prevent).

⇒ **Widening half 2 is the dangerous direction.** Every pair that moves from unmarked to
marked under this delivery is therefore verified BY READING at its node, one by one, and
listed. Widening half 1 is cheap and self-declaring; widening half 2 is paid for in
adjudication.

Two consequences fixed here, before any code:

- **W1 — the bracketed form gets the full marker vocabulary; the proximity form does
  not.** `has_backmarker` form **A** requires a bracketed span containing BOTH the source
  citation AND a marker word: that structure is itself evidence of a marker, so the word
  list may be the ledger's real marker vocabulary. Form **B** has no bracket — only
  proximity — so it stays keyed on the supersession lexicon (`KEYRE`), which is the
  stronger word. This makes half 2's two forms deliberately asymmetric, and the asymmetry
  is the argument above.
- **W2 — the session-16 scan-owner ruling BINDS the event half only.** *"Discharge
  vocabulary stays outside the supersession lexicon BY DESIGN"*
  (`fable-dispatch/archive/update-s16.md:68`). A discharge is not a supersession, so
  `DISCHARGED`/`ANSWERED`/`DELIVERED`/`PAID` stay out of `KEYRE` — **unconditionally, this
  is not re-decided here.** They are admissible in `MARKRE` form A for the opposite
  reason: `[OWED SWEEP DISCHARGED 2026-08-26, §11.152 …]` at a target IS a §11.113(p)
  back-marker naming its source, and refusing to credit it manufactures a false arrear.
  The ruling is not weakened; it is applied to the half it was made about.

**Vocabulary rule (strict credit, literally):** a word enters either lexicon ONLY if a
**named specimen in the ledger** demonstrates it. No word is added by imagination. Every
addition below carries its specimen and its citation.

---

## 2. Member 1 — stub resolution (`intent_backmarker_scan.py`)

**Defect, two arms.**
- **1-A, the register collision** (§11.165 FOURTH-INSTRUMENT-PROPERTY note): `stub(idnum)`
  matches the first `INTENT.md` line starting with the number after the dot
  `[observed: intent_backmarker_scan.py:40-45]`, and §5's register precedes §11's, so
  `stub("11.104")` returns **§5.104's** stub. **79** §11 entry files affected [measured at
  §11.165's note]; F59 measured the mirror case (`stub("5.31")` → §11.31's line, 23 retired
  §5 numbers).
- **1-B, the one-line truncation**: `stub()` returns ONE line. A register row whose later
  content lives in an indented continuation block is invisible. Live specimen this
  session: §5.117's `MEASURED AND PARTLY CORRECTED 2026-09-01 (F77, §11.196)` block, which
  the supervisor's own one-line probe missed at the F77 acceptance. The dispatch prompt
  classes this as *"member (1)'s class"*; it is taken here on that classification.

**The change.** Parse `INTENT.md` once into its `## N.` register spans (the shape
`intent_pair_check.enumerate_pairs` already uses); resolve `stub("R.N")` to the block that
begins at the `N. ` line **inside register R's span** and runs to the next numbered line or
the next `## ` header. No home ⇒ empty string, never a wrong-but-plausible one.

**Predicted effects.**

| counter | prediction | class | argument |
|---|---|---|---|
| raw event LINES | **UNCHANGED (129)** | HARD | half 2 only; half 1 never calls `stub()` |
| candidate PAIRS | **UNCHANGED (170)** | HARD | same |
| UNMARKED | moves; **net down or equal**, `|Δ| ≤ 15` | RISKY | 1-B is monotone (strictly more text ⇒ credit can only be found, never lost). 1-A is two-signed: a §11.N target now reads its OWN stub instead of §5.N's — credit appears where §11.N's stub carries the marker (−U) and disappears where §5.N's stub was carrying a *coincidental* qualifying span (+U). §11.165 probed the false-positive path on §11.104 against four plausible sources and it *"did not fire today"*, so I expect the −U arm to dominate. |

**Refutation.** Any move in the first two counters refutes the implementation (half 1 was
touched) — the commit is reverted, not explained. `|ΔU| > 15`, or a NET INCREASE in U,
refutes the argument above and is reported as such with the pairs enumerated. Every pair
that gains credit is read at its node before the commit stands.

**Shown able to fail.** A decoy: inject into a scratch tree a marker span at a §11.N
target's **own** stub naming a source that its §5.N twin does not name. v1 must miss it
(U unchanged); v2 must credit it (U −1). And the inverse decoy: a qualifying span placed
in the §5.N line only must be credited by v1 and refused by v2.

## 2b. Member 1b — the same fact, the other instrument (I2; separable commit, veto-open)

`intent_pair_check.py` resolves a stub as `lines[i]`, ONE line
`[observed: intent_pair_check.py:104]`. That is the SAME fact — *"the stub of id X"* —
resolved a second way, in a second file, with the same truncation defect (I2: duplication
is pending silent desync; here both copies are wrong and differently so). Fixing the scan
alone leaves the desync in place.

**Deviation flagged**: the F78 section assigns member 1 to the scan's `stub()`. Taking the
pair-check's stub extraction with it is my judgment call, argued above, committed
SEPARATELY so it can be reverted alone. Its counters are the supervisor's gate, so its
predictions are stated as sharply as the scan's.

**The change.** A stub is the numbered line PLUS its continuation lines, up to the next
numbered line or the next `## ` header. Enumeration (which ids exist) is untouched.

| counter | prediction | class | argument |
|---|---|---|---|
| entry files / live pairs / archived-in-place / inline stubs | **UNCHANGED (212/187/25/95)** | HARD | the change is to a stub's TEXT, not to which stubs exist |
| **D** (stub → entry) | **UP or equal** | HARD-DIRECTION | D = stub atoms − entry atoms; a longer stub has ≥ as many atoms. Every new flag is the authority-inversion class D exists to catch (content living only in the derived view) |
| **D2** (stub strike/marker spans) | **UP or equal** | HARD-DIRECTION | more spans found in a longer stub |
| **I** (entry header → stub) | **DOWN or equal** | HARD-DIRECTION | I = header atoms − stub atoms; a longer stub covers more. This is strict credit in its purest form: the stub *does* carry it |
| **I2** (entry marker dates → stub) | **DOWN or equal** | HARD-DIRECTION | same mechanism, on dates |

**Refutation.** Any of the four enumeration counters moving; D or D2 falling; I or I2
rising. Any one refutes the change and the commit is reverted.

## 3. Member 2 — the event lexicon has no term for *this claim was incomplete*

**Defect** (§11.177, *"What survives as an observation"*): a marker can satisfy §11.161(g)
completely and be invisible, because credit is keyed on a lexicon with no term for
incompleteness. §11.177(m)'s own specimen: the marker placed at **§11.48(b)** —
*"the A17 residual list is INCOMPLETE"*.

**The change.** `KEYRE` gains **`INCOMPLETE`** — one word, one named specimen. Nothing
else. W2 holds: no discharge vocabulary.

| counter | prediction | class | argument |
|---|---|---|---|
| raw event LINES | UP or equal | HARD-DIRECTION | a wider `KEYRE` can only add keyword positions |
| candidate PAIRS | UP or equal | HARD-DIRECTION | same |
| UNMARKED | UP or equal | HARD-DIRECTION | pairs only enter the flagged set; existing verdicts are untouched |
| **the §11.48(b) specimen** | its pair appears | RISKY | if `(11.177, 11.48)` does not become a candidate pair, either the marker is not on one line with its citation inside 160 chars, or the specimen was misread — reported either way |

**Refutation.** Any counter falling. Or: the specimen fails to appear ⇒ the member's own
motivating case is not reached, which is reported as a partial member, not hidden.

## 4. Member 3 — the marker-half lexicon blind spot, and the past-participle gap

Two arms; both measured and named at §11.179(i). Arms are implemented in one commit and
**measured separately**, so each arm's delta is attributable.

**Arm (a) — `MARKRE`, the six specimens (§11.179 M1).** Every one is a genuine, dated,
correctly-placed §11.113(p) marker that half 2 cannot see: `STUB REFRESHED` (§5.2) ·
`ROOT-CAUSED` (§5.53) · `OWED SWEEP DISCHARGED` (§5.89) · `FIXED AND CLOSED` (§5.104,
§5.80) · `EXTENSION … corrected` (§5.80). Added stems, each traced to its specimen and to
nothing else: `REFRESH`, `ROOT-CAUSE`, `DISCHARG`, `FIXED`, `CLOSED`. Plus, from
§11.177(m)'s two placed markers, `TESTED` and `INCOMPLETE` — the same demonstration on the
§11 axis. **Form A only** (W1).

**Arm (b) — `KEYRE`, present tense (§11.179 M2).** `§11.130 → §5.63`'s source line heads
*"THREE THINGS §5.63 SAID THAT THIS RETRACTS OR CORRECTS"*; the five lexicon words are
past participles only, so a MARKED pair carrying three covered claims is invisible.
`KEYRE` gains the `-S` forms of its own five words: `SUPERSEDES|REFUTES|CORRECTS|RETRACTS|
WITHDRAWS`. No new concept enters — only the other tense of words already ruled in.

| counter | arm | prediction | class | argument |
|---|---|---|---|---|
| LINES / PAIRS | (a) | **UNCHANGED** | HARD | `MARKRE` is half 2 only |
| UNMARKED | (a) | **DOWN or equal** | HARD-DIRECTION | credit can only be gained; **each gain read at its node** (§1) |
| LINES / PAIRS | (b) | UP or equal | HARD-DIRECTION | wider `KEYRE` |
| UNMARKED | (b) | two-signed | RISKY | new pairs arrive unmarked (+), and form B of half 2 also widens, which can credit existing pairs (−). Net unpredicted; both components reported |
| **the §5.63 specimen** | (b) | `(11.130, 5.63)` appears and is MARKED | RISKY | §11.179 M2 says the pair is marked and invisible; if it appears UNMARKED the specimen's own verdict is wrong and that is reported |

**Refutation.** Arm (a) moving LINES or PAIRS; arm (a) raising UNMARKED; arm (b) lowering
LINES or PAIRS. Any credit gained under (a) that reading shows is NOT a genuine marker
refutes the word that produced it, and that word is removed.

## 5. Member 4 — the STALE/keyword miss

**Defect** (§11.180(l), the fifth member): F60 predicted a scan move, measured none, and
the reason was that **`STALE` is not in the lexicon** — every refutation in §11.180 is
written lowercase-bold and its three placed markers open `STALE ROUTING`. Two independent
invisibilities were measured there: the keyword, and §13-row targets (which are not `§N.M`
citations at all — **out of scope here**, and stated: v2 does not learn `A<n>`/`B<n>`,
because §11.180(i) measured that axis as not machine-decidable, 15 of 73 references
NOT-A-ROW from seven namespace collisions).

**The change.** `STALE` enters **both** lexicons: `KEYRE` (an assertion that a claim is
stale is a correction-class event) and `MARKRE` form A (`[STALE ROUTING <date>, §src …]`
at a target is a back-marker naming its source).

| counter | prediction | class | argument |
|---|---|---|---|
| LINES / PAIRS | UP or equal | HARD-DIRECTION | wider `KEYRE` |
| UNMARKED | two-signed, small | RISKY | as member 3(b) |
| **F60's own three markers** | still invisible | RISKY-NEGATIVE | their targets are §13 row ids; the keyword fix alone cannot reach them, and predicting otherwise would be the same mistake §11.180(l) recorded. If they DO appear, my reading of that entry is wrong and it is reported |

**Refutation.** Counters falling. Or `STALE` producing a pair whose reading shows no
correction-class assertion anywhere on the line ⇒ the word over-generates and is reported
with the count, and its removal is proposed to the supervisor rather than enacted silently.

## 6. Member 5 — the proximity false positive and the named-exception partition

This member is **a record, not a lexicon change**: no regex separates *"the §11.161 rule
this entry FOLLOWED"* from *"the §11.161 claim this entry corrects"* — §11.177 kept
`§11.177 → §11.161` deliberately as the specimen, and §11.156/§11.165/§11.176 all refused
to reword evidence to quiet a filter. The deliverable is therefore **the re-partition**
(§9), with each named exception's reason **re-verified at its node**, never carried by
label — §11.179(a)'s *"83 → 82"* is the precedent for a carried label being wrong.

**One machine-decidable reporting split is taken, and it is member 5's own named case.**
The session-18 §3(e) exception `§11.182 → §5.10` is a **namespace collision**: that `§5.10`
is scedit's `tests/derivation-diff.md` §5.10, and §5.10 has **no home in this ledger at
all** (no live register row, no `INTENT/5.10.md`). A target with no home **cannot carry a
marker**, so filing it under *"no back-marker at the target"* states something the corpus
cannot make true — F59's `NO-LIVE-NODE` verdict class, applied. v2 therefore prints a
sub-count line and marks those rows in the list.

| counter | prediction | class | argument |
|---|---|---|---|
| LINES / PAIRS / UNMARKED | **UNCHANGED** | HARD | reporting only — the headline counter's DEFINITION does not move, so the supervisor's gate keeps its meaning and the split is one line to reject |

**Refutation.** Any of the three counters moving.

## 7. Member 6 — `INTENT.md` is not read as an EVENT SOURCE

**Defect** (session-19 close note; measured at F59(b) and again at F76/F77): the scan reads
only `INTENT/<id>.md` as sources and opens `INTENT.md` solely to resolve a *target's* stub,
so a correction asserted in an inline register row is invisible as a source. F59 measured
**27 of 62** candidate lines on the §5-target axis alone to be exactly that shape. Two
dated specimens arrived this session, both predicted in advance by their executors and both
verified: **F76's §5-sourced annotations** and **F77's §11.196 → §5.115/§5.117 correction
pair** — invisible on two counts each (the uppercase-at-target lexicon, and this member).

**The change.** Half 1's source set gains `INTENT.md`'s **numbered register lines inside
the `## 5.` and `## 11.` spans**, each attributed to its own id (`5.N` / `11.N`), read as
the full block (member 1's resolver, reused — one implementation, I2). Lines of `INTENT.md`
outside those two spans are **not** attributable to an id and are skipped; the count
skipped is reported, so the bound stays honest.

| counter | prediction | class | argument |
|---|---|---|---|
| raw event LINES | **UP, substantially** (≥ +27 by F59's floor; my band **+30 … +90**) | RISKY | the register rows are single very long lines carrying whole correction histories; F59's 27 was one axis of one target class |
| candidate PAIRS | **UP, more than LINES** | RISKY | a register row is one line carrying many citations, and the 160-char window applies per citation |
| UNMARKED | **UP** | HARD-DIRECTION | new pairs enter unmarked unless the target credits them |
| the **shape** of the new unmarked pairs | **majority DIRECTION-INVERTED** | RISKY | a marker at row X naming §Y produces `(X, Y)` and then asks §Y for a marker naming X, which is not a thing — §11.165(b)'s mechanism, now reached from the register side. If the majority are NOT inversions, my model of what those rows contain is wrong and the partition says so |

**This is the member that makes the instrument noisier, and that is stated in advance.**
It buys detection (a correction asserted only in a register row is now a candidate) at the
cost of flags. The partition (§9) is what keeps it readable; if the residual becomes
unreadable, that is a finding for the supervisor, not a reason to quietly drop the member.

**Refutation.** LINES not rising ⇒ implementation bug. LINES rising but PAIRS not ⇒ bug.
A new pair whose source line is not in the §5/§11 register spans ⇒ bug.

## 8. Member 7 — the pair-check's both-homes marker test (a new counter)

**Defect**: three measured instances this session and last (§11.187's REVERSED marker
reaching the entry and not the stub — F72 acceptance, executor-caught; §11.192's SUPERSEDED
marker reaching the entry and not the stub — F73 executor's §0.7 report; the F73 acceptance
naming it a **CLASS** and queueing a pair-check extension into this package). No existing
test catches it: **D2** and **I2** compare marker spans by **DATE**, so a marker whose date
appears anywhere else in the other home passes.

**The bound (§11.180(i), applied literally).** The test asserts something only where it is
machine-decidable: **both homes are asked for the same object — a DATED marker span citing
source `§X`.** Anything softer (*"does the stub relay the claim the entry corrected"*) is a
reading, not a test, and is not implemented.

**The change.** New test **`M`**, its own counter, reported beside D/D2/I/I2:

```
markercites(home) = { X : some span S of `home` has MARK_RE(S) and DATE_RE(S)
                          and S cites §X }
M(pair)           = symmetric difference, reported as
                    entry_only = markercites(entry) - markercites(stub)
                    stub_only  = markercites(stub)  - markercites(entry)
```

| counter | prediction | class | argument |
|---|---|---|---|
| entry files / live pairs / archived / inline stubs | **UNCHANGED** | HARD | enumeration untouched |
| D / D2 / I / I2 | **UNCHANGED** | HARD | M is additive; no existing test's inputs change |
| **M** | **> 0**, and dominated by `entry_only` | RISKY | the stub is a derived summary and mirrors a marker only when it relays the superseded claim (§11.156(f)), so entry-only markers are expected in bulk. M is a FILTER like its siblings, never a verdict |
| the §11.192 specimen | present if unrepaired | RISKY | F73 reported it; whether it was repaired at acceptance is measured, not assumed |

**Refutation.** Any of the eight existing counters moving. `M == 0` ⇒ the test cannot
discriminate and is reported as such rather than shipped as a green.

**Shown able to fail.** A decoy dated marker span citing a source, injected into one home
of a scratch tree only, must raise M by exactly one on exactly that pair and move nothing
else; injected into BOTH homes, must move nothing.

## 8b. The REQUIRED root, re-verified — `intent_pair_check.py` gets it too

Per §0b(2): the pair-check carries the identical silent-wrong-tree trap the F52 acceptance
removed from the scan, and §11.168(m)'s clause asserting otherwise is false at the source.
The acceptance's own reason — *"the instrument-chain rule prefers loud-fail"*; *"a probe
that can silently measure the wrong target converts observation into fiction"* — applies
verbatim. v2 makes the pair-check's root **required**, matching the scan.

**Judgment call, flagged**: this changes the supervisor's invocation (`python3
intent_pair_check.py .` rather than a bare call), and it is one line to reverse. It is
committed separately, and `harness/README.md` carries the new invocation for both
instruments. Counters: **UNCHANGED** (input contract only, measurement logic untouched) —
HARD; verified by running v2 with an explicit root against the v1 numbers.

Because this corrects a claim §11.168(m) makes, the delivery carries a **back-marker at
§11.168(m)** (entry file and, if its stub relays the claim, the stub) in the same commit
(§11.161(g)). That marker is ledger TEXT written for the §11.113(p) rule, not to move a
counter; its own effect on the counters is measured and attributed like any other.

## 9. The re-partition (scope 4) — the LIST is the baseline, the counters its summary

After the last member, every pair in the v2 unmarked set is enumerated in
`harness/artifacts/f78/PARTITION.tsv` with **one class per pair**:

| class | meaning |
|---|---|
| `INVERTED` | the line IS the back-marker at the superseded node, read backwards (§11.165(b)) |
| `CATALOGUE` | a recording entry quoting another entry's marker (N5 by §11.156(b)) |
| `MINT-ROUTE` | *"NEW §5.104"* — a mint/route relation, not a supersession |
| `PROXIMITY` | the keyword belongs to a neighbouring claim (§11.177 → §11.161's class) |
| `NO-HOME` | the target has no home in this ledger (§5.10's namespace collision) |
| `OFF-AXIS` | source is a §5 row or a §13 row — no §11 entry to ask about (F59's class) |
| `REAL` | a genuine entry-to-entry supersession whose target lacks its back-marker — an **arrear** |

**The 13 named exceptions carried since F59/F60 are re-verified AT THEIR NODES, never by
label** — §11.179(a) is the precedent for a carried label being wrong (its own "83" was
stale by one). Sum of classes == the v2 unmarked counter, asserted by the tool, not by
arithmetic in prose.

**A `REAL` pair is a FINDING, recorded and placed by the supervisor at acceptance.** This
task does not write a marker into the ledger to move a counter — that boundary is the
section's and it is absolute. (The §11.168(m) marker of §8b is the one ledger-text write,
and it is owed by §11.161(g) for a claim this delivery refutes, not by a counter.)

## 10. Boundaries (restated so a violation is visible)

- Instrument code only. **No ledger text is edited to move a counter.**
- **No eighth member enacted.** Candidates found en route are RECORDED with their
  measurement and left for the supervisor. Two are already open at this commit:
  - **C1 — the pair-check `SHA_RE` over-flag**: `SHA_RE = r'`([0-9a-f]{7,8})`'` requires a
    BARE backticked sha, and every recent entry writes `` `master-beta @ <sha>` `` in one
    span; **9 of the 35 current D flags are this one shape** (F77's adjudication). Measured,
    not enacted.
  - **C2 — a machine-decidable inversion detector**: if the pair `(A, B)` is produced by a
    line in A's OWN home and that same line satisfies `has_backmarker(A, B)`, the line IS a
    marker at A naming B and the pair is inverted BY CONSTRUCTION. This is decidable and
    would mechanize the largest class of the residual. **Not enacted** (it would be the
    eighth member); it is measured against the v2 partition so the supervisor can price it.
- v1 preserved byte-exact as `intent_backmarker_scan.v1.py` / `intent_pair_check.v1.py`,
  md5s recorded, so the re-baselining stays A/B-able forever.
- ONE MEMBER PER COMMIT, each commit recording predicted vs measured.
- Discriminating check (a) at the end: **the v1 scripts must reproduce 212/187/25/95 ·
  35/11/87/34 · 129/170/102 on the delivery tree** — the ledger's text having moved only by
  this delivery's own entry, stub and §8b marker, whose deltas are attributed individually.
  That is the proof that the instruments moved and the corpus did not.
