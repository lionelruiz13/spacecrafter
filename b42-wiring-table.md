# B42 - the wiring class, before it is built: what the code says today

For the owner, to rule on. Nothing is built, nothing in `src/` was touched. Code `main` @ `47b348bf`.
Written 2026-09-20 by a dispatched Claude Fable 5.1 agent (task F126); derivation and raw lists: `claude/INTENT/11.262.md`.

**STATUS after your ruling of 2026-09-27 (section 15 of `DECISIONS_PENDING.md`; record `claude/INTENT/11.294.md`, F132) - the ruling read against this page's nine questions, verified line by line against the code at `20ad3ef5`:**

Your words: *"Both engines must be informed of every events and must handle them on their own - the only shared surface is the input surface, never the internals - two fully undependants lines, whose representation shall temporarily converge for pixel comparison to prove they are the same and could then diverge in precision (new path must be as precise or more precise than old path, as observable). There could be two interfaces which coexist, gated on the mode (old vs new) for the use, and thus two implementations, to keep each path internally clean, otherwise it would prove nothing."*

- **The premise of this page is RETIRED by your words**: a request does NOT lose its second write when the last shared reader is answered - the 42 pairs of section 2 are the input surface and stay. What the request side removes is INTERNALS LEAKS (one engine's state written into the other: 7 found, 5 of them a direct old-to-new write - `core.cpp:1885`, `ssystem_factory.hpp:652, 724`, `ssystem_factory.cpp:260, 267`), READOUTS of one engine feeding a command sent to both (9 sites; 5 are the FOV's own: `CoreLink::getFov` behind `zoom delta_fov`, `saveBackup`, `autoZoomIn` / `autoZoomOut`, `updateMove`'s order magnitude, the wheel's aim - benign) and GAPS (21 request lines reaching one engine; 4 of them old-only CONCEPTS whose own handling is none by design: eye-relative / quaternion mode, the level's altitude unit, the clipping planes, free flight). No write from the new engine into the old exists. The census, with file:line and the predicted fix per row: `INTENT/11.294.md` (c).
- **Question 1 (order): ANSWERED by your words - DISSOLVED.** No flag day: the READERS move, the fov first (the tester's row 5). Slice 1 is predicted in `INTENT/11.294.md` (d): one gated reader `Core::drawnFov()` beside the four that exist, consumed by the camera UBO (`Core::uboCamUpdate`) and by the five fov readouts; 4 files, +22 / -12, no class. Your `drawModularSystem` is read where it is; nothing new holds the mode.
- **Question 2 (scope): still yours; default stands (the space only).** The body seams of the same form are at `ssystem_factory.cpp:680-686` (the new tree cleared only if the old one removed something) and `ssystem_factory.hpp:244-247` (`body action initial`, old tree only) - listed, not touched.
- **Question 3 (the screen): still yours; default stands (`Projector` keeps it in both modes).** ONE SUB-QUESTION from the code: the sky's SHADER-side projection law is already ONE implementation fed with values through the camera UBO (`custom_project_advanced.glsl:147-150` dispatches on `main_clipping_fov`; 20 shader files read it). Does your *"two implementations"* reach it, or is that law the screen's, both engines feeding it values? Slice 1 stands on the second reading (H2 below).
- **Question 4 (the projection law on the new path): ANSWERED against this page's default** - *"two implementations ... otherwise it would prove nothing"* excludes one CPU law fed by both; the new path's CPU law is its own (`ProjectionTransfer` is the seed). Its COST stays yours - and the code sharpens it: after slice 1 the star field and nine other layers (constellations, sky lines, sky display, nebula hints, meteors, cardinal points, the pointer, images, every label) still project on the CPU with the Projector's fov, so **the tester's "stars move late" is closed by THIS slice, not by slice 1** (H3).
- **Question 5 (the hand-over): ANSWERED by your words - DISSOLVED** (both engines always informed). What can differ is in-flight plans (two laws) - bounded - and ONE class that is not bounded: after a key step during a timed zoom the two engines end at different REST values (5.166), because `Projector::changeFov` lets the plan overwrite the step next frame (`projector.cpp:186-189, 658-680`) while `Camera::setHalfFov(x, 0)` abandons the plan (`Camera.cpp:690-699`). Removing the leak at `core.cpp:1885` removes the 13-deg copy, not this gap; converging the two step-during-plan laws is yours (H6).
- **Question 6 (who holds the mode, its name): still yours.** Slice 1 adds no state; the gated reader's name (`drawnFov`, in the vocabulary of the agents' `drawnPlace`) is flagged for you (H4); the class that folds the (then five) gated readers comes with the frame matrices, slice 2.
- **Question 7 (requests with no responsible): ANSWERED against this page's default** - gaps are filled per engine, never a logged no-op; per site, an old-only CONCEPT may have its own handling = none (the four above; H9). One existing validate-and-continue to name: `syncCameraReference`'s WARNING when the new tree lacks the body (`ssystem_factory.cpp:866`).
- **Question 8 (the comparators): ANSWERED as your (iii)** - both engines stay informed, so the ring and the dumps stay valid; the PROOF is a bounded pixel comparison of the two PINS, never bit-identity. CONSEQUENCE for the tester page: `f125_fovlaw.py`'s `RESID` lines stay GREEN after slice 1 and forever (they compare the ring's two columns with the two laws); the sign that slice 1 landed is a CAPTURE with a body, a grid and stars mid-zoom (`INTENT/11.294.md` (d7)).
- **Question 9 (`toggleMountMode` / `dragView` ask the mount): still yours.** `Core::getMountMode` reads the navigator in both modes (`core.hpp:229-231`, 5.170).
- **b47 question 6**: in SHAPE answered (the Camera decides its own level; `enterSystem -> changeSystem -> switchToBody`, `ssystem_factory.cpp:613-619, 256-270`, is the leak); in LAW yours. **b47 question 10**: above a system the galaxy-level UBO consumers (`starNav`, `dso3d`, `tully`, the old milky-way draw) take the gate's FOV in slice 1 while their ORIENTATION stays the Navigator's (`inGalaxyModule.cpp:128`, ungated): two informations, two slices. **Tester Q-A** answered here; **Q-B** shape yes / law yours; **Q-C, Q-D** yours; **Q-E** can wait.

