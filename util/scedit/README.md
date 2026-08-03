# scedit — script & stellar-system-file editor for spacecrafter

A standalone editor tool for `.sts` show scripts and stellar-system data
files, with a direct TCP mode against a running spacecrafter (port
`io:tcp_port_in`, shipped 7805). Target feature set (recorded in
`claude/FEATURE_REQUESTS.md` [2026-07-29], from Vixy): mouse-driven TUI,
autocomplete, static analysis reporting errors early, inline documentation
of the command under edit and of the key/value under the cursor with
defaults greyed out and offered as completion.

**Name note:** "TUI" inside spacecrafter means the in-app dome text menu
(`ui_tuiconf`, channel 8 in `claude/capability-surface.md`). This tool is
*scedit* everywhere, to keep the two unambiguous.

## Status — slice 1 (headless core)

Built now:
- `grammar/sc-grammar.json` — the machine-readable command-surface
  contract: parse model, all token families (60 commands / 97 flags /
  43 set / 46 color / 7 obsolete / 24 reserved vars / 10 font targets),
  argument-token vocabulary, and lint-rule seeds. Every entry carries its
  source anchor; per-command argument specs are explicitly UNEXTRACTED
  until the per-handler pass (no guessed content).
- `scedit` (CLI) — loads and self-validates the contract (count gates +
  uniqueness), `--list <family>` for shell use.

Not yet: tokenizer/analyzer (`--check`), ssystem-file grammar, TCP client,
FTXUI front end. Roadmap and decisions: `claude/util/scedit/INTENT.md`
(harness repo).

## Authority

Until the engine emits the grammar file, **source wins** on divergence:
`src/interfaceModule/app_command_init.cpp` (registration),
`base_command_interface.hpp` (spellings), `app_command_interface.cpp`
(parse + handlers). The seed was hand-extracted 2026-08-03 at
`master-beta @ d13681eb` and count-gated against the B38 census
(`claude/capability-surface.md`), deltas explained in `_meta`.

## Build & use

    cmake -B build && cmake --build build
    ./build/scedit                       # validate grammar (from util/scedit/)
    ./build/scedit --list flags          # print a family

Dependency: nlohmann/json v3.11.3, vendored single-header under
`third_party/` (same pattern as `src/stb_image.h` in the main tree).
