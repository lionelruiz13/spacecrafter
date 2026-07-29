# B31 — Exhaustive session save + persistent-body write-back: DESIGN PASS

**What this file is.** The design pass §13.B B31 demands before the row may be
dispatched (*"needs a design pass before dispatch, NOT dispatched blind"*,
§11.66(d)). It is an AUTHORITY FILE in the `capability-surface.md` sense: the
inventory and the design rules below are a living view that later waves
re-verify and extend, while `INTENT/11.110.md` (the journal entry that created
it) is append-only and carries the method and the measurements. On divergence
about a row's *state*, this file wins; on divergence about *what was measured on
2026-07-25*, §11.110 wins. Every claim carries provenance; re-verify at source
before acting (§5.2 class).

**ALL SEVEN DECISION POINTS ANSWERED — 2026-07-26 [vixy], propagated 2026-07-29
→ `INTENT/11.113.md` (i)–(o); B31 is now DISPATCHABLE.** The answers are recorded
in `DECISIONS_PENDING.md` D30–D36 with their propagation marks; §11.113 is the
record and **wins over this file** wherever the two differ (this file is derived).
Summary, and the sections each answer changes: **D30** hybrid — delta where an
authored value exists, snapshot where none does (*"for now"*, tester/user feedback
being its stated invalidation trigger) · **D31** `body action reload` is a LOAD
operation: it does NOT re-apply the ledger, the state-preserving route is
save-then-reload, and the file may equally be a hand edit ⇒ the §11.66(a)-vs-R8
conflict dissolves; the third verb of option (c) is not adopted (unneeded, cheap
later) · **D32** transients snap to their settled target, carve-outs as proposed —
and the file is also a DIAGNOSTIC artifact for user reports, so portability + the
manifest become requirements, not conveniences [derived] · **D33 §3.5** EXPLICIT
ONLY: no autosave, no autoload, no policy key — the operator's need is *recovery
to an established clean state* between audiences, so the artifact is a PRESET and
**load must be idempotent** (§10's first acting-default row RETIRES with it) ·
**D34 §4.2** the key is plain `englishName`; the system-qualified path survives
only inside the MISS REPORT; report-and-keep adopted; the non-body catalogue key
is NOT answered and stays an in-row design item · **D35 §5.4** composed files +
the session file ONLY — legacy `ssystem.ini` is read-only forever, and the
answer's revert clause is elevated to a domain constraint (**§2.0 D13**,
downgrade must stay possible) · **D36** declarative show state IN, time-bearing
show state OUT.

**Status: DESIGN ONLY — no product code was written or changed by the pass that
produced it.** Seven product decisions it cannot take are listed in §7 and
appended to `DECISIONS_PENDING.md` as **D30–D36**. ~~Implementation dispatches
only after Vixy reviews them; a dispatch taken before then would be improvising
user-visible semantics.~~ **THAT GATE IS MET (2026-07-26 answers, §11.113): all
seven are answered, so implementation may be dispatched — carrying the answers
above, not this file's pre-answer options.**

**Authority chain for everything below**: §11.66 (the mandate and the superseded
write-back contract), §2.0 D8 (the as-if rule — this feature IS D8 applied to
persistence, stated so in D8's own third bullet), §2.0 D9 (data is the product,
field frozen), §2.0 D12 (acting defaults are logged), §11.55(i) + §11.65 (the
measured reload behaviours this subsumes), §11.70(g) R8 (the operator's
colour-on-reload preference), §11.51(a) A32 (file location + ownership split +
the dual-use writer), §11.52(a) (atomic write, clauses 1–2 standing),
`capability-surface.md` (the channel model and what an operator can reach).

---

## 0. The mandate, decomposed into obligations

Verbatim authority [vixy 2026-07-22, §11.66]:

> *"the state save of spacecrafter must exhaustively save all the state so that
> saving, quitting and reloading is as-if we continued from the time we saved.
> For modular body systems, it's designed for transparent unloading/reloading of
> stellar systems and the possibility to push persistent bodies from script which
> then remains across sessions, to save resources - although it means dynamically
> rewriting the stellar system files without dropping malformed or unused fields
> and requires comment to be preserved and used automatically used to tell about
> errors inline and which default is used and which values are valid - better
> than logs because they land along with the erroneous data and help solving it
> locally."*

Decomposed, each obligation tagged with the section that answers it:

| # | obligation | answered in |
|---|---|---|
| O1 | exhaustively save ALL the state | §2 inventory |
| O2 | save→quit→reload is **as-if we continued** | §6 criterion + D32 (how deep "as-if" cuts) |
| O3 | transparent unload/reload of stellar systems (resource saving) | §2 group G + §3.3 |
| O4 | script-pushed **persistent bodies** surviving sessions | §4.1 (they become authored data) |
| O5 | rewriting stellar-system files **without dropping malformed or unused fields** | §5.2 (the writer rework) |
| O6 | **comments preserved** | §5.2 |
| O7 | comments **used automatically** for inline error/default/valid-value annotation | §5.3 |
| O8 | annotation lands **next to the erroneous datum**, not only in the log | §5.3 (and: it must be ABOVE it — §5.3(c), source-forced) |
| O9 | atomic sibling-temp-then-rename (§11.52(a) clauses 1–2, standing) | §5.1 — already implemented, unchanged |
| O10 | ONE serialization authority (I2; §11.66(d) says so explicitly) | §3.1 |

Two riders the mandate does not state but the ledger attaches to this row:

* **R8** [tester, §11.70(g)]: *"It must reset to the body default or color asked
  in the script. The user must reload the rule to be applied to it as well."* —
  the "rule" is what this ledger persists. It also **collides** with §11.66(a)
  for the in-session `body action reload` case → **D31**.
* **R13** [tester, §11.70(k)]: the anchor must be memorised **per mode** and
  restored on return. Whoever does §6.9/B31 inherits it. Nothing to persist yet
  (the per-mode memory does not exist) — §2 group B row B17.

---

## 1. Enumeration bases, and their residuals

No single base is trusted; the union is what bounds the residual (the §11.73(a)
/ §11.108 §2 pattern). Where a base is incomplete the rows it would have
produced are **missing, not wrong**, and that is said in its own row.

