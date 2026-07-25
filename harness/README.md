# Dual-path projection trace harness (INTENT.md 11.14)

Purpose: replace eye-based A/B comparison of the two body paths with numeric,
scriptable comparison - the Moon-divergence investigation is the first client;
every future port keeps it as regression infrastructure.

Why it is trustworthy by construction: `SSystemFactory::update` feeds BOTH
paths from the same `timeMgr->getJDay()` every frame [ssystem_factory.cpp],
and the script freezes time (`timerate rate 0`), so the 1s A/B draw toggle
cannot make one path's draw-side state stale - both settle at the same jd.

ALTERNATION IS OPT-IN SINCE 2026-07-21 (INTENT 11.50(c), verified 11.53).
The default is now the NEW path, PINNED - nothing alternates on its own. Any
recipe below that assumes the 1 s auto-toggle needs
`~/.spacecrafter/beta_features.ini` containing:

    [dual_path]
    render_path = alternate

Remove that file afterwards: absence == every experimental default, and a
leftover file silently re-specifies the next run.  `flag experimental_path
on|off` pins a path at any time (and stops the alternation) either way.

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
`init_fov = 340` in config.ini. (Historical note: an earlier revision claimed
`zoom fov` does NOT reach Camera::setHalfFov - CORRECTED, INTENT 11.40: it
always did; `zoom fov X duration 0` reaches the new path immediately.) The
wide config init_fov keeps Sun/Moon/Mars inside the new path's visibility
cone so their rotations are fresh (not chimera) from the first frame.

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
tracked Moon + axis (scene M), and Charon from Pluto's surface (scene P).
REWRITTEN 2026-07-25 (F0, INTENT 11.101(g) -> 11.103): it no longer samples
the 1000 ms toggle and no longer INFERS the path phases by clustering (that
construction always emitted two clusters and never compared `between` to
`within`, so it could not fail). Each half is now shot under its own
`flag experimental_path off|on` PIN - the partition is commanded, so path
identity is witnessed - tracking is released before capture (11.80(c)), and
the run ASSERTS `between > 10 * max(within_old, within_new, 5)` on px>8,
with the in-scene same-path floor measured in the same run (K and the floor
clamp derived in the file header from 11.53(d)/11.35/11.80(b)). No
`beta_features.ini` needed. A non-separating scene is reported INCONCLUSIVE
and exits 1 - it is NOT reported as "the class is absent", because a
swallowed pin produces the same pixels. Measured 2026-07-25 at code
`d006ee92`: scene M within 0/0, between 1176 px>8 (235x the clamp); scene P
within 338/336, between 315427 px>8 (933x) - and the same script with both
halves pinned to the SAME path returns 0/2 SEPARATED, exit 1.
Historic note: the pre-rewrite instrument-sensitivity counterfactual (rotate
one phase's disc by the class angle) measured x163 (Moon, 23.44 deg) / x1212
(Charon, 115.6 deg) headroom over the observed AA/pointer floor.
Uses `set moon_scale` (mirrored seam).
`planet_scale name X scale N` is now DUAL too (INTENT 11.45 closed the
11.35 seam gap - new-path scaledRadius scales exactly with the command);
usable in A/B scenes. NOTE: setScaling is an ASmooth ease - settle it
(~6-8 s) before dumping, and the command syntax is keyword-based
(`planet_scale name X scale N`, not positional).

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

## Dual-path default flip (B26, 2026-07-21) - INTENT 11.50(c) / 11.53

`b26_run_case.sh <tag>` + `b26_default_flip.py` + `b26_analyze.py` +
`b26_probe.gdb`. Verifies what an unconfigured launch actually draws, and
that `beta_features.ini` is honoured. One case per invocation; **the caller
places or removes `~/.spacecrafter/beta_features.ini`** - the file's state IS
the case, so the runner never writes it.

    rm -f ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c1_default
    printf '[dual_path]\nrender_path = alternate\n' > ~/.spacecrafter/beta_features.ini
    ./b26_run_case.sh c2_alternate
    rm -f ~/.spacecrafter/beta_features.ini          # restore the shipped state
    ./b26_analyze.py c1_default ; ./b26_analyze.py c2_alternate

Two instruments, deliberately independent:
- **pixels** - `body action screenshot`, 24 shots at 0.25 s, each classified
  against two in-run pinned references (`flag experimental_path off|on`).
- **memory** - the app runs UNDER gdb (ptrace_scope=1 blocks attach) and
  `b26_probe.gdb` prints `drawModularSystem`/`pathPinned` at every capture.
  They agreed 24/24 on the alternate burst; a disagreement is the finding.

Criterion, corrected at 11.53 (the 11.50(c) wording is unsafe):
- discriminator = **px>32**; measured 0 for every same-path pair, 133..136
  for every cross-path pair. `max|d|` and `px>8` do NOT separate cleanly and
  "byte-identical" is false even for a correct build (11.53(e)/B30: the new
  path is not bit-stable on a frozen scene - <=31/255 on <=0.09% of pixels).
- separation = an **odd multiple of 1.0 s**. The toggle is a 1000 ms square
  wave, so 2.0 s always lands in the SAME phase and 2.5 s differs only 50%
  of the time (7/14 measured) - "two shots >= 2.5 s apart must differ" is a
  coin flip, not a test.

Freeze witness: two `dual_dump` headers bracket the burst; equal jd is the
proof `timerate rate 0` took effect (the auto-playing
`scripts/fscripts/startup.sts` sets `timerate rate 1` and must be overridden
after it, not before).

## Hidden-body ticking (B19, 2026-07-21) - INTENT 11.54 / 13.B B19

`b19_hidden_tick.py` - regression lock on the Vixy-ratified semantics
(USER_QUESTIONS Q13 / INTENT 11.48(a) A10, verbatim: *"It should be where it
is now"*).  Fresh launch, `enable_tcp`, no init_fov requirement (mat-layer
position state only - no screen-layer px):

    DISPLAY=:2 ./build-claude/src/spacecrafter &     # wait for port 7805
    ./b19_hidden_tick.py [outdir]                    # default artifacts/b19
    # exit 0 = all pass; artifacts/b19/b19_result.json = machine-readable

Observable = `ecl` (ModularBody::eclipticPos), written only by
transformParentToBodyPos/transformBodyToParent right after the orbit is
evaluated: a freeze optimisation stops calling them, so `ecl` keeps its
hide-time value.  `lastJD` is a corroborating witness, never the criterion.
Four assertion families - membership (`relation` actually flipped: the
instrument-chain check, without which a mistyped hide passes vacuously),
time control (every header jd == commanded; `timerate rate 0` goes FIRST,
before the epoch, or the gap after startup.sts's `timerate rate 1` leaks
~1.4e-05 d into the first dump - measured), advance (new-path |dEcl| vs the
OLD path's own advance over the same 20 simulated minutes - old is the
reference implementation, solarsystem_display.cpp:343-357 computes every body
hidden or not), and "where it is now" (|ecl_new - ecl_old| at t1, tolerance
CALIBRATED in-run from never-hidden control bodies).  Both entries of the
reversible pair (hide->show->hide->show, entry 2 starting from entry 1's
show state).

Discrimination is measured, not assumed - the same script, same binary:
hidden legs FAIL / shown legs PASS on a build without the fix (2026-07-21:
moved_new = 0.00 km vs moved_old = 1306.81 km Moon / 2576.26 km Phobos), all
32 assertions pass with it.  Subjects are Moon (direct child of the camera
reference) and Phobos (grandchild under the HIDDEN parent Mars - it proves
the whole hidden subtree ticks, not just the hidden node).

## System reload (B16, 2026-07-21) - INTENT 11.55 / 13.B B16

`body action reload` - rebuild the current system from its data file, keeping
the observation state (camera + date).  Three drivers, one entry point; every
run is a FRESH launch under gdb, whose breakpoint on
`SSystemFactory::reloadCurrentSystem` is the "the command reached its handler"
evidence that does NOT come from the handler's own log (the 11.54(j)
silently-swallowed-command class: count the breakpoint hits against the
commands issued, 1:1 or the spelling is fiction):

    DISPLAY=:2 ./b16_run.sh b16_reload.py    [outdir]   # default artifacts/b16
    DISPLAY=:2 ./b16_run.sh b16_overrides.py <outdir>
    cp b16_reload_check.sts ~/.spacecrafter/scripts/    # channel-2 input
    DISPLAY=:2 ./b16_run.sh b16_channels.py  <outdir>
    rm ~/.spacecrafter/scripts/b16_reload_check.sts

