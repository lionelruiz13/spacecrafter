# B42 - the wiring class, before it is built: what the code says today

For the owner, to rule on. Nothing is built, nothing in `src/` was touched. Code `main` @ `47b348bf`.
Written 2026-09-20 by a dispatched Claude Fable 5.1 agent (task F126); derivation and raw lists: `claude/INTENT/11.262.md`.

Your ruling, verbatim (2026-09-19): *"FOV - like everything about the space, only Projector is responsible for fov when
running the old path, only Camera is responsible for fov when running the new path, if needed you can build a wiring
class to gate to the right class depending on the mode, which could be removed (or replaced by Camera) when the old path
finally get dropped."* and *"Old render path unchanged by construction means everything in the old path is gated by the
old path, the gating is another responsibility to be hold by another class."* and *"About the sky on the new path, the
Projector/Navigator/Observer are old path, the Camera is the new path."*

Read here as the TARGET. What follows is TODAY, from the code. The three facts that decide the design:

1. **Today the sky is drawn through Projector / Navigator in BOTH modes** (stars, nebulae, grids, lines, constellations,
   meteors, cardinal points, text: `SolarSystemModule::draw`, solarSystemModule.cpp:188-220, ungated). What keeps it
   right on the new path is exactly the "inform both" the ruling removes. So a request cannot be gated before the last
   shared reader of that information is answered by the Camera - otherwise the sky freezes on the new path.
2. **The Camera answers 13 of the 65 query methods the shared code uses today** (107 of 387 calls). 39 methods / 204
   calls have NO answer on the new path; the largest hole about the space itself is the frame matrices (10 methods, 58 of
   those calls), through which eight of the informations below reach the sky.
3. **At a mode switch nothing crosses** (`ssystem_factory.cpp:659-660` is a bare flip). Agreement at the switch is, today,
   a by-product of informing both all the time.

How to check any row: `python3 claude/harness/shape/trio_census.py -v` (totals); one method's sites:
`/usr/bin/grep -rnE '\b(prj|proj|projection|projector|nav|navigation|navigator|observatory|observer|obs)\s*(->|\.)\s*getFov\s*\(' src --include='*.[ch]pp' | /usr/bin/grep -v experimentalModule`
(the census strips `//` comments first, grep does not). File:line are given for every request site.

## 1. The mode: who decides, who reads

Held by `SSystemFactory::drawModularSystem` (your name, `da858612`) + `pathPinned`, both PUBLIC members
(ssystem_factory.hpp:786,788). `pathPinned`, `getExperimentalPath`, `setExperimentalPath`, `setRenderPathMode` are agents' names.

- **Written at 5 sites**: the two initialisers; `setRenderPathMode` (hpp:92-93, from `Core::setRenderPathMode`, core.cpp:894,
  from `beta_features.ini`, app.cpp:591); `setExperimentalPath` (hpp:96-97, from `flag experimental_path`,
  app_command_interface.cpp:1191); the alternation flip in `SSystemFactory::update` (cpp:659-660).
- **Read at 24 sites in 8 files** - this is the wiring class's present, scattered form:

| file | reads | what each gate selects |
|---|---|---|
| executorModule/solarSystemModule.cpp | 6 | sky brightness source (161), milky way / environment backdrop (183), old Oort (190), meteor gate (216), atmosphere + landscape / environment sky (225), atmosphere colour input (272) |
| executorModule/stellarSystemModule.cpp | 5 | the same, minus Oort (167, 192, 220, 227, 275) |
| bodyModule/ssystem_factory.cpp | 4 | old bodies or `drawExperimental` (632, 643); the flip reads `pathPinned` (659); the mode PASSED INTO the module as `driveEngines` (670) |
| coreModule/coreLink.hpp | 3 | readers of the space: `drawnPlace` (785), `getViewOffset` (986), `getHeading` (1004) |
| coreModule/core.cpp | 3 | planet-grid sync (661), `getFlagLockSkyPosition` (877), home-body pick gate (1152) |
| coreModule/core.hpp | 1 | `needOldSelectionPointer` (335) |
| coreModule/coreLink.cpp | 1 | instrument: `dumpControlSurface` prints the drawn path (572) |
| interfaceModule/app_command_interface.cpp | 1 | the flag's own status (719) |

  Of the 24: 4 gate a reader of the space, 16 gate what is drawn, 1 hands the mode to the module, 1 is the flip, 2 report it.
  The executors read the public member directly (11 sites), never the accessor.
