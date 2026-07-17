# Dual-path projection trace harness (INTENT.md 11.14)

Purpose: replace eye-based A/B comparison of the two body paths with numeric,
scriptable comparison - the Moon-divergence investigation is the first client;
every future port keeps it as regression infrastructure.

Why it is trustworthy by construction: `SSystemFactory::update` feeds BOTH
paths from the same `timeMgr->getJDay()` every frame [ssystem_factory.cpp],
and the script freezes time (`timerate rate 0`), so the 1s A/B draw toggle
cannot make one path's draw-side state stale - both settle at the same jd.

## Parts
- `dual-dump.sts` - deterministic scene script (fixed jd, frozen time, surface
  observer, two samples at different dates). Install as
  `<scriptDir>/fscripts/startup.sts` (played automatically at launch,
  app.cpp:589) or play it manually.
- `body action dual_dump filename <path>` - the command (commandBody ->
  Core::ssystemDualDump -> SSystemFactory::dumpTracePaths). Emits JSON lines:
  header (jd + Camera::dumpTrace) then per old-path body: `Body::dumpTrace`
  (double mat, pre-convert) + matching `ModularBody::dumpTrace` by english
  name (`null` when absent - a finding in itself, INTENT 11.3 class).
- `analyze.py <file>` - per-body dEcl/dPos/dDist/relative-rotation table +
  discrimination: identical R_rel across all bodies = E3 (observer-frame
  convention); branch-localized divergence = E2 (per-hop element);
  significant dEcl = E1/E6 (ephemeris/data inputs). See
  projection-paths.md C8/C9 for the condition definitions.

## Run
    # headless
    xvfb-run -a spacecrafter        # with dual-dump.sts installed as startup.sts
    ./analyze.py /tmp/dual_trace.json
    ./analyze.py /tmp/dual_trace_2.json   # second date: time-dependent vs constant error

PRECONDITION (INTENT 11.33): the analyzers' px conversions assume the FISHEYE
transfer (r = theta/halfFov). Run scenes with config projection = FISHEYE;
under other modes, apply ProjectionTransfer::radius at the conversion points
before trusting screen-layer numbers (mat-layer P1-P5 are projection-free).

## Reading the output against projection-paths.md C9
- Common nonzero R_rel angle ~90 deg around z across all bodies -> the
  [-Y,X,Z] xy-signature; check the axis.
- dDist ~ 4.26e-5 AU on Earth-branch bodies for a surface observer -> the
  topocentric-vs-geocentric component; compare free-mode vs surface-mode runs.
- Orientation-convention caveat: the two paths' matrices may map in opposite
  directions by convention; a uniform R_rel pattern reveals that too -
  measured, never assumed (analyze.py header note).

## First-run findings (2026-07-11) - full trace in INTENT.md 11.14a
1. EMB CONFIRMED: one path's Earth = Earth-Moon barycenter, the other = Earth
   center (delta points along Moon direction, cos=+1.0000 at two dates;
   magnitude = lunar mass fraction x Moon distance, 4-digit match).
2. Non-rigid direction errors (Moon 41.5 deg vs Sun 67.2 deg from the same
   observer, inter-body angle not preserved) => per-hop rotation component
   (E2) exists; camera-only convention refuted as sole cause. Both paths read
   IDENTICAL Moon eclipticPos - the frame interpreting it differs.
3. Under Moon tracking, old = centered, new = behind the observer => E3
   (az/alt or tracking-sync) component present too; tracking propagation to
   the new Camera unverified.
Known caveats: dScreen column mixes units (old pixels vs new NDC) -
indicative only. Proper conversion (INTENT 11.19): old screen = render px on
the scissor (render_size, e.g. 2048); new screen = rect [-1,1];
px = (rect*0.5+0.5)*render_size. With that conversion the screen layer is a
first-class comparison target - it caught the three view-layer roots the
mat-layer P1-P5 could not see (all camera-frame rolls/offsets). Exit segfault
after 'shutdown action now' (post-dump, unattributed, possibly pre-existing).

## Quadruplet mode (quad.py) - INTENT 11.14b
Earth/Moon/Sun/Mars [vixy]: identity / down-hop / up-hop / up-then-down -
minimal set separating translation, common rotation, hop accumulation.
quad.py DISCOVERS each path's effective composition (tests hop-grammars
against dumped finals). Run findings: old composition validated exactly
(parent-rotation post-multiply, Moon unprecessed/ELP82); new reference =
raw camera mat, up-chain exact, Moon/Mars mats are chimeras (invisible =>
translation-only; invisible WITH children => nothing, t=0 - preUpdate
early-return skips the store); SolarSystem root spin = 90 deg about -z
(matches the [-Y,X,Z] note signature - orientation-layer lead); no single
rotation maps fresh positions (12-36 deg residuals) => per-branch
composition differences, not a camera-only error.

## Triplet resolution (predict.py + fix-validation.sts) - INTENT 11.15

predict.py supersedes quad.py's discovery role once the composition is known:
it PREDICTS both paths' matrices from dumped inputs and accepts only float-eps
residuals. Sections: P1 old model, P2 new model, P3 relative geometry (THE
promise), P4 observer parity, P5 rotation differentials with named causes.

fix-validation.sts is the validation scene (dual-dump derivative): requires
`init_fov = 340` in config.ini - `zoom fov` does NOT reach Camera::setHalfFov
(seam gap, INTENT 11.15c), only config init does; the wide fov keeps
Sun/Moon/Mars inside the new path's visibility cone so their rotations are
fresh (not chimera).

