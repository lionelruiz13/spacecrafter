# B12 — The near-surface star family: DESIGN PASS

Task F9 (`claude/fable-dispatch.md` §1 F9), row **B12** (§13.B), unblocked by **Q21**.
Written 2026-07-31 by the F9 executor (Opus 5), against code `master-beta @ 1e44b639`
and harness `CC-harness @ cfb79d4`.

Precedent for the form: `claude/b31-design.md` (F6). Provenance grammar per INTENT.md
header: `[observed: file:line]` `[measured: instrument → value]` `[derived: argument]`
`[stated: …]` `[vixy: …]` `[assumed — flagged]`.

**Standing on**: §11.44 (the port that named the residual), §11.48(a) (Q21's record),
§2(a) (a divergence where old forecloses is a WIN, and must carry its mechanism),
§2.0 D4/D9/D10/D11/D12/D13, G1/G4/G6, I1–I6, and the old `StarViewer`/`BodyStar`
sources as the reference implementation (the standing rule: the old path's *shape*
encodes requirements written nowhere else).

---

## 0. What this note decides, and what it deliberately does not

**Decides** (and hence what a reader may rely on):
1. Where the near-surface star family engages, expressed against the G4 thresholds
   that already exist, and *which measurable settles that boundary* (§3).
2. The family's shape — which modules exist, what each owns, how they compose with
   `StarModule` and the module system, and why this is not a MESH shader-swap (§4).
3. The I4 home of each of Q21's four observables — limb darkening, granulation,
   spots, chromosphere glow (§5).
4. The alternatives rejected, each with the reason (§6).
5. The veto points I take (cheap to reverse, silence = endorsed) and the decisions
   that are NOT mine (§7).
6. The D11 budget claim the family must meet, with its denominator (§8).
7. The D9/D13 data story: what new product surface the family costs — **for the
   Phase-2 slice, none** (§9).

**Does not decide** (recorded, deliberately left open):
- The *content* of granulation and spots (procedural parameters, authored spot data)
  — the content slice, §5.2/§5.3, gated on a grammar word (§9.2).
- Whether a near-surface star is ENTERABLE (the `in` regime — what one sees inside a
  star). Recorded at §3.4, out of scope in every direction.
- The Q22 line: multi-star systems, per-instance halo colour/texture, "galaxy mode →
  stellar_system mode" navigation. That is row A7/A30 territory and stays suspended.

---

## 1. The defect, restated from the measurements

The Sun's disc in the new path is drawn by the generic MESH module (`BasicMesh`,
`bodyMesh.frag`), whose fragment is `color * min(NdotL⁺ + Ambient, 1)`
[observed: shaders/src/bodyMesh.frag:34-41]. The Sun is self-lit: the light source
*is* its own centre, so every visible surface normal faces away from it and
`NdotL < 0` ⇒ `diffuse = 0` ⇒ the disc is drawn at **ambient only**
[derived: bodyMesh.frag + ModularBody::getLightPosition].

Measured consequence [§11.44, path-pinned A/B, zoom fov 30, r0-20 ring medians]:
new disc median **21**, old **725** (sum-of-RGB scale, max 765). The outer halo
byte-matches (r20-40: 242/242, 143/142, 63/63) — i.e. the divergence is exactly the
disc, and exactly the lighting model.

The old path draws the same texture **unlit**: `FragColor = vec4(texture(mapTexture,
TexCoord).rgb, 1.0)` [observed: shaders/src/body_sun.frag:16-17], selected not from a
data key but from the C++ **class** (`Sun::createSunShader` → `myShader = SHADER_SUN`,
[observed: src/bodyModule/body_sun.cpp:194-…]).

Two facts that matter for the design and are easy to get wrong:

- **(a) `lighting = false` is a DEAD key in both paths.** The shipped Sun authors it
  [observed: ~/.spacecrafter/ssystem.ini:16], but a whole-tree grep finds only two
  *writes* and zero reads: `bodyParams["lighting"] = "false"`
  [observed: src/bodyModule/protosystem.cpp:105, src/bodyModule/ssystem_factory.cpp:389].
  So "honour `lighting=false`" is **not** old-path parity; the old path keys
  emissiveness on the body's TYPE. §11.44's phrase "the data `lighting=false` is
  unhonored" is true but must not be read as "the fix is to honour it": under §2(a2)
  that key is a *permission* gate (identity, forecloses) rather than a *parameter*
  (expands), so the correct home for the answer is the body's structural nature, which
  the engine already carries as the STAR bit (`BodyType::STAR`, "A body who emit
  light", tested by `isStar()`) [observed: src/experimentalModule/ModularBody.hpp:52,
  1355].
