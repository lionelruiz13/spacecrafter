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
indicative only; exit segfault after 'shutdown action now' (post-dump,
unattributed, possibly pre-existing).

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
