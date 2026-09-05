# Developer entry document

For someone who has just cloned this repository and will change its code.
It is not a tutorial and not a user guide (the user guide is the rest of
`doc/`). It is the shortest path from a clone to a first change that is
correct for reasons you can check.

**Every claim below carries its source in brackets** -- a file (with a line
where the line is the point), or an entry in the project's reasoning ledger
written `Sec.N.M`, which you resolve in `claude/INTENT.md` or in
`claude/INTENT/<id>.md` where the entry has its own file. Nothing here is
asserted on memory, and every path and `Sec.` id in this file is checked
mechanically by `claude/harness/f85_links.py`. Where a question has no answer
yet, this file says so and names who can answer it: an invented answer costs
more than a gap.

## 1. Two repositories, one working tree

The code is here. The reasoning about the code -- why a thing is the way it
is, what was measured, what is still open -- lives in a **second repository
nested inside this one** at `<spacecrafter>/claude/` [`claude/README.md:20-24`],
on the same GitHub remote (`git@github.com:lionelruiz13/spacecrafter.git`
[observed: `git remote -v`, both repos]) but on an **orphan branch
`CC-harness`** with unrelated history [`:28-30`]. This repository's
`.gitignore` excludes `claude` and `.claude` [`.gitignore:84-85`], so a fresh
`git clone` of the code does **not** contain it [measured 2026-09-05]. To get
it beside the code:

    cd <spacecrafter>
    git clone -b CC-harness <same-remote-url> claude

The layout is a precondition, not a preference: harness scripts resolve the
binary at `../../build-claude/src/spacecrafter`, overridable with `SC_BIN`
[`claude/README.md:21-23`].

**The dependency is one-way.** The harness reasons about a code state; the
code never depends on the harness [`:3-7`]. That is why this file is here and
not there: a conclusion that constrains future code edits "must be promoted
into the code repo ... this repo is the lab notebook, not where other
contributors will look" [`:13-16`].

**Commit convention.** Every harness commit records the code state it was made
against, as a trailer `Code: <branch> @ <short-sha>`; when a change spans both
repos, **commit code first**, so the trailer names a commit that exists
[`:34-41`]. Never merge `CC-harness` into a code branch or the reverse -- git
will not stop you, and the result is a mess [`:28-30`]. The working branch is
`master-beta`; pull requests target `2023-master` [`claude/CLAUDE.md`].

## 2. The code map

`src/` has 18 top-level directories. Each line below is what the directory
owns, from a header in that directory where a header says it.

| directory | owns | source |
|---|---|---|
| `src/appModule/` | the application shell: main loop, framerate pacing, fades, screenshots, the mkfifo control pipe | no header statement; class `App` at `src/appModule/app.hpp:72` |
| `src/atmosphereModule/` | daylight sky colour, sky luminance, eye adaptation | `src/atmosphereModule/atmosphere.hpp:27` |
| `src/bodyModule/` | the OLD path's solar system: `Body`, orbits, and `SSystemFactory` over per-system display/colour/scale/texture/selection | `src/bodyModule/ssystem_factory.hpp:73` |
| `src/coreModule/` | `Core`, the main object holding and driving every sky manager | `src/coreModule/core.hpp:120` |
| `src/EntityCore/` | git submodule -- the Vulkan layer (see below) | `src/EntityCore/README.md:2` |
| `src/eventModule/` | the typed event bus and its handler registry | `src/eventModule/event_handler.hpp:51` |
| `src/executorModule/` | mode switching: which update/draw path runs at the current altitude | `src/executorModule/executor.hpp:48` |
| `src/experimentalModule/` | the NEW modular render path (see below) | `src/experimentalModule/Renderer.hpp:20` |
| `src/inGalaxyModule/` | in-galaxy 3D: the hypercube star database and its navigators | `src/inGalaxyModule/starManager.hpp:194` |
| `src/interfaceModule/` | the script command surface: parsing, variables, colours, `if` | `src/interfaceModule/app_command_eval.hpp:23` |
| `src/mainModule/` | process bootstrap: SDL window/context, config validation, CPU features, signals | no header statement; `SDLFacade` at `src/mainModule/sdl_facade.hpp:36` |
| `src/mediaModule/` | scriptable media: images, text, audio, subtitles, video, VR360 | no header statement; `Media` at `src/mediaModule/media.hpp:90` |
| `src/navModule/` | observer placement and viewing frame: anchors and the navigator | `src/navModule/anchor_manager.hpp:16` |
| `src/ojmModule/` | the OJM/OBJ model format: parse, upload, async load, sphere LOD | no header statement; `OjmMgr` at `src/ojmModule/ojm_mgr.hpp:43` |
| `src/scriptModule/` | script lifecycle: tokenize, record, play, annotate | no header statement; `ScriptMgr` at `src/scriptModule/script_mgr.hpp:47` |
| `src/starModule/` | the Hipparcos catalogue: geodesic zoning, packed zones, star draw | `src/starModule/hip_star_mgr.hpp:142` |
| `src/tools/` | cross-cutting utilities: math, faders, logging, INI parsing, translation, textures, the object base, the Vulkan `Context` | grab-bag, per-file briefs; e.g. `src/tools/app_settings.hpp:32` |
| `src/uiModule/` | input and on-screen UI: keyboard, mouse, joypad, the TUI | no header statement; `UI` at `src/uiModule/ui.hpp:91` |

