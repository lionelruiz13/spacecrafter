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

**XAUTHORITY may be inherited WRONG** (seen 2026-08-08, F28): a session can
start with `DISPLAY=:0` and `XAUTHORITY=/run/user/1000/.mutter-Xwaylandauth.*`
- another uid's runtime dir - and then every display, including `:2`, answers
`Authorization required, but no authorization protocol specified`. The file to
use is the one under YOUR runtime dir:

    export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*)
    DISPLAY=:2 xdpyinfo | head -3      # positive check before any launch

Check it with `xdpyinfo`, not with a launch: the app failing to open a display
looks like a dozen other faults.

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

## F4 — the capability-audit instruments (INTENT §11.108, 2026-07-25)

- **`xkey.c`** — hold a REAL X11 key on the app window (XTEST), resolved to the
  CLIENT window exactly as `xclick.c` does it. Built at run time by the driver:
  `gcc -O1 -o xkey xkey.c -lX11 /usr/lib/x86_64-linux-gnu/libXtst.so.6`.
  Usage `xkey spacecrafter Left 2500 1024x1024`. Exists because the interactive
  navigation ramps have NO command entry: the only honest way to say what a key
  reaches is to press it (§11.36's rule).
- **`f4_keyprobe.py`** (**superseded 2026-08-02 by `f25_ramp.py`, and expected to FAIL on any binary from code `d9de42ac` on** — it asserts the DEFECT, which is what a discovery probe does, so its two finding legs failing IS the fix showing; kept unchanged as F4's record and as a pre-fix witness) — presses Left and asks what moved. Carries its own
  positive control (the OLD phase must move, or the key never arrived) and
  asserts on the PER-PATH dump, not on the screenshot: in that scene the lit
  content is path-independent (48 px>32 cross-path), so a screenshot cannot
  attribute a camera. Result: the new path's camera and its drawn body position
  are bit-identical across the keystroke while the old path's body moves 872 px.
- **`b21_keypath.py` + `b21_keypath_run.sh`** — the interactive ALTITUDE ramp,
  which has no key binding at all (joypad button only, and this host cannot
  inject joystick events: `/dev/uinput` is ACL-denied, no `evdev`). The runner
  gives gdb a FIFO for stdin so the driver can SIGINT the inferior, `call`
  `Core::lowerHeight`/`Core::updateMove` — the exact functions the UI calls —
  and resume. **Cadence matters**: one application per stop, real frames in
  between, because `Camera::descend` reads the reference's CACHED matrix
  (§5.32) and N applications inside one frame compound linearly, not
  geometrically. Pass `SC_BIN=` to run the same script on a pre-fix binary (the
  RED control).
- **`s526_heading.py` / `s526_ref.py`** — `set heading` parity at the screen:
  an offset × heading sweep, and a reference-switch scene whose sharpest probe
  is `heading delta_azimuth 0` (a semantic no-op that writes the OLD path's
  heading to BOTH — if the view moves, the two authorities disagree, and the
  pixels say by how much).

## F5 — the B25 GALACTIC gate (`b25_galactic.py`) — INTENT §11.109, 2026-07-25

Legacy-vs-composed equivalence for **addSystem / galactic star systems** — the
half of the B25 migration vehicle `b24_equivalence.py` never touched.

    cd claude/harness && DISPLAY=:2 ./b25_galactic.py [absOutdir] [--mutate]

**Read this before running it**: on a shipped install the galactic corpus is
NEVER OPENED. `core.cpp:320` calls `loadGalacticSystem(".", "galactic.ini")` and
the callee opens `path + name` — literally `.galactic.ini` — with the same
missing separator one level down for `.stellar_systems/<file>` (§5.37 / D29,
fix SUSPENDED because it would light up 17 systems + anchors on every install).
The driver therefore builds a temp-HOME farm (§11.103(a)) carrying **dot-prefixed
copies** of `galactic.ini` (byte-identical to the field file, md5 asserted) and of
the corpus directory, so the PRODUCTION path runs unmodified:
`loadGalacticSystem` → `loadSystem` → `addSystem` → `createModularSystem` →
`loadSystem` + `generateComposedTwin`. Nothing in the app is patched to make the
gate run.

Phase A = legacy load (asserts a twin per galactic system). Phase B = every
galactic twin adopted (`.disabled` dropped — the documented workflow) and
reloaded. **SolarSystem is deliberately NOT adopted**: it stays legacy in both
phases and is the in-run control (90 bodies, `composedDecl` false in both).

Three assertion families the solar gate has no subject for, plus b24's:
- **MEMBERSHIP** — a twin may declare only bodies of its OWN system. This is the
  gate's headline: `generateComposedTwin` used to resolve sections through the
  GLOBAL name registry, and the shipped `galactic.ini` points FIVE entries at one
  `stellar_systems` file, so four systems that loaded NOTHING got a full twin
  (§5.40, fixed `14bb627f`). Measured RED before the fix, GREEN after.
- **ORDER** — the twin's node sequence is the legacy file's section sequence,
  restricted to the sections that produced a body in this system.
- **BYTES** — every legacy key/value verbatim (ISO-8859 included, unknown keys
  included); `bound_to_surface` is the one translation (`relation = grounded`),
  with no duplicate authority left behind.
Per-body comparison is IMPORTED from `b24_equivalence` (I2), not copied.

`b25_corpus/` is the authored input: one full-featured foreign system
(`system_proxima.ini`), one SHARED file (`system_white_dwarf.ini` — five
galactic entries point at it, which is what makes MEMBERSHIP load-bearing) and
11 zero-byte files reproducing the shipped field state (every installed
`stellar_systems/*.ini` is 0 bytes). It covers **TAIL, OJM and GRID
declarations, which no shipped twin contains** (`ssystem.ini` has no Comet, no
Artificial body, no `planet_grid`). Values are SYNTHETIC and labelled as such in
the files — §11.51(d) forbids recalled physical constants, and none is claimed.

`--mutate` deletes `[PxB:MESH]` from the ADOPTED `ProximaSystem.ini` before phase
B: the discrimination run, which must fail with exactly two divergences
(`PxB.modules` loses MESH, `PxB.routing.near` 4→3).

Two instrument lessons recorded in the file: the order assertion must ask the
MEMBERSHIP authority, not "is this name in the dump" (the writer's own defect is
easy to re-commit in the checker); and the shadowing-log assertion matches the
line's TAIL, because the app's stdout interleaves `cLog` with
`SSystemFactory::loadSystem`'s raw `std::cout` and one line was measured
truncated mid-prefix.

D9: the real `~/.spacecrafter` is never written — md5 asserted in == out on
every top-level file plus `stellar_systems/` and `modularSystem/` (96 files
before B40; **113** after it, the 17 new galactic twins included).

**REWORKED 2026-07-30 by B40 (INTENT §11.115).** The farm now carries the corpus
under the PRODUCTION names (`galactic.ini`, `stellar_systems/`), because
`loadGalacticSystem` finally opens those: the gate exercises the shipped
placement and **a regression of the path repair takes it down** — measured, the
pre-B40 binary on this farm gives 18 divergences (17 × "no twin generated"). Two
mirror legs: `--dotted` restores the pre-B40 placement (2 divergences: the
corpus is invisible, only 17 EMPTY system nodes appear), `--mutate` is unchanged
(exactly 2). The three mirrored parsers collapsed into ONE `ini_line()`, which
is what the code did (`src/tools/ini_line.hpp`). The ISO-8859 leg was RE-POINTED
rather than dropped: high bytes in a VALUE must still be verbatim in the twin,
high bytes in a trailing COMMENT must now be ABSENT — a comment is a comment now.
`build_farm(farm=, dotted=, corpus=)` is the farm-shape authority `b40_parity.py`
and any future T-farm variant drive through.

## B40 — cross-binary tree parity on the REAL install (`b40_parity.py`) — INTENT §11.115, 2026-07-30

The D9 question for a LOAD-time change: what did the shipped data mean before,
and what does it mean now — per body, per field.

    cd claude/harness && DISPLAY=:2 ./b40_parity.py <absOutdir> tag=/abs/binary [tag2=/abs/binary2 ...]

Each binary is launched fresh on the real `~/.spacecrafter` (the `b24_equivalence`
phase-A protocol, whose comparison authority this file IMPORTS), driven to the
same frozen scene, dumped; consecutive pairs are compared. What it reports:
body-set delta by name (the AUTHORISED change is visible as itself — B40 adds 17
`<X>System` nodes and nothing else), per-body structural fields exact plus
`ecl`/`boundingRadius` under b24's own tolerance, every galactic system node
checked against `galactic.ini`'s own x/y/z read correctly (this is §5.38's
discriminator — a parse regression shows as a SIGN FLIP, and the path-only
counterfactual binary measured 6 of 17 wrong), the app's own `Params :` block
count, the new malformed-line warnings, and `config/ssystem/galactic/anchor` md5
in == out around every launch. The machine-owned `modularSystem/` twins are
EXPECTED to move and are reported, not asserted.

Use it with a saved pre-change binary: staging binaries are what make a
single-variable claim measurable (B40 kept three — pre-fix, parse-only,
path-only — and every claim in §11.115 names which one it came from).

## F7 — the B4 anchor-kind gate (`b4_anchors.py`) — INTENT §11.111, 2026-07-25

The three camera-anchor kinds R3 named, in both §2(c) channels.

    cd claude/harness && DISPLAY=:2 ./b4_anchors_run.sh [absOutdir]

The runner owns the app lifecycle (fresh launch, temp-HOME farm) and authors the
farm's `anchor.ini` as **the shipped file byte-verbatim + `b4_anchors.ini`**, so
every run also re-loads the whole FIELD grammar corpus through the production
path — all 14 anchors must declare, and P6's count check is what makes a silent
drop of the shipped 10 a failure rather than a quiet pass. Real `~/.spacecrafter`
md5 asserted in == out (config, ssystem, anchor). No `init_fov` requirement:
every observable is mat/position layer.

Predictions are committed in the file header before the run, and the authored
orbit is a CIRCLE (a = 200000 km, P = 0.5 d) precisely so each one is exact:
`|ecl| = a`, chord over dt `= 2a sin(pi dt/P)`. Three of the checks discriminate
IN-RUN, same binary and same instrument — keep-angle vs follow-rotation (a
bit-identical camera frame vs one rotated by the body's whole 87.72 deg spin),
orbit anchor vs fixed point (153073 km vs 0.000 km over the same date advance),
and authored vs commanded vs differently-parametrized anchors.

Two instrument lessons recorded here, both learned in this run:

- **The per-switch float floor is MEASURED, never assumed.** Four consecutive
  switches to the SAME anchor (physically no-ops) give F = 4.8e-07 rad =
  2.75e-05 deg per switch, and the drift is LINEAR in the extraction count —
  which is what attributes it to `Camera::recoverParams`' ZXZ Euler extraction
  (§11.61's floor). The camera state is therefore split by observable: POSE
  (lon/lat/distance/bind/mode/reference) is required EXACT and measured exact;
  only the VIEW half (alt/az/heading, and `mat` with them) carries N x F, with N
  counted from the scene (1 for the channel-parity pair, 5 for the reversible
  pairs). Requiring bit-equality of the VIEW half is what an earlier revision
  did, and it failed 8 checks on float noise.
- **Never `acos((tr-1)/2)` on a dumped rotation.** At 9 printed digits it turns
  ulp noise into ~0.026 deg of phantom rotation — measured here on a pair of
  BIT-IDENTICAL matrices. Use the Frobenius small-angle form (same caveat
  `orientation_check.py` carries).

- **A screen witness must be aimed, and its floor derived.** P7's first version
  asserted a frame-wide `px>32` above a GUESSED floor of 1000 and measured 534 —
  because the Moon was outside the frame at both dates (dumped screenPos
  x = 1.14, |NDC| > 1), so the shots differed only by the star field. It now
  aims (select + track, tracking RELEASED before every shot — tracking would
  re-centre the Moon at t1 and hide the motion under test) and tests a 2x2
  contrast at the DUMPED screen positions with a window radius PREDICTED from
  the dumped R/d/halfFov (28.3 px disc, 512 px separation). Measured 212/255 at
  the Moon's own position vs 40/170 at the other date's.

Also: with the observer on a far fixed point the OLD path emits bare `nan`
screen coordinates in `dual_dump` (2 lines of 103; the NEW side has none), so
the loader normalizes `nan` -> `NaN` rather than losing the file.

## F12 — the decision-implementation batch (INTENT §11.118, 2026-07-30)

Three new drivers, one runner repair, and two fixes to shared instruments.

### `f12_s545.py` — a malformed `galactic.ini` section must not kill the app

    cd claude/harness && DISPLAY=:2 ./f12_s545.py <absOutdir> pre=/abs/binary post=/abs/binary

Temp-HOME farm in `b25_galactic.build_farm`'s `corpus=None` field-state variant
(the real tree is never written and the run asserts it). Four corpora x two
binaries: shipped bytes, `[Proxima] z` deleted, `[Toliman] name` deleted,
`[Keid] x = ,5`. Each defect leg asserts its own MEASURED pre-fix outcome, which
is not the same for all of them: the two `stod`-throwing legs abort (rc -6, no
port), while a missing `name` never aborted - it built a system node called
`System` and lost `TolimanSystem`. Post-fix each leg wants exactly one warning
naming the section AND the key, the section's node absent, and every OTHER
system still loaded.

### `f12_b27_split.py` — D27's `light_source` / `primary` split

    cd claude/harness && DISPLAY=:2 ./f12_b27_split.py <absOutdir> [--bin B] [--prebin B]

Phase A is eight corpora over fresh launches (control, each key stripped, both,
legacy, a DARK PRIMARY on Earth, and a `compose = deduced` pair), plus the same
bytes on a pre-split binary. It generates the twin with the binary under test
first, so the corpus carries whatever that binary emits.

Two things it had to learn the hard way, both now enforced in the file:
- **in a composed corpus the module SET is DECLARED** (`compose = explicit` turns
  G6 deduction off), so `isSatellite()`'s deduction consumer is inert there and
  NO dump field moves with it. That is why the deduced pair exists and why the
  structural half is measured at the screen.
- **a screen leg must fail when the app rejects a command.** `flag
  satellite_orbits off` is `satellites_orbits`; the app said so in its own
  diagnostic and the harness ignored it, so a leg measured 0 px for a whole run
  with the satellite master left at its config default. Every screen leg now
  greps the applog for the rejection message.

Phase B measures the two moved consumers that a dump cannot see - the hint gate
(boxed on the Sun, box asserted empty of other DRAWN bodies) and the orbit
master flag (with a same-launch positive control that flips the satellite master
and shows the same line appearing).

### `f12_b33_heading.py` — the heading readout vs the drawn roll

    cd claude/harness && DISPLAY=:2 ./f12_b33_heading.py <absOutdir> [--bin B] [--prebin B]

`s526_ref.py`'s scene (Earth reference, then the Earth->Moon switch). **The
readout has exactly one observable channel on this build**: the script-log line
`heading from : X to: Y` that `heading delta_azimuth 0` writes, with X =
`CoreLink::getHeading()` before it acts. `get status position` reads the same
getter and is side-effect free, but its reply never reaches the driving client
(INTENT §5.47) - **true of the binary F12 measured, FIXED 2026-08-02 (F27,
§11.135): that reply now lands on the connection that issued it, so a heading
sample no longer has to write. `f27_reply.py` leg E is the demonstration (3
samples across two `experimental_path` pins in ONE launch). This script is
unchanged and still uses the writing channel - it is F12's record.** The channel
therefore WRITES after it reads, so each sample is
the last act of its leg and the two `experimental_path` pins need two launches.
The leg that matters is rendered: `heading delta_azimuth 0` is a semantic no-op,
so the drawn view must not move - and getting that honest needs EVERY old-path
observer-frame layer off, because they legitimately follow `Navigator::heading`
(nebula circles alone contributed 1225 px>32 before the list was widened).

### `b10_cmd_battery_run.sh` — asserts now, with exit codes

`0` green - `1` a launch never came up - `7` scene E failed - `4` the frozen
ssystem corpus is not the pristine one - `3` `config.ini` was not restored
byte-identically, in that precedence. `SSYS_PRISTINE` overrides the expected
corpus md5 for another delivery; overriding it to a wrong value is the cheapest
way to see the assert fire live (measured: exit 4, scene E still green).
`f12_runner_exit.sh` drives the runner's restore/assert tail, extracted VERBATIM
and asserted line-by-line against the runner, through all 7 exit paths including
the two precedence cases. `b14_sat6_battery_run.sh` is now a wrapper on this
runner - it was a byte-for-byte copy differing only in its own dead md5 echo.

### Shared-instrument fixes

- **`b24_equivalence.load_dump` is NaN-tolerant.** C++ prints `nan`, Python's
  json accepts only `NaN`, and the `except` used to swallow the WHOLE BODY - so
  a body carrying one non-finite float read as *absent* to every gate built on
  this loader. `b4_anchors.py` had already hit this and fixed it locally; the
  fix now lives once, in the shared loader (I2). A NaN-carrying body is visible
  and fails a value check loudly instead of disappearing.
- **`b24_equivalence.py --strip SECTION:KEY`** removes one key from the enabled
  twin before phase B, so a capability field added to the compared list can be
  shown able to FAIL on demand (measured: `Sun.primary: True != False`).
- **`b40_parity.py`**: a field one binary does not emit AT ALL is a dump-FORMAT
  difference, reported once, not a per-body divergence on every body.

## F13 — the write-back layer's gate (`b31_writeback.py`) — INTENT §11.119, 2026-07-30

    cd claude/harness && ./b31_writeback.py [outdir]        # no display, ~4 s

Gates B31 slice 1: the §11.66(b) line-preserving parse/write layer
(`b31-design.md` §5.2/§5.3, check T9). It **compiles the product sources**
(`src/experimentalModule/ModularSystemFormat.cpp` + `src/tools/log.cpp`) into
`b31_format/format_gate.cpp` and drives them — there is no Python parser here,
because a Python parser would only ever test itself. `format_gate` is a general
tool, not a fixed scenario:

    format_gate <in> <out|-> [op ...]
      set|<section>|<key>|<value>                give a key a value
      remove|<section>|<key>|<reason>            retire a key (commented out)
      annotate|<section>|<key>|<reason>|<text>   what a loader diagnosed
      dump                                       the parse, as JSON with hex
                                                 keys/values (no encoding can lie)
    <section> = a header text (first match) or '#<n>', the 0-based index.
    exit 2 = an operation was REFUSED (a legitimate answer the gate asserts on).

Thirteen legs: round-trip byte-identity, malformed-is-not-a-key, in-place
value change, new-key placement, removal-as-comment, annotation placement,
**T9 idempotence**, stale-marker removal, annotation replacement, **second
pass** (the whole operation set re-applied to its own output, so every
operation is shown to be a fixed point and not just the annotation),
representability refusal, CRLF, last line. The corpus is inline in the script
(27 lines / 654 bytes) and every line of it exists to be destroyed by a writer
that does not preserve — including the shipped `[Sedna]` malformed class and
the shipped `[mimas]` trailing-comment class.

**Discrimination is built in and reported on every run** (`note:` lines):
- the **pre-rework writer**, compiled on the fly from `f1151c63` (override with
  `B31_PRE_REV`), returns **297 bytes for 654 in and loses 14 of 27 lines**;
- the **naive annotator** (insert above the datum, never look for an earlier
  one) **grows the file by 3 lines per rewrite** and fails the T9 leg.

Not covered here, deliberately: the app-level legs. The twin byte-identity
check (delete `~/.spacecrafter/modularSystem/*.disabled`, relaunch, compare
against a pre-change snapshot) and `b24_equivalence` / `b25_galactic` /
`b40_parity` / `b4_anchors` are the run that proves the four readers of the
`.ini` grammar still read what they read — run them whenever `ini_line.hpp` or
this layer changes.

## F14 — where the IAU prime meridian lands ON THE TEXTURE (§5.28, INTENT §11.120, 2026-07-30)

    cd claude/harness && DISPLAY=:2 ./f14_meridian.py <absOutdir> <tag> expect=0.50
    ./f14_predict.py <absOutdir>            # before a change: write the predictions
    ./f14_predict.py <absOutdir> --check    # after: score the post capture against them
    DISPLAY=:2 ./f14_mercury.py <absOutdir> pre=/abs/bin post=/abs/bin
    DISPLAY=:2 ./f14_placeholder.py <absOutdir> pre=/abs/bin post=/abs/bin

`f14_meridian.py` is the gate D22 (§11.113(a)) required and `b14_w0_analyze.py`
could not be: it never writes the expression the conversion solves against.
It reads the render's own dumped `tilt`/`spin`, the mesh's own texcoord law
(`u = θ/360 − 0.25`, SphereObjL.cpp:153) and the body's own texture FILE, and
answers one question — which texture column the IAU meridian is drawn on.
**0.50 is correct; 0.75 is §5.28.** `expect=` picks which, so the file is RED
on a pre-fix binary instead of silent.

Three things it does that any successor gate should copy:
- **the observer is on the PRIMARY, never on the moon.** A surface-bound
  observer co-rotates with its reference, so the sub-observer texture column
  is invariant under the spin phase being measured — that scene is blind by
  construction, which is the §5.28 shape one level up.
- **it correlates ALBEDO, high-passed, not brightness.** Raw brightness scored
  the ±90° alternatives at 0.91–0.97 against 0.99 at the truth: the shading is
  the larger signal and does not move when the meridian convention does. After
  dividing by the model's Lambert term and high-passing at ~0.065 disc radii,
  the same scene reads 0.44 against 0.00/0.03/−0.07.
- **it fails on an under-lit capture.** After a large `date jday` jump the
  frame is black for several seconds (measured: max grey 10 at +3 s on a disc
  that reads 209 at +11 s). Both screen scripts sleep 8 s and treat an empty
  frame as a FAILURE, never as "unchanged".

`f14_predict.py` is the prediction-before-the-fact artifact: predicted offsets
for every hopped body, a SYNTHESISED post-fix image per moon, and the corpus
anchor (the corrected conversion reproducing the four registration-bearing
planets' shipped `rot_rotation_offset` from their fetched W0, to 0.014–2.788°).
`f14_mercury.py` is D22's counterfactual in a temp-HOME farm; `f14_placeholder.py`
answers "do the 17 placeholder-textured moons change?" with a measurement
(they do — `generic.png` σ = 8.15, `asteroid.png` σ = 13.33: unregistered, not
featureless).

## F15 — the persistent-body gate (`f15_persist.py`) — INTENT §11.121, 2026-07-30

    cd claude/harness && DISPLAY=:2 ./f15_persist.py [absOutdir] [--mutate]

Seven launches, ~4 min, on the real `~/.spacecrafter` (like `b24_equivalence`;
the shipped state — no enabled composed file — is restored in a `finally`,
whatever happens). It owns the app lifecycle; the caller launches nothing.

What it measures (b31-design §6.2 T4/T6/T9, §6.3), each leg named on stdout:
**T6** a body pushed with `body action load`, saved with `body action save`,
survives quit + fresh relaunch with the same parent/relation/module set/routing
— and so does every other body (**121 bodies, 0 divergent fields**); the
per-body comparator is IMPORTED from `b24_equivalence` (I2). **T6-CONTROL** the
same push WITHOUT the save leaves 120 bodies and no rover: the same assertion,
measured in both directions in one run, so T6 cannot pass for another reason.
**NO-DELTA** the live-tree save of the shipped tree equals the machine twin
**line for line** (4160 declaration lines, banner excluded) — two sources, one
answer; and the same comparison against a save WITH a body pushed shows exactly
that body's 22-line block, 0 removals. **T4** a second save of unchanged state
is byte-identical, in both regimes (built from a legacy tree; re-saved from the
composed file the first save produced — the reversible pair entered twice).
**PRESERVE** a save over an authored file gives back all 4169 of its lines
byte-identically (human comment, malformed line, unknown ISO-8859 key, irregular
spacing) and appends what it lacks. **ANNOTATE** a load leaves the file
byte-identical (D33: no rewrite at load), a save puts each diagnosis on the line
ABOVE its datum, a re-save is byte-identical, human comments are untouched; the
**negative control** removes the two diagnosable data and exactly their two
annotations disappear. **D9** across the save command itself, of 8077 files
under `~/.spacecrafter`, exactly the target changed. **REFUSE** a path and the
machine-owned `.disabled` name write nothing and say why. **§2(c) channel 2**: the
same command played from an `.sts` file (installed and removed by the script)
writes the same file — one registration serving both channels, measured rather
than inherited from §11.55(h).

**SCREEN** is the terminal observable and the leg is self-calibrating: the body's
footprint is measured WITHIN one launch (rover shot vs pre-push shot, **8462 px**,
no launch noise in it), then the live-vs-restored pair is split at it — **118 px**
moved inside the footprint (1.39 %), of which **1** survives a 2 px erosion, i.e.
the residual is the disc's own edge, which is what a cross-launch sub-pixel jitter
can move and all it can move. Its own counterfactual runs on the same frames: the
comparison against a rover-free frame reports **8308/8462 px (98 %)**.

`--mutate` deletes the `[F15Rover:HINT]` declaration from the SAVED file between
the save and the relaunch: that run is EXPECTED to fail and does, on
`modules ['MESH','AXIS','HINT'] != ['MESH','AXIS']` and `routing far 1 != 0` —
F15Rover only, the other 120 bodies still 0 divergent fields (plus T4's own
consequence of the same deletion). The SCREEN leg stays green there, correctly:
the deleted declaration is the hint marker, not what draws the disc.

Standing note: three shipped bodies (**Hyperion, Juno, Vesta**) declare neither
`rot_periode` nor `orbit_period`, so a COMPOSED load of them annotates the 24 h
default that acts (D12). That is why a composed re-save of a file built from a
legacy tree is 3 lines longer than the file it re-saves — predicted before the
run, and the gate asserts the difference is exactly those 3 lines.

**Instrument fix riding this task**: `b40_parity.py` run with no `<tag>=<binary>`
argument used to launch nothing and print `OK` (exit 0). It now refuses with a
usage line and exit 2 — the §11.101(g) vacuous-gate class, met again in the F15
battery run and fixed at the instrument.

## F20 — the §5.32 both-ways gate (`f20_s532.py`) — INTENT §11.128, 2026-08-01

Two legs, each self-certifying, each run on BOTH binaries:

    cd claude/harness && DISPLAY=:2 ./f20_s532.py [absOutdir]
    SC_BIN=/path/to/pre-fix-binary DISPLAY=:2 ./f20_s532.py [absOutdir] --prefix

`--prefix` INVERTS every expectation, so the counterfactual run asserts the
defect is PRESENT rather than merely failing to assert it is gone.

* **DESCEND** — ten `camera action descend coef 0.99` issued two ways: all ten in
  ONE socket write (they land in one frame) and one per send (one per frame).
  The discriminator is the DIFFERENCE between the two cadences WITHIN one binary,
  so the leg needs no claim about how many frames elapsed: the spread leg is its
  own control. Pre-fix 179.999429 vs 180.876177 km (876.7 m apart); post-fix
  0.000 m.
* **SPIN** — a `rot_periode 24` body (period 1 d, so the spin over dJD days is
  exactly 2π·dJD), anchored bound observer 1e6 km up, fov 30, looking away, so
  the reference falls OUTSIDE the cull cone and `dispatchUpdate` skips its
  `update()`. The leg COMPUTES the reference's angle off the view axis and the
  cone it must clear, from the dump itself, and fails if the scene did not put it
  outside — two earlier scenes did not, and the pre-fix null (0.000000° against
  36.000000 predicted) is what proves the third one does.

## F20 — the session-file gate (`f20_session.py`) — INTENT §11.128, b31-design §6.2

Twelve launches, ~20 min, on the real `~/.spacecrafter`; owns the app lifecycle
and removes the sessions it wrote.

    cd claude/harness && DISPLAY=:2 ./f20_session.py [absOutdir] [--mutate]

T2 (dump field-by-field after a QUIT and a fresh launch) with a T2-CONTROL taken
before the restore · T2-tracked · T4 (byte-identical second file + dump equality
at both exits of save→restore→save→restore) · T5a (a different scene's session
moves the dump; and restoring back is order-free) · T5b (one key edited by hand
moves exactly its own field) · T10 (a body hidden — frozen — for 30 simulated
days comes back where the never-frozen scene puts it) · §6.3 (of 8077 files,
exactly the session changed, measured across the save COMMAND).

**THREE SCENES, one per question, and that is a lesson rather than a
convenience**: under tracking, and under a live sky lock, `alt`/`az`/`heading`
and `lockedSkyRot` are DERIVED every frame, so a leg that needs "exactly one
field moves" cannot run on parameters the engine derives from each other.

**T1 (screen) is NOT MET** — see §5.63. The A/A floor is measured IN-SCENE by a
second launch that rebuilds the scene by commands (0 px>8 for scene A), and the
restored-vs-saved difference is 3185 px>8, photometric and unattributed.
`--mutate` breaks the saved file between the save and the relaunch.

**Both gates assert no other spacecrafter instance first** (§11.121(m)), matching
on the COMMAND being the binary (`ps -e -o args=`) — a `pgrep -f spacecrafter`
also matches the shell whose command line mentions the path, which is a
self-confirming instrument. This caught two orphaned instances holding port 7805
on 2026-08-01, which every socket in the session would otherwise have driven.

---

## F21 — the read half, the ledger, and the §5.63 probes — INTENT §11.129, 2026-08-01

    cd claude/harness && DISPLAY=:2 ./f21_flags.py  [absOutdir]
    cd claude/harness && DISPLAY=:2 ./f21_ledger.py [absOutdir]
    cd claude/harness && DISPLAY=:2 ./f21_s563.py   [absOutdir]   # §5.63, wave 1
    cd claude/harness && DISPLAY=:2 ./f21_s563b.py  [absOutdir]   # §5.63, wave 2
    cd claude/harness && DISPLAY=:2 ./f21_s563c.py  [absOutdir]   # §5.63, wave 3

**`f21_flags.py` — the flag surface's read half.** `AppCommandInterface::readFlag`
made the session file's `[flags]` section a READBACK, so the gate uses it as the
instrument for the refactor that produced it: for each of the 93 flags the file
carries, toggle twice, save, and compare **that flag's own value**. Side effects
on OTHER flags are printed, not failed — `flag atmosphere` deliberately drives
fog and star twinkle, which is old behaviour. The counterfactual (an ODD number
of toggles on three probes) must show them MOVED, or the identity leg cannot
fail and means nothing. Also asserts the per-§2-ROW exclusions: `track_object`,
`lock_sky_position`, `experimental_path`, `experimental_shadows` must NOT be in
`[flags]`, and `heading`, `home_planet`, `landscape_name`, `zoom_offset` must
NOT be in `[values]`.

**`f21_ledger.py` — the per-body override ledger, T7 and T8.** Overrides of every
group-D row that has a command, then: the ledger's own round trip through quit +
fresh launch + restore (with a CONTROL proving the fresh launch does not already
carry them), T4 byte-identity with the ledger populated, T5b on a hand-edited
ledger key, **T7** (a body renamed between save and restore — reported, applied
to nothing else, entry kept and annotated) and **T8** (an authored value changed
between save and restore — the new value in effect with the override on top).
T7's rename and T8's authored change both ride `body action load`, the runtime
declaration channel, so **no file in the frozen corpus is touched** and the md5s
are asserted in == out around the run. T7 and T8 need SEPARATE bodies: T8 needs
its body to still exist, so sharing one would make the rename resolve and the
miss could never fire (that is how the first version of this gate passed T7 for
the wrong reason).

**`f21_s563*.py` — the §5.63 probes, and they are ladders.** Wave 1 walks a
CONTENT ladder (stars → milky way → nebulae → atmosphere → landscape → planets),
shooting at each stage on three launches — saved, A/A floor, restored — so the
stage at which the difference collapses names the content that carries it; it
also shoots the restored scene at t0/t+15 s/t+40 s to separate a settling term
from a persistent one. Wave 2 walks a MECHANISM ladder: each stage re-asserts
ONE candidate state to the SAME value on both sides, so a stage that collapses
the difference names a divergent state and a stage that does not EXCLUDES its
state. Wave 3 does the same for the view frame. All three keep the in-scene A/A
floor as a column, because a ladder without a floor is a list of numbers.

**The lesson worth reusing**: the residual was recorded as *photometric with no
displacement structure* on the strength of a uniform mean and a bbox. Comparing
the two frames' lit SETS instead — 0 of 400 overlapping, median nearest
neighbour 50.2 px — says the opposite. A difference summarised by an average has
not been looked at.

## F22 — the old path's view-state readback and the §5.63 closer — INTENT §11.130, 2026-08-01

    cd claude/harness && DISPLAY=:2 ./f22_s563_view.py  [absOutdir]
    cd claude/harness && DISPLAY=:2 ./f22_b10_offset.py [absOutdir]

**`f22_s563_view.py` — the same three launches, with the dump the row was owed.**
Saved / in-scene A/A floor / restored, and at each one both a screenshot AND a
`body action dual_dump`, so the field table and the pixel table come from one
run. Every leaf of the header is flattened and compared; a field is a CANDIDATE
only if it differs saved-vs-restored **and** agrees in the A/A floor. That
second half is what makes it an instrument: the floor's own differing set is 26
leaves and every one is a CAMERA field (`alt`/`az`/`lockedSkyRot`/`mat`, ~1e-3,
because the sky lock captures at whatever frame the command lands on), while the
old-path fields agree between two rebuilds to **1e-6 degrees**.

**The control that does NOT apply, and why it is written down rather than
silently dropped.** The dispatch asked for a stars-off control — a field that
differs with the stars on and agrees with them off. View state does not behave
that way and should not: turning the stars off removes the CONSEQUENCE, and a
"cause" that vanished when you stopped drawing the thing it aims would be a
consequence. Measured: stars off, the screen agrees (43 px>8, floor 48) and 87
of the 88 fields still differ. The one that collapses is `stars.drawIdx`. The
discriminating control here is the A/A floor, and the both-ways flip is the
proof: pre-fix 3294 px>8 / direction 107.634° apart / 392 stars drawn against
689 — delivered 38 px>8 / 0.0° / 392 against 392.

**`f22_b10_offset.py` — §2 row B10, both ways, in two launches.** The old
spelling (`set view_offset`) must still be refused and move nothing; the
registered one (`set zoom_offset`) must move the offset on BOTH paths. Then the
latch: unarmed the scalar is stored and the effective offset is 0, a `look_at`
arms it on both paths, and a save→quit→restore brings back scalar AND latch on
both. The refusal is counted from the log **after the app exits** — the app's
stdout is block-buffered into that file, so a count taken while it runs reads 0
whatever happened (this leg failed on its own instrument first).

**Why the `look_at` matters beyond the offset**: it aims the OLD navigator away
from its launch default, so the restore leg is run against a direction a fresh
launch does not happen to start at — restored **0.00e+00°** from the saved one
against a control **101.999°** away. A fix that merely refreshed the stale
transform pair would land on the launch default and fail exactly there.

**Floor note that supersedes the numbers above it in this file**: the in-scene
A/A floor of a sky-LOCKED scene is **not 0**. §11.128/§11.129 recorded 0;
measured at 26–51 px>8 across six runs of two harnesses here. Measure it
in-scene every run (§11.80(a)) — every number in §11.130 carries its own.

## F23 — B33's readouts: what the control surface answers vs what draws — INTENT §11.131, 2026-08-01

    cd claude/harness && DISPLAY=:2 ./f23_b33_control.py <absOutdir> [--bin B] [--prebin B]
    cd claude/harness && DISPLAY=:2 ./f23_b33_inject_run.sh [absOutdir]      # SC_BIN=… F23_PRE=1

**The readback these two gates rest on is in the app, not in them.** `body action
dual_dump`'s header carries a `control` object — for every member of the B33
class, `{reported, old, new}` in the getter's own units, plus `drawnPath`. Before
it existed no member of the class could be measured on any binary: `get status
position` queues a reply that never arrives (§5.47), the view-offset readout's
one live reader is a TUI item, and the mount readout has no live reader at all.
A dump therefore discriminates by itself — `reported == old` on a binary that
reads the old authority, `reported == new` on one that reads the drawn path —
and the RED half is the same script against a pre-fix binary.

**`f23_b33_control.py` — the two members with a REAL channel.** Altitude:
`camera action descend` is new-path-only by design, so two shipped commands
split the authorities 4x, and `moveto multiply_alt 1` — a semantic no-op — then
moves the drawn observer **0 px>8** on the fixed binary against **738 266 px>8**
on the pre-fix one, which teleports it 10 000 → 40 000 km. Sky lock:
select-while-tracking sets `flag_lock_equ_pos` alone (four shipped sites do),
so the toggle read `true` while nothing held the sky; fixed, the toggle LOCKS
and the camera re-derives **15.0411°** over a sidereal hour, against **0.0000°**
pre-fix. Both branches of `Camera::getPlace()` are exercised (anchored and free).

**Two scene traps this gate hit, both worth reusing.** (1) Its first altitude
ladder sat at 200 km, i.e. inside `distance < 2·scaledRadius`, where the parent
draws its empty `groundedComponents` and therefore draws NOTHING (the
§11.97(e)/§11.100(g)(ii) hole — `b3_ladder.py`'s `earth_surface` site is the
same fact). Every screenshot had 28 lit px of 4.2 M and every screen leg read 0
whatever happened; the only leg that could catch it was the RED control. The
ladder now runs 40 000 → 10 000 km, outside the boundary at both ends, and
`lit()` fails the run on an empty frame. (2) Tracking the camera's OWN reference
body does not aim at it — `lookTo` bails on a zero-length direction and the
view stays where it was (measured: alt 0.042 rad after `select planet Earth` +
`flag track_object on` from 200 km above Earth, anchored). b3's recipe (free
mode, then select, then track) is the one that aims.

**`f23_b33_inject.py` + `f23_b33_inject_run.sh` — the two LATENT members.**
Nothing shipped writes one authority of the view offset or the mount without the
other (one writer; one config key), so the check is derived from the mechanism —
two authorities exist, the drawn one must be reported — and the divergence is
written onto the DRAWN path in the live process through b21_keypath's gdb-FIFO
instrument. `F23_PRE=1` asserts the DEFECT instead of the fix, so the red half
is an assertion rather than a leg that quietly does not run. Note for anyone
extending it: **`CameraMount::ALTAZ` is a syntax error in a gdb expression** —
cast the value (`(CameraMount)0`). The first run did not, injected nothing, and
the mount leg passed vacuously on two equal values; both `reported` legs now
require the divergence they report about.

**The regression half is a leg of `f23_b33_control.py`, not an argument.** The
shipped scene with every old-path layer ON (1 136 780 lit px), pre-fix binary vs
delivered: **1965 px>8 against in-run A/A floors of 2063 and 2052** — below
launch variance. What does move is the old observer's own altitude,
75.000000 → 75.104276 m, toward the drawn path: the camera holds the place as a
float AU distance, and `UI::init` re-applies the place through the dual seam, so
the two authorities now land on the same value instead of 0.104276 m apart.

## F24 — B34's mechanical seam mirrors: clear, preload, trail restart, the bookmark — INTENT §11.132, 2026-08-01

    cd claude/harness && DISPLAY=:2 ./f24_b34_seams.py <absOutdir> [--bin B] [--prebin B]
                                                       [--only clear|preload|position]
    cd claude/harness && DISPLAY=:2 ./b11_run.sh b11_trail_gate.py <absOutdir>   # SC_BIN=…

**The pre-fix binary must carry the INSTRUMENTS, and this wave learnt it the
expensive way.** The first preload RED half ran against the F23 HEAD and reported
*"nothing acquired on the pre-fix binary"* — that binary has no `bigTextures` key
in its dump at all, so the reader returned `[]` for the instrument's ABSENCE and
the leg was fiction. The delivery is therefore split in two commits, `0674d517`
(the provenance bit, the two counters, `s_texture::dumpBigTextures` and every new
mechanism function, **nothing wired**) and `d88f5be2` (the five call sites), and
`--prebin` points at a build of the first. Same rule as §11.131(a), one layer up:
build the RED binary from the instrument commit, never from the commit before it.

**Three readouts this wave added, all read-only.** `supplemental` per body (the
provenance bit `body action clear` selects on — a clear only ever shows what it
TOOK, so without this a mistyped mark passes vacuously); `preloadCount` per body;
and the `bigTextures` header array (`s_texture::dumpBigTextures` — the table a
preload writes into; its only previous reader was `debugBigTexture()`, which has
zero callers). The table reader must never call `getBigTexture()`, which acquires
and refreshes lifetimes: an instrument that performs the act it reports is not one.

**`f24_b34_seams.py`.** Each member both ways, through its command, on a live app,
in a temp-HOME farm (`b25_galactic.build_farm`) so the `body action save` leg
writes into this run's own `modularSystem/` and never into the installed data.
CLEAR pushes three bodies — plain, `hidden true`, and a CHILD of the plain one
(hidden because old's rule walks `systemBodies`, which hidden bodies are in;
the child because the new tree owns children by `unique_ptr`) — and measures
90 601 px>8 across the clear against 0 px>8 pre-fix, where all three survive on
the new path alone (`"old":null` in the dump's new-only section, the §11.3
channel). PRELOAD needs a subject OLD CANNOT SEE, because the two paths share one
`texRecap` per file name: `body action load name F24Only parent SolarSystem …`
names a system NODE, so old's `addBody` refuses the push and the new loader
accepts it. POSITION uses the shipped spelling `position action save` /
`position action load` (not `position save` — an earlier revision of this script
sent that, got *"unknown parameter"*, and measured a no-op as a failure to
restore); the scene moves in ALTITUDE only, because a lat/lon swing takes the disc
out of frame and the screen legs then compare two 2834-lit-px frames.

**Its residual is attributed, not tolerated**: a save/load round trip loses
**4.353871 m** at 10 000 km, identically on both entries, which is **4.000000
float32 ulps** of the camera's AU distance (1 ulp = 1.0884678 m at 6.6846e-5 AU) —
the camera holds the place as a float AU distance and free-flight `getPlace()`
derives it from `position`. 6 px>8 on a 831 386-lit-px frame.

**`b11_trail_gate.py` phase 5 — the restart that is not the display flag.** The
flag stays ON throughout (the only regime where the two are distinguishable) and
the pair is entered twice, Earth → Mercury → Earth, each entry with its own
accumulated span. **The check is the SPAN** (head jd == tail jd, `pathLength` 0),
not the head's position or date: a home-planet change moves the observer between
planets, so the light-time-corrected date the trail samples at shifts by minutes
and the surviving point legitimately sits thousands of km from where the body is
by dump time (measured: Mars 8006.57 km, −5.7 min). Delivered 39 → 1 points and
570.0 d / 8.02 AU → 0; pre-fix 39 → 39 with the span intact, and **exactly those
12 assertions differ between the binaries** — the gate is re-pointed, not loosened.
The seam's other live caller, the config-init call, is inert by construction (no
body has a point yet) and is stated rather than tested.

## F25 — the interactive VIEW and ZOOM ramps (`f25_ramp.py`, `f25_drag.py`) — INTENT §11.133, 2026-08-02

    cd claude/harness && DISPLAY=:2 ./f25_ramp_run.sh <absOutdir> [--phase frames,turn,diag,fov,zoom]
    cd claude/harness && SC_BIN=$PWD/sc_f25_pre F25_PRE=1 DISPLAY=:2 ./f25_ramp_run.sh <absOutdir>
    cd claude/harness && DISPLAY=:2 ./f25_drag_run.sh <absOutdir>        # gdb-driven, see below

**The claim is PER-STEP, so the instrument is too.** `body action dual_dump` carries
a `"ramp"` object: one row per `Core::updateMove` frame in which a ramp is active
**plus the first frame after it stops**, with the frame's inputs (`dt`, the OLD
projector fov, the DRAWN `ModularBody::halfFov`, the scaled steps, the joypad
coefficients) and BOTH paths' view parameters before and after the step. Two
absolute dumps around a key hold cannot say *same law, same step, same count, same
stop*; this can. The release row exists so a key-up is OBSERVED and not inferred
from an absent row.

**Compare in VIEW space, never in the camera's parameters.** The camera's `az`/`alt`
are the NEGATIVES of the view azimuth/altitude (`Camera::paramForward`). The
`frames` phase measures that rather than assuming it, and it is a DISCRIMINATION
between the two candidate signs, not an absolute bar: with both paths aimed at the
same body on five bodies over 140° of azimuth, `alt_cam + altVision_old` spreads
9.8e-08 rad while `alt_cam − altVision_old` spreads 0.81 rad. The residual there is
the AIM residual (two aiming laws, two trees' positions for one body), which is why
a bit-level bar would be the wrong instrument.

**Bars are derived, and one of them was wrong in an instructive way.** Per-step
1e-06 rad (2 float32 ulps of an O(1) parameter + one ulp for the wrap); cumulative
5e-05 (0.5 ulp per step, random walk over ~360 steps = 1.1e-06, worst case 2.2e-05
= 0.014 px). The pole leg first FAILED at 4.418e-06 on the delivered binary, and the
row that failed was the **clamp transition**: old does not add `deltaAlt` there, it
PINS, so that row's size is *whatever reaches the pin* and it absorbs the float32
divergence accumulated before it. Clamp rows now go to the pin check, which asserts
the exactly-computed gap between old's double `π/2 − 1e-6` and the camera's float32
form of the same expression — **9.0037e-08 predicted, 8.997e-08 measured**.

**The composed-screen leg needs `flag stars off`, and forgetting it reproduces F4's
own confound.** The star field is drawn by the OLD pipeline in BOTH phases, so with
it on the "new phase" moved **1861 px>32 of 2217 lit on a PRE-FIX binary whose
camera never moved a bit**. With the sky off and the leg at fov 10 the lit content
is the body the new path draws, and the pair discriminates completely: pre-fix
**0 px>32** of 857 lit while the old phase moves 916 and the two phases go 213 →
**999** apart; delivered **1087** and the two phases stay at 232 against 213 at rest.
The Moon is a CRESCENT at this date, so 857 lit px is a fraction of its ~114-px
disc — the content guard is scene-derived, not an absolute.

**`xkey.c` takes a comma-separated keysym list** (`Left,Up`), pressed together and
released in reverse: the diagonal needs two `vzm` components live in the same frame,
which one key cannot produce.

**`f25_drag.py` is gdb-driven because the drag channel is DEAD on this host.**
`xdrag.c` reports the pointer after each fake motion and measures it: `XQueryPointer`
returns **(0,0) after every one of eight `XTestFakeMotionEvent`s** while
`Button1Mask` is held — the root window is 0×0, so XTEST pointer MOTION goes
nowhere. A fake BUTTON event still lands, which is why `xclick.c` has always worked;
a drag is press + MOTION + release. So `Core::dragView` is called in the live process
on the b21 gdb-FIFO pattern — the exact function `UI::handleMove` calls, one layer
below SDL (§11.108(d)'s precedent, residual stated). Keep `xdrag.c`'s step report:
without it a fake motion that goes nowhere reads as "nothing moved".

## F26 — §5.62's isolation: the same mid-band scene on two named-commit binaries, in ONE epoch (`f26_epoch.sh`) — INTENT §11.134, 2026-08-02

`f26_epoch.sh <label> <binary>` does one fresh-launch run of **F18's scene, unchanged**:
it drives `f18_run.sh` + `f18_disc.py` and adds the preconditions §5.62 itself checked, as
recorded asserts — wall clock, binary md5, concurrent-instance count, frozen
`config.ini`/`ssystem.ini` md5 in and out, the enabled-`modularSystem` set,
`beta_features.ini`, and the `t-*.dat` texture-cache listing before/after. Artifacts land
in `artifacts/f26/<label>/` (`f26_meta.txt` is the per-run record).

**Two instrument facts that cost a re-run to learn — keep them:**

- `f18_run.sh` clears `*.log`, `*.json`, `*.png` in its outdir as its FIRST act, so any
  file a wrapper writes there before invoking it (including the redirect capturing its own
  stdout) is unlinked while still open. Wrapper files use `.txt`.
- **The concurrent-instance assert must not be a `pgrep -f <path>`.** `f18_run.sh`'s own
  pattern (`spacecrafter/build.*/src/spacecrafter`) cannot see a binary built outside the
  code tree; and any pattern that can see it also matches the wrapper's own command line —
  measured: 3 reported with nothing running. `f26_epoch.sh` reads `/proc/<pid>/comm`
  instead (executable name, world-readable ⇒ covers every account, no command-line text),
  and it is positively mapped both ways: 1 with a decoy named `spacecrafter`, 0 without.

**The A/A floor of the mid-band disc measurement is per-body and NOT zero** — measured over
two same-binary launches in one epoch, §11.134(d): Sun **0** (new_disc 19666 bit-identical),
Jupiter **4 counts = 0.10 %**, Mars **39 counts = 0.57 %**; the pre-§5.52 binary repeats
bit-exactly on all three. Old-path discs are 23536 / 7683 / 3908 and have been bit-stable
across every run of both epochs. Read any new/old ratio against those floors.

**Rebuilding the pair** (staged binaries `sc_f26_pre` md5 `df00c3e3` = `96a94a46`,
`sc_f26_child` md5 `23ac7fdb` = `2117ccb0`, both untracked):

    git worktree add --detach /home/claude/sc-f26/wt-pre 96a94a46
    git -C /home/claude/sc-f26/wt-pre submodule update --init      # 224eba7a, same at both
    cmake -S /home/claude/sc-f26/wt-pre -B /home/claude/sc-f26/build-pre \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -C … -j8       # never `make install`:
                                                                  # only install touches shaders

### `f27_reply.py` — where a `get`'s answer goes, measured on the wire (INTENT §5.47 / §11.135)

    cd claude/harness && DISPLAY=:2 ./f27_reply.py <absOutdir> --bin <binary> \
        --expect pre|post [--legs A,B,C,D,E,F]

One script measures BOTH binaries; `--expect` selects which direction each
discriminating leg must go. `sc_f27_pre` (untracked, md5 `42f83cd3` = code
`d9de42ac`) is the pre-fix binary; the delivered one is `8a93ca97` = `d13681eb`.

**The fact the legs are built around**: the output queue was never stuck. It is
drained on every pass of `ServerSocket::run`, through `broadcast`, which
addresses the clients that subscribed to the feedback channel with `$LOGON` -
so the answer went to the log subscribers and, with nobody subscribed, was
popped off the queue and lost. Leg B is the load-bearing control: the SAME
socket, the SAME command, on the PRE-FIX binary, answers in 0.002 s once it has
sent `$LOGON`. Without that leg, "no reply" cannot be told apart from "no
instrument" - the F26 lesson, in the smallest form it takes.

  A  plain driving socket: the §11.118(i) scenario (6 s poll, twice) + the six
     commands §5.47 lists as working on that same connection + the answer's
     CONTENT against `body action dual_dump`'s `control`, in a scene where the
     two heading authorities are 6.16° apart
  B  the same socket after `$LOGON` (the positive map; also asserts the
     delivered binary does not send TWO copies to a subscriber that asked)
  C  subscribed listener + plain issuer, then the issuer's connection replaced
     in the same slot: an answer follows the CONNECTION, not the slot number
  D  a `get` issued by a SCRIPT: must reach the subscribers and never the last
     client that spoke (the latch must clear when the command batch drains)
  E  the read-only heading pin: 3 samples across two `experimental_path`
     toggles in ONE launch (0/3 pre-fix) - use this instead of
     `heading delta_azimuth 0`, which writes both authorities after reading
  F  F1 an answer nobody can receive (script `get`, no subscriber): the app
     must SAY so; F2 the HTTP channel, which pushes a command and hangs up in
     the same pass, so its answer always outlives its issuer

**Instrument note, learned by a false FAIL**: the app writes six log files at
once. Marking a position by the LENGTH of their concatenation makes "what was
logged since" the tail of the last file, and a line written to the fourth is
invisible - leg F1 reported a diagnostic missing that was present. `logmark()`
/ `lognew()` mark per file.

**Reply parsing**: `ServerSocket::send` writes `strlen(buffer)+1` bytes, so
every message on the wire carries its terminating NUL, and the queued answers
end in `'\n'`. Split on NUL, then match. Note that the harness's other driving
helpers (`b25_galactic.run_phase`, `f23_b33_control.App.send`) `recv()` into
the void after each command - which is why a missing answer was never visible
from a harness script until this one kept what it read.

## F28 — how long an answer is, and where it lands (`f28_send_buffer.py`) — INTENT §5.73 / §11.138

    cd claude/harness && DISPLAY=:2 ./f28_send_buffer.py <absOutdir> --mode census
    cd claude/harness && DISPLAY=:2 [ASAN_OPTIONS=halt_on_error=0:detect_leaks=0] \
        ./f28_send_buffer.py <absOutdir> --mode overflow|logon --tag <name> \
        --bin <binary> --expect pre|post

Launches through `f27_reply.Session` (I2 - same concurrent-instance assert, same
frozen-md5 assert, same farm), so the two scripts cannot drift apart on what a
fresh launch means.

**`census`** - one launch, the row's owed datum: `get status object` for five
planets and a nebula, the `maxobject` ladder, the whole a-z first-letter surface
of `search`, and the `get status planets_position` body-load ladder. Measured on
the shipped corpus: object info **114-142 B**; `search` **53-1024 B** with
prefix `n` reaching the clamp; `planets_position` **854 B**.

**`overflow`** - the same six answers on any binary, shortest first:
854 / 854 / 916 / **1022** / **1023** / **1024 clamped**. 1022 is the last
length that FITS a 1024-byte buffer once `'\n'` and the terminator are added, so
the 1022/1023 pair is a one-byte-wide discrimination on one code path. The
lengths are computed INSIDE the launch from what it measures (one
`planets_position` entry costs `name + 32 B`), never from a constant carried
between launches; 1023 cannot be reached from 1022 by adding (an entry costs
>= 33 B), so that step drops the body and puts it back with a one-character
longer name. Every answer is written to `<tag>_step<N>_reply.bin` - pre/post
identity is `cmp`, not a claim.

**`logon`** - the fixed answers `computeNormalString` used to `strcpy` into the
receive buffer, driven as the reversible pair they are: `$NOTICE`, `$LOGON`,
`$LOGON` again (`REQUEST ERROR`), `$LOGOFF`, `$LOGOFF` again, `$LOGON` a second
time from the state the first exit left, and a `get` inside and outside the
subscription.

**Two spellings this script had to learn the hard way** (both caught by controls,
not by reading): the `search` argument is `maxobject`, not `max_object`
(`base_command_interface.hpp:100`) - with the wrong one every rung of the ladder
answered the default 5 and the growth control failed; and the `get` argument is
`planets_position`, not the macro's name `planet_p`.

**ASan note**: `build-asan` needs `cmake .` before `make` if the file list moved
(it failed to link on `SessionFile::save/load` after F20/F21). Report counts are
DISTINCT-PC counts - ASan's `suppress_equal_pcs` default means a second overflow
at the same `strcpy` is silent - so pre/post claims are presence vs absence.

### `f29_upchain.py` / `f29_run.sh` / `f29_compare.py` — §5.46, where an up-chain ancestor's line draws

    cd claude/harness && ./f29_run.sh <absOutdir>                    # post-fix binary
    SC_BIN=/abs/pre-fix-binary ./f29_run.sh <absOutdir>              # counterfactual
    ./f29_compare.py <pre_outdir> <post_outdir>                      # exit 0/1

`f29_upchain.py`'s header IS the prediction (committed before the first run,
harness `2af2ee1`); `f29_compare.py` evaluates the cross-binary halves. Five
legs in one launch: **E** observer on Earth · **S** system centre · **X** on
Mars with the Sun tracked (the FREEZE SOURCE) · **M** on the Moon at fov 340 ·
**N** the same instant at fov 140 with the subject tracked.

Things this scene had to learn, each one measured, each one a trap for the next
line-drawing gate:

- **The ORBIT pass is depth-bucketed** (`Renderer.hpp:156-167`): from a moon's
  surface `flag planets_orbits on` adds **0 px** — no planet orbit line reaches
  the frame at all. The TRAIL pass is depth-free and is what a line-placement
  gate can read. The three consumers (orbit/trail/tail) share one expression,
  so the trail's verdict is theirs.
- **The trail geom shader drops segments longer than 0.4 NDC** unless
  `main_clipping_fov[2] < 2.7` (`body_trail.geom:19-24`). A trail's newest
  vertex sits AT its body, so for a body one moon away the first segment spans
  ~84° and the head never rasterises at fov 340. Read line ENDS at fov <= 140.
- **`TrailModule::accumulate` truncates to int** and `date` is light-retarded,
  so a `date jday` step of exactly `DeltaTrail` (1.0 day) records NOTHING —
  21 of 30 samples taken, newest a full day stale (→ §5.75). Step **1.5 days**.
  The instrument reports `trail_head_lag_days` per body so a stale head
  disqualifies the reading instead of being read as fresh.
- **Isolate a trail by a COLOUR DIFFERENTIAL, not by hue**: shoot it black, then
  coloured, and diff. Recolouring does not touch the recorded points; body
  discs are themselves red-dominant and had otherwise supplied the "brightest
  red pixel".
- **The gate is the CLOSEST APPROACH of the polyline to the predicted point**,
  not the head pixel: the head pixel is `argmax` over a quantised vertex alpha
  whose level spans ~74 px along a long segment.
- `halfFov` is SOLVED OUT of the dump (`|screen| = acos(-z/d)/halfFov`, agreeing
  to 8e-7 across bodies) and the y-flip is picked by the control body — the
  projection is reconstructed, never assumed.

Measured verdicts at delivery (§11.139): subject trail **550.39 px** from its
body pre-fix / **9.2e-05 px** post-fix, control body **0.69 px on both**;
invariant `eclRoot == mat.translation` violated by exactly the up-chain pre-fix,
by nobody post-fix over 120 bodies; control scenes bit-identical.

`f29_reversible.py` (same runner) drives the pair this fix touches — a body's
membership of the walk, DESCENT <-> UP-CHAIN — as `Earth -> Moon -> Earth ->
Moon -> Earth`, asserting at every state both the invariant over 120 bodies and
the subject's own cached frame in km. It is the shortest statement of §5.46
there is: on the PRE-fix binary, standing on the Moon, Earth's frame says Earth
is **6378.240 km** away (its own radius plus the observer's 100 m) instead of
**359 624 km**, at both entries.

## F31 — which of the two it is (`f31_search_drive.py`) — INTENT §5.74 / §11.141

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f31_search_drive.py <absOutdir> [--bin <binary>]

§5.74 owed a discrimination: `search` returns no `(S)` and no `(C)` because the
star/constellation NAME catalogues are not loaded, or because the prefix match
never fires. Both candidates are read on ONE launch, each at its own surface.

- The app runs UNDER gdb (`f31_search.gdb` + `f31_probe.py`, 12 breakpoints,
  B10-cmd precedent — ptrace_scope=1 blocks attach). Every breakpoint is silent
  and continues, so the driver on the other side sees latency, never a hang.
- **The probe writes to its own file, not to gdb's stdout**: gdb's stream is
  block-buffered when redirected, and evidence still in a buffer at the end of a
  run is a silent no-op probe (§11.47). Path via `$F31_PROBE`, set by the driver.
- **The catalogue's own count** is printed at each of the four
  `listMatchingObjectsI18n` entries — the container the match loop walks, plus
  the `maxNbItem` quota. Planets and nebulae are read on the SAME channel in the
  SAME call: they are the positive control, not a second instrument.
- **The load sites are positively mapped both ways by the run**: the two
  breakpoints that must read 0 in phase 1 (`loadLinesAndArt`, `loadCommonNames`)
  are the two that must read nonzero in phase 2, so a phase-1 zero is a
  measurement rather than an unresolved symbol. `no_pending_breakpoints` is
  checked against gdb's own `info breakpoints` in the applog for the same reason.
- **Phase 2 loads a sky culture from a fixture OUTSIDE the frozen field** with
  the shipped `sky_culture action load path <abs dir>`; nothing under
  `~/.spacecrafter` is written and the frozen md5 pair is asserted in == out.
  `star_names.fab` is a verbatim, md5-asserted copy of the installed
  `stars/name.fab`; the two constellation files are SYNTHETIC and labelled
  (`Zzprobe*` over HIP ids read out of that same real file) because no
  constellation data exists on this host to copy.
- **The prefix comes out of the LIVE index**, parsed from the probe's own sample
  of `common_names_index_i18n`, never typed (§11.51(d)). It is one character
  because the installed star-name file has no ASCII name with two leading
  letters — which also makes the post-load command byte-identical to one of the
  36 phase-1 commands, so pre/post is one command compared with itself.
- The sweep is **36 prefixes** (26 letters + 10 digits): the index is
  Bayer/Flamsteed, so most of it is keyed on names beginning with a digit.
- Controls are compared only on prefixes whose post answer is under 1024 B: a
  clamped answer can lose `(P)`/`(N)` entries to the newly interleaved `(S)`
  ones, which is the clamp and not a change of catalogue.

`f27_reply.Session` gained `launch_prefix` (argv prefix, used here for gdb) and
`port_wait`. Default behaviour is unchanged, and that was measured rather than
asserted: `f27_reply.py --legs A --expect post` on the prefix-free path is
**0 FAIL** after the edit (`artifacts/f31/f27_regression/`).

Measured verdict at delivery (§11.141): phase 1 planets **90** / nebulae **407**
vs constellations **0** / star index **0**, sweep **P 84 · C 0 · N 243 · S 0**,
loaders entered **0 times**, culture gate rejected once; phase 2 catalogues
**3** / **3183**, same 36 commands **P 84 · C 3 · N 241 · S 1085**, live-index
prefix `1` going **0 → 104 (S)**. 20/20 checks PASS, app exit 0, md5 in == out.

## F32 — what an `Object` assignment lets go of (`f32_object_leak.py`) — INTENT §5.34 / §11.142, 2026-08-09

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && export DISPLAY=:2
    ./f32_object_leak.py <absOutdir> --mode discover --bin <binary>          # HIP ladder
    ASAN_OPTIONS=detect_leaks=1:halt_on_error=0:malloc_context_size=25 \
      ./f32_object_leak.py <absOutdir> --mode leak  --tag asan_pre  --bin <asan bin> \
        --expect pre  --stars 9 --port-wait 300 --exit-wait 300
    ASAN_OPTIONS=...  ./f32_object_leak.py <absOutdir> --mode churn --tag churn_post \
        --bin <asan bin> --rounds 3 --port-wait 300 --exit-wait 300
    ./f32_object_leak.py <absOutdir> --mode render  --tag nat_pre --bin <native bin>
    ./f32_object_leak.py <absOutdir> --mode compare --a nat_pre --b nat_post [--floor N]

Four modes, one launch each, all through `f27_reply.Session` (so the /proc-comm
concurrent-instance assert, the temp-HOME farm and the frozen-md5 pair come with them).

- **`discover`** asks the app which HIP ids its own catalogue answers for and writes
  `f32_hip.json`, which the other modes read: the ladder is DATA, never recalled
  (§11.51(d)). On this install **3 of 14** swept ids resolve — the star index is sparse,
  reproduced on two sweeps, same family as §5.74.
- **`leak`** drives the selection surface on both paths and reads LeakSanitizer's report
  at `shutdown action now`, split **per allocation site** (`Star1::createStelObject` for
  the old path, `SSystemFactory::searchObjectByEnglishName` for the new one), so the count
  is per-defect and not a heap total. The predicted counts are written to
  `f32_predict_<tag>.json` BEFORE the launch. Measured: pre 11 + 8, post 0 + 0, with the
  rest of the heap unmoved.
- **`churn`** is the use-after-free hunt: mixed-type churn (the planet/nebula names come
  out of the app's own `search` answer), reassignment while tracking, `mode jump` both
  ways, `body action reload` under a live composed selection — every reversible pair
  entered twice.
- **`render`/`compare`** are the screen A/B. **Read the A/A first**: this scene's
  launch-to-launch floor is ~4000 px>0 / ~20 px>8, from a ~2.9 % tone-adaptation
  luminance scale plus one bright object displaced 8 px by the unpinned startup view.
  Pinning the view with `select Sun` + `flag track_object on` was tried and is 16× WORSE
  (A/A 67593 px>0) because tracking convergence is itself launch-dependent (§11.94(d)).

The composed bodies are **b24_select's fixture, imported** (I2) and written into the farm
by `Session`'s new `prepare` hook; `Session` also gained `env_extra` (ASan options) and
`stop(exit_wait=)` (an ASan leak check takes real time after main returns). All three
default to inert.

**A zero from a sanitizer needs a positive map.** `parse_sanitizer` was run this epoch
over a fresh F28 overflow launch of `harness/sc_f28_asan_pre` and reported
**3 `heap-buffer-overflow`**, against 0 on all four F32 runs
(`artifacts/f32/f32_lsan_blocks.txt`, tail).

## F35 — the two degenerate-input guards (§5.81 distance-0 `screenPos`, §5.79 empty `getSelected`), INTENT §11.145

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f35_degenerate.py <absOutdir> --bin <binary> --expect pre|post \
              [--legs A,B1,B2,B3]
    DISPLAY=:2 ./f35_branch.py <absOutdir> [--bin <binary>]
    ./f35_compare.py <pre_result.json> <post_result.json>

`f35_degenerate.py` drives each degenerate input through a **shipped** command with a
positive control beside it, and its predictions P1–P4 are in the docstring, committed
before the first measuring run. Leg **A** = `camera action transition_to target point`
(the dump's `screen` for `temp_point`: `[nan,nan]` pre, `[0,0]` post, `dist` 0, with 120
other bodies finite either side). Legs **B1/B2** = `select constellation_star` on a
never-grown and on a cleared selection vector — the two manifestations of one root
(SIGSEGV rc −11 / a silent re-selection of `43 And`). Leg **B3** is what the fix
deliberately does NOT cover (§5.87). B2/B3 build **F31's fixture culture** outside the
frozen field via `f31_search_drive.build_fixture` (I2 — the fixture is imported, not
re-authored).

Three instrument facts worth not re-learning:

- **`dual_dump` nests per-path state under `old`/`new`.** `o["new"]["screen"]` is §5.81's
  subject; a new-tree-only body (the point anchor) has `"old": null`.
- **`get status object` embeds alt/az and hour angle**, which move between two reads of
  the SAME selection. Identity is the first two lines (name + HP), never the whole string.
- **A cross-launch dump diff is NOT an inertness instrument.** `f35_compare.py` reports
  278 differing fields between two runs of the same scene on two binaries; the control
  that proves the instrument rather than the fix is at fault is that the **untouched old
  path** moves the same way (`old.ecl` 22 bodies, `old.matLocalToParent` 22, `old.mat`
  19). `evalCount` differed 2015 vs 1991 and the iterative solvers ride the evaluation
  history. Use `b24_equivalence` (within-launch) and the branch probe instead.

`f35_branch.py` + `f35_branch.gdb` observe the guard branch itself, and the reason they
exist in this shape is a probe that lied: **`break ModularBody.hpp:508` resolves to the
line's statement start**, which the compiler placed on the COMMON path (the `ucomiss` of
`distance == 0.f`), so it fired **22613** times with Earth, Moon and Mercury among the
bodies. The address is taken from the branch body's own instruction instead (`movlps` at
`+72`, resolved after `start` because the binary is PIE) and the landed instruction is
printed into the run's log as the probe's positive map. Measured 0 (no transition) /
1298 (after it), every hit `temp_point`. **`handle SIGUSR1 nostop noprint pass` is
mandatory** — the app's stall watchdog otherwise stops the inferior and a batch script
then quits, killing the app mid-run (measured: the port never reopened for leg 2).

## F36 — which startup failures never reach the app's log (`f36_*.py`), INTENT §5.77 / §11.146

Four pieces, each the authority for one step, so a later run cannot measure a site
that was never enumerated or classify one that does not exist (I2).

**`f36_enum.py` — the census.** Writes `artifacts/f36/f36_sites.json`. Two things a
`grep` gets wrong here, both learned by getting them wrong:

- **Comments.** `grep 'std::cerr\|std::cout' src/` returns 335 hits (307 outside
  EntityCore) and only 138 of the project ones are live: **169 are commented-out
  debug prints**, two of them (`checkConfig.cpp:505,508`) inside a `/* … */` block
  that a per-line `//` test cannot see. So the stripper is a real character-state
  machine (code / `//` / `/* */` / string / char) that blanks comments while
  preserving line and column numbers — an enumeration whose line numbers do not
  open in an editor is not evidence.
- **Channels.** The class does not live on iostreams. The largest failure-report
  cluster in the app is `ZoneArray::create`: 13 `printf`, 1 `fprintf(stderr, …)`,
  1 `std::cout`. `main` uses `SDL_Log`. `sprintf`/`snprintf`/`fprintf(<file>, …)`
  are excluded by negative lookbehind and by requiring the stream argument.

**`f36_probe.py` + `f36_reach.py` — is the site on the startup path.** Run as
`DISPLAY=:2 ./f36_reach.py artifacts/f36 --bin <abs path>`. The measurable unit is
the **enclosing function**, not the line: a failure-report line does not execute on
a healthy startup, so reading a good launch's console enumerates what FIRED, never
what COULD. One `gdb.Breakpoint` per function whose `stop()` records, sets
`enabled = False` and returns False — the inferior is never left stopped, and a
breakpoint on `FilePath::FilePath` costs one stop for the whole run instead of one
per call. **The boundary is itself a breakpoint** on `App::startMainLoop`, so
"during startup" is read off the same channel as the hit.

Two habits worth copying:

- **The MANIFEST line.** Before `run`, the probe emits one line per breakpoint with
  its resolved location count. Without it, an unresolved breakpoint and a function
  never entered give identical evidence — silence (§11.47). It paid immediately: the
  first launch produced 77 pending breakpoints all reporting `locations=0` because
  `--bin` was relative and `Session` launches with `cwd` = the farm, so gdb started
  with no executable. **A full table of zeros reads exactly like a finding**; the
  manifest made it one look. `--bin` is now resolved and existence-asserted.
- **Read the applog as a second surface.** Two specs did not resolve (class-body
  inline members, `-O2`; gdb says "No compiled code for line …"), and one of them,
  `Executor::onAltitudeChange`, demonstrably fired **5×** during startup. The
  breakpoint hole was covered by the app's own output on the same run.

**`f36_class.py` — the classification and the sizing.** Carries a verdict per site
(`klass` / `logged` / `blocker`) with the observation supporting it, and prints the
row's number. `logged = PARTIAL` is the interesting bucket: the log records the
ATTEMPT and never the outcome, so it does not merely omit — it implies success.

Result on `04ae1d3e`: 214 live sites (186 project) in 76 functions; 25 functions
entered during startup; **37 startup failure reports the app log does not carry**;
and every one of the 37 carries a blocker, so the uniform additive `cLog` routing
§5.77 expected does not exist. See §11.146(f) for the five blocker kinds and (j)
for the one question that decides the fix.

## F39 — D21's two layers, and the `flag moon_scaled off` line's new reading — INTENT §5.27 / §5.102 / §5.103 / §11.152, 2026-08-26

    DISPLAY=:2 ./f39_d21.py <absOutdir> pre=/abs/binary post=/abs/binary
    DISPLAY=:2 ./f39_scenes.sh <leg> <abs binary>          # the A–D battery, one leg
    ./f39_cmp.py <dirA> <dirB>                             # field-by-field, the §11.150(m) shape
    DISPLAY=:2 ./f39_farm.sh <outdir> [--sed EXPR] [--env K=V] [--driver P] [--dwell N]

`f39_d21.py` runs the §11.78(a) mandate scene — a composed OJM rover grounded on
the Moon — **under the shipped config**, `moon_scale = 5`, no `flag moon_scaled
off`. Two binaries through the same commands on one temp-HOME farm (symlink
mirror; `config.ini`, `log/` and `modularSystem/` are the only real entries, so
the field pair 03fbee59/545a51ef is asserted in==out around every run).

The gate that carries D21's ratified acceptance criterion is **one scalar**:
rover extent over the Moon's DISPLAYED datum, which *"visually identical to
unscaled"* says must not move. Pre-fix: 1.38582 unscaled → **0.27716** scaled
(exactly the rival reading §11.151(b) killed) and 1.0249 / 0.3050 while the ramp
is in flight. Post: **1.3858165 ± 1.5e-7 across all seven legs**, mid-ramp
included. A ratio, not a pixel count, because the observer's own altitude is
NOT dilated (`moveto alt` is real metres above the displayed surface), so the
apparent size legitimately differs while the neighbourhood stays self-similar.

Rare paths, each traversed twice with the second entry starting from the state
the first exit produced: the scaling toggle and hide/show of the grounded child
(the path the new `hiddenBodies` push exists for). Every exit screen is
BIT-IDENTICAL to the first (`post_scaled` == `post_settled` == `post_shown2` ==
`post_settled2`, max abs diff 0 over 2048×2048).

**Standing-rule change: `flag moon_scaled off` is no longer a workaround.**
§5.27 is closed; a grounded child inherits its parent's display scaling. The
line STAYS in `b24_screen`, `b3_ladder`, `b24_select`, `f23_b33_control`,
`f24_b34_seams`, `f25_ramp` and `f29_upchain`, re-read as a **scene
declaration** — those harnesses' subject is the REAL geometry and every one of
their committed baselines was measured with scaling off, so removing the line
would silently change what they measure. `f39_d21.py` is the scaled twin.

**`dumpread.py` is the dump channel's single reader** (§5.103). It now holds the
grammar `b24_equivalence.sanitize_nonfinite` used to hold — that name is
re-exported unchanged, so all fifteen importers are unaffected — plus two things
F39 added: `unquote_nonfinite`, because the new path's emitters now QUOTE their
non-finite values (`src/experimentalModule/JsonNum.hpp`) and every consumer
compares numbers; and a `missing_old` list, because a COMPOSED body has no
old-path twin and `rec["old"].get(...)` on it raised AttributeError mid-analysis
in `analyze.py`.

## F40 — the freeMode converter as the composer's exact inverse — INTENT §5.80 / §5.106 / §11.153, 2026-08-26

    g++ -O0 -std=c++20 -I../../src -o /tmp/f40_probe f40_probe.cpp && /tmp/f40_probe
    DISPLAY=:2 ./f40_inverse.py <absOutdir> --bin B --expect pre|post
    DISPLAY=:2 ./f40_disc.py    <absOutdir> --bin B --tag pre|post   # the rover discriminator
    DISPLAY=:2 ./f40_env.py     <absOutdir> --bin B --tag pre|post   # the four flag cells
    DISPLAY=:2 ./f40_anchored.py <absOutdir> --bin B                 # the anchored regression
    ./f40_cmp.py <preOutdir> <postOutdir>                            # its comparator
    DISPLAY=:2 ./f40_scenes.sh <leg> <abs binary>                    # A–D battery, one leg

**`f40_inverse.py` is ONE instrument for BOTH conventions.** Every claim is scored
against `H_A` (`position = spheToRect(-lon,lat)·distance`, the pre-fix converter)
AND `H_composer` (`position = -posePart(lon,lat,distance)`), and `--expect`
chooses which one the gates ask for; the printed numbers are identical either
way. A pre binary answers 29/29 and a post binary 32/32 on the same nine legs,
so a delivery is one measurement read twice rather than two harnesses. Anything
that touches the converter again should keep that shape.

**`f40_probe.cpp` needs `-std=c++20`** — `vecmath.hpp` uses `requires` clauses.
`f34_probe.cpp`'s recorded command line omits the flag and no longer builds on
this toolchain.

**The convention hazard this task created, for every harness that follows.** A
free-flight `moveto lat/lon` now lands where the anchored one lands. Before F40
the free-mode sub-observer longitude was `180 - lon`; it is now the anchored
`lon - 90` (the `-90` is §5.49's own longitude origin, unchanged). **30 harness
files** combine `camera action free_mode state on` with a `moveto … lon`; only
those whose gates couple the observer's longitude to authored content actually
move, and two of those are corrected here, each with its arithmetic at the site:

- `b24_screen.py` — new `OBS_LON_CAM = (OBS_LON + 150) % 360`, because
  `180 - 60 == 210 - 90 == 120°`. Reproduces the pre-fix scene to within a pixel
  (close rover 3751 px vs 3750, shadow 94 893 vs 94 888, far 44 748 vs 44 746).
- `b24_select.py` — `nadir_lon()` returns `OBS_LON - 90.0` instead of
  `180.0 - OBS_LON`. Its whole scene is authored relative to that one function,
  so the geometry is preserved exactly; its FOUR remaining failures are older
  than F40 (verified: pre binary + pre-convention harness gives the same four,
  same numbers).

The rest are unaudited. A committed baseline measured in free flight at a
longitude describes a different place than it did.

**The A–D battery is not phase-locked.** Two legs of the SAME binary can differ
on `oldView.stars.faderFinal` and the whole star channel when one is caught
mid-ramp (`camera.plans.viewT != 0` in the dump says so, and `jd` differs by
~1 s). Run a same-binary control before reading any cross-binary difference:
F40 measured pre/post = 5 non-numeric differences, post/post2 = the same 5,
pre/post2 = 0.

**`camera.position` is dumped non-zero while anchored, and it is dead state** —
`switchToBody`'s internal `setFreeMode(true) … setFreeMode(old)` round trip
leaves the converter's output there at startup. It is expected to differ across
the F40 boundary (by exactly the transform F40 made) and `f40_cmp.py` reports it
instead of gating it.
