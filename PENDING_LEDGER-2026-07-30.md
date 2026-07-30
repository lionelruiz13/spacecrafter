# PENDING_LEDGER — 2026-07-30 (holding buffer, supervising session 3)

**Purpose:** Vixy answers received while the F10 executor is live on this shared tree —
held here to avoid concurrent `INTENT.md`/`DECISIONS_PENDING.md` writes; merges into a
§11 entry at the next quiescent point (F10 delivery review), then this file is DELETED.
Until merged, this file records the authority statements. [Authority: Vixy's words
verbatim where quoted; everything tagged [derived] carries a veto point.]

## 1. D15 + D21 — scheduled, not stalled

[vixy 2026-07-30]: worked "on the last week of august" — "I should have optimal time
and attention at this date. Working on those earlier risk rushing it"; "those questions
requires deep reasoning (the cached reasoning didn't answer them, the first reasoning
pass didn't complete it under the high-reliability regime when tried)."

- Consequence [derived, checked]: no dispatched or queued task touches either — F10–F12,
  B31-impl, B14/D22 all avoid them by construction; B18 residuals + D21-gated mandate
  scenes stay parked through August with no downstream task starving.
- reasoningCache empirical datum [derived]: D15/D21 are correct cache MISSES — the cache
  reported no coverage rather than feigning it; consistent with the cacheability-filter
  property (un-derivable-yet ⇒ no entry, never a wrong entry).

## 2. §11.98(c) "oort too early" — datum answer

[vixy 2026-07-30]: "I thought I saw the oort while the system was still quite visible,
but it might also have been due to zooming, I forgot about this aspect and I don't know
which actions you took at the time it happened - one variable I forgot to trace, that I
will then never know if it was it or something else - or it was my expectation which
was wrong."

- Three live hypotheses: (i) genuinely early oort; (ii) zoom-induced; (iii) expectation
  wrong. The discriminating variable (zoom/fov state at perception time) was untraced.
- Channel status [derived]: Vixy-memory channel EXHAUSTED; supervisor-side channel NOT —
  session transcripts + committed harness scenes persist on this host. Asymmetric
  discriminator: zoom/fov commands ABSENT across all candidate sessions ⇒ (ii) refuted;
  PRESENT ⇒ (ii) merely stays alive (Vixy's perception timestamp is unrecoverable, so
  presence cannot convict).
- **SWEEP COMPLETED 2026-07-30 (Explore agent; accepted on the report's own both-ways
  detector evidence — its zoom detector DID fire on separate Moon/eclipse launches and
  on doc-text, correctly classified — not re-run by the supervisor):**
  - 8 candidate sessions drove an oort scene in 2026-07-15..25; **zero fov/zoom commands
    during oort visibility in ALL of them**; FOV pinned once pre-launch
    (`init_fov=340`, `b5_oort_run.sh:18`/`b5_ladder_run.sh:23`) and never touched; the
    only variable moved during oort visibility is altitude (`moveto altitude`).
  - **No trace gap**: transcripts retained continuously back to 2026-07-11; the oort
    harness was born 2026-07-24 (`36eeffd`), so no earlier oort scene existed on this
    host — the candidate set is closed, not sampled.
  - **The originating observation was RECOVERED verbatim** (`f783687d…jsonl:772`,
    2026-07-24T06:30:55Z, typed): *"oort are currently randomly generated if I'm not
    wrong, making them psuedo-random (using a frozen random seed) won't make a major
    difference but enable testing for non-regression when going further, taking
    screenshot at each power of two of the distance in Mm, starting from 1Mm of
    distance (anchored on earth) in both path, and compare pixels between captures of
    same-distance. I think the oort shadow are showing too early so maybe there is
    other things wrong as well to look for."*
  - Reading [derived]: "(anchored on earth) 1Mm powers-of-two" is the SPEC of the
    ladder being requested (b5_ladder, Earth-anchored, ran 07:01Z — 31 min after the
    prompt), NOT the watched configuration; the observation sentence itself is
    configuration-silent. The ONLY oort-displaying run on this host before the prompt:
    **04:29Z `b5_oort.py` — free_mode, Sun-referenced, fov 340 pinned, altitude
    ladder, dual dumps** ⇒ reconstructed observation configuration = that run.
    **Precondition [veto, one word suffices]:** the observation came from this host's
    session-driven output (supported by Vixy's own "I don't know which actions you
    took"); an independent run on another install would void the reconstruction.
- **Hypothesis state after the sweep** [derived, under the precondition]:
  (ii) zoom — **REFUTED** (fov pinned, zero zoom commands, closed candidate set).
  (i) genuinely early — **STILL LIVE and NEVER TESTED in the observation's own
  configuration**: §11.98's refutation was measured anchored-Earth; the observation's
  reconstructed config is free-mode/Sun-ref, which that refutation does not reach.
  (iii) expectation wrong — still live. Note the verbatim says "oort SHADOW … showing
  too early" — the shadow qualifier was dropped by the later paraphrase ("oort too
  early") and must survive into the tester item (test what was observed, not the
  paraphrase).
- Class-level [vixy]: the tester reviews this in the FINAL test pass before testing
  deployment. **Sharpened by the sweep**: the tester item is oort-SHADOW onset in the
  free-mode Sun-referenced altitude-ladder configuration (anchored-Earth already
  measured and refuting), state-stamped per the rider below.
- Prevention-of-class rider [derived, veto]: the tester-pass protocol must STAMP STATE
  at the moment of each subjective judgment (fov, anchor, active flags, scene id — a
  one-command state dump) — else any divergent tester report reproduces this exact
  ambiguity (untraced variable behind a perception). Design input for the tester-pass
  harness; NOT built now.

## 3. Tester batching — standing principle

[vixy 2026-07-30]: tester items accumulate "so that everything is tested by the tester
in the least amount of passes" — one final test pass before testing deployment.

- Rider [derived, veto]: the A15 re-ask (listed "sendable now" in fable-dispatch §3)
  JOINS that final pass instead of being sent standalone; the §3 line is corrected at
  merge time.
- At merge time: check whether `USER_QUESTIONS*` already serves as the tester-pass
  aggregation channel; if not, ONE consolidated tester-pass list should exist (I2) —
  current known members: A15 (§11.82 fade), §11.98(c) class (oort onset), B10 floor
  feel-test is VIXY's not tester's (§11.79(f) — keep the two lists distinct).
