# B5 / T1.3 - The DSO layer on the new path: DESIGN PASS

Task F117 (`claude/fable-dispatch.md` S1, section `### F117`), row **B5** (S13.B,
`claude/INTENT.md:1190`), DEPLOYMENT-MAP **T1.3** (`claude/DEPLOYMENT-MAP.md:82`),
on the owner's direction **S11.233(c)** [vixy 2026-09-12]:

> *"the reach/visibility coupling if I'm not wrong, the as-if rule operate on the
> visible effects, or those which may became visible. Ideally, dso3d and ojmMgr
> must be bodies (modularBody), for tully it's harder because it is massively
> intanciated so it can't be handled this way. Maybe as a BodyModule of the
> milkyway ?"*

Written 2026-09-13 by the F117 executor (Opus 5) against code `master-beta @
87d429bd` and harness `CC-harness @ 385fe60`. **No engine line is changed by this
note.** Precedent for the form: `claude/b12-design.md` (F9), `claude/b31-design.md`
(F6).

Provenance grammar per the INTENT.md header: `[observed: file:line]`
`[measured: instrument -> value]` `[derived: argument]` `[stated: ...]`
`[vixy: ...]` `[assumed - flagged]`.

**Standing on**: S11.96 (the oort pilot and the reach/visibility coupling it
found), S11.98 (the ladder, the instrument caveat, the three new suspended
items), S11.102(e) (the audit of both), S11.103 (F0's four closures), S11.217(b)
(the frame contract and the nesting spine), S11.222(d) (what `14.sts` actually
is), S11.223(e) (the executor-mode line), S11.51(e) (A6: one renderer
instanced-batch service, generalize the live batch, never duplicate it), S2.0
**D8** (the as-if rule), **D9** (data is the product; the field is frozen),
**D11** (1 ms/frame), **D12** (acting defaults are logged), **D13** (downgrade
must stay possible), G4 (compute only what the observer can distinguish),
I1-I7, and S11.52(b) (the old path is the comparison baseline, unchanged by
construction).

**ASCII spelling, stated once.** The dispatch binds new harness files to pure
ASCII, so section references here are spelled `S11.96(e)` where the ledger
spells them with the section sign - the code tree's own established convention
(`OortModule.hpp:48` *"see the INTENT S6.9 coupling note"*,
`ssystem_factory.cpp:601` `"B5 \xc2\xa7" "6.9 pilot"`). The D14 gate itself does
not reach this file: `harness/f70_partition.tsv`'s pathspecs are CODE-tree paths
and `claude/` is git-ignored by the code repo, so no EXCLUDE row is possible or
needed (the two precedent notes carry 794 and 1184 non-ASCII bytes and the gate
passes) [measured: `python3 harness/f70_ascii.py gate` -> PASS, 972 CONVERT
files]. The constraint was met by writing, not by classifying.

---

## 0. What this note decides, and what it deliberately does not

**Decides** (and hence what a reader may rely on):

1. **The census** of what the old DSO layer IS, by command - including three
   corrections to the premises this task was minted on (S1).
2. **The as-if table**: every observable channel the census found, classified
   visible / may-become-visible / never-visible WITH the mechanism that makes it
   so, and the as-if obligation each class creates (S2).
3. **The reach/visibility decoupling MECHANISM** - the answer to S11.96(e)(2),
   stated as a change that is inert by construction on every body that exists
   today, and measurable both ways (S3.2).
4. **Which node each content class hangs from** and why the as-if table forces
   it: Dso3d and the OJM models as `ModularBody` children, Tully as a
   `BodyModule` of the MilkyWay node (S3.3-S3.5).
5. **Which renderer role Tully needs**, measured against what exists: the
   batched-family service, not a new instancing mechanism (S3.6).
6. **What becomes of the four executor draw sites** (S3.7).
7. **A disposition for each of S11.96(e)(1)-(6) and S11.98(f)(i)-(iii)** -
   answered by a row of the as-if table, or left to the owner with both readings
   and their costs (S4).
8. **The slices**: ordered, decision-free, sized, each with a check stated so it
   CAN fail, its parity instrument, and the byte-unchanged assertion for the
   flag-off tree (S5).

**Does not decide** (recorded, deliberately left open - S3.8 carries the
arguments and S4 the prices):

- **The catalogue-to-AU conversion VALUE** (S1.6). No source in the tree states
  it; getting it wrong is not a visual defect but a navigational one.
- **Whether the ported content is engine-instantiated or authored data** (the
  oort pilot's choice vs a data home). D9 product surface; S3.8 (V-D1).
- **The high-edge semantics** S11.96(e)(4) and the onset ramp S11.98(f)(iii) -
  both are visible-channel choices between two behaviours that both exist today
  on different paths; priced in S2 rows D5 and F2, decided by nobody here.
- **Whether the labels keep riding `flag star_names` / `flag nebula_names`**
  (S2 rows L2/L3) - an operator-surface question with a measured field cost.
- **Tully's selectability after the port** (S2 row S1) - both readings priced.
- **Whether the `dso3d` / `dso2D` command-token inversion is repaired** (S1.1) -
  a user-visible authoring surface, D9.

---

## 1. The census, by command

### 1.1 The layer is FIVE classes, not four, and the command named after one of them drives another

The dispatch section enumerates four classes. The command layer enumerates five
objects in the same neighbourhood, and two of the names are crossed:

| class | what it draws | the command token that reaches it | executor site |
|---|---|---|---|
| `Dso3d` `[observed: inGalaxyModule/dso3d.hpp:40]` | 32 catalogue nebulae as textured sprites + their names | **`dso2D`** `[observed: app_command_interface.cpp:1695]` | IN_GALAXY only `[observed: inGalaxyModule.cpp:141]` |
| `DsoNavigator` `[observed: inGalaxyModule/dsoNavigator.hpp:43]` | volumetric DSO (a 3-D texture box) | **`dso3d`** `[observed: app_command_interface.cpp:1676]` | IN_GALAXY `[observed: inGalaxyModule.cpp:144]` + IN_UNIVERSE `[observed: inUniverseModule.cpp:129]` |
| `Tully` `[observed: coreModule/tully.hpp:54]` | 30432 + N galaxies as points and squares, + names | `flag tully`, `flag tully_color_mode` `[observed: base_command_interface.hpp:489,491]` | IN_UNIVERSE `[observed: inUniverseModule.cpp:115,119]` |
| `OjmMgr` `[observed: ojmModule/ojm_mgr.hpp:43]` | named OJM meshes placed per mode | `body ... mode in_galaxy\|in_universe\|in_sandbox` `[observed: app_command_interface.cpp:4119]` | IN_GALAXY `[observed: inGalaxyModule.cpp:142]`, IN_UNIVERSE `[observed: inUniverseModule.cpp:121]` |
| `Oort` `[observed: coreModule/oort.hpp:77]` | 10000 points, altitude-gated | `flag oort` `[observed: base_command_interface.hpp:482]` | SOLAR_SYSTEM `[observed: solarSystemModule.cpp:203]`, conditional |

**The inversion, at source**: `AppCommandInterface::commandDso3D` calls
`coreLink->dsoNavInsert` / `dsoNavSetupVolumetric` / `dsoNavOverrideCurrent`
`[observed: app_command_interface.cpp:1679-1687]`, all three of which reach
`core->dsoNav` `[observed: coreLink.cpp:841,846,851]`; `commandDso2D` calls
`stcore->loadDso2d` `[observed: app_command_interface.cpp:1706]` = `Core::loadDso2d`,
whose whole body is `return dso3d->loadCommand(...)` `[observed: core.cpp:2635-2638]`.
So **the script word `dso3d` does not reach the class `Dso3d`; the script word
`dso2D` does.** This is not a detail of naming: a census that partitioned the
field corpus by the token `dso3d` would attribute the wrong scripts to the
class being ported. `DsoNavigator` is NOT in this task's scope and is named here
only so the next reader does not re-discover the crossing.

A sixth object sits in the same draw sites and is named for completeness because
Tully's build depends on it: `VolumObj3D core->volumGalaxy` `[observed: core.hpp:724]`,
the volumetric Milky Way, which `Tully::build` takes as an argument and whose
`isInside` flips Tully's draw split every frame `[observed: tully.cpp:510-515]`.

### 1.2 `Dso3d` - 32 nebulae, one texture atlas, one label pass

- **Data**: `dso3d.dat`, loaded once at core init `[observed: core.cpp:474]`; 32
  records `[measured: wc -l ~/.spacecrafter/dso3d.dat -> 32]`, each
  `name xyz alpha delta r size type` `[observed: dso3d.cpp:123]`. When `xyz != 1`
  the loader converts (alpha, delta, r) to rectangular, applies an ecliptic
  rotation of -23.43928 degrees and **divides by two** `[observed: dso3d.cpp:134-144]`.
  Texture atlas `dsocat.png` `[observed: core.cpp:473]`, sub-texture count derived
  from the atlas aspect ratio `[observed: dso3d.cpp:211-218]`.
- **Draw**: one pre-recorded command buffer, `POINT_LIST` through a geometry
  shader that emits a camera-facing quad `[observed: dso3d.cpp:67-77]`, pass
  `PASS_MULTISAMPLE_DEPTH` with depth enabled `[observed: dso3d.cpp:67-69]`, additive
  blend `[:70]`. The sprite radius law is `scale * 60 / distance`, with a special
  case for texture index 31 (radius = scale, i.e. fixed size)
  `[observed: shaders/src/dso3d.geom:33-36]`, and nothing is emitted below
  radius 1.0 `[observed: shaders/src/dso3d.geom:40]`.
- **Labels**: `drawDsoName` prints every name through `printGravity180` with the
  font registered as `CLASS_NEBULAE` `[observed: dso3d.cpp:221-236, core.cpp:193]`,
  colour `labelColor` * `names_fader` * `fader` `[observed: dso3d.cpp:223-225]`.
- **Faders**: `fader` (shown) and `names_fader` (labels), both `LinearFader`
  `[observed: dso3d.hpp:118-119]`. `fader = true` at construction
  `[observed: dso3d.cpp:35]` - there is **no config key for the show state**.
- **The show channel**: `CoreLink::nebulaSetFlag` sets the 2-D nebula layer AND
  this cloud `[observed: coreLink.cpp:858-861]`, so the operator word is
  `flag nebulae`; at startup only `nebulas` is set from `flag_nebula`
  `[observed: core.cpp:374]` and the cloud keeps its constructor `true`.
- **The label channel**: startup from `flag_star_name` `[observed: core.cpp:366]`,
  runtime from `flag nebula_names` `[observed: app_command_interface.cpp:1167-1169]`.
  **Two different operator words for one state, one at each end of the session**
  - recorded as an out-of-scope finding (S6, F117-a).
- **Colour**: `nebula_label_color` `[observed: core.cpp:1674]`.
- **Runtime content**: `loadCommand` appends one nebula and rebuilds the buffer
  `[observed: dso3d.cpp:160-194]`; `removeSupplementalDso` clears everything and
  re-reads the catalogue `[observed: dso3d.cpp:196-205]`. Reached by
  `dso2D action load` / `dso2D action clear` `[observed: app_command_interface.cpp:1699-1712]`.
  **Field use: zero** (S1.7).
- **Dead surface**: `CoreLink::dso3dSetDuration` `[observed: coreLink.cpp:863]` has no
  caller outside `coreLink.cpp` `[measured: grep -rn dso3dSetDuration src --include=*.cpp]`
  - no command token reaches it.

### 1.3 `Tully` - the massively instanced one

- **Data**: `tully.dat` (30432 records `[measured: wc -l]`) plus a big catalogue
  `6df.dat` with an optimal-distance fade parameter of `5e+12`
  `[observed: core.cpp:470-471]`. Record format
  `name r g b x y z type` `[observed: tully.cpp:167]`; positions are multiplied by
  **200** with a Y/Z swap and a sign flip `[observed: tully.cpp:170-172]`; the
  per-type scale is a switch of eight cases `[observed: tully.cpp:180-189]`.
- **Draw**: TWO geometries per frame - a static point list built from the
  catalogue, and a **dynamic square list rebuilt every frame** by
  `computeSquareGalaxies`, which loops over all `nbGalaxy`, keeps those whose
  `3.0/(distance*scale) >= 2`, sorts them by plane side and distance, and
  `memcpy`s 5 floats per survivor into a staging buffer
  `[observed: tully.cpp:412-468]`. Six command buffers, three for custom colour
  and three for white `[observed: tully.cpp:282-285]`.
- **The volumetric coupling**: when `core->volumGalaxy` is loaded, the executor
  passes it to `build` `[observed: inUniverseModule.cpp:111-115]`, and `draw`
  rebuilds the command buffers whenever the camera crosses the object's surface
  and re-orders the two half-catalogues by plane side
  `[observed: tully.cpp:510-529]`. This is a `vkDeviceWaitIdle` on the crossing
  frame `[observed: tully.cpp:513]`.
- **Labels**: only for galaxies closer than `sqrt(0.1)` in catalogue units
  `[observed: tully.cpp:580-581]`, font `CLASS_HIPSTARS` `[observed: core.cpp:195]`,
  colour = the galaxy's own RGB `[observed: tully.cpp:588]`.
- **Selection**: `Tully::searchAround` builds a `TullyWrapper` (an `ObjectBase`)
  for every catalogue entry inside the cone `[observed: tully.cpp:470-489]`, typed
  `OBJECT_STAR_CLUSTER` `[:486]`; `Core::findAndSelect`'s candidate loop calls it
  only in IN_UNIVERSE and only when the show flag is on
  `[observed: core.cpp:1308-1309]`. The wrapper's observable content: a type word
  ("Globular Cluster" for type 9, else "Galaxy"), the name, J2000 and of-date
  RA/DE, Alt/Az `[observed: tullyWrapper.cpp:24-53]`; `getMag` returns **0**
  `[observed: tullyWrapper.cpp:74-77]` and `getNameI18n` returns the empty string
  `[observed: TullyWrapper.hpp:55-57]`.
- **Faders / flags**: `fader = true` at construction `[observed: tully.cpp:47]` -
  again no config key for the show state; `useWhiteColor = true` by default
  `[observed: tully.hpp:180]`; names from `flag_star_name` at startup
  `[observed: core.cpp:472]` and from `flag star_names` at runtime
  `[observed: app_command_interface.cpp:1088-1090]`.
- **Dead surface**: `CoreLink::tullySetDuration` `[observed: coreLink.cpp:971]` has no
  caller `[measured: grep]`.

### 1.4 `OjmMgr` - the field's actual DSO workhorse

- **What it is**: a flat vector of `(name, model matrix, STATE_POSITION, uniform)`
  containers `[observed: ojm_mgr.hpp:83-97]` with four states
  `IN_UNIVERSE / IN_GALAXY / IN_SANDBOX / OTHER` `[observed: ojm_mgr.hpp:45-50]`.
- **Commands**: `body action load|remove|clear mode <state> name <n> filename <f>
  pos_x pos_y pos_z scale` - the mode short-circuits `commandBody` before any
  body handling `[observed: app_command_interface.cpp:4119-4135]`; the `.ojm`
  extension and the directory are appended by the command layer
  `[observed: app_command_interface.cpp:4122]`.
- **Draw**: one command buffer per frame index, rebuilt whenever the state
  changes or content moves `[observed: ojm_mgr.cpp:147-154]`; the per-frame work is
  one model-view + normal matrix per model `[observed: ojm_mgr.cpp:163-170]`.
  **The recorded buffer begins with a full-screen depth CLEAR**
  `[observed: ojm_mgr.cpp:187-189]`, and `draw` executes it unconditionally, even
  when no model exists for the current state `[observed: ojm_mgr.cpp:171]`.
- **Scale**: the authored `scale` is multiplied by the model's own radius
  `[observed: ojm_mgr.cpp:70]`.
- **UI hook**: the inline `initial.sts` block of `UI::init` clears all three
  modes at startup `[observed: ui.cpp:184, :245-247]`.
- **ONE MODEL IS LOADED BY THE ENGINE, NOT BY A SCRIPT** - and it is the Milky
  Way: when the volumetric galaxy is absent, `Core::init` loads
  `Milkyway/Milkyway.ojm` into the `in_universe` state at scale 0.01
  `[observed: core.cpp:486]`, as the `else` arm of `volumGalaxy->loaded()`
  `[observed: core.cpp:479-486]`. So the in-universe OJM content is
  data-dependent: with the volumetric galaxy present the model is not loaded at
  all. A port that only re-routes the COMMAND would silently drop it.
- **Uniform cost**: 128 bytes per loaded model, and it is the class that walks
  into an exhausted pool in the field (S11.222(d): 200 of `14.sts`'s 201
  executed in-galaxy loads were refused and reported successful).

### 1.5 `Oort`, and the pilot that already exists

- **Old**: 10000 points (config `oort_elements` `[observed: core.cpp:459]`) from the
  shared spatial law `oortSamplePoint` `[observed: oort.cpp:62-76]` seeded from the
  frozen `OORT_SEED = 20260724` `[observed: oort.cpp:46]`; `PASS_BACKGROUND`,
  `POINT_LIST` `[observed: oort.cpp:102-103]`; the draw gates on the observer's
  altitude - nothing below 1e13 m or above 1e16 m - and ramps the intensity
  between `[observed: oort.cpp:159-162]`; `fader = false` at construction
  `[observed: oort.cpp:80]`, set from `flag_oort` at init `[observed: core.cpp:630]`.
- **New (the pilot, config-gated OFF)**: `OortModule` is a `BodyModule` of a
  `ModularBody` named "Oort" created as an ORBITING child of the "Solar" system
  node with `still_orbit`, `BodyType::VOID` and `radius =
  OORT_REGIME_RADIUS_AU = 50` `[observed: ssystem_factory.cpp:569-590]`; the module
  is loaded into an explicit `"OORT"` slot of the CUSTOM family
  `[observed: ssystem_factory.cpp:597]`, keyed by `oort=true`
  `[observed: OortLoader.cpp:5-11]`, and routed as a NEAR component
  `[observed: OortLoader.cpp:28]`. It reports **the cloud extent** as its bounding
  radius `[observed: OortModule.cpp:87-96]` - the fact that produced the coupling
  this note has to decide.
- **The seam**: the old cloud draws unless the modular one was actually
  instantiated `[observed: solarSystemModule.cpp:202-203]`; the show flag and the
  colour each have one writer driving both `[observed: core.cpp:630-631, 1724-1726]`.
- **Instantiation is gated by `flag_experimental_oort`, default false**
  `[observed: core.cpp:466]`; the field config carries `flag_oort = true` and no
  `flag_experimental_oort` `[measured: grep ~/.spacecrafter/config.ini]`.

### 1.6 Frames, units and scales - the fact the port cannot avoid

Each executor mode draws in its own world scale, fixed by its clipping planes:

| mode | clipping planes | unit of the content drawn there |
|---|---|---|
| SOLAR_SYSTEM | `(1e-6, 200)` `[observed: solarSystemModule.cpp:185]` | AU (`AU = 149597870.691` km `[observed: sc_const.hpp:45]`) |
| IN_GALAXY | `(0.01, 2000.01)` `[observed: inGalaxyModule.cpp:119]` | the `dso3d.dat` radius / 2 `[observed: dso3d.cpp:142-144]` |
| IN_UNIVERSE | `(0.0001, 10)` `[observed: inUniverseModule.cpp:104]` | the `tully.dat` coordinate * 200 `[observed: tully.cpp:170-172]` |
| IN_SANDBOX | commented out `[observed: inSandBoxModule.cpp:114]` | - |

Both clouds are drawn with the SAME matrix and camera position the solar system
uses - `nav->getHelioToEyeMat()` and `nav->getObserverHelioPos()`
`[observed: dso3d.cpp:242-243, tully.cpp:507-508]` - so the mode's world scale is
carried entirely by the clipping planes and by the catalogue's own units. **No
constant converting either catalogue to AU exists anywhere in the tree**
`[measured: grep -rni 'parsec|light_year|lightyear|LY_TO|AU_TO' src -> one comment
at TullyWrapper.hpp:76 ("observer's position in parsec"), no constant]`.

**Consequence for the port, and it is the load-bearing one.** The new path has
ONE tree in AU with per-body depth slices and no per-mode rescale
`[observed: ModularBody.hpp:655-664, the frame contract]`. Porting this content
therefore REQUIRES a catalogue-to-AU factor per catalogue, and the factor is not
a rendering detail: it sets each object's distance, hence its screen size, hence
its regime, hence (S3.2) its reach. **This note does not choose the numbers**
(S3.8, V-D2); it states the two ways to obtain them and the check that discriminates:
`dso3d.dat`'s own entries are identifiable objects with published distances (the
first record is the Veil nebula at alpha 312.15 / delta 30.88 / r 2500
`[measured: head -1 ~/.spacecrafter/dso3d.dat]`), so the factor is a MEASUREMENT
against the catalogue's own astronomy - a cited fetch, never a recalled number
(S11.51(d) red line) - and the discriminating check is that two different
catalogue entries agree on the same factor to within their own quoted precision.

### 1.7 The field's use, measured

`harness/f117_census.py` partitions the tester's corpus **by the command
grammar** rather than by token, because the token count the mint stands on is
an artifact: `~/.spacecrafter/scripts` holds 2772 files of which **2230 are
binary media** (png/mp4/avi) whose bytes contain the searched words
`[measured: f117_census.py -> artifacts/f117/field-census.txt.gz]`.

| what the field does | files | lines |
|---|---|---|
| `body ... mode in_galaxy\|in_universe` -> **OjmMgr** | 10 | **560** (527 of them in `fscripts/14.sts`) |
| `dso3d ...` -> DsoNavigator | 2 | 3 |
| `dso ...` -> the 2-D nebula layer (not this task) | 6 | 544 |
| `flag nebula_names` (Dso3d's labels) | 6 | 6 |
| `flag star_names` (Tully's labels) | 6 | 6 |
| `flag milky_way` | 24 | 25 |
| `dso2D ...` -> **Dso3d** | **0** | **0** |
| `flag tully` / `flag tully_color_mode` | **0** | **0** |
| `flag oort` | **0** | **0** |

The two `tully` token hits in text scripts are an image path
(`tully_jump/none.png`) `[measured: f117_census.py --lines]`. The 527 in-galaxy
loads of `14.sts` are S11.222(d)'s stars, reproduced here independently.

**Four premise corrections, reported as dispatcher-side findings** (S11.179(a):
none of them is an input this task stands on - the task re-adjudicates the
census by construction - so none is an abort trigger):

1. `ojm = 96 files` is a binary-match artifact; the command-level answer is
   **10 files / 560 lines**, and the partition the mint asked for ("mesh-module
   `body action load ... .ojm` uses partitioned OUT") does not exist in the
   corpus at all: **zero** text scripts name a `.ojm` file outside the
   mode-qualified form, because the extension is appended by the command layer
   `[observed: app_command_interface.cpp:4122]`.
2. `dso3d = 2` counts DsoNavigator scripts, not `Dso3d` scripts (S1.1). The
   `Dso3d` class has **no field user at all**.
3. `oort = 1` and `tully = 2` are binary/path matches; the field commands
   neither.
4. **Code-side, and of a different kind** (S1.8): the premise
   `grep -rl 'OjmMgr\|ojmMan' src ... | wc -l => 7` reproduces exactly, but its
   second alternative is a TYPO - the member is spelled `ojmMgr`, not `ojmMan`
   `[observed: core.hpp:727]` - so the set it enumerates is the files naming the
   TYPE, and it excludes `coreModule/coreLink.cpp`, which holds the manager's
   entire command surface `[observed: coreLink.cpp:635-646]`. With the intended
   pattern the reader set is **8 files, not 7**. A count that reproduces is not
   the same fact as a set that is right.

### 1.8 The reader set, file by file - every file of the premise counts explained

Check (b)'s obligation. The counts below are the case-insensitive sets (the
premises' own patterns are narrower and are noted where they differ). "caller" =
the file calls into the class; "surface" = it declares the operator-visible
word; "include only" = it includes the header and never calls; "mention" = the
name appears in a comment.

**`OjmMgr` - 8 files** (`grep -rli` over `OjmMgr|ojmMgr`; the premise's 7 is the
same set minus `coreLink.cpp`, see correction 4):

| file | role |
|---|---|
| `ojmModule/ojm_mgr.{hpp,cpp}` | the class itself |
| `coreModule/core.hpp:727` | owns it (`std::unique_ptr<OjmMgr> ojmMgr`) |
| `coreModule/core.cpp:118, :477, :485` | constructs, `init()`s, and LOADS the Milky Way model itself |
| `coreModule/coreLink.cpp:635-646` | the whole command surface (`BodyOJMLoad/Remove/RemoveAll`) |
| `executorModule/inGalaxyModule.cpp:142` | caller, `STATE_POSITION::IN_GALAXY` |
| `executorModule/inUniverseModule.cpp:121` | caller, `STATE_POSITION::IN_UNIVERSE` |
| `executorModule/inSandBoxModule.cpp:133` | caller, **commented out** |

Not in the set and reaching it anyway: `interfaceModule/app_command_interface.cpp:4119-4135`
and `uiModule/ui.cpp:245-247`, which go through `CoreLink`'s `BodyOJM*` spelling.

**`Dso3d` - 15 files case-insensitively, of which only 7 touch the class**:
`inGalaxyModule/dso3d.{hpp,cpp}` (the class), `core.{hpp,cpp}` (owns,
constructs, catalogue, fonts, label colour, `loadDso2d`), `coreLink.{cpp,hpp}`
(the show/label/duration surface), `executorModule/inGalaxyModule.cpp:101,:141`
(the only live caller: update + draw). **`interfaceModule/app_command_init.cpp`,
`app_command_interface.{cpp,hpp}` and `base_command_interface.hpp` carry the
token `dso3d`, which reaches `DsoNavigator`** (S1.1) - four files, not three. `inGalaxyModule/dsoNavigator.hpp:55` mentions it in a
comment (*"the subtexture of the texture to use (like dso3d)"*).
**`executorModule/solarSystemModule.cpp:33` and `stellarSystemModule.cpp:34`
INCLUDE `dso3d.hpp` and never call it** - dead includes, recorded not fixed.
`inSandBoxModule.cpp:96, :132` are commented-out calls.

**`Tully` - 13 files**: `coreModule/tully.{hpp,cpp}` (the class),
`coreModule/TullyWrapper.hpp` + `tullyWrapper.cpp` (the selection object),
`core.{hpp,cpp}` (owns, constructs, catalogues, font, the `searchAround` gate),
`coreLink.{cpp,hpp}` (show / white-colour / names / duration),
`executorModule/inUniverseModule.cpp` (update + the build/draw pair),
`interfaceModule/app_command_{init,interface}.cpp` and
`base_command_interface.hpp` (the `tully` and `tully_color_mode` flag tokens and
their readback), and `tools/object.hpp:48` - a **comment** naming `TullyWrapper`
as one of the four `ObjectBase` wrappers, which is where the selection channel's
type discipline is documented.

**`Oort` - 24 files**, the largest set because the pilot doubled it:
old path `coreModule/oort.{hpp,cpp}`, `core.{hpp,cpp}`, `coreLink.{cpp,hpp}`,
`executorModule/solarSystemModule.cpp` (update + the dual seam);
new path `experimentalModule/bodyModules/OortModule.{hpp,cpp}`,
`moduleLoader/OortLoader.{hpp,cpp}`, `modules.cpp:68` (the registration),
`bodyModule/ssystem_factory.{hpp,cpp}` (`createExperimentalOort`);
surface `interfaceModule/app_command_{init,interface}.cpp`,
`base_command_interface.hpp`, `mainModule/define_key.hpp`,
`mainModule/checkConfig.cpp` (the three config keys and their defaults),
`uiModule/ui.cpp:1433` (Ctrl+F);
and four **mentions only**: `experimentalModule/ModularBody.hpp:1991, :2011`,
`ModularSystem.{hpp,cpp}` - comments naming the pilot oort as the example of an
engine-minted body that is not system content.
`executorModule/stellarSystemModule.cpp:37` INCLUDES `oort.hpp` and never calls
it - the same dead include as `dso3d.hpp` above.

---

## 2. The as-if table

**The criterion** [vixy, S2.0 D8]: *"from the user standpoint the software must
behave as if everything were physically exact and fully computed; any internal
deviation is valid exactly as long as no user-reachable observable can tell"*,
narrowed for this batch by the owner's own sentence: *"the as-if rule operate on
the visible effects, or those which may became visible"* [vixy S11.233(c)].

**How a row is read.** `visible` = an observable a user reaches today on shipped
data with shipped commands. `may-become` = an observable that no shipped scene
reaches today but that a user CAN reach with an existing command or an existing
data key - the as-if rule covers it, so it must be preserved or consciously
retired, never silently dropped. `never` = an internal deviation with no
user-reachable channel: free to change. **The obligation column is what the port
owes**, and it is what S5's checks test.

### 2.1 Drawn output

| id | channel | class | today | mechanism | obligation |
|---|---|---|---|---|---|
| D1 | 32 nebula sprites + atlas selection, IN_GALAXY only | Dso3d | **visible** | `inGalaxyModule.cpp:141`; sprite law `dso3d.geom:33-36` | same objects at the same apparent positions and sizes in the in-galaxy regime; sprite law is a rendering internal, the apparent size is not |
| D2 | 30432 points + the square overlay, IN_UNIVERSE only | Tully | **visible** | `inUniverseModule.cpp:115,119`; `tully.cpp:412-468` | same |
| D3 | named OJM meshes per mode | OjmMgr | **visible** (560 field lines) | `ojm_mgr.cpp:143-172` | same meshes, same placement, same mode semantics |
| D4 | 10000 oort points, 1e13..1e16 m | Oort | **visible** (config default on) | `oort.cpp:159-162` | S11.98(b)'s ladder is the measured baseline |
| D5 | the far edge: old hard-cuts at 1e16 m, the module fades physically | Oort | **visible**, and the two paths already DIFFER | `oort.cpp:159` vs the regime ladder | **owner's** (S11.96(e)(4)); a port must not silently pick one |
| D6 | OjmMgr clears the whole depth attachment every frame it draws | OjmMgr | **visible** (anything drawn before it in `PASS_MULTISAMPLE_DEPTH` loses its depth) | `ojm_mgr.cpp:187-189` + unconditional execute `:171` | a port that draws the same meshes through the tree's depth slices will NOT reproduce this; it is a rendering-order effect with a visible consequence -> S5 slice 3's check |
| D7 | Tully's two half-catalogues swap order when the camera crosses the volumetric galaxy | Tully | **visible** | `tully.cpp:516-529` | preserved or consciously retired; it is a depth-order effect, not a content effect |
| D8 | the Milky Way's own OJM model, loaded IN_UNIVERSE by the engine as the `else` arm of `volumGalaxy->loaded()` | OjmMgr | **visible when the volumetric galaxy is absent** - i.e. data-dependent, and neither arm is a command | `core.cpp:479-486` | the port owes BOTH arms: re-routing the command alone drops this model silently, on exactly the installs that have no volumetric galaxy |

### 2.2 Selection and navigation reach

| id | channel | class | today | mechanism | obligation |
|---|---|---|---|---|---|
| S1 | a Tully galaxy can be SELECTED, and reports type/name/RA-DE/Alt-Az | Tully | **visible** | `core.cpp:1308-1309`, `tullyWrapper.cpp:24-53` | preserve, or retire with the owner's word; a `ModularBody`-based port gets selection from `ModularSystem::findBodyAt` instead, which is a DIFFERENT picker (S3.5) |
| S2 | a Dso3d nebula cannot be selected today | Dso3d | **never** today, **may-become** after the port | no `searchAround` for `Dso3d` `[measured: grep -n searchAround src/coreModule/core.cpp]` | porting them as bodies MAKES them selectable; that is a new observable and must be either wanted or gated |
| S3 | an OJM model cannot be selected today | OjmMgr | same as S2 | - | same |
| R1 | the camera's reference body changes with distance | all | **visible** (altitude readout, movement speed, `moveto altitude` datum) | `ModularBody.hpp:1576-1593` -> `experimentalModule/Camera.cpp:757` | **the reach half**: a decoration must not capture the reference (S3.2) |
| R2 | a node's collapse/resolve threshold | all | **visible** (a system becomes a dot) | `ModularSystem.cpp:772-776` | a decoration's extent must not move another node's threshold |
| R3 | the AoI sibling cap | all | **visible** through R1 | `ModularBody.cpp:681-683` | as R1 |

**R1-R3 are the S11.96(c) coupling, and the as-if rule decides them in one
sentence**: the reach effects ARE visible (they change what the observer can do
and what the altitude readout says), so they may not be changed as a side effect
of adding content - but they are visible as *the reference's* behaviour, not as
*the decoration's*, so the coupling is broken by making the decoration's extent
stop feeding reach, NOT by shrinking its extent (which would break D1/D2/D4).
That is S3.2.

### 2.3 Labels, fonts, faders

| id | channel | today | mechanism | obligation |
|---|---|---|---|---|
| L1 | Dso3d names, font CLASS_NEBULAE, colour `nebula_label_color` | **visible** (6 field files switch them) | `dso3d.cpp:221-236` | same names at the same positions with the same font class |
| L2 | Dso3d names are driven by `flag nebula_names` at runtime but by `flag_star_name` at startup | **visible** (a config-vs-command disagreement a user can observe) | `core.cpp:366` vs `app_command_interface.cpp:1167-1169` | **owner's**: preserve the oddity or unify (S6 F117-a) |
| L3 | Tully names ride `flag star_names` | **visible** | `app_command_interface.cpp:1088-1090` | same word must keep driving them, or the change is disclosed |
| L4 | Tully labels only within `distanceGal < 0.1` | **visible** | `tully.cpp:581` | a distance-gated label rule, not a magnitude one - the port must not substitute a magnitude rule silently |
| F1 | show/hide fades over the `LinearFader` duration | **visible** | `dso3d.hpp:118`, `tully.hpp:148` | the new path's `show()` is instant (S11.102(e5)) - the same operator-visible difference the oort pilot already carries |
| F2 | the ONSET ramp: old ramps intensity, the module switches sharply | **visible** in 67..112 AU | S11.98(b) measured | **owner's** (S11.98(f)(iii)) |
| F3 | fader DURATION setters exist for both clouds and no command reaches them | **never** | `coreLink.cpp:863,971` + `[measured: grep]` | free: a port may drop them; dropping them is recorded, not silent |

### 2.4 Commands, config keys, UI

| id | channel | today | mechanism | obligation |
|---|---|---|---|---|
| C1 | `dso2D action load` adds a nebula at runtime | **may-become** (0 field uses, the command exists and works) | `app_command_interface.cpp:1699-1712` | the port owes an equivalent, or the retirement is disclosed (D9: the command word is product surface) |
| C2 | `dso2D action clear` restores the catalogue | **may-become** | `core.cpp:2640-2643` | same |
| C3 | `body ... mode <m> action load/remove/clear` | **visible** (560 lines) | `app_command_interface.cpp:4119-4135` | **the port must keep this exact grammar working**; this is the single highest-traffic surface in the whole layer |
| C4 | `flag oort` / `flag tully` / `flag tully_color_mode` / `flag nebulae` / `flag nebula_names` / `flag star_names` | **visible** (oort/tully unused in the field but reachable) | `app_command_interface.cpp:1211-1223, 1088, 1167` | every token keeps its meaning; `readFlag`'s "primary state" contract `[observed: app_command_interface.cpp:490-493]` keeps its answer |
| C5 | `get` / script replies of those flags | **visible** | `app_command_interface.cpp:737-744` | unchanged answers |
| K1 | `oort_elements`, `oort_color`, `flag_oort` | **visible** | `define_key.hpp:117,197,285`; `core.cpp:459,468,630` | preserved by the pilot already |
| K2 | `flag_star_name` -> Tully + Dso3d labels at startup | **visible** | `core.cpp:366,472` | see L2/L3 |
| K3 | `nebula_label_color` -> Dso3d labels | **visible** | `core.cpp:1674` | preserved |
| K4 | `flag_nebula` does NOT reach Dso3d at startup | **visible as an absence** | `core.cpp:374` | a port that wires it would be a behaviour change, not a fix |
| K5 | no config key exists for Tully's or Dso3d's show state | **visible as an absence** | `tully.cpp:47`, `dso3d.cpp:35` | a port that adds one adds product surface (D9) |
| U1 | Ctrl+F toggles the oort flag and runs `internal/comet.sts` | **visible** | `ui.cpp:1411,1432-1436` | unchanged |
| U2 | startup clears all three OJM modes | **visible** | `ui.cpp:245-247` | the port owes the same clear, whatever it is called then |
| C6 | the tokens the census found and this design does NOT touch: `dso3d ...` (3 field lines, reaches `DsoNavigator`) and `dso ...` (544 field lines, reaches the 2-D `NebulaMgr` layer) | **visible**, both | `app_command_interface.cpp:1676` and `:1599` | listed so no token the census found is left without a row: both keep their present meaning, and a port of THIS layer must not change either - which is also why the `dso3d`/`dso2D` inversion is not repaired in passing (S6, F117-f) |
| C7 | `flag milky_way` (25 field lines) drives the old `MilkyWay` sky engine, which `MilkyWayEnv` wraps | **visible** | `app_command_interface.cpp:1172-1174`; `MilkyWayEnv.hpp:27-35` | after the port the word still drives the BACKDROP and NOT the Tully module hanging off the same node - an operator could expect otherwise, so it is named here rather than discovered; changing it would be a new coupling, not a restoration |

### 2.5 What the table classifies as NEVER-visible (free to change)

- The pre-recorded command-buffer strategy of each class (`dso3d.cpp:87-102`,
  `tully.cpp:276-325`, `ojm_mgr.cpp:174-202`) - an internal.
- The per-frame CPU rebuild of Tully's square list (`tully.cpp:412-468`): its
  OUTPUT is visible (D2), its schedule is not - which is what licenses the
  batched-family form in S3.6.
- The unused `distance` parameter of `Dso3d::draw` `[observed: dso3d.cpp:238]`.
- `OjmMgr::update`, an empty function `[observed: ojm_mgr.cpp:140-141]`.
- The fader-duration setters of F3.
- The `rand()`-to-`mt19937` change already landed in the oort's law (S11.98(a),
  authorized).

---

## 3. The design

### 3.1 The tree each content class hangs from

The new path's spine already exists and is explicit:

```
Universe        ModularSystem, BodyType::SYSTEM, radius 0   [ssystem_factory.cpp:92-111]
  +-- MilkyWay  ModularSystem, BodyType::GALAXY, radius 3.2e9 AU, INNER child
                                                            [ssystem_factory.cpp:117-139]
        +-- Solar   ModularSystem ("ssystem.ini")            [ssystem_factory.cpp:146]
              +-- Sun, planets, ... and the pilot's "Oort"   [ssystem_factory.cpp:590]
```

with the comment that states the intent in the owner's own vocabulary: *"The
nesting spine (G2, INTENT 11.36): universe > milkyway > systems. ... the milkyway
is its INNER child - a galaxy is a body like any other, and its 2D backdrop
(MilkyWayEnv, wired in wireEnvironment) shows only while the camera's reference
chain includes it"* `[observed: ssystem_factory.cpp:81-86]`.

**"The milkyway" in the owner's sentence has two candidate referents and the note
must pick one.** They are:

- **(a) the `MilkyWay` NODE** - the `ModularSystem` at `ssystem_factory.cpp:139`.
  It has a frame, a radius, children, a regime and a depth slice.
- **(b) `MilkyWayEnv`** - an `EnvironmentModule` `[observed: MilkyWayEnv.hpp:25]`
  wrapping the old 2-D backdrop engine, selected by chain membership
  `[observed: EnvironmentModule.hpp:38-45]`, drawn camera-relative.

**Pick (a), and the reason is a property of the content, not a preference**:
Tully's galaxies, the dso3d nebulae and the OJM models all have POSITIONS in a
frame, and they must keep them (rows D1, D2, D3). An `EnvironmentModule` draws a
backdrop whose geometry is a function of the camera's orientation, not of a
world position - it is the right home for the Milky Way's own texture and the
wrong home for anything the observer can fly past. The owner's phrase *"BodyModule
of the milkyway"* names (a) by construction: `BodyModule` is the `ModularBody`
slot interface `[observed: BodyModule.hpp:162]`, and only (a) is a `ModularBody`.

**Node assignment, per class**:

| content | node | relation | why |
|---|---|---|---|
| the 32 `dso3d` nebulae | children of **MilkyWay** | `BodyRelation::INNER` | they are inside the galaxy; INNER is the relation the spine already uses for a body inside its parent's extent `[observed: ssystem_factory.cpp:139]` |
| `mode in_galaxy` OJM models | children of **MilkyWay** | INNER | the mode word means "placed in the galaxy"; 527 of the 560 field lines are this |
| `mode in_universe` OJM models | children of **Universe** | INNER | same argument one level up |
| `mode in_sandbox` OJM models | **undecided** - S3.8 (V-D3) | - | the sandbox executor draws nothing today `[observed: inSandBoxModule.cpp:132-135, commented out]`, so there is no behaviour to preserve and no node the word points at |
| the Tully catalogue | a **`BodyModule` of the MilkyWay node** | - | 30432 objects; one module, one buffer, one draw (S3.5) |
| the oort | unchanged: ORBITING child of "Solar" | - | the pilot's choice, already measured (S11.96(a)) |

### 3.2 The reach/visibility split - the mechanism (answers S11.96(e)(2))

**The coupling, at source.** One member carries both meanings:

```
boundingRadius = scaledRadius;                        [ModularBody.cpp:613]
for (auto &module : nearComponents) { ... if (boundingRadius < module->getBoundingRadius())
                                           boundingRadius = module->getBoundingRadius(); }
                                                      [ModularBody.cpp:614-617]
```

and it then feeds, without any further filter:

- **visibility**: the cone test `[ModularBody.hpp:440-441, 464-465]`, `screenSize`
  `[ModularBody.hpp:477]` and thus every G4 gate `[ModularBody.hpp:234-259]`, and the
  depth slice `[ModularBody.hpp:612, 616, 640]`;
- **reach**: `subsystemRadius = 1.1 * max(boundingRadius, children)`
  `[ModularBody.cpp:658-665]` and `areaOfInfluence = max(boundingRadius * 128 /
  displayScaling, subsystemRadius * 16)` `[ModularBody.cpp:680]`, which decide the
  camera's reference through `isInAreaOfInfluence` `[ModularBody.hpp:1688]` and
  `findBetterReference` `[ModularBody.hpp:1576-1593]`, called from `Camera::update`
  `[experimentalModule/Camera.cpp:757]`.

The code itself records that this is undecided and why: *"the REACH half of the
scaled-bounding coupling is S11.96(e)'s promotion-grade item and D21 does NOT
decide it"* `[observed: ModularBody.cpp:674-678]`.

**The decision, from the as-if table.** Rows D1/D2/D4 say the drawn extent must
stay the content's true extent (an oort seen from inside is all-sky; a Tully
cloud is 30 Mpc across). Rows R1-R3 say the reference behaviour must not change
because a decoration was added. Both are visible. Therefore the two meanings must
become two quantities. The as-if rule does not merely permit this split - it
**requires** it, because keeping one quantity forces breaking one of the two
visible channels.

**The shape, chosen so that every body that exists today is bit-identical.**

1. `BodyModule` gains one more declarative trait bit, in the existing
   `BodyModuleTraits` enum `[observed: BodyModule.hpp:65-101]`, with the next free
   value (0x200 is RETIRED-but-reserved `[observed: BodyModule.hpp:78-84]`, so the
   next free bit is 0x1000):
   `BMT_NO_REACH = 0x00001000, // extent is visibility only: excluded from the reach max`.
2. `ModularBody::updateCache` keeps ONE loop and computes TWO maxima: the existing
   `boundingRadius` over all near components (unchanged, so every visibility gate
   and every depth slice keeps today's value to the bit), and a new
   `reachRadius`, the same max taken over the near components that do NOT declare
   `BMT_NO_REACH`, seeded like the other from `scaledRadius`.
3. `ModularBody::updateReach` reads `reachRadius` where it reads `boundingRadius`
   today `[ModularBody.cpp:658, 680]`. Nothing else changes.

**Why this is inert by construction**: no module in the tree declares the new bit
on the day it lands, so `reachRadius == boundingRadius` for every body, and
`updateReach` computes the same two floats it computes today. The change is
therefore a refactor with a *capability*, and the capability is exercised only by
the new decoration modules, which declare it. This is the S2(a2) test: it
declares a capability, it forecloses nothing.

**Why not the alternatives** (each was considered and is recorded with its
defect):

- *Shrink the decoration's `boundingRadius` to the body radius* - what the pilot's
  `OORT_REGIME_RADIUS_AU = 50` peg half-does. Breaks D1/D2/D4: the cone test then
  culls the surrounding cloud whenever the view points away from centre
  (`OortModule.hpp:44-48` states exactly this), and the depth slice stops
  bracketing the geometry, which the rasterizer's NDC clip turns into "drawn
  nothing" (`ModularBody.hpp:635-639`).
- *Cap the AoI by body type* (`BodyType::VOID`, `GALAXY`). Type is not the
  property being tested - a VOID body could legitimately be navigable, and the
  same body may carry one module that is a decoration and one that is not. I4:
  the property belongs to the module that inflates the extent.
- *A per-body data key* (`no_reach = true`). That is product surface (D9) for a
  fact no author needs to know; and it puts the declaration one level away from
  the module whose extent causes it.
- *Filter in `findBetterReference` instead* - i.e. let the AoI inflate and refuse
  to switch to decorations. Wrong site twice over: `subsystemRadius` would still
  be inflated (R2's collapse thresholds move for the PARENT), and the refusal
  would have to name a class of bodies at the navigation layer, which is where
  S11.96(c) says the conflation already hurts.

**The pilot's peg, disposed by this mechanism.** With the split in place, the
oort body no longer needs `radius = 50 AU` to protect the reference; the radius
becomes a pure regime low-edge value and can be set from the measured old gate
(S11.98(b): old shows from 66.8 AU, ramped to ~134 AU) rather than tuned against
a hijack. That retires the "empirical peg" half of S11.96(e)(5); what remains
of that item is the *onset semantics* question, which is S11.98(f)(i)/(iii) and
stays the owner's (S4).

### 3.3 `Dso3d` -> 32 `ModularBody` children of the MilkyWay node

- **One body per catalogue record**, `still_orbit` with the converted position
  (S1.6), `radius` = the catalogue's `size` field in AU after the same conversion,
  `BodyType::VOID` (the pilot's precedent for a decoration
  `[observed: ssystem_factory.cpp:587]`), halo disabled.
- **One module**, in the CUSTOM family with an explicit slot (`"DSO_SPRITE"`),
  the GRID/OORT precedent `[observed: modules.cpp:66-68]`, registered like
  `OortLoader` and keyed on an explicit opt-in marker so it competes for nothing
  else `[observed: OortLoader.cpp:5-11]`. It declares `BMT_NO_REACH` (S3.2) and
  `BMT_TRANSLUCENT` (it is additively blended
  `[observed: dso3d.cpp:70]`, and `addNearComponent` orders translucent modules
  after opaque ones `[observed: ModuleLoader.hpp:37-55]`).
- **The sprite geometry stays the old one**: `dso3d.vert/geom/frag` used
  VERBATIM (the I2 shape the oort pilot proved - `oort.vert/frag` are shared
  between the two paths `[observed: OortModule.cpp:63]`), with the atlas texture
  bound per module. The sprite radius law `scale*60/distance` is then computed in
  the SAME shader from the SAME quantities; only the matrix supplying the frame
  changes.
- **The labels** are the `HINT` slot's job, not the sprite's: the new path already
  has a per-body label module and a batched hint service
  `[observed: modules.cpp:58; PipelineRegistry.cpp:988-1023]`. Row L1's obligation
  is then met by the body's name and label colour, and the old `drawDsoName` loop
  disappears. **The residual to check** (S5 slice 2) is that the hint service's
  placement and font class produce the same label positions the
  `printGravity180` call produced `[observed: dso3d.cpp:234]`.
- **Why bodies and not one module with 32 sprites**: 32 is small; each nebula has
  a name, a position and a size, i.e. exactly what a body is; and it makes rows
  S2/C1/C2 expressible - `dso2D action load` becomes "create a body", which the
  command layer already knows how to do. The owner's *"must be bodies"* and the
  structure agree.

### 3.4 `OjmMgr` -> `ModularBody` + the OJM slot that already exists

This is the cheapest of the three and the highest-traffic (row C3, 560 lines).

- The new path **already has an OJM module and its loader**
  `[observed: modules.cpp:61, bodyModules/OjmModule.*]`. An `OjmMgr` entry is
  therefore a body with an `OJM` slot module, created under the node the `mode`
  word names (S3.1).
- **The command grammar does not change**: `commandBody`'s mode branch
  `[observed: app_command_interface.cpp:4119-4135]` keeps its words and its
  arguments, and routes to a body creation instead of `OjmMgr::load`. The
  `filename -> filename/filename.ojm` convention `[observed: :4122]` moves with
  it unchanged.
- **What must be preserved explicitly**, because it is not automatic:
  1. `scale * model radius` `[observed: ojm_mgr.cpp:70]` - the authored scale is
     relative to the model's own extent.
  2. `remove` / `clear` by mode `[observed: ojm_mgr.cpp:91-138]` - the new path's
     equivalent is body removal restricted to the same node, and the startup clear
     of row U2 `[observed: ui.cpp:245-247]`.
  3. **Row D6, the depth clear** `[observed: ojm_mgr.cpp:187-189]`. The old manager
     wipes the depth buffer before drawing; bodies get per-body depth slices
     instead `[observed: ModularBody.hpp:612]`. The visible consequence is
     occlusion ordering against everything else drawn in the same pass. This is
     the one place where a faithful port and a correct port may differ, and S5
     slice 3 makes the difference measurable before anything is decided.
  4. **Row D8, the engine's own model**: `Core::init` loads `Milkyway.ojm`
     into `in_universe` when the volumetric galaxy is absent
     `[observed: core.cpp:479-486]`. The port owes an equivalent engine-side
     creation under the Universe node, gated on the same condition - and it is
     the one piece of this class that is NOT reached by re-routing the command.
  5. The `getOk()`-only success test and its 200 measured false successes
     (S11.222(g3)) must NOT be reproduced: the new path's load authority is
     `ModularSystem::loadBody`, and S11.222(h)(2) already names it as the
     responsibility anchor for a refusal.

### 3.5 `Tully` -> a `BodyModule` of the MilkyWay node

- **One module, the whole catalogue.** The module owns the static point buffer
  (built once from `tully.dat`), the big-catalogue buffer (`6df.dat`), the atlas,
  and the per-frame square list. It declares `BMT_NO_REACH` and
  `BMT_TRANSLUCENT`, and is routed as a NEAR component of the MilkyWay node, so
  the node's own regime machinery gates it - exactly the `OortLoader` shape
  `[observed: OortLoader.cpp:18-30]`.
- **The bounding radius it reports is the catalogue's extent** (visibility), and
  the MilkyWay node's reach is unaffected (S3.2). Without the split this single
  module would multiply the MilkyWay node's AoI by the ratio of the Tully extent
  to 3.2e9 AU - the S11.96(c) hijack, at galaxy scale.
- **The per-frame square pass is the only part that needs a service** (S3.6).
- **Selection (row S1) is the open half.** `TullyWrapper` exists because the old
  selection layer is `ObjectBase`-based `[observed: TullyWrapper.hpp:30]`; the new
  path selects `ModularBody`s through `ModularSystem::findBodyAt`
  `[observed: ModularSystem.cpp:1473]`, and a catalogue inside a module is not a
  body. Three readings, priced, none chosen here (S3.8, V-D4):
  1. **Keep `TullyWrapper`**: the module answers a `searchAround`-shaped query and
     builds wrappers as today. Cost: the old selection object survives the port,
     and the new path's picker still cannot see galaxies.
  2. **Promote the selected galaxy to a transient body** on pick. Cost: a body
     that exists only while selected - new lifetime semantics (I5).
  3. **Retire the selection** for Tully. Cost: a visible channel disappears; the
     owner's word is required (row S1 is `visible`, not `may-become`).

### 3.6 The renderer role Tully needs (A6 / S11.51(e)), measured against what exists

A6's resolution says: *"ONE Renderer instanced-batch service (the INSTANCED
hint's intended role) - generalize the live TAIL batch (S11.43), don't duplicate
it (I2); INSTANCED autodetected AND manually declarable"* `[observed: INTENT.md:1190,
B2 row]`. Measured at HEAD, that description does not match the tree, and the
difference changes what Tully should use:

- **There is no `INSTANCED` hint anywhere**: `grep -rn '\bINSTANCED\b' src/`
  returns nothing `[measured]`. Instancing exists only as a Vulkan vertex input
  rate, and the new path uses it in exactly one place - the TAIL batch's binding 1
  `[observed: PipelineRegistry.cpp:1065]`.
- **The TAIL batch is a family with fixed geometry and per-instance rows**
  (`NB_MAX_TAILS = 1024`, one `vkCmdDrawIndexed` for all of them
  `[observed: PipelineRegistry.cpp:1183]`) - the right shape for *many copies of one
  mesh*, which is what a tail is and what an asteroid ring would be.
- **The generic batching service is a different thing and it is the right one
  here**: `PipelineFamilyDesc::batch` / `BatchDesc{instanceStride,
  perFrameCapacity}` `[observed: PipelineFamily.hpp:227-231]` with
  `Renderer::batchPush` `[observed: Renderer.hpp:317]` accumulates per-frame rows
  into one buffer and issues one `vkCmdDraw` per family
  `[observed: PipelineRegistry.cpp:894]`. The HALO service uses it with a 6-float
  stride and a capacity of 8192 `[observed: PipelineRegistry.cpp:952]`; the HINT
  service with 6 floats and 48*128 `[observed: PipelineRegistry.cpp:1020]`. The
  header even states the boundary: *"Shared geometry (Tail-style instanced strips)
  comes from the family's VertexArray instance-rate entries, not from this
  struct"* `[observed: PipelineFamily.hpp:225]`.

**So Tully needs two things and neither is new machinery**: (i) a static
point-list family with its own vertex buffer - the `OortModule` shape
`[observed: OortModule.cpp:35, 57-73]`; (ii) the existing batched family for the
per-frame square list, with a stride of 5 floats `[observed: tully.cpp:447-456]`
and a capacity sized by the measured survivor count. **What it does need from the
service is one generalization**: today's capacities are compile-time constants
per family; Tully's survivor count is data-sized. The slice that lands Tully
therefore owes a capacity that is declared from the catalogue size, and an
overflow behaviour that is logged rather than silent (D12) - the TAIL batch's own
clamp-and-log-once is the precedent `[observed: PipelineRegistry.cpp:1146-1160]`.

**A6 itself is NOT discharged by this note**: nothing here needs instancing, so
nothing here can decide the autodetect-vs-declare question. Recorded, unchanged,
at its row.

### 3.7 The four executor draw sites, and S11.223(e)(2)

Today: `Dso3d` draws only from IN_GALAXY `[observed: inGalaxyModule.cpp:141]`,
`Tully` only from IN_UNIVERSE `[observed: inUniverseModule.cpp:115,119]`, `OjmMgr`
from both with a state argument
`[observed: inGalaxyModule.cpp:142, inUniverseModule.cpp:121]`, `Oort` from SOLAR_SYSTEM
`[observed: solarSystemModule.cpp:203]`. The new path already draws in every mode
from the same four sites `[observed: inGalaxyModule.cpp:151, inUniverseModule.cpp:133]`,
which is S11.80's draw-half.

**What the port does to those sites**: each class's draw call is deleted when the
class's content has become tree content, and NOTHING replaces it - the content is
drawn by `drawExperimental` in every mode, gated by the tree's own regime and
chain machinery. The mode-conditional visibility that the old sites express
becomes chain membership (the comment at `inGalaxyModule.cpp:146-150` already
says so for bodies), which is exactly the owner's architectural line: *"a system
which can be seen from outside as it is shown from inGalaxy mode"* [vixy,
S11.223(e)(2)].

**This note relates to that line; it does not enact it.** The line is about the
executor MODES going away; the port only removes four draw calls and lets the
tree draw their content. The modes' other duties - altitude bands, screen fader
interludes, the mode switch itself `[observed: inUniverseModule.cpp:45-46, :98]` -
are untouched and out of scope here.

**The consequence that must be measured, not assumed** (S5 slice 1's check): a
tree-drawn dso3d nebula is visible whenever the chain resolves it, which may be
in modes where the old layer drew nothing. That is a row-D1 change (content
appearing where it did not) and it is the single most likely source of a
surprise. The first slice therefore lands with the old site still drawing and the
new content flag-gated OFF - the pilot's exact discipline
`[observed: solarSystemModule.cpp:196-203]`.

### 3.8 What this design deliberately does not decide

Recorded as veto points (cheap to reverse, silence = endorsed) and owner
decisions (nothing here depends on them; every slice in S5 stands without them).

**Veto points I take:**

- **V1 - the split is `boundingRadius` + `reachRadius`, declared by a module
  trait** (S3.2). Reversal cost: one enum value, one member, two lines in
  `updateReach`. The alternative homes are listed with their defects in S3.2.
- **V2 - the DSO sprite module rides the CUSTOM family with an explicit slot**,
  the GRID/OORT precedent, rather than claiming the reserved `VOLUMETRIC` type
  `[observed: BodyModule.hpp:20]`. Reversal: one enum value and one
  `registerModule` line. Reason: `VOLUMETRIC` is unregistered today
  `[observed: modules.cpp:55-68 - no VOLUMETRIC row]` and its intended meaning
  (a 3-D texture volume, what `DsoNavigator` draws) is not what a sprite is.
- **V3 - the ported OJM content keeps the command grammar unchanged** (row C3),
  including the `filename/filename.ojm` convention. Reversal: the command layer.
- **V4 - labels move to the HINT slot** rather than being re-implemented inside
  the sprite module (S3.3). Reversal: one module.

**Not mine - the owner's, with both readings:**

- **V-D1 - engine-instantiated or authored?** The oort pilot is engine-built
  (D9-safe, no field edit); the dso3d nebulae and the OJM models are FIELD
  content today (a catalogue file and 560 script lines). Reading A: keep them
  engine-instantiated from the same files, no product surface, no D9 exposure.
  Reading B: give them a data home in the composed format, which is new grammar
  (B28 sign-off) but makes them first-class bodies with all the authoring the
  format already offers. Cost of A: two loaders forever. Cost of B: product
  surface, and a migration for the field's 527 lines.
- **V-D2 - the catalogue-to-AU factors** (S1.6). Not a preference: a measurement
  plus a ratification. What the owner owes is the ratification; what a task owes
  is the cited measurement and the cross-entry agreement check.
- **V-D3 - what `mode in_sandbox` means after the port** (S3.1). The executor
  draws nothing today; the command accepts the word.
- **V-D4 - Tully's selection** (S3.5, row S1), three readings priced.
- **V-D5 - row D6, the OJM depth clear**: preserve the clear (faithful) or adopt
  per-body depth slices (correct, and different where models overlap other
  content). S5 slice 3 measures the difference first.
- **V-D6 - rows D5/F2**, unchanged since the pilot: the far-edge hard cut vs the
  physical fade, and the onset ramp vs the sharp switch.
- **V-D7 - row L2**, the startup/runtime flag disagreement for Dso3d's labels:
  preserve or unify.

---

## 4. Disposition of the nine suspended items

| item | statement | disposition |
|---|---|---|
| **S11.96(e)(1)** | point-cloud module family: VOLUMETRIC slot vs MINOR_BODY/cluster traits | **ANSWERED** by S3.3/V2 and S3.6: the family is CUSTOM + explicit slot for the sprites, and the point clouds need no family of their own - they need the existing batched-family service. `VOLUMETRIC` stays unregistered and reserved for `DsoNavigator`'s kind of content. |
| **S11.96(e)(2)** | the reach-vs-visibility decoupling MECHANISM | **ANSWERED** by S3.2 (rows R1-R3 of the as-if table force it; `BMT_NO_REACH` + `reachRadius` is the shape, inert by construction). |
| **S11.96(e)(3)** | the oort's true floor: SolarSystem child vs EnvironmentModule backdrop vs a new intermediate floor | **ANSWERED** by S3.1 and S3.2 together: the EnvironmentModule option was the way to side-step reach, and the split removes the reason to side-step it; the oort stays a SolarSystem child, as piloted. The "new intermediate floor" is not needed - the spine already has three levels. |
| **S11.96(e)(4)** | high-edge semantics: old hard cut vs physical fade | **OWNER'S** (V-D6). Row D5 says it is visible and that the two paths already differ; no criterion in this note picks between two physically defensible behaviours. |
| **S11.96(e)(5)** | the `OORT_REGIME_RADIUS_AU = 50` empirical peg | **ANSWERED IN PART** by S3.2: with reach split off, the peg is no longer load-bearing for the reference and becomes a pure regime low-edge value that S11.98(b)'s measurement can set. The residual - which onset the operator should see - merges into S11.98(f)(i). |
| **S11.96(e)(6)** | oort selectability (huge-screenSize `findBodyAt` candidate) | **ANSWERED** by rows S2/S3 + S3.2: a decoration body declaring `BMT_NO_REACH` is still a `findBodyAt` candidate, because that picker ranks by `screenSize` and distance, not by reach `[observed: ModularSystem.cpp:1492, 1518-1526]`. So the port owes an explicit decision per content class, and the note's answer is: decorations are NOT selectable unless a row of S2 says they are today (S1 says Tully is; S2/S3 say the others are not). Implementation: the picker's candidate test excludes bodies whose only modules declare `BMT_NO_REACH` - stated as the mechanism, measured by S5 slice 1's check 4. |
| **S11.98(f)(i)** | the peg's measured correction: lower ~50 -> ~33 AU, or rework the gate to old's altitude semantics | **OWNER'S** (V-D6), now decoupled from reach by S3.2 - i.e. it is a pure appearance question and can be answered without touching navigation. |
| **S11.98(f)(ii)** | gating QUANTITY: home-planet altitude (old) vs heliocentric regime (new) | **ANSWERED as a consequence, not as a preference**: once the content is tree content, the gating quantity IS the body's regime, because there is no executor left to supply an altitude (S3.7). The old quantity cannot be preserved without re-introducing a mode. This is a visible change on row D4 and it is disclosed here rather than discovered later. |
| **S11.98(f)(iii)** | whether old's intensity RAMP should be reproduced | **OWNER'S** (V-D6, row F2). Note the ramp is expressible either way now: a fader on the module reproduces it, the regime ladder alone does not. |

**Score**: five answered by the criterion, one answered in part, three left to the
owner with both readings - and the three that are left are all the same KIND of
question (which of two defensible appearances the audience should see), which is
the kind S11.161(c) routes to the owner and the tester, not to an executor.

---

## 5. The slices

Ordered, decision-free (none of V-D1..V-D7 gates any of them), sized, each with
its discriminating check stated so it CAN fail and its parity instrument. The
byte-unchanged assertion for the flag-off tree is the pilot's, and it is
repeated in every slice because it is what makes the slices safe to land one at a
time: **with the new content's flag off, every existing scene must be
byte-identical to the pre-change binary.**

### Slice 1 (S) - the reach/visibility split, alone, with nothing using it

**Scope**: `BMT_NO_REACH` in `BodyModuleTraits`; `reachRadius` beside
`boundingRadius` in `ModularBody`; `updateReach` reads it. No module declares it.
No content moves.

**Predictions, before the run**: (1) every dump value is unchanged, including
`boundingRadius`, `subsystemRadius` and `refAoI`; (2) the reference-transition
altitudes are unchanged; (3) the frame cost is unchanged within noise.

**Checks, each able to fail**:
1. `b24_equivalence` + `b5_drawhalf` + scene E: byte-identical to the pre-change
   binary. *Fails if any pixel moves.*
2. The dump's `boundingRadius` / `subsystemRadius` / `refAoI` columns compared
   pre/post over a fixed scene: identical to the last digit. *Fails on any delta -
   which is the point: the refactor claims bit-identity, so any delta is a bug in
   it.*
3. **The discrimination that makes 1 and 2 non-vacuous**: a MUTANT binary in
   which `OortModule` declares `BMT_NO_REACH` and the pilot flag is ON must move
   the reference-transition altitude (measured today at ~533 AU, S11.96(c)) and
   must NOT move any pixel of the oort cloud. *Fails if the mutant is
   indistinguishable from the control - which would mean the split is not wired
   to anything.*

**Instrument**: `b5_oort.py`'s shape (a parity ratio plus a path-identity
assertion, S11.103(d)) plus the dump comparison; the mutant leg is F104's shape
(a mutation binary built and measured beside the control, S11.225).

### Slice 2 (M) - the 32 dso3d nebulae as bodies, gated OFF

**Scope**: the sprite module + its loader + an explicit slot; the 32 bodies
created under the MilkyWay node behind a config flag
(`flag_experimental_dso3d`, the pilot's exact shape
`[observed: core.cpp:466]`); the old `Dso3d` draw site untouched; labels through
the HINT slot; the catalogue-to-AU factor as a named constant with its derivation
in the comment (V-D2 is the ratification, not the landing).

**Checks**:
1. Flag OFF: the whole tree byte-unchanged (the pilot's 24/24 shape).
2. Flag ON, in-galaxy scene: every one of the 32 nebulae is drawn, at an angular
   position matching the old sprite's to within the view-transform residual
   S11.98(e) measured (footprint A/B, not pixel A/B).
3. Flag ON: the camera's reference at a fixed in-galaxy altitude is the same body
   it is with the flag OFF. *Fails if a nebula captured the reference - i.e. if
   slice 1's split was not declared by this module.*
4. Flag ON: `findBodyAt` at a nebula's screen position selects what it selected
   before (row S2's decision made visible).
5. Labels: the label count and their screen positions against the old
   `drawDsoName` output at the same scene.

### Slice 3 (M) - the OJM models as bodies, gated OFF, and row D6 measured

**Scope**: route `body ... mode <m> action load` to a body creation under the
node the mode names, behind the same kind of flag; `remove`/`clear` by node; the
startup clear; the `scale * model radius` convention preserved.

**Checks**:
1. Flag OFF: byte-unchanged.
2. Flag ON, `fscripts/14.sts` replayed: the same 527 models exist, with the same
   names, at the same positions (the dump's body list is the instrument - no
   screenshot needed for existence).
3. **Row D6, stated as a check that can fail**: a scene with one OJM model and
   one other object that the old depth clear would let the model overdraw. The
   old path and the new path are compared on that scene. *Fails if they agree* -
   because then the depth clear was inert on this corpus and V-D5 is moot, which
   is itself the finding; *fails if they differ and no one looked*, which is what
   the check prevents.
4. The uniform-pool refusal path: with the pool exhausted, the new load authority
   says so (S2(f)) instead of logging success - S11.222(g3)'s 200 false
   successes must not reappear.

### Slice 4 (M) - Tully as a BodyModule of the MilkyWay node, gated OFF

**Scope**: the module (static points + big catalogue + the batched square pass),
its loader, the capacity generalization of S3.6 with a logged overflow; NOT
selection (row S1 waits on V-D4).

**Checks**:
1. Flag OFF: byte-unchanged.
2. Flag ON, in-universe scene: footprint parity against the old draw at three
   altitudes spanning the band, with the flag-off leg as the zero control
   (the 159x gate ratio of S11.96(a) is the precedent for a discrimination that
   proves the measurement is not vacuous).
3. The square pass: the survivor COUNT per frame equals the old
   `computeSquareGalaxies` count at the same camera position. *Fails on any
   difference - the survivor rule is the visible part of D2.*
4. The MilkyWay node's `subsystemRadius` and `refAoI` are unchanged by the
   module's presence. *Fails if the module inflated reach - the galaxy-scale
   instance of the S11.96(c) hijack.*
5. The volumetric-galaxy crossing (row D7) either reproduces the old plane-order
   swap or is recorded as retired, with the frame on which it happens named.

### Slice 5 (S) - retire the old draw sites, one class at a time

Only after the corresponding slice's flag has been ON by default for one full
battery. Each retirement is its own commit and its own check: the old site's
removal must leave the scene it drew byte-unchanged, because the tree now draws
the same content.

---

## 6. Out-of-scope findings, recorded and not fixed

- **F117-a - two operator words for one state.** Dso3d's label flag is written
  from `flag_star_name` at startup `[observed: core.cpp:366]` and from
  `flag nebula_names` at runtime `[observed: app_command_interface.cpp:1167-1169]`.
  A user who sets `flag_star_name = false` and then types `flag nebula_names on`
  gets nebula names; the config word that controls them is named after stars.
  Row L2; a S5 candidate for whoever holds the row (reachable from a shipped
  surface, with a consequence: the state after startup is not the state the
  config word names).
- **F117-b - `OjmMgr::removeAll` erases while iterating and then decrements the
  erased iterator** `[observed: ojm_mgr.cpp:129-137]`. `std::vector::erase`
  invalidates iterators at and after the erase point; the code relies on the
  invalidated iterator remaining usable, and on `--iter` at `begin()`. It works on
  every implementation this project ships against and is formally UB. Reachable
  from `body action clear mode in_galaxy` and from `UI::init`
  `[observed: ui.cpp:245-247]`.
- **F117-c - `OjmMgr::draw` executes its command buffer unconditionally**
  `[observed: ojm_mgr.cpp:171]`, so the full-screen depth clear at
  `[ojm_mgr.cpp:187-189]` happens in IN_GALAXY and IN_UNIVERSE on every frame even
  when no model is loaded for that state. Row D6's mechanism; measured by slice 3.
- **F117-d - two dead setter chains**: `CoreLink::dso3dSetDuration`
  `[observed: coreLink.cpp:863]` and `CoreLink::tullySetDuration`
  `[observed: coreLink.cpp:971]` have no caller `[measured: grep]`. Recorded so a
  future slice does not "restore" a channel that was never reachable.
- **F117-e - `Dso3d::draw`'s `distance` parameter is unused**
  `[observed: dso3d.cpp:238-249]` while `Tully::draw`'s drives the big-catalogue
  fader `[observed: tully.cpp:550]`. The two clouds have the same call shape and
  only one of them means it.
- **F117-f - the `dso3d` / `dso2D` token inversion** (S1.1). D9 product surface:
  the words are field-visible, so repairing them is an owner decision, not a
  cleanup.

---

## 7. Citation resolution (the proof of check (a))

Every `file:line` in this note was resolved against code `master-beta @ 87d429bd`
and harness `CC-harness @ 385fe60` by `harness/f117_cites.py`, whose output is
`artifacts/f117/citation-table.txt.gz`: for each citation, the file, the line, and
the line's text at HEAD. A citation that does not hold the text this note claims
is a defect of this note.
