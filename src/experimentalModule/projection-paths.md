# Projection paths — old vs new (investigation deliverable, 2026-07-11)

Requested by Vixy after invalidating §10.3.7 (Projector/Navigator carriage): track how
projection genuinely works in each path, then differential — what differs exactly, what
it enables, what it changes, functional equivalence in relative coordinates.

Provenance: two exhaustive sweeps (old path / new path), every claim file:line-tagged;
the load-bearing claims (fisheye.glsl math, projectCustom math, ModularBody::update
screenPos, Camera::update matrix build, the Moon-divergence note) re-verified first-hand
on source. `[derived]` marks algebra done here, not present in any source.

---

## A. Old path (Projector / Navigator / Body::computeDraw)

- **One projection model, equidistant fisheye**, implemented FOUR times:
  1. GPU: `fisheyeProject` — `win = ModelViewMatrix·pos`; θ = `asin(rq/depth)` with `z>0 → π−θ`;
     NDC.xy = `win.xy·θ/(rq·halfFov)`; depth = `(|win|−zNear)/(zFar−zNear)` linear-Euclidean; `w=1`,
     no perspective divide [verified: fisheye.glsl:6-19].
  2. CPU double, any point/frame: `Projector::projectCustom` — `a = π/2 + atan(z/rq)` (≡ same θ
     `[derived]`), pixels = `center + xy·a·(1/halfFov)·viewport_radius/rq`, depth identical form
     [verified: projector.cpp:186-220; scale factor projector.cpp:92].
  3. CPU float, per body center: inline in `Body::computeDraw` → `screenPos` [body.cpp:985-994].
  4. CPU replication for overlay geometry: axis/planetGrid [axis.cpp:65-79].
- **Frames**: ephemeris = heliocentric ecliptic VSOP87 AU, origin = system center, `Vec3d`
  [body.cpp:610-617]. Navigator owns the full double `Mat4d` chain local↔earthEqu↔J2000↔helio↔eye
  [navigator.hpp:224-244, navigator.cpp:215-330]. The single re-centering:
  `mat_helio_to_local` embeds `translation(−observerCenterPoint)` **in double** — AU-scale
  magnitudes are cancelled before any float exists [navigator.cpp:234-247].
- **Per-body model-view**: two-level composition only (satellite → parent → helio)
  [body.cpp:903-941]; `mat = helioToEye · translation(helioPos)·rot`, all `Mat4d`; float boundary
  = `mat.convert()` + `zrotation(axis_rotation+90°)` at draw [body_bigbody.cpp:318-319], and
  `eye_planet`/`eye_sun` narrowed to `Vec3f` [body.cpp:943-949].
- **clipping_fov = (zNear, zFar, halfFov_rad)** [projector.hpp:264-266; named literally at
  body_bigbody.cpp:673-675].
- **Depth**: far→near walk builds merged buckets `[dist − 1.1·bounding, dist + 1.1·bounding]`
  (note the **1.1 margin**), `setClippingPlanes` + depth clear per bucket; orbit pass uses the
  union bucket + depth-only body spheres [solarsystem_display.cpp:141-176,265-336].
- **Text**: `printGravity180(font, x, y, …)` consumes an ALREADY-PROJECTED pixel anchor; its own
  math is only `theta = π + atan2(dx, dy−1)` from viewport center + ortho2D MVP + tangential
  rotation [projector.cpp:398-414]. Its real dependencies: pixel anchor, viewport center/radius,
  a font. **No frame matrices.**
- Problems solved: dome-correct wide-fov projection; CPU/GPU pixel agreement for overlays and
  picking; float32 viability by double-until-boundary; depth precision by bucketing.

## B. New path (Camera / ModularBody chain)

- **No global frame ever exists.** Camera holds reference body + local pose (all float); per
  frame builds eye→reference-local `Mat4f`:
  `zrotation(heading)·xrotation(π/2−alt)·zrotation(az−π/2)` then free-position translation OR
  `(0,0,−distance)`+`xrotation(latitude−π/2)·zrotation(longitude)` (surface), optionally
  `·computeSurfaceToBody()` (co-rotate with ground) [verified: Camera.cpp:85-98].
- **Routing through the closest common parent** [ModularBody.cpp:122-145]: `dispatchUpdate`
  updates the reference subtree with the eye at its origin, then walks UP to the system root
  (`transformBodyToParent`: `zxrotation(precession−node, −obliquity)` + `+eclipticPos`), routing
  the eye DOWN into each sibling subtree (`selectiveUpdate` → `transformParentToBodyPos`:
  `−eclipticPos`; `computeBodyPosToBody`: `xzrotation(obliquity, node−precessionRate·Δt)`)
  [ModularBody.hpp:294-380]. Every translation is a parent-relative orbital offset; the explicit
  common-parent router is `calculateSwitchCompensation` (`findCommonParent`, up-compose,
  down-compose) [ModularBody.cpp:286-303, ModularBody.hpp:420-428] — the seamless reference
  switch (G3).
- **Precision**: float everywhere; double ONLY for `jd` and orbit evaluation, immediately
  narrowed: `Vec3d tmp → Vec3f eclipticPos` [ModularBody.hpp:294-306,301]. Design rationale
  in-tree [INTENT.md D1/D2, G2].