`b16_reload.py` is the main assertion run: no-reload CONTROL pair (the
instrument floor - tracking never exactly settles and the new path is not
bit-stable on a frozen scene, B30), then MUTATE `~/.spacecrafter/ssystem.ini`
(`[moon] radius` x2) -> reload -> RESTORE byte-identically (md5 asserted
in-driver) -> reload -> reload.  Without the mutation the run proves nothing:
a no-op reload passes every state-preservation check trivially.  Observables:
new-path `boundingRadius`/`screenSize` from `dual_dump` (the live tree, not
the file), the composed 2048^2 screen at px>8, and `Camera::dumpTrace`
(`reference`, `tracked`, lon/lat/distance, mount, fov) + the header `jd`.
Selection pointer OFF (`select planet Moon pointer off`): its bracket radius
eases toward the object's apparent size for seconds after any size change and
would dominate the screen A/B with something that is not the reload.

`b16_overrides.py` characterises what the reload does NOT keep: body-scoped
runtime overrides (`moon_scale`, `body name X hidden true`) are reset to the
file values while the old path keeps its own - the suspended question in
11.55(i).  `b16_channels.py` exercises both 2(c) channels in one launch (live
TCP command, then a script whose single line is the command).

## Reference-change view continuity (B13, 2026-07-22) - INTENT 11.61 / 13.B B13

