# F58 scope 1 — the audit universe, stated and bounded BEFORE any classification

Code repo at `master-beta @ d6aec251`. Every `file:line` in this audit resolves at that
commit. Product code is READ-ONLY throughout; nothing here proposes or applies a fix.

## 1. What §11.169 makes auditable

§11.169(b) gives two schemas and one scope floor:

- **error schema** (§2(f) refined) — **WHAT** the error is · **CONSEQUENCES** of the error
  · **PREVENTION**: the action the reading user takes so it does not happen (again);
- **acting-default schema** (§2.0 D12 refined) — **CAUSE** (what made the implicit
  decision necessary) · **CONTENT** (what was decided implicitly) · **OVERRIDE**: the
  action the reading user takes to define/override it explicitly;
- **self-containment** binds the action element of BOTH: usable *"not requiring deep
  understanding of the whole system"*, with the operational test — *"a field set to an
  invalid value must enumerate the valid values, or their SHAPE if not enumerable"*;
- **scope floor**, Vixy's own words: *"at least for user-facing error like scripts or
  datas"*. Wider application is unforbidden, not required.

§11.169(b) also states the double face: one invalid field value produces BOTH records
when the software proceeds, and **one log entry carrying all elements satisfies both**.
That last clause is why this audit scores an EMISSION EVENT, not a source line: where a
refusal is emitted as a pair of lines from one chokepoint (`Could not execute: <command>`
+ the message), both lines are one record and are scored together.

## 2. The universe

**Universe = the diagnostic-emission sites of the files below, on the three channels a
user can read, plus the silent sites enumerated by §4's method.**

The file list IS the boundary. It is committed inside the instrument
(`claude/harness/f58_census.py`, constant `UNIVERSE`) so that re-running the instrument
reproduces the site list exactly, and so that changing the boundary is a visible diff.

### arm A — the script/TCP command surface (the path a script line or a TCP/mkfifo command travels)

| file | why |
|---|---|
| `src/interfaceModule/app_command_interface.cpp` | the command dispatcher; owns `debug_message`, the refusal channel |
| `src/interfaceModule/app_command_eval.cpp` | argument/variable evaluation (`evalDouble`, reserved vars) |
| `src/interfaceModule/app_command_init.cpp` | the vocabulary tables + `searchNeighbour` ("did you mean") |
| `src/scriptModule/script.cpp` | script file open/tokenise/end |
| `src/scriptModule/script_mgr.cpp` | script lifecycle, recording, the per-line execution loop |
| `src/scriptModule/script_interface.cpp` | the façade (0 emission sites — recorded, not a gap by itself) |
| `src/tools/io.cpp` | the TCP server socket |
| `src/appModule/app.cpp` | `App::updateFromSharedData` — the TCP/mkfifo command INTAKE; also a config consumer |
| `src/appModule/mkfifo.cpp` | the mkfifo transport |

**Judgment call, stated:** the dispatch names arm (i) as *"script/TCP command handling
(`app_command_interface`/`app_command_eval` refusals + diagnostics)"*. Those two files are
a naming example, not an exhaustive list: `App::updateFromSharedData`
[`app.cpp:756-776`] is literally where a TCP or mkfifo command enters the engine, and the
arm cannot answer "what does a TCP client learn when its command is refused" without it.
Included with this note rather than cited from outside the boundary.

### arm B — the five NAMED data/config loader families

| family | files |
|---|---|
| ssystem | `src/bodyModule/solarsystem.cpp`, `ssystem_factory.cpp`, `protosystem.cpp` |
| config | `src/mainModule/checkConfig.cpp`, `src/tools/init_parser.cpp`, `src/tools/app_settings.cpp` |
| star | `src/starModule/hip_star_mgr.cpp`, `zone_array.cpp` |
| nebula | `src/coreModule/nebula_mgr.cpp` |
| sky-culture | `src/coreModule/sky_localizer.cpp`, `src/coreModule/constellation_mgr.cpp` |

Family → file mapping derived by reading each family's entry point, not by name matching.

### arm C — the ledger-banked instances

| instance | site |
|---|---|
| §11.170(f) — the camera line | `src/bodyModule/ssystem_factory.cpp:1084` (already inside arm B) |
| the illuminate clamp (F53 acceptance) | `src/coreModule/illuminate_mgr.cpp` |
| §5.115 — the script log's lifecycle and gate | `src/tools/log.cpp`, `src/main.cpp` |

## 3. What is OUT, and why (the scope floor, applied)

