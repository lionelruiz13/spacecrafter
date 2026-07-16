# Shadow paths — old vs new (S5/G7 design deliverable, 2026-07-12)

Requested by Vixy: make the new path's shadowing work as well or better than the legacy
system — the "experimental shadow" (`experimental_shadows`) being the legacy generalized
generation — mapped onto the ModularBody/BodyModule primitives. Companion of
projection-paths.md, same method: per-path report, then differential. Every load-bearing
claim re-verified first-hand on source this pass (not carried from summaries). `[derived]`
marks algebra done here; `[vixy: 2026-07-12]` marks decisions from the S5 planning session.

Status: section B is the DESIGN — normative once implemented, at which point each contract's
authoritative home is the header owning it (I1) and B reduces to pointers + rationale.

---

## A. Old path — two generations, per-body selected

Selection between them [observed]: `ProtoSystem::computeDraw` nominates a center-of-interest
(CoI) `mainBody` only when `Context::experimental_shadows && Context::shadow_ready`
(protosystem.cpp:992-1000); each body's `selectShader()` picks a `*Shadowed` pipeline only if
`isCenterOfInterest` (body_bigbody.cpp:121,181; body_artificial.cpp:114). Flag off → no CoI →
Gen-1 everywhere. Flag on → exactly one body (plus its parent when
`isSatellite && !ARTIFICIAL-pair` — with the in-code TODO "find a better heuristic",
solarsystem_display.cpp:67-70) uses Gen-2; everyone else still Gen-1.

### A1. Generation 1 — analytic eclipse-map LUT (always on, every body)

- Data: one shared LUT `bodies/eclipse_map.png` (`Body::tex_eclipse_map`, body.hpp:643,
  loaded solarsystem_tex.cpp:73-74); per-frame `sun_half_angle` (body.cpp:948-958).
- CPU occluder feed, up to 4 slots (`MoonPosition1-4`/`MoonRadius1-4` in globalFrag):
  - planets: satellites within the sun-line corridor
    `moonDotLight>0 && length·sin(acos(moonDotLight)) <= radius + 2·moonRadius`
    (body_bigbody.cpp:359-398, unused slots zeroed) → solar-eclipse spots;
  - moons: slot 1 = the parent planet (body_moon.cpp:317-330) → lunar eclipse.
- Shader math, identical block per occluder (body_normal.frag:42-79, my_earth.frag,
  my_moon.frag:57-66, body_ringed.frag:63-98):
  `ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0, 1)`,
  `ratio.x = angularSeparation/(moonHalfAngle+SunHalfAngle)`,
  `diffuse *= texture(shadowTexture, ratio).r` — the LUT encodes the penumbra intensity as a
  function of (separation, size-ratio); analytic, viewpoint-independent, works at any screen
  size.
- Umbra color (my_moon only): `UmbraColor = (0.4,0.12,0)` iff isEarthMoon
  (body_moon.cpp:319,324); composition
  `diffuse·(shadowScale + umbra·max(0, 1-shadowScale))` (my_moon.frag:69) — an ADDITIVE dim-red
  floor inside the umbra, hardcoded to one body pair.
- Ring analog, fully analytic: ring→planet by ray-projecting the surface point onto the ring
  plane and darkening by ring alpha (`diffuse *= mix(1.0, 0.3, rcolor.a)`,
  body_ringed.frag:44-56); planet→ring by angular-disc eclipse test
  (`SeparationAngle < PlanetHalfAngle → diffuse = 0`, ring_planet.vert:41-43 + frag:23).
- Small-body variant zeroes all occluders (body_smallbody.cpp:178-181) — Gen-1 receivers are
  effectively big bodies and moons.

### A2. Generation 2 — "experimental" projected shadow maps (CoI only)

End-to-end dataflow [all observed]:
1. **Caster selection** (solarsystem_display.cpp:85-134): per receiver (mainBody, optionally
   its CoI parent), scan ALL sorted system bodies with a light-cylinder test in heliocentric
   double coordinates: keep body when `d = v1·v2 ∈ (0, sd1)` and
   `((v2 − v1·(d/sd1))/(r1+sunRadius+d·(−sunRadius/sd1)+r2))² < 1`
   — i.e. inside the cone-widened corridor between sun and receiver.
2. **Occlusion gate** (solarsystem_display.cpp:192-195,213-216): penumbra growth radius
   `smoothRadius = (sunRadius/|v1|²)·axialDist`; caster kept iff
   `smoothRadius < 4·casterBoundingRadius`. `[derived]` peak occlusion ≈
   (casterRadius/penumbraRadius)² — the rule is exactly **peak occlusion ≥ 1/16**; the code
   comment "less than 4% of occlusion" is the loose paraphrase. Confirmed as the intended
   criterion [vixy: 2026-07-12]: ISS-on-Earth negligible, Moon-on-Earth not; NOT gated by
   receiver screen size.
