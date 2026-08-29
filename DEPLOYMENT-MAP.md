# DEPLOYMENT-MAP — the space between HERE and "the main tester operates the new path, fully transparently"

**Status**: DERIVED VIEW over the ledger (compiled 2026-08-29 by Claude Fable 5 at harness
`4cebcd0`, code `d6aec251`, on Vixy's request — §11.162 records the compilation). On any
divergence the ledger (§5 rows, §13 rows, INTENT/<id>.md entries) wins and the divergence is
a staleness bug HERE. Statuses were re-extracted mechanically from the ledger at compile time
+ this session's row reads; rows marked ⚠ were NOT re-read in full at compile time — triage
them before relying. Maintenance: correct in place with dated strikes; regenerate wholesale
when the drift exceeds reading comfort.

**T0 — The criterion** [derived from the ask + D15(b)'s transparency language]: the tester
runs his existing workflows — the scripts and data he authors, TUI/keyboard operation,
search/select/readouts, saves/config, multi-hour shows — on a deployment where the NEW path
draws, and observes nothing he must adapt to; wherever behavior deliberately differs, he was
INFORMED first (the final-pass revise/revert offer, §11.116(c)). "Restricted" = his install,
his data, forward-corrected delivery (D9).

## T1 — Decision gates (Vixy's; nothing below them can close these)

Ordered by operational weight for the tester, not by age:

1. **§5.100 + §5.101 — `zoom auto in` / `zoom auto initial`** (asked 2026-08-26,
   unanswered): the two paths end **99° apart** after a shipped unzoom; `zoom auto in`
   starts tracking on the OLD path only. These are bread-and-butter operator commands —
   the single largest transparency hole with a one-line-class fix already scoped.
2. **§11.92(d) — the heading≠0 × offset≠0 coupling (B17 residual, the TILTED-DOME
   question)**: old rolls the view offset with the heading, the new path keeps it
   dome-fixed. Planetarium-geometry-central (tilted domes are the deployment reality);
   blocks the §5.66 `look_at` family and §5.71's `panView` port. NOTE: per §11.161(c) the
   *expectation* half ("what should a tilted-dome operator see") may be tester-routable.
3. **§11.96(e)(1–6) + §11.98(f)(i–iii) — the reach/visibility decoupling batch**: gates
   the B5 remainder = dso3d/tully/ojmMgr floors — i.e. the DEEP-SKY content classes on
   the new path. A planetarium show without its DSO layer is not transparent.
4. **§11.4's two numbered decisions** (RA zero point −90.0003° epoch-stable; origin
   observer- vs body-centred — origin sub-question tester-routable) → unlocks the §5.86
   fix (+ §5.19 folds in). Until then the new path's RA/DE readouts for composed bodies
   answer in a scrambled frame — he reads coordinates professionally.
5. **§5.109's layer half** — what `moveto alt` means above a display-scaled body (drawn
   vs physical surface). He authors scenes on scaled bodies.
6. **The script-semantics batch** (both-paths defects his authoring will hit; each is a
   one-liner-class fix behind a semantic call): §5.64/§5.76 (pause doesn't hold the
   clock / time runs backward), §5.65 (lock-after-move latch), §5.69 (`keep_time` 8-bit),
   §5.70 (unreplayable ramp recording — respell constrained by `delta_alt`), §5.72
   ($LOGON), §5.75 (trail drops on date jumps), §5.82 (`transition_to point name`
   dropped), §5.85 (`align_with` doesn't align), §5.87 (`constellation_star` acts on
   previous selection), §5.91/§5.93/§5.94/§5.95/§5.96 (script-surface family incl. the
   recorder diverging from the author's text), §5.53 (colour-map level jump), the ≥1023 B
   truncation-marker policy (§11.138).
7. **Forks whose owed data is now PAID, awaiting the call**: §5.88 (catalogue-load
   reporting — only caller-check names file+key), §5.89 (the one-site dead guard, must
   cover the mid-session route), §5.90 (stars.ini pairing + is `~/.spacecrafter/stars/`
   a search path — tester-routable per §11.161(c1)), A40 (quit vs incomplete frame),
   A41/A42/A43 (early-visibility gate px / texture-level switch / preview-asset data —
   A43 is a DATA regeneration, i.e. the paid product), A44 (ring shadow contract),
   §5.106 (free-flight environment: close-as-accepted vs design question), the two
   §11.144 riders (free-flight `moveto` meaning + `get status position` — defaults live,
   confirm or redirect), §5.108 (`flag_sun_scaled` dead — reviving it is a behavior
   change).
8. **B31 completeness residue**: C4's non-body catalogue key (D34's unanswered half) +
   D30's DELTA branch for `display_scale` (located, unimplemented). Session save/restore
   is otherwise functionally complete (T1 met, T3/T4/T10 green).

## T2 — Work, dispatchable now or upon its T1 gate

- **§5.111** — wrap the new path's info strings in `_()` (parity restoration; the tester
  operates in FRENCH; decision-free candidate, next round).
- **§5.110** — the owed live check (one script), then the type-filter fix routing.
- **§5.49's owed render measurement** (`f14_meridian.py` `u_sub`) — settles whether
  `moveto lon 0` stands over Greenwich, the map centre, or 90° off; the row's conclusion
  is recorded in-doubt (§11.153(k)). DATA-AUTHOR-CENTRAL: he places content by lon/lat.
  Measurement is S and unblocked; any fix is Vixy's.