**Headers are the specification.** They are the owner's formalization medium,
"the closest representation of what is wanted", so a header's comment block is
normative intent, not commentary; and when a header and the ledger disagree,
neither silently wins -- reconcile explicitly [`claude/INTENT.md` header].

### The new path, in one screen

`src/experimentalModule/` is the new body-rendering path. Its spine:

- `ModularBody` [`src/experimentalModule/ModularBody.hpp:348`] -- the tree
  node: owns its children, solves position, carries module slots.
- `ModularSystem` [`src/experimentalModule/ModularSystem.hpp:21`] -- the
  system level, which "decides WHICH bodies shadow which -- per-module hooks
  only say HOW" [`:8-20`].
- `ModularObject` [`src/experimentalModule/ModularObject.hpp:8`] -- the
  refcounted selection bridge to `ObjectBase`; a selection outlives the call
  that made it, and the body is not owned by it [`:13-23`].
- `Camera` [`src/experimentalModule/Camera.hpp:34`] -- owns reference, pose
  and modes, and "knows nothing about names"; the named-anchor vocabulary is
  `CameraAnchors` [`src/experimentalModule/CameraAnchors.hpp:16-23`].
- `Renderer` [`src/experimentalModule/Renderer.hpp:83`] -- "the ONLY Vulkan
  surface of the experimental module: bodies and modules DESCRIBE ..., the
  Renderer EXECUTES ... No module owns a pipeline or records into a raw
  command buffer on its own authority" [`:20-26`]. The descriptive half of
  that split is `src/experimentalModule/PipelineFamily.hpp:17-29`.

Bodies are built by loaders, and loader selection is **competitive**: each
`ModuleLoader` answers `isLikely()`, highest wins, 255 short-circuits, and no
capable loader means a logged warning and nothing installed
[`src/experimentalModule/ModuleLoaderMgr.hpp:22-31`,
`src/experimentalModule/ModuleLoader.hpp:8-22`]; the registration table is
`ModuleLoaderMgr::init` in `src/experimentalModule/modules.cpp:43` (9 orbit
loaders at `:45-53`, 14 module loaders at `:55-68`). A system file enters
through `ModularSystem::loadSystem` (legacy format,
`src/experimentalModule/ModularSystem.hpp:68`) or `loadComposedSystem`
(composed format, `:97`). Body and mesh modules derive from `BodyModule`
[`src/experimentalModule/BodyModule.hpp:162`], whose slot enum names which
loader pool handles a kind [`:12-16`]; environment modules derive from
`EnvironmentModule` [`src/experimentalModule/EnvironmentModule.hpp:103`],
scoped to "everything 'outside' whose rendering depends on the camera's
location relative to its body" [`:19-23`].

### `src/EntityCore/`

A git submodule [`.gitmodules:1-3`], described by its own README as a
"Vulkan-based engine used to simplify use of Vulkan and debugging"
[`src/EntityCore/README.md:2`]; four subdirectories under `src/EntityCore/`:
Core, Executor, Resource and Tools. It is the owner's stratum: **treat it as
read-only** unless he says otherwise, and route questions to him [`Sec.11.161`].

### The command surface