3. **Silhouette pass**: `shadowMat = mat3((lookAt(sun→receiver) · casterModel ·
   scaling(initialRadius/(boundingRadius+smoothRadius), z·one_minus_oblateness)))`
   (body.cpp:1222-1230); `shadow_trace.vert` = pure orthographic
   `gl_Position = vec4(tmp.xy, tmp.z·0.5+0.5, 1)`; pipeline `shadowShape`, stencil REPLACE
   mask 0x01 on `renderShadow` (bodyShader.cpp:428-444, app.cpp:360-367); target
   `shadowTrace` D24S8 **stencil aspect** at `shadowRes`² (app.cpp:284-291 — with in-code
   notes: STORAGE unsupported / S8_UINT unsupported → the D24S8 stand-in).
4. **Disc blur** (shadow.comp): sliding circular box filter of `radius` px (= the projected
   sun angular radius: penumbra), `offsets[i] = sqrt(r²−i²)` and `pixelCount = 4·Σ+1`
   computed CPU-side per assignment (draw_helper.cpp:463-471); one ComputePipeline per
   integer radius, spec-consts (radius, border=shadowRes); reads the stencil as
   `usampler2DRect` (the r8ui-typed access producing the
   `Undefined-Value-StorageImage-FormatMismatch-ImageView` VUID — INTENT §11.24, ×10 at
   scene D), writes layer `idx` of `shadow` R8_UNORM array (context.cpp:68, layers =
   `maxShadowCast`).
5. **Layer cache** (DrawHelper::drawShadower, draw_helper.cpp:491-546): slots keyed by
   caster; reuse when `|Δradius_px| < 0.4` (SHADOW_RADIUS_TOLERANCE) AND light-direction
   cos² ≥ 0.998·norms (SHADOW_INVALIDATING_ANGLE); over-budget → ERROR log "shadow shapes
   are mostly unpredictible" + same-type slot steal. The blurred layer is caster-specific
   and SHARED across receivers; only receiver uniforms are per-receiver.
6. **Receiver uniforms** (BigBody::bindShadows, body_bigbody.cpp:635-652; moon/artificial
   analogs): `ShadowMatrix = mat3(lookAt·(model·zrot(axis_rotation)))`,
   `sinSunAngle = 2·sinSunHalfAngle`, per caster
   `posRadius /= initialRadius·(1+0.01·altimetryFactor)`, `idx`; refreshed EVERY frame
   (cheap) — only layer content is cached.
7. **Receiver sampling** (my_earth_shadow.frag:120-126, my_moon_shadow.frag:113-119,
   body_artificial_shadow_tex.frag:56-61): project the surface point by ShadowMatrix,
   `tmp = (shadowPos.xy − pos)/size`; inside the unit disc →
   `shadowing *= 1 − texture(bodyShadows, vec3(tmp·0.5+0.5, idx)).r` — SCALAR darkening,
   always toward black. Receiver arrays hardcode `shadowingBodies[4]` while
   `max_shadow_cast` defaults to 8 (checkConfig.cpp:147) — a silent cap mismatch.
8. **Self-shadow** (separate machinery, same generation): depth-only pass on
   `renderSelfShadow` into `shadowBuffer` D24S8 `self_shadow_resolution`² (default 4096,
   app.cpp:267-275), depth GREATER (bodyShader.cpp:410-426); consumed by 4-tap PCF
   `computeEnlightment` (selfShadow.glsl) for artificial bodies, and by the terrain
   ray-march inside my_earth/my_moon_shadow.frag (their own heightmap walk, :127-154).
9. **Pass ordering** (DrawHelper::submit, draw_helper.cpp:436-482): transfers → self-shadow
   pass → per-caster {stencil pass → FRAGMENT→COMPUTE barrier → blur → COMPUTE→FRAGMENT
   barrier} → color frame. All shadow GPU work precedes color, same command buffer.
10. **Startup cost**: `maxRadius = min(shadowRes/2, 512)−1` ComputePipelines (255 at default
    512, clamp 1280, res forced %256 — app.cpp:276-283) built by 4 detached threads
    (context.cpp:76-99); Gen-2 inert until `shadow_ready`.

### A3. Legacy limits inventory (what "as well or better" is measured against)

1. Gen-2 receivers = CoI + parent ONLY; arbitrary receiver pairs impossible.
2. `flag experimental_shadows toggle/on` is a no-op when the config default is false
   (XOR against the default — app_command_interface.cpp:1039-1050); runtime-immutable in
   practice.