- **Composed-body i18n selection asymmetry** (§11.158(d5)): `select object <translated>`
  cannot reach composed bodies while `select planet <english>` can — measure the
  consequence for a French-locale operator, then route.
- **§5.41/§5.42** ⚠ — `configuration action save` destroys comments; `camera action
  save` broken as shipped. Both-paths; he uses the TUI. Rows not re-read this session —
  triage first.
- **§5.20/§5.21** ⚠ — `linearOrbit` lerp weights swapped; `LocationOrbit` degraded spin.
  Authored-orbit-visible if real; rows not re-read this session — triage first.
- **§5.107** — extent-cache latch (5th member of the closed latch class; the fix shape
  exists).
- **§5.71 + §5.66** — the `panView` port + `look_at` halves (after T1.2).
- **B35/B36/B37 residues** — config-only / declared-but-driverless / UI-only capability
  audits: they BOUND what "the new path" can express; completing them completes the
  transparency claim's denominator.
- Already-queued hygiene (not tester-facing): b3_ladder's `922701c9` check · the five
  §11.156(g) back-markers · the §11.161(f) stratigraphy validation.

## T3 — Verify at HIS field (restricted-deployment-specific; mostly final-pass cargo)

- **Field-content family on HIS install** (tester-routable, ratified §11.161(c1)): does
  his deployment carry sky-culture content (§5.74 — search currently returns NOTHING on
  our 2922×0-byte field), stellar_systems, the full star catalogues (§5.90 — our install
  silently runs 26 561 stars instead of millions; his stars.ini↔catalogue pairing)?
- **His content census** [NEW question for the final pass]: which content classes do his
  real shows actually load? This BOUNDS T1.3's urgency and whether B1/B2 (D4 streaming,
  RING asteroid, INSTANCED) block him at all — today they are assumed architectural-only.
- **His hardware**: §5.60 (the unconditional 2.68 GB video staging allocation vs his
  GPU's limit), his real dome/projector stack vs our headless `:2` (every cadence and
  pixel baseline here is stack-local — §11.159(k7)), his `maximum_fps`.
- **His config migration**: the new keys (`attached`, `flag_lock_sky_position`,
  twin-emitted `display_scale`) measured D13-tolerant on both parser routes (§11.150) —
  re-verify on his actual config version.
- **The B14 data package** (poles/W0/periods corrections) riding the next
  `spacecrafter-data` delivery, forward-only (D9); his baselines shift accordingly.
- **His script corpus**: the shipped 434 are our proxy; HIS files are the real test —
  needs his cooperation (superscript.sts is already his own rewrite).

## T4 — Deliberate divergences he gets INFORMED about, not fixes (final-pass cargo, §11.116(c))

D15(a)–(d) INFORM ×4 (with the revise/revert offer) · D15(b) heading-stability CONFIRM ·
D37 as a QUESTION with its premise fact · A15 fade thresholds · the oort-SHADOW onset
item (state-stamped) · `transition_to body` keeps whole orientation (D28/A38 — the two
paths' images deliberately differ at that member) · the free-flight defaults (riders, if
Vixy confirms) · §5.106's environment semantics (if closed-as-accepted) · the field-content
questions (T3) · the content census (T3) · possibly §11.4's origin sub-question.

## T5 — UNMAPPED (the map's own edges; completeness > certitude)

1. **No end-to-end tester-workflow rehearsal has ever run.** The A–D battery, b24_*, and
   scene harnesses are proxies built from OUR model of operation. The closing audit
   before "ready" is a his-day-in-the-app suite: author a body, run a show, search,
   select, save, reload, quit — one sitting, new path, French locale. CANDIDATE TASK
   (M), buildable now, sharpest after T1.1/T1.4.
2. **Multi-hour soak under show load** (the stability class: §5.61 lost wakeup, §5.59/A40
   teardown, B7's intermittent §11.15d segfault, B30 frozen-scene micro-instability,
   §5.62's unattributed epoch shift) — never run. A planetarium session is hours.
3. **B38's residual state** ⚠ (dead tokens + reachable-but-defective handlers) — the
   command-surface sweep's defect row; verify what remains open at the row.
4. **Joystick/hardware UI path** (B37 territory) — no hardware here; untestable until his
   field.
5. **Older ⚠ rows never re-read this session**: §5.5, §5.35, §5.36, §5.53, §5.56, §5.83,
   §5.84, §5.98 — status OPEN by marker; triage into T1/T2/T4 or close.

## Explicitly NOT blocking (so the exclusion is challengeable)

- **B8 old-path removal** — transparent operation KEEPS old present (it is the baseline).
- **B1/B2/B3 architectural lines** — unless T3's content census says his shows need those
  classes; today they are Vixy-paced by design.
- **Perfect parity on interactive free-flight residuals** — ≤2.3 m/toggle accepted with
  structure (§11.154(a)); the usage-path model covers the tester's interactive use.

## The critical path, compressed

Vixy answers T1.1 + T1.2 + T1.3 (operator basics · dome geometry · DSO content) and the
T1.4 pair → two or three dispatch rounds burn T2 → the final pass fires carrying T3+T4
(one batch, state-stamped, now including the content census and the field-content
questions) → the T5.1 rehearsal + a T5.2 soak gate the word "ready". The decision batches
are the long pole; every measured datum they were waiting on is, as of session 14, PAID.