`src/interfaceModule/` and `src/scriptModule/` are the script command surface,
which has a second, machine-readable description outside the engine:
`util/scedit/grammar/sc-grammar.json` is "the single machine-readable contract
for spacecrafter's command surface" [`util/scedit/README.md:589-591`] and
`util/scedit/grammar/ss-grammar.json` a second contract for what the engine
reads out of a stellar-system file [`util/scedit/README.md:644-646`]. Both
cite engine `file:line`s, and a ctest gate checks that every one of those
citations still resolves [`:789`].

## 3. The two render paths, and which one draws

Both paths are alive simultaneously **by design**: old path =
`src/bodyModule` (`Body`, `ProtoSystem`, `SolarSystem*`), new path =
`src/experimentalModule` [`claude/INTENT.md` header, "Comparison baseline"].

**The new path draws by default.** Measured, not recalled, in three parts:

1. The startup selection is not in `config.ini` at all: it is
   `beta_features.ini`, section `[dual_path]`, key `render_path`
   [`src/mainModule/define_key.hpp:54-55`], read at
   `src/appModule/app.cpp:614-618`. That file is **optional** -- absence means
   every experimental setting at its default, and the loader returns false
   without touching anything [`src/tools/app_settings.cpp:111-123`].
2. With the file absent, `setRenderPathMode` is never called and the member
   defaults stand: `drawModularSystem = true`
   [`src/bodyModule/ssystem_factory.hpp:1070`], `pathPinned = true` [`:1080`].
   The header says the same thing: "The DEFAULT is NEW+pinned: the new path is
   what a user gets with no configuration at all, and nothing alternates on
   its own" [`:100-104`].
3. One launch from a `$HOME` that had never existed, with no
   `beta_features.ini` anywhere, dumped `"drawnPath":"new"`
   [measured 2026-09-05, `body action dual_dump` header; the field is written
   at `src/coreModule/coreLink.cpp:583`]. The bootstrap does not create a
   `beta_features.ini`, so a new install starts in the same state [measured].

The other values are `old` and `alternate`; `alternate` swaps the drawn path
once a second and exists "to make divergence visible, not to be run in"
[`src/bodyModule/ssystem_factory.hpp:104-106`, toggle at
`src/bodyModule/ssystem_factory.cpp:810-814`]. A script overrides all of this
at runtime with `flag experimental_path <on|off>`, which also pins the choice
[`src/bodyModule/ssystem_factory.hpp:113-117`,
`src/interfaceModule/base_command_interface.hpp:495`]. A second, independent
gate, `experimental_shadows`, IS a `config.ini` key
[`src/mainModule/define_key.hpp:120`, read at `src/appModule/app.cpp:605`].

**The old path is the comparison baseline and is unchanged by construction.**
Parity against it has two conditions, both stated by the owner: the
equivalence level is *reasonable vision capabilities* -- perceptual, not
bit-level -- and the parity domain is behaviour where the old path was
**already physically exact**. Where the old path had a defect that did not
reliably represent reality, the new path must **not** reproduce it, and the
defect is tracked as one [`Sec.11.52`, clause (b)]. So a divergence is not
automatically a bug: it is either a defect of the new path or a deliberate
correction of an old one, and the ledger says which. The deliberate, accepted
ones are a tier of their own -- `claude/DEPLOYMENT-MAP.md:383-398`, section
T4, things a user is INFORMED about rather than fixes [`Sec.11.116` (c)].

**Before you call something a new-path problem, check whether it is a
both-paths problem.** When thirteen suspected new-path defects were re-read at
source, ten lived in the command interface, the TUI, the config layer or the
old core and behaved identically whichever path drew -- so they were not
transparency holes at all, but behaviour the user already had [`Sec.11.163`
(h)]. The distinction changes who a defect belongs to and whether it blocks.

## 4. Domain constraints D8 to D14

These are the owner's constraints on the domain, not style rules. Each is one
paragraph here with a pointer; the full statement, with the owner's verbatim
words, is in `claude/INTENT.md` section 2.0 [`Sec.2.0`].

**D8 -- the as-if rule.** From the user's standpoint the software must behave
*as if* everything were physically exact and fully computed. Any internal
deviation -- lazy, frozen, skipped, iterative -- is valid exactly as long as
no user-reachable observable can tell [`Sec.2.0`, D8]. That is the whole
licence and the whole limit: the optimization is legal until an observable
channel reveals it, so an optimization ships with the check that would reveal it.