3. Receiver shader cap 4 vs budget 8 (A2.7).
4. The D24S8-stencil-as-r8ui VUID class (A2.4).
5. "We can't self-shadow two body artificial" + parent-heuristic TODO
   (solarsystem_display.cpp:67-70).
6. Umbra color hardcoded to Earth's Moon; every other umbra is black (A1).
7. `ring_shadow` ssystem key parsed by nothing (default_ssystem.ini:482 — dead).
8. Eclipse dimming without atmosphere unimplemented (solarSystemModule.cpp:167-170 TODO).
9. body_normal.frag occluder-3 swizzle typo `.rbg` (harmless — only .r consumed).

---

## B. New path — one generalized system (G7) on the reserved primitives

Decisions shaping this design [vixy: 2026-07-12]: Gen-1 is REPLACED OUTRIGHT (no LUT in the
new path); the selection criterion is peak occlusion ≥ 1/16 per caster→receiver pair;
execution is synchronous-interim (shadow passes in the frame before color), with the
received-shadow state as the seam for the S4 move to the dedicated compute thread + VkQueue
(RenderChain.hpp thread model). Shadows are not always black: application is modulated by the
CASTER's `shadowAbsorbtion` per channel — Earth {0,1,1} absorbs G/B and not R, so its umbra
is red: the atmosphere-diffraction emulation, riding BMT_PROJECT_G1_SHADOW.

### B1. Primitive mapping (old mechanism → new home)

| Old | New | Notes |
|---|---|---|
| CoI nomination + selectShader shadowed variants | dissolved | every drawn body is a receiver candidate; ONE mesh family, always shadow-capable, 0 casters = loop never entered |
| computePreDraw cylinder scan + 4%-rule | `ModularSystem::updateSystem` selection | same math, observer-local frame (positions relative to light = `mat.getTranslation() − lightPosition`; dot/length frame-invariant); gate named as occlusion ≥ 1/16 |
| ShadowRenderData/UShadowingBody + bindShadows | `ShadowProjection` entries in `ModularBody::receivedShadows` | the "Received shadows" relation (ModularBody.hpp:106), realized |
| shadowData slots + DrawHelper::drawShadower cache | Renderer-owned layer pool | same keys: caster + radius ±0.4px + lightdir ≥0.998 (parameters); shared across receivers; THE S4 SEAM |
| Body::drawShadow(params) matrix math | orchestration computes shadowMat; module hook records geometry | `drawShadow(renderer, body, mat, idx)`: Renderer binds the SHADOW_STENCIL service family once per stream, hook pushes + draws (BasicMesh stub shape) |
| shadowShape/shadowTrace pipelines | SHADOW_STENCIL / SELF_SHADOW PassKinds | registry wiring live (PipelineRegistry.cpp:210-218); S5 validates state profiles against bodyShader.cpp:410-444 |
| shadow.comp pipeline bank (4 startup threads) | ONE COMPUTE family, EAGER_ASYNC_ALL, radius = integer variant key | registry COMPUTE support lands with S5 (open #4); interim builder thread = S1 precedent; not-yet-built radius → skip shadow this frame, log-once (C3) |
| tex_eclipse_map + MoonPosition1-4 LUT loop | RETIRED | Gen-2 covers all regimes; occluder fields become old-path-only in bodyShaderInterface.hpp |
| UmbraColor hardcode | `shadowAbsorbtion` per body (`shadow_color` param) | ctor drop FIXED with S5.3; multiplicative per-channel: `shadowing[ch] *= 1 − coverage·absorbtion[ch]` |
| shadowingBodies[4] hardcode | spec constant = caster budget | no cap mismatch by construction |
| experimental_shadows XOR quirk | plain toggle on a new-path kill-switch | default ON (G7 is core, not experimental); "off" = no shadows at all (LUT gone) — divergence vs old-off documented in C |
| MINOR_BODY (new concept) | excluded both directions | Renderer.hpp:69, D3 border case |

### B2. Per-frame flow (synchronous interim)

1. `updateSystem` (after body updates, light source known): for each drawn body
   (`isVisible & isBodyVisible`, non-MINOR_BODY, screenSize above the halo-only floor),
   run the light-cylinder scan over sorted system bodies restricted to bodies whose modules
   declare BMT_PROJECT_*; keep pairs with occlusion ≥ 1/16; rank by occlusion; budget
   overflow drops lowest + logs (old failsafe semantics, better selection).
2. For each kept pair: acquire a layer from the pool (cache hit → nothing to record;
   miss/invalidated → mark layer dirty with {caster, radius_px, lightDir, shadowMat});
   write the receiver's `ShadowProjection` entry {layer idx, ShadowMatrix, posRadius,
   sinSunAngle, absorbtion} — every frame, like old bindShadows.