- **Two gates that do not read the flag**: `Atmosphere::computeColor` branches on `in.prj` being null (atmosphere.cpp:237;
  set by the old overload at :154, nulled by `EnvironmentManager` at EnvironmentManager.cpp:153);
  `EnvironmentManager::update` branches on its `driveEngines` parameter (EnvironmentManager.cpp:130). Inside the module the
  flag itself is read nowhere (`grep -rnE 'drawModularSystem|pathPinned|getExperimentalPath' src/experimentalModule` = 0).
- In-galaxy, in-universe and sandbox executors read the mode nowhere: `drawExperimental()` gates itself (cpp:643).

## 2. The request table: who writes the space today

One row = one information. `old / new` = file:line of the two writes inside one function. **[yours]** = the Camera line
is blamed on you (`da858612` 2025-09-20, or `8ce65acc`); unmarked = an agent's. cL = `coreModule/coreLink.hpp`,
c = `coreModule/core.cpp`, fH / fC = `bodyModule/ssystem_factory.hpp` / `.cpp`.

| information | old responsible | new responsible | functions that inform BOTH today (old / new) | asked of the old path only | asked of the new path only |
|---|---|---|---|---|---|
| fov | `Projector` `setFov` `zoomTo` `changeFov` | `Camera::setHalfFov` -> `ModularBody::halfFov` | `CoreLink::zoomTo` (cL:312 / 313 **[yours]**); `CoreLink::setFov` (cL:328 / 329 **[yours]**); `Core::zoomToBothPaths` (c:1252 / 1254, 8 callers in `autoZoomIn` / `autoZoomOut`); `Core::updateMove` (c:1883 / 1885 - the Camera is fed the projector's landed value); `CoreBackup::loadBackup` (backup_mgr.cpp:51 / 53) | `CoreLink::setMaxFov` (cL:334): `Camera::maxHalfFov` is private, no setter | - |
| view direction | `Navigator` `moveTo` `setLocalVision` `updateMove` `lookAt` | `Camera::lookTo` x2, `lookRel` | `CoreLink::lookAt` (cL:378 / 376 **[yours]**); `CoreLink::setLocalVision` (cL:1016 / 1017 **[yours]**); `Core::dragView` (c:1807 / 1808 **[yours]**); `Core::updateMove` (c:1892 / 1894); `Core::autoZoomOut` (c:1307 / 1312 and 1344 / 1352) | `Core::gotoSelectedObject` (core.hpp:241); `Core::panView` (core.hpp:247); `Core::lookAnchor` (c:2630; the new side only arms the offset, 2633); the re-aim inside `Core::setViewOffset` (c:2545); `Core::restoreSkyVision` (c:838-840) | - |
| tracking | `Navigator::setFlagTraking` (+ `moveTo` on the object) | `Camera::trackBody` | `Core::setFlagTracking` (c:2492,2496,2497 / 2494,2498 **[yours]**); `Core::autoZoomIn` (c:1265-1266 / 1269); `Core::dragView` (c:1810 / 1809 **[yours]**) | - | `SSystemFactory::reloadCurrentSystem` (fC:776 **[yours]**) |
| heading | `Navigator` `setHeading` `changeHeading` | `Camera::setHeading` `moveHeading` | `CoreLink::setHeading` (cL:993 / 995); `CoreLink::moveHeadingRelative` (cL:940 / 941 **[yours]**) | `CoreLink::setDefaultHeading` (cL:999), `Core::init` (c:522): the Camera has no default heading | - |
| mount | `Navigator::setViewingMode` | `Camera::setMount` (`toggleMount` is called by nobody) | `Core::setMountMode` (c:884 / 886); `Core::toggleMountMode` (core.hpp:233) ASKS `getMountMode`, then tells | - | - |
| sky lock | `Navigator::setFlagLockEquPos` | `Camera::setSkyLock` | `Core::setFlagLockSkyPosition` (c:870 / 872) | - | - |
| view offset + armed latch | `Navigator::setViewOffset` `setViewOffsetTransition`; the ARMING is a side effect of an auto-move: `moveTo`'s `zooming` argument decides it (navigator.cpp:373), `updateVisionVector` ramps it (navigator.cpp:77-80) | `Camera::setViewOffset` `armViewOffset` `restoreViewOffsetLatch` | `Core::setViewOffset` (c:2542 / 2548); `Core::restoreViewOffset` (c:826 / 828); and the arming restated beside each old `moveTo`: `armViewOffset` at c:1268, 1311, 1351, 2501, 2633, cL:377 (6 sites, 5 functions) | - | - |
| place (longitude, latitude, altitude) | `Observer` `setLatitude/Longitude/Altitude` `moveTo` `moveRel*` `multAltitude` | `Camera` - the same names, `multAlt` | `CoreLink::observatorySetLatitude` (cL:836 / 837), `...Longitude` (841 / 842), `...Altitude` (849 / 850), `observerMoveTo` (889 / 890), `observerMoveRelLon/Lat/Alt` (895 / 896, 900 / 901, 905 / 906) - all seven **[yours]**; `Core::updateMove` (c:1877 / 1879) | the executors' altitude hand-off between their modes, `observer->setAltitude`: inGalaxyModule.cpp:59,63, inSandBoxModule.cpp:59,63, inUniverseModule.cpp:55,138, solarSystemModule.cpp:74; `CoreLink::observerSetConf` (cL:910) | `CoreLink::cameraDescend` (cL:885) |
| reference body (home planet) | `AnchorManager::switchToAnchor` -> `Observer::setAnchorPoint` | `Camera` `relocateToBody` `switchToBody` `warpToBody` `rebindReference` | `SSystemFactory::switchToAnchor` x2 (fH:722 / 724, 729 / 730, through `syncCameraReference`, fC:864), reached from `Core::setHomePlanet` (c:2177,2179) and solarSystemModule.cpp:78; `SSystemFactory::cameraSwitchToAnchor` (fH:649 / 650,652); `SSystemFactory::changeSystem` (fC:259,263 / 260,267 **[yours]**) | - | `SSystemFactory::reloadCurrentSystem` (fC:766) |
| free flight, bound to surface | none: the old path has no such state | `Camera::setFreeMode` `setBoundToSurface` | - | - | `Core::init` (c:411, config key `attached`); `CoreLink::cameraSetFreeMode` (cL:880) |
| eye-relative mode, quaternion mode | `Observer::setEyeRelativeMode` `setQuaternionMode` | none | - | `CoreLink::setEyeRelativeMode` (cL:926), `Core::init` (c:397), `CoreLink::setQuaternionMode` (cL:918) | - |
| anchors (the set, the current one, travels, follow-rotation) | `AnchorManager` | `CameraAnchors`, driving the Camera | 10 functions of `SSystemFactory`: `cameraAddAnchor` (fH:637 / 638), `cameraRemoveAnchor` (643 / 644), `cameraMoveToPoint` x2 (657 / 659, 664 / 666), `cameraMoveToBody` (671 / 673), `cameraTransitionToPoint` (682 / 684), `cameraTransitionToBody` (689 / 690), `cameraSetFollowRotation` (695 / 696), `anchorManagerInit` (711 / 712), `updateAnchorManager` (717 / 718) | `cameraMoveRelativeXYZ` (678), `cameraSetRotationMultiplierCondition` (701), `cameraAlignWithBody` (705), `cameraSave` (735), `loadCameraPosition` (739), `cameraDisplayAnchor` (626) | - |
| the frame step | each executor's `update`: `Observer::update`, `Navigator::update` `updateTransformMatrices` `updateVisionVector`, `Projector::updateAutoZoom` `setModelViewMatrices` | `Camera::update` through `SSystemFactory::updateExperimental` | `Executor::update` (executor.cpp:60 / 61): both engines are advanced every frame, in every mode | `Core::applyClippingPlanes` (c:653,655: clipping planes and viewport, every executor's `draw`) | - |

