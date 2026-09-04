# F82 - the portrait leg: one launch per aspect, both paths, both readback channels

binary `c8e12950` - code master-beta @ 85cc2785 (engine sources ba7a32a8), target `Sun`, jd 2461288.9841951793 (F81's measured meridian transit), view_offset 0 everywhere.

Channel A = the app's own readback (`body action screenshot`); channel B = `x11grab -window_id <client>` of the app's window (11.172(c)).


## 1. What the two launches report about themselves

| reading | source | square (control) | portrait | same? |
|---|---|---|---|---|
| config asked | farm config.ini | 1024x1024 | 768x1024 | no |
| config read back | farm config.ini | 1024x1024 | 768x1024 | no |
| `Windows size is` | sdl_facade.cpp:199 (the REQUESTED size) | 1024x1024 | 768x1024 | no |
| X client window | xwininfo -root -tree | 1024x1024 | 768x1024 | no |
| `Swapchain :` | VulkanMgr.cpp:149 | (1024, 1024) | (768, 1024) | no |
| `Scaling :` | VulkanMgr.cpp:148 | 0.5 | 0.375 | no |
| `Viewport :` | VulkanMgr.cpp:150 | (2048, -2048) | (2048, -2048) | YES |
| `Rect :` | VulkanMgr.cpp:155 | (2048, 2048, 0, 0) | (2048, 2048, 0, 0) | YES |
| FrameBuffer 'main 0' | applog | (2048, 2048) | (2048, 2048) | YES |
| projector.viewport @ fov 180 | the projector's OWN dump | [0, 0, 2048, 2048] | [0, 0, 2048, 2048] | YES |
| projector.viewportRadius @ fov 180 | the projector's OWN dump | 1024 | 1024 | YES |
| projector.viewportCenter @ fov 180 | the projector's OWN dump | [1024, 1024, 0] | [1024, 1024, 0] | YES |
| camera.halfFov @ fov 180 | the camera's OWN dump | 1.57079637 | 1.57079637 | YES |
| channel A frame dims @ fov 180 | the written PNG | [2048, 2048] | [2048, 2048] | YES |
| A: H == 2*viewportRadius @ fov 180 | measured, not inherited | True | True | YES |
| channel B grab dims @ fov 180 | x11grab of the client window | 1024x1024 | 768x1024 | no |
| projector.viewport @ fov 90 | the projector's OWN dump | [0, 0, 2048, 2048] | [0, 0, 2048, 2048] | YES |
| projector.viewportRadius @ fov 90 | the projector's OWN dump | 1024 | 1024 | YES |
| projector.viewportCenter @ fov 90 | the projector's OWN dump | [1024, 1024, 0] | [1024, 1024, 0] | YES |
| camera.halfFov @ fov 90 | the camera's OWN dump | 0.785398185 | 0.785398185 | YES |
| channel A frame dims @ fov 90 | the written PNG | [2048, 2048] | [2048, 2048] | YES |
| A: H == 2*viewportRadius @ fov 90 | measured, not inherited | True | True | YES |
| channel B grab dims @ fov 90 | x11grab of the client window | 1024x1024 | 768x1024 | no |
| projector.viewport @ fov 40 | the projector's OWN dump | [0, 0, 2048, 2048] | [0, 0, 2048, 2048] | YES |
| projector.viewportRadius @ fov 40 | the projector's OWN dump | 1024 | 1024 | YES |
| projector.viewportCenter @ fov 40 | the projector's OWN dump | [1024, 1024, 0] | [1024, 1024, 0] | YES |
| camera.halfFov @ fov 40 | the camera's OWN dump | 0.34906584 | 0.34906584 | YES |
| channel A frame dims @ fov 40 | the written PNG | [2048, 2048] | [2048, 2048] | YES |
| A: H == 2*viewportRadius @ fov 40 | measured, not inherited | True | True | YES |
| channel B grab dims @ fov 40 | x11grab of the client window | 1024x1024 | 768x1024 | no |

## 2. The zero control (offset 0, the tracked target at the dome centre), FIRST on each aspect

| aspect | path | viewportRadius | old dump screen | channel A centroid | distance from (R,R) px | pass (<= 0.25) |
|---|---|---:|---|---|---:|---|
| square | old | 1024 | [1024, 1024] | (1024.18, 1024.044) | 0.1853 | PASS |
| square | new | 1024 | [1024, 1024] | (1024.143, 1024.031) | 0.1463 | PASS |
| portrait | old | 1024 | [1024, 1024] | (1024.18, 1024.044) | 0.1853 | PASS |
| portrait | new | 1024 | [1024, 1024] | (1024.143, 1024.031) | 0.1463 | PASS |

## 3. The render -> window map, FITTED from the bodies found in both channels

model: `window = a * A_centroid + b`, least squares over every body found in channel A and channel B of the same frame.

| aspect | path | n bodies | a (x) | b (x) | max res x | a (y) | b (y) | max res y | dome centre in window | bodies found: bottom / centred / top |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| square fov 180 | old | 24 | 0.499351 | 0.1377 | 0.1899 | 0.499571 | -0.0539 | 0.2756 | [511.473, 511.507] | 24 / 24 / 24 |
| square fov 180 | new | 24 | 0.499332 | 0.1598 | 0.1466 | 0.499529 | -0.0078 | 0.2164 | [511.476, 511.51] | 24 / 24 / 24 |
| square fov 90 | old | 20 | 0.499504 | -0.0089 | 0.0213 | 0.499537 | -0.018 | 0.002 | [511.483, 511.508] | 20 / 20 / 20 |
| square fov 90 | new | 20 | 0.499497 | -0.0004 | 0.0241 | 0.499532 | -0.0145 | 0.0063 | [511.485, 511.506] | 20 / 20 / 20 |
| square fov 40 | old | 2 | 0.499514 | 0.0001 | 0.0 | 0.499647 | -0.1374 | 0.0 | [511.502, 511.501] | 2 / 2 / 2 |
| square fov 40 | new | 2 | 0.499505 | 0.0035 | 0.0 | 0.499729 | -0.2224 | 0.0 | [511.497, 511.5] | 2 / 2 / 2 |
| portrait fov 180 | old | 24 | 0.374484 | 0.0302 | 0.1097 | 0.374576 | 255.9246 | 0.2633 | [383.502, 639.49] | 24 / 0 / 0 |
| portrait fov 180 | new | 24 | 0.374476 | 0.0406 | 0.104 | 0.374332 | 256.1567 | 0.3059 | [383.504, 639.473] | 24 / 0 / 0 |
| portrait fov 90 | old | 20 | 0.37451 | -0.0074 | 0.0197 | 0.374395 | 256.1434 | 0.0227 | [383.491, 639.524] | 20 / 0 / 0 |
| portrait fov 90 | new | 20 | 0.374515 | -0.0129 | 0.0193 | 0.3744 | 256.1251 | 0.0132 | [383.49, 639.511] | 20 / 0 / 0 |
| portrait fov 40 | old | 2 | 0.374663 | -0.1555 | 0.0 | 0.374324 | 256.1895 | 0.0 | [383.499, 639.497] | 2 / 0 / 0 |
| portrait fov 40 | new | 2 | 0.374661 | -0.1594 | 0.0 | 0.374454 | 256.0493 | 0.0 | [383.493, 639.49] | 2 / 0 / 0 |

**At the square control the three candidate seeds COINCIDE** (swapchain height == the scaled square), so the counts there are 24/24/24 by construction and discriminate nothing - which is exactly why the control cannot see this. Portrait separates them by 128 px and the counts are 24 / 0 / 0.


The predictor, evaluated from each launch's OWN measured swapchain extent (VulkanMgr.cpp:145-163 + VulkanMgr.hpp:122-124):

| aspect | swapchain | scaled = min(W,H) | a = (scaled-1)/(2R) | dome x = (W-1)/2 | dome y, BOTTOM = H-1-(scaled-1)/2 | dome y, CENTRED (mutation) | measured dome centre |
|---|---|---:|---:|---:|---:|---:|---|
| square | 1024x1024 | 1024 | 0.499512 | 511.5 | 511.5 | 511.5 | [511.483, 511.508] |
| portrait | 768x1024 | 768 | 0.374512 | 383.5 | 639.5 | 511.5 | [383.491, 639.524] |

predicted before the run (integer form): portrait a=0.375, b=0.0, d=256.0, dome centre [384.0, 640.0]; square a=0.5, b=0.0, d=0.0, dome centre [512.0, 512.0]. The half-pixel between those and the row above is the `(scaled - 1)` term of `mouseNorm.scaleX/Y`, which the pre-run form rounded; it moves nothing that this leg decides.

mutation (centred letterbox): the model a reader would assume, and the one the X axis obeys: d = (swapH - scaledH)/2 = 128 -> disc rows [128, 895], dome centre (384, 512). It differs from the prediction by 128 px and one measurement decides between them.


## 4. The disc extent, second and independent read (star field on, fov 180) - geometry only

| aspect | path | channel | frame/grab dims | lit bbox x0..x1 | y0..y1 | bbox w x h |
|---|---|---|---|---|---|---|
| square | old | A | 2048x2048 | 2..2045 | 0..2039 | 2044 x 2040 |
| square | old | B | 1024x1024 | 1..1017 | 0..1018 | 1017 x 1019 |
| square | new | A | 2048x2048 | 2..2045 | 0..2039 | 2044 x 2040 |
| square | new | B | 1024x1024 | 1..1017 | 0..1018 | 1017 x 1019 |
| portrait | old | A | 2048x2048 | 2..2045 | 0..2039 | 2044 x 2040 |
| portrait | old | B | 768x1024 | 5..765 | 262..1015 | 761 x 754 |
| portrait | new | A | 2048x2048 | 2..2045 | 0..2039 | 2044 x 2040 |
| portrait | new | B | 768x1024 | 5..765 | 262..1015 | 761 x 754 |

## 5. The band the blit never writes (channel B, two grabs 1.5 s apart)

| aspect | grab | window | band rows | band max | band mean | nonzero px | grab md5 |
|---|---|---|---|---:|---:|---:|---|
| square | grab0 | 1024x1024 | none | - | - | - | `79e14ba9` |
| square | grab1 | 1024x1024 | none | - | - | - | `f6267cd9` |
| portrait | grab0 | 768x1024 | 0..256 | 0.0 | 0.0 | 0 | `a5f15161` |
| portrait | grab1 | 768x1024 | 0..256 | 0.0 | 0.0 | 0 | `50f1d3e2` |

## 6. Parity old vs new, and the ONE question this leg exists to answer: does any divergence exist ONLY at portrait? (11.52(b): perceptual, geometry only)

Every body is measured twice per aspect: the engine's own dual dump (old render px vs new normalized, brought to render px) and the luminance centroid in the frame the app itself wrote. `sq - pt` is the whole question: a non-zero there is a divergence the aspect created.

| fov | body | dump d, square | dump d, portrait | sq - pt | frame d, square | frame d, portrait | sq - pt |
|---:|---|---:|---:|---:|---:|---:|---:|
| 180 | Adrastea | 0.0321 | 0.0321 | 0.0 | 0.0318 | 0.0318 | 0.0 |
| 180 | Deimos | 0.0643 | 0.0643 | 0.0 | 0.069 | 0.069 | 0.0 |
| 180 | Mercury | 0.0092 | 0.0092 | 0.0 | 0.0139 | 0.0139 | 0.0 |
| 180 | Moon | 0.0855 | 0.0855 | 0.0 | 4.9769 | 4.9769 | 0.0 |
| 180 | Sun | 0.0 | 0.0 | 0.0 | 0.0392 | 0.0392 | 0.0 |
| 180 | Venus | 0.0497 | 0.0497 | 0.0 | 0.0436 | 0.0436 | 0.0 |
| 90 | Adrastea | 0.0641 | 0.0641 | 0.0 | 0.0653 | 0.0653 | 0.0 |
| 90 | Mercury | 0.0184 | 0.0184 | 0.0 | 0.0221 | 0.0221 | 0.0 |
| 90 | Sun | 0.0 | 0.0 | 0.0 | 0.02 | 0.02 | 0.0 |
| 90 | Venus | 0.0994 | 0.0994 | 0.0 | 0.0915 | 0.0915 | 0.0 |
| 40 | Mercury | 0.0414 | 0.0414 | 0.0 | 0.0403 | 0.0403 | 0.0 |
| 40 | Sun | 0.0001 | 0.0001 | 0.0 | 0.0269 | 0.0269 | 0.0 |

*fov 180: 17 bodies share ONE luminance blob at (1319.004, 894.796) (Adrastea, Amalthea, Ananke, Callisto, Carme, Elara, Europa, Ganymede, Himalia, Io, Jupiter, Leda, Lysithea, Metis, Pasiphae, Sinope, Thebe) - the frame channel cannot separate them, the dump channel can; the first is listed above and the rest are the same measurement.*

*fov 180: 3 bodies share ONE luminance blob at (1617.021, 769.363) (Deimos, Mars, Phobos) - the frame channel cannot separate them, the dump channel can; the first is listed above and the rest are the same measurement.*

*fov 90: 17 bodies share ONE luminance blob at (1614.021, 765.603) (Adrastea, Amalthea, Ananke, Callisto, Carme, Elara, Europa, Ganymede, Himalia, Io, Jupiter, Leda, Lysithea, Metis, Pasiphae, Sinope, Thebe) - the frame channel cannot separate them, the dump channel can; the first is listed above and the rest are the same measurement.*

## 7. SUPPLEMENTARY LEG - the same portrait window at the DEFAULT `render_size` (0), which is what `checkConfig.cpp:106` writes into a generated config.ini

This is a THIRD launch and a scope expansion, taken so that any mint from this leg can name which branch it fires in rather than assume it. Predictions in `f82_predictions_supplement.json`, committed before it ran.

| reading | portrait, render_size 2048 (field, authored) | portrait, render_size 0 (default) | predicted for the default branch |
|---|---|---|---|
| applog `Scaling` / `Viewport` / `Swapchain` / `Rect` | all four present ([0.375] / (2048, -2048) / (768, 1024) / (2048, 2048, 0, 0)) | ABSENT - all four are printed only inside `dedicatedViewport` | absent (the prediction named only three of the four; `Swapchain` is in that function too - prediction S_P1 partly WRONG, kept) |
| FrameBuffer 'main 0' | (2048, 2048) | (768, 1024) | (768, 1024) |
| projector.viewport | [0, 0, 2048, 2048] | [0, 128, 768, 768] | [0, 128, 768, 768] |
| projector.viewportCenter | [1024, 1024, 0] | [384, 512, 0] | [384, 512, 0] |
| projector.viewportRadius | 1024 | 384 | 384 |
| channel A readback dims | [2048, 2048] | [768, 768] | [768, 768] |
| target in channel A (new) | (1024.18, 1024.044) at R 1024 | (384.136, 384.028) at R 384 | (384, 384) +- the instrument zero |
| target in channel A (old) | (1024.18, 1024.044) at R 1024 | (384.184, 384.046) at R 384 | (384, 384) +- the instrument zero |
| **dome centre in the WINDOW** (new) | [383.504, 639.473] | (384.134, 512.028) | (384, 512) - CENTRED |
| **dome centre in the WINDOW** (old) | [383.502, 639.49] | (384.181, 512.048) | (384, 512) - CENTRED |

**The 128 px is the whole finding**: the same window, the same scene, the same binary; the authored branch puts the dome centre at y 639.5 and the default branch at y 512.0. The default branch also makes the PROJECTOR aspect-aware (viewport [0,128,768,768], radius 384) where the authored branch keeps it square and window-blind.


## 8. Environment asserts

- **portrait**: GetActive `(false,)`, no other spacecrafter before the launch, real `~/.spacecrafter` md5 in `03fbee59`/`545a51ef` == out `03fbee59`/`545a51ef`; fails: none
- **square**: GetActive `(false,)`, no other spacecrafter before the launch, real `~/.spacecrafter` md5 in `03fbee59`/`545a51ef` == out `03fbee59`/`545a51ef`; fails: none
- **defaultrender (supplementary)**: GetActive `(false,)`, no other spacecrafter before the launch, real `~/.spacecrafter` md5 in `03fbee59`/`545a51ef` == out `03fbee59`/`545a51ef`; fails: none
