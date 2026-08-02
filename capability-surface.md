# Capability surface — the S6 reachability audit (2026-07-25, task F4)

**What this file is.** The live inventory of spacecrafter's *capabilities* and how each
one is reachable at runtime. It is an AUTHORITY FILE, not a journal entry: the table
below is a living view that later waves re-verify and extend, while `INTENT/11.108.md`
(the journal entry that created it) is append-only and carries the method, the verdicts
and the minted rows. On divergence about a row's *state*, this file wins; on divergence
about *what was measured on 2026-07-25*, §11.108 wins. Every row carries provenance;
re-verify at source before acting on a row (§5.2 class).

**Why it exists — the bar, and why it is not "every command still works".**
§2(c) states the control-surface intent: *scriptable AND scriptless-with-full-potential*,
neither channel foreclosing the other. §11.55 raised the audit bar explicitly:
> the criterion is not "every old command still works" but **"every capability is
> reachable dynamically"**, which is a strictly larger surface — the audit must
> enumerate *capabilities*, not *commands* (a capability with no command is exactly
> what a command-keyed audit cannot see; §11.33's spec-8 inventory is the same blind
> spot one layer down).

So the enumeration below starts from what the ENGINE can do (its own surfaces), not from
what the parser accepts, and asks of each: *by what channel can an operator reach this at
runtime?*

---

## 1. The channel model (observed, not assumed)

Before classifying reachability, the channels themselves have to be enumerated — the
§2(c) reading depends on which of them exist.

| # | Channel | Kind | Enabled by | Entry point | Sink |
|---|---|---|---|---|---|
| 1 | **Authored script** (`.sts`) | pre-written | always | `script action play filename …`, startup.sts (`app.cpp:589`), masterput drop (`app.cpp:949-959`, `io:flag_masterput`, shipped **true**) | `AppCommandInterface::executeCommand` |
| 2 | **TCP line protocol** | LIVE, arbitrary command string | `io:enable_tcp` (shipped **true**), port `io:tcp_port_in` = 7805 | `app.cpp:698-709` → `io.cpp:462,485,505,625` → `app.cpp:746` | same |
| 3 | **HTTP `GET ?command=`** on the same socket | LIVE, arbitrary command string | same as 2 | `io.cpp:526-555` (`#ifdef LINUX`, set at `CMakeLists.txt:38`) | same |
| 4 | **Named pipe** | LIVE, arbitrary command string | `io:enable_mkfifo` (shipped **false**) | `app.cpp:712-717` → `mkfifo.cpp:79-125` → `app.cpp:737` | same |
| 5 | **Joypad `command("…")` bindings** | LIVE, operator-authored in `joypad.ini` | joystick plugged | `joypad_controller.cpp:363-384,221-225` → `ui.cpp:769` | same |
| 6 | **Keyboard** | interactive | always | `ui.cpp:787-905`, `handleKeyPressed` `ui.cpp:1064-3346` | mixed: mostly `executeCommand`/FlagEvent, some direct `core->` calls |
| 7 | **Mouse** | interactive | always (`gui:flag_mouse_usable_in_script` gates during scripts) | `ui.cpp:842-888`, `handleClic` `:417`, `handleMove` `:344` | direct `core->` calls |
| 8 | **TUI** (`;`) | interactive | `gui:flag_enable_tui_menu` (shipped **true**) | `ui_tuiconf.cpp:184-396`, `ui.cpp:1151` | mixed: mostly `executeCommand`, some direct setters |
| 9 | **Joypad axes/buttons** (non-`command`) | interactive | joystick plugged | `joypad_controller.cpp:302-467` | direct `UI::`/`core->` calls |
| 10 | **config.ini / beta_features.ini / joypad.ini** | startup only | — | `App::init`, `Core::init`, `SSystemFactory::loadCamera` | — |
| 11 | **ssystem.ini / composed system file** | load time (+ runtime via `body action load … replace true`) | — | `ModularSystem::loadBody` | — |

**Consequence for the §2(c) reading [derived — flagged, this is the audit's one
interpretive step]**: channels 2–5 mean *a command string is issuable live, without
authoring a script*. So "scriptless with full potential" is NOT violated by a capability
that has a command but no key — the operator can issue it live. What the two-channel
intent DOES forbid, and what this audit therefore counts as gaps:

* **G-UNREACHABLE** — no channel at all: the capability exists in code and nothing can
  drive it. (Includes the `freeMode`/`reloadSystem` class that §11.36/§11.55 closed.)
* **G-UI-ONLY** — reachable only by moving a mouse/pressing a key: **not scriptable**,
  so it cannot appear in a show — a direct R1 loss (realised use cases foreclosed).
* **G-CONFIG-ONLY** — settable only at startup: not reachable *dynamically*, which is
  §11.55's own wording of the bar.