**The start-up is a pair that is not one function**: `Core::init` writes the old engine (c:396 place, 404 home planet, 405
tracking, 432 view, 508-510 mount, 518 fov, 521 heading) and `SSystemFactory::loadCamera` the new one (fC:132-148, **[yours]**
except the mount and the view lines) - from the same config keys (`[init_location]`, `viewing_mode`, `heading`, `init_fov`, `init_view_pos`).

**Session restore adds no writer of its own**: `SessionHost` (coreLink.cpp:1254-1306) forwards to the functions above
(`core.setHomePlanet`, `link.observerMoveTo`, `link.zoomTo`, `core.setFlagLockSkyPosition`, `core.restoreViewOffset`,
`core.setFlagTracking`); its one old-only entry is `setSkyVision` -> `Core::restoreSkyVision`; the Camera restores itself
inside the module (`Camera::restoreSession`).

**Counts** (from `request_entries.tsv`, one line per entry; the python beside it in 11.262 counts them):
42 entries inform both, in **39 functions** - cL 13, fH 13, c 10, fC 1, backup_mgr.cpp 1, executor.cpp 1; the Camera line is
yours in 16 of the 42, an agent's in 26. Asked of the old path only: **29 call lines in 22 functions**. Of the new path only: 5 call lines in 4.
Camera write calls outside the module: 54 lines (fC 16, c 21, cL 16, backup_mgr 1; yours 26, agents 28), 27 distinct
members; read calls: 33 lines, 16 members; plus 11 `cameraAnchors->` lines in fH.

