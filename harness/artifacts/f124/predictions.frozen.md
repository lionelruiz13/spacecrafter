# INTENT §11.261

CHECKPOINT (F124 executor; update these lines at every checkpoint - the successor starts here)
- state: CP1 2026-09-20 - gate 20 PASS / 0 FAIL; code read; PREDICTIONS written (below); NOTHING LAUNCHED YET.
- next: freeze the predictions copy -> coverage map -> build `harness/f124_legs.py` + `f124_run.sh` by extending the b21 pattern -> `--no-scene` canary -> able-to-fail per leg -> trusted runs.
- files written so far: this file only. Nothing committed, nothing built, nothing under `src/` touched.

261. **F124 - THE INPUT PATH'S INSTRUMENT (row B45 item (1) as extended by §11.257(c)(5)).** (2026-09-20, Claude Fable 5.1, executor of F124 under session 34; code `main` @ `47b348bf`, binary md5 `13910b38`, harness `CC-harness` @ `5f341af`. HARNESS ONLY: nothing under `src/` edited, nothing built, nothing committed by the executor.)

## (P) PREDICTIONS - written from the code at HEAD BEFORE ANY LAUNCH

Frozen copy (mtime evidence for discriminating check (a)): `harness/artifacts/f124/predictions.frozen.md`.
Every law below was READ at `47b348bf`, file and line beside it. Magnitudes are arithmetic on those laws
(the command that produced them is in the executor's report); the scene is the b21 one: a synthetic
body `DescB radius 6000` (km), so R = 6000 km and the floor 0.125 R = 750 km. R is RE-READ from the
dump at run time; the predictions are stated in R so that a different scaled radius re-scales them.

### What the code says, in one table (the laws the legs stand on)

| information | new path (Camera) | old path | where read |
|---|---|---|---|
| mover base | `velocityScaling(1)` = max(`distance` - scaledDatum, 0.125 x scaledRadius), taken PER CALL | none: `altitude * coef` | `Camera.cpp:980-987`, `:30-33`; `observer.cpp:227-232` |
| anchored `multAlt(coef)` | `distance += base * (coef-1)`; no clamp at the datum when anchored | `setAltitude(altitude*coef)` | `Camera.cpp:733-739`, `:415-428` |
| free ascent (coef>=1) | step = (coef-1) x max(LIVE ground altitude, 0.125 x scaledRadius), along the view ray | `altitude*coef` | `Camera.cpp:750-760` |
| free descent (coef<1) | (1-coef) of the way to the ray/ground hit: NO floor | `altitude*coef` | `Camera.cpp:761-772` |
| lateral, anchored | `longitude += x` (rad), latitude likewise, NOT clamped | `moveTo(lat+x clamped to +-90, lon+x, alt, 0)` | `Camera.hpp:101-116`, `Camera.cpp:640-656`; `observer.cpp:204-218`, `:234-262` |
| lateral, free | eye-relative translation of x(rad) x 5 x `velocityScaling(1)` | the ANCHORED law (the old observer has no free mode) | `Camera.hpp:101-116` |
| interactive zoom | `setHalfFov(projector fov * pi/360, 0)` after `projection->changeFov` | `changeFov(+-5 x zoom_speed x dt x fov)`, clamps -0.15 fov / +20 | `core.cpp:1866-1889`; `Camera.cpp:682-726` |
| dt | enters the ZOOM only (`deplzoom`); `deltaHeight` is the constant 1.01 / 0.99; the lateral path has no dt argument at all | same | `core.cpp:1844-1880`, `:1783-1791`; `ui.cpp:588-610` |

INPUT PATH, as read (it decides what each leg drives):
- raise: joypad button `raise_height` -> `UI::raiseHeight()` -> `Core::raiseHeight(1)` -> `vzm.deltaHeight = 1.01` -> `Core::updateMove` -> `Observer::multAltitude` + `Camera::multAlt` (`joypad_controller.cpp:414-417`, `ui.cpp:640-643`, `core.cpp:1783-1786`, `:1876-1880`).
- lower: joypad button `lower_height` -> `UI::lowerHeight()` -> **`UI::lowerHeight(double 1)`** -> `CoreLink::observerMoveRelAlt(-0.02 x altitude, 0)` (`ui.cpp:720-723`, `:612-617`). **`Core::lowerHeight(1)` HAS NO CALLER in `src/`** (only `lowerHeight(0)` from `UI::stopLowerHeight`); legacy since the 2020 import (`git blame`: `f8efb16e`), not an agent change. The descent through `multAlt` is reached by the script command `camera action descend` (`CoreLink::cameraDescend`, new engine only) and by nothing else. `b21_keypath.py` leg K drives `Core::lowerHeight(1)`: a function no input reaches.
- lateral: joypad axis `move_lon` / `move_lat` -> `UI::moveLon(x)` / `UI::moveLat(x)` -> `CoreLink::observerMoveRelLon|Lat(+-x, DURATION_COMMAND)`. `DURATION_COMMAND` is the double 0.1 passed to an `int delay`: it truncates to 0, so every lateral input is a SNAP on both engines (`ui.cpp:59-60`, `coreLink.hpp:894-902`). They are NOT reached from `Core::updateMove`.
- zoom: keys / joypad buttons -> `Core::zoomIn(1)` / `zoomOut(1)` -> `vzm.deltaFov = -+1` -> `Core::updateMove`.
- NOTHING on these lines reads the render-path mode: both engines are informed whatever path is pinned (the stopgap of §11.253(c)). PREDICTION FOR BOTH PINS, every leg: the same numbers with `flag experimental_path on` and `off`, within the float32 quantum of `distance` (about 0.5 m at 8000 km).

### Leg Z - zoom feed (`Core::zoomIn|zoomOut(1)` + `updateMove(dt)`, one application per stop)
- Z1 (no zoom plan in flight): the Camera's `duration + zoomDuration == 0` branch applies at once, so after EVERY step 2 x halfFov (deg) == projector fov: |delta| <= 1e-6 relative (float32), mid-ramp as at the end. Per call the fov is multiplied by (1 - 5 x 0.0001 x dt): 0.9950 at dt = 10, 0.9800 at dt = 40 (config `zoom_speed = 0.0001`).
- Z2, TRANSITION (a key step landing on a zoom COMMAND in flight, `zoom fov <x> duration <d>`): SAME way -> the Camera takes the "already zooming" branch and does NOT snap: the two fovs DIFFER at that step (the Camera re-plans toward the projector's landed value, the projector's own auto-zoom keeps its cubic and overwrites the key delta on the next frame). OPPOSITE way -> the Camera snaps (the fix of §11.251(a)(3)) and its plan ends, while the projector's auto-zoom is still flagged and resumes: they differ again from the next frame on. Expected delta: non-zero mid-ramp in both sub-cases, zero once both plans have ended only if both land on the command's aim. This is the two-fov-law seam, not a defect of the feed; it is recorded, not judged.
- Limits: Camera clamps halfFov to [8.7e-7, 3.05] rad = full fov [9.97e-5, 349.51] deg; projector to [0.0001, max_fov (350 by default, config may lower it)]. If `max_fov` reads 350 in the dump, a zoom-out to the stop leaves a residual of 0.49 deg (350 - 349.51).

### Leg M - anchored `multAlt` (raise = the input path; lower = `Core::lowerHeight(1)` driven directly, NO input caller)
- M-high (start 2000 km, above the floor): new == old, x1.01 per step: 2020.0, 2040.2, 2060.602 km; |delta| <= 2 m.
- M-low up (start 10 km): new +7.5000 km per step (0.125 R x 0.01): 17.5, 25.0, 32.5 km; old 10.1, 10.201, 10.303 km. Delta new-old +7.400, +14.799, +22.197 km. CAUSE CHECK: step / (coef-1) == 0.125 x scaledRadius, and it must scale with R on a second body (Moon, scaled radius cited 8687 km in §11.258(b): step 10.859 km).
- M-low down (start 10 km) - "both ways where it was outward only", WITH ITS CONSEQUENCE: new 2.5 km, then **-5.0 km, then -12.5 km: the Camera goes UNDER THE DATUM** (no anchored clamp exists in `Camera::update`); old 9.9, 9.801, 9.703 km. Delta -7.400, -14.801, -22.203 km.
- M-datum - "the base is datum-relative where it was ground-relative": with `datum_radius` != `ground_radius` on DescB and a start above the floor, step / (coef-1) == altitude above the DATUM (the control surface's `altitude.new`), not above the ground. With the two radii 100 km apart at 2000 km the two readings differ by 1 km per step, 2000x the quantum.

### Leg A - free near-field ascent floor, both directions across it (looking at nadir: tracked, selected DescB)
- up, below the floor (start 200 km): +7.5 km per step whatever the altitude; old +2.0 km (x1.01). Delta +5.5 km on step 1.
- up, ACROSS it (start 740 km): 747.5, 755.0, then the law turns proportional: 762.55, 770.18 km (continuous at 750 km).
- down, ACROSS it (start 760 km): 752.40, 744.88, 737.43, 730.05 km - pure x0.99, NO floor in the descent, equal to the old path within the quantum.
- ASYMMETRY: below the floor one raise then one lower does not return: 200 -> 207.5 -> 205.425 km (old: 200 -> 202 -> 199.98).
- The free ascent floors the GROUND-relative live altitude, the anchored law the DATUM-relative `distance` member: the one remaining restatement of §11.251(a)(2), two different bases for one constant.

### Leg L - lateral movers (`UI::moveLon|moveLat` if gdb can reach the `UI` object, else the `CoreLink::observerMoveRel*` seam it calls - said in the report)
- anchored: both engines +x deg exactly (snap, delay 0): |delta| <= 1e-5 deg.
- anchored, ACROSS THE POLE (transition): from latitude 89.5 deg, +1 deg -> old clamps at 90.0, new reads 90.5 (no clamp in `Camera::moveRel`): residual +0.5 deg, and the new path's `fold()` is then derived with a latitude past the pole.
- free: new = eye-relative translation of x(rad) x 5 x base; for x = 1 deg and base = the floor: 65.45 km. The old observer applies its anchored law (+1 deg of longitude = 108.2 km of arc at 6200 km): the two engines separate by construction in free mode - recorded, it is the "free mode is new-only" seam b21's P4 already showed.
- free, STALE BASE (predicted defect): `velocityScaling` reads the `distance` MEMBER; in free mode the place lives in `position` and nothing on the free path writes `distance`. Enter free mode at 2000 km, `moveto alt 200` (free: writes `position` only): the lateral step should stay **174.53 km** (base 2000 km, stale) where the live altitude would give the floor's 65.45 km.

### Leg D - each of the above at `updateMove(10)` and `updateMove(40)`
- M and A: the per-CALL step is IDENTICAL at both dt (ratio 1.000): `deltaHeight` is a constant and the base is `velocityScaling(1)`, not `velocityScaling(deltaTime)`. Per SECOND the displacement therefore follows the frame rate: below the floor 750 km/s = 0.125 R/s at 100 fps against 187.5 km/s = 0.03125 R/s at 25 fps (ratio 4.0); above it x2.7048 /s against x1.2824 /s. `MIN_MOVEMENT_SPEED` is declared "in body_radius/s" (`ModularBody.hpp:100-101`): it is met at exactly 100 fps with coef 1.01 and nowhere else. The OLD path has the same frame-rate dependence (legacy): the dt leg separates no engine from the other, it measures both against the second.
- Z: per-call factor 0.9950 / 0.9800; per second 0.60577 / 0.60346 - dt-proportional to first order (0.76 % apart in log rate, the compounding), on both engines.
- L: dt does not exist on the path (`UI::moveLon(double)` has no time argument): identical by construction, shown once, not a separate campaign.

### Leg S (only if capacity remains) - `Core::findAndSelect(int x, int y)`; no prediction is written until its code is read.
