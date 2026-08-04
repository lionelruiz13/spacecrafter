# scedit — script & stellar-system-file editor for spacecrafter

## What it is for

A spacecrafter show is a `.sts` script: one command per line, each command a
word followed by key/value pairs. The engine reads such a line with a parser
that has several sharp edges and **no error reporting at all** — a misspelt flag
is a silent no-op, a trailing key with no value is silently dropped, a value the
engine does not recognise silently means "off". An author finds out in the dome.

scedit exists to move that discovery to the desk. It reads a script *exactly*
the way the engine will, and reports every place where what the engine will do
differs from what the author plainly meant — with the engine's own source line
named in each message. Around that core it grows into the editor Vixy asked for
(`claude/FEATURE_REQUESTS.md` [2026-07-29]): a mouse-driven TUI, autocomplete
with ghost-text preview, and a documentation line for the command and for the
key under the cursor, so that *someone with no knowledge of scripting can
understand and modify any script through this editor*.

Second artefact class, same tool: stellar-system data files (`ssystem.ini` and
the composed new-format files).

**Name note:** "TUI" inside spacecrafter means the in-app dome text menu
(`ui_tuiconf`, channel 8 in `claude/capability-surface.md`). This tool is
*scedit* everywhere, to keep the two unambiguous.

## Build

Standalone, deliberately not wired into the spacecrafter build (the `util/`
sibling pattern). C++17, no external dependency: nlohmann/json v3.11.3 is
vendored as a single header under `third_party/` (the `src/stb_image.h`
precedent).

    cd util/scedit
    cmake -B build && cmake --build build
    cd build && ctest --output-on-failure     # 5 gates, see "Verification"

## Use

    scedit [--grammar FILE] [--list FAMILY]          # default: validate the contract
    scedit [--grammar FILE] [--rules] --check FILE...

`--grammar FILE` — the contract to read. Default `grammar/sc-grammar.json`,
resolved relative to the working directory, so run from `util/scedit/` or pass
the path.