**Managers per information (S7), today -> after.** Today every row above has TWO holders, one per engine, both informed
in both modes, plus the seam functions that must keep them equal. After: one informed per mode; the wiring class holds the mode and nothing else.

| information | old holder: direct writers | new holder: direct writers | seam functions informing both | after |
|---|---|---|---|---|
| fov | `Projector::fov`: `setFov` (+ `zoom_move`: `zoomTo`, `updateAutoZoom`) | `ModularBody::halfFov`, written through `ModularBody::setHalfFov` by `Camera::setHalfFov` and `Camera::update` - NOT a Camera field, `writers.py` cannot see it (non-member call) | 5 | 1 per mode |
| view direction | `local_vision`: `restoreVision` `setLocalVision` `updateMove` `updateVisionVector` | `alt` / `az`: `advanceView` `lookRel` `lookTo` `recoverParams` `update` | 5 | 1 per mode |
| heading | `heading`: `changeHeading` `setDefaultHeading` `setHeading` `update` | `heading`: `advanceView` `moveHeading` `recoverParams` `setHeading` | 2 | 1 per mode |
| mount / sky lock / tracking | `viewing_mode` 1 / `flag_lock_equ_pos` 1 / `flag_traking` 1 | `mount` 1 / `skyLocked` 1 (+ `lockedSkyRot` 4) / `target` 2 | 1 / 1 / 3 | 1 per mode |
| view offset | `view_offset` 1, `view_offset_transition` 2 | `viewOffset` 1, `viewOffsetArmed` 2, `viewOffsetTransition` 2 | 2 (+ 6 arming sites) | 1 per mode |
| place | `longitude` 5, `latitude` 4, `altitude` 4 | `longitude` 7, `latitude` 6, `distance` 6, `position` 7 | 8 | 1 per mode |
| reference body | `Observer::anchor`: `setAnchorPoint` | `reference`: `rebindReference` `switchToBody` `warpToBody` | 4 | 1 per mode |

(`python3 claude/harness/shape/writers.py HEAD <Class> <hpp> <cpp>`; regex, no compiler - blind to a write through a
non-member call, an alias or another class. `AnchorManager` / `CameraAnchors` were not run.)

Seen while reading, not acted on: `CoreLink::setFov` sets the projector at once and the Camera over the default 0.5 s
(cL:329 passes no duration; `Projector::setFov`, projector.cpp:92, has none). `CoreLink::setLocalVision` hands the
Navigator's local vector to `Camera::lookTo` as is (cL:1017), while the three `Core` sites convert it with
`Camera::oldLocalToLocal` first (c:1312, 1352; fC:148); its one caller is the OLD path's own `Observer::setEyeRelativeMode`
(observer.cpp:439, yours, 2022) - an old-path class driving the Camera through the seam. Neither was run (no launch in this task).

## 3. The query table: who reads Projector / Navigator / Observer outside the module