`b13_viewcont.py [absOutPrefix]` - measures whether the ABSOLUTE sky direction
is held across a reference switch (`set home_planet` = warpToBody) and free-mode
entry/exit.  Fresh launch, `enable_tcp`, FISHEYE; no init_fov requirement
(mat-layer only, no screen px).  Pass an ABSOLUTE dump prefix - the app writes
`dual_dump` files relative to ITS cwd, not the harness dir.

    DISPLAY=:2 ./build-claude/src/spacecrafter &        # wait for port 7805
    ./b13_viewcont.py /abs/path/artifacts/b13/post      # -> *_result.json

Observable = `absFwd` (added to `Camera::dumpTrace`): the eye-forward (-z) in the
ROOT-aligned common-inertial frame = `(-r[2],-r[6],-r[10])` of
`lastDispatchedMat . reference->accumulatedBodyToBodyPos(jd)` - the same `flat`
dispatchUpdate builds, so it is directly comparable ACROSS a reference switch
(the dump's `mat` is in the reference's own equatorial frame, body-specific).
The DISCRIMINATOR is the alt/az delta: absolute-held => absFwd fixed, alt/az
moves by the inter-frame rotation; frame-relative-held (the pre-B13 defect) =>
alt/az fixed, absFwd jumps ~78 deg.  The two ALWAYS swap - that swap is the test.

Two residual floors, both attributed: settled continuity is the `recoverParams`
Euler floor (~6e-6 deg, shared with switchToBody); a fresh launch's FIRST switch
can show up to ~5e-3 deg = B30 frame-reconstruction non-determinism (measured
independently as the spread between two fresh-launch samples of the SAME state),
not a compensation artifact.  Settle the app before trusting sub-0.01 deg.

Scene E (`scene_e_spine.py`) carries the regression-locked version: 8 asserts
(absFwd held across set_home_planet Earth<->Mars and free enter/exit/enter, alt/az
moved on the switch, a switchToBody positive control).  Discrimination proven by
temporarily disabling `recoverParams(R)` in warpToBody - the 3 ref-switch asserts
flip to FAIL, the rest stay green.  App rewrites config.ini on shutdown, so any
`cp`-restore of an init_fov edit must run AFTER the process is fully dead.

## Composed-system authoring (B24/B25, 2026-07-22) - INTENT 11.78 / 13.B B24/B25

