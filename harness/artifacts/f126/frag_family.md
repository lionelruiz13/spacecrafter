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