`trio_census.py` at HEAD: **668 calls, 124 method names** (Projector 39 / 242, Navigator 46 / 226, Observer 39 / 200). All 124 are
classified below (appendix). 3 of the names and 11 of the calls are not methods of these classes: `observatory.get()` x9 and
`observatory.reset()` x1 on the `std::shared_ptr`, and `obs.transfo4d` on a `Vec3d` (meteor.cpp:151). The census matches
receiver NAMES by regex: it is blind to a call through another name. A probe for the declared method names behind any other
receiver (generic names such as `update`, `moveTo`, `load` left out) found 12 more calls, all inside `navigator.cpp`
(`position->...`, the Navigator reading the Observer): old path proper.

**By kind** (methods / calls): query 77 / 524 - request 34 / 94 - frame step 7 / 36 - instrument (`dumpTrace`) 3 / 3 - not a method 3 / 11.

**By caller, classified PER FILE** (72 files):

| caller | files | calls | queries | what it is |
|---|---|---|---|---|
| old path proper - keeps its direct call | 21 | 141 | 99 | the `Body` family and its decorations (body*.cpp, halo, axis, trail, tail, ring, hints, orbit_2d/3d, atm_ext, planet_grid, body_trace), `solarsystem_display.cpp`, `protosystem.cpp`, and `navModule/anchor_manager.cpp` (60). All of `src/bodyModule` EXCEPT `ssystem_factory.cpp` |
| shared file, but this entry is reached only under the old gate today | 4 | 4 | 4 | `landscape.cpp:271` and `fog.cpp:133` (`landscape->draw(prj, nav)`, called only in the else-branch, solarSystemModule.cpp:232, stellarSystemModule.cpp:234; `Fog::draw(prj, nav)` has no caller found); `atmosphere.cpp:238` (behind `in.prj`); `oort.cpp:147` (behind solarSystemModule.cpp:190) |
| the seam: `core.cpp` `core.hpp` `coreLink.cpp` `coreLink.hpp` `ssystem_factory.cpp` `backup_mgr.cpp` `app.cpp` | 7 | 156 | 92 | where the requests land and the four gated readers live |
| - of which comparators and dumps reading BOTH engines by design | (3) | 37 | 34 | `Core::recordSeamStep` 19, `CoreLink::dumpControlSurface` 8, `Core::dumpOldViewState` 3, `rampVisionAzAlt` 3, the ramp record in `Core::updateMove` 2, `SSystemFactory::dumpTracePaths` 2 |
| the executors | 6 | 96 | 62 | the frame step, the draw order, 11 of the mode's 24 readers |
| the sky, tools, media, in-galaxy code | 34 | 234 | 233 | `skyline.cpp` 43, `skyDisplay.cpp` 24, `cardinals.cpp` 24, `image.cpp` 11, `meteor.cpp` 10, ... - 207 of the 234 calls are on a parameter or local named `prj` / `nav` / `observer`, handed down by the executor, not on `core->projection` / `navigation` / `observatory` |
| **total** | **72** | **668** | **524** | the 11 non-method calls sit in the seam (8), the executors (2), the sky (1) |

So the wiring class's query callers are **387 calls to 65 methods** (seam 92 + executors 62 + sky 233), and the Camera
answers them today as follows:

| answered today by | methods | calls |
|---|---|---|
| a public `Camera` member | 12 | 69 |
| another member of the module, the Camera being its writer (`ModularBody::halfFov` for `getFov`) | 1 | 38 |
| partly: the public ingredients exist, no member composes them | 13 | 76 |
| NONE | 39 | 204 |

**By information** (queries only, all callers; "shared" = everything but the old path proper):

@@FAMILY_TABLE@@

## 4. The gaps: what the new path cannot answer yet, largest first

1. **Frame matrices** - 10 methods, 63 shared calls, 0 answered. `Camera::viewMat()` and `renderViewRotation()` are private;
   `viewRotation()` and `placementRotation()` are public and give equatorial -> eye and local -> eye by rotation only;
   heliocentric and J2000 -> eye need the reference's `accumulatedBodyToBodyPos`, which `Camera::update` composes as a local
   (`absMat`, Camera.cpp:444) and publishes to nobody; the two dome matrices have no counterpart at all.
2. **The screen** - 8 methods, 71 shared calls: viewport sizes, centre, radius, the 2D ortho matrix, `printGravity180`. Not an
   information about the space, and nothing of it is in the Camera.
