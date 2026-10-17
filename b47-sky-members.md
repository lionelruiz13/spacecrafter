# B47 - the sky as members of the body tree, before anything is built: what the code says today

For the owner, to rule on. Nothing is built, nothing in `src/` was touched. Code `main` @ `47b348bf`.
Written 2026-09-21 by a dispatched Claude Fable 5.1 agent (task F127); derivation and raw lists: `claude/INTENT/11.266.md`.

**STATUS after the round of 2026-09-21 (record `claude/INTENT/11.270.md`) - what moved since this page was written:**
- **Slice 0 LANDED for one layer** (harness, `INTENT/11.267.md`): `bash claude/harness/f128_leg.sh SOL|GAL|TILT` - the milky way alone, per level, old path pinned against new path pinned, and a gdb leg that says whose tilt it follows. Not built: the other layers, the stellar level; the universe level is reached and unjudged.
- **Slice 1 LANDED** (code `160b3e87`, `INTENT/11.268.md`): a member of a body above the current system receives its OWNER's matrix. Identical while every `re` is zero (new path bit-identical); with a tilt poked under gdb the milky way now follows the galaxy and no longer the system. Question 9 below is CLOSED by it. Two things it leaves with you: the climb is a file-local `observedMat` in `EnvironmentManager.cpp` (the task's write set stopped there) - its natural home is `ModularBody`, beside `getMat()`; and `MilkyWayEnv.cpp:18` rotates the zodiacal ecliptic normal, a quantity of the SYSTEM's flat frame, with the galaxy's matrix (with the system's before): wrong under any tilt either way - whose member is the zodiacal light?
- **Slice 2 STOPPED, measured, nothing committed** (`INTENT/11.269.md`): gated exactly as the solar module's, the galaxy-level milky way comes out TURNED BY 139.64 deg on the new path. Above a system your Camera and the old observer do not stand at the same place: 214.72 deg apart in longitude (the old observer's longitude there carries the wall clock of the app's START - `anchor_manager.cpp:835,853-855`; the Camera's is the sidereal time of the script's date) and 8.31 deg in roll. Today it is invisible because the Navigator draws the WHOLE sky at the galaxy and universe levels in both modes. So this page's claim that slice 2 *needs nothing from B42* is REFUTED, and section 8's order changes: **above a system no sky layer can move to the Camera one by one before somebody is responsible for where the observer stands there - question 10, new, below.**

Your words, verbatim (2026-09-21): *"The stars, the milkyway, the landscape, those are dependent on where we are (stars
drawn while in the solar system, when outside the 3D version is used instead), so they must have been converted into
EnvironmentModule and use the matrix depending on where they camme from to have the right orientation."* - *"the milkyway
is owned by the galaxy, because it is the galaxy's internal representation, same as landscape being the body's landed
representation"* - *"The solar system own the 2D stars, the galaxy own the 3D stars, while in universe it's tully"* -
*"the inSolarSystem, inStellarSystem, inGalaxy and inUniverse, those levels should be identical, but implicit with the
body system so that it can be extanded transparently as needed."*

Read here as the TARGET. What follows is TODAY, from the code. Five facts that decide the design:

1. **The layers fall in three classes by what they READ, and the 2D stars are in the dearest one.** (A) one matrix in -
   heliocentric -> eye, plus the observer's position and the fov (every 3D layer: 3D stars, tully, starGalaxy, dso, oort);
   (B) one rotation in (milky way, VR360, grids, the 2D star lines); (C) projected point by point on the CPU through the
   `Projector`'s LAW (2D stars, nebulae, constellations, sky lines, sky display, meteors, cardinal points, the pointer,
   and every LABEL of every class). `zone_array.cpp:275,291-292`: each 2D star goes J2000 -> local through the Navigator,
   is refracted in the local frame, then local -> screen through `prj->projectLocal`. The new path has no CPU
   projection law (page B42, section 4 item 4, question 4 - not answered yet). So the most visible slice is not the cheapest.
2. **`StarNavigator::drawRaw(const Mat4f &matrix)` is yours (`97394924`, 2022-03-31) and is already the matrix-in entry
   of the 3D stars**; `StarLines` already holds the 2D / 3D pair as two entries over one private `drawGL(Mat4f&)`
   (`starLines.cpp:352-369`); `CloudNavigator::draw(const Mat4f &mat, prj)` exists too (`cloudNavigator.hpp:49`).
3. **The member chain is DRAWN from two of the four mode modules only.** `drawEnvironmentBackdrop` / `drawEnvironmentSky`
   are called by the solar and stellar modules; the galaxy module draws the milky way through the OLD entry, ungated
   (`inGalaxyModule.cpp:128`), and the universe module calls neither. The chain is UPDATED in every mode (`executor.cpp:61`).
4. **The level is decided by the OLD observer's altitude in both modes**, and the Camera is not told when the level goes
   up: `Observer::setAltitude` -> `AltitudeEvent` (`observer.cpp:66,163`) -> `Executor::onAltitudeChange`
   (`executor.hpp:65-69`) -> `testValidAltitude`; `SSystemFactory::leaveSystem` (`ssystem_factory.cpp:621-628`) changes the
   old `currentSystem` and never the Camera's reference. The Camera leaves a body by itself only in free flight
   (`findBetterReference`, `Camera.cpp:449-454`, by area of influence). So today the chain's level and the executor's
   mode are two deciders that can disagree.
5. **Your draft declares TWO homes for what a system shows around its bodies.** The `EnvironmentModule` lists
   (`groundedEnvironment` / `environment`, `ModularBody.hpp:1120-1121`), and the system's own `inComponents` /
   `nearComponents`, which `ModularSystem::drawSystem` draws before / after the bodies with the system's `mat`
   (`ModularSystem.cpp:563-564,581-582`; the same at your `4dfe7bb3`, `ModularSystem.cpp:93-94,103-104`) - inside
   `Camera::draw`, so in every mode, with no call from a mode module. Your direction of 2026-09-12 already put `dso3d`,
   `ojmMgr` and `tully` in the tree as bodies / a `BodyModule` (`claude/b5-dso-design.md`, not re-derived here).

How to check a row: every row carries file:line. Reads of Projector / Navigator / Observer are those of
`python3 claude/harness/shape/trio_census.py -v` (668 calls; page B42), filtered per engine file; it matches receiver NAMES
(`prj proj projection projector nav navigation navigator observatory observer obs`) and is blind to any other name.

## 1. The layer table - every draw of the four mode modules, in order