**Slice 1's cost to the audience, stated plainly** (`INTENT/11.294.md` (d8)): mid-zoom in new mode the seam moves from "bodies against the sky" to "the CPU-projected sky against everything else" - the landscape, the milky way, the grids, the star lines and the nebula textures join the bodies; the star field, the constellations and every label stay behind. Better for a body-and-horizon zoom, worse for a grid-or-band zoom (the stars detach from their own milky way), unchanged for the tester's own sentence. Q4's slice is what closes it.

**What stays yours after this ruling** - each: the premise, a default, its reason (section 17 of `DECISIONS_PENDING.md` is the one list; these add to it, never answer it):
- **H1 - Question 2, scope.** Premise: two body seams of the two-engine form (above). Default: the space only; the body seams listed. Reason: your sentence names Projector and Camera.
- **H2 - Question 3, and the GPU projection law.** Premise: `custom_project*.glsl` is one law fed with values by whichever engine writes the UBO; the CPU law (`Projector::projectCustom` vs `ProjectionTransfer`) is two. Default: the shader law is the screen's (path-neutral by construction); "two implementations" means the two engines' laws. Reason: the shaders are not an engine's internals; a second shader copy would prove nothing more.
- **H3 - Question 4, cost.** Premise: after slice 1 ten layers, the star field first, stay on the Projector's CPU law; the tester's row 5 stays open. Default: measure the D11 cost of 46 gated law calls in the structure task of that slice (unchanged). Reason: the cost is not measured; the visible symptom is.
- **H4 - Question 6, the names.** Premise: slice 1 needs one gated reader in `Core`; the existing four are agents' names (`drawnPlace`, `getViewOffset`, `getHeading`, `getFlagLockSkyPosition`). Default: `Core::drawnFov()` now, folded into your wiring class at slice 2 under the name you give. Reason: one method with no state is less code than a class; the class is worth it at ten methods.
- **H5 - Question 9.** Unchanged: `toggleMountMode` tells; `dragView` waits for the frame it is measured in.
- **H6 - NEW: a snap during a zoom plan.** Premise: `Projector::setFov` / `changeFov` do not cancel an auto-zoom (the plan rewrites the fov next frame); `Camera::setHalfFov(x, 0)` ends or re-targets its plan; the two engines end apart at rest after a key step during a timed zoom (5.166) even with the copy at `core.cpp:1885` gone. Default: the old law is the oracle for the proof phase - a snap does not cancel a plan; the Camera's `setHalfFov(x, 0)` during a plan keeps the plan. Reason: *"temporarily converge ... to prove they are the same"*; it is a law inside your `Camera`, so yours.
- **H7 - NEW: the fov limits.** Premise: `CoreLink::setMaxFov` reaches the Projector only; the Camera clamps at its statics `minHalfFov` 8.7e-7 / `maxHalfFov` 3.05 rad (349.5 deg) where the field's `max_fov` reads 350. Default: `loadCamera` takes the same config keys `Core::init` reads. Reason: one input, both engines; the 0.49-deg stop residual goes.
- **H8 - NEW: a command's verdict when the two registries disagree.** Premise: seven anchor commands return `oldOk || newOk` (`ssystem_factory.hpp:653-697`) - success if EITHER engine accepted - and one (`cameraSwitchToAnchor`) repairs the new engine from the old when only the old accepted; 5.182's mechanism. Default: both must accept, else the refusal is reported (S11); the repair line goes. Reason: an asymmetric acceptance is a leak by another door.
- **H9 - per site: old-only CONCEPTS whose own handling is none by design.** Premise: eye-relative mode and quaternion mode (`coreLink.hpp:917-932`), the executors' altitude rescale at a level change (`inGalaxyModule.cpp:59,63` and siblings), the clipping planes (`core.cpp:653,655`), and new-only free flight (`coreLink.hpp:879-881`). Default: none, each. Reason: no counterpart concept exists in the other engine; a built one would be a second manager of nothing.
- **H10 - NEW: the readout-fed `moveto`.** Premise: a partial `moveto` (a delta, or one component) starts from the DRAWN engine's readout and writes the absolute result into both (5.174, 5.184). Default: the command carries the ORDER - per-component setters and relative movers, which exist (`coreLink.hpp:836-850, 894-906`) - and no readout. Reason: the input surface must not carry one engine's state into the other; the full `moveto lat lon alt` is unchanged.
- **H11 - the wheel's aim.** Premise: `CoreLink::getAimFov` is the Projector's; both engines hold the same aim by construction (same commands), up to the limits of H7. Default: kept on the Projector in slice 1. Reason: no owner-named aim getter exists on the Camera (`getZoomDstHalfFov` is an agent's, for the trace); the residual is H7's.
- **H3' - Question 4's COST, MEASURED BY READING (F139, `INTENT/11.301.md`), and the shape under it.** Premise: the star field's loop visits every star of every zone its search cone returns, drawn or not, and at fov >= 90 the Projector's cone is the WHOLE SPHERE (`projector.cpp:158-161`): on this host's catalogue that is 26561 stars per frame (the log: levels 0 + 1, `max_geodesic_level 1`), each with 2 Navigator conversions, 1 law call and, when drawn, 1 `getFov()` (`hip_star_mgr.cpp:633`) - about 8e4 calls per frame on the executor's WORKER thread (`solarSystemModule.cpp:279`), not the 1e4 assumed at the mint; the nine other CPU layers add about 2e3 here (about 2e4 in the field with constellation lines on). Cost of a gated call, per shape: (a) an abstract `Projector*` / `Navigator*` surface = 3 indirect calls per visited star = about +160 us per frame, a sixth of D11's 1 ms, at wide fov; (b)/(c) the engine fed values through a template policy = 0 added and about -120 us (the two conversions composed once on the new entry, the per-star `RangeMap` hoisted); the eight frame-thread layers under (a) < 50 us - `skyline` and `skyDisplay` already dispatch the law through a `Projector` member-function pointer (`proj_func`), so (a) adds nothing there. Default: (c) for the star field - the module's own law object over `ProjectionTransfer::radius` fed `ModularBody::halfFov` / `projectionMode` / `cullHalfFov` / `getViewportRadius()` and the centre from the screen rect as `Renderer::printGravity` already takes it, handed to the engine as a value, the OLD entry keeping its two Navigator products and its per-star fov read so the old pin stays bit-identical by construction; then (a) for the eight frame-thread layers over the same object (90 declarations). Reason: the cost sits in ONE loop that already runs off the frame thread, where a virtual is a sixth of the budget and a template policy is free. Two facts for your Q3 / H2 from the same read: the SCREEN already has a second holder in your module (`ModularBody::viewportRadius` from the Vulkan screen rect; `Renderer::printGravity`'s centre / radius, *"equals the old Projector disk-viewport center/radius by construction"*), so this page's Q3 premise *"no second holder exists"* does not hold and the new CPU law reads no Projector value at all; and `ProjectionTransfer.hpp:6` declares itself the shader's twin (*"must equal shaders/src/custom_project.glsl"*) - your module already treats the GPU law as the reference both CPU laws copy. The one-step slice's audience state, plainly: the stars, the band, the grids and the bodies move as one fov; the constellation LINES, the equator / ecliptic LINES, the nebula HINTS and the POINTER stay on the projector's for 2-4 frames per click - lines off the stars they join is a new side issue one step aside, in the overlay a planetarium uses most; the second step (the eight, shape (a)) is what removes it. Both steps are predicted in `INTENT/11.301.md`.

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

| information | methods | calls | of which shared | Camera / module / partly / NONE | the methods |
|---|---|---|---|---|---|
| screen | 8 | 76 | 71 | 0 / 0 / 0 / 8 | `getMatProjectionOrtho2D`, `printGravity180`, `getViewportHeight`, `getViewportWidth`, `getViewportRadius`, `getViewportCenter`, `getViewport`, `getViewportFloatCenter` |
| frame matrix | 10 | 74 | 63 | 0 / 0 / 4 / 6 | `getHelioToEyeMat`, `getLocalToEyeMat`, `getJ2000ToEyeMat`, `getDomeFixedMat`, `getEarthEquToEyeMat`, `geTdomeMat`, `getEarthEquToEyeMatFixed`, `getMatJ2000ToEye`, `getMatEarthEquToEye`, `getMatLocalToEye` |
| place | 8 | 88 | 59 | 4 / 0 / 3 / 1 | `getObserverHelioPos`, `getAltitude`, `getLatitude`, `getHeliocentricPosition`, `getLongitude`, `getObserverCenterPoint`, `getLongitudeForDisplay`, `getDistanceFromCenter` |
| frame conversion | 8 | 54 | 48 | 0 / 0 / 2 / 6 | `earthEquToLocal`, `helioToEarthPosEqu`, `j2000ToEarthEqu`, `helioToLocal`, `earthEquToJ2000`, `helioToEarthEqu`, `earthPosEquToHelio`, `localToEarthEqu` |
| reference body | 5 | 53 | 43 | 3 / 0 / 2 / 0 | `getHomeBody`, `isOnBody`, `isEarth`, `getHomePlanetEnglishName`, `isSun` |
| fov | 2 | 57 | 43 | 0 / 1 / 1 / 0 | `getFov`, `getAimFov` |
| projection law | 9 | 37 | 35 | 0 / 0 / 0 / 9 | `projectLocal`, `projectEarthEqu`, `projectLocalCheck`, `projectCustom`, `projectJ2000LineCheck`, `projectCustomCheck`, `projectDomeFixed`, `projectJ2000`, `projectJ2000Check` |
| inverse law | 5 | 11 | 11 | 0 / 0 / 4 / 1 | `unprojectEarthEqu`, `unprojectLocal`, `getCursorPosEqu`, `unprojectNormalizedLocal`, `unprojectViewport` |
| view direction | 3 | 10 | 9 | 1 / 0 / 1 / 1 | `getLocalVision`, `getPrecEquVision`, `getEquVision` |
| clipping + fov | 1 | 23 | 9 | 0 / 0 / 0 / 1 | `getClippingFov` |
| tracking | 1 | 6 | 6 | 1 / 0 / 0 / 0 | `getFlagTraking` |
| heading | 1 | 6 | 6 | 1 / 0 / 0 / 0 | `getHeading` |
| motion plan | 3 | 6 | 6 | 0 / 0 / 2 / 1 | `getFlagAutoMove`, `getFlagChangeHeading`, `getMoveCoef` |
| mount | 1 | 4 | 4 | 0 / 0 / 0 / 1 | `getViewingMode` |
| place default | 3 | 3 | 3 | 0 / 0 / 0 / 3 | `getDefaultAltitude`, `getDefaultLatitude`, `getDefaultLongitude` |
| sky lock | 1 | 2 | 2 | 1 / 0 / 0 / 0 | `getFlagLockEquPos` |
| view offset | 1 | 2 | 2 | 1 / 0 / 0 / 0 | `getViewOffset` |
| frame rotation | 2 | 5 | 2 | 0 / 0 / 1 / 1 | `getRotEquatorialToVsop87`, `getRotLocalToEquatorial` |
| eye-relative mode | 1 | 2 | 1 | 0 / 0 / 0 / 1 | `getEyeRelativeMode` |
| quaternion mode | 1 | 1 | 1 | 0 / 0 / 0 / 1 | `getQuaternionMode` |
| projection matrix | 1 | 1 | 1 | 0 / 0 / 0 / 1 | `getMatProjection` |
| anchor | 1 | 2 | 0 | 0 / 0 / 0 / 1 | `getAnchorPoint` |
| clipping | 1 | 1 | 0 | 0 / 0 / 0 / 1 | `getClippingPlanes` |
| **total** | **77** | **524** | **425** | **12 / 1 / 20 / 44** | |

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

**Projector** - 39 names, 242 calls

| method | kind | calls | old path proper | shared | information | answered today by / Camera counterpart |
|---|---|---|---|---|---|---|
| `getFov` | query | 55 | 14 | 41 | fov | module: ModularBody::halfFov (static, rad, half angle; Camera::setHalfFov / update are its writers; no Camera getter) |
| `getMatProjectionOrtho2D` | query | 27 | 0 | 27 | screen | NONE: viewport-to-clip matrix; nothing in Camera holds the screen |
| `getClippingFov` | query | 23 | 14 | 9 | clipping + fov | NONE: zNear, zFar, fov for GLSL; the new path's depth partition is internal to the module |
| `printGravity180` | query | 19 | 1 | 18 | screen (text law) | NONE: draws text along the dome curvature; a drawing law, not an information |
| `getViewportHeight` | query | 17 | 3 | 14 | screen | NONE |
| `projectLocal` | query | 15 | 0 | 15 | projection law (local -> screen) | NONE: the module projects in shaders; CPU side only ProjectionTransfer's radial law |
| `projectEarthEqu` | query | 10 | 0 | 10 | projection law (equatorial -> screen) | NONE |
| `unprojectEarthEqu` | query | 6 | 0 | 6 | inverse law (screen -> equatorial) | partly: observedPosToRaDe / observedToBodyEquPos take an EYE direction; screen -> eye is not in Camera |
| `getMatJ2000ToEye` | query | 5 | 0 | 5 | frame matrix (J2000 -> eye) | NONE: needs the reference's accumulated rotation, a ModularBody member |
| `getViewportWidth` | query | 5 | 0 | 5 | screen | NONE |
| `getMatEarthEquToEye` | query | 4 | 1 | 3 | frame matrix (equatorial -> eye) | partly: viewRotation() * placementRotation(), rotation only, view offset excluded |
| `getViewportRadius` | query | 4 | 1 | 3 | screen | NONE |
| `projectLocalCheck` | query | 4 | 0 | 4 | projection law (local -> screen) | NONE |
| `getMatLocalToEye` | query | 3 | 1 | 2 | frame matrix (local -> eye) | partly: observedToLocalPos is the inverse applied; renderViewRotation() is private |
| `getAimFov` | query | 2 | 0 | 2 | fov (zoom target) | partly: getZoomDstHalfFov() - "for the trace harness"; unset until the first timed zoom |
| `getViewportCenter` | query | 2 | 0 | 2 | screen | NONE |
| `projectCustom` | query | 2 | 2 | 0 | projection law (given matrix -> screen) | NONE |
| `projectJ2000LineCheck` | query | 2 | 0 | 2 | projection law (J2000 -> screen) | NONE |
| `unprojectLocal` | query | 2 | 0 | 2 | inverse law (screen -> local) | partly: observedToLocalPos / observedPosToAltAz take an EYE direction; screen -> eye is not in Camera |
| `getClippingPlanes` | query | 1 | 1 | 0 | clipping | NONE |
| `getCursorPosEqu` | query | 1 | 0 | 1 | inverse law (screen -> equatorial) | partly: as unprojectEarthEqu |
| `getMatProjection` | query | 1 | 0 | 1 | projection matrix | NONE |
| `getViewport` | query | 1 | 0 | 1 | screen | NONE |
| `getViewportFloatCenter` | query | 1 | 0 | 1 | screen | NONE |
| `projectCustomCheck` | query | 1 | 0 | 1 | projection law (given matrix -> screen) | NONE |
| `projectDomeFixed` | query | 1 | 0 | 1 | projection law (dome -> screen) | NONE |
| `projectJ2000` | query | 1 | 0 | 1 | projection law (J2000 -> screen) | NONE |
| `projectJ2000Check` | query | 1 | 0 | 1 | projection law (J2000 -> screen) | NONE |
| `unprojectNormalizedLocal` | query | 1 | 0 | 1 | inverse law (screen -> local) | partly: same; atmosphere.cpp:237 already branches on in.prj and uses ProjectionTransfer + eyeToLocal |
| `unprojectViewport` | query | 1 | 0 | 1 | inverse law (viewport -> sky cap) | NONE |
| `setClippingPlanes` | request | 7 | 6 | 1 | clipping | no counterpart |
| `setFov` | request | 3 | 0 | 3 | fov | setHalfFov(x, 0) - NB the owner's pair passes no duration, so the default 0.5 s applies |
| `zoomTo` | request | 2 | 0 | 2 | fov | setHalfFov(x, duration) |
| `applyViewport` | request | 1 | 0 | 1 | screen | no counterpart; called once per frame by Core::applyClippingPlanes |
| `changeFov` | request | 1 | 0 | 1 | fov (relative) | none: today Camera is fed the projector's landed value (core.cpp:1885) |
| `setMaxFov` | request | 1 | 0 | 1 | fov limit | none: Camera::maxHalfFov is a private static with no setter |
| `setModelViewMatrices` | frame step | 4 | 0 | 4 | frame matrices (navigator -> projector push) | old engine internal; Camera::update dispatches its own mat |
| `updateAutoZoom` | frame step | 4 | 0 | 4 | fov (zoom plan advance) | Camera::update |
| `dumpTrace` | instrument | 1 | 0 | 1 | - | Camera::dumpTrace |

**Navigator** - 46 names, 226 calls

| method | kind | calls | old path proper | shared | information | answered today by / Camera counterpart |
|---|---|---|---|---|---|---|
| `getHelioToEyeMat` | query | 29 | 9 | 20 | frame matrix (heliocentric -> eye) | NONE: ingredients: private viewMat() + the body tree; getRootPosition() gives the place only |
| `earthEquToLocal` | query | 21 | 3 | 18 | frame conversion (equatorial -> local) | partly: placementRotation() public; no member converts a vector this way |
| `getObserverHelioPos` | query | 16 | 8 | 8 | place (in the system frame) | partly: getRootPosition() / positionRelativeTo(body): other frame (root, body-equatorial) |
| `helioToEarthPosEqu` | query | 11 | 2 | 9 | frame conversion (heliocentric -> equatorial, position) | NONE |
| `j2000ToEarthEqu` | query | 10 | 0 | 10 | frame conversion (J2000 -> equatorial) | NONE |
| `getLocalToEyeMat` | query | 8 | 0 | 8 | frame matrix (local -> eye) | partly: observedToLocalPos is the inverse applied; renderViewRotation() is private |
| `getJ2000ToEyeMat` | query | 7 | 0 | 7 | frame matrix (J2000 -> eye) | NONE |
| `getFlagTraking` | query | 6 | 0 | 6 | tracking | Camera: getTrackedBody() != nullptr |
| `getHeading` | query | 6 | 0 | 6 | heading | Camera: getHeading() (rad; old is degrees) |
| `getLocalVision` | query | 6 | 1 | 5 | view direction (local) | Camera: getForwardLocal() - "for the trace harness"; new local = oldLocalToLocal(old local) |
| `getDomeFixedMat` | query | 5 | 0 | 5 | frame matrix (dome, no heading) | NONE |
| `getEarthEquToEyeMat` | query | 5 | 0 | 5 | frame matrix (equatorial -> eye) | partly: viewRotation() * placementRotation() |
| `helioToLocal` | query | 5 | 0 | 5 | frame conversion (heliocentric -> local) | NONE |
| `geTdomeMat` | query | 4 | 0 | 4 | frame matrix (dome, heading) | NONE |
| `getEarthEquToEyeMatFixed` | query | 4 | 0 | 4 | frame matrix (equatorial -> eye, fixed) | NONE |
| `getViewingMode` | query | 4 | 0 | 4 | mount | NONE: getMount was deleted by the owner (toggleMount: tell, never ask) |
| `earthEquToJ2000` | query | 3 | 0 | 3 | frame conversion (equatorial -> J2000) | NONE |
| `getPrecEquVision` | query | 3 | 0 | 3 | view direction (equatorial, precessed) | NONE |
| `getFlagAutoMove` | query | 2 | 0 | 2 | motion plan (view) | partly: getPlanTimers()[0] > 0 - "for the trace harness" |
| `getFlagChangeHeading` | query | 2 | 0 | 2 | motion plan (heading) | partly: getPlanTimers()[1] > 0 - "for the trace harness" |
| `getFlagLockEquPos` | query | 2 | 0 | 2 | sky lock | Camera: getSkyLock() |
| `getMoveCoef` | query | 2 | 0 | 2 | motion plan (progress) | NONE |
| `getViewOffset` | query | 2 | 0 | 2 | view offset | Camera: getViewOffset() |
| `helioToEarthEqu` | query | 2 | 0 | 2 | frame conversion (heliocentric -> equatorial) | NONE |
| `earthPosEquToHelio` | query | 1 | 1 | 0 | frame conversion (equatorial -> heliocentric) | NONE |
| `getEquVision` | query | 1 | 0 | 1 | view direction (equatorial) | partly: observedToBodyEquPos applied to the eye forward |
| `localToEarthEqu` | query | 1 | 0 | 1 | frame conversion (local -> equatorial) | partly: localToBodyEqu() does it and is private |
| `moveTo` | request | 6 | 0 | 6 | view direction (timed) | lookTo(dir, duration) / trackBody(body) |
| `setFlagTraking` | request | 4 | 0 | 4 | tracking | trackBody |
| `setLocalVision` | request | 4 | 1 | 3 | view direction (snap) | lookTo(dir, 0) |
| `updateMove` | request | 4 | 0 | 4 | view direction (relative) | lookRel(dAlt, dAz, 0) |
| `setHeading` | request | 3 | 1 | 2 | heading | setHeading(h, 0) / moveHeading(delta) |
| `setViewingMode` | request | 3 | 0 | 3 | mount | setMount / toggleMount |
| `changeHeading` | request | 2 | 1 | 1 | heading (timed) | setHeading(h, duration) |
| `setDefaultHeading` | request | 2 | 0 | 2 | heading default | none |
| `alignUpVectorTo` | request | 1 | 1 | 0 | view roll | none |
| `lookAt` | request | 1 | 0 | 1 | view direction (alt, az) | lookTo(alt, az, duration) |
| `restoreVision` | request | 1 | 0 | 1 | view direction (session) | restoreSession |
| `setFlagLockEquPos` | request | 1 | 0 | 1 | sky lock | setSkyLock |
| `setViewOffset` | request | 1 | 0 | 1 | view offset | setViewOffset |
| `setViewOffsetTransition` | request | 1 | 0 | 1 | view offset (latch) | restoreViewOffsetLatch |
| `updateTransformMatrices` | frame step | 13 | 7 | 6 | frame matrices | Camera::update |
| `update` | frame step | 4 | 0 | 4 | view plan advance | Camera::update |
| `updateVisionVector` | frame step | 4 | 0 | 4 | view direction (tracking advance) | Camera::update |
| `updateViewMat` | frame step | 2 | 0 | 2 | frame matrices | Camera::update |
| `dumpTrace` | instrument | 1 | 0 | 1 | - | Camera::dumpTrace |

**Observer** - 39 names, 200 calls

| method | kind | calls | old path proper | shared | information | answered today by / Camera counterpart |
|---|---|---|---|---|---|---|
| `getAltitude` | query | 32 | 3 | 29 | place (altitude) | Camera: getPlace()[2] (AU above the reference; old is metres) / distanceToReference() |
| `getHomeBody` | query | 20 | 2 | 18 | reference body | partly: getReferenceBody() returns ModularBody*, the callers use the old Body type |
| `isOnBody` | query | 14 | 8 | 6 | reference body (on a body or not) | partly: !isFreeMode() && !getReferenceBody()->isSystem() - composed today in EnvironmentManager.cpp:68 |
| `getLatitude` | query | 12 | 2 | 10 | place (latitude) | Camera: getPlace()[1] (rad, both modes) / getLatitude() (anchored only) |
| `getHeliocentricPosition` | query | 11 | 11 | 0 | place (in the system frame) | partly: getRootPosition() / positionRelativeTo(body): other frame |
| `isEarth` | query | 11 | 0 | 11 | reference body | Camera: getReferenceBody(), name test |
| `getLongitude` | query | 10 | 4 | 6 | place (longitude) | Camera: getPlace()[0] (rad, both modes) / getLongitude() (anchored only) |
| `getHomePlanetEnglishName` | query | 5 | 0 | 5 | reference body | Camera: getReferenceBody()->getEnglishName() |
| `getObserverCenterPoint` | query | 4 | 0 | 4 | place (centre of the reference, system frame) | NONE: a body property on the new path (getCachedRootPosition) |
| `getRotEquatorialToVsop87` | query | 4 | 2 | 2 | frame rotation (equatorial -> ecliptic) | NONE: a body property on the new path |
| `isSun` | query | 3 | 0 | 3 | reference body | Camera: getReferenceBody(), name test |
| `getAnchorPoint` | query | 2 | 2 | 0 | anchor | NONE: CameraAnchors is the new-path counterpart, not the Camera |
| `getEyeRelativeMode` | query | 2 | 1 | 1 | eye-relative mode | NONE: old-path-only concept |
| `getLongitudeForDisplay` | query | 2 | 0 | 2 | place (longitude) | Camera: getPlace()[0], wrapped by the caller (coreLink.hpp:812 does it) |
| `getDefaultAltitude` | query | 1 | 0 | 1 | place default | NONE: config value, not the Camera's |
| `getDefaultLatitude` | query | 1 | 0 | 1 | place default | NONE: config value, not the Camera's |
| `getDefaultLongitude` | query | 1 | 0 | 1 | place default | NONE: config value, not the Camera's |
| `getDistanceFromCenter` | query | 1 | 1 | 0 | place (radial distance) | partly: distance is private; distanceToReference() + the reference's altitude reference |
| `getQuaternionMode` | query | 1 | 0 | 1 | quaternion mode | NONE: old-path-only concept |
| `getRotLocalToEquatorial` | query | 1 | 1 | 0 | frame rotation (local -> equatorial) | partly: placementRotation() transposed |
| `setAltitude` | request | 11 | 3 | 8 | place (altitude) | setAltitude(metres) |
| `setLongitude` | request | 9 | 8 | 1 | place (longitude) | setLongitude(deg) |
| `setAnchorPoint` | request | 7 | 7 | 0 | reference body | warpToBody / switchToBody / relocateToBody |
| `setLatitude` | request | 6 | 5 | 1 | place (latitude) | setLatitude(deg) |
| `moveTo` | request | 2 | 1 | 1 | place (timed) | moveTo(lon, lat, alt AU) |
| `setEyeRelativeMode` | request | 2 | 0 | 2 | eye-relative mode | none |
| `load` | request | 1 | 0 | 1 | place (init from config) | SSystemFactory::loadCamera |
| `moveRelAlt` | request | 1 | 0 | 1 | place (relative) | moveRelAlt |
| `moveRelLat` | request | 1 | 0 | 1 | place (relative) | moveRelLat |
| `moveRelLon` | request | 1 | 0 | 1 | place (relative) | moveRelLon |
| `multAltitude` | request | 1 | 0 | 1 | place (altitude, multiplicative) | multAlt |
| `setConf` | request | 1 | 0 | 1 | place (save to config) | none |
| `setQuaternionMode` | request | 1 | 0 | 1 | quaternion mode | none |
| `setRelRotation` | request | 1 | 1 | 0 | quaternion rotation | none |
| `update` | frame step | 5 | 0 | 5 | place plan advance | Camera::update |
| `dumpTrace` | instrument | 1 | 0 | 1 | - | Camera::dumpTrace |
| `get` | not a method | 9 | 0 | 9 | - | std::shared_ptr::get() on `observatory` - not an Observer method |
| `reset` | not a method | 1 | 0 | 1 | - | std::shared_ptr::reset() on `observatory` - not an Observer method |
| `transfo4d` | not a method | 1 | 0 | 1 | - | Vec3d::transfo4d on a local named `obs` (meteor.cpp:151) - not an Observer method |