3. **Frame conversions** - 8 methods, 48 shared calls (`earthEquToLocal` 21, `helioToEarthPosEqu` 11, `j2000ToEarthEqu` 10):
   the Camera converts FROM the eye (`observedToLocalPos`, `observedToBodyEquPos`), never between two sky frames;
   `localToBodyEqu` does one of them and is private.
4. **The projection law and its inverse** - 14 methods, 46 shared calls. The module projects in shaders; on the CPU it holds only
   the radial law, `ProjectionTransfer`. `atmosphere.cpp:237-250` already carries the inverse both ways, side by side.
5. **The reference body as the old `Body` type** - `getHomeBody` 20 calls, 18 shared: `getReferenceBody()` answers with a `ModularBody*`,
   the callers use the `Body` API (`getAtmosphereParams`, `isSatellite`, ...). `isOnBody` is composed today in
   `EnvironmentManager.cpp:68` (`!isFreeMode() && !reference->isSystem()`), not by a Camera member.
6. **`getFov`** - 41 shared calls: the current value is `ModularBody::halfFov` (as in your own `4dfe7bb3`); the Camera has a setter
   and no getter; `getAimFov`'s counterpart `getZoomDstHalfFov` is labelled "for the trace harness" and is unset until the first timed zoom.
7. **`getViewingMode`** - 4 calls: you deleted `getMount` (`toggleMount`: tell, never ask). The two askers that are not
   instruments are `Core::dragView` (c:1798, picks the frame to unproject in) and `Core::getMountMode` (core.hpp:230, for `toggleMountMode`).
8. Several Camera members the seam already leans on say "for the trace harness" in the header: `getViewParams`, `getAbsFwd`,
   `getForwardLocal`, `getPlanTimers`, `getZoomSrcHalfFov`, `getZoomDstHalfFov` - the only answers to `getLocalVision` and the motion-plan flags.
9. Requests with no responsible on the new path: the 29 old-only sites of section 2. With none on the old path: the 5 new-only ones.

## 5. The hand-over at a mode switch

- **Where**: `SSystemFactory::update` (fC:655-661): a static 1000 ms countdown, then `drawModularSystem = !drawModularSystem`
  when `!pathPinned`. `setExperimentalPath` (fH:95-98) sets and pins. Neither calls anything.
- **What crosses: nothing.** The two engines agree at a switch only because (a) the 39 functions of section 2 inform both at
  every request, (b) `Executor::update` advances both every frame in every mode, (c) start-up reads the same config twice.
- **What flows between the engines at all** - two one-way sites, old -> new, never the reverse: `Core::updateMove` feeds the
  Camera the projector's landed fov (c:1885); `cameraSwitchToAnchor` re-references the Camera when only the old side knew the
  anchor (fH:651-652). No site writes the old trio from the Camera (the 33 Camera read lines outside the module are the four
  gated readers, the comparators, and the factory's own use of `getCurrentSystem` / `getReferenceBody` / `getTrackedBody`).
- **What does not cross, so can differ at a switch today**: the 29 old-only and 5 new-only request sites; every in-flight
  plan (two laws per motion: `Navigator::moveTo` / `Camera::lookTo`, `Projector::updateAutoZoom` / `Camera::update`); the
  Camera's own change of reference in free flight (`findBetterReference` -> `switchToBody`, Camera.cpp:452-453: the old
  home planet is not told); the shared engines written by whichever path draws (atmosphere model and flag: `BodyDecor` /
  `EnvironmentManager` behind `driveEngines`; `core->sky_brightness`, solarSystemModule.cpp:161-174).
- **What this means for the ruling**: once only one engine is informed per mode, the other is stale at the flip, so the
  hand-over stops being a by-product and becomes a function somebody owns.

## 6. The shape is a choice - three forms, measured, not chosen

Inputs common to all: 65 query methods / 387 calls from the wiring class's callers; of the 65, 27 are laws or the screen
(screen 8, projection 8, frame conversion 7, inverse 4) and 38 read a value, in 16 informations; 22 are called only
from the sky files, 18 only from the seam files. 31 request methods / 59 calls, 7 frame-step methods / 29 calls. 282 function
declarations outside the module carry a `Projector` / `Navigator` / `Observer` parameter (old path proper 95, sky 160, seam 15,
executors 7, old-gated 5; regex over headers, single-line parameter lists only). The labels A / B / C are mine.