**default action — validate the contract.** Re-derives every family count from
the data and compares it against `_meta.expected_counts`, checks in-family
uniqueness, subfamily links, per-entry completeness (`registration`, `doc`,
`args_complete`), the argument-token count and roles, and lint-id uniqueness.
This is the *seed gate*: it catches an accidental edit of the contract file, not
a divergence from the engine (that is the extraction passes' job).

**`--list FAMILY`** — print one family, one name per line, for shell use.
Families: `commands`, `flags`, `set_names`, `color_names`, `obsolete_tokens`,
`reserved_variables`, `font_targets`.

**`--check FILE...`** — analyse scripts. Diagnostics go to **stdout** (they are
the product); tool failures go to stderr. Output is gcc-shaped, per decision D6:

    file:line: severity: message [-Wid]

Every id is a name from the contract file's `lint_seeds`, and so is its
severity — retuning a severity is a data edit, not a code change. Example:

    doc/superscript.sts:94: error: column 433 holds a no-break space, the ISO-8859
    spelling (byte 0xA0), not a space: the engine separates words on space, tab, CR,
    LF, VT and FF only, so what is written on either side of this byte is read as ONE
    word [-Winvisible-separator]

**`--rules`** — with `--check`, first print the rules this build does **not**
emit, and why. An unarmed rule is visible rather than silent: a check that
cannot be grounded in the contract at zero false positives is not armed at all,
and this is where you see which ones and what they are waiting for.

### Exit codes

| code | meaning |
|---|---|
| 0 | clean — no findings (or the validation/listing succeeded) |
| 1 | findings were reported (or the contract failed validation) |
| 2 | usage error, or a file could not be read / parsed |

## The grammar file

`grammar/sc-grammar.json` is the single machine-readable contract for
spacecrafter's command surface. Nothing in the code hard-codes a command, a
flag or a family member: a name that is not in the file does not exist for
scedit.

    _meta                        schema version, provenance, expected counts,
                                 count deltas against the dated census
    parse_model                  how a line becomes (command, args) — including
                                 every sharp edge, each with its engine line
    families.commands            60 registered + comment/uncomment, each with a
                                 doc line, its argument keys and their specs
    families.flags / set_names / color_names / obsolete_tokens /
    reserved_variables / font_targets
    argument_token_vocabulary    the 238 W_* spellings, with the role each plays
    lint_seeds                   diagnostic id, severity, rule, engine source

Two shapes coexist in the families on purpose. A family whose documentation
pass has run carries objects (`[{name, doc, value, default, required, source}]`
— `set_names` today, per decision D7); one whose pass has not run carries plain
names, and says so in its own `_schema_note`. Consumers must accept both.

Per-command, the field that governs `--check`'s argument vocabulary is
**`args_complete`**. It answers one question — *is the key list here the WHOLE
vocabulary this command accepts?* — because "I listed some keys" and "I listed
all of them" are different claims and only the second licenses calling a key
unknown. Four handlers hand their whole parsed map to another module; two of
them (`dso3d`, `landscape`) have that module's keys extracted and answer `true`,
two (`body`, `camera`, and `flyto` which is `camera`) answer `false` because
their downstream vocabulary is the stellar-system contract's to state. Those
last three are silent on their keys **by construction, not by omission**, and
`--rules` says so.

### Authority chain

**Until the engine emits this file, SOURCE WINS on any divergence:**

1. `src/interfaceModule/app_command_init.cpp` — registration (which names exist)
2. `src/interfaceModule/base_command_interface.hpp` — token spellings
3. `src/interfaceModule/app_command_interface.cpp` — the parser and the handlers

Every doc line, value domain and default in the file carries a `file:line`
anchor into one of those, or into the downstream module that actually reads the
key. Nothing is written from recall or inference: where the code cannot answer,
the entry is `null` and the question is listed in that entry's `flagged` array
for Vixy. `UNEXTRACTED`/`null` are the honest states (constraint C2).

The granular source of the per-key data is `grammar/args/unit-{1,2,3,4}.json`,
the four fragments of the per-handler extraction sweep. They stay in the tree
and each carries its own count gate (every `args[` line in its range mapped to
exactly one key); the merged contract cites them in `_meta.merge_provenance`.
**A correction goes into the fragment first, then into the merged file** — and
the seed gate compares the two on every run, so forgetting the second half is a
test failure rather than a slow divergence.

Target state, already seamed in the engine: `AppCommandInit` keeps
`commandList`/`flagList`/`colorList`/`setList` copies "to futur exploitation",
and `SessionFile::CommandSurface::forEach*` enumerates flags, values and colours
at runtime — so this file can become a build artefact the engine emits, and the
authority chain above collapses into one link.

## Verification

Five `ctest` gates, all green on a clean build:

| gate | what it measures |
|---|---|
| `tokenizer` | 150 constructed lines, one per sharp edge of the parse model, each with its expected tokenization |
| `parse_oracle` | scedit's reading vs a **verbatim copy of the engine's `parseCommand`**, over exhaustively enumerated short strings, ISO-8859 high-byte lines and every line of the real corpus: 53 058 comparisons |
| `seed_gate` | the contract file validates (counts re-derived from the data, not asserted) — **and every fact in the four `grammar/args/` fragments is still byte-identical in the merged file**, which is what keeps the granular source and the merged contract from drifting apart |
| `lint_rules` | `tests/lint_cases.sts` — one construct per armed id, proving the rule fires with the right id, severity and shape; plus a section that must stay silent |
| `corpus_gate` | `--check` over the real corpus produces exactly the recorded findings |

The last two compare against `tests/lint-expected.txt` and
`tests/corpus-expected.txt`. **Those files are a record, not a silencer**: every
line in them is dispositioned in `tests/derivation-diff.md` §7 with the engine
site named, and a finding that appears without being recorded there fails the
gate. That is constraint C3 — zero false positives before a rule ships — and it
is why the expected files are edited deliberately and never regenerated blind.

`tests/derivation-diff.md` is the audit that makes engine fidelity (C1) a
measurement instead of a claim: `parseCommand` line by line against the scedit
code that implements it, every contract clause mapped, and every engine
behaviour scedit does *not* reproduce raised as a question rather than taken as
a decision.

## Status

Landed: the contract file (schema v2, per-key argument data merged), the
tokenizer library (`src/sc_tokenizer.hpp` — also the TUI's cursor→token engine),
`--check` with its lint rules, and the five gates above.

Not yet: the stellar-system-file grammar (second contract file), `$`-variable
semantics for the `reserved_variables` family, the TCP client mode, and the
FTXUI front end. Roadmap, decisions and the open-question ledger:
`claude/util/scedit/INTENT.md` (harness repo).
