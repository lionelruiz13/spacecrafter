# F58 — the §11.169 gap table: summary

Universe + method: `f58_universe.md`. Table: `f58_gap_table.tsv`. Verdicts and element tests: `f58_gaptable.py` (self-asserting against the census — it exits non-zero if the verdict set and the site set differ).

Code `master-beta @ d6aec251`, product code read-only.

## Sites by class

| class | sites | meaning |
|---|---:|---|
| ERR | 263 | user-facing error record |
| INFO | 74 | progress/state; neither schema applies |
| TRACE | 45 | internal debug trace; outside the user-facing floor |
| ACT | 21 | acting-default record (D12) |
| BOTH | 17 | one record wearing both faces |
| CHAN | 8 | emits another site's record; owns no content |
| SILENT | 6 | schema applies, nothing emitted |
| NOISE | 4 | emitted where nothing went wrong and nothing was decided |
| DELIB | 2 | known-deliberate, cost-forced silence (§5.115's bulk class) |
| **total** | **440** | |

## Element coverage over the 309 schema-applicable sites

Y = present · P = partial · N = absent · – = the schema's other face.

| element | Y | P | N | – | Y-rate over sites where it applies |
|---|---:|---:|---:|---:|---:|
| WHAT (error) | 140 | 123 | 25 | 21 | 0.486 |
| CONSEQUENCES (error) | 20 | 18 | 250 | 21 | 0.069 |
| PREVENTION (error) | 17 | 42 | 229 | 21 | 0.059 |
| CAUSE (acting default) | 33 | 2 | 8 | 266 | 0.767 |
| CONTENT (acting default) | 19 | 12 | 12 | 266 | 0.442 |
| OVERRIDE (acting default) | 4 | 2 | 37 | 266 | 0.093 |
| self-containment of the action | 18 | 45 | 23 | 223 | 0.209 |

## Whole-schema results

| | sites | all three elements | zero elements |
|---|---:|---:|---:|
| error schema (ERR + BOTH) | 280 | 11 | 14 |
| acting-default schema (ACT + BOTH) | 38 | 4 | 0 |

The 11 error records carrying all three elements:

- `src/bodyModule/protosystem.cpp:637`
- `src/bodyModule/ssystem_factory.cpp:645`
- `src/bodyModule/ssystem_factory.cpp:708`
- `src/bodyModule/ssystem_factory.cpp:967`
- `src/bodyModule/ssystem_factory.cpp:997`
- `src/bodyModule/ssystem_factory.cpp:1005`
- `src/interfaceModule/app_command_interface.cpp:4110`
- `src/interfaceModule/app_command_interface.cpp:4115`
- `src/interfaceModule/app_command_interface.cpp:4125`
- `src/interfaceModule/app_command_interface.cpp:4130`
- `src/tools/io.cpp:722`

The 4 acting-default records carrying all three elements:

- `src/bodyModule/protosystem.cpp:530`
- `src/bodyModule/ssystem_factory.cpp:371`
- `src/bodyModule/ssystem_factory.cpp:520`
- `src/bodyModule/ssystem_factory.cpp:967`

## Where the records go (the channel question)

| sink | schema-applicable sites |
|---|---:|
| `LOG_FILE::SCRIPT` | 184 |
| `LOG_FILE::INTERNAL` | 76 |
| `(console)` | 37 |
| `(none)` | 8 |
| `LOG_FILE::TCP` | 4 |

| severity as emitted | schema-applicable sites |
|---|---:|
| `(via executeCommandStatus L_DEBUG)` | 157 |
| `LOG_TYPE::L_WARNING` | 51 |
| `LOG_TYPE::L_ERROR` | 44 |
| `(console)` | 37 |
| `LOG_TYPE::L_INFO` | 9 |
| `(none)` | 8 |
| `LOG_TYPE::L_DEBUG` | 3 |

## Per-file

| file | sites | ERR | ACT | BOTH | SILENT | DELIB | INFO | TRACE | NOISE | CHAN |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `src/appModule/app.cpp` | 19 | 0 | 2 | 0 | 1 | 0 | 16 | 0 | 0 | 0 |
| `src/appModule/mkfifo.cpp` | 9 | 4 | 0 | 0 | 0 | 0 | 5 | 0 | 0 | 0 |
| `src/bodyModule/protosystem.cpp` | 21 | 15 | 2 | 0 | 0 | 0 | 3 | 1 | 0 | 0 |
| `src/bodyModule/solarsystem.cpp` | 3 | 2 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| `src/bodyModule/ssystem_factory.cpp` | 18 | 10 | 2 | 2 | 0 | 0 | 2 | 2 | 0 | 0 |
| `src/coreModule/constellation_mgr.cpp` | 13 | 11 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| `src/coreModule/illuminate_mgr.cpp` | 4 | 2 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 |
| `src/coreModule/nebula_mgr.cpp` | 8 | 4 | 2 | 0 | 1 | 0 | 1 | 0 | 0 | 0 |
| `src/coreModule/sky_localizer.cpp` | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
| `src/interfaceModule/app_command_eval.cpp` | 8 | 1 | 4 | 0 | 1 | 0 | 2 | 0 | 0 | 0 |
| `src/interfaceModule/app_command_init.cpp` | 2 | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `src/interfaceModule/app_command_interface.cpp` | 194 | 160 | 0 | 11 | 1 | 0 | 14 | 2 | 0 | 6 |
| `src/main.cpp` | 15 | 5 | 0 | 0 | 0 | 0 | 10 | 0 | 0 | 0 |
| `src/mainModule/checkConfig.cpp` | 4 | 0 | 3 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| `src/navModule/anchor_manager.hpp` | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
| `src/scriptModule/script.cpp` | 7 | 2 | 0 | 0 | 0 | 0 | 1 | 4 | 0 | 0 |
| `src/scriptModule/script_mgr.cpp` | 13 | 4 | 0 | 0 | 0 | 0 | 8 | 1 | 0 | 0 |
| `src/starModule/hip_star_mgr.cpp` | 23 | 13 | 1 | 4 | 0 | 0 | 5 | 0 | 0 | 0 |
| `src/starModule/zone_array.cpp` | 25 | 23 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| `src/tools/app_settings.cpp` | 26 | 0 | 0 | 0 | 0 | 0 | 1 | 25 | 0 | 0 |
| `src/tools/init_parser.cpp` | 15 | 3 | 4 | 0 | 0 | 0 | 0 | 4 | 4 | 0 |
| `src/tools/io.cpp` | 8 | 1 | 1 | 0 | 0 | 0 | 0 | 6 | 0 | 0 |
| `src/tools/log.cpp` | 3 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 |