* **G-OLD-ONLY** — the command exists and lands, but only on the old path: under the new
  render path the operator's action does nothing visible. A §12/S6 seam gap.
* **G-QUERY** — the setter is dual but the GETTER reads the other path: the readout and
  every relative command computed from it are wrong for the path that draws (§11.102(b2)
  is one instance; §3 below shows it is a class).

Not gaps: a command with no key (channels 2–5 cover it); a data key with no runtime
setter **when `body action load … replace true` can re-author it** (`ModularSystem.cpp:960`)
— the cost of that route (the body is rebuilt, per-body runtime state resets) is itself
a recorded open item (§11.55(i)).

---

## 2. Enumeration method, and its completeness argument

Six independent bases were planned and unioned — **four completed, two not** (B and F
below). No single base is trusted to be complete, and the overlap between them is what
bounds the residual (the §11.73(a) pattern); where a base is incomplete the rows that
would have come from it are missing, not wrong, and that is said in its own row:

| base | what it enumerates | how | state / residual |
|---|---|---|---|
| **A** | the §9 seam table's 12 categories | read | DONE. §9 is itself a curated view (it names *examples* per row) ⇒ not a closed set |
| **B** | `coreModule/coreLink.hpp` public methods (the engine's own control-surface API) | receiver-anchored grep (`coreLink->` / `CoreLink::instance->`) over 694 lines, comment-filtered by a column-before-`//` test, negative routes checked (line-broken calls, member pointers, macros, `#if 0` — all 0), every ZERO re-verified by an independent bare-name grep | ~~PARTIAL — the largest open residual~~ **CLOSED 2026-07-25 (same day, recovered delegation — see §3.7).** Coverage: the whole public section L34–L1026, **336 declaration lines / 330 names**, 6 overload pairs (all 12 overloads individually live, checked by arity). Buckets: **CMD 201 · BOTH 53 · UI 26 · OTHER 20 · CMD+OTHER 14 · ZERO 14 · BOTH+OTHER 6 · UI+OTHER 2**. Residual: methods reached through `core->` directly bypass this receiver anchor (they are covered by base C where a new-path counterpart exists) |
| **C** | the new path's runtime-settable state: every public non-const method + every `static` flag under `src/experimentalModule/` (37+12+5+3+14+9 files) | header sweep + bare-symbol grep over `src/` for each claim | DONE (delegated, then re-grepped by the recorder for every "dead"/"only caller" row quoted here). Not covered: GLSL-side spec constants, `PipelineRegistry` internals, EntityCore |
| **D** | data-authorable behaviour: every `param["…"]` key read by a loader | `grep -rhoE '(param\|params)\["[a-z_0-9]+"\]'` over `moduleLoader/ orbitModules/ ModularBody.cpp ModularSystem.cpp` | DONE for the new path; old-path-only keys (`bodyModule/`) not swept |
| **E** | the input surfaces: every key/mouse/TUI/joypad binding | `ui.cpp` (3447 lines) + `ui_tuiconf.cpp` + `joypad_controller.cpp` + `mkfifo.cpp` + `io.cpp`'s server read end-to-end | DONE (delegated, channel-model claims re-verified by the recorder). TUI menus 7/8 line citations approximate |
| **F** | the COMMAND side: every `flag <name>` / `set <name>` / sub-grammar key → its engine call, + tokens declared with no handler | table-driven: every `m_commands` / `m_flags` / `m_set` / `m_color` registration parsed and joined to its case label + callee; `app_command_interface.cpp` read end-to-end (1–4298); all **366** `args[KEY]` occurrences reconciled | ~~NOT DONE~~ **CLOSED 2026-07-25 (same day, recovered delegation — see §3.8).** Counts cross-checked three ways: `m_commands` 59 keys / 58 handlers (+`comment`/`uncomment`/`struct` as pre-table literals), `m_flags` **97 = 97 cases = 97 enum**, `m_set` 43 keys / 44 cases (43 + NONE), `m_color` **46 = 46 = 46**. Residual, named by the census itself: five arg-hashes are passed through to key sets that live OUTSIDE the interface (`dsoNavInsert`/`dsoNavSetupVolumetric` `coreLink.cpp:773,783`; `loadLandscape` `core.cpp:759`; `addSolarSystemBody`/`preloadSolarSystemBody` `core.cpp:780,827` = the ssystem.ini grammar; `cameraAddAnchor` `anchor_manager.cpp:271`) — those grammars are base D's and the loaders', not this one's |

**What the union does NOT cover, stated so the claim is falsifiable**: (i) capabilities
that exist only as an *intention* (a header contract with no implementation — e.g. row-16
surface streaming) are outside "reachable" by construction; (ii) old-path internal
behaviours with no control surface at all (they retire with the old path, §12); (iii) the
*effectiveness* of a reachable seam — this audit establishes that a caller exists, not
that the callee does what its name says (the `setDayKeyMode` no-op in §4 is exactly why
that distinction is kept explicit); (iv) `EntityCore` (submodule, read-only).

---

## 3. Inventory — by capability domain

Reachability classes: **CMD** (command string, hence channels 1–5) · **UI** (key/mouse/
TUI/joypad only) · **CFG** (startup only) · **DATA** (authored data; ± runtime via body
replace) · **DEAD** (no driver) · **OLD** (command lands on the old path only).

### 3.1 Observer / camera

| capability | new-path authority | reach | evidence |
|---|---|---|---|
| position lon/lat/alt goto | `Camera::moveTo` | CMD `moveto …` | `app_command_interface.cpp:3025` → `coreLink.hpp:879-881` (dual) |
| relative lon/lat step | `Camera::moveRelLon/Lat` | **UI** | `coreLink.hpp:885-892` ← `ui.cpp:581-598,2027,2042,2276,2291` only |
| relative altitude step | `Camera::moveRelAlt` | **UI** | `coreLink.hpp:894-897` ← `ui.cpp:605,612` only |
| interactive altitude RAMP (hold) | `Camera::multAlt`→`descend` | **UI** (joypad button) — **dual since F4** | `core.cpp:1795-1808` (this task); was G-OLD-ONLY, `Camera::multAlt` had 0 callers (§11.71) |
| view-directed descent | `Camera::descend` | CMD `camera action descend coef <c>` | `app_command_interface.cpp:3924-3935` → `coreLink.hpp:874-876` |
| free mode on/off | `Camera::setFreeMode` | CMD `camera action free_mode state on\|off` | `app_command_interface.cpp:3911-3921` (§11.36) |
| reference switch | `Camera::warpToBody` | CMD `set home_planet X` | `core.cpp:1818` → `ssystem_factory.cpp:744` |
| look at alt/az | `Camera::lookTo` | CMD `look_at azimuth … altitude …` | `app_command_interface.cpp:2749-2752` → `coreLink.hpp:374` |
| continuous pan (hold) | `Camera::lookRel` | **UI** (dual since 2026-08-02) | `core.cpp` `Core::updateMove` → `navigation->updateMove` **+ `Camera::lookRel(vzm.deltaAlt, vzm.deltaAz, 0)`** (§11.133, code `d9de42ac`). Was OLD-ONLY and measured so (xkey, Left 2500 ms: new camera az/alt bit-identical, new-path Moon |Δ| 0.000e+00 NDC vs old 872.79 px). **MEASURED live both ways** on the delivered binary: 360 steps, per-step view deltas equal to **1.965e-08 rad**, release row stepping neither path, drawn frame **1087 px>32 of 857 lit** against **0** pre-fix → **B34 CLOSED** |
| mouse-drag look-around | `Camera::lookRel` | **UI** (dual, and its VERTICAL sign was INVERTED until 2026-08-02) | `core.cpp` `Core::dragView` → both paths — *the only caller is `ui.cpp:364`*. It hands old's own two numbers to `lookRel`, which read one of them in the opposite sign until §11.133 gave that function old's convention: **delivered old +0.100918 / new +0.100918 rad, pre-fix old +0.100918 / new −0.100918** (`f25_drag.py`, gdb-driven — XTEST pointer MOTION is dead on this host) |
| pixel pick (click-select) | `ModularSystem::findBodyAt` | **UI** | `ui.cpp:479` → `core.cpp:1015-1029` → `ssystem_factory.cpp:642`; no command (§11.106(a)) |
| heading (absolute) | `Camera::setHeading` | CMD `set heading X` / `heading azimuth X [duration d]` | `coreLink.hpp:978-983` (dual) |
| heading (relative) | `Camera::moveHeading` | CMD `heading delta_azimuth d` + UI | `app_command_interface.cpp:2125-2132`; `coreLink.hpp:931` ← `ui.cpp:2562,2569,2737,2744` |
| sky lock | `Camera::setSkyLock` | CMD `flag lock_sky_position` | `app_command_interface.cpp:948-952` → `core.cpp:799-800` |
| fov | `Camera::setHalfFov` | CMD `zoom fov …` / `set fov` | `coreLink.hpp:311,327` |
| continuous zoom (hold) | `Camera::setHalfFovNow` | **UI** (dual since 2026-08-02) | `core.cpp` `Core::updateMove` → `projection->changeFov` **+ `Camera::setHalfFovNow(projection->getFov()·π/360)`** (§11.133, code `d9de42ac`). PROBED before it was mirrored, and measured MISSING: 215 steps moving the old fov **60.00000 → 28.30499°** with the drawn `ModularBody::halfFov` **bit-identical on every one of them**. Mirrored from old's POST-clamp fov so old's `[min_fov, max_fov]` is the one clamp decision (the camera's range is not the same — B35); delivered, the two agree at every step to **3.4e-06 degrees** → **B34 CLOSED** |
| **mount ALTAZ↔EQUATORIAL** | `Camera::setMount` | **CFG** | `ssystem_factory.cpp:165` only; `Core::setMountMode`/`toggleMountMode` (`core.hpp:231,239`) have **zero callers in `src/`** ⇒ config-only on BOTH paths → **B35** |
| bound-to-surface | `Camera::setBoundToSurface` | **CFG** | `ssystem_factory.cpp:155` only → **B35** |
| fov clamp min/max | `Camera::minHalfFov/maxHalfFov` | **DEAD** (hard-coded) | `Camera.cpp:13-14`; `CoreLink::setMaxFov` reaches only `Projector` → **B35** |
| tracking on/off | `Camera::trackBody` | CMD `flag track_object` | `core.cpp:2146-2151` |
| view offset | `Camera::setViewOffset` | CMD `set zoom_offset v` (+ CFG) | `core.cpp:2181-2211` (§11.92) |
| offset ARMING | `Camera::armViewOffset` | CMD (indirect: look_at/zoom/track) | 4 of the **5** old arming sites mirrored; `gotoSelectedObject` is not → §11.102(b1) |

### 3.2 Camera/observer QUERIES — the G-QUERY class — **CLASS CLOSED 2026-08-01**

Every getter below was on the control surface and read the **old** path while its setter
was dual, i.e. under the new render path it was a readout of a state that is not what
draws. **All of them now ask `Core::getExperimentalPath()` and report the path that
DRAWS** — F12 (`heading`, §11.118(f)) and F23 (the other four, §11.131). The table is
kept because the ROW SHAPE is the finding, and re-verified at the delivered HEAD.

| query | control-surface getter | reads NOW | the drawn path's own member | evidence |
|---|---|---|---|---|
| heading | `CoreLink::getHeading` | **the path that draws** | `Camera::getHeading` | §11.118(f); pre-fix divergence 6.16° (§11.108(c)) |
| view offset | `CoreLink::getViewOffset` | **the path that draws** | `Camera::getViewOffset` | §11.131(d) — LATENT (one writer, `Core::setViewOffset`); injected old 0.3 / drawn 0.15 |
| latitude / longitude / altitude | `CoreLink::observatoryGet*` | **the path that draws** | `Camera::getPlace()` (the inverse of `moveTo`'s target, both modes) | §11.131(b) — REAL channel: `camera action descend`, old 40 000 000 m vs drawn 9 999 999.363 m |
| mount | `Core::getMountMode` | **the path that draws** | `Camera::getMount` | §11.131(d) — LATENT (one config key); injected old equatorial / drawn altaz. **The WRITE half is still old-only** → B35 |
| sky lock | `Core::getFlagLockSkyPosition` | **the path that draws** | `Camera::getSkyLock` | §11.131(c) — REAL channel: four sites write the old flag alone; the toggle was a no-op in one direction |
| selected object RA/DE, Alt/Az | `ModularObject` | new path | — | offset-skewed while armed: §11.102(b2) (SUSPENDED with §11.92(d)) |

`heading delta_azimuth d` **computed its target from the old getter** and wrote it to
both (`app_command_interface.cpp:2125`), silently re-synchronising the two headings —
measured (§11.108(c)), fixed §11.118(f). The same shape at the place: `moveto` with any
absent component, `moveto multiply_alt`/`delta_alt`, `mode jump … altitude ±x`, the
joypad height axis and the TUI location callback all build an ABSOLUTE target from these
getters and write it to both paths — measured pre-fix as a **738 266 px>8** teleport of
the drawn observer under a semantic no-op (§11.131(b)).

**Second readers of the same readouts** (bypassing the getters, hence outside the fix
until each is folded): `CoreLink::tcpGetPosition` — **folded** (§11.118(f) heading,
§11.131(f) place); `CoreBackup::saveBackup`/`loadBackup` — ~~not folded~~ **FOLDED 2026-08-01 (F24, §11.132(e))**: read
through these getters + restore through `observerMoveTo`, in one change; `position action load` now
brings the CAMERA back (9 999 995.009 m, screen 6 px>8 from the bookmark) where the pre-fix binary
leaves it where the move put it (39 999 997.452 m, 0 px>8);
`Core::dragView`'s direct `getViewingMode()` read → **B35**. The readout channel itself
was the class's own obstacle: ~~`get status position` never replies (§5.47)~~ **FIXED
2026-08-02 (F27, §11.135, code `d13681eb`): a `get`'s answer now lands on the connection
that issued it — the queue was always drained, but through `broadcast`, i.e. only to the
clients subscribed to the log feed with `$LOGON`. Measured 6.007 s of silence pre-fix vs a
reply in 0.002 s delivered, content field-for-field equal to `control.reported` in a scene
whose two heading authorities are 6.16° apart.** The `control` object on `body action
dual_dump` (§11.131(a)) is what made any of this measurable, and remains the per-member
instrument (`{reported, old, new}` is a comparison the single-value TCP answer cannot make).
→ **B33 (CLOSED)**.

### 3.3 Bodies

| capability | reach | evidence |
|---|---|---|
| create / replace a body from any data key | CMD `body action load name X [replace true] …` | `app_command_interface.cpp:3560` → `ModularSystem::loadBody`, replace gate `ModularSystem.cpp:960` |
| drop a body | CMD `body action drop name X` | `ssystem_factory.hpp:698` (dual) |
| **drop script-added bodies** | CMD `body action clear` — ~~OLD ONLY~~ **DUAL since F24 (§11.132(b))** | `SSystemFactory::removeSupplementalBodies` mirrors on old's own refusal answer (I2); provenance is `ModularBody::supplemental`, written by the loader, a `replace` inheriting the NAME's provenance. Measured: 3 pushed bodies (plain + HIDDEN + nested child) gone from both trees, 120 declared surviving, screen **90 601 px>8** vs **0 px>8** pre-fix; old's removal set 93 → 90 identical on both binaries |
| **preload a body's resources** | CMD `body action preload` — ~~OLD ONLY~~ **DUAL since F24 (§11.132(c))** | `SSystemFactory::preloadBody` mirrors the per-body half (the purge half was always engine-wide: s_texture's pools are static). `ModularBody::preload` leaves B36's list; `keep_time` is threaded to the modules, which hardcoded a lifetime of 100. Measured on a NEW-PATH-ONLY subject (old cannot see it): `preloadCount` 0 → 1 and the big-texture record ACQUIRED (16384×8192) vs 0/absent pre-fix. **`keep_time` itself is 8-bit-truncated → §5.69** |
| reload the system from file | CMD `body action reload` | `ssystem_factory.cpp:649-660` (§11.55) |
| **write the system to a composed file** (a script-pushed body becomes authored data) | CMD `body action save [filename <name>]` — **NEW-path concept, no old mirror** | `app_command_interface.cpp:3585` → `SSystemFactory::saveCurrentSystem` (`ssystem_factory.cpp:793`, path convention + D35 refusals) → `ModularSystem::saveSystem` (§11.121, B31 slice 2). Spelling is a **veto point** |
| hide / show | CMD `body name X hidden on\|off\|toggle` (+ DATA `hidden`) | `ssystem_factory.hpp:315-317` |
| per-body colour (halo/label/orbit/trail/all) | CMD `body name X color <ch> value r,g,b` | `ssystem_factory.hpp:455-461` (§11.65) |
| skin texture create / switch | CMD `body name X skin_tex …` / `skin_use …` | `ssystem_factory.hpp:413,430` (§11.46) |
| per-body orbit line | CMD `body name X orbit on\|off` | `ssystem_factory.hpp:400` |
| **per-body trail** | no command (only the global focus filter) | `ModularBody::setFlagTrail` reachable only via `ssystem_factory.hpp:347` → **B37** |
| datum / ground radius | CMD `body name X datum_radius\|ground_radius <km>` | `ssystem_factory.cpp:755,764` (§11.71) |
| display scaling | CMD `planet_scale name X scale s`, `flag moon_scaled\|sun_scaled`, `set moon_scale\|sun_scale` | `ssystem_factory.hpp:573` (§11.45) |
| tesselation level | CMD `body tesselation <name> value v` | shared `BodyTesselation` (§11.26) |
| **trail fresh-restart** | not a command (the RESTART semantic: config init + `setHomePlanet`) — ~~old only~~ **DUAL since F24 (§11.132(d))** | `SSystemFactory::startTrails` mirrors through `ModularSystem::startTrails` → `ModularBody::startTrail`, old's per-system hidden-inclusive scope. `TrailModule::startTrail` leaves B36's list. Measured (`b11_trail_gate` phase 5): 39 → 1 points and a 570 d / 8.02 AU span → 0, against 39 → 39 pre-fix. The control-surface wrapper `CoreLink::startPlanetsTrails` is still a **ZERO** (§3.7 row 7) |
| ~90 other data keys (rotation, orbit, albedo, atmosphere, textures, tails, rings…) | DATA + CMD via body replace | `ModularSystem.cpp:855-1290`, `moduleLoader/*` |

### 3.4 Per-module toggles (new path)

| module | toggle | reach |
|---|---|---|
| Hint (label+circle) | `HintModule::show` | CMD `flag planet_names` |
| Axis | `AxisModule::show` | CMD `flag planets_axis` |
| **PlanetGrid** | `PlanetGridModule::show` | **no flag of its own** — rides `flag planets_axis` (`ssystem_factory.hpp:352-359`); old-path parity, recorded at A4(c) |
| PlanetGrid tropics / polar circles | `showTropics`/`showPolarCircles` | CMD `flag tropic_lines` / `flag polar_circle` (per-frame poll, `core.cpp:633-646`) |
| Orbit | `setGlobalPlanets`/`setGlobalSatellites` | CMD `flag planets_orbits` / `flag satellites_orbits` |
| Trail | `setGlobalShow` | CMD `flag object_trails` |
| Oort | `OortModule::show` | CMD `flag oort` |
| Shadows | `ShadowService::enabled` | CMD `flag experimental_shadows` |
| Pointer | `Renderer::showPointer` | CMD `select … pointer off` |
| **Ring / AtmExt / StarModule big halo / Tail / Ojm / meshes** | **no toggle exists on either path** | not a gap against old (parity), recorded as the *shape* of the per-feature surface |
| **Oort cloud COLOUR** | `OortModule::cloudColor` | **CFG only** — `Core::setColorScheme` is called only from `app.cpp:640` (startup); refines §11.102(e3)'s "runtime" wording → **B35** |

### 3.5 Application-level

| capability | reach | note |
|---|---|---|
| screenshot / domemaster | CMD `domemasters action snapshot` + UI KWIN+S | same sink |
| save / load configuration | CMD `configuration action save\|load` + TUI 8.1/8.2 | |
| shutdown | CMD `shutdown action now` + keys + TUI 8.3 | |
| **open the TUI menu** | **UI only** | `ui.cpp:1151`; no command → B37 |
| **mouse cursor show/hide/warp, cursor timeout** | **UI only** (timeout also TUI 6.7) | `ui.cpp:2912-2917,560,574,664` → B37 |
| **preset sky time** | **UI only** (TUI 2.4) | `ui_tuiconf.cpp:662`; commands only READ it (`app_command_interface.cpp:3464`) → B37 |
| **day-key mode (calendar/sidereal)** | TUI 2.3 — **INERT** | `App::setDayKeyMode` (`app.hpp:129-130`) declares a shadowing LOCAL ⇒ no-op; and `DayKeyMode` has no behavioural consumer at all → **§5.35** |
| **debug dumps** (`cameraDisplayAnchor`, `observerDisplayPos`, `AppSettings::display_all`) | **UI only** | `ui.cpp:1303,1306,1262` → B37 |
| script record / play / pause / speed | CMD + UI | |

### 3.6 Dead capabilities (no driver at all)

| symbol | decl | note |
|---|---|---|
| ~~`ModularBody::preload`~~ **DRIVEN 2026-08-01 (F24, §11.132(c))** | `ModularBody.hpp` | `SSystemFactory::preloadBody` calls it, and with it `BasicMesh`/`LayeredMesh`/`PhotosphereModule::preload`; measured 0 → 1 `preloadCount` and an acquired big-texture record |
| `ModularBody::pin` / `unpin` | `:1213` / `:1218` | the whole C2 work-domain pin mechanism, incl. `RenderChain::onPinDrained` |
| `ModularBody::findBodyNameI18n` | `:1159` (def `ModularBody.cpp:433`) | i18n lookup |
| ~~`TrailModule::startTrail(bool)`~~ **DRIVEN 2026-08-01 (F24, §11.132(d))** | `TrailModule.hpp` | reached from `SSystemFactory::startTrails` via `ModularSystem::startTrails` → `ModularBody::startTrail`; measured 39 → 1 points across a perspective change |
| `Camera::multAlt` | `Camera.hpp:139` | **CLOSED by F4** (`core.cpp:1806`) |
| `ModularSystem::hasSystemFile` | `ModularSystem.hpp:56` | |
| `EnvironmentManager::getAtmosphereUserFlag` | `EnvironmentManager.hpp:66` | |
| `BasicMesh::invalidate()` | `BasicMesh.hpp:33` | **declared, never defined**, never called |
| `Core::setMountMode` / `toggleMountMode` | `core.hpp:231,239` | zero callers ⇒ the mount toggle is unreachable |
| `UI::handleKeysReleased` / `UI::handleJoyHat` | `ui.cpp:3348` / `:517` | dead duplicates of live handlers |

---

### 3.7 CoreLink control-surface API — the ZERO set (base B, 14 of 330 names)

Declared on the engine's own control-surface API, **no live caller anywhere in `src/`**.
Each was re-verified by an independent bare-name grep; the "other hits" column is what a
naive grep would have scored them as, and is the reason this class survived four earlier
audits.

| # | method | decl | what a naive grep sees |
|---|---|---|---|
| 1 | `tullySetDuration` | L134 | — |
| 2 | `illuminateSetSize` | L139 | — |
| 3 | `starNavGetMaxMagName` | L202 | — |
| 4 | `setMaxFov` | L331 | 5 hits = `Projector`/`MagConverter` methods of the same name |
| 5 | `cameraMoveRelativeXYZ` | L363 | 5 hits in `ui.cpp` (2599, 2628, 2659, 2718, 2774) — **all commented out**; a naive grep scores it UI |
| 6 | `constellationGetArtFadeDuration` | L434 | — |
| 7 | `startPlanetsTrails` | L474 | see the B34 reconciliation below |
| 8 | `setPlanetsSelected` | L478 | 7 `core.cpp` hits = trailing comments on `ssystemFactory->setSelected` lines |
| 9 | `getMoonScale` | L496 | `SSystemFactory`/`SolarSystem` methods |
| 10 | `getSunScale` | L502 | same |
| 11 | `planetSetFlagOrbits` | L583 | **one character** from the live plural `planetsSetFlagOrbits` |
| 12 | `planetGetColor` | L593 | — |
| 13 | `milkyWayChangeState` | L709 | 3 hits = the substring of the LIVE `…WithoutIntensity` |
| 14 | `dso3dSetDuration` | L748 | — |

**B34 RECONCILIATION (my own row was imprecise — corrected here and in the ledger):** the
trail fresh-restart is NOT "a command that lands on the old path only". `flag object_trails`
reaches `CoreLink::planetsSetFlagTrails` → `SSystemFactory::setFlagTrails`, which **is dual**
(the `TrailModule::show` mirror, `ssystem_factory.hpp:333`). The restart semantic lives in
`CoreLink::startPlanetsTrails` (`coreLink.cpp:1144`), and **that wrapper has zero callers** —
the only live callers of `SSystemFactory::startTrails` are `core.cpp:370` (config init) and
`core.cpp:1833` (`setHomePlanet`), both old-only. So the item is simultaneously ZERO-class
(the control-surface entry) and OLD-ONLY (the two internal callers); the *command* claim was
wrong.

**Doc-rot traps recorded for future audits** (not defects, but they cost time): `ui.cpp`
references `coreLink->timeResetMultiplier()` **six** times (L782, 972, 1086, 1438, 1472, 2354)
and the method **does not exist on `CoreLink` at all** — every reference is inside a comment;
and `coreLink.hpp:821-823`'s own marker `// Fonctions non utilisée ?` is **wrong** (all three
methods under it are live). Ambiguous-name traps: `setMaxFov`/`getFov`/`setFov`/`zoomTo`/
`getAimFov` (Projector, MagConverter) · `get/setMoonScale`, `get/setSunScale` (SSystemFactory,
SolarSystem) · `lookAt`, `set/getHeading`, `set/getLocalVision`, `setDefaultHeading` (Navigator) ·
`observatory*`, `observerMoveRel*` (Observer) · `getMag` (star wrappers).

**Record-path-only methods** (their sole caller is the script-RECORDING path, so they are
control-surface *queries* with no read channel): `getDateSecond` L635 (the only date getter
with no `$var` route), `getSelectedPlanetEnglishName` L653, `getHomePlanetEnglishName` L657,
`getFlagTracking` L1002. `timeLock`/`timeUnlock` are OTHER-bucket (only `observer.cpp:429,433`).

### 3.8 Command grammar — dead tokens and the reachable-but-defective set (base F)

**DEAD TOKENS** — declared keyword, no handler:

| token | decl | verdict |
|---|---|---|
| `ACP_FN_SKY_DRAW "sky_draw"` | `base_command_interface.hpp:476` | **the one genuinely orphaned flag name**: not in `m_flags`, and not on the obsolete list (`app_command_init.cpp:14-23`), so `flag sky_draw on` takes the unknown-name path |
| `external_mplayer` `:346`, `movetocity` `:360` | | covered by the obsolete list — deliberate, not a defect |
| `W_FALSE` `:244`, `W_OFF` `:188`, `W_TERMINATION` `:311`, `W_MINIMAL` `:316`, `W_REPLACE` `:317`, `W_RECURSIVE` `:318` | | never referenced anywhere in `src/` |
| duplicate `#define`s | `W_SIZE`(80,93), `W_INCREMENT`(86,145), `W_DECREMENT`(87,144), `W_LOOP`(169,284), `W_OGG`(266,280) | same spelling defined twice |

Correction to the census on `sky_draw` [re-verified at source by the recorder]: it is **not**
diagnostic-free. `AppCommandInterface::setFlag` (`:304-313`) misses in `m_flags`, runs
`searchSimilarFlag(name)` (the did-you-mean), returns false, and `commandFlag` then sets
*"Unrecognized or malformed flag argument"*. What IS defective there is one line: the
name-specific message at `:309` is **commented out** while `:310` still logs `debug_message` —
i.e. it prints an EMPTY (or stale, since the member is not cleared per command) string. A §2(f)
actionable-diagnostics instance, recorded not fixed (writing the message the §2(f) bar wants is
a small design act, not a one-liner).

**REACHABLE-BUT-DEFECTIVE** — the class my own residual (iii) predicted (*a caller exists ≠ the
callee does what its name says*). Each verified at source by the recorder:

| item | site | behaviour |
|---|---|---|
| `script speed faster\|slower\|default` | `:2421-2434` | applied the effect, then fell into an unconditional *"missing action argument"* ⇒ reported failure AND was dropped from recordings (`executeCommandStatus` skips `recordCommand` on failure). **FIXED 2026-07-25**, measured both ways — see §4 |
| `media subtitle toggle` | `:3329` | tests `argAction` where the correct sibling at `:3178` tests `argSubtitle` ⇒ `toggle` falls through `isTrue("toggle")==false` and turns subtitles **OFF**. One-word fix identified; **not taken** — no verification is available on this host (`~/.spacecrafter/videos` is empty and the subtitle state has no readout on the control surface) → **§5.36** |
| `set mode <v>` | `:1801` | `case APP_MODE: break;` — parses, reports success, calls nothing |
| `flag a on b on` | `:1122-1140` | reads only `args.begin()` of a `std::map` ⇒ silently applies the alphabetically FIRST pair only (the code's own comment: *"could loop if want to allow that syntax"*). Same single-pair restriction on `define`/`add`/`sub`/`multiply`/`divide`/`modulo`/`tangent`/`trunc`/`sinus` (`:4035-4149`) |
| `comment` / `uncomment` | `:214,217` | literal-compared before the table ⇒ absent from `m_commands` ⇒ the Levenshtein did-you-mean can never suggest them |
| command→string map | `m_commands_ToString` | built by `emplace(second, first)` ⇒ `"flyto"` is lost for the recording round-trip (`"camera"` wins the key) |
| dead branches | `:297` (struct case, intercepted at `:220`), `:3463` (`==W_PRESET \|\| ==W_PRESET`, textually identical disjunct) | unreachable by construction |

**Value-grammar note for every `flag` row above**: the value is `toggle | isTrue(v) | ELSE→OFF`
(`:1096-1102`) — **any typo silently means off**. The universal `default` value is accepted by
11 `set` names only.

## 4. Gap register (rows minted 2026-07-25, §11.108)

| row | class | content |
|---|---|---|
| **B33** | G-QUERY | ~~the query half of the control surface reads the OLD path while setters are dual (§3.2)~~ **CLOSED 2026-08-01** — heading §11.118(f), the other four §11.131; two of them (place, sky lock) had a REAL shipped divergence channel, not a latent one. Residues belong to other rows: the mount's write half → B35, `position save`/`load` → B34, the setter clamp asymmetry → §5.68 |
| **B34** | G-OLD-ONLY | ~~S6 residual dual-seam set: `body action clear`, `body action preload`, trail fresh-restart~~ **CLOSED 2026-08-02.** The three mechanical members + `position save`/`load` landed 2026-08-01 (F24, §11.132), each measured through its command on both binaries; the interactive VIEW and ZOOM ramps landed 2026-08-02 (F25, §11.133) — 24 terms tabulated, the two signs measured before they were written, per-step view deltas equal to **1.965e-08 rad** over 360 steps against bit-identical pre-fix, and no Vixy feel item owed (the one inexact term is a float32 pole quantum of **9.0e-08 rad**, predicted exactly). Residues, neither its own: `Core::panView` → **§5.71** (rides §5.66/§11.92(d)); the unreplayable recorded turn action → **§5.70** |
| **B35** | G-CONFIG-ONLY | capabilities with no runtime channel: mount switch (both paths, dead `toggleMountMode`), `boundToSurface`, new-path fov clamps, oort cloud colour |
| **B36** | G-UNREACHABLE | declared-but-undriven capabilities: the new-path set (§3.6) **plus the CoreLink ZERO set (14 of 330, §3.7)** — each needs a driver or a retirement. **Two members left the list 2026-08-01 (F24, §11.132)**: `ModularBody::preload` (with `BasicMesh`/`LayeredMesh`/`PhotosphereModule::preload`) and `TrailModule::startTrail`, both driven from the B34 seam mirrors — `Camera::multAlt` (F4) is the precedent. `CoreLink::startPlanetsTrails` stays a ZERO: its two live callers reach `SSystemFactory::startTrails` directly |
| **B38** | command-surface | dead tokens + the reachable-but-defective handlers (§3.8). One member fixed and measured (`script speed`); one blocked on verification (**§5.36**); the rest recorded |
| **B37** | G-UI-ONLY | interactive-only, unscriptable: pixel pick, mouse-drag look, continuous pan/zoom ramps, relative lon/lat/alt steps, TUI-only setters, TUI open, cursor control, debug dumps |
| **A38** | decision | reference-switch ROLL: new holds the whole orientation (B13/Q2), old re-derives the roll from its mount frame — measured 6.16°/6.49° divergence → DECISIONS_PENDING **D28** |
| **§5.35** | defect | `day_key_mode` is inert end-to-end (shadowed-local setter + no behavioural consumer) |
| **§5.32** | annotation | `Camera::descend` reads the reference's CACHED matrix — measured stale-use observable |