**D9 -- data is the product, and the field is frozen.** Code is open source;
the data is what is paid for. Deliveries are frozen individually and updates
are offline, operated by hand, so in-field data is immutable in practice --
corrections propagate forward into future deliveries, and backward
compatibility with an already-installed file is forced rather than chosen
[`Sec.2.0`, D9].

**D10 -- optimize the potential.** "What is not needed by (almost) everyone is
not what's not needed, to me." The enthusiast pushing past the tested
capabilities is a first-class constituency, because that is where discovery
comes from. Read it as: prefer the general mechanism to the fitted one, even
when only the fitted one is currently asked for [`Sec.2.0`, D10].

**D11 -- 1 ms/frame.** The global timing goal is 1 ms/frame soft-realtime for
the engine's work on the reference hardware, so older hardware keeps up and
enthusiasts keep margin. Cost claims use that denominator, not a
percentage of an unstated total [`Sec.2.0`, D11].

**D12 -- acting defaults are logged.** A default that ACTS is logged; only
inaction may be silent. If your code picks a value on the user's behalf and
that choice does something, it says so in the log [`Sec.2.0`, D12].

**D13 -- downgrade must stay possible.** Reverting to an older build must not
break a working install, as long as the newer features and formats are not
actively used -- so a format change is additive by default [`Sec.2.0`, D13].

**D14 -- every source file is pure ASCII.** Accents are removed to get there
[`Sec.2.0`, D14]. It governs source BYTES only: user-visible output may
genuinely carry non-ASCII, and the way to write such a literal is a `\xNN`
escape, same bytes [`Sec.11.189`, clause (c)]. The standing check is
`python3 claude/harness/f70_ascii.py gate`, which fails on a non-ASCII file in
a path the partition table does not classify
[`claude/harness/f70_partition.tsv`].

## 5. Build, run, test

**Build and install:** `INSTALL`, sections 3 and 4 -- clone with
`--recurse-submodules`, then `sh install_src.sh -j<n>`, which erases `build/`,
configures Release, compiles and runs `sudo cmake --install` into
`/usr/local`; `--update` rebuilds without erasing. New source files are picked
up automatically because the source list is globbed with `CONFIGURE_DEPENDS`
[`src/CMakeLists.txt:3`]. Read `INSTALL` section 4's last paragraph before
installing anywhere else: the data directory is fixed at compile time as
`CONFIG_DATA_DIR` in `src/spacecrafter.hpp`, so `-DCMAKE_INSTALL_PREFIX`
changes where files are copied and not where the program looks.

**What the repository does not contain:** no textures, no star catalogues, no
sky cultures, no landscapes, no fonts, no audio, no scripts, no 3D models, no
translations, no videos [`INSTALL`, section 5]. A tree install is the binary,
the compiled shaders and eleven small metadata files [`Sec.11.204`]. The
program still starts without the content and still logs "Completed copy of
textures" whether the copy worked or not [`INSTALL`, section 5], so an empty
`~/.spacecrafter/textures/` is the thing to check, not the log.

Where the content comes from is **not** an open question about whether it
exists: by default only limited catalogues are loaded, and the correct ones
are loaded by "an outside installation procedure" [owner,
`claude/USER_QUESTIONS_ROUND3.md` R23, committed at `c5be42b` 2026-09-05; the
commit does not record whether he was relaying the main tester].
**This repository does not document that procedure**, and where it lives and
who owns it is the one thing still to ask the owner. Same source:
`~/.spacecrafter/stars/` is where catalogue files *should* go but is not where
they are today ("We should but for now it is in another directory", R24), so
it is not a search path you can rely on.

**The harness:** `claude/harness/README.md`, whose "Run" section is the entry
point [`:40-49`]; scripts default their binary to
`build-claude/src/spacecrafter` and take an `SC_BIN` override
[`claude/README.md:21-23`]. The per-task sections below it document each
instrument and, more usefully, the gotchas each one cost a measurement.

**The smoke suite** is one command and one exit code --
`DISPLAY=:2 claude/harness/f90_rehearsal_run.sh <absOutdir>` drives a launch on
a private farm through the shipped command surface (launch, author a body, run
a shipped show, search, select and read out, save, reload, one keyboard ramp,
quit), prints every step's observable and pass criterion *before* it runs, and
exits non-zero on any failed step [`Sec.11.211`].