`b24_equivalence.py` - the corpus-wide legacy-vs-composed gate. Owns its app
lifecycle (launches BOTH phases itself; do NOT pre-launch):

    cd claude/harness && DISPLAY=:2 python3 ./b24_equivalence.py

Phase A = shipped state (asserts no enabled file; the launch itself
regenerates the machine-owned twin ~/.spacecrafter/modularSystem/
SolarSystem.ini.disabled - generation-at-load is product behavior). Phase B =
twin copied to SolarSystem.ini (the documented adoption workflow), fresh
launch, SAME frozen scene; enabled file ALWAYS removed afterwards (a leftover
silently re-specifies every next run - B26 hygiene class). Compares the
new-path tree by name: parent/relation/modules/routing/boundingRadius/lastJD
+ ecl as EXACT STRINGS. `axisRot` is deliberately NOT compared cross-launch:
an A-vs-A control measured the same scatter on the same ~20 pole-bearing
moons (launch-wall-clock spin staleness, INTENT 5.24 / B32). Discrimination
proven by deleting [Moon:MESH] from the enabled copy (drops exactly the MESH
slot + one near-routing entry). Twin values carry the source's ISO-8859
bytes - read with latin-1, copy with read_bytes.

`b24_compose.py` - the composition-mandate scene (rover on the Moon + rover
on Earth grounded, orbiting controls, rocket on an ascent ramp), authored
through the REAL adoption workflow (twin + appended composed sections).
Numeric layer, default observer, frame-proof gates - READ THE DOCSTRING
before touching the assertions: the dual_dump eclRoot base is
camera-composed and rotates with the observer's reference surface; the
docstring carries the three-iteration attribution record (a physically-fixed
control "rotating" by exactly the Moon spin was the instrument, not the
fold). Gates: Earth grounded pair net rotation 0.0000 deg (pre-fix
child-spin fold reads ~90.2 deg - measured on the real pre-fix binary);
orbiting control == full spin advance; Moon pair chord length point-predicted
from the dumped spin (9 m vs 2849 km frozen); rocket |ecl| exact lerp replay.

## Draw-half mode independence (B5, 2026-07-23) - INTENT 11.80 / 13.B B5

`b5_run.sh b5_drawhalf.py [absOutdir]` - verifies the new path DRAWS in the
inGalaxy/inUniverse executor modes (drawExperimental), that nested systems
RESOLVE (subsystem-geometry px classification), and that the collapsed
system shows its star-proxy dot.  Fresh launch, init_fov=340, FISHEYE;
config restored byte-identically.  Pass an ABSOLUTE outdir (the app writes
dumps relative to ITS cwd - b13 lesson).

Discriminators (b26 px>32 class, in-run floors + counterfactual build):
"galexec" (dot through the galaxy executor, '->InGalaxy' witnessed) and
"uniband" (interior+dot inside the B22 cross-fade band at refDist 1340 AU,
'->InUniverse' witnessed) - both collapse to exactly 0 on a pre-fix binary
while every witness stays green.  "gal" (resolved interior at 694 AU) is
the in-run flag-liveness control (>0 in every build).

Facts the driver rests on (INTENT 11.80): solar executor mode reaches 1e16 m;
inGalaxy entry clamps the OLD observer to 1e10 m; inUniverse needs a second
moveto >1e14 m and re-bases to 1e9 m (+ entry fade, cleared by one more
moveto).  Free-mode `moveto altitude X` at a MilkyWay reference lands at
3.2e9 AU + X (the B10(c) datum, unimplemented) - the NEW camera is placed
with `camera action descend coef <c>` instead.  Phase-toggle floors: assert
the NEW phase only (old big-halo re-entry easing pollutes the old floor,
278 px full-scale at the view centre, old-path-only); aim via select+track,
then track OFF before shots.  At fov 340 the DAY-surface cross-phase px32
is 0 - validate any cross-path control in-scene, never assume b26's 133 px.

`b5_probe.gdb` + `b5_diag.py` - the attribution instruments (breakpoints on
drawNested/drawStarProxy/Renderer::drawHalo with member prints; the
proxy-entered-801x/drawHalo-0x measurement that located the halo-flag gate).

