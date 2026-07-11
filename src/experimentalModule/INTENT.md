# experimentalModule — Refactoring intent, target state, and gap analysis

Provenance method: every claim is tagged.
- `[stated: file:line]` — explicit in source (comment, name, or strategy.txt).
- `[observed: file:line]` — code fact, verified by reading.
- `[inferred]` — reconstruction of intent from converging evidence; **needs validation by Vixy** — if an inferred goal is wrong, everything derived from it below must be re-derived.
- `[vixy: 2026-07-11]` — provided directly by Vixy (review of this document's first version); highest-authority intent source.
- `[defect]` — found during analysis; listed in §5 regardless of comfort.
- `[open]` / `[resolved]` — decision state; resolved entries keep their answer visible (traceability), they are not deleted.

Document status: `experimentalModule` is halfway between a draft and a specification [vixy: 2026-07-11]. Headers are Vixy's formalization medium — the closest representation of what is wanted [vixy: 2026-07-11]; therefore ModularBody.hpp's comment blocks (relations, shadow projection, depth buffers, threads — lines 95-137) are normative intent, and implementation-in-header is partly a drafting artifact, not only a G10 choice. This file is the consolidated extraction + gap/decision tracker; on divergence between it and a header, neither silently wins — reconcile explicitly.

Comparison baseline: old path = `src/bodyModule` (`Body`, `ProtoSystem`, `SolarSystem*`), new path = `src/experimentalModule`. Both are alive simultaneously by design (§1.3).

---

## 1. Current situation

### 1.1 What the old path is (the problem being refactored away)

- `Body` is a god class: ~700-line header, 9 friend classes, and one hardcoded member per feature — `trail`, `hints`, `axis`, `planetGrid`, `orbitPlot`, `halo`, `atmExt` [observed: bodyModule/body.hpp:653-659]. Every body carries every feature slot whether applicable or not; adding a feature means modifying `Body` itself.
- Behavior specialization by inheritance: `body_bigbody`, `body_moon`, `body_smallbody`, `body_star`, `body_sun`, `body_artificial`, `body_center` [observed: bodyModule/ directory]. Nature is encoded in the class taxonomy, so feature combinations (ringed star, artificial with atmosphere…) require new classes or flags — the structure must deform to accept new cases.
- Rendering plumbing lives inside each body: per-body command buffers `cmds[3]`, `drawState`, `virtual selectShader()=0` [observed: body.hpp:379,596-599]. Astronomy, rendering, UI (hints/labels/translation), and selection (ObjectBase) are one class.
- System-level behavior is split into aspect managers each iterating all bodies (`solarsystem_color/display/scale/selected/tex`) [observed: bodyModule/ directory] — behavior far from the data it depends on (violates I2/I4 of programmation-principles).
- Fixed two-level hierarchy: `parent` + `satellites`, `is_satellite` flag [observed: body.hpp:614-615,628]. No arbitrary nesting, no multiple systems as first-class citizens; special cases (Earth, Moon, Sun) spread through the code.
- Lifetime: `shared_ptr` web + `enable_shared_from_this` [observed: body.hpp:116].

### 1.2 What the new path is

Files: `ModularBody` (tree node + orchestrator), `BodyModule` (feature unit), `ModularSystem` (system root, itself a `ModularBody`), `ModuleLoader`/`ModuleLoaderMgr` + `modules.cpp` (data-driven instantiation), `ModularBodyPtr` (safe long-lived reference), `Camera` (observer with reference-body semantics), `Renderer` (sole Vulkan owner), `AsyncHub` (tick hub), `EnvironmentModule` (stub, scope now defined — §3.8), `ModularObject` (bridge to old `ObjectBase` UI), plus concrete modules in `bodyModules/`, `meshModules/`, `orbitModules/`. Exactly one `BodyModule` family implemented so far: MESH via `BasicMesh` [vixy: 2026-07-11; observed: meshModules/].

### 1.3 The migration strategy itself (dual path)

- `SSystemFactory` holds **both** paths and updates both every frame [observed: ssystem_factory.cpp:393-395]; `drawModularSystem` selects which one draws [observed: ssystem_factory.cpp:381-388] and auto-toggles every 1000 ms [observed: ssystem_factory.cpp:397-401].
- Purpose: **visual comparison** of the two paths — the old path is the reference, the alternation makes divergence directly visible to the eye [vixy: 2026-07-11].
- Active investigation (uncommitted): Moon position divergence. Old path prints `WANT:` [observed: git diff, body.cpp:995-999], new path prints `GOT:` in `preUpdate` for `EARTH_MOON` [observed: ModularBody.hpp:200-204], plus a creation print [observed: ModularSystem.cpp:278] and removal of the `#ifndef NDEBUG` guard around the toggle [observed: git diff ssystem_factory.cpp]. Investigation artifacts, not design; must not survive into a commit (the `preUpdate` print sits in a hot inline path in a header).

---

## 2. Extracted intent

### 2.0 Domain constraints that force the design [vixy: 2026-07-11]

These are facts about the problem space, prior to any architecture. Most of the structure below is their consequence; each carries its propagation edges.

- **D1 — Child body size is generally negligible relative to parent body size.**
  → per-body depth slices instead of one global depth range (§3.6; `Renderer::clearDepth(zCenter, boundingRadius)` [observed: Renderer.hpp:18-19]);
  → self-shadow drawing doubles as depth-prefill of grounded bodies' slices with the parent body — a grounded body cannot occlude correctly against a parent whose scale dwarfs its whole depth slice (§3.1; matches "parent's depth trace is drawn in each DepthBuffer" [stated: ModularBody.hpp:123]).
- **D2 — Body size is generally negligible relative to the distance between a parent and its children.**
  → same depth-slicing consequence as D1: a depth range covering parent-child distance leaves no precision for the body itself;
  → `[inferred]` together with D1, forces the tree of *local* coordinate frames and observer-relative matrices (G2/G3): float32 global coordinates cannot hold AU-scale distances and meter-scale surface detail simultaneously. **Needs validation.**
- **D3 — There can easily be thousands of bodies, but very few are actually visible at a time (the rest halo-only), often exactly one.**
  → halo-only is the *common case*, full drawing the exception — G4's gating is the normal mode, not an optimization edge (§ constants ModularBody.hpp:80-93; halo fallback ModularBody.hpp:308-312);
  → depth buckets are assigned only to the few bodies needing one (`notableBody`, §3.6);
  → frame-coherent O(n) resort is viable because the sorted set is mostly stable [stated: ModularSystem.cpp:70-72].
  Border case: an upscaled asteroid ring makes *many* small bodies visible simultaneously — this is what `MINOR_BODY` exists for: mass-instanced, simple bodies that do **not** cast shadows between each other (in reality too small for that to be observable) [vixy: 2026-07-11] → connects `BodyType::MINOR_BODY` [observed: ModularBody.hpp:37] to the replication/cluster traits (`MBT_REPLICATED`, `MBT_CLUSTER` [observed: ModularBody.hpp:47-51], `BMT_REPLICATED` [observed: BodyModule.hpp:46]) and partially answers §6.3.
- **D4 — A whole-body texture at surface-level resolution does not fit in VRAM.**
  → approaching the surface requires **module substitution**, not just parameter scaling: a different `BodyModule` takes over drawing, switching from whole-body texture to visible-surface-segment-only, activating whenever the difference becomes visible and relevant [vixy: 2026-07-11];
  → this is the reason the regime lists (far/near/grounded/in) exist as *substitution* points and not only as work gates (G4), and why "Surface BodyModule (shown instead of Outer BodyModule…)" is a distinct relation [stated: ModularBody.hpp:104];
  → surface-segment streaming is the most demanding client of G5's async loading (segments must appear as the camera moves, without stalls).
- **D5 — Hardware spans roughly an order of magnitude in every dimension: 4–64 cores, 2–32 GiB VRAM, 1k–8k screens** [vixy: 2026-07-11].
  → no fixed budgets or thread counts anywhere: pool sizes, VRAM budget, LoD caps are parameters with degradation ladders;
  → at the 4-core floor, a preempted lock-holder can stall the render thread for a scheduling quantum → render-thread boundaries must be lock-free, not merely briefly-locked (§8).
- **D6 — Resource asymmetry** [vixy: 2026-07-11]: textures are the hard case — loaded *and* unloaded dynamically by what is load-bearing now/soon, 2–3 LoD levels, lowest always resident, highest by relevance or anticipation. Models are VRAM-cheap: build-once (memcpy from native-form cache when available; building from source is slow), no eviction pressure.
  → residency management is a texture problem; models need only the build/handoff path;
  → the LoD ladder means handoff is "upgrade/downgrade between resident levels", never "appear from nothing" — together with `drawLoaded`, something drawable is always resident.
- **D7 — Completions are rare but bursty, never sustained; resources are heavily shared** [vixy: 2026-07-11]: a script can load thousands of bodies of which few are visible; hundreds of small completions/second at 8k is possible as a peak, never as a steady state. Some textures and most models are heavily shared across bodies.
  → the handoff must absorb bursts without per-frame cost in the idle case (lock-free chains grow unbounded; drain budget as safety valve, never the sizing case);
  → the unit of loading is the **resource**, not the (body, module) pair: request deduplication + completion fan-out to all attached bodies is mandatory, refcounted lifetime included (`AsyncBuilder::useCount` is the existing pattern).

### 2.1 Goals

Each goal states the evidence that forces it.

**G1 — Composition over inheritance: all per-feature variability lives in `BodyModule` instances; `ModularBody` never knows feature specifics.**
Evidence: abstract `BodyModule` interface with draw/shadow/trace hooks [observed: BodyModule.hpp:58-103]; `ModularBody` holds anonymous component lists, no feature members [observed: ModularBody.hpp:689-697]; no `ModularBody` subclass except `ModularSystem` (a structural, not feature, specialization) [observed: ModularSystem.hpp:7]. Residual hardcoding is quarantined and named as such (`applyHardcodedContent`, `BodyType::EARTH/EARTH_MOON` "Bodies with hard-coded specificities") [stated: ModularBody.hpp:42-44, ModularSystem.cpp:272-280].

**G2 — One uniform, arbitrarily deep hierarchy: galaxy ⊃ systems ⊃ bodies ⊃ moons ⊃ surface objects, with systems as bodies.**
Evidence: `ModularSystem : public ModularBody` [observed: ModularSystem.hpp:7]; grounded/orbiting/inner/hidden child lists [observed: ModularBody.hpp:680-683]; `milkyway` as root system holding solar systems [observed: ssystem_factory.hpp:593, ssystem_factory.cpp:273]; `boundToSurface` bodies (surface objects in the same tree) [observed: ModularBody.hpp:745, ModularSystem.cpp:198-199]. ← D1+D2 `[inferred]`: local frames per node are what keeps float32 viable across scales.

**G3 — Seamless navigation across scales: the observer holds a reference body, switches reference by area of influence, without visual discontinuity.**
Evidence: `Camera::switchToBody` "without moving" [stated: Camera.hpp:20-23]; `calculateSwitchCompensation` "for a seamless change of body reference" [stated: ModularBody.hpp:439-440]; `findBetterReference` + `areaOfInfluence` [observed: ModularBody.hpp:492-500,728].

**G4 — Visibility- and distance-driven work gating *and module substitution*: compute only what the observer can distinguish, and draw each regime with the representation built for it.**
Evidence (gating): cheap `preUpdate` visibility classification before any full update; invisible subtrees only get their translation refreshed [observed: ModularBody.hpp:200-242,391-402]; explicit pixel-size and distance thresholds as named constants [stated: ModularBody.hpp:80-93]; draw degrades to halo-only below thresholds [observed: ModularBody.hpp:275-314] — the common case, per D3.
Evidence (substitution): far/near/grounded/in component lists [stated: ModularBody.hpp:694-697]; surface-segment module replacing whole-body module near the ground ← D4 [vixy: 2026-07-11].

**G5 — Asynchronous, prioritized resource lifecycle: the render thread never waits on loading.**
The core design problem here — mostly *searched*, not yet solved — is **how to transmit completed loads from the loading thread to the render thread without data race and with minimal overhead** [vixy: 2026-07-11].
Evidence: priority ladder LAZY/BACKGROUND/PRELOAD/ACTIVE [stated: strategy.txt:1-5, ResourcePriority ModularBody.hpp:139-146 — naming unified, legacy `NOW` renamed, verified in tree 2026-07-11]; `isLoaded`/`preload` on modules and `drawLoaded` degraded-draw path [observed: BodyModule.hpp, ModularBody.hpp:272-273,306-307]; `MAX_LAZY_TEXTURE_SIZE` [stated: ModularBody.hpp:90-91]; thread inventory [stated: ModularBody.hpp:131-137]; `AsyncHub` "used to reduce constraints" [stated: AsyncHub.hpp:7].
The handoff sketch in strategy.txt ("Taskable: ModularBody" + "ModularBody drawing: Update position / **Execute tasks**" [stated: strategy.txt:7-13]) — loader threads post completion-tasks to the body, render thread executes them at draw — reading **confirmed** [vixy: 2026-07-11]: it was the best strategy found at the time, explicitly not claimed best possible. Superseded by the §8 proposal (global drain replaces at-draw execution, closing the D3 starvation channel). ← D4: surface-segment streaming is the client that makes this load-bearing.
Constraints for the rework [vixy: 2026-07-11]: **C1** no race between loading-side changes to a body and drawing; **C2** no destruction of a body while a task on it is pending; **C3** draw thread never blocks; **C4** loading thread may block.

**G6 — Data-driven instantiation: which modules a body gets is deduced from its data, loaders compete, registration is centralized.**
Evidence: `deduceBodyModuleList` from params [observed: ModularBody.cpp:301-312]; `isLikely` competitive selection, highest bidder wins [stated+observed: ModuleLoader.hpp:10-11, ModuleLoaderMgr.cpp:46-64]; all registration in one place, `modules.cpp` [stated: ModuleLoaderMgr.hpp:17].

**G7 — Generalized shadow system replacing special-cased eclipse handling.**
Evidence: shadow trait bits [observed: BodyModule.hpp:45-56]; the four drawing types (§3.1) [vixy: 2026-07-11]; per-body `shadowAbsorbtion` (Earth's `{0,1,1}`) [stated: ModularBody.hpp:117-118]; shadow-projection and depth-buffer strategy [stated: ModularBody.hpp:120-129]; self-shadow resolutions [observed: ModularBody.hpp:92-93]. Old path: moon-specific `tex_eclipse_map`, `sun_half_angle` [observed: body.hpp:637,640]. Scope boundary ← D3: `MINOR_BODY` mass-instanced bodies are exempt from inter-body shadowing [vixy: 2026-07-11].

**G8 — Safe non-owning references without a shared_ptr web.**
Evidence: `ModularBodyPtr` registry with `redirect(from, to)` on body removal, "prefer raw pointer when short-lived" [stated: ModularBodyPtr.hpp:9-10,39]. Invariant I5 (reference-without-ownership legal because the target notifies/redirects on destruction), replacing `enable_shared_from_this`.

**G9 — Centralized rendering: `Renderer` is the only Vulkan surface in the module; bodies and modules describe, the renderer executes.**
Evidence: `Renderer` owns command buffers, depth management, halo drawing [observed: Renderer.hpp]; no Vulkan member in `ModularBody`; modules receive `Renderer&` [observed: BodyModule.hpp:73]. Extended responsibility ← D1/D2/D3: depth-range partitioning across visible bodies (§3.6).

**G10 — Performance as a structural driver, not an afterthought.**
Evidence: frame-coherent O(n) resort with documented rationale [stated: ModularSystem.cpp:70-72] ← D3; `casify` string switch [observed: ModularSystem.cpp:11-15]; `findBody` last-hit cache with a cache-bypass variant for one-shot searches [stated: ModularBody.hpp:515-522]; `StringID` interning over string keys [observed: ModularBody.hpp:623]; hot paths inlined in headers [observed: ModularBody.hpp draw/preUpdate/update].

**G11 — Old-path compatibility during migration, not after.**
Evidence: `ModularObject : ObjectBase` bridge [observed: ModularObject.hpp]; "Compatibility methods" on Camera [stated: Camera.hpp:130]; deprecated fields carried and explicitly marked in the new structs [stated: ModularBody.hpp:74-77,705,730].

---

## 3. Target state — how it should be (header-level contracts)

Headers are the specification (programmation-principles I1) — doubly so here, since headers are the medium in which this design is being thought [vixy: 2026-07-11]. Each class below is defined by what it promises and what it must not know. Anything a caller needs that is not stated here is a defect of this spec, to be fixed here first.

### 3.1 `BodyModule` — the unit of feature composition
- Contract: a drawable/loadable feature attached to a body. Knows *how* to draw itself in a given regime; never *when* — regime selection belongs to `ModularBody` (I4).
- **Four drawing types** [vixy: 2026-07-11]:
  1. **Color** (`draw`, with `drawNoDepth` as its minimalist depth-less variant for small screen sizes) — the visible image.
  2. **Self-shadow** (`drawSelfShadow`) — dual purpose: (a) fill the self-shadow depth buffer; (b) prefill the depth-buffer slice of each grounded body with the parent body ← D1 (the grounded body's whole depth slice is negligible at parent scale; without prefill, parent-vs-grounded occlusion is wrong).
  3. **Shadow** (`drawShadow`) — stencil map of the shadow this body projects onto *other* bodies.
  4. **Trace** (`drawTrace`) — depth-like pass that cuts a hole where the orbit line must be hidden by the body (the new-path analog of old `drawOrbit(cmdBodyDepth,…)`).
- Lifecycle: `isLoaded()` is a non-blocking query; `preload()` is a hint, never a stall; `update()` returns true when it no longer needs updates (self-deregistration semantics); `draw*()` must be callable with partial resources only via the `drawLoaded` path.
- Ordering guarantee (now stated in the header, 2026-07-11): `update()` must have run before `compare()`/`getBoundingRadius()` are meaningful — `boundingRadius` is undefined until then.
- Trait bits declare rendering-pipeline needs (depth, shadows, batching); the renderer reads traits, modules never touch the pipeline directly.

### 3.2 `ModularBody` — tree node and orchestrator
- Owns: identity (name), tree position (parent/children by relation), transforms (orbit + rotation elements), cached observer-relative state (mat, distance, screenSize, visibility flags), component ownership (`components` by slot) and routing (four regime lists).
- Must not know: any concrete module type, any Vulkan object, any UI concept.
- Update pipeline (per frame): `selectiveUpdate` → `transformParentToBodyPos` → `preUpdate` (visibility classification, cheap — the common exit, per D3) → full `update` + `recursiveUpdate` only if visible.
- Position updating is to become **iterative across frames** [vixy: 2026-07-11]: one refinement iteration per frame, reusing the previous frame's position — visually equivalent to full per-frame convergence (the eye tracks fast movers with less precision than slow ones; fast movers lose precision exactly where it isn't seen). This is what keeps the synchronous update part cheap enough to live on the main loop's cadence tick (§8.2.2).
- Draw pipeline: regime selection by screenSize/distance thresholds (§ constants ModularBody.hpp:80-93) → dispatch to the appropriate component list (substitution point, per D4) → halo fallback below full-visibility threshold (common case, per D3).
- Slot model: `components[StringID]` is the **ownership** home (unique per slot, replacement erases the old module's routing) [observed: ModularBody.hpp:611-622]. The four regime lists are **routing** only (raw pointers, no ownership).
- Loading contract (order is load-bearing): `ModuleLoader::load()` routes the new module into regime lists via `add*Component`; `ModuleLoaderMgr::loadModule` then installs ownership into the slot; `slot()` erases only the *replaced* module's routing [observed: ModuleLoaderMgr.cpp:59-64, BasicMeshLoader.cpp:46]. Now stated in ModuleLoader.hpp/ModularBody.hpp headers (2026-07-11) per I1.

### 3.3 `ModularSystem` — coordinate and sorting root
- A `ModularBody` with `isNotIsolated = false` (update/draw not transmitted upward) [observed: ModularSystem.cpp:44, ModularBody.hpp:739].
- Owns: the distance-sorted flat body list of its subtree (frame-coherent resort ← D3), the star (light source) designation, and body loading from `ssystem.ini`-format data.
- Hardcoded content is only ever introduced through `applyHardcodedContent` and only when the data says `hardcoded=true` [observed: ModularSystem.cpp:195-196].

### 3.4 `ModuleLoader` / `ModuleLoaderMgr` / `modules.cpp`
- Loaders compete via `isLikely` (0 = cannot, 255 = certain, first max wins) [observed: ModuleLoaderMgr.cpp:48-57]; a loader can identify its own products (`isLoaderOf`) for future reload/invalidations.
- All loader registration lives in `modules.cpp` exclusively — adding a module family never touches core files.
- Orbit loading mirrors the same pattern keyed by `coord_func` string with a default fallback [observed: ModuleLoaderMgr.hpp:20-28].

### 3.5 `Camera`
- Owns the observer state (reference body, view rotator, free/surface modes) and the reference-switch logic; consumes `findBetterReference`/`calculateSwitchCompensation` from bodies.
- Is the entry point for the new-path frame: `update(jd, dt)` then `draw(renderer)` drives system update, sort, and draw [observed: ssystem_factory.cpp:381-395].
- Its position relative to the current body also selects the active environment (§3.8).

### 3.6 `Renderer`
- Sole owner of command buffers, depth-buffer strategy (the four buffers stated at ModularBody.hpp:125-129), and tone-mapped halo emission. Everything else describes; only the renderer records.
- **Depth-range partitioning** ← D1/D2/D3 [vixy: 2026-07-11]: the full depth range must be *split between visible bodies according to their needs*, because no single range can hold parent-scale distances and child-scale details (D1/D2), and only a handful of bodies need a bucket at any time (D3). `notableBody` is the designed input: the per-frame list of bodies large enough on screen to need a depth bucket [stated: ModularBody.hpp:756; vixy: 2026-07-11]. The consumer is unwritten (§5.3); when written, it must also drain/clear the list each frame.

### 3.7 `ModularBodyPtr` / `ModularBodySelector`
- Long-lived references must use `ModularBodyPtr` (registry + `redirect` on removal); short-lived ones use raw pointers within a frame [stated: ModularBodyPtr.hpp:9-10]. Selection side-effects (pointer count, isSelected) go through the friend classes only.

### 3.8 `EnvironmentModule`
- Scope [vixy: 2026-07-11]: **everything "outside" whose rendering depends on the camera's location relative to the body** — milkyway (earth-centered 2D version vs 3D version), atmosphere, landscape, and so on.
- Structure already reserved for it: grounded environment takes priority over InAoI environment [stated: ModularBody.hpp:107-108]; `enterEnvironment()`/`leaveEnvironment()` are the (currently empty) transition hooks [observed: ModularBody.hpp:576-579].
- Note the propagation: the milkyway 2D/3D switch being an environment concern means "backdrop vs world" is a *camera-relative state*, not a property of the object itself — consistent with G2 (the galaxy is a body like any other; only its rendering mode depends on where you stand).
- Convergence updates [vixy: 2026-07-11, in-tree annotations in EnvironmentModule.hpp + plan review]: visibility gating (the old BodyDecor questions) is **aggregated by the system**; simultaneous environments draw in **fixed per-category order** (landscape, milkyway 2D/3D, atmosphere-from-ground, …) — a frame-sequencing input to the Renderer, distinct from the batch occlusion contract (§10.3 decision 6).

### 3.9 Header hygiene targets (applies module-wide)
- Every contract currently discoverable only from implementations gets written into the header owning it (I1). Done 2026-07-11: loader routing order (§3.2), `update`-before-`compare` (§3.1), `findBody` miss-is-exceptional (§5.6). Still pending: `halfFov` must-set-before-update [stated but easy to miss: ModularBody.hpp:525-526].
- `TEXMAP*`/`TEX` macros are texture utilities, not body concepts — wrong home in ModularBody.hpp [observed: ModularBody.hpp:19-30]; move next to `s_texture`/big-texture machinery (I2).
- Debug I/O has no place in headers (currently in `preUpdate`).

---

## 4. Gap analysis — current vs target

| Area | Target (goal) | State | Evidence |
|---|---|---|---|
| Body tree + transforms | G2 | Working, under live visual A/B validation | ModularBody.cpp/hpp, active Moon divergence |
| Visibility gating | G4 | Implemented, halo fallback partially broken | strategy.txt "fix halo" |
| Mesh drawing | G1 | One family only: `BasicMesh` (MESH) [vixy] | meshModules/, bodyModules/ |
| Surface-segment drawing (near-ground substitution) | G4/D4 | **Missing** — whole-body only; no segment module, no VRAM-bounded streaming | [vixy: 2026-07-11]; relation reserved at ModularBody.hpp:104 |
| Ring/Hint/Pointer/Orbit/Trail/Axis/Tail/Atmosphere modules | G1 | **Missing** (deduce handles only tex_ring/tex_map/model_name; only MESH loader registered) | ModularBody.cpp:301-312, modules.cpp:40 |
| OJM/RING load path | G6 | Missing loader now logs a warning (Phase B); registration lands with the module implementations | ModuleLoaderMgr.cpp loadModule |
| Shadows: 4 drawing types | G7 | Hooks + traits + doc; no implementation, no stencil/self-shadow buffers in Renderer | BodyModule.hpp defaults empty, Renderer.hpp |
| Depth-range partitioning | G9/D1-D3 | `clearDepth` exists; bucket-splitting consumer of `notableBody` unwritten | Renderer.hpp:18-19, §5.3 |
| Environment (milkyway 2D/3D, atmosphere, landscape) | §3.8 | Empty stub; scope now defined | EnvironmentModule.hpp |
| Async loading: cross-thread handoff | G5 | Strategy proposed (§8, pending convergence); implementation absent | [vixy: 2026-07-11], §8 |
| Selection/pointer draw | G11 | Missing | strategy.txt "draw pointer when isSelected" |
| Labels/fonts | G11 | Broken ("s_font not shown") | strategy.txt |
| Old-path parity | §1.3 | Moon position divergence under investigation | uncommitted WANT/GOT prints |
| Slot/component mgmt | §3.2 | Mid-rework; loading contract now documented | commits 128412c9, 4dfe7bb3 |

---

## 5. Defects — status tracked, none silently dropped

1. **`BMT_BASIC_SELF_SHADOW == BMT_RGBA8_SELF_SHADOW` bit collision** — **FIXED** by Vixy (renumbered 0x20/0x40/…/0x200); verified in tree 2026-07-11 [observed: BodyModule.hpp:51-56].
2. **`altitudeRelativeToRadius = Utility::isFalse("solid")`** — **OPEN**. Tests the literal string `"solid"`, not `param["solid"]`; constant-folds to `false` for every body, contradicting the member default `true` [observed: ModularSystem.cpp:188 vs ModularBody.hpp:746]. Off the Moon-investigation critical path; queued.
3. **`notableBody` grows unboundedly** — **DRAIN RESOLVED (D4, 2026-07-11); consumer pending**. Purpose: input to the Renderer's depth-range splitting (§3.6) [vixy]. Lifecycle now: cleared at `dispatchUpdate` start (every frame, whichever path draws — a draw-side-only drain was unbounded during old-path toggle phases), filled by update(), read by the Renderer via `drainNotableBodies()` between updates. The bucket-partitioning math itself remains second-pass.
4. **Debug artifacts in hot/committed paths** — **OPEN (deliberate, temporary)**. `std::cout` in inline `preUpdate` (header), in `Body::computeDraw`, in `applyHardcodedContent`; `#ifndef NDEBUG` removed from the auto-toggle [observed: git diff]. Instruments of the active investigation; must be reverted or gated before commit.
5. **Hardcoding keyed on `englishName`** — **OPEN (acknowledged)**. Any body named "Moon"/"Earth" with `hardcoded=true` becomes EARTH/EARTH_MOON; TODO exists [stated: ModularObject.cpp:68].
6. **`findBody` exception on miss** — **RECLASSIFIED: not a defect — undocumented expectation (now documented)**. Misses are exceptional *by design*: callers look up names they expect to exist; the exception fires only for absent bodies [vixy: 2026-07-11]. Header comment added 2026-07-11 per I1. Residual nit: `catch (...)` could be `catch (std::out_of_range)` for precision; equivalent in practice here.
7. **`DedicatedBodyModuleSlot` is dead** — **RESOLVED** (removed, Phase B 2026-07-11; StringID slots are the slot identity, BodyModuleType the loader-family selector — both documented).
8. **`BodyModule::loaded`** — **RESOLVED** (removed from the base, Phase B 2026-07-11: module-internal state, moved into BasicMesh; `isLoaded()` is the contract).
9. **`boundingRadius` read-before-write hazard** — **RESOLVED** (ordering contract + zero-initialization, Phase B 2026-07-11).

---

## 6. Decisions — open and resolved (resolved entries keep their answer)

1. **Priority ladder naming** — **RESOLVED** [vixy: 2026-07-11]: `ResourcePriority` naming wins; legacy `NOW` renamed to `ACTIVE`; strategy.txt updated (verified in tree).
2. **Inline-in-header policy** — **OPEN, context added**: headers are the thinking medium [vixy: 2026-07-11], so implementation-in-header is partly drafting state; G10 justifies keeping *hot* paths inline. Still to decide: which inlines are load-bearing (stay) vs drafting residue (migrate to .cpp once parity is proven).
3. **`BodyType` residual role** — **PARTIALLY RESOLVED**: `MINOR_BODY` is justified — mass-instanced small bodies, exempt from inter-body shadows, cluster-optimizable ← D3 [vixy: 2026-07-11]. Still open: SPHERICAL_BODY / SINGLE_BODY / CUSTOM_BODY — pre-G1 remnants or load-bearing classes (e.g. oblateness fast path [stated: ModularBody.hpp:68])?
4. **Taskable / cross-thread handoff** — **PROPOSAL v2 (§8, 2026-07-11), pending Vixy convergence**. Constraints C1–C4 + ordering + burst/sharing (D7) recorded. v2 = synthesis of Vixy's in-progress `Taskable` primitive (strict per-body ordering, marker-exchange continuation) with v1's ready-list drain (starvation-freedom, render affinity, O(1) idle cost).
5. **EnvironmentModule scope** — **RESOLVED** [vixy: 2026-07-11]: everything camera-location-relative "outside" — milkyway 2D(earth-centered)/3D, atmosphere, landscape, etc. (§3.8). Open remainder: the module's interface (the class is still empty).
6. **Milkyway root lifetime** — **OPEN**: "Never destroyed, there is no parent to delegate remnant ModularBodyPtr to" [stated: ssystem_factory.hpp:593] — rule of the model or temporary simplification?
7. **`deduceBodyModuleList` coverage** — **RESOLVED** [vixy, plan phase 2026-07-11]: deduction extended per ported family (existing .ini files keep working) PLUS explicit module/slot declaration params for overrides and customs; param syntax lands with the first multi-module body (D2). Contract documented on `loadModule`.

---

## 7. Suggested next steps (ordered by information gain, not by ease)

1. Close the Moon divergence (active investigation) — it validates/invalidates the transform pipeline, which everything else sits on.
2. Converge on §8 (handoff strategy proposal, 2026-07-11) — G5 is load-bearing for D4 (surface streaming) and the largest unimplemented goal. The task-at-draw reading was validated and superseded the same day; §8 is the successor.
3. Fix §5.2 (`isFalse("solid")`) as soon as it's off the investigation's critical path — silent-wrong-result class.
4. Write the `notableBody` consumer (depth-range splitting in Renderer, §3.6) — its design is now specified enough to build, and multi-visible-body correctness depends on it; include the per-frame drain.
5. Port module families in visibility order of user impact: font/labels, pointer, halo fix (strategy.txt TODOs) — each port also stress-tests the loader contract (§3.2) before it ossifies.
6. Surface-segment module (D4) after G5's handoff exists — it is the client that will prove the async design honest.

---

## 8. Threading / handoff strategy — PROPOSAL v2 (2026-07-11, pending Vixy convergence)

Successor to the strategy.txt task-at-draw sketch (confirmed best-found-at-the-time [vixy: 2026-07-11]). v1 proposed a single global completion queue; the EntityCore/Executor inventory (§8.6) found that Vixy's in-progress `Taskable` primitive [observed: EntityCore/Executor/Taskable.hpp, written 2026-07-11] provides strict per-body ordering [vixy: 2026-07-11 — named requirement] and a *better* resolution of the MPSC transient-window than v1's defer-retry. v2 is the synthesis. Once converged and implemented, the authoritative home of each contract becomes the header of the component owning it; this section then reduces to a pointer.

### 8.1 Constraints it must satisfy
C1 no loading↔drawing race · C2 no body destruction with pending tasks · C3 draw thread never blocks · C4 loader threads may block · strict per-body task ordering · burst absorption without idle-case cost (D7) · shared-resource dedup + fan-out (D7) · minimal overhead · D5 hardware ranges · D6 texture/model asymmetry (all [vixy: 2026-07-11]).

### 8.2 Shape: per-body Taskable chains + global ready-list, boundary-localized lock-freedom
Lock-free machinery exists **only** where the render thread meets producers; loader-side internals may block (C4 — INT-2: complexity lives where the binding boundary is).

1. **The render chain is an "effective thread"** [vixy: 2026-07-11]: draw-visible state is protected by one render-side `Taskable`; everything touching that state runs as a Task on its chain. Sequential consistency comes from the Taskable ordering promise, **not from thread identity** — rendering itself may occasionally execute on a worker or events thread, and that is correct by design. C1 = chain serialization; per-body publish ordering is automatic (a single FIFO chain preserves per-key order). The chain **may and should go idle** — see the stack note in 4.
2. **Main loop** [vixy: 2026-07-11, corrected 2nd pass]: a pure cadencer at the target framerate — `execute()`s the **frame task** onto the render chain (inplace when idle, lowest latency; chained behind pending publishes otherwise) and waits until the next tick. **Update+draw live inside the frame task itself** — one Task, scheduled/executed every frame [vixy]: the residual synchronous update part is cheap, cache-local, and uses the exact elapsed time in non-recording mode.
3. **Two protection domains** `[inferred — effectively confirmed by the fence discipline below]`: work tasks/chains do the hard work and the async sequencing (completion events: the chain waits, no thread waits) and **never touch draw-visible state**; their final step schedules a lightweight subscribe/finalize/publish task onto the render chain [vixy: 2026-07-11 — "tasks on the Taskable for rendering only subscribe/finalize the render-thread-side part, not doing hard work"]. **Fence discipline** [vixy: 2026-07-11, 2nd pass]: fence waits happen as late as possible, only where the touched resource requires them, and **only the rendering part ever waits on a fence** — with multiple frames in flight, the acquire/present semaphore structure makes an actual block genuinely unlikely; the fence is the guarantee, not the expected wait. The separation restated with this: long holds belong to the work domain; the render chain carries only late, rarely-blocking guarantees. Starvation-free: publish execution is independent of whether the body is drawn (closes the D3 channel of the at-draw sketch).
4. **Cost profile**: idle = zero — no drain point exists; the render chain sleeps between frame tasks. Per completion = the work chain's exchanges + one publish push. Bursts chain behind the frame task and execute between frames as O(N) lightweight publishes (heavy work — decode, build, upload — already done in the work domain; a publish is a pointer/descriptor swap, O(small) by contract). **Stack note** [vixy: 2026-07-11]: synchronous `endTask` cascades nest on the stack (unless tail-call optimization applies), so chains going idle is *preferable* — it resets the depth; bounded bursts (D7) are the sizing assumption.
5. **Lifetime (C2) under the granularity decision** [vixy: 2026-07-11, 2nd pass]: synchronization goes through a **single Taskable** — the render chain — likely sufficient granularity, and cheaply revisable if it turns out wrong, thanks to how Task/Taskable compose [vixy]. **All Taskables live for the whole application duration** → `TASKABLE_REFERENCE_COUNT`/`TASKABLE_TASK_KEEP_REFERENCE` stay off. Two consequences:
   - **Pins need no atomics at all**: pin increments (request emission, inside the frame task) and decrements (publish/finalization tasks) both execute within the effective thread — a plain integer under chain serialization.
   - **The hand-off rule stops being discipline and becomes forced**: with a non-atomic pin, worker code *cannot* legally release — the only path is transferring the hold into the publish task. The earlier `[inferred]` rule is now enforced by the type, not by convention.
   `remove()` on a pinned body detaches from the tree immediately (world-consistent, ModularBodyPtr::redirect philosophy); memory follows when the pin reaches zero inside the chain. Reopening trigger (membership, not magnitude): sustained publish volume contending with frame-task latency → split to per-body chains — KEEP_REFERENCE returns, and §8.6(f) must be fixed first.
   - Pin counter deliberately **not** merged with render-local `pointerCount` (different synchronization domains; merging forces atomics onto the render-local path — I2: one info, one domain).
   - Module replacement during pending load needs no pin: integration validates `body->slot(id) == module` at drain — render-thread-local, race-free by construction, drops stale payloads.
6. **Requests, priorities, cancellation**: `AsyncBuilder`-style atomic priority field is the existing pattern ("prioritize on activation" = one atomic store); workers re-read at pop and re-check before expensive phases (drops uploads for bodies gone halo-only). Cancelled tasks still schedule a payload-less completion for the unpin. The `LoadPriority` fused state+priority encoding (one atomic carries DONE/COMPLETED/LOADING/LAZY→NOW) is kept — one-word lifecycle — but needs a header comment and the NOW→ACTIVE rename for consistency.
7. **Shared resources (D7)**: the loadable unit is a refcounted resource object (`AsyncBuilder::useCount` pattern exists). Dedup by resource identity at request time (render-side registry; s_texture already dedups by name — verify overlap at plan time). On completion, fan-out = schedule one integration task per attached (body, module) — each push lock-free, per-body order preserved. Attachment-list synchronization: either a small loader-side mutex (legal, C4) or the resource itself as a `Taskable` in `execute` mode (attachments serialized as tasks, no lock) — plan-mode choice.
8. **Texture residency (D6)**: `gc.hpp` is the existing eviction machinery — cycle-based grace (`preservationCycles`) is the hysteresis; `used()` at draw is the relevance mark. Eviction decisions stay render-side; VkImage destruction deferred by frames-in-flight. VRAM budget = D5 parameter with degradation ladder. Models: same pipeline, no eviction path (D6). `gc` cross-thread sync has a known open point (`deletable` non-atomic, flagged in-source) — verify at plan time.
9. **Task semantics — the contract** [vixy: 2026-07-11]: strict ordering applies **only inside `[start(), endTask())`**; a task's `start()` returning does *not* mean the task is done — `endTask` being called on it does; a task must hold nothing with side effects past its `endTask`; outside the window, no guarantee. Tasks/Taskables compose (a Task may hold another Task which calls the enclosing task's `endTask`).
   → **Asynchronous chain holds**: a task may start async work (GPU transfer, fence) and call `endTask` later from wherever completion fires — the *chain* waits, no *thread* waits. This is the natural carrier for the upload→integrate sequence with ordering preserved.
   → ~~Render-hop pattern~~ **SUPERSEDED same day** by the effective-thread model (§8.2.1/3): no thread affinity exists to restore — cross-thread continuation is correct because protection is per-chain, not per-thread; the "hop" is simply the work chain's natural final step of publishing onto the render chain. Kept as a record of the wrong frame (affinity-thinking) it came from.
10. **Thread model** [vixy: 2026-07-11, extended 2nd pass] (concretizes the draft inventory at ModularBody.hpp:131-137): 1 video-player thread (video frame loading) · 1 main thread as cadencer (frame-task `execute` at target framerate, opportunistic inplace execution) · 1 events/scripts thread (SDL event polling, scripts, non-recurrent actions: body building — heavy part lazy — with attachment published as a render-chain Task, orbits/hints display, detach/remove, move, selection) · 1 compute thread (shadow projection, owns a compute VkQueue) [initially forgotten — vixy] · 1 worker executing **only ACTIVE-priority** tasks (a guaranteed free lane for the "needed now" class — reactivity) · nproc−3 generic workers (default, configurable) executing all tasks. The dedicated threads are rarely stressed simultaneously (< 3 active at a time in real-world use) and don't compete with other programs → REALTIME FIFO scheduling shouldn't hurt latency by design [vixy]; the "< 3 active" property is the load-bearing claim, not the exact thread count. **Open integration item** [vixy, initially forgotten]: screen capture for video rendering must fit the task system — `[inferred sketch]` readback rides the render chain after the frame task, encode is worker-domain, file write on the capture path's own ordering.

### 8.3 Rejected alternatives (with rejecting precondition)
- Per-body mutex: C3 + D5-floor (preempted holder stalls render a scheduling quantum); unconditional hot-path cost.
- Task execution at the owning body's draw (strategy.txt sketch): D3 starvation — halo-only bodies never draw → tasks never run → pin/VRAM leak. Its ordering property is retained via Taskable; its execution trigger is replaced by the ready-list drain.
- v1 single global completion queue (2026-07-11, same day): no per-body ordering guarantee across producers (two loaders finishing LoD1/LoD2 of one body can interleave); transient-window handled by defer-retry (one frame latency) where Taskable's marker-exchange resolves it with zero latency and exactly-once continuation. Superseded by §8.2.
- v2.1 never-idle render chain (2026-07-11, same day): a self-rescheduling frame task keeping the chain permanently busy bought "affinity by construction" — at the price of **unbounded stack growth**: synchronous `endTask` cascades nest, and a chain that never ends never resets the depth (unless TCO) [vixy: 2026-07-11]. The stack was the uncounted resource in the "by construction" reasoning. Superseded by the effective-thread model, where idle chains are correct (inplace execution is serialized by the chain promise, no affinity needed) and going idle resets the stack.
- Per-module atomic payload slots: O(visible) per-frame checks for rare events; check cost dominates event cost.
- shared_ptr lifetime: G8. · Generation counters + indirection: same guarantee as pins, more machinery. · Blocking flush on removal: blocks whichever thread removes.

### 8.4 Preconditions (design invalid if violated)
1. **All draw-visible state mutation happens through Tasks on the render chain** — reformulated 2026-07-11 from "on the render thread": the effective-thread model makes physical thread identity irrelevant; what must hold is that no draw-visible state is touched outside the chain. Scripts/events build offline and publish via chain Tasks (e.g. body attachment) — **VERIFIED as the intended management path** [vixy: 2026-07-11].
2. Integration tasks are O(small); violations show as frame spikes at drain (measurable, attributable).
3. Loaders read bodies only via pinned references; mutable inputs are snapshotted into the request at emission (render-side).

### 8.5 Open parameters (plan-mode decisions, not architecture)
Pool sizing policy over 4–64 cores · VRAM budget detection + degradation ladder values · `preservationCycles`/hysteresis values · attachment-list mechanism (mutex vs resource-as-Taskable) · overlap between s_texture dedup and the resource registry · **Task memory-validity flag candidate** (per §8.6.d "maybe worth a new flag"): dual-hold count on Task — init 2 (execution + linkage); `endTask` releases the execution hold (both on CAS-success, since no successor can then exist); the successor's producer releases the linkage hold after its `next.exchange`; at 0 → `recycle()`. Under ifdef, zero overhead when the external guarantee (task embedded in refcounted resource object) is used instead.

### 8.6 EntityCore/Executor inventory (2026-07-11) — reuse map + draft defects
**Reusable as-is or with port**: `AsyncLoaderMgr` (worker pools, BigSave native cache, O_DIRECT once-reads, priority scan, minPriority gate) — its main-thread `update()` pump (O(all-pending) scan per frame, poll not push — I3) is what the ready-list drain replaces; `AsyncBuilder` (refcount + atomic priority + two-phase asyncLoad/postLoad = the resource object pattern); `gc` (eviction/grace); `Taskable`/`Task` (the ordering primitive, in-progress).
**Defects found in the 2026-07-11 `Taskable.hpp` draft** (statuses per Vixy review, 2026-07-11):
- (a) Local shadows member: **RESOLVED on disk** — `execute()` renamed by Vixy; `scheduleExecution()` applied by Claude (2026-07-11, authorized). GCC behavior clarified [vixy]: the new local wins (consistent with the standard point-of-declaration reading; earlier divergence note was a misparse on my side — dissolved).
- (b) Missing memory edge on the marker handoff: **RESOLVED on disk** — producer half (`acq_rel` lines 21/32) by Vixy; consumer half (`endTask` marker exchange → `acq_rel`) applied by Claude (2026-07-11, authorized).
- (c) `Task::start` undefined → link error: **FIXED by Vixy** (now pure virtual; verified on disk 2026-07-11).
- **Contract comments written** (2026-07-11, per Vixy's "specify the promises where they weren't made explicit"): Taskable.hpp carries the full promise block (effective-thread framing, ordering window, completion = endTask, no-affinity, memory validity/marker window, scheduleExecution == false meaning, stack-nesting caution); Task.hpp carries the completion-semantics pointer.
- (d) Node reclamation: **RESOLVED as contract, promises to be written into the headers** [vixy: 2026-07-11 — "definitely worth specifying the promises where they weren't made explicit"]: ordering guarantee only within `[start, endTask)`; task holds nothing with side effects past `endTask`; lifetime via `TASKABLE_TASK_KEEP_REFERENCE` or an external guarantee (ifdef = zero overhead when external); not pooling. Memory residual **confirmed as real and hard to determine externally** [vixy] — a Task's storage must stay valid until successor-linked or chain-empty (the successor's producer may write `task->next` after `endTask` returned); "maybe worth a new flag" [vixy] → candidate design in §8.5.
- (e) `delete this` on releasing thread: **RESOLVED BY STRUCTURE** [vixy: 2026-07-11] — see §8.2.5: lightweight render-chain finalization tasks carry the releases, so the last release lands on the render thread by construction; the checkable hand-off rule is `[inferred — pending convergence]`.
- (f) `TASKABLE_TASK_KEEP_REFERENCE` acquire/release asymmetry (found 2026-07-11, second pass): `execute()`'s empty-chain branch acquires but `scheduleExecution()`'s does not, while `endTask`'s CAS-success releases unconditionally → scheduled-mode chain start with both ifdefs on underflows the refcount → premature delete. **MOOT under the chosen granularity** [vixy: 2026-07-11]: single-chain synchronization, all Taskables application-lifetime, flags off. Remains **latent** — must be fixed before any per-body-chain split enables the flag (reopening trigger in §8.2.5).
**Defects in existing `AsyncLoaderMgr` (pre-dating, lower priority — the port supersedes them)**: `addLoad`/`addBuild` `push_front` without holding the list spinlock while workers iterate under it (list-head data race); `AsyncLoader::priority` is plain (non-atomic) but written by worker and main threads (`AsyncBuilder::priority` is atomic — the asymmetry looks unintended); `update()`'s lock-busy branch iterates the list without the lock while a worker may scan it.

---

## 9. SSystemFactory seam table (Phase C, 2026-07-11)

The external world reaches the body system exclusively through `SSystemFactory` (callers: coreLink.cpp, core.cpp, executorModule). Per category, where each call routes when the new path is active (`drawModularSystem`). "dual" = already routed to both paths today.

| Category | Old-surface examples | New-path route |
|---|---|---|
| Selection/search | setSelected, searchByNamesI18, searchAround, listMatchingObjectsI18n | `ModularBody::findBody*` + `ModularBodySelector` + `ModularObject` (B7 surface); pointer draw via ObjectBase::drawPointer over ModularObject (D2) |
| Queries | getSelectedAZ/ALT/RA/DE, getPlanetsPosition, getSunAltitude/Azimuth | `ModularObject::getAltAz/getRaDeValue` (Camera-conversion idiom); az convention verify at D2 |
| Scale/size | setScale, setPlanetSizeScale, set/getMoonScale, setSizeLimit | `ModularBody::setScaling` (ASmooth) + global params; moon/sun scale = hardcoded-body scaling |
| Color | setBodyColor/getBodyColor, setDefaultBodyColor | per-module color members (Hint/Orbit/Trail modules) + defaults; haloColor already on body |
| Flags | setFlagHints/Axis/Trails/Orbits/Clouds, setPlanetHidden, toggleHideSatellites | module static `show` flags (HintModule etc.) + `ModularBody::hide/show`; per-name orbit flag = per-body module toggle |
| Textures | switchPlanetTexMap, createTexSkin, planetTesselation | BODY-module texture variants + global tesselation param (Renderer/config) — second pass |
| Body add/remove | addBody, preloadBody, removeBody, removeSupplementalBodies | `ModularSystem::loadBody` (dual for removeBody already); preload = `ModularBody::preload` |
| Anchors/camera | cameraMoveToPoint/Body, transitionTo*, alignCameraToBody, saveCameraPosition, switchToAnchor | `Camera` (motion/reference layer, live) + `CameraAnchors` sketch (named/persistence/scripted layer — second pass) |
| Per-frame | computePositions, computePreDraw, draw, update | frame task on the render chain (D1): update+draw fused; computePreDraw's buckets → Renderer depth partitioning (D4) |
| System transitions | enterSystem/leaveSystem, changeSystem | `Camera::switchToBody`/`warpToBody` + `ModularSystem::systemOf` |
| Tools | bodyTrace pen (upPen/downPen/togglePen) | stays a coreModule-level tool (not a body-render concern) |
| Fonts | registerFont | `HintModule::setFont` at the seam |

---

## 10. Renderer pipeline-family API — requirements (2026-07-11) + design (§10.3)

D2's blocker. §10.1/10.2 are observed need, not invented design; the design itself now exists (§10.3 + `PipelineFamily.hpp` + the registry block in `Renderer.hpp`, 2026-07-11) — headers written, implementation deliberately absent, pending Vixy convergence. B3's contract already fixes the frame: modules DESCRIBE (traits + descriptor contract), the Renderer EXECUTES (owns every drawState).

### 10.1 What it must manage
- **Pipeline families** replacing `SHADER_USE`/`selectShader` [observed: bodyShader.hpp:41-60]: mesh-layered (the hard one: night/clouds/specular/bump/tesselation feature COMBINATIONS — the old encoded them as enum entries + `pipelineOffset` variants [observed: body_bigbody.hpp:92]), OJM/model (± shadowed), ring (+ instanced-asteroid + depth-trace variants), lines (orbit/trail/axis/grid — depth-tested and depth-free variants), screen-space 2D (hint circle, pointer), big-halo/star, translucent shell (AtmExt), tail.
- **Per-pass variants**: a family must provide its variant per pass kind it participates in — COLOR, NO-DEPTH, SELF-SHADOW, SHADOW-STENCIL, TRACE [BodyModule.hpp hooks] — keyed at least by (family, pass, feature-variant). Old path had explicit SHADOW/DEPTH_TRACE drawStates [observed: bodyShader.hpp].
- **Render-pass compatibility**: pipelines are bound to FrameMgr passes (`PASS_MULTISAMPLE_DEPTH` etc. [observed: Renderer.cpp beginDraw; body.cpp:1036-1068]); self-shadow targets variable-size attachments (8192 main / 2048 secondary [ModularBody.hpp constants]) → dynamic viewport or per-resolution builds.
- **Descriptor machinery**: set layouts per family; set allocation for modules (BasicMesh owns an EntityCore `Set` [observed: BasicMesh.hpp]); big-texture rebind generations (`bigTexRecap::binding` [observed: s_texture.hpp:97-108], old `bigSet` lazy rebuild [observed: body_bigbody.cpp:445-478]); **descriptor POOL sizing per family needs** — live evidence this is currently broken: one `vkAllocateDescriptorSets` validation error per run, pool created without `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE` (§11.2).
- **Batching as a Renderer service**, dissolving the global batchers (`Halo::global` beginDraw/nextDraw/endDraw [observed: halo.hpp:66-70], `Tail::global` [observed: tail.hpp:43-46]): begin/submit/flush per family at pass boundaries; `BMT_REPLICATED` is the declared hint.
- **Global config → specialization**: tesselation levels (`BodyTesselation`, shared static [observed: body_tesselation.hpp]) and dynamic-resolution (`BMT_DYNAMIC`) parameterize pipeline builds → config changes trigger REBUILDS (work-domain) with publish-swap, never in-frame stalls.
- **The two existing old-path borrows to absorb first** (the API's first clients): `Renderer.cpp` includes `bodyModule/halo.hpp` and calls `Halo::beginDraw` in `beginBodyDraw` [observed]; `BasicMesh` includes `bodyModule/bodyShader.hpp` and uses its drawStates [observed]. The new path currently RUNS ON old-path pipelines — dissolving these is the API's acceptance test.

### 10.2 Constraints it must satisfy
- **C1/effective-thread**: registry mutation (new pipeline published, variant swapped) = chain task; pipeline USE within the frame task is read-only. Creation is heavy (old path: static init + 4 dedicated shadow-pipeline build threads [observed: context.cpp:79-83]) → creation is WORK-DOMAIN (AsyncBuilder fits), publication is a publish task.
- **C3**: never build-on-first-use synchronously in the frame. Corollary — **base-variant-always-ready**: each family keeps one variant resident from init, mirroring D6's lowest-LoD-always-resident (something drawable always exists; a missing fancy variant degrades, never stalls). Same ladder philosophy, pipelines instead of textures.
- **D5 budget**: do NOT precompile the feature-combination cross-product (2 GiB floor); lazy variant builds via work domain + the EntityCore pipelineCache for fast rebuild [observed: "No changes in the pipelineCache" shutdown log].
- **Trait-driven routing**: the Renderer routes a module through exactly the passes its BMT_* traits declare (a trait not declared = a pass never received [BodyModule.hpp contract]); MINOR_BODY exemption applies at body level above module traits.
- **Frame structure**: per-frameIdx command buffers, 3 frames in flight [observed: Context, app.cpp:373-408]; teardown order deterministic (old `deleteShader` pattern).
- **Text is a delegation, not a family**: labels go through the Projector gravity-text path + shared s_font [observed: hints.cpp:72] — the API must expose access to that path (or the frame task's Projector), not reimplement text.

### 10.3 The design (2026-07-11, headers written — pending Vixy convergence)

Authoritative contract: `PipelineFamily.hpp` + the registry block in `Renderer.hpp` (headers are the medium; this section records decisions, evidence and opens — not the contract itself).

Evidence base: 3 file:line-backed sweeps (old-path pipeline inventory / EntityCore capabilities / new-path borrow surface). Scale being replaced: ~41 shared old-path graphics pipelines + 3 per Sun/Star instance + 3 per Ring instance + ≤255 shadow-blur ComputePipelines.

Decisions, each with the fact that forced it:
1. **Exactly four PassKinds = the four drawing types; NO-DEPTH is the reserved variant bit `VARIANT_NO_DEPTH`, not a pass.** All body-path color/trace/line/halo pipelines live in ONE subpass (PASS_BACKGROUND==PASS_MULTISAMPLE_DEPTH==0 [observed: context.hpp:43-49]); no-depth is pipeline state (old `pipelineNoDepth = pipeline+2` [observed: bodyShader.cpp:128]); ringed's legitimate no-depth non-support [observed: body.cpp:1055-1059] maps onto bit-drop fallback. Real separate render passes exist only for shadows and already exist [observed: app.cpp:347-363, renderSelfShadow/renderShadow].
2. **Bind-and-record**: a module calls `bind(family, wanted)` (resolved against the Renderer's current pass kind — a COLOR pipeline structurally cannot bind while TRACE records) then records its own geometry. Retained draw-description submission rejected: per-draw marshalling on the hot path for zero safety gain, forbids module-owned record patterns (ObjL::bind/draw), contradicts G10 + the normative `operator VkCommandBuffer`. Reopen only if a pass appears whose recording the render chain cannot serialize. [open #1]
3. **Variant model**: uint16 feature bits; axes declare effect (SPEC_CONSTANT | SHADER_SWAP | STATE_OVERRIDE) + dropPriority; base variant of every declared pass built synchronously at registration (base-always-ready ← C3, mirrors D6 lowest-LoD-resident); non-base variants lazy in the work domain, published as a chain task (← C1; precedent: 4-thread shadow builds + atomic publish [observed: context.cpp:79-100]; pipelineCache concurrent use is legal); never the cross-product (← D5). EntityCore's 3-update deferred pipeline destroy [observed: Pipeline.hpp:70] covers publish-swap across frames in flight. SHADER_SWAP axes port today's distinct shader files unchanged; a future layered mesh shader collapses them into SPEC_CONSTANT axes with no API change.
4. **Layout-invariance hard rule**: a variant axis never changes the descriptor contract — (family, pass) → layout is constant; layout-changing features are separate families. Evidence: artificial vs artificialShadowed set-count divergence [observed: bodyShader.cpp:240-285]; myEarthShadowed omits the global UBO set that shaderShadowedTes includes [observed: bodyShader.cpp:359-370 vs 385-395] — normalized at port, not reproduced.
5. **Declarative descriptor contract → pool aggregation**: the same SetDesc data builds the PipelineLayout and sizes Renderer-owned pools — a pool is always sized from the layouts it serves; structural closure of the §11.1 error class (new-path scope; the live old-path instance needs the EntityCore fix, open #2).
6. **Batching as a Renderer service under the occlusion contract** [vixy: 2026-07-11]: batched screen-space content (halo, hint) occluded as-if drawn farthest→closest against bodies — flush at the per-body command-buffer boundaries, the mechanism already live [observed: Renderer.cpp:36-59]; halo↔hint mutual order inside a segment is free whenever it improves performance. Dissolves `Halo::global` (+ the Renderer friend-poke into its pData), Tail's ride-on-halo coupling [observed: halo.cpp:55,72], and the DrawHelper hint batch at the seam. **Scope note** [vixy: 2026-07-11]: "per-category order" is EnvironmentModule frame-sequencing (§3.8), NOT a batch-flush rule.
7. **Frame context on the Renderer** (Projector/Navigator set before beginDraw) — **INVALIDATED** [vixy: 2026-07-11, review]: Projector/Navigator are old-path projection machinery (system-center reference); the new path doesn't use them at all — projection rides the parent↔child matrix chain routed through the closest common parent between Camera and target (better precision by construction). Delegating labels to Projector would carry old-path state into new-path drawing — the exact borrow class this API dissolves. The gravity-text machinery must be partially rewritten against the new chain (duplicate code between paths is possible); the module-facing channel is redefined by the projection-path investigation (§11.14). Header carriage removed. [open #3 → investigation delivered a proposal: Renderer text service as a pure function of `screenPos` + viewport + font — printGravity180's frame-matrix dependency was apparent, not real (projection-paths.md C7); pending Vixy convergence]
8. **Compute families in the same registry** (kind=COMPUTE; shadow-blur bank = one family, radius = variant, EAGER_ASYNC_ALL policy reproducing today's behavior); dispatch-side ergonomics deferred to G7's shadow implementation. [open #4]
9. **Describe-once → descriptor paradigm** [vixy: 2026-07-11, header review — supersedes the first C-shaped draft]: descriptions are one-time cold-path costs, so they use owning types (std::string/std::vector — lifetime correctness by construction, no externally-asserted `const char*`/pointer-array clauses); allocation returns a descriptor (integer index wrapped in `RegistryHandle<Tag>`: copies share via registry refcount, destruction releases implicitly); hot paths carry only the index; all enums are `enum class`, declared before use. Structural payoff: the SetRole enum (FAMILY_OWNED/GLOBAL_UBO/EXTERNAL) **dissolved** — all three were ownership cases of one mechanism, "a `SetContract` allocated once, referenced by handle" (global UBO = a pre-allocated contract via `globalUboContract()`; traceLayout sharing = the same handle held by two families; a desc holding handles keeps its references alive by construction). Root cause of the first draft's shape, traced: hot-path constraints (zero-alloc, static tables) misapplied to a cold path + matching existing style weighed over long-run optimality — the criterion "optimal in the long run (decades), code debt prevention is critical, not nice-to-have" [vixy: 2026-07-11] now explicit.

Coverage check (every §10.1 family walked against the inventory; the walk itself forced two contract additions — `FixedState::stripBreaks` [Tail's primitive-restart strips, observed: tail.cpp] and `SpecConstant` device/config-derived values [float64 id 7 across families, sun big-halo viewport height, artificialShadowed shadow resolution — the "global config → specialization" input, rebuilt-not-stalled on change]; after which every family is expressible without struct change): mesh-layered (SHADER_SWAP now → spec-const later) · OJM + OJM_SHADOWED (rule 4) · RING color+trace, RING_ASTEROID instanced+batched · orbit/trail/axis/grid as separate families where push-constant contracts differ · HINT (batched) / POINTER · SUN / SUN_HALO / STAR_VIEWER / CORONA · ATM_EXT (SRC_ALPHA + MAX blend-op expressible in FixedState) · TAIL (instanced batch, shared geometry via VertexArray instance-rate) · trace/self-shadow/shadow-shape = Renderer-owned service families (traceLayout sharing = one SetContract held by both families). Sun/pointer prebuilt static command buffers dissolve into frame-task recording at port time (C1 requires it anyway).

**Open convergence points**: (1) bind-and-record confirmed as the module contract? (2) SetMgr SAMPLED_IMAGE pool-table fix in EntityCore — submodule authority (2-line class fix; decision 5 only covers new-path pools). (3) label/text channel: Renderer text service, pure function of `screenPos` + viewport + font (projection-paths.md C7) — accept? (4) compute dispatch deferred to G7 — accept? (5) variant budget: uint16, bit 15 reserved — sufficient? (6) depth-slice margin policy — **RESOLVED** [vixy: 2026-07-11]: boundingRadius is DEFINED as the smallest sphere enclosing the whole traced body — inclusive by definition, which is exactly why one value serves both the visibility cone test and depth bounding; no margin factor (the old 1.1 compensated a non-inclusive radius). Documented at BodyModule.hpp member + Renderer.hpp clearDepth. Consequence: a module whose drawn extent exceeds its mesh radius (terrain, shells, rings) must RETURN that larger radius from update() — the contract, not a renderer guard, carries the safety.

---

## 11. Open investigation log (post-context-clear continuation)

Ordered by priority-to-the-refactoring; each entry = observation + where to look.

1. **Descriptor-pool validation error** — **LOCATED (2026-07-11); fix pending Vixy (EntityCore authority)**. NOT BasicMesh: its layout is 2×UNIFORM + 2×COMBINED_IMAGE_SAMPLER, legal against the pool [observed: bodyShader.cpp:185-189, BasicMesh.cpp:13]. Culprit: `shadowLayout` binding 1 = SAMPLED_IMAGE [observed: context.cpp:18]; `ShadowData` allocates its Set from the app's single global SetMgr pool [observed: context.cpp:31]; SetMgr's pool-size table hardcodes 5 types with no SAMPLED_IMAGE and the ctor cannot express more [observed: SetMgr.cpp:6-16, SetMgr.hpp:11] — the only `vkCreateDescriptorPool` in the tree [observed: SetMgr.cpp:27]. Fix = 2-line SetMgr table entry (§10.3 open #2); new-path pools covered structurally by §10.3 decision 5.
2. **Earth requests an OJM loader** [live run, B6 warning]: `model_name` present on Earth in ssystem.ini? Intended (3D Earth model) or data quirk? Also Vesta OJM + pre-existing "Too many vertices for Vesta_1L.ojm (keep below 4000)" perf warning.
3. **Vixy's GOT probe is silent**: Moon is not `EARTH_MOON`-typed in this data — `hardcoded=true` apparently absent for Moon → `applyHardcodedContent` skipped → the Moon-divergence instrumentation observes only the old path's WANT side. Check ssystem.ini Moon section; the investigation may be mis-instrumented. (WANT fired; GOT never did; "Earth moon created" never printed — verified via tr'd logs.)
4. **Azimuth convention + label order**: old `Body::getAltAz` applies `az = 3π − az (mod 2π)` [body.cpp:366-376]; new `ModularObject::getAltAz` returns Camera-frame raw (flagged in-code). AND pre-existing inconsistency: `getInfoString` reads `observedPosToAltAz` pair as (Alt, Az) while `getShortInfoNavString` labels the same pair "Az/Alt" [ModularObject.cpp] — one mislabels; derivation says first=alt (rectToSphe arg order). Verify both at D2 with the pointer visuals.
5. **"Frame stall detected" + "CRITICAL [LinuxExecutor]: FAILED TO SUBMIT REQUEST"**: pre-existing (revert-A/B attributed, fire once around startup) but UNLOCATED — grep of src/ and src/EntityCore found neither string, yet the binary prints them. Find the source (EntityCore Tools? generated?); unattributed messages are unexplained deviations.
6. **isFalse("solid") behavioral fix still pending** (INTENT 5.2) — deferred because it shifts altitude semantics mid-Moon-investigation; apply on Vixy's go.
7. **AsyncLoaderMgr pre-existing races** (§8.6 list) — fix at D3 when the file is open anyway.
8. **ModularObject TODO** [ModularObject.cpp:68]: englishName=="Sun" execution path — Vixy's in-code question ("isStar()? system star?") awaits their answer; same class of hardcode as §5.5.
9. **strategy.txt TODOs**: fix halo (note: halo currently rides the OLD-path `Halo::global` through Renderer.cpp's borrow — the halo bug may live in that borrow's integration, look there first), s_font not shown (D2), pointer when selected (D2).
10. **Camera.cpp in-code Moon notes** [committed, update()]: "Moon: Got [...], expected [...]; Shift approximation: [-Y, X, Z]" — Vixy's active divergence hypothesis; do not disturb, context for #3.
11. **Tooling note (process, not code)**: `git stash push -- <paths>` failed silently in this tree ("did you forget git add?" while diff showed changes) — cause unidentified (submodule? locale? pathspec). Use diff-to-patch + checkout for reverts here. Also: exit codes through pipes need `set -o pipefail` — two maskings occurred this session.
12. **CMake GLOB caveat**: new .cpp files need a manual `cmake .` re-run (no CONFIGURE_DEPENDS) — D2/D3 will add files.
13. **Draw-hook activation state** (evidence recorded for §10.3): `drawShadow`/`drawSelfShadow`/`drawTrace` and `drainNotableBodies` have zero call sites in the new path (BasicMesh::drawSelfShadow body fully commented out) [observed: grep 2026-07-11]; `BasicMesh::drawTrace` pushes constants without binding a pipeline [observed: BasicMesh.cpp:121-123] — already the shape the §10.3 service-family model formalizes (the Renderer binds once per TRACE stream; modules push+draw).
14. **Projection-path investigation** — **DELIVERED (2026-07-11): `projection-paths.md`** (two per-path reports + differential; load-bearing claims re-verified on source). Key outcomes: (a) both paths share the SAME GPU fisheye + clipping semantics — the entire difference is matrix/clipping supply (equivalence verdict: equivalent by construction at GPU stage, intended-equivalent at chain stage, factually NOT equivalent — the Moon divergence is chain-stage; conditions E1-E6 enumerated, E2/E3 prime suspects); (b) label channel resolved as proposal: `printGravity180`'s real inputs are pixel anchor + viewport + font only → Renderer text service, pure function of already-available `screenPos` — no Projector/Navigator, no frame context (→ §10.3 open #3); (c) found delta: old depth buckets carry a 1.1 margin, new `clearDepth` none → slice-boundary clipping risk, margin policy belongs in the §3.6 partitioning contract; (d) Moon-divergence observations [derived from Camera.cpp:82-84 numbers]: the [-Y,X,Z] xy-signature = zrotation(+90°) but z-sign disagrees (second component exists), and Δ|distance| ≈ 4800 km ≈ Earth-radius order (topocentric-vs-geocentric hint); ±90° convention sites listed. Analysis only — investigation remains Vixy's (§11.10); (e) fisheye exists 4× in old CPU/GPU code + 1 new form — consolidation target: 1 GLSL + 1 CPU helper.