| base | what it enumerates | how | state / residual |
|---|---|---|---|
| **A** | the new path's own observer/camera state | `Camera.hpp:353-426` private members read in full + `Camera::dumpTrace` (`Camera.cpp:883-928`) | DONE. The dump is a curated subset; the member list is the closed set for this class |
| **B** | everything an operator can change at runtime | `capability-surface.md` §3 (the F4 audit) — 97 `flag`, 43 `set`, 46 `color`, the per-body command set, the application-level set | DONE by import. Residual is F4's own: it establishes that a caller exists, not that the callee does what its name says (§5.35 is the standing example) |
| **C** | what the project already considered worth persisting | `App::saveCurrentConfig` (`app.cpp:907-941`), `Core::saveCurrentConfig` (`core.cpp:1494-1580`), `UI::saveCurrentConfig` (`ui.cpp:332-341`), `Observer::setConf` (`observer.cpp:143-153`) | DONE. This base is a *view of the past*, not a specification — it reads OLD-path authorities throughout (the B33 G-QUERY class) |
| **D** | the existing in-memory bookmark | `BackupWorkspace` (`backup_mgr.hpp:45-56`), `CoreBackup::saveBackup/loadBackup` (`backup_mgr.cpp:45-73`) | DONE, and it carries a finding: `heading` and `observer_vision` are DECLARED in the struct and **never written** by `saveBackup()` — dead fields, i.e. a state item someone intended to bookmark and did not |
| **E** | the new path's per-body mutable state | `ModularBody.hpp` private members written by runtime setters (`:1504` relation, `:1508` hiddenBodies, `:1609` haloColor, `:1619` scaling, `:1623-1624` datum/ground) + the module-owned colour members (§11.65(a)) | DONE for the classes the command surface reaches. **Residual**: module-internal state with no command (trail point history is the one that matters — row D10) |
| **F** | manager-owned runtime state outside the body tree | header sweep of `ScriptMgr` (`script_mgr.hpp:172-198`), `Media` (`media.hpp:496-518`), `TimeMgr` (`time_mgr.hpp:112-119`), `AnchorManager`, `EnvironmentManager` | **PARTIAL — the largest residual.** Sky/star/nebula/Tully/DSO managers were enumerated only through base B (their command surface), not through their members. Rows that would come from a member sweep are missing |

