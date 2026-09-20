
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