| | A - one forwarding method per trio method the shared code calls | B - one accessor per INFORMATION, the laws kept as one implementation fed with values | C - an interface with two implementations, the mode swaps one pointer |
|---|---|---|---|
| methods on the class | 65 queries (+ 31 requests if they move there too) | about 16 value accessors + the 39 request entry points already in the seam; the 27 law / screen methods stay where the law lives, taking values | 65 (+ 31) declared once, implemented twice |
| call sites whose TEXT changes | 0 in the sky if the class is handed down under the same parameter names and keeps the method names; the parameter TYPE changes in up to 187 declarations (sky 160, seam 15, executors 7, old-gated 5) | 387 (every shared query is rewritten to the accessor) minus what keeps its name | as A |
| new-path bodies to write before it works | 52 (39 NONE + 13 partly) | the 10 frame matrices + `getFov` + reference body + place: the laws then run unchanged on them | as A |
| where the mode is tested | in each of the 65 (+ 31) methods | in each accessor, about 16 (+ 39) | once, at the switch; the 24 scattered reads become one |
| lines, estimated [derived, not measured: 3 lines per forwarder, 5 per accessor] | about 200 (+ 95) + the 52 bodies | about 80 + the matrix family + the law's extraction out of `Projector` | about 100 of interface + 200 (+ 95) old adapter + the 52 bodies |
| when the old path is dropped | delete the class, 387 call sites keep compiling only if the Camera takes the same names | delete the old branch of 16 accessors | delete the adapter; "or replaced by Camera" = the Camera is the other implementation |
| not measured | - | whether the projection law can leave `Projector` without moving the old path | the cost of a virtual call inside the sky's inner loops against 1 ms / frame (D11) |

Your own measure applies to all three: *"unless it's more code than it's worth"* - A and C are 65 to 96 forwarders whose only content is the gate.

## 7. What the code does not answer - yours to rule. Each: the premise, a default, its reason

1. **Order.** Premise: fact 1 above - the sky reads the trio in both modes, so "inform both" is what draws the sky right
   on the new path today. Default: queries first, per information; a request loses its second write only when the last
   shared reader of that information is answered by the Camera; the frame matrices first, since view, heading, place, mount,
   sky lock, view offset, tracking and reference body all reach the sky through them. Reason: every commit keeps the new
   path's sky right; the other order is a flag day across 39 functions.
2. **Scope: "everything about the space".** Premise: the same two-engines form also exists for the BODIES - 24 inline functions
   of `SSystemFactory` write an old manager and a module symbol in one body (`setPlanetHidden`, `setFlagTrails`, `setFlagAxis`,
   `setFlagHints`, `setFlagPlanetsOrbits` x2, `setBodyColor`, `setDefaultBodyColor` x2, the four Moon / Sun scale setters,
   `setScale`, `setSizeLimit`, `switchPlanetTexMap`, `createTexSkin`, `setSelected`, `removeBody`, ...; regex over the header)
   - and for the environment (`BodyDecor` / `EnvironmentManager`). Default: the wiring class covers the space only: Projector
   / Navigator / Observer / `AnchorManager` against Camera / `CameraAnchors`; the body and environment seams are listed, not
   touched. Reason: your sentence names Projector and Camera; and a body flag told to one tree only is lost at a switch,
   which is question 5, not this class.
3. **The screen.** Premise: 8 methods / 71 shared calls read the viewport and the 2D matrix from `Projector`; it is not the
   space, the Camera holds none of it, and no second holder exists to gate against. Default: not gated; it stays with
   `Projector` in both modes and is listed as what needs a home the day `Projector` leaves. Reason: a gate between one
   manager and nothing is code with no effect.
4. **The projection law on the new path.** Premise: 14 methods / 46 shared calls are one law over (matrix, fov, projection
   type, viewport), implemented in `Projector`; the sky needs it on the CPU (labels, line clipping, picking) in both modes.
   Default [the form is mine]: the law becomes a function of VALUES that both `Projector` and the new side call, as
   `ProjectionTransfer` already is for its radial part. Reason: S2 - the alternative is the law written a second time
   beside the first, which is the defect you named; `atmosphere.cpp:237-250` is that defect in small, today.