**What the union does NOT cover, stated so the claim is falsifiable**: (i) old-path
internal state with no control surface (it retires with the old path, §12) —
excluded by construction, not by omission; (ii) `EntityCore` (submodule,
read-only); (iii) GPU-side state (pipelines, caches) — derived by construction;
(iv) the *effectiveness* of a restore path (this design establishes what must be
carried, not that the carrier works — that is §6's job).

---

## 2. State inventory

Classification: **MUST-SAVE** · **DERIVED** (reconstructable at load from
something else that is saved) · **EXCLUDED** (with the decision that excludes it)
· **DECISION** (a product choice, listed in §7).

Counts, by group (A 6 · B 19 · C 5 · D 11 · E 6 · F 4 · G 8 · H 6 · I 4 · J 3 ·
K 3 = **75 rows**): **39 MUST-SAVE · 8 DERIVED · 12 EXCLUDED · 16 DECISION**.
Rows carrying both a class and a rider (B10, D2) are counted once, by their
primary class.

### A — Time

| # | state | owning authority | class | note |
|---|---|---|---|---|
| A1 | simulation date `JDay` | `time_mgr.hpp:113` | MUST-SAVE | the single most load-bearing scalar; see §3.4 on precision |
| A2 | time rate `time_speed` | `:112` | MUST-SAVE | |
| A3 | `FlagTimePause` | `:114` | MUST-SAVE | |
| A4 | `timeLockCount` | `:116` | DECISION | a lock held by a script at save time; rides **D36** |
| A5 | in-flight time-speed ramp (`start_time_speed`/`end_time_speed`/`move_to_coef`/`move_to_mult`, `FlagChangeTimeSpeed`) | `:115,117-119` | DECISION | rides **D32** |
| A6 | timezone, date/time display format, `startup_time_mode`, `preset_sky_time` | `app.cpp:918-930` | DERIVED | already persisted by the config channel; a session inherits them |

### B — Observer / camera (the NEW path is the drawn authority)

| # | state | owning authority | class | note |
|---|---|---|---|---|
| B1 | reference body | `Camera.hpp:359` (`ModularBodyPtr`) | MUST-SAVE | by identity key, §4.2 |
| B2 | tracked body (`target`) | `:360` | MUST-SAVE | measured re-seatable by name across a rebuild (§11.55(e)) |
| B3 | longitude / latitude / distance | `:415-417` | MUST-SAVE | `distance` in AU |
| B4 | `position` / `deltaPosition` / `moveDuration` | `:404-406` | DECISION | an in-flight `moveTo`; rides **D32** |
| B5 | `alt` / `az` | `:378-379` | MUST-SAVE | |
| B6 | `heading` | `:380` | MUST-SAVE | **what it MEANS across a reference change is D28-pending** (§11.108(c)) — dependency, not decided here |
| B7 | `foldLat` | `:387` | DERIVED | re-derived by `update()` from latitude |
| B8 | `mount` (ALTAZ/EQUATORIAL) | `:388` | MUST-SAVE | CFG-only today (B35); saving it does not create the runtime channel |
| B9 | `skyLocked` + `lockedSkyRot` | `:393-394` | MUST-SAVE | the held matrix is state, not derivable — it is the rotation captured when the lock engaged |
| B10 | `viewOffset` / `viewOffsetTransition` / `viewOffsetArmed` | `:401-403` | MUST-SAVE (scalar + armed) / DECISION (transition) | the arming latch is sticky state; the ramp rides **D32** |
| B11 | `halfFov` | `ModularBody.hpp:1186` (static) | MUST-SAVE | |
| B12 | zoom plan (`zoomDuration`/`zoomTimer`/`srcHalfFov`/`dstHalfFov`) | `Camera.hpp:418-421` | DECISION | rides **D32** |
| B13 | `cullHalfFov` | `ModularBody.hpp:1196` | DERIVED | |
| B14 | `freeMode` | `Camera.hpp:424` | MUST-SAVE | |
| B15 | `boundToSurface` | `:425` | MUST-SAVE | CFG-only today (B35); note §5.32 — it is a persistent default, not an altitude invariant |
| B16 | view-smoothing plan (`viewFrom`/`viewAxis`/`viewAngle`/`viewT1`/`viewT`/`viewTimer`/`viewV0`/`viewA`) and the heading plan (`hdg*`) | `:367-377` | DECISION | rides **D32**. NB tracking re-plans every frame (`Camera.cpp:264-266`) so this has no exact fixed point (§11.55(d)) |
| B17 | per-MODE anchor memory (R13) | **does not exist** | DECISION | R13 mandates it; nothing to serialize until §6.9/B20 builds it. Dependency, recorded so it is not lost |
| B18 | `minHalfFov`/`maxHalfFov` | `:422-423`, `Camera.cpp:13-14` | EXCLUDED | hard-coded, no channel (B35) — nothing to save |
| B19 | old-path `Observer`/`Navigator` twin state | `navModule/` | EXCLUDED | old retires (§12); setters are dual so it follows. **But**: it is what the CONFIG channel currently reads (base C) — see §3.4 |

### C — Selection / tracking

| # | state | owning authority | class | note |
|---|---|---|---|---|
| C1 | new-path selection | `ModularBody::selectedBody` (`ModularBody.hpp:1746`) | MUST-SAVE | by identity key |
| C2 | old-path `Core::selected_object` | `core.cpp` | DERIVED | re-established by replaying the selection through the ONE routing seam (`SSystemFactory::searchObjectByEnglishName`, §11.106(c)) |
| C3 | the `ModularObject` bridge instance | §11.106(h) | DERIVED | rebuilt; refcount-owned (and §5.34's leak is pre-existing, untouched) |
| C4 | selection of a NON-body (star / nebula / DSO) | old trees | DECISION | needs its own identity key (catalogue id) — the A30(iii) class; rides **D34** |
| C5 | `flag track_object` | command surface | MUST-SAVE | one of the 97; listed here because it is meaningless without C1 |

### D — Per-body runtime overrides (the B16/B29 ledger — §11.66(d) says B31 subsumes it)

| # | state | owning authority | class | note |
|---|---|---|---|---|
| D1 | hidden / shown | `relation` + `hiddenBodies` (`ModularBody.hpp:1504,1508`) | MUST-SAVE | measured reset-by-reload (§11.55(i): `relation` 1→4) |
| D2 | per-body display `scaling` | `:1619` (`ASmooth<…,5.f>`) | MUST-SAVE | measured reset-by-reload (5.0000 → ×1, §11.55(i)). Save the TARGET or the in-ramp value? → **D32** |
| D3 | halo colour | `:1609` | MUST-SAVE | measured reset-by-reload (§11.65(e)); R8 tension → **D31** |
| D4 | label / orbit / trail colour | `HintModule`/`OrbitModule`/`TrailModule` members (§11.65(a)) | MUST-SAVE | same |
| D5 | runtime colour DEFAULTS (module statics + `defaultHaloColor` `:1709`) | §11.65(b) | MUST-SAVE | they change what FUTURE bodies get — invisible in a per-body snapshot |
| D6 | `datumRadius` / `groundRadius` | `:1623-1624` | MUST-SAVE | §11.71 |
| D7 | skin textures created + selected (`skin_tex`/`skin_use`) | §11.46 | MUST-SAVE | a *created* skin is content (a texture path), not a scalar — it belongs in the content half, §3.2 |
| D8 | per-body orbit-line flag, per-body trail flag | `ssystem_factory.hpp:400,347` | MUST-SAVE | the trail one has no command (B37) — saving it does not create the channel |
| D9 | tesselation level | shared `BodyTesselation` (§11.26) | MUST-SAVE | dual-by-shared-object |
| D10 | **accumulated trail points** | `TrailModule` history | DECISION | a trail is accumulated *observable content*: "as-if continued" says the drawn trail is still there; a re-derivation from jd is possible only for closed-form orbits. Rides **D32** |
| D11 | old-path body overrides | `BodyColor`, `Body` | EXCLUDED | old retires; measured to persist where new resets (§11.65(d)) — the desync is recorded, not carried |

### E — Global display / flags / values

| # | state | owning authority | class | note |
|---|---|---|---|---|
| E1 | `moon_scale` / `sun_scale` + `flag_moon_scaled` / `flag_sun_scaled` | `ssystem_factory.hpp:556`, `core.cpp:1505-1506,1531-1532` | MUST-SAVE | **carries the §11.101(f) load-ORDER hazard** — see §6 T3 |
| E2 | `planet_scale name X` | `ssystem_factory.hpp:573` | MUST-SAVE | rides D2 |
| E3 | the **97** `flag` values | `app_command_init.cpp:112-208` | MUST-SAVE (bulk) | **measured**: all 97 `case FLAG_NAMES::` blocks contain an `FV_TOGGLE` branch (100 occurrences over 97 cases) ⇒ every flag has a live read of its own current value. Readback exists for the whole surface |
| E4 | the **43** `set` values | `:281-324` | MUST-SAVE (bulk), minus A6-class preferences | readback NOT established (`set mode` is `case APP_MODE: break;`, §11.108(k)) — an implementation-phase enumeration, and the design does not depend on it (§3.4: the save reads the MODEL, not the command surface) |
| E5 | the **46** `color` values | `m_color` (F4 census) | MUST-SAVE (bulk) | the per-body half is D3/D4 |
| E6 | `flag experimental_path` / `experimental_shadows` | §11.50(c) | EXCLUDED | experiment gates that retire with the old path (§11.50(a)); saving them would persist a dev switch into a show artefact |

### F — Environment

| # | state | owning authority | class | note |
|---|---|---|---|---|
| F1 | landscape selection **and whether it was auto-selected or manually pinned** | `Core::setLandscape` / `setLandscapeToBody` (`core.cpp:649-700`) | MUST-SAVE | R9/A27: the auto-rule is engine built-in, a hand/script pick **sticks until the next landing or a script change** ⇒ the *pin* is state, the auto-choice is DERIVED. Saving only the name would silently convert a pin into a coincidence |
| F2 | atmosphere / fog flags | rides E3 | MUST-SAVE | |
| F3 | sky culture, sky locale, srt locale, app locale | `core.cpp:1497-1498`, `app.cpp:917` | MUST-SAVE | all four are runtime-settable (`set sky_culture` …) yet live in config today — a session that changes one must carry it |
| F4 | `EnvironmentManager::activeChain` / `lastReference` | `EnvironmentManager.cpp:31-37,136` | DERIVED | rebuilt from the reference chain; I5-notified since §11.55(f) |

### G — Loaded content

| # | state | owning authority | class | note |
|---|---|---|---|---|
| G1 | which system files are loaded, and by which reader | `ModularSystem.hpp:209-211` (`systemFilename` + composed/legacy discriminator) | MUST-SAVE | O3's carrier: a system's residency is state |
| G2 | **script-added bodies** absent from every file | `ModularSystem::loadBody` (runtime `body action load`) | MUST-SAVE **as content** | this is O4; §4.1 |
| G3 | file-declared bodies DROPPED at runtime (`body action drop`, `body action clear`) | `ssystem_factory.hpp:698,702` | MUST-SAVE **as a negative delta** | a restore that re-loads the file resurrects them otherwise — an as-if failure that a positive-only snapshot cannot see |
| G4 | file-declared bodies RE-AUTHORED at runtime (`body action load … replace true`) | `ModularSystem.cpp:960` | MUST-SAVE as a delta | the capability-surface's own "not a gap" escape hatch (§1 note) — it mutates declared data at runtime |
| G5 | adopted composed twins (`modularSystem/X.ini`) | A32 ownership split | DERIVED | the FILE is the state; the session references it, never copies it |
| G6 | catalogs loaded at runtime (`galaxy_stars load`, `star_navigator save/load`, dso/tully loads) | `coreLink` | DECISION | rides **D36** |
| G7 | galactic / foreign star systems | `SSystemFactory::addSystem` | EXCLUDED-pending | the whole surface is measured DEAD (§5.37); revives with **D29** and then joins G1 |
| G8 | camera **anchors** | `AnchorManager` | EXCLUDED | **[Q5: no cross-session persistence]** — recorded as the boundary, not as an omission. `anchor.ini` remains the authoring channel |

### H — Media / presentation

| # | state | owning authority | class | note |
|---|---|---|---|---|
| H1 | images (position, fade, projection) | `ImageMgr` (`media.hpp:497`) | DECISION | rides **D36** |
| H2 | video + audio playback and position | `VideoPlayer`/`Audio` (`:496,498`), `m_videoState` (`:518`) | DECISION | **D36** |
| H3 | on-dome text | `TextMgr` (`:501`) | DECISION | **D36** |
| H4 | subtitles | `Subtitle` (`:502`) | DECISION | **D36**; note §5.36 (`media subtitle toggle` is broken today) |
| H5 | VR360 / viewport / `dualViewport` | `:499,500,508` | DECISION | **D36** |
| H6 | screen fader | `set screen_fader` | MUST-SAVE | rides E4; a session restored mid-fade to black is a black dome |

### I — Script engine

| # | state | owning authority | class | note |
|---|---|---|---|---|
| I1 | playing script + queue position + `wait_time` + loop state + `multiplierRate` | `script_mgr.hpp:172-198` | DECISION | **D36** — the whole "is a running show part of the session?" question |
| I2 | script RECORDING state + its open `fstream` | `ScriptRecord` (`:176-180`) | EXCLUDED (recommended) | a recording is an output artefact being produced, not state being observed. Restoring it would mean re-opening someone's file for append |
| I3 | script variables (`define`, `AppEval`) | `appEval` | DECISION | **D36** |
| I4 | `global_lock_count`, `flagSkipPause`, `waitOnVideo` | `:188-198` | DERIVED with I1 | meaningless without I1 |

### J — Interactive / UI-only

| # | state | owning authority | class | note |
|---|---|---|---|---|
| J1 | mouse cursor visibility + timeout, TUI open/position | `ui.cpp:2912-2917,1151` | EXCLUDED (recommended) | capability-surface B37 class: interactive-only, never part of a show. Recorded rather than dropped |
| J2 | `day_key_mode` | `app.hpp:129-130` | EXCLUDED | **§5.35: inert end to end** — saving it would persist a value that does nothing |
| J3 | TUI display flags (`flag show_latlon`, tui datetime …) | rides E3 | MUST-SAVE | they are flags; no special treatment |

### K — Application / venue

| # | state | owning authority | class | note |
|---|---|---|---|---|
| K1 | projection mode (fisheye / fullsphere / …) | config | EXCLUDED (recommended) | [vixy, D6 §11.79(c)]: *"The projection modes are matching the physical medium/lens"* ⇒ a venue property, not session state. A session file carried between venues must not re-aim the lens |
| K2 | window / swapchain size, screen count | config | EXCLUDED | venue property, same argument |
| K3 | `set mode` | `app_command_interface.cpp:1801` | EXCLUDED | `case APP_MODE: break;` — parses, reports success, calls nothing (§11.108(k)) |

---

## 3. Save format and trigger

### 3.1 One serialization authority (O10, I2)

`ModularSystemFormat` is already declared to be it — *"This is the ONE
serialization authority (INTENT §11.51(a): dual-use writer, generation at load
AND future script-triggered save go through here)"* [observed:
`ModularSystemFormat.hpp:55-56`]. Three surfaces must share it and this design
does not add a fourth:

1. **B25's machine twin** — `generateComposedTwin` (`ModularSystem.cpp:1486`),
   live since §11.78(f).
2. **B25's remaining half** — script-triggered on-the-fly system save; its
   missing piece was named as *"the live-tree serialization source"*, which is
   exactly what §4.1 below specifies.
3. **B31** — persistent bodies (content) and the session file.

The writer needs the rework of §5 before (2) and (3) may use it on user files.
Until then it is only safe on machine-owned files it regenerates wholesale, which
is precisely the scope it has today.

### 3.2 TWO artifacts, not one

| artifact | holds | owner | lives in |
|---|---|---|---|
| **content files** | persistent bodies (G2), re-authored/dropped declarations (G3/G4) | the USER (A32: extension-dropped `.ini` is user-owned, the generator never touches it) | `~/.spacecrafter/modularSystem/<System>.ini` |
| **session file** | everything else in §2 — time, observer, selection, flags, the override ledger — plus a MANIFEST naming the content files the session needs | machine-owned, disposable | `~/.spacecrafter/sessions/<name>.ini` (new directory; nothing to bootstrap — absence means "no session", so `CallSystem::checkIniFiles` (`call_system.cpp:81-95`) needs no new entry) |

**Why split.** Content is *data*: D9 product surface, hand-editable, shipped in
deliveries, and the thing a paid delivery replaces. A session is a *bookmark*:
disposable, machine-written, meaningless a version later. Merging them has two
costs the split avoids — (i) every session save would rewrite paid data (a
D9-destruction surface opened at the highest-frequency operation there is), and
(ii) a session file would become a data delivery, so "restore my session" would
have to obey the frozen-field rule. Splitting also puts the two halves under the
right ownership: content in the user-owned `.ini` Vixy already specified
(*"targeting without the .disabled or under a different name from scripts"*,
§11.51(a)), the bookmark in a machine-owned place.

### 3.3 What the manifest is for (O3)

A session records, per loaded system: its file, its reader (legacy vs composed),
and whether it was resident. That is the same datum a *transparent in-session
unload/reload* needs — which is O3's resource-saving mechanism and its
cross-session twin serialized by the same code (I2). Concretely: unloading a
stellar system means writing its live state to the same session representation,
dropping the tree, and rebuilding from it on demand; the cross-session case is
the identical operation with a process boundary in the middle. Designing them
apart would produce two authorities for one question.

### 3.4 Rejected alternatives, with their reasons

**(a) config.ini as the vehicle — REJECTED, and the reason is measured.**
`configuration action save` already exists (`app_command_interface.cpp:1915-1917`
→ `App::saveCurrentConfig`, `app.cpp:907-941`) and writes a subset of state to
`~/.spacecrafter/config.ini` through `InitParser::save` → `iniparser_dump_ini`.
Measured on a temp-HOME farm copy, 2026-07-25 (real tree md5 `03fbee59` /
`545a51ef` in and out; predictions committed before the run, all matched):

| observation | before | after `configuration action save` |
|---|---|---|
| comment lines (`#`/`;`), file-level and in-section | 3 | **0** |
| inline trailing comment on a value | `texture_caching = true   ; F6 …` | `texture_caching = true` |
| unknown key in a known section | `f6_unknown_key = keepme` | preserved |
| entirely unknown section | `[f6_unknown_section]` | preserved |
| mixed-case key | `F6_MixedCase_Key` | **`f6_mixedcase_key`** (value case kept) |
| keys the user never authored | — | **+9 materialized** (`nautical_alt`, `nautical_ra`, `object_coordinates`, `mouse_coordinates`, `angular_distance`, `loxodromy`, `orthodromy`, `vertical_line`, `flag_star_pick`) |
| file length | 325 lines | 330 lines |

Two more properties, source-certain and deliberately not probed: `InitParser::save`
opens the **target itself** with `fopen(…, "wt")` (`init_parser.cpp:79`) — truncate
in place, no temp, no rename ⇒ **non-atomic**; and `iniparser_load` returns NULL if
any line is a syntax error, whereupon `InitParser::load` calls **`exit(-1)`**
(`iniparser.c:812-818`, `init_parser.cpp:62-72`) ⇒ one malformed line in
`config.ini` kills the app at startup. So the config channel is
protect-by-abstention taken to refusal on read, and unconditional destruction on
write — the exact inverse of the §11.66(b) contract, on a user-editable file.
It is not a foundation to build on, and it is not the place to put session state.
(That the writer behaves this way on `config.ini` today is a **pre-existing
finding**, recorded in §11.110 and proposed as **§5.42**; fixing it is not B31's
scope, but B31 must not extend its reach.)

**(b) A `.sts` command journal replayed at load — REJECTED.** (i) Replay is not
as-if: durations, eased ramps and the every-frame tracking re-plan
(`Camera.cpp:264-266`) make the end state a function of *when* each command lands,
and §11.55(d) measured that tracking has no exact fixed point. (ii) Commands are
not idempotent and some are irreversible (`body action clear`). (iii) The journal
grows without bound across a long show. (iv) The channel already exists for its
right purpose — `script action record` — and a second consumer would make one
artefact serve authoring and restoration, which are different contracts. Keep
recording as authoring.

**(c) A binary blob — REJECTED.** D9 makes hand-editability and inspectability
first-class, and §11.66(b)'s whole point is that a *comment next to the datum*
beats a log. Neither survives a blob.

**(d) Build it on the existing bookmark or the existing camera-save — REJECTED,
both measured deficient.**
`position action save/load` (`app_command_interface.cpp:2829-2836` →
`CoreBackup`) is a single in-memory slot holding jd, lat/lon/alt, fov,
home-planet and module (`backup_mgr.cpp:63-73`); its struct declares `heading`
and `observer_vision` and **never writes them** (`backup_mgr.hpp:52,51`).
`camera action save/load` (`app_command_interface.cpp:3889-3916`) is worse and
was measured, 2026-07-25, predictions first:

* `CoreLink::cameraSave` prepends `"anchors/"` (`coreLink.cpp:57-59`) and
  `SSystemFactory::cameraSave` prepends `"anchors/"` **again**
  (`ssystem_factory.hpp:871-873`) ⇒ the target is `anchors/anchors/<name>`.
  Ladder, one variable at a time: no `anchors/` → FAILS; `anchors/` created →
  **still FAILS**; `anchors/anchors/` created → **succeeds**. Two log lines name
  the doubled path verbatim. `~/.spacecrafter/anchors` does not exist on this
  install, so **the command cannot succeed as shipped**. Same doubling on load.
* The file it writes is old-path-only (`anchor_manager.cpp:696-739`): name/type,
  `alt`/`lon`/`lat` from `Observer`, `vx/vy/vz` from `Navigator::getLocalVision`,
  `time` from `TimeMgr` — no heading, no fov, no mount, no reference, no
  selection, no new-path term at all.
* It writes with **default ostream precision**: the session's JD 2461247.34443
  came back as `time 2.46125e+06` = 2461250, a **2.66-day error**, and the
  round-trip resolution of the date is **10 days**.

Proposed **§5.41** for the double prefix + the precision loss. The design
consequence: neither surface may become the session authority; both should be
re-expressed on top of it when B31 lands (`position action save` = the same
serializer targeting memory), which is the I2 form.

**(e) Read the state through the control-surface getters — REJECTED, and this is
the sharpest rule in the design.** **The save must read the model that draws.**
`capability-surface.md` §3.2 (B33) measured the getter half of the control
surface reading the OLD path while its setters are dual — heading measured
divergent by **6.16°** (§11.108(c)). Base C shows the existing config save doing
exactly that: `Observer::setConf` (`observer.cpp:143-153`) writes the OLD
observer's lat/lon/alt into `[init_location]`, and `Core::saveCurrentConfig`
reads old-path managers throughout. A save built on that surface persists a
state that is not what was on the dome. Rule: every save-side read resolves to
the owning authority named in §2's "owning authority" column, never to a
`CoreLink` getter.

### 3.5 Trigger

**ANSWERED [vixy 2026-07-26, D33 → §11.113(l)]: EXPLICIT ONLY** — no autosave at quit, no autoload at start, no config key for the policy (*"user want control and especially the ability to recover to an established clean state, most often used between sessions each time with new public"*). The explicit commands below stand as designed; the opt-in autosave key and everything downstream of it are retired, and the artifact's role is a PRESET loaded repeatedly, not a resume point ⇒ **load must be idempotent**.

* **Explicit command** `session action save [filename <name>]` /
  `session action load filename <name>` — one registration serves all of
  channels 1–5 (§11.55(h)), so §2(c) is satisfied without a second artefact.
  Spelling follows the `<noun> action <verb>` grammar the surface already uses
  (`body action reload`, `camera action save`, `position action save`) — it is a
  sibling, not a new grammar branch (the §11.55(a) argument).
* **Config channel does NOT apply to the act** (§11.55(h)'s own rule: nobody
  chooses "save" at startup). It DOES apply to the *policy* — see D33.
* **Autosave / autoload is an ACTING DEFAULT (D12)** and changes what launching
  the app means. Not taken by this design → **D33**. The executor recommendation
  there is: no implicit save, an opt-in config key, and a log line on every
  automatic save or restore.
* **The save is a USE-SITE under D8** — see §6.1. It therefore runs at a frame
  boundary as a task, and the file I/O happens off the draw thread (C3, D11):
  the atomic rename means a partially written file is unobservable, so the write
  need not be synchronous with anything.

---

## 4. Cross-session identity

### 4.1 Persistent bodies (O4) — no new identity key is needed

A script-pushed body that must survive the session **becomes authored data**.
`session action save` (or B25's script-triggered system save, the same code)
serializes it as a section in a composed system file under
`~/.spacecrafter/modularSystem/`; from the next launch it is an ordinary declared
body loaded by the ordinary loader, and its identity is what every authored
body's identity already is. This is Vixy's own stated route [§11.51(a)]:
*"A feature which may also be used to save a system on-the-fly as well by
targeting without the .disabled or under a different name from scripts, just in
case."*

This is the smallest structure that carries the requirement (I6: if the structure
does not carry what is asked, rework at the root — here the structure *already*
carries it). Consequences, stated rather than discovered later:

* The **live-tree serialization source** B25's remaining half was waiting for is
  the same function: walk the tree, emit each body's declared parameters plus its
  live capability keys. `generateComposedTwin` is 80 % of it already; what it
  lacks is a source for bodies with **no legacy section** (a runtime-created body
  has no `stringHash_t` to copy). That gap is real work, not a decision: the
  parameters must come from the loaders that consumed them, which is why
  `saveOrbit()` exists on every orbit class (`orbit.hpp:70`) and why
  `SurfacePointOrbitLoader::saveOrbit` (`SurfacePointOrbitLoader.hpp:47-60`)
  round-trips **data keys, not derived state** — that is the pattern the rest
  must follow.
* A body pushed twice with the same name collides exactly as two authored bodies
  do (`loadBody`'s skip, `ModularSystem.cpp:963`); §11.109(h3) recorded that as a
  product question and it is unchanged by this design.
* Ownership: the target file is USER-owned (A32), so writing it is a write-back
  into user data ⇒ it is governed by §5 and by **D35**.

### 4.2 The override ledger DOES need a key — the A29 hazard

**ANSWERED [vixy 2026-07-26, D34 → §11.113(m)]: option (i), plain `englishName`** — on the answer's own two grounds, the identifier users already know and uniqueness ENFORCED internally (global registry + duplicate-skip). Option (ii)'s single structural advantage (breaking DETECTABLY) is kept without its cost by putting the system-qualified tree path in the **miss report**, never in the key. Report-and-keep on an unresolved key is uncontradicted ⇒ adopted. The A29 hazard's residue under (i) is precisely §5.40's failure mode — a restore must REPORT a miss rather than bind to a same-named body another system loaded — and that report is a gate the implementation owes. [Veto point: if the answer described expectation rather than the choice, this reverses at near-zero cost — machine-written file, nothing fielded.] The folded **non-body selection key** (star/nebula catalogue identifier) is NOT answered and stays an in-row design item.

The ledger (§2 group D) references bodies it does not own. Keying it by
`englishName` re-imports exactly the hazard A29 was converted to close (§5.5,
"hardcoding keyed on englishName"), and A30(iii) already named the same trap for
the file↔star key: *"name-matching would re-import the A29 identity-keyed hazard
into a new place, decide before the first file exists."* Options and costs:

| option | resolves | cost |
|---|---|---|
| (i) `englishName` alone | nothing | the name registry is GLOBAL (`ModularBody.cpp:18`), so two systems can hold one name and the second silently loses the body (`loadBody` skip, measured §11.109(c)). An override then lands on a foreign body. A data delivery that renames a body silently drops the override |
| (ii) **system-qualified tree path** — `<systemFile>::<parentChain>/<name>` | the cross-system collision, and it states *which* tree the override belongs to (the same authority `isInSubtreeOf` gave the twin, §11.109(c), I4) | still name-fragile *within* a system: a rename in a data delivery breaks the entry — but breaks it **detectably** (the path fails to resolve) rather than silently |
| (iii) minted stable id (`uid = …` written into the data at first save) | renames too | a NEW product-surface key on paid data (D9); it must be written into the user's file at save time — a write-back the user did not ask for; and legacy files have none, so identity becomes two-regime |
| (iv) content hash | nothing useful | any authored edit changes identity — the exact opposite of what the ledger wants, since surviving a data correction IS the D9 forward-propagation case |

**Executor recommendation: (ii)**, because it needs no data change, no new
product surface, and no decision about legacy files — and it is a prefix-
compatible stepping stone to (iii) if renames ever become common enough to pay
for a minted id. Recorded as **D34** because it is a product choice, not an
implementation one.

**The failure semantics are the load-bearing half.** An override whose key does
not resolve at restore must be **reported, never silently dropped and never
applied to a near match** (§2(f) + D12): keep the entry in the session file,
annotate it in place with the reason and the valid alternatives (§5.3 — the
session file is written by the same writer, so it gets the same inline
annotation), and log once per entry. A silently dropped override is a show that
looks wrong with no trace of why, which is the class §2(f) exists to forbid.

**C4 (star/nebula selection) inherits the same question** at a different scale —
its key is a catalogue identifier (HIP-class), which is A30(iii)'s own answer
shape. Not decided here; folded into D34 so the two cannot diverge.

---

## 5. Write-back under §11.66(b)

### 5.1 What already holds, unchanged

Clauses 1 and 2 of §11.52(a) STAND (§11.66(b) says so explicitly) and are
implemented: `ModularSystemFormat::write` writes a sibling temp in the target's
own directory and renames over the target; on any failure it removes the temp and
leaves the original untouched, with a §2(f)-shaped diagnostic
[observed: `ModularSystemFormat.cpp:63-97`]. Nothing here changes that, and
§11.109(e) verified it end-to-end on a galactic corpus. Clause 1
("only when needed") for a session save means: the operator asked; for a content
file it means: byte-compare the serialization against the file and skip the
rename when equal (the twin path already uses byte-identity as its own check,
§11.109(e)).

### 5.2 What must change — the parse/write layer becomes line-preserving

Measured gap against the (b) contract, at source:

| (b) clause | current behaviour | verdict |
|---|---|---|
| preserve **comments** | `parse` drops every `#` line (`ModularSystemFormat.cpp:35`); `write` emits only the banner (`:74-75`) | **NOT compliant** |
| preserve **malformed** fields | `parse` drops any line without `=` (`:46-47`) — this is exactly `[Sedna]`'s `orbit_LongOfPericenter 95.58754` (§5.39) | **NOT compliant** |
| preserve **unused** fields | unknown keys land in `params` and are re-emitted | **compliant** [measured §11.109(e): an unknown `note` key with high bytes round-tripped byte-for-byte] |
| preserve byte layout | `Section::params` is `stringHash_t` = `std::map` (`utility.hpp:85`) ⇒ keys are re-emitted **alphabetically**, not in file order, and re-spaced to `key = value` | values preserved, layout not |
| whitespace-bearing keys | `write` emits `key = value`, `parse` trims ⇒ a key the legacy loader read as `"radius "` cannot be represented | **§11.109(h2) residual — SUSPENDED, unchanged** |

Design: promote a `Section` from `{header, map}` to an **ordered line list**.

```
struct Line { enum Kind { COMMENT, BLANK, KEY, RAW } kind;
              std::string raw;            // the input line, verbatim
              std::string key, value; };  // filled for KEY only
struct Section { std::string header; std::string rawHeader; std::vector<Line> lines; };
```

* `parse` classifies and keeps **every** input line in order. `RAW` = neither
  comment nor `key = value`: carried verbatim, never interpreted, never dropped.
* `write` emits lines in order. A key whose value changed is rewritten **in
  place, preserving its own key text and spacing verbatim** — which incidentally
  makes the (h2) whitespace case round-trip for the *rewrite* path without
  deciding the format-semantics question it raises. New keys append at the end of
  their section. A key that must be REMOVED is **commented out with a reason**,
  never deleted: deleting authored text is the D9 destruction class, and a
  commented line is both preserved and inert.
* Key lookup stays O(log n) through a side index built at parse; **the ordered
  line list is the authority and the index is derived** (I2 — one authority, one
  derived accessor, never two mutable copies).

### 5.3 Inline annotation (O7/O8) — the new half

* **Producer**: only the LOADER knows a datum is invalid, which default it
  applied, and which values are valid — that is D12's and §2(f)'s content. So the
  loader emits an *annotation set* keyed by (section, key, reason) as it loads,
  and the writer consumes it. The annotation is not invented by the writer.
* **Consequence: §5.39 must be resolved first, in the direction the defect
  itself states.** Today two parsers read one legacy format — the loader's own
  `substr` arithmetic (`ModularSystem.cpp:1370-1373`) and
  `ModularSystemFormat::parse`. If the annotation is produced by one and written
  by the other, they can disagree about which line a key came from, and the
  annotation lands next to the wrong datum. So **the loader must adopt the format
  parser** (extended per §5.2), making "what was loaded" and "what will be
  written" the same object. §5.39 already records this half as *decision-free*
  ("unifying the parse … is decision-free and closes the desync"); this design
  makes it a **precondition**, not a cleanup. The suspended half (whitespace
  representability, tightening the legacy parser on frozen data) stays suspended
  and is not needed for the annotation to work. Acceptance for the unification is
  ready-made: `b24_equivalence` compares 93 bodies and their capabilities.
* **(c) The annotation must go ABOVE the datum, not after it — source-forced.**
  The legacy loader takes the value as *the rest of the line*
  (`bodyParams[line.substr(0, pos-1)] = line.substr(pos+2)`,
  `ModularSystem.cpp:1373`) with no comment stripping. A trailing `# …`
  annotation would therefore be swallowed **into the value**, inert only where
  the consumer happens to be `strtod` — §11.109(e) recorded a shipped `[mimas]`
  line whose numeric value carries a trailing comment and survives for exactly
  that reason. For any string-valued key (`name`, `tex_map`, `model_name`) a
  trailing annotation corrupts the value outright. Annotate above.
* **Content of an annotation** follows §2(f) verbatim in shape: what is wrong,
  which states are valid, what was applied instead, what action fixes it. D12
  adds: an annotation is mandatory wherever a default **acted**, and forbidden
  where it merely did nothing.
* **Idempotence is a requirement, not a nicety**: a file rewritten twice with the
  same diagnosis must be byte-identical the second time, or every save inflates
  the file with duplicated annotations. Mechanism: annotations carry a stable
  machine-readable marker (`#!sc: …`) so a re-write replaces its own previous
  annotation for the same (section, key, reason) and leaves human comments alone.
  This is testable directly (§6 T9) and is the check that a naive implementation
  fails.

### 5.4 Which files may be rewritten — ~~NOT decided here~~ **DECIDED [vixy 2026-07-26, D35 → §11.113(n)]: option (1) — composed files and the session file ONLY; legacy `ssystem.ini` is READ-ONLY forever.** The mandate's *"dynamically rewriting the stellar system files"* is scoped to the composed format. The answer's second half is general and became **§2.0 D13** (*downgrade must stay possible*): a revert to an older build must not break an install that did not opt into new features/formats, so nothing may be written into a file an older parser reads — no comments, no new keys, no reformatting. Note the two contracts are distinct: §11.66(b) preservation protects the AUTHOR's content, D13 protects the OLD PARSER's grammar, and a writer can satisfy the first while breaking the second.

The mandate's own words are *"dynamically rewriting the stellar system files"*,
which reads as including the **legacy** `ssystem.ini`. That file is frozen paid
data (D9), it is read by the loader's own parser, and §5.39 measured seven keys
where the two parsers already disagree on it. Rewriting it is a different risk
class from writing a composed file this engine generated. → **D35**.

---

## 6. The as-if criterion, operationalized

### 6.1 The criterion, and the obligation it creates

D8's bar is *no user-reachable observable can tell*. The observable channels D8
names are the screen, script fetch, warp-to and — explicitly — **save**
(§2.0 D8: *"any user-reachable read — draw/show, selection, save included"*).
Two consequences, both new:

* **The save is a USE-SITE.** Under the §11.76 barrier a frozen body must be
  recomputed *at the use*, with 4 extra iterations. A save that reads positions
  from bodies the engine deliberately froze (D8's licensed mechanism, live since
  B32/§11.93) would serialize stale values, and the restored session would differ
  from the saved one by however long each body had been frozen. The save must
  therefore drive the same recompute-at-use path every other channel does. **This
  is the first consumer that touches EVERY body rather than the few in view**,
  which is what makes it a design obligation rather than a detail.
* **§11.101(i)(3)'s clean negative is spent, deliberately.** That entry recorded
  the save channel as structurally clean — *"no jd-derived state — spin, position
  or orientation — is serialized anywhere in the new path"*, `saveOrbit()` with
  no live caller in the new path. This design **spends** that property on
  purpose: jd, observer position, orientation scalars and per-body overrides all
  become serialized. Recorded here so the next audit finds the decision rather
  than the drift. (Correction to that entry's letter, from this pass:
  `saveOrbit()` does have one live caller — `AnchorPointOrbit::saveAnchor`
  (`anchor_point_orbit.cpp:72`), reached from `AnchorManager::saveCameraPosition`
  (`anchor_manager.cpp:723`) — on the OLD path, and unreachable in practice
  because of the §3.4(d) double prefix. The entry's claim holds for the new path;
  its "zero callers" wording does not.)
* **§5.32 is in this path.** The camera reads its reference's *cached* state with
  no drawn gate; a save that reads the camera without first refreshing the
  reference serializes the same staleness, and (unlike a frame) the error is then
  permanent. The single fix that entry proposes — recompute the reference's spin
  and reach at the top of `Camera::update` — is a precondition of a trustworthy
  save, not an unrelated defect.

### 6.2 The discriminating checks the implementation dispatch inherits

Written as tests, each with what it can catch that the others cannot.

* **T1 — SCREEN, the terminal observable.** Build a scene that exercises every
  group of §2 (reference + tracking + fov + armed offset + a hidden body + a
  recoloured body + a scaled parent + a pinned landscape + a composed rover),
  save, quit, relaunch **fresh**, restore, screenshot. Criterion: px>8 = 0
  against a floor **measured in-scene by an A/A pair of launches of the same
  binary**, never inherited (§11.80(a); and §11.109(f) is the standing warning
  that a cross-launch floor can be three orders above the recorded jitter in a
  scene that makes it visible). Same binary both sides, so the §11.106(g)
  cross-build codegen floor does not apply.
* **T2 — DUMP, field by field.** `Camera::dumpTrace` (`Camera.cpp:883-928`) is
  already the instrument that proved bit-identity across three consecutive
  reloads (§11.55(d)). Every field it emits is a save target; equality of every
  field is the assert. Extend it only where §2 names state the dump does not
  carry (`mount` is there, `skyLocked` is there, `foldLat` is derived — the gaps
  are `lockedSkyRot`, the plans, and the per-body ledger, which want their own
  dump).
* **T3 — THE LATCH TEST (§11.101(f), recorded as a prediction, still untested).**
  Scene: a composed rover grounded on the Moon, `flag moon_scaled on`
  (`moon_scale = 5`, the shipped default). Prediction, derived from source:
  `SurfacePointOrbitLoader.hpp:88,112` takes `datum = parent->getAltitudeReference()`
  (the **scaled** datum) once and bakes it into a `const double altStart` replayed
  every frame (`:62-78`), while `core.cpp:101` constructs the system BEFORE
  `:573` applies `moon_scale` through a 5 s `ASmooth` ramp. Therefore a restore
  that re-loads the system latches a **different** datum than the save did, and
  the rover moves by up to `radius·(scale−1)` = **6949.6 km** on the Moon.
  Assert: the rover's dumped `ecl` after restore equals its value at save, within
  the in-run floor. This test fails today **unless** the restore either retires
  the latch (recompute-at-use — the closed B15/B19/B32 class, which §11.101(f)(ii)
  argues may need no new decision at the model layer) or guarantees the scaling
  is applied before the system loads and is not mid-ramp. It is the same test
  that decides §5.27/D21, which is why the two must be sequenced together.
* **T4 — REVERSIBLE PAIR, traversed TWICE.** save → restore → save → restore,
  the second entry starting from the state the first exit produced. Assert dump
  equality at both exits **and that the second session file is byte-identical to
  the first** — a save that is not a fixed point is not a save, and byte
  comparison is available here because both files come from the same binary in
  the same scene.
* **T5 — DISCRIMINATION (the instrument must be able to fail).** (a) Restore a
  session saved from a *different* scene and assert the dump moves. (b) Mutate
  exactly one key in the session file by hand and assert exactly the
  corresponding dump field moves — the `--mutate` shape §11.109(f) used.
* **T6 — CONTENT.** A script-pushed persistent body exists after quit + relaunch
  with the same parent, relation, module set and routing. The comparator exists:
  import `b24_equivalence`'s per-body comparison rather than copying it (the I2
  call §11.109(b) made).
* **T7 — UNRESOLVED KEY.** Rename a body in the data between save and restore.
  Assert the override is **reported** (log + inline annotation in the session
  file) and NOT applied to anything else. This is the test that A29's hazard is
  actually closed rather than merely renamed.
* **T8 — D9 FORWARD PROPAGATION, the check that discriminates D30.** Change an
  *authored* value in the data file between save and restore; assert the NEW
  authored value is in effect while the operator's override still applies on top.
  A snapshot design fails this test by construction; a delta design passes it.
  This is the sharpest single argument in the whole design and it is measurable.
* **T9 — WRITE-BACK PRESERVATION.** A data file carrying a comment, a malformed
  line, an unknown key, an ISO-8859 value and a duplicate key: rewrite it and
  assert every one survives byte-identically, that annotations appear ABOVE their
  datum, and that a second rewrite with the same diagnosis is byte-identical to
  the first (§5.3 idempotence). Plus the atomicity leg: make the temp
  un-writable and assert the original is untouched (§11.52(a) clause 2).
* **T10 — FROZEN-BODY SAVE (the use-site barrier).** Freeze bodies (the B32/D20
  mechanism), let jd advance, save, restore, and compare a frozen body's position
  against the same scene where it was never frozen. Equality within the
  recomputed-at-use tolerance is what proves §6.1's obligation was honoured;
  a divergence proportional to freeze duration is the failure signature.

### 6.3 What "no regression" means for this row

The standing battery (`b24_equivalence` 93, `b5_oort` 11/11, `b3_ladder`,
`scene_e_spine` 26/26, `b25_galactic`, 0 VUID, config/ssystem md5 in == out)
plus, specific to this row: **the md5 of every data file the session did not
explicitly target must be unchanged after a save** — a save that touches a file
it was not asked to touch is the D9 destruction class, and it is cheap to assert.

---

## 7. Decision points (→ `DECISIONS_PENDING.md` D30–D36) — **ALL ANSWERED 2026-07-26, propagated §11.113(i)–(o)**

*The table below is kept as the question set (traceability); each row's answer is in `DECISIONS_PENDING.md` under its key and in §11.113. The header note at the top of this file carries the summary.*

Each is a genuine product choice: it changes user-visible semantics or the
meaning of shipped data, and none is traceable to an existing recorded
resolution. Full text with options and executor recommendations is in
`DECISIONS_PENDING.md` §12; the one-line form here is the index.

| key | question | why it is not the executor's |
|---|---|---|
| **D30** | **snapshot vs delta**: does the session file record the *effective* value of everything, or only the operator's *deltas* over what the data declares? | decides whether a restored session sees corrected data from a later delivery (D9 forward-propagation) or freezes the values that were current at save |
| **D31** | **`body action reload` vs session restore**: §11.66(a) resolved B16 as "reload keeps body-scoped overrides"; R8 says colour must reset to the body/script default and *"the user must reload the rule"*. Two recorded answers point opposite ways for the same operation | a direct conflict between a Vixy resolution and a tester preference, on user-visible behaviour |
| **D32** | **how deep "as-if" cuts**: do in-flight ramps, moves, zooms, eased scalings, fades, trails-in-progress and timers restore mid-flight, or snap to their settled target? | "as-if we continued" read literally says mid-flight; the cost and the observable both change by an order of magnitude between the two readings |
| **D33** | **trigger policy**: explicit only, or autosave-at-quit / autoload-at-start? | an autosave is an ACTING DEFAULT (D12) that changes what launching the app means |
| **D34** | **identity key** for the override ledger and for non-body selections | the A29 hazard; A30(iii) says decide *before* the first file exists |
| **D35** | **which files the writer may rewrite**: composed files only, or the legacy `ssystem.ini` too (the mandate's own wording)? | rewriting frozen paid data is a D9 risk class of its own |
| **D36** | **is a running show part of the session**: script engine position, variables, media/video/audio/text/subtitle state | decides the feature's scope; a mid-video restore is a different product from a mid-scene restore |

---

## 8. Dependencies, and the order they force

| depends on | why | state |
|---|---|---|
| **D28** (A38) — reference-switch roll | B6 `heading` is a save target whose *meaning* across a reference change is unresolved; saving a number whose semantics is pending would bake the pending answer | OPEN, Vixy's |
| **D21 / §5.27** — grounded children vs display scaling | T3 fails until the load-time latch is retired or the restore order is guaranteed; the two share one mechanism | OPEN, Vixy's |
| **§5.32** — camera-reference cached state | a save reading a stale reference makes the staleness permanent (§6.1) | OPEN defect, fix identified |
| **§5.39** — two parsers, one format | the loader must own the parse the writer round-trips, or annotations land next to the wrong datum (§5.3) | OPEN; the unification half is decision-free by its own record |
| **§11.66(b) writer rework** (§5.2) | B25's remaining half (script-triggered system save) needs the same writer | not started; B31 owns it |
| **B33** — G-QUERY | the save must not read the control-surface getters (§3.4(e)) | row open; B31 does not need it fixed, only avoided |
| **R13 / §6.9** — per-mode anchor memory | row B17: nothing to serialize until it exists | not started |
| **D29 / §5.37** — galactic path | G7: those systems cannot be part of a session while they cannot load | SUSPENDED |

Sequencing that follows: **§5.39 unification → writer rework (§5.2/§5.3) →
persistent-body serialization (§4.1, which also completes B25's half) → the
session file (§3.2) → the ledger (§2 group D)**. The camera/latch defects
(§5.32, §5.27) must land before T3/T10 can pass, but they do not block the
earlier steps.

---

## 9. Backward compatibility (D9), stated explicitly

A save file is **new product surface**. The frozen-field rule shapes it as
follows:

1. **Nothing in the field has a session file**, so there is no legacy version to
   keep reading — but from the first byte written there is. The file therefore
   carries `format = <n>` in its first section, and a restore of an unknown
   version **refuses with a §2(f) diagnostic** rather than half-applying. Silent
   partial restoration of a future file is the worst available failure.
2. **A session file is not data and must never become a data delivery.** It
   references content by file path and key; it never inlines body parameters.
   That keeps D9's "field is frozen" applying to data only, and it is what lets a
   session survive a delivery that corrects the data underneath it (T8).
3. **A restore must not assume the data is unchanged.** Every reference resolves
   by key; misses are reported, annotated in place and kept (§4.2). This is the
   D9-compatible behaviour: corrections propagate forward, the session carries
   only the operator's deltas.
4. **The writer never deletes authored text** (§5.2): a removed key is commented
   out with its reason.
5. **The save must not touch files it was not asked to touch** — asserted, not
   assumed (§6.3).
6. **No bootstrap entry**: absence of a session directory means "no session", so
   `CallSystem::checkIniFiles` (`call_system.cpp:81-95`) is not extended and no
   `default_session.ini` ships. A file that does not exist cannot be stale.

## 10. Acting defaults this design would introduce (D12)

Listed so none is silent, per D12's own rule that a default which ACTS is logged:

* ~~**restoring anything at all at startup** — only under D33's opt-in; logged with
  the session name every time.~~ **RETIRED [vixy 2026-07-26, D33 → §11.113(l)]:
  explicit only — nothing is restored at startup, so this acting default never
  comes into existence. The remaining rows below stand (they attach to an
  explicit load, which is an operator ACT and still owes its log lines).**
* **a key that failed to resolve at restore** — logged once per entry AND
  annotated in the session file (§4.2). Not an inaction: the operator's override
  did not fire.
* **a value clamped or rejected at restore** (e.g. a fov outside the clamps, a
  reference body that no longer exists) — logged with what was applied instead.
* **a session file written to a name that already existed** — the rename is
  atomic and destructive by design; the log names the file it replaced.
* Purely inactive cases stay silent: an absent session file, an override whose
  value already equals the authored one.
