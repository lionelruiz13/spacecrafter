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

Five independent bases, unioned — no single base is trusted to be complete, and the
overlap between them is what bounds the residual (the §11.73(a) pattern):

| base | what it enumerates | how | residual |
|---|---|---|---|
| **A** | the §9 seam table's 12 categories | read | §9 is itself a curated view (it names *examples* per row) ⇒ not a closed set |
| **B** | `coreModule/coreLink.hpp` public methods (the engine's own control-surface API) | declaration sweep + whole-`src` caller grep per method | overloads/name collisions; methods reached through `core->` directly bypass it |
| **C** | the new path's runtime-settable state: every public non-const method + every `static` flag under `src/experimentalModule/` (37+12+5+3+14+9 files) | header sweep + bare-symbol grep over `src/` | GLSL-side spec constants, `PipelineRegistry` internals, EntityCore |
| **D** | data-authorable behaviour: every `param["…"]` key read by a loader | `grep -rhoE '(param\|params)\["[a-z_0-9]+"\]'` over `moduleLoader/ orbitModules/ ModularBody.cpp ModularSystem.cpp` | old-path-only keys (`bodyModule/`) not swept here |
| **E** | the input surfaces: every key/mouse/TUI/joypad binding | `ui.cpp` (3447 lines) + `ui_tuiconf.cpp` + `joypad_controller.cpp` read end-to-end | TUI menus 7/8 line numbers approximate |

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
| continuous pan (hold) | — | **UI** | `core.cpp:1817-1826` → `navigation->updateMove` (old) + no Camera mirror; keys `ui.cpp:1091-1109` |
| mouse-drag look-around | `Camera::lookRel` | **UI** (dual) | `core.cpp:1739-1757` (`dragView` mirrors to Camera:1754) — *the only caller is `ui.cpp:364`* |
| pixel pick (click-select) | `ModularSystem::findBodyAt` | **UI** | `ui.cpp:479` → `core.cpp:1015-1029` → `ssystem_factory.cpp:642`; no command (§11.106(a)) |
| heading (absolute) | `Camera::setHeading` | CMD `set heading X` / `heading azimuth X [duration d]` | `coreLink.hpp:978-983` (dual) |
| heading (relative) | `Camera::moveHeading` | CMD `heading delta_azimuth d` + UI | `app_command_interface.cpp:2125-2132`; `coreLink.hpp:931` ← `ui.cpp:2562,2569,2737,2744` |
| sky lock | `Camera::setSkyLock` | CMD `flag lock_sky_position` | `app_command_interface.cpp:948-952` → `core.cpp:799-800` |
| fov | `Camera::setHalfFov` | CMD `zoom fov …` / `set fov` | `coreLink.hpp:311,327` |
| continuous zoom (hold) | — | **UI** | `core.cpp:1718-1726` → `projection->changeFov` (old only) |
| **mount ALTAZ↔EQUATORIAL** | `Camera::setMount` | **CFG** | `ssystem_factory.cpp:165` only; `Core::setMountMode`/`toggleMountMode` (`core.hpp:231,239`) have **zero callers in `src/`** ⇒ config-only on BOTH paths → **B35** |
| bound-to-surface | `Camera::setBoundToSurface` | **CFG** | `ssystem_factory.cpp:155` only → **B35** |
| fov clamp min/max | `Camera::minHalfFov/maxHalfFov` | **DEAD** (hard-coded) | `Camera.cpp:13-14`; `CoreLink::setMaxFov` reaches only `Projector` → **B35** |
| tracking on/off | `Camera::trackBody` | CMD `flag track_object` | `core.cpp:2146-2151` |
| view offset | `Camera::setViewOffset` | CMD `set zoom_offset v` (+ CFG) | `core.cpp:2181-2211` (§11.92) |
| offset ARMING | `Camera::armViewOffset` | CMD (indirect: look_at/zoom/track) | 4 of the **5** old arming sites mirrored; `gotoSelectedObject` is not → §11.102(b1) |

### 3.2 Camera/observer QUERIES — the G-QUERY class

Every getter below is on the control surface and reads the **old** path while its setter
is dual. Under the new render path these are readouts of a state that is not what draws.

| query | control-surface getter | reads | new-path value | evidence |
|---|---|---|---|---|
| heading | `CoreLink::getHeading` | old `Navigator` | `Camera::getHeading` — **0 external readers** | `coreLink.hpp:988-991`; MEASURED divergent: 6.16° (§11.108(c)) |
| view offset | `CoreLink::getViewOffset` | old `Navigator` | `Camera::getViewOffset` — 0 readers | `coreLink.hpp:972-975` |
| latitude / longitude / altitude | `CoreLink::observatoryGet*` | old `Observer` | `Camera::getLatitude` — 0 readers | `coreLink.hpp:784+` |
| mount | `Core::getMountMode` | old `Navigator` | `Camera::getMount` — 0 readers | `core.hpp:235-237` |
| sky lock | `Core::getFlagLockSkyPosition` | old `Navigator` | `Camera::getSkyLock` — 0 readers | `core.hpp:225-228` |
| selected object RA/DE, Alt/Az | `ModularObject` | new path | — | offset-skewed while armed: §11.102(b2) (SUSPENDED with §11.92(d)) |

`heading delta_azimuth d` **computes its target from the old getter** and writes it to
both (`app_command_interface.cpp:2125`), so it silently re-synchronises the two headings
— measured (§11.108(c)). → **B33**.

### 3.3 Bodies

| capability | reach | evidence |
|---|---|---|
| create / replace a body from any data key | CMD `body action load name X [replace true] …` | `app_command_interface.cpp:3560` → `ModularSystem::loadBody`, replace gate `ModularSystem.cpp:960` |
| drop a body | CMD `body action drop name X` | `ssystem_factory.hpp:698` (dual) |
| **drop script-added bodies** | CMD `body action clear` — **OLD ONLY** | `core.cpp:865` → `ssystem_factory.hpp:702-704` → `ProtoSystem::removeSupplementalBodies` → **B34** |
| **preload a body's resources** | CMD `body action preload` — **OLD ONLY** | `core.cpp:827-834` → `ssystem_factory.hpp:692-694` → old; `ModularBody::preload` (`ModularBody.hpp:1031`) has **0 callers** → **B34/B36** |
| reload the system from file | CMD `body action reload` | `ssystem_factory.cpp:649-660` (§11.55) |
| hide / show | CMD `body name X hidden on\|off\|toggle` (+ DATA `hidden`) | `ssystem_factory.hpp:315-317` |
| per-body colour (halo/label/orbit/trail/all) | CMD `body name X color <ch> value r,g,b` | `ssystem_factory.hpp:455-461` (§11.65) |
| skin texture create / switch | CMD `body name X skin_tex …` / `skin_use …` | `ssystem_factory.hpp:413,430` (§11.46) |
| per-body orbit line | CMD `body name X orbit on\|off` | `ssystem_factory.hpp:400` |
| **per-body trail** | no command (only the global focus filter) | `ModularBody::setFlagTrail` reachable only via `ssystem_factory.hpp:347` → **B37** |
| datum / ground radius | CMD `body name X datum_radius\|ground_radius <km>` | `ssystem_factory.cpp:755,764` (§11.71) |
| display scaling | CMD `planet_scale name X scale s`, `flag moon_scaled\|sun_scaled`, `set moon_scale\|sun_scale` | `ssystem_factory.hpp:573` (§11.45) |
| tesselation level | CMD `body tesselation <name> value v` | shared `BodyTesselation` (§11.26) |
| **trail fresh-restart** | CMD `flag object_trails` — reaches the OLD trail only | `coreLink.cpp:1145` → `ssystem_factory.hpp:235-237` → old; `TrailModule::startTrail` (`TrailModule.hpp:88`) has **0 callers** → **B34** |
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
| `ModularBody::preload` | `ModularBody.hpp:1031` | drags `BasicMesh::preload`, `LayeredMesh::preload`, `BodyModule::preload` |
| `ModularBody::pin` / `unpin` | `:1213` / `:1218` | the whole C2 work-domain pin mechanism, incl. `RenderChain::onPinDrained` |
| `ModularBody::findBodyNameI18n` | `:1159` (def `ModularBody.cpp:433`) | i18n lookup |
| `TrailModule::startTrail(bool)` | `TrailModule.hpp:88` (def `.cpp:109`) | the fresh-restart semantic |
| `Camera::multAlt` | `Camera.hpp:139` | **CLOSED by F4** (`core.cpp:1806`) |
| `ModularSystem::hasSystemFile` | `ModularSystem.hpp:56` | |
| `EnvironmentManager::getAtmosphereUserFlag` | `EnvironmentManager.hpp:66` | |
| `BasicMesh::invalidate()` | `BasicMesh.hpp:33` | **declared, never defined**, never called |
| `Core::setMountMode` / `toggleMountMode` | `core.hpp:231,239` | zero callers ⇒ the mount toggle is unreachable |
| `UI::handleKeysReleased` / `UI::handleJoyHat` | `ui.cpp:3348` / `:517` | dead duplicates of live handlers |

---

## 4. Gap register (rows minted 2026-07-25, §11.108)

| row | class | content |
|---|---|---|
| **B33** | G-QUERY | the query half of the control surface reads the OLD path while setters are dual (§3.2); `heading delta_azimuth` computes from the wrong authority — measured |
| **B34** | G-OLD-ONLY | S6 residual dual-seam set: `body action clear`, `body action preload`, trail fresh-restart |
| **B35** | G-CONFIG-ONLY | capabilities with no runtime channel: mount switch (both paths, dead `toggleMountMode`), `boundToSurface`, new-path fov clamps, oort cloud colour |
| **B36** | G-UNREACHABLE | declared-but-undriven new-path capabilities (§3.6) — each needs a driver or a retirement |
| **B37** | G-UI-ONLY | interactive-only, unscriptable: pixel pick, mouse-drag look, continuous pan/zoom ramps, relative lon/lat/alt steps, TUI-only setters, TUI open, cursor control, debug dumps |
| **A38** | decision | reference-switch ROLL: new holds the whole orientation (B13/Q2), old re-derives the roll from its mount frame — measured 6.16°/6.49° divergence → DECISIONS_PENDING **D28** |
| **§5.35** | defect | `day_key_mode` is inert end-to-end (shadowed-local setter + no behavioural consumer) |
| **§5.32** | annotation | `Camera::descend` reads the reference's CACHED matrix — measured stale-use observable |