Final measurements (2026-07-11, two dates): predicted==observed ~5e-8 both
paths; relative positions old==new <= 7e-6 deg / 2.3e-7 distances; observer
parity 6-23 km (float-ulp on AU chain); all rotation differentials modeled at
~1e-7 (causes: view-state D_common, old's parent-rot accumulation on the Moon,
old's skipped rotation elements on parentless bodies). Structural causes and
fixes: INTENT.md 5.10-5.16.

## Non-surface generalization (drive_scenes.py) - INTENT 11.16

Three scenes: Earth surface 100 m (baseline) / Earth 50 km / observer ON the
Moon (satellite reference). Driven over the TCP command interface (port 7805,
enable_tcp) because startup.sts autoplay proved racy (the app's default init
chain can preempt it); launch the app, wait for init, then run
harness/drive_scenes.py. Still requires init_fov = 340 in config.ini.

Scene C exposed and led to fixing (INTENT 11.16): the observer-body seam
(switchToAnchor never reached the new Camera), the moon/sun scale seam (5x
altitude-reference divergence), the rotation-offset unit bug (degrees added
to a radian formula: 20.76 deg spin lag on the Moon), the ACCUMULATED
equatorial frame for observer placement (old parity: pol==lat and
az==sidereal+lon hold exactly in rot_earth.rot_moon, not in rot_moon alone),
an ASmooth 0/0 (double set in one tick -> permanent NaN), and the NaN-date
freeze in the shared Kepler solver (elliptic_to_rectangular.c infinite
Newton loop). Final: all three scenes at float epsilon on P1-P5.

## Mars generalization (scene D, 2026-07-12) - INTENT 11.17

Observer ON Mars (100 m), tracking Earth's Moon, quadruplet at two dates
88 days apart (appended to drive_scenes.py). Two firsts: cross-branch
reference (common parent = Sun: up-hop + two down-hops) and fully generic
reference body (pole-RA/DE elements, no hardcoded content, generic spin +
offset-degrees fix). Result: ZERO new defects - first scene passing on
first attempt; P1-P5 at float epsilon both dates (P4 = 39/11 km on a ~2 AU
chain = the same 1.4-ulp class as 10-18 km on 1 AU chains). Scenes A-C
re-run as regression: unchanged.

Environment note: this machine has a live X server (DISPLAY=:2), no xvfb -
launch `DISPLAY=:2 ./build-claude/src/spacecrafter` directly; the xvfb-run
line above is the generic recipe.

`asmooth_sim.py` - off-domain but homed here for traceability: exact-formula
replay behind the EntityCore ASmooth analysis (INTENT 11.18); not a
body-path tool.

## Orientation consolidation (2026-07-17) - INTENT 11.34/11.35

`orientation_check.py <dump>` - convention checker, the predictive GATE of
the 6.8 implementation: transcribes old observer (getRotEquatorialToVsop87),
old render (one-hop wrong-side), new current, and the accumulated fix from
dumped pieces; prints the divergence table + the planet-moon commutator
spectrum + P-d (live render/observer contradiction at the reference: was
23.4422 deg at a Moon reference, 0.0000 post-fix). Metric note: angle() uses
the Frobenius small-angle form near identity - acos((tr-1)/2) turns float-ulp
matrix noise into ~0.014 deg phantom rows.

`ab_orientation.py` - terminal-observable A/B (verification height): scaled
tracked Moon + axis, and Charon from Pluto's surface; clusters screenshots
into path phases and measures disc diffs. Pair with the instrument-
sensitivity counterfactual (rotate one phase's disc by the class angle):
measured x163 (Moon, 23.44 deg) / x1212 (Charon, 115.6 deg) headroom over
the observed AA/pointer floor. Uses `set moon_scale` (mirrored seam);
`planet_scale` is OLD-PATH-ONLY (seam gap, INTENT 11.35) - do not use in
A/B scenes.

Dump extension: dumpTracePaths hops set is Earth/Moon/Sun/Mars/Pluto/Charon
(tilt pieces stay fresh through recursiveTranslationUpdate even invisible).

## Hierarchy spine (scene E, 2026-07-17) - INTENT 11.36

`scene_e_spine.py` - reference-transition ladder over the nested tree
(universe > milkyway > SolarSystem > Sun > Earth): AoI thresholds computed
OFFLINE from the updateCache formulas transcribed on a baseline dump, then
the reference sequence asserted at bracketing altitudes; multi-shell
escalation AND capture cascades, second entry of both; anchored legacy legs
(reference pinned, home_planet+moveto race-free). Requires init_fov=340 +
fresh launch. Auto-transitions are FREE-FLIGHT-ONLY (11.36 policy).
Camera dump fields refAoI/refDist/refCached/refParent = the transition
inputs; per-body `relation` = the membership authority (BodyRelation:
<3 hidden, >=3 visible) - the ONLY valid hide/show observable: dump
PRESENCE iterates the name registry, which includes hidden bodies.
Instrument caveat learned on mw_out2: dumps ride the events thread - a
healthy dumped snapshot does not prove the decision path runs; discriminate
with call-time prints, screenshot-materialization (draw liveness), and
per-thread CPU accumulation. Bit-identical dumps under timerate 0 are NOT
frozen-loop evidence.