Counted with `/usr/bin/grep -c 'core->[A-Za-z_0-9]*->draw' <file>`: solar 24, stellar 22, galaxy 10, universe 8 LINES. The
regex also matches the mode flag (`core->ssystemFactory->drawModularSystem`) and the new path's own calls, and it cannot
match `core->selected_object.drawPointer(` (a dot), `->preDraw(`, `->update(prj, nav, ...)`, `->computePosition(` nor
`Executor::draw`'s `imageDraw`. Classified: solar 24 = 15 layer draws + 6 mode reads (161 183 190 216 225 272) + 3 new-path
calls (184 202 226); stellar 22 = 14 + 5 (167 192 220 227 275) + 3 (193 209 228); galaxy 10 = 9 + 0 + 1 (143); universe
8 = 7 + 0 + 1 (127). None unclassified. sol = `solarSystemModule.cpp`, ste = `stellarSystemModule.cpp`, gal =
`inGalaxyModule.cpp`, uni = `inUniverseModule.cpp`, all under `src/executorModule/`.

| # | layer (member of `Core`) | engine class, file | draw signature | sol | ste | gal | uni | condition the mode module puts around it |
|---|---|---|---|---|---|---|---|---|
| 1 | `milky_way` | `MilkyWay`, `coreModule/milkyway.cpp` | `draw(eye, prj, nav, julianDay)` | 186 | 195 | 128 | - | sol, ste: only when NOT `drawModularSystem` (else `drawEnvironmentBackdrop()`, 184 / 193); gal: none |
| 2 | `media` VR360 | `VR360`, `mediaModule/vr360.cpp` | `drawVR360(prj, nav)` | 188 | 197 | 126 | 109 | none |
| 3 | `nebulas` | `NebulaMgr`, `coreModule/nebula_mgr.cpp` + `nebula.cpp` | `draw(prj, nav, eye, sky_brightness)` | 189 | 198 | - | - | brightness argument is 0 unless `atmosphere->getFlagShow()` |
| 4 | `oort` | `Oort`, `coreModule/oort.cpp` | `draw(distance, nav)` | 191 | - | - | - | not when `drawModularSystem && hasExperimentalOort()` (190); `distance` = `observer->getAltitude()` |
| 5 | `illuminates` | `IlluminateMgr`, `coreModule/illuminate_mgr.cpp` | `draw(prj, nav)` | 192 | 199 | - | - | none |
| 6 | `asterisms` | `ConstellationMgr`, `coreModule/constellation_mgr.cpp` + `constellation.cpp` | `draw(prj, nav)` | 193 | 200 | - | - | none |
| 7 | `starLines` | `StarLines`, `coreModule/starLines.cpp` | `draw(prj)` = 2D; `draw(nav)` = 3D | 194 (2D) | 201 (3D) | 135 (3D) | - | none |
| 8 | **`hip_stars`** (2D stars) | `HipStarMgr`, `starModule/hip_star_mgr.cpp` + `zone_array.cpp` | `draw(grid, eye, prj, timeMgr, altitude)`; the work is `preDraw(grid, eye, prj, nav, timeMgr, altitude, atmosphere)` | 195; preDraw 279 | preDraw 282 ONLY | - | - | `preDraw` runs on the module's worker thread; in the stellar module it runs every frame and nothing draws its result |
| 9 | **`starNav`** (3D stars) | `StarNavigator`, `inGalaxyModule/starNavigator.cpp` | `draw(nav, prj, scaling)` -> `drawRaw(matrix)`; `computePosition(posI)` | - | 202 (`true`); position 76, 119 | 140 (`false`); position 121 | - | ste: the selected star is hidden / shown around altitude 7.91706e+08 (105-117) |
| 10 | `skyGridMgr` | `SkyGridMgr`, `coreModule/skygrid_mgr.cpp` + `skygrid.cpp` | `draw(prj)` | 196 | 203 | - | - | none |
| 11 | `skyLineMgr` | `SkyLineMgr`, `coreModule/skyline_mgr.cpp` + `skyline.cpp` | `draw(prj, nav, timeMgr, observatory)` | 197 | 204 | - | - | none |
| 12 | `skyDisplayMgr` | `SkyDisplayMgr`, `coreModule/skydisplay_mgr.cpp` + `skyDisplay.cpp` | `draw(prj, nav, equPos, oldEquPos)`; `drawPerson(prj, nav)` | 198 | 205 | 134 (person) | 122 (person) | none |
| 13 | the bodies | `SSystemFactory::draw` / `drawExperimental` | - | 202 | 209 | 143 | 127 | gated inside (`ssystem_factory.cpp:630-647`); gal, uni: new path only |
| 14 | selection pointer | `Object::drawPointer`, `tools/object_base.cpp` | `drawPointer(delta_time, prj, nav)` | 207 | 212 | 131 | 125 | `core->needOldSelectionPointer()` |
| 15 | `meteors` | `MeteorMgr`, `coreModule/meteor_mgr.cpp` + `meteor.cpp` | `update(proj, nav, timeMgr, eye, delta_time)`; `draw(proj, nav)` | 210, 220 | 215, 224 | - | - | draw: (`drawModularSystem` ? `EnvironmentState.allowMeteors` : `bodyDecor->canDrawMeteor()`) && `sky_brightness < 0.1` |
| 16 | `atmosphere` | `Atmosphere`, `atmosphereModule/atmosphere.cpp` | `draw()`; `computeColor(...)` on the worker thread | 228; 273 / 275 | 230; 276 / 278 | - | - | only when NOT `drawModularSystem` (else `drawEnvironmentSky()`, 226 / 228) |
| 17 | `landscape` | `Landscape`, `coreModule/landscape.cpp` + `fog.cpp` | `draw(prj, nav)` | 232 | 234 | - | - | same else-branch, and `bodyDecor->canDrawLandscape()` |
| 18 | `cardinals_points` | `Cardinals`, `coreModule/cardinals.cpp` | `draw(prj, latitude)` | 236 | 238 | - | - | none; AFTER the landscape |
| 19 | `dso3d` | `Dso3d`, `inGalaxyModule/dso3d.cpp` | `draw(distance, prj, nav)` | - | - | 138 | - | none |
| 20 | `ojmMgr` | `OjmMgr`, `ojmModule/ojm_mgr.cpp` | `draw(prj, nav, STATE_POSITION)` | - | - | 139 `IN_GALAXY` | 121 `IN_UNIVERSE` | none |
| 21 | `dsoNav` | `DsoNavigator`, `inGalaxyModule/dsoNavigator.cpp` | `draw(nav, prj)`; `computePosition(posI, prj)` | - | - | 141; 123 | 126; 106 | none |
| 22 | `cloudNav` | `CloudNavigator`, `inGalaxyModule/cloudNavigator.cpp` | `draw(nav, prj)`; `computePosition(posI, prj)` | - | - | 142; 122 | - | none |
| 23 | **`tully`** | `Tully`, `coreModule/tully.cpp` (+ `volumObj3D.cpp` through `build(volumGalaxy)`) | `draw(distance, nav, prj)` | - | - | - | 115 / 119 | two branches of `volumGalaxy->loaded()`; the first build lowers the mode's floor to 1e8 (113) |
| 24 | **`starGalaxy`** | `StarGalaxy`, `inGalaxyModule/starGalaxy.cpp` | `draw(nav, prj)` | - | - | - | 123 | none |
| 25 | images | `ImageMgr`, `mediaModule/image.cpp` | `imageDraw(nav, prj)` | all | all | all | all | `Executor::draw`, `executor.cpp:55`, after every mode |