### Oort content-migration PILOT (B5 partial, 2026-07-24) - INTENT 6.9 / 13.B B5

`b5_oort_run.sh b5_oort.py [absOutdir]` - verifies the OORT cloud, migrated to a
MODULAR BODY at the SolarSystem floor (OortModule, CUSTOM/OORT slot), reproduces
the OLD altitude-gated draw through the new path's REGIME machinery. SEPARATE run
from b5_drawhalf.py: the runner enables `flag_experimental_oort=true` (the default
tree keeps it OFF, so b5_drawhalf stays 24/24 unperturbed - the pilot's node-reach
coupling is exactly why it is gated). Fresh launch, init_fov=340, fisheye, config
restored byte-identically.

Method: in each render phase isolate that path's oort by `flag oort` on/off (the
command drives BOTH clouds through one CoreLink choke point) - px32(on,off) = the
oort ALONE, the rest of the frame (bodies) identical (discrimination by
construction). Solar mode, ref=Sun (the clean regime-gated band). Legs: LOW-hide
(refDist ~20 AU, both < 800 px), MID-show (refDist ~334 AU, NEW vs OLD within
0.1%), discrimination (shown/hidden ratio 159x by the altitude gate, witnesses
drawn in both). RECORDED coupling (not a pass leg): past refDist ~533 AU the
reference becomes 'Oort' - the cloud's 6400 AU extent inflated its AoI and
hijacked the camera reference from MilkyWay (why the b5 galactic legs break with
it on). High-edge divergence: OLD hard-cuts at 1e16 m, NEW fades on angular size
(NEW 3210 vs OLD 276 px at 1e16 m) - old arbitrary cut vs new physical fade.

## View-directed free descent (B21, 2026-07-22) - INTENT 11.72 / 13.B B21

`b21_descent.py` (full) + `b21_far.py` (fast far-only) + `b21_probe.py`
(selection sanity).  Numeric/vector layer, FISHEYE, run via `./b10_run.sh
b21_descent.py <out>` (reuses B10's config/init_fov + md5 restore).

Driver = the NEW command `camera action descend coef <c>` (coef<1 descends,
coef>1 ascends).  The view-directed descent geometry was UI-key-only before
(multAlt/moveRelAlt, B10 finding), so a command had to be routed to test it.

Four parts: (1) near SIGN - |pos| drops toward the ground; (2) VIEW-DIRECTED
discriminator - two views ±35deg apart land at DIFFERENT surface points (280 km)
while `moveto altitude` lands at the SAME sub-observer point (0 km); (3) R4 CLAMP
- hold at ground_radius, reversible x2, enter->centre; (4) FAR - at a SYSTEM
reference (`sun_aoi*1.07`) descend aims at `getSelected()`.

FAR-case gotchas learned here:
- the observed distance to a runtime-loaded body is NOT in the per-body dump
  list (that iterates the OLD current system, which COLLAPSES at galactic
  distance).  Read it from the camera dump's `selDist` (= obs->selected, added
  this row) - frame-independent, and it IS the quantity the descent moves along.
- `select planet X` searches the OLD current system, so SELECT WHILE NEAR (the
  selection persists through the fly-out + escalation).
- real planets sit ~1 AU from the system centre => ~0deg apart from a
  system-distance observer (float-noise discriminator); use OFF-CENTRE synthetic
  targets (FarA +y, FarB +z, 12000/20000 AU).
- a big step (coef 0.5) de-escalates SolarSystem->Sun mid-measurement (the
  transition machinery re-bases the frame); a SMALL step (coef 0.96) keeps
  ref=SolarSystem and selDist ratio is EXACTLY coef == exact-aim proof.

## Instruments — CaptureMetrics (EntityCore) [vixy: 2026-07-23] + environment note

- `CaptureMetrics` (src/EntityCore/Tools/CaptureMetrics.hpp, MIT) is the sanctioned
  realtime performance-capture tool: typed per-frame timepoints (type 0 = frame
  delimiter, names from CAPTURE_FLAG_NAMES), lock-light (atomic ring, 2048-pt
  batched writes), already wired as `context.stat` -> `log/statistics.dat`
  (app.cpp:117).
