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