Per level, the mode module also sets the clipping planes (`applyClippingPlanes`: sol / ste `0.000001, 200`; gal
`0.01, 2000.01`; uni `0.0001, 10`), pushes the Navigator's seven matrices into the Projector (`setModelViewMatrices`,
sol:135, ste:145, gal:105, uni:90), and the solar and stellar modules switch the pass between the meteors and the
atmosphere (`nextDraw(PASS_FOREGROUND)`, sol:222, ste:226).

**What `starGalaxy` IS** (its header says nothing): a point cloud read from `gal3d.dat` (`core.cpp:110`), each point
pushed 200 units away from the plane, scaled by 8e-7 and placed by a transform written in the loader -
`translation(-0.002, 0.0001, -0.005) * yawPitchRoll(112, 0, 0)` (`starGalaxy.cpp:235,260-261`) - then drawn with the
heliocentric -> eye matrix and faded by `opacityAdjustment / distance / fov` (`starGalaxy.cpp:211-215`). `core.hpp:680`
calls it *"draw galaxy stars when in universe"*; it is your *"Add experimental galaxy"* (`13426b28`, 2022-02-04). So it
is the Milky Way seen from OUTSIDE, with its place and orientation in the loader instead of in a node.

## 2. What each layer reads from the old observer

N = Navigator, P = Projector, O = Observer. Read per engine FILE (a read in `searchAround` counts too). Classes: **A** one
matrix in; **B** one rotation in; **C** needs the CPU projection law; +L = its labels are class C.

| layer | class | matrix or rotation | fov, screen | place, body, flags, time, tone |
|---|---|---|---|---|
| `milky_way` | B | N.getJ2000ToEyeMat milkyway:208; zodiacal: N.getHelioToEyeMat :201, N.getObserverHelioPos :197 | - | `CoreLink::instance->getObserverHomeBody` :190; tone `adaptLuminance` :223 |
| VR360 | B | N.getJ2000ToEyeMat vr360:189 | - | - |
| `nebulas` | C | P.getMatJ2000ToEye nebula:336; N.j2000ToEarthEqu nebula:228, nebula.hpp:85; N.earthEquToLocal nebula:246; P.projectJ2000 nebula.hpp:161; N.getPrecEquVision nebula_mgr:176 | P.getFov nebula_mgr:175,182 nebula.hpp:153; P.getViewportHeight / Width nebula_mgr:175,182 nebula:359,360; P.printGravity180 nebula:418,422 | tone nebula:355 |
| `oort` | A | N.getHelioToEyeMat oort:147 | - | altitude as argument |
| `illuminates` | B (culled on the CPU) | P.getMatJ2000ToEye illuminate_mgr:217; N.getPrecEquVision :201 | P.getFov, P.getViewportHeight / Width :200 | `CoreLink::instance->getBaseMag` :148 |
| `asterisms` | C | P.getMatJ2000ToEye constellation:157; P.projectJ2000LineCheck :126,310; P.projectJ2000Check :180; N.getPrecEquVision :180 | P.getFov :159 | - |
| `starLines` | B (2D) / A (3D) | P.getMatJ2000ToEye starLines:367 / N.getHelioToEyeMat :352 | - | - |
| **`hip_stars`** | **C** | N.j2000ToEarthEqu + N.earthEquToLocal zone_array:275,364; P.projectLocal :291,380; P.projectLocalCheck :292,381; P.unprojectViewport hip_star_mgr:792 | P.getFov hip_star_mgr:633,659,794; P.printGravity180 :919 | `timeMgr->getJulian` :784; tone :681,695,740; altitude and the refraction flag as arguments |
| **`starNav`** | A +L | N.getHelioToEyeMat starNavigator:597,599; labels: N.helioToEarthPosEqu :617,619 (+ :567 in `searchAround`), P.projectEarthEqu :621 | P.printGravity180 :580; `starViewer`: P.projectCustomCheck :126, P.getClippingFov :129 | position through `computePosition(N.getObserverHelioPos)`; `CoreLink::instance->getFlagIngalaxy` starNavigator:453 |
| `skyGridMgr` | B | P.getMatEarthEquToEye skygrid:228,231; P.getMatJ2000ToEye :234; P.getMatLocalToEye :237 | P.getMatProjectionOrtho2D :249 | - |
| `skyLineMgr` | C | P.projectEarthEqu skyline:1254,1260,1396,1402; N.helioToLocal :1745; O.getRotEquatorialToVsop87 :1246,1385 | P.getMatProjectionOrtho2D x12 | O.isEarth x7, O.getHomeBody x5, O.isOnBody x3, O.isSun, O.getAltitude x3, O.getLatitude x3, O.getLongitude x2; `timeMgr->getJDay` :478; `CoreLink::instance->ssystemFactoryGetEarth / GetMoon` :1704,1713 |
| `skyDisplayMgr` | C | N.earthEquToLocal x7; P.getMatEarthEquToEye, P.getMatLocalToEye skyDisplay:226; P.getCursorPosEqu :596 | P.getMatProjectionOrtho2D x12; P.getFov :428,601 | `CoreLink::instance->skyDisplayMgrCheckDraw` :597 |
| pointer | C | P.projectEarthEqu object_base:212 | P.getViewportHeight :223,237,266; P.getViewportRadius :234 | - |
| `meteors` | C | N.helioToEarthEqu meteor_mgr:42, meteor:126; N.localToEarthEqu meteor:150; N.earthEquToLocal :294,295,314; P.projectLocal :316; P.projectLocalCheck :299,300 | P.getFov :221 | `timeMgr->getTimeSpeed` meteor_mgr:90; tone meteor:220. (`obs.transfo4d` meteor:151 is a `Vec3d`, not the Observer) |
| `atmosphere` | - | none in `draw()`; `computeColor`: P.unprojectNormalizedLocal atmosphere:238, behind `if (in.prj)` | - | tone :275 |
| `landscape`, fog | B | N.getLocalToEyeMat landscape:271, fog:133 (old entries only) | - | - |
| `cardinals_points` | C | P.projectLocal x12 cardinals:82-118 | P.printGravity180 x12 | latitude as argument |
| `dso3d` | A +L | N.getHelioToEyeMat dso3d:242; N.getObserverHelioPos :243; labels N.helioToEarthPosEqu :229, P.projectEarthEqu :232 | P.printGravity180 :234 | altitude as argument |
| `ojmMgr` | A | N.getHelioToEyeMat ojm_mgr:156 | P.getMatProjection :158 | - |
| `dsoNav` | A | N.getHelioToEyeMat dsoNavigator:329 | P.getClippingFov :331; P.getFov, P.getViewportHeight :190 | position as argument |
| `cloudNav` | A | N.getHelioToEyeMat cloudNavigator:215 | P.getClippingFov :223; P.getFov, P.getViewportHeight :154 | position as argument |
| **`tully`** | A +L | N.getHelioToEyeMat tully:503, volumObj3D:267,311,350; N.getObserverHelioPos tully:504; labels N.helioToEarthPosEqu :571 (+ :478), P.projectEarthEqu :574 | P.printGravity180 :490; P.getClippingFov volumObj3D:284,353; P.getFov :297,339 | altitude as argument |
| **`starGalaxy`** | A | N.getHelioToEyeMat starGalaxy:211 | P.getClippingFov :213; P.getFov :214 | - |
| images | B | N.getLocalToEyeMat image:688,1006; N.getDomeFixedMat :693; N.getJ2000ToEyeMat :698; N.getEarthEquToEyeMat :703 | P.getMatProjectionOrtho2D :765; P.getClippingFov :1007,1051; viewport :148-151 | - |
| every label | C | `Projector::printGravity180` -> `s_font.cpp`: P.projectDomeFixed :402 | viewport radius, centre, size :390-415 | - |