5. **The hand-over.** Premise: section 5 - nothing crosses today, and with one engine informed per mode the incoming engine
   is stale. Default: at the switch, the wiring class tells the incoming engine the outgoing one's resting values through
   the request entry points that already exist for each information (section 2), snapped; plans in flight are not
   carried. Reason: it composes existing primitives (S3). It needs an answer from the outgoing engine for each
   information: on the Camera side `mount` has none, by your own deletion of `getMount`, and the current fov is
   `ModularBody::halfFov` - those two are the ones to rule on.
6. **Who holds the mode.** Premise: `drawModularSystem` is a public member of `SSystemFactory`, written at 5 sites and read
   at 24 in 8 files. Default: the mode moves into the wiring class, as its only state, with the alternation timer;
   `SSystemFactory` and the executors ask it. Reason: you ruled the gating is another class's responsibility, and the mode
   is the one information the gate needs. The NAME is yours to give: `drawModularSystem` is yours, the four accessors are not.
7. **Requests with no responsible in the running mode** (29 old-only sites, 5 new-only; section 2). Default: routed to the
   one path that has them; a no-op in the other mode, logged once with what to do instead; none is built now. Reason:
   nothing is built beside your structure without a name from you (S5), and a silent no-op hides the gap. Three have a
   one-line Camera counterpart today if you want them closed instead: `gotoSelectedObject` -> `lookTo`, `panView` ->
   `lookRel` with its duration, `lookAnchor` -> `lookTo`.
8. **The comparators.** Premise: 37 trio calls and 33 Camera read lines sit in instruments that compare the two engines
   in ONE run (`recordSeamStep`, the ramp record, `dumpControlSurface`, `dumpOldViewState`, `dumpTracePaths`); 173 of the
   373 harness scripts mention one of their keys (loose grep, upper bound, not read). With one engine informed per mode the
   other half of every such comparison is stale. Default: they stay outside the wiring class, reading each engine
   directly, and parity is measured across two pinned launches of the same script. Reason: a harness switch that
   re-informs both would keep the second manager compiled in. The cost is the harness's, and it is not measured here.
9. **`Core::toggleMountMode` and `Core::dragView` ask the mount.** Default: `toggleMountMode` tells (`toggleMount` exists
   and nobody calls it, inside the module or out); `dragView` is left for you - it needs the frame in which the drag is measured, and that is a
   design of yours, not a forwarding. Reason: your deletion of `getMount` is the ruling (S3, S5).

## 8. Against the ledger's table (`INTENT/11.254.md` (d)) - derived from the code FIRST, then compared

The ledger's 11 rows were an index over agents' readings of deleted comments. In the code and absent from it:
`CoreBackup::loadBackup` (fov); `Core::dragView` and `Core::autoZoomOut` (view), `Core::autoZoomIn` and `Core::dragView`
(tracking); `SSystemFactory::changeSystem`, `switchToAnchor` x2 and `cameraSwitchToAnchor` as the functions that inform both
for the reference body (it names their helper `syncCameraReference`, which writes the Camera only); `anchorManagerInit` and
`updateAnchorManager` (it says "nine seams", the code has ten besides `cameraSwitchToAnchor`); 5 of the 6 view-offset arming
sites; the frame step; the start-up pair; every old-only request except `cameraDescend` / `cameraSetFreeMode`; free
flight as an information of its own; eye-relative and quaternion modes; that 16 of the 42 pairs are yours. In the ledger
and not so in the code: `Core::restoreSkyVision` is listed among functions that "drive BOTH engines" - it writes the
Navigator only; the raw harvest says `syncCameraReference -> Camera::warpToBody` - since `47b348bf` it is `relocateToBody`;
row B42's "68 call sites, ~33 Camera members" - the code has 87 Camera call lines and 43 distinct members outside the module.
Confirmed as written: the four gated readers, the four fov functions, the place and heading rows, sky lock, mount.

## Appendix - every name of the census, classified (124)

Columns: calls = all; old path proper = the 21 files of section 3; shared = the rest. For a query, the last column says who
answers it on the new path today: `Camera` (a public member), `module` (another member of the module), `partly`, or `NONE`.
For a request or a frame step it names the Camera counterpart.
@@FULL_TABLE@@