- **(b) The Sun's colour map carries NO baked limb darkening.** Measured over the
  shipped texture [measured: PIL over ~/.spacecrafter/textures/bodies/sun.jpg,
  8192×4096 → mean RGB (250.7, 195.9, 94.5); row means across the 10-sample ladder
  187.7 … 171.1, column means 179.7 … 181.3 — flat to ~4%]. It is an equirectangular
  *map* (surface coordinates), not a disc photograph, so applying a limb-darkening
  law to it is not double counting. This was checked before designing the law, because
  if it had been a disc photo the whole slice would have been wrong.

**Q21's answer** [§11.48(a); USER_QUESTIONS.md:164-166 verbatim]: *"Actual surface
appearance. But it could be glowing around like an atmosphere around a planet (with
red glow, if possible not uniform to match the prominences)."* — i.e. limb darkening,
granulation, spots, **plus** a separate non-uniform reddish shell. The row's own
conclusion follows from the observable, not from taste: four distinct observables, two
distinct geometries (a surface and a shell) ⇒ a family, not a flag on the planet mesh.

---

## 2. The reference implementation (old path), and what its shape encodes

Old `BodyStar` (`type = Star`, unreachable in shipped data) is the star near-surface
path that already exists, designed and never connected [Q22 confirms: *"A detailed
close-up view … is designed but not connected to anything yet"*]:

```
BodyStar::drawGL:  if (isVisibleOnScreen())  starViewer->draw(...)      // NEAR
                   else if (isVisible && tex_big_halo) drawBigHalo(...) // FAR
```
[observed: src/bodyModule/body_star.cpp:70-105]

`StarViewer::draw` records **two** draws [observed: src/inGalaxyModule/starViewer.cpp:120-151]:
1. `pipelineCorona` — a screen-space quad, BLEND_ADD, `big_star_halo.vert` +
   **`big_star_corona.frag`**: 4-D simplex-noise-offset radial falloff, animated by
   the global `time`, sized `4 × screen_size` around the body's screen position.
2. `pipeline` — the sphere mesh (`objl`), BLEND_NONE, cull on, **`big_star.frag`**:
   4-octave value noise as the base intensity, a two-simplex product as **sunspots**,
   and a `dot(cam_view, fPosition) * 0.5` term — a *limb* term (it vanishes at the
   limb and reaches −0.5 at disc centre, i.e. the old sketch **brightens** the limb;
   physically inverted, see §7 V1).

What that shape encodes, and what I therefore preserve:

- **R1 — the near-surface star is a SURFACE draw plus a SHELL draw, not one draw.**
  The corona quad is additive, screen-space, larger than the disc; the surface is an
  opaque depth-tested mesh. Two geometries, two blend modes ⇒ two modules (I4).
- **R2 — the near representation and the far halo are ALTERNATIVES.** Old switches
  between them (`if/else`). This is the "own regime" of the B12 row, in the old path's
  own hand.
- **R3 — granulation and spots are PROCEDURAL and TIME-ANIMATED.** The old sketch
  never intended a spot texture; it intended a noise field driven by `time` (available
  to every fragment shader through `cam_block.glsl`) [observed: shaders/include/cam_block.glsl].
  This is what makes the content slice cheap and the D4 texture-cap argument
  *inapplicable* to a star (see §3.3).
- **R4 — the star's own colour drives the surface**, not only the halo
  (`StarViewer(myColor->getHalo(), radius, objl)`, `uFrag->color`
  [observed: src/bodyModule/body_star.cpp:63]).

The reachable Sun (`class Sun`) differs from `BodyStar` in exactly one way that
matters here: it draws the halo **and** the disc every frame (no `else`)
[observed: src/bodyModule/body_sun.cpp:245-273], which is what the new path already
reproduces (`StarModule` far + MESH near, §11.44). §4.4 keeps that.

---

## 3. (i) The regime boundary vs the G4 thresholds

### 3.1 The thresholds that exist today

`ModularBody::draw` is the G4 substitution point [observed: ModularBody.hpp:508-555]:

**SUPERSEDED SPELLING, same boundaries [2026-07-31, F18/§11.127, §5.54]:** the literals
below were the shipped gates when this note was written; they are now DERIVED from px
constants (`BODY_EARLY_VISIBILITY_BOUNDING_SIZE` = 3.072 px, `BODY_DEPTH_BUCKET_...` =
8.192, `BODY_FULL_VISIBILITY_...` = 16.384, `BODY_CLOSE_RANGE_...` and
`BODY_BIG_TEXTURE_...` = 409.6, all bounding **diameters**) divided by the current
render width. At the 2048-wide render this note's measurements were taken on, the
derived values are BIT-IDENTICAL to the literals, so every number here stands; at any
other width the boundaries now sit at those pixel sizes instead of at these fractions.
Read the table's conditions as `earlyVisibilityGate()` / `fullVisibilityGate()` /
`closeRangeGate()`. Also §5.52: the depth-less mid band drew NO surface when this note
was written — fixed in §11.127, so the second row now draws what it says it draws.

| condition (screenSize = halfAngularSize / halfFov) | what draws |
|---|---|
| `screenSize ≤ 0.0015` | `farComponents` + `drawHalo` — **no surface at all** |
| `0.0015 < screenSize ≤ 0.008` | `far` + `near->drawNoDepth` + `drawHalo` |
| `0.008 < screenSize ≤ 0.2` | `far` + `clearDepth` + `near->draw` |
| `screenSize > 0.2`, `distance ≥ 2·scaledRadius` | `clearDepth` + `near->draw` |
| `screenSize > 0.2`, `distance < 2·scaledRadius` (`BODY_SURFACE_HEIGHT`) | `clearDepth` + `groundedComponents` |
| `screenSize > 0.2`, `distance < scaledRadius` | `clearDepth` + `inComponents` |

`screenSize · 2 · viewportRadius` is the on-screen **diameter in px**, and
`viewportRadius = screenWidth/2` [observed: ModularBody.cpp:639; the px identity is
used at StarModule.cpp:29, HintModule.cpp:58, RingModule.cpp:150, Renderer.cpp:51].
That identity is exactly what §5.54 turned from a fact repeated at seven call sites
into one conversion with one owner (`ModularBody::setViewportRadius`).

### 3.2 The load-bearing finding: EMISSIVE is not a regime, DETAIL is

A star's photosphere has (to the accuracy anyone can see) **distance-invariant surface
brightness**: radiance is conserved along a ray, so a resolved solar disc has the same
per-pixel brightness at 1 AU and at 3 R☉ — only its *size* changes [derived: radiative
transfer in vacuum, dI/ds = 0]. Therefore:

> **The emissive/limb-darkened base must draw in every regime where the star's disc is
> drawn at all — it cannot be gated on proximity.** What the near regime adds is
> *detail* (granulation, spots), i.e. structure whose angular size only becomes
> resolvable close up.

This is not a nicety: the recorded defect is measured at **1 AU** (zoom fov 30 from
Earth, §11.44) — a *distance-far, screen-near* configuration. A family gated on
`distance < 2·scaledRadius` would leave the recorded defect standing. Any design that
reads "near-surface" as a *distance* gate fails the row's own acceptance measurement.
[derived: §11.44's measured scene vs the table in §3.1 — at fov 30 from Earth,
`distance = 1 AU ≫ 2·R☉`, so the disc draws from `nearComponents`.]

Consequence for the "far-regime bit-inert" check: the inert regime is the
**halo-only** regime (`screenSize ≤ 0.0015`, the D3 common case — no surface module is
invoked at all), plus **every non-star body in every regime** (the family is
star-gated at the loader). It is NOT "the Sun at 1 AU": there the disc is *supposed*
to change, and that change IS the deliverable. §10.3 states the check in the form that
can actually fail.

### 3.3 Where the DETAIL boundary sits, and the measurable that settles it

D4's forcing arguments for module substitution are (a) VRAM and (b) the 32k texture
dimension cap. **Neither applies to a star**: the near representation is procedural
(R3), so it has no texture at all. What remains is the pure G4 criterion — *compute
only what the observer can distinguish* — which gives a measurable boundary:

> **Engage the detail representation when the smallest structure it adds subtends ≥ 1
> screen pixel.**

Applied to solar granulation: granule diameter ≈ 1 Mm against R☉ = 696 Mm
[derived from the shipped `radius = 696000.` km, ~/.spacecrafter/ssystem.ini:5; the
granule scale is a MODEL parameter of the procedural field, chosen by us, not a
fetched datum — see §7 V4]. A structure of relative size `g = s/R` is 1 px when

```
    discRadiusPx = screenSize · viewportRadius ≥ 1/g
    ⇒ screenSize ≥ 1 / (g · viewportRadius)
```

With `g = 1/696` and `viewportRadius = 1024` (a 2048-wide render): `screenSize ≥ 0.68`
— i.e. the disc is more than 2/3 of the half-FOV, comfortably inside the existing
`screenSize > 0.2` band and *close to* the header's own wording for the surface slot
("*shown instead of Outer BodyModule when the body is the nearest to be bigger than
the viewport*" [stated: ModularBody.hpp:231]).

So the family's detail boundary is **expressible in the existing vocabulary**, and it
is **resolution-relative** (it must be: D5 spans 1k–8k screens). The boundary is a
*derived* quantity `1/(g·viewportRadius)`, not a new magic constant — which is what
makes it defensible without a Vixy tuning pass (contrast A15's constant family).

### 3.4 What the family does NOT claim about `grounded` and `in`

`groundedComponents` (`distance < 2·scaledRadius`) and `inComponents`
(`distance < scaledRadius`) are meaningful for a star only if the operator can fly
there. Free descent stops at `groundRadius` (= `radius` by default), so `in` is
unreachable for the shipped Sun without an authored `ground_radius = 0`
[observed: ModularBody.hpp:126-140].

**The `grounded` band is empty for EVERY shipped body, not just for stars**: no
shipped module routes into `groundedComponents` [observed: the loaders' `add*Component`
calls; measured: `routing.grounded == 0` on the Sun in every dump this task took], so
between 1 and 2 body radii from a centre, a body large on screen draws *nothing at
all*. That is the standing G4 gap — "*Surface-segment drawing (near-ground
substitution): **Missing***" [INTENT §2.1 table] — and it belongs to D4's
surface-segment line, not to B12. The star family therefore:
- routes the photosphere into `near` **only**, which is byte-identical routing to
  today's `BasicMesh`, so the slice adds no unexercised path and no star-specific
  exception to a gap every body shares;
- inherits the `grounded` answer from whatever closes that gap generally;
- claims **nothing** about `in`. "What do you see inside a star" is a product question
  (§7 D2), recorded, not answered.

---

## 4. (ii) The family shape

### 4.1 Members

| module | slot / loader family | regime list | owns |
|---|---|---|---|
| `StarModule` (**exists**, §11.44) | `CUSTOM` (deduced; StarLoader bids 200) | `far` | the unresolved star: the additive screen-space big halo |
| **`PhotosphereModule`** (new) | `MESH` (deduced by the existing `tex_map` rule; a new `PhotosphereLoader` bids 200 on `isStar()`) | `near` (`grounded` when the general gap closes, §3.4) | the star's own emitted surface radiance: emissive map × limb darkening; later granulation + spots as a **variant axis** |
| **`ChromosphereModule`** (later) | its own family key (§9.2 — a grammar word, sign-off) | `near`, translucent | the non-uniform reddish shell above the limb (Q21's second half); the `big_star_corona.frag` lineage |

### 4.2 How the MESH is suppressed for stars — by COMPETITION, not by a gate

§11.44's sketch called for "MESH-suppression-for-stars". The design does better: it
needs **no suppression rule anywhere**. G6's competitive loader selection already is
the mechanism [observed: ModuleLoaderMgr::loadModule — highest `isLikely` wins,
ModuleLoaderMgr.cpp:74-97]:

- `deduceBodyModuleList` keeps emitting `MESH` for any body with `tex_map`
  — **unchanged** [observed: ModularBody.cpp:651-652];
- `BasicMeshLoader::isLikely` returns 16, `LayeredMeshLoader` outbids on layered
  texture keys, and the new `PhotosphereLoader::isLikely` returns 200 iff
  `target->isStar() && !params["tex_map"].empty()`, else 0;
- so the star's **MESH slot** is filled by the photosphere and the planet's by
  `BasicMesh` — one slot, one surface, no double disc, no `if (isStar())` on any draw
  path (I4: the *loader* answers "which surface type is this body's surface", which is
  precisely the question G6 exists to answer).

Zero change to `ModularBody::draw`, zero change to the deduction table, zero new gate
on a hot path.

### 4.3 Why this is a FAMILY and not "a MESH emissive shader-swap"

The row's exclusion is substantive, not cosmetic. A `VARIANT_EMISSIVE` bit on
`MeshFamilies::meshNormal()` would mean:
- the generic planet-surface type carries star behaviour it must then keep carrying
  (limb law, granulation field, spot state, chromosphere coupling) — I4 inverted:
  behaviour parked on the wrong type;
- the *shell* (chromosphere) has nowhere to live at all: it is not a variant of a
  surface, it is a second geometry with a different blend mode (R1);
- the near/far alternation (R2) would still need a rule somewhere outside the module.

The family answer puts each behaviour on the type that owns the observable, and uses
the *existing* composition machinery (slots, loader competition, regime lists, family
variant axes) for every join. Within the star family, a *detail* LOD is legitimately a
variant axis of the photosphere family (`VariantEffect::SHADER_SWAP`,
`VariantAxis::dropPriority`, base-always-resident ladder
[observed: PipelineFamily.hpp:100-125]) — that is a variant of *the star's own type*,
which is the opposite of parking star behaviour on the planet type.

### 4.4 Composition with the existing `StarModule`

Unchanged and untouched. The far halo keeps drawing in every regime the Sun spans
(the reachable `class Sun` behaviour, §2), so no alternation logic is introduced in
this design. The old `BodyStar` if/else (R2) is the *other* star's behaviour and stays
suspended with A7 — recorded here so that when a `type = Star` body becomes reachable,
the alternation question is already located (§7 D3).

### 4.5 The generality that must NOT be dropped in the port

`BasicMesh` carries four capabilities that the photosphere inherits the obligation for,
each with the requirement it serves — the standing rule for a parity port:

| capability | requirement it serves | disposition |
|---|---|---|
| **skin seam** (`createTexSkin`/`switchTexSkin`, `body name X skin_tex/skin_use`) | the shipped Sun's own data carries `#tex_skin = bodies/sun304A-sdo.jpg` [observed: ~/.spacecrafter/ssystem.ini:11] — swapping the Sun to an SDO 304Å image is a *demonstrated* operator use | **PRESERVED** |
| **big-texture (TEXMAP1) binding** at `screenSize > 0.2` | resolution near the surface — exactly the regime this family is about | **PRESERVED** |
| **`drawTrace`** (BMT_DEPTH_TRACE, orbit-hole prepass) | a star with an orbit (binary star, script-loaded) must occlude its own orbit line | **PRESERVED** |
| **`BMT_PROJECT_G1_SHADOW` / `BMT_RECEIVE_SHADOW`** | inter-body shadowing | **DROPPED — and provably inert**: the shadow orchestration already skips `body->isStar()` in the caster scan, the receiver scan and the self-shadow nomination [observed: ModularSystem.cpp:321, 375, 404]. The Sun's mesh has declared these traits since row 1 and has never once been selected. If a future decision un-exempts stars, the module declares them then. |

Preserving the first two forbids copy-pasting the binding-state machine (I2:
duplication = pending silent desync). The slice therefore lifts the *policy* half —
which texture is active, and when the descriptor must be rebound — into one shared
authority used by both mesh types, leaving each type its own family/set/uniforms
(§10.2).

---

## 5. (iii) The I4 home of each Q21 observable

I4 = behaviour belongs to the type. Q21 names four observables; they do not all belong
to the same type, and saying so is most of this design's value.

### 5.1 Limb darkening → the PHOTOSPHERE type (Phase 2)

Limb darkening is not decoration: it is the angular dependence of the emergent
intensity of the photosphere itself — the same physical object whose map is `tex_map`.
It is a function of one quantity the surface fragment already has (μ, the cosine
between the outward normal and the direction to the eye). Home: `PhotosphereModule`
and its fragment shader. Nothing else needs to know.

**The law, derived rather than recalled** (the §11.51(d) red line forbids physical
constants from memory; this law has *no* free constant to recall):

Plane-parallel grey atmosphere, radiative equilibrium, LTE (S = J), Eddington closure
K = J/3 ⇒ `S(τ) = (3F/4π)(τ + 2/3)`. Emergent intensity at μ = cos θ:

```
    I(0,μ) = ∫₀^∞ S(τ) e^{−τ/μ} dτ/μ
           = (3F/4π) · [ a·μ + b ]  with a=1, b=2/3
           since ∫₀^∞ τ e^{−τ/μ} dτ = μ² and ∫₀^∞ e^{−τ/μ} dτ = μ
           = (3F/4π)(μ + 2/3)
```

Normalised to disc centre (μ = 1):

> **`L(μ) = (μ + 2/3) / (5/3) = (3μ + 2)/5 = 0.4 + 0.6·μ`**

Every step is elementary and auditable; no measured coefficient enters. A *band-
specific* measured coefficient (the `1 − u(1−μ)` family) is a later refinement and
would require a cited fetch — recorded, not taken (§7 V1).

Predicted disc profile in the far limit (μ = √(1−x²), x = r/r_limb): 1.000 at centre,
0.920 at x = 0.5, 0.760 at x = 0.8, 0.587 at x = 0.95, **0.400 at the limb**. The
finite-distance form is exact in the shader (μ comes from the actual normal and eye
vector) and is computed exactly in the analyzer from d/R (§10.3).

### 5.2 Granulation → the PHOTOSPHERE type, as a DETAIL VARIANT (content slice)

Granulation is convective structure *of the photosphere surface*: same geometry, same
type, time-animated, procedural (R3). Home: the same module, engaged as a variant of
its own family under the §3.3 boundary. It is **not** a second module: it has no
geometry of its own and no state the surface does not already have.

### 5.3 Spots → the PHOTOSPHERE type by default; the *authored* case is a decision

Spots are photospheric features that rotate with the surface, so the type is the same.
Two candidate mechanisms, and they differ in product surface, not in rendering:
- **procedural** (the old sketch's two-simplex product, `big_star.frag`) — no data, no
  grammar, content-free: the default this design takes;
- **authored** (positions/sizes/lifetimes in data, e.g. to reproduce a real spot group
  for a show) — needs data keys ⇒ D9 product surface ⇒ B28 sign-off (§9.2).

Home for both is the photosphere type; the *choice* is §7 D1, recorded for the content
slice and NOT taken here (nothing in Phase 2 depends on it).

### 5.4 Chromosphere glow → ITS OWN type (content slice)

Q21: *"glowing around like an atmosphere around a planet (with red glow, if possible
not uniform to match the prominences)"*. That is:
- a different **geometry** (a shell/screen quad *larger* than the disc — the corona
  quad is `4 × screen_size` [observed: starViewer.cpp:140-147]),
- a different **blend mode** (additive over what is behind, vs the opaque surface),
- a different **regime membership** (translucent ⇒ ordered after opaque siblings by
  `addNearComponent`'s partition [observed: ModuleLoader.hpp:56-70]),
- and its own **state** (the noise field's animation, prominence coupling).

Four independent reasons ⇒ a distinct type (I4). It is *not* the existing ATMOSPHERE
module: `AtmExtModule` is the from-space scattering rim gated on `has_atmosphere` /
`atmosphere_ext_model` [observed: ModularBody.cpp:666-674] — a star's chromosphere is
self-emission, not scattering of another body's light, and reusing that type would put
two physics on one type for the sake of a slot.

**Note for the content slice**: deduced modules land in their *type's default slot*
[observed: ModuleLoaderMgr.cpp:88-92], so the chromosphere cannot be a second
`CUSTOM` module — `StarModule` already holds the Sun's `CUSTOM` slot. It needs either
a new `BodyModuleType` word or an explicit named slot. Both are product surface
(§9.2).

---

## 6. (iv) Rejected alternatives, with reasons

**A1 — `VARIANT_EMISSIVE` on `MeshFamilies::meshNormal()` (the shader-swap).**
Rejected: §4.3. Also the row's explicit exclusion [§13.B B12, Q21-derived].

**A2 — honour the `lighting = false` data key.** Rejected: the key is dead in both
paths (§1(a)); it is an *identity/permission* key under §2(a2) (gates a capability
⇒ forecloses); and it would make the star's appearance depend on an authoring flag
that the 28-body shipped corpus sets on exactly one body. The STAR bit already carries
"emits light" as a *capability* [observed: ModularBody.hpp:52]. **Retiring the key is
not proposed either** — D9 keeps every in-field key parseable; it simply stays unread,
as it is today.

**A3 — extend `StarModule` with a near-surface draw.** Rejected on three counts:
(i) §11.44's own conclusion ("one module cannot cleanly be both the far halo AND the
near surface"); (ii) `StarModule` sits in the `CUSTOM` slot routed to `farComponents`
— a module in one regime list cannot replace the MESH in another without the body
learning about star-ness; (iii) the MESH slot would still draw its lit disc underneath.

**A4 — a distance-gated family (`distance < k·radius`).** Rejected: it does not retire
the recorded defect, which is measured at 1 AU (§3.2). This is the alternative most
likely to be reached for by someone reading "near-surface" literally, which is why it
is recorded rather than merely avoided.

**A5 — port `StarViewer` wholesale as the near representation.** Rejected *for this
slice*, kept as the content lineage: its surface shader has no texture input at all
(pure noise), so it would *drop* `tex_map` — and the shipped Sun's authored map (and
the operator's SDO skin, §4.5) is content the family must keep able to show. The right
composition is texture **×** limb law **×** procedural detail, with the detail arriving
as a variant. Its corona half is directly the chromosphere lineage (§5.4).

**A6 — fix it in the old path.** Excluded by §11.52(b) and by the F9 stop boundary:
old is the comparison baseline, and its dark disc is not even the defect — its disc is
the *reference* the new path currently fails to reach.

**A7 — a new `BodyType` bit (e.g. `EMISSIVE_SURFACE`).** Rejected: §6.3's standing
test — `BodyType` carries what modules cannot express; "has an emissive surface" is
exactly what a module expresses. `isStar()` already answers the loader's question.

---

## 7. (v) Veto points and decisions that are not mine

**Veto points** — cheap-to-reverse decisions I take under the standing protocol
(recorded to the ledger; silence = endorsed):

- **V1 — the limb-darkening law is the Eddington grey-atmosphere law `0.4 + 0.6μ`,
  centre-normalised.** Reversal cost: one line in one fragment shader. Two sub-choices
  and their reasons: (a) *derived, not fitted* — no recalled coefficient enters, which
  keeps the §11.51(d) red line intact; (b) *centre-normalised* — at μ = 1 the disc
  reproduces the OLD path's value exactly (`texture × 1.0` = `body_sun.frag`), so the
  change is monotone (never brighter than the authored texture) and the disc centre is
  a *parity anchor* rather than a new value. A flux-normalised alternative would
  brighten the centre above the author's texture value with nothing to check it
  against.
- **V2 — the star's surface type is selected by `isStar()`, at the loader.** Reversal
  cost: one predicate. (Alternative homes — a data key, a `BodyType` bit — are A2/A7.)
- **V3 — `BMT_PROJECT_G1_SHADOW` / `BMT_RECEIVE_SHADOW` are not declared by the
  photosphere.** Provably inert today (§4.5); reversal cost: two bits.
- **V4 — the granule scale used to *derive* the detail boundary (1 Mm / R) is a MODEL
  parameter of the procedural field, not a measured solar datum.** It sizes a
  threshold, not an appearance. Reversal: one constant, one recomputation.

**Not mine — recorded, and Phase 2 does not depend on any of them:**

- **D1 — authored vs procedural spots** (§5.3). Authored ⇒ new data keys ⇒ B28
  sign-off. Content slice.
- **D2 — is a star enterable, and what is seen inside?** (`inComponents`, §3.4.)
- **D3 — does a resolved star's big halo alternate with its surface** (the old
  `BodyStar` if/else, R2) or coexist (the reachable `Sun` behaviour)? Only becomes
  observable when a `type = Star` body is reachable (A7/A30 territory).
- **D4 — the chromosphere's grammar word** (§9.2): a new `BodyModuleType` value is a
  new `module =` value in the composed format, i.e. product surface.

None of D1–D4 blocks the minimal slice, so this dispatch does **not** stop at the
design (contrast the F6 precedent, where a decision genuinely gated the work).

---

## 8. (vi) The D11 story — the budget claim, on the 1 ms denominator

The denominator is **1 ms/frame** for the engine's work [§2.0 D11], never 16.7 ms.

**Claim.** The Phase-2 photosphere costs, per drawn star:
- **CPU**: the same one pipeline bind + one set bind + one uniform fill + one indexed
  sphere draw as `BasicMesh` (the module is structurally the same recording), minus
  `fillPlainShadows`'s caster loop (never populated for a star, §4.5) — i.e.
  **≤ `BasicMesh`'s cost**, not above it.
- **GPU per fragment**: one `normalize` + one `dot` + one `fma` on top of the texture
  fetch, and **no** additional texture fetch, **no** additional pass, **no** additional
  draw. It *removes* the shadow-sampling branch of `bodyMesh.frag`
  [observed: bodyMesh.frag:35-40].
- **In the halo-only regime**: exactly zero — the module is not invoked (§3.1 row 1).

**Bound.** The claim to verify is therefore *no measurable regression*: the frame-cost
delta between the pre-change and post-change binaries, in both regimes, is within the
measurement noise of the instrument, and in any case ≪ 1 ms. A new *pipeline family* is
a one-time allocation in the registration domain (D11 is a per-frame budget; family
allocation is a cold cost by the PipelineFamily paradigm
[stated: PipelineFamily.hpp:20-31]).

**Budget statement for the future content slice** (recorded now, since the row asks a
family to state its budget claim): the detail variant adds 4-octave value noise + two
simplex evaluations per fragment, engaged **only** above `screenSize ≈ 0.68`
(§3.3) — i.e. exactly when at most one body covers the screen and the rest of the
system is gated out by G4. Its budget case is "one full-screen body, nothing else",
which is the cheapest possible frame for every other subsystem. If that variant ever
fails to hold the budget, the ladder is already in the design: `dropPriority` drops
the detail bit and the base variant draws (base-always-resident, C3).

---

## 9. (vii) The D9 / D13 data story

### 9.1 Phase 2: no new product surface at all

- **No new data key.** The photosphere reads `tex_map` (existing), the STAR bit
  (existing, from legacy `type = Sun|Star` or composed `light_source = true`), the
  body's colour (existing). The limb-darkening law has no authored parameter.
- **No new `module =` value.** The photosphere fills the existing `MESH` slot, and
  `BodyModule::getType()` returns `MESH`, so the composed twin emits `module = MESH`
  exactly as today [observed: ModuleLoaderMgr::moduleTypeName / the B24 grammar note,
  ModuleLoaderMgr.hpp:33-37].
- **Twin round-trip is stable by construction**: `generateComposedTwin` emits
  `light_source = true` for a star [observed: ModularSystem.cpp:1732-1733], so
  re-loading a twin re-runs the same loader competition and re-selects the photosphere.
  (This is the failure mode a naive implementation *would* hit: had star-ness been read
  from the legacy `type` string instead of the STAR bit, every composed twin's Sun
  would have silently reverted to the dark lit disc. The twins battery is the
  discriminator — §10.3.)
- **D13 (downgrade)**: nothing is written anywhere; an older build reading the same
  data gets its old behaviour. Neutral by construction.
- **D12 (acting defaults)**: the photosphere selection is *not* an acting default in
  D12's sense — it does not cause behaviour the author did not write; it renders the
  body the author declared as a light source as a light source. No new log line is
  owed. (The B28 loader diagnostics are untouched.)

### 9.2 What the content slice WILL cost, recorded now for sign-off planning

- a `BodyModuleType` value for the chromosphere ⇒ a new `module =` word (D9: *"grammar
  and key spellings are PRODUCT SURFACE"*), B28 protocol, Vixy sign-off;
- optionally, authored-spot keys (§7 D1) ⇒ same;
- both are additive and forecloses nothing (§2(a2) test: they declare a *capability*),
  so they are the "expands" side of the test — but the *spelling* still needs sign-off.

---

## 10. Phase 2 — the minimal slice, and how it can fail

### 10.1 Scope

**In**: a `PhotosphereModule` + its pipeline family + its shader pair + a
`PhotosphereLoader` registered under `MESH`, routed to `nearComponents` exactly where
`BasicMesh` routes today; emissive texture × `L(μ)`; skin seam, big-texture binding and
`drawTrace` preserved.
**Out**: granulation, spots, chromosphere (content, §5.2–5.4); `grounded`/`in` routing
(§3.4); anything touching the old path; any new data key or grammar word.

### 10.2 The I2 obligation

The colour-binding *policy* shared with `BasicMesh` (which texture is active — map,
skin, or big-texture mapping — and when the descriptor must be rebound) is lifted to
one authority used by both types; each type keeps its own family, set and uniform
block. No copy of the state machine exists after the slice.

### 10.3 Discriminating checks — each stated so it CAN fail, predictions first

1. **The disc is no longer dark.** Scene: the §11.44 configuration (Earth reference,
   `zoom fov 30`, Sun centred), fresh launch. *Predicted before the run*: with the big
   halo suppressed (`body name Sun color halo value 0,0,0` — the halo colour feeds
   `StarModule` live [observed: StarModule.hpp:31-33]), the pre-change disc is
   `texture × ambient` and the post-change disc is `texture × L(μ)`; at the disc centre
   the post-change value must equal the OLD path's value **to within JPEG/mesh-LOD
   noise**, because `L(1) = 1` and `body_sun.frag` writes the texel unmodified. Fails
   if the ratio to old at centre is not ≈ 1.
2. **Limb darkening is measurable radially.** Same capture. *Predicted before the run*:
   the ring-median ratio new/old equals `L(μ(x))` with μ computed exactly from the
   scene's d/R — 1.000, 0.920, 0.760, 0.587, 0.400 at x = 0, 0.5, 0.8, 0.95, 1. The
   colour target is `VK_FORMAT_B8G8R8A8_UNORM` [observed: src/appModule/app.cpp:323],
   so the ratio is the law itself, un-encoded. Fails if the profile is flat (no law
   applied) or if it lands on a different curve.
3. **A true near-surface approach**: `set home_planet Sun` + free-mode descent (the
   scene-E recipe [observed: harness/scene_e_spine.py:74-83]) to a disc that fills the
   viewport — the disc is bright and limb-darkened there too, and the app survives the
   approach twice (rare-path discipline).
4. **Star-gated inertness**: on the standard scenes, every changed pixel between the
   pre-change and post-change binaries lies inside the Sun's disc; every non-star body
   and the halo-only regime are byte-identical. Fails if any planet pixel moves.
5. **Battery**: `b24_equivalence` (+RED leg), `b25_galactic`, `b40_parity`,
   `b4_anchors`, scene E spine; twins 18/18 (the §9.1 twin-stability argument is
   exactly what this checks).
6. **D11**: frame cost measured in both regimes against the pre-change binary.

---

## 11. Out-of-scope findings recorded here (not fixed by F9)

- **F9-a — two spellings of the G4 threshold family, one dead.**
  `BODY_EARLY_VISIBILITY_BOUNDING_SIZE = 2` and `BODY_FULL_VISIBILITY_BOUNDING_SIZE = 16`
  are declared in px [observed: ModularBody.hpp:193-196] and have **zero consumers**
  tree-wide [measured: grep over src/ → declarations only]; `ModularBody::draw` gates
  on the raw literals `0.0015 / 0.008 / 0.2` [observed: ModularBody.hpp:510-540]. At
  `viewportRadius = 1024` the literals reproduce the named px values almost exactly
  (0.0015 → 3.1 px, 0.008 → 16.4 px), so the literals *are* those constants frozen at a
  2048-wide render — but since `viewportRadius = screenWidth/2`
  [observed: ModularBody.cpp:639], the shipped gates are resolution-INDEPENDENT
  fractions, i.e. at 8k the "16 px" gate is 65 px. Two authorities for one threshold
  family (I2), one of them dead, and a D5 (1k–8k) scaling question underneath. Not
  touched by F9: changing any of those literals changes every body's regime in every
  scene.
- **F9-b — `lighting` is a dead data key** in both paths (§1(a)). Recorded so no
  future slice "fixes" it into life without noticing it is a §2(a2) permission gate.