- **CPU projection surface = ONE point**: body center, branchless form
  `f = acos(−z/distance)/(rq·halfFov)`, `screenPos = (r12·f, r13·f)`; `screenSize =
  halfAngularSize/halfFov` [verified: ModularBody.hpp:222-226]. Same θ as A's forms `[derived:
  acos(−z/d) = π/2+atan(z/rq) = flip∘asin(rq/d)]`. Visibility = view-cone test on the same
  quantities [verified: ModularBody.hpp:183-219]. No unproject: selection is screen-space
  distance to body centers [ModularSystem.cpp:209-232].
- **GPU: the SAME fisheye** — BasicMesh borrows old-path pipelines, fills `globalVertProj` with
  `ModelViewMatrix = ModularBody::mat(·bodyToSurface)`, `clipping_fov = renderer.getClippingFov()`
  [BasicMesh.cpp:44-55]; `clippingFov = (zCenter−boundingRadius, zCenter+boundingRadius,
  ModularBody::halfFov)` [Renderer.cpp:30,39,57-58]. Same shader, same math; only the matrix
  supply changed.
- **Depth**: per-notable-body `clearDepth(distance, boundingRadius)` — clear + reranged slice,
  **no margin factor, no merging yet** (notableBody partitioning consumer pending)
  [ModularBody.hpp:259, Renderer.cpp:49-59, INTENT §5.3].
- **UI bridge**: ModularObject answers legacy queries via Camera conversions
  (`observedPosToRaDe/AltAz`); Projector/Navigator appear ONLY as unused vtable parameters —
  zero old-projection calls in the module [ModularObject.cpp:100-175; grep sweep].
- Problems solved: float32 viability BY TOPOLOGY (magnitudes bounded by construction); uniform
  arbitrary-depth hierarchy; seamless rebasing; cheap gating without a CPU projection service.

---

## C. Differential

**C1 — Shared invariant (verified).** Both paths project on the GPU with the *same* shader
function and the *same* `(zNear, zFar, halfFov_rad)` semantics. Given equal ModelViewMatrix and
clipping triple, pixels are identical. The entire difference lives in how that matrix and triple
are produced. FOV semantics equivalent: old `fov°·π/360` = half-fov radians = new `halfFov`
`[derived, both packings verified]`.

**C2 — Reference architecture.** Old: absolute heliocentric frames (double) + ONE observer
re-centering subtraction, then truncate. New: no absolute frame; the eye matrix is routed
up/down the tree through the closest common ancestor; only relative offsets are ever
materialized. Consequence `[derived]`: errors on the SHARED segment of the eye→A and eye→B
chains cancel in any A-vs-B relation; only the divergent segment (minimal, by definition of
closest common parent) contributes relative error. Old has no analogous cancellation — it needs
double to survive the absolute detour; new makes float sufficient structurally.

**C3 — Error model `[derived]`.** Old: exact until the float boundary; error ≈ 1e-7 relative,
applied once, on the observer-relative residual. New: ≈1e-7 relative per chain hop (ephemeris
downcast + float compose), hops = divergent-path depth (2-5 typical); errors scale with the
LOCAL offset each hop applies — scale-free by construction. Both end in the same float GPU
stage. Both are sub-pixel at normal fov; both approach visibility only near `minHalfFov =
8.7e-7` [Camera.cpp:11-12]. Net: no precision regression vs old, large structural win vs
old-at-float, and the double matrix machinery (cost) is gone.

**C4 — CPU projection surface.** Old: full projection service, 4 implementations (§A). New: one
body-center projection; everything else GPU. The old service exists to serve labels, picking,
overlays — consumers the new path either replaced (picking → screen-space centers) or hasn't
ported yet (labels, overlay lines). **Consolidation map (code-debt item)**: 1 GLSL function +
1 CPU helper (the acos body-center form) is the end state; old's #2/#3/#4 die with the old path;
new modules must NOT re-replicate CPU fisheye (axis/grid-style modules feed the same GPU shaders
through TRACE/COLOR families instead).

**C5 — Depth ranging: found delta.** Old buckets use `±1.1·boundingRadius` margin
[solarsystem_display.cpp:147-148]; new `clearDepth` uses exact `±boundingRadius`
[ModularBody.hpp:259]. With `w=1`, depth outside [0,1] is clipped — geometry exceeding the
bounding sphere (oblateness is inside, but terrain, atmosphere shells, rings later) will clip at
the slice boundary. The 1.1 was the old path's guard. **RESOLVED [vixy: 2026-07-11]**: boundingRadius is
DEFINED as the smallest sphere enclosing the whole traced body — inclusive by definition
(exactly why one value serves both the cone test and the depth bounds); no margin factor.
The contract carries the safety: a module drawing beyond its mesh radius (terrain, shell,
ring) must return the larger radius from update(). Documented in BodyModule.hpp +
Renderer.hpp clearDepth.

**C6 — Hierarchy.** Old composition is structurally two-level [body.cpp:914-927]; new is
arbitrary-depth with systems-as-bodies. This is what makes G2/G3 (galaxy ⊃ systems ⊃ …,
seamless rebasing) possible at all — the old path cannot express a reference switch without a
discontinuity because there is no common-parent route, only the absolute frame.

**C7 — What it changes for consumers (the §10.3.7 rework, open #3).**
- **Labels**: `printGravity180`'s only real inputs are pixel anchor + viewport geometry + font
  (§A). New path already computes the anchor (`screenPos`, same NDC family as the halo path).
  → The gravity-text rewrite is a small pure function `(screenPos, viewport, font, str) → draw`,
  homed as a Renderer text service; **no Projector, no Navigator, no frame context needed.**
  The invalidated frame-context channel is replaced by: nothing — the data was already there.
- **Picking**: old `unproject` (ray) vs new screen-space center distance — functionally
  sufficient for body selection; ray-unproject only needed if sub-body picking appears (defer).
- **UI queries**: ModularObject bridge exists; az-convention caveat open [ModularObject.cpp:118-121,
  INTENT §11.4].
- **Overlay geometry** (axis/grids/orbits): port as line families (§10.3) drawing through the
  same GPU projection — kills implementation #4.

**C8 — Functional equivalence in relative coordinates: verdict.**
- **By construction at the GPU stage: equivalent** (C1).
- **By intent at the chain stage: equivalent** — both produce eye→body-local; same ephemeris
  sources (`positionAtTimevInVSOP87Coordinates` both paths), same rotation elements.
- **In fact, currently: NOT equivalent** — the live Moon divergence is precisely a chain-stage
  disagreement [Camera.cpp:79-84, committed note; INTENT §1.3].
- **Equivalence conditions** (each must hold; unverified ones marked):
  E1 same orbit evaluation inputs (jd timing, element sets) — unverified;
  E2 same rotation-composition conventions per hop (old `rot_local_to_parent`
     [body.cpp:531-539] vs new `xzrotation`/`zxrotation` [ModularBody.hpp:308-367]) — unverified,
     prime suspect;
  E3 same observer pose conventions (az origin, heading sign; old alt-az local frame vs new
     `zrotation(az−π/2)` [Camera.cpp:85]) — unverified, prime suspect ("az : NO-OP" note);
  E4 halfFov semantics — VERIFIED equivalent;
  E5 depth-range policy — differs (C5), affects clipping not coordinates;
  E6 same body data (ssystem.ini parse deltas, e.g. INTENT §5.2/§11.3 hardcoded-flag absence).

**C9 — Moon-divergence analysis `[derived from the committed numbers, Camera.cpp:82-84]`.**
- Vixy's shift approximation `[-Y, X, Z]` is, on the xy-plane, exactly `zrotation(+90°)`
  (x′=−y, y′=x). Check against the numbers: predicted from expected = [+0.000103, −0.002451, −0.000536];
  Got = [+0.000141, −0.002141, +0.001364] — xy agrees in sign and order (hence "approximation"),
  **z disagrees in sign**: the discrepancy is NOT a pure z-rotation; a second component exists.
- `|Got| − |expected| = 3.21e-5 AU ≈ 4 800 km` — Earth-radius order: initially read as a
  topocentric-vs-geocentric hint. **SUPERSEDED by harness run (2026-07-11): the component is
  the Earth–Moon barycenter offset, CONFIRMED** — the Earth position difference between paths
  points exactly along the Moon direction (cos = +1.0000 at two dates 88 days apart, Moon
  direction rotated ~150° between them) with magnitude/lunar-distance = 0.01213 vs lunar mass
  fraction 1/82.3 = 0.01215 (4-digit match). New path's Earth is displaced toward the Moon
  relative to old ⇒ one path evaluates Earth at the EMB, the other corrects to Earth's center.
  [harness/analyze.py on /tmp/dual_trace{,_2}.json]
- ±90° z-rotation convention sites, candidates for the xy part: old draw applies
  `zrotation(axis_rotation + 90°)` [body_bigbody.cpp:318-319] where new `computeBodyToSurface`
  uses raw `axisRotation` [ModularBody.hpp:242-244]; new Camera builds `zrotation(az − π/2)`
  [Camera.cpp:85]; new surface-bound `transformBodyToParent` merges `zrotation(−π/2 −
  axisRotation)` [ModularBody.hpp:352-367]. (Axis-rotation sites affect orientation, not the
  Moon's observed position — they matter only through the Camera's surface/boundToSurface fold.)
- Status: analysis only — Vixy's active investigation, code untouched (INTENT §11.10
  do-not-disturb respected). These are discriminating observations, not a conclusion.

---

## D. Feeds

1. §10.3 open #3 → resolved proposal (C7): Renderer text service, pure function of
   screenPos + viewport + font; no frame context. Awaiting Vixy convergence.
2. Duplicate-code consolidation map (C4) — long-run: 1 GLSL + 1 CPU helper.
3. Found deltas for Vixy: C5 missing depth margin; C9 divergence observations; E1-E3/E6
   unverified equivalence conditions (E2/E3 prime suspects).
4. Moon investigation context (INTENT §11.3/§11.10): C9.