- Key property [vixy]: the RAW capture drops no structural data — per-frame,
  per-point timing structure is fully preserved in statistics.dat; analyze()/
  display() (and the query_statistics summary channel the B22 cost measurement
  used) DEGRADE it to min/max/avg. When the signal is structural (state
  persistence, stepping patterns, burst shapes — B30/B32 class), read the RAW
  file, not the summary.
- Environment note (session-scoped): from 2026-07-23 ~21:30 the host is quiet —
  no CPU/RAM-heavy foreign process until end of session [vixy]. Timing
  measurements taken under the earlier Minecraft-JVM contention (11.81 cost
  noise bands, 11.82(d) launch crashes) should not be extrapolated; retry loops
  stay (the 11.15d race predates the contention) but the "severe blocker"
  framing is over.

## Grounded depth ladder (B3, `b3_ladder.py`) — INTENT 5.29 / 5.30 / 5.33, 11.104 / 11.105

The discriminating instrument for the ray-regime depth defects. Grounded exact
unit-sphere OJM bodies of known drawn radius and known altitude are composed on
a parent, the observer sits at nadir inside the ray band, and the present-vs-
absent diff against a scene-absent baseline launch gives each body's rendered
cap radius in px. Whatever the parent leaves in the merged depth bucket is a
WALL, and the cap radius inverts to the wall height.

    b3_ladder_run.sh <state> [outdir] [--site moon|earth|earth_noatm] [--families sph,cur]
    b3_ladder.py <outdir> --predict [--site ...]      # prediction file only

- **States are wall HYPOTHESES**, each predicted from source arithmetic before
  the run: `none` (no depth written — nothing is ever occluded), `shell` (the
  ray-march proxy shell, `scaledRadius*(1+0.01*altimetryLevel)` — defect 5.29,
  and the failure mode 11.104(c) warns about for Earth), `terrain` (the true
  surface, the fixed state), `atm` (the ATMOSPHERE shell at
  `scaledRadius*atmosphere_radius_factor` — defect 5.33). `pre`/`post` alias
  `shell`/`terrain` for the F1-P1 moon invocations.
- **Sites**: `moon` = F1-P1's SIZE ladder at alt 0 (11.104(b)/(d)); `earth` =
  F1-P2's ALTITUDE ladder at fixed radius 45 km, alt −150…+400 km, which
  separates all four states at several legs each without needing the heightmap
  texture-u convention (the site is ocean under all four candidates);
  `earth_noatm` = the same Earth with `[Earth:ATMOSPHERE]` removed from the
  composed twin in the farm copy — the counterfactual that isolates the MESH
  row's depth (5.30) from the atmosphere shell's (5.33).
- **Preconditions asserted, not assumed** (a violation fails the run): parent
  drawn UNSCALED (`scaledDatumRadius` == authored radius — 5.27), parent inside
  the ray band [2R, 64R], the parent's module set == the one the site declares
  (so a counterfactual that silently did not apply fails), per-leg drawn radius
  == authored × model radius (`OjmLoader.cpp:42`), per-leg observer→body
  distance == predicted (the `moveto lon L` ↔ `orbit_lon` = 180−L calibration,
  11.104(g)), per-leg site lit, and a shadow witness on every leg the state
  predicts invisible (the shadow pass is offscreen and does not depth-test, so
  a depth-killed body still proves loaded/lit/placed).
- **Instrument resolution** (11.104(d2), inherited): cap radius carries a
  −2 % multiplicative bias (0.9770 ± 0.20 %) from the |Δ| > 16 diff threshold
  eating the antialiased rim, plus ±1 px reading. `dwall/dpx = (cap/q0)·km/px`
  and is reported **null** outside the strict band 0 < q0 < r — a saturated or
  dark leg carries no wall information and must never be read as metric.
- Runs under the temp-HOME farm (`b3_farm.sh`, 11.103(a)); the runner asserts
  the real `~/.spacecrafter` md5 in == out.
