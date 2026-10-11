- **Session-31 decision items (2026-09-18, Friday evening — the DEPLOYMENT-MAP / DUAL-CAMERA-SEAM round: F120 · F121, two for two delivered and supervisor-verified same session; you were PRESENT: four messages, verbatim at §11.244, and the round was re-cut three times on them). Q-70's shape: one decision per node, its held set at the node, anchors by id and symbol. The executors' own veto lists are §11.243(k) and §11.245(k); read order for Saturday is [F1].**

  ```
  YOURS — the seam (F121 → §11.245; what your tester saw, measured per frame for the first time)
  [Y30] §5.157 · THE GO-TO ─ after `internal/fly_to_selected.sts` (WIN+^) the two engines stand 156 000 km apart
        (93.52 deg of longitude around the Moon): the Moon is on screen on BOTH paths, against skies 91.79 deg
        apart. A PLACE error, attributed by elimination to `camera action move_to target body` — the two travel
        registries; neither `transition_to` moves an eye (they move the READOUT: +90.000000 deg at a point
        anchor). NOT fixed: it crosses the anchor layer. My candidate, next round on your silence: the shape of
        the fov fix — the Camera's place FOLLOWS the old observer during old-driven travels  → §11.245(e)(f)
  [Y31] §5.156 · TRACKING ─ `flag track_object on`: old SNAPS, the Camera eases — a 56–65 deg transient over
        ~1.4 s. Identical-first says snap (one token); your D15(c) says *"Continual tracking must be preserved
        and smooth"*. Two of your sentences, opposite ways. Snap now and smooth in the CLEAN phase, or keep the
        transient as a named divergence                                                        → §11.245(h)
  [Y32] §5.155 · `look_at azimuth` ─ 107.66 deg apart AT REST, never recovered: two azimuth conventions (old:
        (-1,0,0) turned by Z(-az); Camera: az = -lng of its forward). Which one `look_at` MEANS is yours; the fix
        is then one conversion with one home (the §5.101 precedent)                            → §11.245(h)
  [Y33] §11.52(b) AGAINST "identical first" ─ reported at §11.244(f), applied by F121 as proposed: an ALREADY
        accepted divergence is NOT reverted, it enters the identity test as a PREDICTED residual (old's 5 s
        heading ramp after `transition_to target body`, §11.141). Say if accepted divergences should instead be
        reverted for the identical phase                                                       → §11.244(f)
  [Y34] the fov fix's shape (`cfea62e7`) ─ the OLD projector is the single fov authority, the Camera's zoom
        interpolator bypassed, not deleted. The CLEAN phase — authority moving INTO `Camera` — is yours, untouched
  [Y35] the recorder (`923823b6`) ─ armed by the environment variable `SC_SEAM_RECORD`, read through the existing
        `body action dual_dump`; no command, no config key; ONE line in `executor.cpp` (outside the task's
        listed files, argued: the only instant both engines have advanced for the same frame — I endorsed it)
  [Y36] `camera action transition_to target point` IGNORES its `name` (hard-coded `temp_point`) — your shipped
        script passes `name Space`, which does nothing. Honour it, or document that the command owns the name

  YOURS — the map (F120 → §11.243; claude/DEPLOYMENT-MAP.md — its §0–§4, about 340 lines, are the acting surface)
  [Y37] Q2 · does the DEVELOPER's criterion (*"workable … could disrupt work there"*) inherit the TESTER's
        both-paths exclusion (§11.163(h), derived from *"nothing he must ADAPT to"*)? Never ruled. 48 open rows
        are carried under BOTH readings until you say                                          → map §0 · §11.243(b)
  [Y38] Q1 (which criterion "deployment" names today: R0 / T0 / both, R0 first — the file is written for the
        third) · Q3 (*"once ready"*: both gates the map set have RUN; nothing instrument-shaped is in front of
        the word) · Q4 (`R1` has THREE meanings — the map's tier R, the round-2 replies, the ledger's roots —
        and minting R7–R11 widened the overlap; a rename is yours, priced at eleven citations)
  [Y39] [V1] the map is 732 lines, not shorter ─ the executor argues the tiers are the derivation that keeps the
        three WHO-ACTS lists challengeable. *"Focused"* was your word: say if the tiers should leave the file

  YOURS — the environment (you are its owner; each of these reached you as a report, none was absorbed)
  [Y40] the canary's COVERAGE ─ a release upgrade reached it through four pixels of Remmina's chrome; it has no
        member for the release, kernel, toolchain, interpreter or a soname, and the driver move
        (580.159.03 → 580.178.04) is recorded and never gated. Queued: those members + the driver gate. Veto if
        you would rather keep the fingerprint small and the side effects sharp (your message 2 can be read so)
  [Y41] the STACK ─ classic SDL2 2.32.10 (your install), NOT the SDL3-based `libsdl2-compat`; the band held on it
        12/12. Say the word if the compat stack is the one to bank
  [Y42] Q-83 / Q-84 (~/shared) ─ two questions back each, premise-shaped: is *"information maximisation"* your
        name for the generator behind several recorded practices or for this loop; is *"the oracle is spent by
        the first deliberate divergence in its region"* YOUR reason for identical-first, or mine

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] the REBUILD on 26.04 (fresh configure, the old build directory MOVED to /home/claude/sc-build-pre-26.04,
       not deleted); the re-bank on your ruling, the old geometry struck
  [V2] archival pass 23 at open (update-s29 + F117/F118/F119, byte-exact)
  [V3] F120 to an EXECUTOR and not the supervisor (INT-8: the map is the assessor the queue is read against);
       the predecessor archived WHOLE-FILE once, per-id from the new form on
  [V4] F121's two engine commits; three const getters on the old navigator (0 removed lines); the three STOPs
  [V5] the four stale entries F120 found are QUEUED, not repaired (the round was re-cut under them)
  [V6] this block

  HELD OPEN, not absorbed
  [H9]  THE NEAR-BODY DISAPPEARANCE ─ NOT REACHED. No leg, no lever, no attribution: a hole, not a null
  [H10] every key, mouse and joypad input is UNMEASURED by the recorder's legs (the TCP channel carries
        commands, not SDL events) — and your tester drives with them. The fov fix covers the wheel and
        CTRL+Up/Down BY CONSTRUCTION (same authority), not by measurement
  [H11] §5.158 ─ five OLD-ONLY transitions (`camera action rotate`/`flyto`, `align_with`, `camera action load`,
        `set heading default`, `mode jump`'s 10 s quaternion): found, unmeasured
  [H12] one transient UNATTRIBUTED: position jumps to 1.0038 AU for ~0.9 s at `transition_to target point`
        inside the script, and reads 8.8e-08 AU when the same command is isolated — state-dependent
  [H13] the session-30 list, unchanged: [H1] the DSO slices on your reading · [H2] §5.154's fix · [H3] the fold
        leg · [H4] B41 · [Y17]–[Y29]

  FACTS, no decision asked
  [F1] SATURDAY READING ORDER, by commit: `cfea62e7` (the smallest commit, the biggest before/after) → the two
       frames `claude/harness/artifacts/f121/script_pre/end_oldpath.png` + `end_newpath.png` side by side (your
       tester's report as an image) → `923823b6` (`struct SeamStep` in `core.hpp` first) → §11.245 (e)(f) →
       `harness/artifacts/f121/transitions.tsv` (18 of its 30 rows say "not measured": the map of what is left)
  [F2] the engine BUILDS AND RUNS ON 26.04 LTS with no source change (ffmpeg 8, cmake 4.2.3, gcc 15.2.0) — the
       intern is on Linux; `glslangValidator` is an optional missing component; `FindFFmpeg` prints "3.3"
  [F3] the class your tester reported in seconds was ALREADY in the ledger as four separate rows (B34, §5.153,
       §5.68, §5.26/B33) that no map carried until F120's sweep, the same hour. Every parity instrument here
       measured states AT REST; the canary DWELLS. Q-84 holds the guard: enumerate TRANSITIONS, at the dispatch
       anchor, not only states
  [F4] THIRTEEN dispatcher defects, none reaching a delivery (the list is in the session note). The pattern of
       the round: a prevention fired exactly where it had become a LINE THE GATE RE-RUNS (the control beside a
       zero-expectation premise) and nowhere it was a thing I meant to remember — including forty minutes after
       writing it down. Two executor defects reached the record (twelve marks dated the 19th; an analyzer mode
       that crashes on the committed dumps); both corrected or queued at acceptance
  [F5] executors' commits carry `Co-Authored-By: Claude Fable 5.1` under author `Claude Opus 5` (the harness's
       attribution line reaches the subagent); mine carry the author field alone. Your tool dropped "redundant
       self co-authors" on its last run — say if the executors' line is one of those
  ```