**scedit** (`util/scedit/`) is a standalone script and stellar-system-file
editor and checker, deliberately not wired into the spacecrafter build, C++17
with everything vendored [`util/scedit/README.md:35-43`]:

    cd util/scedit
    cmake -B build && cmake --build build
    cd build && ctest --output-on-failure

There are **nineteen** ctest gates [`util/scedit/README.md:767`, table at
`:774-794`]. Four run `--check` over real corpora -- the tracked scripts, the
installed script package, the tracked stellar-system files and the field ones
-- and two of those report a SKIP, never a pass, when their input is not on
the machine [`:767-772`]. Exit codes: 0 clean, 1 findings, 2 usage or
unreadable input [`:297-303`].

**Measurement discipline**, in five lines -- each one exists because ignoring
it produced a wrong number at least once:

1. **Fresh launch**, started for the measurement, with `~/.spacecrafter`'s
   `config.ini` and `ssystem.ini` md5s asserted equal before and after
   [`claude/fable-dispatch.md` section 0.5].
2. **No concurrent instance**, asserted via `/proc/<pid>/comm` on any account
   -- concurrent sessions share `~/.spacecrafter`, and the older
   `pgrep -f <path>` pattern self-matches its own wrapper [`Sec.11.134` (b)].
3. **Run the environment canary first** (`claude/harness/f56_canary.sh`;
   `--no-scene` for functional work, full for photometric). A non-zero exit
   stops the measurement and gets reported -- never mitigated silently, never
   widened away [`Sec.11.176`].
4. **No absolute photometry across stacks.** Trust counter ratios and in-run
   A/B; a display-stack change invalidates cross-session absolutes
   [`Sec.11.123`].
5. **A green that could not have gone red is not evidence.** Name the check
   that would fail if the claim were false, and where you can, commit the
   prediction before the run [`Sec.11.159`].

## 6. Asking the ledger whether something is already known

`claude/INTENT.md` is the single authority. Section 2.0 holds the domain
constraints above; **section 5** is the defect register, one numbered row per
known defect with its state, nothing silently dropped; **section 11** is the
append-only investigation journal (what was measured, when, with what
instrument); **section 13** is the open-item ledger, where 13.A is suspended
for the owner -- blocked by protocol, not by dependencies -- and 13.B is
active work.

Large section 5 and 11 entries live in their own file `claude/INTENT/<id>.md`;
the line in `INTENT.md` is a **derived stub** and on divergence **the entry
file wins**, so a correction lands on the entry file first and only then on
the stub [`claude/INTENT.md` header].

**Searching.** Closed entries are archived to `claude/INTENT/archive/` and
references are never rewritten, so a lateral search must span live and archive
together -- grep `claude/INTENT.md`, `claude/INTENT/` and
`claude/INTENT/archive/` at once, never the live surface alone. To resolve a
reference, probe the stated path, then the same path with an `archive` component
inserted at the failing point. An ID gap in a live list is an archival marker, not
a loss [`claude/INTENT.md` header].

**Provenance tags.** Every ledger claim is tagged, and the tag is how much
weight it carries: `[stated: file:line]` explicit in source,
`[observed: file:line]` a code fact verified by reading, `[measured]` an
instrument result, `[inferred]` a reconstruction still needing the owner's
validation, `[vixy: date]` given directly by the owner and highest authority,
`[defect]` found during analysis [`claude/INTENT.md` header]. An untagged
sentence is not yet a fact.

**Answers from outside** arrive in named channels and nowhere else:
`claude/USER_QUESTIONS.md`, `claude/USER_QUESTIONS_ROUND2.md`,
`claude/USER_QUESTIONS_ROUND3.md` (tester and owner replies, kept inline),
`claude/SCRIPT_SURFACE.md` (script-surface divergences and decisions) and
`claude/FEATURE_REQUESTS.md` (requests from outside the tracker)
[`claude/INTENT.md` section 13.D]. These are views: on divergence the ledger
wins, and the divergence is itself a bug to fix.

## 7. Sharp edges

Read these before touching the code they live in.

- **`search` is deprecated.** The owner's word, verbatim: *"Search is
  deprecated."* [`claude/USER_QUESTIONS_ROUND3.md` R22, committed at
  `c5be42b`]. The open search-related defects are therefore not fix targets
  -- confirm the disposition before spending anything on them.