- **Internal/debug channels**: renderer, Vulkan/EntityCore, shader, `LOG_FILE::VULKAN`
  and `LOG_FILE::SHADER` producers. §11.169(b)'s floor is user-facing surfaces.
- **UI / media / ojm / experimental-module internals**: not a script-or-data surface.
- **Data loaders OUTSIDE the five named families** — landscape, milkyway, tully, dso3d,
  the in-galaxy star manager/navigator, starLines, textures, fonts, media, anchors,
  orbit creators, the UI. These ARE user-facing data surfaces and they ARE the natural
  extension of this audit; they are excluded because the mandate named five families and
  a boundary that drifts is not a boundary. They are **inventoried, not classified**, by
  the same instrument (`f58_census.py --adjacent` →
  `f58_adjacent_inventory.tsv`): **274 live emission sites over 20 files**, which is the
  size of the follow-on, measured rather than guessed.

## 4. Enumeration method (so the boundary is checkable)

`python3 claude/harness/f58_census.py --root /home/claude/spacecrafter`

The instrument is deliberately syntactic and complete over its file list:

1. **`cLog::get()->write(...)`** — balanced-paren extraction of the full call, including
   multi-line calls; argument 0 is the emitted text AS WRITTEN IN SOURCE, arguments 1–2
   the `LOG_TYPE` / `LOG_FILE` (defaults `L_INFO` / `INTERNAL` when omitted).
2. **`debug_message = …` / `+= …`** — the script command surface's refusal channel,
   emitted by `AppCommandInterface::executeCommandStatus` [`app_command_interface.cpp:1163-1178`].
3. **`std::cerr <<` / `std::cout <<`** — the C++ console channel, which bypasses `cLog`.
4. **`printf` / `fprintf` / `perror`** — the C console channel.

Sites whose line is commented out are captured and flagged (`commented`), not dropped:
one of them is evidence (`app_command_interface.cpp:311`).

Output: `f58_sites_raw.tsv`, **432 live sites** (arm A 257 · arm B 155 · arm C 20) out of
487 captured including commented-out ones.

**Three instrument faults were found and fixed WHILE censusing, each named with the site
that exposed it — recorded because an instrument that silently mis-measures its own
universe is this audit's own version of the defect it audits:**

| fault | exposed by | effect before the fix |
|---|---|---|
| statement end found with a bare `find(";")` | `app_command_interface.cpp:4110` | 4 messages truncated mid-sentence — the quoted text would have been wrong for exactly the four best records in arm A |
| only `//` comments detected, not `/* */` | `checkConfig.cpp:512` | 3 dead sites counted as live |
| only the C++ console channel enumerated | `zone_array.cpp:212` | 24 `printf`/`fprintf` fault reports invisible — the star-catalogue loader would have scored as nearly silent instead of as loud-on-the-wrong-channel |

Each fix was verified by diffing the census before and after: 4 rows changed, 3 removed,
24 added — the fixes touched exactly what they should and nothing else.

**Silent sites — the half no grep can find.** A missing diagnostic has no syntax. The
audit therefore enumerates silent sites by a stated READ method, and does NOT claim
completeness over them:

- **S1** the three ledger-banked instances (arm C above);
- **S2** every branch of the arm-A dispatch SPINE, read in full and line by line:
  `executeCommand` [:191-303], `setFlag(name,value,newval)` [:306-321],
  `setFlag(FLAG_NAMES,FLAG_VALUES)` [:323-340], `readFlag` [:350-…],
  `setFlag(FLAG_NAMES,FLAG_VALUES,bool&)` [:656-…], `convertStrToFlagValues`
  [:1154-1161], `executeCommandStatus` [:1163-1178], `commandFlag` [:1180-1198],
  `commandSet` / `parseCommandSet` / `evalCommandSet` [:2110-2232], and
  `AppCommandEval::evalDouble` / `evalInt` [`app_command_eval.cpp:116-133`] with
  `Utility::strToDouble` / `strToInt` / `strToBool` / `isTrue`
  [`utility.cpp:399-467`, `utility.hpp:160-171`]. These are the funnels every
  script command passes through, so a silent site here has maximal reach;
- **S3** silent branches found in the immediate context of an enumerated emission site
  while classifying it (recorded with their cites as they came).

Everything else silent is **named-not-swept**: a complete silent-site census needs a
per-branch read of ~60 command handlers and 13 loaders, which is a separate task and is
recorded as such rather than implied by this one.
