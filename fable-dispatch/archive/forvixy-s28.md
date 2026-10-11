- **Session-28 decision items (2026-09-11/12, the SEED-STALENESS / LOG-RETENTION / RESIDUAL-STEP round —
  F107 · F108 · F109, three for three; F109 a STOP endorsed). Q-70's shape: one decision per node, its held
  set at the node, anchors by ID and symbol, correlated items together. Nothing asked in-session (Friday
  night). Your Saturday run meets `supervised-by.sh` EXACTLY as the session-27 [Y4] below describes it —
  nothing in this round touched it.**

  ```
  YOURS — nothing here substitutes them
  [Y1] §5.149 · every iterative solver is SEEDED AT JD 0 by the constructor ─ the CAUSE of §5.145's
       [2026-09-12: the pointer was UNRESOLVABLE for you (§11.233(h)) — §5.149 is the inline row
       `149. **…**` at INTENT.md:515, no entry file; the shape question (i)–(iv) still stands]
       1.198725°; the two-step fix PAYS for it (all ten steps a use buys spent walking back 75.765 rad)
       fact    ModularBody.cpp:121 lastJD = parent->lastJD (0 before the first frame), :126 the
               evaluation; the replay on the sliced text reproduces F104's pre AND post dumps float32
               for float32; a one-line seed reset on the pre tree reads 1.06e-05° at ONE step per call
       shapes  (i) skip the ctor evaluation — measured UNAVAILABLE: isSystemCentered() answers by
               POSITION and both orbit loaders read it at LOAD time (49 records move)
               (ii) reset the seed after the ctor's call — fixes Eris, breaks Sedna by 55° (the comet
               family's first step from (H,c,s) = (0,1,0) is 44 rad; the ctor's call is what burns it)
               (iii) evaluate the ctor at a date the body will be used at (the loader has no date)
               (iv) iterate to a criterion — already written and dead in warp() (§11.225(j1))
       ask     which shape, or none: today the post-fix arm has 10 steps and this seed needs 8; a
               slower converger in an authored system re-exposes it
       →       §11.229(b)(c)(d)(h1) · §5.149 · §5.145 · §11.76(b)

  [Y2] F108 · the log window ─ four veto points, implemented-and-live, each cheap to reverse
       V1      a COMPILED constant (LOG_RETENTION_LAUNCHES = 8, src/tools/log.hpp), NOT a config key: a
               key through checkConfigIni writes into the field's config.ini (the 03fbee59 precondition
               every task asserts) and D13 would have an older build read a key it does not know; so
               D12's self-contained action is "change that line and rebuild", which the log line says.
               If you want it operator-settable, say so — the key lands in checkConfigIni and the
               precondition is re-banked once
       V2      the depth is UNIFORM: four channels that used to truncate now keep 8 launches — the field
               dir went 5 files / 194 KB → 40 files / 1.46 MB, bounded there forever (vulkan ~100 KB +
               spacecrafter ~82 KB per launch are almost all of it); per-channel depth is yours to rule
       V3      six D12 lines per launch, 2203 B, every line self-contained
       V4      a REFUSED second instance consumes one slot of eight (the opens sit 30 lines above the
               lock check); moving them below it is a main.cpp ordering change — yours
       legacy  the 34 dated script-*.log (5.8 MB on this host) are NEVER touched by the app: one line
               names them; delete by hand
       →       §11.230(c)(d)(m) · §5.115 (retention FIXED, density OPEN) · §11.173(b)

  [Y3] §5.152 · a SECOND instance ABORTS ─ exit −6 (SIGABRT): a joinable LinuxExecutor thread is
       destroyed at main.cpp:250's `return 0`; and the refusal line never reaches the console
       (setDebug runs 19 lines lower). Both binaries, pre-existing. Fix shape: join/detach before
       the early return, or route it through the normal shutdown (:399 already waits for it)
       →       §11.230(l) · §5.152 · main.cpp:248-251

  [Y4] §11.231 · the "~10 % residual step" L1 judged IS NOT ON THE SCREEN ─ two items
       ANSWERED [vixy 2026-09-12] → §11.233(g): "he was answering our 10%, if it's not visible the tester
       won't care - because he care about what the public will see" — CLOSED, no profile change, the
       tester question withdrawn
       fact    the number was b22_live_analyze.py's own (an offline px axis 1.3–1.7 % low, the
               dot's own brightening, 8-bit clipping); with the dot suppressed the interior emits
               0.12 % of the swing at the lowest in-band px; the collapse is continuous at T
       tester  ONE question to transmit: does he SEE a brightness step at the collapse, or was he
               answering the 10 % we quoted him? (his L1 answer addressed our number)
       you     whether to change the fade's PROFILE at all (linear in α, measured 91–95 %; perceived
               brightness is not) — user-visible appearance; §11.82(b)'s named lever is measured to
               make it WORSE (energy ∝ α³: a dip near T and a late rush)
       →       §11.231(f)(g)(h)(l) · §11.82(b) REFUTED · A15 · §11.207(g) item 6 refuted

  [Y5] §5.151 · Sedna's shipped orbit is not the authored one ─ data, the tester's: three malformed
       keys in one [Sedna] section (orbit_Period with a capital P → Gauss mean motion; 76,0616 with
       a comma → 76.0; orbit_LongOfPericenter without '='); forward-only (D9)
       →       §11.229(i1) · §5.151

  [Y6] isSystemCentered() answers by POSITION and both orbit loaders read it at LOAD time
       (ModularBody.hpp:1788-1800, ElipticOrbitLoader.hpp:12, CometOrbitLoader.hpp:14) — was a
       declared flag the intent? a body's JD-0 position decides which frame its children's orbits
       are built in                                                        → §11.229(d)(l2)

  [Y7] the re-bank ─ the host rebooted 20:06:16; you re-provisioned the same real session on :2 at
       20:13:30; the three per-boot members re-banked with the argument; green on 17 runs since.
       Your correction may differ (another display to bank on) — one edit
       →       HOST-EVENTS 2026-09-11 · f56_canary.sh VALUES block

  [Y8] scratch trees ─ sc-f107 (1.9 G: a git worktree at 4cd00139 + the control and three mutant
       binaries), sc-f108 (937 M), sc-f109 (365 M) join the list; `git worktree list` now carries
       sc-f107/tree beside sc-f89/tree

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] the canary re-bank (the block's own prescription for a per-boot red)
  [V2] archival pass 20, and the pass instrument written down (harness/fd_archive_pass.py)
  [V3] F108's shape under the fixed invariants: numbered rotation, the constant, the six lines,
       logread.py as the ONE reader of any cLog channel (ten drivers routed through it)
  [V4] four §5 rows at acceptances: §5.149 [Y1]; §5.150 (the orbit-line sampler perturbs the
       position seed — new path; the old path's batchLastE shape named as the fix, S); §5.151 [Y5];
       §5.152 [Y3]
  [V5] F109's STOP endorsed: no engine line moved; §11.82(b)'s attribution marked REFUTED
  [V6] two b22 instrument fixes at the root (goto convergence under a body reference; the kill by
       /proc/<pid>/exe + the config restore as an EXIT trap — the runner had killed its own caller)
  [V7] this block; supervised-by.sh and purge-path.sh UNTOUCHED by decision

  HELD OPEN, not absorbed
  [H1] the concurrent-instance probe is BLIND to staging binaries (comm == "spacecrafter" exactly,
       in f26_epoch.sh / f27_reply.py / f56_canary.sh) — measured with two live; the exe-identity
       probe is next round's first item (S); §0.5 corrected meanwhile
  [H2] §5.150's fix (a separate plot seed in OrbitModule) — S; the predicted one-frame transient
       (834 arcsec, Pasiphae, high time rates) still uncaught in the engine
  [H3] with orbit lines ON the OLD path's own moons move by up to 1.87e-07 AU through the static
       ephemeris cache — an instrument caveat at §5.84 for any parity run with the lines on
  [H4] §11.207(g)'s list carries three "decision-free" labels refuted at their rows this session
       (§5.66, §5.71, item 6) — the list is re-cut before anything is dispatched from it again
  [H5] §5.115's density half; EntityCore's own uncapped EntityCore-logs-<epoch>.txt pile (387
       files from unclean exits, read-only submodule) — yours
  [H6] dumpread.load_dump cannot read F100's pinned_*.json.gz (AttributeError, dumpread.py:157; my
       own probe at the mint) — the fourth-reader family, instrument residue
  [H7] the M flags 11.229 / 11.196 (the entry-wins shape, named)

  FACTS, no decision asked
  [F1] three for three delivered AND supervisor-verified same session, every delivery re-run by my
       own hand; code 7ceea976 → fcc277c9 (ONE executor commit, F108's three files); binary
       e411b838 → 6d63e6c1
  [F2] the host: rebooted 20:06:16, same boot since; sessions 5/6 and :2 at open and close; RAM
       53–54 GiB; 17 canary runs (one red by design, then green throughout); ~54 launches; no lock
       file at any check; the field pair in == out throughout
  [F3] thirteen dispatcher defects (5 value, 8 structure), all output-side, all caught before a
       delivery — two new sub-classes named in Q-67 (a two-range sed; a per-round value in prose)
  [F4] unpushed at close: 99 code / 857 harness (measured 01:11:13); the push is yours, Saturday
       → PUSHED 2026-09-12 11:03–11:04 by you, 0 / 0 at 11:13:30 (§11.232(b))
  ```


