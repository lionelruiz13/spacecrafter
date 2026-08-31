# PENDING_LEDGER — 2026-08-31 (round-3 compilation session, Claude Fable 5)

**Contract** (per the §11.116 precedent, `PENDING_LEDGER-2026-07-30.md`):
this is a holding buffer written while the F68/F69 executor session is
live on this tree — INTENT.md and its entry files are that session's to
touch. The supervising session MERGES this into the ledger at a
quiescent point and DELETES this file in the merge commit. Nothing here
is new work; it is in-conversation Vixy testimony + its derived
consequences, received by the round-3 compilation session (commits
`626c4b1` → `59052d1` → `ef6adcc` → `7285dfc` → `0b5378a` → `3d95c82` →
this one), recorded before they can go stale. Every [derived] carries
its veto point.

## 1. Routing directives [vixy 2026-08-31, in-conversation]

*"Parts of the last part shall land to him, in case it hold defects
(heading is not used by many)"* + *"Implicitly resolvable where
astronomical grounding resolves it, otherwise route to him."* — the
round-3 "Not included" section's routing filter, partially exercised by
the owner. Enacted in `USER_QUESTIONS_ROUND3.md` (still DRAFT, NOT
SENT): §11.4(2) → R27, §11.92(d) → R28, NEW R29 (see §3). The final-pass
list (ledger-owned per §11.116(c)) grows by these three; the ledger's
copy of the list should be updated at merge.

## 2. §11.4's two decisions — one resolved, one routed

- **(1) RA zero point: RESOLVED by astronomical definition** [derived,
  under the owner's explicit delegation above; veto point: if the
  readout was ever meant as something other than standard RA]. RA = 0 at
  the vernal equinox is definitional — certainty class (i),
  dependency-free, no convention freedom in any catalog or epoch frame.
  The new path's constant −90.0003° offset (§11.158(f)) is therefore a
  frame-construction defect (RA measured from an axis 90° away), not a
  choice. Root the correcting gate at CITED CATALOG values of known
  bright stars (§11.51(d) red line: never from recall), not at old-path
  parity alone (verification height: the terminal observable is
  catalog agreement; old is cross-check). The ~1 arcsec residual beyond
  the 90° must be ATTRIBUTED during the fix, never absorbed (residuals
  are axes).
- **(2) origin (observer- vs body-centred): routed to the tester = R27**,
  framed as topocentric (pointing number) vs geocentric (catalog
  number). Cross-check recorded: the ledger's measured "~1° on the
  Moon, invisible elsewhere" IS the Moon's parallax (~57′) — confirming
  the two halves separate exactly on the definitional/conventional
  line. Old = observer-centred = inherited, no recallable intent
  (§11.161(d)).

## 3. A43's premise REFRAMED by owner testimony — fix gated on R29

[vixy 2026-08-31]: the main tester deliberately authors **distinct
normal/miniature skins** to give a body two appearances (far vs close)
— a misuse of the LoD/miniature system (*"intended for miniature, not
for redesign"*), unreliable (keys on apparent size: far-but-zoomed
shows the wrong face), and Vixy has already warned him it can break at
any time. Consequences:
- **A43** ("Sun/Moon previews don't match their full maps — regenerate")
  assumed the mismatch is a slip. The tester is the data's principal
  author (§11.161(c1)) ⇒ the mismatch may be his DESIGN; regenerating
  would destroy it. **A43's fix direction is now GATED on round-3 R29**
  (slip-or-design, per body). A42's swap-distance answer feeds from the
  same answer. Row annotations owed at A43/A42.
- **[script-trigger]'s use case materialized**: a trigger on
  distance/visibility change + the existing `body … skin_tex`/`skin_use`
  swap (grammar-verified present) is the supported replacement for the
  hack. Recorded at FEATURE_REQUESTS (commit `3d95c82`) with the `*`
  wildcard refinement (special name `*` matches every running script)
  and the [parallel-script] prerequisite chain (commit `59052d1`).

## 4. §11.92(d)/B17 — premise corrected, old-as-spec weakened, a lead

[vixy 2026-08-31]: *"His dome doesn't use it, but someone else's dome
did and… there were already some long-standing fixes about it at some
point, not sure it's really clean now though."*
- **Premise correction**: the main tester's OWN dome does not use the
  view offset; ONE other installation's tilted dome drove the feature.
  R11's *"it can change during a show"* re-scopes to field knowledge,
  not his install. R28 rewritten accordingly (knows-or-expects basis,
  with the R13-style exit: "no dome I know uses it" downgrades the item
  to an engineering call).
- **Old-as-spec WEAKENED for this surface** [derived]: §11.52(b)'s
  parity is conditioned on old being exact; here the owner testifies
  old's offset behavior is the end state of accumulated fixes whose
  cleanliness he doubts — the old behavior may be accident, not intent
  (same class as D28's roll-rebuild-is-convention). Caveat lands on
  B17's shape-(1) "reproduce old exactly" (D6): the answer stands, but
  what "old exactly" is worth there is now qualified. Annotation owed
  at §11.92(d)/B17.
- **Named lead, not run**: the long-standing fixes are in git — an
  archaeology pass over the old view-offset fix history
  (navigator/projector territory, §11.161(d)'s maximal-struggle cell)
  could reconstruct what the fixes were converging toward, i.e. the
  requirement, independent of anyone's memory. Candidate task,
  supervisor's queue.

## 5. Interface rule candidate (final-pass protocol)

[vixy 2026-08-31]: a proposal item must lead with its visible interest
— what it enables, simplifies, or removes from the reader's hands; a
change whose interest is not visible is RIGHTLY dismissed; the cost
must not look unbalanced; habits change hardly unless worth it.
Generalized into `USER_QUESTIONS_ROUND3.md`'s header as the eighth
format rule beside §11.177(i)'s seven. Candidate for the ledger home of
the final-pass question-shape rules (§11.173(d) neighborhood) at merge.

## 6. Provenance note

The three script requests ([parallel-script]/[script-binding]/
[script-trigger]) are VIXY's, not the tester's — FEATURE_REQUESTS.md
mixes both sources, and its header still scopes the file to requests
"from outside the development process itself", which no longer matches
its contents (flagged to Vixy 2026-08-31, his call: amend header or
split log by origin). R25 was corrected to attribute explicitly
(commit `ef6adcc`).