Totals per layer are in `claude/harness/artifacts/f127/layer_reads.txt` (produced by `layer_reads.py` beside it; the
directory is git-ignored). All the Projector reads of class C go through ONE door: the seven matrices the mode module
pushes each frame (`setModelViewMatrices`) plus the Projector's own fov and viewport. `Projector::projectCustom(v, win,
mat)` (`projector.hpp:258`) already takes the matrix as a VALUE; it reads the fov, the viewport and the projection type
from the Projector.

## 3. Owner, family, matrix - by your test (*whose inside view is it?*) and by the level that draws it

| layer | owner by the level that draws it | owner by your test | what orients it | what `ModularBody::mat` gives |
|---|---|---|---|---|
| `milky_way` | every level but the universe | **the galaxy [RULED]** | J2000 -> eye, rotation only | the galaxy's `mat` is NOT maintained while we stand in a system (section 5); the manager hands the system's |
| **`hip_stars`** | the Solar system only | **the Solar system [RULED]** | J2000 -> local (the refraction is in the ZENITH frame of where we stand), then local -> eye | J2000 -> eye = rotation of the system's `mat` x `mat_j2000_to_vsop87` (what `MilkyWayEnv.cpp:9-21` composes). The zenith frame is NOT a body matrix: it is the observer's place on the reference body - `Camera::viewRotation()`, which is why `LandscapeEnv` asks the Camera (`LandscapeEnv.cpp:24`) |
| **`starNav`** | stellar system (`scaling` = true), galaxy (false) | **the galaxy [RULED]** | heliocentric -> eye x a constant rotation (`xrotation(-pi/2 - 23.4392803...deg)`), x `scaling(1e-6)` in a stellar system; the observer's position in the catalogue's unit | needs the galaxy's OWN `mat` WITH its translation: the substitution of section 5 is wrong for it |
| **`tully`** | the universe | **the universe [RULED, *"probably"*]** - against *"maybe as a BodyModule of the milkyway"* (2026-09-12): question 4 | heliocentric -> eye + observer position | the root's `mat`, not maintained below the root (section 5) |
| `starGalaxy` | the universe | the galaxy BODY seen from OUTSIDE (section 1) - not an inside view | heliocentric -> eye; place and orientation hard-coded in the loader | the MilkyWay node's `mat` would carry what the loader hard-codes |
| `nebulas`, `illuminates`, `asterisms`, `starLines` 2D | solar and stellar system | the system (sky-sphere catalogues in J2000, as the 2D stars) - NOT ruled | J2000 -> eye | as `hip_stars` |
| `oort` | the Solar system | a BODY of the Solar system (the pilot, `ssystem_factory.cpp:486`; B5 design) | heliocentric -> eye | the oort body's `mat` (done, opt-in) |
| `skyGridMgr`, `skyLineMgr`, `skyDisplayMgr` | solar and stellar system | one manager, several frames: equatorial of date = the reference BODY's equator; alt-azimuthal, meridian, zenith = the body's landed view; J2000, galactic, ecliptic = the system's - NOT ruled | P.getMatEarthEquToEye / LocalToEye / J2000ToEye | equatorial of date -> eye = the reference body's `mat` (`ModularBody.cpp:244`: flat frame x accumulated tilt) |
| `meteors`, `cardinals_points`, `atmosphere`, `landscape` | solar and stellar system | the reference BODY, landed (atmosphere and landscape are there already) | local -> eye | not a body matrix (see `hip_stars`) |
| `dso3d`, `dsoNav`, `cloudNav`, `starLines` 3D, `ojmMgr` `IN_GALAXY` | the galaxy (`dsoNav` the universe too) | the galaxy - as BODIES by your 2026-09-12 direction for `dso3d` and `ojmMgr`; `dsoNav`, `cloudNav` in no design | heliocentric -> eye | B5 design, section 3 |
| VR360, images, pointer, person marker | every level (person: galaxy and universe) | no body: they belong to the SCREEN or to the show, not to where we are - NOT ruled | J2000 / local / dome -> eye | - |

**Families** [derived from the draw lists; the word is INTENT 11.265(o)'s, not yours]: STARS = {`hip_stars` in the Solar
system; `starNav` scaled in another system; `starNav` in the galaxy; `tully` (+ `starGalaxy`) in the universe}. STAR LINES
= {`starLines` 2D; 3D}. MILKY WAY = {the 2D band inside; `starGalaxy` / `volumGalaxy` outside}. One family also gates the
SELECTION and the star hide / show routing, not only the draw: `core.cpp:1148,1170,1176,1182,1189`,
`coreLink.cpp:765,770`, `app.cpp:1014`. A rule that serves the draw only leaves a second decider there.

**What the reference-to-root walk already gives** [read: `EnvironmentManager.cpp:47-66, 199-229`]: membership (an owner
draws because it is in the chain - so in the universe the galaxy's members are silent with no rule at all); the order
across owners (backdrop root-first: the galaxy's milky way before the system's stars, as in the legacy); the order inside
one owner (the order of `addEnvironment`); grounded over InAoI (your line, `ModularBody.hpp:125`); enter / leave edges.
**What it lacks**: (1) supersession inside a family - in the Solar system the galaxy's 3D stars ARE in the chain;
(2) the `scaling` variant - the same member drawn differently one level down; (3) a third draw position - the legacy
draws the meteors BEFORE the pass switch and the cardinal points AFTER the landscape, the interface has backdrop and sky;
(4) an ancestor's own matrix (section 5); (5) being called at all at the galaxy and universe levels (fact 3).
[derived, mine] `update` already walks reference -> root and fills one `EnvironmentState` (`EnvironmentManager.cpp:69-87`):
the nearest owner runs FIRST, so a family can be claimed there and seen by the ancestors with no new walk.

## 4. The precedent - how the three members were path-neutralized (`500effcd`, 2026-07-16, Fable 5) and the oort (`c0181da9`)

| engine | what was ADDED to the old engine | one law or two | the reason, recovered from `git show 500effcd` (the prose left the code on 2026-09-19) |
|---|---|---|---|
| `MilkyWay` | `drawEnv(eye, j2000ToEye, julianDay)`; later `ZodiacalInput` + a second `drawEnv` overload (`3ce1e49b`, 07-17) | ONE: the old `draw` reads the Navigator three times and forwards (`milkyway.cpp:187-209`) | *"j2000ToEye ... old path: nav->getJ2000ToEyeMat(); new path: system chain rotation * mat_j2000_to_vsop87. The texture alignment matrices stay internal - single authority."* |
| `Landscape`, `Fog` | `drawEnv(const Mat4f &localToEye)` on both (virtual on `Landscape`; `LandscapeSpherical` overrides IT, no longer `draw`) | ONE: `draw(prj, nav)` forwards `nav->getLocalToEyeMat()` (`landscape.cpp:266-269`, `fog.cpp:124-127`) | *"the mesh lives in the OLD local frame (x=South, y=East, z=Up) ... MV = viewRotation() * Z(-pi/2)"*; the landing animation moved to `drawEnv` *"so both paths get it"* |
| `Atmosphere` | `struct AtmosphereComputeInput` + `computeColor(in, eye)`; `draw()` untouched (it reads nothing) | ONE sky model, the old signature forwards (`atmosphere.cpp:140-156`) - and TWO inverse projections side by side inside it, chosen by `in.prj` being null (`atmosphere.cpp:237-251`) | *"The skylight models consume only angles between (view direction, sun, moon, zenith), so any right-handed local frame with z = zenith is valid"*; the grid direction had no new-path source, so the inverse fisheye was written in place |
| `Oort` (other FORM) | nothing: a new `OortModule` (a `BodyModule`) on an ORBITING body under "Solar"; the old draw is gated off when it exists (`solarSystemModule.cpp:190`) | TWO engines, one sample law (INTENT 11.96) | B5 pilot: position-bearing content is a body, not a backdrop |

Around them, added by the same commit: `EnvironmentManager` (229 + 66 lines today), three members (22+24, 26+18, 26+19
lines), `BodyEnvironmentParams` / `EnvironmentState`, seven forwarders on `SSystemFactory` (`ssystem_factory.hpp:106-115`),
and the gates in the two mode modules (6 + 5 of the 24 mode reads of page B42). The manager's own reason for the matrix
it hands, verbatim from that commit: *"for owners above the current system (isolated roots, whose mat dispatchUpdate does
not maintain), the manager substitutes the CURRENT SYSTEM's mat - correct for orientation by the flat-chain contract (all
root-aligned frames share one orientation; only translations differ, irrelevant for backdrop spheres)."* And what it left
open, verbatim: *"the milkyway 2D/3D regime switch (inGalaxy/inUniverse executors keep the old drawing until then - this
wave covers the solar/stellar executors)."*

## 5. What exists already - every name read first (S4), then per need

Read in full: `EnvironmentModule.hpp` (5 virtuals, 2 structs), `EnvironmentManager.hpp` (13 function names: 10 public, constructor, destructor, 1 private), `ModularSystem.hpp`
(27 public, 7 private), `ModularBody.hpp` (1266 lines), `Camera.hpp` (299 lines), the three `*Env` pairs, and your
`4dfe7bb3` for the same names. The name lists are in the entry, item (d).

| need | state | where |
|---|---|---|
| a member that enters, leaves, updates, draws behind / in front of the bodies | EXISTS | `EnvironmentModule`: `enter` `leave` `update` `drawBackdrop` `drawSky`. At `4dfe7bb3` the class is empty and `enterEnvironment` / `leaveEnvironment` are empty hooks the Camera calls on the REFERENCE only (`Camera.cpp:26,308-310,321-323` today); today they are still empty and the manager's chain diff does the edges |
| the walk reference -> root | EXISTS | `EnvironmentManager::update` (`.cpp:48`), `ModularBody::getParent` |
| attach a member to its owner | EXISTS | `ModularBody::addEnvironment(module, grounded)` (`ModularBody.hpp:961`); 3 call sites |
| content a system draws around its bodies, with its own `mat`, in every mode | EXISTS, yours | `inComponents` / `nearComponents` in `ModularSystem::drawSystem` (fact 5) |
| the owner's own matrix for an owner ABOVE the current system | a related space to extend minimally | `dispatchUpdate` stops at the system by your rule (`isNotIsolated`: *"update and draw are not transmitted to the parent body"*, `ModularBody.hpp:1210`; `ModularBody.cpp:278`). The cached, const hops exist for use outside the update walk: `transformBodyToParent(Mat4f&) const` (`.hpp:458`), `accumulatedBodyPosToBody()` with no argument (`.hpp:407`), `getMatLocalToBodyPos()` (`.hpp:586`) |
| J2000 -> eye from a `mat` | EXISTS once | `MilkyWayEnv.cpp:9-21` (a second user would copy 5 lines: one function then) |
| zenith -> eye | EXISTS | `Camera::viewRotation()` (public) |
| the observer's position in an owner's frame | EXISTS | `Camera::positionRelativeTo(body)`, `getRootPosition()` (AU) |
| fov, time | EXISTS | `ModularBody::halfFov`, `ModularBody::currentJD`, `EnvironmentManager::julianDay` |
| the level we are at | EXISTS as facts, no name | `Camera::getReferenceBody()`, `getCurrentSystem()`, `ModularBody::isSystem()`, `BodyType::GALAXY` / `SYSTEM` (private `bodyType`: `isStar()` and `isMinorBody()` read it, nothing reads GALAXY), `findBetterReference` + `areaOfInfluence` (`updateReach`, `ModularBody.cpp:401-416`) |
| supersession inside a family | TO BUILD (or to carry in `EnvironmentState`, section 3) | nothing |
| a third draw position (before the pass switch; after the landscape) | TO BUILD, or order of `addEnvironment` + moving the pass switch | nothing |
| the CPU projection law on the new path (class C, every label) | TO BUILD - it is page B42's question 4 | `ProjectionTransfer` holds the radial law only; `Projector::projectCustom(v, win, mat)` takes the matrix as a value |
| catalogue unit -> AU for the 3D layers | NOT IN THE TREE | B5 design 1.6: *"No constant converting either catalogue to AU exists anywhere in the tree"*; the legacy carries it as per-mode clipping planes, `scaling(1e-6)`, and the altitude rewrites of section 6. A data value: measured or fetched, never recalled |

**The limit at `500effcd` and its precondition.** `aboveSystem ? system->getMat() : b->getMat()` (2 sites,
`EnvironmentManager.cpp:206,218`). The premise says `ModularSystem.hpp` has no orientation member
(`/usr/bin/grep -c -i 'orientation\|tilt\|rotation' ModularSystem.hpp` = 0). True of the header - and the regex is
blind to what the class INHERITS: a `ModularSystem` is a `ModularBody`, so it has `re` (`RotationElements`), and
`dispatchUpdate` already applies the system's own tilt to its `mat` (`ModularBody.cpp:281,296`:
`parentTilted = flat x parent->accumulatedBodyPosToBody(jd)`). What keeps the substitution right today is three
LITERALS: the only creation sites of a system pass `.re = {}` (`ssystem_factory.cpp:85,99,302`; `createChildSystem` has
two callers, :108 and :311, plus the root at :94). Giving one of them a tilt is a one-line change there, none in the class
or in the manager - and the milky way would turn with that system, against your ruling, silently. And for a 3D layer the substitution is wrong TODAY in
translation (*"irrelevant for backdrop spheres"* does not hold for the 3D stars).

## 6. The mode machinery - who decides the level today

| what | where |
|---|---|
| the four modes and their links | `Executor::Executor`, `executor.cpp:35-48`: solar and stellar go UP to galaxy; galaxy goes DOWN to solar, or to stellar when the selected anchor is not "Sun" (`inGalaxyModule.cpp:156`), UP to universe; universe goes DOWN to galaxy. `InSandBoxModule` and `InPauseModule` are compiled and never instantiated |
| thresholds (the OLD observer's altitude, the unit changing with the mode) | solar up 1.E16 (`solarSystemModule.cpp:54`); stellar up 1.E10 (`stellarSystemModule.cpp:56`); galaxy down 1.E10, up 1.E14 (`inGalaxyModule.cpp:48-49`); universe down 1.E9, 1.e8 once the volumetric galaxy is built, up 1.E15 as a clamp (`inUniverseModule.cpp:45-46,113,137-138`) |
| the altitude rewritten at a switch | solar `onEnter`: x 1.E6 (`:73-74`); galaxy `onEnter`: below the floor -> the CEILING, above -> the FLOOR (`:57-63`); universe `onEnter`: the floor (`:55`) |
| the trigger | `Observer::setAltitude` and the observer's move (`observer.cpp:66,163`) -> `CoreHandler.cpp:33` -> `Executor::onAltitudeChange` -> `updateMode`; by command `App::switchMode` (`app.cpp:176,621,1012-1014`; `app_command_interface.cpp:3498`) |
| what a switch does besides the draw list | clears the selection (`executor.cpp:71,98`); `enterSystem` / `leaveSystem`; zodiacal on / off (`solarSystemModule.cpp:76,90`); `dsoNav->drop()`; screen fader events; `starNav->computePosition(center)`; starts / joins the worker thread |
| readers of the mode outside the executor (`Core::currentModule`, `core.hpp:530-535,689`) | selection per family `core.cpp:1148,1170,1176,1182,1189`; `coreLink.hpp:243,875`; `coreLink.cpp:765,770` through `isDrawingHipStarMgr` (`app.cpp:1014`); `backup_mgr.cpp:57,69`; `starNavigator.cpp:453`; the altitude readout's unit, 4 cases (`ui_tuiconf.cpp:78-89`); `ui->draw(module)` (`app.cpp:864`) |

Commands: `/usr/bin/grep -rnE 'updateMode\(|switchMode\(|onAltitudeChange|setFlagIngalaxy|getFlagIngalaxy|getExecutorModule|MODULE::|flagIngalaxy|executor->' src`
= 43 lines; it cannot match a bare enum constant, so a second pass on
`\b(SOLAR_SYSTEM|IN_GALAXY|IN_UNIVERSE|STELLAR_SYSTEM|IN_SANDBOX)\b|currentModule|isDrawingHipStarMgr` found 22 more lines
(8 of them `OjmMgr::STATE_POSITION`, another enum with the same words).

**The facts the body system already holds that answer the same question**: the Camera's reference and its chain to the
root; `isSystem()`; the node types `SYSTEM` / `GALAXY`; the spine `Universe > MilkyWay > <name>System`
(`ssystem_factory.cpp:94,108,311`); `areaOfInfluence` and `findBetterReference` (free flight only). Level = the first
isolated node of the chain and its type: a system named "Solar" / another system / the galaxy / the root.
**The minimum** [derived, proposed]: the level is READ from the chain, never stored; the two things that then need a
ruling are WHO moves the reference when we are not in free flight (today nobody: fact 4) and whether it moves at the
legacy's thresholds or at the area of influence - the two laws differ, and *identical* is measured against the first.

## 7. The identity oracle per layer

H = `claude/harness/`. Swept by a read-only sub-agent [agent: `claude` / `fable`]; the rows marked (*) were re-checked by me
against the script. No gate was RUN in this task: every threshold is as written in the script.

| layer | gate that reaches it on BOTH pinned paths | or the nearest miss |
|---|---|---|
| 2D stars (`hip_stars`) | (*) `H/b17_screen.py`: `flag equatorial_grid on`, `flag stars on`, `flag star_twinkle off` (:77), pins :52 / :54, metric px32 = pixels with a channel differing by more than 32 (:39). It asserts that the cross-path difference does not GROW under a view offset (`px32(gnp, gop) < gbase + 500`, :85); `gbase` itself is unbounded. Stars and grid are not separated | `H/f45_probe.py:131-147` a stars-only scene, no pin; `H/f56_starfield.py` compares such frames offline, no threshold |
| star labels | not reached | every hit is `flag star_names off`; (*) all 2922 files under `~/.spacecrafter/sky_cultures` are 0 bytes on this host (`find . -type f -size +0 \| wc -l` = 0) |
| 3D stars, stellar system | not reached | no gate enters the stellar mode |
| 3D stars, `dsoNav`, `cloudNav`, `dso3d`, `ojmMgr`, milky way - galaxy level | captured together, never isolated: (*) `H/b20d_anchored_draw.py:165` `noaim_inert` asserts whole-frame near-identity new / old (`cross1 <= 8 and maxdiff1 <= 8`) with default layers | `H/b5_drawhalf.py:204-210` asserts only that something is drawn |
| `tully`, `starGalaxy`, `volumGalaxy` | captured in the universe quads of `b20d` (:203) and `b5_drawhalf` (:214-222), no background parity asserted | 0 harness hits for `starGalaxy` / `volumGalaxy` |
| milky way in a system (`MilkyWayEnv`) | not reached | `H/s33_symptoms.py:16-21` saves `night_new.png` / `night_old.png` under both pins; no script reads them |
| zodiacal light, nebulae, illuminates, constellations, star lines, sky lines, sky display, person, meteors, cardinal points, fog, VR360 | not reached | constellations cannot be reached with the installed data (0-byte cultures); (*) every `meteors zhr` line of the harness is `zhr 0` (44 by my grep, 41 by the agent's), so the path-split meteor gate has no gate; illuminates and sky display are isolated on ONE unpinned path (`H/f53_guards.py:555-641`) |
| atmosphere, landscape | recorded, not asserted: `H/b5_drawhalf.py:146-148` 'earth' quad, both pins | `H/f40_env.py:97-98` a 2x2 table, no pin, no assert |
| **`oort`** | **REACHED**: (*) `H/b5_oort.py:77-85,101,126-129` - per pinned path, shot with `flag oort on` and with `off`; px32 between the two is that path's layer ALONE; then `\|NEW-OLD\| / OLD < 0.15`, hidden < 800, shown > 8000 | - |
| selection pointer | not reached on the old pin | `H/b39_hidden.py:352-367` isolates it by on / off on the NEW path only |
| images | one unpinned path, log criteria (`H/f90_rehearsal.py`) | - |

Gates that leave the Solar level: by altitude `H/b5_drawhalf.py:204,214-215`, `H/b20d_anchored_draw.py:156,202-203`,
`H/b22_live.py:70-74`; by command `H/f32_object_leak.py:355,358` (no pixels). None enters a stellar system.

**The cheapest instrument that would reach a layer** [derived]: the oort's pattern, per level - under each pin, one
shot with the layer ON and one with it OFF; px32 between them isolates the layer on that path; then the two isolated
images are compared ACROSS the pins, by count and by position. Reusable as they are: `H/b3_ladder.py` `wait_port` :529,
`send` :539, `shot` :547 (importable), the runner `H/f121_run.sh` (fresh launch, md5 in == out), the offline comparator
`H/f56_starfield.py`. There is NO shared pixel module: 25 scripts define their own `px32`. What pixels cannot see:
"the paths agree" against "the pin was swallowed" - the witness exists (`control.drawnPath` in the dump header,
`coreLink.cpp:572`) and no pixel gate reads it; the new path is not bit-stable (hence px > 32); twinkle alone moves about
2200 px; thresholds are per scene and per layer and must be MEASURED, never pasted (SHAPE.md 7b). For the levels above
the system the first thing the oracle needs is the REFERENCE at the right level on the new pin (fact 4).

## 8. The slices - proposed, not chosen

Lines are [derived] from the precedent's sizes (a member pair 45 lines, an engine split +6 to +10, a gate +3), not measured.
"Observer today -> after" is the S7 count: who positions the layer in NEW mode.

| # | slice | files | + / - (est.) | observer today -> after | deletable the day the old path goes | gate that shows it identical |
|---|---|---|---|---|---|---|
| 0 | per level, old pinned against new pinned, one image per layer switched on alone (harness only) | `claude/harness/` | 0 in `src/` | - | - | section 7 |
| 1 | an ancestor's member receives its OWNER's matrix (walk up from the system with the cached hops of section 5); the two `aboveSystem` ternaries go | `EnvironmentManager.cpp` | +8 / -6 | milky way: the SYSTEM's matrix -> the galaxy's | - | identical by construction while every `re` is `{}`: the existing landed-parity scenes; + one scene with a tilted test system to show the difference exists |
| 2 | the chain is drawn at the galaxy level too: the milky way of `inGalaxyModule.cpp:128` gated as in the solar module | `inGalaxyModule.cpp` | +3 / 0 | milky way in the galaxy: Navigator -> Camera | the old branch | galaxy level, milky way alone |
| 3 | the 3D stars as the galaxy's member over YOUR `drawRaw`, without labels; silent when a nearer owner claims the stars | 1 member pair, `ssystem_factory.cpp` (+1), 2 mode modules (gates) | +55 / 0 | Navigator -> Camera | `StarNavigator::draw`'s matrix lines, 2 draw sites | galaxy and stellar level, stars alone. BLOCKED on a number: the catalogue unit -> AU (section 5, last row) and on slice 1 (translation) |
| 4 | **the 2D stars as the Solar system's member** - the candidate. `HipStarMgr::preDraw` / `draw` split in the precedent's form; `ZoneArray::draw` takes the J2000 -> local rotation and the law as VALUES | `hip_star_mgr.{hpp,cpp}`, `zone_array.{hpp,cpp}`, 1 member pair, `solarSystemModule.cpp`, `ModularSystem.cpp` or `ssystem_factory.cpp` (+1) | +80 / -15, the law NOT counted | Projector + Navigator -> Camera | the old `preDraw` / `draw` forwarders; `solarSystemModule.cpp:195,279`; `stellarSystemModule.cpp:282` | Solar level, stars alone, labels off, then on. **THE CODE DISAGREES that it is the smallest**: it needs the CPU projection law, the fov, the viewport and `unprojectViewport` on the new path = page B42 question 4. It is the most VISIBLE one |
| 5 | the other J2000 catalogues of the system (nebulae, illuminates, constellations, 2D star lines), one per commit | each engine + 1 member pair | +55 / -10 each | as 4 | as 4 | as 4, per layer |
| 6 | the landed layers on the reference body (meteors, cardinal points; alt-azimuthal grid and lines) | each engine + 1 member pair | +55 / -10 each | as 4 | as 4 | landed scene, per layer |
| 7 | `dso3d`, `ojmMgr`, `tully` as tree content | - | B5 design, slices 1-5 (not re-derived) | - | the four draw sites | B5 design |
| 8 | the level read from the chain; the mode modules lose their draw lists | `executorModule/*` | - 871 lines of `.cpp`, - 221 of `.hpp` at the END (`wc -l`), not before every layer above is identical | two deciders -> one | the executor | every level |

**The other order, measured the same way** [derived]: if page B42's first step lands first (in new mode the seven
matrices of `setModelViewMatrices` and the Navigator's conversions are fed from the Camera, fov from
`ModularBody::halfFov`), every class B and C layer follows the Camera with NO engine edited - the engines stay what
they are, law machines - and B47 then only changes WHO CALLS them and WITH WHOSE matrix. Slices 4-6 shrink to the member
and the call move. What that order costs: the Projector and the Navigator stay in a new-mode frame (as laws, not as
observers) until the law has another home. INTENT 11.265(m)6 said it: whichever comes first decides how much of the other
is built.

## 9. Against INTENT 11.265(o)2 - derived from the code first, then compared

IN THE CODE, NOT IN ITS TABLE:
1. The person marker (`skyDisplayMgr->drawPerson`) is drawn at the GALAXY level too (`inGalaxyModule.cpp:134`); the table has it in the universe only.
2. VR360 at all four levels (sol:188, ste:197, gal:126, uni:109) and the selection pointer at all four (207, 212, 131, 125): in no row (INTENT 11.265(l)3 names them for the solar module).
3. `media->imageDraw` after every mode (`executor.cpp:55`): in no row.
4. The stellar module runs `hip_stars->preDraw` every frame (`:282`) and never draws the result; the table says *"NO hip_stars"*.
5. `starLines` is two entries: 2D in the Solar system, 3D in a stellar system and in the galaxy; the table names it once per row without the overload.
6. `volumGalaxy` is drawn INSIDE `tully` (`tully.cpp:506-524,554-555`), and the first build lowers the universe's floor from 1e9 to 1e8.
7. *"Every mode then ends with drawExperimental() / ssystemFactory->draw"*: true of galaxy and universe; in solar and stellar the bodies are drawn at :202 / :209 and followed by the pointer, the meteors, the atmosphere, the landscape and the cardinal points.
8. The hide / show of the selected star in a stellar system (`stellarSystemModule.cpp:105-117`) - the *"starNav Sol vs proxy dot"* of row B5, seen in code.
IN ITS TABLE, NOT SO IN THE CODE: nothing found. CONFIRMED: the star rows of all four levels, *"minus oort"*, the galaxy's and the universe's lists, `scaling` = true / false.

## 10. What the code does not answer - yours to rule. Each: the premise, a default, its reason

1. **The order against B42, for class C.** Premise: fact 1 - the 2D stars, and ten other layers, need the CPU projection
   law, which is page B42's question 4. Default: B42's first step (the frame matrices and the fov answered by the Camera)
   BEFORE slice 4; slices 0, 1, 2 need nothing from B42 and can go first. Reason: the law is then not written a second
   time inside a star engine (S2), and what the tester saw sliding is fixed by the step that is already first on B42's page.
2. **Which home: `EnvironmentModule` member, or the system's `inComponents` / `nearComponents`?** Premise: fact 5 - both
   are yours, the second is drawn inside `Camera::draw` in every mode with the system's `mat` and needs no manager; your
   words of 2026-09-21 say `EnvironmentModule`, your direction of 2026-09-12 says body / `BodyModule` for `dso3d`,
   `ojmMgr`, `tully`. Default: what is seen from INSIDE and has no position to fly past (2D stars, milky way band,
   nebulae, constellations, grids, landed layers) = `EnvironmentModule`; what has a POSITION (3D stars, dso, tully,
   starGalaxy, oort) = tree content, as the B5 design argues. Reason: your own test plus the oort precedent. It moves
   the 3D stars out of your sentence *"the 3D version"* = `EnvironmentModule`: if that sentence is the ruling, say so.
3. **Supersession.** Premise: section 3 - in the Solar system the galaxy's stars are in the chain; the legacy also uses
   the level for SELECTION and hide / show. Default [the form is mine]: the nearest owner claims the family in
   `EnvironmentState` during `update` (reference first, already the order), ancestors read the claim in `draw`, and
   `Core`'s five selection gates read the same claim instead of `currentModule`. Reason: one decision, one holder (S2),
   no new walk.
4. **`tully`: the universe's, or a `BodyModule` of the MilkyWay node?** Premise: *"maybe as a BodyModule of the milkyway"*
   (2026-09-12, the B5 design is built on it) against *"in the universe it's probably tully"* (2026-09-21); both hedged.
   Default: the universe owns it, carried as ONE `BodyModule` of the ROOT node (the B5 design's carrier, one node up).
   Reason: the legacy draws it at the universe level only, and a module of the MilkyWay node would draw whenever the
   galaxy is visible.
5. **`starGalaxy`.** Premise: section 1 - it is the galaxy seen from outside, its place hard-coded in the loader.
   Default: a `BodyModule` of the MilkyWay node (its appearance from outside), the loader's transform becoming the node's.
   Reason: by your distinction it is a body's appearance, not an environment; not built before slice 3 proves the unit.
6. **Who moves the reference when we are not in free flight, and at which law.** Premise: fact 4 and section 6.
   Default: for IDENTITY, the legacy's thresholds move the reference too (the `leaveSystem` / `enterSystem` seam tells
   the Camera), and the area-of-influence law replaces them only after every level is identical. Reason: Q-84, identical
   first; two laws compared at once cannot be told apart.
7. **The layers with no body** (VR360, images, pointer, person marker). Default: they stay where they are and are listed
   as what needs a home the day the executor goes. Reason: they do not depend on where we are.
8. **The sky managers that mix frames** (`skyGridMgr`, `skyLineMgr`, `skyDisplayMgr`: equatorial of date, alt-azimuthal,
   J2000, galactic in one class). Default: ONE member on the reference body, handed both frames, split later only if a
   level needs one without the other. Reason: the legacy draws them together and only in a system; splitting first is
   code nobody asked for.
9. **[CLOSED 2026-09-21 by slice 1, code `160b3e87`]** **The tilt precondition.** Premise: section 5 - the class already applies a system's tilt; only three `.re = {}` literals keep it at zero. Default: slice 1 first (the
   member receives its owner's matrix), so the precondition disappears instead of being written down. Reason: a barrier
   that is a fact about data files fails silently.
10. **[NEW 2026-09-21, from slice 2's measured STOP] Above a system, WHO PLACES THE OBSERVER?** Premise: at the galaxy
   level the Camera's rotation and the Navigator's stand 139.64 deg apart (214.72 deg of longitude + 8.31 deg of roll;
   0.03 deg at the Solar level). Each got there by its own law from the same two commands: the Camera by
   `findBetterReference` -> `switchToBody`, the old observer by the altitude event and `AnchorManager::selectAnchor`,
   whose longitude above a system depends on WHEN the program was started (two launches 107 s apart differ visibly).
   So *identical to the legacy* is, at that level, identical to a wall-clock-dependent picture. Default: none proposed -
   this is page B42's subject (one responsible per mode) reached from the sky side. What the code allows, for you to
   choose between: (a) in new mode the galaxy and universe modules read the Camera for their frame matrices (B42's first
   step) - every layer there follows the Camera at once and slice 2 is then three lines; (b) the legacy's start-clock
   term is ruled a defect of the old observer and fixed THERE first, after which the two longitudes agree to about
   0.003 deg on the measured scene [a hypothesis from one scene, not established] and the roll of 8.31 deg remains to be
   explained; (c) the levels above the system stay wholly on the Navigator until the old path goes. Related: question 6.