3. Frame recording, before color (Renderer): for each dirty layer — SHADOW_STENCIL pass
   (service family bound once; caster module `drawShadow` hooks push + draw low-LOD) →
   barrier → blur dispatch (radius-keyed variant; offsets/pixelCount CPU-filled at
   assignment, old math) → barrier. Self-shadow pass for bodies with BMT_*_SELF_SHADOW
   (MAIN bucket = interest body by importance ranking, SECONDARY for others — ModularSystem
   sketch; first real client = OJM/terrain at their ports).
4. Color pass: the mesh fragment iterates `nbShadowingBodies` entries — old sampling math,
   absorbtion-colored application.

S4 relocation plan: steps 2-3's production side moves onto the compute thread/queue; the
layer pool and `ShadowProjection` entries are the handoff surface — consumers (step 4)
unchanged. This is why the cache object, not the pass recording, is the contract.

### B3. Buffers, formats, budgets

- Blurred layers: R8_UNORM 2D array, layers = caster budget (`max_shadow_cast`, default 8,
  D5 parameter), extent `shadow_resolution` (default 512, ≤1280, %256 — old constraints
  carried: SHADOW_LOCAL_SIZE=256 workgroup, offsets[511] ceiling).
- Silhouette target — the format decision closing the VUID class (A3.4), decided at
  implementation with validation layers as arbiter:
  - candidate A: R8_UNORM color attachment (own render pass; no aliasing; blur reads
    `sampler2D`/float — shader change);
  - candidate B: keep D24S8 stencil write + a legal stencil-aspect view sampled as
    `utexture2D` + samplerless texelFetch (SAMPLED_IMAGE descriptor, type-correct).
  **RESOLVED (2026-07-12, measured): candidate B.** Full eclipse scenes + regression under
  validation layers: ZERO messages from the new machinery (shadowBlur.comp / SHADOW_SHAPE /
  registry pools), while the old path's blur fired its known class ×10 in the same runs.
  Candidate A rejected with precondition: only needed if stencil-aspect sampling had stayed
  ill-defined — it did not; A would have added a render pass + pass-table divergence for
  no measured gain. Reopen if a target platform's stencil-texturing support breaks B.
- Self-shadow depth: MAIN_SELF_SHADOWING_RESOLUTION 8192 / SECONDARY 2048
  (ModularBody.hpp:92-93) as intended targets, `self_shadow_resolution` config key honored
  as override — D5 ladder, not fixed budgets.
- Precision note `[derived]`: old computed shadow geometry in heliocentric double; new is
  observer-local float. The shadow relation is caster-vs-receiver — the shared observer
  segment cancels (projection-paths.md C2 argument); residual error scales with the
  divergent segment, same class as the position parity already measured at float epsilon.

### B4. What carries each Gen-1 effect after retirement

| Gen-1 effect | Carrier in the new path |
|---|---|
| Solar-eclipse spot on a planet (satellite occluder) | G1 projected shadow, penumbra = blur radius (physically the same disc convolution the LUT tabulated) |
| Lunar eclipse darkening + red umbra | G1 projected shadow of the parent + its shadowAbsorbtion (Earth {0,1,1}) — generalized to ANY body via `shadow_color` |
| Ring→planet / planet→ring analytic shadows | RING port (row 4): G8 greyscale projection both directions; per-frame G8 budget = 10 simultaneous projections [vixy: 2026-07-12] |
| Eclipsed satellite halo dimming | already ported (ModularBody::drawHalo eclipse rule, halo.cpp:153-163 parity) |
| Ground-level solar-eclipse sky dimming | EnvironmentModule EnvironmentState (S8; the old TODO A3.8 gets its structural home there) |

### B5. Trait semantics (converged state)

- `BMT_BASIC_SELF_SHADOW`: depth-only self-shadow (old selfShadow.glsl class).
- `BMT_RGBA8_SELF_SHADOW`: colored self-shadow — client unresolved, converge at first client.
- `BMT_PROJECT_G1_SHADOW`: binary silhouette, disc-blurred to penumbra, applied through the
  caster's shadowAbsorbtion (NOT always black) [vixy: 2026-07-12].
- `BMT_PROJECT_G8_SHADOW`: greyscale transmission (alpha-graded casters — rings); up to 10
  simultaneous G8 projections per frame is the sizing budget [vixy: 2026-07-12].
- `BMT_PROJECT_BISHADOW`: bicolor projection, only valid when ~10× closer — exact semantics
  unresolved, no client; converge before first use.