- **One script line can kill the app** [`Sec.5.92`]. `dso3d action restart
  maxobject 2` with no `depth` key runs `std::stoi` on an absent argument
  [`src/interfaceModule/app_command_interface.cpp:1688`; the row records
  `:1525`, which the code has since moved past] and nothing on the execution
  chain catches, so the process terminates. OPEN and deliberately
  unpatched: the guard belongs to a policy for numeric arguments across the
  whole command surface, and fixing this one site contradicts the class fix.
- **The orbit and anchor creator chain** [`Sec.5.127`]. Two of five members
  were fixed in September 2026; three remain OPEN, and the awkward one is a
  data-grammar fact rather than a patchable bug: `orbit_semimajoraxis` is
  KILOMETRES under `ell_orbit` and ASTRONOMICAL UNITS under `comet_orbit`, on
  both paths. Also open: `EllipticalOrbit::saveOrbit` applies AU twice, so a
  saved section reloaded is wrong by 1.496e8 [`src/bodyModule/orbit.cpp:600`,
  `:606`]; and the chain's end logs the wrong class name
  [`src/bodyModule/orbit_creator_cor.cpp:283`; the row records `:260`].
- **A NaN at startup on a cold `$HOME`** [`Sec.5.48`]. Body scaling is
  intermittently left non-finite at init, on whichever body the config scales;
  the guard for the same shape lives in `EntityCore`, which is read-only here.
  Re-measured at 0 of 6 fresh homes in September 2026 -- rare, not gone.
- **The app can fail to quit** [`Sec.5.59`]. `App::draw`
  [`src/appModule/app.cpp:823`] waits on the previous frame at `:831`, and
  that wait is an untimed, uncancellable `std::atomic` wait
  [`src/tools/draw_helper.cpp:405-418`]; teardown is serviced by the same main
  loop, so under load a shutdown request can simply not be serviced. If a run
  hangs on exit, this is the first hypothesis, not a mystery. (The row records
  that call at line 795, from before the file moved.)
- **A version bump rewrites the user's config**: upgrading to a build with a
  different version string makes the program rewrite
  `~/.spacecrafter/config.ini` by itself, dropping comments and deleting
  unknown keys [`INSTALL`, section 6]. Keep a copy before upgrading.

Every content class in the list above is in real field use, with one caveat
from the owner worth carrying: *"All have been tested, but sometimes long ago,
so maybe some features could have altered the way it shall work."*
[`claude/USER_QUESTIONS_ROUND3.md` R21, committed at `c5be42b`]. Read it as:
a feature being shipped is not evidence it still works.

## 8. Conventions for your own commits

- **Commit code first** when a change spans both repositories, so the harness
  trailer names a commit that exists [`claude/README.md:34-41`], and **never
  merge `CC-harness` into a code branch** [`:28-30`].
- **Always `git -C <explicit path>`.** The two repositories are nested;
  relying on the current directory is how commits land in the wrong one
  [`claude/CLAUDE.md`].
- **New source is pure ASCII** (D14). Run
  `claude/harness/f70_ascii.py` with the `gate` argument before you commit.
- **Nothing derived enters the harness repository** -- no compiled artifact,
  no measurement output. A pre-commit hook enforces it by content, not by
  name, installed with `claude/githooks/install.sh`; git config is not cloned,
  so a fresh clone runs no hook until you install it [`claude/README.md`].
- **Authorship**, observed rather than prescribed: the history carries both
  human and automated authors, the machine-authored commits under a model name
  with a `Co-Authored-By` trailer [observed: `git log --format='%an'`, both
  repos, 2026-09-05]. No document states a rule for human contributors, so use
  your own git identity.

## 9. Engineering principles

**Placeholder -- to be written by Calvin Ruiz, the project owner.**

The ledger argues from a set of engineering invariants it cites by number,
`I1` to `I7` -- you will meet them constantly in section 5 and section 11
reasoning [`claude/INTENT.md`; `I2` alone appears 33 times, measured
2026-09-05]. **Neither repository states what they say.** The text is the
owner's and lives outside both, so it is not restated here: a paraphrase of a
principle you cannot check against its author is worse than an empty section.
Until he fills this in, read an `I<n>` citation as a pointer to him.