---

## C. Differential — equal / better / accepted divergence

- **Equal (parity criteria)**: penumbra softness (same disc blur, same radius math); shadow
  placement (same lookAt/shadowMat composition, float-vs-double residual in the measured
  ulp class); pass ordering; layer cache behavior; Jupiter+Galilean scene vs old Gen-2.
- **Better (by construction)**: any drawn body receives shadows (A3.1 dissolved — two+
  simultaneous shadowed receivers impossible in old); umbra color data-driven per body
  (A3.6); no cap mismatch (A3.3, spec-const); no XOR quirk (A3.2); zero validation errors
  in the shadow machinery (A3.4 closed structurally); selection criterion named and exact
  (1/16), not paraphrased; MINOR_BODY mass-instancing exemption explicit.
- **Accepted divergences**: (1) `experimental_shadows off` → old shows LUT eclipses, new
  shows none (Gen-1 retired; flag-on is the shipping intent); (2) umbra rendering model
  changes from LUT-lookup+additive-floor to projected-coverage×absorbtion — perceptual
  match via `shadow_color` tuning is the criterion, not bit-identity; (3) terrain ray-march
  and artificial self-shadow arrive with their module ports (rows 2-3), not S5.

## E. S5 landing record (2026-07-12) — what shipped + measurements

Implementation homes (headers are the contracts): `ShadowService.hpp/.cpp` (layer pool,
production, recording window), `ShadowProjection.hpp` (received-shadow relation),
`ModularSystem::computeShadows` (selection), `bodyMesh.frag`/`shadowBlur.comp` (+ CPU
mirror `meshFrag`), registry COMPUTE banks (PipelineFamily.hpp), DrawHelper
`preFrameRecorder` hook, seams (app.cpp config init, plain-toggle flag command,
`applyHardcodedContent` Earth `{0,1,1}` default with explicit `shadow_color` precedence).

Measured (sandbox llvmpipe-class driver, 2048 render, config shadow_res 1280 / casters 8):
- **Lunar eclipse 2026-03-03 (jd 2461102.98), Moon tracked at fov 0.5** — new phase: umbra
  mean RGB [124, 3.6, 3.6] (max R 232) = the {0,1,1} absorbtion model; old phase:
  [60, 20.6, 4.2] (max [98,34,9]) = LUT × UmbraColor. Parity-matched absorbtion would be
  ≈ {0.58, 0.85, 0.96} (computed from the channel peaks) — tuning is data (D2 below).
- **Solar eclipse 2026-08-12 (jd 2461265.24), Earth from the Moon at fov 4** — alignment
  verified with the app's own ephemeris via dual_dump + the exact corridor test
  (miss/corridor 0.605 at minimum, occlusion gate passes, window jd 2461265.20–.32).
  New phase: blurred umbra spot over the Arctic. Old phase: its analytic LUT gradient only —
  **the old experimental (Gen-2) contribution is INVISIBLE on this driver, by its own A3.4
  defect** (undefined blur values read as zero; its dispatches fired ×10 in the log). The
  new path renders what the legacy generalized system fails to render here.
- **Profile fidelity (visual-fidelity mandate [vixy 2026-07-12])**: the first capture showed
  a too-wide flat-black core; analytic lens profile + an exact offline simulation of the
  blur algorithm both disagreed with the GPU output → inputs, not algorithm. Probe:
  `radiusPx = 0` — `star->getRadius() == 0` because **ModularSystem init sets star = the
  system itself and the `!star` assignment test was dead code: the Sun never became the
  star** (latent since ModularSystem existed; invisible until S5 consumed the radius; the
  light POSITION coincidentally matched, only the size was lost). Fixed (star == this is
  the unassigned state). After the fix, deterministic captures (see the path flag below):
  spot radial profile old `[5.5, 16.8, 24.7, 27.5, 31.3, 32.6, 36.5, 44.0, 50.7]` vs new
  `[9.1, 19.3, 23.1, 26.7, 30.1, 31.7, 35.1, 42.2, 48.9]` per 30-px annulus — within ~1–4
  units everywhere, both matching the analytic lens shape (LUT tabulation vs exact
  convolution + the D7 base-brightness delta account for the residual).
- **`flag experimental_path on/off/toggle` [vixy 2026-07-12]**: script-settable rendered-path
  selection (new/old), replacing the 1s A/B auto-toggle once used — deterministic
  single-path captures; retires phase-guessing (label-glyph/luminance clustering) for all
  remaining A/B work. Chain: AppCommandInterface → Core → SSystemFactory (pathPinned).
- **Flag cycle (reversible pair, 2nd entry)**: on→off→on — spot 4.9 → 75.8 (gone; and the
  new-phase eclipsed Moon goes full-bright [124,122,121] while old keeps its LUT umbra =
  the documented accepted divergence C1) → 4.9 (cache re-produces after invalidation).
- **Scenes A–D regression (fresh launch)**: P1 exact; P2 ≤ 7.9e-8 (visible bodies);
  P3 ≤ 1.8e-5 deg / ≤ 3.2e-7 rel; P4 10.8–55.5 km (the 1.4-ulp class); P5 view terms
  ≤ 0.0213° z-only ease tail, NO unmodeled component. Harness precondition made explicit:
  scenes are defined from a FRESH launch — a dirty session (leftover home_planet/fov/flag
  state from other scenarios) produces spurious P5 residuals (observed, reproduced clean).
- **Validation**: zero messages from all new machinery across every run; the pool-coverage
  defect found live (S1-era pool predating the blur contract's SAMPLED_IMAGE type) is
  closed structurally (per-pool type mask re-checked at every allocSet) — 1 → 0.
- Not staged live (visible-by-construction when hit, §11.22 precedent): pool exhaustion
  (>8 aligned casters — log-once + lowest-occlusion drop), blur-bank-still-building
  transient (acquire gate, silent by design), caster removal while holding a layer.

Design deltas found while porting (both are classes the old path avoided by frame choice):
- **Camera-independence**: the old path worked heliocentric; the new path only has
  eye-space. The sun-frame basis uses world-tied vectors (light axis × receiver spin axis)
  and the cache's light-direction key is CASTER-LOCAL — eye-frame keys would invalidate or
  silently rotate cached layers under camera motion (ShadowProjection.hpp header).
- **Units**: everything in observer-local AU (no initialRadius normalization) — the old
  normalization existed because its frames mixed unit systems.

## F. Composition-typed rework (2026-07-16) — every shadow composition type through one seam

Mandate: manage every shadow composition type and cast any composition onto any body
object. Analysis located the structural bottleneck at the PRODUCTION SEAM + CASTER
GRANULARITY: S5 hardwired "one opaque mesh per caster body" at three mutually-reinforcing
places — produce(idx, mat, ObjL*) had a one-word vocabulary; the D24S8 stencil target was
binary by construction (graded G8 transmission inexpressible); computeShadows excluded
caster==body, making BOTH flagship G8 directions (ring<->planet: ONE ModularBody under G1
composition) unreachable, and OR-ed module traits into a single layer per body (silently
wrong the moment mesh+ring coexist). This seam is also the S4 seam (jobs-as-data is the
compute-thread handoff), so reworking it after the RING/OJM ports would re-plumb the
handoff (I6). Receiver application needed NOTHING: (1 - cov·absorbtion) per entry carries
G8 unchanged — old ring mix(1.0, 0.3, a) == coverage=a x absorbtion {0.7,0.7,0.7} exactly.

Authoritative contracts (headers, I1): ShadowService.hpp (typed vocabulary + pool),
ShadowProjection.hpp (per-module entries, self-exclusion, clip half-space + derivation),
BodyModule.hpp (ShadowCaster descriptor, getShadowCaster hook, BMT_RECEIVE_SHADOW),
RingModule.hpp (first G8 client, caster half). Structure landed:

- **Layer = transmission silhouette of one (caster body, projecting MODULE)**. Products
  commute, so per-module layers applied as per-entry multiplications compose EXACTLY like
  one combined caster map — while making per-module absorbtion and within-body exclusion
  expressible. ShadowProjection gains `source` (self-exclusion key: a surface never
  samples a layer containing its own silhouette) and `clip` (below).
- **One production mechanism, typed words**: all silhouettes render into an R8 coverage
  COLOR target (context.shadowShape/renderShadowShape, PassKind renamed SHADOW_SHAPE),
  composited coverage-over (1-T1·T2), then the shared disc blur. Words: OPAQUE_MESH
  (shadow_trace.vert REUSED + shadow_shape.frag writing 1) and TEXTURED_ANNULUS
  (vertex-less quad, frag = radial ring alpha; SHADOW_RING family sharing the trace
  SetContract). BISHADOW = a future word (+ layer channel if bicolor requires it) —
  semantics stay suspended (D1); nothing else moves when it lands.
- **B3 addendum — the stencil retired**: candidate B's precondition space was G1-only.
  G8 forces a float target to exist, and the stencil was itself the fallback for
  unsupported R8-storage (app.cpp's own notes) — one float path now serves all types.
  The blur reads the R8 as float QUANTIZED to integer 0..255 (pixelCount carries the
  x255), keeping the sliding-window accumulator exact: G1 layers bit-identical to the
  stencil-era output, G8 at the layer's own 8-bit precision.
- **Selection module-granular + within-body pairs** (ModularSystem::computeShadows):
  casters = (body, module) with per-module ShadowCaster {radius, absorbtion, clip};
  receivers gated by BMT_RECEIVE_SHADOW (no dead fills); within-body pairs skip the
  corridor (always in it), smooth=0 (sharp — old analytic parity), rank FLT_MAX (never
  dropped), and are emitted only when another RECEIVE module exists on the body. G8
  budget = 10/frame [vixy] enforced here (occlusion-ordered drop + log-once).
- **The clip half-space (planar casters)**: the blurred layer is Z-LESS, so a ray
  crossing the ring plane BEHIND the surface it lights would still read coverage — false
  ring bands on the caster's summer hemisphere. Exact receiver-side rule (derivation in
  ShadowProjection.hpp, replaces body_ringed.frag:50's analytic dot-test): apply the
  entry iff dot(P, n_sunward) + w <= 0, n = ring normal toward the sun, w = -n·center;
  for the own-planet sphere no band can straddle (an annulus crossing inside the disc
  footprint satisfies x²+y²+z² >= inner² > R²); cross-body receivers keep the whole far
  ring correctly. Solid casters carry the degenerate plane (0,0,0,-1). Verified live:
  band on the winter hemisphere only (Iapetus scene).
- **RingModule caster half** (RingLoader, deduction gated on rings=true — old parse
  parity, the hint=false lesson): traits PROJECT_G8, outer-radius silhouette, absorbtion
  {0.7,0.7,0.7} = the old composition constant, plane clip from the spin axis;
  boundingRadius stays 0 until the COLOR port (a caster half has no drawn extent —
  setting outer radius would inflate Saturn's screenSize ~x2.3, the 11.26(2) class, to
  escalate WITH row 4 where 10.3(6) makes it mandatory). COLOR/TRACE/receive = row 4.

### F1. Landing record + measurements (2026-07-16; first verification on REAL GPU)

Environment note (load-bearing): this pass ran on an NVIDIA RTX 5090 — the first
hardware-GPU verification of the whole new path (every prior record: llvmpipe-class
sandbox). Two classes the software driver had masked surfaced immediately:

1. **Implicit-LOD sampling UB (fixed across all 9 receiver frags)**: `texture()` uses
   implicit derivatives, UNDEFINED in non-uniform control flow; the ray-march receiver's
   deeply divergent flow made NVIDIA read ZERO from the layers (the S5 umbra spot was
   invisible on this hardware — bisected via layered probes: selection/fill/gates all
   correct, textureLod read the coverage, texture() read 0). Fix: explicit
   `textureLod(..., 0)` everywhere (the layer is single-mip; defined in any flow). The
   mid-family receivers had the same UB latent — they merely happened to survive this
   driver's derivative handling.
2. **Old-path A3 inventory addition — severity upgrade**: the 11.26 fov-0.05 distant-CoI
   defect ("black disc" on llvmpipe) WEDGES the GPU on NVIDIA (frozen frame loop, app
   alive but frameless). Reproduced with shadows fully disabled; absent with the new path
   pinned from launch. Pre-existing, old-path-only, retires with it.

Also found and closed en route: `axisRotation` uninitialized (the 5.16 `distance`
sibling: heap-layout-dependent NaN into dual_dump JSON — surfaced as a predict.py
hard-stop); `layerViews` leaked 8 ImageViews at shutdown (createView returns RAW views;
the S5 "views die with the Texture" comment was wrong — vkDestroyDevice object-tracking
caught it on the first pinned-path graceful shutdown); disc-filter `sqrtf(negative)` NaN
when radiusPx < 1 (latent in the old drawShadower math, unreachable until within-body
smooth=0 — radiusPx now floored at 1 before keying).

Measured (2048², shadow_res 1280, casters 8, NVIDIA):
- **Lunar eclipse 2026-03-03** (fov 0.5, tracked): umbra old [54.7,16.1,0.3] vs new
  [48.3,14.2,0.2], peaks [91,27,3]/[91,28,3] — the D2 composition class (ratio ~0.88),
  G1 through the R8 target value-preserving. Within-phase captures bit-identical.
- **Solar eclipse 2026-08-12** (Earth from Moon, fov 4, ray-march receiver): umbra spot
  over the Arctic restored (100432 px on/off, radial profile 39.6 core -> 85 ambient =
  the lens shape); experimental_shadows off->on bit-restores (max 0).
- **G8 first light — Saturn ring shadow** (jd 2462654): from Titan (edge-on, crescent)
  378 px band; from 40000 km above Iapetus (inclined vantage): the graded band across
  the winter hemisphere, 14133 px, min transmission 0.335 ≈ the derived 0.30 floor
  (1 - 0.7·a_max), radial grading (Cassini-class structure) visible, old-phase band same
  geometry + grading (old base-lighting delta = the known row-2 class); NO false band on
  the summer hemisphere (clip verified); from Earth at fov 0.05 (new path pinned): band
  visible through the atmosphere. Flag cycle bit-restores; within-phase bit-identical.
- **Scenes A–D regression (final binary, fresh launch)**: P1 exact; P2 ≤ 9.5e-8; P3
  ≤ 1.5e-5 deg; P4 13.3–44.4 km = the 1.4-ulp class; P5 view ≤ 0.0208° z-only ease
  tail, all differentials named, NO unmodeled.
- **Validation**: zero messages from all new machinery (SHADOW_SHAPE/SHADOW_RING/blur/
  layer pool); the old path's Shadow-Stencil-Buffer VUID baseline unchanged (x7); zero
  leaked objects at graceful shutdown (was 8). Loader contract: RING resolves for
  Saturn/Uranus, no missing-loader warnings beyond the known OJM class.
- Not staged live (visible-by-construction when hit, 11.22 precedent): G8 budget
  overflow (>10 — log-once + drop), pool exhaustion with mixed kinds, caster-module
  removal while its layer is held.

## D. Convergence points (Vixy) + feeds

1. BISHADOW semantics + RGBA8_SELF client — before their first use (B5).
2. ~~Earth `shadow_color` tuning~~ **RESOLVED [vixy 2026-07-12: visual fidelity is the
   goal]**: the shipped default is DERIVED, not tuned — old `diffuse·(s + U·(1−s))` equals
   new `diffuse·(1−cov·a)` exactly under `a = 1−UmbraColor = {0.6, 0.88, 1.0}`, `cov = 1−s`
   — the old composition under a change of variable, penumbra-wide. Measured after: umbra
   means old [59.8, 20.6, 4.2] vs new [52.8, 18.1, 3.6], peaks [98,34,10] vs [100,35,9].
   "Better" = smoother penumbra + cases legacy never covered (arbitrary receivers, G8) —
   never a different look where legacy had one [vixy].
3. Flag semantics AS IMPLEMENTED: initialized from the `experimental_shadows` config key
   (A/B parity at init — the sandbox config ships true), plain toggle at the command (the
   XOR quirk not reproduced). Overrule here if different shipping semantics are wanted at
   switchover (with the LUT retired, flag-off = no eclipse rendering at all).
4. ~~Silhouette format decision~~ RESOLVED — B3 candidate B (measured), then SUPERSEDED
   by the F rework (2026-07-16): its preconditions shifted when G8 forced a float target
   to exist — one R8 color path now serves every composition word (F, B3 addendum);
   G1 layer content verified bit-preserving through the quantized-exact blur.
5. Feed to S3: self-shadow's grounded-slice-prefill dual purpose (D1) needs the depth
   partitioning consumer; stated, not blocked.
6. Feed to RING port: ~~G8 budget sizing (10) + both-direction projection contracts~~
   both landed with F (budget enforced in selection; ring->planet live, planet->ring
   entry already emitted — the ring COLOR draw consumes it at row 4). Remaining for the
   row-4 port: ring COLOR/TRACE + BMT_RECEIVE_SHADOW on the ring + boundingRadius
   escalation (F: the ~x2.3 screenSize coupling, decide WITH the color port).
7. Pre-existing, surfaced by the eclipse A/B (row-2 scope, not S5): the unshadowed Earth
   disc differs ~10% in brightness between paths (old my_earth layered lighting vs
   bodyMesh base lighting — E section, solar scene ref 104–107 vs 95.4).
8. (F, 2026-07-16) Per-ring shadow-color data key: the caster-half ships the derived
   old-parity constant {0.7,0.7,0.7}; a `ring_shadow_color` ssystem key (and whether the
   dead `ring_shadow` key of A3.7 should gate casting) is a data-model decision.
9. (F) Within-body penumbra: smooth = 0 (sharp, old-parity). Physically the ring shadow
   has a small penumbra (~sun angular radius x ring-to-surface distance); if visual
   fidelity ever wants it, the within-body pair needs a per-pair smooth estimate instead
   of the corridor formula's degenerate 0.
10. (F) The old path's fov-0.05 distant-CoI freeze on real GPUs (F1.2) — pre-existing
   A3 class, but it now BLOCKS old-path A/B verification of any zoomed-distant scene on
   this hardware; worth knowing before any further old-path-referenced measurement
   campaigns on NVIDIA.
